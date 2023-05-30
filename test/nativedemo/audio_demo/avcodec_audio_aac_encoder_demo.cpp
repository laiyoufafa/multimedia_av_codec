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

#include <iostream>
#include <unistd.h>
#include "securec.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "media_description.h"
#include "native_avformat.h"
#include "demo_log.h"
#include "native_avcodec_base.h"
#include "avcodec_codec_name.h"
#include "avcodec_audio_aac_encoder_demo.h"

using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::AudioAacDemo;
using namespace std;
namespace {
constexpr uint32_t CHANNEL_COUNT = 2;
constexpr uint32_t SAMPLE_RATE = 44100;
constexpr uint32_t BITS_RATE = 112000;
constexpr uint32_t BITS_PER_CODED_RATE = 16;
constexpr uint32_t FRAME_DURATION_US = 33000;
constexpr uint32_t CHANNEL_LAYOUT = 3;
constexpr int32_t SAMPLE_FORMAT = 8;
constexpr int32_t INPUT_FRAME_BYTES = 2 * 1024 * 4;

constexpr string_view INPUT_FILE_PATH = "/data/media/test_fltp.pcm";
constexpr string_view OUTPUT_FILE_PATH = "/data/media/encode2.aac";
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
    AEncSignal *signal = static_cast<AEncSignal *>(userData);
    cout << "OnInputBufferAvailable received, index:" << index << endl;
    unique_lock<mutex> lock(signal->inMutex_);
    signal->inQueue_.push(index);
    signal->inBufferQueue_.push(data);
    signal->inCond_.notify_all();
}

static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                    void *userData)
{
    (void)codec;
    AEncSignal *signal = static_cast<AEncSignal *>(userData);
    cout << "OnOutputBufferAvailable received, index:" << index << endl;
    unique_lock<mutex> lock(signal->outMutex_);
    signal->outQueue_.push(index);
    signal->outBufferQueue_.push(data);
    if (attr) {
        cout << "OnOutputBufferAvailable received, index:" << index << ", attr->size:" << attr->size
             << ", attr->flags:" << attr->flags << endl;
        signal->attrQueue_.push(*attr);
    } else {
        cout << "OnOutputBufferAvailable error, attr is nullptr!" << endl;
    }
    signal->outCond_.notify_all();
}

void AEncAacDemo::RunCase()
{
    std::cout << "RunCase enter" << std::endl;
    DEMO_CHECK_AND_RETURN_LOG(CreateEnc() == AVCS_ERR_OK, "Fatal: CreateEnc fail");

    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), SAMPLE_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), BITS_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(), BITS_PER_CODED_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_AUDIO_SAMPLE_FORMAT.data(), SAMPLE_FORMAT);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT.data(), CHANNEL_LAYOUT);

    DEMO_CHECK_AND_RETURN_LOG(Configure(format) == AVCS_ERR_OK, "Fatal: Configure fail");

    DEMO_CHECK_AND_RETURN_LOG(Start() == AVCS_ERR_OK, "Fatal: Start fail");

    while (isRunning_.load()) {
        sleep(1);
    }

    DEMO_CHECK_AND_RETURN_LOG(Stop() == AVCS_ERR_OK, "Fatal: Stop fail");
    DEMO_CHECK_AND_RETURN_LOG(Release() == AVCS_ERR_OK, "Fatal: Release fail");
}

AEncAacDemo::AEncAacDemo()
    : isRunning_(false),
      audioEnc_(nullptr),
      signal_(nullptr),
      frameCount_(0)
{
    inputFile_ = std::make_unique<std::ifstream>(INPUT_FILE_PATH, std::ios::binary);
}

AEncAacDemo::~AEncAacDemo()
{
    OH_AudioEncoder_Destroy(audioEnc_);
    if (signal_) {
        delete signal_;
        signal_ = nullptr;
    }
}

int32_t AEncAacDemo::CreateEnc()
{
    audioEnc_ = OH_AudioEncoder_CreateByName((AVCodecCodecName::AUDIO_ENCODER_AAC_NAME).data());
    DEMO_CHECK_AND_RETURN_RET_LOG(audioEnc_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: CreateByName fail");

    signal_ = new AEncSignal();
    DEMO_CHECK_AND_RETURN_RET_LOG(signal_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    cb_ = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
    int32_t ret = OH_AudioEncoder_SetCallback(audioEnc_, cb_, signal_);
    DEMO_CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_UNKNOWN, "Fatal: SetCallback fail");

    return AVCS_ERR_OK;
}

int32_t AEncAacDemo::Configure(OH_AVFormat *format)
{
    return OH_AudioEncoder_Configure(audioEnc_, format);
}

int32_t AEncAacDemo::Start()
{
    isRunning_.store(true);

    inputLoop_ = make_unique<thread>(&AEncAacDemo::InputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(inputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    outputLoop_ = make_unique<thread>(&AEncAacDemo::OutputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(outputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    return OH_AudioEncoder_Start(audioEnc_);
}

int32_t AEncAacDemo::Stop()
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

    return OH_AudioEncoder_Stop(audioEnc_);
}

int32_t AEncAacDemo::Flush()
{
    return OH_AudioEncoder_Flush(audioEnc_);
}

int32_t AEncAacDemo::Reset()
{
    return OH_AudioEncoder_Reset(audioEnc_);
}

int32_t AEncAacDemo::Release()
{
    return OH_AudioEncoder_Destroy(audioEnc_);
}

void AEncAacDemo::HandleEOS(const uint32_t &index)
{
    OH_AVCodecBufferAttr info;
    info.size = 0;
    info.offset = 0;
    info.pts = 0;
    info.flags = AVCODEC_BUFFER_FLAGS_EOS;
    OH_AudioEncoder_PushInputData(audioEnc_, index, info);
    std::cout << "end buffer\n";
    signal_->inQueue_.pop();
    signal_->inBufferQueue_.pop();
}

void AEncAacDemo::InputFunc()
{
    DEMO_CHECK_AND_RETURN_LOG(inputFile_ != nullptr && inputFile_->is_open(), "Fatal: open file fail");
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
        DEMO_CHECK_AND_BREAK_LOG(buffer != nullptr, "Fatal: GetInputBuffer fail");

        if (!inputFile_->eof()) {
            inputFile_->read((char *)OH_AVMemory_GetAddr(buffer), INPUT_FRAME_BYTES);
        } else {
            HandleEOS(index);
            break;
        }
        DEMO_CHECK_AND_BREAK_LOG(buffer != nullptr, "Fatal: GetInputBuffer fail");
        OH_AVCodecBufferAttr info;
        info.size = INPUT_FRAME_BYTES;
        info.offset = 0;

        int32_t ret = AVCS_ERR_OK;
        if (isFirstFrame_) {
            info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
            ret = OH_AudioEncoder_PushInputData(audioEnc_, index, info);
            isFirstFrame_ = false;
        } else {
            info.flags = AVCODEC_BUFFER_FLAGS_NONE;
            ret = OH_AudioEncoder_PushInputData(audioEnc_, index, info);
        }
        timeStamp_ += FRAME_DURATION_US;
        signal_->inQueue_.pop();
        signal_->inBufferQueue_.pop();
        frameCount_++;
        if (ret != AVCS_ERR_OK) {
            cout << "Fatal error, exit" << endl;
            break;
        }
    }
    inputFile_->close();
}

void AEncAacDemo::OutputFunc()
{
    std::ofstream outputFile;
    outputFile.open(OUTPUT_FILE_PATH.data(), std::ios::out | std::ios::binary);
    if (!outputFile.is_open()) {
        std::cout << "open " << OUTPUT_FILE_PATH << " failed!" << std::endl;
    }

    while (true) {
        if (!isRunning_.load()) {
            cout << "stop, exit" << endl;
            break;
        }

        unique_lock<mutex> lock(signal_->outMutex_);
        cout << "lock, before" << endl;
        signal_->outCond_.wait(lock, [this]() { return (signal_->outQueue_.size() > 0 || !isRunning_.load()); });
        cout << "lock, after" << endl;

        if (!isRunning_.load()) {
            cout << "wait to stop, exit" << endl;
            break;
        }

        uint32_t index = signal_->outQueue_.front();

        OH_AVCodecBufferAttr attr = signal_->attrQueue_.front();
        OH_AVMemory *data = signal_->outBufferQueue_.front();
        if (data != nullptr) {
            cout << "OutputFunc write file,buffer index" << index << ", data size = :" << attr.size << endl;
            outputFile.write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), attr.size);
        }
        cout << "attr.flags: " << attr.flags << endl;
        if (attr.flags == AVCODEC_BUFFER_FLAGS_EOS || attr.size == 0) {
            cout << "encode eos" << endl;
            isRunning_.store(false);
        }

        signal_->outBufferQueue_.pop();
        signal_->attrQueue_.pop();
        signal_->outQueue_.pop();
        if (OH_AudioEncoder_FreeOutputData(audioEnc_, index) != AV_ERR_OK) {
            cout << "Fatal: FreeOutputData fail" << endl;
            break;
        }
        if (attr.flags == AVCODEC_BUFFER_FLAGS_EOS) {
            cout << "decode eos" << endl;
            isRunning_.store(false);
        }
    }
    outputFile.close();
}
