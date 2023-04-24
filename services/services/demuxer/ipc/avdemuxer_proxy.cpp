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
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "avsharedmemory_ipc.h"
// #include "avcodec_parcel.h"
#include "avcodec_xcollie.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVDemuxerProxy"};
}

namespace OHOS {
namespace Media {
AVDemuxerProxy::AVDemuxerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAVDemuxerService>(impl)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVDemuxerProxy::~AVDemuxerProxy()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVDemuxerProxy::DestroyStub()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVDemuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = -1;
    COLLIE_LISTEN(error = Remote()->SendRequest(DESTROY_STUB, data, reply, option),
        "AVDemuxerProxy::DestroyStub");
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call DestroyStub, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AVDemuxerProxy::Init(uint64_t attr)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(AVDemuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteUint64(attr);
    int32_t error = -1;
    COLLIE_LISTEN(error = Remote()->SendRequest(INIT, data, reply, option),
        "AVDemuxerProxy::Init");
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call Init, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AVDemuxerProxy::AddSourceTrackByID(uint32_t index)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(AVDemuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteInt32(index);
    int32_t error = -1;
    COLLIE_LISTEN(error = Remote()->SendRequest(ADD_SOURCE_TRACK_BY_ID, data, reply, option),
        "AVDemuxerProxy::AddSourceTrackByID");
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call AddSourceTrackByID, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t AVDemuxerProxy::RemoveSourceTrackByID(uint32_t index)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(AVDemuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteInt32(index);
    int32_t error = -1;
    COLLIE_LISTEN(error = Remote()->SendRequest(REMOVE_SOURCE_TRACK_BY_ID, data, reply, option),
        "AVDemuxerProxy::RemoveSourceTrackByID");
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call RemoveSourceTrackByID, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t AVDemuxerProxy::CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(AVDemuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteInt64(bufferInfo->presentationTimeUs);
    data.WriteInt32(bufferInfo->size);
    data.WriteInt32(bufferInfo->offset);
    data.WriteInt32(bufferInfo->flag);

    WriteAVSharedMemoryToParcel(buffer->buffer, parcel);
    WriteAVSharedMemoryToParcel(buffer->metaData, parcel);

    int32_t error = -1;
    COLLIE_LISTEN(error = Remote()->SendRequest(COPY_CURRENT_SAMPLE_TO_BUF, data, reply, option),
        "AVDemuxerProxy::CopyCurrentSampleToBuf");
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call CopyCurrentSampleToBuf, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t AVDemuxerProxy::SeekToTimeStamp(int64_t mSeconds, const SeekMode mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(AVDemuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteInt64(mSeconds);
    data.WriteInt32(static_cast<int32_t>(mode));
    int32_t error = -1;
    COLLIE_LISTEN(error = Remote()->SendRequest(SEEK_TO_TIME_STAMP, data, reply, option),
        "AVDemuxerProxy::SeekToTimeStamp");
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call SeekToTimeStamp, error: %{public}d", error);
    return reply.ReadInt32();
}
}  // namespace Media
}  // namespace OHOS