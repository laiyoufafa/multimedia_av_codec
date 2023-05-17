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

#include <vector>
#include <queue>
#include <mutex>
#include <gtest/gtest.h>
#include "native_avcodec_audiodecoder.h"
#include "audio_ffmpeg_adapter.h"
#include "format.h"
#include "avcodec_audio_decoder.h"
#include "avcodec_codec_name.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "media_description.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Media;

namespace {
const string CODEC_MP3_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_MP3_NAME_KEY);
const string CODEC_FLAC_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_FLAC_NAME_KEY);
const string CODEC_AAC_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_AAC_NAME_KEY);
constexpr uint32_t MAX_CHANNEL_COUNT = 2;
constexpr uint32_t INVALID_CHANNEL_COUNT = 3;
constexpr uint32_t DEFAULT_SAMPLE_RATE = 8000;
constexpr uint32_t INVALID_SAMPLE_RATE = 9999990;
constexpr uint32_t DEFAULT_BITRATE = 128000;
constexpr uint32_t DEFAULT_WIDTH = 0;
constexpr uint32_t DEFAULT_BITS_PER_CODED_RATE = 16;
constexpr uint32_t DEFAULT_AAC_TYPE = 1;
constexpr uint32_t DEFAULT_AAC_LATM_TYPE = 0;
} // namespace

namespace OHOS {
namespace Media {
class ADecSignal {
public:
    std::mutex inMutex_;
    std::mutex outMutex_;
    std::condition_variable inCond_;
    std::condition_variable outCond_;
    std::queue<int32_t> inIdxQueue_;
    std::queue<int32_t> outIdxQueue_;
    std::queue<AVCodecBufferInfo> infoQueue_;
    std::queue<AVCodecBufferFlag> flagQueue_;
};

class BufferCallback : public AVCodecCallback {
public:
    explicit BufferCallback(ADecSignal *userData) : userData_(userData) {}
    virtual ~BufferCallback() = default;
    ADecSignal *userData_;
    virtual void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    virtual void OnOutputFormatChanged(const Format &format) override;
    virtual void OnInputBufferAvailable(size_t index) override;
    virtual void OnOutputBufferAvailable(size_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
};

void BufferCallback::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
    (void)errorType;
    cout << "Error errorCode=" << errorCode << endl;
}

void BufferCallback::OnOutputFormatChanged(const Format &format)
{
    (void)format;
    cout << "Format Changed" << endl;
}

void BufferCallback::OnInputBufferAvailable(size_t index)
{
    unique_lock<mutex> lock(userData_->inMutex_);
    userData_->inIdxQueue_.push(index);
    userData_->inCond_.notify_all();
}

void BufferCallback::OnOutputBufferAvailable(size_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    unique_lock<mutex> lock(userData_->outMutex_);
    userData_->outIdxQueue_.push(index);
    userData_->infoQueue_.push(info);
    userData_->flagQueue_.push(flag);
    userData_->outCond_.notify_all();
    (void)flag;
}

class AudioCodeDecoderInnerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int32_t ProceMp3Func();
    int32_t ProceFlacFunc();
    int32_t ProceAacFunc();

protected:
    int32_t index_;
    int64_t timeStamp_{0};

    ADecSignal *signal_;

    FILE *inFile_{nullptr};
    FILE *dumpFd_{nullptr};
    std::string codecName_{CODEC_MP3_NAME};

    OHOS::Media::Format format_;
    std::shared_ptr<AVCodecAudioDecoder> adec_ = {nullptr};
};

void AudioCodeDecoderInnerUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void AudioCodeDecoderInnerUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void AudioCodeDecoderInnerUnitTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
    adec_ = AudioDecoderFactory::CreateByName(codecName_);
    ASSERT_NE(nullptr, adec_);

    signal_ = new ADecSignal();
    ASSERT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK,
              adec_->SetCallback(std::shared_ptr<AVCodecCallback>(std::make_shared<BufferCallback>(signal_))));
}

void AudioCodeDecoderInnerUnitTest::TearDown(void)
{
    adec_->Release();
    cout << "[TearDown]: over!!!" << endl;
}

int32_t AudioCodeDecoderInnerUnitTest::ProceMp3Func(void)
{
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);

    if (adec_->Configure(format_) != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    } else if (adec_->Start() != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodeDecoderInnerUnitTest::ProceFlacFunc(void)
{
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_BITS_PER_CODED_SAMPLE_KEY, DEFAULT_BITS_PER_CODED_RATE);

    if (adec_->Configure(format_) != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    } else if (adec_->Start() != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodeDecoderInnerUnitTest::ProceAacFunc(void)
{
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_TYPE);

    if (adec_->Configure(format_) != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    } else if (adec_->Start() != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Configure_01, TestSize.Level1)
{
    // lack of correct key
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Configure_02, TestSize.Level1)
{
    // correct key input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);

    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Configure_03, TestSize.Level1)
{
    // correct key input with redundancy key input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Configure_04, TestSize.Level1)
{
    // correct key input with wrong value type input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Configure_05, TestSize.Level1)
{
    // correct key input with wrong value type input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, INVALID_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, INVALID_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Configure_06, TestSize.Level1)
{
    // empty format input
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Start_01, TestSize.Level1)
{
    // correct flow 1
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Start_02, TestSize.Level1)
{
    // correct flow 2
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Start_03, TestSize.Level1)
{
    // wrong flow 1
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Start_04, TestSize.Level1)
{
    // wrong flow 2
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Stop_01, TestSize.Level1)
{
    // correct flow 1
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Stop_02, TestSize.Level1)
{
    // correct flow 2
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Flush_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Flush());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Flush_02, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Flush());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Reset_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Reset_02, TestSize.Level1)
{
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Reset_03, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Release_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Release_02, TestSize.Level1)
{
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_Release_03, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_SetParameter_01, TestSize.Level1)
{
    // 尚未实现
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE, adec_->SetParameter(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_GetOutputFormat_01, TestSize.Level1)
{
    // 尚未实现
    EXPECT_EQ(AVCS_ERR_OK, ProceMp3Func());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->GetOutputFormat(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_GetInputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    sleep(1);
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;
    index_ = -1;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = 1024;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = signal_->inIdxQueue_.front();
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_NE(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_QueueInputBuffer_01, TestSize.Level1)
{
    AVCodecBufferInfo info;
    AVCodecBufferFlag flag = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;

    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceMp3Func());
    sleep(1);
    // case0 传参异常
    index_ = -1;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY, adec_->QueueInputBuffer(index_, info, flag));
    // case1 info未赋值
    index_ = 1024;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY, adec_->QueueInputBuffer(index_, info, flag));

    // case2 EOS帧数据
    index_ = 0;
    info.presentationTimeUs = 0;
    info.size = -1;
    info.offset = 0;
    flag = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->QueueInputBuffer(index_, info, flag));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_GetOutputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceMp3Func());
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;

    // case1 传参异常
    index_ = -1;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
    index_ = 1024;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Mp3_ReleaseOutputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceMp3Func());

    // case1 传参异常
    index_ = -1;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
    index_ = 1024;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
    // case2 传参正常
    index_ = 0;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_SetCodecName_01, TestSize.Level1)
{
    codecName_ = CODEC_FLAC_NAME;
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Configure_01, TestSize.Level1)
{
    // lack of correct key
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Configure_02, TestSize.Level1)
{
    // correct key input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_BITS_PER_CODED_SAMPLE_KEY, DEFAULT_BITS_PER_CODED_RATE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Configure_03, TestSize.Level1)
{
    // correct key input with redundancy key input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_BITS_PER_CODED_SAMPLE_KEY, DEFAULT_BITS_PER_CODED_RATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Configure_04, TestSize.Level1)
{
    // correct key input with wrong value type input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_BITS_PER_CODED_SAMPLE_KEY, DEFAULT_BITS_PER_CODED_RATE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Configure_05, TestSize.Level1)
{
    // correct key input with wrong value type input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, INVALID_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, INVALID_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_BITS_PER_CODED_SAMPLE_KEY, DEFAULT_BITS_PER_CODED_RATE);
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Configure_06, TestSize.Level1)
{
    // empty format input
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Start_01, TestSize.Level1)
{
    // correct flow 1
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Start_02, TestSize.Level1)
{
    // correct flow 2
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Start_03, TestSize.Level1)
{
    // wrong flow 1
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Start_04, TestSize.Level1)
{
    // wrong flow 2
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Stop_01, TestSize.Level1)
{
    // correct flow 1
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Stop_02, TestSize.Level1)
{
    // correct flow 2
    format_.PutIntValue(MediaDescriptionKEY::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_BITS_PER_CODED_SAMPLE_KEY, DEFAULT_BITS_PER_CODED_RATE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Flush_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Flush());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Flush_02, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Flush());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Reset_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Reset_02, TestSize.Level1)
{
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_BITS_PER_CODED_SAMPLE_KEY, DEFAULT_BITS_PER_CODED_RATE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Reset_03, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Release_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Release_02, TestSize.Level1)
{
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_BITS_PER_CODED_SAMPLE_KEY, DEFAULT_BITS_PER_CODED_RATE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_Release_03, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_SetParameter_01, TestSize.Level1)
{
    // 尚未实现
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE, adec_->SetParameter(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_GetOutputFormat_01, TestSize.Level1)
{
    // 尚未实现
    EXPECT_EQ(AVCS_ERR_OK, ProceFlacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->GetOutputFormat(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_GetInputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    sleep(1);
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;
    index_ = -1;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = 1024;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = signal_->inIdxQueue_.front();
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_NE(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_QueueInputBuffer_01, TestSize.Level1)
{
    AVCodecBufferInfo info;
    AVCodecBufferFlag flag = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
    ;

    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFlacFunc());
    sleep(1);
    // case0 传参异常
    index_ = -1;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY, adec_->QueueInputBuffer(index_, info, flag));
    // case1 info未赋值
    index_ = 1024;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY, adec_->QueueInputBuffer(index_, info, flag));

    // case2 EOS帧数据
    index_ = 0;
    info.presentationTimeUs = 0;
    info.size = -1;
    info.offset = 0;
    flag = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->QueueInputBuffer(index_, info, flag));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_GetOutputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFlacFunc());
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;

    // case1 传参异常
    index_ = -1;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
    index_ = 1024;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Flac_ReleaseOutputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFlacFunc());

    // case1 传参异常
    index_ = -1;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
    index_ = 1024;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
    // case2 传参正常
    index_ = 0;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_SetCodecName_02, TestSize.Level1)
{
    codecName_ = CODEC_AAC_NAME;
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_01, TestSize.Level1)
{
    // lack of correct key
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_02, TestSize.Level1)
{
    // correct key input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_TYPE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_03, TestSize.Level1)
{
    // correct key input with redundancy key input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_TYPE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_04, TestSize.Level1)
{
    // correct key input with wrong value type input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_TYPE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_05, TestSize.Level1)
{
    // correct key input with wrong value type input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, INVALID_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, INVALID_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_TYPE);
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_06, TestSize.Level1)
{
    // correct key input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_LATM_TYPE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_07, TestSize.Level1)
{
    // correct key input with redundancy key input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_LATM_TYPE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_08, TestSize.Level1)
{
    // correct key input with wrong value type input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_LATM_TYPE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_09, TestSize.Level1)
{
    // correct key input with wrong value type input
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, INVALID_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, INVALID_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_LATM_TYPE);
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Configure_10, TestSize.Level1)
{
    // empty format input
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Start_01, TestSize.Level1)
{
    // correct flow 1
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Start_02, TestSize.Level1)
{
    // correct flow 2
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Start_03, TestSize.Level1)
{
    // wrong flow 1
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Start_04, TestSize.Level1)
{
    // wrong flow 2
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Stop_01, TestSize.Level1)
{
    // correct flow 1
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Stop_02, TestSize.Level1)
{
    // correct flow 2
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_TYPE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Flush_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Flush());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Flush_02, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Flush());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Reset_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Reset_02, TestSize.Level1)
{
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_TYPE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Reset_03, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Release_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Release_02, TestSize.Level1)
{
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, DEFAULT_AAC_TYPE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_Release_03, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_SetParameter_01, TestSize.Level1)
{
    // 尚未实现
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    format_.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, DEFAULT_BITRATE);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE, adec_->SetParameter(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_GetOutputFormat_01, TestSize.Level1)
{
    // 尚未实现
    EXPECT_EQ(AVCS_ERR_OK, ProceAacFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->GetOutputFormat(format_));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_GetInputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    sleep(1);
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;
    index_ = -1;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = 1024;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = signal_->inIdxQueue_.front();
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_NE(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_QueueInputBuffer_01, TestSize.Level1)
{
    AVCodecBufferInfo info;
    AVCodecBufferFlag flag = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;

    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceAacFunc());
    sleep(1);
    // case0 传参异常
    index_ = -1;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY, adec_->QueueInputBuffer(index_, info, flag));
    // case1 info未赋值
    index_ = 1024;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY, adec_->QueueInputBuffer(index_, info, flag));

    // case2 EOS帧数据
    index_ = 0;
    info.presentationTimeUs = 0;
    info.size = -1;
    info.offset = 0;
    flag = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->QueueInputBuffer(index_, info, flag));
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_GetOutputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceAacFunc());
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;

    // case1 传参异常
    index_ = -1;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
    index_ = 1024;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderInnerUnitTest, audioDecoder_Aac_ReleaseOutputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceAacFunc());

    // case1 传参异常
    index_ = -1;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
    index_ = 1024;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
    // case2 传参正常
    index_ = 0;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
} // namespace Media
} // namespace OHOS