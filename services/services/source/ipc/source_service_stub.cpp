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

#include "source_service_stub.h"
#include "avcodec_server_manager.h"
#include "media_errors.h"
#include "media_log.h"
#include "avsharedmemory_ipc.h"
#include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceServiceStub"};
}

namespace OHOS {
namespace Media {
sptr<SourceServiceStub> SourceServiceStub::Create()
{
    sptr<SourceServiceStub> sourceStub = new(std::nothrow) SourceServiceStub();
    CHECK_AND_RETURN_RET_LOG(sourceStub != nullptr, nullptr, "Failed to create source service stub");

    int32_t ret = sourceStub->InitStub();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init SourceServiceStub");
    return sourceStub;
}

SourceServiceStub::SourceServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

SourceServiceStub::~SourceServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t SourceServiceStub::InitStub()
{
    sourceServer_ = SourceServer::Create();
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Failed to create muxer server");

    sourceFuncs_[INIT] = &SourceServiceStub::Init;
    sourceFuncs_[GET_TRACK_COUNT] = &SourceServiceStub::GetTrackCount;
    sourceFuncs_[DESTROY] = &SourceServiceStub::Destroy;
    sourceFuncs_[SET_PARAMETER] = &SourceServiceStub::SetParameter;
    sourceFuncs_[GET_TRACK_FORMAT] = &SourceServiceStub::GetTrackFormat;
    sourceFuncs_[GET_SOURCE_ATTR] = &SourceServiceStub::GetSourceAttr;
    sourceFuncs_[DESTROY_STUB] = &SourceServiceStub::DestroyStub;
    return AVCS_ERR_OK;
}

int SourceServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    AVCODEC_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (SourceServiceStub::GetDescriptor() != remoteDescriptor) {
        AVCODEC_LOGE("Invalid descriptor");
        return AVCS_ERR_INVALID_OPERATION;
    }

    auto itFunc = sourceFuncs_.find(code);
    if (itFunc != sourceFuncs_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            int32_t ret = (this->*memberFunc)(data, reply);
            CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call memberFunc");
            return AVCS_ERR_OK;
        }
    }
    AVCODEC_LOGW("Failed to find corresponding function");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t SourceServiceStub::DestroyStub()
{
    sourceServer_ = nullptr;
    AVCodecServerManager::GetInstance().DestroyStubObject(AVCodecServerManager::MUXER, AsObject());
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::Init(const std::string &uri)
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, nullptr, "source server is nullptr");
    return sourceServer_->Init(uri);
}

int32_t SourceServiceStub::GetTrackCount()
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, nullptr, "source server is nullptr");
    return sourceServer_->GetTrackCount();
}

int32_t SourceServiceStub::Destroy()
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, nullptr, "source server is nullptr");
    return sourceServer_->Destroy();
}

int32_t SourceServiceStub::SetParameter(const Format &param, uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, nullptr, "source server is nullptr");
    return sourceServer_->SetParameter(param, trackId);
}

int32_t SourceServiceStub::GetTrackFormat(Format &format, uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, nullptr, "source server is nullptr");
    return sourceServer_->GetTrackFormat(format, trackId);
}

uint64_t SourceServiceStub::GetSourceAttr()
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, nullptr, "source server is nullptr");
    return sourceServer_->GetSourceAttr();
}

int32_t SourceServiceStub::Init(MessageParcel &data, MessageParcel &reply)
{
    std::string uri = data.ReadString();
    CHECK_AND_RETURN_RET(reply.WriteInt32(Init(uri)), AVCS_ERR_UNKNOWN);
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::GetTrackCount(MessageParcel &data, MessageParcel &reply)
{
    CHECK_AND_RETURN_RET(reply.WriteInt32(GetTrackCount()), AVCS_ERR_UNKNOWN);
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::Destroy(MessageParcel &data, MessageParcel &reply)
{
    CHECK_AND_RETURN_RET(reply.WriteInt32(Destroy()), AVCS_ERR_UNKNOWN);
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::SetParameter(MessageParcel &data, MessageParcel &reply)
{
    Format param;
    (void)AVCodecParcel::Unmarshalling(data, param);
    uint32_t trackId = data.ReadUint32();
    CHECK_AND_RETURN_RET(reply.WriteInt32(SetParameter(param, trackId)), AVCS_ERR_UNKNOWN);
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::GetTrackFormat(MessageParcel &data, MessageParcel &reply)
{
    uint32_t trackId = data.ReadUint32();
    Format format;
    CHECK_AND_RETURN_RET(reply.WriteInt32(GetTrackFormat(format, trackId)), AVCS_ERR_UNKNOWN);
    AVCodecParcel::Marshalling(reply, format);
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::GetSourceAttr(MessageParcel &data, MessageParcel &reply)
{
    CHECK_AND_RETURN_RET(reply.WriteUint64(GetSourceAttr()), AVCS_ERR_UNKNOWN);
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::DestroyStub(MessageParcel &data, MessageParcel &reply)
{
    CHECK_AND_RETURN_RET(reply.WriteInt32(DestroyStub()), AVCS_ERR_UNKNOWN);
    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS