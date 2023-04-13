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

// namespace {
// constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "OH_AVCapability"};
// }

using namespace OHOS::MediaAVCodec;

OH_AVCapability::OH_AVCapability(const CapabilityData &capabilityData)
    : capabilityData_(capabilityData)
{
}

OH_AVCapability::~OH_AVCapability()
{

}

bool OH_AVCapability_IsVendor(const struct OH_AVCapability *capability)
{
    if (capability == nullptr) {
        return false;
    }
    return capability->capabilityData_.isVendor;
}

bool OH_AVCapability_IsSizeSupported(const struct OH_AVCapability *capability, int32_t width, int32_t height)
{
    if (capability == nullptr) {
        return false;
    }

    auto &alignment = capability->capabilityData_.alignment;
    if (blockSize.width == 0 || blockSize.height == 0 ||
        alignment.width == 0 || alignment.height == 0 ||
        width % alignment.width != 0 || height % alignment.height != 0) {
        return false;
    }

    auto &widthRange = capability->capabilityData_.width;
    auto &heightRange = capability->capabilityData_.height;
    if (width < widthRange.minVal || width > widthRange.maxVal ||
        height < heightRange.minVal || height > heightRange.maxVal) {
        return false;
    }

    auto &blockSize = capability->capabilityData_.blockSize;
    if (blockSize.width == 0 || blockSize.height == 0) {
        return false;
    }
    int blockNum = ((width + blockSize.width - 1) / blockSize.width) * 
        ((height + blockSize.height - 1) / blockSize.height);
    
    auto &blockPerFrame = capability->capabilityData_.blockPerFrame;
    if (blockNum < blockPerFrame.minVal || blockNum > blockPerFrame.maxVal) {
        return false;
    }

    return true;
}

bool OH_AVCapability_AreSizeAndFrameRateSupported(const struct OH_AVCapability *capability,
    int32_t width, int32_t height, int32_t fps)
{
    if (capability == nullptr) {
        return false;
    }

    auto &alignment = capability->capabilityData_.alignment;
    if (blockSize.width == 0 || blockSize.height == 0 ||
        alignment.width == 0 || alignment.height == 0 ||
        width % alignment.width != 0 || height % alignment.height != 0) {
        return false;
    }

    auto &widthRange = capability->capabilityData_.width;
    auto &heightRange = capability->capabilityData_.height;
    auto &fpsRange = capability->capabilityData_.frameRate;
    if (width < widthRange.minVal || width > widthRange.maxVal ||
        height < heightRange.minVal || height > heightRange.maxVal ||
        fps < fpsRange.minVal || fps > fpsRange.maxVal) {
        return false;
    }

    auto &blockSize = capability->capabilityData_.blockSize;
    if (blockSize.width == 0 || blockSize.height == 0) {
        return false;
    }
    int blockNum = ((width + blockSize.width - 1) / blockSize.width) *
        ((height + blockSize.height - 1) / blockSize.height);
    int blockOneSecond = blockNum * fps;
    auto &blockPerFrame = capability->capabilityData_.blockPerFrame;
    auto &blockPerSecond = capability->capabilityData_.blockPerSecond;
    if (blockNum < blockPerFrame.minVal || blockNum > blockPerFrame.maxVal ||
        blockOneSecond < blockPerSecond.minVal || blockOneSecond > blockPerSecond.maxVal ) {
        return false;
    }
    return true;
}

bool OH_AVCapability_IsSampleRateSupported(const struct OH_AVCapability *capability, int32_t sampleRate)
{
    if (capability == nullptr) {
        return false;
    }
    std::vector<int32_t> &sampleRateVec = capability->capabilityData_.sampleRate;
    return find(sampleRateVec.begin(), sampleRateVec.end(), sampleRate) != sampleRateVec.end();
}

void OH_AVCapability_GetBitrateRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    if (capability == nullptr) {
        *minVal = 0;
        *maxVal = 0;
        return;
    }
    *minVal = capability->capabilityData_.bitrate.minVal;
    *maxVal = capability->capabilityData_.bitrate.maxVal;
}

void OH_AVCapability_GetChannelsRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    if (capability == nullptr) {
        *minVal = 0;
        *maxVal = 0;
        return;
    }
    *minVal = capability->capabilityData_.channels.minVal;
    *maxVal = capability->capabilityData_.channels.maxVal;
}

void OH_AVCapability_GetComplexityRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    if (capability == nullptr) {
        *minVal = 0;
        *maxVal = 0;
        return;
    }
    *minVal = capability->capabilityData_.complexity.minVal;
    *maxVal = capability->capabilityData_.complexity.maxVal;
}

int32_t OH_AVCapability_GetWidthAlignment(const struct OH_AVCapability *capability)
{
    if (capability == nullptr) {
        return 0;
    }
    return capability->capabilityData_.alignment.width;
}

int32_t OH_AVCapability_GetHeightAlignment(const struct OH_AVCapability *capability)
{
    if (capability == nullptr) {
        return 0;
    }
    return capability->capabilityData_.alignment.height;
}

void OH_AVCapability_GetWidthRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    if (capability == nullptr) {
        *minVal = 0;
        *maxVal = 0;
        return;
    }
    *minVal = capability->capabilityData_.width.minVal;
    *maxVal = capability->capabilityData_.width.maxVal;
}

void OH_AVCapability_GetHeightRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    if (capability == nullptr) {
        *minVal = 0;
        *maxVal = 0;
        return;
    }
    *minVal = capability->capabilityData_.width.minVal;
    *maxVal = capability->capabilityData_.width.maxVal;
}

void OH_AVCapability_GetFrameRateRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    if (capability == nullptr) {
        *minVal = 0;
        *maxVal = 0;
        return;
    }
    *minVal = capability->capabilityData_.frameRate.minVal;
    *maxVal = capability->capabilityData_.frameRate.maxVal;
}

void OH_AVCapability_GetSupportedFrameRateRangeForSize(const struct OH_AVCapability *capability, 
    int32_t width, int32_t height, int32_t *minVal, int32_t *maxVal)
{
    *minVal = 0;
    *maxVal = 0;
    if (capability == nullptr) {
        return;
    }

    if (!OH_AVCapability_IsSizeSupported(capability, width, height)) {
        return;
    }

    auto &blockSize = capability->capabilityData_.blockSize;
    auto &blockPerSecond = capability->capabilityData_.blockPerSecond;
    if (blockSize.width == 0 || blockSize.height == 0 ||
        blockPerSecond.minVal <= 0 || blockPerSecond.maxVal <= 0) {
        return;
    }
    int blockNum = ((width + blockSize.width - 1) / blockSize.width) *
        ((height + blockSize.height - 1) / blockSize.height);

    auto &fpsRange = capability->capabilityData_.frameRate;
    *minVal = std::max((blockPerSecond.minVal + blockNum - 1) / blockNum, fpsRange.minVal);
    *maxVal = std::min(blockPerSecond.maxVal / blockNum, fpsRange.maxVal);
}

void OH_AVCapability_GetEncodeQualityRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal)
{
    if (capability == nullptr) {
        *minVal = 0;
        *maxVal = 0;
        return;
    }
    *minVal = capability->capabilityData_.encodeQuality.minVal;
    *maxVal = capability->capabilityData_.encodeQuality.maxVal;
}

int32_t *OH_AVCapability_GetSampleRateArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    if (capability == nullptr) {
        *arraySize = 0;
        return nullptr;
    }
    std::vector<int32_t> &vec = capability->capabilityData_.sampleRate;
    *arraySize = vec.size();
    return vec.data();
}

int32_t *OH_AVCapability_GetFormatArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    if (capability == nullptr) {
        *arraySize = 0;
        return nullptr;
    }
    std::vector<int32_t> &vec = capability->capabilityData_.format;
    *arraySize = vec.size();
    return vec.data();
}

int32_t *OH_AVCapability_GetProfilesArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    if (capability == nullptr) {
        *arraySize = 0;
        return nullptr;
    }
    std::vector<int32_t> &vec = capability->capabilityData_.profiles;
    *arraySize = vec.size();
    return vec.data();
}

bool OH_AVCapability_isBitratesModeSupported(const struct OH_AVCapability *capability, OH_BitrateMode bitrateMode)
{
    if (capability == nullptr) {
        return false;
    }
    std::vector<int32_t> &bitrateModeVec = capability->capabilityData_.bitrateMode;
    return find(bitrateModeVec.begin(), bitrateModeVec.end(), bitrateMode) != bitrateModeVec.end();
}

int32_t *OH_AVCapability_GetLevelsArray(const struct OH_AVCapability *capability, uint32_t *arraySize)
{
    std::vector<int32_t> &vec = capability->capabilityData_.levels;
    *arraySize = vec.size();
    return vec.data();
}

// TODO std::map<int32_t, std::vector<int32_t>> profileLevelsMap;

void OH_AVCapability_GetPreferredFrameRateRangeForSize(const struct OH_AVCapability *capability,
    int32_t width, int32_t height, int32_t *minVal, int32_t *maxVal)
{
    *minVal = 0;
    *maxVal = 0;
    if (capability == nullptr) {
        return;
    }

    if (!OH_AVCapability_IsSizeSupported(capability, width, height)) {
        return;
    }

    auto &blockSize = capability->capabilityData_.blockSize;
    if (blockSize.width == 0 || blockSize.height == 0) {
        return;
    }
    int32_t blockNum = ((width + blockSize.width - 1) / blockSize.width) *
        ((height + blockSize.height - 1) / blockSize.height);

    int32_t minDiff = INT32_MAX;
    Range fpsRange;
    float factor = 1.0f;
    float blockNumFloat{blockNum};
    auto &measureFps = capability->capabilityData_.measuredFrameRate;
    auto iter = measureFps.first();
    while (iter != measureFps.end()) {
        int32_t curBlockNum = (iter->first.width + blockSize.width - 1) / blockSize.width *
            ((iter->first.height + blockSize.height - 1) / blockSize.height);
        int32_t curDiff = std::abs(curBlockNum - blockNum);
        if (curDiff < minDiff) {
            minDiff = curDiff;
            fpsRange = iter->second;
            factor = blockNumFloat / curBlockNum;
        }
        iter++;
    }

    *minVal = static_cast<int32_t>(fpsRange.minVal * factor);
    *maxVal = static_cast<int32_t>(fpsRange.maxVal * factor);
}

bool OH_AVCapability_GetSwapWidthHeightFlag(const struct OH_AVCapability *capability)
{
    return capability->capabilityData_.supportSwapWidthHeight;
}