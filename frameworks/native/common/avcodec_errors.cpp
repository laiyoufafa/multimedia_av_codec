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
#include "native_averrors.h"

namespace OHOS {
namespace AVCodec {
using ErrorMessageFunc = std::function<std::string(const std::string& param1, const std::string& param2)>;
const std::map<AVCodecServiceErrCode, std::string> AVCS_ERRCODE_INFOS = {
    {AVCS_ERR_OK,                                    "success"},
    {AVCS_ERR_NO_MEMORY,                             "no memory"},
    {AVCS_ERR_INVALID_OPERATION,                     "operation not be permitted"},
    {AVCS_ERR_INVALID_VAL,                           "invalid argument"},
    {AVCS_ERR_UNKNOWN,                               "unkown error"},
    {AVCS_ERR_SERVICE_DIED,                          "avcodec service died"},
    {AVCS_ERR_CREATE_AVCODEC_SUB_SERVICE_FAILED,     "create avcodec sub service failed"},
    {AVCS_ERR_CREATE_MUXER_SUB_SERVICE_FAILED,       "create muxer sub service failed"},
    {AVCS_ERR_CREATE_DEMUXER_SUB_SERVICE_FAILED,     "create demuxer sub service failed"},
    {AVCS_ERR_INVALID_STATE,                         "the state is not support this operation"},
    {AVCS_ERR_UNSUPPORT,                             "unsupport interface"},
    {AVCS_ERR_UNSUPPORT_AUD_SRC_TYPE,                "unsupport audio source type"},
    {AVCS_ERR_UNSUPPORT_AUD_SAMPLE_RATE,             "unsupport audio sample rate"},
    {AVCS_ERR_UNSUPPORT_AUD_CHANNEL_NUM,             "unsupport audio channel"},
    {AVCS_ERR_UNSUPPORT_AUD_ENC_TYPE,                "unsupport audio encoder type"},
    {AVCS_ERR_UNSUPPORT_AUD_PARAMS,                  "unsupport audio params(other params)"},
    {AVCS_ERR_UNSUPPORT_VID_SRC_TYPE,                "unsupport video source type"},
    {AVCS_ERR_UNSUPPORT_VID_ENC_TYPE,                "unsupport video encoder type"},
    {AVCS_ERR_UNSUPPORT_VID_PARAMS,                  "unsupport video params(other params)"},
    {AVCS_ERR_UNSUPPORT_CONTAINER_TYPE,              "unsupport container format type"},
    {AVCS_ERR_UNSUPPORT_PROTOCOL_TYPE,               "unsupport protocol type"},
    {AVCS_ERR_UNSUPPORT_VID_DEC_TYPE,                "unsupport video decoder type"},
    {AVCS_ERR_UNSUPPORT_AUD_DEC_TYPE,                "unsupport audio decoder type"},
    {AVCS_ERR_UNSUPPORT_STREAM,                      "internal data stream error"},
    {AVCS_ERR_UNSUPPORT_FILE,                        "this appears to be a text file"},
    {AVCS_ERR_UNSUPPORT_SOURCE,                      "unsupport source type"},
    {AVCS_ERR_AUD_ENC_FAILED,                        "audio encode failed"},
    {AVCS_ERR_AUD_RENDER_FAILED,                     "audio render failed"},
    {AVCS_ERR_VID_ENC_FAILED,                        "video encode failed"},
    {AVCS_ERR_AUD_DEC_FAILED,                        "audio decode failed"},
    {AVCS_ERR_VID_DEC_FAILED,                        "video decode failed"},
    {AVCS_ERR_MUXER_FAILED,                          "stream avmuxer failed"},
    {AVCS_ERR_DEMUXER_FAILED,                        "stream demuxer or parser failed"},
    {AVCS_ERR_OPEN_FILE_FAILED,                      "open file failed"},
    {AVCS_ERR_FILE_ACCESS_FAILED,                    "read or write file failed"},
    {AVCS_ERR_START_FAILED,                          "audio or video start failed"},
    {AVCS_ERR_PAUSE_FAILED,                          "audio or video pause failed"},
    {AVCS_ERR_STOP_FAILED,                           "audio or video stop failed"},
    {AVCS_ERR_SEEK_FAILED,                           "audio or video seek failed"},
    {AVCS_ERR_NETWORK_TIMEOUT,                       "network timeout"},
    {AVCS_ERR_NOT_FIND_CONTAINER,                    "not find a demuxer"},
    {AVCS_ERR_EXTEND_START,                          "extend start error code"},
};

const std::map<AVCodecServiceErrCode, AVCodecServiceExtErrCode> AVCSERRCODE_TO_OHAVCODECERRCODE = {
    {AVCS_ERR_OK,                                  AVCODEC_ERR_OK},
    {AVCS_ERR_NO_MEMORY,                           AVCODEC_ERR_NO_MEMORY},
    {AVCS_ERR_INVALID_OPERATION,                   AVCODEC_ERR_OPERATE_NOT_PERMIT},
    {AVCS_ERR_INVALID_VAL,                         AVCODEC_ERR_INVALID_VAL},
    {AVCS_ERR_UNKNOWN,                             AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_SERVICE_DIED,                        AVCODEC_ERR_SERVICE_DIED},
    {AVCS_ERR_CREATE_AVCODEC_SUB_SERVICE_FAILED,   AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_CREATE_MUXER_SUB_SERVICE_FAILED,     AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_CREATE_DEMUXER_SUB_SERVICE_FAILED,   AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_INVALID_STATE,                       AVCODEC_ERR_INVALID_STATE},
    {AVCS_ERR_UNSUPPORT,                           AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_AUD_SRC_TYPE,              AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_AUD_SAMPLE_RATE,           AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_AUD_CHANNEL_NUM,           AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_AUD_ENC_TYPE,              AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_AUD_PARAMS,                AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_VID_SRC_TYPE,              AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_VID_ENC_TYPE,              AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_VID_PARAMS,                AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_CONTAINER_TYPE,            AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_PROTOCOL_TYPE,             AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_VID_DEC_TYPE,              AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_AUD_DEC_TYPE,              AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_STREAM,                    AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_FILE,                      AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_UNSUPPORT_SOURCE,                    AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_AUD_RENDER_FAILED,                   AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_AUD_ENC_FAILED,                      AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_VID_ENC_FAILED,                      AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_AUD_DEC_FAILED,                      AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_VID_DEC_FAILED,                      AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_MUXER_FAILED,                        AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_DEMUXER_FAILED,                      AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_OPEN_FILE_FAILED,                    AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_FILE_ACCESS_FAILED,                  AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_START_FAILED,                        AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_PAUSE_FAILED,                        AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_STOP_FAILED,                         AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_SEEK_FAILED,                         AVCODEC_ERR_UNKNOWN},
    {AVCS_ERR_NETWORK_TIMEOUT,                     AVCODEC_ERR_TIMEOUT},
    {AVCS_ERR_NOT_FIND_CONTAINER,                  AVCODEC_ERR_UNSUPPORT},
    {AVCS_ERR_EXTEND_START,                        AVCODEC_ERR_EXTEND_START},
};

const std::map<AVCodecServiceExtErrCode, std::string> OHAVCODECERRCODE_INFOS = {
    {AVCODEC_ERR_OK,                    "success"},
    {AVCODEC_ERR_NO_MEMORY,             "no memory"},
    {AVCODEC_ERR_OPERATE_NOT_PERMIT,    "operation not be permitted"},
    {AVCODEC_ERR_INVALID_VAL,           "invalid argument"},
    {AVCODEC_ERR_IO,                    "IO error"},
    {AVCODEC_ERR_TIMEOUT,               "network timeout"},
    {AVCODEC_ERR_UNKNOWN,               "unkown error"},
    {AVCODEC_ERR_SERVICE_DIED,          "avcodec service died"},
    {AVCODEC_ERR_INVALID_STATE,         "the state is not support this operation"},
    {AVCODEC_ERR_UNSUPPORT,             "unsupport interface"},
    {AVCODEC_ERR_EXTEND_START,          "extend err start"},
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

    if (code > AVCODEC_ERR_EXTEND_START) {
        return "extend error:" + std::to_string(static_cast<int32_t>(code - AVCODEC_ERR_EXTEND_START));
    }

    return "invalid error code:" + std::to_string(static_cast<int32_t>(code));
}

std::string AVCSExtErrorToString(AVCodecServiceExtErrCode code)
{
    if (OHAVCODECERRCODE_INFOS.count(code) != 0) {
        return OHAVCODECERRCODE_INFOS.at(code);
    }

    if (code > AVCODEC_ERR_EXTEND_START) {
        return "extend error:" + std::to_string(static_cast<int32_t>(code - AVCODEC_ERR_EXTEND_START));
    }

    return "invalid error code:" + std::to_string(static_cast<int32_t>(code));
}

std::string AVCSErrorToExtErrorString(AVCodecServiceErrCode code)
{
    if (AVCS_ERRCODE_INFOS.count(code) != 0 && AVCSERRCODE_TO_OHAVCODECERRCODE.count(code) != 0) {
        AVCodecServiceExtErrCode extCode = AVCSERRCODE_TO_OHAVCODECERRCODE.at(code);
        if (OHAVCODECERRCODE_INFOS.count(extCode) != 0) {
            return OHAVCODECERRCODE_INFOS.at(extCode);
        }
    }

    return "unkown error";
}

AVCodecServiceExtErrCode AVCSErrorToExtError(AVCodecServiceErrCode code)
{
    if (AVCS_ERRCODE_INFOS.count(code) != 0 && AVCSERRCODE_TO_OHAVCODECERRCODE.count(code) != 0) {
        return AVCSERRCODE_TO_OHAVCODECERRCODE.at(code);
    }

    return AVCODEC_ERR_UNKNOWN;
}

} // namespace AVCodec
} // namespace OHOS
