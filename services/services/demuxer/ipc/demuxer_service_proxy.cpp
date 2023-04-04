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
#include "demuxer_service_proxy.h"
#include "av_log.h"
#include "media_errors.h"
#include "avsharedmemory_ipc.h"
// #include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "DemuxerServiceProxy"};
}

namespace OHOS {
namespace AVCodec {
DemuxerServiceProxy::DemuxerServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardDemuxerService>(impl)
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

DemuxerServiceProxy::~DemuxerServiceProxy()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t DemuxerServiceProxy::DestroyStub()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = Remote()->SendRequest(DESTROY_STUB, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call DestroyStub, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t DemuxerServiceProxy::AddSourceTrackByID(uint32_t index)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    CHECK_AND_RETURN_RET(data.WriteInt32(index), MSERR_UNKNOWN);
    int32_t error = Remote()->SendRequest(ADD_SOURCE_TRACK_BY_ID, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call AddSourceTrackByID, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t DemuxerServiceProxy::RemoveSourceTrackByID(uint32_t index)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    CHECK_AND_RETURN_RET(data.WriteInt32(index), MSERR_UNKNOWN);
    int32_t error = Remote()->SendRequest(REMOVE_SOURCE_TRACK_BY_ID, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call RemoveSourceTrackByID, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t DemuxerServiceProxy::CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteInt64(bufferInfo->presentationTimeUs);
    data.WriteInt32(bufferInfo->size);
    data.WriteInt32(bufferInfo->offset);
    data.WriteInt32(bufferInfo->flag);

    WriteAVSharedMemoryToParcel(buffer->buffer, parcel);
    WriteAVSharedMemoryToParcel(buffer->metaData, parcel);

    int32_t error = Remote()->SendRequest(COPY_CURRENT_SAMPLE_TO_BUF, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call CopyCurrentSampleToBuf, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t DemuxerServiceProxy::SeekToTimeStamp(int64_t mSeconds, const SeekMode mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    CHECK_AND_RETURN_RET(data.WriteInt64(mSeconds), MSERR_UNKNOWN);
    CHECK_AND_RETURN_RET(data.WriteInt32(static_cast<int32_t>(mode)), MSERR_UNKNOWN);
    int32_t error = Remote()->SendRequest(SEEK_TO_TIME_STAMP, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call SeekToTimeStamp, error: %{public}d", error);
    return reply.ReadInt32();
}
}  // namespace AVCodec
}  // namespace OHOS