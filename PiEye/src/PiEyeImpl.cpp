/* MIT License

Copyright (c) 2017 Philippe Beckers

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "PiEyeImpl.h"

#include <iostream>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <interface/mmal/mmal.h>
#include <interface/mmal/util/mmal_default_components.h>
#include <interface/mmal/util/mmal_connection.h>
#include <interface/mmal/util/mmal_util_params.h>
#include <interface/mmal/util/mmal_util.h>

#include "BufferLock.h"
#include "PiEyeException.hpp"
#include "Util.hpp"
#include "Log.hpp"

#define PIEYE_PORT_PREVIEW 0
#define PIEYE_PORT_VIDEO 1
#define PIEYE_PORT_STILL 2
#define PIEYE_MIN_VIDEO_BUFFERS (unsigned int)3
#define PIEYE_MIN_STILL_BUFFERS (unsigned int)3

namespace {
    void
    LogCameraSettings(const MMAL_PARAMETER_CAMERA_SETTINGS_T& settings) {
		EZLOG_DEBUG("Exposure [" << settings.exposure << "], "
					<< "analog gain [" << settings.analog_gain.num << "/" << settings.analog_gain.den << "], "
					<< "digital gain [" << settings.digital_gain.num << "/" << settings.digital_gain.den << "], "
					<< "red gain [" << settings.awb_red_gain.num << "/" << settings.awb_red_gain.den << "], "
					<< "blue gain [" << settings.awb_blue_gain.num << "/" << settings.awb_blue_gain.den << "], "
					<< "focus pos [" << settings.focus_position << "]");
    }
	
	unsigned int
	GetMmalEncoding(const Encoding& encoding) {
		switch(encoding) {
			case Encoding::NATIVE_BGR:
				return MMAL_ENCODING_BGR24;
			case Encoding::NATIVE_GRAYSCALE:
				return MMAL_ENCODING_I420;
		}
		
		throw PiEyeException("Encoding not supported");
	}
}

PiEyeImpl::PiEyeImpl() {
    
}

PiEyeImpl::~PiEyeImpl() {
    try {
        destroyCamera();
    } catch (const std::exception& e) {
        EZLOG_ERROR("Unable to destroy camera: " << e.what());
    } catch (...) {
        EZLOG_ERROR("Unable to destroy camera.");
    }
}

void
PiEyeImpl::createCamera() {
    MMAL_STATUS_T status;
    
    // Skip if already open
    if (_camera) {
        return;
    }

    try {
        // Create component
        status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &_camera);
        CheckStatus(status, "Unable to create default camera component");
        if (_camera == nullptr) {
            throw PiEyeException("Created camera is a nullptr");
        } else if (!_camera->control) {
            throw PiEyeException("Camera has no control port");
        } else if (!_camera->output_num) {
            throw PiEyeException("Camera has no output ports");
        }
        
        // Enable camera control callback
        const MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T changeEvent = {{MMAL_PARAMETER_CHANGE_EVENT_REQUEST, sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T)},
            MMAL_PARAMETER_CAMERA_SETTINGS, true};
        status = mmal_port_parameter_set(_camera->control, &changeEvent.hdr);
        CheckStatus(status, "Unable to register change event on camera control port");
        
        // Set camera config
        setCameraConfig();
        
        // Enable camera control port
        status = mmal_port_enable(_camera->control, ControlCallback);
        CheckStatus(status, "Unable to enable camera control port");
        
        // Enable camera
        status = mmal_component_enable(_camera);
        CheckStatus(status, "Unable to enable camera");
		
    } catch (...) {
        destroyCamera();
        throw;
    }
}

void
PiEyeImpl::destroyCamera() {
    EZLOG_TRACE("Destroying camera");
    MMAL_STATUS_T status;
    stopVideo();
	closeStill();
    if (_camera) {
        
        // Disable
        EZLOG_TRACE("Disabling camera");
        if (_camera->is_enabled) {
            status = mmal_component_disable(_camera);
            CheckStatus(status, "Unable to disable camera");
        }
        
        // Destroy
        EZLOG_TRACE("Destroying camera component");
        status = mmal_component_destroy(_camera);
        CheckStatus(status, "Unable to destroy camera component");
        _camera = nullptr;
    }
    EZLOG_TRACE("Camera destroyed!");
}

void
PiEyeImpl::startVideo() {
	EZLOG_TRACE("Opening video port");
    MMAL_STATUS_T status;

    // Skip if already started
    if (_videoPort != nullptr) {
		EZLOG_TRACE("Video already opened");
        return;
    }
    
    // Check if camera is opened and video is available
    if (_camera == nullptr) {
        throw StateException("Cannot start video before camera was created");
    } else if (_camera->output_num <= PIEYE_PORT_VIDEO || _camera->output[PIEYE_PORT_VIDEO] == nullptr) {
        throw PiEyeException("Video port is not available");
    }
    
    try {
        // Configure
        _videoPort = _camera->output[PIEYE_PORT_VIDEO];
        _videoPort->userdata = (struct MMAL_PORT_USERDATA_T*) this;
        setFormat(*_videoPort, _fps);
        setFormat(*_camera->output[1], _fps); // TODO - should not be here
        
        // Make sure enough buffers are available for video and preview
        if (_videoPort->buffer_num < PIEYE_MIN_VIDEO_BUFFERS) {
            _videoPort->buffer_num = PIEYE_MIN_VIDEO_BUFFERS;
        }
        
        // Enable video port
		EZLOG_TRACE("Enabling video port");
        status = mmal_port_enable(_videoPort, BufferCallback);
        CheckStatus(status, "Unable to enable video port");
        
        // Create buffer pool
		EZLOG_TRACE("Creating video pool with [" << _videoPort->buffer_num << "] buffers of [" << _videoPort->buffer_size << "] bytes");
        _videoPool = mmal_port_pool_create(_videoPort, _videoPort->buffer_num, _videoPort->buffer_size);
        if (!_videoPool) {
            throw PiEyeException("Unable to allocate video pool");
        }
        
        // Inject all buffers
        const int bufferCount = mmal_queue_length(_videoPool->queue);
		EZLOG_TRACE("Injecting [" << bufferCount << "] buffers in video port");
        for(int i = 0; i < bufferCount; ++i) {
            MMAL_BUFFER_HEADER_T* buffer = mmal_queue_get(_videoPool->queue);

            if (!buffer) {
                throw PiEyeException("Unable to get buffer");
            }
            
            status = mmal_port_send_buffer(_videoPort, buffer);
            CheckStatus(status, "Unable to send a buffer to the video port");
        }
        
        // Go!
		EZLOG_TRACE("Enabling capture parameter on video port");
        setParameter(_videoPort, MMAL_PARAMETER_CAPTURE, true);
        
		EZLOG_TRACE("Video enabled successfully");
    } catch (...) {
		EZLOG_WARN("Could not enable video port");
        stopVideo();
        throw;
    }
}

void
PiEyeImpl::stopVideo() {
	if (_videoPort == nullptr) {
		EZLOG_TRACE("Video already stopped");
	} else {
		EZLOG_TRACE("Stopping video");
		
        // Disable video port
        if (_videoPort->is_enabled) {
			EZLOG_TRACE("Disabling video port");
            const MMAL_STATUS_T status = mmal_port_disable(_videoPort);
            CheckStatus(status, "Unable to disable video port");
        }
        
        // Free video pool
        if (_videoPool) {
			EZLOG_TRACE("Destroying video pool");
            mmal_port_pool_destroy(_videoPort, _videoPool);
            _videoPool = nullptr;
        }
	
		_videoPort = nullptr;	
		EZLOG_TRACE("Video stopped");
    }
}

void
PiEyeImpl::grabFrame(cv::Mat& data) {
	// TODO: Check if video started
	EZLOG_TRACE("Grabbing a frame");
    _frameRequests.insert(&data);
    _videoWait.wait(30);
	EZLOG_TRACE("Grabbed a frame");
}

void
PiEyeImpl::grabStill(cv::Mat& data) {
	EZLOG_DEBUG("Grabbing still");
	
	// Check if camera is opened and still port is available
    if (_camera == nullptr) {
        throw StateException("Cannot take still before camera was created");
    } else if (_camera->output_num <= PIEYE_PORT_STILL || _camera->output[PIEYE_PORT_STILL] == nullptr) {
        throw PiEyeException("Still port is not available");
    }
    
    try {
		initStill();
		_stillRequest = &data;
        
        // Go!
		EZLOG_TRACE("Enabling capture parameter on still port");
        setParameter(_stillPort, MMAL_PARAMETER_CAPTURE, true);
		
        // Wait...
		EZLOG_TRACE("Waiting for the callback...");
		_stillWait.wait(5);
		EZLOG_TRACE("Grabbed a still");
		_stillRequest = nullptr;
        
    } catch (...) {
		EZLOG_ERROR("Something went wrong while taking still");
        closeStill();
        throw;
    }
}

void
PiEyeImpl::setSensorMode(const SensorMode mode) {
    requireCamera();
    setParameter(_camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, (unsigned int) mode);
}

void
PiEyeImpl::setEncoding(const Encoding& encoding) {
	_encoding = encoding;
}

const Encoding&
PiEyeImpl::getEncoding() const {
	return _encoding;
}

void
PiEyeImpl::setShutterSpeed(unsigned short millis) {
    requireCamera();
    const unsigned int micros = 1000 * millis;
    setParameter(_camera->control, MMAL_PARAMETER_SHUTTER_SPEED, micros);
}

void
PiEyeImpl::setIso(unsigned short iso) {
    requireCamera();
    setParameter(_camera->control, MMAL_PARAMETER_ISO, (unsigned int) iso);
}

void
PiEyeImpl::setAnalogGain(float gain) {
    requireCamera();
    setParameter(_camera->control, MMAL_PARAMETER_ANALOG_GAIN, gain);
}

void
PiEyeImpl::setDigitalGain(float gain) {
    requireCamera();
    setParameter(_camera->control, MMAL_PARAMETER_DIGITAL_GAIN, gain);
}

void
PiEyeImpl::setWhiteBalanceMode(const AwbMode& mode) {
	requireCamera();
	MMAL_PARAM_AWBMODE_T awbMode = MMAL_PARAM_AWBMODE_AUTO;
	switch (mode) {
		case AwbMode::OFF:
			awbMode = MMAL_PARAM_AWBMODE_OFF;
			break;
		case AwbMode::AUTO:
			awbMode = MMAL_PARAM_AWBMODE_AUTO;
			break;
		case AwbMode::SUNLIGHT:
			awbMode = MMAL_PARAM_AWBMODE_SUNLIGHT;
			break;
		case AwbMode::CLOUDY:
			awbMode = MMAL_PARAM_AWBMODE_CLOUDY;
			break;
		case AwbMode::SHADE:
			awbMode = MMAL_PARAM_AWBMODE_SHADE;
			break;
		case AwbMode::TUNGSTEN:
			awbMode = MMAL_PARAM_AWBMODE_TUNGSTEN;
			break;
		case AwbMode::FLUORESCENT:
			awbMode = MMAL_PARAM_AWBMODE_FLUORESCENT;
			break;
		case AwbMode::INCANDESCENT:
			awbMode = MMAL_PARAM_AWBMODE_INCANDESCENT;
			break;
		case AwbMode::FLASH:
			awbMode = MMAL_PARAM_AWBMODE_FLASH;
			break;
		case AwbMode::HORIZON:
			awbMode = MMAL_PARAM_AWBMODE_HORIZON;
			break;
	}
	
	const MMAL_PARAMETER_AWBMODE_T param = {{MMAL_PARAMETER_AWB_MODE, sizeof(param)}, awbMode};
	const MMAL_STATUS_T status = mmal_port_parameter_set(_camera->control, &param.hdr);
	CheckStatus(status, "Unable to set AWB mode");
}

void
PiEyeImpl::setWhiteBalanceGain(float redGain, float blueGain) {
    requireCamera();
    const MMAL_PARAMETER_AWB_GAINS_T param = {{MMAL_PARAMETER_CUSTOM_AWB_GAINS,sizeof(param)},
        ToRational(redGain), ToRational(blueGain)};
    const MMAL_STATUS_T status = mmal_port_parameter_set(_camera->control, &param.hdr);
    CheckStatus(status, "Unable to set gains for AWB");
}

void
PiEyeImpl::setFpsRange(float minFps, float maxFps) {
    requireCamera();
    const MMAL_PARAMETER_FPS_RANGE_T fpsRange = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fpsRange)},
        ToRational(minFps), ToRational(maxFps)};
    MMAL_STATUS_T status = mmal_port_parameter_set(_camera->output[0], &fpsRange.hdr); // TODO: Use preview port instead of 0
    CheckStatus(status, "Unable to set FPS range on preview port");
	
	// TODO: Put this somewhere else, or will we always sync video and preview ports?
	status = mmal_port_parameter_set(_videoPort, &fpsRange.hdr);
	CheckStatus(status, "Unable to set FPS range on video port");
}


void
PiEyeImpl::ControlCallback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
    if (buffer == nullptr) {
        EZLOG_WARN("Got empty buffer from camera control");
    } else if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED) {
        MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
        if (param == nullptr) {
            EZLOG_WARN("Got invalid parameter changed event from camera control");
        } else if (param->hdr.id == MMAL_PARAMETER_CAMERA_SETTINGS) {
            MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
            LogCameraSettings(*settings);
        } else {
            EZLOG_WARN("Received weird parameter update [" << param->hdr.id << "]");
        }
    } else {
        EZLOG_WARN("Received unexpected camera control callback event [" << buffer->cmd << "]");
    }

    mmal_buffer_header_release(buffer);
}

void
PiEyeImpl::BufferCallback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
    BufferLock bufferLock(buffer);
    PiEyeImpl* instance = (PiEyeImpl*) port->userdata;
    EZLOG_DEBUG("Encoder callback: [" << buffer->length << "] bytes");
	MMAL_POOL_T* bufferPool = nullptr;
    if (instance == nullptr) {
        throw PiEyeException("Unable to determine PiEyeImpl instance in callback");
    } else if (buffer == nullptr) {
        throw PiEyeException("Received an invalid buffer");
    } else if (port == instance->_videoPort){
        instance->parseVideoBuffer(*buffer);
		bufferPool = instance->_videoPool;
    } else if (port == instance->_stillPort) {
		instance->parseStillBuffer(*buffer);
		bufferPool = instance->_stillPool;
	} else {
        EZLOG_WARN("Received a buffer from an unknown port");
	}
    bufferLock.unlock();
	
	// Only release buffer back to pool if it came from one of our pools
	if (bufferPool) {
		mmal_buffer_header_release(buffer);
		
		// Re-inject buffer
		if (port->is_enabled) {
			EZLOG_TRACE("Re-injecting a buffer");
			MMAL_BUFFER_HEADER_T* nextBuffer = mmal_queue_get(bufferPool->queue);
			if (nextBuffer) {
				const MMAL_STATUS_T status = mmal_port_send_buffer(port, nextBuffer);
				CheckStatus(status, "Unable to send buffer back to port");
			} else {
				EZLOG_WARN("Unable to get a new buffer from pool");
			}
		}
	}
    
	EZLOG_TRACE("BufferCallback completed");
}

void
PiEyeImpl::parseVideoBuffer(const MMAL_BUFFER_HEADER_T& buffer) {
	// Skip empty buffer
	if (buffer.length == 0) {
		EZLOG_DEBUG("Skipping empty buffer");
		return;
	}
	
    // Loop over target buffers:
    const std::set<cv::Mat*>::const_iterator requestEnd = _frameRequests.end();
    for (std::set<cv::Mat*>::iterator it = _frameRequests.begin(); it != requestEnd; ++it) {
        if (*it != nullptr) {
            try {
				decodeBuffer(buffer, **it);
            } catch (const PiEyeException& e) {
                std::cout << "Error occurred, skipping a frame request: " << e.what() << std::endl;
            }
        }
    }
    
    _videoWait.notify();
}

void
PiEyeImpl::parseStillBuffer(const MMAL_BUFFER_HEADER_T& buffer) {
	// Skip empty buffer
	if (buffer.length == 0) {
		EZLOG_DEBUG("Skipping empty buffer");
		return;
		
	// Only if pending request
	} else if (_stillRequest == nullptr) {
        EZLOG_WARN("Cannot parse still buffer as there is no target to store it");
		return;
	}
	decodeBuffer(buffer, *_stillRequest);
	
	EZLOG_TRACE("Notifying still waits");
	_stillWait.notify();
}

void
PiEyeImpl::setCameraConfig() {
    if (_camera == nullptr) {
        throw PiEyeException("Tried to set camera config while camera is not loaded yet");
    }
    MMAL_PARAMETER_CAMERA_CONFIG_T config = {{MMAL_PARAMETER_CAMERA_CONFIG, sizeof(config)}};
    config.max_stills_w = _width;
    config.max_stills_h = _height;
    config.stills_yuv422 = 0;
    config.one_shot_stills = 1;
    config.max_preview_video_w = _width;
    config.max_preview_video_h = _height;
    config.num_preview_video_frames = _previewFrames;
    config.stills_capture_circular_buffer_height = 0;
    config.fast_preview_resume = 0;
    //config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RAW_STC;
    config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC;
    const MMAL_STATUS_T status = mmal_port_parameter_set(_camera->control, &config.hdr);
    CheckStatus(status, "Unable to set camera configuration");
}

void
PiEyeImpl::setFormat(MMAL_PORT_T& port, unsigned short fps) {
    // Get format from port
    if (port.format == nullptr) {
        throw PiEyeException("Format for a port not available");
    }
    MMAL_ES_FORMAT_T& format = *port.format;
    
    // Fill details
	format.encoding = GetMmalEncoding(_encoding);
    format.encoding_variant = GetMmalEncoding(_encoding);
    format.es->video.width = _width;
    format.es->video.height = _height;
    format.es->video.crop.x = 0;
    format.es->video.crop.y = 0;
    format.es->video.crop.width = _width;
    format.es->video.crop.height = _height;
    format.es->video.frame_rate.num = fps;
    format.es->video.frame_rate.den = 1;
    
    // Set
    const MMAL_STATUS_T status = mmal_port_format_commit(&port);
    CheckStatus(status, "Unable to set format for preview port");
}

void
PiEyeImpl::setParameter(MMAL_PORT_T* port, unsigned int parameter, bool value) {
    if (port == nullptr) {
        throw PiEyeException("Cannot set boolean parameter on a NULL port");
    } else {
		EZLOG_DEBUG("Setting parameter [" << ParameterToString(parameter) << "] to [" << value << "]");
        const MMAL_STATUS_T status = mmal_port_parameter_set_boolean(port, parameter, value);
        CheckStatus(status, "Unable to set boolean parameter [" + ParameterToString(parameter) + "] to [" + std::to_string(value) + "]");
    }
}

void
PiEyeImpl::setParameter(MMAL_PORT_T* port, unsigned int parameter, unsigned int value) {
    if (port == nullptr) {
        throw PiEyeException("Cannot set unsigned int parameter on a NULL port");
    } else {
		EZLOG_DEBUG("Setting parameter [" << ParameterToString(parameter) << "] to [" << value << "]");
        const MMAL_STATUS_T status = mmal_port_parameter_set_uint32(port, parameter, value);
        CheckStatus(status, "Unable to set unsigned int parameter [" + ParameterToString(parameter) + "] to [" + std::to_string(value) + "]");
    }
}

void
PiEyeImpl::setParameter(MMAL_PORT_T* port, unsigned int parameter, float value) {
    if (port == nullptr) {
        throw PiEyeException("Cannot set rational parameter on a NULL port");
    } else {
		EZLOG_DEBUG("Setting parameter [" << ParameterToString(parameter) << "] to [" << value << "]");
        const MMAL_STATUS_T status = mmal_port_parameter_set_rational(port, parameter, ToRational(value));
        CheckStatus(status, "Unable to set rational parameter [" + ParameterToString(parameter) + "] to [" + std::to_string(value) + "]");
    }
}

void
PiEyeImpl::requireCamera() {
    if (_camera == nullptr) {
        throw StateException("Camera must be created first");
    } else if (_camera->control == nullptr) {
        throw PiEyeException("Control port not available");
    }
}

void
PiEyeImpl::decodeBuffer(const MMAL_BUFFER_HEADER_T& buffer, cv::Mat& target) {
	switch (_encoding) {
		case Encoding::NATIVE_BGR:
			target.create(_height, _width, CV_8UC3);
			break;
		case Encoding::NATIVE_GRAYSCALE:
			target.create(_height, _width, CV_8UC1);
			break;
	}
	
	unsigned long dataSize = target.total() * target.elemSize();
	if (dataSize > buffer.length) {
		throw PiEyeException("Buffer size [" + std::to_string(buffer.length) + "] too small for image size [" + std::to_string(dataSize) + "]");
	} else if (target.isContinuous()) {
		memcpy(target.ptr<uchar>(0), buffer.data + buffer.offset, dataSize);
	} else {
		// TODO: Implement row by row memcpy
        EZLOG_WARN("Image not continuous, not even bothering");
	}
}

void
PiEyeImpl::initStill() {
	if (_stillPort != nullptr) {
		EZLOG_TRACE("Still port already open");
		return;
	}
	
	EZLOG_DEBUG("Initializing still port");
	requireCamera();
	
	// Configure
	_stillPort = _camera->output[PIEYE_PORT_STILL];
	_stillPort->userdata = (struct MMAL_PORT_USERDATA_T*) this;
	setFormat(*_stillPort, 0);
	//setFormat(*_camera->output[1], _fps); // TODO - should not be here
	
	// Make sure enough buffers are available
	unsigned short minBufferCount = std::max(PIEYE_MIN_STILL_BUFFERS, _stillPort->buffer_num_recommended);
	if (_stillPort->buffer_num < minBufferCount) {
		_stillPort->buffer_num = minBufferCount;
	}
	
	// Enable port
	EZLOG_TRACE("Enabling still port");
	MMAL_STATUS_T status = mmal_port_enable(_stillPort, BufferCallback);
	CheckStatus(status, "Unable to enable still port");
	
	// Create buffer pool
	EZLOG_TRACE("Creating still pool with [" << _stillPort->buffer_num << "] buffers of [" << _stillPort->buffer_size << "] bytes");
	_stillPool = mmal_port_pool_create(_stillPort, _stillPort->buffer_num, _stillPort->buffer_size);
	if (!_stillPool) {
		throw PiEyeException("Unable to allocate still pool");
	}
    
    // Inject all buffers
    const int bufferCount = mmal_queue_length(_stillPool->queue);
    EZLOG_TRACE("Injecting [" << bufferCount << "] buffers in still port");
    for (int i = 0; i < bufferCount; ++i) {
        MMAL_BUFFER_HEADER_T* buffer = mmal_queue_get(_stillPool->queue);

        if (!buffer) {
            throw PiEyeException("Unable to get buffer");
        }
        
        status = mmal_port_send_buffer(_stillPort, buffer);
        CheckStatus(status, "Unable to send a buffer to the still port");
    }
}

void
PiEyeImpl::closeStill() {
    EZLOG_TRACE("CloseStill");
	if (_stillPort == nullptr) {
		EZLOG_TRACE("Still port already closed");
	} else {
		EZLOG_TRACE("Closing still port");
		
		// Set capture off:
		EZLOG_TRACE("Turning off still capture parameter");
		setParameter(_stillPort, MMAL_PARAMETER_CAPTURE, false);
		
        // Disable still port
        if (_stillPort->is_enabled) {
			EZLOG_TRACE("Disabling still port");
            const MMAL_STATUS_T status = mmal_port_disable(_stillPort);
            CheckStatus(status, "Unable to disable still port");
        }
        
        // Free pool
        if (_stillPool) {
			EZLOG_TRACE("Destroying still pool");
            mmal_port_pool_destroy(_stillPort, _stillPool);
            _stillPool = nullptr;
        }
		
		_stillPort = nullptr;
		EZLOG_TRACE("Still port closed");
    }
}
