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

#include "avcodec_list_impl.h"
#include "media_log.h"
#include "media_errors.h"
#include "i_media_service.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecListImpl"};
}
namespace OHOS {
namespace Media {
std::shared_ptr<AVCodecList> AVCodecListFactory::CreateAVCodecList()
{
    std::shared_ptr<AVCodecListImpl> impl = std::make_shared<AVCodecListImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to new AVCodecListImpl");

    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "failed to init AVCodecListImpl");

    return impl;
}

int32_t AVCodecListImpl::Init()
{
    codecListService_ = MediaServiceFactory::GetInstance().CreateAVCodecListService();
    CHECK_AND_RETURN_RET_LOG(codecListService_ != nullptr, MSERR_UNKNOWN, "failed to create AVCodecList service");
    return MSERR_OK;
}

AVCodecListImpl::AVCodecListImpl()
{
    MEDIA_LOGD("AVCodecListImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecListImpl::~AVCodecListImpl()
{
    if (codecListService_ != nullptr) {
        (void)MediaServiceFactory::GetInstance().DestroyAVCodecListService(codecListService_);
        codecListService_ = nullptr;
    }
    MEDIA_LOGD("AVCodecListImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

std::string AVCodecListImpl::FindVideoDecoder(const Format &format)
{
    return codecListService_->FindVideoDecoder(format);
}

std::string AVCodecListImpl::FindVideoEncoder(const Format &format)
{
    return codecListService_->FindVideoEncoder(format);
}

std::string AVCodecListImpl::FindAudioDecoder(const Format &format)
{
    return codecListService_->FindAudioDecoder(format);
}

std::string AVCodecListImpl::FindAudioEncoder(const Format &format)
{
    return codecListService_->FindAudioEncoder(format);
}

CapabilityData AVCodecListImpl::GetCapabilityData(std::string codecName)
{
    return codecListService_->GetCapabilityData(codecName);
}

} // namespace Media
} // namespace OHOS