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
#include "native_avcodec.h"
#include "native_avmagic.h"
#include "avcodec.h"
#include "avsharedmemory.h"
#include "media_log.h"
#include "media_errors.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NativeAVCodec"};
}

using namespace OHOS::AVCodec;
class NativeCodecCallback;

struct CodecObject : public OH_AVCodec {
    explicit CodecObject(const std::shared_ptr<AVCodec> &codec)
        : codec_(codec) {}
    ~CodecObject() = default;

    const std::shared_ptr<AVCodec> codec_;
    std::list<OHOS::sptr<OH_AVBufferElement>> memoryObjList_;
    std::shared_ptr<NativeCodecCallback> callback_ = nullptr;
    std::atomic<bool> isFlushing_ = false;
    std::atomic<bool> isStop_ = false;
    std::atomic<bool> isEOS_ = false;
};

class NativeCodecCallback : public AVCodecCallback {
public:
    NativeCodecCallback(OH_AVCodec *codec, struct OH_AVCodecCallback cb, void *userData)
        : codec_(codec), callback_(cb), userData_(userData) {}
    virtual ~NativeCodecCallback() = default;

    void OnError(AVCodecErrorType errorType, int32_t errorCode) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        (void)errorType;
        if (codec_ != nullptr && callback_.onError != nullptr) {
            int32_t extErr = MSErrorToExtError(static_cast<MediaServiceErrCode>(errorCode));
            callback_.onError(codec_, extErr, userData_);
        }
    }

    void OnOutputFormatChanged(const Format &format) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (codec_ != nullptr && callback_.onFormatChanged != nullptr) {
            OHOS::sptr<OH_AVFormat> object = new(std::nothrow) OH_AVFormat(format);
            // The object lifecycle is controlled by the current function stack
            callback_.onFormatChanged(codec_, reinterpret_cast<OH_AVFormat *>(object.GetRefPtr()), userData_);
        }
    }

    void OnInputBufferAvailable(uint32_t index) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (codec_ != nullptr && callback_.onInputDataReady != nullptr) {
            struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec_);
            CHECK_AND_RETURN_LOG(codecObj->codec_ != nullptr, "codec_ is nullptr!");

            if (codecObj->isFlushing_.load() || codecObj->isStop_.load() || codecObj->isEOS_.load()) {
                AVCODEC_LOGD("At flush, eos or stop, no buffer available");
                return;
            }
            OH_AVBufferElement *data = GetInputData(codec_, index);
            if (data != nullptr) {
                callback_.onInputDataReady(codec_, index, data, userData_);
            }
        }
    }

    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (codec_ != nullptr && callback_.onOutputDataReady != nullptr) {
            struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec_);
            CHECK_AND_RETURN_LOG(codecObj->codec_ != nullptr, "codec_ is nullptr!");

            if (codecObj->isFlushing_.load() || codecObj->isStop_.load()) {
                AVCODEC_LOGD("At flush or stop, ignore");
                return;
            }
            struct OH_AVCodecBufferAttr bufferAttr;
            bufferAttr.pts = info.presentationTimeUs;
            bufferAttr.size = info.size;
            bufferAttr.offset = info.offset;
            bufferAttr.flags = flag;
            // The bufferInfo lifecycle is controlled by the current function stack
            callback_.onOutputDataReady(codec_, index, nullptr, &bufferAttr, userData_);
        }
    }

    void StopCallback()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        codec_ = nullptr;
    }

private:
    OH_AVBufferElement *GetInputData(struct OH_AVCodec *codec, uint32_t index)
    {
        CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "input codec is nullptr!");

        struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
        CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, nullptr, "codec_ is nullptr!");

        std::shared_ptr<AVBufferElement> bufferElement = codecObj->codec_->GetInputBuffer(index);
        CHECK_AND_RETURN_RET_LOG(bufferElement != nullptr, nullptr, "get input buffer is nullptr!");

        for (auto &memoryObj : codecObj->memoryObjList_) {
            if (memoryObj->IsEqualBufferElement(bufferElement)) {
                return reinterpret_cast<OH_AVBufferElement *>(memoryObj);
            }
        }

        OHOS::sptr<OH_AVBufferElement> object = new(std::nothrow) OH_AVBufferElement(bufferElement);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "failed to new OH_AVBufferElement");

        codecObj->memoryObjList_.push_back(object);
        return reinterpret_cast<OH_AVBufferElement *>(object);
    }

    OH_AVBufferElement *GetOutputData(struct OH_AVCodec *codec, uint32_t index)
    {
        CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "input codec is nullptr!");

        struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
        CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, nullptr, "codec_ is nullptr!");

        std::shared_ptr<AVBufferElement> bufferElement = codecObj->codec_->GetOutputBuffer(index);
        CHECK_AND_RETURN_RET_LOG(bufferElement != nullptr, nullptr, "get output buffer is nullptr!");

        for (auto &memoryObj : codecObj->memoryObjList_) {
            if (memoryObj->IsEqualBufferElement(bufferElement)) {
                return reinterpret_cast<OH_AVBufferElement *>(memoryObj);
            }
        }

        OHOS::sptr<OH_AVBufferElement> object = new(std::nothrow) OH_AVBufferElement(bufferElement);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "failed to new OH_AVBufferElement");

        codecObj->memoryObjList_.push_back(object);
        return reinterpret_cast<OH_AVBufferElement *>(object);
    }

    struct OH_AVCodec *codec_;
    struct OH_AVCodecCallback callback_;
    void *userData_;
    std::mutex mutex_;
};

struct OH_AVCodec *OH_AVCodec_CreateEncoderByMime(const char *mime)
{
    CHECK_AND_RETURN_RET_LOG(mime != nullptr, nullptr, "input mime is nullptr!");

    std::shared_ptr<AVCodec> codec = CodecFactory::CreateByMime(mime, true);
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "failed to CodecFactory::CreateByMime");

    struct CodecObject *object = new(std::nothrow) CodecObject(codec);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "failed to new CodecObject");

    return object;
}

struct OH_AVCodec *OH_AVCodec_CreateDecoderByMime(const char *mime)
{
    CHECK_AND_RETURN_RET_LOG(mime != nullptr, nullptr, "input mime is nullptr!");

    std::shared_ptr<AVCodec> codec = CodecFactory::CreateByMime(mime, false);
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "failed to CodecFactory::CreateByMime");

    struct CodecObject *object = new(std::nothrow) CodecObject(codec);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "failed to new CodecObject");

    return object;
}

struct OH_AVCodec *OH_AVCodec_CreateByName(const char *name)
{
    CHECK_AND_RETURN_RET_LOG(name != nullptr, nullptr, "input name is nullptr!");

    std::shared_ptr<AVCodec> codec = CodecFactory::CreateByName(name);
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "failed to CodecFactory::CreateByMime");

    struct CodecObject *object = new(std::nothrow) CodecObject(codec);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "failed to new CodecObject");

    return object;
}

OH_AVErrCode OH_AVCodec_Destroy(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);

    if (codecObj != nullptr && codecObj->codec_ != nullptr) {
        codecObj->callback_->StopCallback();
        codecObj->memoryObjList_.clear();
        codecObj->isStop_.store(false);
        int32_t ret = codecObj->codec_->Release();
        if (ret != MSERR_OK) {
            AVCODEC_LOGE("codec Release failed!");
            delete codec;
            return AV_ERR_OPERATE_NOT_PERMIT;
        }
    } else {
        AVCODEC_LOGD("codec_ is nullptr!");
    }

    delete codec;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_Configure(struct OH_AVCodec *codec, struct OH_AVFormat *format)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(format != nullptr, AV_ERR_INVALID_VAL, "input format is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec is nullptr!");

    int32_t ret = codecObj->codec_->Configure(format->format_);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "codec Configure failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_Start(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");
    codecObj->isStop_.store(false);
    codecObj->isEOS_.store(false);
    int32_t ret = codecObj->codec_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "codec Start failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_Stop(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    codecObj->isStop_.store(true);
    AVCODEC_LOGD("Set stop status to true");

    int32_t ret = codecObj->codec_->Stop();
    if (ret != MSERR_OK) {
        codecObj->isStop_.store(false);
        AVCODEC_LOGE("codec Stop failed! Set stop status to false");
        return AV_ERR_OPERATE_NOT_PERMIT;
    }
    codecObj->memoryObjList_.clear();

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_Flush(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    codecObj->isFlushing_.store(true);
    AVCODEC_LOGD("Set flush status to true");

    int32_t ret = codecObj->codec_->Flush();
    if (ret != MSERR_OK) {
        codecObj->isFlushing_.store(false);
        AVCODEC_LOGD("codec Flush failed! Set flush status to false");
        return AV_ERR_OPERATE_NOT_PERMIT;
    }

    codecObj->memoryObjList_.clear();
    codecObj->isFlushing_.store(false);
    AVCODEC_LOGD("Set flush status to false");
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_Reset(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");
    codecObj->isStop_.store(false);
    AVCODEC_LOGD("Set stop status to true");
    int32_t ret = codecObj->codec_->Reset();
    if (ret != MSERR_OK) {
        codecObj->isStop_.store(false);
        AVCODEC_LOGE("codec Reset failed! Set stop status to false");
        return AV_ERR_OPERATE_NOT_PERMIT;
    }

    codecObj->memoryObjList_.clear();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_VideoDecoderSetSurface(OH_AVCodec *codec, OHNativeWindow *window)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(window != nullptr, AV_ERR_INVALID_VAL, "input window is nullptr!");
    CHECK_AND_RETURN_RET_LOG(window->surface != nullptr, AV_ERR_INVALID_VAL, "input surface is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    int32_t ret = codecObj->codec_->SetOutputSurface(window->surface);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "codec SetOutputSurface failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_QueueInputBuffer(struct OH_AVCodec *codec, uint32_t index, OH_AVCodecBufferAttr attr)
{
    AVCODEC_LOGD("In OH_AVCodec_QueueInputBuffer");
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    struct AVCodecBufferInfo bufferInfo;
    bufferInfo.presentationTimeUs = attr.pts;
    bufferInfo.size = attr.size;
    bufferInfo.offset = attr.offset;
    enum AVCodecBufferFlag bufferFlag = static_cast<enum AVCodecBufferFlag>(attr.flags);

    int32_t ret = codecObj->codec_->QueueInputBuffer(index, bufferInfo, bufferFlag);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "codec QueueInputBuffer failed!");
    if (bufferFlag == AVCODEC_BUFFER_FLAG_EOS) {
        codecObj->isEOS_.store(true);
    }

    return AV_ERR_OK;
}

OH_AVFormat *OH_AVCodec_GetOutputFormat(struct OH_AVCodec *codec)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, nullptr, "codec_ is nullptr!");

    Format format;
    int32_t ret = codecObj->codec_->GetOutputFormat(format);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "codec GetOutputFormat failed!");

    OH_AVFormat *avFormat = OH_AVFormat_Create();
    avFormat->format_ = format;

    return avFormat;
}

OH_AVErrCode OH_AVCodec_VideoDecoderRenderFrame(struct OH_AVCodec *codec, uint32_t index)
{
    AVCODEC_LOGD("In OH_AVCodec_VideoDecoderRenderFrame");
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    int32_t ret = codecObj->codec_->ReleaseOutputBuffer(index, true);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "codec ReleaseOutputBuffer failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_ReleaseOutputData(struct OH_AVCodec *codec, uint32_t index)
{
    AVCODEC_LOGD("In OH_AVCodec_ReleaseOutputData");
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    int32_t ret = codecObj->codec_->ReleaseOutputBuffer(index, false);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "codec ReleaseOutputBuffer failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_SetParameter(struct OH_AVCodec *codec, struct OH_AVFormat *format)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");
    CHECK_AND_RETURN_RET_LOG(format != nullptr, AV_ERR_INVALID_VAL, "input format is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    int32_t ret = codecObj->codec_->SetParameter(format->format_);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "codec SetParameter failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_SetCallback(
    struct OH_AVCodec *codec, struct OH_AVCodecCallback callback, void *userData)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    codecObj->callback_ = std::make_shared<NativeCodecCallback>(codec, callback, userData);
    CHECK_AND_RETURN_RET_LOG(codecObj->callback_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    int32_t ret = codecObj->codec_->SetCallback(codecObj->callback_);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "codec SetCallback failed!");

    return AV_ERR_OK;
}

OH_AVBufferElement* OH_AVCodec_GetInputBuffer(OH_AVCodec *codec, size_t index)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    std::shared_ptr<AVBufferElement> bufferElement = codecObj->codec_->GetInputBuffer(index);
    CHECK_AND_RETURN_RET_LOG(bufferElement != nullptr, nullptr, "get input buffer is nullptr!")

    return bufferElement
}

OH_AVBufferElement* OH_AVCodec_GetOutputBuffer(OH_AVCodec *codec, size_t index)
{
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, AV_ERR_INVALID_VAL, "input codec is nullptr!");

    struct CodecObject *codecObj = reinterpret_cast<CodecObject *>(codec);
    CHECK_AND_RETURN_RET_LOG(codecObj->codec_ != nullptr, AV_ERR_INVALID_VAL, "codec_ is nullptr!");

    std::shared_ptr<AVBufferElement> bufferElement = codecObj->codec_->GetOutputBuffer(index);
    CHECK_AND_RETURN_RET_LOG(bufferElement != nullptr, nullptr, "get input buffer is nullptr!");    

    return bufferElement;   
}

int32_t OH_AVCodec_DequeueInputBuffer(OH_AVCodec *codec, int64_t timeoutUs)
{
    return 0;
}

int32_t OH_AVCodec_DequeueOutputBuffer(OH_AVCodec *codec, int64_t timeoutUs)
{
    return 0;
}

OH_AVErrCode OH_AVCodec_VideoEncoderGetSurface(OH_AVCodec *codec, OHNativeWindow **window)
{
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_VideoEncoderGetPersistentSurface(OHNativeWindow **window)
{
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_VideoEncoderSetSurface(OH_AVCodec *codec, OHNativeWindow *window)
{
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCodec_VideoEncoderNotifyEndOfStream(OH_AVCodec *codec)
{
    return AV_ERR_OK;
}
