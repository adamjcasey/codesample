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

#include <cstdio>
#include <cstring>

#include "include/agg_wrapper.h"
#include "include/gui_font.h"
#include "include/gui_element_infobox.h"
#include "include/parameters.h"

//------------------------------------------------------------------------------
void GUIElementInfobox::Draw() const
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
    auto header_color = is_faded_ ? header_color_.Faded() : header_color_;
    auto font_color = is_faded_ ? font_color_.Faded() : font_color_;

    // Create the body rectangle
    GUI::RoundedRectangle rectangle(x_, y_, x_ + width_, y_ + height_, 5);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color));

    // Create the header rectangle
    GUI::RoundedRectangle rectangle_header(x_, y_, x_ + width_, y_ + header_height_, 5);
    rectangle_header.normalize_radius();
    rasterizer.add_path(rectangle_header);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(header_color));

    // Write the header text
    GUIFontMedium font_medium(context_);
    font_medium.RenderText(header_text_, header_height_ * 0.346,
                                         x_ + (header_height_ * 0.385),
                                         y_ + (header_height_ * 0.692), font_color);
}

//-----------------------------------------------------------------------------
void GUIElementInfobox::ClearInfoArea() const
{
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
    GUI::RoundedRectangle rectangle(x_, y_ + header_height_, x_ + width_, y_ + height_, 5);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color));
}

//-----------------------------------------------------------------------------
void GUIElementInfobox::RefreshInfoArea() const
{
    context_.ForceRedraw(static_cast<uint16_t>(x_),
                         static_cast<uint16_t>(y_ + header_height_),
                         static_cast<uint16_t>(x_+ width_),
                         static_cast<uint16_t>(y_+ height_));
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxMultiline::SetText(const char *text)
{
    // Split the input
    SplitLines(text);
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxMultiline::DrawText() const
{
    // Update the colors
    auto font_color = is_faded_ ? font_color_.Faded() : font_color_;

    // Write the text
    GUIFontBold font_bold(context_);
    for (int i = 0; i < NUM_LINES; i++)
    {
        font_bold.RenderText(text_lines_[i], header_height_ * 0.4, x_ + (header_height_ * 0.385),
            y_ + header_height_ + (header_height_ * 0.746 + (i * header_height_ * 0.55)), font_color);
    }
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxMultiline::SplitLines(const char * message)
{
    memset(&text_lines_, '\0', sizeof(text_lines_));

    // If message is nullptr, we simply clear the lines and avoid further processing
    if (!message)
        return;

    // Split the text into NUM_LINES lines.  This is not taking general input; all strings passed here are
    // carefully thought out and tested.  If they look wrong, they will be adjusted by the caller.
    // Here we just need to ensure that all are successfully terminated.
    char buffer[64];
    strncpy(buffer, message, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    char *string_to_parse = buffer;

    for (int num_lines = 0; num_lines < NUM_LINES; num_lines++)
    {
        char *end_of_token = const_cast<char *>(strchr(string_to_parse, '\n'));

        // If a '\n' character was found, replace the '\n' with a null-terminator,
        // and copy the string. Then, set search location to one past '\n' location.
        if (end_of_token)
        {
            *end_of_token = '\0';

            strncpy(text_lines_[num_lines], string_to_parse, MAX_LINE_LENGTH - 1);

            // Manually add null-termination (strncpy won't if the source has more
            // than MAX_TOKEN_SIZE - 1 characters)
            text_lines_[num_lines][MAX_LINE_LENGTH - 1] = '\0';

            string_to_parse = end_of_token + 1;
        }
        // If a '\n' character is not found, you are at the end of the string already.
        // Copy over what remains, increment counter (because loop is about to be broken),
        // then exit the loop.
        else
        {
            strncpy(text_lines_[num_lines], string_to_parse, MAX_LINE_LENGTH - 1);

            // Manually add null-termination (strncpy won't if the source has more
            // than MAX_TOKEN_SIZE characters)
            text_lines_[num_lines][MAX_LINE_LENGTH - 1] = '\0';
            break;
        }
    }
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxStatus::Draw() const
{
    // Draw the parent element
    GUIElementInfobox::Draw();

    // Update the text
    DrawText();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxStatus::UpdateText(uint8_t status_type, const char *text)
{
    // If status isn't in list return without doing anything
    if (status_type >= static_cast<uint8_t>(StatusType::NUMBER_OF_STATUSES))
        return;

    // Update message and visibility
    statuses_[status_type].SetVisible(true);
    statuses_[status_type].SetMessage(text);

    UpdateTextWithHighestPriorityStatusMessage();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxStatus::UpdateText(const char *text)
{
    return UpdateText(static_cast<uint8_t>(StatusType::GENERAL), text);
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxStatus::UpdateTextWithHighestPriorityStatusMessage()
{
    if (!visible_)
        return;

    // Clear the rectangle
    ClearInfoArea();

    // Draw the text
    SetText(GetHighestPriorityStatusMessage());
    DrawText();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxStatus::ClearStatusByType(uint8_t status_type)
{
    // If status isn't in list return without doing anything
    if (status_type >= static_cast<uint8_t>(StatusType::NUMBER_OF_STATUSES))
        return;

    statuses_[status_type].SetVisible(false);

    UpdateTextWithHighestPriorityStatusMessage();
}

//-----------------------------------------------------------------------------
const char * GUIElementInfoboxStatus::GetHighestPriorityStatusMessage()
{
    // Set status_index to the highest possible status type
    int32_t status_index = static_cast<uint8_t>(StatusType::NUMBER_OF_STATUSES) - 1;

    for (; status_index >= 0; status_index--)
    {
        // When we encounter the first set status return its message
        if (statuses_[status_index].GetVisible())
        {
            return statuses_[status_index].GetMessage();
        }
    }

    // If none found, return nullptr
    return nullptr;
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxStatus::Status::SetMessage(const char * message)
{
    // Clear message if nullptr
    if (!message)
    {
        memset(message_, '\0', sizeof(message_));
        return;
    }

    // copy the string to the member variable and add a null terminator at the end
    strncpy(message_, message, sizeof(message_) - 1);
    message_[sizeof(message_) - 1] = '\0';
}

//-----------------------------------------------------------------------------
const char * GUIElementInfoboxAlert::header_text_error_ = "ERROR";
const char * GUIElementInfoboxAlert::header_text_system_status_ = "SYSTEM STATUS";

//-----------------------------------------------------------------------------
void GUIElementInfoboxAlert::Draw() const
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
    auto font_color = is_faded_ ? font_color_.Faded() : font_color_;

    // Create the body rectangle
    GUI::RoundedRectangle rectangle(x_, y_, x_ + width_, y_ + height_, 5);
    rectangle.normalize_radius();
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color));

    // Create the header line
    GUI::RoundedRectangle header_line(x_, y_ + header_height_ - 2, x_ + width_, y_ + header_height_, 1);
    rasterizer.add_path(header_line);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(font_color));

    // Write the header text
    GUIFontMedium font_medium(context_);
    font_medium.RenderText(header_text_, header_height_ * 0.346,
                                         x_ + (header_height_ * 0.385),
                                         y_ + (header_height_ * 0.692), font_color);

    // Draw the text
    DrawText();

    // Draw the button
    DrawButton();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxAlert::UpdateText(const char *text)
{
    if (!visible_)
        return;

    // Clear the rectangle
    ClearInfoArea();

    // Draw the text
    SetText(text);
    DrawText();

    // Enable the button
    button_active_ = true;
    DrawButton();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxAlert::DrawButton() const
{
    if (!button_active_)
        return;

    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
        context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Update the colors
    auto button_font_color = is_faded_ ? button_font_color_.Faded() : button_font_color_;
    auto button_color = is_faded_ ? button_color_.Faded() : button_color_;

     // Draw the button
    GUI::RoundedRectangle rectangle_button_outline(x_ + (width_ * 0.24),
        y_ + (height_ * 0.63),
        x_ + width_ - (width_ * 0.24),
        y_ + height_ - (height_ * 0.067),
        5);
    rectangle_button_outline.normalize_radius();
    rasterizer.add_path(rectangle_button_outline);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(button_font_color));

    GUI::RoundedRectangle rectangle_button_(x_ + (width_ * 0.24) + 2,
        y_ + (height_ * 0.63) + 2,
        x_ + width_ - (width_ * 0.24) - 2,
        y_ + height_ - (height_ * 0.067) - 2,
        5);
    rectangle_button_.normalize_radius();
    rasterizer.add_path(rectangle_button_);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(button_color));

    double font_height = body_height_ * 0.11;
    GUIFontBold font_bold(context_);
    double text_width = font_bold.GetStringWidth("OK", font_height);
    font_bold.RenderText("OK", font_height, x_ + (width_ / 2.0) - (text_width / 2.0),
        y_ + (height_ * 0.82), button_font_color);
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxAlert::SetErrorState(bool error)
{
    if (error)
        header_text_ = header_text_error_;
    else
        header_text_ = header_text_system_status_;

    Draw();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxAlert::DismissButton()
{
    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
        context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Update the colors
    auto body_color = is_faded_ ? body_color_.Faded() : body_color_;

    // Erase the button
    GUI::RoundedRectangle rectangle_button_outline(x_ + (width_ * 0.24) - 2,
        y_ + (height_ * 0.63) - 2,
        x_ + width_ - (width_ * 0.24) + 2,
        y_ + height_ - (height_ * 0.067) + 2,
        0);
    rasterizer.add_path(rectangle_button_outline);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(body_color));

    button_active_ = false;
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxAlert::EnablePopupAlert(uint8_t alert_type, const char * message, bool show_confirm_button)
{
    return EnablePopupAlert(alert_type, message, show_confirm_button, ActionOnDisablePopupAlert::CLEAR_POPUP);
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxAlert::EnablePopupAlert(uint8_t alert_type, const char * message, bool show_confirm_button,
    ActionOnDisablePopupAlert action_on_disable_popup_alert)
{
    // If alert isn't in list return without doing anything
    if (alert_type >= static_cast<uint8_t>(PopupAlertType::NUMBER_OF_ALERTS))
        return;

    // Update alert_type message and warning state
    popup_alerts_[alert_type].SetWarningState();
    popup_alerts_[alert_type].SetMessage(message);
    popup_alerts_[alert_type].SetShowConfirmButton(show_confirm_button);
    popup_alerts_[alert_type].SetActionOnDisablePopupAlert(action_on_disable_popup_alert);

    // If the new alert (alert_type) is equal or higher priority than the highest
    // priority active alert, update the popup
    // If new alert is lower priority than the currently displayed alert (alert_type_index)
    // the update will occur after the currently displayed popup is cleared with ClearPopupAlert()
    if (alert_type >= GetHighestPriorityActiveAlertIndex())
    {
        DrawPopupAlert(alert_type);
    }
}

//-----------------------------------------------------------------------------
bool GUIElementInfoboxAlert::IsPopupAlertEnabled(uint8_t alert_type)
{
    if (alert_type >= static_cast<uint8_t>(PopupAlertType::NUMBER_OF_ALERTS))
        return false;

    return popup_alerts_[alert_type].GetWarningState();
}

//-----------------------------------------------------------------------------
bool GUIElementInfoboxAlert::IsConfirmButtonVisible()
{
    // Return whether the confirm button is visible for the current shown alert
    int32_t alert_type_index = GetHighestPriorityActiveAlertIndex();
    if (alert_type_index >= 0)
    {
        return popup_alerts_[alert_type_index].GetShowConfirmButton();
    }

    return false;
}

//-----------------------------------------------------------------------------
// Returns false if a popup remains, returns true if all popups are cleared,
// in which case ClearPopup() must be called from gui_screen.cc
// This function can only clear a popup if there's a confirm button visible
bool GUIElementInfoboxAlert::DisablePopupAlert()
{
    // Clear the highest priority active alert that has a confirm button visible
    int32_t alert_type_index = GetHighestPriorityActiveAlertIndex();
    if ((alert_type_index >= 0) && popup_alerts_[alert_type_index].GetShowConfirmButton())
    {
        switch (popup_alerts_[alert_type_index].GetActionOnDisablePopupAlert())
        {
            case ActionOnDisablePopupAlert::REMOVE_CONFIRM_BUTTON:
                // Leave the warning, but remove the button when redrawn below
                popup_alerts_[alert_type_index].SetShowConfirmButton(false);
                break;

            default:
                popup_alerts_[alert_type_index].ClearWarningState();
        }
    }

    // Find the highest priority active alert and set its popup
    // If GetHighestPriorityActiveAlertIndex() returns -1, there are
    // no more popups to set
    alert_type_index = GetHighestPriorityActiveAlertIndex();

    if (alert_type_index >= 0)
    {
        DrawPopupAlert(alert_type_index);
        return false;
    }

    // If we got this far we know that there are no alerts to display
    // in which case we want to tell the main app to clear the popup
    return true;
}

//-----------------------------------------------------------------------------
// Returns false if a popup remains, returns true if all popups are cleared,
// in which case ClearPopup() must be called from gui_screen.cc
// This function will clear a specific popup regardless of whether there
// is a confirm button visible
bool GUIElementInfoboxAlert::DisablePopupAlertExplicit(uint8_t alert_type)
{
    // If alert isn't in list return false without doing anything
    if (alert_type >= static_cast<uint8_t>(PopupAlertType::NUMBER_OF_ALERTS))
        return false;

    // Determine if that alert is currently being displayed
    if (alert_type == GetHighestPriorityActiveAlertIndex())
    {
        // Clear the warning state of the alert passed in
        popup_alerts_[alert_type].ClearWarningState();

        // Find the highest priority active alert and set its popup
        // If GetHighestPriorityActiveAlertIndex() returns -1, there are
        // no more popups to set
        int32_t alert_type_index = GetHighestPriorityActiveAlertIndex();

        if (alert_type_index >= 0)
        {
            DrawPopupAlert(alert_type_index);
            return false;
        }
    }
    // If the alert being cleared isn't currently being displayed just clear it
    else
    {
        // Clear the warning state of the alert passed in
        popup_alerts_[alert_type].ClearWarningState();
        // Since entering this loop means there is a higher priority
        // alert currently on the display, we return false
        return false;
    }

    // If we got this far we know that there are no alerts to display
    // in which case we want to tell the main app to clear the popup
    return true;
}

//-----------------------------------------------------------------------------
int32_t GUIElementInfoboxAlert::GetHighestPriorityActiveAlertIndex()
{
    // Set alert_type_index to the highest possible alert type
    int32_t alert_type_index = static_cast<uint8_t>(PopupAlertType::NUMBER_OF_ALERTS) - 1;

    for (; alert_type_index >= 0; alert_type_index--)
    {
        // When we encounter the first set warning return its index
        if (popup_alerts_[alert_type_index].GetWarningState())
        {
            return alert_type_index;
        }
    }
    // If none found, return -1
    return -1;
}

//-----------------------------------------------------------------------------
// Performs actions necessary to display the popup specified by alert_type
void GUIElementInfoboxAlert::DrawPopupAlert(uint32_t alert_type)
{
    SetVisible(true);
    SetClickable(popup_alerts_[alert_type].GetShowConfirmButton());
    SetErrorState(false);
    UpdateText(popup_alerts_[alert_type].GetMessage());

    // If the popup isn't clearable, dismiss the button
    if (!popup_alerts_[alert_type].GetShowConfirmButton())
        DismissButton();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxAlert::PopupAlert::SetMessage(const char * message)
{
    // Clear message if nullptr
    if (!message)
    {
        memset(message_, '\0', sizeof(message_));
        return;
    }

    // copy the string to the member variable and add a null terminator at the end
    strncpy(message_, message, sizeof(message_) - 1);
    message_[sizeof(message_) - 1] = '\0';
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxGeneralTemperature::Draw() const
{
    if (!visible_)
        return;

    // Draw the parent element
    GUIElementInfobox::Draw();

    // Update the colors
    auto font_color = is_faded_ ? font_color_.Faded() : font_color_;

    // Write the Celsius symbol
    GUIFontBold font_bold(context_);
    const char celsius[] = "\xB0" "C";
    font_bold.RenderText(celsius, (header_height_ * 0.385), x_ + width_ - (header_height_ * 0.961),
                            y_ + (header_height_ * 0.692), font_color);

    DrawTemperature();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxGeneralTemperature::SetCurrentTemp(double temp) const
{
    current_temp_ = temp;
    enabled_ = true;

    if (!visible_)
        return;

    DrawTemperature();
    RefreshInfoArea();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxGeneralTemperature::Disable() const
{
    enabled_ = false;

    if (!visible_)
        return;

    DrawTemperature();
    RefreshInfoArea();
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxGeneralTemperature::DrawTemperature() const
{
    // Get the value to render
    char buffer[6];

    FormatTemperatureIntoBuffer(buffer);

    // Clear the rectangle
    ClearInfoArea();

    // Update the colors
    auto font_color = is_faded_ ? font_color_.Faded() : font_color_;

    // Render the value
    GUIFontMedium font_medium(context_);
    double font_height = body_height_ * 0.509;
    double text_width = font_medium.GetStringWidth(buffer, font_height);
    font_medium.RenderText(buffer, font_height,
                                x_ + (width_ / 2.0) - (text_width / 2.0),
                                y_ + header_height_ + (body_height_ * 0.754),
                                font_color);
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxGeneralTemperature::FormatTemperatureIntoBuffer(char (&buffer)[6]) const
{
    if (enabled_)
        snprintf(buffer, sizeof(buffer), "%0.1f", current_temp_);
    else
        snprintf(buffer, sizeof(buffer), "--.-");
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxThresholdTemperature::FormatTemperatureIntoBuffer(char (&buffer)[6]) const
{
    if (enabled_)
        snprintf(buffer, sizeof(buffer), "%0.0f", current_temp_);
    else
        snprintf(buffer, sizeof(buffer), "--.-");
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxCoreTemperature::FormatTemperatureIntoBuffer(char (&buffer)[6]) const
{
    if (enabled_)
    {
        if (current_temp_ < Parameters::CoreTemperature::CORE_TEMPERATURE_MIN_DISPLAY_VALUE_C)
        {
            snprintf(buffer, sizeof(buffer), "<%0.0f",
                    Parameters::CoreTemperature::CORE_TEMPERATURE_MIN_DISPLAY_VALUE_C);
        }
        else if (current_temp_ > Parameters::CoreTemperature::CORE_TEMPERATURE_MAX_DISPLAY_VALUE_C)
        {
            snprintf(buffer, sizeof(buffer), ">%0.0f",
                    Parameters::CoreTemperature::CORE_TEMPERATURE_MAX_DISPLAY_VALUE_C);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "%0.1f", current_temp_);
        }
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "--.-");
    }
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxPeakTemperature::FormatTemperatureIntoBuffer(char (&buffer)[6]) const
{
    if (enabled_)
    {
        if (current_temp_ < Parameters::PeakTemperature::PEAK_TEMPERATURE_MIN_DISPLAY_VALUE_C)
        {
            snprintf(buffer, sizeof(buffer), "<%0.0f",
                    Parameters::PeakTemperature::PEAK_TEMPERATURE_MIN_DISPLAY_VALUE_C);
        }
        else if (current_temp_ > Parameters::PeakTemperature::PEAK_TEMPERATURE_MAX_DISPLAY_VALUE_C)
        {
            snprintf(buffer, sizeof(buffer), ">%0.0f",
                    Parameters::PeakTemperature::PEAK_TEMPERATURE_MAX_DISPLAY_VALUE_C);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "%0.1f", current_temp_);
        }
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "--.-");
    }
}

//-----------------------------------------------------------------------------
void GUIElementInfoboxPeakTemperature::SetAlertColoring(bool alert) const
{
    if (alert)
    {
        header_color_ = alert_header_color_;
        body_color_ = alert_body_color_;
        font_color_ = alert_font_color_;
    }
    else
    {
        header_color_ = base_header_color_;
        body_color_ = base_body_color_;
        font_color_ = base_font_color_;
    }
}
