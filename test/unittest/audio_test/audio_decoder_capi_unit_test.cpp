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
#include <iostream>
#include <unistd.h>
#include <atomic>
#include <fstream>
#include <queue>
#include <string>
#include <thread>
#include "avcodec_codec_name.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "avcodec_mime_type.h"
#include "media_description.h"
#include "native_avcodec_base.h"
#include "native_avformat.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "native_avcodec_audiodecoder.h"
#include "securec.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::MediaAVCodec;

namespace {
const string CODEC_MP3_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_MP3_NAME);
const string CODEC_FLAC_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_FLAC_NAME);
const string CODEC_AAC_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_AAC_NAME);
const string CODEC_VORBIS_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_VORBIS_NAME);
constexpr uint32_t MAX_CHANNEL_COUNT = 2;
constexpr uint32_t DEFAULT_SAMPLE_RATE = 44100;
constexpr uint32_t DEFAULT_MP3_BITRATE = 60000;
constexpr uint32_t DEFAULT_FLAC_BITRATE = 261000;
constexpr uint32_t DEFAULT_AAC_BITRATE = 199000;
constexpr uint32_t DEFAULT_VORBIS_BITRATE = 320000;
constexpr uint32_t DEFAULT_AAC_TYPE = 1;
constexpr string_view INPUT_AAC_FILE_PATH = "/data/test/media/aac_2c_44100hz_199k.dat";
constexpr string_view OUTPUT_AAC_PCM_FILE_PATH = "/data/test/media/aac_2c_44100hz_199k.pcm";
constexpr string_view INPUT_FLAC_FILE_PATH = "/data/test/media/flac_2c_44100hz_261k.dat";
constexpr string_view OUTPUT_FLAC_PCM_FILE_PATH = "/data/test/media/flac_2c_44100hz_261k.pcm";
constexpr string_view INPUT_MP3_FILE_PATH = "/data/test/media/mp3_2c_44100hz_60k.dat";
constexpr string_view OUTPUT_MP3_PCM_FILE_PATH = "/data/test/media/mp3_2c_44100hz_60k.pcm";
constexpr string_view INPUT_VORBIS_FILE_PATH = "/data/test/media/vorbis_2c_44100hz_320k.dat";
constexpr string_view OUTPUT_VORBIS_PCM_FILE_PATH = "/data/test/media/vorbis_2c_44100hz_320k.pcm";
} // namespace

namespace OHOS {
namespace MediaAVCodec {
class ADecSignal {
public:
    std::mutex inMutex_;
    std::mutex outMutex_;
    std::mutex startMutex_;
    std::condition_variable inCond_;
    std::condition_variable outCond_;
    std::condition_variable startCond_;
    std::queue<uint32_t> inQueue_;
    std::queue<uint32_t> outQueue_;
    std::queue<OH_AVMemory *> inBufferQueue_;
    std::queue<OH_AVMemory *> outBufferQueue_;
    std::queue<OH_AVCodecBufferAttr> attrQueue_;
};

class BufferCallback : public AVCodecCallback {
public:
    explicit BufferCallback(ADecSignal *userData) : userData_(userData) {}
    virtual ~BufferCallback() = default;
    ADecSignal *userData_;
    void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    void OnOutputFormatChanged(const Format &format) override;
    void OnInputBufferAvailable(uint32_t index) override;
    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
};

static void OnError(OH_AVCodec *codec, int32_t errorCode, void *userData)
{
    (void)codec;
    (void)errorCode;
    (void)userData;
    cout << "Error received, errorCode:" << errorCode << endl;
}

static void OnOutputFormatChanged(OH_AVCodec *codec, OH_AVFormat *format, void *userData)
{
    (void)codec;
    (void)format;
    (void)userData;
    cout << "OnOutputFormatChanged received" << endl;
}

static void OnInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, void *userData)
{
    (void)codec;
    ADecSignal *signal = static_cast<ADecSignal *>(userData);
    unique_lock<mutex> lock(signal->inMutex_);
    signal->inQueue_.push(index);
    signal->inBufferQueue_.push(data);
    signal->inCond_.notify_all();
}

static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                    void *userData)
{
    (void)codec;
    ADecSignal *signal = static_cast<ADecSignal *>(userData);
    unique_lock<mutex> lock(signal->outMutex_);
    signal->outQueue_.push(index);
    signal->outBufferQueue_.push(data);
    if (attr) {
        signal->attrQueue_.push(*attr);
    } else {
        cout << "OnOutputBufferAvailable error, attr is nullptr!" << endl;
    }
    signal->outCond_.notify_all();
}

class AudioCodeCapiDecoderUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int32_t InitFile(const string &codecName);
    void InputFunc();
    void OutputFunc();
    int32_t CreateCodecFunc(const string &codecName);
    void HandleInputEOS(const uint32_t index);
    int32_t HandleNormalInput(const uint32_t &index, const int64_t pts, const size_t size);
    int32_t Configure(const string &codecName);
    int32_t Start();
    int32_t Stop();
    void Release();

protected:
    std::atomic<bool> isRunning_ = false;
    std::unique_ptr<std::thread> inputLoop_;
    std::unique_ptr<std::thread> outputLoop_;
    struct OH_AVCodecAsyncCallback cb_;
    ADecSignal *signal_ = nullptr;
    OH_AVCodec *audioDec_ = nullptr;;
    OH_AVFormat *format_ = nullptr;;
    bool isFirstFrame_ = true;
    std::ifstream inputFile_;
    std::ofstream pcmOutputFile_;
};

void AudioCodeCapiDecoderUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void AudioCodeCapiDecoderUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void AudioCodeCapiDecoderUnitTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}

void AudioCodeCapiDecoderUnitTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;

    if (signal_) {
        delete signal_;
        signal_ = nullptr;
    }
    if (inputFile_.is_open()) {
        inputFile_.close();
    }
    if (pcmOutputFile_.is_open()) {
        pcmOutputFile_.close();
    }
}

void AudioCodeCapiDecoderUnitTest::Release()
{
    Stop();
    OH_AudioDecoder_Destroy(audioDec_);
}

void AudioCodeCapiDecoderUnitTest::HandleInputEOS(const uint32_t index)
{
    OH_AVCodecBufferAttr info;
    info.size = 0;
    info.offset = 0;
    info.pts = 0;
    info.flags = AVCODEC_BUFFER_FLAGS_EOS;
    OH_AudioDecoder_PushInputData(audioDec_, index, info);
    signal_->inBufferQueue_.pop();
    signal_->inQueue_.pop();
}

int32_t AudioCodeCapiDecoderUnitTest::HandleNormalInput(const uint32_t &index, const int64_t pts, const size_t size)
{
    OH_AVCodecBufferAttr info;
    info.size = size;
    info.offset = 0;
    info.pts = pts;

    int32_t ret = OH_AVErrCode::AV_ERR_OK;
    if (isFirstFrame_) {
        info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
        ret = OH_AudioDecoder_PushInputData(audioDec_, index, info);
        EXPECT_EQ(AV_ERR_OK, ret);
        isFirstFrame_ = false;
    } else {
        info.flags = AVCODEC_BUFFER_FLAGS_NONE;
        ret = OH_AudioDecoder_PushInputData(audioDec_, index, info);
        EXPECT_EQ(AV_ERR_OK, ret);
    }
    signal_->inQueue_.pop();
    signal_->inBufferQueue_.pop();
    return ret;
}

void AudioCodeCapiDecoderUnitTest::InputFunc()
{
    int64_t size;
    int64_t pts;

    while (true) {
        if (!isRunning_.load()) {
            break;
        }
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.wait(lock, [this]() { return (signal_->inQueue_.size() > 0 || !isRunning_.load()); });

        if (!isRunning_.load()) {
            break;
        }

        uint32_t index = signal_->inQueue_.front();
        auto buffer = signal_->inBufferQueue_.front();
        if (buffer == nullptr) {
            cout << "Fatal: GetInputBuffer fail" << endl;
            break;
        }
        inputFile_.read(reinterpret_cast<char *>(&size), sizeof(size));
        if (inputFile_.eof() || inputFile_.gcount() == 0) {
            HandleInputEOS(index);
            cout << "end buffer\n";
            break;
        }
        if (inputFile_.gcount() != sizeof(size)) {
            cout << "Fatal: read size fail" << endl;
            break;
        }
        inputFile_.read(reinterpret_cast<char *>(&pts), sizeof(pts));
        if (inputFile_.gcount() != sizeof(pts)) {
            cout << "Fatal: read size fail" << endl;
            break;
        }
        inputFile_.read((char *)OH_AVMemory_GetAddr(buffer), size);
        if (inputFile_.gcount() != size) {
            cout << "Fatal: read buffer fail" << endl;
            break;
        }

        int32_t ret = HandleNormalInput(index, pts, size);
        if (ret != AV_ERR_OK) {
            cout << "Fatal error, exit" << endl;
            break;
        }
    }
    inputFile_.close();
}

void AudioCodeCapiDecoderUnitTest::OutputFunc()
{
    if (!pcmOutputFile_.is_open()) {
        std::cout << "open " << OUTPUT_MP3_PCM_FILE_PATH << " failed!" << std::endl;
    }
    while (true) {
        if (!isRunning_.load()) {
            cout << "stop, exit" << endl;
            break;
        }

        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.wait(lock, [this]() { return (signal_->outQueue_.size() > 0 || !isRunning_.load()); });

        if (!isRunning_.load()) {
            cout << "wait to stop, exit" << endl;
            break;
        }

        uint32_t index = signal_->outQueue_.front();
        OH_AVCodecBufferAttr attr = signal_->attrQueue_.front();
        OH_AVMemory *data = signal_->outBufferQueue_.front();
        if (data != nullptr) {
            pcmOutputFile_.write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), attr.size);
        }

        if (attr.flags == AVCODEC_BUFFER_FLAGS_EOS) {
            cout << "decode eos" << endl;
            isRunning_.store(false);
        }
        signal_->outBufferQueue_.pop();
        signal_->attrQueue_.pop();
        signal_->outQueue_.pop();
        EXPECT_EQ(AV_ERR_OK, OH_AudioDecoder_FreeOutputData(audioDec_, index));
    }

    pcmOutputFile_.close();
    signal_->startCond_.notify_all();
}

int32_t AudioCodeCapiDecoderUnitTest::Start()
{
    isRunning_.store(true);
    inputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::InputFunc, this);
    if (inputLoop_ == nullptr) {
        cout << "Fatal: No memory" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }
    
    outputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::OutputFunc, this);
    if (outputLoop_ == nullptr) {
        cout << "Fatal: No memory" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }

    return OH_AudioDecoder_Start(audioDec_);
}

int32_t AudioCodeCapiDecoderUnitTest::Stop()
{
    isRunning_.store(false);
    if (!signal_) {
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }
    signal_->startCond_.notify_all();
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        {
            unique_lock<mutex> lock(signal_->inMutex_);
            signal_->inCond_.notify_all();
        }
        inputLoop_->join();
    }

    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        {
            unique_lock<mutex> lock(signal_->outMutex_);
            signal_->outCond_.notify_all();
        }
        outputLoop_->join();
    }
    return OH_AudioDecoder_Stop(audioDec_);
}

int32_t AudioCodeCapiDecoderUnitTest::InitFile(const string &codecName)
{
    if (codecName.compare(CODEC_MP3_NAME) == 0) {
        inputFile_.open(INPUT_MP3_FILE_PATH.data(), std::ios::binary);
        pcmOutputFile_.open(OUTPUT_MP3_PCM_FILE_PATH.data(), std::ios::out | std::ios::binary);
    } else if (codecName.compare(CODEC_FLAC_NAME) == 0) {
        inputFile_.open(INPUT_FLAC_FILE_PATH.data(), std::ios::binary);
        pcmOutputFile_.open(OUTPUT_FLAC_PCM_FILE_PATH.data(), std::ios::out | std::ios::binary);
    } else if (codecName.compare(CODEC_AAC_NAME) == 0) {
        inputFile_.open(INPUT_AAC_FILE_PATH.data(), std::ios::binary);
        pcmOutputFile_.open(OUTPUT_AAC_PCM_FILE_PATH.data(), std::ios::out | std::ios::binary);
    } else if (codecName.compare(CODEC_VORBIS_NAME) == 0) {
        inputFile_.open(INPUT_VORBIS_FILE_PATH.data(), std::ios::binary);
        pcmOutputFile_.open(OUTPUT_VORBIS_PCM_FILE_PATH.data(), std::ios::out | std::ios::binary);
    } else {
        cout << "Fatal: audio format type not support" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }

    if (!inputFile_.is_open()) {
        cout << "Fatal: open input file failed" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }
    if (!pcmOutputFile_.is_open()) {
        cout << "Fatal: open output file failed" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }
    return OH_AVErrCode::AV_ERR_OK;
}

int32_t AudioCodeCapiDecoderUnitTest::CreateCodecFunc(const string &codecName)
{
    if (codecName.compare(CODEC_MP3_NAME) == 0) {
        audioDec_ = OH_AudioDecoder_CreateByName((AVCodecCodecName::AUDIO_DECODER_MP3_NAME).data());
    } else if (codecName.compare(CODEC_FLAC_NAME) == 0) {
        audioDec_ = OH_AudioDecoder_CreateByName((AVCodecCodecName::AUDIO_DECODER_FLAC_NAME).data());
    } else if (codecName.compare(CODEC_AAC_NAME) == 0) {
        audioDec_ = OH_AudioDecoder_CreateByName((AVCodecCodecName::AUDIO_DECODER_AAC_NAME).data());
    } else if (codecName.compare(CODEC_VORBIS_NAME) == 0) {
        audioDec_ = OH_AudioDecoder_CreateByName((AVCodecCodecName::AUDIO_DECODER_VORBIS_NAME).data());
    } else {
        cout << "audio name not support" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }

    if (audioDec_ == nullptr) {
        cout << "Fatal: CreateByName fail" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }

    signal_ = new ADecSignal();
    if (signal_ == nullptr) {
        cout << "Fatal: create signal fail" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }
    cb_ = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
    int32_t ret = OH_AudioDecoder_SetCallback(audioDec_, cb_, signal_);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        cout << "Fatal: SetCallback fail" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }

    return OH_AVErrCode::AV_ERR_OK;
}

int32_t AudioCodeCapiDecoderUnitTest::Configure(const string &codecName)
{
    format_ = OH_AVFormat_Create();
    if (format_ == nullptr) {
        cout << "Fatal: create format failed" << endl;
        return OH_AVErrCode::AV_ERR_UNKNOWN;
    }

    uint32_t bitRate = DEFAULT_MP3_BITRATE;
    OH_AVFormat_SetIntValue(format_, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format_, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    if (codecName.compare(CODEC_FLAC_NAME) == 0) {
        bitRate = DEFAULT_FLAC_BITRATE;
    }
    if (codecName.compare(CODEC_AAC_NAME) == 0) {
        bitRate = DEFAULT_AAC_BITRATE;
        OH_AVFormat_SetIntValue(format_, MediaDescriptionKey::MD_KEY_AAC_IS_ADTS.data(), DEFAULT_AAC_TYPE);
    }
    if (codecName.compare(CODEC_VORBIS_NAME) == 0) {
        bitRate = DEFAULT_VORBIS_BITRATE;
        if (!inputFile_.is_open()) {
            cout << "Fatal: open input file failed" << endl;
            return OH_AVErrCode::AV_ERR_UNKNOWN;
        }
        int64_t extradataSize = 0;
        inputFile_.read(reinterpret_cast<char*>(&extradataSize), sizeof(int64_t));
        if (inputFile_.gcount() != sizeof(int64_t) || extradataSize < 0) {
            cout << "Fatal: read extradataSize bytes error" << endl;
            return OH_AVErrCode::AV_ERR_UNKNOWN;
        }
        char buffer[extradataSize];
        inputFile_.read(buffer, extradataSize);
        if (inputFile_.gcount() != extradataSize) {
            cout << "Fatal: read extradata bytes error" << endl;
            return OH_AVErrCode::AV_ERR_UNKNOWN;
        }
        OH_AVFormat_SetBuffer(format_, MediaDescriptionKey::MD_KEY_CODEC_CONFIG.data(), (uint8_t*)buffer,
                              extradataSize);
    }
    OH_AVFormat_SetLongValue(format_, MediaDescriptionKey::MD_KEY_BITRATE.data(), bitRate);
    
    return OH_AudioDecoder_Configure(audioDec_, format_);
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_CreateByMime_01, TestSize.Level1)
{
    audioDec_ = OH_AudioDecoder_CreateByMime(AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_MPEG.data());
    EXPECT_NE(nullptr, audioDec_);
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_CreateByName_01, TestSize.Level1)
{
    audioDec_ = OH_AudioDecoder_CreateByName((AVCodecCodecName::AUDIO_DECODER_MP3_NAME).data());
    EXPECT_NE(nullptr, audioDec_);
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Configure_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_SetParameter_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_SetParameter_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Start_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Start_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Stop_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    sleep(1);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Flush_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Reset_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Reset_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Reset_03, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Destroy_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Destroy_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_GetOutputFormat_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));

    EXPECT_NE(nullptr, OH_AudioDecoder_GetOutputDescription(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_IsValid_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    bool isValid = false;
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_IsValid(audioDec_, &isValid));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_Prepare_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Prepare(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_PushInputData_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());

    // case0 传参异常
    uint32_t index = 0;
    OH_AVCodecBufferAttr attr;
    attr.pts = 0;
    attr.size = -1;
    attr.offset = 0;
    attr.flags = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_PushInputData(audioDec_, index, attr));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Mp3_ReleaseOutputBuffer_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_MP3_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_MP3_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());

    // case0 传参异常
    uint32_t index = 1024;
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_FreeOutputData(audioDec_, index));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_CreateByMime_01, TestSize.Level1)
{
    audioDec_ = OH_AudioDecoder_CreateByMime(AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC.data());
    EXPECT_NE(nullptr, audioDec_);
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_CreateByName_01, TestSize.Level1)
{
    audioDec_ = OH_AudioDecoder_CreateByName((AVCodecCodecName::AUDIO_DECODER_FLAC_NAME).data());
    EXPECT_NE(nullptr, audioDec_);
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Configure_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_SetParameter_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_SetParameter_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Start_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Start_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Stop_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    sleep(1);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Flush_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Reset_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Reset_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Reset_03, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Destroy_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Destroy_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_GetOutputFormat_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));

    EXPECT_NE(nullptr, OH_AudioDecoder_GetOutputDescription(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_IsValid_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    bool isValid = false;
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_IsValid(audioDec_, &isValid));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_Prepare_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Prepare(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_PushInputData_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());

    // case0 传参异常
    uint32_t index = 0;
    OH_AVCodecBufferAttr attr;
    attr.pts = 0;
    attr.size = -1;
    attr.offset = 0;
    attr.flags = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_PushInputData(audioDec_, index, attr));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Flac_ReleaseOutputBuffer_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_FLAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_FLAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());

    // case0 传参异常
    uint32_t index = 1024;
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_FreeOutputData(audioDec_, index));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_CreateByMime_01, TestSize.Level1)
{
    audioDec_ = OH_AudioDecoder_CreateByMime(AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_AAC.data());
    EXPECT_NE(nullptr, audioDec_);
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_CreateByName_01, TestSize.Level1)
{
    audioDec_ = OH_AudioDecoder_CreateByName((AVCodecCodecName::AUDIO_DECODER_AAC_NAME).data());
    EXPECT_NE(nullptr, audioDec_);
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Configure_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_SetParameter_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_SetParameter_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Start_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Start_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Stop_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    sleep(1);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Flush_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Reset_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Reset_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Reset_03, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Destroy_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Destroy_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_GetOutputFormat_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));

    EXPECT_NE(nullptr, OH_AudioDecoder_GetOutputDescription(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_IsValid_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    bool isValid = false;
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_IsValid(audioDec_, &isValid));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_Prepare_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Prepare(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_PushInputData_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());

    // case0 传参异常
    uint32_t index = 0;
    OH_AVCodecBufferAttr attr;
    attr.pts = 0;
    attr.size = -1;
    attr.offset = 0;
    attr.flags = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_PushInputData(audioDec_, index, attr));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Aac_ReleaseOutputBuffer_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_AAC_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_AAC_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());

    // case0 传参异常
    uint32_t index = 1024;
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_FreeOutputData(audioDec_, index));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_CreateByMime_01, TestSize.Level1)
{
    audioDec_ = OH_AudioDecoder_CreateByMime(AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_VORBIS.data());
    EXPECT_NE(nullptr, audioDec_);
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_CreateByName_01, TestSize.Level1)
{
    audioDec_ = OH_AudioDecoder_CreateByName((AVCodecCodecName::AUDIO_DECODER_VORBIS_NAME).data());
    EXPECT_NE(nullptr, audioDec_);
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Configure_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_SetParameter_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_SetParameter_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Start_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Start_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Stop_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    sleep(1);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Flush_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Reset_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Reset_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Reset_03, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Destroy_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());
    {
        unique_lock<mutex> lock(signal_->startMutex_);
        signal_->startCond_.wait(lock, [this]() { return (!(isRunning_.load())); });
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Stop());
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Destroy_02, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_GetOutputFormat_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));

    EXPECT_NE(nullptr, OH_AudioDecoder_GetOutputDescription(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_IsValid_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    bool isValid = false;
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_IsValid(audioDec_, &isValid));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_Prepare_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Prepare(audioDec_));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_PushInputData_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());

    // case0 传参异常
    uint32_t index = 0;
    OH_AVCodecBufferAttr attr;
    attr.pts = 0;
    attr.size = -1;
    attr.offset = 0;
    attr.flags = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_PushInputData(audioDec_, index, attr));
    Release();
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Vorbis_ReleaseOutputBuffer_01, TestSize.Level1)
{
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, InitFile(CODEC_VORBIS_NAME));
    ASSERT_EQ(OH_AVErrCode::AV_ERR_OK, CreateCodecFunc(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Configure(CODEC_VORBIS_NAME));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, Start());

    // case0 传参异常
    uint32_t index = 1024;
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_FreeOutputData(audioDec_, index));
    Release();
}
} // namespace MediaAVCodec
} // namespace OHOS