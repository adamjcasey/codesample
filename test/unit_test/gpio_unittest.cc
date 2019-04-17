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

Created by Adam Casey 2016
------------------------------------------------------------------------------*/

#include <fcntl.h>
#include <string.h>

#include "include/gpio.h"
#include "include/watchdog_contributing_threads.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

#include "test/include/mock_hardware.h"
#include "vendor/google/gmock/include/gmock/gmock.h"

#pragma GCC diagnostic pop

#include "vendor/google/gtest/include/gtest/gtest.h"

// Import of entire testing namespace useful in unit test files
using namespace ::testing;  // NOLINT(build/namespaces)

//-----------------------------------------------------------------------------
// BaseGpioTest test class using Mocks
//-----------------------------------------------------------------------------
class BaseGpioTest : public testing::Test
{
 protected:
        // Test objects
        BaseGpioTest() {}
        virtual void SetUp() {}

        void SetupInitializeSuccessExpectCalls()
        {
            EXPECT_CALL(system_file_, Open(StrEq(pin_export_path_), O_WRONLY | O_SYNC, _))
                .WillOnce(Return(GenerateOpenSuccessResult(handle_export_)));

            EXPECT_CALL(system_file_, Write(handle_export_, StrEq(valid_pin_string_), valid_pin_string_size_))
                .WillOnce(Return(std::error_code()));

            EXPECT_CALL(system_file_, Close(handle_export_))
            .WillOnce(Return(std::error_code()));

            EXPECT_CALL(system_file_, Open(StrEq(valid_pin_value_path_), O_RDWR, _))
                .WillOnce(Return(GenerateOpenSuccessResult(handle_value_)));
        }

        void SetupCloseExpectCalls()
        {
            EXPECT_CALL(system_file_, Close(handle_value_))
                .WillOnce(Return(std::error_code()));

            EXPECT_CALL(system_file_, Open(StrEq(pin_unexport_path_), O_WRONLY | O_SYNC, _))
                .WillOnce(Return(GenerateOpenSuccessResult(handle_unexport_)));

            EXPECT_CALL(system_file_, Write(handle_unexport_, StrEq(valid_pin_string_), valid_pin_string_size_))
                .WillOnce(Return(std::error_code()));

            EXPECT_CALL(system_file_, Close(handle_unexport_))
                .WillOnce(Return(std::error_code()));
        }

        void SetupIOPinQueryExpectCalls(bool expected_state)
        {
            EXPECT_CALL(system_file_, Lseek(handle_value_, 0, SEEK_SET))
                .WillOnce(Return(std::error_code()));

            // Copy the expected file contents to arg1 (buffer pointer)
            EXPECT_CALL(system_file_, Read(handle_value_, _, 2))
                .WillOnce(DoAll(
                    WithArgs<1, 2>(
                        Copy((expected_state) ? valid_pin_value_true_ : valid_pin_value_false_)),
                    Return(std::error_code())));
        }

        ISystemFile::OpenResult GenerateOpenSuccessResult(int fd)
        {
            return {.handle = fd, .error = std::error_code()};
        }

        MockSystem system_;
        MockSystemFile system_file_;

        std::error_code error_badf_ = std::error_code(EBADF, std::system_category());
        ISystemFile::OpenResult open_failure_ = { .handle = -1, .error = error_badf_ };

        int handle_value_ = 10;
        int handle_direction_ = 11;
        int handle_edge_ = 12;
        int handle_export_ = 13;
        int handle_unexport_ = 14;

        static const char *pin_export_path_;
        static const char *pin_unexport_path_;
        static const uint16_t valid_pin_number_;
        static const char *valid_pin_string_;
        static const size_t valid_pin_string_size_;
        static const char *valid_pin_value_path_;
        static const char *valid_pin_value_true_;
        static const size_t valid_pin_value_true_size_;
        static const char *valid_pin_value_false_;
        static const size_t valid_pin_value_false_size_;
        static const char *valid_pin_direction_path_;
        static const char *valid_pin_direction_out_;
        static const size_t valid_pin_direction_out_size_;
        static const char *valid_pin_direction_in_;
        static const size_t valid_pin_direction_in_size_;
        static const char *valid_pin_edge_path_;
        static const char *valid_pin_edge_rising_;
        static const size_t valid_pin_edge_rising_size_;
};

const char *BaseGpioTest::pin_export_path_ = "/sys/class/gpio/export";
const char *BaseGpioTest::pin_unexport_path_ = "/sys/class/gpio/unexport";
const uint16_t BaseGpioTest::valid_pin_number_ = 400;
const char *BaseGpioTest::valid_pin_string_ = "400";
const size_t BaseGpioTest::valid_pin_string_size_ = 3;
const char *BaseGpioTest::valid_pin_value_path_ = "/sys/class/gpio/gpio400/value";
const char *BaseGpioTest::valid_pin_value_true_ = "1";
const size_t BaseGpioTest::valid_pin_value_true_size_ = 1;
const char *BaseGpioTest::valid_pin_value_false_ = "0";
const size_t BaseGpioTest::valid_pin_value_false_size_ = 1;
const char *BaseGpioTest::valid_pin_direction_path_ = "/sys/class/gpio/gpio400/direction";
const char *BaseGpioTest::valid_pin_direction_out_ = "out";
const size_t BaseGpioTest::valid_pin_direction_out_size_ = 3;
const char *BaseGpioTest::valid_pin_direction_in_ = "in";
const size_t BaseGpioTest::valid_pin_direction_in_size_ = 2;
const char *BaseGpioTest::valid_pin_edge_path_ = "/sys/class/gpio/gpio400/edge";
const char *BaseGpioTest::valid_pin_edge_rising_ = "rising";
const size_t BaseGpioTest::valid_pin_edge_rising_size_ = 6;

//-----------------------------------------------------------------------------
// Base GPIO
//-----------------------------------------------------------------------------

TEST_F(BaseGpioTest, Initialize_Fail_Pin_Number_Low)
{
    BaseGPIOPin base_pin(system_, system_file_, BaseGPIOPin::GPIO_NUMBER_MIN - 1);

    auto error = base_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Initialize_Fail_Pin_Number_High)
{
    BaseGPIOPin base_pin(system_, system_file_, BaseGPIOPin::GPIO_NUMBER_MAX + 1);

    auto error = base_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Initialize_Fail_Export_Open)
{
    EXPECT_CALL(system_file_, Open(StrEq(pin_export_path_), O_WRONLY | O_SYNC, _))
        .WillOnce(Return(open_failure_));

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Initialize_Fail_Export_Write)
{
    EXPECT_CALL(system_file_, Open(StrEq(pin_export_path_), O_WRONLY | O_SYNC, _))
        .WillOnce(Return(GenerateOpenSuccessResult(handle_export_)));

    EXPECT_CALL(system_file_, Write(handle_export_, StrEq(valid_pin_string_), valid_pin_string_size_))
        .WillOnce(Return(error_badf_));

    EXPECT_CALL(system_file_, Close(handle_export_))
    .WillOnce(Return(std::error_code()));

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Initialize_Fail_Export_Close)
{
    EXPECT_CALL(system_file_, Open(StrEq(pin_export_path_), O_WRONLY | O_SYNC, _))
        .WillOnce(Return(GenerateOpenSuccessResult(handle_export_)));

    EXPECT_CALL(system_file_, Write(handle_export_, StrEq(valid_pin_string_), valid_pin_string_size_))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(system_file_, Close(handle_export_))
    .WillOnce(Return(error_badf_));

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Initialize_Fail_Value_Open)
{
    {
        InSequence s;

        EXPECT_CALL(system_file_, Open(StrEq(pin_export_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_export_)));

        EXPECT_CALL(system_file_, Write(handle_export_, StrEq(valid_pin_string_), valid_pin_string_size_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Close(handle_export_))
        .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_value_path_), O_RDWR, _))
            .WillOnce(Return(open_failure_));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Initialize_Succeed)
{
    {
        InSequence s;

        SetupInitializeSuccessExpectCalls();
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;
}

TEST_F(BaseGpioTest, Initialize_Multiple)
{
    {
        InSequence s;

        // Should not call again on second initialization
        SetupInitializeSuccessExpectCalls();
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;
}

TEST_F(BaseGpioTest, Close_Without_Initialize)
{
    {
        InSequence s;

        EXPECT_CALL(system_file_, Close(_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Open(StrEq(pin_unexport_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_unexport_)));

        EXPECT_CALL(system_file_, Write(handle_unexport_, StrEq(valid_pin_string_), valid_pin_string_size_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Close(handle_unexport_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    // Expect all close calls to occur, even though never initialized.
    // Some calls would return errors, but want to verify all calls made.
    // Should still return an error because never initialized.
    auto error = base_pin.Close();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Close_Fail_Value_Close)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Close(handle_value_))
            .WillOnce(Return(error_badf_));

        EXPECT_CALL(system_file_, Open(StrEq(pin_unexport_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_unexport_)));

        EXPECT_CALL(system_file_, Write(handle_unexport_, StrEq(valid_pin_string_), valid_pin_string_size_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Close(handle_unexport_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.Close();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Close_Fail_Unexport_Open)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Close(handle_value_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Open(StrEq(pin_unexport_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(open_failure_));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.Close();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Close_Fail_Unexport_Write)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Close(handle_value_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Open(StrEq(pin_unexport_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_unexport_)));

        EXPECT_CALL(system_file_, Write(handle_unexport_, StrEq(valid_pin_string_), valid_pin_string_size_))
            .WillOnce(Return(error_badf_));

        EXPECT_CALL(system_file_, Close(handle_unexport_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.Close();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Close_Fail_Unexport_Close)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Close(handle_value_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Open(StrEq(pin_unexport_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_unexport_)));

        EXPECT_CALL(system_file_, Write(handle_unexport_, StrEq(valid_pin_string_), valid_pin_string_size_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Close(handle_unexport_))
            .WillOnce(Return(error_badf_));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.Close();
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Close_Success)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();
        SetupCloseExpectCalls();
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.Close();
    EXPECT_FALSE(error) << error;
}

TEST_F(BaseGpioTest, Set_Direction_Fail_Uninitialized)
{
    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.SetDirection(IGPIO::Direction::IN);
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Set_Direction_Fail_Open)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_direction_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(open_failure_));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.SetDirection(IGPIO::Direction::IN);
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Set_Direction_Fail_Write)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_direction_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_direction_)));

        EXPECT_CALL(system_file_, Write(handle_direction_, StrEq(valid_pin_direction_in_), valid_pin_direction_in_size_))
            .WillOnce(Return(error_badf_));

        EXPECT_CALL(system_file_, Close(handle_direction_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.SetDirection(IGPIO::Direction::IN);
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Set_Direction_Fail_Write_Close)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_direction_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_direction_)));

        EXPECT_CALL(system_file_, Write(handle_direction_, StrEq(valid_pin_direction_in_), valid_pin_direction_in_size_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Close(handle_direction_))
            .WillOnce(Return(error_badf_));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.SetDirection(IGPIO::Direction::IN);
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Set_Direction_In_Success)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_direction_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_direction_)));

        EXPECT_CALL(system_file_, Write(handle_direction_, StrEq(valid_pin_direction_in_), valid_pin_direction_in_size_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Close(handle_direction_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.SetDirection(IGPIO::Direction::IN);
    EXPECT_FALSE(error) << error;
}

TEST_F(BaseGpioTest, Set_Direction_Out_Success)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_direction_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_direction_)));

        EXPECT_CALL(system_file_, Write(handle_direction_, StrEq(valid_pin_direction_out_), valid_pin_direction_out_size_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Close(handle_direction_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.SetDirection(IGPIO::Direction::OUT);
    EXPECT_FALSE(error) << error;
}

TEST_F(BaseGpioTest, Set_Edge_Fail_Uninitialized)
{
    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.SetEdge(IGPIO::Edge::RISING);
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Set_Edge_Fail_Open)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_edge_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(open_failure_));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.SetEdge(IGPIO::Edge::RISING);
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Set_Edge_Fail_Write)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_edge_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_edge_)));

        EXPECT_CALL(system_file_, Write(handle_edge_, StrEq(valid_pin_edge_rising_), valid_pin_edge_rising_size_))
            .WillOnce(Return(error_badf_));

        EXPECT_CALL(system_file_, Close(handle_edge_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.SetEdge(IGPIO::Edge::RISING);
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Set_Edge_Fail_Write_Close)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_edge_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_edge_)));

        EXPECT_CALL(system_file_, Write(handle_edge_, StrEq(valid_pin_edge_rising_), valid_pin_edge_rising_size_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Close(handle_edge_))
            .WillOnce(Return(error_badf_));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.SetEdge(IGPIO::Edge::RISING);
    EXPECT_TRUE(error);
}

TEST_F(BaseGpioTest, Set_Edge_Success)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Open(StrEq(valid_pin_edge_path_), O_WRONLY | O_SYNC, _))
            .WillOnce(Return(GenerateOpenSuccessResult(handle_edge_)));

        EXPECT_CALL(system_file_, Write(handle_edge_, StrEq(valid_pin_edge_rising_), valid_pin_edge_rising_size_))
            .WillOnce(Return(std::error_code()));

        EXPECT_CALL(system_file_, Close(handle_edge_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    error = base_pin.SetEdge(IGPIO::Edge::RISING);
    EXPECT_FALSE(error) << error;
}

TEST_F(BaseGpioTest, Set_Value_Uninitialized)
{
    EXPECT_CALL(system_file_, Write(_, _, _)).Times(0);

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    base_pin.SetValue(true);
}

TEST_F(BaseGpioTest, Set_Value_True)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Write(handle_value_, StrEq(valid_pin_value_true_), valid_pin_value_true_size_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    base_pin.SetValue(true);
}

TEST_F(BaseGpioTest, Set_Value_False)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Write(handle_value_, StrEq(valid_pin_value_false_), valid_pin_value_false_size_))
            .WillOnce(Return(std::error_code()));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    base_pin.SetValue(false);
}

TEST_F(BaseGpioTest, Get_Value_Fail_Uninitialized)
{
    EXPECT_CALL(system_file_, Read(_, _, _)).Times(0);

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    base_pin.GetValue();
}

// Note: tests for ISystemFile Lseek and Read failures do not necessarily detect failure,
// as there is no expectation on the output of the GetValue call. There's no returned
// std::error_code, and the returned Boolean cannot be used to indicate whether or not
// a failure occured. If this is problematic, it is recommended to modify
// BaseGPIOPin::GetValue to return std::error_codes in addition to the Boolean result.

TEST_F(BaseGpioTest, Get_Value_Fail_Lseek)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_file_, Lseek(handle_value_, 0, SEEK_SET))
            .WillOnce(Return(error_badf_));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    base_pin.GetValue();
}

TEST_F(BaseGpioTest, Get_Value_Fail_Read)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

            EXPECT_CALL(system_file_, Lseek(handle_value_, 0, SEEK_SET))
                .WillOnce(Return(std::error_code()));

            // Copy the expected file contents to arg1 (buffer pointer)
            EXPECT_CALL(system_file_, Read(handle_value_, _, 2))
                .WillOnce(DoAll(
                    WithArgs<1, 2>(
                        Copy(valid_pin_value_true_)),
                    Return(error_badf_)));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    base_pin.GetValue();
}

TEST_F(BaseGpioTest, Get_Value_True)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();
        SetupIOPinQueryExpectCalls(true);
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    bool value = base_pin.GetValue();
    EXPECT_TRUE(value);
}

TEST_F(BaseGpioTest, Get_Value_False)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();
        SetupIOPinQueryExpectCalls(false);
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    bool value = base_pin.GetValue();
    EXPECT_FALSE(value);
}

TEST_F(BaseGpioTest, PollValue_Fail_Poll_Error)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_, Poll(_, 1, 999))
            .WillOnce(DoAll(SetErrno(EBADF), Return(-1)));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    auto error_with_irq = base_pin.PollValue(999);
    EXPECT_TRUE(error_with_irq.first);
    EXPECT_FALSE(error_with_irq.second);
}

TEST_F(BaseGpioTest, PollValue_Poll_Success)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_, Poll(_, 1, 999))
            .WillOnce(DoAll(WithArg<0>(SetPollFdRevents(POLLPRI | POLLERR)), Return(1)));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    auto error_with_irq = base_pin.PollValue(999);
    EXPECT_FALSE(error_with_irq.first) << error_with_irq.first;
    EXPECT_TRUE(error_with_irq.second);
}

TEST_F(BaseGpioTest, PollValue_Poll_Timeout)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        EXPECT_CALL(system_, Poll(_, 1, 999))
            .WillOnce(Return(0));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    auto error_with_irq = base_pin.PollValue(999);
    EXPECT_FALSE(error_with_irq.first) << error_with_irq.first;
    EXPECT_FALSE(error_with_irq.second);
}

TEST_F(BaseGpioTest, PollValue_Poll_Invalid_Revents)
{
    {
        InSequence s;
        SetupInitializeSuccessExpectCalls();

        // Note: valid revents value is (POLLPRI | POLLERR)
        EXPECT_CALL(system_, Poll(_, 1, 999))
            .WillOnce(DoAll(WithArg<0>(SetPollFdRevents(POLLPRI)), Return(1)));
    }

    BaseGPIOPin base_pin(system_, system_file_, valid_pin_number_);

    auto error = base_pin.Initialize();
    EXPECT_FALSE(error) << error;

    auto error_with_irq = base_pin.PollValue(999);
    EXPECT_TRUE(error_with_irq.first);
    EXPECT_FALSE(error_with_irq.second);
}

//-----------------------------------------------------------------------------
// GpioTest test class using Mocks
//-----------------------------------------------------------------------------
class GpioTest : public testing::Test
{
 protected:
        // Test objects
        GpioTest() {}
        virtual void SetUp() {}

        MockGPIO gpio_pin_;
        MockWatchdog watchdog_;

        std::error_code gpio_failure_ = GPIOError::GPIO_UNINITIALIZED;
        std::error_code error_badf_ = std::error_code(EBADF, std::system_category());

        std::error_code callback_error_;
        bool callback_state_;
        uint16_t callback_number_of_times_success_;
        uint16_t callback_number_of_times_failure_;

        static const uint16_t pin_number_ = 400;

 public:
        void CallOnInterruptCallback(std::error_code error, bool state)
        {
            if (!callback_error_)
                callback_error_ = error;

            if (error)
            {
                callback_number_of_times_failure_++;
            }
            else
            {
                callback_state_ = state;
                callback_number_of_times_success_++;
            }
        }
};

// Output pin tests

TEST_F(GpioTest, Output_Initialize_Fail_Base)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(gpio_failure_));

    OutputPin output_pin(gpio_pin_);

    auto error = output_pin.Initialize(false);
    EXPECT_TRUE(error);
}

TEST_F(GpioTest, Output_Initialize_Fail_SetDirection)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::OUT))
        .WillOnce(Return(gpio_failure_));

    OutputPin output_pin(gpio_pin_);

    auto error = output_pin.Initialize(false);
    EXPECT_TRUE(error);
}

TEST_F(GpioTest, Output_Initialize_Success_Initial_High)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::OUT))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetValue(1));

    OutputPin output_pin(gpio_pin_);

    auto error = output_pin.Initialize(true);
    EXPECT_FALSE(error) << error;
}

TEST_F(GpioTest, Output_Initialize_Success_Initial_Low)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::OUT))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetValue(0));

    OutputPin output_pin(gpio_pin_);

    auto error = output_pin.Initialize(false);
    EXPECT_FALSE(error) << error;
}

TEST_F(GpioTest, Output_Close_Failure)
{
    EXPECT_CALL(gpio_pin_, Close())
        .WillOnce(Return(error_badf_));

    OutputPin output_pin(gpio_pin_);

    auto error = output_pin.Close();
    EXPECT_TRUE(error);
}

TEST_F(GpioTest, Output_Close_Success)
{
    EXPECT_CALL(gpio_pin_, Close())
        .WillOnce(Return(std::error_code()));

    OutputPin output_pin(gpio_pin_);

    auto error = output_pin.Close();
    EXPECT_FALSE(error) << error;
}

TEST_F(GpioTest, Output_Set)
{
    EXPECT_CALL(gpio_pin_, SetValue(true));

    OutputPin output_pin(gpio_pin_);

    output_pin.Set();
}

TEST_F(GpioTest, Output_Clear)
{
    EXPECT_CALL(gpio_pin_, SetValue(false));

    OutputPin output_pin(gpio_pin_);

    output_pin.Clear();
}

TEST_F(GpioTest, Output_Query_High)
{
    EXPECT_CALL(gpio_pin_, GetValue())
        .WillOnce(Return(true));

    OutputPin output_pin(gpio_pin_);

    bool value = output_pin.Query();
    EXPECT_TRUE(value);
}

TEST_F(GpioTest, Output_Query_Low)
{
    EXPECT_CALL(gpio_pin_, GetValue())
        .WillOnce(Return(false));

    OutputPin output_pin(gpio_pin_);

    bool value = output_pin.Query();
    EXPECT_FALSE(value);
}

TEST_F(GpioTest, Output_Toggle)
{
    {
        InSequence s;

        EXPECT_CALL(gpio_pin_, GetValue())
            .WillOnce(Return(true));
        EXPECT_CALL(gpio_pin_, SetValue(false));

        EXPECT_CALL(gpio_pin_, GetValue())
            .WillOnce(Return(false));
        EXPECT_CALL(gpio_pin_, SetValue(true));

        EXPECT_CALL(gpio_pin_, GetValue())
            .WillOnce(Return(true));
        EXPECT_CALL(gpio_pin_, SetValue(false));
    }

    OutputPin output_pin(gpio_pin_);

    output_pin.Toggle();
    output_pin.Toggle();
    output_pin.Toggle();
}

// Input pin tests

TEST_F(GpioTest, Input_Initialize_Fail_Base)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(gpio_failure_));

    InputPin input_pin(gpio_pin_);

    auto error = input_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(GpioTest, Input_Initialize_Fail_SetDirection)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::IN))
        .WillOnce(Return(gpio_failure_));

    InputPin input_pin(gpio_pin_);

    auto error = input_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(GpioTest, Input_Initialize_Success)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::IN))
        .WillOnce(Return(std::error_code()));

    InputPin input_pin(gpio_pin_);

    auto error = input_pin.Initialize();
    EXPECT_FALSE(error) << error;
}

TEST_F(GpioTest, Input_Close_Failure)
{
    EXPECT_CALL(gpio_pin_, Close())
        .WillOnce(Return(error_badf_));

    InputPin input_pin(gpio_pin_);

    auto error = input_pin.Close();
    EXPECT_TRUE(error);
}

TEST_F(GpioTest, Input_Close_Success)
{
    EXPECT_CALL(gpio_pin_, Close())
        .WillOnce(Return(std::error_code()));

    InputPin input_pin(gpio_pin_);

    auto error = input_pin.Close();
    EXPECT_FALSE(error) << error;
}

TEST_F(GpioTest, Input_Query_High)
{
    EXPECT_CALL(gpio_pin_, GetValue())
        .WillOnce(Return(true));

    InputPin input_pin(gpio_pin_);

    bool value = input_pin.Query();
    EXPECT_TRUE(value);
}

TEST_F(GpioTest, Input_Query_Low)
{
    EXPECT_CALL(gpio_pin_, GetValue())
        .WillOnce(Return(false));

    InputPin input_pin(gpio_pin_);

    bool value = input_pin.Query();
    EXPECT_FALSE(value);
}

// Input with rising edge callback pin tests

TEST_F(GpioTest, InputRisingEdge_Initialize_Fail_Base)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(gpio_failure_));

    InputPinWithRisingEdgeCallback input_pin(gpio_pin_, watchdog_, WatchdogContributingThreads::HFSM);

    auto error = input_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(GpioTest, InputRisingEdge_Initialize_Fail_SetDirection)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::IN))
        .WillOnce(Return(gpio_failure_));

    InputPinWithRisingEdgeCallback input_pin(gpio_pin_, watchdog_, WatchdogContributingThreads::HFSM);

    auto error = input_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(GpioTest, InputRisingEdge_Initialize_Fail_SetEdge)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::IN))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetEdge(IGPIO::Edge::RISING))
        .WillOnce(Return(gpio_failure_));

    InputPinWithRisingEdgeCallback input_pin(gpio_pin_, watchdog_, WatchdogContributingThreads::HFSM);

    auto error = input_pin.Initialize();
    EXPECT_TRUE(error);
}

TEST_F(GpioTest, InputRisingEdge_Initialize_Success)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::IN))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetEdge(IGPIO::Edge::RISING))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, GetValue())
        .WillOnce(Return(false));

    // Note: initialize launches the thread which Polls and calls the watchdog.
    // Because this test immediately destructs the input pin, this will likely
    // occur 0-1 times. WillRepeatedly is used for its flexibility.
    EXPECT_CALL(gpio_pin_, PollValue(1000))
        .WillRepeatedly(DoAll(
            USleep(100000),
            Return(std::make_pair(std::error_code(), false))));
    EXPECT_CALL(watchdog_, ResetThreadExpiration(0))
        .WillRepeatedly(Return());

    InputPinWithRisingEdgeCallback input_pin(gpio_pin_, watchdog_, WatchdogContributingThreads::HFSM);

    auto error = input_pin.Initialize();
    EXPECT_FALSE(error) << error;

    // No additional calls should be made from second initialization attempt
    error = input_pin.Initialize();
    EXPECT_FALSE(error) << error;
}

TEST_F(GpioTest, InputRisingEdge_Watchdog_Signaled_Each_Timeout)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::IN))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetEdge(IGPIO::Edge::RISING))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, GetValue())
        .WillOnce(Return(false));

    // 350,000 us should be enough to call 4 times.
    // 3 occur at n*100,000 us, and the last occurs on destruction (because the poll
    // will be in progress at 320,000 us, so that thread loop iteration will finish)
    EXPECT_CALL(gpio_pin_, PollValue(1000))
        .Times(4)
        .WillRepeatedly(DoAll(
            USleep(100000),
            Return(std::make_pair(std::error_code(), false))));
    EXPECT_CALL(watchdog_, ResetThreadExpiration(0))
        .Times(4);

    EXPECT_CALL(gpio_pin_, Close())
        .WillOnce(Return(std::error_code()));

    InputPinWithRisingEdgeCallback input_pin(gpio_pin_, watchdog_, WatchdogContributingThreads::HFSM);

    auto error = input_pin.Initialize();
    EXPECT_FALSE(error) << error;

    // Allow time for test to run
    usleep(350000);

    error = input_pin.Close();
    EXPECT_FALSE(error) << error;
}

TEST_F(GpioTest, InputRisingEdge_AllInterrupt_Query_High)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::IN))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetEdge(IGPIO::Edge::RISING))
        .WillOnce(Return(std::error_code()));

    // 350,000 us should be enough to call 4 times.
    // 3 occur at n*100,000 us, and the last occurs on destruction (because the poll
    // will be in progress at 320,000 us, so that thread loop iteration will finish)
    EXPECT_CALL(gpio_pin_, PollValue(1000))
        .Times(4)
        .WillRepeatedly(DoAll(
            USleep(100000),
            Return(std::make_pair(std::error_code(), true))));
    EXPECT_CALL(watchdog_, ResetThreadExpiration(0))
        .Times(4);

    {
        InSequence s;

        // Initialization dummy ready
        EXPECT_CALL(gpio_pin_, GetValue())
            .WillOnce(Return(false));

        // Calls during thread
        EXPECT_CALL(gpio_pin_, GetValue())
            .Times(4)
            .WillRepeatedly(Return(true));
    }

    EXPECT_CALL(gpio_pin_, Close())
        .WillOnce(Return(std::error_code()));

    InputPinWithRisingEdgeCallback input_pin(gpio_pin_, watchdog_, WatchdogContributingThreads::HFSM);

    callback_error_ = std::error_code();
    callback_number_of_times_success_ = 0;
    callback_number_of_times_failure_ = 0;

    auto callback = std::bind(&GpioTest::CallOnInterruptCallback, this,
        std::placeholders::_1, std::placeholders::_2);

    input_pin.SetRisingEdgeCallback(callback);

    auto error = input_pin.Initialize();
    EXPECT_FALSE(error) << error;

    // Allow time for test to run. Should execute 4 times.
    usleep(350000);

    error = input_pin.Close();
    EXPECT_FALSE(error) << error;

    EXPECT_FALSE(callback_error_) << callback_error_;
    EXPECT_EQ(callback_number_of_times_success_, 4u);
    EXPECT_EQ(callback_number_of_times_failure_, 0u);
    EXPECT_TRUE(callback_state_);
}

TEST_F(GpioTest, InputRisingEdge_AllInterrupt_Query_Low)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::IN))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetEdge(IGPIO::Edge::RISING))
        .WillOnce(Return(std::error_code()));

    // 350,000 us should be enough to call 4 times.
    // 3 occur at n*100,000 us, and the last occurs on destruction (because the poll
    // will be in progress at 320,000 us, so that thread loop iteration will finish)
    EXPECT_CALL(gpio_pin_, PollValue(1000))
        .Times(4)
        .WillRepeatedly(DoAll(
            USleep(100000),
            Return(std::make_pair(std::error_code(), true))));
    EXPECT_CALL(watchdog_, ResetThreadExpiration(0))
        .Times(4);

    {
        InSequence s;

        // Initialization dummy ready
        EXPECT_CALL(gpio_pin_, GetValue())
            .WillOnce(Return(false));

        // Calls during thread
        EXPECT_CALL(gpio_pin_, GetValue())
            .Times(4)
            .WillRepeatedly(Return(false));
    }

    EXPECT_CALL(gpio_pin_, Close())
        .WillOnce(Return(std::error_code()));

    InputPinWithRisingEdgeCallback input_pin(gpio_pin_, watchdog_, WatchdogContributingThreads::HFSM);

    callback_error_ = std::error_code();
    callback_number_of_times_success_ = 0;
    callback_number_of_times_failure_ = 0;

    auto callback = std::bind(&GpioTest::CallOnInterruptCallback, this,
        std::placeholders::_1, std::placeholders::_2);

    input_pin.SetRisingEdgeCallback(callback);

    auto error = input_pin.Initialize();
    EXPECT_FALSE(error) << error;

    // Allow time for test to run. Should execute 4 times.
    usleep(350000);

    error = input_pin.Close();
    EXPECT_FALSE(error) << error;

    EXPECT_FALSE(callback_error_) << callback_error_;
    EXPECT_EQ(callback_number_of_times_success_, 4u);
    EXPECT_EQ(callback_number_of_times_failure_, 0u);
    EXPECT_FALSE(callback_state_);
}

TEST_F(GpioTest, InputRisingEdge_AllError)
{
    EXPECT_CALL(gpio_pin_, Initialize())
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetDirection(IGPIO::Direction::IN))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, SetEdge(IGPIO::Edge::RISING))
        .WillOnce(Return(std::error_code()));

    EXPECT_CALL(gpio_pin_, GetValue())
        .WillOnce(Return(false));

    // 350,000 us should be enough to call 4 times.
    // 3 occur at n*100,000 us, and the last occurs on destruction (because the poll
    // will be in progress at 320,000 us, so that thread loop iteration will finish)
    EXPECT_CALL(gpio_pin_, PollValue(1000))
        .Times(4)
        .WillRepeatedly(DoAll(
            USleep(100000),
            Return(std::make_pair(gpio_failure_, false))));
    EXPECT_CALL(watchdog_, ResetThreadExpiration(0))
        .Times(4);

    EXPECT_CALL(gpio_pin_, Close())
        .WillOnce(Return(std::error_code()));

    InputPinWithRisingEdgeCallback input_pin(gpio_pin_, watchdog_, WatchdogContributingThreads::HFSM);

    callback_error_ = std::error_code();
    callback_number_of_times_success_ = 0;
    callback_number_of_times_failure_ = 0;

    auto callback = std::bind(&GpioTest::CallOnInterruptCallback, this,
        std::placeholders::_1, std::placeholders::_2);

    input_pin.SetRisingEdgeCallback(callback);

    auto error = input_pin.Initialize();
    EXPECT_FALSE(error) << error;

    // Allow time for test to run. Should execute 4 times.
    usleep(350000);

    error = input_pin.Close();
    EXPECT_FALSE(error) << error;

    EXPECT_EQ(callback_error_, gpio_failure_);
    EXPECT_EQ(callback_number_of_times_success_, 0u);
    EXPECT_EQ(callback_number_of_times_failure_, 4u);
}
