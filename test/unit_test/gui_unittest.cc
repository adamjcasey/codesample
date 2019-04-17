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

#include <fcntl.h>
#include <sys/mman.h>

#include "include/gui_context.h"
#include "include/gui_element.h"
#include "include/gui_element_infobox.h"
#include "include/gui_element_tempslider.h"
#include "include/gui_element_button.h"
#include "include/gui_element_timedatebar.h"
#include "include/gui_screen_main.h"
#include "include/gui_screen_aux.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

#include "test/include/mock_hardware.h"
#include "vendor/google/gmock/include/gmock/gmock.h"

#pragma GCC diagnostic pop

#include "vendor/google/gtest/include/gtest/gtest.h"
#include "vendor/linux/include/i2c-dev.h"

// Import of entire testing namespace useful in unit test files
using namespace ::testing;  // NOLINT(build/namespaces)

//-----------------------------------------------------------------------------
// Testing GUIContextTFT
//-----------------------------------------------------------------------------
class GUIContextTFTTest : public testing::Test
{
 protected:
        // Test objects
        GUIContextTFTTest() {}
        virtual void SetUp() {}

        MockSystem system_;
        MockSystemFile system_file_;
        static const char *device_path_;
        int oflag_ = O_RDWR;
        int mflag_ = PROT_READ | PROT_WRITE;
        int handle_fail_ = -1;
        int handle_succeed_ = 10;
        std::error_code success_ = std::error_code();
        std::error_code failure_ = std::error_code(121, std::system_category());;
        ISystemFile::OpenResult open_success_ = { .handle = handle_succeed_, .error = success_ };
        ISystemFile::OpenResult open_failure_ = { .handle = handle_fail_, .error = failure_ };
        uint8_t buffer_[1];
};

const char *GUIContextTFTTest::device_path_ = "/dev/fb0";

TEST_F(GUIContextTFTTest, Initialize_ErrorOpen)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_failure_));

    // Create test object
    GUIContextTFT context_(system_, system_file_);

    // Call method under test
    auto error = context_.Initialize();

    // Check assertions
    EXPECT_TRUE(error);
}

TEST_F(GUIContextTFTTest, Initialize_ErrorMmap)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_success_));

    EXPECT_CALL(system_, Mmap(nullptr, 272 * 480 * 4, mflag_, MAP_SHARED, handle_succeed_, 0))
        .WillOnce(Return(MAP_FAILED));

    // Create test object
    GUIContextTFT context_(system_, system_file_);

    // Call method under test
    auto error = context_.Initialize();

    // Check assertions
    EXPECT_TRUE(error);
}

TEST_F(GUIContextTFTTest, Initialize_Success)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_success_));

    EXPECT_CALL(system_, Mmap(nullptr, 272 * 480 * 4, mflag_, MAP_SHARED, handle_succeed_, 0))
        .WillOnce(Return(static_cast<void *>(buffer_)));

    // Create test object
    GUIContextTFT context_(system_, system_file_);

    // Call method under test
    auto error = context_.Initialize();

    // Check assertions
    EXPECT_FALSE(error) << error;
}

TEST_F(GUIContextTFTTest, Close_Success)
{
    // Setup expects
    InSequence s;

    // Initialize methods
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_success_));

    EXPECT_CALL(system_, Mmap(nullptr, 272 * 480 * 4, mflag_, MAP_SHARED, handle_succeed_, 0))
        .WillOnce(Return(static_cast<void *>(buffer_)));

    // Close methods
    EXPECT_CALL(system_, Munmap(_, 272 * 480 * 4))
        .WillOnce(Return(0));

    EXPECT_CALL(system_file_, Close(_))
        .WillOnce(Return(success_));

    // Create test object
    GUIContextTFT context_(system_, system_file_);

    auto error = context_.Initialize();
    EXPECT_FALSE(error) << error;

    // Call method under test
    error = context_.Close();

    // Check assertions
    EXPECT_FALSE(error) << error;
}

TEST_F(GUIContextTFTTest, Close_SkipInvalidMunmap)
{
    // Setup expects
    InSequence s;

    // Munmap should not be called if not initialized, as a segmentation
    // fault would occur if passed an invalid pointer.

    EXPECT_CALL(system_file_, Close(_))
        .WillOnce(Return(success_));

    // Create test object
    GUIContextTFT context_(system_, system_file_);

    // Call method under test
    auto error = context_.Close();

    // Check assertions
    EXPECT_FALSE(error) << error;
}

TEST_F(GUIContextTFTTest, Width_Confirm)
{
    // Setup expects
    // none

    // Create test object
    GUIContextTFT context_(system_, system_file_);

    // Call method under test
    uint16_t width = context_.Width();

    // Check assertions
    EXPECT_EQ(width, 272);
}

TEST_F(GUIContextTFTTest, Height_Confirm)
{
    // Setup expects
    // none

    // Create test object
    GUIContextTFT context_(system_, system_file_);

    // Call method under test
    uint16_t height = context_.Height();

    // Check assertions
    EXPECT_EQ(height, 480);
}

TEST_F(GUIContextTFTTest, Stride_Confirm)
{
    // Setup expects
    // none

    // Create test object
    GUIContextTFT context_(system_, system_file_);

    // Call method under test
    int stride = context_.Stride();

    // Check assertions
    EXPECT_EQ(stride, 272 * 3);
}

//-----------------------------------------------------------------------------
// Testing GUIContextTFT
//-----------------------------------------------------------------------------
class GUIContextDVITest : public testing::Test
{
 protected:
        // Test objects
        GUIContextDVITest() {}
        virtual void SetUp() {}

        MockSystem system_;
        MockSystemFile system_file_;
        static const char *device_path_;
        static const char *device_path_i2c_;
        int oflag_ = O_RDWR;
        int mflag_ = PROT_READ | PROT_WRITE;
        int handle_fail_ = -1;
        int handle_succeed_ = 10;
        int handle_succeed_i2c_ = 11;
        std::error_code error_badf_ = std::error_code(EBADF, std::system_category());
        std::error_code success_ = std::error_code();
        std::error_code failure_ = std::error_code(121, std::system_category());;
        ISystemFile::OpenResult open_success_ = { .handle = handle_succeed_, .error = success_ };
        ISystemFile::OpenResult open_success_i2c_ = { .handle = handle_succeed_i2c_, .error = success_ };
        ISystemFile::OpenResult open_failure_ = { .handle = handle_fail_, .error = failure_ };
        uint8_t buffer_[1];
};

const char *GUIContextDVITest::device_path_ = "/dev/fb1";
const char *GUIContextDVITest::device_path_i2c_ = "/dev/i2c-1";

TEST_F(GUIContextDVITest, Initialize_ErrorOpen)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_failure_));

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    // Call method under test
    auto error = context_.Initialize();

    // Check assertions
    EXPECT_TRUE(error);
}

TEST_F(GUIContextDVITest, Initialize_ErrorMmap)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_success_));

    EXPECT_CALL(system_, Mmap(nullptr, 1280 * 720 * 4, mflag_, MAP_SHARED, handle_succeed_, 0))
        .WillOnce(Return(MAP_FAILED));

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    // Call method under test
    auto error = context_.Initialize();

    // Check assertions
    EXPECT_TRUE(error);
}

TEST_F(GUIContextDVITest, Initialize_Error_I2COpen)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_success_));

    EXPECT_CALL(system_, Mmap(nullptr, 1280 * 720 * 4, mflag_, MAP_SHARED, handle_succeed_, 0))
        .WillOnce(Return(static_cast<void *>(buffer_)));

    EXPECT_CALL(system_file_, Open(StrEq(device_path_i2c_), oflag_, _))
        .WillOnce(Return(open_failure_));

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    // Call method under test
    auto error = context_.Initialize();

    // Check assertions
    EXPECT_TRUE(error);
}

TEST_F(GUIContextDVITest, Initialize_FailIoctl)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_success_));

    EXPECT_CALL(system_, Mmap(nullptr, 1280 * 720 * 4, mflag_, MAP_SHARED, handle_succeed_, 0))
        .WillOnce(Return(static_cast<void *>(buffer_)));

    EXPECT_CALL(system_file_, Open(StrEq(device_path_i2c_), oflag_, _))
        .WillOnce(Return(open_success_i2c_));

    EXPECT_CALL(system_, Ioctl(handle_succeed_i2c_, I2C_SLAVE, 0x39))
        .WillOnce(Return(handle_fail_));

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    // Call method under test
    auto error = context_.Initialize();

    // Check assertions
    EXPECT_TRUE(error);
}

TEST_F(GUIContextDVITest, Initialize_FailWriteI2CInit)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_success_));

    EXPECT_CALL(system_, Mmap(nullptr, 1280 * 720 * 4, mflag_, MAP_SHARED, handle_succeed_, 0))
        .WillOnce(Return(static_cast<void *>(buffer_)));

    EXPECT_CALL(system_file_, Open(StrEq(device_path_i2c_), oflag_, _))
        .WillOnce(Return(open_success_i2c_));

    EXPECT_CALL(system_, Ioctl(handle_succeed_i2c_, I2C_SLAVE, 0x39))
        .WillOnce(Return(handle_succeed_i2c_));

    EXPECT_CALL(system_, SMBusReadByteData(handle_succeed_i2c_, _))
        .WillOnce(Return(0));

    EXPECT_CALL(system_, SMBusWriteByteData(handle_succeed_i2c_, _, _))
        .WillOnce(Return(-1));

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    // Call method under test
    auto error = context_.Initialize();

    // Check assertions
    EXPECT_TRUE(error);
}

TEST_F(GUIContextDVITest, Initialize_Success)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_success_));

    EXPECT_CALL(system_, Mmap(nullptr, 1280 * 720 * 4, mflag_, MAP_SHARED, handle_succeed_, 0))
        .WillOnce(Return(static_cast<void *>(buffer_)));

    EXPECT_CALL(system_file_, Open(StrEq(device_path_i2c_), oflag_, _))
        .WillOnce(Return(open_success_i2c_));

    EXPECT_CALL(system_, Ioctl(handle_succeed_i2c_, I2C_SLAVE, 0x39))
        .WillOnce(Return(handle_succeed_i2c_));

    EXPECT_CALL(system_, SMBusReadByteData(handle_succeed_i2c_, _))
        .WillRepeatedly(Return(0));

    EXPECT_CALL(system_, SMBusWriteByteData(handle_succeed_i2c_, _, _))
        .WillRepeatedly(Return(0));

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    // Call method under test
    auto error = context_.Initialize();

    // Check assertions
    EXPECT_FALSE(error) << error;
}

TEST_F(GUIContextDVITest, Close_Success)
{
    // Setup expects
    EXPECT_CALL(system_file_, Open(StrEq(device_path_), oflag_, _))
        .WillOnce(Return(open_success_));

    EXPECT_CALL(system_, Mmap(nullptr, 1280 * 720 * 4, mflag_, MAP_SHARED, handle_succeed_, 0))
        .WillOnce(Return(static_cast<void *>(buffer_)));

    EXPECT_CALL(system_file_, Open(StrEq(device_path_i2c_), oflag_, _))
        .WillOnce(Return(open_success_i2c_));

    EXPECT_CALL(system_, Ioctl(handle_succeed_i2c_, I2C_SLAVE, 0x39))
        .WillOnce(Return(handle_succeed_i2c_));

    EXPECT_CALL(system_, SMBusReadByteData(handle_succeed_i2c_, _))
        .WillRepeatedly(Return(0));

    EXPECT_CALL(system_, SMBusWriteByteData(handle_succeed_i2c_, _, _))
        .WillRepeatedly(Return(0));

    EXPECT_CALL(system_, Munmap(_, 1280 * 720 * 4))
        .WillOnce(Return(0));

    EXPECT_CALL(system_file_, Close(_))
        .WillRepeatedly(Return(success_));

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    auto error = context_.Initialize();
    EXPECT_FALSE(error) << error;

    // Call method under test
    error = context_.Close();

    // Check assertions
    EXPECT_FALSE(error) << error;
}

TEST_F(GUIContextDVITest, Close_SkipInvalidMunmap)
{
    // Setup expects
    InSequence s;

    // Munmap should not be called if not initialized, as a segmentation
    // fault would occur if passed an invalid pointer.

    EXPECT_CALL(system_file_, Close(_))
        .WillRepeatedly(Return(success_));

    // Create test object
    GUIContextTFT context_(system_, system_file_);

    // Call method under test
    auto error = context_.Close();

    // Check assertions
    EXPECT_FALSE(error) << error;
}

TEST_F(GUIContextDVITest, Width_Confirm)
{
    // Setup expects
    // none

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    // Call method under test
    uint16_t width = context_.Width();

    // Check assertions
    EXPECT_EQ(width, 1280);
}

TEST_F(GUIContextDVITest, Height_Confirm)
{
    // Setup expects
    // none

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    // Call method under test
    uint16_t height = context_.Height();

    // Check assertions
    EXPECT_EQ(height, 720);
}

TEST_F(GUIContextDVITest, Stride_Confirm)
{
    // Setup expects
    // none

    // Create test object
    GUIContextDVI context_(system_, system_file_);

    // Call method under test
    int stride = context_.Stride();

    // Check assertions
    EXPECT_EQ(stride, 1280 * 3);
}

//-----------------------------------------------------------------------------
// Testing GUIElement
//-----------------------------------------------------------------------------
class GUIElementTest : public testing::Test
{
 protected:
        // Test objects
        GUIElementTest() {}
        virtual void SetUp() {}

        // There are virtual methods that we need to test, so we need a child
        // class to access
        class Child : public GUIElement
        {
         public:
                Child(const IGUIContext& context, double x, double y,
                            double width, double height) :
                        GUIElement(context, x, y, width, height, true) {}
                void Reset()
                {
                    draw_count_ = 0;
                    on_click_count_ = 0;
                    on_click_last_x_ = 0;
                    on_click_last_y_ = 0;
                    on_drag_count_ = 0;
                    on_drag_last_x_ = 0;
                    on_drag_last_y_ = 0;
                    on_release_count_ = 0;
                }
                void OnClick(uint16_t x, uint16_t y) const override
                {
                    on_click_count_++;
                    on_click_last_x_ = x;
                    on_click_last_y_ = y;
                }
                void OnDrag(uint16_t x, uint16_t y) const override
                {
                    on_drag_count_++;
                    on_drag_last_x_ = x;
                    on_drag_last_y_ = y;
                }
                void OnRelease() const override
                {
                    on_release_count_++;
                }
                void Draw() const override
                {
                    draw_count_++;
                }
                mutable uint16_t draw_count_;
                mutable uint16_t on_click_count_;
                mutable uint16_t on_click_last_x_;
                mutable uint16_t on_click_last_y_;
                mutable uint16_t on_drag_count_;
                mutable uint16_t on_drag_last_x_;
                mutable uint16_t on_drag_last_y_;
                mutable uint16_t on_release_count_;
        };

        MockGUIContext context_;
};

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, IsInRegion_BelowX)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Click(99, 125);

    // Check assertions
    EXPECT_EQ(testchild.on_click_count_, 0);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, IsInRegion_AboveX)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Click(150, 125);

    // Check assertions
    EXPECT_EQ(testchild.on_click_count_, 0);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, IsInRegion_BelowY)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Click(125, 99);

    // Check assertions
    EXPECT_EQ(testchild.on_click_count_, 0);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, IsInRegion_AboveY)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Click(125, 150);

    // Check assertions
    EXPECT_EQ(testchild.on_click_count_, 0);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, IsInRegion_InTopLeft)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Click(100, 100);

    // Check assertions
    EXPECT_EQ(testchild.on_click_count_, 1);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, IsInRegion_InBottomight)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Click(149, 149);

    // Check assertions
    EXPECT_EQ(testchild.on_click_count_, 1);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, OnClick_InOutIn)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Click(100, 100);
    testchild.Click(99, 100);
    testchild.Click(149, 125);

    // Check assertions
    EXPECT_EQ(testchild.on_click_count_, 2);
    EXPECT_EQ(testchild.on_click_last_x_, 149);
    EXPECT_EQ(testchild.on_click_last_y_, 125);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, OnDrag_InOutIn)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Click(100, 100);
    testchild.Drag(100, 100);
    testchild.Drag(99, 100);
    testchild.Drag(149, 125);

    // Check assertions
    EXPECT_EQ(testchild.on_drag_count_, 2);
    EXPECT_EQ(testchild.on_drag_last_x_, 149);
    EXPECT_EQ(testchild.on_drag_last_y_, 125);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, OnRelease_3Times)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Release();
    testchild.Release();
    testchild.Release();

    // Check assertions
    EXPECT_EQ(testchild.on_release_count_, 3);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, Draw_3Times)
{
    // Setup expects
    // none

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Draw();
    testchild.Draw();
    testchild.Draw();

    // Check assertions
    EXPECT_EQ(testchild.draw_count_, 3);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTest, Refresh_CorrectExpects)
{
    // Setup expects
    EXPECT_CALL(context_, ForceRedraw(100, 100, 150, 150));

    // Create test object
    Child testchild(context_, 100, 100, 50, 50);
    testchild.Reset();

    // Call method under test
    testchild.Refresh();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
// Testing GUIElementInfobox
//-----------------------------------------------------------------------------
class GUIElementInfoboxTest : public testing::Test
{
 protected:
        // Test objects
        GUIElementInfoboxTest() {}
        virtual void SetUp() {}
        void SetupExpectsForDrawing(int times)
        {
            EXPECT_CALL(context_, Buffer()).Times(times);
            EXPECT_CALL(context_, Width()).Times(times);
            EXPECT_CALL(context_, Height()).Times(times);
            EXPECT_CALL(context_, Stride()).Times(times);
        }

        MockGUIContext context_;
};

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxTest, Draw_CorrectExpects)
{
    // Setup expects
    // There are two calls to the context
    SetupExpectsForDrawing(2);

    // Create test object
    GUIElementInfobox infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test", true);

    // Call method under test
    infobox.Draw();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxTest, ClearInfoArea_CorrectExpects)
{
    // Setup expects
    SetupExpectsForDrawing(1);

    // Create test object
    GUIElementInfobox infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test", true);

    // Call method under test
    infobox.ClearInfoArea();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxTest, RefreshInfoArea_CorrectExpects)
{
    // Setup expects
    EXPECT_CALL(context_, ForceRedraw(10, 10+25, 110, 110));

    // Create test object
    GUIElementInfobox infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test", true);

    // Call method under test
    infobox.RefreshInfoArea();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
// Testing GUIElementInfoboxPeakTemperature
//-----------------------------------------------------------------------------
class GUIElementInfoboxPeakTemperatureTest : public testing::Test
{
 protected:
        // Test objects
        GUIElementInfoboxPeakTemperatureTest() {}
        virtual void SetUp() {}
        void SetupExpectsForDrawing(int times)
        {
            EXPECT_CALL(context_, Buffer()).Times(times);
            EXPECT_CALL(context_, Width()).Times(times);
            EXPECT_CALL(context_, Height()).Times(times);
            EXPECT_CALL(context_, Stride()).Times(times);
        }

        MockGUIContext context_;
};

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxPeakTemperatureTest, Draw_CorrectExpects)
{
    // Setup expects
    SetupExpectsForDrawing(5);

    // Create test object
    GUIElementInfoboxPeakTemperature infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              GUIColor(14, 33, 55),
                              GUIColor(14, 33, 55),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test", [](){});

    // Call method under test
    infobox.Draw();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxPeakTemperatureTest, SetCurrentTemp_CorrectExpects_AndReturn)
{
    // Setup expects
    SetupExpectsForDrawing(2);
    EXPECT_CALL(context_, ForceRedraw(10, 10+25, 110, 110));

    // Create test object
    GUIElementInfoboxPeakTemperature infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              GUIColor(14, 33, 55),
                              GUIColor(14, 33, 55),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test", [](){});

    // Call method under test
    infobox.SetCurrentTemp(45.6);

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxPeakTemperatureTest, Disable_CorrectExpects)
{
    // Setup expects
    SetupExpectsForDrawing(2);
    EXPECT_CALL(context_, ForceRedraw(10, 10+25, 110, 110));

    // Create test object
    GUIElementInfoboxPeakTemperature infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              GUIColor(14, 33, 55),
                              GUIColor(14, 33, 55),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test", [](){});

    // Call method under test
    infobox.Disable();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
// Testing GUIElementInfoboxCurrentTemp
//-----------------------------------------------------------------------------
class GUIElementInfoboxCurrentTempTest : public testing::Test
{
 protected:
        // Test objects
        GUIElementInfoboxCurrentTempTest() {}
        virtual void SetUp() {}
        void SetupExpectsForDrawing(int times)
        {
            EXPECT_CALL(context_, Buffer()).Times(times);
            EXPECT_CALL(context_, Width()).Times(times);
            EXPECT_CALL(context_, Height()).Times(times);
            EXPECT_CALL(context_, Stride()).Times(times);
        }

        MockGUIContext context_;
};

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxCurrentTempTest, Draw_CorrectExpects)
{
    // Setup expects
    SetupExpectsForDrawing(5);

    // Create test object
    GUIElementInfoboxGeneralTemperature infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test", true);

    // Call method under test
    infobox.Draw();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxCurrentTempTest, SetCurrentTemp_CorrectExpects_AndReturn)
{
    // Setup expects
    SetupExpectsForDrawing(2);
    EXPECT_CALL(context_, ForceRedraw(10, 10+25, 110, 110));

    // Create test object
    GUIElementInfoboxGeneralTemperature infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test", true);

    // Call method under test
    infobox.SetCurrentTemp(33.5);

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxCurrentTempTest, Disable_CorrectExpects)
{
    // Setup expects
    SetupExpectsForDrawing(2);
    EXPECT_CALL(context_, ForceRedraw(10, 10+25, 110, 110));

    // Create test object
    GUIElementInfoboxGeneralTemperature infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test", true);

    // Call method under test
    infobox.Disable();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
// Testing GUIElementInfoboxStatus
//-----------------------------------------------------------------------------
class GUIElementInfoboxStatusTest : public testing::Test
{
 protected:
        // Test objects
        GUIElementInfoboxStatusTest() {}
        virtual void SetUp() {}
        void SetupExpectsForDrawing(int times)
        {
            EXPECT_CALL(context_, Buffer()).Times(times);
            EXPECT_CALL(context_, Width()).Times(times);
            EXPECT_CALL(context_, Height()).Times(times);
            EXPECT_CALL(context_, Stride()).Times(times);
        }

        MockGUIContext context_;
};

//-----------------------------------------------------------------------------
TEST_F(GUIElementInfoboxStatusTest, UpdateText_CorrectExpects)
{
    // Setup expects
    SetupExpectsForDrawing(5);

    // Create test object
    GUIElementInfoboxStatus infobox(context_,
                              GUIColor(244, 245, 246),
                              GUIColor(208, 212, 216),
                              GUIColor(14, 33, 55),
                              10, 10, 100, 100, 25, "Test");

    // Call method under test
    infobox.UpdateText("Testing");

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
// Testing GUIElementTextButton
//-----------------------------------------------------------------------------
class GUIElementTextButtonTest : public testing::Test
{
 protected:
        // Test objects
        GUIElementTextButtonTest() {}
        virtual void SetUp()
        {
            text_button_clicked_ = 0;
        }
        void SetupExpectsForDrawing(int times)
        {
            EXPECT_CALL(context_, Buffer()).Times(times);
            EXPECT_CALL(context_, Width()).Times(times);
            EXPECT_CALL(context_, Height()).Times(times);
            EXPECT_CALL(context_, Stride()).Times(times);
        }

        MockGUIContext context_;
        int text_button_clicked_;

 public:
        void TextButtonClicked()
        {
            text_button_clicked_++;
        }
};

//-----------------------------------------------------------------------------
TEST_F(GUIElementTextButtonTest, Draw_CorrectExpects)
{
    // Setup expects
    // none

    // Create test object
    auto callback = std::bind(&GUIElementTextButtonTest::TextButtonClicked, this);

    GUIElementTextButton start_button(context_,
                            GUIColor(52, 162, 149),
                            GUIColor(10, 76, 112),
                            11, 395, 187, 73, 16,
                            callback);

    // Call method under test
    start_button.Draw();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTextButtonTest, Click_Outside)
{
    // Setup expects
    // none

    // Create test object
    auto callback = std::bind(&GUIElementTextButtonTest::TextButtonClicked, this);

    GUIElementTextButton start_button(context_,
                            GUIColor(52, 162, 149),
                            GUIColor(10, 76, 112),
                            10, 300, 100, 50, /* test coordinates */
                            16,
                            callback);

    // Call method under test
    start_button.Click(150, 400);

    // Check assertions
    ASSERT_EQ(text_button_clicked_, 0);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTextButtonTest, Click_Inside)
{
    // Setup expects
    // none

    // Create test object
    auto callback = std::bind(&GUIElementTextButtonTest::TextButtonClicked, this);

    GUIElementTextButton start_button(context_,
                            GUIColor(52, 162, 149),
                            GUIColor(10, 76, 112),
                            10, 300, 100, 50, /* test coordinates */
                            16,
                            callback);

    // Call method under test
    start_button.Click(10, 300);

    // Check assertions
    ASSERT_EQ(text_button_clicked_, 1);
}

//-----------------------------------------------------------------------------
// Testing GUIElementTimeDateBar
//-----------------------------------------------------------------------------
class GUIElementTimeDateBarTest : public testing::Test
{
 protected:
        // Test objects
        GUIElementTimeDateBarTest() {}
        virtual void SetUp()
        {
            button_clicked_ = 0;
        }
        void SetupExpectsForDrawing(int times)
        {
            EXPECT_CALL(context_, Buffer()).Times(times);
            EXPECT_CALL(context_, Width()).Times(times);
            EXPECT_CALL(context_, Height()).Times(times);
            EXPECT_CALL(context_, Stride()).Times(times);
        }

        MockGUIContext context_;
        int button_clicked_;

 public:
        void ButtonClicked()
        {
            button_clicked_++;
        }
};

//-----------------------------------------------------------------------------
TEST_F(GUIElementTimeDateBarTest, Draw_CorrectExpects)
{
    // Setup expects
    SetupExpectsForDrawing(3);

    // Create test object
    auto callback = std::bind(&GUIElementTimeDateBarTest::ButtonClicked, this);

    GUIElementTimeDateBar timedate(context_,
        GUIColor(67, 81, 98),
        GUIColor(255, 255, 255),
        0, 0, 272, 30, 10, callback);

    // Call method under test
    timedate.Draw();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTimeDateBarTest, Click_Outside)
{
    // Setup expects
    // none

    // Create test object
    auto callback = std::bind(&GUIElementTimeDateBarTest::ButtonClicked, this);

    GUIElementTimeDateBar timedate(context_,
        GUIColor(67, 81, 98),
        GUIColor(255, 255, 255),
        0, 0, 272, 30, 10, callback);

    // Call method under test
    timedate.Click(150, 400);

    // Check assertions
    ASSERT_EQ(button_clicked_, 0);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTimeDateBarTest, Click_Inside)
{
    // Setup expects
    // none

    // Create test object
    auto callback = std::bind(&GUIElementTimeDateBarTest::ButtonClicked, this);

    GUIElementTimeDateBar timedate(context_,
        GUIColor(67, 81, 98),
        GUIColor(255, 255, 255),
        0, 0, 272, 30, 10, callback);

    // Call method under test
    timedate.Click(10, 10);

    // Check assertions
    ASSERT_EQ(button_clicked_, 1);
}

//-----------------------------------------------------------------------------
// Testing GUIElementTempSlider
//-----------------------------------------------------------------------------
class GUIElementTempSliderTest : public testing::Test
{
 protected:
        // Test objects
        GUIElementTempSliderTest() :
            hot_changed_callback_(std::bind(&GUIElementTempSliderTest::HotChanged, this, std::placeholders::_1)),
            cold_changed_callback_(std::bind(&GUIElementTempSliderTest::ColdChanged, this, std::placeholders::_1)),
            hot_touched_callback_(std::bind(&GUIElementTempSliderTest::HotTouched, this)),
            cold_touched_callback_(std::bind(&GUIElementTempSliderTest::ColdTouched, this)),
            hot_released_callback_(std::bind(&GUIElementTempSliderTest::HotReleased, this, std::placeholders::_1)),
            cold_released_callback_(std::bind(&GUIElementTempSliderTest::ColdReleased, this, std::placeholders::_1)),
            hot_pointer_dirty_rectangle_callback_(std::bind(&GUIElementTempSliderTest::OnHotPointerDirtyRectangle, this)),
            cold_pointer_dirty_rectangle_callback_(std::bind(&GUIElementTempSliderTest::OnColdPointerDirtyRectangle, this)) {}

        virtual void SetUp()
        {
            setpoint_cold_ = 0;
            setpoint_hot_ = 0;

            num_times_cold_changed_ = 0;
            num_times_hot_changed_ = 0;
            num_times_cold_touched_ = 0;
            num_times_hot_touched_ = 0;
            num_times_cold_released_ = 0;
            num_times_hot_released_ = 0;
            num_times_hot_pointer_dirty_rectangle_ = 0;
            num_times_cold_pointer_dirty_rectangle_ = 0;
        }
        void SetupExpectsForDrawing(int times)
        {
            EXPECT_CALL(context_, Buffer()).Times(times);
            EXPECT_CALL(context_, Width()).Times(times);
            EXPECT_CALL(context_, Height()).Times(times);
            EXPECT_CALL(context_, Stride()).Times(times);
        }

        MockGUIContext context_;
        int setpoint_cold_;
        int setpoint_hot_;
        int num_times_cold_changed_;
        int num_times_hot_changed_;
        int num_times_cold_touched_;
        int num_times_hot_touched_;
        int num_times_cold_released_;
        int num_times_hot_released_;
        int num_times_hot_pointer_dirty_rectangle_;
        int num_times_cold_pointer_dirty_rectangle_;

        GUIElementTempSlider::TempSetpointChangeCallback hot_changed_callback_;
        GUIElementTempSlider::TempSetpointChangeCallback cold_changed_callback_;
        GUIElementTempSlider::TempSetpointTouchCallback hot_touched_callback_;
        GUIElementTempSlider::TempSetpointTouchCallback cold_touched_callback_;
        GUIElementTempSlider::TempSetpointReleaseCallback hot_released_callback_;
        GUIElementTempSlider::TempSetpointReleaseCallback cold_released_callback_;
        GUIElementTempSlider::TempPointerDirtyRectangleCallback hot_pointer_dirty_rectangle_callback_;
        GUIElementTempSlider::TempPointerDirtyRectangleCallback cold_pointer_dirty_rectangle_callback_;

 public:
        void ColdChanged(int temp)
        {
            setpoint_cold_ = temp;
            num_times_cold_changed_++;
        }
        void HotChanged(int temp)
        {
            setpoint_hot_ = temp;
            num_times_hot_changed_++;
        }
        void ColdReleased(int temp)
        {
            setpoint_cold_ = temp;
            num_times_cold_released_++;
        }
        void HotReleased(int temp)
        {
            setpoint_hot_ = temp;
            num_times_hot_released_++;
        }
        void ColdTouched()
        {
            num_times_cold_touched_++;
        }
        void HotTouched()
        {
            num_times_hot_touched_++;
        }
        void OnHotPointerDirtyRectangle()
        {
            num_times_hot_pointer_dirty_rectangle_++;
        }
        void OnColdPointerDirtyRectangle()
        {
            num_times_cold_pointer_dirty_rectangle_++;
        }
};

//-----------------------------------------------------------------------------
TEST_F(GUIElementTempSliderTest, TemperatureFromYPosition_50px)
{
    // Setup expects
    // none

    // Create test object
    GUIElementTempSlider tempslider(context_,
                                    208, 42, 59, 426,
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246),
                                    hot_changed_callback_,
                                    cold_changed_callback_,
                                    hot_touched_callback_,
                                    cold_touched_callback_,
                                    hot_released_callback_,
                                    cold_released_callback_,
                                    hot_pointer_dirty_rectangle_callback_,
                                    cold_pointer_dirty_rectangle_callback_,
                                    50, 20);

    // Call method under test
    double temp = tempslider.TemperatureFromYPosition(59);

    // Check assertions
    EXPECT_EQ(temp, 68);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTempSliderTest, TemperatureFromYPosition_300px)
{
    // Setup expects
    // none

    // Create test object
    GUIElementTempSlider tempslider(context_,
                                    208, 42, 59, 426,
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246),
                                    hot_changed_callback_,
                                    cold_changed_callback_,
                                    hot_touched_callback_,
                                    cold_touched_callback_,
                                    hot_released_callback_,
                                    cold_released_callback_,
                                    hot_pointer_dirty_rectangle_callback_,
                                    cold_pointer_dirty_rectangle_callback_,
                                    50, 20);

    // Call method under test
    double temp = tempslider.TemperatureFromYPosition(300);

    // Check assertions
    EXPECT_EQ(temp, 15);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTempSliderTest, YPositionFromTemperature_50degrees)
{
    // Setup expects
    // none

    // Create test object
    GUIElementTempSlider tempslider(context_,
                                    208, 42, 59, 426,
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246),
                                    hot_changed_callback_,
                                    cold_changed_callback_,
                                    hot_touched_callback_,
                                    cold_touched_callback_,
                                    hot_released_callback_,
                                    cold_released_callback_,
                                    hot_pointer_dirty_rectangle_callback_,
                                    cold_pointer_dirty_rectangle_callback_,
                                    50, 20);

    // Call method under test
    uint16_t y = tempslider.YPositionFromTemperature(50.0);

    // Check assertions
    EXPECT_EQ(y, 141);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTempSliderTest, YPositionFromTemperature_20degrees)
{
    // Setup expects
    // none

    // Create test object
    GUIElementTempSlider tempslider(context_,
                                    208, 42, 59, 426,
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246),
                                    hot_changed_callback_,
                                    cold_changed_callback_,
                                    hot_touched_callback_,
                                    cold_touched_callback_,
                                    hot_released_callback_,
                                    cold_released_callback_,
                                    hot_pointer_dirty_rectangle_callback_,
                                    cold_pointer_dirty_rectangle_callback_,
                                    50, 20);

    // Call method under test
    uint16_t y = tempslider.YPositionFromTemperature(20.0);

    // Check assertions
    EXPECT_EQ(y, 277);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTempSliderTest, Draw_CorrectExpects)
{
    // Setup expects
    SetupExpectsForDrawing(8);

    // Create test object
    GUIElementTempSlider tempslider(context_,
                                    208, 42, 59, 426,
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246),
                                    hot_changed_callback_,
                                    cold_changed_callback_,
                                    hot_touched_callback_,
                                    cold_touched_callback_,
                                    hot_released_callback_,
                                    cold_released_callback_,
                                    hot_pointer_dirty_rectangle_callback_,
                                    cold_pointer_dirty_rectangle_callback_,
                                    50, 20);

    // Call method under test
    tempslider.Draw();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTempSliderTest, Drag_HotSlider)
{
    // Setup expects
    SetupExpectsForDrawing(8);
    EXPECT_CALL(context_, ForceRedraw(219, 39, 267, 161));

    // Create test object
    GUIElementTempSlider tempslider(context_,
                                    208, 42, 59, 426,
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246),
                                    hot_changed_callback_,
                                    cold_changed_callback_,
                                    hot_touched_callback_,
                                    cold_touched_callback_,
                                    hot_released_callback_,
                                    cold_released_callback_,
                                    hot_pointer_dirty_rectangle_callback_,
                                    cold_pointer_dirty_rectangle_callback_,
                                    50, 20);

    // Call method under test
    tempslider.Click(220, 141);
    tempslider.Drag(220, 59);
    tempslider.Release();

    // Check assertions
    EXPECT_EQ(setpoint_hot_, 68);
    EXPECT_EQ(setpoint_cold_, 0);
    EXPECT_EQ(0, num_times_cold_changed_);
    EXPECT_EQ(1, num_times_hot_changed_);
    EXPECT_EQ(0, num_times_cold_touched_);
    EXPECT_EQ(1, num_times_hot_touched_);
    EXPECT_EQ(0, num_times_cold_released_);
    EXPECT_EQ(1, num_times_hot_released_);
    EXPECT_EQ(1, num_times_hot_pointer_dirty_rectangle_);
}

//-----------------------------------------------------------------------------
TEST_F(GUIElementTempSliderTest, Drag_ColdSlider)
{
    // Setup expects
    SetupExpectsForDrawing(8);
    EXPECT_CALL(context_, ForceRedraw(219, 257, 267, 320));

    // Create test object
    GUIElementTempSlider tempslider(context_,
                                    208, 42, 59, 426,
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246), GUIColor(244, 245, 246),
                                    GUIColor(244, 245, 246),
                                    hot_changed_callback_,
                                    cold_changed_callback_,
                                    hot_touched_callback_,
                                    cold_touched_callback_,
                                    hot_released_callback_,
                                    cold_released_callback_,
                                    hot_pointer_dirty_rectangle_callback_,
                                    cold_pointer_dirty_rectangle_callback_,
                                    50, 20);

    // Call method under test
    tempslider.Click(220, 277);
    tempslider.Drag(220, 300);
    tempslider.Release();

    // Check assertions
    EXPECT_EQ(setpoint_hot_, 0);
    EXPECT_EQ(setpoint_cold_, 15);
    EXPECT_EQ(1, num_times_cold_changed_);
    EXPECT_EQ(0, num_times_hot_changed_);
    EXPECT_EQ(1, num_times_cold_touched_);
    EXPECT_EQ(0, num_times_hot_touched_);
    EXPECT_EQ(1, num_times_cold_released_);
    EXPECT_EQ(0, num_times_hot_released_);
}

//-----------------------------------------------------------------------------
// Testing GUIScreenMain
//-----------------------------------------------------------------------------
class GUIScreenMainTest : public testing::Test
{
 protected:
        // Test objects
        GUIScreenMainTest() {}
        virtual void SetUp()
        {
        }
        void SetupExpectsForDrawing(int times)
        {
            EXPECT_CALL(context_, Buffer()).Times(AtLeast(times));
            EXPECT_CALL(context_, Width()).Times(AtLeast(times));
            EXPECT_CALL(context_, Height()).Times(AtLeast(times));
            EXPECT_CALL(context_, Stride()).Times(AtLeast(times));
        }

        MockGUIContext context_;
};

//-----------------------------------------------------------------------------
TEST_F(GUIScreenMainTest, Render)
{
    // Setup expects
    EXPECT_CALL(context_, Clear());
    SetupExpectsForDrawing(6);
    EXPECT_CALL(context_, ForceRedraw());

    GUIScreenMain screen(context_);

    // Call method under test
    screen.Render();

    // Check assertions
    // none
}

//-----------------------------------------------------------------------------
// Testing GUIScreenAux
//-----------------------------------------------------------------------------
class GUIScreenAuxTest : public testing::Test
{
 protected:
        // Test objects
        GUIScreenAuxTest() {}
        virtual void SetUp()
        {
        }
        void SetupExpectsForDrawing(int times)
        {
            EXPECT_CALL(context_, Buffer()).Times(AtLeast(times));
            EXPECT_CALL(context_, Width()).Times(AtLeast(times));
            EXPECT_CALL(context_, Height()).Times(AtLeast(times));
            EXPECT_CALL(context_, Stride()).Times(AtLeast(times));
        }

        MockGUIContext context_;
};

//-----------------------------------------------------------------------------
TEST_F(GUIScreenAuxTest, Render)
{
    // Setup expects
    EXPECT_CALL(context_, Clear());
    SetupExpectsForDrawing(6);
    EXPECT_CALL(context_, ForceRedraw());

    GUIScreenAux screen(context_);

    // Call method under test
    screen.Render();

    // Check assertions
    // none
}
