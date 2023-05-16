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

#include "avmuxer_sample.h"
#include "nocopyable.h"
using namespace std;

namespace OHOS {
namespace Media {
AVMuxerSample::AVMuxerSample()
{
}

AVMuxerSample::~AVMuxerSample()
{
}

bool AVMuxerSample::CreateMuxer(int32_t fd, const OutputFormat format)
{
    muxer_ = AVMuxerMockFactory::CreateMuxer(fd, format);
    return muxer_ != nullptr;
}

int32_t AVMuxerSample::Destroy()
{
    if (muxer_ == nullptr) {
        return AV_ERR_INVALID_VAL;
    }
    return muxer_->Destroy();
}

int32_t AVMuxerSample::Start()
{
    if (muxer_ == nullptr) {
        return AV_ERR_INVALID_VAL;
    }
    return muxer_->Start();
}

int32_t AVMuxerSample::Stop()
{
    if (muxer_ == nullptr) {
        return AV_ERR_INVALID_VAL;
    }
    return muxer_->Stop();
}

int32_t AVMuxerSample::AddTrack(int32_t &trackIndex, std::shared_ptr<FormatMock> &trackFormat)
{
    if (muxer_ == nullptr) {
        return AV_ERR_INVALID_VAL;
    }
    return muxer_->AddTrack(trackIndex, trackFormat);
}

int32_t AVMuxerSample::WriteSample(uint32_t trackIndex, uint8_t *sample, const AVCodecBufferAttrMock &info)
{
    if (muxer_ == nullptr) {
        return AV_ERR_INVALID_VAL;
    }
    return muxer_->WriteSample(trackIndex, sample, info);
}

int32_t AVMuxerSample::SetLocation(float latitude, float longitude)
{
    if (muxer_ == nullptr) {
        return AV_ERR_INVALID_VAL;
    }
    return muxer_->SetLocation(latitude, longitude);
}

int32_t AVMuxerSample::SetRotation(int32_t rotation)
{
    if (muxer_ == nullptr) {
        return AV_ERR_INVALID_VAL;
    }
    return muxer_->SetRotation(rotation);
}
}  // namespace Media
}  // namespace OHOS
