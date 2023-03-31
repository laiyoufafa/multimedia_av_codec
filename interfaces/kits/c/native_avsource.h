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

#ifndef NATIVE_AVSOURCE_H
#define NATIVE_AVSOURCE_H

#include <stdint.h>
#include <native_avcodec_base.h>
#include <native_averrors.h>
#include <native_avformat.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief Creates a source that models the media at the specified URI.
 * @syscap SystemCapability.Multimedia.AVCodec.Source
 * @param uri A URI to a local, remote, ot HTTP Live Streaming media resource.
 * @return Returns AV_ERR_OK if the execution is successful,
 *         otherwise returns a specifis error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVSource *OH_AVSource_CreateWithURI(char *uri);

/**
 * @brief Creates a source that models the media at the specified FileDescriptor.
 * @syscap SystemCapability.Multimedia.AVCodec.Source
 * @param fd The fileDescriptor data source.
 * @param offset The offset into the file where the data be read starts.
 * @param size the length in bytes of the data to be read.
 * @return Returns AV_ERR_OK if the execution is successful,
 *         otherwise returns a specifis error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVSource *OH_AVSource_CreateWithFd(int32_t fd, int64_t offset, int64_t size);

/**
 * @brief Destroy the source and free its resources.
 * @syscap SystemCapability.Multimedia.AVCodec.Source
 * @param source Pointer to an OH_AVSource instance.
 * @return Returns AV_ERR_OK if the execution is successful,
 *         otherwise returns a specifis error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVSource_Destroy(OH_AVSource *source);

/**
 * @brief Get the count of tracks in the source.
 * @syscap SystemCapability.Multimedia.AVCodec.Source
 * @param source Pointer to an OH_AVSource instance.
 * @return Returns the count of tracks in the source.
 * @since 10
 * @version 1.0
*/
uint32_t OH_AVSource_GetTrackCount(OH_AVSource *source);

/**
 * @brief loads a track contains the specified identifier from the source.
 * @syscap SystemCapability.Multimedia.AVCodec.Source
 * @param source Pointer to an OH_AVSource instance.
 * @param trackId The identifier of the track to load.
 * @return Returns the track's format at the specified index.
 * @since 10
 * @version 1.0
*/
OH_AVSourceTrack *OH_AVSource_LoadSourceTrackByID(OH_AVSource *source, uint32_t trackId);

/**
 * @brief Set the parameters to track at the specified index.
 * @syscap SystemCapability.Multimedia.AVCodec.SourceTrack
 * @param sourceTrack Pointer to an OH_AVSourceTrack instance.
 * @param param OH_AVFormat handle pointer
 * @return Returns AV_ERR_OK if the execution is successful,
 *         otherwise returns a specifis error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVSourceTrack_SetParameter(OH_AVSourceTrack *sourceTrack, OH_AVFormat *param);

/**
 * @brief Get the track format at the specified index.
 * @syscap SystemCapability.Multimedia.AVCodec.SourceTrack
 * @param sourceTrack Pointer to an AVSourceTrack instance.
 * @return Returns the track's format.
 * @since 10
 * @version 1.0
*/
OH_AVFormat *OH_AVSourceTrack_GetTrackFormat(OH_AVSourceTrack *sourceTrack);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVSOURCE_H