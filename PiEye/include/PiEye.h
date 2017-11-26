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

#include "SensorMode.hpp"
#include "Encoding.hpp"
#include "AwbMode.hpp"

namespace cv {
    class Mat;
}

class PiEyeImpl;

class PiEye {
public:

    PiEye();
    ~PiEye();

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
    PiEyeImpl* _impl;

};