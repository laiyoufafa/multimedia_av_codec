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

#ifndef NATIVE_AVCODEC_CODECLIST_H
#define NATIVE_AVCODEC_CODECLIST_H

#include "native_avformat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Find the supported video encoder name by format(must contains video MIME).
 * @param format Indicates a media description which contains required video encoder capability.
 * @return  Returns video encoder name, if not find, return null.
 * @since 10
 * @version 1.0
 */
char *OH_AVCodec_FindVideoDecoder(const OH_AVFormat *format);

/**
 * @brief Find the supported video decoder name by format(must contains video MIME).
 * @param format Indicates a media description which contains required video decoder capability.
 * @return  Returns video decoder name, if not find, return null.
 * @since 10
 * @version 1.0
 */
char *OH_AVCodec_FindVideoDecoder(const OH_AVFormat *format);

/**
 * @brief Find the supported audio encoder name by format(must contains audio MIME).
 * @param format Indicates a media description which contains required audio encoder capability.
 * @return  Returns audio encoder name, if not find, return null.
 * @since 10
 * @version 1.0
 */
char *OH_AVCodec_FindAudioEncoder(const OH_AVFormat *format);

/**
 * @brief Find the supported audio decoder name by format(must contains audio MIME).
 * @param format Indicates a media description which contains required audio decoder capability.
 * @return  Returns audio decoder name, if not find, return empty string.
 * @since 10
 * @version 1.0
 */
char *OH_AVCodec_FindAudioDecoder(const OH_AVFormat *format);

/**
 * @brief Get the capabilities by codec name
 * @param codeName Codec name
 * @return Returns an array of supported video decoder capability, if not find, return null.
 * @since 10
 * @version 1.0
 */
OH_AVCapability *OH_AVCodec_GetCapability(const char *name);

#ifdef __cplusplus
}
#endif

#endif //NATIVE_AVCODEC_CODECLIST_H