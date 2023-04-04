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
#ifndef NATIVE_AVERRORS_H
#define NATIVE_AVERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief AV error code
 * @syscap SystemCapability.Multimedia.AVCodec.Core
 * @since 10
 * @version 4.0
 */
typedef enum OH_AVCodecErrCode {
    /**
     * the operation completed successfully.
     */
    AVCODEC_ERR_OK = 0,
    /**
     * no memory.
     */
    AVCODEC_ERR_NO_MEMORY = 1,
    /**
     * opertation not be permitted.
     */
    AVCODEC_ERR_OPERATE_NOT_PERMIT = 2,
    /**
     * invalid argument.
     */
    AVCODEC_ERR_INVALID_VAL = 3,
    /**
     * IO error.
     */
    AVCODEC_ERR_IO = 4,
    /**
     * avcodec service timeout.
     */
    AVCODEC_ERR_TIMEOUT = 5,
    /**
     * unknown error.
     */
    AVCODEC_ERR_UNKNOWN = 6,
    /**
     * avcodec service died.
     */
    AVCODEC_ERR_SERVICE_DIED = 7,
    /**
     * the state is not support this operation.
     */
    AVCODEC_ERR_INVALID_STATE = 8,
    /**
     * unsupport interface.
     */
    AVCODEC_ERR_UNSUPPORT = 9,
    /**
     * extend err start.
     */
    AVCODEC_ERR_EXTEND_START = 100,
} OH_AVErrCode;

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVERRORS_H
