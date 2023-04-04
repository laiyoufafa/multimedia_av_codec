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

#include "avmuxer_service_stub.h"
#include "avcodec_server_manager.h"
#include "media_errors.h"
#include "media_log.h"
#include "avsharedmemory_ipc.h"
#include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MuxerServiceStub"};
}

namespace OHOS {
namespace AVCodec {
sptr<MuxerServiceStub> MuxerServiceStub::Create()
{
    sptr<MuxerServiceStub> avmuxerStub = new(std::nothrow) MuxerServiceStub();
    CHECK_AND_RETURN_RET_LOG(avmuxerStub != nullptr, nullptr, "Failed to create avmuxer service stub");

    int32_t ret = avmuxerStub->Init();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "Failed to init MuxerServiceStub");
    return avmuxerStub;
}

MuxerServiceStub::MuxerServiceStub()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

MuxerServiceStub::~MuxerServiceStub()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t MuxerServiceStub::Init()
{
    muxerServer_ = MuxerServer::Create();
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, MSERR_NO_MEMORY, "Failed to create muxer server");

    muxerFuncs_[SET_LOCATION] = &MuxerServiceStub::SetLocation;
    muxerFuncs_[SET_ROTATION] = &MuxerServiceStub::SetRotation;
    muxerFuncs_[SET_PARAMETER] = &MuxerServiceStub::SetParameter;
    muxerFuncs_[ADD_TRACK] = &MuxerServiceStub::AddTrack;
    muxerFuncs_[START] = &MuxerServiceStub::Start;
    muxerFuncs_[WRITE_SAMPLE_BUFFER] = &MuxerServiceStub::WriteSampleBuffer;
    muxerFuncs_[STOP] = &MuxerServiceStub::Stop;
    muxerFuncs_[DESTROY] = &MuxerServiceStub::DestroyStub;
    return MSERR_OK;
}

int32_t MuxerServiceStub::DestroyStub()
{
    muxerServer_ = nullptr;
    AVCodecServerManager::GetInstance().DestroyStubObject(AVCodecServerManager::MUXER, AsObject());
    return MSERR_OK;
}

int MuxerServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    MEDIA_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (MuxerServiceStub::GetDescriptor() != remoteDescriptor) {
        MEDIA_LOGE("Invalid descriptor");
        return MSERR_INVALID_OPERATION;
    }

    auto itFunc = muxerFuncs_.find(code);
    if (itFunc != muxerFuncs_.end()) {
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

int32_t MuxerServiceStub::SetLocation(float latitude, float longitude)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, MSERR_NO_MEMORY, "Muxer SerMuxeres not exist");
    return muxerServer_->SetLocation(latitude, longitude);
}

int32_t MuxerServiceStub::SetRotation(int32_t rotation)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, MSERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->SetRotation(rotation);
}

int32_t MuxerServiceStub::SetParameter(const Format &generalFormat)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, MSERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->SetParameter(generalFormat);
}

int32_t MuxerServiceStub::AddTrack(const Format &trackFormat)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, MSERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->AddTrack(trackFormat);
}

int32_t MuxerServiceStub::Start()
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, MSERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->Start();
}

int32_t MuxerServiceStub::WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, MSERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->WriteSampleBuffer(trackIndex, sampleBuffer, info);
}

int32_t MuxerServiceStub::Stop()
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, MSERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->Stop();
}

int32_t MuxerServiceStub::DestroyStub(MessageParcel &data, MessageParcel &reply)
{
    CHECK_AND_RETURN_RET(reply.WriteInt32(DestroyStub()), MSERR_UNKNOWN);
    return MSERR_OK;
}

int32_t MuxerServiceStub::SetLocation(MessageParcel &data, MessageParcel &reply)
{
    float latitude = data.ReadFloat();
    float longitude = data.ReadFloat();
    CHECK_AND_RETURN_RET(reply.WriteInt32(SetLocation(latitude, longitude)), MSERR_UNKNOWN);
    return MSERR_OK;
}

int32_t MuxerServiceStub::SetRotation(MessageParcel &data, MessageParcel &reply)
{
    int32_t rotation = data.ReadInt32();
    CHECK_AND_RETURN_RET(reply.WriteInt32(SetRotation(rotation)), MSERR_UNKNOWN);
    return MSERR_OK;
}

int32_t MuxerServiceStub::SetParameter(MessageParcel &data, MessageParcel &reply)
{
    Format format;
    AVCodecParcel::Unmarshalling(data, format);
    CHECK_AND_RETURN_RET(reply.WriteInt32(SetParameter(format)), MSERR_UNKNOWN);
    return MSERR_OK;
}

int32_t MuxerServiceStub::AddTrack(MessageParcel &data, MessageParcel &reply)
{
    Format generalFormat;
    (void)AVCodecParcel::Unmarshalling(data, generalFormat);
    CHECK_AND_RETURN_RET(reply.WriteInt32(AddTrack(generalFormat)), MSERR_UNKNOWN);
    return MSERR_OK;
}

int32_t MuxerServiceStub::Start(MessageParcel &data, MessageParcel &reply)
{
    CHECK_AND_RETURN_RET(reply.WriteInt32(Start()), MSERR_UNKNOWN);
    return MSERR_OK;
}

int32_t MuxerServiceStub::WriteSampleBuffer(MessageParcel &data, MessageParcel &reply)
{


    return MSERR_OK;
}

int32_t MuxerServiceStub::Stop(MessageParcel &data, MessageParcel &reply)
{
    CHECK_AND_RETURN_RET(reply.WriteInt32(Stop()), MSERR_UNKNOWN);
    return MSERR_OK;
}
}  // namespace AVCodec
}  // namespace OHOS