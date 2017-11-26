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

#include <interface/mmal/mmal_types.h>
#include <interface/mmal/mmal_encodings.h>
#include "PiEyeException.hpp"

#define PIEYE_RATIONAL_SCALE 1000

std::string
StatusToString(const MMAL_STATUS_T& status) {
	switch (status) {
	case MMAL_SUCCESS:
		return "Success";
		break;
	case MMAL_ENOMEM:
		return "Out of memory";
		break;
	case MMAL_ENOSPC:
		return "Out of resources";
		break;
	case MMAL_EINVAL:
		return "Argument is invalid";
		break;
	case MMAL_ENOSYS:
		return "Function not implemented";
		break;
	case MMAL_ENOENT:
		return "No such file or directory";
		break;
	case MMAL_ENXIO:
		return "No such device or address";
		break;
	case MMAL_EIO:
		return "I/O error";
		break;
	case MMAL_ESPIPE:
		return "Illegal seek";
		break;
	case MMAL_ECORRUPT:
		return "Data is corrupt";
		break;
	case MMAL_ENOTREADY:
		return "Component is not ready";
		break;
	case MMAL_ECONFIG:
		return "Component is not configured";
		break;
	case MMAL_EISCONN:
		return "Port is already connected";
		break;
	case MMAL_ENOTCONN:
		return "Port is disconnected";
		break;
	case MMAL_EAGAIN:
		return "Resource temporarily unavailable. Try again later";
		break;
	case MMAL_EFAULT:
		return "Bad address";
		break;
	case MMAL_STATUS_MAX:
		return "Force to 32 bit";
		break;
	}
	
	return "UNKNOWN";
}

void
CheckStatus(const MMAL_STATUS_T& status, const std::string& message) {
    if (status != MMAL_SUCCESS) {
        throw StatusException(status, message + ": " + StatusToString(status));
    }
}

std::string
ParameterToString(const unsigned int parameter) {
    switch (parameter) {
        case MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG:
            return "CAMERA_CUSTOM_SENSOR_CONFIG";
            break;
        case MMAL_PARAMETER_CAPTURE:
            return "CAPTURE";
            break;
        case MMAL_PARAMETER_SHUTTER_SPEED:
            return "SHUTTER_SPEED";
            break;
        case MMAL_PARAMETER_ISO:
            return "ISO";
            break;
        case MMAL_PARAMETER_ANALOG_GAIN:
            return "ANALOG_GAIN";
            break;
        case MMAL_PARAMETER_DIGITAL_GAIN:
            return "DIGITAL_GAIN";
            break;
        case MMAL_PARAMETER_CUSTOM_AWB_GAINS:
            return "CUSTOM_AWB_GAINS";
            break;
        case MMAL_PARAMETER_FPS_RANGE:
            return "FPS_RANGE";
            break;
    }
    return "Unknown parameter [" + std::to_string(parameter) + "]";
}

MMAL_RATIONAL_T ToRational(float value) {
    const float scaledValue = value * PIEYE_RATIONAL_SCALE;
    return {(int) scaledValue, PIEYE_RATIONAL_SCALE};
}
