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

#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <thread>   // NOLINT(build/c++11)

#include "include/uart.h"
#include "include/gpio_interface.h"
#include "include/system_interface.h"
#include "include/system_file_interface.h"

//-----------------------------------------------------------------------------
const char * UARTLoopback::uart_device_path_ = "/dev/ttyAL0";
const char * UARTttyAL1::uart_device_path_ = "/dev/ttyAL1";
const char * UARTttyAL2::uart_device_path_ = "/dev/ttyAL2";
const char * UARTttyS1::uart_device_path_ = "/dev/ttyS1";

//-----------------------------------------------------------------------------
std::error_code UART::Initialize() const
{
    if (initialized_)
        return std::error_code();

    // File open flags:
    // O_RDWR - read/write access to file
    // O_NOCTTY - don't make this terminal device the controlling terminal device for the process
    // O_NDELAY - force calls to read to return immediately if no bytes available, rather than blocking
    auto open_result = system_file_.Open(GetDevicePath(), O_RDWR | O_NOCTTY | O_NDELAY);

    std::error_code error = open_result.error;
    if (error)
        return error;

    fd_uart_ = open_result.handle;

    error = Configure();
    if (error)
        return error;

    initialized_ = true;

    return std::error_code();
}

//-----------------------------------------------------------------------------
std::error_code UART::Close() const
{
    bool was_initialized = initialized_;
    initialized_ = false;

    auto error_to_return = system_file_.Close(fd_uart_);

    // If wasn't initialized, this is likely the cause of any above errors.
    // If was initialized, return any above error.
    if (!was_initialized)
        return UARTError::UART_UNINITIALIZED;
    else
        return error_to_return;
}

//-----------------------------------------------------------------------------
std::error_code UART::Read(char *buffer, size_t nbyte) const
{
    if (!initialized_)
        return UARTError::UART_UNINITIALIZED;

    return system_file_.Read(fd_uart_, buffer, nbyte);
}

//-----------------------------------------------------------------------------
std::error_code UART::Write(const char *buffer, size_t nbyte) const
{
    if (!initialized_)
        return UARTError::UART_UNINITIALIZED;

    return system_file_.Write(fd_uart_, buffer, nbyte);
}

//-----------------------------------------------------------------------------
std::error_code UART::Configure() const
{
    struct termios tty;

    if (system_.Tcgetattr(fd_uart_, &tty) < 0)
        return std::error_code(errno, std::system_category());

    // Set baud rate (output and input)
    if (system_.Cfsetispeed(&tty, GetBaudRate()) < 0)
        return std::error_code(errno, std::system_category());

    if (system_.Cfsetospeed(&tty, GetBaudRate()) < 0)
        return std::error_code(errno, std::system_category());

    // Cause input terminal to make input available character by character, with echoing disabled,
    // and all special processing of terminal input and output characters disabled
    system_.Cfmakeraw(&tty);

    // According to http://man7.org/linux/man-pages/man3/termios.3.html,
    // with VMIN > 0 and VTIME > 0:
    // Once an initial byte of input becomes available, the timer is
    // restarted after each further byte is received, and read returns when:
    // VMIN bytes have been received, or the interbyte timer expires.
    // If data is already available at the time of the call to read, the call
    // behaves as though the data was received immediately after the call

    tty.c_cc[VMIN] = 1;    // Min # characters for non-canonical read
    tty.c_cc[VTIME] = 10;  // Timeout in deciseconds for non-canonical read

    tty.c_cflag &= ~static_cast<tcflag_t>(CSTOPB);   // 1 stop bit
    tty.c_cflag &= ~static_cast<tcflag_t>(CRTSCTS);  // no hardware flow control

    // According to: https://www.cmrr.umn.edu/~strupp/serial.html
    // The c_cflag member contains two options that should always be enabled,
    // CLOCAL and CREAD. These will ensure that your program does not become the 'owner'
    // of the port subject to sporadic job control and hangup signals, and also that the
    // serial interface driver will read incoming data bytes.
    tty.c_cflag |= (static_cast<tcflag_t>(CLOCAL) | static_cast<tcflag_t>(CREAD));

    // Set UART parameters immediately
    if (system_.Tcsetattr(fd_uart_, TCSANOW, &tty) < 0)
        return std::error_code(errno, std::system_category());

    return std::error_code();
}

//-----------------------------------------------------------------------------
std::error_code UARTttyS1::Write(const char *buffer, size_t nbyte) const
{
    if (!initialized_)
        return UARTError::UART_UNINITIALIZED;

    ssize_t signed_nbyte = static_cast<ssize_t>(nbyte);
    while (signed_nbyte > 0)
    {
        ssize_t nbyte_written = system_.Write(fd_uart_, buffer, static_cast<size_t>(signed_nbyte));

        // An error has occurred
        if (nbyte_written < 0)
        {
            // If the error was a signal interrupt, try again. Otherwise, abort.
            if ((errno == EINTR) || (errno == EAGAIN))
                continue;
            else
                return std::error_code(errno, std::system_category());
        }

        // If some bytes were written, increment the pointer and continue,
        // until there are no bytes left to write
        signed_nbyte -= nbyte_written;
        buffer += nbyte_written;
    }

    return std::error_code();
}
