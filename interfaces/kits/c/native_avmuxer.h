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

#ifndef NATIVE_AVMUXER_H
#define NATIVE_AVMUXER_H

#include <stdint.h>
#include <stdio.h>
#include "native_avcodec_base.h"
#include "native_averrors.h"
#include "native_avformat.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct OH_AVMuxer OH_AVMuxer;

/**
 * @brief Create an OH_AVMuxer instance by output file handler and format.
 * @syscap SystemCapability.Multimedia.Media.AVMuxer
 * @return Returns a pointer to an OH_AVMuxer instance
 * @since 10
 * @version 1.0
 */
OH_AVMuxer *OH_AVMuxer_Create(int32_t fd, OH_AVOutputFormat format);

/**
 * @brief Set and store the location to the output file. 
 * Note: This interface can only be called before OH_AVMuxer_Start.
 * The location is stored in the output file if the output format is {@link OH_AVOutputFormat} 
 * AV_OUTPUT_FORMAT_MPEG_4, and is ignored for other output formats. 
 * @syscap SystemCapability.Multimedia.Media.AVMuxer
 * @param muxer Pointer to an OH_AVMuxer instance.
 * @param latitude must be in the range [-90, 90].
 * @param longitude must be in the range [-180, 180].
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVMuxer_SetLocation(OH_AVMuxer *muxer, float latitude, float longitude);

/**
 * @brief Set the rotation for output video playback. 
 * Note: This interface can only be called before OH_AVMuxer_Start.
 * @syscap SystemCapability.Multimedia.Media.AVMuxer
 * @param muxer Pointer to an OH_AVMuxer instance.
 * @param rotation The supported angles are 0, 90, 180, and 270 degrees.  {@link OH_VideoRotation}
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVMuxer_SetRotation(OH_AVMuxer *muxer, int32_t rotation);

/**
 * @brief Set parameters to the muxer. 
 * Note: This interface can only be called before OH_AVMuxer_Start.
 * @syscap SystemCapability.Multimedia.Media.AVMuxer
 * @param muxer Pointer to an OH_AVMuxer instance
 * @param generalFormat OH_AVFormat handle pointer contain metadata
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVMuxer_SetParameter(OH_AVMuxer *muxer, OH_AVFormat *generalFormat);

/**
 * @brief Add track format to the muxer. 
 * Note: This interface can only be called before OH_AVMuxer_Start.
 * @syscap SystemCapability.Multimedia.Media.AVMuxer
 * @param muxer Pointer to an OH_AVMuxer instance
 * @param trackFormat OH_AVFormat handle pointer contain track format
 * @return The track index for this newly added track, and it should be used in the OH_AVMuxer_WriteSampleBuffer.
 * The track index is greater than or equal to 0, otherwise returns a specific error code, refer to {@link OH_AVErrCode}.
 * @since 10
 * @version 1.0
 */
int32_t OH_AVMuxer_AddTrack(OH_AVMuxer *muxer, OH_AVFormat *trackFormat);

/**
 * @brief Start the muxer. 
 * Note: This interface is called after OH_AVMuxer_AddTrack and before OH_AVMuxer_WriteSampleBuffer.
 * @syscap SystemCapability.Multimedia.Media.AVMuxer
 * @param muxer Pointer to an OH_AVMuxer instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVMuxer_Start(OH_AVMuxer *muxer);

/**
 * @brief Write an encoded sample to the muxer. 
 * Note: This interface can only be called after OH_AVMuxer_Start and before OH_AVMuxer_Stop. The application needs to 
 * make sure that the samples are written to the right tacks. Also, it needs to make sure the samples for each track are 
 * written in chronological order. 
 * For MPEG4 mime type format, the duration of the last sample in a track can be set by passing an additional empty buffer 
 * (info.size = 0) with flags (info.flags = AVCODEC_BUFFER_FLAGS_EOS) {@link OH_AVCodecBufferInfo and OH_AVCodecBufferFlags}
 * and a suitable presentation timestamp set in info parameter as the last sample of that track. This last sample's presentation 
 * timestamp shall be a sum of the presentation timestamp and the duration preferred for the original last sample. If no explicit
 * AVCODEC_BUFFER_FLAGS_EOS sample was passed, then the duration of the last sample would be the same as that of the sample before that.
 * @syscap SystemCapability.Multimedia.Media.AVMuxer
 * @param muxer Pointer to an OH_AVMuxer instance
 * @param trackIndex The track index for this sample
 * @param sampleBuffer The encoded sample buffer
 * @param info The buffer information related to this sample {@link OH_AVCodecBufferInfo}
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVMuxer_WriteSampleBuffer(OH_AVMuxer *muxer, uint32_t trackIndex, uint8_t *sampleBuffer, OH_AVCodecBufferInfo info);

/**
 * @brief Stop the muxer. 
 * Note: Once the muxer stops, it can not be restarted.
 * @syscap SystemCapability.Multimedia.Media.AVMuxer
 * @param muxer Pointer to an OH_AVMuxer instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVMuxer_Stop(OH_AVMuxer *muxer);

/**
 * @brief Clear the internal resources of the muxer and destroy the muxer instance
 * @syscap SystemCapability.Multimedia.Media.AVMuxer
 * @param codec Pointer to an OH_AVMuxer instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVMuxer_Destroy(OH_AVMuxer *muxer);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVMUXER_H