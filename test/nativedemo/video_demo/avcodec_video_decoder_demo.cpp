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
#include "surface.h"
#include "buffer_queue_producer.h"
#include "consumer_surface.h"

#include "av_common.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "demo_log.h"
#include "media_description.h"
#include "native_avcodec_base.h"
#include "native_avformat.h"
#include "avcodec_video_decoder_demo.h"

using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::VideoDemo;
using namespace std;
namespace {
constexpr uint32_t DEFAULT_WIDTH = 320;
constexpr uint32_t DEFAULT_HEIGHT = 240;
constexpr uint32_t FRAME_DURATION_US = 33000;

constexpr string_view inputFilePath = "/data/test/media/out_320_240_10s.h264";
constexpr string_view outputFilePath = "/data/test/media/out_320_240_10s.yuv";
constexpr string_view outputSurfacePath = "/data/test/media/out_320_240_10s.rgba";
constexpr uint32_t SLEEP_TIME = 10;
uint32_t writeFrameCount = 0;
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
    VDecSignal *signal_ = static_cast<VDecSignal *>(userData);
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
    VDecSignal *signal_ = static_cast<VDecSignal *>(userData);
    unique_lock<mutex> lock(signal_->outMutex_);
    signal_->outQueue_.push(index);
    signal_->outBufferQueue_.push(data);
    if (attr) {
        cout << "OnOutputBufferAvailable received, index:" << index << ", attr->size:" << attr->size << endl;
        signal_->attrQueue_.push(*attr);
    } else {
        cout << "OnOutputBufferAvailable error, attr is nullptr!" << endl;
    }
    signal_->outCond_.notify_all();
}

void VDecDemo::RunCase(bool isSurfaceMode)
{
    isSurfaceMode_ = isSurfaceMode;
    sptr<Surface> cs = nullptr;
    sptr<Surface> ps = nullptr;

    DEMO_CHECK_AND_RETURN_LOG(CreateDec() == AVCS_ERR_OK, "Fatal: CreateDec fail");

    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_WIDTH.data(), DEFAULT_WIDTH);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_HEIGHT.data(), DEFAULT_HEIGHT);
    DEMO_CHECK_AND_RETURN_LOG(Configure(format) == AVCS_ERR_OK, "Fatal: Configure fail");

    if (isSurfaceMode) {
        cs = Surface::CreateSurfaceAsConsumer();
        sptr<IBufferConsumerListener> listener =
            new InnerVideoDemo::TestConsumerListener(cs, outputSurfacePath, writeFrameCount);
        cs->RegisterConsumerListener(listener);
        auto p = cs->GetProducer();
        ps = Surface::CreateSurfaceAsProducer(p);
        OHNativeWindow *nativeWindow = CreateNativeWindowFromSurface(&ps);
        DEMO_CHECK_AND_RETURN_LOG(SetSurface(nativeWindow) == AVCS_ERR_OK, "Fatal: SetSurface fail");
    }

    DEMO_CHECK_AND_RETURN_LOG(Start() == AVCS_ERR_OK, "Fatal: Start fail");

    while (isRunning_.load()) {
        sleep(SLEEP_TIME); // sleep 1s
    }

    DEMO_CHECK_AND_RETURN_LOG(Stop() == AVCS_ERR_OK, "Fatal: Stop fail");
    std::cout << "end stop!" << std::endl;
    DEMO_CHECK_AND_RETURN_LOG(Release() == AVCS_ERR_OK, "Fatal: Release fail");
}

VDecDemo::VDecDemo()
{
    codec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (codec_ == nullptr) {
        std::cout << "find h264 fail" << std::endl;
        exit(1);
    }

    parser_ = av_parser_init(codec_->id);
    if (parser_ == nullptr) {
        std::cout << "parser init fail" << std::endl;
        exit(1);
    }

    codec_ctx_ = avcodec_alloc_context3(codec_);
    if (codec_ctx_ == nullptr) {
        std::cout << "alloc context fail" << std::endl;
        exit(1);
    }

    if (avcodec_open2(codec_ctx_, codec_, NULL) < 0) {
        std::cout << "codec open fail" << std::endl;
        exit(1);
    }

    pkt_ = av_packet_alloc();
    if (pkt_ == nullptr) {
        std::cout << "alloc pkt fail" << std::endl;
        exit(1);
    }
    pkt_->data = NULL;
    pkt_->size = 0;
}

VDecDemo::~VDecDemo()
{
    if (signal_) {
        delete signal_;
        signal_ = nullptr;
    }

    avcodec_free_context(&codec_ctx_);
    av_parser_close(parser_);
    av_packet_free(&pkt_);

    if (inputFile_ != nullptr) {
        inputFile_->close();
    }

    if (outFile_ != nullptr) {
        outFile_->close();
    }
}

int32_t VDecDemo::CreateDec()
{
    videoDec_ = OH_VideoDecoder_CreateByName("video_decoder.avc");
    DEMO_CHECK_AND_RETURN_RET_LOG(videoDec_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: CreateByName fail");

    signal_ = new VDecSignal();
    DEMO_CHECK_AND_RETURN_RET_LOG(signal_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    cb_ = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
    int32_t ret = OH_VideoDecoder_SetCallback(videoDec_, cb_, signal_);
    DEMO_CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_UNKNOWN, "Fatal: SetCallback fail");

    return AVCS_ERR_OK;
}

int32_t VDecDemo::Configure(OH_AVFormat *format)
{
    return OH_VideoDecoder_Configure(videoDec_, format);
}

int32_t VDecDemo::SetSurface(OHNativeWindow *window)
{
    return OH_VideoDecoder_SetSurface(videoDec_, window);
}

int32_t VDecDemo::Start()
{
    inputFile_ = std::make_unique<std::ifstream>();
    DEMO_CHECK_AND_RETURN_RET_LOG(inputFile_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");
    inputFile_->open(inputFilePath.data(), std::ios::in | std::ios::binary);

    if (!isSurfaceMode_) {
        outFile_ = std::make_unique<std::ofstream>();
        DEMO_CHECK_AND_RETURN_RET_LOG(outFile_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");
        outFile_->open(outputFilePath.data(), std::ios::out | std::ios::binary);
    }

    isRunning_.store(true);

    inputLoop_ = make_unique<thread>(&VDecDemo::InputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(inputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    outputLoop_ = make_unique<thread>(&VDecDemo::OutputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(outputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    return OH_VideoDecoder_Start(videoDec_);
}

int32_t VDecDemo::Stop()
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
    std::cout << "start stop!" << std::endl;
    return OH_VideoDecoder_Stop(videoDec_);
}

int32_t VDecDemo::Flush()
{
    return OH_VideoDecoder_Flush(videoDec_);
}

int32_t VDecDemo::Reset()
{
    return OH_VideoDecoder_Reset(videoDec_);
}

int32_t VDecDemo::Release()
{
    return OH_VideoDecoder_Destroy(videoDec_);
}

int32_t VDecDemo::ExtractPacket()
{
    int32_t len = 0;
    int32_t ret = 0;

    if (data_ == nullptr) {
        data_ = inbuf_;
        (void)inputFile_->read(reinterpret_cast<char *>(inbuf_), VIDEO_INBUF_SIZE);
        data_size_ = inputFile_->gcount();
    }

    if ((data_size_ < VIDEO_REFILL_THRESH) && !file_end_) {
        memmove_s(inbuf_, data_size_, data_, data_size_);
        data_ = inbuf_;
        (void)inputFile_->read(reinterpret_cast<char *>(data_ + data_size_), VIDEO_INBUF_SIZE - data_size_);
        len = inputFile_->gcount();
        if (len > 0) {
            data_size_ += len;
        } else if (len == 0 && data_size_ == 0) {
            file_end_ = true;
            cout << "extract file end" << endl;
        }
    }

    if (data_size_ > 0) {
        ret = av_parser_parse2(parser_, codec_ctx_, &pkt_->data, &pkt_->size, data_, data_size_, AV_NOPTS_VALUE,
                               AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            cout << "av_parser_parser2 Error!" << endl;
        }
        data_ += ret;
        data_size_ -= ret;
        if (pkt_->size) {
            return AVCS_ERR_OK;
        } else {
            return AVCS_ERR_UNKNOWN;
        }
    }
    return AVCS_ERR_UNKNOWN;
}

void VDecDemo::InputFunc()
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
        if (!file_end_ && (ExtractPacket() != AVCS_ERR_OK || pkt_->size == 0)) {
            continue;
        }

        OH_AVCodecBufferAttr info;
        if (file_end_) {
            info.size = 0;
            info.offset = 0;
            info.pts = 0;
            info.flags = AVCODEC_BUFFER_FLAGS_EOS;
            OH_VideoDecoder_PushInputData(videoDec_, index, info);
            signal_->inBufferQueue_.pop();
            signal_->inQueue_.pop();
            cout << "push end" << endl;
            break;
        }

        info.size = pkt_->size;
        info.offset = 0;
        info.pts = pkt_->pts;
        DEMO_CHECK_AND_BREAK_LOG(buffer != nullptr, "Fatal: GetInputBuffer fail");
        memcpy_s(OH_AVMemory_GetAddr(buffer), pkt_->size, pkt_->data, pkt_->size);

        int32_t ret = AVCS_ERR_OK;
        if (isFirstFrame_) {
            info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
            ret = OH_VideoDecoder_PushInputData(videoDec_, index, info);
            isFirstFrame_ = false;
        } else {
            info.flags = AVCODEC_BUFFER_FLAGS_NONE;
            ret = OH_VideoDecoder_PushInputData(videoDec_, index, info);
        }

        if (ret != AVCS_ERR_OK) {
            cout << "Fatal error, exit" << endl;
            break;
        }

        timeStamp_ += FRAME_DURATION_US;
        signal_->inQueue_.pop();
        signal_->inBufferQueue_.pop();
    }
}

void VDecDemo::OutputFunc()
{
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
        if (outFile_ != nullptr && attr.size != 0 && data != nullptr && OH_AVMemory_GetAddr(data) != nullptr) {
            cout << "OutputFunc write file,buffer index" << index << ", data size = :" << attr.size << endl;
            outFile_->write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), attr.size);
            writeFrameCount++;
        }

        if (attr.flags == AVCODEC_BUFFER_FLAGS_EOS) {
            cout << "decode eos, write frame:" << writeFrameCount << endl;
            isRunning_.store(false);
        }

        if (!isSurfaceMode_ && OH_VideoDecoder_FreeOutputData(videoDec_, index) != AV_ERR_OK) {
            cout << "Fatal: FreeOutputData fail" << endl;
            break;
        }

        if (isSurfaceMode_ && OH_VideoDecoder_RenderOutputData(videoDec_, index) != AV_ERR_OK) {
            cout << "Fatal: RenderOutputData fail" << endl;
            break;
        }

        signal_->outBufferQueue_.pop();
        signal_->attrQueue_.pop();
        signal_->outQueue_.pop();
    }
}