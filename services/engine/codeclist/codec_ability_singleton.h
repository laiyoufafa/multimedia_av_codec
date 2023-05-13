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

#ifndef CODEABILITY_SINGLETON_H
#define CODEABILITY_SINGLETON_H

#include <mutex>
#include "avcodec_info.h"

namespace OHOS {
namespace Media {
class __attribute__((visibility("default"))) CodecAbilitySingleton : public NoCopyable {
public:
    ~CodecAbilitySingleton();
    static CodecAbilitySingleton& GetInstance();
    void RegisterCapabilityArray(const std::vector<CapabilityData> &capaArray);
    std::vector<CapabilityData> GetCapabilityArray();

private:
    CodecAbilitySingleton();
    bool ParseCodecXml();
    std::vector<CapabilityData> capabilityDataArray_;
    std::mutex mutex_;
    bool isParsered_{false};
};
} // namespace Media
} // namespace OHOS
#endif // CODEABILITY_SINGLETON_H