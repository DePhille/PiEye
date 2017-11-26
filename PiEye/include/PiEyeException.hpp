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

#include <stdexcept>
#include <interface/mmal/mmal_types.h>

/**
 * Used so that clients have the option to catch all exceptions coming from this library.
 */
class PiEyeException : public std::runtime_error {
public:
    PiEyeException(const std::string& message) : std::runtime_error(message) {
    }
};

class StatusException : public PiEyeException {
public:
    StatusException(const MMAL_STATUS_T& status, const std::string& message) : PiEyeException(message), _status(status) {
    }
    
private:
    const MMAL_STATUS_T _status;
};

class StateException : public PiEyeException {
public:
	StateException(const std::string& message) : PiEyeException(message) {
	}
};

class TimeOutException : public PiEyeException {
public:
	TimeOutException(const std::string& message) : PiEyeException(message) {
	}
};