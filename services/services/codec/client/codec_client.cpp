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

#include "codec_client.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecClient"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<CodecClient> CodecClient::Create(const sptr<IStandardCodecService> &ipcProxy)
{
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, nullptr, "ipcProxy is nullptr..");

    std::shared_ptr<CodecClient> codec = std::make_shared<CodecClient>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "failed to new CodecClient..");

    int32_t ret = codec->CreateListenerObject();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "failed to create listener object..");

    return codec;
}

CodecClient::CodecClient(const sptr<IStandardCodecService> &ipcProxy)
    : codecProxy_(ipcProxy)
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecClient::~CodecClient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (codecProxy_ != nullptr) {
        (void)codecProxy_->DestroyStub();
    }
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void CodecClient::AVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    codecProxy_ = nullptr;
    listenerStub_ = nullptr;
    if (callback_ != nullptr) {
        callback_->OnError(AVCODEC_ERROR_INTERNAL, MSERR_SERVICE_DIED);
    }
}

int32_t CodecClient::CreateListenerObject()
{
    std::lock_guard<std::mutex> lock(mutex_);
    listenerStub_ = new(std::nothrow) CodecListenerStub();
    CHECK_AND_RETURN_RET_LOG(listenerStub_ != nullptr, MSERR_NO_MEMORY, "Failed to new CodecListenerStub object");
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    sptr<IRemoteObject> object = listenerStub_->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, MSERR_NO_MEMORY, "listener object is nullptr..");

    MEDIA_LOGD("SetListenerObject");
    return codecProxy_->SetListenerObject(object);
}

int32_t CodecClient::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("Init");
    return codecProxy_->Init(type, isMimeType, name);
}

int32_t CodecClient::Configure(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("Configure");
    return codecProxy_->Configure(format);
}

int32_t CodecClient::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("Start");
    return codecProxy_->Start();
}

int32_t CodecClient::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("Stop");
    return codecProxy_->Stop();
}

int32_t CodecClient::Flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("Flush");
    return codecProxy_->Flush();
}

int32_t CodecClient::NotifyEos()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("NotifyEos");
    return codecProxy_->NotifyEos();
}

int32_t CodecClient::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("Reset");
    return codecProxy_->Reset();
}

int32_t CodecClient::Release()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("Release");
    int32_t ret = codecProxy_->Release();
    (void)codecProxy_->DestroyStub();
    codecProxy_ = nullptr;
    return ret;
}

sptr<OHOS::Surface> CodecClient::CreateInputSurface()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, nullptr, "codec service does not exist.");

    MEDIA_LOGD("CreateInputSurface");
    return codecProxy_->CreateInputSurface();
}

int32_t CodecClient::SetOutputSurface(sptr<Surface> surface)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("SetOutputSurface");
    return codecProxy_->SetOutputSurface(surface);
}

std::shared_ptr<AVBufferElement> CodecClient::GetInputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, nullptr, "codec service does not exist.");

    MEDIA_LOGD("GetInputBuffer");
    return codecProxy_->GetInputBuffer(index);
}

int32_t CodecClient::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("QueueInputBuffer");
    return codecProxy_->QueueInputBuffer(index, info, flag);
}

std::shared_ptr<AVBufferElement> CodecClient::GetOutputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, nullptr, "codec service does not exist.");

    MEDIA_LOGD("GetOutputBuffer");
    return codecProxy_->GetOutputBuffer(index);
}

int32_t CodecClient::GetOutputFormat(Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("GetOutputFormat");
    return codecProxy_->GetOutputFormat(format);
}

int32_t CodecClient::ReleaseOutputBuffer(uint32_t index, bool render)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("ReleaseOutputBuffer");
    return codecProxy_->ReleaseOutputBuffer(index, render);
}

int32_t CodecClient::SetParameter(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("SetParameter");
    return codecProxy_->SetParameter(format);
}

int32_t CodecClient::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, MSERR_NO_MEMORY, "input param callback is nullptr.");
    CHECK_AND_RETURN_RET_LOG(listenerStub_ != nullptr, MSERR_NO_MEMORY, "listenerStub_ is nullptr.");

    callback_ = callback;
    MEDIA_LOGD("SetCallback");
    listenerStub_->SetCallback(callback);
    return MSERR_OK;
}

int32_t CodecClient::SetInputSurface(sptr<PersistentSurface> surface)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("SetInputSurface");
    return codecProxy_->SetInputSurface(surface);
}

int32_t CodecClient::DequeueInputBuffer(uint32_t *index, int64_t timetUs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");

    MEDIA_LOGD("DequeueInputBuffer");
    return codecProxy_->DequeueInputBuffer(index, timetUs);
}

int32_t CodecClient::DequeueOutputBuffer(uint32_t *index, int64_t timetUs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecProxy_ != nullptr, MSERR_NO_MEMORY, "codec service does not exist.");
    
    MEDIA_LOGD("DequeueOutputBuffer");
    return codecProxy_->DequeueInputBuffer(index, timetUs);
}

} // namespace Media
} // namespace OHOS
