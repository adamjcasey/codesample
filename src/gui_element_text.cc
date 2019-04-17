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
#include "include/gui_element_text.h"
#include "include/gui_system_colors.h"


//-----------------------------------------------------------------------------
void GUIElementLabelMedium::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Write the text
    GUIFontMedium font_bold(context_);
    font_bold.RenderText(text_, font_height_, x_, y_ + font_height_, font_color_);
}

//-----------------------------------------------------------------------------
void GUIElementLabelRegular::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Write the text
    GUIFontRegular font_regular(context_);
    font_regular.RenderText(text_, font_height_, x_, y_ + font_height_, font_color_);
}

//-----------------------------------------------------------------------------
void GUIElementLabelTimeDateValue::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Draw the lines
    GUI::RoundedRectangle top_line(x_, y_, x_ + width_, y_ + 2, 0);
    rasterizer.add_path(top_line);
    GUI::RoundedRectangle bottom_line(x_, y_ + height_ - 2, x_ + width_, y_ + height_, 0);
    rasterizer.add_path(bottom_line);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::DarkGray));
}

//-----------------------------------------------------------------------------
void GUIElementLabelTimeDateValue::UpdateText(const char * text) const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Clear the text area
    GUI::RoundedRectangle rectangle(x_, y_ + 3, x_ + width_, y_ + height_ - 3, 0);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::DarkBlue));

    // Render the value
    GUIFontRegular font_regular(context_);
    double text_width = font_regular.GetStringWidth(text, font_height_);
    double text_origin_x = x_ + (width_ / 2.0) - (text_width / 2.0);
    double text_origin_y = y_ + (height_ / 2.0) + (font_height_ / 2.0);
    font_regular.RenderText(text, font_height_, text_origin_x, text_origin_y, font_color_);
}

//-----------------------------------------------------------------------------
void GUIElementLabelRegularVariable::UpdateText(const char * text) const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Clear the text area
    GUI::RoundedRectangle rectangle(x_, y_, x_ + width_, y_ + height_, 0);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::DarkBlue));

    // Render the value
    GUIFontRegular font_regular(context_);
    font_regular.RenderText(text, font_height_, x_, y_ + height_, font_color_);
}
