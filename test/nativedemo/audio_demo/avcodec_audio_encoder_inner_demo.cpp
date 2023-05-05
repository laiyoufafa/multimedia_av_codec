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

#include "avcodec_audio_encoder_inner_demo.h"
#include <iostream>
#include <unistd.h>
#include "securec.h"
#include "demo_log.h"
#include "avcodec_errors.h"

using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::InnerAudioDemo;
using namespace std;
namespace {
    constexpr uint32_t CHANNEL_COUNT = 2;
    constexpr uint32_t SAMPLE_RATE = 44100;
    constexpr uint32_t BITS_RATE = 169000;
    constexpr uint32_t BITS_PER_CODED_RATE = 4;
    constexpr uint32_t FRAME_DURATION_US = 33000;
    constexpr uint32_t DEFAULT_FRAME_COUNT = 1;
}

void ADecInnerDemo::RunCase()
{
    DEMO_CHECK_AND_RETURN_LOG(CreateDec() == AVCS_ERR_OK, "Fatal: CreateDec fail");

    Format format;
    format.PutIntValue("channel-count", CHANNEL_COUNT);
    format.PutIntValue("sample-rate", SAMPLE_RATE);
    format.PutLongValue("bits-rate", BITS_RATE);
    format.PutIntValue("bits_per_coded-rate", BITS_PER_CODED_RATE);
    DEMO_CHECK_AND_RETURN_LOG(Configure(format) == AVCS_ERR_OK, "Fatal: Configure fail");

    DEMO_CHECK_AND_RETURN_LOG(Start() == AVCS_ERR_OK, "Fatal: Start fail");
    sleep(3); // start run 3s
    DEMO_CHECK_AND_RETURN_LOG(Stop() == AVCS_ERR_OK, "Fatal: Stop fail");
    DEMO_CHECK_AND_RETURN_LOG(Release() == AVCS_ERR_OK, "Fatal: Release fail");
}

int32_t ADecInnerDemo::CreateDec()
{
    audioDec_ = AudioDecoderFactory::CreateByName("OH.Media.Codec.MP3.FFMPEGMp3");
    DEMO_CHECK_AND_RETURN_RET_LOG(audioDec_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: CreateByName fail");

    signal_ = make_shared<ADecSignal>();

    cb_ = make_unique<ADecDemoCallback>(signal_);
    DEMO_CHECK_AND_RETURN_RET_LOG(cb_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");
    DEMO_CHECK_AND_RETURN_RET_LOG(audioDec_->SetCallback(cb_) == AVCS_ERR_OK, AVCS_ERR_UNKNOWN, "Fatal: SetCallback fail");

    return AVCS_ERR_OK;
}

int32_t ADecInnerDemo::Configure(const Format &format)
{
    return audioDec_->Configure(format);
}


int32_t ADecInnerDemo::Start()
{
    isRunning_.store(true);

    testFile_ = std::make_unique<std::ifstream>();
    DEMO_CHECK_AND_RETURN_RET_LOG(testFile_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");
    testFile_->open("/data/media/video.es", std::ios::in | std::ios::binary);

    inputLoop_ = make_unique<thread>(&ADecInnerDemo::InputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(inputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    outputLoop_ = make_unique<thread>(&ADecInnerDemo::OutputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(outputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    return audioDec_->Start();
}

int32_t ADecInnerDemo::Stop()
{
    isRunning_.store(false);

    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inQueue_.push(0);
        signal_->inCond_.notify_all();
        lock.unlock();
        inputLoop_->join();
        inputLoop_.reset();
    }

    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outQueue_.push(0);
        signal_->outCond_.notify_all();
        lock.unlock();
        outputLoop_->join();
        outputLoop_.reset();
    }

    return audioDec_->Stop();
}

int32_t ADecInnerDemo::Flush()
{
    return audioDec_->Flush();
}

int32_t ADecInnerDemo::Reset()
{
    return audioDec_->Reset();
}

int32_t ADecInnerDemo::Release()
{
    return audioDec_->Release();
}

void ADecInnerDemo::InputFunc()
{
    while (true) {
        if (!isRunning_.load()) {
            break;
        }

        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.wait(lock, [this](){ return signal_->inQueue_.size() > 0; });

        if (!isRunning_.load()) {
            break;
        }

        uint32_t index = signal_->inQueue_.front();
        auto buffer = audioDec_->GetInputBuffer(index);
        DEMO_CHECK_AND_BREAK_LOG(buffer != nullptr, "Fatal: GetInputBuffer fail");
        DEMO_CHECK_AND_BREAK_LOG(testFile_ != nullptr && testFile_->is_open(), "Fatal: open file fail");

        constexpr uint32_t bufferSize = 0; // replace with the actual size
        char *fileBuffer = static_cast<char *>(malloc(sizeof(char) * bufferSize + 1));
        DEMO_CHECK_AND_BREAK_LOG(fileBuffer != nullptr, "Fatal: malloc fail");

        (void)testFile_->read(fileBuffer, bufferSize);
        
        DEMO_CHECK_AND_BREAK_LOG(buffer != nullptr, "Fatal: GetInputBuffer fail");
        if (memcpy_s(buffer->GetBase(), buffer->GetSize(), fileBuffer, bufferSize) != EOK) {
            free(fileBuffer);
            cout << "Fatal: memcpy fail" << endl;
            break;
        }

        AVCodecBufferInfo info;
        info.size = bufferSize;
        info.offset = 0;
        info.presentationTimeUs = timeStamp_;

        int32_t ret = AVCS_ERR_OK;
        if (isFirstFrame_) {
            ret = audioDec_->QueueInputBuffer(index, info, AVCODEC_BUFFER_FLAG_CODEC_DATA);
            isFirstFrame_ = false;
        } else {
            ret = audioDec_->QueueInputBuffer(index, info, AVCODEC_BUFFER_FLAG_NONE);
        }

        free(fileBuffer);
        timeStamp_ += FRAME_DURATION_US;
        signal_->inQueue_.pop();

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

void ADecInnerDemo::OutputFunc()
{
    while (true) {
        if (!isRunning_.load()) {
            break;
        }

        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.wait(lock, [this](){ return signal_->outQueue_.size() > 0; });

        if (!isRunning_.load()) {
            break;
        }

        uint32_t index = signal_->outQueue_.front();
        if (audioDec_->ReleaseOutputBuffer(index) != AVCS_ERR_OK) {
            cout << "Fatal: ReleaseOutputBuffer fail" << endl;
            break;
        }

        signal_->outQueue_.pop();
    }
}

ADecDemoCallback::ADecDemoCallback(shared_ptr<ADecSignal> signal)
    : signal_(signal)
{
}

void ADecDemoCallback::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
    cout << "Error received, errorType:" << errorType << " errorCode:" << errorCode << endl;
}

void ADecDemoCallback::OnOutputFormatChanged(const Format &format)
{
    (void)format;
    cout << "OnOutputFormatChanged received" << endl;
}

void ADecDemoCallback::OnInputBufferAvailable(uint32_t index)
{
    cout << "OnInputBufferAvailable received, index:" << index << endl;
    unique_lock<mutex> lock(signal_->inMutex_);
    signal_->inQueue_.push(index);
    signal_->inCond_.notify_all();
}

void ADecDemoCallback::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    (void)info;
    (void)flag;
    cout << "OnOutputBufferAvailable received, index:" << index << endl;
    unique_lock<mutex> lock(signal_->outMutex_);
    signal_->outQueue_.push(index);
    signal_->outCond_.notify_all();
}