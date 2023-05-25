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

#include "codeclist_service_proxy.h"
#include "avcodec_parcel.h"
#include "codeclist_parcel.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecListServiceProxy"};
}

namespace OHOS {
namespace Media {
CodecListServiceProxy::CodecListServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardCodecListService>(impl)
{
    AVCODEC_LOGD("Create codeclist proxy successful");
}

CodecListServiceProxy::~CodecListServiceProxy()
{
    AVCODEC_LOGD("Destroy codecList proxy successful");
}

std::string CodecListServiceProxy::FindDecoder(const Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(CodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, "", "Failed to write descriptor!");
    (void)AVCodecParcel::Marshalling(data, format);
    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(AVCodecListServiceMsg::FIND_DECODER), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, "", "Find decoder failed");
    return reply.ReadString();
}

std::string CodecListServiceProxy::FindEncoder(const Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(CodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, "", "Failed to write descriptor!");
    (void)AVCodecParcel::Marshalling(data, format);
    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(AVCodecListServiceMsg::FIND_ENCODER), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, "", "Find encoder failed");
    return reply.ReadString();
}

CapabilityData CodecListServiceProxy::GetCapability(const std::string &mime, bool isEncoder, AVCodecCategory category)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    Format profileFormat;
    CapabilityData capabilityData;
    bool token = data.WriteInterfaceToken(CodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, capabilityData, "Write descriptor failed!");
    (void)data.WriteString(mime);
    (void)data.WriteBool(isEncoder);
    (void)data.WriteInt32(static_cast<int32_t>(category));
    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(AVCodecListServiceMsg::GET_CAPABILITY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, capabilityData, "GetCodecCapabilityInfos failed, send request error");
    (void)CodecListParcel::Unmarshalling(reply, capabilityData);

    return capabilityData;
}

int32_t CodecListServiceProxy::DestroyStub()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(CodecListServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Write descriptor failed!");
    int32_t ret = Remote()->SendRequest(static_cast<uint32_t>(AVCodecListServiceMsg::DESTROY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_INVALID_OPERATION,
                             "Destroy codeclist stub failed, send request error");
    return reply.ReadInt32();
}
} // namespace Media
} // namespace OHOS