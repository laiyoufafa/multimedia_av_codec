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

#ifndef NATIVE_AVCODEC_DEMUXER_H
#define NATIVE_AVCODEC_DEMUXER_H

#include <stdint.h>
#include "native_avcodec_base.h"
#include "native_averrors.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief Creates a demuxer to demux media from source.
 * @syscap SystemCapability.Multimedia.AVCodec.Demuxer
 * @param source Pointer to an OH_AVSource instance.
 * @return Returns a pointer to an OH_AVDemuxer instance
 * @since 10
 * @version 4.0
*/
OH_AVDemuxer *OH_AVDemuxer_CreateWithSource(OH_AVSource *source);

/**
 * @brief Destroy the demuxer and free its resources.
 * @syscap SystemCapability.Multimedia.AVCodec.Demuxer
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specifis error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_Destroy(OH_AVDemuxer *demuxer);

/**
 * @brief Add a track to the demuxer. Subsequent calls to 
 * `OH_AVDemuxer_CopyCurrentSampleToBuf` only retrieve information for 
 * the subset of tracks selected. One track can only be added once, and 
 * add the same track multiple times has no effect. 
 * @syscap SystemCapability.Multimedia.AVCodec.Demuxer
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @param trackId Enter the index value corresponding to the track.
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specifis error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_AddSourceTrackByID(OH_AVDemuxer *demuxer, uint32_t trackId);

/**
 * @brief Remove a track from the demuxer. Subsequent calls to 
 * `OH_AVDemuxer_CopyCurrentSampleToBuf` only retrieve information for 
 * the subset of tracks selected.
 * @syscap SystemCapability.Multimedia.AVCodec.Demuxer
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @param trackId Enter the index value corresponding to the Track.
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specifis error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_RemoveSourceTrackByID(OH_AVDemuxer *demuxer, uint32_t trackId);

/**
 * @brief Retrieve the current sample in selected tracks and store it in buffer, 
 * and store buffer's info to attr.
 * @syscap SystemCapability.Multimedia.AVCodec.Demuxer
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @param buffer The empty buffer for storing data getting from `OH_AVCodec_GetInputBuffer`.
 * @param attr The empty OH_AVCodecBufferAttr struct for storing buffer info.
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specifis error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_CopyCurrentSampleToBuf(OH_AVDemuxer *demuxer, OH_AVBufferElement *buffer, OH_AVCodecBufferAttr *bufferInfo);

/**
 * @brief All selected tracks seek near to the requested time according to 
 * the seek mode.
 * @syscap SystemCapability.Multimedia.AVCodec.Demuxer
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @param mSeconde The seconds for seeking.
 * @param mode The mode for seeking. Value is:
 *             SEEK_NEXT_SYNC     > sync to keyframes after the time point.
 *             SEEK_PREVIOUS_SYNC > sync to keyframes before the time point.
 *             SEEK_CLOSEST_SYNC  > sync to closest keyframes.
 *             SEEK_CLOSEST       > sync to frames closest the time point.
 * @return Returns AV_ERR_OK if the execution is successful,
 *         otherwise returns a specifis error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_SeekToTimeStamp(OH_AVDemuxer *demuxer, int64_t mSeconds, OH_AVSeekMode mode);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVCODEC_DEMUXER_H
