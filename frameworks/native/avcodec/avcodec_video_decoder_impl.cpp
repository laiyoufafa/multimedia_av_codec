/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "avcodec_video_decoder_impl.h"
#include "i_avcodec_service.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecVideoDecoderImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVCodecVideoDecoder> VideoDecoderFactory::CreateByMime(const std::string &mime)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    std::shared_ptr<AVCodecVideoDecoderImpl> impl = std::make_shared<AVCodecVideoDecoderImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, 
        nullptr, "Failed to create AVCodec video decoder impl");

    int32_t ret = impl->Init(AVCODEC_TYPE_VIDEO_DECODER, true, mime);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, 
        nullptr, "Failed to init AVCodec video decoder impl");

    return impl;
}

std::shared_ptr<AVCodecVideoDecoder> VideoDecoderFactory::CreateByName(const std::string &name)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    std::shared_ptr<AVCodecVideoDecoderImpl> impl = std::make_shared<AVCodecVideoDecoderImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, 
        nullptr, "Failed to create AVCodec video decoder impl");

    int32_t ret = impl->Init(AVCODEC_TYPE_VIDEO_DECODER, false, name);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, 
        nullptr, "Failed to init AVCodec video decoder impl");

    return impl;
}

int32_t AVCodecVideoDecoderImpl::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    codecService_ = AVCodecServiceFactory::GetInstance().CreateCodecService();
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_UNKNOWN, "Failed to create codec service");

    return codecService_->Init(type, isMimeType, name);
}

AVCodecVideoDecoderImpl::AVCodecVideoDecoderImpl()
{
    AVCODEC_LOGD("AVCodecVideoDecoderImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecVideoDecoderImpl::~AVCodecVideoDecoderImpl()
{
    if (codecService_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroyCodecService(codecService_);
        codecService_ = nullptr;
    }
    AVCODEC_LOGD("AVCodecVideoDecoderImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVCodecVideoDecoderImpl::Configure(const Format &format)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Configure(format);
}

int32_t AVCodecVideoDecoderImpl::Prepare()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return AVCS_ERR_OK;
}

int32_t AVCodecVideoDecoderImpl::Start()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Start();
}

int32_t AVCodecVideoDecoderImpl::Stop()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Stop();
}

int32_t AVCodecVideoDecoderImpl::Flush()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Flush();
}

int32_t AVCodecVideoDecoderImpl::Reset()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Reset();
}

int32_t AVCodecVideoDecoderImpl::Release()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Release();
}

int32_t AVCodecVideoDecoderImpl::SetOutputSurface(sptr<Surface> surface)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->SetOutputSurface(surface);
}

std::shared_ptr<AVSharedMemory> AVCodecVideoDecoderImpl::GetInputBuffer(uint32_t index)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        nullptr, "Codec service is nullptr");
    return codecService_->GetInputBuffer(index);
}

int32_t AVCodecVideoDecoderImpl::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->QueueInputBuffer(index, info, flag);
}

std::shared_ptr<AVSharedMemory> AVCodecVideoDecoderImpl::GetOutputBuffer(uint32_t index)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        nullptr, "Codec service is nullptr");
    return codecService_->GetOutputBuffer(index);
}

int32_t AVCodecVideoDecoderImpl::GetOutputFormat(Format &format)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->GetOutputFormat(format);
}

int32_t AVCodecVideoDecoderImpl::ReleaseOutputBuffer(uint32_t index, bool render)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->ReleaseOutputBuffer(index, render);
}

int32_t AVCodecVideoDecoderImpl::SetParameter(const Format &format)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->SetParameter(format);
}

int32_t AVCodecVideoDecoderImpl::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, 
        AVCS_ERR_INVALID_VAL, "Callback is nullptr");
    return codecService_->SetCallback(callback);
}
} // namespace Media
} // namespace OHOS
