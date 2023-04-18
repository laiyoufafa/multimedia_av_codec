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
namespace MediaAVCodec {
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVDemuxerServer"};
}

std::shared_ptr<IAVDemuxer> AVDemuxerServer::Create()
{
    std::shared_ptr<AVDemuxerServer> demuxerServer = std::make_shared<AVDemuxerServer>();
    CHECK_AND_RETURN_RET_LOG(demuxerServer != nullptr, nullptr, "Demuxer Service does not exist");
    int32_t ret = demuxerServer->InitServer();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init demuxer server");
    return demuxerServer;
}

AVDemuxerServer::AVDemuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVDemuxerServer::~AVDemuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    std::lock_guard<std::mutex> lock(mutex_);
    // demuxerEngine_ = nullptr;
}

int32_t AVDemuxerServer::InitServer()
{
    // 引擎创建

    return AVCS_ERR_OK;
}

int32_t AVDemuxerServer::Init(uint64_t attr)
{
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t AVDemuxerServer::AddSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");

    // OH_AVErrCode ret = demuxerEngine_->AddSourceTrackByID(index);
    // CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    // return index; // only for test
    return AVCS_ERR_OK;
}

int32_t AVDemuxerServer::RemoveSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");

    // OH_AVErrCode ret = demuxerEngine_->AddSourceTrackByID(index);
    // CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    // return index; // only for test
    return AVCS_ERR_OK;
}

int32_t AVDemuxerServer::CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");

    // OH_AVErrCode ret = demuxerEngine_->AddSourceTrackByID(index);
    // CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    return AVCS_ERR_OK;
}

int32_t AVDemuxerServer::SeekToTimeStamp(int64_t mSeconds, const AVSeekMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");

    // OH_AVErrCode ret = demuxerEngine_->AddSourceTrackByID(index);
    // CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    printf("SeekToTimeStamp: %lld %d\n", mSeconds, int(mode)); // only for test

    AVCODEC_LOGI("SeekToTimeStamp: %{public}lld", mSeconds);
    // return int(mode); // only for test
    return AVCS_ERR_OK;
}
}  // namespace MediaAVCodec
}  // namespace OHOS