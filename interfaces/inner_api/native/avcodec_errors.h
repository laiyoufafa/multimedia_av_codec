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

namespace OHOS {
namespace AVCodec {
using AVCSErrCode = ErrCode;

// bit 28~21 is subsys, bit 20~16 is Module. bit 15~0 is code
// TODO: confirm module offset
constexpr AVCSErrCode AVCS_MODULE = 0X01000;
constexpr AVCSErrCode AVCS_ERR_OFFSET = ErrCodeOffset(SUBSYS_MULTIMEDIA, AVCS_MODULE);
enum AVCodecServiceErrCode : ErrCode {
    AVCSERR_OK                = ERR_OK,
    AVCSERR_NO_MEMORY         = AVCS_ERR_OFFSET + ENOMEM, // no memory
    AVCSERR_INVALID_OPERATION = AVCS_ERR_OFFSET + ENOSYS, // opertation not be permitted
    AVCSERR_INVALID_VAL       = AVCS_ERR_OFFSET + EINVAL, // invalid argument
    AVCSERR_UNKNOWN           = AVCS_ERR_OFFSET + 0x200,  // unkown error.
    AVCSERR_SERVICE_DIED,                                 // avcodec service died
    AVCSERR_CREATE_REC_ENGINE_FAILED,                     // create recorder engine failed.
    AVCSERR_CREATE_PLAYER_ENGINE_FAILED,                  // create player engine failed.
    AVCSERR_CREATE_AVMETADATAHELPER_ENGINE_FAILED,        // create avmetadatahelper engine failed.
    AVCSERR_CREATE_AVCODEC_ENGINE_FAILED,                 // create avcodec engine failed.
    AVCSERR_INVALID_STATE,                                // the state is not support this operation.
    AVCSERR_UNSUPPORT,                                    // unsupport interface.
    AVCSERR_UNSUPPORT_AUD_SRC_TYPE,                       // unsupport audio source type.
    AVCSERR_UNSUPPORT_AUD_SAMPLE_RATE,                    // unsupport audio sample rate.
    AVCSERR_UNSUPPORT_AUD_CHANNEL_NUM,                    // unsupport audio channel.
    AVCSERR_UNSUPPORT_AUD_ENC_TYPE,                       // unsupport audio encoder type.
    AVCSERR_UNSUPPORT_AUD_PARAMS,                         // unsupport audio params(other params).
    AVCSERR_UNSUPPORT_VID_SRC_TYPE,                       // unsupport video source type.
    AVCSERR_UNSUPPORT_VID_ENC_TYPE,                       // unsupport video encoder type.
    AVCSERR_UNSUPPORT_VID_PARAMS,                         // unsupport video params(other params).
    AVCSERR_UNSUPPORT_CONTAINER_TYPE,                     // unsupport container format type.
    AVCSERR_UNSUPPORT_PROTOCOL_TYPE,                      // unsupport protocol type.
    AVCSERR_UNSUPPORT_VID_DEC_TYPE,                       // unsupport video decoder type.
    AVCSERR_UNSUPPORT_AUD_DEC_TYPE,                       // unsupport audio decoder type.
    AVCSERR_UNSUPPORT_STREAM,                             // internal data stream error.
    AVCSERR_UNSUPPORT_FILE,                               // this appears to be a text file.
    AVCSERR_UNSUPPORT_SOURCE,                             // unsupport source type.
    AVCSERR_AUD_RENDER_FAILED,                            // audio render failed.
    AVCSERR_AUD_ENC_FAILED,                               // audio encode failed.
    AVCSERR_VID_ENC_FAILED,                               // video encode failed.
    AVCSERR_AUD_DEC_FAILED,                               // audio decode failed.
    AVCSERR_VID_DEC_FAILED,                               // video decode failed.
    AVCSERR_MUXER_FAILED,                                 // stream avmuxer failed.
    AVCSERR_DEMUXER_FAILED,                               // stream demuxer or parser failed.
    AVCSERR_OPEN_FILE_FAILED,                             // open file failed.
    AVCSERR_FILE_ACCESS_FAILED,                           // read or write file failed.
    AVCSERR_START_FAILED,                                 // audio/video start failed.
    AVCSERR_PAUSE_FAILED,                                 // audio/video pause failed.
    AVCSERR_STOP_FAILED,                                  // audio/video stop failed.
    AVCSERR_SEEK_FAILED,                                  // audio/video seek failed.
    AVCSERR_NETWORK_TIMEOUT,                              // network timeout.
    AVCSERR_NOT_FIND_CONTAINER,                           // not find a demuxer.
    AVCSERR_DATA_SOURCE_IO_ERROR,                         // avcodec data source IO failed.
    AVCSERR_DATA_SOURCE_OBTAIN_MEM_ERROR,                 // avcodec data source get mem failed.
    AVCSERR_DATA_SOURCE_ERROR_UNKNOWN,                    // avcodec data source error unknow.
    AVCSERR_EXTEND_START      = AVCS_ERR_OFFSET + 0xF000, // extend err start.
};

// avcodec api error code
enum AVCodecServiceExtErrCode : ErrCode {
    AVCSERR_EXT_OK = 0,
    AVCSERR_EXT_NO_MEMORY = 1,           // no memory.
    AVCSERR_EXT_OPERATE_NOT_PERMIT = 2,  // opertation not be permitted.
    AVCSERR_EXT_INVALID_VAL = 3,         // invalid argument.
    AVCSERR_EXT_IO = 4,                  // IO error.
    AVCSERR_EXT_TIMEOUT = 5,             // network timeout.
    AVCSERR_EXT_UNKNOWN = 6,             // unknown error.
    AVCSERR_EXT_SERVICE_DIED = 7,        // avcodec service died.
    AVCSERR_EXT_INVALID_STATE = 8,       // the state is not support this operation.
    AVCSERR_EXT_UNSUPPORT = 9,           // unsupport interface.
    AVCSERR_EXT_EXTEND_START = 100,      // extend err start.
};

__attribute__((visibility("default"))) std::string AVCSErrorToString(AVCodecServiceErrCode code);
__attribute__((visibility("default"))) std::string AVCSExtErrorToString(AVCodecServiceExtErrCode code);
__attribute__((visibility("default"))) std::string AVCSErrorToExtErrorString(AVCodecServiceErrCode code);
__attribute__((visibility("default"))) AVCodecServiceExtErrCode AVCSErrorToExtError(AVCodecServiceErrCode code);

} // namespace AVCodec
} // namespace OHOS
#endif // AVCODEC_ERRORS_H
