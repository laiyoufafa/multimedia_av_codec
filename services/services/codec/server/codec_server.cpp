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
#include "avcodec_dfx.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
// #include "engine_factory_repo.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecServer"};
    const std::map<OHOS::Media::CodecServer::CodecStatus, std::string> CODEC_STATE_MAP = {
        {OHOS::Media::CodecServer::UNINITIALIZED, "uninitialized"},
        {OHOS::Media::CodecServer::INITIALIZED, "initialized"},
        {OHOS::Media::CodecServer::CONFIGURED, "configured"},
        {OHOS::Media::CodecServer::RUNNING, "running"},
        {OHOS::Media::CodecServer::FLUSHED, "flushed"},
        {OHOS::Media::CodecServer::END_OF_STREAM, "end of stream"},
        {OHOS::Media::CodecServer::ERROR, "error"},
    };
}

namespace OHOS {
namespace Media {
std::shared_ptr<ICodecService> CodecServer::Create()
{
    std::shared_ptr<CodecServer> server = std::make_shared<CodecServer>();
    int32_t ret = server->InitServer();
    if (ret != AVCS_ERR_OK) {
        AVCODEC_LOGE("failed to init CodecServer");
        return nullptr;
    }
    return server;
}

CodecServer::CodecServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecServer::~CodecServer()
{
    std::unique_ptr<std::thread> thread = std::make_unique<std::thread>(&CodecServer::ExitProcessor, this);
    if (thread != nullptr && thread->joinable()) {
        thread->join();
    }
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void CodecServer::ExitProcessor()
{
    codecEngine_ = nullptr;
}

int32_t CodecServer::InitServer()
{

    return AVCS_ERR_OK;
}

int32_t CodecServer::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("CodecServer::Init");
    if (isMimeType) {
        codec_ = CodecBase::Create(type == AVCODEC_TYPE_ENCODER,  name);
    } else {
        codec_ = CodecBase::Create(name);
    }
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");

    std::shared_ptr<AVCodecCallback> callback = std::make_shared<CodecBaseCallback>(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AVCS_ERR_NO_MEMORY, "failed to new CodecBaseCallback");
    CHECK_AND_RETURN_RET_LOG(codec_->SetCallback(callback), AVCS_ERR_INVALID_OPERATION,
        "CodecBase SetCallback failed, error: %{public}d", ret);

    status_ = INITIALIZED;
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    return AVCS_ERR_OK;
}

int32_t CodecServer::Configure(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("CodecServer::Configure");
    CHECK_AND_RETURN_RET_LOG(status_ == INITIALIZED, AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    config_ = format;
    int32_t ret = codec_->Configure(format);
    status_ = (ret == AVCS_ERR_OK ? CONFIGURED : ERROR);
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    return ret;
}

int32_t CodecServer::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("CodecServer::Start");
    CHECK_AND_RETURN_RET_LOG(status_ == FLUSHED || status_ == CONFIGURED,
        AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codec_->Start();
    if (codecCb_) {
        status_ = (ret == AVCS_ERR_OK ? RUNNING : ERROR);
    } else {
        status_ = (ret == AVCS_ERR_OK ? FLUSHED : ERROR);
    }
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    return ret;
}

int32_t CodecServer::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("CodecServer::Stop");
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING, AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codec_->Stop();
    status_ = (ret == AVCS_ERR_OK ? CONFIGURED : ERROR);
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    // ResetTrace();
    return ret;
}

int32_t CodecServer::Flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("CodecServer::Flush");
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING || status_ == END_OF_STREAM,
        AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codec_->Flush();
    status_ = (ret == AVCS_ERR_OK ? FLUSHED : ERROR);
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    // ResetTrace();
    return ret;
}

int32_t CodecServer::NotifyEos()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("CodecServer::NotifyEos");
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING, AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codec_->NotifyEos();
    if (ret == AVCS_ERR_OK) {
        status_ = END_OF_STREAM;
        // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
        // AVCODEC_LOGI("EOS state");
    }
    return ret;
}

int32_t CodecServer::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("CodecServer::Reset");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codec_->Reset();
    status_ = (ret == AVCS_ERR_OK ? INITIALIZED : ERROR);
    // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
    lastErrMsg_.clear();
    // ResetTrace();
    return ret;
}

int32_t CodecServer::Release()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // MediaTrace trace("CodecServer::Release");
    std::unique_ptr<std::thread> thread = std::make_unique<std::thread>(&CodecServer::ExitProcessor, this);
    if (thread != nullptr && thread->joinable()) {
        thread->join();
    }
    // ResetTrace();
    return AVCS_ERR_OK;
}

sptr<Surface> CodecServer::CreateInputSurface()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == CONFIGURED, nullptr, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, nullptr, "codecBase is nullptr");
    sptr<Surface> surface = codec_->CreateInputSurface();
    firstFrameTraceId_ = FAKE_POINTER(surface.GetRefPtr());
    return surface;
}

int32_t CodecServer::SetOutputSurface(sptr<Surface> surface)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == CONFIGURED, AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    return codec_->SetOutputSurface(surface);
}

std::shared_ptr<AVBufferElement> CodecServer::GetInputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING, nullptr, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, nullptr, "codecBase is nullptr");
    return codec_->GetInputBuffer(index);
}

int32_t CodecServer::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    MediaTrace trace("CodecServer::QueueInputBuffer");
    std::lock_guard<std::mutex> lock(mutex_);
    firstFrameTraceId_ = FAKE_POINTER(this);
    if (isFirstFrameIn_) {
        MediaTrace::TraceBegin("CodecServer::FirstFrame", firstFrameTraceId_);
        isFirstFrameIn_ = false;
    }
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING, AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codec_->QueueInputBuffer(index, info, flag);
    if (flag & AVCODEC_BUFFER_FLAG_EOS) {
        if (ret == AVCS_ERR_OK) {
            status_ = END_OF_STREAM;
            // BehaviorEventWrite(GetStatusDescription(status_), "AVCodec");
            AVCODEC_LOGI("EOS state");
        }
    }
    if (inQueue_.size() && ret == AVCS_ERR_OK) {
        inQueue_.pop();
    }
    return ret;
}

std::shared_ptr<AVBufferElement> CodecServer::GetOutputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING || status_ == END_OF_STREAM,
        nullptr, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, nullptr, "codecBase is nullptr");
    return codec_->GetOutputBuffer(index);
}

int32_t CodecServer::GetOutputFormat(Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ != UNINITIALIZED, AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    return codec_->GetOutputFormat(format);
}

int32_t CodecServer::ReleaseOutputBuffer(uint32_t index, bool render)
{
    MediaTrace trace("CodecServer::ReleaseOutputBuffer");
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING || status_ == END_OF_STREAM,
        AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    int32_t ret = codec_->ReleaseOutputBuffer(index, render);
    if (ret == AVCS_ERR_OK && outQueue_.size()) {
        outQueue_.pop();
    }
    return ret;
}

int32_t CodecServer::SetParameter(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ != INITIALIZED && status_ != CONFIGURED,
        AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    return codec_->SetParameter(format);
}

int32_t CodecServer::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == INITIALIZED, AVCS_ERR_INVALID_OPERATION, "invalid state");
    {
        std::lock_guard<std::mutex> cbLock(cbMutex_);
        codecCb_ = callback;
    }
    // CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    // (void)codec_->SetCallback(callback);
    return AVCS_ERR_OK;
}

int32_t CodecServer::SetInputSurface(sptr<PersistentSurface> surface)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == CONFIGURED, AVCS_ERR_INVALID_OPERATION, "invalid state");
    CHECK_AND_RETURN_RET_LOG(codec_ != nullptr, AVCS_ERR_NO_MEMORY, "codecBase is nullptr");
    return codec_->SetOutputSurface(surface);
}

int32_t CodecServer::DequeueInputBuffer(uint32_t *index, int64_t timetUs)
{

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING, AVCS_ERR_INVALID_OPERATION, "invalid state");

    if (inQueue_.size() <= 0) {
        return AVCS_ERR_INVALID_OPERATION;
    }
    *index = inQueue_.front();
    return AVCS_ERR_OK;
}

int32_t CodecServer::DequeueOutputBuffer(uint32_t *index, int64_t timetUs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING, AVCS_ERR_INVALID_OPERATION, "invalid state");

    if (outQueue_.size() <= 0) {
        return AVCS_ERR_INVALID_OPERATION;
    }
    *index = outQueue_.front();
    return AVCS_ERR_OK;
}

int32_t CodecServer::DumpInfo(int32_t fd)
{
    std::string dumpString;
    dumpString += "In CodecServer::DumpInfo\n";
    dumpString += "Current CodecServer state is: " + std::to_string(status_) + "\n";
    if (lastErrMsg_.size() != 0) {
        dumpString += "CodecServer last error is: " + lastErrMsg_ + "\n";
    }
    dumpString += config_.Stringify();
    write(fd, dumpString.c_str(), dumpString.size());

    return AVCS_ERR_OK;
}

const std::string &CodecServer::GetStatusDescription(OHOS::Media::CodecServer::CodecStatus status)
{
    static const std::string ILLEGAL_STATE = "PLAYER_STATUS_ILLEGAL";
    if (status < OHOS::Media::CodecServer::UNINITIALIZED ||
        status > OHOS::Media::CodecServer::ERROR) {
        return ILLEGAL_STATE;
    }

    return CODEC_STATE_MAP.find(status)->second;
}

void CodecServer::OnError(int32_t errorType, int32_t errorCode)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    lastErrMsg_ = MSErrorToExtErrorString(static_cast<MediaServiceErrCode>(errorCode));
    FaultEventWrite(lastErrMsg_, "Codec");
    if (codecCb_ == nullptr) {
        return;
    }
    codecCb_->OnError(static_cast<AVCodecErrorType>(errorType), errorCode);
}

void CodecServer::OnOutputFormatChanged(const Format &format)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (codecCb_ == nullptr) {
        return;
    }
    codecCb_->OnOutputFormatChanged(format);
}

void CodecServer::OnInputBufferAvailable(uint32_t index)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (codecCb_ == nullptr) {
        return;
    }
    inQueue_.push(index);
    codecCb_->OnInputBufferAvailable(index);
}

void CodecServer::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (isFirstFrameOut_) {
        MediaTrace::TraceEnd("CodecServer::FirstFrame", firstFrameTraceId_);
        isFirstFrameOut_ = false;
    } else {
        MediaTrace::TraceEnd("CodecServer::Frame", FAKE_POINTER(this));
    }

    if (flag == AVCODEC_BUFFER_FLAG_EOS) {
        ResetTrace();
    } else {
        MediaTrace::TraceBegin("CodecServer::Frame", FAKE_POINTER(this));
    }

    if (codecCb_ == nullptr) {
        return;
    }
    outQueue_.push(index);
    codecCb_->OnOutputBufferAvailable(index, info, flag);
}

void CodecServer::ResetTrace()
{
    isFirstFrameIn_ = true;
    isFirstFrameOut_ = true;
    MediaTrace::TraceEnd("CodecServer::Frame", FAKE_POINTER(this));
    MediaTrace::TraceEnd("CodecServer::FirstFrame", firstFrameTraceId_);
}



CodecBaseCallback::CodecBaseCallback(const sptr<CodecServer> &codec)
    : codec_(codec)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecBaseCallback::~CodecBaseCallback()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void CodecBaseCallback::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
    if (codec_ != nullptr) {
        codec_->OnError(errorType, errorCode);
    }
}

void CodecBaseCallback::OnOutputFormatChanged(const Format &format)
{
    if (codec_ != nullptr) {
        codec_->OnOutputFormatChanged(format);
    }
}

void CodecBaseCallback::OnInputBufferAvailable(uint32_t index)
{
    if (codec_ != nullptr) {
        codec_->OnInputBufferAvailable(index);
    }
}

void CodecBaseCallback::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    if (codec_ != nullptr) {
        codec_->OnOutputBufferAvailable(index, info, flag);
    }
}

} // namespace Media
} // namespace OHOS
