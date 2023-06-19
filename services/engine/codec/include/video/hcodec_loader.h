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

#ifndef HCODEC_LOADER_H
#define HCODEC_LOADER_H
#include "codecbase.h"
#include "codeclistbase.h"
namespace OHOS {
namespace Media {
class HCodecLoader {
public:
    static std::shared_ptr<CodecBase> CreateByName(const std::string &name);
    static int32_t GetCapabilityList(std::vector<CapabilityData> &caps);

private:
    HCodecLoader() = default;
    ~HCodecLoader() = default;
    using CreateByNameFuncType = void (*)(const std::string &name, std::shared_ptr<CodecBase> &codec);
    using GetCapabilityFuncType = int32_t (*)(std::vector<CapabilityData> &caps);
    std::shared_ptr<void> codecHandle_ {nullptr};
    CreateByNameFuncType createFunc_;
    GetCapabilityFuncType getCapsFunc_;
    static HCodecLoader &GetInstance();
    int32_t Init();
    std::shared_ptr<CodecBase> Create(const std::string &name);
    int32_t GetCaps(std::vector<CapabilityData> &caps);
};
} // namespace Media
} // namespace OHOS
#endif