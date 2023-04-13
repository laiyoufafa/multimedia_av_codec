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

#include "native_avcapability.h"
#include "native_avmagic.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

// namespace {
// constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "OH_AVCapability"};
// }

using namespace OHOS::Media;

OH_AVCapability::OH_AVCapability(const CapabilityData &capabilityData)
    : capabilityData_(capabilityData)
{
}

OH_AVCapability::~OH_AVCapability()
{

}

bool OH_AVCapability_IsVendor(const struct OH_AVCapability *capability)
{
    return capability->capabilityData_.isVendor;
}

bool OH_AVCapability_IsResolutionSupport(const struct OH_AVCapability *capability, int32_t width, int32_t height)
{
    return width >= capability->capabilityData_.width.minVal &&
       width <= capability->capabilityData_.width.maxVal &&
       height >= capability->capabilityData_.height.minVal &&
       height <= capability->capabilityData_.height.maxVal;
}

void OH_AVCapability_GetBitrateRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.bitrate.minVal;
    *maxVal = capability->capabilityData_.bitrate.maxVal;
}

void OH_AVCapability_GetChannelsRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.channels.minVal;
    *maxVal = capability->capabilityData_.channels.maxVal;
}

void OH_AVCapability_GetComplexityRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.complexity.minVal;
    *maxVal = capability->capabilityData_.complexity.maxVal;
}

void OH_AVCapability_GetAlignmentRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.alignment.minVal;
    *maxVal = capability->capabilityData_.alignment.maxVal;
}

void OH_AVCapability_GetWidthRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.width.minVal;
    *maxVal = capability->capabilityData_.width.maxVal;
}

void OH_AVCapability_GetHeightRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.width.minVal;
    *maxVal = capability->capabilityData_.width.maxVal;
}

void OH_AVCapability_GetFrameRateRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.frameRate.minVal;
    *maxVal = capability->capabilityData_.frameRate.maxVal;
}

void OH_AVCapability_GetEncodeQualityRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.encodeQuality.minVal;
    *maxVal = capability->capabilityData_.encodeQuality.maxVal;
}

void OH_AVCapability_GetQualityRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.quality.minVal;
    *maxVal = capability->capabilityData_.quality.maxVal;
}

void OH_AVCapability_GetBlockPerFrameRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.blockPerFrame.minVal;
    *maxVal = capability->capabilityData_.blockPerFrame.maxVal;
}

void OH_AVCapability_GetBlockPerSecondRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capabilityData_.blockPerSecond.minVal;
    *maxVal = capability->capabilityData_.blockPerSecond.maxVal;
}

void OH_AVCapability_GetBlockSize(const struct OH_AVCapability *capability, int32_t *blockWidth, int32_t *blockHeight)
{
    *blockWidth = capability->capabilityData_.blockSize.width;
    *blockHeight = capability->capabilityData_.blockSize.height;
}

int32_t *OH_AVCapability_GetSampleRateArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> vec = capability->capabilityData_.sampleRate;
    *arraySize = vec.size();
    return vec.data();
}

int32_t *OH_AVCapability_GetFormatArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> vec = capability->capabilityData_.format;
    *arraySize = vec.size();
    return vec.data();
}

int32_t *OH_AVCapability_GetProfilesArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> vec = capability->capabilityData_.profiles;
    *arraySize = vec.size();
    return vec.data();
}

int32_t *OH_AVCapability_GetBitrateModeArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> vec = capability->capabilityData_.bitrateMode;
    *arraySize = vec.size();
    return vec.data();
}

int32_t *OH_AVCapability_GetLevelsArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> vec = capability->capabilityData_.levels;
    *arraySize = vec.size();
    return vec.data();
}

// TODO std::map<int32_t, std::vector<int32_t>> profileLevelsMap;

int32_t OH_AVCapability_GetMaxSupportedFrameRate(const struct OH_AVCapability *capability, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        return 0;
    }

    int32_t minVal;
    int32_t maxVal;
    OH_AVCapability_GetWidthRange(capability, &minVal, &maxVal);
    if (width < minVal || width > maxVal) {
        return 0;
    }

    OH_AVCapability_GetHeightRange(capability, &minVal, &maxVal);
    if (height < minVal || height > maxVal) {
        return 0;
    }

    int32_t blockWidth;
    int32_t blockHeight;
    OH_AVCapability_GetBlockSize(capability, &blockWidth, &blockHeight);
    if (blockWidth <= 0 || blockHeight <= 0) {
        return 0;
    }

    int32_t targetBlockSum = (width + blockWidth - 1) / blockWidth * ((height + blockHeight - 1) / blockHeight);
    int32_t minDiff = INT32_MAX;
    Range fps;
    auto iter = capability->capabilityData_.measuredFrameRate.begin();
    while (iter != capability->capabilityData_.measuredFrameRate.end()) {
        int32_t curBlockNum = (iter->first.width + blockWidth - 1) / blockWidth *
            ((iter->first.height + blockHeight - 1) / blockHeight);
        if (curBlockNum >= targetBlockSum) {
            minDiff = minDiff > curBlockNum - targetBlockSum ? (curBlockNum - targetBlockSum) : minDiff;
            fps = iter->second;
        }
        iter++;
    }

    return fps.maxVal;
}

bool OH_AVCapability_GetSwapWidthHeightFlag(const struct OH_AVCapability *capability)
{
    return capability->capabilityData_.supportSwapWidthHeight;
}