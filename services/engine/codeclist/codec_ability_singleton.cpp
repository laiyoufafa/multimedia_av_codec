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

#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "codeclist_builder.h"
#include "hcodec_loader.h"
#include "codec_ability_singleton.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecAbilitySingleton"};
}

namespace OHOS {
namespace Media {
std::unordered_map<CodecType, std::shared_ptr<CodecListBase>> GetCodecLists()
{
    std::unordered_map<CodecType, std::shared_ptr<CodecListBase>> codecLists;
    std::shared_ptr<CodecListBase> vcodecList = std::make_shared<VideoCodecList>();
    codecLists.insert(std::make_pair(CodecType::AVCODEC_VIDEO_CODEC, vcodecList));
    std::shared_ptr<CodecListBase> acodecList = std::make_shared<AudioCodecList>();
    codecLists.insert(std::make_pair(CodecType::AVCODEC_AUDIO_CODEC, acodecList));
    return codecLists;
}

CodecAbilitySingleton &CodecAbilitySingleton::GetInstance()
{
    static CodecAbilitySingleton instance;
    AVCODEC_LOGI("Get Codec ability singleton success");
    return instance;
}

CodecAbilitySingleton::CodecAbilitySingleton()
{
    std::vector<CapabilityData> capaArray;
    if (HCodecLoader::GetCapabilityList(capaArray) == AVCS_ERR_OK) {
        RegisterCapabilityArray(capaArray, CodecType::AVCODEC_HCODEC);
    }
    std::unordered_map<CodecType, std::shared_ptr<CodecListBase>> codecLists = GetCodecLists();
    for (auto iter = codecLists.begin(); iter != codecLists.end(); iter++) {
        CodecType codecType = iter->first;
        std::vector<CapabilityData> capaArray;
        int32_t ret = iter->second->GetCapabilityList(capaArray);
        if (ret == AVCS_ERR_OK) {
            RegisterCapabilityArray(capaArray, codecType);
        }
    }
    AVCODEC_LOGI("Capability instances create successful");
}

CodecAbilitySingleton::~CodecAbilitySingleton()
{
    AVCODEC_LOGI("Capability instances destroy successful");
}

void CodecAbilitySingleton::RegisterCapabilityArray(const std::vector<CapabilityData> &capaArray, CodecType codecType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t beginIdx = capabilityDataArray_.size();
    capabilityDataArray_.insert(capabilityDataArray_.end(), capaArray.begin(), capaArray.end());
    for (auto iter = capaArray.begin(); iter != capaArray.end(); iter++) {
        std::string mimeType = (*iter).mimeType;
        std::vector<size_t> idxVec;
        if (mimeCapIdxMap_.find(mimeType) == mimeCapIdxMap_.end()) {
            mimeCapIdxMap_.insert(std::make_pair(mimeType, idxVec));
        }
        mimeCapIdxMap_.at(mimeType).emplace_back(beginIdx);
        nameCodecTypeMap_.insert(std::make_pair((*iter).codecName, codecType));
        beginIdx++;
    }
    AVCODEC_LOGD("Register capability successful");
}

std::vector<CapabilityData> CodecAbilitySingleton::GetCapabilityArray()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return capabilityDataArray_;
}

std::unordered_map<std::string, CodecType> CodecAbilitySingleton::GetNameCodecTypeMap()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return nameCodecTypeMap_;
}

std::unordered_map<std::string, std::vector<size_t>> CodecAbilitySingleton::GetMimeCapIdxMap()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return mimeCapIdxMap_;
}
} // namespace Media
} // namespace OHOS
