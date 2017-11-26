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
#include <chrono>
#include <thread>

#include <opencv2/opencv.hpp>

#include <PiEye.h>
#include <Log.hpp>

struct CameraSetting {
	unsigned short analogGain;
	unsigned short shutterSpeed;
};

unsigned short calcAvgIntensity(const cv::Mat& image) {
	// Timer begin
	const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	
	// Analyze img
	unsigned long totalPixel = 0;
	const uchar* imageData = image.ptr<uchar>(0);
	for (int i = 0; i < (image.rows * image.cols); ++i) {
		totalPixel += imageData[i];
	}
	
	// Calculate average intensity
	const unsigned short avgIntensity = totalPixel / (image.rows * image.cols);
	
	// Timer end
	const std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
	unsigned int millis = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
	EZLOG_INFO("Found average intensity [" << avgIntensity << "] in [" << millis << "] msec");
	
	return avgIntensity;
}

int main(int argc, char* argv[]) {
	try {
        const unsigned short isoSteps = 5;
        const unsigned short shutterSteps = 9;
		
		// Prepare some settings
		std::vector<CameraSetting> settings;
		settings.resize(isoSteps * shutterSteps);
		unsigned short iso = 1;
		for (unsigned short isoIndex = 0; isoIndex < isoSteps; ++isoIndex) {
			unsigned short shutterSpeed = 10;
			for (unsigned short ssIndex = 0; ssIndex < shutterSteps; ++ssIndex) {
				CameraSetting& setting = settings.at((isoIndex * shutterSteps) + ssIndex);
				setting.analogGain = iso;
				setting.shutterSpeed = shutterSpeed;
				shutterSpeed *= 2;
			}
			iso *= 2;
		}
		
		
		// Start
        EZLOG_INFO("Starting");
		PiEye cam;
		cam.createCamera();
		//cam.setSensorMode(SensorMode::V2_BINNED_169);
		cam.setEncoding(Encoding::NATIVE_GRAYSCALE);
        cam.setSensorMode(SensorMode::V2_AUTO);
		cam.startVideo();
		
		// Run
		EZLOG_INFO("Started");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		//cam.setFpsRange(0.05, 1);
		//cam.setShutterSpeed(10);
		//cam.setIso(1);
		cam.setWhiteBalanceMode(AwbMode::OFF);
		cam.setWhiteBalanceGain(1.0, 1.0);
		//std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		cv::Mat img;
		
		for (auto&& setting : settings) {
			//EZLOG_INFO("grabbing still with ISO [" << setting.iso << "] and shutter speed [" << setting.shutterSpeed << "]");
			EZLOG_INFO("grabbing still with analog gain [" << setting.analogGain << "] and shutter speed [" << setting.shutterSpeed << "]");
			cam.setShutterSpeed(setting.shutterSpeed);
			//cam.setIso(setting.iso);
			cam.setAnalogGain(setting.analogGain);
            
            // Grab still twice, first one is usually still with old settings
			cam.grabStill(img);
            cam.grabStill(img);
			//cv::imwrite("still_" + std::to_string(setting.iso) + "x" + std::to_string(setting.shutterSpeed) + ".jpg", img);
			cv::imwrite("still_" + std::to_string(setting.analogGain) + "x" + std::to_string(setting.shutterSpeed) + ".jpg", img);
		}
		
		unsigned int counter = 0;
		while (false) {
			cam.grabFrame(img);
			//cv::imwrite("frame_" + std::to_string(counter) + ".jpg", img);
			counter++;
		}
		
		// Stop
		EZLOG_INFO("Stopping");
		//cam.stopVideo();
		EZLOG_INFO("Stopped");
		//std::this_thread::sleep_for(std::chrono::milliseconds(20000));
		EZLOG_INFO("Destroy camera");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		cam.destroyCamera();
		EZLOG_INFO("Camera destroyed");
	} catch (const std::exception& e) {
		EZLOG_WARN("Something went wrong: " << e.what());
	} catch (...) {
		EZLOG_WARN("Unknown error");
	}
	
	// Exit
	EZLOG_INFO("Exiting");
    return 0;
}