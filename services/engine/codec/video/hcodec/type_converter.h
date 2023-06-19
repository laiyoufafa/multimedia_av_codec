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
 *
 * Description: header of Type converter from framework to OMX
 */

#ifndef TYPE_CONVERTER_H
#define TYPE_CONVERTER_H

#include <cstdint>
#include "av_common.h"  // foundation/multimedia/av_codec/interfaces/inner_api/native/
#include "avcodec_info.h"
#include "media_description.h"
#include "OMX_IVCommon.h"  // third_party/openmax/api/1.1.2
#include "OMX_Video.h"
#include "OMX_VideoExt.h"
#include "codec_omx_ext.h"
#include "surface_type.h" // foundation/graphic/graphic_2d/interfaces/inner_api/surface/
#include "v1_0/codec_types.h"

namespace OHOS::MediaAVCodec {

struct OmxCodingType {
    OMX_VIDEO_CODINGTYPE type;
    OHOS::HDI::Codec::V1_0::AvCodecRole role;
};

class TypeConverter {
public:
    // coding type
    static OMX_VIDEO_CODINGTYPE HdiRoleToOmxCodingType(OHOS::HDI::Codec::V1_0::AvCodecRole role);
    static std::optional<OmxCodingType> MimeToOmxCodingType(const std::string &mime);
    // pixel format
    static std::optional<GraphicPixelFormat> InnerFmtToDisplayFmt(VideoPixelFormat format);
    static std::optional<GraphicPixelFormat> OmxFmtToDisplayFmt(OMX_COLOR_FORMATTYPE format);
    static std::optional<VideoPixelFormat> OmxFmtToInnerFmt(OMX_COLOR_FORMATTYPE format);
    // rotate
    static std::optional<GraphicTransformType> InnerRotateToDisplayRotate(VideoRotation rotate);
    // color aspects
    static RangeType RangeFlagToOmxRangeType(bool isFullRange) { return isFullRange ? RANGE_FULL : RANGE_LIMITED; }
    static Primaries InnerPrimaryToOmxPrimary(ColorPrimary primary);
    static Transfer InnerTransferToOmxTransfer(TransferCharacteristic transfer);
    static MatrixCoeffs InnerMatrixToOmxMatrix(MatrixCoefficient matrix);

    static std::optional<OMX_VIDEO_AVCPROFILETYPE> AvcProfileToOmxAvcProfile(int32_t avcProfile);
    static std::optional<int32_t> HdiAvcProfileToAvcProfile(OHOS::HDI::Codec::V1_0::Profile hdiAvcProfile);

    static std::optional<CodecHevcProfile> HevcProfileToOmxHevcProfile(int32_t hevcProfile);
    static std::optional<int32_t> HdiHevcProfileToHevcProfile(OHOS::HDI::Codec::V1_0::Profile hdiHevcProfile);
};

}
#endif // TYPE_CONVERTER_H
