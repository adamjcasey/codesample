#include <windows.h>

#include <cstdio>
#include <cstdint>
#include <sstream>

#include "include/gui_element_heatmap.h"
#include "include/gui_element_infobox.h"
#include "include/gui_element_timedatebar.h"
#include "include/gui_element_tempslider.h"
#include "include/gui_element_button.h"
#include "include/gui_context_interface.h"
#include "include/gui_color.h"
#include "include/generic_pointer_wrapper_array.h"
#include "include/agg_wrapper.h"

#include "include/gui_screen_main.h"
#include "include/gui_screen_aux.h"


HWND hwnd_lcd, hwnd_dvi;
uint8_t buffer_hardware_lcd[272 * 480 * 4];
uint8_t buffer_hardware_dvi[1280 * 720 * 4];

double testmap[60][128];

class GUIContextLCD : public IGUIContext
{
public:
    GUIContextLCD() {}
    std::error_code Initialize() const;
    std::error_code Close() const;
    void Clear() const;
    void ForceRedraw() const;
    void ForceRedraw(int x0, int y0, int x1, int y1) const;
    uint8_t * Buffer() const { return buffer_renderer_; }
    uint16_t Width() const { return 272; }
    uint16_t Height() const { return 480; }
    int Stride() const { return 272 * 3; }
    void SetPixelDirectly(uint16_t x, uint16_t y, uint32_t rgbx) const;
    void SetPixelRegionDirectly(uint16_t x_start, uint16_t y_start,
        uint16_t x_width, uint16_t y_height,
        uint32_t * rgbx_buffer) const;

protected:
    static uint8_t buffer_renderer_[272 * 480 * 3];
    mutable uint8_t *buffer_hardware_;
};

uint8_t GUIContextLCD::buffer_renderer_[272 * 480 * 3];

//-----------------------------------------------------------------------------
std::error_code GUIContextLCD::Initialize() const
{
    // Get the pointer to the framebuffer
    buffer_hardware_ = buffer_hardware_lcd;

    return std::error_code();
}

//-----------------------------------------------------------------------------
std::error_code GUIContextLCD::Close() const
{
    return std::error_code();
}

//-----------------------------------------------------------------------------
void GUIContextLCD::Clear() const
{
    GUI::RenderingBuffer rbuf(Buffer(), Width(), Height(), Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    renderer_buffer.clear(GUI::Color(GUIColor(20, 38, 60)));
}

//-----------------------------------------------------------------------------
void GUIContextLCD::ForceRedraw() const
{
    ForceRedraw(0, 0, Width(), Height());
}

//-----------------------------------------------------------------------------
void GUIContextLCD::ForceRedraw(int x0, int y0, int x1, int y1) const
{
    HDC hdc = GetDC(hwnd_lcd);

    for (int y = y0; y < y1; y++)
    {
        int yval = y * Width() * 3;
        for (int x = x0; x < x1; x++)
        {
            int mapped_value = (x * 3) + yval;
            uint8_t red = buffer_renderer_[mapped_value + 0];
            uint8_t green = buffer_renderer_[mapped_value + 1];
            uint8_t blue = buffer_renderer_[mapped_value + 2];

            SetPixel(hdc, x, y, RGB(red, green, blue));
        }
    }

    ReleaseDC(hwnd_lcd, hdc);
}

//-----------------------------------------------------------------------------
void GUIContextLCD::SetPixelDirectly(uint16_t x, uint16_t y, uint32_t rgbx) const
{
    HDC hdc = GetDC(hwnd_lcd);
    SetPixel(hdc, x, y, (COLORREF)rgbx);
    ReleaseDC(hwnd_lcd, hdc);
}
//-----------------------------------------------------------------------------

void GUIContextLCD::SetPixelRegionDirectly(uint16_t x_start, uint16_t y_start,
    uint16_t x_width, uint16_t y_height,
    uint32_t * rgbx_buffer) const
{

}

class GUIContextDVI : public IGUIContext
{
public:
    GUIContextDVI() {}
    std::error_code Initialize() const;
    std::error_code Close() const;
    void Clear() const;
    void ForceRedraw() const;
    void ForceRedraw(int x0, int y0, int x1, int y1) const;
    uint8_t * Buffer() const { return buffer_renderer_; }
    uint16_t Width() const { return 1280; }
    uint16_t Height() const { return 720; }
    int Stride() const { return 1280 * 3; }
    void SetPixelDirectly(uint16_t x, uint16_t y, uint32_t rgbx) const;
    void SetPixelRegionDirectly(uint16_t x_start, uint16_t y_start,
        uint16_t x_width, uint16_t y_height,
        uint32_t * rgbx_buffer) const;

protected:
    static uint8_t buffer_renderer_[1280 * 720 * 3];
    mutable uint8_t *buffer_hardware_;
};

uint8_t GUIContextDVI::buffer_renderer_[1280 * 720 * 3];

//-----------------------------------------------------------------------------
std::error_code GUIContextDVI::Initialize() const
{
    // Get the pointer to the framebuffer
    buffer_hardware_ = buffer_hardware_dvi;

    return std::error_code();
}

//-----------------------------------------------------------------------------
std::error_code GUIContextDVI::Close() const
{
    return std::error_code();
}

//-----------------------------------------------------------------------------
void GUIContextDVI::Clear() const
{
    GUI::RenderingBuffer rbuf(Buffer(), Width(), Height(), Stride());
    GUI::GammaLutType gamma(1.8);
    GUI::PixelFormat pixel_format(rbuf, gamma);
    GUI::RendererBase renderer_buffer(pixel_format);
    renderer_buffer.clear(GUI::Color(GUIColor(20, 38, 60)));
}

//-----------------------------------------------------------------------------
void GUIContextDVI::ForceRedraw() const
{
    ForceRedraw(0, 0, Width(), Height());
}

//-----------------------------------------------------------------------------
void GUIContextDVI::ForceRedraw(int x0, int y0, int x1, int y1) const
{
    HDC hdc = GetDC(hwnd_dvi);

    for (int y = y0; y < y1; y++)
    {
        int yval = y * Width() * 3;
        for (int x = x0; x < x1; x++)
        {
            int mapped_value = (x * 3) + yval;
            uint8_t red = buffer_renderer_[mapped_value + 0];
            uint8_t green = buffer_renderer_[mapped_value + 1];
            uint8_t blue = buffer_renderer_[mapped_value + 2];

            SetPixel(hdc, x, y, RGB(red, green, blue));
        }
    }

    ReleaseDC(hwnd_dvi, hdc);
}

//-----------------------------------------------------------------------------
void GUIContextDVI::SetPixelDirectly(uint16_t x, uint16_t y, uint32_t rgbx) const
{
    HDC hdc = GetDC(hwnd_dvi);
    uint8_t *p = (uint8_t *)&rgbx;
    uint8_t red = p[2];
    uint8_t green = p[1];
    uint8_t blue = p[0];
    SetPixel(hdc, x, y, RGB(red, green, blue));
    ReleaseDC(hwnd_dvi, hdc);
}

void GUIContextDVI::SetPixelRegionDirectly(uint16_t x_start, uint16_t y_start,
    uint16_t x_width, uint16_t y_height,
    uint32_t * rgbx_buffer) const
{

}

GUIContextDVI context_aux_;
GUIContextLCD context_main_;
GUIScreenMain screen_main_(context_main_);
GUIScreenAux screen_aux_(context_aux_);
double temperature_core_ = 38.0;
double temperature_ir_ = 38.0;

//----------------------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WORD x, y;
    double update;
    switch (message)
    {
    case WM_PAINT:
        context_main_.ForceRedraw();
        context_aux_.ForceRedraw();
        break;
    case WM_LBUTTONDOWN:
        x = LOWORD(lParam);
        y = HIWORD(lParam);
        screen_main_.TouchDown(x, y);
        break;

    case WM_MOUSEMOVE:
        x = LOWORD(lParam);
        y = HIWORD(lParam);
        if (wParam == MK_LBUTTON)
        {
            screen_main_.TouchDown(x, y);
        }
        break;

    case WM_LBUTTONUP:
        x = LOWORD(lParam);
        y = HIWORD(lParam);
        screen_main_.TouchUp();
        break;

    case WM_CLOSE: // FAIL THROUGH to call DefWindowProc
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
            temperature_ir_ += 0.6;
            screen_aux_.SetPeakTemperature(temperature_ir_);
            screen_main_.SetPeakTemperature(temperature_ir_);
            break;

        case VK_DOWN:
            temperature_ir_ -= 0.6;
            screen_aux_.SetPeakTemperature(temperature_ir_);
            screen_main_.SetPeakTemperature(temperature_ir_);
            break;

        case VK_LEFT:
            temperature_core_ += 0.6;
            screen_aux_.SetCurrentTemperature(temperature_core_);
            screen_main_.SetCurrentTemperature(temperature_core_);
            break;

        case VK_RIGHT:
            temperature_core_ -= 0.6;
            screen_aux_.SetCurrentTemperature(temperature_core_);
            screen_main_.SetCurrentTemperature(temperature_core_);


            break;
        }
        break;

    case WM_TIMER:
           SetTimer(hwnd_lcd, 1, 1000, NULL);
           update = (rand() / (double)(RAND_MAX)) * 5.0 - 2.5;
           temperature_ir_ += update;
           screen_aux_.SetPeakTemperature(temperature_ir_);
           screen_main_.SetPeakTemperature(temperature_ir_);
           break;

    default:
        break; // FAIL to call DefWindowProc //
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void ButtonClicked()
{
    screen_main_.TextButtonStopImaging();

    screen_main_.SetPopupAlert(static_cast<uint8_t>(GUIElementInfoboxAlert::PopupAlertType::HARDWARE_FAILURE),
                                    "FAULT DETECTED\nREMOVE PROBE\nFROM PATIENT", true);
    screen_aux_.SetPopupAlert(static_cast<uint8_t>(GUIElementInfoboxAlert::PopupAlertType::HARDWARE_FAILURE),
                                    "FAULT DETECTED\nREMOVE PROBE\nFROM PATIENT", true);
}

void TimeDateChanged(time_t newtime)
{
    char timestring[32];
    strftime(timestring, sizeof(timestring), "%I:%M:%S\n%d %b, %Y", localtime(&newtime));

    screen_main_.SetStatusBox(timestring);
    screen_aux_.SetStatusBox(timestring);
}

void PopupClicked()
{
}

void PeakClicked()
{
}

void HotReleased(int temp)
{
    screen_aux_.SetHotTemperatureLimit(temp);
}

void ColdReleased(int temp)
{
    screen_aux_.SetColdTemperatureLimit(temp);
}

//----------------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
    // Create the display window
    static TCHAR szAppName[] = TEXT("GUI Mockup");
    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    double temperature = -20;

    for (int j = 0; j < 60; j++)
        for (int i = 0; i<128; i++)
        {
            testmap[j][i] = 37.0;
        }
    for (int j = 30; j < 40; j++)
        for (int i = 59; i<69; i++)
        {
            testmap[j][i] = 42.1;
        }
    for (int j = 31; j < 39; j++)
        for (int i = 60; i<68; i++)
        {
            testmap[j][i] = 42.2;
        }
    for (int j = 32; j < 38; j++)
        for (int i = 61; i<67; i++)
        {
            testmap[j][i] = 42.3;
        }
    for (int j = 33; j < 37; j++)
        for (int i = 62; i<66; i++)
        {
            testmap[j][i] = 42.4;
        }
    for (int j = 34; j < 36; j++)
        for (int i = 63; i<65; i++)
        {
            testmap[j][i] = 42.5;
        }

    // Initialize the Hot Springs gui context_lcd
    context_main_.Initialize();
    context_main_.Clear();
    context_aux_.Initialize();
    context_aux_.Clear();

    // Draw the screen
    screen_main_.Render();
    screen_main_.SetTempSliderCallbacks(HotReleased, ColdReleased);
    screen_main_.SetButtonClickedCallback(ButtonClicked);
    screen_main_.SetTimeDateChangedCallback(TimeDateChanged);
    screen_main_.SetPeakTempClickedCallback(PeakClicked);
    screen_main_.SetPopupClickedCallback(PopupClicked);
    screen_main_.SetDateBarClickedActive(true);

    screen_aux_.Render();
    // Register and display the window
    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, "Registering the class failled", "Error", MB_OK | MB_ICONERROR);
        exit(0);
    }

    hwnd_lcd = CreateWindow(szAppName, "gui-lcd-mockup",
        WS_OVERLAPPEDWINDOW,
        0,
        0,
        286,
        518,
        NULL,
        NULL,
        hInstance,
        NULL);
    if (!hwnd_lcd)
    {
        MessageBox(NULL, "LCD Window Creation Failed!", "Error", MB_OK);
        exit(0);
    }
    ShowWindow(hwnd_lcd, iCmdShow);
    UpdateWindow(hwnd_lcd);
    SetTimer(hwnd_lcd, 1, 1000, NULL);

    hwnd_dvi = CreateWindow(szAppName, "gui-lcd-mockup",
        WS_OVERLAPPEDWINDOW,
        286,
        0,
        1296,
        758,
        NULL,
        NULL,
        hInstance,
        NULL);
    if (!hwnd_dvi)
    {
        MessageBox(NULL, "DVI Window Creation Failed!", "Error", MB_OK);
        exit(0);
    }
    ShowWindow(hwnd_dvi, iCmdShow);
    UpdateWindow(hwnd_dvi);

    // Start the the Win32 message pump
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
