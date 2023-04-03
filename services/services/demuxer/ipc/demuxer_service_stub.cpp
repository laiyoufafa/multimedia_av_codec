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
#include "demuxer_service_stub.h"
#include "media_server_manager.h"
#include "media_errors.h"
#include "av_log.h"
#include "avsharedmemory_ipc.h"
// #include "media_parcel.h"
namespace OHOS {
namespace AVCodec {
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "DemuxerServiceStub"};
}

sptr<DemuxerServiceStub> DemuxerServiceStub::Create()
{
    sptr<DemuxerServiceStub> demuxerStub = new(std::nothrow) DemuxerServiceStub();
    CHECK_AND_RETURN_RET_LOG(demuxerStub != nullptr, nullptr, "Failed to create avmuxer service stub");

    int32_t ret = demuxerStub->Init();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "Failed to init DemuxerServiceStub");
    return demuxerStub;
}

DemuxerServiceStub::DemuxerServiceStub()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

DemuxerServiceStub::~DemuxerServiceStub()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t DemuxerServiceStub::Init()
{
    demuxerServer_ = DemuxerService::Create();
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, MSERR_NO_MEMORY, "Failed to create muxer server");
    
    demuxerFuncs_[ADD_SOURCE_TRACK_BY_ID] = &DemuxerServiceStub::AddSourceTrackByID;
    demuxerFuncs_[REMOVE_SOURCE_TRACK_BY_ID] = &DemuxerServiceStub::RemoveSourceTrackByID;
    demuxerFuncs_[COPY_CURRENT_SAMPLE_TO_BUF] = &DemuxerServiceStub::CopyCurrentSampleToBuf;
    demuxerFuncs_[SEEK_TO_TIME_STAMP] = &DemuxerServiceStub::SeekToTimeStamp;

    demuxerFuncs_[DESTROY] = &DemuxerServiceStub::DestroyStub;

    return MSERR_OK;
}

int DemuxerServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    MEDIA_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (DemuxerServiceStub::GetDescriptor() != remoteDescriptor) {
        MEDIA_LOGE("Invalid descriptor");
        return MSERR_INVALID_OPERATION;
    }

    auto itFunc = demuxerFuncs_.find(code);
    if (itFunc != demuxerFuncs_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            int32_t ret = (this->*memberFunc)(data, reply);
            CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, ret, "Failed to call memberFunc");
            return MSERR_OK;
        }
    }
    MEDIA_LOGW("Failed to find corresponding function");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DemuxerServiceStub::DestroyStub(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    CHECK_AND_RETURN_RET(reply.WriteInt32(DestroyStub()), MSERR_UNKNOWN);
    return MSERR_OK;
}

int32_t DemuxerServiceStub::DestroyStub()
{
    demuxerServer_ = nullptr;
    MediaServerManager::GetInstance().DestroyStubObject(MediaServerManager::DEMUXER, AsObject());
    return MSERR_OK;
}

int32_t DemuxerServiceStub::AddSourceTrackByID(MessageParcel &data, MessageParcel &reply)
{
    uint32_t index = data.ReadUint32();
    CHECK_AND_RETURN_RET(reply.WriteInt32(AddSourceTrackByID(index)), MSERR_UNKNOWN);   
    return MSERR_OK;
}

int32_t DemuxerServiceStub::RemoveSourceTrackByID(MessageParcel &data, MessageParcel &reply)
{
    uint32_t index = data.ReadUint32();
    CHECK_AND_RETURN_RET(reply.WriteInt32(RemoveSourceTrackByID(index)), MSERR_UNKNOWN);   
    return MSERR_OK;
}

int32_t DemuxerServiceStub::CopyCurrentSampleToBuf(MessageParcel &data, MessageParcel &reply)
{
    AVCodecBufferInfo info;
    info.presentationTimeUs = data.ReadInt64();
    info.size = data.ReadInt32();
    info.offset = data.ReadInt32();
    info.flag = data.ReadInt32();

    AVBufferElement buffer;
    buffer->buffer = ReadAVSharedMemoryFromParcel(parcel);
    buffer->metaData = ReadAVSharedMemoryFromParcel(parcel);

    CHECK_AND_RETURN_RET(reply.WriteInt32(CopyCurrentSampleToBuf(&info, &buffer)), MSERR_UNKNOWN);   
    return MSERR_OK;
}

int32_t DemuxerServiceStub::SeekToTimeStamp(MessageParcel &data, MessageParcel &reply)
{
    int64_t mSeconds = data.ReadInt64();
    int32_t mode = data.ReadInt32();
    SeekMode seekMode = static_cast<SeekMode>(mode);
    CHECK_AND_RETURN_RET(reply.WriteInt32(SeekToTimeStamp(mSeconds, seekMode)), MSERR_UNKNOWN);   
    return MSERR_OK;
}

int32_t DemuxerServiceStub::AddSourceTrackByID(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, MSERR_NO_MEMORY, "Demuxer server is nullptr");
    return demuxerServer_->AddSourceTrackByID(index);
}
int32_t DemuxerServiceStub::RemoveSourceTrackByID(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, MSERR_NO_MEMORY, "Demuxer server is nullptr");
    return demuxerServer_->RemoveSourceTrackByID(index);
}
int32_t DemuxerServiceStub::CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, MSERR_NO_MEMORY, "Demuxer server is nullptr");
    return demuxerServer_->CopyCurrentSampleToBuf(buffer, bufferInfo);
}
int32_t DemuxerServiceStub::SeekToTimeStamp(int64_t mSeconds, const SeekMode mode)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, MSERR_NO_MEMORY, "Demuxer server is nullptr");
    return demuxerServer_->SeekToTimeStamp(mSeconds, mode);
}

}  // namespace AVCodec
}  // namespace OHOS  