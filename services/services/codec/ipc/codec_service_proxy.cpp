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

#include "codec_service_proxy.h"
#include "codec_listener_stub.h"
#include "avsharedmemory_ipc.h"
#include "media_errors.h"
#include "media_log.h"
#include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecServiceProxy"};
}

namespace OHOS {
namespace AVCodec {
class CodecServiceProxy::CodecBufferCache : public NoCopyable {
public:
    CodecBufferCache() = default;
    ~CodecBufferCache() = default;

    int32_t ReadFromParcel(uint32_t index, MessageParcel &parcel, std::shared_ptr<AVBufferElement> &memory)
    {
        auto iter = caches_.find(index);
        CacheFlag flag = static_cast<CacheFlag>(parcel.ReadUint8());
        if (flag == CacheFlag::HIT_CACHE) {
            if (iter == caches_.end()) {
                MEDIA_LOGE("mark hit cache, but can find the index's cache, index: %{public}u", index);
                return MSERR_INVALID_VAL;
            }
            memory = iter->second;
            return MSERR_OK;
        }

        if (flag == CacheFlag::UPDATE_CACHE) {
            memory = std::make_shared<AVBufferElement>();
            memory->buffer = ReadAVSharedMemoryFromParcel(parcel);
            memory->metaData = ReadAVSharedMemoryFromParcel(parcel);
            CHECK_AND_RETURN_RET(memory != nullptr, MSERR_INVALID_VAL);
            if (iter == caches_.end()) {
                MEDIA_LOGI("add cache, index: %{public}u", index);
                caches_.emplace(index, memory);
            } else {
                iter->second = memory;
                MEDIA_LOGI("update cache, index: %{public}u", index);
            }
            return MSERR_OK;
        }

        // invalidate cache flag
        if (iter != caches_.end()) {
            iter->second = nullptr;
            caches_.erase(iter);
        }
        memory = nullptr;
        MEDIA_LOGE("invalidate cache for index: %{public}u, flag: %{public}hhu", index, flag);
        return MSERR_INVALID_VAL;
    }

private:
    enum CacheFlag : uint8_t {
        HIT_CACHE = 1,
        UPDATE_CACHE,
        INVALIDATE_CACHE,
    };

    std::unordered_map<uint32_t, std::shared_ptr<AVBufferElement>> caches_;
};

CodecServiceProxy::CodecServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardCodecService>(impl)
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecServiceProxy::~CodecServiceProxy()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t CodecServiceProxy::SetListenerObject(const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    (void)data.WriteRemoteObject(object);
    int32_t ret = Remote()->SendRequest(SET_LISTENER_OBJ, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "SetListenerObject failed, error: %{public}d", ret);

    return reply.ReadInt32();
}


int32_t CodecServiceProxy::DestroyStub()
{
    inputBufferCache_ = nullptr;
    outputBufferCache_ = nullptr;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(DESTROY, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "DestroyStub failed, error: %{public}d", ret);

    return reply.ReadInt32();
}


int32_t CodecServiceProxy::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteInt32(static_cast<int32_t>(type));
    data.WriteBool(isMimeType);
    data.WriteString(name);
    int32_t ret = Remote()->SendRequest(INIT_PARAMETER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "Init failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t CodecServiceProxy::Configure(const Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    (void)AVCodecParcel::Marshalling(data, format);
    int32_t ret = Remote()->SendRequest(CONFIGURE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "Configure failed, error: %{public}d", ret);

    return reply.ReadInt32();
}


int32_t CodecServiceProxy::Start()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(START, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "Start failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t CodecServiceProxy::Stop()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(STOP, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "Stop failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t CodecServiceProxy::Flush()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(FLUSH, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "Flush failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t CodecServiceProxy::NotifyEos()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(NOTIFY_EOS, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "NotifyEos failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t CodecServiceProxy::Reset()
{
    inputBufferCache_ = nullptr;
    outputBufferCache_ = nullptr;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(RESET, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "Reset failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t CodecServiceProxy::Release()
{
    inputBufferCache_ = nullptr;
    outputBufferCache_ = nullptr;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(RELEASE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "Release failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

sptr<OHOS::Surface> CodecServiceProxy::CreateInputSurface()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, nullptr, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(CREATE_INPUT_SURFACE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr,
        "CreateInputSurface failed, error: %{public}d", ret);

    sptr<IRemoteObject> object = reply.ReadRemoteObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "failed to read surface object");

    sptr<IBufferProducer> producer = iface_cast<IBufferProducer>(object);
    CHECK_AND_RETURN_RET_LOG(producer != nullptr, nullptr, "failed to convert object to producer");

    return OHOS::Surface::CreateSurfaceAsProducer(producer);
}

int32_t CodecServiceProxy::SetOutputSurface(sptr<OHOS::Surface> surface)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(surface != nullptr, MSERR_NO_MEMORY, "surface is nullptr");
    sptr<IBufferProducer> producer = surface->GetProducer();
    CHECK_AND_RETURN_RET_LOG(producer != nullptr, MSERR_NO_MEMORY, "producer is nullptr");

    sptr<IRemoteObject> object = producer->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, MSERR_NO_MEMORY, "object is nullptr");

    const std::string surfaceFormat = "SURFACE_FORMAT";
    std::string format = surface->GetUserData(surfaceFormat);
    MEDIA_LOGI("surfaceFormat is %{public}s!", format.c_str());

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    (void)data.WriteRemoteObject(object);
    data.WriteString(format);
    int32_t ret = Remote()->SendRequest(SET_OUTPUT_SURFACE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "SetOutputSurface failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

std::shared_ptr<AVBufferElement> CodecServiceProxy::GetInputBuffer(uint32_t index)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, nullptr, "Failed to write descriptor!");

    data.WriteUint32(index);
    int32_t ret = Remote()->SendRequest(GET_INPUT_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr,
        "GetInputBuffer failed, error: %{public}d", ret);

    std::shared_ptr<AVBufferElement> memory = nullptr;
    if (inputBufferCache_ != nullptr) {
        ret = inputBufferCache_->ReadFromParcel(index, reply, memory);
        CHECK_AND_RETURN_RET(ret == MSERR_OK, nullptr);
    }
    CHECK_AND_RETURN_RET_LOG(memory != nullptr, nullptr, "Failed to GetInputBuffer");
    return memory;
}

int32_t CodecServiceProxy::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteUint32(index);
    data.WriteInt64(info.presentationTimeUs);
    data.WriteInt32(info.size);
    data.WriteInt32(info.offset);
    data.WriteInt32(static_cast<int32_t>(flag));
    int32_t ret = Remote()->SendRequest(QUEUE_INPUT_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "QueueInputBuffer failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

std::shared_ptr<AVBufferElement> CodecServiceProxy::GetOutputBuffer(uint32_t index)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, nullptr, "Failed to write descriptor!");

    data.WriteUint32(index);
    int32_t ret = Remote()->SendRequest(GET_OUTPUT_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr,
        "GetOutputBuffer failed, error: %{public}d", ret);

    std::shared_ptr<AVBufferElement> memory = nullptr;
    if (outputBufferCache_ != nullptr) {
        ret = outputBufferCache_->ReadFromParcel(index, reply, memory);
        CHECK_AND_RETURN_RET(ret == MSERR_OK, nullptr);
    }
    CHECK_AND_RETURN_RET_LOG(memory != nullptr, nullptr, "Failed to GetOutputBuffer");
    return memory;
}

int32_t CodecServiceProxy::GetOutputFormat(Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t ret = Remote()->SendRequest(GET_OUTPUT_FORMAT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "GetOutputFormat failed, error: %{public}d", ret);

    (void)AVCodecParcel::Unmarshalling(reply, format);
    return MSERR_OK;
}

int32_t CodecServiceProxy::ReleaseOutputBuffer(uint32_t index, bool render)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteUint32(index);
    data.WriteBool(render);
    int32_t ret = Remote()->SendRequest(RELEASE_OUTPUT_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "ReleaseOutputBuffer failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t CodecServiceProxy::SetParameter(const Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    (void)AVCodecParcel::Marshalling(data, format);
    int32_t ret = Remote()->SendRequest(SET_PARAMETER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "SetParameter failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t CodecServiceProxy::SetInputSurface(sptr<PersistentSurface> surface)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(surface != nullptr, MSERR_NO_MEMORY, "surface is nullptr");
    sptr<IBufferProducer> producer = surface->GetProducer();
    // CHECK_AND_RETURN_RET_LOG(producer != nullptr, MSERR_NO_MEMORY, "producer is nullptr");

    // sptr<IRemoteObject> object = producer->AsObject();
    // CHECK_AND_RETURN_RET_LOG(object != nullptr, MSERR_NO_MEMORY, "object is nullptr");

    // const std::string surfaceFormat = "SURFACE_FORMAT";
    // std::string format = surface->GetUserData(surfaceFormat);
    // MEDIA_LOGI("surfaceFormat is %{public}s!", format.c_str());

    // bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    // CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    // (void)data.WriteRemoteObject(object);
    // data.WriteString(format);
    // int32_t ret = Remote()->SendRequest(SET_OUTPUT_SURFACE, data, reply, option);
    // CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
    //     "SetOutputSurface failed, error: %{public}d", ret);

    return reply.ReadInt32();
}
int32_t CodecServiceProxy::DequeueInputBuffer(uint32_t *index, int64_t timetUs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteInt64(timetUs);
    int32_t ret = Remote()->SendRequest(RELEASE_OUTPUT_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "DequeueInputBuffer failed, error: %{public}d", ret);
    *index = reply.ReadUint32();
    return reply.ReadInt32();
}

int32_t CodecServiceProxy::DequeueOutputBuffer(uint32_t *index, int64_t timetUs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(CodecServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, MSERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteInt64(timetUs);
    int32_t ret = Remote()->SendRequest(RELEASE_OUTPUT_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, MSERR_INVALID_OPERATION,
        "DequeueOutputBuffer failed, error: %{public}d", ret);
    *index = reply.ReadUint32();
    return reply.ReadInt32();
}
} // namespace AVCodec
} // namespace OHOS
