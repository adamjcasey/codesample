/*------------------------------------------------------------------------------
 Copyright © 2016 Continuum

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

Created by Jared Kirschner 2016
------------------------------------------------------------------------------*/

#ifndef TEST_INCLUDE_MOCK_HARDWARE_H_
#define TEST_INCLUDE_MOCK_HARDWARE_H_

#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <utility>

#include "include/a_scan.h"
#include "include/a_scan_broadcaster_interface.h"
#include "include/a_scan_handler_interface.h"
#include "include/audio_notification_interface.h"
#include "include/b_scan.h"
#include "include/b_scan_interface.h"
#include "include/result_outputs.h"
#include "include/file_manager_interface.h"
#include "include/device_storage_interface.h"
#include "include/gpio_interface.h"
#include "include/fpga_dual_port_ram.h"
#include "include/fpga_memory_layout_interface.h"
#include "include/guillotine_interface.h"
#include "include/gui_context_interface.h"
#include "include/gui_screen_interface.h"
#include "include/hardware_version_verifier_interface.h"
#include "include/infrared_amplifier_interface.h"
#include "include/infrared_imager_interface.h"
#include "include/infrared_scan.h"
#include "include/lifetime_updater_interface.h"
#include "include/log_file_interface.h"
#include "include/log_manager_interface.h"
#include "include/system_interface.h"
#include "include/system_file_interface.h"
#include "include/system_time_interface.h"
#include "include/fpga_memory_mapped_registers_interface.h"
#include "include/motor_controller_interface.h"
#include "include/motor_monitor_interface.h"
#include "include/piezo_speaker_interface.h"
#include "include/power_controller_interface.h"
#include "include/probe_interface.h"
#include "include/procedure_interface.h"
#include "include/rfid_interface.h"
#include "include/state_of_health_interface.h"
#include "include/streaming_port_interface.h"
#include "include/thermocouple_interface.h"
#include "include/touchscreen_interface.h"
#include "include/uart_interface.h"
#include "include/watchdog_interface.h"
#include "vendor/google/gmock/include/gmock/gmock.h"
#include "vendor/google/gtest/include/gtest/gtest.h"

// Import of entire testing namespace useful in unit test files
using namespace ::testing;  // NOLINT(build/namespaces)

ACTION_P(SetErrno, val ) { errno = val; }

ACTION_P(Copy, data) { strncpy(arg0, data, arg1); }

ACTION_P(AssignPtr, value) { *arg0 = value; }

ACTION_P(AssignRef, value) { arg0 = value; }

ACTION_P(Sleep, val) { sleep(val); }

ACTION_P(USleep, val) { usleep(val); }

ACTION_P(SetPollFdRevents, flags) { arg0->revents = static_cast<uint16_t>(flags); }

ACTION_P(SetStatMode, mode) { arg0->st_mode = static_cast<mode_t>(mode); }

// Used to set pointee argument to input of mock function call, as in UART::Write().
// NOTE: the type of "dest" needs to be a pointer to a pointer, so that the
// position of the pointer can be updated.
ACTION_TEMPLATE(CopyArgPointeeWithLengthLimit,
                HAS_2_TEMPLATE_PARAMS(size_t, src_index, size_t, src_len_index),
                AND_2_VALUE_PARAMS(dest, dest_size_max))
{
    size_t len_copy = std::min(std::get<src_len_index>(args), dest_size_max);
    std::memcpy(*dest, std::get<src_index>(args), len_copy);
    *dest += len_copy;
}

// Used to set pointee argument to expectation from mock function call, as in UART::Read().
ACTION_TEMPLATE(SetArgPointeeWithLengthLimit,
                HAS_2_TEMPLATE_PARAMS(size_t, dest_index, size_t, dest_len_index),
                AND_2_VALUE_PARAMS(src, src_size_max))
{
    size_t len_copy = std::min(std::get<dest_len_index>(args), src_size_max);
    std::memcpy(std::get<dest_index>(args), src, len_copy);
}

MATCHER_P2(BufferEqual, data, size, "Matches if the memory is the same for given pointer and size")
{
    return !std::memcmp(arg, data, size);
}

//-----------------------------------------------------------------------------
class MockAScanBroadcaster : public IAScanBroadcaster
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD1(Broadcast, std::error_code(AScan *a_scan));
};

//-----------------------------------------------------------------------------
class MockAScanHandler : public IAScanHandler
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Reset, void());
        MOCK_CONST_METHOD0(ConfigureScanCallbacks, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(HandleScansInBuffer, std::error_code());
        MOCK_CONST_METHOD0(ReturnThenClearLastError, std::error_code());
};

//-----------------------------------------------------------------------------
class MockAudioNotification : public IAudioNotification
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(SoundPersistentAlertBeep, std::error_code());
        MOCK_CONST_METHOD0(SoundAlertBeep, std::error_code());
        MOCK_CONST_METHOD0(SoundNotificationBeep, std::error_code());
        MOCK_CONST_METHOD0(Silence, std::error_code());
        MOCK_CONST_METHOD0(HardSilence, std::error_code());
};

//-----------------------------------------------------------------------------
class MockBScanAssembler : public IBScanAssembler
{
 public:
        MOCK_METHOD1(ProcessAScan, void(AScan& a_scan));
        MOCK_METHOD0(ResetOnImagingSessionStart, void());
        MOCK_METHOD2(SetBScanAvailableCallback, void(
            IBScanAssembler::BScanAvailableCallback callback, IBScanAssembler::BScanErrorCallback error_callback));
};

//-----------------------------------------------------------------------------
class MockDataFile : public IDataFile
{
 public:
        MOCK_CONST_METHOD0(Initialize, void());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD1(Log, std::error_code(BScan& b_scan));
};

//-----------------------------------------------------------------------------
class MockDeviceStorage : public IDeviceStorage
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD1(WriteUsageTime, std::error_code(uint32_t usage_time_s));
        MOCK_CONST_METHOD1(WriteLastServiceTimestamp, std::error_code(uint32_t epoch_service_timestamp));
        MOCK_CONST_METHOD0(ReadUsageTime, IDeviceStorage::UnsignedIntResult());
        MOCK_CONST_METHOD0(ReadLastServiceTimestamp, IDeviceStorage::UnsignedIntResult());
        MOCK_CONST_METHOD0(ReadSerialNumber, IDeviceStorage::UnsignedIntResult());
        MOCK_CONST_METHOD0(ReadNotificationTemperatureCelsiusHot, IDeviceStorage::IntResult());
        MOCK_CONST_METHOD0(ReadNotificationTemperatureCelsiusCold, IDeviceStorage::IntResult());
        MOCK_CONST_METHOD1(WriteNotificationTemperatureCelsiusHot, std::error_code(int setting_value));
        MOCK_CONST_METHOD1(WriteNotificationTemperatureCelsiusCold, std::error_code(int setting_value));
};

//-----------------------------------------------------------------------------
class MockEStopFile : public IEStopFile
{
 public:
        MOCK_CONST_METHOD0(Initialize, void());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(EmptyAScanBuffer, void());
        MOCK_CONST_METHOD1(AddAScanToBuffer, std::error_code(AScan& a_scan));
        MOCK_CONST_METHOD0(FlushAScanBufferToDisk, std::error_code());
};

//-----------------------------------------------------------------------------
class MockFlashManager : public IFileManager
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD2(OpenFile, IFileManager::OpenFileResult(const char * filename, int oflag));
};

//-----------------------------------------------------------------------------
class MockFPGADualPortRAM : public IFPGADualPortRAM
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD2(GetBankAddressInfo, std::error_code(Bank bank, BankAddressInfo * bank_address_info));
};

//-----------------------------------------------------------------------------
class MockFPGAMemoryLayout : public IFPGAMemoryLayout
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD2(GetAddressInMemorySegment, char *(
            IFPGAMemoryLayout::MemorySegment memory_segment, size_t address_offset));
};

//-----------------------------------------------------------------------------
class MockFPGAMemoryMappedRegisters : public IFPGAMemoryMappedRegisters
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD1(GetClockPeriod, std::error_code(uint32_t * clock_period_container));
        MOCK_CONST_METHOD1(GetAScanSize, std::error_code(uint32_t * a_scan_size_container));
        MOCK_CONST_METHOD1(GetAScanFormat, std::error_code(uint32_t * a_scan_format_container));
        MOCK_CONST_METHOD1(GetRotarySpeed, std::error_code(uint32_t * rotational_speed_container));
        MOCK_CONST_METHOD1(GetEmergencyStopError, std::error_code(uint32_t * emergency_stop_error_container));
        MOCK_CONST_METHOD1(GetLinearPosition, std::error_code(uint32_t * linear_position_container));
        MOCK_CONST_METHOD1(GetThermocoupleError, std::error_code(uint32_t * thermocouple_error_container));
        MOCK_CONST_METHOD1(GetThermocoupleTemperature, std::error_code(uint32_t * thermocouple_temperature_container));
        MOCK_CONST_METHOD1(GetThermocoupleTimestamp, std::error_code(uint64_t * thermocouple_timestamp_container));

        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsVrefDiv2, std::error_code(uint32_t * soh_vref_div2_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsVrefDiv4, std::error_code(uint32_t * soh_vref_div4_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsLinearMotorCurrent,
            std::error_code(uint32_t * soh_linear_motor_current_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsRotaryMotorCurrent,
            std::error_code(uint32_t * soh_rotary_motor_current_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsPcbTemperature, std::error_code(uint32_t * soh_temperature_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsDetectorBias, std::error_code(uint32_t * soh_detector_bias_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsV24, std::error_code(uint32_t * soh_v24_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsV15Positive, std::error_code(uint32_t * soh_v15_positive_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsV15Negative, std::error_code(uint32_t * soh_v15_negative_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsDetectorZero, std::error_code(uint32_t * soh_detector_zero_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsSterlingDiode, std::error_code(uint32_t * soh_sterline_diode_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsVdd, std::error_code(uint32_t * soh_vdd_container));
        MOCK_CONST_METHOD1(GetStateOfHealthAdcCountsVdda, std::error_code(uint32_t * soh_vdda_container));

        MOCK_CONST_METHOD1(GetIrBufferSamplesPerRev, std::error_code(uint32_t * samples_per_rev_container));
        MOCK_CONST_METHOD1(GetIrBufferNumberOfAScans, std::error_code(uint32_t * number_of_a_scans_container));
        MOCK_CONST_METHOD1(GetIrBufferIrDataOffset, std::error_code(uint32_t * ir_data_offset_container));
        MOCK_CONST_METHOD1(GetIrBufferRamSize, std::error_code(uint32_t * ram_size_container));
        MOCK_CONST_METHOD1(GetIrBufferStartAddressBank1, std::error_code(uint32_t * bank_1_start_address_container));
        MOCK_CONST_METHOD1(GetIrBufferStartAddressBank2, std::error_code(uint32_t * bank_2_start_address_container));

        MOCK_CONST_METHOD1(GetLinearEmergencyStopReason, std::error_code(uint32_t * linear_emergency_stop_reason_container));
        MOCK_CONST_METHOD1(GetRotaryEmergencyStopReason, std::error_code(uint32_t * rotary_emergency_stop_reason_container));

        MOCK_CONST_METHOD1(GetRotarySpeedSimple, std::error_code(uint32_t * rotary_speed_simple_container));

        MOCK_CONST_METHOD1(SetDebugUartControl, std::error_code(uint32_t debug_uart_control_to_set));
        MOCK_CONST_METHOD1(SetPiezoOnTime, std::error_code(uint32_t on_time_cycles));
        MOCK_CONST_METHOD1(SetPiezoOffTime, std::error_code(uint32_t off_time_cycles));
        MOCK_CONST_METHOD1(SetPiezoNumberOfOnOffCycles, std::error_code(uint32_t number_of_on_off_cycles));
};

//-----------------------------------------------------------------------------
class MockGPIO : public IGPIO
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD1(SetDirection, std::error_code(IGPIO::Direction direction));
        MOCK_CONST_METHOD1(SetEdge, std::error_code(IGPIO::Edge));
        MOCK_CONST_METHOD1(SetValue, void(bool));
        MOCK_CONST_METHOD0(GetValue, bool());
        MOCK_CONST_METHOD1(PollValue, std::pair<std::error_code, bool>(int timeout_ms));
};

//-----------------------------------------------------------------------------
class MockGUIContext : public IGUIContext
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(Clear, void());
        MOCK_CONST_METHOD0(ForceRedraw, void());
        MOCK_CONST_METHOD4(ForceRedraw, void(int x0, int y0, int x1, int y1));
        MOCK_CONST_METHOD0(Buffer, uint8_t*());
        MOCK_CONST_METHOD0(Width, uint16_t());
        MOCK_CONST_METHOD0(Height, uint16_t());
        MOCK_CONST_METHOD0(Stride, int());
        MOCK_CONST_METHOD3(SetPixelDirectly, void(uint16_t x, uint16_t y, uint32_t rgbx));
        MOCK_CONST_METHOD5(SetPixelRegionDirectly, void(uint16_t x_start, uint16_t y_start,
                                                        uint16_t x_width, uint16_t y_height,
                                                        uint32_t * rgbx_buffer));
};

//-----------------------------------------------------------------------------
class MockGuillotine : public IGuillotine
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(TriggerOpen, std::error_code());
        MOCK_CONST_METHOD0(TriggerClose, std::error_code());
        MOCK_CONST_METHOD0(ClearOpen, std::error_code());
        MOCK_CONST_METHOD0(ClearClose, std::error_code());
        MOCK_CONST_METHOD0(IsEnergized, bool());
        MOCK_CONST_METHOD0(Position, IGuillotine::SwitchState());
};

//-----------------------------------------------------------------------------
class MockGUIScreenAux : public IGUIScreenAux
{
 public:
        MOCK_METHOD0(Render, void());
        MOCK_METHOD2(TouchDown, void(uint16_t x, uint16_t y));
        MOCK_METHOD0(TouchUp, void());
        MOCK_METHOD0(PeakTempInfobox, GUIElementInfoboxPeakTemperature&());
        MOCK_METHOD0(CurrentTempInfobox, GUIElementInfoboxCoreTemperature&());
        MOCK_METHOD0(StatusInfobox, GUIElementInfoboxStatus&());
        MOCK_METHOD0(AlertInfobox, GUIElementInfoboxAlert&());
        MOCK_METHOD0(TimeDateBar, GUIElementTimeDateBar&());
        MOCK_METHOD0(TempSlider, GUIElementTempSlider&());
        MOCK_METHOD0(Heatmap, GUIElementHeatmap&());
        MOCK_METHOD0(LineGraph, GUIElementLineGraph&());

        MOCK_METHOD3(SetPopupAlert, void(uint8_t alert_type, const char *message, bool clearable_by_click));
        MOCK_METHOD2(SetPopupAlertWithAcknowledgeButNotDismiss, void(uint8_t alert_type, const char *message));
        MOCK_METHOD1(IsPopupAlertSet, bool(uint8_t alert_type));
        MOCK_METHOD0(ClearPopupAlert, void());
        MOCK_METHOD1(ClearPopupAlertExplicit, void(uint8_t alert_type));
        MOCK_METHOD0(ClearAllPopupAlerts, void());
        MOCK_METHOD2(SetPopup, void(bool error, const char *status));
        MOCK_METHOD0(ClearPopup, void());
        MOCK_METHOD1(SetStatusBox, void(const char *status));
        MOCK_METHOD2(SetStatusBox, void(uint8_t status_type, const char *status));
        MOCK_METHOD1(ClearStatusTypeFromStatusBox, void(uint8_t status_type));
        MOCK_METHOD1(SetPeakTemperature, void(double temperature));
        MOCK_METHOD0(DisablePeakTemperature, void());
        MOCK_METHOD0(DisableCurrentTemperature, void());
        MOCK_METHOD1(SetCurrentTemperature, void(double temperature));
        MOCK_METHOD1(SetHotTemperatureLimit, void(int temperature));
        MOCK_METHOD1(SetColdTemperatureLimit, void(int temperature));
        MOCK_METHOD0(PausePeakTemperatureGraph, void());
        MOCK_METHOD0(PeakTemperatureColorController, void());
};

//-----------------------------------------------------------------------------
class MockGUIScreenMain : public IGUIScreenMain
{
 public:
        MOCK_METHOD0(Render, void());
        MOCK_METHOD2(TouchDown, void(uint16_t x, uint16_t y));
        MOCK_METHOD0(TouchUp, void());
        MOCK_METHOD0(PeakTempInfobox, GUIElementInfoboxPeakTemperature&());
        MOCK_METHOD0(CurrentTempInfobox, GUIElementInfoboxCoreTemperature&());
        MOCK_METHOD0(SetTempInfobox, GUIElementInfoboxThresholdTemperature&());
        MOCK_METHOD0(StatusInfobox, GUIElementInfoboxStatus&());
        MOCK_METHOD0(AlertInfobox, GUIElementInfoboxAlert&());
        MOCK_METHOD0(TimeDateBar, GUIElementTimeDateBar&());
        MOCK_METHOD0(TempSlider, GUIElementTempSlider&());
        MOCK_METHOD0(ActionButton, GUIElementTextButton&());

        MOCK_METHOD2(SetTempSliderCallbacks, void(GUIElementTempSlider::TempSetpointReleaseCallback hot_released,
                                                    GUIElementTempSlider::TempSetpointReleaseCallback cold_released));
        MOCK_METHOD1(SetButtonClickedCallback, void(IGUIElement::ClickCallback button_clicked));
        MOCK_METHOD0(ClearButtonClickedCallback, void());
        MOCK_METHOD1(SetDateBarClickedActive, void(bool active));
        MOCK_METHOD1(SetPeakTempClickedCallback, void(IGUIElement::ClickCallback peak_temp_clicked));
        MOCK_METHOD1(SetPopupClickedCallback, void(IGUIElement::ClickCallback popup_clicked));
        MOCK_METHOD1(SetTimeDateChangedCallback, void(IGUIScreenMain::TimeDateChangedCallback time_date_changed));

        MOCK_METHOD0(TextButtonInactive, void());
        MOCK_METHOD0(TextButtonStartImaging, void());
        MOCK_METHOD0(TextButtonStop, void());
        MOCK_METHOD0(TextButtonStopImaging, void());

        MOCK_METHOD3(SetPopupAlert, void(uint8_t alert_type, const char *message, bool clearable_by_click));
        MOCK_METHOD2(SetPopupAlertWithAcknowledgeButNotDismiss, void(uint8_t alert_type, const char *message));
        MOCK_METHOD1(IsPopupAlertSet, bool(uint8_t alert_type));
        MOCK_METHOD0(IsPopupAlertDismissButtonTouchable, bool());
        MOCK_METHOD0(ClearPopupAlert, void());
        MOCK_METHOD1(ClearPopupAlertExplicit, void(uint8_t alert_type));
        MOCK_METHOD0(ClearAllPopupAlerts, void());
        MOCK_METHOD3(SetPopup, void(bool error, const char *status, bool clickable));
        MOCK_METHOD0(ClearPopup, void());
        MOCK_METHOD1(SetStatusBox, void(const char *status));
        MOCK_METHOD2(SetStatusBox, void(uint8_t status_type, const char *status));
        MOCK_METHOD1(ClearStatusTypeFromStatusBox, void(uint8_t status_type));
        MOCK_METHOD1(SetPeakTemperature, void(double temperature));
        MOCK_METHOD0(DisablePeakTemperature, void());
        MOCK_METHOD0(DisableCurrentTemperature, void());
        MOCK_METHOD1(SetCurrentTemperature, void(double temperature));
        MOCK_METHOD0(CloseDateBarMenu, void());
        MOCK_METHOD1(SetHotTemperatureLimit, void(int temperature));
        MOCK_METHOD1(SetColdTemperatureLimit, void(int temperature));
        MOCK_METHOD0(PeakTemperatureColorController, void());
};

//-----------------------------------------------------------------------------
class MockHardwareVersionVerifier : public IHardwareVersionVerifier
{
 public:
        MOCK_CONST_METHOD0(Verify, std::error_code());
};

//-----------------------------------------------------------------------------
class MockInfraredAmplifier : public IInfraredAmplifier
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(GetGain, std::pair<std::error_code, uint8_t>());
        MOCK_CONST_METHOD0(Close, std::error_code());
};

//-----------------------------------------------------------------------------
class MockInfraredImager : public IInfraredImager
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Enable, std::error_code());
        MOCK_CONST_METHOD0(Disable, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD2(SetInfraredScanAvailableCallback, std::error_code(
            IInfraredImager::InfraredScanAvailableCallback callback,
            IInfraredImager::InfraredScanErrorCallback error_callback));
};

//-----------------------------------------------------------------------------
class MockInputPin : public IInputPin
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(Query, bool());
};

//-----------------------------------------------------------------------------
class MockInputPinWithRisingEdgeCallback : public IInputPinWithRisingEdgeCallback
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(Query, bool());
        MOCK_CONST_METHOD1(SetRisingEdgeCallback, void(
            IInputPinWithRisingEdgeCallback::RisingEdgeCallback callback));
};

//-----------------------------------------------------------------------------
class MockLifetimeUpdater : public ILifetimeUpdater
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(ImagingSessionStarted, void());
        MOCK_CONST_METHOD0(ImagingSessionStopped, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(UpdateUsageTime, std::error_code());
        MOCK_CONST_METHOD0(IsServicingDue, bool());
};

//-----------------------------------------------------------------------------
class MockLogFile : public ILogFile
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD1(Log, std::error_code(const char *null_terminated_cmd));
        MOCK_CONST_METHOD2(Log, std::error_code(const char *cmd, size_t len));
        MOCK_CONST_METHOD0(FlushToDiskIfLoggingEnabled, std::error_code());
};

//-----------------------------------------------------------------------------
class MockLogManager : public ILogManager
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD3(Log, std::error_code(int log_file_descriptor, const char * buffer, size_t buffer_length));
        MOCK_CONST_METHOD0(AsynchronouslyEnableLogging, std::error_code());
        MOCK_CONST_METHOD0(AsynchronouslyDisableLogging, std::error_code());
        MOCK_CONST_METHOD0(IsAsynchronousLoggingEnableComplete, bool());
        MOCK_CONST_METHOD1(StartImagingSession, std::error_code(uint32_t probe_serial_number));
        MOCK_CONST_METHOD0(StopImagingSession, void());
        MOCK_CONST_METHOD0(GetImagingSessionID, uint32_t());
        MOCK_CONST_METHOD2(OpenLogFile, std::pair<std::error_code, int>(
            ILogManager::LogRoot log_root, const char *filename));
        MOCK_CONST_METHOD2(DoesFileExist, std::pair<std::error_code, bool>(
            ILogManager::LogRoot log_root, const char *filename));
        MOCK_CONST_METHOD0(GetInitDateTime, ISystemTime::DateTime());
        MOCK_CONST_METHOD0(GetImagingSessionDateTime, ISystemTime::DateTime());
        MOCK_CONST_METHOD0(GetProbeSerialNumber, uint32_t());
        MOCK_CONST_METHOD0(GetDeviceSerialNumber, uint32_t());
        MOCK_CONST_METHOD0(IsImagingSessionActive, bool());
};

//-----------------------------------------------------------------------------
class MockMotorController : public IMotorController
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD2(Command, std::error_code(const char *cmd, size_t len));
        MOCK_CONST_METHOD2(Query, std::pair<std::error_code, int32_t>(const char *cmd, size_t len));
        MOCK_CONST_METHOD0(Enable, std::error_code());
        MOCK_CONST_METHOD0(Disable, std::error_code());
};

//-----------------------------------------------------------------------------
class MockMotorMonitor : public IMotorMonitor
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Reset, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(IsRotaryMotorSpinningBackwards, IMotorMonitor::BoolResult());
};

//-----------------------------------------------------------------------------
class MockOutputPin : public IOutputPin
{
 public:
        MOCK_CONST_METHOD1(Initialize, std::error_code(bool initial_state_logic_high));
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(Set, void());
        MOCK_CONST_METHOD0(Clear, void());
        MOCK_CONST_METHOD0(Toggle, void());
        MOCK_CONST_METHOD0(Query, bool());
};

//-----------------------------------------------------------------------------
class MockPeakFile : public IPeakFile
{
 public:
        MOCK_CONST_METHOD0(Initialize, void());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD3(Log, std::error_code(
            double epoch_time_s, double peak_temperature_celsius, double core_temperature_celsius));
        MOCK_CONST_METHOD2(LogWithInvalidPeakTemperature, std::error_code(
            double epoch_time_s, double core_temperature_celsius));
};

//-----------------------------------------------------------------------------
class MockPiezoSpeaker : public IPiezoSpeaker
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD3(StartTone, std::error_code(double on_time_s, double off_time_s, uint32_t number_of_cycles));
        MOCK_CONST_METHOD0(StopTone, std::error_code());
};

//-----------------------------------------------------------------------------
class MockPowerController : public IPowerController
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD1(SetShutdownRequestCallback, void(ShutdownRequestCallback callback));
        MOCK_CONST_METHOD0(ProgrammaticShutdownRequest, void());
        MOCK_CONST_METHOD0(SafeShutdown, void());
        MOCK_CONST_METHOD0(SignalApplicationExiting, void());
        MOCK_CONST_METHOD0(Close, std::error_code());
};

//-----------------------------------------------------------------------------
class MockProbe : public IProbe
{
 public:
        MOCK_CONST_METHOD0(Initialize, void());
        MOCK_CONST_METHOD0(Close, void());
        MOCK_CONST_METHOD0(Attached, void());
        MOCK_CONST_METHOD0(Removed, void());
        MOCK_CONST_METHOD0(ImagingSessionStarted, void());
        MOCK_CONST_METHOD0(ImagingSessionStopped, void());
        MOCK_CONST_METHOD1(PreImagingSessionWrite, std::error_code(IRFID::RFIDOperationCompleteCallback callback));
        MOCK_CONST_METHOD2(PostImagingSessionWrite, std::error_code(ProbeErrorCode probe_error_code,
                                                                    IRFID::RFIDOperationCompleteCallback callback));
        MOCK_CONST_METHOD1(ReadFromRfidTag, std::error_code(IRFID::RFIDOperationCompleteCallback callback));
        MOCK_CONST_METHOD0(GetReadOnlyData, const ReadOnlyPayload *());
        MOCK_CONST_METHOD0(GetReadWriteData, const ReadWritePayload *());
        MOCK_CONST_METHOD1(GetProbeStatus, ProbeStatus(bool ignore_error_code));
};

//-----------------------------------------------------------------------------
class MockProcedure : public IProcedure
{
 public:
        MOCK_CONST_METHOD0(ClearProcedureHistory, void());
        MOCK_CONST_METHOD1(UpdateProcedureDeterminationHistory,
            IProcedure::ProcedureType(IProcedure::ProcedureType procedure_type));
        MOCK_CONST_METHOD0(GetCurrentProcedureDetermination, IProcedure::ProcedureType());
        MOCK_CONST_METHOD1(UpdatePeakTemperatureCelsiusHistory, void(double peak_temperature_celsius));
        MOCK_CONST_METHOD1(GetPeakTemperatureChangeCelsius, bool(double & peak_temperature_change_celsius_out));
        MOCK_CONST_METHOD1(GetPeakTemperatureAverageCelsius, bool(double & peak_temperature_average_celsius_out));
        MOCK_CONST_METHOD1(GetLowestCryoProcedureTemperatureCelsius, bool(double & temperature_celsius_out));
        MOCK_CONST_METHOD1(GetHighestHeatProcedureTemperatureCelsius, bool(double & temperature_celsius_out));
};

//-----------------------------------------------------------------------------
class MockRFID : public IRFID
{
 public:
        MOCK_CONST_METHOD0(Initialize, void());
        MOCK_CONST_METHOD0(Close, void());
        MOCK_CONST_METHOD3(Read, std::error_code(char (&read_only_payload)[IRFID::READ_ONLY_PAYLOAD_BYTES],
                                                 char (&read_write_payload)[IRFID::READ_WRITE_PAYLOAD_BYTES],
                                                 IRFID::RFIDOperationCompleteCallback callback));
        MOCK_CONST_METHOD4(Read, std::error_code(char (&read_only_payload)[READ_ONLY_PAYLOAD_BYTES],
                                                 char (&read_write_payload)[READ_WRITE_PAYLOAD_BYTES],
                                                 int * discovered_tag,
                                                 IRFID::RFIDOperationCompleteCallback callback));
        MOCK_CONST_METHOD3(Write, std::error_code(const char (&read_only_payload)[READ_ONLY_PAYLOAD_BYTES],
                                                  const char (&read_write_payload)[READ_WRITE_PAYLOAD_BYTES],
                                                 IRFID::RFIDOperationCompleteCallback callback));
};

//-----------------------------------------------------------------------------
class MockRootFilesystemManager : public IFileManager
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD2(OpenFile, IFileManager::OpenFileResult(const char * filename, int oflag));
};

//-----------------------------------------------------------------------------
class MockRotationalMotorController : public IRotationalMotorController
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(Rotate, std::error_code());
        MOCK_CONST_METHOD0(IsAtSpeed, std::pair<std::error_code, bool>());
        MOCK_CONST_METHOD0(Halt, std::error_code());
        MOCK_CONST_METHOD0(IsHalted, std::pair<std::error_code, bool>());
};

//-----------------------------------------------------------------------------
class MockStateOfHealth : public IStateOfHealth
{
 public:
        MOCK_CONST_METHOD0(Check, StateOfHealthStatus());
        MOCK_CONST_METHOD0(VrefDiv2Voltage, float());
        MOCK_CONST_METHOD0(VrefDiv4Voltage, float());
        MOCK_CONST_METHOD0(LinearMotorCurrent, float());
        MOCK_CONST_METHOD0(RotaryMotorCurrent, float());
        MOCK_CONST_METHOD0(PcbTemperature, float());
        MOCK_CONST_METHOD0(DetectorBiasVoltage, float());
        MOCK_CONST_METHOD0(V24Voltage, float());
        MOCK_CONST_METHOD0(V15PositiveVoltage, float());
        MOCK_CONST_METHOD0(V15NegativeVoltage, float());
        MOCK_CONST_METHOD0(DetectorZeroVoltage, float());
        MOCK_CONST_METHOD0(SterlingDiodeVoltage, float());
        MOCK_CONST_METHOD0(VddVoltage, float());
        MOCK_CONST_METHOD0(VddaVoltage, float());
};

//-----------------------------------------------------------------------------
class MockStreamingPort : public IStreamingPort
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(EnableStreaming, void());
        MOCK_CONST_METHOD0(DisableStreaming, void());
        MOCK_CONST_METHOD1(WritePacket_SystemStatus, std::error_code(SystemStatus& system_status));
        MOCK_CONST_METHOD1(WritePacket_BScanData, std::error_code(BScanData& b_scan_data));
        MOCK_CONST_METHOD2(WritePacket_LogData, std::error_code(char *buffer, uint16_t len));
};

//-----------------------------------------------------------------------------
class MockSystem : public ISystem
{
 public:
        // File IO
        MOCK_CONST_METHOD3(Open, int(const char *path, int oflag, mode_t mode));
        MOCK_CONST_METHOD1(Close, int(int handle));
        MOCK_CONST_METHOD3(Read, ssize_t(int handle, char *buffer, size_t nbyte));
        MOCK_CONST_METHOD3(Write, ssize_t(int handle, const char *buffer, size_t nbyte));
        MOCK_CONST_METHOD1(Remove, int(const char* fname));
        MOCK_CONST_METHOD3(Lseek, off_t(int fd, off_t offset, int whence));
        MOCK_CONST_METHOD3(Poll, int(struct pollfd *fds, nfds_t nfds, int timeout));
        MOCK_CONST_METHOD3(Ioctl, int(int handle, uint32_t request, int parameter));
        MOCK_CONST_METHOD2(FTruncate, int(int file_descriptor, off_t size));

        MOCK_CONST_METHOD1(Opendir, DIR*(const char *name));
        MOCK_CONST_METHOD2(Mkdir, int(const char *pathname, mode_t mode));
        MOCK_CONST_METHOD1(Readdir, struct dirent*(DIR *dirp));
        MOCK_CONST_METHOD1(Closedir, int(DIR *dirp));

        MOCK_CONST_METHOD5(Mount, int(const char *source, const char *target, const char *filesystemtype,
            uint32_t mountflags, const void *data));
        MOCK_CONST_METHOD1(Umount, int(const char *target));

        MOCK_CONST_METHOD2(Stat, int(const char *path, struct stat *buf));
        MOCK_CONST_METHOD2(Statvfs, int(const char *path, struct statvfs *buf));

        MOCK_CONST_METHOD4(Nftw64, int(const char *dirpath, Nftw64FunctionCallback fn, int nopenfd, int flags));

        // Mmap
        MOCK_CONST_METHOD6(Mmap, void*(void *addr, size_t length, int prot, int flags, int fd, off_t offset));
        MOCK_CONST_METHOD2(Munmap, int(void *addr, size_t length));

        // I2C
        MOCK_CONST_METHOD2(SMBusReadByteData, int32_t(int fd, uint8_t addr));
        MOCK_CONST_METHOD3(SMBusWriteByteData, int32_t(int fd, uint8_t addr, uint8_t data));

        // UART
        MOCK_CONST_METHOD2(Tcgetattr, int(int fd, struct termios *termios_p));
        MOCK_CONST_METHOD3(Tcsetattr, int(int fd, int optional_actions, const struct termios *termios_p));
        MOCK_CONST_METHOD2(Cfsetispeed, int(struct termios *termios_p, speed_t speed));
        MOCK_CONST_METHOD2(Cfsetospeed, int(struct termios *termios_p, speed_t speed));
        MOCK_CONST_METHOD1(Cfmakeraw, void(struct termios *termios_p));

        // Power Controller
        MOCK_CONST_METHOD0(Sync, void());
        MOCK_CONST_METHOD1(Reboot, void(int cmd));

        // String
        MOCK_CONST_METHOD2(Strcmp, int(const char *s1, const char *s2));
        MOCK_CONST_METHOD3(Strncpy, char*(char *dest, const char *src, size_t n));

        // Time
        MOCK_CONST_METHOD1(Time, time_t(time_t *t));
        MOCK_CONST_METHOD2(Localtime_r, struct tm*(const time_t *timep, struct tm *result));
        MOCK_CONST_METHOD3(Strptime, char*(const char *s, const char *format, struct tm *tm));
        MOCK_CONST_METHOD1(Mktime, time_t(struct tm *timeptr));
        MOCK_CONST_METHOD1(Stime, int(time_t *t));
};

//-----------------------------------------------------------------------------
class MockSystemFile : public ISystemFile
{
 public:
        MOCK_CONST_METHOD3(Open, ISystemFile::OpenResult(const char *path, int oflag, mode_t mode));
        MOCK_CONST_METHOD1(Close, std::error_code(int handle));
        MOCK_CONST_METHOD3(Read, std::error_code(int handle, char *buffer, size_t nbyte));
        MOCK_CONST_METHOD3(Lseek, std::error_code(int handle, off_t offset, int whence));
        MOCK_CONST_METHOD3(Write, std::error_code(int handle, const char *buffer, size_t nbyte));
        MOCK_CONST_METHOD2(CreateDirectoryIfNonexistent, std::error_code(const char *directory_path, mode_t mode));
        MOCK_CONST_METHOD5(Mount, std::error_code(const char *source, const char *target, const char *filesystemtype,
            uint32_t mountflags, const void *data));
        MOCK_CONST_METHOD1(Umount, std::error_code(const char *target));
        MOCK_CONST_METHOD2(FTruncate, std::error_code(int file_descriptor, off_t size));
};

//-----------------------------------------------------------------------------
class MockSystemTime : public ISystemTime
{
 public:
        MOCK_CONST_METHOD0(Current, ISystemTime::DateTime());
        MOCK_CONST_METHOD1(ConvertEpochSecondsToDateTime, ISystemTime::DateTime(double epoch_seconds));
        MOCK_CONST_METHOD0(SecondsSinceEpoch, double());
        MOCK_CONST_METHOD1(Set, std::error_code(ISystemTime::DateTime date_time));
        MOCK_CONST_METHOD1(Set, std::error_code(time_t newtime));
};

//-----------------------------------------------------------------------------
class MockThermocouple : public IThermocouple
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(ReadTemperatureLatest, TemperatureCelsiusReading());
        MOCK_CONST_METHOD0(ReadTemperatureExponentialAverage, TemperatureCelsiusReading());
        MOCK_CONST_METHOD0(Enable, std::error_code());
        MOCK_CONST_METHOD0(Disable, std::error_code());
};

//-----------------------------------------------------------------------------
class MockTouchscreen : public ITouchscreen
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD2(SetCallbacks, void(TouchCallback touch_callback, ReleaseCallback release_callback));
};

//-----------------------------------------------------------------------------
class MockTranslationalMotorController : public ITranslationalMotorController
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(Home, std::error_code());
        MOCK_CONST_METHOD0(IsAtHome, std::pair<std::error_code, bool>());
        MOCK_CONST_METHOD0(MoveToStart, std::error_code());
        MOCK_CONST_METHOD0(IsAtStart, std::pair<std::error_code, bool>());
        MOCK_CONST_METHOD0(Translate, std::error_code());
        MOCK_CONST_METHOD0(StopTranslation, std::error_code());
        MOCK_CONST_METHOD0(IsStopped, std::pair<std::error_code, bool>());
};

//-----------------------------------------------------------------------------
class MockUART : public IUART
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD2(Read, std::error_code(char *buffer, size_t nbyte));
        MOCK_CONST_METHOD2(Write, std::error_code(const char *buffer, size_t nbyte));
};

//-----------------------------------------------------------------------------
class MockWatchdog : public IWatchdog
{
 public:
        MOCK_CONST_METHOD0(Initialize, std::error_code());
        MOCK_CONST_METHOD0(Close, std::error_code());
        MOCK_CONST_METHOD0(HasExpired, bool());
        MOCK_CONST_METHOD1(SetWatchdogExpirationCallback, void(WatchdogExpirationCallback callback));
        MOCK_CONST_METHOD1(ResetThreadExpiration, void(size_t thread_to_reset_id));
};

#endif  // TEST_INCLUDE_MOCK_HARDWARE_H_
