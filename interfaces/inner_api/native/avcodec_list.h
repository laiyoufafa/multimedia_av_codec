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

#ifndef AVCODEC_LIST_H
#define AVCODEC_LIST_H

#include <cstdint>
#include <memory>
#include "avcodec_info.h"

namespace OHOS {
namespace Media {
class AVCodecList {
public:
    virtual ~AVCodecList() = default;

    /**
     * @brief Find the supported video decoder name by format(must contains video MIME).
     * @param format Indicates a media description which contains required video decoder capability.
     * @return  Returns video decoder name, if not find, return empty string.
     * @since 10
     * @version 1.0
     */
    virtual std::string FindVideoDecoder(const Format &format) = 0;

    /**
     * @brief Find the supported video encoder name by format(must contains video MIME).
     * @param format Indicates a media description which contains required video encoder capability.
     * @return  Returns video encoder name, if not find, return empty string.
     * @since 10
     * @version 1.0
     */
    virtual std::string FindVideoEncoder(const Format &format) = 0;

    /**
     * @brief Find the supported audio decoder name by format(must contains audio MIME).
     * @param format Indicates a media description which contains required audio decoder capability.
     * @return  Returns audio decoder name, if not find, return empty string.
     * @since 10
     * @version 1.0
     */
    virtual std::string FindAudioDecoder(const Format &format) = 0;

    /**
     * @brief Find the supported audio encoder name by format(must contains audio MIME).
     * @param format Indicates a media description which contains required audio encoder capability.
     * @return  Returns audio encoder name, if not find, return empty string.
     * @since 10
     * @version 1.0
     */
    virtual std::string FindAudioEncoder(const Format &format) = 0;

    /**
     * @brief Get the capabilities by codec name
     * @param codeName Codec name
     * @return Returns an array of supported video decoder capability, if not find, return invalid CapabilityData.
     * @since 10
     * @version 1.0
     */
    virtual CapabilityData GetCapabilityData(std::string codecName) = 0;

};

class __attribute__((visibility("default"))) AVCodecListFactory {
public:
#ifdef UNSUPPORT_CODEC
    static std::shared_ptr<AVCodecList> CreateAVCodecList()
    {
        return nullptr;
    }
#else
    static std::shared_ptr<AVCodecList> CreateAVCodecList();
#endif
private:
    AVCodecListFactory() = default;
    ~AVCodecListFactory() = default;
};

} // namespace Media
} // namespace OHOS
#endif // AVCODEC_LIST_H
