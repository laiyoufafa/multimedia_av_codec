#include "demuxer_client.h"
#include "media_errors.h"
#include "av_log.h"
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "DemuxerClient"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<DemuxerClient> DemuxerClient::Create(const sptr<IStandardDemuxerService> &ipcProxy)
{
    std::shared_ptr<DemuxerClient> demuxerClient = std::make_shared<DemuxerClient>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(demuxerClient != nullptr, nullptr, "Failed to create demuxer client");
    return demuxerClient;
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

void DemuxerClient::DemuxerClient::AVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    demuxerProxy_ = nullptr;
}


int32_t DemuxerClient::AddSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");

    AVCODEC_LOGD("AddSourceTrackByID");
    return demuxerProxy_->AddSourceTrackByID(index);
}
int32_t DemuxerClient::RemoveSourceTrackByID(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");
    
    AVCODEC_LOGD("RemoveSourceTrackByID");
    return demuxerProxy_->RemoveSourceTrackByID(index);
}
int32_t DemuxerClient::CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");
    AVCODEC_LOGD("CopyCurrentSampleToBuf");
    return demuxerProxy_->CopyCurrentSampleToBuf(buffer, bufferInfo);
}
int32_t DemuxerClient::SeekToTimeStamp(int64_t mSeconds, const SeekMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service does not exist.");
    AVCODEC_LOGD("SeekToTimeStamp");
    return demuxerProxy_->SeekToTimeStamp(mSeconds, mode);
}

}  // namespace Media
}  // namespace OHOS