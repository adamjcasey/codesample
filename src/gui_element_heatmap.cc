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

#include <math.h>

#include "include/agg_wrapper.h"
#include "include/dsp.h"
#include "include/gui_color.h"
#include "include/gui_color_map.h"
#include "include/gui_font.h"
#include "include/gui_element_heatmap.h"

//------------------------------------------------------------------------------
void GUIElementHeatmap::Draw() const
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
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color_));
}

//-----------------------------------------------------------------------------
void GUIElementHeatmap::DrawHeatmap(double (&temperature)[HEATMAP_HEIGHT][HEATMAP_WIDTH]) const
{
    // The heatmap is a large chunk of data that will be updated many times a second.
    // Flicker will not be an issue, since the data will be changing slowly.
    // Thus, for speed an efficiency, we should write directly to the framebuffer
    // using methods provided in the GUIContext.

    // For now we will use nearest neighbor interpolation.
    double x_ratio = HEATMAP_WIDTH / width_;
    double y_ratio = HEATMAP_HEIGHT / height_;

    for (int y = 0; y < height_; y++)
    {
        for (int x = 0; x < width_; x++)
        {
            // Note: if this is too slow, consider this as a non fp alternative

            double px = floor(x * x_ratio);
            double py = floor(y * y_ratio);

            // Get the temperature of the pixel
            double t = temperature[static_cast<int>(py)][static_cast<int>(px)];

            // Set pixels to the correct value in the frame buffer
            context_.SetPixelDirectly(x_ + x, y_ + y, GUIColorMap::ColorFromTemperature(t));
        }
    }
}

//-----------------------------------------------------------------------------
void GUIElementHeatmap::ResetHeatmap() const
{
    auto body_color_value = static_cast<uint32_t>(body_color_);
    for (int y = 0; y < height_; y++)
    {
        for (int x = 0; x < width_; x++)
        {
            // Set pixels to the correct value in the frame buffer
            context_.SetPixelDirectly(x_ + x, y_ + y, body_color_value);
        }
    }
}

//-----------------------------------------------------------------------------
void GUIElementHeatmapAuxDisplay::DrawHeatmap(double (&temperature)[HEATMAP_HEIGHT][HEATMAP_WIDTH]) const
{
    // Bilinear interpolation version requires final dimensions of heatmap to be known
    // at compile-time. The GUIElementHeatmapAuxDisplay class thus has a fixed dimension.

    number_heatmaps_received_++;

    // Copy current data to previous data
    uint32_t * current_data_start = &current_display_color_indices_[0][0];
    uint32_t * current_data_end = current_data_start + DISPLAY_TOTAL_PIXELS;
    uint32_t * previous_data_start = &previous_display_color_indices_[0][0];

    std::copy(current_data_start, current_data_end, previous_data_start);

    double filtered_temperature[HEATMAP_HEIGHT][HEATMAP_WIDTH];
    double resized_temperature[DISPLAY_HEIGHT][DISPLAY_WIDTH];

    // Note: these operations take approximately 52 ms to complete

    DSP::Convolve2DWithSeparableKernel(temperature, filtered_temperature, LOW_PASS_FILTER_KERNEL);
    DSP::Resize2DWithLinearInterpolation(filtered_temperature, resized_temperature);

    // Convert from temperature map to color index map
    for (size_t y = 0; y < DISPLAY_HEIGHT; y++)
    {
        for (size_t x = 0; x < DISPLAY_WIDTH; x++)
        {
            uint32_t color = GUIColorMap::GetColorIndexFromTemperature(resized_temperature[y][x]);
            current_display_color_indices_[y][x] = color;
        }
    }

    last_b_scan_update_time_ = std::chrono::steady_clock::now();

    // If at least two sets of data haven't been received, exit without drawing,
    // so that we only draw / animate real data
    if (number_heatmaps_received_ < 2)
        return;

    DrawFrame(previous_display_color_indices_);
}

//-----------------------------------------------------------------------------
void GUIElementHeatmapAuxDisplay::AnimateHeatmap() const
{
    // Each frame takes approximately 36.5 ms to complete

    // If at least two sets of data haven't been received, exit without drawing,
    // so that we only draw / animate real data
    if (number_heatmaps_received_ < 2)
        return;

    uint32_t heatmap_frame_color_indices[DISPLAY_HEIGHT][DISPLAY_WIDTH];

    auto now = std::chrono::steady_clock::now();
    size_t milliseconds_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_b_scan_update_time_).count();

    double frame_ratio = static_cast<double>(milliseconds_elapsed) / EXPECTED_MILLISECONDS_BETWEEN_SCANS;

    // Want to start at the previous, and move towards the current
    // A frame takes about 15 ms to generate
    DSP::TemporallySmoothWithLinearInterpolationSingleFrame(
        previous_display_color_indices_,
        current_display_color_indices_,
        heatmap_frame_color_indices,
        frame_ratio
    );

    // Takes about 21.5 ms to complete
    DrawFrame(heatmap_frame_color_indices);
}

//-----------------------------------------------------------------------------
void GUIElementHeatmapAuxDisplay::DrawFrame(const uint32_t (&heatmap_frame_color_indices)[DISPLAY_HEIGHT][DISPLAY_WIDTH]) const
{
    // Takes about 21.5 ms to complete
    // - Approximately 17 ms is used to generate heatmap_frame_colors
    //   TODO(jkirschner): this seems to defeat the purpose of converting to indices from temperatures // NOLINT(whitespace/todo)
    //   only once, as this routine seems to take much longer than expected
    // - Approximately 4.5 ms is used to draw via SetPixelRegionDirectly

    uint32_t heatmap_frame_colors[DISPLAY_HEIGHT][DISPLAY_WIDTH];

    for (size_t y = 0; y < DISPLAY_HEIGHT; y++)
    {
        for (size_t x = 0; x < DISPLAY_WIDTH; x++)
        {
            heatmap_frame_colors[y][x] = GUIColorMap::GetColorFromColorIndex(heatmap_frame_color_indices[y][x]);
        }
    }

    // Set pixels to the correct value in the frame buffer
    context_.SetPixelRegionDirectly(x_, y_, DISPLAY_WIDTH, DISPLAY_HEIGHT,
        &heatmap_frame_colors[0][0]);
}

//-----------------------------------------------------------------------------
void GUIElementHeatmapAuxDisplay::ResetHeatmap() const
{
    number_heatmaps_received_ = 0;

    GUIElementHeatmap::ResetHeatmap();
}

const double GUIElementHeatmapAuxDisplay::LOW_PASS_FILTER_KERNEL[GUIElementHeatmapAuxDisplay::LOW_PASS_FILTER_KERNEL_SIZE] = {
    0.05504587, 0.2440367, 0.40183486, 0.2440367, 0.05504587
};
