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
#include "Wait.h"

#include <mutex>
#include <condition_variable>

#include "PiEyeException.hpp"
#include "Log.hpp"

Wait::Wait() : _mutex(new std::mutex()), _condition(new std::condition_variable()) {
}

Wait::~Wait() {
	if (_mutex) {
		delete _mutex;
		_mutex = nullptr;
	}
	
	if (_condition) {
		delete _condition;
		_condition = nullptr;
	}
}

void
Wait::wait(unsigned short seconds) {
	EZLOG_TRACE("Waiting...");
	std::unique_lock<std::mutex> lock(*_mutex);
	_wait = true;
	
	while(_wait) {
		if (_condition->wait_for(lock, std::chrono::seconds(seconds)) == std::cv_status::timeout) {
			throw TimeOutException("Time out occurred");
		}
	}
	EZLOG_TRACE("Done waiting");
}

void
Wait::notify() {
	EZLOG_TRACE("Notifying");
	std::unique_lock<std::mutex> lock(*_mutex);
	_wait = false;
	_condition->notify_all();
	EZLOG_TRACE("Notified");
}