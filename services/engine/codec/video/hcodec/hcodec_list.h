/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef HCODEC_HCODEC_LIST_H
#define HCODEC_HCODEC_LIST_H

#include "codeclistbase.h"
#include "avcodec_errors.h"
#include "v1_0/codec_types.h"
#include "v1_0/icodec_component_manager.h"

namespace OHOS::MediaAVCodec {

class HCodecList : public CodecListBase {
public:
    HCodecList() = default;
    int32_t GetCapabilityList(std::vector<CapabilityData>& caps) override;
private:
    CapabilityData HdiCapToUserCap(const OHOS::HDI::Codec::V1_0::CodecCompCapability &hdiCap);
    int32_t GetCodecType(const OHOS::HDI::Codec::V1_0::CodecType& hdiCodecType);
    std::string GetCodecMime(const OHOS::HDI::Codec::V1_0::AvCodecRole& hdiRole);
    std::vector<int32_t> GetBitrateMode(const OHOS::HDI::Codec::V1_0::CodecVideoPortCap& hdiVideoCap);
    std::vector<int32_t> GetCodecFormats(const OHOS::HDI::Codec::V1_0::CodecVideoPortCap& hdiVideoCap);
    std::map<ImgSize, Range> GetMeasuredFrameRate(const OHOS::HDI::Codec::V1_0::CodecVideoPortCap& hdiVideoCap);
    std::map<int32_t, std::vector<int32_t>> GetCodecProfileLevels(
        const OHOS::HDI::Codec::V1_0::CodecCompCapability& hdiCap);
    bool IsSupportedVideoCodec(const OHOS::HDI::Codec::V1_0::CodecCompCapability &hdiCap);
};

sptr<OHOS::HDI::Codec::V1_0::ICodecComponentManager> GetManager();
std::vector<OHOS::HDI::Codec::V1_0::CodecCompCapability> GetCapList();

}

#endif