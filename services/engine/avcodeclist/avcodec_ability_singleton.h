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

#ifndef AVCODEABILITY_SINGLETON_H
#define AVCODEABILITY_SINGLETON_H

#include <mutex>
#include "avcodec_info.h"
namespace OHOS {
namespace Media {
class __attribute__((visibility("default"))) AVCodecAbilitySingleton : public NoCopyable {
public:
    ~AVCodecAbilitySingleton();
    static AVCodecAbilitySingleton& GetInstance();
    bool RegisterCapabilitys(const std::vector<CapabilityData> &capaArray);
    std::vector<CapabilityData> GetCapabilitys();

private:
    AVCodecAbilitySingleton();
    std::vector<CapabilityData> capabilityDataArray_;
    std::mutex mutex_;
};
} // namespace Media
} // namespace OHOS
#endif // AVCODEABILITY_SINGLETON_H