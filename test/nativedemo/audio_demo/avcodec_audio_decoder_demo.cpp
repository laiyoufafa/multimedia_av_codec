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

#include "avcodec_audio_decoder_demo.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "demo_log.h"
#include "native_avcodec_base.h"
#include "securec.h"
#include <iostream>
#include <unistd.h>

using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::AudioDemo;
using namespace std;
namespace {
constexpr uint32_t CHANNEL_COUNT = 2;
constexpr uint32_t SAMPLE_RATE = 44100;
constexpr uint32_t BITS_RATE = 169000;
constexpr uint32_t BITS_PER_CODED_RATE = 4;
constexpr uint32_t FRAME_DURATION_US = 33000;
constexpr uint32_t DEFAULT_FRAME_COUNT = 1;
constexpr string_view inputFilePath = "/data/test441_2_noid3.mp3";
// constexpr string_view outputFilePath = "/data/audio_test.pcm";
} // namespace

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
    ADecSignal *signal_ = static_cast<ADecSignal *>(userData);
    cout << "OnInputBufferAvailable received, index:" << index << endl;
    unique_lock<mutex> lock(signal_->inMutex_);
    signal_->inQueue_.push(index);
    signal_->inBufferQueue_.push(data);
    signal_->inCond_.notify_all();
}

static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                    void *userData)
{
    (void)codec;
    ADecSignal *signal_ = static_cast<ADecSignal *>(userData);
    cout << "OnOutputBufferAvailable received, index:" << index << endl;
    unique_lock<mutex> lock(signal_->outMutex_);
    signal_->outQueue_.push(index);
    signal_->outBufferQueue_.push(data);
    signal_->attrQueue_.push(attr);
    signal_->outCond_.notify_all();
}

void ADecDemo::RunCase()
{
    DEMO_CHECK_AND_RETURN_LOG(CreateDec() == AVCS_ERR_OK, "Fatal: CreateDec fail");

    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUD_CHANNEL_COUNT, CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUD_SAMPLE_RATE, SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, "bits_per_coded-rate", BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, OH_MD_KEY_BITRATE, BITS_RATE);
    DEMO_CHECK_AND_RETURN_LOG(Configure(format) == AVCS_ERR_OK, "Fatal: Configure fail");

    DEMO_CHECK_AND_RETURN_LOG(Start() == AVCS_ERR_OK, "Fatal: Start fail");
    sleep(3); // start run 3s

    DEMO_CHECK_AND_RETURN_LOG(Stop() == AVCS_ERR_OK, "Fatal: Stop fail");
    std::cout << "end stop!\n";
    DEMO_CHECK_AND_RETURN_LOG(Release() == AVCS_ERR_OK, "Fatal: Release fail");
}

ADecDemo::ADecDemo()
{
    if (avformat_open_input(&fmpt_ctx, inputFilePath.data(), NULL, NULL) < 0) {
        std::cout << "open file failed"
                  << "\n";
        exit(1);
    }
    if (avformat_find_stream_info(fmpt_ctx, NULL) < 0) {
        std::cout << "get file stream failed"
                  << "\n";
        exit(1);
    }
    frame = av_frame_alloc();
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
}

ADecDemo::~ADecDemo()
{
    OH_AudioDecoder_Destroy(audioDec_);
    if (signal_) {
        delete signal_;
        signal_ = nullptr;
    }
}

int32_t ADecDemo::CreateDec()
{

    audioDec_ = OH_AudioDecoder_CreateByName("OH.Media.Codec.MP3.FFMPEGMp3");
    DEMO_CHECK_AND_RETURN_RET_LOG(audioDec_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: CreateByName fail");

    signal_ = new ADecSignal();
    DEMO_CHECK_AND_RETURN_RET_LOG(signal_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    cb_ = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
    int32_t ret = OH_AudioDecoder_SetCallback(audioDec_, cb_, signal_);
    DEMO_CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_UNKNOWN, "Fatal: SetCallback fail");

    return AVCS_ERR_OK;
}

int32_t ADecDemo::Configure(OH_AVFormat *format)
{
    return OH_AudioDecoder_Configure(audioDec_, format);
}

int32_t ADecDemo::Start()
{
    isRunning_.store(true);

    testFile_ = std::make_unique<std::ifstream>();
    DEMO_CHECK_AND_RETURN_RET_LOG(testFile_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");
    testFile_->open("/data/media/audio.mp3", std::ios::in | std::ios::binary);

    inputLoop_ = make_unique<thread>(&ADecDemo::InputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(inputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    outputLoop_ = make_unique<thread>(&ADecDemo::OutputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(outputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    return OH_AudioDecoder_Start(audioDec_);
}

int32_t ADecDemo::Stop()
{
    isRunning_.store(false);
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.notify_all();
        lock.unlock();
        inputLoop_->join();
    }

    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.notify_all();
        lock.unlock();
        outputLoop_->join();
    }
    std::cout << "start stop!\n";
    return OH_AudioDecoder_Stop(audioDec_);
}

int32_t ADecDemo::Flush()
{
    return OH_AudioDecoder_Flush(audioDec_);
}

int32_t ADecDemo::Reset()
{
    return OH_AudioDecoder_Reset(audioDec_);
}

int32_t ADecDemo::Release()
{
    return OH_AudioDecoder_Destroy(audioDec_);
}

void ADecDemo::InputFunc()
{
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

        int32_t ret = av_read_frame(fmpt_ctx, &pkt);

        if (ret < 0) {
            OH_AVCodecBufferAttr info;
            info.size = 0;
            info.offset = 0;
            info.pts = 0;
            info.flags = AVCODEC_BUFFER_FLAGS_EOS;
            av_packet_unref(&pkt);
            OH_AudioDecoder_PushInputData(audioDec_, index, info);
            signal_->inBufferQueue_.pop();
            signal_->inQueue_.pop();
            std::cout << "end buffer\n";
            break;
        }
        std::cout << "start read frame: size:" << pkt.size << ",pts:" << pkt.pts << "\n";
        DEMO_CHECK_AND_BREAK_LOG(buffer != nullptr, "Fatal: GetInputBuffer fail");
        OH_AVCodecBufferAttr info;
        info.size = pkt.size;
        info.offset = 0;
        info.pts = pkt.pts;
        memcpy_s(OH_AVMemory_GetAddr(buffer), pkt.size, pkt.data, pkt.size);

        ret = AVCS_ERR_OK;
        if (isFirstFrame_) {
            info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
            ret = OH_AudioDecoder_PushInputData(audioDec_, index, info);
            isFirstFrame_ = false;
        } else {
            info.flags = AVCODEC_BUFFER_FLAGS_NONE;
            ret = OH_AudioDecoder_PushInputData(audioDec_, index, info);
        }

        // free(fileBuffer);
        timeStamp_ += FRAME_DURATION_US;
        signal_->inQueue_.pop();
        signal_->inBufferQueue_.pop();

        frameCount_++;
        if (frameCount_ == DEFAULT_FRAME_COUNT) {
            cout << "Finish decode, exit" << endl;
            break;
        }

        if (ret != AVCS_ERR_OK) {
            cout << "Fatal error, exit" << endl;
            break;
        }
    }
}

void ADecDemo::OutputFunc()
{
    while (true) {
        if (!isRunning_.load()) {
            break;
        }

        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.wait(lock, [this]() { return (signal_->outQueue_.size() > 0 || !isRunning_.load()); });

        if (!isRunning_.load()) {
            break;
        }

        uint32_t index = signal_->outQueue_.front();
        if (OH_AudioDecoder_FreeOutputData(audioDec_, index) != AV_ERR_OK) {
            cout << "Fatal: ReleaseOutputBuffer fail" << endl;
            break;
        }
        OH_AVCodecBufferAttr *attr = signal_->attrQueue_.front();
        if (attr->flags == AVCODEC_BUFFER_FLAGS_EOS) {
            isRunning_.store(false);
        }
        signal_->outBufferQueue_.pop();
        signal_->attrQueue_.pop();
        signal_->outQueue_.pop();
    }
}