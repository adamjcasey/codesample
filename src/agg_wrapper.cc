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

//-----------------------------------------------------------------------------
void GUI::RenderScanlinesAASolid(Rasterizer& ras, Scanline& s, RendererBase& rb, agg::rgba8 c)
{
    agg::render_scanlines_aa_solid(ras, s, rb, c);
}

//-----------------------------------------------------------------------------
void GUI::RenderScanlines(Rasterizer& ras, Scanline& s, RendererGradient& rg)
{
    agg::render_scanlines(ras, s, rg);
}

//-----------------------------------------------------------------------------
void GUI::RenderScanlines(Rasterizer& ras, Scanline& s, RendererSolid& rs)
{
    agg::render_scanlines(ras, s, rs);
}

//-----------------------------------------------------------------------------
agg::rgba8 GUI::Color(GUIColor c)
{
    return agg::rgba8(c.red_, c.green_, c.blue_, c.alpha_);
}

//-----------------------------------------------------------------------------
GUI::VectorPath GUI::CreatePathFromVectorTable(GUIVectorPoint *table)
{
    // Go through all the curves in the data table
    VectorPath path;

    int i = 0;
    while (table[i].type_ != GUIVectorPointType::EXIT)
    {
        switch (table[i].type_)
        {
            case GUIVectorPointType::START:
                path.remove_all();
                break;

            case GUIVectorPointType::MOVE:
                path.move_to(table[i].end_x_, table[i].end_y_);
                break;

            case GUIVectorPointType::CURVE_Q:
                path.curve3(table[i].control_x1_, table[i].control_y1_, table[i].end_x_, table[i].end_y_);
                break;

            case GUIVectorPointType::CURVE_C:
                path.curve4(table[i].control_x1_, table[i].control_y1_, table[i].control_x2_,
                    table[i].control_y2_, table[i].end_x_, table[i].end_y_);
                break;

            case GUIVectorPointType::LINE:
                path.line_to(table[i].end_x_, table[i].end_y_);
                break;

            case GUIVectorPointType::CLOSE:
                path.close_polygon();
                break;

            case GUIVectorPointType::EXIT:
                break;
        }
        i++;
    }

    return path;
}

//-----------------------------------------------------------------------------
GUI::VectorPath GUI::CreatePathFromVectorTable(GUIVectorPoint *table,
                                                double scaling, double x, double y, bool rotate)
{
    // Go through all the curves in the data table
    VectorPath path = CreatePathFromVectorTable(table);

    // Perform transformations
    agg::trans_affine shape_mtx;
    shape_mtx.flip_y();
    shape_mtx *= agg::trans_affine_scaling(scaling);
    if (rotate)
        shape_mtx *= agg::trans_affine_rotation(agg::pi * 1.5);
    shape_mtx *= agg::trans_affine_translation(x, y);

    path.transform(shape_mtx);

    return path;
}
