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
#include "include/gui_font.h"
#include "include/gui_element_button.h"
#include "include/gui_system_colors.h"
#include "include/assets/gui_icons.h"

//-----------------------------------------------------------------------------
void GUIElementTextButton::Draw() const
{
    if (!visible_)
        return;

    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Update the colors
    auto body_color = is_faded_ ? body_color_.Faded() : body_color_;

    // Create the body rectangle
    GUI::RoundedRectangle rectangle(x_, y_, x_ + width_, y_ + height_, 5);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color));

    // Render the text
    RenderText();
}

//-----------------------------------------------------------------------------
void GUIElementTextButton::RenderText() const
{
    // Update the colors
    auto font_color = is_faded_ ? font_color_.Faded() : font_color_;

    // Write the text
    GUIFontBold font_bold(context_);
    for (int i = 0; i < NUM_LINES; i++)
    {
        double text_width = font_bold.GetStringWidth(text_lines_[i], 18);
        font_bold.RenderText(text_lines_[i], 18,
                                x_ + (width_ / 2.0) - (text_width / 2.0),
                                y_ + 30 + (i * 25),
                                font_color);
    }
}

//-----------------------------------------------------------------------------
void GUIElementTextButton::SetText(char (&textline)[NUM_LINES][MAX_LINE_LENGTH])
{
    for (int i = 0; i < NUM_LINES; i++)
    {
        strncpy(text_lines_[i], textline[i], MAX_LINE_LENGTH);
        text_lines_[i][MAX_LINE_LENGTH - 1] = '\0';
    }
}

//-----------------------------------------------------------------------------
void GUIElementTextButton::SetColors(GUIColor body_color, GUIColor font_color)
{
    body_color_ = body_color;
    font_color_ = font_color;
}

//-----------------------------------------------------------------------------
void GUIElementUpButton::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Draw the up arrow
    GUI::VectorPath path = GUI::CreatePathFromVectorTable(GUIIcons::UpArrow(), 1.0, x_ + 15, y_ + height_ - 7, false);
    GUI::VectorShape shape(path);
    rasterizer.add_path(shape);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::DarkGray));
}

//-----------------------------------------------------------------------------
void GUIElementDownButton::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
        context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Draw the up arrow
    GUI::VectorPath path = GUI::CreatePathFromVectorTable(GUIIcons::DownArrow(), 1.0, x_ + 15, y_ + height_ - 7, false);
    GUI::VectorShape shape(path);
    rasterizer.add_path(shape);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::DarkGray));
}

//-----------------------------------------------------------------------------
void GUIElementOKButton::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
        context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

     // Draw the button
    GUI::RoundedRectangle rectangle_button_outline(x_, y_, x_ + width_, y_ + height_, 5);
    rectangle_button_outline.normalize_radius();
    rasterizer.add_path(rectangle_button_outline);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::White));

    GUI::RoundedRectangle rectangle_button_(x_+ 2, y_ + 2, x_ + width_ - 2, y_ + height_ - 2, 5);
    rectangle_button_.normalize_radius();
    rasterizer.add_path(rectangle_button_);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::Green));

    // Draw the checkmark
    GUI::VectorPath path = GUI::CreatePathFromVectorTable(GUIIcons::CheckMark(), 1.0, x_, y_ + height_, false);
    GUI::VectorStroke stroke(path);
    stroke.width(3);
    rasterizer.add_path(stroke);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::White));
}

//-----------------------------------------------------------------------------
void GUIElementCancelButton::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
        context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

     // Draw the button
    GUI::RoundedRectangle rectangle_button_outline(x_, y_, x_ + width_, y_ + height_, 5);
    rectangle_button_outline.normalize_radius();
    rasterizer.add_path(rectangle_button_outline);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::White));

    GUI::RoundedRectangle rectangle_button_(x_+ 2, y_ + 2, x_ + width_ - 2, y_ + height_ - 2, 5);
    rectangle_button_.normalize_radius();
    rasterizer.add_path(rectangle_button_);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::Orange));

    // Draw the X
    GUI::VectorPath path = GUI::CreatePathFromVectorTable(GUIIcons::XMark(), 1.0, x_, y_ + height_, false);
    GUI::VectorStroke stroke(path);
    stroke.width(3);
    rasterizer.add_path(stroke);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::White));
}
