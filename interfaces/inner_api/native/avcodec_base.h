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

#ifndef AV_BASE_H
#define AV_BASE_H

#include <memory>
#include "native_avbase.h"

namespace OHOS {
namespace Media {
using AudioSampleFormat = OH_AudioSampleFormat;
using AudioChannelSet = OH_AudioChannelSet;
using AudioChannelMask = OH_AudioChannelMask;
using AudioAacProfile = OH_AudioAacProfile;
using VideoPixelFormat = OH_VideoPixelFormat;
using VideoH264Profile = OH_VideoH264Profile;
using OutputFormat = OH_AVOutputFormat ;
using AVCodecBufferInfo = OH_AVCodecBufferInfo;
using AVCodecBufferFlags = OH_AVCodecBufferFlags;

enum Status : int32_t {
    CSERR_OK = 0,                     ///< The execution result is correct.
    CSERR_UNKNOWN = -1,               ///< An unknown error occurred.
    CSERR_PLUGIN_ALREADY_EXISTS = -2, ///< The plugin already exists, usually occurs when in plugin registered.
    CSERR_INCOMPATIBLE_VERSION = -3,  ///< Incompatible version, may occur during plugin registration or function calling.
    CSERR_NO_MEMORY = -4,             ///< The system memory is insufficient.
    CSERR_WRONG_STATE = -5,           ///< The function is called in an invalid state.
    CSERR_UNIMPLEMENTED = -6,         ///< This method or interface is not implemented.
    CSERR_INVALID_PARAMETER = -7,     ///< The plugin does not support this parameter.
    CSERR_INVALID_DATA = -8,          ///< The value is not in the valid range.
    CSERR_MISMATCHED_TYPE = -9,       ///< Mismatched data type
    CSERR_TIMED_OUT = -10,            ///< Operation timeout.
    CSERR_UNSUPPORTED_FORMAT = -11,   ///< The plugin not support this format/name.
    CSERR_NOT_ENOUGH_DATA = -12,      ///< Not enough data when read from source.
    CSERR_NOT_EXISTED = -13,          ///< Source is not existed.
    CSERR_AGAIN = -14,                ///< Operation is not available right now, should try again later.
    CSERR_PERMISSION_DENIED = -15,    ///< Permission denied.
    CSERR_NULL_POINTER = -16,         ///< Null pointer.
    CSERR_INVALID_OPERATION = -17,    ///< Invalid operation.
    CSERR_CLIENT = -18,               ///< Http client error
    CSERR_SERVER = -19,               ///< Http server error
    CSERR_DELAY_READY = -20,          ///< Delay ready event
};
} // namespace Media
} // namespace OHOS
#endif // AV_BASE_H
