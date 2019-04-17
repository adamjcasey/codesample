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
#include <ctime>

#include "include/agg_wrapper.h"
#include "include/gui_element_linegraph.h"
#include "include/gui_font.h"
#include "include/gui_vector.h"
#include "include/parameters.h"
#include "include/utility.h"

//-----------------------------------------------------------------------------
const char * GUIElementLineGraph::y_axis_labels_[] = {"70", "60", "50", "40", "30", "20", "10", "0", "-10", "-20"};

//-----------------------------------------------------------------------------
void GUIElementLineGraph::Draw() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Create the body rectangle
    GUI::RoundedRectangle rectangle(x_, y_, x_ + width_, y_ + height_, 5);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color_));

    // Draw the labels for the y axis
    for (int i = 0; i <= NUMBER_OF_DIVISIONS_TEMPERATURE; i++)
    {
        GUIFontRegular font_label(context_);
        double font_height_label = graph_body_y_axis_division_height_ * 0.300;
        double text_width_label = font_label.GetStringWidth(y_axis_labels_[i], font_height_label);
        font_label.RenderText(y_axis_labels_[i], font_height_label,
            graph_body_x_ - 6 - text_width_label,
            graph_body_y_ + (graph_body_y_axis_division_height_ * i) + 3,
            font_color_);
    }

    // Draw the axis titles.  Note that this needs to be broken up this way because
    // \xhh will keep going until it sees no more hex characters, and C is a hex digit.
    const char y_axis_label[] = "PEAK TEMP (\xB0" "C)";
    GUIFontRegular font_axis_label(context_);
    double font_height_label = graph_body_y_axis_division_height_ * 0.400;
    font_axis_label.RenderText(y_axis_label, font_height_label,
        x_ + (width_ * 0.040),
        y_ + (height_ * 0.700),
        font_color_, true);

    const char x_axis_label[] = "ELAPSED MINUTES";
    font_axis_label.RenderText(x_axis_label, font_height_label,
        x_ + (width_ * 0.380),
        y_ + (height_ * 0.970),
        font_color_);

    DrawGraphArea();
}

//-----------------------------------------------------------------------------
void GUIElementLineGraph::DrawGraphArea() const
{
    DrawGraphBody();
    DrawTimeLinesAndAxes();
    DrawGraphLines();
}

//-----------------------------------------------------------------------------
void GUIElementLineGraph::DrawGraphBody() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
        context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Create the graph area
    GUI::RoundedRectangle rectangle(graph_body_x_, graph_body_y_,
        graph_body_x_ + graph_body_width_, graph_body_y_ + graph_body_height_, 0);
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(background_color_));

    // Draw the lines for the y axis
    for (int i = 1; i < NUMBER_OF_DIVISIONS_TEMPERATURE; i++)
    {
        GUI::RoundedRectangle y_axis_line(graph_body_x_, graph_body_y_ + (graph_body_y_axis_division_height_ * i),
            graph_body_x_ + graph_body_width_,
            graph_body_y_ + (graph_body_y_axis_division_height_ * i) + 1, 0);
        rasterizer.add_path(y_axis_line);
        GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color_));
    }
}

//-----------------------------------------------------------------------------
void GUIElementLineGraph::DrawTimeLinesAndAxes() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
        context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Clear the x axis labels
    GUI::RoundedRectangle rectangle(graph_body_x_ - 7,
                                    graph_body_y_ + graph_body_height_,
                                    graph_body_x_ + graph_body_width_ + 10,
                                    graph_body_y_ + graph_body_height_ + 20, 0);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color_));

    // Figure out the offset in minutes
    double minutes = (graph_last_update_time_seconds_ / 60.0);

    // Divide the minutes by the granularity of the axis
    double whole_offset;
    double fractional_offset = modf(minutes / X_AXIS_GRANULARITY_MINUTES, &whole_offset);
    double rounded_to_granularity = (floor(minutes / X_AXIS_GRANULARITY_MINUTES)) * X_AXIS_GRANULARITY_MINUTES;

    // If the fractional offset is less than the minimum of one pixel, round to zero.
    if (fractional_offset < graph_fractional_offset_threshold_)
        fractional_offset = 0.0;

    // Draw the axis lines and labels
    for (int i = 0; i < NUMBER_OF_DIVISIONS_TIME; i++)
    {
        // Get the position of the axis (starting from the right)
        double x_axis_position = graph_body_x_ + graph_body_width_ - (graph_body_x_axis_division_width_ * (fractional_offset + i));

        // Draw the line label if it is not negative
        int label_value = static_cast<int>(rounded_to_granularity - (i * X_AXIS_GRANULARITY_MINUTES));
        if (label_value >= 0)
        {
            char label_text[5];
            snprintf(label_text, sizeof(label_text), "%d", label_value);
            GUIFontRegular font_label(context_);
            double font_height_label = graph_body_y_axis_division_height_ * 0.300;
            double text_width_label = font_label.GetStringWidth(label_text, font_height_label);
            font_label.RenderText(label_text, font_height_label,
                x_axis_position - (text_width_label / 2.0),
                graph_body_y_ + graph_body_height_ + 14,
                font_color_);
        }

        // If this is the first division, and the fractional offset is less than a pixel,
        // don't draw the line, just continue
        if ((i == 0) && (fractional_offset == 0))
            continue;

        // Draw the axis line
        GUI::RoundedRectangle x_axis_line(x_axis_position, graph_body_y_,
                                          x_axis_position + 1, graph_body_y_ + graph_body_height_, 0);
        rasterizer.add_path(x_axis_line);
        GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color_));
    }

    // If the fractional offset is zero, then we need a label at the end of the graph, if the label isn't zero
    int final_label_value = static_cast<int>(whole_offset - (NUMBER_OF_MINUTES_SHOWN * X_AXIS_GRANULARITY_MINUTES));
    if ((fractional_offset == 0) && (final_label_value >= 0))
    {
        char label_text[4];
        snprintf(label_text, sizeof(label_text), "%d", final_label_value);
        GUIFontRegular font_label(context_);
        double font_height_label = graph_body_y_axis_division_height_ * 0.300;
        double text_width_label = font_label.GetStringWidth(label_text, font_height_label);
        font_label.RenderText(label_text, font_height_label,
            graph_body_x_ - (text_width_label / 2.0),
            graph_body_y_ + graph_body_height_ + 14,
            font_color_);
    }
}

//-----------------------------------------------------------------------------
void GUIElementLineGraph::DrawGraphLines() const
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
        context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Draw the data graph line.  This will return the points from oldest to newest.
    DataPoint instantaneous_data_snapshot_[DATA_BUFFER_SIZE];
    auto points_available = data_queue_.Peek(instantaneous_data_snapshot_, DATA_BUFFER_SIZE);
    bool first_point = true;
    uint16_t table_index = 0;
    // We set the table sizes to be 1 greater than the DATA_BUFFER_SIZE since the first value in the table
    // must be GUIVectorPointType::START and data x-values begin at <table>[1]
    GUIVectorPoint data_table[DATA_BUFFER_SIZE + 1];
    GUIVectorPoint hot_limit_table[DATA_BUFFER_SIZE + 1];
    GUIVectorPoint cold_limit_table[DATA_BUFFER_SIZE + 1];

    for (int i = 0; i < points_available; i++)
    {
        // If the point is outside of the range shown by the graph (which is possible because
        // we have added some room for slop) then don't graph it.
        // We use 'continue' here because the oldest data is loaded first so we want to iterate
        // through the points until we reach data recent enough to be displayed.
        double minutes = (graph_last_update_time_seconds_ - instantaneous_data_snapshot_[i].time_seconds) / 60.0;
        if (minutes > NUMBER_OF_MINUTES_SHOWN)
            continue;

        // Figure out the coordinates
        double x = graph_body_x_ + graph_body_width_ - ((minutes / NUMBER_OF_MINUTES_SHOWN) * graph_body_width_) + 1;
        x = Utility::Coerce<double>(x, graph_body_x_, (graph_body_x_ + graph_body_width_ - 1));

        double y = graph_body_y_ + (graph_body_height_ * ((instantaneous_data_snapshot_[i].temperature - 70.0) / -90.0));
        y = Utility::Coerce<double>(y, graph_body_y_, (graph_body_y_ + graph_body_height_ - 1));

        // Calculate corresponding hot and cold temperature limits
        // No need to coerce as these values cannot go out of bounds
        double y_position_hot_limit = graph_body_y_ + graph_body_height_ *
                                        ((instantaneous_data_snapshot_[i].hot_temperature_threshold - 70.0) / -90.0);       
        double y_position_cold_limit = graph_body_y_ + graph_body_height_ *
                                        ((instantaneous_data_snapshot_[i].cold_temperature_threshold - 70.0) / -90.0);

        // If this is the first point, start the path
        if (first_point)
        {
            first_point = false;

            data_table[table_index].type_ = GUIVectorPointType::START;
            hot_limit_table[table_index].type_ = GUIVectorPointType::START;
            cold_limit_table[table_index].type_ = GUIVectorPointType::START;

            table_index++;

            data_table[table_index].type_ = GUIVectorPointType::MOVE;
            hot_limit_table[table_index].type_ = GUIVectorPointType::MOVE;
            cold_limit_table[table_index].type_ = GUIVectorPointType::MOVE;

            data_table[table_index].end_x_ = x;
            hot_limit_table[table_index].end_x_ = x;
            cold_limit_table[table_index].end_x_ = x;

            data_table[table_index].end_y_ = y;
            hot_limit_table[table_index].end_y_ = y_position_hot_limit;
            cold_limit_table[table_index].end_y_ = y_position_cold_limit;

            table_index++;
        }
        else
        {
            data_table[table_index].type_ = GUIVectorPointType::LINE;
            hot_limit_table[table_index].type_ = GUIVectorPointType::LINE;
            cold_limit_table[table_index].type_ = GUIVectorPointType::LINE;

            data_table[table_index].end_x_ = x;
            hot_limit_table[table_index].end_x_ = x;
            cold_limit_table[table_index].end_x_ = x;

            data_table[table_index].end_y_ = y;
            hot_limit_table[table_index].end_y_ = y_position_hot_limit;
            cold_limit_table[table_index].end_y_ = y_position_cold_limit;

            table_index++;
        }
    }

    GUI::VectorPath data_path = GUI::CreatePathFromVectorTable(data_table);
    GUI::VectorStroke peak_temperature_graph_data(data_path);
    rasterizer.add_path(peak_temperature_graph_data);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(peak_temperature_color_));

    GUI::VectorPath hot_path = GUI::CreatePathFromVectorTable(hot_limit_table);
    GUI::VectorStroke peak_temperature_graph_hot_limit(hot_path);
    rasterizer.add_path(peak_temperature_graph_hot_limit);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(hot_limit_color_));   

    GUI::VectorPath cold_path = GUI::CreatePathFromVectorTable(cold_limit_table);
    GUI::VectorStroke peak_temperature_graph_cold_limit(cold_path);
    rasterizer.add_path(peak_temperature_graph_cold_limit);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(cold_limit_color_));   
}

//-----------------------------------------------------------------------------
void GUIElementLineGraph::Pause()
{
    is_paused_ = true;

    DataPoint data_point;
    auto error = data_queue_.PeekLast(data_point);

    if (error)
    {
        // If error, assume queue is empty, and there has been no elapsed time
        time_elapsed_when_paused_seconds_ = 0.0;
    }
    else
    {
        // Assume the last point in the data queue + 1 sample (one per second)
        // is the total time elapsed when paused. When a new point comes in,
        // we will assume the time is time_elapsed_when_paused_seconds_.
        time_elapsed_when_paused_seconds_ = data_point.time_seconds + 1.0;
    }
}

//-----------------------------------------------------------------------------
void GUIElementLineGraph::Update(double temperature)
{
    DataPoint data_point;

    if (is_paused_)
    {
        // Resume on first Update call after paused
        is_paused_ = false;
        graph_last_resume_time_ = GraphClock::now();

        // Place the point immediately after the last point before being paused in time.
        data_point.time_seconds = time_elapsed_when_paused_seconds_;
    }
    else
    {
        // The time associated with this point is:
        // - the time elapsed when last paused +
        // - the time elapsed since the last resume
        std::chrono::duration<double, Seconds> duration_since_last_resume_seconds = GraphClock::now() - graph_last_resume_time_;

        data_point.time_seconds = time_elapsed_when_paused_seconds_ + duration_since_last_resume_seconds.count();
    }

    // Limit the line graph so that if temperature is out of range
    // it will draw the line at its closest limit
    if (temperature < Parameters::PeakTemperature::PEAK_TEMPERATURE_MIN_DISPLAY_VALUE_C)
    {
        data_point.temperature = Parameters::PeakTemperature::PEAK_TEMPERATURE_MIN_DISPLAY_VALUE_C;
    }
    else if (temperature > Parameters::PeakTemperature::PEAK_TEMPERATURE_MAX_DISPLAY_VALUE_C)
    {
        data_point.temperature = Parameters::PeakTemperature::PEAK_TEMPERATURE_MAX_DISPLAY_VALUE_C;
    }
    else
    {
        data_point.temperature = temperature;
    }

    // Each datapoint has an associated hot and cold temperature threshold
    data_point.hot_temperature_threshold = hot_temperature_celsius_;
    data_point.cold_temperature_threshold = cold_temperature_celsius_;

    graph_last_update_time_seconds_ = data_point.time_seconds;
    data_queue_.Push(data_point, true);
    DrawGraphArea();
}

//-----------------------------------------------------------------------------
void GUIElementLineGraph::UpdateHotLimit(int temperature)
{
    hot_temperature_celsius_ = temperature;
    DrawGraphArea();
}

//-----------------------------------------------------------------------------
void GUIElementLineGraph::UpdateColdLimit(int temperature)
{
    cold_temperature_celsius_ = temperature;
    DrawGraphArea();
}

//-----------------------------------------------------------------------------
void GUIElementLineGraph::RefreshGraphArea()
{
    context_.ForceRedraw(static_cast<uint16_t>(graph_body_x_ - 6),
        static_cast<uint16_t>(graph_body_y_),
        static_cast<uint16_t>(graph_body_x_ + graph_body_width_ + 10),
        static_cast<uint16_t>(graph_body_y_ + graph_body_height_ + 20));
}
