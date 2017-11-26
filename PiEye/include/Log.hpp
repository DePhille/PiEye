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

#include "EzLogger.h"
#define EZLOG_LEVEL_ERROR 5
#define EZLOG_LEVEL_WARNING 4
#define EZLOG_LEVEL_INFO 3
#define EZLOG_LEVEL_DEBUG 2
#define EZLOG_LEVEL_TRACE 1

#ifndef EZLOG_LEVEL
#define EZLOG_LEVEL EZLOG_LEVEL_TRACE
#endif

#define EZLOG_LOG(level, msg)                       \
{                                                   \
    EzMessage message(level, __FILE__, __LINE__);   \
    message << msg;                                 \
    EzLogger::GetDefault().log(message);            \
}

#if EZLOG_LEVEL <= EZLOG_LEVEL_ERROR
#define EZLOG_ERROR(msg) EZLOG_LOG(EZLOG_LEVEL_ERROR, msg)
#else
#define EZLOG_ERROR(msg)
#endif

#if EZLOG_LEVEL <= EZLOG_LEVEL_WARNING
#define EZLOG_WARN(msg) EZLOG_LOG(EZLOG_LEVEL_WARNING, msg)
#else
#define EZLOG_WARN(msg)
#endif

#if EZLOG_LEVEL <= EZLOG_LEVEL_INFO
#define EZLOG_INFO(msg) EZLOG_LOG(EZLOG_LEVEL_INFO, msg)
#else
#define EZLOG_INFO(msg)
#endif

#if EZLOG_LEVEL <= EZLOG_LEVEL_DEBUG
#define EZLOG_DEBUG(msg) EZLOG_LOG(EZLOG_LEVEL_DEBUG, msg)
#else
#define EZLOG_DEBUG(msg)
#endif

#if EZLOG_LEVEL <= EZLOG_LEVEL_TRACE
#define EZLOG_TRACE(msg) EZLOG_LOG(EZLOG_LEVEL_TRACE, msg)
#else
#define EZLOG_TRACE(msg)
#endif