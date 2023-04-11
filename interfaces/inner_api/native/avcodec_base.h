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
namespace AVCodec {
using AudioSampleFormat = OH_AudioSampleFormat;
using AudioChannelSet = OH_AudioChannelSet;
using AudioChannelMask = OH_AudioChannelMask;
using AudioAacProfile = OH_AudioAacProfile;
using VideoPixelFormat = OH_VideoPixelFormat;
using VideoH264Profile = OH_VideoH264Profile;
using OutputFormat = OH_AVOutputFormat ;
using AVCodecBufferFlags = OH_AVCodecBufferFlags;

} // namespace AVCodec
} // namespace OHOS
#endif // AV_BASE_H
