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
int32_t MuxerCapiMock::Destroy()
{
    if (muxer_ != nullptr) {
        int ret = OH_AVMuxer_Destroy(muxer_);
        if (ret != AV_ERR_OK) {
            muxer_ = nullptr;
            return ret;
        }
        return AV_ERR_OK;
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerCapiMock::Start()
{
    if (muxer_ != nullptr) {
        return OH_AVMuxer_Start(muxer_);
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerCapiMock::Stop()
{
    if (muxer_ != nullptr) {
        return OH_AVMuxer_Stop(muxer_);
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerCapiMock::AddTrack(int32_t &trackIndex, std::shared_ptr<FormatMock> &trackFormat)
{
    if (muxer_ != nullptr) {
        auto formatMock = std::static_pointer_cast<AVFormatCapiMock>(trackFormat);
        return OH_AVMuxer_AddTrack(muxer_, &trackIndex, formatMock->GetFormat());
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerCapiMock::WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, const AVCodecBufferAttrMock &info)
{
    if (muxer_ != nullptr) {
        OH_AVCodecBufferAttr bufferAttr;
        bufferAttr.pts = info.pts;
        bufferAttr.size = info.size;
        bufferAttr.offset = info.offset;
        bufferAttr.flags = info.flags;
        // bufferAttr.trackId = info.trackId;
        return OH_AVMuxer_WriteSampleBuffer(muxer_, trackIndex, sampleBuffer, bufferAttr);
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerCapiMock::SetLocation(float latitude, float longitude)
{
    if (muxer_ != nullptr) {
        return OH_AVMuxer_SetLocation(muxer_, latitude, longitude);
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerCapiMock::SetRotation(int32_t rotation)
{
    if (muxer_ != nullptr) {
        return OH_AVMuxer_SetRotation(muxer_, rotation);
    }
    return AV_ERR_UNKNOWN;
}
} // namespace Media
} // namespace OHOS