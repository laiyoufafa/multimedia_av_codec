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

#ifndef AVCODEC_DUMP_UTILS_H
#define AVCODEC_DUMP_UTILS_H

#include <map>
#include <string>
#include <vector>
#include "format.h"

namespace OHOS {
namespace Media {
class __attribute__((visibility("default"))) AVCodecDumpControler {
public:
    int32_t AddInfo(const uint32_t dumpIdx, const std::string &name, const std::string &value = "");
    int32_t AddInfoFromFormat(const uint32_t dumpIdx, const Format &format,
                              const std::string_view &key, const std::string &name);
    int32_t AddInfoFromFormatWithMapping(const uint32_t dumpIdx, const Format &format, const std::string_view &key,
                                         const std::string &name, std::map<int32_t, const std::string> mapping);
    int32_t GetDumpString(std::string &dumpString);

private:
    uint32_t GetLevel(const uint32_t dumpIdx);
    std::map<uint32_t, std::pair<std::string, std::string>> dumpInfoMap_;   // <dumpIdx, <name, value>>
    std::vector<uint32_t> length_ = std::vector<uint32_t>(4, 0);
};
} // namespace OHOS
} // namespace Media
#endif // AVCODEC_DUMP_UTILS_H