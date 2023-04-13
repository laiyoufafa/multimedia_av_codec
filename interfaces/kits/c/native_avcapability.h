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

#ifndef NATIVE_AVCAPABILITY_H
#define NATIVE_AVCAPABILITY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct OH_AVCapability OH_AVCapability;

/**
 * @brief The bitrate mode of video encoder.
 * @since 10
 * @version 1.0
 */
typedef enum OH_BitrateMode {
    /* constant bit rate mode. */
    CBR = 0,
    /* variable bit rate mode. */
    VBR = 1,
    /* constant quality mode. */
    CQ = 2,
    /* Constrained VariableBit Rate. */
    VCBR = 3,
    /* Average Bit Rate. */
    ABR = 4
} OH_BitrateMode;

/**
 * @brief Check whether is vendor codec.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return true indicate vendor codec, false indicate software codec
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_IsVendor(const struct OH_AVCapability *capability);

/**
 * @brief Check whether is this video size supported.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width codec width
 * @param height codec height
 * @return true indicate supported, false indicate not supported
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_IsSizeSupported(const struct OH_AVCapability *capability, int32_t width, int32_t height);

/**
 * @brief Check whether is this video size and fps supported.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width codec width
 * @param height codec height
 * @param fps codec fps
 * @return true indicate supported, false indicate not supported
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_AreSizeAndFrameRateSupported(const struct OH_AVCapability *capability,
    int32_t width, int32_t height, int32_t fps);

/**
 * @brief Check whether is this audio sampleRate supported.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param sampleRate sample rate
 * @return true indicate supported, false indicate not supported
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_IsSampleRateSupported(const struct OH_AVCapability *capability, int32_t sampleRate);

/**
 * @brief Get bitrate range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param minVal return bitrate low limit
 * @param maxVal return bitrate high limit
 * @since 10
 * @version 1.0
*/
void OH_AVCapability_GetBitrateRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal);

/**
 * @brief Get channels range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param minVal return channels low limit
 * @param maxVal return channels high limit
 * @since 10
 * @version 1.0
*/
void OH_AVCapability_GetChannelsRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal);

/**
 * @brief Get channels range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param minVal return channels low limit
 * @param maxVal return channels high limit
 * @since 10
 * @version 1.0
*/
void OH_AVCapability_GetComplexityRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal);

/**
 * @brief Get width alignment
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return width alignment
 * @since 10
 * @version 1.0
*/
int32_t OH_AVCapability_GetWidthAlignment(const struct OH_AVCapability *capability);

/**
 * @brief Get height alignment
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return height alignment
 * @since 10
 * @version 1.0
*/
int32_t OH_AVCapability_GetHeightAlignment(const struct OH_AVCapability *capability);

/**
 * @brief Get width range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param minVal return width low limit
 * @param maxVal return width high limit
 * @since 10
 * @version 1.0
*/
void OH_AVCapability_GetWidthRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal);

/**
 * @brief Get height range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param minVal return height low limit
 * @param maxVal return height high limit
 * @since 10
 * @version 1.0
*/
void OH_AVCapability_GetHeightRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal);

/**
 * @brief Get frame rate range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param minVal return frame rate low limit
 * @param maxVal return frame rate high limit
 * @since 10
 * @version 1.0
*/
void OH_AVCapability_GetFrameRateRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal);

/**
 * @brief Get frame rate range when knew video size
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width video width
 * @param height video height
 * @param minVal return frame rate low limit
 * @param maxVal return frame rate high limit
 * @since 10
 * @version 1.0
*/
void OH_AVCapability_GetSupportedFrameRateRangeForSize(const struct OH_AVCapability *capability, 
    int32_t width, int32_t height, int32_t *minVal, int32_t *maxVal);

/**
 * @brief Get encode quality range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param minVal return encode quality low limit
 * @param maxVal return encode quality high limit
 * @since 10
 * @version 1.0
*/
void OH_AVCapability_GetEncodeQualityRange(const struct OH_AVCapability *capability, int32_t *minVal, int32_t *maxVal);

/**
 * @brief Get sampleRate array
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param arraySize sampleRate array size
 * @return sampleRate array pointer
 * @since 10
 * @version 1.0
*/
const int32_t *OH_AVCapability_GetSampleRateArray(const struct OH_AVCapability *capability, uint32_t *arraySize);

/**
 * @brief Get format array
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param arraySize format array size
 * @return format array pointer
 * @since 10
 * @version 1.0
*/
const int32_t *OH_AVCapability_GetFormatArray(const struct OH_AVCapability *capability, uint32_t *arraySize);

/**
 * @brief Get profiles array
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param arraySize profiles array size
 * @return profiles array pointer
 * @since 10
 * @version 1.0
*/
const int32_t *OH_AVCapability_GetProfilesArray(const struct OH_AVCapability *capability, uint32_t *arraySize);

/**
 * @brief Check whether is this bitrate mode supported.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param mode bitrateMode
 * @return true indicate supported, false indicate not supported
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_isBitratesModeSupported(const struct OH_AVCapability *capability, OH_BitrateMode mode);

/**
 * @brief Get levels array
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param arraySize levels array size
 * @return levels array pointer
 * @since 10
 * @version 1.0
*/
const int32_t *OH_AVCapability_GetLevelsArray(const struct OH_AVCapability *capability, uint32_t *arraySize);

/**
 * @brief Get preferred frameRate at set resolution, these framerates can be reach the performance.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width video width
 * @param height video height
 * @param minVal return frame rate low limit
 * @param maxVal return frame rate high limit
 * @since 10
 * @version 1.0
*/
void OH_AVCapability_GetPreferredFrameRateRangeForSize(const struct OH_AVCapability *capability,
    int32_t width, int32_t height, int32_t *minVal, int32_t *maxVal);

/**
 * @brief Get swapWidthHeightFlag
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return swapWidthHeightFlag
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_GetSwapWidthHeightFlag(const struct OH_AVCapability *capability);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVCAPABILITY_H