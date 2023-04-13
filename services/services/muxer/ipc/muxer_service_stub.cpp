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

#include "muxer_service_stub.h"
#include "avcodec_server_manager.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avsharedmemory_ipc.h"
#include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MuxerServiceStub"};
}

namespace OHOS {
namespace MediaAVCodec {
sptr<MuxerServiceStub> MuxerServiceStub::Create()
{
    sptr<MuxerServiceStub> muxerStub = new(std::nothrow) MuxerServiceStub();
    CHECK_AND_RETURN_RET_LOG(muxerStub != nullptr, nullptr, "Failed to create muxer service stub");

    int32_t ret = muxerStub->InitStub();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init MuxerServiceStub");
    return muxerStub;
}

MuxerServiceStub::MuxerServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

MuxerServiceStub::~MuxerServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t MuxerServiceStub::InitStub()
{
    muxerServer_ = MuxerServer::Create();
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Failed to create muxer server");

    muxerFuncs_[INIT] = &MuxerServiceStub::Init;
    muxerFuncs_[SET_LOCATION] = &MuxerServiceStub::SetLocation;
    muxerFuncs_[SET_ROTATION] = &MuxerServiceStub::SetRotation;
    muxerFuncs_[SET_PARAMETER] = &MuxerServiceStub::SetParameter;
    muxerFuncs_[ADD_TRACK] = &MuxerServiceStub::AddTrack;
    muxerFuncs_[START] = &MuxerServiceStub::Start;
    muxerFuncs_[WRITE_SAMPLE_BUFFER] = &MuxerServiceStub::WriteSampleBuffer;
    muxerFuncs_[STOP] = &MuxerServiceStub::Stop;
    muxerFuncs_[DESTROY_STUB] = &MuxerServiceStub::DestroyStub;
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::DestroyStub()
{
    muxerServer_ = nullptr;
    AVCodecServerManager::GetInstance().DestroyStubObject(AVCodecServerManager::MUXER, AsObject());
    return AVCS_ERR_OK;
}

int MuxerServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    AVCODEC_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (MuxerServiceStub::GetDescriptor() != remoteDescriptor) {
        AVCODEC_LOGE("Invalid descriptor");
        return AVCS_ERR_INVALID_OPERATION;
    }

    auto itFunc = muxerFuncs_.find(code);
    if (itFunc != muxerFuncs_.end()) {
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

int32_t MuxerServiceStub::Init()
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "muxer service is nullptr");
    return muxerServer_->Init();
}


int32_t MuxerServiceStub::SetLocation(float latitude, float longitude)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "muxer service is nullptr");
    return muxerServer_->SetLocation(latitude, longitude);
}

int32_t MuxerServiceStub::SetRotation(int32_t rotation)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "muxer service is nullptr");
    return muxerServer_->SetRotation(rotation);
}

int32_t MuxerServiceStub::SetParameter(const Format &generalFormat)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "muxer service is nullptr");
    return muxerServer_->SetParameter(generalFormat);
}

int32_t MuxerServiceStub::AddTrack(const Format &trackFormat)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "muxer service is nullptr");
    return muxerServer_->AddTrack(trackFormat);
}

int32_t MuxerServiceStub::Start()
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "muxer service is nullptr");
    return muxerServer_->Start();
}

int32_t MuxerServiceStub::WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "muxer service is nullptr");
    return muxerServer_->WriteSampleBuffer(trackIndex, sampleBuffer, info);
}

int32_t MuxerServiceStub::Stop()
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "muxer service is nullptr");
    return muxerServer_->Stop();
}

int32_t MuxerServiceStub::DestroyStub(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    // TODO: 补充LOG说明
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(DestroyStub()), AVCS_ERR_UNKNOWN, "");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::SetLocation(MessageParcel &data, MessageParcel &reply)
{
    float latitude = data.ReadFloat();
    float longitude = data.ReadFloat();

    // TODO: 补充LOG说明
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(SetLocation(latitude, longitude)), AVCS_ERR_UNKNOWN, "");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::SetRotation(MessageParcel &data, MessageParcel &reply)
{
    int32_t rotation = data.ReadInt32();

    // TODO: 补充LOG说明
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(SetRotation(rotation)), AVCS_ERR_UNKNOWN, "");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::SetParameter(MessageParcel &data, MessageParcel &reply)
{
    Format format;
    AVCodecParcel::Unmarshalling(data, format);

    // TODO: 补充LOG说明
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(SetParameter(format)), AVCS_ERR_UNKNOWN, "");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::AddTrack(MessageParcel &data, MessageParcel &reply)
{
    Format generalFormat;
    (void)AVCodecParcel::Unmarshalling(data, generalFormat);

    // TODO: 补充LOG说明
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(AddTrack(generalFormat)), AVCS_ERR_UNKNOWN, "");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::Start(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    // TODO: 补充LOG说明
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(Start()), AVCS_ERR_UNKNOWN, "");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::WriteSampleBuffer(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    (void)reply;

    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::Stop(MessageParcel &data, MessageParcel &reply)
{
    // TODO: 补充LOG说明
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(Stop()), AVCS_ERR_UNKNOWN, "");
    return AVCS_ERR_OK;
}
}  // namespace MediaAVCodec
}  // namespace OHOS