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
#include "EzLogger.h"

#include <iostream>

#include "Log.hpp"

EzLogger* EzLogger::_instance = nullptr;

namespace {
    const char*
    GetLevelStr(unsigned short level) {
        switch (level) {
        case EZLOG_LEVEL_TRACE:
            return "TRACE";
        case EZLOG_LEVEL_DEBUG:
            return "DEBUG";
        case EZLOG_LEVEL_INFO:
            return "INFO";
        case EZLOG_LEVEL_WARNING:
            return "WARN";
        case EZLOG_LEVEL_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }
    
    std::string
    GetDateStr(const unsigned long long microTime) {
        char buf[64];
        long secPart = microTime / 1000 / 1000;
        long microPart = microTime - (secPart * 1000 * 1000);
        if (microPart < 0) {
            secPart--;
            microPart += 1000 * 1000;
        }
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&secPart));
        const std::string millis = std::to_string(microPart / 1000);
        
        return std::string(buf) + "." + std::string(3 - millis.size(), '0') + millis;
    }
}

EzLogger&
EzLogger::GetDefault() {
    if (_instance == nullptr) {
        _instance = new EzLogger();
    }
    return *_instance;
}

void
EzLogger::log(const EzMessage& message) {
    std::cout << GetDateStr(message.getMicroTime()) << " - "
        << "[" << GetLevelStr(message.getLevel()) << "]";
    if (message.getFile()) {
        std::cout << " - " << message.getFile() << ":" << message.getLine() << " - ";
    } else {
        std::cout << ":";
    }
    std::cout << message.getMessage() << std::endl;
}
