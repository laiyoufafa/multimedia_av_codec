/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef CODECLIST_XML_PARSER_H
#define CODECLIST_XML_PARSER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "avcodec_info.h"

namespace OHOS {
namespace Media {
enum NodeName : int32_t {
    CODECS,
    AUDIO_CODECS,
    VIDEO_CODECS,
    AUDIO_DECODER,
    AUDIO_ENCODER,
    VIDEO_DECODER,
    VIDEO_ENCODER,
    UNKNOWN
};

class CodeclistXmlParser {
public:
    CodeclistXmlParser();
    ~CodeclistXmlParser();
    bool LoadConfiguration();
    bool Parse();
    void Destroy();
    std::vector<CapabilityData> GetCapabilityDataArray() const;

private:
    static bool IsNumberArray(const std::vector<std::string> &strArray);
    static bool TransStrAsRange(const std::string &str, Range &range);
    static bool TransStrAsSize(const std::string &str, ImgSize &size);
    static std::vector<int32_t> TransMapAsIntegerArray(const std::unordered_map<std::string, int> &capabilityMap,
                                                const std::vector<std::string> &spilt);
    static std::vector<int32_t> TransStrAsIntegerArray(const std::vector<std::string> &spilt);
    static bool SpiltKeyList(const std::string &str, const std::string &delim, std::vector<std::string> &spilt);
    static bool SetCapabilityStringData(std::unordered_map<std::string, std::string&> dataMap,
                                const std::string &capabilityKey, const std::string &capabilityValue);
    static bool SetCapabilityIntData(std::unordered_map<std::string, int32_t&> dataMap,
                                const std::string &capabilityKey, const std::string &capabilityValue);
    static bool SetCapabilityBoolData(std::unordered_map<std::string, bool&> dataMap,
                                const std::string &capabilityKey, const std::string &capabilityValue);
    static bool SetCapabilityRangeData(std::unordered_map<std::string, Range&> dataMap,
                                const std::string &capabilityKey, const std::string &capabilityValue);
    static bool SetCapabilityVectorData(std::unordered_map<std::string, std::vector<int32_t>&> dataMap,
                                const std::string &capabilityKey, const std::string &capabilityValue);
    static bool SetCapabilitySizeData(std::unordered_map<std::string, ImgSize&> dataMap,
                                const std::string &capabilityKey, const std::string &capabilityValue);
    static bool SetCapabilityHashRangeData(std::unordered_map<std::string, std::map<ImgSize, Range>&> dataMap,
                                const std::string &capabilityKey, const std::string &capabilityValue);
    static bool SetCapabilityHashVectorData(std::unordered_map<std::string, std::map<int32_t, std::vector<int32_t>>&> dataMap,
                                const std::string &capabilityKey, const std::string &capabilityValue);

    static NodeName GetNodeNameAsInt(xmlNode *node);
    bool SetCapabilityData(CapabilityData &data, const std::string &capabilityKey,
                            const std::string &capabilityValue) const;
    bool ParseInternal(xmlNode *node);
    bool ParseData(xmlNode *node);
    std::vector<CapabilityData> capabilityDataArray_;
    xmlDoc *mDoc_ = nullptr;
    std::vector<std::string> capabilityKeys_;
};
} // namespace Media
} // namespace OHOS
#endif // CODECLIST_XML_PARSER_H