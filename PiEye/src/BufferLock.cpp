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
#include "BufferLock.h"

#include <interface/mmal/mmal.h>
#include "PiEyeException.hpp"
#include "Log.hpp"

BufferLock::BufferLock(MMAL_BUFFER_HEADER_T* buffer) {
    EZLOG_TRACE("Locking buffer");
    const MMAL_STATUS_T status = mmal_buffer_header_mem_lock(buffer);
    if (status == MMAL_SUCCESS) {
        _buffer = buffer;
    } else {
        throw PiEyeException("Unable to lock buffer");
    }
}

BufferLock::~BufferLock() {
    unlock();
}

void
BufferLock::unlock() {
    if (_buffer != nullptr) {
        EZLOG_TRACE("Unlocking buffer");
        mmal_buffer_header_mem_unlock(_buffer);
        _buffer = nullptr;
    }
}
