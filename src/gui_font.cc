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
#include "include/assets/FontHumanSansBold.h"
#include "include/assets/FontHumanSansMedium.h"
#include "include/assets/FontHumanSansRegular.h"
#include "include/gui_font.h"

//-----------------------------------------------------------------------------
void GUIFont::Render(const char *text, double size, double x, double y,
                        GUIColor color,
                        GetVectorDataForGlyphFunction vector_func,
                        GetWidthForGlyphFunction width_func,
                        double height, bool rotate) const
{
    if ((!vector_func) || (!width_func))
        return;

    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    double scaling = size / height;

    // Loop through the string
    while (*text)
    {
        GUIVectorPoint *table = vector_func(*text);
        GUI::VectorPath path = GUI::CreatePathFromVectorTable(table, scaling, x, y, rotate);
        GUI::VectorShape shape(path);
        rasterizer.add_path(shape);
        GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(color));

        // Update position and character.  If rotated, change the y axis value.  If normal, change the x.
        if (rotate)
            y -= (scaling * width_func(*text));
        else
            x += (scaling * width_func(*text));
        text++;
    }
}

//-----------------------------------------------------------------------------
double GUIFont::CalculateStringWidth(const char *text, double size,
                                        GetWidthForGlyphFunction width_func, double height) const
{
    if (!width_func)
        return 0.0;

    double scaling = size / height;
    double width = 0;

    // Loop through the string
    while (*text)
    {
        width += (scaling * width_func(*text));
        text++;
    }

    return width;
}

//-----------------------------------------------------------------------------
void GUIFontMedium::RenderText(const char *text, double size,
                                double x, double y, GUIColor color, bool rotate) const
{
    GUIFont::Render(text, size, x, y, color, FontHumanSansMedium::GetVectorDataForGlyph,
                                             FontHumanSansMedium::GetWidthOfGlyph,
                                             FontHumanSansMedium::Height(), rotate);
}

//-----------------------------------------------------------------------------
void GUIFontBold::RenderText(const char *text, double size,
                                double x, double y, GUIColor color, bool rotate) const
{
    GUIFont::Render(text, size, x, y, color, FontHumanSansBold::GetVectorDataForGlyph,
                                             FontHumanSansBold::GetWidthOfGlyph,
                                             FontHumanSansBold::Height(), rotate);
}

//-----------------------------------------------------------------------------
void GUIFontRegular::RenderText(const char *text, double size,
                                double x, double y, GUIColor color, bool rotate) const
{
    GUIFont::Render(text, size, x, y, color, FontHumanSansRegular::GetVectorDataForGlyph,
                                             FontHumanSansRegular::GetWidthOfGlyph,
                                             FontHumanSansRegular::Height(), rotate);
}

//-----------------------------------------------------------------------------
double GUIFontMedium::GetStringWidth(const char *text, double size) const
{
    return GUIFont::CalculateStringWidth(text, size, FontHumanSansMedium::GetWidthOfGlyph,
                                             FontHumanSansMedium::Height());
}

//-----------------------------------------------------------------------------
double GUIFontBold::GetStringWidth(const char *text, double size) const
{
    return GUIFont::CalculateStringWidth(text, size, FontHumanSansBold::GetWidthOfGlyph,
                                             FontHumanSansBold::Height());
}

//-----------------------------------------------------------------------------
double GUIFontRegular::GetStringWidth(const char *text, double size) const
{
    return GUIFont::CalculateStringWidth(text, size, FontHumanSansRegular::GetWidthOfGlyph,
                                             FontHumanSansRegular::Height());
}

