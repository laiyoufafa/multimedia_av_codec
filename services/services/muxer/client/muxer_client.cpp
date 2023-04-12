/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <mutex>
#include "muxer_client.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MuxerClient"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<MuxerClient> MuxerClient::Create(const sptr<IStandardMuxerService> &ipcProxy)
{
    std::shared_ptr<MuxerClient> muxerClient = std::make_shared<MuxerClient>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(muxerClient != nullptr, nullptr, "Failed to create muxer client");
    return muxerClient;
}

MuxerClient::MuxerClient(const sptr<IStandardMuxerService> &ipcProxy)
    : muxerProxy_(ipcProxy)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

MuxerClient::~MuxerClient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (muxerProxy_ != nullptr) {
        (void)muxerProxy_->DestroyStub();
        muxerProxy_ = nullptr;
    }
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void MuxerClient::AVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    muxerProxy_ = nullptr;
}

int32_t MuxerClient::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(muxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerProxy_->Init();
}

int32_t MuxerClient::SetLocation(float latitude, float longitude)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(muxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerProxy_->SetLocation(latitude, longitude);
}

int32_t MuxerClient::SetRotation(int32_t rotation)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(muxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerProxy_->SetRotation(rotation);
}

int32_t MuxerClient::SetParameter(const Format &generalFormat)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(muxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "codec service does not exist.");
    return muxerProxy_->SetParameter(generalFormat);
}

int32_t MuxerClient::AddTrack(const Format &trackFormat)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(muxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerProxy_->AddTrack(trackFormat);
}

int32_t MuxerClient::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(muxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerProxy_->Start();
}

int32_t MuxerClient::WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sampleBuffer != nullptr, AVCS_ERR_INVALID_VAL, "sampleBuffer is nullptr");
    CHECK_AND_RETURN_RET_LOG(muxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerProxy_->WriteSampleBuffer(trackIndex, sampleBuffer, info);
}

int32_t MuxerClient::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(muxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerProxy_->Stop();
}
}  // namespace Media
}  // namespace OHOS