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
#ifndef MEDIA_ERRORS_H
#define MEDIA_ERRORS_H

#include <map>
#include <string>
#include "errors.h"

namespace OHOS {
namespace Media {
using MSErrCode = ErrCode;

// bit 28~21 is subsys, bit 20~16 is Module. bit 15~0 is code
constexpr MSErrCode MS_MODULE = 0X01000;
constexpr MSErrCode MS_ERR_OFFSET = ErrCodeOffset(SUBSYS_MULTIMEDIA, MS_MODULE);
enum MediaServiceErrCode : ErrCode {
    MSERR_OK                = ERR_OK,
    MSERR_NO_MEMORY         = MS_ERR_OFFSET + ENOMEM, // no memory
    MSERR_INVALID_OPERATION = MS_ERR_OFFSET + ENOSYS, // opertation not be permitted
    MSERR_INVALID_VAL       = MS_ERR_OFFSET + EINVAL, // invalid argument
    MSERR_UNKNOWN           = MS_ERR_OFFSET + 0x200,  // unkown error.
    MSERR_SERVICE_DIED,                               // media service died
    MSERR_CREATE_REC_ENGINE_FAILED,                   // create recorder engine failed.
    MSERR_CREATE_PLAYER_ENGINE_FAILED,                // create player engine failed.
    MSERR_CREATE_AVMETADATAHELPER_ENGINE_FAILED,      // create avmetadatahelper engine failed.
    MSERR_CREATE_AVCODEC_ENGINE_FAILED,               // create avcodec engine failed.
    MSERR_INVALID_STATE,                              // the state is not support this operation.
    MSERR_UNSUPPORT,                                  // unsupport interface.
    MSERR_UNSUPPORT_AUD_SRC_TYPE,                     // unsupport audio source type.
    MSERR_UNSUPPORT_AUD_SAMPLE_RATE,                  // unsupport audio sample rate.
    MSERR_UNSUPPORT_AUD_CHANNEL_NUM,                  // unsupport audio channel.
    MSERR_UNSUPPORT_AUD_ENC_TYPE,                     // unsupport audio encoder type.
    MSERR_UNSUPPORT_AUD_PARAMS,                       // unsupport audio params(other params).
    MSERR_UNSUPPORT_VID_SRC_TYPE,                     // unsupport video source type.
    MSERR_UNSUPPORT_VID_ENC_TYPE,                     // unsupport video encoder type.
    MSERR_UNSUPPORT_VID_PARAMS,                       // unsupport video params(other params).
    MSERR_UNSUPPORT_CONTAINER_TYPE,                   // unsupport container format type.
    MSERR_UNSUPPORT_PROTOCOL_TYPE,                    // unsupport protocol type.
    MSERR_UNSUPPORT_VID_DEC_TYPE,                     // unsupport video decoder type.
    MSERR_UNSUPPORT_AUD_DEC_TYPE,                     // unsupport audio decoder type.
    MSERR_UNSUPPORT_STREAM,                           // internal data stream error.
    MSERR_UNSUPPORT_FILE,                             // this appears to be a text file.
    MSERR_UNSUPPORT_SOURCE,                           // unsupport source type.
    MSERR_AUD_RENDER_FAILED,                          // audio render failed.
    MSERR_AUD_ENC_FAILED,                             // audio encode failed.
    MSERR_VID_ENC_FAILED,                             // video encode failed.
    MSERR_AUD_DEC_FAILED,                             // audio decode failed.
    MSERR_VID_DEC_FAILED,                             // video decode failed.
    MSERR_MUXER_FAILED,                               // stream avmuxer failed.
    MSERR_DEMUXER_FAILED,                             // stream demuxer or parser failed.
    MSERR_OPEN_FILE_FAILED,                           // open file failed.
    MSERR_FILE_ACCESS_FAILED,                         // read or write file failed.
    MSERR_START_FAILED,                               // audio/video start failed.
    MSERR_PAUSE_FAILED,                               // audio/video pause failed.
    MSERR_STOP_FAILED,                                // audio/video stop failed.
    MSERR_SEEK_FAILED,                                // audio/video seek failed.
    MSERR_NETWORK_TIMEOUT,                            // network timeout.
    MSERR_NOT_FIND_CONTAINER,                         // not find a demuxer.
    MSERR_DATA_SOURCE_IO_ERROR,                       // media data source IO failed.
    MSERR_DATA_SOURCE_OBTAIN_MEM_ERROR,               // media data source get mem failed.
    MSERR_DATA_SOURCE_ERROR_UNKNOWN,                  // media data source error unknow.
    MSERR_EXTEND_START      = MS_ERR_OFFSET + 0xF000, // extend err start.
};

// media api error code
enum MediaServiceExtErrCode : ErrCode {
    MSERR_EXT_OK = 0,
    MSERR_EXT_NO_MEMORY = 1,           // no memory.
    MSERR_EXT_OPERATE_NOT_PERMIT = 2,  // opertation not be permitted.
    MSERR_EXT_INVALID_VAL = 3,         // invalid argument.
    MSERR_EXT_IO = 4,                  // IO error.
    MSERR_EXT_TIMEOUT = 5,             // network timeout.
    MSERR_EXT_UNKNOWN = 6,             // unknown error.
    MSERR_EXT_SERVICE_DIED = 7,        // media service died.
    MSERR_EXT_INVALID_STATE = 8,       // the state is not support this operation.
    MSERR_EXT_UNSUPPORT = 9,           // unsupport interface.
    MSERR_EXT_EXTEND_START = 100,      // extend err start.
};

// media api9 error code
enum MediaServiceExtErrCodeAPI9 : ErrCode {
    MSERR_EXT_API9_OK = 0,                          // use for determine error
    MSERR_EXT_API9_NO_PERMISSION = 201,             // permission denied (AccessToken).
    MSERR_EXT_API9_PERMISSION_DENIED = 202,         // permission denied (system API).
    MSERR_EXT_API9_INVALID_PARAMETER = 401,         // invalid parameter.
    MSERR_EXT_API9_UNSUPPORT_CAPABILITY = 801,      // unsupport api.
    MSERR_EXT_API9_NO_MEMORY = 5400101,             // no memory.
    MSERR_EXT_API9_OPERATE_NOT_PERMIT = 5400102,    // opertation not be permitted.
    MSERR_EXT_API9_IO = 5400103,                    // IO error.
    MSERR_EXT_API9_TIMEOUT = 5400104,               // opertate timeout.
    MSERR_EXT_API9_SERVICE_DIED = 5400105,          // media service died.
    MSERR_EXT_API9_UNSUPPORT_FORMAT = 5400106,      // unsupport format.
};

__attribute__((visibility("default"))) std::string MSErrorToString(MediaServiceErrCode code);
__attribute__((visibility("default"))) std::string MSExtErrorToString(MediaServiceExtErrCode code);
__attribute__((visibility("default"))) std::string MSErrorToExtErrorString(MediaServiceErrCode code);
__attribute__((visibility("default"))) MediaServiceExtErrCode MSErrorToExtError(MediaServiceErrCode code);

__attribute__((visibility("default"))) std::string MSExtErrorAPI9ToString(MediaServiceExtErrCodeAPI9 code,
    const std::string& param1, const std::string& param2);
__attribute__((visibility("default"))) std::string MSErrorToExtErrorAPI9String(MediaServiceErrCode code,
    const std::string& param1, const std::string& param2);
__attribute__((visibility("default"))) MediaServiceExtErrCodeAPI9 MSErrorToExtErrorAPI9(MediaServiceErrCode code);

__attribute__((visibility("default"))) std::string MSExtAVErrorToString(MediaServiceExtErrCodeAPI9 code);
} // namespace Media
} // namespace OHOS
#endif // MEDIA_ERRORS_H
