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
#ifndef AVCODECLISTBASE_H
#define AVCODECLISTBASE_H

#include <string>
#include "avcodec_info.h"

namespace OHOS {
namespace Media {
class CodecListBase {
public:
    virtual ~CodecListBase() = default;
    virtual int32_t GetCapabilityList(std::vector<CapabilityData> &caps) = 0;
};
} // namespace Media
} // namespace OHOS
#endif // CODECLISTBASE_H