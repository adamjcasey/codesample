/*------------------------------------------------------------------------------
 Copyright © 2017 Continuum

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

Created by Adam Casey 2017
------------------------------------------------------------------------------*/

#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "include/agg_wrapper.h"
#include "include/gui_system_colors.h"
#include "include/gui_context.h"

#include "vendor/linux/include/i2c-dev.h"

//-----------------------------------------------------------------------------
uint8_t GUIContextTFT::buffer_renderer_[RENDERER_BUFFER_SIZE];
const char GUIContextTFT::device_path_[] = "/dev/fb0";

//-----------------------------------------------------------------------------
std::error_code GUIContextTFT::Initialize() const
{
    if (initialized_)
        return std::error_code();

    // Get the pointer to the framebuffer
    auto open_result = system_file_.Open(device_path_, O_RDWR);
    if (open_result.error)
        return open_result.error;

    fd_framebuffer_ = open_result.handle;

    buffer_hardware_ = reinterpret_cast<uint8_t *>(system_.Mmap(
        nullptr, HARDWARE_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_framebuffer_, 0));
    if (buffer_hardware_ == MAP_FAILED)
        return std::error_code(errno, std::system_category());

    initialized_ = true;

    return std::error_code();
}

//-----------------------------------------------------------------------------
std::error_code GUIContextTFT::Close() const
{
    initialized_ = false;

    auto error_to_return = std::error_code();

    // A segmentation fault will occur if munmap is called on an invalid pointer
    if (buffer_hardware_ != MAP_FAILED)
    {
        int ret = system_.Munmap(buffer_hardware_, HARDWARE_BUFFER_SIZE);
        if (ret < 0)
            error_to_return = std::error_code(errno, std::system_category());
        buffer_hardware_ = static_cast<uint8_t*>(MAP_FAILED);
    }

    auto error = system_file_.Close(fd_framebuffer_);
    if (error)
        error_to_return = error;

    return error_to_return;
}

//-----------------------------------------------------------------------------
void GUIContextTFT::Clear() const
{
    if (!initialized_)
        return;

    GUI::RenderingBuffer rbuf(Buffer(), Width(), Height(), Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    renderer_buffer.clear(GUI::Color(GUISystemColors::DarkBlue));
}

//-----------------------------------------------------------------------------
void GUIContextTFT::ForceRedraw() const
{
    GUIContextTFT::ForceRedraw(0, 0, WIDTH - 1, HEIGHT - 1);
}

//-----------------------------------------------------------------------------
void GUIContextTFT::ForceRedraw(int x0, int y0, int x1, int y1) const
{
    if (!initialized_)
        return;

    // Brute force transform
    int x0_trans = y0;
    int x1_trans = y1;
    int y0_trans = WIDTH - 1 - x1;
    int y1_trans = WIDTH - 1 - x0;

    for (int b = y0_trans; b <= y1_trans; b++)
    {
        for (int a = x0_trans; a <= x1_trans; a++)
        {
            int x = WIDTH - 1 - b;
            int y = a;
            int hardware_index = (b * HEIGHT * 4) + (a * 4);
            int renderer_index = (y * WIDTH * 3) + (x * 3);
            buffer_hardware_[hardware_index + 0] = buffer_renderer_[renderer_index + 2];
            buffer_hardware_[hardware_index + 1] = buffer_renderer_[renderer_index + 1];
            buffer_hardware_[hardware_index + 2] = buffer_renderer_[renderer_index + 0];
            buffer_hardware_[hardware_index + 3] = 0xFF;
        }
    }
}

//-----------------------------------------------------------------------------
void GUIContextTFT::SetPixelDirectly(uint16_t x, uint16_t y, uint32_t rgbx) const
{
    if (!initialized_)
        return;

    if (x >= WIDTH)
        return;

    if (y >= HEIGHT)
        return;

    uint32_t *p = reinterpret_cast<uint32_t *>(&buffer_hardware_[(y * HEIGHT * 4) + (x * 4)]);
    *p = rgbx;
}

//-----------------------------------------------------------------------------
void GUIContextTFT::SetPixelRegionDirectly(
    uint16_t x_start, uint16_t y_start,
    uint16_t x_width, uint16_t y_height,
    uint32_t * rgbx_buffer) const
{
    // NOTE: rgbx_buffer is expected to be a pointer to the start of a 2D array:
    // int32_t rgbx_buffer[y_height][x_width]

    if (!initialized_)
        return;

    if ((x_start + x_width) >= WIDTH)
        return;

    if ((y_start + y_height) >= HEIGHT)
        return;

    auto y_end = y_start + y_height;

    // For each pixel row, get the starting index, and iterate over the row,
    // writing the appropriate rgbx_buffer value in each spot
    for (uint16_t y = y_start; y < y_end; y++)
    {
        uint32_t * pixel_buffer = reinterpret_cast<uint32_t *>(&buffer_hardware_[((y * WIDTH) + x_start) * 4]);
        uint32_t * pixel_buffer_row_end = pixel_buffer + x_width;

        for (; pixel_buffer < pixel_buffer_row_end; pixel_buffer++, rgbx_buffer++)
        {
            *pixel_buffer = *rgbx_buffer;
        }
    }
}

//-----------------------------------------------------------------------------
uint8_t GUIContextDVI::buffer_renderer_[RENDERER_BUFFER_SIZE];

//-----------------------------------------------------------------------------
const char GUIContextDVI::device_path_[] = "/dev/fb1";

//-----------------------------------------------------------------------------
const char GUIContextDVI::control_device_path_[] = "/dev/i2c-1";

//-----------------------------------------------------------------------------
const GUIContextDVI::DVIControlCommand GUIContextDVI::initialization_[] =
{
    // The GUIContextDVI::DVIControlCommand structures are initialized in this order:
    // { address, data, mask }
    { 0x41, 0x00, 0x40 },         // Power Down Register, normal operation: x0xxxxxx

    { 0x98, 0x03, 0xFF },         // Fixed register for proper operation
    { 0x9A, 0xE0, 0xFE },         // Fixed register for proper operation
    { 0x9C, 0x30, 0xFF },         // Fixed register for proper operation
    { 0x9D, 0x01, 0xFF },         // Fixed register for proper operation
    { 0xA2, 0xA4, 0xFF },         // Fixed register for proper operation
    { 0xA3, 0xA4, 0xFF },         // Fixed register for proper operation
    { 0xE0, 0xD0, 0xFF },         // Fixed register for proper operation
    { 0xF9, 0x00, 0xFF },         // Fixed register for proper operation

    { 0x15, 0x00, 0x0F },         // Video Format Id: 4:4:4
    { 0x16, 0x30, 0xB1 },         // Input Color Depth: 8 bit color depth
    { 0x17, 0x02, 0x02 }          // Aspect Ratio: 16x9
};

//-----------------------------------------------------------------------------
std::error_code GUIContextDVI::Initialize() const
{
    if (initialized_)
        return std::error_code();

    // Get the pointer to the framebuffer
    auto open_result = system_file_.Open(device_path_, O_RDWR);
    if (open_result.error)
        return open_result.error;

    fd_framebuffer_ = open_result.handle;

    buffer_hardware_ = reinterpret_cast<uint8_t *>(system_.Mmap(
        nullptr, HARDWARE_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_framebuffer_, 0));
    if (buffer_hardware_ == MAP_FAILED)
        return std::error_code(errno, std::system_category());

    // Open the I2C port that connects to the DVI controller IC
    auto open_result_i2c = system_file_.Open(control_device_path_, O_RDWR);
    if (open_result_i2c.error)
        return open_result_i2c.error;

    fd_i2c_ = open_result_i2c.handle;

    // Set the slave address
    auto ioctl_ret = system_.Ioctl(fd_i2c_, I2C_SLAVE, CONTROLLER_ADDRESS);
    if (ioctl_ret < 0)
        return std::error_code(errno, std::system_category());

    // Send all commands
    size_t num_commands = sizeof(initialization_) / sizeof(DVIControlCommand);
    for (size_t i = 0; i < num_commands; i++)
    {
        // Read the existing value, and only write to those bits we wish to change according to the mask
        int ret_read = system_.SMBusReadByteData(fd_i2c_, initialization_[i].address);
        if (ret_read < 0)
            return std::error_code(errno, std::system_category());

        // Read succeeded, so extract the data byte
        uint8_t data = static_cast<unsigned int>(ret_read) & 0xFF;

        // Write the updated data
        uint8_t write_data = (data & ~initialization_[i].mask) | (initialization_[i].data & initialization_[i].mask);
        int ret_write = system_.SMBusWriteByteData(fd_i2c_, initialization_[i].address, write_data);
        if (ret_write < 0)
            return std::error_code(errno, std::system_category());
    }

    initialized_ = true;

    return std::error_code();
}

//-----------------------------------------------------------------------------
void GUIContextDVI::Clear() const
{
    if (!initialized_)
        return;

    GUI::RenderingBuffer rbuf(Buffer(), Width(), Height(), Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    renderer_buffer.clear(GUI::Color(GUISystemColors::DarkBlue));
}

//-----------------------------------------------------------------------------
void GUIContextDVI::ForceRedraw() const
{
    GUIContextDVI::ForceRedraw(0, 0, WIDTH - 1, HEIGHT - 1);
}

//-----------------------------------------------------------------------------
void GUIContextDVI::ForceRedraw(int x0, int y0, int x1, int y1) const
{
    if (!initialized_)
        return;

    // Brute force transform
    for (int y = y0; y <= y1; y++)
    {
        for (int x= x0; x <= x1; x++)
        {
            buffer_hardware_[(y * WIDTH * 4) + (x * 4) + 0] = buffer_renderer_[(y * WIDTH * 3) + (x * 3) + 2];
            buffer_hardware_[(y * WIDTH * 4) + (x * 4) + 1] = buffer_renderer_[(y * WIDTH * 3) + (x * 3) + 1];
            buffer_hardware_[(y * WIDTH * 4) + (x * 4) + 2] = buffer_renderer_[(y * WIDTH * 3) + (x * 3) + 0];
            buffer_hardware_[(y * WIDTH * 4) + (x * 4) + 3] = 0xFF;
        }
    }
}

//-----------------------------------------------------------------------------
void GUIContextDVI::SetPixelDirectly(uint16_t x, uint16_t y, uint32_t rgbx) const
{
    if (!initialized_)
        return;

    if (x >= WIDTH)
        return;

    if (y >= HEIGHT)
        return;

    uint32_t *p = reinterpret_cast<uint32_t *>(&buffer_hardware_[((y * WIDTH) + x) * 4]);
    *p = rgbx;
}

//-----------------------------------------------------------------------------
void GUIContextDVI::SetPixelRegionDirectly(
    uint16_t x_start, uint16_t y_start,
    uint16_t x_width, uint16_t y_height,
    uint32_t * rgbx_buffer) const
{
    // NOTE: rgbx_buffer is expected to be a pointer to the start of a 2D array:
    // int32_t rgbx_buffer[y_height][x_width]

    if (!initialized_)
        return;

    if ((x_start + x_width) >= WIDTH)
        return;

    if ((y_start + y_height) >= HEIGHT)
        return;

    auto y_end = y_start + y_height;

    // For each pixel row, get the starting index, and iterate over the row,
    // writing the appropriate rgbx_buffer value in each spot
    for (uint16_t y = y_start; y < y_end; y++)
    {
        uint32_t * pixel_buffer = reinterpret_cast<uint32_t *>(&buffer_hardware_[((y * WIDTH) + x_start) * 4]);
        uint32_t * pixel_buffer_row_end = pixel_buffer + x_width;

        for (; pixel_buffer < pixel_buffer_row_end; pixel_buffer++, rgbx_buffer++)
        {
            *pixel_buffer = *rgbx_buffer;
        }
    }
}

//-----------------------------------------------------------------------------
std::error_code GUIContextDVI::Close() const
{
    initialized_ = false;

    auto error_to_return = std::error_code();

    // A segmentation fault will occur if munmap is called on an invalid pointer
    if (buffer_hardware_ != MAP_FAILED)
    {
        int ret = system_.Munmap(buffer_hardware_, HARDWARE_BUFFER_SIZE);
        if (ret < 0)
            error_to_return = std::error_code(errno, std::system_category());
        buffer_hardware_ = static_cast<uint8_t*>(MAP_FAILED);
    }

    auto error = system_file_.Close(fd_framebuffer_);
    if (error)
        error_to_return = error;

    error = system_file_.Close(fd_i2c_);
    if (error)
        error_to_return = error;

    return error_to_return;
}
