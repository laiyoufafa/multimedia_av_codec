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

#include "avcodeclist_service_proxy.h"
#include "media_parcel.h"
// #include "avsharedmemory_ipc.h"
#include "av_log.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecListServiceProxy"};
}

namespace OHOS {
namespace AVCodec {
AVCodecListServiceProxy::AVCodecListServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardAVCodecListService>(impl)
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecListServiceProxy::~AVCodecListServiceProxy()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

std::string AVCodecListServiceProxy::FindVideoDecoder(const Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVCodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, "", "Failed to write descriptor!");

    (void)MediaParcel::Marshalling(data, format);
    int32_t ret = Remote()->SendRequest(FIND_VIDEO_DECODER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, "", "FindVideoDecoder failed");
    return reply.ReadString();
}

std::string AVCodecListServiceProxy::FindVideoEncoder(const Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVCodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, "", "Failed to write descriptor!");

    (void)MediaParcel::Marshalling(data, format);
    int32_t ret = Remote()->SendRequest(FIND_VIDEO_ENCODER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, "", "FindVideoEncoder failed");
    return reply.ReadString();
}

std::string AVCodecListServiceProxy::FindAudioDecoder(const Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVCodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, "", "Failed to write descriptor!");

    (void)MediaParcel::Marshalling(data, format);
    int32_t ret = Remote()->SendRequest(FIND_AUDIO_DECODER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, "", "FindAudioDecoder failed");

    return reply.ReadString();
}

std::string AVCodecListServiceProxy::FindAudioEncoder(const Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVCodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, "", "Failed to write descriptor!");

    (void)MediaParcel::Marshalling(data, format);
    int32_t ret = Remote()->SendRequest(FIND_AUDIO_ENCODER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, "", "FindAudioEncoder failed");

    return reply.ReadString();
}

CapabilityData AVCodecListServiceProxy::GetCapabilityData()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    Format profileFormat;
    CapabilityData capabilityData;

    bool token = data.WriteInterfaceToken(AVCodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, capabilityData, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(GET_CAPABILITYDATA, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, capabilityData,
        "GetCodecCapabilityInfos failed, error: %{public}d", ret);
    // (void)AVCodecListParcel::Unmarshalling(reply, capabilityData); // TODO

    return capabilityData;
}

int32_t AVCodecListServiceProxy::DestroyStub()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(AVCodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(DESTROY, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_INVALID_OPERATION,
        "DestroyStub failed, error: %{public}d", ret);
    return reply.ReadInt32();
}
} // namespace Media
} // namespace OHOS
