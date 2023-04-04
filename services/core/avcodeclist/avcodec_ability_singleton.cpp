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

#include "avcodec_ability_singleton.h"
#include "media_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecAbilitySingleton"};
}

namespace OHOS {
namespace Media {
AVCodecAbilitySingleton& AVCodecAbilitySingleton::GetInstance()
{
    static AVCodecAbilitySingleton instance;
    return instance;
}

AVCodecAbilitySingleton::AVCodecAbilitySingleton()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecAbilitySingleton::~AVCodecAbilitySingleton()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void AVCodecAbilitySingleton::RegisterCapabilitys(const std::vector<CapabilityData> &capaArray)
{
    std::lock_guard<std::mutex> lock(mutex_);
    capabilityDataArray_.insert(capabilityDataArray_.end(), capaArray.begin(),
        capaArray.end());
    MEDIA_LOGD("RegisterCapability success");
}

std::vector<CapabilityData> AVCodecAbilitySingleton::GetCapabilitys()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return capabilityDataArray_;
}
} // namespace Media
} // namespace OHOS
