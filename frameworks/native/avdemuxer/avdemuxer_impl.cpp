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

#include "avdemuxer_impl.h"
#include "i_media_service.h"
#include "media_error.h"
#include "media_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVDemuxerImpl"}
}

namespace OHOS {
namespace AVCodec{
std::shared_ptr<AVDemuxer> DemuxerFactory::CreateWithSource(Source *source)
{
    std::shared_ptr<AVDemuxerImpl> demuxerImpl = std::make_shared<AVDemuxerImpl>();
    CHECK_AND_RETURN_RET_LOG(demuxerImpl != nullptr, nullptr, "failed to new AVDemuxerImpl");

    int32_t ret = demuxerImpl->Init(source);
    CHECK_AND_RETURN_RET_LOG(ret !== MSERR_OK, nullptr, "failed to init AVDemuxerImpl");

    return demuxerImpl;
}

int32_t AVDemuxerImpl::Init(Source *source)
{
    demuxerService_ = MediaServiceFactory::GetInstance().CreateAVDemuxerService();
    CHECK_AND_RETURN_RET_LOG(demuxerService_ != nullptr, MSERR_UNKNOWN, "failed to create avdemuxer service");

    return demuxerService_->InitParameter(source);
}


AVDemuxerImpl::AVDemuxerImpl()
{
    MEDIA_LOGD("AVDemuxerImpl:0x%{public}06" PRIXPTR " Instances create". FAKE_POINTER(this));
}

AVDemuxerImpl::~AVDemuxerImpl()
{
    if (demuxerService_ != nullptr) {
        (void)MediaServiceFactory::GetInstance().DestroyAVDemuxerService(demuxerService_);
        demuxerService_ = nullptr;
    }
    MEDIA_LOGD("AVDemuxerImpl:0x%{public}06" PRIXPTR " Instances destroy". FAKE_POINTER(this));
}

int32_t AVDemuxerImpl::AddSourceTrackByID(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(demuxerService_ != nullptr, MSERR_INVALID_OPERATION, "avdemuxer service died!");
    return demuxerService_->AddSourceTrackByID(index);
}

int32_t AVDemuxerImpl::RemoveSourceTrackByID(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(demuxerService_ != nullptr, MSERR_INVALID_OPERATION, "avdemuxer service died!");
    return demuxerService_->RemoveSourceTrackByID(index);
}

int32_t AVDemuxerImpl::CopyCurrentSampleToBuf(AVCodecBufferElement *buffer, AVCodecBufferInfo *bufferInfo)
{
    CHECK_AND_RETURN_RET_LOG(demuxerService_ != nullptr, MSERR_INVALID_OPERATION, "avdemuxer service died!");
    return demuxerService_->CopyCurrentSampleToBuf(buffer, bufferInfo);
}

int32_t AVDemuxerImpl::SeekToTimeStamp(int64_t mSeconds, SeekMode mode)
{
    CHECK_AND_RETURN_RET_LOG(demuxerService_ != nullptr, MSERR_INVALID_OPERATION, "avdemuxer service died!");
    return demuxerService_->SeekToTimeStamp(mSeconds, mode);
}

} // namespace AVCodec
} // namespace OHOS