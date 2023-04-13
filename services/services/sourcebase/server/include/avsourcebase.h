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
#ifndef AVSOURCEBASE_H
#define AVSOURCEBASE_H

#include <memory>
#include <cstdint>
#include <string>
#include "avcodec_common.h"
#include "libavformat/avformat.h"

namespace OHOS {
namespace MediaAVCodec {
class AVSourceBase {
public:
    static std::shared_ptr<AVSourceBase> Create(const std::string& uri);
    virtual int32_t GetTrackCount() = 0;
    virtual int32_t Destroy() = 0;
    virtual int32_t SetParameter(const Format &param, uint32_t trackId) = 0;
    virtual int32_t GetTrackFormat(Format &format, uint32_t trackId) = 0;
    virtual size_t GetSourceAttr() = 0;

private:
    std::shared_ptr<AVFormatContext> formatContext_;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // AVSOURCEBASE_H