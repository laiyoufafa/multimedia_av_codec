/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef IMPL_NAPITEST_H
#define IMPL_NAPITEST_H

#include <cinttypes>
#include <string>
#include <memory>
#include <vector>
#include <cmath>
#include <map>
#include <any>
#include "avcodec_log.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "FCodec"};
}

using NUMBER_TYPE_1 = int32_t;
using NUMBER_TYPE_2 = int64_t;
using NUMBER_TYPE_3 = int64_t;

namespace napitest {
class AVFileDescriptor {
public:
    NUMBER_TYPE_1 fd;
    NUMBER_TYPE_2 offset;
    NUMBER_TYPE_3 length;
};

bool setSurfaceID(std::string& surfaceID, AVFileDescriptor& inJsFp, int32_t& outFd, uint32_t& outErrCode, std::string& out);
bool getSurfaceID(uint32_t& outErrCode, std::string& out);
}
#endif // IMPL_NAPITEST_H
