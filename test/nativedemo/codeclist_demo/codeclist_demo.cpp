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

#include <iostream>
#include "native_avmagic.h"
#include "codeclist_demo.h"

namespace OHOS {
namespace Media {

void CodecListDemo::RunCase()
{
    std::cout << "===== ============== ======" << std::endl;
    const char *mime = "video/mpeg2";
    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetStringValue(format, "codec_mime", mime);
    const char *codecName = OH_AVCodec_FindDecoder(format);
    std::cout << codecName << std::endl;
    std::cout << "start getting caps" << std::endl;
    OH_AVCapability *cap = OH_AVCodec_CreateCapability(codecName);
    const char *mimetype = OH_AVCapability_GetMimeType(cap);
    CapabilityData capsData = cap->capabilityData_;
    std::cout << mimetype << std::endl;
    std::cout << capsData.maxInstance << std::endl;
    std::cout << "get caps successful" << std::endl;
}
} // namespace Media
} // namespace OHOS
