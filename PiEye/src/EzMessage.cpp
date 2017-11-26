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
#include "EzMessage.h"

#include <sys/time.h>

namespace {
    unsigned long long
    GetMicroTime() {
        struct timeval timeVal;
        gettimeofday(&timeVal, nullptr);
        const unsigned long long microTime = timeVal.tv_sec;
        return (microTime * 1000 * 1000) + timeVal.tv_usec;
    }
}

EzMessage::EzMessage(unsigned short level) : _file(nullptr), _level(level), _line(0), _microTime(GetMicroTime()) {
}

EzMessage::EzMessage(unsigned short level, const char* file, const unsigned int line) : 
        _file(file), _level(level), _line(line), _microTime(GetMicroTime()) {
}

unsigned short
EzMessage::getLevel() const {
    return _level;
}

void
EzMessage::setLevel(unsigned short level) {
    _level = level;
}

unsigned long long
EzMessage::getMicroTime() const {
    return _microTime;
}

void
EzMessage::setMicroTime(unsigned long long microTime) {
    _microTime = microTime;
}

const std::string&
EzMessage::getMessage() const {
    return _message;
}

void
EzMessage::setMessage(const std::string& message) {
    _message = message;
}

const char*
EzMessage::getFile() const {
    return _file;
}

unsigned int
EzMessage::getLine() const {
    return _line;
}

EzMessage&
EzMessage::operator<<(const char* value) {
    _message += value;
    return *this;
}

EzMessage&
EzMessage::operator<<(const std::string& value) {
	_message += value;
	return *this;
}

EzMessage&
EzMessage::operator<<(const bool value) {
	_message += value ? "true" : "false";
	return *this;
}

EzMessage&
EzMessage::operator<<(const unsigned int value) {
    _message += std::to_string(value);
    return *this;
}

EzMessage&
EzMessage::operator<<(const int value) {
    _message += std::to_string(value);
    return *this;
}

EzMessage&
EzMessage::operator<<(const unsigned long value) {
    _message += std::to_string(value);
    return *this;
}

EzMessage&
EzMessage::operator<<(const float value) {
	_message += std::to_string(value);
	return *this;
}