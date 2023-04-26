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


#include "avmuxer_proxy.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "avsharedmemory_ipc.h"
#include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVMuxerProxy"};
}

namespace OHOS {
namespace Media {
AVMuxerProxy::AVMuxerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAVMuxerService>(impl)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVMuxerProxy::~AVMuxerProxy()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVMuxerProxy::DestroyStub()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVMuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor");

    int32_t ret = Remote()->SendRequest(DESTROY_STUB, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call DestroyStub");
    return reply.ReadInt32();
}

int32_t AVMuxerProxy::Init()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVMuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor");

    int32_t ret = Remote()->SendRequest(INIT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call Init");
    return reply.ReadInt32();
}

int32_t AVMuxerProxy::SetLocation(float latitude, float longitude)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVMuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor");

    data.WriteFloat(latitude);
    data.WriteFloat(longitude);
    int32_t ret = Remote()->SendRequest(SET_LOCATION, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetLocation");
    return reply.ReadInt32();
}

int32_t AVMuxerProxy::SetRotation(int32_t rotation)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVMuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor");

    data.WriteInt32(rotation);
    int32_t ret = Remote()->SendRequest(SET_ROTATION, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    return reply.ReadInt32();
}


int32_t AVMuxerProxy::SetParameter(const Format &generalFormat)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVMuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor");

    AVCodecParcel::Marshalling(data, generalFormat);
    int32_t ret = Remote()->SendRequest(SET_PARAMETER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_INVALID_OPERATION, "SetParameter failed");

    return reply.ReadInt32();
}

int32_t AVMuxerProxy::AddTrack(uint32_t &trackIndex, const Format &trackFormat)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVMuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor");

    data.WriteInt32(trackIndex);
    AVCodecParcel::Marshalling(data, trackFormat);
    int32_t ret = Remote()->SendRequest(ADD_TRACK, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call AddTrack");
    
    return reply.ReadInt32();
}

int32_t AVMuxerProxy::Start()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVMuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor");

    int32_t ret = Remote()->SendRequest(START, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call Start");
    return reply.ReadInt32();
}

int32_t AVMuxerProxy::WriteSampleBuffer(uint32_t trackIndex, const std::shared_ptr<AVSharedMemory> &sampleBuffer, AVCodecBufferInfo info)
{
    CHECK_AND_RETURN_RET_LOG(sampleBuffer != nullptr, AVCS_ERR_INVALID_VAL, "sampleBuffer is nullptr");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(AVMuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");
    data.WriteInt32(trackIndex);
    WriteAVSharedMemoryToParcel(sampleBuffer, data);
    data.WriteInt64(info.presentationTimeUs);
    data.WriteInt32(info.size);
    data.WriteInt32(info.offset);
    // data.WriteInt32(info.flag);

    int32_t ret = Remote()->SendRequest(WRITE_SAMPLE_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call WriteTrackSample");
    return reply.ReadInt32();
}

int32_t AVMuxerProxy::Stop()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVMuxerProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int ret = Remote()->SendRequest(STOP, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call Stop");
    return reply.ReadInt32();
}
}  // namespace Media
}  // namespace OHOS