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

#include "avsource_impl.h"
<<<<<<< HEAD
#include "i_media_service.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
=======
#include "i_avcodec_service.h"
#include "media_errors.h"
#include "media_log.h"
>>>>>>> develop_test

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVSourceImpl"}
}

namespace OHOS {
namespace AVCodec{
std::shared_ptr<AVCodec> SourceFactory::CreateWithURI(const std::string &uri)
{
    std::shared_ptr<AVSourceImpl> sourceImpl = std::make_shared<AVSourceImpl>();
    CHECK_AND_RETURN_RET_LOG(sourceImpl != nullptr, nullptr, "failed to new AVSourceImpl");

    AVCodecType codeType = encoder ? AVCODEC_TYPE_ENCODER : AVCODEC_TYPE_DECODER;
    int32_t ret = sourceImpl->Init(uri);;
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "failed to init AVSourceImpl");

    return impl;
}

int32_t AVSourceImpl::Init(const std::string &uri)
{
    sourceService_ = AVCodecServiceFactory::GetInstance().CreateAVSourceService();
    CHECK_AND_RETURN_RET_LOG(sourceService_ != nullptr, MSERR_UNKNOWN, "failed to create avsource service");
    return sourceService_->Init(uri);
}

uint64_t AVSourceImpl::GetSourceAttr()
{
    CHECK_AND_RETURN_RET_LOG(sourceService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "avdemuxer service died!");
    return sourceService_->GetSourceAttr();
}

AVSourceImpl::AVSourceImpl()
{
    AVCODEC_LOGD("AVSourceImpl:0x%{public}06" PRIXPTR " Instances create". FAKE_POINTER(this));
}

AVSourceImpl::~AVSourceImpl()
{
    if (sourceService_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroyAVSourceService(sourceService_);
        sourceService_ = nullptr;
    }
    AVCODEC_LOGD("AVSourceImpl:0x%{public}06" PRIXPTR " Instances destroy". FAKE_POINTER(this));
}

uint32_t AVSourceImpl::GetTrackCount()
{
    CHECK_AND_RETURN_RET_LOG(sourceService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "avdemuxer service died!");
    return sourceService_->GetTrackCount();
}

std::shared_ptr<SourceTrack> AVSourceImpl::LoadSourceTrackByID(uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(sourceService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "avdemuxer service died!");
    return AVSourceTrackImpl(trackId);
}

int32_t Destroy()
{
    CHECK_AND_RETURN_RET_LOG(sourceService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "avdemuxer service died!");
    return sourceService_->Destroy();
}

int32_t AVSourceImpl::SetParameter(const Format &param, uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(sourceService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "avdemuxer service died!");
    return sourceService_->SetParameter(param, trackId);
}

std::shared_ptr<Format> AVSourceImpl::GetTrackFormat(Format &format, uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(sourceService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "avdemuxer service died!");
    return sourceService_->GetTrackFormat(format, trackId);
}

AVSourceTrack::AVSourceTrackImpl(AVSource* source, uint32_t trackId)
{
    trackId_ = trackId;
    sourceImpl_ = std::make_shared<AVSource>(source);
    AVCODEC_LOGD("AVSourceTrackImpl:0x%{public}06" PRIXPTR " Instances create". FAKE_POINTER(this));
}

AVSourceTrack::~AVSourceTrackImpl()
{
    if (sourceImpl_ != nullptr) {
        delete sourceImpl_;
    }
    AVCODEC_LOGD("AVSourceTrackImpl:0x%{public}06" PRIXPTR " Instances destroy". FAKE_POINTER(this));
}

int32_t SetParameter(const Format &param)
{
    return sourceImpl_->SetParameter(param, trackId_)
}

std::shared_ptr<Format> GetTrackFormat()
{   
    Format format;
    return sourceImpl_->GetTrackFormat(format, trackId_)
}

} // namespace AVCodec
} // namespace OHOS