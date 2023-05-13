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
#include "avcodec_log.h"
#include "string_ex.h"
#include "codeclist_xml_parser.h"
using namespace std;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodeclistXmlParser"};
constexpr int32_t PAIR_LENGTH = 2;
const string AVCODEC_CAPS_FILE = "/etc/codec/avcodec_caps.xml";
} // namespace
namespace OHOS {
namespace Media {
const std::unordered_map<string, int> VIDEO_PROFILE_MAP = {
    // H263
    {"H263BackwardCompatible", H263_PROFILE_BACKWARD_COMPATIBLE},
    {"H263Baseline", H263_PROFILE_BASELINE},
    {"H263H320Coding", H263_PROFILE_H320_CODING},
    {"H263HighCompression", H263_PROFILE_HIGH_COMPRESSION},
    {"H263HighLatency", H263_PROFILE_HIGH_LATENCY},
    {"H263ISWV2", H263_PROFILE_ISW_V2},
    {"H263ISWV3", H263_PROFILE_ISW_V3},
    {"H263Interlace", H263_PROFILE_INTERLACE},
    {"H263Internet", H263_PROFILE_INTERNET},
    // H264
    {"AVCBaseline", AVC_PROFILE_BASELINE},
    {"AVCHighCompression", AVC_PROFILE_CONSTRAINED_BASELINE},
    {"AVCConstrainedHigh", AVC_PROFILE_CONSTRAINED_HIGH},
    {"AVCExtended", AVC_PROFILE_EXTENDED},
    {"AVCHigh", AVC_PROFILE_HIGH},
    {"AVCHigh10", AVC_PROFILE_HIGH_10},
    {"AVCHigh422", AVC_PROFILE_HIGH_422},
    {"AVCHigh444", AVC_PROFILE_HIGH_444},
    {"AVCMain", AVC_PROFILE_MAIN},
    // H265
    {"HEVCMain", HEVC_PROFILE_MAIN},
    {"HEVCMain10", HEVC_PROFILE_MAIN_10},
    {"HEVCMainStill", HEVC_PROFILE_MAIN_STILL},
    // MPEG2
    {"MPEG2_422", MPEG2_PROFILE_422},
    {"MPEG2High", MPEG2_PROFILE_HIGH},
    {"MPEG2Main", MPEG2_PROFILE_MAIN},
    {"MPEG2SNR", MPEG2_PROFILE_SNR},
    {"MPEG2Simple", MPEG2_PROFILE_SIMPLE},
    {"MPEG2Spatial", MPEG2_PROFILE_SPATIAL},
    // MPEG4
    {"MPEG4AdvancedCoding", MPEG4_PROFILE_ADVANCED_CODING},
    {"MPEG4AdvancedCore", MPEG4_PROFILE_ADVANCED_CORE},
    {"MPEG4AdvancedRealTime", MPEG4_PROFILE_ADVANCED_REAL_TIME},
    {"MPEG4AdvancedScalable", MPEG4_PROFILE_ADVANCED_SCALABLE},
    {"MPEG4AdvancedSimple", MPEG4_PROFILE_ADVANCED_SIMPLE},
    {"MPEG4BasicAnimated", MPEG4_PROFILE_BASIC_ANIMATED},
    {"MPEG4Core", MPEG4_PROFILE_CORE},
    {"MPEG4CoreScalable", MPEG4_PROFILE_CORE_SCALABLE},
    {"MPEG4Hybrid", MPEG4_PROFILE_HYBRID},
    {"MPEG4Main", MPEG4_PROFILE_MAIN},
    {"MPEG4Nbit", MPEG4_PROFILE_NBIT},
    {"MPEG4ScalableTexture", MPEG4_PROFILE_SCALABLE_TEXTURE},
    {"MPEG4Simple", MPEG4_PROFILE_SIMPLE},
    {"MPEG4SimpleFBA", MPEG4_PROFILE_SIMPLE_FBA},
    {"MPEG4SimpleFace", MPEG4_PROFILE_SIMPLE_FACE},
    {"MPEG4SimpleScalable", MPEG4_PROFILE_SIMPLE_SCALABLE},
    // VP8
    {"VP8Main", VP8_PROFILE_MAIN},
};

const std::unordered_map<string, int> AUDIO_PROFILE_MAP = {
    {"AAC_LC", AAC_PROFILE_LC},     {"AAC_ELD", AAC_PROFILE_ELD},     {"AAC_ERLC", AAC_PROFILE_ERLC},
    {"AAC_HE", AAC_PROFILE_HE},     {"AAC_HE_V2", AAC_PROFILE_HE_V2}, {"AAC_LD", AAC_PROFILE_LD},
    {"AAC_Main", AAC_PROFILE_MAIN},
};

const std::unordered_map<string, int> VIDEO_LEVEL_MAP = {
    // H264
    {"AVC_LEVEL_1", AVC_LEVEL_1},
    {"AVC_LEVEL_1b", AVC_LEVEL_1b},
    {"AVC_LEVEL_11", AVC_LEVEL_11},
    {"AVC_LEVEL_12", AVC_LEVEL_12},
    {"AVC_LEVEL_13", AVC_LEVEL_13},
    {"AVC_LEVEL_2", AVC_LEVEL_2},
    {"AVC_LEVEL_21", AVC_LEVEL_21},
    {"AVC_LEVEL_22", AVC_LEVEL_22},
    {"AVC_LEVEL_3", AVC_LEVEL_3},
    {"AVC_LEVEL_31", AVC_LEVEL_31},
    {"AVC_LEVEL_32", AVC_LEVEL_32},
    {"AVC_LEVEL_4", AVC_LEVEL_4},
    {"AVC_LEVEL_41", AVC_LEVEL_41},
    {"AVC_LEVEL_42", AVC_LEVEL_42},
    {"AVC_LEVEL_5", AVC_LEVEL_5},
    {"AVC_LEVEL_51", AVC_LEVEL_51},
    // H265
    {"HEVC_LEVEL_1", HEVC_LEVEL_1},
    {"HEVC_LEVEL_2", HEVC_LEVEL_2},
    {"HEVC_LEVEL_21", HEVC_LEVEL_21},
    {"HEVC_LEVEL_3", HEVC_LEVEL_3},
    {"HEVC_LEVEL_31", HEVC_LEVEL_31},
    {"HEVC_LEVEL_4", HEVC_LEVEL_4},
    {"HEVC_LEVEL_41", HEVC_LEVEL_41},
    {"HEVC_LEVEL_5", HEVC_LEVEL_5},
    {"HEVC_LEVEL_51", HEVC_LEVEL_51},
    {"HEVC_LEVEL_52", HEVC_LEVEL_52},
    {"HEVC_LEVEL_6", HEVC_LEVEL_6},
    {"HEVC_LEVEL_61", HEVC_LEVEL_61},
    {"HEVC_LEVEL_62", HEVC_LEVEL_62},

    {"MPEG2_LEVEL_LL", MPEG2_LEVEL_LL},
    {"MPEG2_LEVEL_ML", MPEG2_LEVEL_ML},
    {"MPEG2_LEVEL_H14", MPEG2_LEVEL_H14},
    {"MPEG2_LEVEL_HL", MPEG2_LEVEL_HL},

    {"MPEG4_LEVEL_0", MPEG4_LEVEL_0},
    {"MPEG4_LEVEL_0B", MPEG4_LEVEL_0B},
    {"MPEG4_LEVEL_1", MPEG4_LEVEL_1},
    {"MPEG4_LEVEL_2", MPEG4_LEVEL_2},
    {"MPEG4_LEVEL_3", MPEG4_LEVEL_3},
    {"MPEG4_LEVEL_4", MPEG4_LEVEL_4},
    {"MPEG4_LEVEL_4A", MPEG4_LEVEL_4A},
    {"MPEG4_LEVEL_5", MPEG4_LEVEL_5},
};

const std::unordered_map<string, int> VIDEO_FORMAT_MAP = {
    {"YUV420P", YUV420P}, {"NV12", NV12}, {"NV21", NV21}, {"RGBA", RGBA}, {"BGRA", BGRA},
};

const std::unordered_map<string, int> AUDIO_BITDEPTH_MAP = {
    {"U8", SAMPLE_U8},
    {"S16LE", SAMPLE_S16LE},
    {"S24LE", SAMPLE_S24LE},
    {"S32LE", SAMPLE_S32LE},
};

const std::unordered_map<string, int> BITRATE_MODE_MAP = {
    {"CBR", CBR},
    {"VBR", VBR},
    {"CQ", CQ},
};

const std::unordered_map<string, AVCodecType> CODEC_TYPE_MAP = {
    {"VIDEO_ENCODER", AVCODEC_TYPE_VIDEO_ENCODER},
    {"VIDEO_DECODER", AVCODEC_TYPE_VIDEO_DECODER},
    {"AUDIO_ENCODER", AVCODEC_TYPE_AUDIO_ENCODER},
    {"AUDIO_DECODER", AVCODEC_TYPE_AUDIO_DECODER},
};

CodeclistXmlParser::CodeclistXmlParser()
{
    capabilityKeys_ = {
        "codecName",
        "codecType",
        "mimeType",
        "isVendor",
        "maxInstance",
        "bitrate",
        "channels",
        "complexity",
        "alignment",
        "width",
        "height",
        "frameRate",
        "encodeQuality",
        "blockPerFrame",
        "blockPerSecond",
        "blockSize",
        "sampleRate",
        "pixFormat",
        "bitDepth",
        "profiles",
        "bitrateMode",
        "profileLevelsMap",
        "measuredFrameRate",
        "supportSwapWidthHeight",
    };
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodeclistXmlParser::~CodeclistXmlParser()
{
    Destroy();
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

bool CodeclistXmlParser::LoadConfiguration()
{
    mDoc_ = xmlReadFile(AVCODEC_CAPS_FILE.c_str(), nullptr, 0);
    if (mDoc_ == nullptr) {
        AVCODEC_LOGE("AVCodec xmlReadFile failed");
        return false;
    }
    return true;
}

bool CodeclistXmlParser::Parse()
{
    xmlNode *root = xmlDocGetRootElement(mDoc_);
    if (root == nullptr) {
        AVCODEC_LOGE("AVCodec xmlDocGetRootElement failed");
        return false;
    }
    return ParseInternal(root);
}

void CodeclistXmlParser::Destroy()
{
    if (mDoc_ != nullptr) {
        xmlFreeDoc(mDoc_);
    }
    return;
}

bool CodeclistXmlParser::ParseInternal(xmlNode *node)
{
    xmlNode *currNode = node;
    for (; currNode != nullptr; currNode = currNode->next) {
        if (currNode->type == XML_ELEMENT_NODE) {
            switch (GetNodeNameAsInt(currNode)) {
                case AUDIO_DECODER:
                case AUDIO_ENCODER:
                case VIDEO_DECODER:
                case VIDEO_ENCODER: {
                    ParseData(currNode);
                    break;
                }
                default: ParseInternal(currNode->children); break;
            }
        }
    }
    return true;
}

bool CodeclistXmlParser::TransStrAsRange(const string &str, Range &range)
{
    if (str == "null" || str == "") {
        AVCODEC_LOGD("str is null");
        return false;
    }
    size_t pos = str.find("-");
    if (pos != str.npos && pos + 1 < str.size()) {
        string head = str.substr(0, pos);
        string tail = str.substr(pos + 1);
        if (!StrToInt(head, range.minVal)) {
            AVCODEC_LOGE("call StrToInt func false, input str is: %{public}s", head.c_str());
            return false;
        }
        if (!StrToInt(tail, range.maxVal)) {
            AVCODEC_LOGE("call StrToInt func false, input str is: %{public}s", tail.c_str());
            return false;
        }
    } else {
        AVCODEC_LOGE("Can not find the delimiter of \"-\" in : %{public}s", str.c_str());
        return false;
    }
    return true;
}

bool CodeclistXmlParser::TransStrAsSize(const string &str, ImgSize &size)
{
    if (str == "null" || str == "") {
        AVCODEC_LOGD("str is null");
        return false;
    }
    size_t pos = str.find("x");
    if (pos != str.npos && pos + 1 < str.size()) {
        string head = str.substr(0, pos);
        string tail = str.substr(pos + 1);
        if (!StrToInt(head, size.width)) {
            AVCODEC_LOGE("call StrToInt func false, input str is: %{public}s", head.c_str());
            return false;
        }
        if (!StrToInt(tail, size.height)) {
            AVCODEC_LOGE("call StrToInt func false, input str is: %{public}s", tail.c_str());
            return false;
        }
    } else {
        AVCODEC_LOGD("Can not find the delimiter of \"x\" in : %{public}s", str.c_str());
        return false;
    }
    return true;
}

std::vector<int32_t> CodeclistXmlParser::TransStrAsIntegerArray(const std::vector<string> &spilt)
{
    std::vector<int32_t> array;
    for (auto iter = spilt.begin(); iter != spilt.end(); iter++) {
        int32_t num = -1;
        if (!StrToInt(*iter, num)) {
            AVCODEC_LOGE("call StrToInt func false, input str is: %{public}s", iter->c_str());
            return array;
        }
        array.push_back(num);
    }
    return array;
}

std::vector<int32_t> CodeclistXmlParser::TransMapAsIntegerArray(const std::unordered_map<string, int> &capMap,
                                                                const std::vector<string> &spilt)
{
    std::vector<int32_t> res;
    for (auto iter = spilt.begin(); iter != spilt.end(); iter++) {
        if (capMap.find(*iter) != capMap.end()) {
            res.emplace_back(capMap.at(*iter));
        } else {
            AVCODEC_LOGD("can not find %{public}s in capabilityMap", iter->c_str());
        }
    }
    return res;
}

bool CodeclistXmlParser::SpiltKeyList(const string &str, const string &delim, std::vector<string> &spilt)
{
    if (str == "") {
        return false;
    }
    string strAddDelim = str;
    if (str.back() != delim.back()) {
        strAddDelim = str + delim;
    }
    size_t size = strAddDelim.size();
    for (size_t i = 0; i < size; ++i) {
        size_t pos = strAddDelim.find(delim, i);
        if (pos != strAddDelim.npos) {
            string s = strAddDelim.substr(i, pos - i);
            spilt.push_back(s);
            i = pos + delim.size() - 1;
        }
    }
    return true;
}

bool CodeclistXmlParser::SetCapabilityStringData(std::unordered_map<string, string &> dataMap,
                                                 const string &capabilityKey, const string &capabilityValue)
{
    AVCODEC_LOGE("Get %{public}s: %{public}s", capabilityKey.c_str(), capabilityValue.c_str());
    dataMap.at(capabilityKey) = capabilityValue;
    return true;
}

bool CodeclistXmlParser::SetCapabilityIntData(std::unordered_map<string, int32_t &> dataMap,
                                              const string &capabilityKey, const string &capabilityValue)
{
    if (capabilityKey == "codecType") {
        if (CODEC_TYPE_MAP.find(capabilityValue) == CODEC_TYPE_MAP.end()) {
            return false;
        }
        dataMap.at(capabilityKey) = CODEC_TYPE_MAP.at(capabilityValue);
        AVCODEC_LOGD("The value of %{public}s in the configuration file is incorrect.", capabilityValue.c_str());
    } else {
        if (!StrToInt(capabilityValue, dataMap.at(capabilityKey))) {
            return false;
        }
    }
    return true;
}

bool CodeclistXmlParser::SetCapabilityBoolData(std::unordered_map<string, bool &> dataMap,
                                               const string &capabilityKey, const string &capabilityValue)
{
    if (capabilityValue == "true") {
        dataMap.at(capabilityKey) = true;
    } else if (capabilityValue == "false") {
        dataMap.at(capabilityKey) = false;
    } else {
        AVCODEC_LOGD("The value of %{public}s in the configuration file is incorrect.", capabilityValue.c_str());
        return false;
    }
    return true;
}

bool CodeclistXmlParser::SetCapabilityRangeData(std::unordered_map<string, Range &> dataMap,
                                                const string &capabilityKey, const string &capabilityValue)
{
    Range range;
    bool ret = TransStrAsRange(capabilityValue, range);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "failed:can not trans %{public}s", capabilityValue.c_str());
    dataMap.at(capabilityKey) = range;
    return true;
}

bool CodeclistXmlParser::SetCapabilitySizeData(std::unordered_map<string, ImgSize &> dataMap,
                                               const string &capabilityKey, const string &capabilityValue)
{
    ImgSize size;
    bool ret = TransStrAsSize(capabilityValue, size);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "failed:can not trans %{public}s", capabilityValue.c_str());
    dataMap.at(capabilityKey) = size;
    return true;
}

bool CodeclistXmlParser::SetCapabilityHashRangeData(std::unordered_map<string, std::map<ImgSize, Range> &> dataMap,
                                                    const string &capabilityKey,
                                                    const string &capabilityValue)
{
    std::map<ImgSize, Range> resolutionFrameRateMap;
    std::vector<string> spilt;
    bool ret = SpiltKeyList(capabilityValue, ",", spilt);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "failed:can not split %{public}s", capabilityValue.c_str());
    for (auto iter = spilt.begin(); iter != spilt.end(); iter++) {
        std::vector<string> resolutionFrameRateVector;
        ImgSize resolution;
        Range frameRate;
        ret = SpiltKeyList(*iter, "@", resolutionFrameRateVector);
        CHECK_AND_RETURN_RET_LOG(ret != false && resolutionFrameRateVector.size() == PAIR_LENGTH, false,
                                 "failed:can not trans %{public}s", iter->c_str());
        if (!(TransStrAsSize(resolutionFrameRateVector[0], resolution) &&
              TransStrAsRange(resolutionFrameRateVector[1], frameRate))) {
            AVCODEC_LOGD("failed:can not trans %{public}s for resolution or frame rate", iter->c_str());
            return false;
        }
        resolutionFrameRateMap.insert(std::make_pair(resolution, frameRate));
    }
    dataMap.at(capabilityKey) = resolutionFrameRateMap;
    return true;
}

bool CodeclistXmlParser::SetCapabilityHashVectorData(
    std::unordered_map<string, std::map<int32_t, std::vector<int32_t>> &> dataMap,
    const string &capabilityKey, const string &capabilityValue)
{
    std::map<int32_t, std::vector<int32_t>> profileLevelsMap;
    std::vector<string> spilt;
    bool ret = SpiltKeyList(capabilityValue, ",", spilt);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "failed:can not split %{public}s", capabilityValue.c_str());
    for (auto iter = spilt.begin(); iter != spilt.end(); iter++) {
        std::vector<string> profileLevelsVector;
        std::vector<string> LevelValsVector;
        int32_t profileKey;
        std::vector<int32_t> array;
        ret = SpiltKeyList(*iter, "@", profileLevelsVector);
        CHECK_AND_RETURN_RET_LOG(ret != false && profileLevelsVector.size() == PAIR_LENGTH, false,
                                 "failed:can not trans %{public}s", iter->c_str());
        if (VIDEO_PROFILE_MAP.find(profileLevelsVector[0]) != VIDEO_PROFILE_MAP.end()) {
            profileKey = VIDEO_PROFILE_MAP.at(profileLevelsVector[0]);
        } else if (AUDIO_PROFILE_MAP.find(profileLevelsVector[0]) != AUDIO_PROFILE_MAP.end()) {
            profileKey = AUDIO_PROFILE_MAP.at(profileLevelsVector[0]);
        } else {
            return false;
        }
        ret = SpiltKeyList(profileLevelsVector[1], "#", LevelValsVector);
        CHECK_AND_RETURN_RET_LOG(ret != false && LevelValsVector.size() > 0, false, "failed:can not trans %{public}s",
                                 profileLevelsVector[1].c_str());
        string lev = LevelValsVector[0];
        if (VIDEO_LEVEL_MAP.find(lev) != VIDEO_LEVEL_MAP.end()) {
            array = TransMapAsIntegerArray(VIDEO_LEVEL_MAP, LevelValsVector);
        }
        profileLevelsMap.insert(std::make_pair(profileKey, array));
        AVCODEC_LOGE("Profile: %{public}s=%{public}d, level0: %{public}s=%{public}d", profileLevelsVector[0].c_str(),
                     profileKey, profileLevelsVector[1].c_str(), array[0]);
    }
    dataMap.at(capabilityKey) = profileLevelsMap;
    return true;
}

bool CodeclistXmlParser::IsNumberArray(const std::vector<string> &strArray)
{
    for (auto iter = strArray.begin(); iter != strArray.end(); iter++) {
        for (char const &c : *iter) {
            if (std::isdigit(c) == 0) {
                return false;
            }
        }
    }
    return true;
}

bool CodeclistXmlParser::SetCapabilityVectorData(std::unordered_map<string, std::vector<int32_t> &> dataMap,
                                                 const string &capabilityKey, const string &capabilityValue)
{
    std::vector<string> spilt;
    std::vector<int32_t> array;
    bool ret = SpiltKeyList(capabilityValue, ",", spilt);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "failed:can not split %{public}s", capabilityValue.c_str());
    if (spilt.size() > 0) {
        string probe = spilt[0];
        if (VIDEO_PROFILE_MAP.find(probe) != VIDEO_PROFILE_MAP.end()) {
            array = TransMapAsIntegerArray(VIDEO_PROFILE_MAP, spilt);
        } else if (AUDIO_PROFILE_MAP.find(probe) != AUDIO_PROFILE_MAP.end()) {
            array = TransMapAsIntegerArray(AUDIO_PROFILE_MAP, spilt);
        } else if (VIDEO_FORMAT_MAP.find(probe) != VIDEO_FORMAT_MAP.end()) {
            array = TransMapAsIntegerArray(VIDEO_FORMAT_MAP, spilt);
        } else if (AUDIO_BITDEPTH_MAP.find(probe) != AUDIO_BITDEPTH_MAP.end()) {
            array = TransMapAsIntegerArray(AUDIO_BITDEPTH_MAP, spilt);
        } else if (BITRATE_MODE_MAP.find(probe) != BITRATE_MODE_MAP.end()) {
            array = TransMapAsIntegerArray(BITRATE_MODE_MAP, spilt);
        } else if (IsNumberArray(spilt)) {
            array = TransStrAsIntegerArray(spilt);
        } else {
            AVCODEC_LOGD("The value of %{public}s in the configuration file is incorrect.", capabilityValue.c_str());
            return false;
        }
        dataMap.at(capabilityKey) = array;
    }
    return true;
}

bool CodeclistXmlParser::SetCapabilityData(CapabilityData &data, const string &capabilityKey,
                                           const string &capabilityValue) const
{
    std::unordered_map<string, string &> capabilityStringMap = {
        {"codecName", data.codecName}, {"mimeType", data.mimeType}};
    std::unordered_map<string, int32_t &> capIntMap =
        {{"codecType", data.codecType}, {"maxInstance", data.maxInstance}};
    std::unordered_map<string, bool &> capabilityBoolMap = {
        {"isVendor", data.isVendor}, {"supportSwapWidthHeight", data.supportSwapWidthHeight}};
    std::unordered_map<string, ImgSize &> capSizeMap = {{"blockSize", data.blockSize}, {"alignment", data.alignment}};
    std::unordered_map<string, std::map<ImgSize, Range> &> capabilityHashRangeMap = {
        {"measuredFrameRate", data.measuredFrameRate}};
    std::unordered_map<string, Range &> capabilityRangeMap = {
        {"bitrate", data.bitrate}, {"complexity", data.complexity}, {"frameRate", data.frameRate},
        {"width", data.width}, {"height", data.height}, {"blockPerFrame", data.blockPerFrame},
        {"channels", data.channels}, {"encodeQuality", data.encodeQuality}, {"blockPerSecond", data.blockPerSecond}};
    std::unordered_map<string, std::vector<int32_t> &> capabilityVectorMap = {
        {"sampleRate", data.sampleRate}, {"pixFormat", data.pixFormat}, {"bitDepth", data.bitDepth},
        {"profiles", data.profiles}, {"bitrateMode", data.bitrateMode}};
    std::unordered_map<string, std::map<int32_t, std::vector<int32_t>> &> capabilityHashVectorMap = {
        {"profileLevelsMap", data.profileLevelsMap}};
    bool ret = false;
    if (capabilityStringMap.find(capabilityKey) != capabilityStringMap.end()) {
        ret = SetCapabilityStringData(capabilityStringMap, capabilityKey, capabilityValue);
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "SetCapabilityStringData failed");
    } else if (capIntMap.find(capabilityKey) != capIntMap.end()) {
        ret = SetCapabilityIntData(capIntMap, capabilityKey, capabilityValue);
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "SetCapabilityIntData failed");
    } else if (capabilityBoolMap.find(capabilityKey) != capabilityBoolMap.end()) {
        ret = SetCapabilityBoolData(capabilityBoolMap, capabilityKey, capabilityValue);
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "SetCapabilityBoolData failed");
    } else if (capSizeMap.find(capabilityKey) != capSizeMap.end()) {
        ret = SetCapabilitySizeData(capSizeMap, capabilityKey, capabilityValue);
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "SetCapabilitySizeData failed");
    } else if (capabilityHashRangeMap.find(capabilityKey) != capabilityHashRangeMap.end()) {
        ret = SetCapabilityHashRangeData(capabilityHashRangeMap, capabilityKey, capabilityValue);
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "SetCapabilityHashRangeData failed");
    } else if (capabilityRangeMap.find(capabilityKey) != capabilityRangeMap.end()) {
        ret = SetCapabilityRangeData(capabilityRangeMap, capabilityKey, capabilityValue);
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "SetCapabilityRangeData failed");
    } else if (capabilityVectorMap.find(capabilityKey) != capabilityVectorMap.end()) {
        ret = SetCapabilityVectorData(capabilityVectorMap, capabilityKey, capabilityValue);
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "SetCapabilityVectorData failed");
    } else if (capabilityHashVectorMap.find(capabilityKey) != capabilityHashVectorMap.end()) {
        ret = SetCapabilityHashVectorData(capabilityHashVectorMap, capabilityKey, capabilityValue);
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "SetCapabilityVectorData failed");
    } else {
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "can not find capabilityKey: %{public}s", capabilityKey.c_str());
    }
    return true;
}

bool CodeclistXmlParser::ParseData(xmlNode *node)
{
    xmlNode *child = node->xmlChildrenNode;
    string capabilityValue;
    CapabilityData capabilityData;
    for (; child != nullptr; child = child->next) {
        if (!xmlStrEqual(child->name, reinterpret_cast<const xmlChar *>("Item"))) {
            continue;
        }
        for (auto it = capabilityKeys_.begin(); it != capabilityKeys_.end(); it++) {
            string capabilityKey = *it;
            if (xmlHasProp(child, reinterpret_cast<xmlChar *>(const_cast<char *>(capabilityKey.c_str())))) {
                xmlChar *pXmlProp =
                    xmlGetProp(child, reinterpret_cast<xmlChar *>(const_cast<char *>(capabilityKey.c_str())));
                CHECK_AND_CONTINUE_LOG(pXmlProp != nullptr, "SetCapabilityData failed");
                capabilityValue = string(reinterpret_cast<char *>(pXmlProp));
                bool ret = SetCapabilityData(capabilityData, capabilityKey, capabilityValue);
                CHECK_AND_RETURN_RET_LOG(ret != false, false, "SetCapabilityData failed");
                break;
            }
        }
    }
    capabilityDataArray_.push_back(capabilityData);
    return true;
}

NodeName CodeclistXmlParser::GetNodeNameAsInt(xmlNode *node)
{
    if (xmlStrEqual(node->name, reinterpret_cast<const xmlChar *>("Codecs"))) {
        return CODECS;
    } else if (xmlStrEqual(node->name, reinterpret_cast<const xmlChar *>("AudioCodecs"))) {
        return AUDIO_CODECS;
    } else if (xmlStrEqual(node->name, reinterpret_cast<const xmlChar *>("VideoCodecs"))) {
        return VIDEO_CODECS;
    } else if (xmlStrEqual(node->name, reinterpret_cast<const xmlChar *>("AudioDecoder"))) {
        return AUDIO_DECODER;
    } else if (xmlStrEqual(node->name, reinterpret_cast<const xmlChar *>("AudioEncoder"))) {
        return AUDIO_ENCODER;
    } else if (xmlStrEqual(node->name, reinterpret_cast<const xmlChar *>("VideoDecoder"))) {
        return VIDEO_DECODER;
    } else if (xmlStrEqual(node->name, reinterpret_cast<const xmlChar *>("VideoEncoder"))) {
        return VIDEO_ENCODER;
    } else {
        return UNKNOWN;
    }
}

std::vector<CapabilityData> CodeclistXmlParser::GetCapabilityDataArray() const
{
    return capabilityDataArray_;
}
// #endif
} // namespace Media
} // namespace OHOS