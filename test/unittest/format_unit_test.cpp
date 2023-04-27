#include <string>
#include <cmath>
#include <gtest/gtest.h>
#include "format.h"

using namespace testing::ext;
using namespace OHOS::Media;

namespace {
    constexpr float EPSINON_FLOAT = 0.0001;
    constexpr double EPSINON_DOUBLE = 0.0001;
} // namespace FormatTestParam

class FormatUnitTest : public testing::Test {
    public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);

    std::unique_ptr<Format> format_ {nullptr};
};

void FormatUnitTest::SetUpTestCase() {}

void FormatUnitTest::TearDownTestCase() {}

void FormatUnitTest::SetUp()
{
    format_ = std::make_unique<Format>();
}

void FormatUnitTest::TearDown()
{
    format_ = nullptr;
}

/**
 * @tc.name: Format_Value_0100
 * @tc.desc: format put and get value
 * @tc.type: FUNC
 * @tc.require: issueI5OX06 issueI5P8N0
 */
HWTEST_F(FormatUnitTest, Format_Value_0100, TestSize.Level0)
{
    const std::string_view intKey = "IntKey";
    const std::string_view longKey = "LongKey";
    const std::string_view floatKey = "FloatKey";
    const std::string_view doubleKey = "DoubleKey";
    const std::string_view stringKey = "StringKey";

    int32_t intValue = 1;
    int64_t longValue = 1;
    float floatValue = 1.0;
    double doubleValue = 1.0;
    const std::string stringValue = "StringValue";

    int32_t getIntValue = 0;
    int64_t getLongValue = 0;
    float getFloatValue = 0.0;
    double getDoubleValue = 0.0;
    std::string getStringValue = "";

    ASSERT_TRUE(format_ != nullptr);
    ASSERT_TRUE(format_->PutIntValue(intKey, intValue));
    ASSERT_TRUE(format_->GetIntValue(intKey, getIntValue));
    ASSERT_TRUE(intValue == getIntValue);
    ASSERT_FALSE(format_->GetLongValue(intKey, getLongValue));

    ASSERT_TRUE(format_->PutLongValue(longKey, intValue));
    ASSERT_TRUE(format_->GetLongValue(longKey, getLongValue));
    ASSERT_TRUE(longValue == getLongValue);
    ASSERT_FALSE(format_->GetIntValue(longKey, getIntValue));

    ASSERT_TRUE(format_->PutFloatValue(floatKey, floatValue));
    ASSERT_TRUE(format_->GetFloatValue(floatKey, getFloatValue));
    ASSERT_TRUE(fabs(floatValue - getFloatValue) < EPSINON_FLOAT);
    ASSERT_FALSE(format_->GetDoubleValue(floatKey, getDoubleValue));

    ASSERT_TRUE(format_->PutDoubleValue(doubleKey, doubleValue));
    ASSERT_TRUE(format_->GetDoubleValue(doubleKey, getDoubleValue));
    ASSERT_TRUE(fabs(doubleValue - getDoubleValue) < EPSINON_DOUBLE);
    ASSERT_FALSE(format_->GetFloatValue(doubleKey, getFloatValue));

    ASSERT_TRUE(format_->PutStringValue(stringKey, stringValue.c_str()));
    ASSERT_TRUE(format_->GetStringValue(stringKey, getStringValue));
    ASSERT_TRUE(stringValue == getStringValue);
}

/**
 * @tc.name: Format_Buffer_0100
 * @tc.desc: format put and get buffer
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(FormatUnitTest, Format_Buffer_0100, TestSize.Level0)
{
    constexpr size_t size = 3;
    const std::string_view key = "BufferKey";
    uint8_t buffer[size] = {'a', 'b', 'b'};

    ASSERT_TRUE(format_->PutBuffer(key, buffer, size));
    uint8_t *getBuffer;
    size_t getSize;
    ASSERT_TRUE(format_->GetBuffer(key, &getBuffer, getSize));
    ASSERT_TRUE(size == getSize);
    for (int32_t i = 0; i < size; i++) {
        ASSERT_TRUE(buffer[i] == getBuffer[i]);
    }

    std::string getString;
    ASSERT_FALSE(format_->GetStringValue(key, getString));
}