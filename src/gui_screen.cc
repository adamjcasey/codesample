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
#include "include/gui_screen_aux.h"
#include "include/gui_screen_main.h"
#include "include/parameters.h"

//-----------------------------------------------------------------------------
void GUIScreenMain::Render()
{
    if (show_screen_main_)
        gui_screen_main_.Render();
    else
        gui_screen_timedate_.Render();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::TouchDown(uint16_t x, uint16_t y)
{
    if (show_screen_main_)
        gui_screen_main_.TouchDown(x, y);
    else
        gui_screen_timedate_.TouchDown(x, y);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::TouchUp()
{
    if (show_screen_main_)
        gui_screen_main_.TouchUp();
    else
        gui_screen_timedate_.TouchUp();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetTempSliderCallbacks(GUIElementTempSlider::TempSetpointReleaseCallback hot_released,
                                           GUIElementTempSlider::TempSetpointReleaseCallback cold_released)
{
    hot_released_ = hot_released;
    cold_released_ = cold_released;
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetButtonClickedCallback(IGUIElement::ClickCallback button_clicked)
{
    button_clicked_ = button_clicked;
    start_button_.SetClickable(true);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::ClearButtonClickedCallback()
{
    start_button_.SetClickable(false);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetDateBarClickedActive(bool active)
{
    datebar_clicked_active_ = active;
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetPeakTempClickedCallback(IGUIElement::ClickCallback peak_temp_clicked)
{
    peak_temp_clicked_ = peak_temp_clicked;
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetPopupClickedCallback(IGUIElement::ClickCallback popup_clicked)
{
    popup_clicked_ = popup_clicked;
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetTimeDateChangedCallback(IGUIScreenMain::TimeDateChangedCallback time_date_changed)
{
    time_date_changed_ = time_date_changed;
}

//-----------------------------------------------------------------------------
void GUIScreenMain::HotChanged(int temp)
{
    infobox_set_.SetCurrentTemp(temp);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::HotTouched(int temp)
{
    infobox_esophageal_.SetVisible(false);
    infobox_set_.SetCurrentTemp(temp);
    infobox_set_.SetVisible(true);
    infobox_set_.Draw();
    infobox_set_.Refresh();
    FadeElements(true);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::HotReleased(int temp)
{
    if (hot_released_)
        hot_released_(temp);

    infobox_esophageal_.SetVisible(true);
    infobox_set_.SetVisible(false);
    infobox_esophageal_.Clear();
    infobox_esophageal_.Draw();
    infobox_esophageal_.Refresh();
    FadeElements(false);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::OnHotPointerDirtyRectangle()
{
    // This is called whenever the hot temperature pointer has been changed by the user,
    // but before the hot temperature pointer is drawn in its new position in the
    // rendering buffer.

    // At the extremities of the hot temperature pointer extent, a portion of the rendering
    // buffer under the control of the timedatebar (rather than the tempslider) is updated.
    // As such, at the extremities, the timedatebar must be redrawn to the rendering buffer,
    // before the hot temperature pointer is drawn in its new position. The affected portion
    // must also be pushed to the framebuffer.

    // There is also a portion of the rendering buffer that isn't under the control of any
    // GUI element (it's only written once upon creation of the context). As such, we need
    // to manually redraw that area vertically between the temperature slider and the
    // datetime bar.

    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    // Redraw the dark blue background vertically between the temperature slider and the
    // datetime bar (on the right side of the screen).
    GUI::RoundedRectangle rectangle(208, 30, 272, 42, 0);
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::DarkBlue));

    timedatebar_.Draw();

    // Redraw affected area of the date bar (above the temperature slider).
    // There's no need to force redraw the dark blue background area, as the temperature slider will
    // cause that area to be pushed to the framebuffer when it erases the old pointer tna draws the
    // new pointer.
    // Include an area slightly to the left of the temperature slider, so that if the time is wider
    // than the temperature slider after timedatebar_.Draw(), the time displays correctly.
    context_.ForceRedraw(180, 0, 272, 30);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::ColdChanged(int temp)
{
    infobox_set_.SetCurrentTemp(temp);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::ColdTouched(int temp)
{
    infobox_esophageal_.SetVisible(false);
    infobox_set_.SetCurrentTemp(temp);
    infobox_set_.SetVisible(true);
    infobox_set_.Draw();
    infobox_set_.Refresh();
    FadeElements(true);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::ColdReleased(int temp)
{
    if (cold_released_)
        cold_released_(temp);

    infobox_esophageal_.SetVisible(true);
    infobox_set_.SetVisible(false);
    infobox_esophageal_.Clear();
    infobox_esophageal_.Draw();
    infobox_esophageal_.Refresh();
    FadeElements(false);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::OnColdPointerDirtyRectangle()
{
    // This is called whenever the cold temperature pointer has been changed by the user,
    // but before the cold temperature pointer is drawn in its new position in the
    // rendering buffer.

    // At the extremities of the cold temperature pointer extent, a portion of the
    // rendering buffer that isn't under the control of any GUI element (it's only written
    // once upon creation of the context). As such, we need to manually redraw that area
    // below the temperature slider.

    GUI::RenderingBuffer rbuf(context_.Buffer(), context_.Width(),
                            context_.Height(), context_.Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    GUI::Rasterizer rasterizer;
    GUI::Scanline scanline;

    GUI::RoundedRectangle rectangle(208, 468, 272, 480, 0);
    rasterizer.add_path(rectangle);
    GUI::RenderScanlinesAASolid(rasterizer, scanline, renderer_buffer, GUI::Color(GUISystemColors::DarkBlue));

    // There's no need to force redraw the dark blue background area, as the temperature slider will
    // cause that area to be pushed to the framebuffer when it erases the old pointer tna draws the
    // new pointer.
}

//-----------------------------------------------------------------------------
void GUIScreenMain::FadeElements(bool fade)
{
    if (!show_screen_main_)
        return;

    infobox_peak_.SetFade(fade);
    infobox_peak_.Draw();
    infobox_peak_.Refresh();

    infobox_status_.SetFade(fade);
    infobox_status_.Draw();
    infobox_status_.Refresh();

    start_button_.SetFade(fade);
    start_button_.Draw();
    start_button_.Refresh();

    tempslider_.SetFade(fade);
    tempslider_.Draw();
    tempslider_.Refresh();

    infobox_popup_.SetFade(fade);
    infobox_popup_.Draw();
    infobox_popup_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::TextButtonInactive()
{
    if (!show_screen_main_)
        return;

    char buttontext[2][16] = {"", ""};
    start_button_.SetColors(GUISystemColors::DarkBlue, GUISystemColors::DarkBlue);
    start_button_.SetText(buttontext);
    start_button_.Draw();
    start_button_.Refresh();
    start_button_.SetVisible(false);
    ClearButtonClickedCallback();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::TextButtonStartImaging()
{
    if (!show_screen_main_)
        return;

    start_button_.SetVisible(true);
    char buttontext[2][16] = {"START", "IMAGING"};
    start_button_.SetColors(GUISystemColors::Teal, GUISystemColors::DarkGray);
    start_button_.SetText(buttontext);
    start_button_.Draw();
    start_button_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::TextButtonStop()
{
    if (!show_screen_main_)
        return;

    start_button_.SetVisible(true);
    char buttontext[2][16] = {"STOP", ""};
    start_button_.SetColors(GUISystemColors::LightBlue, GUISystemColors::DarkGray);
    start_button_.SetText(buttontext);
    start_button_.Draw();
    start_button_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::TextButtonStopImaging()
{
    if (!show_screen_main_)
        return;

    start_button_.SetVisible(true);
    char buttontext[2][16] = {"STOP", "IMAGING"};
    start_button_.SetColors(GUISystemColors::LightBlue, GUISystemColors::DarkGray);
    start_button_.SetText(buttontext);
    start_button_.Draw();
    start_button_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetPopupAlert(uint8_t alert_type, const char * message, bool show_confirm_button)
{
    SetPopupAlertInternal(alert_type, message, show_confirm_button,
                          GUIElementInfoboxAlert::ActionOnDisablePopupAlert::CLEAR_POPUP);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetPopupAlertWithAcknowledgeButNotDismiss(uint8_t alert_type, const char * message)
{
    SetPopupAlertInternal(alert_type, message, true,
                          GUIElementInfoboxAlert::ActionOnDisablePopupAlert::REMOVE_CONFIRM_BUTTON);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetPopupAlertInternal(uint8_t alert_type, const char * message, bool show_confirm_button,
                                  GUIElementInfoboxAlert::ActionOnDisablePopupAlert action_on_disable_popup_alert)
{
    // Prepare popup alert to be displayed, if alert_type is higher priority than
    // current popup (or there is no current popup) this alert will be displayed,
    // if a higher priority popup is currently displayed it will wait until the
    // current popup is cleared before being displayed
    infobox_popup_.EnablePopupAlert(alert_type, message, show_confirm_button, action_on_disable_popup_alert);

    if (infobox_status_.GetVisible())
        infobox_status_.SetVisible(false);

    if (!show_screen_main_)
        return;
    // Necessary because infobox_popup_ does not automatically refresh itself
    infobox_popup_.Draw();
    infobox_popup_.Refresh();
}

//-----------------------------------------------------------------------------
bool GUIScreenMain::IsPopupAlertSet(uint8_t alert_type)
{
    return infobox_popup_.IsPopupAlertEnabled(alert_type);
}

//-----------------------------------------------------------------------------
bool GUIScreenMain::IsPopupAlertDismissButtonTouchable()
{
    return infobox_popup_.IsConfirmButtonVisible();
}

//-----------------------------------------------------------------------------
// This function will only clear the highest priority popup unless that popup
// is set as not clearable
void GUIScreenMain::ClearPopupAlert()
{
    // DisablePopupAlert returns true if there are no
    // more popups to display, in which case we call
    // ClearPopup().
    // If the highest level popup isn't set as clearable
    // this call will not do anything and DisablePopupAlert
    // will return false.
    if (infobox_popup_.DisablePopupAlert())
    {
        ClearPopup();
    }
    // Draw and refresh infobox_popup to display new popup
    // unless the main screen is hidden
    else if (show_screen_main_)
    {
        infobox_popup_.Draw();
        infobox_popup_.Refresh();
    }
}

//-----------------------------------------------------------------------------
// This function will always clear the popup alert_type passed to it
void GUIScreenMain::ClearPopupAlertExplicit(uint8_t alert_type)
{
    // DisablePopupAlert returns true if there are no
    // more popups to display, in which case we call
    // ClearPopup()
    if (infobox_popup_.DisablePopupAlertExplicit(alert_type))
    {
        ClearPopup();
    }
    // Draw and refresh infobox_popup to display new popup
    // unless the main screen is hidden
    else if (show_screen_main_)
    {
        infobox_popup_.Draw();
        infobox_popup_.Refresh();
    }
}

//-----------------------------------------------------------------------------
// This function will always clear all pop-up alerts
void GUIScreenMain::ClearAllPopupAlerts()
{
    // Clear all pop-ups and display status box
    for (size_t i = 0; i < static_cast<uint8_t>(GUIElementInfoboxAlert::PopupAlertType::NUMBER_OF_ALERTS); i++)
    {
        infobox_popup_.DisablePopupAlertExplicit(i);
    }

    ClearPopup();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::ClearPopup()
{
    infobox_status_.SetVisible(true);
    infobox_popup_.SetVisible(false);

    if (!show_screen_main_)
        return;

    infobox_status_.Draw();
    infobox_status_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::RemovePopupButton()
{
    infobox_popup_.DismissButton();
    infobox_popup_.SetClickable(false);

    if (!show_screen_main_)
        return;

    infobox_popup_.Draw();
    infobox_popup_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetStatusBox(const char *status)
{
    return SetStatusBox(static_cast<uint8_t>(GUIElementInfoboxStatus::StatusType::GENERAL), status);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetStatusBox(uint8_t status_type, const char *status)
{
    // If pop-up is present don't override it, but set the status message
    // ClosePopup() will reveal the status box in this case
    infobox_status_.SetVisible(true);
    infobox_status_.Draw();
    infobox_status_.UpdateText(status_type, status);

    // Toggled the status box visibility back off if there is a popup
    if (infobox_popup_.GetVisible())
        infobox_status_.SetVisible(false);

    // Don't refresh if the main screen is hidden
    if (!show_screen_main_)
        return;

    // If no pop-up is present set and reveal status box
    else if (!infobox_popup_.GetVisible())
    {
        infobox_status_.Refresh();
    }
    else
    {
        infobox_popup_.Draw();
        infobox_popup_.Refresh();
    }
}

//-----------------------------------------------------------------------------
void GUIScreenMain::ClearStatusTypeFromStatusBox(uint8_t status_type)
{
    infobox_status_.ClearStatusByType(status_type);

    // Toggled the status box visibility back off if there is a popup
    if (infobox_popup_.GetVisible())
        infobox_status_.SetVisible(false);

    // Don't refresh if the main screen is hidden
    if (!show_screen_main_)
        return;

    // If no pop-up is present set and reveal status box
    else if (!infobox_popup_.GetVisible())
    {
        infobox_status_.Refresh();
    }
    else
    {
        infobox_popup_.Draw();
        infobox_popup_.Refresh();
    }
}

//-----------------------------------------------------------------------------
void GUIScreenMain::PeakTemperatureColorController()
{
    if (peak_temperature_alert_state_ != GUIElementTempSlider::AlertColoring::NONE)
    {
        // If this is the first time entering initiate the colors and timer
        if (alert_initiated_ == false)
        {
            alert_initiated_ = true;
            last_color_state_ = true;

            // Start countdown timer that facilitates toggling colors
            alert_flash_peak_temperature_timer_.Start(PEAK_TEMPERATURE_ALERT_FLASH_PERIOD_SECONDS);

            // Initialize the colors
            infobox_peak_.SetAlertColoring(last_color_state_);
            tempslider_.SetAlertColoring(peak_temperature_alert_state_);
        }
        // Check flash timer
        if (alert_flash_peak_temperature_timer_.IsComplete())
        {
            // Reset countdown timer
            alert_flash_peak_temperature_timer_.Start(PEAK_TEMPERATURE_ALERT_FLASH_PERIOD_SECONDS);

            // Toggle colors
            infobox_peak_.SetAlertColoring(!last_color_state_);
            if (last_color_state_)
                tempslider_.SetAlertColoring(GUIElementTempSlider::AlertColoring::NONE);
            else
                tempslider_.SetAlertColoring(peak_temperature_alert_state_);

            last_color_state_ = !last_color_state_;
        }

        infobox_peak_.Draw();
        infobox_peak_.Refresh();
    }
    // If this is the first time we are not in an alert state return to normal colors
    else if (alert_initiated_)
    {
        infobox_peak_.SetAlertColoring(false);
        tempslider_.SetAlertColoring(peak_temperature_alert_state_);
        alert_initiated_ = false;

        infobox_peak_.Draw();
        infobox_peak_.Refresh();
    }
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetPeakTemperature(double temperature)
{
    if (!show_screen_main_)
        return;
    infobox_peak_.SetCurrentTemp(temperature);

    // Check if the temperature is outside the range and an alert is needed
    if (temperature < tempslider_.GetColdPointerTemperature())
        peak_temperature_alert_state_ = GUIElementTempSlider::AlertColoring::LOW_TEMP_ALERT;

    else if (temperature > tempslider_.GetHotPointerTemperature())
        peak_temperature_alert_state_ = GUIElementTempSlider::AlertColoring::HIGH_TEMP_ALERT;

    else
        peak_temperature_alert_state_ = GUIElementTempSlider::AlertColoring::NONE;
    // Call this here to ensure quick response
    PeakTemperatureColorController();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::DisablePeakTemperature()
{
    if (!show_screen_main_)
        return;

    peak_temperature_alert_state_ = GUIElementTempSlider::AlertColoring::NONE;
    // Call this here to force any alert coloring off
    PeakTemperatureColorController();

    infobox_peak_.Disable();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::DisableCurrentTemperature()
{
    if (!show_screen_main_)
        return;

    infobox_esophageal_.Disable();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetCurrentTemperature(double temperature)
{
    if (!show_screen_main_)
        return;

    infobox_esophageal_.SetCurrentTemp(temperature);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::TextButtonClicked()
{
    if (button_clicked_)
        button_clicked_();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::DateBarClicked()
{
    // If the datebar is set to active, then show the time/date popup
    if (datebar_clicked_active_)
    {
        show_screen_main_ = false;

        // Render the time date screen
        Render();

        time(&popup_time_);

        char version[32];
        snprintf(version, sizeof(version), "Version %u.%u.%u", Parameters::SoftwareVersion::REVISION_MAJOR,
                                                           Parameters::SoftwareVersion::REVISION_MINOR,
                                                           Parameters::SoftwareVersion::REVISION_PATCH);
        version_static_.UpdateText(version);
        version_static_.Refresh();
        RefreshTimeDateValues();
    }
}

//-----------------------------------------------------------------------------
void GUIScreenMain::PeakClicked()
{
    if (peak_temp_clicked_)
        peak_temp_clicked_();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::PopupClicked()
{
    // Dismiss the button from the GUI, and make it not clickable
    infobox_popup_.DismissButton();
    infobox_popup_.SetClickable(false);

    if (popup_clicked_)
        popup_clicked_();

    infobox_popup_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::RefreshTimeDateValues()
{
    char buffer[5];
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);

    strftime(buffer, sizeof(buffer), "%m", &timedate_setpoint);
    timedate_month_.UpdateText(buffer);
    timedate_month_.Refresh();

    strftime(buffer, sizeof(buffer), "%d", &timedate_setpoint);
    timedate_day_.UpdateText(buffer);
    timedate_day_.Refresh();

    strftime(buffer, sizeof(buffer), "%Y", &timedate_setpoint);
    timedate_year_.UpdateText(buffer);
    timedate_year_.Refresh();

    strftime(buffer, sizeof(buffer), "%H", &timedate_setpoint);
    timedate_hour_.UpdateText(buffer);
    timedate_hour_.Refresh();

    strftime(buffer, sizeof(buffer), "%M", &timedate_setpoint);
    timedate_minute_.UpdateText(buffer);
    timedate_minute_.Refresh();

    strftime(buffer, sizeof(buffer), "%S", &timedate_setpoint);
    timedate_second_.UpdateText(buffer);
    timedate_second_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::DayUpClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_mday++;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::DayDownClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_mday--;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::MonthUpClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);  // NOLINT [runtime/threadsafe_fn]
    timedate_setpoint.tm_mon++;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::MonthDownClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_mon--;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::YearUpClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_year++;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::YearDownClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_year--;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::HourUpClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_hour++;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::HourDownClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_hour--;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::MinuteUpClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_min++;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::MinuteDownClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_min--;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SecondUpClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_sec++;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SecondDownClicked()
{
    // Use the standard library to easily manipulate the datetime
    struct tm timedate_setpoint;
    localtime_r(&popup_time_, &timedate_setpoint);
    timedate_setpoint.tm_sec--;
    popup_time_ = mktime(&timedate_setpoint);

    RefreshTimeDateValues();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::OKClicked()
{
    // Call the time date update callback
    if (time_date_changed_)
        time_date_changed_(popup_time_);

    show_screen_main_ = true;

    // Render the main screen
    Render();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::CancelClicked()
{
    show_screen_main_ = true;

    // Render the main screen
    Render();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::CloseDateBarMenu()
{
    // if the main screen is hidden, call CancelClicked
    if (!show_screen_main_)
        CancelClicked();
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetHotTemperatureLimit(int temperature)
{
    tempslider_.SetHotPointerTemperature(temperature);
}

//-----------------------------------------------------------------------------
void GUIScreenMain::SetColdTemperatureLimit(int temperature)
{
    tempslider_.SetColdPointerTemperature(temperature);
}

//-----------------------------------------------------------------------------
void GUIScreenAux::Render()
{
    gui_screen_.Render();

    // Post render, update the version string
    char version[64];
    snprintf(version, sizeof(version), "Aurora Thermographic System, Version %d.%d.%d",
                                                       Parameters::SoftwareVersion::REVISION_MAJOR,
                                                       Parameters::SoftwareVersion::REVISION_MINOR,
                                                       Parameters::SoftwareVersion::REVISION_PATCH);
    version_static_.UpdateText(version);
    version_static_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenAux::SetPopupAlert(uint8_t alert_type, const char * message, bool show_confirm_button)
{
    SetPopupAlertInternal(alert_type, message, show_confirm_button,
                          GUIElementInfoboxAlert::ActionOnDisablePopupAlert::CLEAR_POPUP);
}

//-----------------------------------------------------------------------------
void GUIScreenAux::SetPopupAlertWithAcknowledgeButNotDismiss(uint8_t alert_type, const char * message)
{
    SetPopupAlertInternal(alert_type, message, true,
                          GUIElementInfoboxAlert::ActionOnDisablePopupAlert::REMOVE_CONFIRM_BUTTON);
}

//-----------------------------------------------------------------------------
void GUIScreenAux::SetPopupAlertInternal(uint8_t alert_type, const char * message, bool show_confirm_button,
                                         GUIElementInfoboxAlert::ActionOnDisablePopupAlert action_on_disable_popup_alert)
{
    // Prepare popup alert to be displayed, if alert_type is higher priority than
    // current popup (or there is no current popup) this alert will be displayed,
    // if a higher priority popup is currently displayed it will wait until the
    // current popup is cleared before being displayed.
    infobox_popup_.EnablePopupAlert(alert_type, message, show_confirm_button, action_on_disable_popup_alert);
    // Explicitly remove the button because the Aux display can't be used for this
    // regardless of whether the popup is set as clickable
    RemovePopupButton();

    // Necessary because infobox_popup_ does not automatically refresh itself
    infobox_popup_.Refresh();
    if (infobox_status_.GetVisible())
        infobox_status_.SetVisible(false);
}

//-----------------------------------------------------------------------------
// This function will only clear the highest priority popup unless that popup
// is set as not clickable
void GUIScreenAux::ClearPopupAlert()
{
    // DisablePopupAlert returns true if there are no
    // more popups to display, in which case we call
    // ClearPopup().
    // If the highest level popup isn't set as clearable
    // this call will not do anything and DisablePopupAlert
    // will return false.
    if (infobox_popup_.DisablePopupAlert())
    {
        ClearPopup();
    }
    // If there is still a popup to draw, refresh the infobox_popup
    else
    {
        // Remove the button because the Aux display can't be used for this
        RemovePopupButton();
        // Necessary because infobox_popup_ does not automatically refresh itself
        infobox_popup_.Draw();
        infobox_popup_.Refresh();
    }
}

//-----------------------------------------------------------------------------
// This function will always clear the popup alert_type passed to it
void GUIScreenAux::ClearPopupAlertExplicit(uint8_t alert_type)
{
    // DisablePopupAlert returns true if there are no
    // more popups to display, in which case we call
    // ClearPopup()
    if (infobox_popup_.DisablePopupAlertExplicit(alert_type))
    {
        ClearPopup();
    }
    // If there are more popups pass false back to inform the
    // state machine to reset the popup clicked callback
    // Also draw and refresh infobox_popup to display new popup
    else
    {
        // Remove the button because the Aux display can't be used for this
        RemovePopupButton();
        // Necessary because infobox_popup_ does not automatically refresh itself
        infobox_popup_.Draw();
        infobox_popup_.Refresh();
    }
}

//-----------------------------------------------------------------------------
// This function will always clear all pop-up alerts
void GUIScreenAux::ClearAllPopupAlerts()
{
    // Clear all pop-ups and display status box
    for (size_t i = 0; i < static_cast<uint8_t>(GUIElementInfoboxAlert::PopupAlertType::NUMBER_OF_ALERTS); i++)
    {
        infobox_popup_.DisablePopupAlertExplicit(i);
    }

    ClearPopup();
}

//-----------------------------------------------------------------------------
void GUIScreenAux::ClearPopup()
{
    infobox_status_.SetVisible(true);
    infobox_status_.Draw();
    infobox_status_.Refresh();
    infobox_popup_.SetVisible(false);
}

//-----------------------------------------------------------------------------
void GUIScreenAux::RemovePopupButton()
{
    infobox_popup_.DismissButton();
    infobox_popup_.SetClickable(false);
    infobox_popup_.Draw();
    infobox_popup_.Refresh();
}

//-----------------------------------------------------------------------------
void GUIScreenAux::SetStatusBox(const char *status)
{
    return SetStatusBox(static_cast<uint8_t>(GUIElementInfoboxStatus::StatusType::GENERAL), status);
}

//-----------------------------------------------------------------------------
void GUIScreenAux::SetStatusBox(uint8_t status_type, const char *status)
{
    // If pop-up is present don't override it, but set the status message
    // ClosePopup() will reveal the status box in this case
    infobox_status_.SetVisible(true);
    infobox_status_.Draw();
    infobox_status_.UpdateText(status_type, status);

    // If no pop-up is present set and reveal status box
    if (!infobox_popup_.GetVisible())
    {
        infobox_status_.Refresh();
        infobox_popup_.SetVisible(false);
    }
    else
    {
        infobox_popup_.Draw();
        infobox_popup_.Refresh();
        infobox_status_.SetVisible(false);
    }
}

//-----------------------------------------------------------------------------
void GUIScreenAux::ClearStatusTypeFromStatusBox(uint8_t status_type)
{
    infobox_status_.ClearStatusByType(status_type);

    // If no pop-up is present set and reveal status box
    if (!infobox_popup_.GetVisible())
    {
        infobox_status_.Refresh();
        infobox_popup_.SetVisible(false);
    }
    else
    {
        infobox_popup_.Draw();
        infobox_popup_.Refresh();
        infobox_status_.SetVisible(false);
    }
}

//-----------------------------------------------------------------------------
void GUIScreenAux::PeakTemperatureColorController()
{
    if (peak_temperature_alert_state_ != GUIElementTempSlider::AlertColoring::NONE)
    {
        // If this is the first time entering initiate the colors and timer
        if (alert_initiated_ == false)
        {
            alert_initiated_ = true;
            last_color_state_ = true;

            // Start countdown timer that facilitates toggling colors
            alert_flash_peak_temperature_timer_.Start(PEAK_TEMPERATURE_ALERT_FLASH_PERIOD_SECONDS);

            // Initialize the colors
            infobox_peak_.SetAlertColoring(last_color_state_);
            tempslider_.SetAlertColoring(peak_temperature_alert_state_);
        }
        // Check flash timer
        if (alert_flash_peak_temperature_timer_.IsComplete())
        {
            // Reset countdown timer
            alert_flash_peak_temperature_timer_.Start(PEAK_TEMPERATURE_ALERT_FLASH_PERIOD_SECONDS);

            // Toggle colors
            infobox_peak_.SetAlertColoring(!last_color_state_);
            if (last_color_state_)
                tempslider_.SetAlertColoring(GUIElementTempSlider::AlertColoring::NONE);
            else
                tempslider_.SetAlertColoring(peak_temperature_alert_state_);

            last_color_state_ = !last_color_state_;
        }

        infobox_peak_.Draw();
        infobox_peak_.Refresh();
    }
    // If this is the first time we are not in an alert state return to normal colors
    else if (alert_initiated_)
    {
        infobox_peak_.SetAlertColoring(false);
        tempslider_.SetAlertColoring(peak_temperature_alert_state_);
        alert_initiated_ = false;

        infobox_peak_.Draw();
        infobox_peak_.Refresh();
    }
}

//-----------------------------------------------------------------------------
void GUIScreenAux::SetPeakTemperature(double temperature)
{
    infobox_peak_.SetCurrentTemp(temperature);

    // Check if the temperature is outside the range and an alert is needed
    if (temperature < tempslider_.GetColdPointerTemperature())
        peak_temperature_alert_state_ = GUIElementTempSlider::AlertColoring::LOW_TEMP_ALERT;

    else if (temperature > tempslider_.GetHotPointerTemperature())
        peak_temperature_alert_state_ = GUIElementTempSlider::AlertColoring::HIGH_TEMP_ALERT;

    else
        peak_temperature_alert_state_ = GUIElementTempSlider::AlertColoring::NONE;
    // Call this here to ensure quick response
    PeakTemperatureColorController();

    // Only update the graph if enough time has passed since the last update.
    auto now = std::chrono::steady_clock::now();
    // Creation of local from GUIElementLineGraph::MINIMUM_SECONDS_BETWEEN_SAMPLES constexpr prevents a linker error
    if ((now - last_graph_sample_time_) >= std::chrono::seconds(int(GUIElementLineGraph::MINIMUM_SECONDS_BETWEEN_SAMPLES)))
    {
        last_graph_sample_time_ = now;
        linegraph_.Update(temperature);
        linegraph_.RefreshGraphArea();
    }
}

//-----------------------------------------------------------------------------
void GUIScreenAux::DisablePeakTemperature()
{
    peak_temperature_alert_state_ = GUIElementTempSlider::AlertColoring::NONE;
    // Call this here to force any alert coloring off
    PeakTemperatureColorController();

    infobox_peak_.Disable();
}

//-----------------------------------------------------------------------------
void GUIScreenAux::SetCurrentTemperature(double temperature)
{
    infobox_esophageal_.SetCurrentTemp(temperature);
}

//-----------------------------------------------------------------------------
void GUIScreenAux::DisableCurrentTemperature()
{
    infobox_esophageal_.Disable();
}

//-----------------------------------------------------------------------------
void GUIScreenAux::SetHotTemperatureLimit(int temperature)
{
    tempslider_.SetHotPointerTemperature(temperature);

    linegraph_.UpdateHotLimit(temperature);
    linegraph_.RefreshGraphArea();
}

//-----------------------------------------------------------------------------
void GUIScreenAux::OnHotPointerDirtyRectangle()
{
    // At the extremities, a portion of the rendering buffer under the control
    // of the backplate (rather than the tempslider) is updated. As such,
    // at the extremities, the backplate must be redrawn to the rendering buffer,
    // then the temperature slider (again). Lastly, the changes from the rendering
    // buffer need to be pushed to the framebuffer with a refresh. Only the
    // overwritten portion actually need to be pushed to the framebuffer.
    backplate_.Draw();
    tempslider_.Draw();

    // Redraw the area including the entire temp slider and the backplate above it
    context_.ForceRedraw(1163, 57, 1243, 637);
}

//-----------------------------------------------------------------------------
void GUIScreenAux::SetColdTemperatureLimit(int temperature)
{
    tempslider_.SetColdPointerTemperature(temperature);

    linegraph_.UpdateColdLimit(temperature);
    linegraph_.RefreshGraphArea();
}

//-----------------------------------------------------------------------------
void GUIScreenAux::OnColdPointerDirtyRectangle()
{
    // At the extremities, a portion of the rendering buffer under the control
    // of the backplate (rather than the tempslider) is updated. As such,
    // at the extremities, the backplate must be redrawn to the rendering buffer,
    // then the temperature slider (again). Lastly, the changes from the rendering
    // buffer need to be pushed to the framebuffer with a refresh. Only the
    // overwritten portion actually need to be pushed to the framebuffer.
    backplate_.Draw();
    tempslider_.Draw();

    // Redraw the area including the entire temp slider and the backplate below it
    context_.ForceRedraw(1163, 85, 1243, 680);
}

//-----------------------------------------------------------------------------
void GUIScreenAux::PausePeakTemperatureGraph()
{
    linegraph_.Pause();
}
