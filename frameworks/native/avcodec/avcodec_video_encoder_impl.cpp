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

#include "avcodec_video_encoder_impl.h"
#include "i_avcodec_service.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecVideoEncoderImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVCodecVideoEncoder> VideoEncoderFactory::CreateByMime(const std::string &mime)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    std::shared_ptr<AVCodecVideoEncoderImpl> impl = std::make_shared<AVCodecVideoEncoderImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, 
        "Failed to new AVCodec video encoder impl");

    int32_t ret = impl->Init(AVCODEC_TYPE_VIDEO_ENCODER, true, mime);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, 
        "Failed to init AVCodec video encoder impl");

    return impl;
}

std::shared_ptr<AVCodecVideoEncoder> VideoEncoderFactory::CreateByName(const std::string &name)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    std::shared_ptr<AVCodecVideoEncoderImpl> impl = std::make_shared<AVCodecVideoEncoderImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, 
        "Failed to new AVCodec video encoder impl");

    int32_t ret = impl->Init(AVCODEC_TYPE_VIDEO_ENCODER, false, name);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, 
        "Failed to init AVCodec video encoder impl");

    return impl;
}

int32_t AVCodecVideoEncoderImpl::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    codecService_ = AVCodecServiceFactory::GetInstance().CreateCodecService();
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_UNKNOWN, "Failed to create codec service");

    return codecService_->Init(type, isMimeType, name);
}

AVCodecVideoEncoderImpl::AVCodecVideoEncoderImpl()
{
    AVCODEC_LOGD("AVCodecVideoEncoderImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecVideoEncoderImpl::~AVCodecVideoEncoderImpl()
{
    if (codecService_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroyCodecService(codecService_);
        codecService_ = nullptr;
    }
    AVCODEC_LOGD("AVCodecVideoEncoderImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVCodecVideoEncoderImpl::Configure(const Format &format)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Configure(format);
}

int32_t AVCodecVideoEncoderImpl::Prepare()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return AVCS_ERR_OK;
}

int32_t AVCodecVideoEncoderImpl::Start()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Start();
}

int32_t AVCodecVideoEncoderImpl::Stop()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Stop();
}

int32_t AVCodecVideoEncoderImpl::Flush()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Flush();
}

int32_t AVCodecVideoEncoderImpl::NotifyEos()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->NotifyEos();
}

int32_t AVCodecVideoEncoderImpl::Reset()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Reset();
}

int32_t AVCodecVideoEncoderImpl::Release()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->Release();
}

sptr<Surface> AVCodecVideoEncoderImpl::CreateInputSurface()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        nullptr, "Codec service is nullptr");
    surface_ = codecService_->CreateInputSurface();
    return surface_;
}

std::shared_ptr<AVSharedMemory> AVCodecVideoEncoderImpl::GetInputBuffer(uint32_t index)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        nullptr, "Codec service is nullptr");
    return codecService_->GetInputBuffer(index);
}

int32_t AVCodecVideoEncoderImpl::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->QueueInputBuffer(index, info, flag);
}

std::shared_ptr<AVSharedMemory> AVCodecVideoEncoderImpl::GetOutputBuffer(uint32_t index)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        nullptr, "Codec service is nullptr");
    return codecService_->GetOutputBuffer(index);
}

int32_t AVCodecVideoEncoderImpl::GetOutputFormat(Format &format)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->GetOutputFormat(format);
}

int32_t AVCodecVideoEncoderImpl::ReleaseOutputBuffer(uint32_t index)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->ReleaseOutputBuffer(index);
}

int32_t AVCodecVideoEncoderImpl::SetParameter(const Format &format)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    return codecService_->SetParameter(format);
}

int32_t AVCodecVideoEncoderImpl::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
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
