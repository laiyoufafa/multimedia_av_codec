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

#include "demuxer_client.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "DemuxerClient"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<DemuxerClient> DemuxerClient::Create(const sptr<IStandardDemuxerService> &ipcProxy)
{
    std::shared_ptr<DemuxerClient> demuxer = std::make_shared<DemuxerClient>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, nullptr, "Failed to create demuxer client");
    return demuxer;
}

DemuxerClient::DemuxerClient(const sptr<IStandardDemuxerService> &ipcProxy)
    : demuxerProxy_(ipcProxy)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

DemuxerClient::~DemuxerClient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (demuxerProxy_ != nullptr) {
        (void)demuxerProxy_->DestroyStub();
        demuxerProxy_ = nullptr;
    }
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t DemuxerClient::Init(uintptr_t sourceAddr)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "source service does not exist.");

    AVCODEC_LOGD("demuxer client call Init");
    return demuxerProxy_->Init(sourceAddr);
}

void DemuxerClient::DemuxerClient::AVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    demuxerProxy_ = nullptr;
}


int32_t DemuxerClient::SelectSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");

    AVCODEC_LOGD("demuxer client call SelectSourceTrackByID");
    return demuxerProxy_->SelectSourceTrackByID(index);
}
int32_t DemuxerClient::UnselectSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");
    
    AVCODEC_LOGD("demuxer client call UnselectSourceTrackByID");
    return demuxerProxy_->UnselectSourceTrackByID(index);
}
int32_t DemuxerClient::CopyNextSample(uint32_t &trackIndex, uint8_t *buffer,
                                      AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");
    AVCODEC_LOGD("demuxer client call CopyNextSample");
    return demuxerProxy_->CopyNextSample(trackIndex, buffer, bufferInfo, flag);
}
int32_t DemuxerClient::SeekToTime(int64_t mSeconds, const AVSeekMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");
    AVCODEC_LOGD("demuxer client call SeekToTime");
    return demuxerProxy_->SeekToTime(mSeconds, mode);
}
}  // namespace Media
}  // namespace OHOS