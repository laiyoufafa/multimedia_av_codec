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

#include <list>
#include <mutex>
#include "native_avcodec_base.h"
#include "native_avcodec_videodecoder.h"
#include "native_avmagic.h"
#include "native_window.h"
#include "avcodec_video_decoder.h"
#include "avsharedmemory.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NativeVideoDecoder"};
}

using namespace OHOS::Media;
class NativeVideoDecoderCallback;

struct VideoDecoderObject : public OH_AVCodec {
    explicit VideoDecoderObject(const std::shared_ptr<AVCodecVideoDecoder> &decoder)
        : OH_AVCodec(AVMagic::AVCODEC_MAGIC_VIDEO_DECODER), videoDecoder_(decoder)
    {
    }
    ~VideoDecoderObject() = default;

    const std::shared_ptr<AVCodecVideoDecoder> videoDecoder_;
    std::list<OHOS::sptr<OH_AVMemory>> memoryObjList_;
    std::shared_ptr<NativeVideoDecoderCallback> callback_ = nullptr;
    std::atomic<bool> isFlushing_ = false;
    std::atomic<bool> isStop_ = false;
    std::atomic<bool> isEOS_ = false;
    bool isOutputSurfaceMode_ = false;
};

class NativeVideoDecoderCallback : public AVCodecCallback {
public:
    NativeVideoDecoderCallback(OH_AVCodec *codec, struct OH_AVCodecAsyncCallback cb, void *userData)
        : codec_(codec), callback_(cb), userData_(userData)
    {
    }
    virtual ~NativeVideoDecoderCallback() = default;

    void OnError(AVCodecErrorType errorType, int32_t errorCode) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        (void)errorType;

        CHECK_AND_RETURN_LOG(codec_ != nullptr, "Codec is nullptr");
        CHECK_AND_RETURN_LOG(callback_.onError != nullptr, "Callback is nullptr");
        int32_t extErr = AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(errorCode));
        callback_.onError(codec_, extErr, userData_);
    }

    void OnOutputFormatChanged(const Format &format) override
    {
        std::unique_lock<std::mutex> lock(mutex_);

        CHECK_AND_RETURN_LOG(codec_ != nullptr, "Codec is nullptr");
        CHECK_AND_RETURN_LOG(callback_.onStreamChanged != nullptr, "Callback is nullptr");
        OHOS::sptr<OH_AVFormat> object = new (std::nothrow) OH_AVFormat(format);
        callback_.onStreamChanged(codec_, reinterpret_cast<OH_AVFormat *>(object.GetRefPtr()), userData_);
    }

    void OnInputBufferAvailable(uint32_t index) override
    {
        std::unique_lock<std::mutex> lock(mutex_);

        CHECK_AND_RETURN_LOG(codec_ != nullptr, "Codec is nullptr");
        CHECK_AND_RETURN_LOG(callback_.onNeedInputData != nullptr, "Callback is nullptr");

        struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec_);
        CHECK_AND_RETURN_LOG(videoDecObj->videoDecoder_ != nullptr, "Video decoder is nullptr!");

        if (videoDecObj->isFlushing_.load() || videoDecObj->isStop_.load() || videoDecObj->isEOS_.load()) {
            AVCODEC_LOGD("At flush, eos or stop, no buffer available");
            return;
        }

        OH_AVMemory *data = GetInputData(codec_, index);
        CHECK_AND_RETURN_LOG(data != nullptr, "Data is nullptr, get input data failed");
        callback_.onNeedInputData(codec_, index, data, userData_);
    }

    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        CHECK_AND_RETURN_LOG(codec_ != nullptr, "Codec is nullptr");
        CHECK_AND_RETURN_LOG(callback_.onNeedOutputData != nullptr, "Callback is nullptr");

        struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec_);
        CHECK_AND_RETURN_LOG(videoDecObj->videoDecoder_ != nullptr, "Video decoder is nullptr!");

        if (videoDecObj->isFlushing_.load() || videoDecObj->isStop_.load()) {
            AVCODEC_LOGD("At flush or stop, ignore");
            return;
        }

        struct OH_AVCodecBufferAttr bufferAttr;
        bufferAttr.pts = info.presentationTimeUs;
        bufferAttr.size = info.size;
        bufferAttr.offset = info.offset;
        bufferAttr.flags = flag;
        OH_AVMemory *data = nullptr;
        if (!videoDecObj->isOutputSurfaceMode_) {
            data = GetOutputData(codec_, index);
            CHECK_AND_RETURN_LOG(data != nullptr, "Data is nullptr, get output data failed");
        }
        callback_.onNeedOutputData(codec_, index, data, &bufferAttr, userData_);
    }

    void StopCallback()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        codec_ = nullptr;
    }

private:
    OH_AVMemory *GetInputData(struct OH_AVCodec *codec, uint32_t index)
    {
        CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "Codec is nullptr!");
        CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, nullptr, "Codec magic error!");

        struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
        CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, nullptr, "Video decoder is nullptr!");

        std::shared_ptr<AVSharedMemory> memory = videoDecObj->videoDecoder_->GetInputBuffer(index);
        CHECK_AND_RETURN_RET_LOG(memory != nullptr, nullptr, "Memory is nullptr, get input buffer failed!");

        for (auto &memoryObj : videoDecObj->memoryObjList_) {
            if (memoryObj->IsEqualMemory(memory)) {
                return reinterpret_cast<OH_AVMemory *>(memoryObj.GetRefPtr());
            }
        }

        OHOS::sptr<OH_AVMemory> object = new (std::nothrow) OH_AVMemory(memory);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "AV memory create failed");

        videoDecObj->memoryObjList_.push_back(object);
        return reinterpret_cast<OH_AVMemory *>(object.GetRefPtr());
    }

    OH_AVMemory *GetOutputData(struct OH_AVCodec *codec, uint32_t index)
    {
        CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "Codec is nullptr!");
        CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, nullptr, "Codec magic error!");

        struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
        CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, nullptr, "Video decoder is nullptr!");

        std::shared_ptr<AVSharedMemory> memory = videoDecObj->videoDecoder_->GetOutputBuffer(index);
        CHECK_AND_RETURN_RET_LOG(memory != nullptr, nullptr, "Video decoder output data is nullptr!");

        for (auto &memoryObj : videoDecObj->memoryObjList_) {
            if (memoryObj->IsEqualMemory(memory)) {
                return reinterpret_cast<OH_AVMemory *>(memoryObj.GetRefPtr());
            }
        }

        OHOS::sptr<OH_AVMemory> object = new (std::nothrow) OH_AVMemory(memory);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "AV memory create failed");

        videoDecObj->memoryObjList_.push_back(object);
        return reinterpret_cast<OH_AVMemory *>(object.GetRefPtr());
    }

    struct OH_AVCodec *codec_;
    struct OH_AVCodecAsyncCallback callback_;
    void *userData_;
    std::mutex mutex_;
};

struct OH_AVCodec *OH_VideoDecoder_CreateByMime(const char *mime)
{
    CHECK_AND_RETURN_RET_LOG(mime != nullptr, nullptr, "Mime is nullptr!");

    std::shared_ptr<AVCodecVideoDecoder> videoDecoder = VideoDecoderFactory::CreateByMime(mime);
    CHECK_AND_RETURN_RET_LOG(videoDecoder != nullptr, nullptr, "Video decoder create by mime failed!");

    struct VideoDecoderObject *object = new (std::nothrow) VideoDecoderObject(videoDecoder);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "Video decoder create by mime failed!");

    return object;
}

struct OH_AVCodec *OH_VideoDecoder_CreateByName(const char *name)
{
    CHECK_AND_RETURN_RET_LOG(name != nullptr, nullptr, "Name is nullptr!");

    std::shared_ptr<AVCodecVideoDecoder> videoDecoder = VideoDecoderFactory::CreateByName(name);
    CHECK_AND_RETURN_RET_LOG(videoDecoder != nullptr, nullptr, "Video decoder create by name failed!");

    struct VideoDecoderObject *object = new (std::nothrow) VideoDecoderObject(videoDecoder);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "Video decoder create by name failed!");

    return object;
}

OH_AVErrCode OH_VideoDecoder_Destroy(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);

    if (videoDecObj != nullptr && videoDecObj->videoDecoder_ != nullptr) {
        if (videoDecObj->callback_ != nullptr) {
            videoDecObj->callback_->StopCallback();
        }
        videoDecObj->memoryObjList_.clear();
        videoDecObj->isStop_.store(false);
        int32_t ret = videoDecObj->videoDecoder_->Release();
        if (ret != AVCS_ERR_OK) {
            AVCODEC_LOGE("Video decoder destroy failed!");
            delete codec;
            return AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret);
        }
    } else {
        AVCODEC_LOGD("Video decoder is nullptr!");
    }

    delete codec;
    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_Configure(struct OH_AVCodec *codec, struct OH_AVFormat *format)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");
    CHECK_AND_RETURN_RET_LOG(format != nullptr, AV_ERR_INVALID_VAL, "Format is nullptr!");
    CHECK_AND_RETURN_RET_LOG(format->magic_ == AVMagic::AVCODEC_MAGIC_FORMAT, AV_ERR_INVALID_VAL,
                             "Format magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    int32_t ret = videoDecObj->videoDecoder_->Configure(format->format_);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret),
                             "Video decoder configure failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_Prepare(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    int32_t ret = videoDecObj->videoDecoder_->Prepare();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret),
                             "Video decoder prepare failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_Start(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    videoDecObj->isStop_.store(false);
    videoDecObj->isEOS_.store(false);
    int32_t ret = videoDecObj->videoDecoder_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret),
                             "Video decoder start failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_Stop(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    videoDecObj->isStop_.store(true);
    int32_t ret = videoDecObj->videoDecoder_->Stop();
    if (ret != AVCS_ERR_OK) {
        videoDecObj->isStop_.store(false);
        AVCODEC_LOGE("Video decoder stop failed");
        return AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret);
    }
    videoDecObj->memoryObjList_.clear();

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_Flush(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    videoDecObj->isFlushing_.store(true);
    int32_t ret = videoDecObj->videoDecoder_->Flush();
    videoDecObj->isFlushing_.store(false);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret),
                             "Video decoder flush failed!");
    videoDecObj->memoryObjList_.clear();

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_Reset(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    videoDecObj->isStop_.store(true);
    int32_t ret = videoDecObj->videoDecoder_->Reset();
    if (ret != AVCS_ERR_OK) {
        videoDecObj->isStop_.store(false);
        AVCODEC_LOGE("Video decoder reset failed!");
        return AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret);
    }
    videoDecObj->memoryObjList_.clear();

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_SetSurface(OH_AVCodec *codec, OHNativeWindow *window)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");
    CHECK_AND_RETURN_RET_LOG(window != nullptr, AV_ERR_INVALID_VAL, "Window is nullptr!");
    CHECK_AND_RETURN_RET_LOG(window->surface != nullptr, AV_ERR_INVALID_VAL, "Input window surface is nullptr!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    int32_t ret = videoDecObj->videoDecoder_->SetOutputSurface(window->surface);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret),
                             "Video decoder set output surface failed!");
    videoDecObj->isOutputSurfaceMode_ = true;

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_PushInputData(struct OH_AVCodec *codec, uint32_t index, OH_AVCodecBufferAttr attr)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");
    CHECK_AND_RETURN_RET_LOG(attr.size >= 0, AV_ERR_INVALID_VAL, "Invalid buffer size!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    struct AVCodecBufferInfo bufferInfo;
    bufferInfo.presentationTimeUs = attr.pts;
    bufferInfo.size = attr.size;
    bufferInfo.offset = attr.offset;
    enum AVCodecBufferFlag bufferFlag = static_cast<enum AVCodecBufferFlag>(attr.flags);

    int32_t ret = videoDecObj->videoDecoder_->QueueInputBuffer(index, bufferInfo, bufferFlag);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret),
                             "Video decoder push input data failed!");
    if (bufferFlag == AVCODEC_BUFFER_FLAG_EOS) {
        videoDecObj->isEOS_.store(true);
    }

    return AV_ERR_OK;
}

OH_AVFormat *OH_VideoDecoder_GetOutputDescription(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, nullptr, "Codec magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, nullptr, "Video decoder is nullptr!");

    Format format;
    int32_t ret = videoDecObj->videoDecoder_->GetOutputFormat(format);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Video decoder get output description failed!");

    OH_AVFormat *avFormat = OH_AVFormat_Create();
    CHECK_AND_RETURN_RET_LOG(avFormat != nullptr, nullptr, "Video decoder get output description failed!");
    avFormat->format_ = format;

    return avFormat;
}

OH_AVErrCode OH_VideoDecoder_RenderOutputData(struct OH_AVCodec *codec, uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    int32_t ret = videoDecObj->videoDecoder_->ReleaseOutputBuffer(index, true);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret),
                             "Video decoder render output data failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_FreeOutputData(struct OH_AVCodec *codec, uint32_t index)
{
    AVCODEC_LOGD("In OH_VideoDecoder_FreeOutputData");
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    int32_t ret = videoDecObj->videoDecoder_->ReleaseOutputBuffer(index, false);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret)),
                             "Video decoder free output data failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_SetParameter(struct OH_AVCodec *codec, struct OH_AVFormat *format)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");
    CHECK_AND_RETURN_RET_LOG(format != nullptr, AV_ERR_INVALID_VAL, "Format is nullptr!");
    CHECK_AND_RETURN_RET_LOG(format->magic_ == AVMagic::AVCODEC_MAGIC_FORMAT, AV_ERR_INVALID_VAL,
                             "Format magic error!");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    int32_t ret = videoDecObj->videoDecoder_->SetParameter(format->format_);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret),
                             "Video decoder set parameter failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_SetCallback(struct OH_AVCodec *codec, struct OH_AVCodecAsyncCallback callback,
                                         void *userData)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");
    CHECK_AND_RETURN_RET_LOG(callback.onError != nullptr,
        AV_ERR_INVALID_VAL, "Callback onError is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback.onNeedInputData != nullptr,
        AV_ERR_INVALID_VAL, "Callback onNeedInputData is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback.onNeedOutputData != nullptr,
        AV_ERR_INVALID_VAL, "Callback onNeedOutputData is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback.onStreamChanged != nullptr,
        AV_ERR_INVALID_VAL, "Callback onStreamChanged is nullptr");

    struct VideoDecoderObject *videoDecObj = reinterpret_cast<VideoDecoderObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(videoDecObj->videoDecoder_ != nullptr, AV_ERR_INVALID_VAL, "Video decoder is nullptr!");

    videoDecObj->callback_ = std::make_shared<NativeVideoDecoderCallback>(codec, callback, userData);

    int32_t ret = videoDecObj->videoDecoder_->SetCallback(videoDecObj->callback_);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret),
                             "Video decoder set callback failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_VideoDecoder_IsValid(OH_AVCodec *codec, bool *isValid)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "Codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(codec->magic_ == AVMagic::AVCODEC_MAGIC_VIDEO_DECODER, AV_ERR_INVALID_VAL,
                             "Codec magic error!");
    CHECK_AND_RETURN_RET_LOG(isValid != nullptr, AV_ERR_INVALID_VAL, "Input isValid is nullptr!");
    *isValid = true;
    return AV_ERR_OK;
}