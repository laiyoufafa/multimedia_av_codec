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

#include "codec_server.h"
#include <map>
#include "media_log.h"
#include "media_errors.h"
// #include "engine_factory_repo.h"
#include "media_dfx.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecServer"};
    const std::map<OHOS::Media::AVCodecServer::AVCodecStatus, std::string> AVCODEC_STATE_MAP = {
        {OHOS::Media::AVCodecServer::AVCODEC_UNINITIALIZED, "uninitialized"},
        {OHOS::Media::AVCodecServer::AVCODEC_INITIALIZED, "initialized"},
        {OHOS::Media::AVCodecServer::AVCODEC_CONFIGURED, "configured"},
        {OHOS::Media::AVCodecServer::AVCODEC_RUNNING, "running"},
        {OHOS::Media::AVCodecServer::AVCODEC_FLUSHED, "flushed"},
        {OHOS::Media::AVCodecServer::AVCODEC_END_OF_STREAM, "end of stream"},
        {OHOS::Media::AVCodecServer::AVCODEC_ERROR, "error"},
    };
}

namespace OHOS {
namespace AVCodec {
std::shared_ptr<IAVCodecService> AVCodecServer::Create()
{
    std::shared_ptr<AVCodecServer> server = std::make_shared<AVCodecServer>();
    // int32_t ret = server->Init();
    if (ret != MSERR_OK) {
        MEDIA_LOGE("failed to init AVCodecServer");
        return nullptr;
    }
    return server;
}

AVCodecServer::AVCodecServer()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecServer::~AVCodecServer()
{
    std::unique_ptr<std::thread> thread = std::make_unique<std::thread>(&AVCodecServer::ExitProcessor, this);
    if (thread != nullptr && thread->joinable()) {
        thread->join();
    }
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void AVCodecServer::ExitProcessor()
{
    codecEngine_ = nullptr;
}

// int32_t AVCodecServer::Init()
// {
//     // auto engineFactory = EngineFactoryRepo::Instance().GetEngineFactory(IEngineFactory::Scene::SCENE_AVCODEC);
//     // CHECK_AND_RETURN_RET_LOG(engineFactory != nullptr, MSERR_CREATE_AVCODEC_ENGINE_FAILED, "failed to get factory");
//     // codecEngine_ = engineFactory->CreateAVCodecEngine();
//     // CHECK_AND_RETURN_RET_LOG(codecEngine_ != nullptr, MSERR_CREATE_AVCODEC_ENGINE_FAILED,
//     //     "Failed to create codec engine");
//     // status_ = AVCODEC_INITIALIZED;
//     // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
//     return MSERR_OK;
// }

int32_t AVCodecServer::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("AVCodecServer::Init");
    if (isMimeType) {
        codecBase_ = AVCodecBase::Create(type == AVCODEC_TYPE_ENCODER,  name);
    } else {
        codecBase_ = AVCodecBase::Create(name);
    }
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");

    std::shared_ptr<AVCodecCallback> callback = std::make_shared<AVCodecBaseCallback>(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, MSERR_NO_MEMORY, "failed to new AVCodecBaseCallback");
    CHECK_AND_RETURN_RET_LOG(codecBase_->SetCallback(callback), MSERR_INVALID_OPERATION,
        "CodecBase SetCallback failed, error: %{public}d", ret);

    status_ = AVCODEC_INITIALIZED;
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    return MSERR_OK;
}


int32_t AVCodecServer::Configure(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("AVCodecServer::Configure");
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_INITIALIZED, MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    config_ = format;
    int32_t ret = codecBase_->Configure(format);
    status_ = (ret == MSERR_OK ? AVCODEC_CONFIGURED : AVCODEC_ERROR);
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    return ret;
}

int32_t AVCodecServer::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("AVCodecServer::Start");
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_FLUSHED || status_ == AVCODEC_CONFIGURED,
        MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codecBase_->Start();
    if (codecCb_) {
        status_ = (ret == MSERR_OK ? AVCODEC_RUNNING : AVCODEC_ERROR);
    } else {
        status_ = (ret == MSERR_OK ? AVCODEC_FLUSHED : AVCODEC_ERROR);
    }
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    return ret;
}

int32_t AVCodecServer::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("AVCodecServer::Stop");
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_RUNNING, MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codecBase_->Stop();
    status_ = (ret == MSERR_OK ? AVCODEC_CONFIGURED : AVCODEC_ERROR);
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    // ResetTrace();
    return ret;
}

int32_t AVCodecServer::Flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("AVCodecServer::Flush");
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_RUNNING || status_ == AVCODEC_END_OF_STREAM,
        MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codecBase_->Flush();
    status_ = (ret == MSERR_OK ? AVCODEC_FLUSHED : AVCODEC_ERROR);
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    // ResetTrace();
    return ret;
}

int32_t AVCodecServer::NotifyEos()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("AVCodecServer::NotifyEos");
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_RUNNING, MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codecBase_->NotifyEos();
    if (ret == MSERR_OK) {
        status_ = AVCODEC_END_OF_STREAM;
        // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
        // MEDIA_LOGI("EOS state");
    }
    return ret;
}

int32_t AVCodecServer::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("AVCodecServer::Reset");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codecBase_->Reset();
    status_ = (ret == MSERR_OK ? AVCODEC_INITIALIZED : AVCODEC_ERROR);
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    lastErrMsg_.clear();
    // ResetTrace();
    return ret;
}

int32_t AVCodecServer::Release()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("AVCodecServer::Release");
    std::unique_ptr<std::thread> thread = std::make_unique<std::thread>(&AVCodecServer::ExitProcessor, this);
    if (thread != nullptr && thread->joinable()) {
        thread->join();
    }
    // ResetTrace();
    return MSERR_OK;
}

sptr<Surface> AVCodecServer::CreateInputSurface()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_CONFIGURED, nullptr, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, nullptr, "codecBase is nullptr");
    sptr<Surface> surface = codecBase_->CreateInputSurface();
    firstFrameTraceId_ = FAKE_POINTER(surface.GetRefPtr());
    return surface;
}

int32_t AVCodecServer::SetOutputSurface(sptr<Surface> surface)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_CONFIGURED, MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    return codecBase_->SetOutputSurface(surface);
}

std::shared_ptr<AVBufferElement> AVCodecServer::GetInputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_RUNNING, nullptr, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, nullptr, "codecBase is nullptr");
    return codecBase_->GetInputBuffer(index);
}

int32_t AVCodecServer::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    MediaTrace trace("AVCodecServer::QueueInputBuffer");
    std::lock_guard<std::mutex> lock(mutex_);
    firstFrameTraceId_ = FAKE_POINTER(this);
    if (isFirstFrameIn_) {
        MediaTrace::TraceBegin("AVCodecServer::FirstFrame", firstFrameTraceId_);
        isFirstFrameIn_ = false;
    }
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_RUNNING, MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codecBase_->QueueInputBuffer(index, info, flag);
    if (flag & AVCODEC_BUFFER_FLAG_EOS) {
        if (ret == MSERR_OK) {
            status_ = AVCODEC_END_OF_STREAM;
            // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
            MEDIA_LOGI("EOS state");
        }
    }
    return ret;
}

std::shared_ptr<AVBufferElement> AVCodecServer::GetOutputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_RUNNING || status_ == AVCODEC_END_OF_STREAM,
        nullptr, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, nullptr, "codecBase is nullptr");
    return codecBase_->GetOutputBuffer(index);
}

int32_t AVCodecServer::GetOutputFormat(Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ != AVCODEC_UNINITIALIZED, MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    return codecBase_->GetOutputFormat(format);
}

int32_t AVCodecServer::ReleaseOutputBuffer(uint32_t index, bool render)
{
    MediaTrace trace("AVCodecServer::ReleaseOutputBuffer");
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_RUNNING || status_ == AVCODEC_END_OF_STREAM,
        MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    return codecBase_->ReleaseOutputBuffer(index, render);
}

int32_t AVCodecServer::SetParameter(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ != AVCODEC_INITIALIZED && status_ != AVCODEC_CONFIGURED,
        MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    return codecBase_->SetParameter(format);
}

int32_t AVCodecServer::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_INITIALIZED, MSERR_INVALID_OPERATION, "invalid state");
    {
        std::lock_guard<std::mutex> cbLock(cbMutex_);
        codecCb_ = callback;
    }
    // CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    // (void)codecBase_->SetCallback(callback);
    return MSERR_OK;
}

int32_t AVCodecServer::SetInputSurface(sptr<PersistentSurface> surface)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_CONFIGURED, MSERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, MSERR_NO_MEMORY, "codecBase is nullptr");
    return codecBase_->SetOutputSurface(surface);
}

int32_t AVCodecServer::DequeueInputBuffer(uint32_t *index, int64_t timetUs)
{

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_RUNNING, MSERR_INVALID_OPERATION, "invalid state");

    if (inQueue_.size() < 0) {
        return MSERR_INVALID_OPERATION;
    }
    *index = inQueue_.front();
    return MSERR_OK;
}

int32_t AVCodecServer::DequeueOutputBuffer(uint32_t *index, int64_t timetUs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == AVCODEC_RUNNING, MSERR_INVALID_OPERATION, "invalid state");

    if (outQueue_.size() < 0) {
        return MSERR_INVALID_OPERATION;
    }
    *index = outQueue_.front();
    return MSERR_OK;
}


int32_t AVCodecServer::DumpInfo(int32_t fd)
{
    std::string dumpString;
    dumpString += "In AVCodecServer::DumpInfo\n";
    dumpString += "Current AVCodecServer state is: " + std::to_string(status_) + "\n";
    if (lastErrMsg_.size() != 0) {
        dumpString += "AVCodecServer last error is: " + lastErrMsg_ + "\n";
    }
    dumpString += config_.Stringify();
    write(fd, dumpString.c_str(), dumpString.size());

    return MSERR_OK;
}

const std::string &AVCodecServer::GetStatusDescription(OHOS::Media::AVCodecServer::AVCodecStatus status)
{
    static const std::string ILLEGAL_STATE = "PLAYER_STATUS_ILLEGAL";
    if (status < OHOS::Media::AVCodecServer::AVCODEC_UNINITIALIZED ||
        status > OHOS::Media::AVCodecServer::AVCODEC_ERROR) {
        return ILLEGAL_STATE;
    }

    return AVCODEC_STATE_MAP.find(status)->second;
}

void AVCodecServer::OnError(int32_t errorType, int32_t errorCode)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    lastErrMsg_ = MSErrorToExtErrorString(static_cast<MediaServiceErrCode>(errorCode));
    FaultEventWrite(lastErrMsg_, "AVCodec");
    if (codecCb_ == nullptr) {
        return;
    }
    codecCb_->OnError(static_cast<AVCodecErrorType>(errorType), errorCode);
}

void AVCodecServer::OnOutputFormatChanged(const Format &format)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (codecCb_ == nullptr) {
        return;
    }
    codecCb_->OnOutputFormatChanged(format);
}

void AVCodecServer::OnInputBufferAvailable(uint32_t index)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (codecCb_ == nullptr) {
        return;
    }
    inQueue_.push(index);
    codecCb_->OnInputBufferAvailable(index);
}

void AVCodecServer::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (isFirstFrameOut_) {
        MediaTrace::TraceEnd("AVCodecServer::FirstFrame", firstFrameTraceId_);
        isFirstFrameOut_ = false;
    } else {
        MediaTrace::TraceEnd("AVCodecServer::Frame", FAKE_POINTER(this));
    }

    if (flag == AVCODEC_BUFFER_FLAG_EOS) {
        ResetTrace();
    } else {
        MediaTrace::TraceBegin("AVCodecServer::Frame", FAKE_POINTER(this));
    }

    if (codecCb_ == nullptr) {
        return;
    }
    outQueue_.push(index);
    codecCb_->OnOutputBufferAvailable(index, info, flag);
}

void AVCodecServer::ResetTrace()
{
    isFirstFrameIn_ = true;
    isFirstFrameOut_ = true;
    MediaTrace::TraceEnd("AVCodecServer::Frame", FAKE_POINTER(this));
    MediaTrace::TraceEnd("AVCodecServer::FirstFrame", firstFrameTraceId_);
}



AVCodecBaseCallback::AVCodecBaseCallback(const sptr<AVCodecServer> &codec)
    : codec_(listener)
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecBaseCallback::~AVCodecBaseCallback()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void AVCodecBaseCallback::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
    if (codec_ != nullptr) {
        codec_->OnError(errorType, errorCode);
    }
}

void AVCodecBaseCallback::OnOutputFormatChanged(const Format &format)
{
    if (codec_ != nullptr) {
        codec_->OnOutputFormatChanged(format);
    }
}

void AVCodecBaseCallback::OnInputBufferAvailable(uint32_t index)
{
    if (codec_ != nullptr) {
        codec_->OnInputBufferAvailable(index);
    }
}

void AVCodecBaseCallback::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    if (codec_ != nullptr) {
        codec_->OnOutputBufferAvailable(index, info, flag);
    }
}

} // namespace AVCodec
} // namespace OHOS
