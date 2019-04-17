/*------------------------------------------------------------------------------
 Copyright © 2016 Continuum

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

  a. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  b. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  c. Neither the name of Continuum nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

Created by Jared Kirschner 2016
------------------------------------------------------------------------------*/

#include <system_error>  // NOLINT(build/c++11)

#include "include/circular_buffer.h"

//-----------------------------------------------------------------------------
class CircularBufferCategory : public std::error_category
{
 public:
    static const std::error_category& Instance()
    {
        static CircularBufferCategory instance;
        return instance;
    }

 private:
    const char* name() const noexcept { return "Circular Buffer Error"; }
    std::string message(int error) const { return "N/A"; }
};

std::error_code make_error_code(CircularBufferError error) {
    return std::error_code(static_cast<int>(error), CircularBufferCategory::Instance());
}
std::error_condition make_error_condition(CircularBufferError error) {
    return std::error_condition(static_cast<int>(error), CircularBufferCategory::Instance());
}

