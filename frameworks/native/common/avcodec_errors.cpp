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

#include "avcodec_errors.h"
#include <map>
#include <string>

namespace OHOS {
namespace AVCodec {
using ErrorMessageFunc = std::function<std::string(const std::string& param1, const std::string& param2)>;
const std::map<AVCodecServiceErrCode, std::string> AVCS_ERRCODE_INFOS = {
    {AVCSERR_OK,                                    "success"},
    {AVCSERR_NO_MEMORY,                             "no memory"},
    {AVCSERR_INVALID_OPERATION,                     "operation not be permitted"},
    {AVCSERR_INVALID_VAL,                           "invalid argument"},
    {AVCSERR_UNKNOWN,                               "unkown error"},
    {AVCSERR_SERVICE_DIED,                          "avcodec service died"},
    {AVCSERR_CREATE_REC_ENGINE_FAILED,              "create recorder engine failed"},
    {AVCSERR_CREATE_PLAYER_ENGINE_FAILED,           "create player engine failed"},
    {AVCSERR_CREATE_AVMETADATAHELPER_ENGINE_FAILED, "create avmetadatahelper engine failed"},
    {AVCSERR_INVALID_STATE,                         "the state is not support this operation"},
    {AVCSERR_UNSUPPORT,                             "unsupport interface"},
    {AVCSERR_UNSUPPORT_AUD_SRC_TYPE,                "unsupport audio source type"},
    {AVCSERR_UNSUPPORT_AUD_SAMPLE_RATE,             "unsupport audio sample rate"},
    {AVCSERR_UNSUPPORT_AUD_CHANNEL_NUM,             "unsupport audio channel"},
    {AVCSERR_UNSUPPORT_AUD_ENC_TYPE,                "unsupport audio encoder type"},
    {AVCSERR_UNSUPPORT_AUD_PARAMS,                  "unsupport audio params(other params)"},
    {AVCSERR_UNSUPPORT_VID_SRC_TYPE,                "unsupport video source type"},
    {AVCSERR_UNSUPPORT_VID_ENC_TYPE,                "unsupport video encoder type"},
    {AVCSERR_UNSUPPORT_VID_PARAMS,                  "unsupport video params(other params)"},
    {AVCSERR_UNSUPPORT_CONTAINER_TYPE,              "unsupport container format type"},
    {AVCSERR_UNSUPPORT_PROTOCOL_TYPE,               "unsupport protocol type"},
    {AVCSERR_UNSUPPORT_VID_DEC_TYPE,                "unsupport video decoder type"},
    {AVCSERR_UNSUPPORT_AUD_DEC_TYPE,                "unsupport audio decoder type"},
    {AVCSERR_UNSUPPORT_STREAM,                      "internal data stream error"},
    {AVCSERR_UNSUPPORT_FILE,                        "this appears to be a text file"},
    {AVCSERR_UNSUPPORT_SOURCE,                      "unsupport source type"},
    {AVCSERR_AUD_ENC_FAILED,                        "audio encode failed"},
    {AVCSERR_AUD_RENDER_FAILED,                     "audio render failed"},
    {AVCSERR_VID_ENC_FAILED,                        "video encode failed"},
    {AVCSERR_AUD_DEC_FAILED,                        "audio decode failed"},
    {AVCSERR_VID_DEC_FAILED,                        "video decode failed"},
    {AVCSERR_MUXER_FAILED,                          "stream avmuxer failed"},
    {AVCSERR_DEMUXER_FAILED,                        "stream demuxer or parser failed"},
    {AVCSERR_OPEN_FILE_FAILED,                      "open file failed"},
    {AVCSERR_FILE_ACCESS_FAILED,                    "read or write file failed"},
    {AVCSERR_START_FAILED,                          "audio or video start failed"},
    {AVCSERR_PAUSE_FAILED,                          "audio or video pause failed"},
    {AVCSERR_STOP_FAILED,                           "audio or video stop failed"},
    {AVCSERR_SEEK_FAILED,                           "audio or video seek failed"},
    {AVCSERR_NETWORK_TIMEOUT,                       "network timeout"},
    {AVCSERR_NOT_FIND_CONTAINER,                    "not find a demuxer"},
    {AVCSERR_EXTEND_START,                          "extend start error code"},
};

const std::map<AVCodecServiceErrCode, AVCodecServiceExtErrCode> AVCSERRCODE_TO_EXTERRORCODE = {
    {AVCSERR_OK,                                  AVCSERR_EXT_OK},
    {AVCSERR_NO_MEMORY,                           AVCSERR_EXT_NO_MEMORY},
    {AVCSERR_INVALID_OPERATION,                   AVCSERR_EXT_OPERATE_NOT_PERMIT},
    {AVCSERR_INVALID_VAL,                         AVCSERR_EXT_INVALID_VAL},
    {AVCSERR_UNKNOWN,                             AVCSERR_EXT_UNKNOWN},
    {AVCSERR_SERVICE_DIED,                        AVCSERR_EXT_SERVICE_DIED},
    {AVCSERR_CREATE_REC_ENGINE_FAILED,            AVCSERR_EXT_UNKNOWN},
    {AVCSERR_CREATE_PLAYER_ENGINE_FAILED,         AVCSERR_EXT_UNKNOWN},
    {AVCSERR_INVALID_STATE,                       AVCSERR_EXT_INVALID_STATE},
    {AVCSERR_UNSUPPORT,                           AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_AUD_SRC_TYPE,              AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_AUD_SAMPLE_RATE,           AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_AUD_CHANNEL_NUM,           AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_AUD_ENC_TYPE,              AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_AUD_PARAMS,                AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_VID_SRC_TYPE,              AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_VID_ENC_TYPE,              AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_VID_PARAMS,                AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_CONTAINER_TYPE,            AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_PROTOCOL_TYPE,             AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_VID_DEC_TYPE,              AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_AUD_DEC_TYPE,              AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_STREAM,                    AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_FILE,                      AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_UNSUPPORT_SOURCE,                    AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_AUD_RENDER_FAILED,                   AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_AUD_ENC_FAILED,                      AVCSERR_EXT_UNKNOWN},
    {AVCSERR_VID_ENC_FAILED,                      AVCSERR_EXT_UNKNOWN},
    {AVCSERR_AUD_DEC_FAILED,                      AVCSERR_EXT_UNKNOWN},
    {AVCSERR_VID_DEC_FAILED,                      AVCSERR_EXT_UNKNOWN},
    {AVCSERR_MUXER_FAILED,                        AVCSERR_EXT_UNKNOWN},
    {AVCSERR_DEMUXER_FAILED,                      AVCSERR_EXT_UNKNOWN},
    {AVCSERR_OPEN_FILE_FAILED,                    AVCSERR_EXT_UNKNOWN},
    {AVCSERR_FILE_ACCESS_FAILED,                  AVCSERR_EXT_UNKNOWN},
    {AVCSERR_START_FAILED,                        AVCSERR_EXT_UNKNOWN},
    {AVCSERR_PAUSE_FAILED,                        AVCSERR_EXT_UNKNOWN},
    {AVCSERR_STOP_FAILED,                         AVCSERR_EXT_UNKNOWN},
    {AVCSERR_SEEK_FAILED,                         AVCSERR_EXT_UNKNOWN},
    {AVCSERR_NETWORK_TIMEOUT,                     AVCSERR_EXT_TIMEOUT},
    {AVCSERR_NOT_FIND_CONTAINER,                  AVCSERR_EXT_UNSUPPORT},
    {AVCSERR_EXTEND_START,                        AVCSERR_EXT_EXTEND_START},
};

const std::map<AVCodecServiceExtErrCode, std::string> AVCSEXTERRCODE_INFOS = {
    {AVCSERR_EXT_OK,                    "success"},
    {AVCSERR_EXT_NO_MEMORY,             "no memory"},
    {AVCSERR_EXT_OPERATE_NOT_PERMIT,    "operation not be permitted"},
    {AVCSERR_EXT_INVALID_VAL,           "invalid argument"},
    {AVCSERR_EXT_IO,                    "IO error"},
    {AVCSERR_EXT_TIMEOUT,               "network timeout"},
    {AVCSERR_EXT_UNKNOWN,               "unkown error"},
    {AVCSERR_EXT_SERVICE_DIED,          "avcodec service died"},
    {AVCSERR_EXT_INVALID_STATE,         "the state is not support this operation"},
    {AVCSERR_EXT_UNSUPPORT,             "unsupport interface"},
    {AVCSERR_EXT_EXTEND_START,          "extend err start"},
};

std::string ErrorMessageOk(const std::string& param1, const std::string& param2)
{
    (void)param1;
    (void)param2;
    return "success";
}

std::string ErrorMessageNoPermission(const std::string& param1, const std::string& param2)
{
    std::string message = "Try to do " + param1 + " failed. User should request permission " + param2 +" first.";
    return message;
}

std::string ErrorMessageInvalidParameter(const std::string& param1, const std::string& param2)
{
    (void)param2;
    std::string message = "The Parameter " + param1 + " is invalid. Please check the type and range.";
    return message;
}

std::string ErrorMessageUnsupportCapability(const std::string& param1, const std::string& param2)
{
    (void)param2;
    std::string message = "Function " + param1 + " can not work correctly due to limited device capability.";
    return message;
}

std::string ErrorMessageNoMemory(const std::string& param1, const std::string& param2)
{
    (void)param2;
    std::string message = "Create " + param1 + " failed due to system memory.";
    return message;
}

std::string ErrorMessageOperateNotPermit(const std::string& param1, const std::string& param2)
{
    (void)param2;
    std::string message = "The operate " + param1 + " failed due to not permit in current state.";
    return message;
}

std::string ErrorMessageIO(const std::string& param1, const std::string& param2)
{
    (void)param2;
    std::string message = "IO error happened due to " + param1 + ".";
    return message;
}

std::string ErrorMessageTimeout(const std::string& param1, const std::string& param2)
{
    std::string message = "Timeout happend when " + param1 + " due to " + param2 + ".";
    return message;
}

std::string ErrorMessageServiceDied(const std::string& param1, const std::string& param2)
{
    (void)param1;
    (void)param2;
    std::string message = "AVCodec Serviced Died.";
    return message;
}

std::string ErrorMessageUnsupportFormat(const std::string& param1, const std::string& param2)
{
    (void)param2;
    std::string message = "The format " + param1 + " is not support.";
    return message;
}

std::string AVCSErrorToString(AVCodecServiceErrCode code)
{
    if (AVCS_ERRCODE_INFOS.count(code) != 0) {
        return AVCS_ERRCODE_INFOS.at(code);
    }

    if (code > AVCSERR_EXTEND_START) {
        return "extend error:" + std::to_string(static_cast<int32_t>(code - AVCSERR_EXTEND_START));
    }

    return "invalid error code:" + std::to_string(static_cast<int32_t>(code));
}

std::string AVCSExtErrorToString(AVCodecServiceExtErrCode code)
{
    if (AVCSEXTERRCODE_INFOS.count(code) != 0) {
        return AVCSEXTERRCODE_INFOS.at(code);
    }

    if (code > AVCSERR_EXT_EXTEND_START) {
        return "extend error:" + std::to_string(static_cast<int32_t>(code - AVCSERR_EXTEND_START));
    }

    return "invalid error code:" + std::to_string(static_cast<int32_t>(code));
}

std::string AVCSErrorToExtErrorString(AVCodecServiceErrCode code)
{
    if (AVCS_ERRCODE_INFOS.count(code) != 0 && AVCSERRCODE_TO_EXTERRORCODE.count(code) != 0) {
        AVCodecServiceExtErrCode extCode = AVCSERRCODE_TO_EXTERRORCODE.at(code);
        if (AVCSEXTERRCODE_INFOS.count(extCode) != 0) {
            return AVCSEXTERRCODE_INFOS.at(extCode);
        }
    }

    return "unkown error";
}

AVCodecServiceExtErrCode AVCSErrorToExtError(AVCodecServiceErrCode code)
{
    if (AVCS_ERRCODE_INFOS.count(code) != 0 && AVCSERRCODE_TO_EXTERRORCODE.count(code) != 0) {
        return AVCSERRCODE_TO_EXTERRORCODE.at(code);
    }

    return AVCSERR_EXT_UNKNOWN;
}

} // namespace AVCodec
} // namespace OHOS
