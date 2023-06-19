/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include "hdecoder_unit_test.h"
#include "hcodec_log.h"
#include "OMX_IVCommon.h"
#include "av_common.h" // foundation/multimedia/player_framework/interfaces/inner_api/native
#include "media_description.h" // foundation/multimedia/player_framework/interfaces/inner_api/native
#include "avcodec_info.h" // foundation/multimedia/player_framework/interfaces/inner_api/native

namespace OHOS::MediaAVCodec {
using namespace std;
using namespace testing::ext;

/*========================================================*/
/*                     HDecoderCallback                   */
/*========================================================*/
void HDecoderCallback::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
}

void HDecoderCallback::OnOutputFormatChanged(const Format &format)
{
}

void HDecoderCallback::OnInputBufferAvailable(uint32_t index)
{
    lock_guard<mutex> lk(signal_->inputMtx_);
    signal_->inputList_.push_back(index);
    signal_->inputCond_.notify_all();
}

void HDecoderCallback::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
}

/*========================================================*/
/*               HDecoderPreparingUnitTest                */
/*========================================================*/
void HDecoderPreparingUnitTest::SetUpTestCase(void)
{
}

void HDecoderPreparingUnitTest::TearDownTestCase(void)
{
}

void HDecoderPreparingUnitTest::SetUp(void)
{
}

void HDecoderPreparingUnitTest::TearDown(void)
{
}

sptr<Surface> HDecoderPreparingUnitTest::CreateOutputSurface()
{
    sptr<Surface> consumerSurface = Surface::CreateSurfaceAsConsumer();
    if (!consumerSurface) {
        LOGE("failed to create consumer surface");
        return nullptr;
    }
    sptr<IBufferProducer> bufferProducer = consumerSurface->GetProducer();
    if (!bufferProducer) {
        LOGE("failed to get producer from surface");
        return nullptr;
    }
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(bufferProducer);
    return producerSurface;
}

/* ============== CREATION ============== */
HWTEST_F(HDecoderPreparingUnitTest, create_by_avc_name, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj != nullptr);
}

HWTEST_F(HDecoderPreparingUnitTest, create_by_hevc_name, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.hevc");
    ASSERT_TRUE(testObj);
}

HWTEST_F(HDecoderPreparingUnitTest, create_by_empty_name, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("");
    ASSERT_FALSE(testObj);
}

/* ============== SET_CALLBACK ============== */
HWTEST_F(HDecoderPreparingUnitTest, set_empty_callback, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.hevc");
    ASSERT_TRUE(testObj);
    int32_t ret = testObj->SetCallback(nullptr);
    ASSERT_NE(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderPreparingUnitTest, set_valid_callback, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.hevc");
    ASSERT_TRUE(testObj);
    shared_ptr<HDecoderSignal> signal = make_shared<HDecoderSignal>();
    shared_ptr<HDecoderCallback> callback = make_shared<HDecoderCallback>(signal);
    int32_t ret = testObj->SetCallback(callback);
    ASSERT_EQ(AVCS_ERR_OK, ret);
}

/* ============== SET_OUTPUT_SURFACE ============== */
HWTEST_F(HDecoderPreparingUnitTest, set_empty_output_surface, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.hevc");
    ASSERT_TRUE(testObj);
    int32_t ret = testObj->SetOutputSurface(nullptr);
    ASSERT_NE(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderPreparingUnitTest, set_valid_output_surface, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);
    sptr<Surface> surface = CreateOutputSurface();
    int32_t ret = testObj->SetOutputSurface(surface);
    ASSERT_EQ(AVCS_ERR_OK, ret);
}

/* ============== CONFIGURE ============== */
HWTEST_F(HDecoderPreparingUnitTest, configure_ok, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);
    Format format;
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_AVC);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 1024);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 768);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, NV12);
    int32_t ret = testObj->Configure(format);
    ASSERT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderPreparingUnitTest, configure_no_width, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);
    Format format;
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_AVC);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 768);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, NV12);
    int32_t ret = testObj->Configure(format);
    ASSERT_EQ(AVCS_ERR_INVALID_VAL, ret);
}

HWTEST_F(HDecoderPreparingUnitTest, configure_no_height, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);
    Format format;
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_AVC);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 1024);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, NV12);
    int32_t ret = testObj->Configure(format);
    ASSERT_EQ(AVCS_ERR_INVALID_VAL, ret);
}

HWTEST_F(HDecoderPreparingUnitTest, configure_no_color_format, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);
    Format format;
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_AVC);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 1024);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 768);
    int32_t ret = testObj->Configure(format);
    ASSERT_EQ(AVCS_ERR_INVALID_VAL, ret);
}

/* ============== SET_PARAMETERS ============== */
HWTEST_F(HDecoderPreparingUnitTest, set_parameters_before_codec_is_running, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    Format format;
    int32_t ret = testObj->SetParameter(format);
    ASSERT_NE(AVCS_ERR_OK, ret);
}

/* ============== GET_OUTPUT_FORMAT ============== */
HWTEST_F(HDecoderPreparingUnitTest, get_output_format_before_configure, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    Format format;
    int32_t ret = testObj->GetOutputFormat(format);
    ASSERT_NE(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderPreparingUnitTest, get_output_format_after_configure, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    Format format;
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_AVC);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 1024);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 768);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, NV12);
    int32_t ret = testObj->Configure(format);
    ASSERT_EQ(AVCS_ERR_OK, ret);

    Format outputFormat;
    outputFormat.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 2048);
    outputFormat.PutIntValue("test", 666);
    ret = testObj->GetOutputFormat(outputFormat);
    ASSERT_EQ(AVCS_ERR_OK, ret);

    int32_t width = 0;
    ASSERT_TRUE(outputFormat.GetIntValue(MediaDescriptionKey::MD_KEY_WIDTH, width));
    ASSERT_EQ(1024, width);

    int32_t height = 0;
    ASSERT_TRUE(outputFormat.GetIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, height));
    ASSERT_EQ(768, height);

    int32_t colorFormat = 0;
    ASSERT_TRUE(outputFormat.GetIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, colorFormat));
    ASSERT_EQ(NV12, colorFormat);

    int32_t test = 0;
    ASSERT_FALSE(outputFormat.GetIntValue("test", test));
}

/* ============== CREATE_INPUT_SURFACE ============== */
HWTEST_F(HDecoderPreparingUnitTest, unsupported_op_create_input_surface, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    sptr<Surface> inputSurface = testObj->CreateInputSurface();
    ASSERT_FALSE(inputSurface);
}

/* ============================ MULTIPLE_INSTANCE ============================ */
HWTEST_F(HDecoderPreparingUnitTest, create_multiple_instance, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj1 = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj1);

    std::shared_ptr<HCodec> testObj2 = HCodec::Create("OMX.hisi.video.decoder.hevc");
    ASSERT_TRUE(testObj2);

    testObj1.reset();
    ASSERT_TRUE(testObj2);
}

/*========================================================*/
/*              HDecoderUserCallingUnitTest               */
/*========================================================*/
void HDecoderUserCallingUnitTest::SetUpTestCase(void)
{
}

void HDecoderUserCallingUnitTest::TearDownTestCase(void)
{
}

void HDecoderUserCallingUnitTest::SetUp(void)
{
    signal_ = make_shared<HDecoderSignal>();
}

void HDecoderUserCallingUnitTest::TearDown(void)
{
}

void HDecoderUserCallingUnitTest::Listener::OnBufferAvailable()
{
    sptr<SurfaceBuffer> buffer;
    int32_t fence;
    int64_t timestamp;
    OHOS::Rect damage;
    GSError err = mTest->mConsumer->AcquireBuffer(buffer, fence, timestamp, damage);
    if (err != GSERROR_OK || buffer == nullptr) {
        LOGW("AcquireBuffer failed");
        return;
    }
    mTest->mConsumer->ReleaseBuffer(buffer, -1);
}

sptr<Surface> HDecoderUserCallingUnitTest::CreateOutputSurface()
{
    sptr<Surface> consumerSurface = Surface::CreateSurfaceAsConsumer();
    if (consumerSurface == nullptr) {
        LOGE("CreateSurfaceAsConsumer failed");
        return nullptr;
    }
    sptr<IBufferConsumerListener> listener = new Listener(this);
    GSError err = consumerSurface->RegisterConsumerListener(listener);
    if (err != GSERROR_OK) {
        LOGE("RegisterConsumerListener failed");
        return nullptr;
    }
    sptr<IBufferProducer> bufferProducer = consumerSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(bufferProducer);
    if (producerSurface == nullptr) {
        LOGE("CreateSurfaceAsProducer failed");
        return nullptr;
    }
    mConsumer = consumerSurface;
    return producerSurface;
}

bool HDecoderUserCallingUnitTest::ConfigureDecoder(std::shared_ptr<HCodec>& decoder)
{
    Format format;
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_AVC);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 1024); // 1024 width of the video
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 768); // 768 hight of the video
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, NV12);
    int32_t ret = decoder->Configure(format);
    return (ret == AVCS_ERR_OK);
}

bool HDecoderUserCallingUnitTest::SetOutputSurfaceToDecoder(std::shared_ptr<HCodec>& decoder)
{
    sptr<Surface> surface = CreateOutputSurface();
    int32_t ret = decoder->SetOutputSurface(surface);
    return (ret == AVCS_ERR_OK);
}

bool HDecoderUserCallingUnitTest::SetCallbackToDecoder(std::shared_ptr<HCodec>& decoder)
{
    shared_ptr<HDecoderCallback> callback = make_shared<HDecoderCallback>(signal_);
    int32_t ret = decoder->SetCallback(callback);
    return (ret == AVCS_ERR_OK);
}

/* ============================ SETTINGS ============================ */
HWTEST_F(HDecoderUserCallingUnitTest, set_callback_when_codec_is_running, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    EXPECT_FALSE(SetCallbackToDecoder(testObj));

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, configure_when_codec_is_running, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    EXPECT_FALSE(ConfigureDecoder(testObj));

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, set_parameters_when_codec_is_running, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    Format format;
    ret = testObj->SetParameter(format);
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

/* ============================ START ============================ */
HWTEST_F(HDecoderUserCallingUnitTest, start_normal, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, start_without_setting_callback, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_NE(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, start_without_setting_output_surface, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, start_without_setting_configure, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_NE(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, start_stop_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Stop();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, start_release_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Start();
    EXPECT_NE(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, start_and_not_release, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

/* ============================ STOP ============================ */
HWTEST_F(HDecoderUserCallingUnitTest, stop_without_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Stop();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, stop_without_configure_and_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.hevc");
    ASSERT_TRUE(testObj);

    int32_t ret = testObj->Stop();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

/* ============================ RELEASE ============================ */
HWTEST_F(HDecoderUserCallingUnitTest, release_without_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, release_without_configure_and_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    int32_t ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

/* ============================ FLUSH ============================ */
HWTEST_F(HDecoderUserCallingUnitTest, start_flush, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Flush();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

/* ============================ RESET ============================ */
HWTEST_F(HDecoderUserCallingUnitTest, reset_without_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.hevc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Reset();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, reset_after_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Reset();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, start_reset_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.hevc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Reset();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ASSERT_TRUE(ConfigureDecoder(testObj));

    ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, start_reset_configure_start, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Reset();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ASSERT_TRUE(ConfigureDecoder(testObj));

    ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

/* ============================ COMBO OP ============================ */
HWTEST_F(HDecoderUserCallingUnitTest, combo_op_1, TestSize.Level1)
{
    // start - stop - start - stop - start - release
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.hevc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Stop();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    EXPECT_TRUE(ConfigureDecoder(testObj));
    ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Stop();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    EXPECT_TRUE(ConfigureDecoder(testObj));
    ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, combo_op_2, TestSize.Level1)
{
    // start - Reset - start - Reset - start - stop - start - release
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Reset();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ASSERT_TRUE(ConfigureDecoder(testObj));
    ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Reset();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Start();
    EXPECT_NE(AVCS_ERR_OK, ret);

    ASSERT_TRUE(ConfigureDecoder(testObj));
    ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Stop();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    EXPECT_TRUE(ConfigureDecoder(testObj));
    ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

/* ============================ GetInputBuffer ============================ */
HWTEST_F(HDecoderUserCallingUnitTest, get_input_buffer_null, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    {
        unique_lock<mutex> lk(signal_->inputMtx_);
        signal_->inputCond_.wait(lk, [this]{return !signal_->inputList_.empty();});
    }
    std::shared_ptr<AVSharedMemoryBase> buffer = testObj->GetInputBuffer(0);
    EXPECT_FALSE(buffer);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, get_input_buffer_ok, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    uint32_t bufferIndex = 0;
    {
        unique_lock<mutex> lk(signal_->inputMtx_);
        signal_->inputCond_.wait(lk, [this]{return !signal_->inputList_.empty();});
        bufferIndex = signal_->inputList_.front();
        signal_->inputList_.pop_front();
    }
    std::shared_ptr<AVSharedMemoryBase> buffer = testObj->GetInputBuffer(bufferIndex);
    EXPECT_TRUE(buffer);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

/* ============================ QueueInputBuffer ============================ */
HWTEST_F(HDecoderUserCallingUnitTest, queue_input_buffer_invalid_index, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    {
        unique_lock<mutex> lk(signal_->inputMtx_);
        signal_->inputCond_.wait(lk, [this]{return !signal_->inputList_.empty();});
    }
    AVCodecBufferInfo bufferInfo;
    ret = testObj->QueueInputBuffer(0, bufferInfo, AVCODEC_BUFFER_FLAG_NONE);
    EXPECT_NE(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

HWTEST_F(HDecoderUserCallingUnitTest, queue_input_buffer_ok, TestSize.Level1)
{
    std::shared_ptr<HCodec> testObj = HCodec::Create("OMX.hisi.video.decoder.avc");
    ASSERT_TRUE(testObj);

    ASSERT_TRUE(SetOutputSurfaceToDecoder(testObj));
    ASSERT_TRUE(SetCallbackToDecoder(testObj));
    ASSERT_TRUE(ConfigureDecoder(testObj));

    int32_t ret = testObj->Start();
    EXPECT_EQ(AVCS_ERR_OK, ret);

    uint32_t bufferIndex = 0;
    {
        unique_lock<mutex> lk(signal_->inputMtx_);
        signal_->inputCond_.wait(lk, [this]{return !signal_->inputList_.empty();});
        bufferIndex = signal_->inputList_.front();
        signal_->inputList_.pop_front();
    }
    std::shared_ptr<AVSharedMemoryBase> buffer = testObj->GetInputBuffer(bufferIndex);
    AVCodecBufferInfo bufferInfo;
    bufferInfo.size = 0;
    ret = testObj->QueueInputBuffer(bufferIndex, bufferInfo, AVCODEC_BUFFER_FLAG_NONE);
    EXPECT_EQ(AVCS_ERR_OK, ret);

    ret = testObj->Release();
    EXPECT_EQ(AVCS_ERR_OK, ret);
}

}