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
#pragma once

#include <set>
#include <interface/mmal/mmal_types.h>
#include "SensorMode.hpp"
#include "Encoding.hpp"
#include "AwbMode.hpp"
#include "Wait.h"

namespace cv {
    class Mat;
}

struct MMAL_COMPONENT_T;
struct MMAL_PORT_T;
struct MMAL_POOL_T;
struct MMAL_PARAMETER_CAMERA_SETTINGS_T;
struct MMAL_BUFFER_HEADER_T;

class PiEyeImpl {
public:

    PiEyeImpl();
    ~PiEyeImpl();

    void
    createCamera();
    
    void
    destroyCamera();
	
	void
	startVideo();
	
	void
	stopVideo();
	
	void
	grabFrame(cv::Mat& data);
    
	void
	grabStill(cv::Mat& data);
    
    void
    setSensorMode(const SensorMode mode);
	
	void
	setEncoding(const Encoding& encoding);
	
	const Encoding&
	getEncoding() const;
	
	void
	setShutterSpeed(unsigned short millis);

    void
    setIso(unsigned short iso);
    
    void
    setAnalogGain(float gain);
    
    void
    setDigitalGain(float gain);
	
	void
	setWhiteBalanceMode(const AwbMode& mode);
    
    void
    setWhiteBalanceGain(float redGain, float blueGain);
    
    void
    setFpsRange(float minFps, float maxFps);
    
private:
    MMAL_COMPONENT_T* _camera = nullptr;
	MMAL_PORT_T* _previewPort = nullptr;
    MMAL_PORT_T* _videoPort = nullptr;
	MMAL_PORT_T* _stillPort = nullptr;
	MMAL_POOL_T* _videoPool = nullptr;
	MMAL_POOL_T* _stillPool = nullptr;
	Encoding _encoding = Encoding::NATIVE_BGR;
    unsigned short _width = 1280;
    unsigned short _height = 720;
    unsigned short _previewFrames = 3;
    unsigned short _fps = 0;
	Wait _videoWait;
	Wait _stillWait;
	std::set<cv::Mat*> _frameRequests;
	cv::Mat* _stillRequest = nullptr;
    
    static void
    ControlCallback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer);

    static void
    BufferCallback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer);
    
    void
    parseVideoBuffer(const MMAL_BUFFER_HEADER_T& buffer);
	
	void
	parseStillBuffer(const MMAL_BUFFER_HEADER_T& buffer);
    
    void
    setCameraConfig();
    
    void
    setFormat(MMAL_PORT_T& port, unsigned short fps);
    
    void
    setParameter(MMAL_PORT_T* port, unsigned int parameter, bool value);
    
    void
    setParameter(MMAL_PORT_T* port, unsigned int parameter, unsigned int value);
    
    void
    setParameter(MMAL_PORT_T* port, unsigned int parameter, float value);
    
    void
    requireCamera();
	
	void
	decodeBuffer(const MMAL_BUFFER_HEADER_T& buffer, cv::Mat& target);

	void
	initStill();
	
	void
	closeStill();
};