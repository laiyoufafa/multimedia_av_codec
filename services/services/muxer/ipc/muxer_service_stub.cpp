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
namespace Media {
sptr<MuxerServiceStub> MuxerServiceStub::Create()
{
    sptr<MuxerServiceStub> muxerStub = new(std::nothrow) MuxerServiceStub();
    CHECK_AND_RETURN_RET_LOG(muxerStub != nullptr, nullptr, "Create muxer service stub failed");

    int32_t ret = muxerStub->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init MuxerServiceStub failed to ");
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

int32_t MuxerServiceStub::Init()
{
    muxerServer_ = MuxerServer::Create();
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Create muxer server failed");

    muxerFuncs_[INIT_PARAMETER] = &MuxerServiceStub::InitParameter;
    muxerFuncs_[SET_LOCATION] = &MuxerServiceStub::SetLocation;
    muxerFuncs_[SET_ROTATION] = &MuxerServiceStub::SetRotation;
    muxerFuncs_[ADD_TRACK] = &MuxerServiceStub::AddTrack;
    muxerFuncs_[START] = &MuxerServiceStub::Start;
    muxerFuncs_[WRITE_SAMPLE_BUFFER] = &MuxerServiceStub::WriteSampleBuffer;
    muxerFuncs_[STOP] = &MuxerServiceStub::Stop;
    muxerFuncs_[RELEASE] = &MuxerServiceStub::Release;
    muxerFuncs_[DESTROY] = &MuxerServiceStub::DestroyStub;
    return AVCS_ERR_OK;
}

int MuxerServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
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

int32_t MuxerServiceStub::InitParameter(int32_t fd, OutputFormat format)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->InitParameter(fd, format);
}

int32_t MuxerServiceStub::SetLocation(float latitude, float longitude)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->SetLocation(latitude, longitude);
}

int32_t MuxerServiceStub::SetRotation(int32_t rotation)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->SetRotation(rotation);
}

int32_t MuxerServiceStub::AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->AddTrack(trackIndex, trackDesc);
}

int32_t MuxerServiceStub::Start()
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->Start();
}

int32_t MuxerServiceStub::WriteSampleBuffer(std::shared_ptr<AVSharedMemory> sampleBuffer, const TrackSampleInfo &info)
{
    CHECK_AND_RETURN_RET_LOG(sampleBuffer != nullptr, AVCS_ERR_INVALID_VAL, "sampleData is nullptr");
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->WriteSampleBuffer(sampleBuffer, info);
}

int32_t MuxerServiceStub::Stop()
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return muxerServer_->Stop();
}

void MuxerServiceStub::Release()
{
    CHECK_AND_RETURN_LOG(muxerServer_ != nullptr, "Muxer Service does not exist");
    muxerServer_->Release();
}

int32_t MuxerServiceStub::DestroyStub()
{
    muxerServer_ = nullptr;
    AVCodecServerManager::GetInstance().DestroyStubObject(AVCodecServerManager::MUXER, AsObject());
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::DumpInfo(int32_t fd)
{
    CHECK_AND_RETURN_RET_LOG(muxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Muxer Service does not exist");
    return std::static_pointer_cast<MuxerServer>(muxerServer_)->DumpInfo(fd);
}

int32_t MuxerServiceStub::InitParameter(MessageParcel &data, MessageParcel &reply)
{
    int32_t fd = data.ReadFileDescriptor();
    OutputFormat format = static_cast<OutputFormat>(data.ReadInt32());
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(InitParameter(fd, format)), AVCS_ERR_UNKNOWN,
        "Reply InitParameter failed!");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::SetLocation(MessageParcel &data, MessageParcel &reply)
{
    float latitude = data.ReadFloat();
    float longitude = data.ReadFloat();
    int32_t ret = SetLocation(latitude, longitude);
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(ret), AVCS_ERR_UNKNOWN, "Reply SetLocation failed!");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::SetRotation(MessageParcel &data, MessageParcel &reply)
{
    int32_t rotation = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(SetRotation(rotation)), AVCS_ERR_UNKNOWN, "WriteInt32 failed!");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::AddTrack(MessageParcel &data, MessageParcel &reply)
{
    MediaDescription trackDesc;
    (void)AVCodecParcel::Unmarshalling(data, trackDesc);
    int32_t trackIndex = -1;
    int32_t ret = AddTrack(trackIndex, trackDesc);
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(trackIndex), AVCS_ERR_UNKNOWN, "Reply AddTrack failed!");
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(ret), AVCS_ERR_UNKNOWN, "Reply AddTrack failed!");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::Start(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(Start()), AVCS_ERR_UNKNOWN, "Reply Start failed!");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::WriteSampleBuffer(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<AVSharedMemory> sampleBuffer = ReadAVSharedMemoryFromParcel(data);
    CHECK_AND_RETURN_RET_LOG(sampleBuffer != nullptr, AVCS_ERR_UNKNOWN, "Read sampleBuffer from parcel failed!");
    TrackSampleInfo info;
    info.trackIndex = data.ReadUint32();
    info.timeUs = data.ReadInt64();
    info.size = data.ReadUint32();
    info.flags = data.ReadUint32();
    int32_t ret = WriteSampleBuffer(sampleBuffer, info);
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(ret), AVCS_ERR_UNKNOWN, "Reply WriteSampleBuffer failed!");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::Stop(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(Stop()), AVCS_ERR_UNKNOWN, "Reply Stop failed!");
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::Release(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    (void)reply;
    Release();
    return AVCS_ERR_OK;
}

int32_t MuxerServiceStub::DestroyStub(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(DestroyStub()), AVCS_ERR_UNKNOWN, "Reply DestroyStub failed!");
    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS