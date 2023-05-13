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
#include "avcodec_dfx.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecVideoEncoderImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVCodecVideoEncoder> VideoEncoderFactory::CreateByMime(const std::string &mime)
{
    AVCODEC_SYNC_TRACE;

    std::shared_ptr<AVCodecVideoEncoderImpl> impl = std::make_shared<AVCodecVideoEncoderImpl>();

    int32_t ret = impl->Init(AVCODEC_TYPE_VIDEO_ENCODER, true, mime);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, 
        "AVCodec video encoder impl init failed");

    return impl;
}

std::shared_ptr<AVCodecVideoEncoder> VideoEncoderFactory::CreateByName(const std::string &name)
{
    AVCODEC_SYNC_TRACE;

    std::shared_ptr<AVCodecVideoEncoderImpl> impl = std::make_shared<AVCodecVideoEncoderImpl>();

    int32_t ret = impl->Init(AVCODEC_TYPE_VIDEO_ENCODER, false, name);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, 
        "AVCodec video encoder impl init failed");

    return impl;
}

int32_t AVCodecVideoEncoderImpl::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    AVCODEC_SYNC_TRACE;
    codecService_ = AVCodecServiceFactory::GetInstance().CreateCodecService();
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_UNKNOWN, "Codec service create failed");

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
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->Configure(format);
}

int32_t AVCodecVideoEncoderImpl::Prepare()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return AVCS_ERR_OK;
}

int32_t AVCodecVideoEncoderImpl::Start()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->Start();
}

int32_t AVCodecVideoEncoderImpl::Stop()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->Stop();
}

int32_t AVCodecVideoEncoderImpl::Flush()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->Flush();
}

int32_t AVCodecVideoEncoderImpl::NotifyEos()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->NotifyEos();
}

int32_t AVCodecVideoEncoderImpl::Reset()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->Reset();
}

int32_t AVCodecVideoEncoderImpl::Release()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->Release();
}

sptr<Surface> AVCodecVideoEncoderImpl::CreateInputSurface()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        nullptr, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    surface_ = codecService_->CreateInputSurface();
    return surface_;
}

std::shared_ptr<AVSharedMemory> AVCodecVideoEncoderImpl::GetInputBuffer(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        nullptr, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->GetInputBuffer(index);
}

int32_t AVCodecVideoEncoderImpl::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->QueueInputBuffer(index, info, flag);
}

std::shared_ptr<AVSharedMemory> AVCodecVideoEncoderImpl::GetOutputBuffer(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        nullptr, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->GetOutputBuffer(index);
}

int32_t AVCodecVideoEncoderImpl::GetOutputFormat(Format &format)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->GetOutputFormat(format);
}

int32_t AVCodecVideoEncoderImpl::ReleaseOutputBuffer(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->ReleaseOutputBuffer(index);
}

int32_t AVCodecVideoEncoderImpl::SetParameter(const Format &format)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");

    AVCODEC_SYNC_TRACE;
    return codecService_->SetParameter(format);
}

int32_t AVCodecVideoEncoderImpl::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, 
        AVCS_ERR_INVALID_OPERATION, "Codec service is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, 
        AVCS_ERR_INVALID_VAL, "Callback is nullptr");
        
    AVCODEC_SYNC_TRACE;
    return codecService_->SetCallback(callback);
}
} // namespace Media
} // namespace OHOS
