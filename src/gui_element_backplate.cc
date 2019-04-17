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

#include "include/agg_wrapper.h"
#include "include/gui_color.h"
#include "include/gui_font.h"
#include "include/gui_element_backplate.h"

//-----------------------------------------------------------------------------
void GUIElementBackplate::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Create the background rectangle fo the heat map and temp slider.
    GUI::RoundedRectangle rectangle(x_, y_, x_ + width_, y_ + height_, 5);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color_));

    // Add the labels
    GUIFontRegular font_label(context_);

    font_label.RenderText("0", font_height_label_, x_ + 20, y_ + 40, font_color_);
    font_label.RenderText("mm", font_height_label_, x_ + 13, y_ + 52, font_color_);
    font_label.RenderText("30", font_height_label_, x_ + 16, y_ + 302, font_color_);
    font_label.RenderText("mm", font_height_label_, x_ + 13, y_ + 314, font_color_);
    font_label.RenderText("60", font_height_label_, x_ + 16, y_ + 563, font_color_);
    font_label.RenderText("mm", font_height_label_, x_ + 13, y_ + 575, font_color_);

    font_label.RenderText("PROXIMAL", font_height_label_, x_ + 178, y_ + 24, font_color_);
    font_label.RenderText("DISTAL", font_height_label_, x_ + 189, y_ + 612, font_color_);

    font_label.RenderText("-180\xB0", font_height_label_, x_ + 45, y_ + 596, font_color_);
    font_label.RenderText("0\xB0", font_height_label_, x_ + 202, y_ + 596, font_color_);
    font_label.RenderText("180\xB0", font_height_label_, x_ + 346, y_ + 596, font_color_);

    font_label.RenderText("\xB0" "C", font_height_label_, x_ + 387, y_ + 24, font_color_);
}
