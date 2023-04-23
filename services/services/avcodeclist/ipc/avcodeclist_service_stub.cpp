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

#include "avcodeclist_service_stub.h"
#include <unistd.h>
#include "avsharedmemory_ipc.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_server_manager.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecListServiceStub"};
}

namespace OHOS {
namespace Media {
sptr<AVCodecListServiceStub> AVCodecListServiceStub::Create()
{
    sptr<AVCodecListServiceStub> codecListStub = new(std::nothrow) AVCodecListServiceStub();
    CHECK_AND_RETURN_RET_LOG(codecListStub != nullptr, nullptr, "failed to new AVCodecListServiceStub");

    int32_t ret = codecListStub->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "failed to codeclist stub init");
    return codecListStub;
}

AVCodecListServiceStub::AVCodecListServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecListServiceStub::~AVCodecListServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVCodecListServiceStub::Init()
{
    codecListServer_ = AVCodecListServer::Create();
    CHECK_AND_RETURN_RET_LOG(codecListServer_ != nullptr, AVCS_ERR_NO_MEMORY, "failed to create AVCodecListServer");
    codecListFuncs_[FIND_DECODER] = &AVCodecListServiceStub::DoFindDecoder;
    codecListFuncs_[FIND_ENCODER] = &AVCodecListServiceStub::DoFindEncoder;
    codecListFuncs_[CREATE_CAPABILITY] = &AVCodecListServiceStub::DoCreateCapability;
    codecListFuncs_[DESTROY] = &AVCodecListServiceStub::DoDestroyStub;
    return AVCS_ERR_OK;
}

int32_t AVCodecListServiceStub::DestroyStub()
{
    codecListServer_ = nullptr;
    AVCodecServerManager::GetInstance().DestroyStubObject(AVCodecServerManager::CODECLIST, AsObject());
    return AVCS_ERR_OK;
}

int AVCodecListServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    AVCODEC_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (AVCodecListServiceStub::GetDescriptor() != remoteDescriptor) {
        AVCODEC_LOGE("Invalid descriptor");
        return AVCS_ERR_INVALID_OPERATION;
    }

    auto itFunc = codecListFuncs_.find(code);
    if (itFunc != codecListFuncs_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            int32_t ret = (this->*memberFunc)(data, reply);
            if (ret != AVCS_ERR_OK) {
                AVCODEC_LOGE("calling memberFunc is failed.");
            }
            return AVCS_ERR_OK;
        }
    }
    AVCODEC_LOGW("AVCodecListServiceStub: no member func supporting, applying default process");

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

std::string AVCodecListServiceStub::FindDecoder(const Format &format)
{
    CHECK_AND_RETURN_RET_LOG(codecListServer_ != nullptr, "", "avcodeclist server is nullptr");
    return codecListServer_->FindDecoder(format);
}

std::string AVCodecListServiceStub::FindEncoder(const Format &format)
{
    CHECK_AND_RETURN_RET_LOG(codecListServer_ != nullptr, "", "avcodeclist server is nullptr");
    return codecListServer_->FindEncoder(format);
}

CapabilityData AVCodecListServiceStub::CreateCapability(const std::string codecName)
{
    CapabilityData capabilityData;
    CHECK_AND_RETURN_RET_LOG(codecListServer_ != nullptr, capabilityData,
        "avcodeclist server is nullptr");
    return codecListServer_->GetCapabilityData(codecName);
}

int32_t AVCodecListServiceStub::DoFindDecoder(MessageParcel &data, MessageParcel &reply)
{
    Format format;
    (void)AVCodecParcel::Unmarshalling(data, format);
    reply.WriteString(FindDecoder(format));
    return AVCS_ERR_OK;
}

int32_t AVCodecListServiceStub::DoFindEncoder(MessageParcel &data, MessageParcel &reply)
{
    Format format;
    (void)AVCodecParcel::Unmarshalling(data, format);
    reply.WriteString(FindEncoder(format));
    return AVCS_ERR_OK;
}

int32_t AVCodecListServiceStub::DoCreateCapability(MessageParcel &data, MessageParcel &reply)
{
    std::string codecName = data.ReadString();
    CapabilityData capabilityData = CreateCapability(codecName);

    (void)AVCodecListParcel::Marshalling(reply, capabilityData);

    return AVCS_ERR_OK;
}

int32_t AVCodecListServiceStub::DoDestroyStub(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    reply.WriteInt32(DestroyStub());
    return AVCS_ERR_OK;
}
} // namespace Media
} // namespace OHOS
