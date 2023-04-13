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

#ifndef AVCODEC_ERRORS_H
#define AVCODEC_ERRORS_H

#include <map>
#include <string>
#include "errors.h"
#include "native_averrors.h"

namespace OHOS {
namespace MediaAVCodec {
using AVCSErrCode = ErrCode;

// bit 28~21 is subsys, bit 20~16 is Module. bit 15~0 is code
// TODO: confirm module offset
constexpr AVCSErrCode AVCS_MODULE = 0X01000;
constexpr AVCSErrCode AVCS_ERR_OFFSET = ErrCodeOffset(SUBSYS_MULTIMEDIA, AVCS_MODULE);
typedef enum AVCodecServiceErrCode : ErrCode {
    AVCS_ERR_OK                = ERR_OK,
    AVCS_ERR_NO_MEMORY         = AVCS_ERR_OFFSET + ENOMEM, // no memory
    AVCS_ERR_INVALID_OPERATION = AVCS_ERR_OFFSET + ENOSYS, // opertation not be permitted
    AVCS_ERR_INVALID_VAL       = AVCS_ERR_OFFSET + EINVAL, // invalid argument
    AVCS_ERR_UNKNOWN           = AVCS_ERR_OFFSET + 0x200,  // unkown error.
    AVCS_ERR_SERVICE_DIED,                                 // avcodec service died
    AVCS_ERR_CREATE_AVCODEC_SUB_SERVICE_FAILED,            // create avcodec sub service failed.
    AVCS_ERR_CREATE_MUXER_SUB_SERVICE_FAILED,              // create muxer sub service failed.
    AVCS_ERR_CREATE_DEMUXER_SUB_SERVICE_FAILED,            // create demuxer sub service failed.
    AVCS_ERR_INVALID_STATE,                                // the state is not support this operation.
    AVCS_ERR_UNSUPPORT,                                    // unsupport interface.
    AVCS_ERR_UNSUPPORT_AUD_SRC_TYPE,                       // unsupport audio source type.
    AVCS_ERR_UNSUPPORT_AUD_SAMPLE_RATE,                    // unsupport audio sample rate.
    AVCS_ERR_UNSUPPORT_AUD_CHANNEL_NUM,                    // unsupport audio channel.
    AVCS_ERR_UNSUPPORT_AUD_ENC_TYPE,                       // unsupport audio encoder type.
    AVCS_ERR_UNSUPPORT_AUD_PARAMS,                         // unsupport audio params(other params).
    AVCS_ERR_UNSUPPORT_VID_SRC_TYPE,                       // unsupport video source type.
    AVCS_ERR_UNSUPPORT_VID_ENC_TYPE,                       // unsupport video encoder type.
    AVCS_ERR_UNSUPPORT_VID_PARAMS,                         // unsupport video params(other params).
    AVCS_ERR_UNSUPPORT_CONTAINER_TYPE,                     // unsupport container format type.
    AVCS_ERR_UNSUPPORT_PROTOCOL_TYPE,                      // unsupport protocol type.
    AVCS_ERR_UNSUPPORT_VID_DEC_TYPE,                       // unsupport video decoder type.
    AVCS_ERR_UNSUPPORT_AUD_DEC_TYPE,                       // unsupport audio decoder type.
    AVCS_ERR_UNSUPPORT_STREAM,                             // internal data stream error.
    AVCS_ERR_UNSUPPORT_FILE,                               // this appears to be a text file.
    AVCS_ERR_UNSUPPORT_SOURCE,                             // unsupport source type.
    AVCS_ERR_AUD_RENDER_FAILED,                            // audio render failed.
    AVCS_ERR_AUD_ENC_FAILED,                               // audio encode failed.
    AVCS_ERR_VID_ENC_FAILED,                               // video encode failed.
    AVCS_ERR_AUD_DEC_FAILED,                               // audio decode failed.
    AVCS_ERR_VID_DEC_FAILED,                               // video decode failed.
    AVCS_ERR_MUXER_FAILED,                                 // stream avmuxer failed.
    AVCS_ERR_DEMUXER_FAILED,                               // stream demuxer or parser failed.
    AVCS_ERR_OPEN_FILE_FAILED,                             // open file failed.
    AVCS_ERR_FILE_ACCESS_FAILED,                           // read or write file failed.
    AVCS_ERR_START_FAILED,                                 // audio/video start failed.
    AVCS_ERR_PAUSE_FAILED,                                 // audio/video pause failed.
    AVCS_ERR_STOP_FAILED,                                  // audio/video stop failed.
    AVCS_ERR_SEEK_FAILED,                                  // audio/video seek failed.
    AVCS_ERR_NETWORK_TIMEOUT,                              // network timeout.
    AVCS_ERR_NOT_FIND_CONTAINER,                           // not find a demuxer.
    AVCS_ERR_DATA_SOURCE_IO_ERROR,                         // avcodec data source IO failed.
    AVCS_ERR_DATA_SOURCE_OBTAIN_MEM_ERROR,                 // avcodec data source get mem failed.
    AVCS_ERR_DATA_SOURCE_ERROR_UNKNOWN,                    // avcodec data source error unknow.
    AVCS_ERR_EXTEND_START      = AVCS_ERR_OFFSET + 0xF000, // extend err start.
} AVCodecServiceErrCode;

__attribute__((visibility("default"))) std::string AVCSErrorToString(AVCodecServiceErrCode code);
__attribute__((visibility("default"))) std::string OHAVErrCodeToString(OH_AVErrCode code);
__attribute__((visibility("default"))) std::string AVCSErrorToOHAVErrCodeString(AVCodecServiceErrCode code);
__attribute__((visibility("default"))) OH_AVErrCode AVCSErrorToOHAVErrCode(AVCodecServiceErrCode code);

} // namespace MediaAVCodec
} // namespace OHOS
#endif // AVCODEC_ERRORS_H
