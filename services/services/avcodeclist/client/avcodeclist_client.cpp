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

#include "avcodeclist_client.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecListClient"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVCodecListClient> AVCodecListClient::Create(const sptr<IStandardAVCodecListService> &ipcProxy)
{
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, nullptr, "ipcProxy is nullptr..");

    std::shared_ptr<AVCodecListClient> codecList = std::make_shared<AVCodecListClient>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(codecList != nullptr, nullptr, "failed to new AVCodecListClient..");

    return codecList;
}

AVCodecListClient::AVCodecListClient(const sptr<IStandardAVCodecListService> &ipcProxy)
    : codecListProxy_(ipcProxy)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecListClient::~AVCodecListClient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (codecListProxy_ != nullptr) {
        (void)codecListProxy_->DestroyStub();
    }
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void AVCodecListClient::MediaServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    codecListProxy_ = nullptr;
}

std::string AVCodecListClient::FindDecoder(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecListProxy_ != nullptr, "", "codeclist service does not exist.");
    return codecListProxy_->FindDecoder(format);
}

std::string AVCodecListClient::FindEncoder(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecListProxy_ != nullptr, "", "codeclist service does not exist.");
    return codecListProxy_->FindEncoder(format);
}

CapabilityData AVCodecListClient::CreateCapability(std::string codecName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CapabilityData capData;
    CHECK_AND_RETURN_RET_LOG(codecListProxy_ != nullptr, capData,
        "codeclist service does not exist.");
    return codecListProxy_->CreateCapability(codecName);
}
} // namespace Media
} // namespace OHOS
