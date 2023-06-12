/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "videodec_unit_test.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;
using namespace testing::mt;
using namespace OHOS::Media::VCodecTestParam;

namespace {
std::atomic<int32_t> vdecCount = 0;

void MultiThreadCreateVDec()
{
    std::shared_ptr<VDecSignal> vdecSignal = std::make_shared<VDecSignal>();
    std::shared_ptr<VDecCallbackTest> adecCallback = std::make_shared<VDecCallbackTest>(vdecSignal);
    ASSERT_NE(nullptr, adecCallback);

    std::shared_ptr<VideoDecSample> videoDec = std::make_shared<VideoDecSample>(vdecSignal);
    ASSERT_NE(nullptr, videoDec);

    EXPECT_LE(vdecCount.load(), 16); // 16: max instances supported
    if (videoDec->CreateVideoDecMockByName(VDEC_AVC_NAME)) {
        vdecCount++;
        cout << "create successed, num:" << vdecCount.load() << endl;
    } else {
        cout << "create failed, num:" << vdecCount.load() << endl;
        return;
    }
    sleep(1);
    videoDec->Release();
    vdecCount--;
}
} // namespace

void VideoDecUnitTest::SetUpTestCase(void) {}

void VideoDecUnitTest::TearDownTestCase(void) {}

void VideoDecUnitTest::SetUp(void)
{
    std::shared_ptr<VDecSignal> vdecSignal = std::make_shared<VDecSignal>();
    vdecCallback_ = std::make_shared<VDecCallbackTest>(vdecSignal);
    ASSERT_NE(nullptr, vdecCallback_);

    videoDec_ = std::make_shared<VideoDecSample>(vdecSignal);
    ASSERT_NE(nullptr, videoDec_);

    format_ = FormatMockFactory::CreateFormat();
    ASSERT_NE(nullptr, format_);
}

void VideoDecUnitTest::TearDown(void)
{
    if (format_ != nullptr) {
        format_->Destroy();
    }
}

bool VideoDecUnitTest::CreateVideoCodecByMime(const std::string &decMime)
{
    if (videoDec_->CreateVideoDecMockByMime(decMime) == false || videoDec_->SetCallback(vdecCallback_) != AV_ERR_OK) {
        return false;
    }
    return true;
}

bool VideoDecUnitTest::CreateVideoCodecByName(const std::string &decName)
{
    if (videoDec_->CreateVideoDecMockByName(decName) == false || videoDec_->SetCallback(vdecCallback_) != AV_ERR_OK) {
        return false;
    }
    return true;
}

void VideoDecUnitTest::CreateByNameWithParam(void)
{
    std::string codecName = GetParam(); // gtest with param
    ASSERT_TRUE(CreateVideoCodecByName(codecName));
    std::cout << "CodecName: " << codecName << std::endl;
}

void VideoDecUnitTest::CreateByMimeWithParam(void)
{
    std::string codecName = GetParam(); // gtest with param
    if (codecName == VDEC_AVC_NAME) {
        ASSERT_TRUE(CreateVideoCodecByMime((CodecMimeType::VIDEO_AVC).data()));
    }
    std::cout << "CodecName: " << codecName << std::endl;
}

void VideoDecUnitTest::SetFormatWithParam(void)
{
    std::string codecName = GetParam(); // gtest with param
    std::cout << "SourcePath: " << VDEC_SOURCE.at(codecName) << std::endl;
    videoDec_->SetSource(VDEC_SOURCE.at(codecName));
    const ::testing::TestInfo *testInfo_ = ::testing::UnitTest::GetInstance()->current_test_info();
    string prefix = "/data/test/media/";
    string fileName = testInfo_->name();
    auto check = [](char it) { return it == '/'; };
    (void)fileName.erase(std::remove_if(fileName.begin(), fileName.end(), check), fileName.end());
    videoDec_->SetOutPath(prefix + fileName);
    if (codecName == VDEC_AVC_NAME) {
        format_->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
        format_->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_HEIGHT);
    }
}

INSTANTIATE_TEST_SUITE_P(VideoDecUnitTestWithParam, VideoDecUnitTest, testing::Values(VDEC_AVC_NAME));

/**
 * @tc.name: videoDecoder_multithread_create_001
 * @tc.desc: try create 100 instances
 * @tc.type: FUNC
 */
HWTEST_F(VideoDecUnitTest, videoDecoder_multithread_create_001, TestSize.Level1)
{
    SET_THREAD_NUM(100);
    vdecCount = 0;
    GTEST_RUN_TASK(MultiThreadCreateVDec);
    cout << "remaining num: " << vdecCount.load() << endl;
}

/**
 * @tc.name: videoDecoder_createWithNull_001
 * @tc.desc: video create
 * @tc.type: FUNC
 */
HWTEST_F(VideoDecUnitTest, videoDecoder_createWithNull_001, TestSize.Level1)
{
    ASSERT_FALSE(CreateVideoCodecByName(""));
}

/**
 * @tc.name: videoDecoder_createWithNull_002
 * @tc.desc: video create
 * @tc.type: FUNC
 */
HWTEST_F(VideoDecUnitTest, videoDecoder_createWithNull_002, TestSize.Level1)
{
    ASSERT_FALSE(CreateVideoCodecByMime(""));
}

/**
 * @tc.name: videoDecoder_create_001
 * @tc.desc: video create
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_create_001, TestSize.Level1)
{
    CreateByNameWithParam();
}

/**
 * @tc.name: videoDecoder_create_002
 * @tc.desc: video create
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_create_002, TestSize.Level1)
{
    CreateByMimeWithParam();
}

/**
 * @tc.name: videoDecoder_configure_001
 * @tc.desc: correct key input.
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_configure_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, 0);     // set rotation_angle 0
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE, 15000); // set max input size 15000
    EXPECT_EQ(AV_ERR_OK, videoDec_->Configure(format_));
}

/**
 * @tc.name: videoDecoder_configure_002
 * @tc.desc: correct key input with redundancy key input
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_configure_002, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, 0);     // set rotation_angle 0
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE, 15000); // set max input size 15000
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, 1);        // redundancy key
    EXPECT_EQ(AV_ERR_OK, videoDec_->Configure(format_));
}

/**
 * @tc.name: videoDecoder_configure_003
 * @tc.desc: correct key input with wrong value type input
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_configure_003, TestSize.Level1)
{
    CreateByNameWithParam();
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, -2); // invalid width size -2
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_HEIGHT);
    EXPECT_EQ(AV_ERR_OK, videoDec_->Configure(format_));
}

/**
 * @tc.name: videoDecoder_configure_004
 * @tc.desc: correct key input with wrong value type input
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_configure_004, TestSize.Level1)
{
    CreateByNameWithParam();
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, -2); // invalid height size -2
    EXPECT_EQ(AV_ERR_OK, videoDec_->Configure(format_));
}

/**
 * @tc.name: videoDecoder_configure_005
 * @tc.desc: empty format input
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_configure_005, TestSize.Level1)
{
    CreateByNameWithParam();
    EXPECT_NE(AV_ERR_OK, videoDec_->Configure(format_));
}

/**
 * @tc.name: videoDecoder_start_001
 * @tc.desc: correct flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_start_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
}

/**
 * @tc.name: videoDecoder_start_002
 * @tc.desc: correct flow 2
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_start_002, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
}

/**
 * @tc.name: videoDecoder_start_003
 * @tc.desc: correct flow 2
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_start_003, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Flush());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
}

/**
 * @tc.name: videoDecoder_start_004
 * @tc.desc: wrong flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_start_004, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_NE(AV_ERR_OK, videoDec_->Start());
}

/**
 * @tc.name: videoDecoder_start_005
 * @tc.desc: wrong flow 2
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_start_005, TestSize.Level1)
{
    CreateByNameWithParam();
    EXPECT_NE(AV_ERR_OK, videoDec_->Start());
}

/**
 * @tc.name: videoDecoder_stop_001
 * @tc.desc: correct flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_stop_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
}

/**
 * @tc.name: videoDecoder_stop_002
 * @tc.desc: correct flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_stop_002, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Flush());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
}

/**
 * @tc.name: videoDecoder_stop_003
 * @tc.desc: wrong flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_stop_003, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_NE(AV_ERR_OK, videoDec_->Stop());
}

/**
 * @tc.name: videoDecoder_flush_001
 * @tc.desc: correct flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_flush_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Flush());
}

/**
 * @tc.name: videoDecoder_flush_002
 * @tc.desc: wrong flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_flush_002, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Release());
    EXPECT_NE(AV_ERR_OK, videoDec_->Flush());
}

/**
 * @tc.name: videoDecoder_reset_001
 * @tc.desc: correct flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_reset_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Reset());
}

/**
 * @tc.name: videoDecoder_reset_002
 * @tc.desc: correct flow 2
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_reset_002, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Reset());
}

/**
 * @tc.name: videoDecoder_reset_003
 * @tc.desc: correct flow 3
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_reset_003, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Reset());
}

/**
 * @tc.name: videoDecoder_release_001
 * @tc.desc: correct flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_release_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Release());
}

/**
 * @tc.name: videoDecoder_release_002
 * @tc.desc: correct flow 2
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_release_002, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Release());
}

/**
 * @tc.name: videoDecoder_release_003
 * @tc.desc: correct flow 3
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_release_003, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Release());
}

/**
 * @tc.name: videoDecoder_setsurface_001
 * @tc.desc: video decodec setsurface
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_setsurface_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));
    ASSERT_EQ(AV_ERR_OK, videoDec_->SetOutputSurface());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
}

/**
 * @tc.name: videoDecoder_setsurface_002
 * @tc.desc: wrong flow 1
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_setsurface_002, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_NE(AV_ERR_OK, videoDec_->SetOutputSurface());
}

/**
 * @tc.name: videoDecoder_setsurface_003
 * @tc.desc: wrong flow 2
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_setsurface_003, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));
    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    ASSERT_NE(AV_ERR_OK, videoDec_->SetOutputSurface());
}

/**
 * @tc.name: videoDecoder_abnormal_001
 * @tc.desc: video codec abnormal func
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_abnormal_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, 20); // invalid rotation_angle 20
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE, -1); // invalid max input size -1

    videoDec_->Configure(format_);
    EXPECT_EQ(AV_ERR_OK, videoDec_->Reset());
    EXPECT_NE(AV_ERR_OK, videoDec_->Start());
    EXPECT_NE(AV_ERR_OK, videoDec_->Flush());
    EXPECT_NE(AV_ERR_OK, videoDec_->Stop());
}

/**
 * @tc.name: videoDecoder_abnormal_002
 * @tc.desc: video codec abnormal func
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_abnormal_002, TestSize.Level1)
{
    CreateByNameWithParam();
    EXPECT_NE(AV_ERR_OK, videoDec_->Start());

    CreateByNameWithParam();
    EXPECT_NE(AV_ERR_OK, videoDec_->Flush());

    CreateByNameWithParam();
    EXPECT_NE(AV_ERR_OK, videoDec_->Stop());
}

/**
 * @tc.name: videoDecoder_setParameter_001
 * @tc.desc: video codec SetParameter
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_setParameter_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    format_ = FormatMockFactory::CreateFormat();
    ASSERT_NE(nullptr, format_);

    format_->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_HEIGHT);
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, YUV420P);
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_FRAME_RATE, DEFAULT_FRAME_RATE);
    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->SetParameter(format_));
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
}

/**
 * @tc.name: videoDecoder_setParameter_002
 * @tc.desc: video codec SetParameter
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_setParameter_002, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    format_ = FormatMockFactory::CreateFormat();
    ASSERT_NE(nullptr, format_);

    format_->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, -2);  // invalid width size -2
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, -2); // invalid height size -2
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, YUV420P);
    format_->PutIntValue(MediaDescriptionKey::MD_KEY_FRAME_RATE, DEFAULT_FRAME_RATE);
    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    EXPECT_EQ(AV_ERR_OK, videoDec_->SetParameter(format_));
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
}

/**
 * @tc.name: videoDecoder_getOutputDescription_001
 * @tc.desc: video codec GetOutputDescription
 * @tc.type: FUNC
 */
HWTEST_P(VideoDecUnitTest, videoDecoder_getOutputDescription_001, TestSize.Level1)
{
    CreateByNameWithParam();
    SetFormatWithParam();
    ASSERT_EQ(AV_ERR_OK, videoDec_->Configure(format_));

    EXPECT_EQ(AV_ERR_OK, videoDec_->Start());
    format_ = videoDec_->GetOutputDescription();
    EXPECT_NE(nullptr, format_);
    EXPECT_EQ(AV_ERR_OK, videoDec_->Stop());
}
