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

#ifndef AVCODECLIST_CORE_H
#define AVCODECLIST_CORE_H

#include <mutex>
#include "nocopyable.h"

namespace OHOS {
namespace Media {
class __attribute__((visibility("default"))) AVCodecListCore : public NoCopyable {
public:
    AVCodecListCore();
    ~AVCodecListCore();
    std::string FindVideoEncoder(const Format &format);
    std::string FindVideoDecoder(const Format &format);
    std::string FindAudioEncoder(const Format &format);
    std::string FindAudioDecoder(const Format &format);
    CapabilityData GetCapabilityData(std::string name);

private:
    bool CheckBitrate(const Format &format, const CapabilityData &data);
    bool CheckVideoResolution(const Format &format, const CapabilityData &data);
    bool CheckVideoPixelFormat(const Format &format, const CapabilityData &data);
    bool CheckVideoFrameRate(const Format &format, const CapabilityData &data);
    bool CheckAudioChannel(const Format &format, const CapabilityData &data);
    bool CheckAudioSampleRate(const Format &format, const CapabilityData &data);
    bool IsVideoCapSupport(const Format &format, const CapabilityData &data);
    bool IsAudioCapSupport(const Format &format, const CapabilityData &data);
    std::string FindCodec(const Format &format, const AVCodecType &codecType);
    std::mutex mutex_;
};
} // namespace Media
} // namespace OHOS
#endif // AVCODECLIST_CORE_H
