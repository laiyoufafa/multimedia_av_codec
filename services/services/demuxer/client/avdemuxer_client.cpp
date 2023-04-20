#include "demuxer_client.h"
#include "media_errors.h"
#include "av_log.h"
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVDemuxerClient"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVDemuxerClient> AVDemuxerClient::Create(const sptr<IAVDemuxerService> &ipcProxy)
{
    std::shared_ptr<AVDemuxerClient> demuxerClient = std::make_shared<AVDemuxerClient>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(demuxerClient != nullptr, nullptr, "Failed to create demuxer client");
    return demuxerClient;
}

AVDemuxerClient::AVDemuxerClient(const sptr<IAVDemuxerService> &ipcProxy)
    : demuxerProxy_(ipcProxy)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVDemuxerClient::~AVDemuxerClient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (demuxerProxy_ != nullptr) {
        (void)demuxerProxy_->DestroyStub();
        demuxerProxy_ = nullptr;
    }
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void AVDemuxerClient::AVDemuxerClient::AVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    demuxerProxy_ = nullptr;
}


int32_t AVDemuxerClient::AddSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");

    AVCODEC_LOGD("AddSourceTrackByID");
    return demuxerProxy_->AddSourceTrackByID(index);
}
int32_t AVDemuxerClient::RemoveSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");
    
    AVCODEC_LOGD("RemoveSourceTrackByID");
    return demuxerProxy_->RemoveSourceTrackByID(index);
}
int32_t AVDemuxerClient::CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");
    AVCODEC_LOGD("CopyCurrentSampleToBuf");
    return demuxerProxy_->CopyCurrentSampleToBuf(buffer, bufferInfo);
}
int32_t AVDemuxerClient::SeekToTimeStamp(int64_t mSeconds, const SeekMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");
    AVCODEC_LOGD("SeekToTimeStamp");
    return demuxerProxy_->SeekToTimeStamp(mSeconds, mode);
}

}  // namespace Media
}  // namespace OHOS