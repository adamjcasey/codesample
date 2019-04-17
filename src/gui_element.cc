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
#include "include/gui_element.h"
#include "include/gui_system_colors.h"

//-----------------------------------------------------------------------------
void GUIElement::Refresh() const
{
    if (!visible_)
        return;

    context_.ForceRedraw(static_cast<uint16_t>(x_),
        static_cast<uint16_t>(y_),
        static_cast<uint16_t>(x_ + width_),
        static_cast<uint16_t>(y_ + height_));
}

//-----------------------------------------------------------------------------
void GUIElement::Clear() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Create the body rectangle
    GUI::RoundedRectangle rectangle(x_, y_, x_ + width_, y_ + height_, 0);
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::DarkBlue));
}

//-----------------------------------------------------------------------------
void GUIElement::Click(uint16_t x, uint16_t y) const
{
    if (IsInRegion(x, y))
    {
        // If this GUI element is clickable, then call the click callback
        if (clickable_)
        {
            if (click_callback_)
                click_callback_();
        }

        // Now call the derived OnClick method (or the dummy if not overridden)
        is_active_ = true;
        OnClick(x, y);
    }
}

//-----------------------------------------------------------------------------
void GUIElement::Drag(uint16_t x, uint16_t y) const
{
    if (IsInRegion(x, y) && is_active_)
        OnDrag(x, y);
}

//-----------------------------------------------------------------------------
void GUIElement::Release() const
{
    is_active_ = false;
    OnRelease();
}

//-----------------------------------------------------------------------------
bool GUIElement::IsInRegion(uint16_t x, uint16_t y) const
{
    // If x outside region, return false
    if ((x < (x_ - RegionOffsetLeft())) || (x > (x_ + width_ - 1 + RegionOffsetRight())))
        return false;

    // If y outside region, return false
    if ((y < (y_ - RegionOffsetTop())) || (y > (y_ + height_ - 1 + RegionOffsetBottom())))
        return false;

    return true;
}
