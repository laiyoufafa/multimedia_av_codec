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

#include "avmuxer_impl.h"
#include "avcodec_log.h"
#include "i_avcodec_service.h"


namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVMuxerImpl"};
}

namespace OHOS {
namespace AVCodec {
std::shared_ptr<AVMuxer> AVMuxerFactory::CreateAVMuxer(int32_t fd, OutputFormat format)
{
    std::shared_ptr<AVMuxerImpl> muxerImpl = std::make_shared<AVMuxerImpl>(fd, format);
    CHECK_AND_RETURN_RET_LOG(muxerImpl != nullptr, nullptr, "Failed to create avmuxer implementation");

    int32_t ret = muxerImpl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init avmuxer implementation");
    return muxerImpl;
}

AVMuxerImpl::AVMuxerImpl(int32_t fd, OutputFormat format) : fd_(fd), format_(format)
{
    (void)fd_;
    (void)format_;
    AVCODEC_LOGD("AVMuxerImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVMuxerImpl::~AVMuxerImpl()
{
    AVCODEC_LOGD("AVMuxerImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVMuxerImpl::Init()
{
    muxerService_ = AVCodecServiceFactory::GetInstance().CreateMuxerService();
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_UNKNOWN, "failed to create muxer service");
    return muxerService_->Init();
}

int32_t AVMuxerImpl::SetLocation(float latitude, float longitude)
{
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer service does not exist");
    return muxerService_->SetLocation(latitude, longitude);
}

int32_t AVMuxerImpl::SetRotation(int32_t rotation)
{
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer service does not exist");
    return muxerService_->SetRotation(rotation);
}

int32_t AVMuxerImpl::SetParameter(const Format &generalFormat)
{
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer service does not exist");
    return muxerService_->SetParameter(generalFormat);
}

int32_t AVMuxerImpl::AddTrack(const Format &trackFormat)
{
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer service does not exist");
    return muxerService_->AddTrack(trackFormat);
}

int32_t AVMuxerImpl::Start()
{
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer service does not exist");
    return muxerService_->Start();
}

int32_t AVMuxerImpl::WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info)
{
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer service does not exist");
    CHECK_AND_RETURN_RET_LOG(sampleBuffer != nullptr && info.size >= 0 && info.presentationTimeUs >= 0, 
                            AVCS_ERR_NO_MEMORY, "Invalid memory");
    return muxerService_->WriteSampleBuffer(trackIndex, sampleBuffer, info);
}

int32_t AVMuxerImpl::Stop()
{
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer service does not exist");
    return muxerService_->Stop();
}
} // namespace AVCodec
} // namespace OHOS