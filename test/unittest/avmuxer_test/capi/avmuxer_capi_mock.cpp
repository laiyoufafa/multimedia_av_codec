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

#include "avmuxer_capi_mock.h"

namespace OHOS {
namespace Media {
int32_t AVMuxerCapiMock::Destroy()
{
    int ret = OH_AVMuxer_Destroy(muxer_);
    if (ret != AV_ERR_OK) {
        return ret;
    }
    muxer_ = nullptr;
    return AV_ERR_OK;
}

int32_t AVMuxerCapiMock::Start()
{
    return OH_AVMuxer_Start(muxer_);
}

int32_t AVMuxerCapiMock::Stop()
{
    return OH_AVMuxer_Stop(muxer_);
}

int32_t AVMuxerCapiMock::AddTrack(int32_t &trackIndex, std::shared_ptr<FormatMock> &trackFormat)
{
    auto formatMock = std::static_pointer_cast<AVFormatCapiMock>(trackFormat);
    return OH_AVMuxer_AddTrack(muxer_, &trackIndex, formatMock->GetFormat());
}

int32_t AVMuxerCapiMock::WriteSampleBuffer(uint32_t trackIndex,
    uint8_t *sampleBuffer, const AVCodecBufferAttrMock &info)
{
    OH_AVCodecBufferAttr bufferAttr;
    bufferAttr.pts = info.pts;
    bufferAttr.size = info.size;
    bufferAttr.offset = info.offset;
    bufferAttr.flags = info.flags;
    return OH_AVMuxer_WriteSampleBuffer(muxer_, trackIndex, sampleBuffer, bufferAttr);
}

int32_t AVMuxerCapiMock::SetLocation(float latitude, float longitude)
{
    return OH_AVMuxer_SetLocation(muxer_, latitude, longitude);
}

int32_t AVMuxerCapiMock::SetRotation(int32_t rotation)
{
    return OH_AVMuxer_SetRotation(muxer_, rotation);
}
} // namespace Media
} // namespace OHOS