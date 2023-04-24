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
#include "avdemuxer_stub.h"
#include <unistd.h>
#include "avcodec_server_manager.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avsharedmemory_ipc.h"
// #include "avcodec_parcel.h"
#include "avcodec_xcollie.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVDemuxerStub"};
}

namespace OHOS {
namespace Media {
sptr<AVDemuxerStub> AVDemuxerStub::Create()
{
    sptr<AVDemuxerStub> demuxerStub = new(std::nothrow) AVDemuxerStub();
    CHECK_AND_RETURN_RET_LOG(demuxerStub != nullptr, nullptr, "Failed to create demuxer service stub");

    int32_t ret = demuxerStub->InitStub();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init AVDemuxerStub");
    return demuxerStub;
}

AVDemuxerStub::AVDemuxerStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVDemuxerStub::~AVDemuxerStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVDemuxerStub::InitStub()
{
    demuxerServer_ = AVDemuxerServer::Create();
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Failed to create muxer server");

    demuxerFuncs_[INIT] = &AVDemuxerStub::Init;
    demuxerFuncs_[ADD_SOURCE_TRACK_BY_ID] = &AVDemuxerStub::AddSourceTrackByID;
    demuxerFuncs_[REMOVE_SOURCE_TRACK_BY_ID] = &AVDemuxerStub::RemoveSourceTrackByID;
    demuxerFuncs_[COPY_CURRENT_SAMPLE_TO_BUF] = &AVDemuxerStub::CopyCurrentSampleToBuf;
    demuxerFuncs_[SEEK_TO_TIME_STAMP] = &AVDemuxerStub::SeekToTimeStamp;

    demuxerFuncs_[DESTROY_STUB] = &AVDemuxerStub::DestroyStub;

    return AVCS_ERR_OK;
}

int AVDemuxerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    AVCODEC_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (AVDemuxerStub::GetDescriptor() != remoteDescriptor) {
        AVCODEC_LOGE("Invalid descriptor");
        return AVCS_ERR_INVALID_OPERATION;
    }

    auto itFunc = demuxerFuncs_.find(code);
    if (itFunc != demuxerFuncs_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            int32_t ret = -1;
            COLLIE_LISTEN(ret = (this->*memberFunc)(data, reply),
                "AVDemuxerStub::OnRemoteRequest");
            CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call memberFunc");
            return AVCS_ERR_OK;
        }
    }
    AVCODEC_LOGW("Failed to find corresponding function");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AVDemuxerStub::DestroyStub(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(DestroyStub()), AVCS_ERR_UNKNOWN, "");
    return AVCS_ERR_OK;
}

int32_t AVDemuxerStub::DestroyStub()
{
    demuxerServer_ = nullptr;
    AVCodecServerManager::GetInstance().DestroyStubObject(AVCodecServerManager::DEMUXER, AsObject());
    return AVCS_ERR_OK;
}

int32_t AVDemuxerStub::Init(MessageParcel &data, MessageParcel &reply)
{
    uint64_t attr = data.ReadUint64();
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(Init(attr)), AVCS_ERR_UNKNOWN, "");   
    return AVCS_ERR_OK;
}

int32_t AVDemuxerStub::AddSourceTrackByID(MessageParcel &data, MessageParcel &reply)
{
    uint32_t index = data.ReadUint32();

    // TODO: 添加LOG描述
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(AddSourceTrackByID(index)), AVCS_ERR_UNKNOWN, "");   
    return AVCS_ERR_OK;
}

int32_t AVDemuxerStub::RemoveSourceTrackByID(MessageParcel &data, MessageParcel &reply)
{
    uint32_t index = data.ReadUint32();

    // TODO: 添加LOG描述
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(RemoveSourceTrackByID(index)), AVCS_ERR_UNKNOWN, "");   
    return AVCS_ERR_OK;
}

int32_t AVDemuxerStub::CopyCurrentSampleToBuf(MessageParcel &data, MessageParcel &reply)
{
    AVCodecBufferInfo info;
    info.presentationTimeUs = data.ReadInt64();
    info.size = data.ReadInt32();
    info.offset = data.ReadInt32();
    info.flag = data.ReadInt32();

    AVBufferElement buffer;


    MessageParcel parcel;   // TODO: 未定义的变量

    buffer.buffer = ReadAVSharedMemoryFromParcel(parcel);
    buffer.metaData = ReadAVSharedMemoryFromParcel(parcel);

    // TODO: 添加LOG描述
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(CopyCurrentSampleToBuf(&buffer, &info)), AVCS_ERR_UNKNOWN, "");   
    return AVCS_ERR_OK;
}

int32_t AVDemuxerStub::SeekToTimeStamp(MessageParcel &data, MessageParcel &reply)
{
    int64_t mSeconds = data.ReadInt64();
    int32_t mode = data.ReadInt32();
    AVSeekMode seekMode = static_cast<AVSeekMode>(mode);

    // TODO: 添加LOG描述
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(SeekToTimeStamp(mSeconds, seekMode)), AVCS_ERR_UNKNOWN, "");   
    return AVCS_ERR_OK;
}

int32_t AVDemuxerStub::DumpInfo(int32_t fd)
{
    std::string dumpInfo;
    dumpInfo += "# AVDemuxerStub \n";
    GetDumpInfo(dumpInfo);

    CHECK_AND_RETURN_RET_LOG(fd != -1, AVCS_ERR_INVALID_VAL, "Attempt to write to a invalid fd: %{public}d", fd);
    write(fd, dumpInfo.c_str(), dumpInfo.size());

    return AVCS_ERR_OK;
}

int32_t AVDemuxerStub::Init(uint64_t attr)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->Init(attr);
}

int32_t AVDemuxerStub::AddSourceTrackByID(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->AddSourceTrackByID(index);
}

int32_t AVDemuxerStub::RemoveSourceTrackByID(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->RemoveSourceTrackByID(index);
}

int32_t AVDemuxerStub::CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->CopyCurrentSampleToBuf(buffer, bufferInfo);
}

int32_t AVDemuxerStub::SeekToTimeStamp(int64_t mSeconds, const AVSeekMode mode)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->SeekToTimeStamp(mSeconds, mode);
}

int32_t AVDemuxerStub::GetDumpInfo(std::string& dumpInfo)
{
    dumpInfo += "## pid: " + std::to_string(getpid());
    dumpInfo += "## uid: " + std::to_string(getuid());
    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS  