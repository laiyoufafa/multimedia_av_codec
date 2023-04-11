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

#include "source_service_proxy.h"
#include "media_log.h"
#include "media_errors.h"
#include "avsharedmemory_ipc.h"
#include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceServiceProxy"};
}

namespace OHOS {
namespace Media {
SourceServiceProxy::SourceServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardSourceService>(impl)
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

SourceServiceProxy::~SourceServiceProxy()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t SourceServiceProxy::DestroyStub()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = Remote()->SendRequest(DESTROY_STUB, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call DestroyStub, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::Init(const std::string &uri)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteString(uri);
    int error = Remote()->SendRequest(INIT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call Init, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::GetTrackCount()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = Remote()->SendRequest(GET_TRACK_COUNT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call GetTrackCount, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::Destroy()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = Remote()->SendRequest(DESTROY, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call Destroy, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::SetParameter(const Format &param, uint32_t trackId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    AVCodecParcel::Marshalling(data, param);
    data.WriteUint32(trackId);
    int error = Remote()->SendRequest(SET_PARAMETER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call SetParameter, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::GetTrackFormat(Format &format, uint32_t trackId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteUint32(trackId);
    int error = Remote()->SendRequest(GET_SOURCE_ATTR, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call GetTrackFormat, error: %{public}d", error);

    AVCodecParcel::Unmarshalling(reply, format);
    return reply.ReadInt32();
}

uint64_t SourceServiceProxy::GetSourceAttr()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = Remote()->SendRequest(DESTROY_STUB, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == MSERR_OK, error, "Failed to call GetSourceAttr, error: %{public}d", error);
    return reply.ReadUint64();
}
}  // namespace Media
}  // namespace OHOS