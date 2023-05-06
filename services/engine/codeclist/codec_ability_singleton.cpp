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

#include "codec_ability_singleton.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "codeclist_builder.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecAbilitySingleton"};
}

namespace OHOS {
namespace Media {

std::vector<std::shared_ptr<CodecListBase>> GetCodecLists(){
    std::vector<std::shared_ptr<CodecListBase>> codecLists;
    std::shared_ptr<CodecListBase> vcodecBuilder = std::make_shared<VideoCodecList>();
    codecLists.emplace_back(vcodecBuilder);
    std::shared_ptr<CodecListBase> acodecBuilder = std::make_shared<AudioCodecList>();
    codecLists.emplace_back(acodecBuilder);
    return codecLists;
}

CodecAbilitySingleton& CodecAbilitySingleton::GetInstance()
{
    static CodecAbilitySingleton instance;
    return instance;
}

CodecAbilitySingleton::CodecAbilitySingleton()
{
    std::vector<std::shared_ptr<CodecListBase>> codecLists = GetCodecLists();
    auto iter = codecLists.begin();
    while (iter != codecLists.end()) {
        std::vector<CapabilityData> capaArray;
        int32_t ret = (*iter)->GetCapabilityList(capaArray);
        if(ret == AVCS_ERR_OK) {
            RegisterCapabilityArray(capaArray);
        }
    }
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecAbilitySingleton::~CodecAbilitySingleton()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void CodecAbilitySingleton::RegisterCapabilityArray(const std::vector<CapabilityData> &capaArray)
{
    std::lock_guard<std::mutex> lock(mutex_);
    capabilityDataArray_.insert(capabilityDataArray_.end(), capaArray.begin(),
        capaArray.end());
    AVCODEC_LOGD("RegisterCapability success");
}

std::vector<CapabilityData> CodecAbilitySingleton::GetCapabilityArray()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return capabilityDataArray_;
}
// #endif
} // namespace Media
} // namespace OHOS
