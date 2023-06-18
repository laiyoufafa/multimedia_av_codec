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

#include <algorithm>
#include "native_avmagic.h"
#include "avcodec_list.h"
#include "avcodec_info.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "native_avcapability.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NativeAVCapability"};
}
using namespace OHOS::MediaAVCodec;

OH_AVCapability::~OH_AVCapability() {}

OH_AVCapability *OH_AVCodec_GetCapability(const char *mime, bool isEncoder)
{
    CHECK_AND_RETURN_RET_LOG(mime != nullptr, nullptr, "Get capability failed: mime is nullptr");
    CHECK_AND_RETURN_RET_LOG(strlen(mime) != 0, nullptr, "Get capability failed: mime is empty");
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    uint32_t sizeOfCap = sizeof(OH_AVCapability);
    CapabilityData *capabilityData = codeclist->GetCapability(mime, isEncoder, AVCodecCategory::AVCODEC_NONE);
    CHECK_AND_RETURN_RET_LOG(capabilityData != nullptr, nullptr,
                             "Get capability failed: cannot find matched capability");
    const std::string &name = capabilityData->codecName;
    CHECK_AND_RETURN_RET_LOG(!name.empty(), nullptr, "Get capability failed: cannot find matched capability");
    void *addr = codeclist->GetBuffer(name, sizeOfCap);
    CHECK_AND_RETURN_RET_LOG(addr != nullptr, nullptr, "Get capability failed: malloc capability buffer failed");
    OH_AVCapability *obj = static_cast<OH_AVCapability *>(addr);
    obj->capabilityData_ = capabilityData;
    AVCODEC_LOGD("OH_AVCodec_GetCapability successful");
    return obj;
}

OH_AVCapability *OH_AVCodec_GetCapabilityByCategory(const char *mime, bool isEncoder, OH_AVCodecCategory category)
{
    CHECK_AND_RETURN_RET_LOG(mime != nullptr, nullptr, "Get capabilityByCategory failed: mime is nullptr");
    CHECK_AND_RETURN_RET_LOG(strlen(mime) != 0, nullptr, "Get capabilityByCategory failed: mime is empty");
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    AVCodecCategory innerCategory;
    if (category == HARDWARE) {
        innerCategory = AVCodecCategory::AVCODEC_HARDWARE;
    } else {
        innerCategory = AVCodecCategory::AVCODEC_SOFTWARE;
    }
    uint32_t sizeOfCap = sizeof(OH_AVCapability);
    CapabilityData *capabilityData = codeclist->GetCapability(mime, isEncoder, innerCategory);
    CHECK_AND_RETURN_RET_LOG(capabilityData != nullptr, nullptr,
                             "Get capabilityByCategory failed: cannot find matched capability");
    const std::string &name = capabilityData->codecName;
    CHECK_AND_RETURN_RET_LOG(!name.empty(), nullptr, "Get capabilityByCategory failed: cannot find matched capability");
    void *addr = codeclist->GetBuffer(name, sizeOfCap);
    CHECK_AND_RETURN_RET_LOG(addr != nullptr, nullptr,
                             "Get capabilityByCategory failed: malloc capability buffer failed");
    OH_AVCapability *obj = static_cast<OH_AVCapability *>(addr);
    obj->capabilityData_ = capabilityData;
    AVCODEC_LOGD("OH_AVCodec_GetCapability successful");
    return obj;
}

const char *OH_AVCapability_GetName(OH_AVCapability *capability)
{
    if (capability == nullptr) {
        AVCODEC_LOGE("Get name failed:  null input");
        return "";
    }
    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    const auto &name = codecInfo->GetName();
    return name.data();
}

bool OH_AVCapability_IsHardware(OH_AVCapability *capability)
{
    if (capability == nullptr) {
        AVCODEC_LOGE("Varified is hardware failed:  null input");
        return false;
    }
    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    return codecInfo->IsHardwareAccelerated();
}

int32_t OH_AVCapability_GetMaxSupportedInstances(OH_AVCapability *capability)
{
    if (capability == nullptr) {
        AVCODEC_LOGE("Get max supported instance failed: null input");
        return 0;
    }
    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    return codecInfo->GetMaxSupportedInstances();
}

OH_AVErrCode OH_AVCapability_GetSupportedProfiles(OH_AVCapability *capability, const int32_t **profiles,
                                                  uint32_t *profileNum)
{
    CHECK_AND_RETURN_RET_LOG(profileNum != nullptr && profiles != nullptr, AV_ERR_INVALID_VAL,
                             "Get supported profiles failed: null input");
    if (capability == nullptr) {
        *profiles = nullptr;
        *profileNum = 0;
        AVCODEC_LOGE("Get supported profiles failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AudioCaps> codecInfo = std::make_shared<AudioCaps>(capability->capabilityData_);
    const auto &profile = codecInfo->GetSupportedProfiles();
    *profiles = profile.data();
    *profileNum = profile.size();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetSupportedLevelsForProfile(OH_AVCapability *capability, int32_t profile,
                                                          const int32_t **levels, uint32_t *levelNum)
{
    CHECK_AND_RETURN_RET_LOG(levels != nullptr && levelNum != nullptr, AV_ERR_INVALID_VAL,
                             "Get supported levels for profile failed: null input");
    if (capability == nullptr) {
        *levels = nullptr;
        *levelNum = 0;
        AVCODEC_LOGE("Get supported levels for profile failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    const auto &profileLevelsMap = codecInfo->GetSupportedLevelsForProfile();
    const auto &levelsmatch = profileLevelsMap.find(profile);
    if (levelsmatch == profileLevelsMap.end()) {
        return AV_ERR_INVALID_VAL;
    }
    *levels = levelsmatch->second.data();
    *levelNum = levelsmatch->second.size();
    return AV_ERR_OK;
}

bool OH_AVCapability_AreProfileAndLevelSupported(OH_AVCapability *capability, int32_t profile, int32_t level)
{
    if (capability == nullptr) {
        AVCODEC_LOGE("Varified profiles and level failed: null input");
        return false;
    }
    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    const auto &profileLevelsMap = codecInfo->GetSupportedLevelsForProfile();
    const auto &levels = profileLevelsMap.find(profile);
    if (levels == profileLevelsMap.end()) {
        return false;
    }
    return find(levels->second.begin(), levels->second.end(), level) != levels->second.end();
}

OH_AVErrCode OH_AVCapability_GetEncoderBitrateRange(OH_AVCapability *capability, OH_AVRange *bitrateRange)
{
    CHECK_AND_RETURN_RET_LOG(bitrateRange != nullptr, AV_ERR_INVALID_VAL,
                             "Get encoder bitrate range failed: null input");
    if (capability == nullptr) {
        bitrateRange->minVal = 0;
        bitrateRange->maxVal = 0;
        AVCODEC_LOGE("Get encoder bitrate range failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AudioCaps> codecInfo = std::make_shared<AudioCaps>(capability->capabilityData_);
    const auto &bitrate = codecInfo->GetSupportedBitrate();
    bitrateRange->minVal = bitrate.minVal;
    bitrateRange->maxVal = bitrate.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetEncoderQualityRange(OH_AVCapability *capability, OH_AVRange *qualityRange)
{
    CHECK_AND_RETURN_RET_LOG(qualityRange != nullptr, AV_ERR_INVALID_VAL, "Get encoder quality failed: null input");
    if (capability == nullptr) {
        qualityRange->minVal = 0;
        qualityRange->maxVal = 0;
        AVCODEC_LOGE("Get encoder quality failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &quality = codecInfo->GetSupportedEncodeQuality();
    qualityRange->minVal = quality.minVal;
    qualityRange->maxVal = quality.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetEncoderComplexityRange(OH_AVCapability *capability, OH_AVRange *complexityRange)
{
    CHECK_AND_RETURN_RET_LOG(complexityRange != nullptr, AV_ERR_INVALID_VAL,
                             "Get encoder complexity range failed: null input");
    if (capability == nullptr) {
        complexityRange->minVal = 0;
        complexityRange->maxVal = 0;
        AVCODEC_LOGE("Get encoder complexity range failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &complexity = codecInfo->GetSupportedComplexity();
    complexityRange->minVal = complexity.minVal;
    complexityRange->maxVal = complexity.maxVal;
    return AV_ERR_OK;
}

bool OH_AVCapability_IsEncoderBitrateModeSupported(OH_AVCapability *capability, OH_BitrateMode bitrateMode)
{
    if (capability == nullptr) {
        AVCODEC_LOGE("Varified encoder bitrate mode failed: null input");
        return false;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &bitrateModeVec = codecInfo->GetSupportedBitrateMode();
    return find(bitrateModeVec.begin(), bitrateModeVec.end(), bitrateMode) != bitrateModeVec.end();
}

OH_AVErrCode OH_AVCapability_GetAudioSupportedSampleRates(OH_AVCapability *capability, const int32_t **sampleRates,
                                                          uint32_t *sampleRateNum)
{
    CHECK_AND_RETURN_RET_LOG(sampleRates != nullptr && sampleRateNum != nullptr, AV_ERR_INVALID_VAL,
                             "Get audio supported samplerates failed: null input");
    if (capability == nullptr) {
        *sampleRates = nullptr;
        *sampleRateNum = 0;
        AVCODEC_LOGE("Get audio supported samplerates failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AudioCaps> codecInfo = std::make_shared<AudioCaps>(capability->capabilityData_);
    const auto &vec = codecInfo->GetSupportedSampleRates();
    *sampleRates = vec.data();
    *sampleRateNum = vec.size();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetAudioChannelCountRange(OH_AVCapability *capability, OH_AVRange *channelCountRange)
{
    CHECK_AND_RETURN_RET_LOG(channelCountRange != nullptr, AV_ERR_INVALID_VAL,
                             "Get audio channel count range failed: null input");
    if (capability == nullptr) {
        channelCountRange->minVal = 0;
        channelCountRange->maxVal = 0;
        AVCODEC_LOGE("Get audio channel count range failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AudioCaps> codecInfo = std::make_shared<AudioCaps>(capability->capabilityData_);
    const auto &channels = codecInfo->GetSupportedChannel();
    channelCountRange->minVal = channels.minVal;
    channelCountRange->maxVal = channels.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoSupportedPixelFormats(OH_AVCapability *capability, const int32_t **pixFormats,
                                                           uint32_t *pixFormatNum)
{
    CHECK_AND_RETURN_RET_LOG(pixFormats != nullptr && pixFormatNum != nullptr, AV_ERR_INVALID_VAL,
                             "Get video supported pixel formats failed: null input");
    if (capability == nullptr) {
        *pixFormats = nullptr;
        *pixFormatNum = 0;
        AVCODEC_LOGE("Get video supported pixel formats failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &pixFmt = codecInfo->GetSupportedFormats();
    *pixFormats = pixFmt.data();
    *pixFormatNum = pixFmt.size();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoWidthAlignment(OH_AVCapability *capability, int32_t *widthAlignment)
{
    CHECK_AND_RETURN_RET_LOG(widthAlignment != nullptr, AV_ERR_INVALID_VAL,
                             "Get video width alignment failed: null input");
    if (capability == nullptr) {
        *widthAlignment = 0;
        AVCODEC_LOGE("Get video width alignment failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    *widthAlignment = codecInfo->GetSupportedWidthAlignment();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoHeightAlignment(OH_AVCapability *capability, int32_t *heightAlignment)
{
    CHECK_AND_RETURN_RET_LOG(heightAlignment != nullptr, AV_ERR_INVALID_VAL,
                             "Get video height alignment failed: null input");
    if (capability == nullptr) {
        *heightAlignment = 0;
        AVCODEC_LOGE("Get video height alignment failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    *heightAlignment = codecInfo->GetSupportedHeightAlignment();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoWidthRangeForHeight(OH_AVCapability *capability, int32_t height,
                                                         OH_AVRange *widthRange)
{
    CHECK_AND_RETURN_RET_LOG(widthRange != nullptr, AV_ERR_INVALID_VAL,
                             "Get video width range for height failed: null input");
    if (capability == nullptr || height <= 0) {
        widthRange->minVal = 0;
        widthRange->maxVal = 0;
        AVCODEC_LOGE("Get video width range for height failed: invalid input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &width = codecInfo->GetVideoWidthRangeForHeight(height);
    widthRange->minVal = width.minVal;
    widthRange->maxVal = width.maxVal;
    if (width.minVal == 0 && width.maxVal == 0) {
        return AV_ERR_INVALID_VAL;
    }
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoHeightRangeForWidth(OH_AVCapability *capability, int32_t width,
                                                         OH_AVRange *heightRange)
{
    CHECK_AND_RETURN_RET_LOG(heightRange != nullptr, AV_ERR_INVALID_VAL,
                             "Get video height range for width failed: null input");
    if (capability == nullptr || width <= 0) {
        heightRange->minVal = 0;
        heightRange->maxVal = 0;
        AVCODEC_LOGE("Get video height range for width failed: invalid input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &height = codecInfo->GetVideoHeightRangeForWidth(width);
    heightRange->minVal = height.minVal;
    heightRange->maxVal = height.maxVal;
    if (height.minVal == 0 && height.maxVal == 0) {
        return AV_ERR_INVALID_VAL;
    }
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoWidthRange(OH_AVCapability *capability, OH_AVRange *widthRange)
{
    CHECK_AND_RETURN_RET_LOG(widthRange != nullptr, AV_ERR_INVALID_VAL, "Get video width range failed: null input");
    if (capability == nullptr) {
        widthRange->minVal = 0;
        widthRange->maxVal = 0;
        AVCODEC_LOGE("Get video width range failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &width = codecInfo->GetSupportedWidth();
    widthRange->minVal = width.minVal;
    widthRange->maxVal = width.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoHeightRange(OH_AVCapability *capability, OH_AVRange *heightRange)
{
    CHECK_AND_RETURN_RET_LOG(heightRange != nullptr, AV_ERR_INVALID_VAL, "Get video height range failed: null input");
    if (capability == nullptr) {
        heightRange->minVal = 0;
        heightRange->maxVal = 0;
        AVCODEC_LOGE("Get video height range failed: null input");
        return AV_ERR_INVALID_VAL;
    }

    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &height = codecInfo->GetSupportedHeight();
    heightRange->minVal = height.minVal;
    heightRange->maxVal = height.maxVal;
    return AV_ERR_OK;
}

bool OH_AVCapability_IsVideoSizeSupported(OH_AVCapability *capability, int32_t width, int32_t height)
{
    if (capability == nullptr) {
        AVCODEC_LOGE("Varified is video size supported failed: null input");
        return false;
    }
    std::shared_ptr<VideoCaps> videoCap = std::make_shared<VideoCaps>(capability->capabilityData_);
    return videoCap->IsSizeSupported(width, height);
}

OH_AVErrCode OH_AVCapability_GetVideoFrameRateRange(OH_AVCapability *capability, OH_AVRange *frameRateRange)
{
    CHECK_AND_RETURN_RET_LOG(frameRateRange != nullptr, AV_ERR_INVALID_VAL,
                             "Get video framerate range failed: null input");
    if (capability == nullptr) {
        frameRateRange->minVal = 0;
        frameRateRange->maxVal = 0;
        AVCODEC_LOGE("Get video framerate range failed: null input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> videoCap = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &frameRate = videoCap->GetSupportedFrameRate();
    frameRateRange->minVal = frameRate.minVal;
    frameRateRange->maxVal = frameRate.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoFrameRateRangeForSize(OH_AVCapability *capability, int32_t width, int32_t height,
                                                           OH_AVRange *frameRateRange)
{
    CHECK_AND_RETURN_RET_LOG(frameRateRange != nullptr, AV_ERR_INVALID_VAL,
                             "Get video framerate range for size failed: null input");
    if (capability == nullptr || width <= 0 || height <= 0) {
        frameRateRange->minVal = 0;
        frameRateRange->maxVal = 0;
        AVCODEC_LOGE("Get video framerate range for size failed: invalid input");
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> videoCap = std::make_shared<VideoCaps>(capability->capabilityData_);
    const auto &frameRate = videoCap->GetSupportedFrameRatesFor(width, height);
    frameRateRange->minVal = frameRate.minVal;
    frameRateRange->maxVal = frameRate.maxVal;
    if (frameRate.minVal == 0 && frameRate.maxVal == 0) {
        return AV_ERR_INVALID_VAL;
    }
    return AV_ERR_OK;
}

bool OH_AVCapability_AreVideoSizeAndFrameRateSupported(OH_AVCapability *capability, int32_t width, int32_t height,
                                                       int32_t frameRate)
{
    if (capability == nullptr) {
        AVCODEC_LOGE("Varified video framerate and size failed: null input");
        return false;
    }
    std::shared_ptr<VideoCaps> videoCap = std::make_shared<VideoCaps>(capability->capabilityData_);
    return videoCap->IsSizeAndRateSupported(width, height, frameRate);
}