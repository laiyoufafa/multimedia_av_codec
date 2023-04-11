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
#include "demuxer_server.h"
#include "media_errors.h"
#include "av_log.h"
namespace OHOS {
namespace Media {
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "DemuxerServer"};
}

std::shared_ptr<IDemuxerService> DemuxerServer::Create()
{
    std::shared_ptr<DemuxerServer> demuxerServer = std::make_shared<DemuxerServer>();
    CHECK_AND_RETURN_RET_LOG(demuxerServer != nullptr, nullptr, "Demuxer Service does not exist");
    int32_t ret = demuxerServer->InitServer();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "Failed to init demuxer server");
    return demuxerServer;
}

DemuxerServer::DemuxerServer()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

DemuxerServer::~DemuxerServer()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    std::lock_guard<std::mutex> lock(mutex_);
    // demuxerEngine_ = nullptr;
}

int32_t DemuxerServer::InitServer()
{
    // 引擎创建

    return MSERR_OK;
}

int32_t DemuxerServer::Init(uint64_t attr)
{
    std::lock_guard<std::mutex> lock(mutex_);

    return MSERR_OK;
}

int32_t DemuxerServer::AddSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, MSERR_INVALID_OPERATION, "Demuxer engine does not exist");

    // OH_AVErrCode ret = demuxerEngine_->AddSourceTrackByID(index);
    // CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, ret, "Failed to call SetRotation");
    // return index; // only for test
    return MSERR_OK;
}

int32_t DemuxerServer::RemoveSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, MSERR_INVALID_OPERATION, "Demuxer engine does not exist");

    // OH_AVErrCode ret = demuxerEngine_->AddSourceTrackByID(index);
    // CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, ret, "Failed to call SetRotation");
    // return index; // only for test
    return MSERR_OK;
}

int32_t DemuxerServer::CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, MSERR_INVALID_OPERATION, "Demuxer engine does not exist");

    // OH_AVErrCode ret = demuxerEngine_->AddSourceTrackByID(index);
    // CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, ret, "Failed to call SetRotation");
    return MSERR_OK;
}

int32_t DemuxerServer::SeekToTimeStamp(int64_t mSeconds, const SeekMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, MSERR_INVALID_OPERATION, "Demuxer engine does not exist");

    // OH_AVErrCode ret = demuxerEngine_->AddSourceTrackByID(index);
    // CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, ret, "Failed to call SetRotation");
    printf("SeekToTimeStamp: %lld %d\n", mSeconds, int(mode)); // only for test

    MEDIA_LOGI("SeekToTimeStamp: %{public}lld", mSeconds);
    // return int(mode); // only for test
    return MSERR_OK;
}
}  // namespace Media
}  // namespace OHOS