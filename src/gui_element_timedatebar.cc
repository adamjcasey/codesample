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

#include <ctime>

#include "include/agg_wrapper.h"
#include "include/gui_font.h"
#include "include/gui_element_timedatebar.h"

//-----------------------------------------------------------------------------
void GUIElementTimeDateBar::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Create the body rectangle
    GUI::RoundedRectangle rectangle(0, 0, width_, height_, 0);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color_));

    // Get the time and date
    char time_string[32];
    char date_string[32];
    time_t now = std::time(nullptr);
    std::strftime(time_string, 32, "%I:%M:%S", std::localtime(&now));
    std::strftime(date_string, 32, "%d %b, %Y", std::localtime(&now));

    // Write the date string at the specified spot from the left border
    GUIFontMedium font_medium(context_);
    font_medium.RenderText(date_string, (height_ * 0.367), x_ + gutter_, y_ + (height_ * 0.633), font_color_);

    // Write the time string at the specified spot from the left border
    double string_width = font_medium.GetStringWidth(time_string, (height_ * 0.367));
    font_medium.RenderText(time_string, (height_ * 0.367), x_ + width_ - gutter_ - string_width,
                                                           y_ + (height_ * 0.633), font_color_);
}
