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
#include <cstdio>

#include "include/agg_wrapper.h"
#include "include/gui_color.h"
#include "include/gui_font.h"
#include "include/gui_system_colors.h"
#include "include/gui_element_tempslider.h"
#include "include/gui_color_map.h"
#include "include/utility.h"

//-----------------------------------------------------------------------------
void GUIElementTempSlider::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Create the gradient
    GUI::GradientFunc gradient_func;
    GUI::TransAffine gradient_mtx;
    GUI::Interpolator span_interpolator(gradient_mtx);
    GUI::SpanAllocator span_allocator;
    GUI::ColorArray gradient_colors;

    // Gradient is of size GUI::GRADIENT_SIZE.
    // It maps the last index to the coldest temp and the first index to the hottest.
    int gradient_index = GUI::GRADIENT_SIZE - 1;
    double temperature_delta_per_index = TEMPERATURE_CELSIUS_RANGE_EXTENT / gradient_index;
    while (gradient_index >= 0)
    {
        double temperature = TEMPERATURE_CELSIUS_RANGE_MAX -
                             temperature_delta_per_index * gradient_index;
        uint32_t hexcolor = GUIColorMap::ColorFromTemperature(temperature);
        GUIColor base_color(hexcolor);
        auto color = is_faded_ ? base_color.Faded() : base_color;
        gradient_colors[gradient_index] = GUI::Color(color);
        gradient_index--;
    }
    GUI::SpanGradient span_gradient(span_interpolator,
                                      gradient_func,
                                      gradient_colors,
                                      y_, y_ + height_);
    GUI::RendererGradient ren_gradient(renderer_buffer, span_allocator, span_gradient);

    // Render the rectangle
    GUI::RoundedRectangle rectangle(x_, y_, x_ + width_, y_ + height_, 0);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(background_color_));

    // Render the gradient
    GUI::RoundedRectangle rectangle_gradient(x_, y_, x_ + (width_ * 0.542), y_ + height_, 6);
    rectangle_gradient.normalize_radius();
    rasterizer.reset();
    rasterizer.add_path(rectangle_gradient);
    GUI::RenderScanlines(rasterizer, scanline, ren_gradient);

    // Add the lines
    GUI::RoundedRectangle header_line_top(
        x_,
        y_ + (height_ * HEADER_LINE_PERCENT_INSET_FROM_END),
        x_ + width_,
        y_ + (height_ * HEADER_LINE_PERCENT_INSET_FROM_END) + HEADER_LINE_WIDTH,
        1
    );
    rasterizer.add_path(header_line_top);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(background_color_));

    GUI::RoundedRectangle header_line_bottom(
        x_,
        y_ + (height_ * (1.0 - HEADER_LINE_PERCENT_INSET_FROM_END)) - HEADER_LINE_WIDTH,
        x_ + width_,
        y_ + (height_ * (1.0 - HEADER_LINE_PERCENT_INSET_FROM_END)),
        1
    );
    rasterizer.add_path(header_line_bottom);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(background_color_));

    // Add the temperature markers
    GUIFontRegular font_regular(context_);
    double xpoint = (x_ + (width_ * 0.644));
    font_regular.RenderText("70", (height_ * 0.02112), xpoint, y_ + (height_ * 0.019), GUISystemColors::White);
    font_regular.RenderText("37", (height_ * 0.02112), xpoint, y_ + (height_ * 0.405), GUISystemColors::White);
    font_regular.RenderText("-20", (height_ * 0.02112), xpoint, y_ + (height_ * 0.995), GUISystemColors::White);

    // Draw the
    switch (alert_coloring_)
    {
        case AlertColoring::NONE:
            DrawPointer(y_position_of_hot_, false);
            DrawPointer(y_position_of_cold_, false);
            break;

        case AlertColoring::LOW_TEMP_ALERT:
            DrawPointer(y_position_of_hot_, false);
            DrawPointer(y_position_of_cold_, true);
            break;

        case AlertColoring::HIGH_TEMP_ALERT:
            DrawPointer(y_position_of_hot_, true);
            DrawPointer(y_position_of_cold_, false);
            break;
    }
}

//-----------------------------------------------------------------------------
void GUIElementTempSlider::DrawPointer(uint16_t y, bool alert) const
{
    double x1 = x_ + (width_ * 0.542);
    double x2 = x_ + (width_ * 0.915);
    double y1 = y - (height_ / 28.4);
    double y2 = y + (height_ / 28.4);

    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::RendererSolid renderer_solid(renderer_buffer);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Create a rounded pentagon
    GUI::RoundedPentagon r(x1, y1, x2, y2);

    // Draw the solid
    rasterizer.add_path(r);

    auto body_color = (alert) ? alert_slider_body_color_ : base_slider_body_color_;
    renderer_solid.color(GUI::Color(body_color));

    GUI::RenderScanlines(rasterizer, scanline, renderer_solid);

    // Draw the outline
    GUI::RoundedPentagonOutline p(r);
    p.width(2.0);
    rasterizer.add_path(p);
    renderer_solid.color(GUI::Color(GUISystemColors::White));
    GUI::RenderScanlines(rasterizer, scanline, renderer_solid);

    // Render the text
    char buffer[4];
    snprintf(buffer, sizeof(buffer), "%d", TemperatureFromYPosition(y));
    GUIFontMedium font_medium(context_);
    double text_width = font_medium.GetStringWidth(buffer, height_ / 30.43);

    auto font_color = (alert) ? alert_slider_font_color_ : base_slider_font_color_;
    font_medium.RenderText(buffer, height_ / 30.43,
                            x_ + (width_ * 0.650) - (text_width / 2.0),
                            y + 7, font_color);
}

//-----------------------------------------------------------------------------
void GUIElementTempSlider::SetHotPointerTemperature(int temperature_celsius) const
{
    // Store the temperature
    hot_temperature_celsius_ = temperature_celsius;

    uint16_t y, y_old;

    // Save the old y so we can update the redraw rectangle efficiently.
    // Also, keep within limits, and keep in mind that y position and temperature
    // are inversely proportional.

    y_old = y_position_of_hot_;

    y = YPositionFromTemperature(temperature_celsius);

    y_position_of_hot_ = Utility::Coerce(y, y_position_hot_max_, y_position_hot_min_);

    CalculateAndRedraw(y, y_old);
}

//-----------------------------------------------------------------------------
void GUIElementTempSlider::SetColdPointerTemperature(int temperature_celsius) const
{
    // Store the temperature
    cold_temperature_celsius_ = temperature_celsius;

    uint16_t y, y_old;

    // Save the old y so we can update the redraw rectangle efficiently.
    // Also, keep within limits, and keep in mind that y position and temperature
    // are inversely proportional.

    y_old = y_position_of_cold_;

    y = YPositionFromTemperature(temperature_celsius);

    y_position_of_cold_ = Utility::Coerce(y, y_position_cold_max_, y_position_cold_min_);

    CalculateAndRedraw(y, y_old);
}

//-----------------------------------------------------------------------------
void GUIElementTempSlider::OnClick(uint16_t x, uint16_t y) const
{
    // Check to see if we are within the hot slider
    // Add small vertical offset to make easier to select
    uint16_t half_pointer_height = static_cast<uint16_t>(height_ / 28.4) + 3;
    if ((y >= y_position_of_hot_ - half_pointer_height) &&
        (y <= y_position_of_hot_ + half_pointer_height))
    {
        // Store the value
        hot_temperature_celsius_ = TemperatureFromYPosition(y_position_of_hot_);

        pointer_hot_selected_ = true;
        pointer_on_selected_y_offset_from_middle_ = y - y_position_of_hot_;

        if (hot_touch_callback_)
            hot_touch_callback_(hot_temperature_celsius_);
    }

    // Otherwise check to see if we are within the cold slider
    else if ((y >= y_position_of_cold_ - half_pointer_height) &&
             (y <= y_position_of_cold_ + half_pointer_height))
    {
        // Store the value
        cold_temperature_celsius_ = TemperatureFromYPosition(y_position_of_cold_);

        pointer_cold_selected_ = true;
        pointer_on_selected_y_offset_from_middle_ = y - y_position_of_cold_;

        if (cold_touch_callback_)
            cold_touch_callback_(cold_temperature_celsius_);
    }
}

//-----------------------------------------------------------------------------
void GUIElementTempSlider::OnDrag(uint16_t x, uint16_t y) const
{
    uint16_t y_old;
    uint16_t y_new;

    // If the currently moving pointer is the hot one, move that
    if (pointer_hot_selected_)
    {
        // Save the old y so we can update the redraw rectangle efficiently.
        // Also, keep within limits, and keep in mind that y position and temperature
        // are inversely proportional.
        y_old = y_position_of_hot_;

        // Maintain the original offset between the click location and the middle of the pointer
        uint16_t y_position_of_hot_uncoerced = y - pointer_on_selected_y_offset_from_middle_;

        // Hot max is higher on the screen (lower y value), which is why it's the lower_limit argument to Coerce
        y_position_of_hot_ = Utility::Coerce(y_position_of_hot_uncoerced, y_position_hot_max_, y_position_hot_min_);
        y_new = y_position_of_hot_;

        // Store the value
        hot_temperature_celsius_ = TemperatureFromYPosition(y_position_of_hot_);

        // Call the hot temp changed callback
        if (hot_change_callback_)
            hot_change_callback_(hot_temperature_celsius_);
    }
    // If the currently moving pointer is the cold one, move that
    else if (pointer_cold_selected_)
    {
        // Save the old y so we can update the redraw rectangle efficiently.
        // Also, keep within limits, and keep in mind that y position and temperature
        // are inversely proportional.
        y_old = y_position_of_cold_;

        // Maintain the original offset between the click location and the middle of the pointer
        uint16_t y_position_of_cold_uncoerced = y - pointer_on_selected_y_offset_from_middle_;

        // Cold max is higher on the screen (lower y value), which is why it's the lower_limit argument to Coerce
        y_position_of_cold_ = Utility::Coerce(y_position_of_cold_uncoerced, y_position_cold_max_, y_position_cold_min_);
        y_new = y_position_of_cold_;

        // Store the value
        cold_temperature_celsius_ = TemperatureFromYPosition(y_position_of_cold_);

        // Call the cold temp changed callback
        if (cold_change_callback_)
            cold_change_callback_(cold_temperature_celsius_);
    }
    // Don't redraw if neither pointer set
    else
    {
        return;
    }

    CalculateAndRedraw(y_new, y_old);
}

//-----------------------------------------------------------------------------
void GUIElementTempSlider::OnRelease() const
{
    // If the currently moving pointer is the hot one, move that
    if (pointer_hot_selected_)
    {
        pointer_hot_selected_ = false;

        // Call the hot temp released callback
        if (hot_release_callback_)
            hot_release_callback_(TemperatureFromYPosition(y_position_of_hot_));
    }

    else if (pointer_cold_selected_)
    {
        pointer_cold_selected_ = false;

        // Call the cold temp released callback
        if (cold_release_callback_)
            cold_release_callback_(TemperatureFromYPosition(y_position_of_cold_));
    }
}

//-----------------------------------------------------------------------------
void GUIElementTempSlider::SetAlertColoring(AlertColoring coloring) const
{
    if (coloring == alert_coloring_)
        return;

    auto previous_color = alert_coloring_;
    alert_coloring_ = coloring;

    // Redraw the area of the display the cold pointer occupies if low temp alert is being toggled
    if ((alert_coloring_ == AlertColoring::LOW_TEMP_ALERT) || (previous_color == AlertColoring::LOW_TEMP_ALERT))
        CalculateAndRedraw(y_position_of_cold_, y_position_of_cold_);

    // Redraw the area of the display the hot pointer occupies if high temp alert is being toggled
    if ((alert_coloring_ == AlertColoring::HIGH_TEMP_ALERT) || (previous_color == AlertColoring::HIGH_TEMP_ALERT))
        CalculateAndRedraw(y_position_of_hot_, y_position_of_hot_);
}

//-----------------------------------------------------------------------------
int GUIElementTempSlider::TemperatureFromYPosition(uint16_t y) const
{
    // Convert the screen position to a temperature value

    // y_position_hot_max_ is the lowest y coordate; y_position_cold_min_ is the highest y coordinate
    double y_position_ratio = static_cast<double>(y - y_position_hot_max_) / (y_position_cold_min_ - y_position_hot_max_);
    return static_cast<int>(round(TEMPERATURE_CELSIUS_RANGE_MIN + TEMPERATURE_CELSIUS_RANGE_EXTENT * (1.0 - y_position_ratio)));
}

//-----------------------------------------------------------------------------
uint16_t GUIElementTempSlider::YPositionFromTemperature(int temp) const
{
    // Convert the temperature value to a screen position

    // y_position_hot_max_ is the lowest y coordate; y_position_cold_min_ is the highest y coordinate
    double temp_range_ratio = (temp - TEMPERATURE_CELSIUS_RANGE_MIN) / TEMPERATURE_CELSIUS_RANGE_EXTENT;
    return static_cast<uint16_t>(y_position_hot_max_ + ((y_position_cold_min_ - y_position_hot_max_) * (1.0 - (temp_range_ratio))));
}

//-----------------------------------------------------------------------------
void GUIElementTempSlider::CalculateAndRedraw(uint16_t y_new, uint16_t y_old) const
{
    uint16_t y_low, y_high;

    // Calculate the redraw rectangle.  This allows for a margin to correctly
    // redraw the pointer outline
    if (y_new > y_old)
    {
        y_low = y_old - static_cast<uint16_t>(height_ / 20.286);
        y_high = y_new + static_cast<uint16_t>(height_ / 20.286);
    }
    else
    {
        y_low = y_new - static_cast<uint16_t>(height_ / 20.286);
        y_high = y_old + static_cast<uint16_t>(height_ / 20.286);
    }

    // If drawing above the temperature slider, call the hot pointer dirty rectangle callback
    if (y_low < y_)
    {
        if (hot_pointer_dirty_rectangle_callback_)
            hot_pointer_dirty_rectangle_callback_();
    }

    // If drawing below the temperature slider, call the hot pointer dirty rectangle callback
    if (y_high > (y_ + height_))
    {
        if (cold_pointer_dirty_rectangle_callback_)
            cold_pointer_dirty_rectangle_callback_();
    }
    Draw();
    context_.ForceRedraw(static_cast<int>(x_ + (width_ * 0.200)),
                         y_low,
                         static_cast<int>(x_ + width_),
                         y_high);
}
