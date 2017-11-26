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
#include "PiEye.h"

#include "PiEyeImpl.h"

PiEye::PiEye() : _impl(new PiEyeImpl()){
    
}

PiEye::~PiEye() {
    if (_impl != nullptr) {
        delete _impl;
        _impl = nullptr;
    }
}

void
PiEye::createCamera() {
    _impl->createCamera();
}

void
PiEye::destroyCamera() {
    _impl->destroyCamera();
}

void
PiEye::startVideo() {
    _impl->startVideo();
}

void
PiEye::stopVideo() {
    _impl->stopVideo();
}

void
PiEye::grabFrame(cv::Mat& data) {
    _impl->grabFrame(data);
}

void
PiEye::grabStill(cv::Mat& data) {
    _impl->grabStill(data);
}

void
PiEye::setSensorMode(const SensorMode mode) {
    _impl->setSensorMode(mode);
}

void
PiEye::setEncoding(const Encoding& encoding) {
	_impl->setEncoding(encoding);
}

const Encoding&
PiEye::getEncoding() const {
	return _impl->getEncoding();
}

void
PiEye::setShutterSpeed(unsigned short millis) {
    _impl->setShutterSpeed(millis);
}

void
PiEye::setIso(unsigned short iso) {
    _impl->setIso(iso);
}

void
PiEye::setAnalogGain(float gain) {
    _impl->setAnalogGain(gain);
}

void
PiEye::setDigitalGain(float gain) {
    _impl->setDigitalGain(gain);
}

void
PiEye::setWhiteBalanceMode(const AwbMode& mode) {
	_impl->setWhiteBalanceMode(mode);
}

void
PiEye::setWhiteBalanceGain(float redGain, float blueGain) {
    _impl->setWhiteBalanceGain(redGain, blueGain);
}

void
PiEye::setFpsRange(float minFps, float maxFps) {
    _impl->setFpsRange(minFps, maxFps);
}