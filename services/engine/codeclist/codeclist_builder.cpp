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

#include "avcodec_log.h"
#include "fcodec.h"
#include "codeclist_builder.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecList_builder"};
}
namespace OHOS {
namespace Media {
int32_t VideoCodecList::GetCapabilityList(std::vector<CapabilityData> &caps)
{
    auto ret = Codec::FCodec::GetCodecCapability(caps);
    if (ret == AVCS_ERR_OK) {
        AVCODEC_LOGI("Get capability from fcodec successful");
    }
    return ret;
}

int32_t AudioCodecList::GetCapabilityList(std::vector<CapabilityData> &caps)
{
    (void)caps;
    return 0;
}
} // namespace Media
} // namespace OHOS