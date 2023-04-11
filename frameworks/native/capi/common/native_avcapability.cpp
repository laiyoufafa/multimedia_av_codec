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
#include <algorithm>
#include "native_avmagic.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "OH_AVCapability"};
}

using namespace OHOS::AVCodec;

OH_AVCapability::OH_AVCapability(const CapabilityData &capabilityData)
    : capabilityData_(capabilityData)
{
}

OH_AVCapability::~OH_AVCapability()
{

}

bool OH_AVCapability_IsVendor(const struct OH_AVCapability *capability)
{
    return capability->capability_.isVendor;
}

bool OH_AVCapability_IsResolutionSupported(const struct OH_AVCapability *capability, int32_t width, int32_t height)
{
    return width >= capability->capability_.width.minVal &&
       width <= capability->capability_.width.maxVal &&
       height >= capability->capability_.height.minVal &&
       height <= capability->capability_.height.maxVal;
}

bool OH_AVCapability_IsSampleRateSupported(const struct OH_AVCapability *capability, int32_t sampleRate)
{
    std::vector<int32_t> &sampleRateVec = capability->capability_.sampleRate;
    return find(sampleRateVec.begin(), sampleRateVec.end(), sampleRate) != sampleRateVec.end();
}

void OH_AVCapability_GetBitrateRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.bitrate.minVal;
    *maxVal = capability->capability_.bitrate.maxVal;
}

void OH_AVCapability_GetChannelsRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.channels.minVal;
    *maxVal = capability->capability_.channels.maxVal;
}

void OH_AVCapability_GetComplexityRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.complexity.minVal;
    *maxVal = capability->capability_.complexity.maxVal;
}

void OH_AVCapability_GetAlignmentRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.alignment.minVal;
    *maxVal = capability->capability_.alignment.maxVal;
}

void OH_AVCapability_GetWidthRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.width.minVal;
    *maxVal = capability->capability_.width.maxVal;
}

void OH_AVCapability_GetHeightRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.width.minVal;
    *maxVal = capability->capability_.width.maxVal;
}

void OH_AVCapability_GetFrameRateRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.frameRate.minVal;
    *maxVal = capability->capability_.frameRate.maxVal;
}

void OH_AVCapability_GetEncodeQualityRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.encodeQuality.minVal;
    *maxVal = capability->capability_.encodeQuality.maxVal;
}

void OH_AVCapability_GetBlockPerFrameRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.blockPerFrame.minVal;
    *maxVal = capability->capability_.blockPerFrame.maxVal;
}

void OH_AVCapability_GetBlockPerSecondRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    *minVal = capability->capability_.blockPerSecond.minVal;
    *maxVal = capability->capability_.blockPerSecond.maxVal;
}

void OH_AVCapability_GetBlockSize(const struct OH_AVCapability *capability, int32_t *blockWidth, int32_t *blockHeight)
{
    *blockWidth = capability->capability_.blockSize.width;
    *blockHeight = capability->capability_.blockSize.height;
}

int32_t *OH_AVCapability_GetSampleRateArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> &vec = capability->capability_.sampleRate;
    *arraySize = vec.size();
    return vec.data();
}

int32_t *OH_AVCapability_GetFormatArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> &vec = capability->capability_.format;
    *arraySize = vec.size();
    return vec.data();
}

int32_t *OH_AVCapability_GetProfilesArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> &vec = capability->capability_.profiles;
    *arraySize = vec.size();
    return vec.data();
}

bool OH_AVCapability_isBitratesModeSupported(const struct OH_AVCapability *capability, OH_BitrateMode bitrateMode)
{
    std::vector<int32_t> &bitrateModeVec = capability->capability_.bitrateMode;
    return find(bitrateModeVec.begin(), bitrateModeVec.end(), bitrateMode) != bitrateModeVec.end();
}

int32_t *OH_AVCapability_GetLevelsArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> &vec = capability->capability_.levels;
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
    auto iter = capability->capability_.measuredFrameRate.first();
    while (iter != capability->capability_.measuredFrameRate.end()) {
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
    return capability->capability_.supportSwapWidthHeight;
}