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

#include "audio_ffmpeg_adapter.h"
#include "avcodec_dfx.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegDecoderAdapter"};
}

namespace OHOS {
namespace Media {

const std::string CHANNEL_COUNT_KEY{"channel-count"};
const std::string SAMPLE_RATE_KEY{"simplerate"};
const std::string BIT_RATE_KEY{"bitrate"};

AudioFFMpegAdapter::AudioFFMpegAdapter(const std::string_view &name) : state_(CodecState::RELEASED), name_(name) {}

AudioFFMpegAdapter::~AudioFFMpegAdapter() {
    callback_ = nullptr;
    audioCodec->release();
    state_ = CodecState::RELEASED;
    audioCodec = nullptr;
}

int32_t AudioFFMpegAdapter::SetCallback(const std::shared_ptr<AVCodecCallback> &callback) {
    AVCodecTrace trace(std::string(__FUNCTION__));
    callback_ = callback;
    AVCODEC_LOGD("SetCallback success");
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAdapter::Configure(const Format &format) {
    AVCodecTrace trace(std::string(__FUNCTION__));
    AVCODEC_LOGD("Configure enter");

    if (!format.ContainKey(CHANNEL_COUNT_KEY)) {
        AVCODEC_LOGE("Configure failed, missing channel count key in format.");
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT;
    }

    if (!format.ContainKey(SAMPLE_RATE_KEY)) {
        AVCODEC_LOGE("Configure failed,missing sample rate key in format.");
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_SAMPLE_RATE;
    }

    if (!format.ContainKey(BIT_RATE_KEY)) {
        AVCODEC_LOGE("adapter configure error,missing bits rate key in format.");
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_BIT_RATE;
    }

    if (state_ != CodecState::RELEASED) {
        AVCODEC_LOGE("Configure failed, state = %{public}s .", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    AVCODEC_LOGI("state from %{public}s to INITLIZING", stateToString(state_).data());
    state_ = CodecState::INITLIZING;
    auto ret = doInit();

    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return ret;
    }

    if (state_ != CodecState::INITLIZED) {
        AVCODEC_LOGE("Configure failed, state =%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_ERROR;
    }

    ret = doConfigure(format);
    AVCODEC_LOGD("Configure exit");
    return ret;
}

int32_t AudioFFMpegAdapter::Start() {
    AVCODEC_LOGD("Start enter");
    if (!callback_) {
        AVCODEC_LOGE("ffmpeg adapter start error, callback not initlized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    if (!audioCodec) {
        AVCODEC_LOGE("ffmpeg adapter start error, audio codec not initlized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    if (state_ == CodecState::FLUSHED) {
        AVCODEC_LOGI("Start, doResume");
        return doResume();
    }

    if (state_ != CodecState::INITLIZED) {
        AVCODEC_LOGE("Start is incorrect, state = %{public}s .", stateToString(state_).data());
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("Start, state from %{public}s to STARTING", stateToString(state_).data());
    state_ = CodecState::STARTING;
    auto ret = doStart();
    return ret;
}

int32_t AudioFFMpegAdapter::Stop() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    AVCODEC_LOGD("Stop enter");
    if (!callback_) {
        AVCODEC_LOGE("Stop failed, call back not initlized.");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    if (state_ == CodecState::INITLIZED || state_ == CodecState::RELEASED || state_ == CodecState::STOPPING ||
        state_ == CodecState::RRELEASING) {
        AVCODEC_LOGD("Stop, state_=%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }
    state_ = CodecState::STOPPING;
    auto ret = doStop();
    AVCODEC_LOGI("adapter Stop, state from %{public}s to INITLIZED", stateToString(state_).data());
    state_ = CodecState::INITLIZED;
    return ret;
}

int32_t AudioFFMpegAdapter::Flush() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    AVCODEC_LOGD("adapter Flush enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter flush error, call back not initlized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    if (state_ == CodecState::FLUSHED) {
        AVCODEC_LOGW("Flush, state is already flushed, state_=%{public}s .", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }
    if (state_ != CodecState::RUNNING) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("Flush failed, state =%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }
    AVCODEC_LOGI("Flush, state from %{public}s to FLUSHING", stateToString(state_).data());
    state_ = CodecState::FLUSHING;
    auto ret = doFlush();
    return ret;
}

int32_t AudioFFMpegAdapter::Reset() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    AVCODEC_LOGD("adapter Reset enter");
    if (worker_) {
        worker_->Release();
    }
    int32_t status = audioCodec->reset();
    state_ = CodecState::INITLIZED;
    AVCODEC_LOGI("adapter Reset, state from %{public}s to INITLIZED", stateToString(state_).data());
    return status;
}

int32_t AudioFFMpegAdapter::Release() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    AVCODEC_LOGD("adapter Release enter");
    if (state_ == CodecState::RELEASED || state_ == CodecState::RRELEASING) {
        AVCODEC_LOGW("adapter Release, state isnot completely correct, state =%{public}s .",
                     stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }

    if (state_ == CodecState::INITLIZING) {
        AVCODEC_LOGW("adapter Release, state isnot completely correct, state =%{public}s .",
                     stateToString(state_).data());
        state_ = CodecState::RRELEASING;
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }

    if (state_ == CodecState::STARTING || state_ == CodecState::RUNNING || state_ == CodecState::STOPPING) {
        AVCODEC_LOGE("adapter Release, state is incorrect, state =%{public}s .", stateToString(state_).data());
    }
    AVCODEC_LOGI("adapter Release, state from %{public}s to RRELEASING", stateToString(state_).data());
    state_ = CodecState::RRELEASING;
    auto ret = doRelease(); // todo:异步调用
    return ret;
}

int32_t AudioFFMpegAdapter::NotifyEos() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    Flush();
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAdapter::SetParameter(const Format &format) {
    AVCodecTrace trace(std::string(__FUNCTION__));
    (void)format;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAdapter::GetOutputFormat(Format &format) {
    AVCodecTrace trace(std::string(__FUNCTION__));
    format = audioCodec->GetFormat();
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

std::shared_ptr<AVSharedMemory> AudioFFMpegAdapter::GetInputBuffer(size_t index) {
    AVCodecTrace trace(std::string(__FUNCTION__));
    AVCODEC_LOGD("adapter GetInputBuffer enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter get input buffer error, call back not initlized .");
        return nullptr;
    }
    if (index < 0) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL);
        AVCODEC_LOGE("index=%{public}d error", index);
        return nullptr;
    }

    std::shared_ptr<AudioBufferInfo> result = worker_->GetInputBufferInfo(index);
    if (result == nullptr) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        AVCODEC_LOGE("getMemory failed");
        return nullptr;
    }

    if (result->GetStatus() == BufferStatus::IDEL) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("GetStatus is IDEL");
        return nullptr;
    }

    return result->GetBuffer();
}

int32_t AudioFFMpegAdapter::QueueInputBuffer(size_t index, const AVCodecBufferInfo &info, AVCodecBufferFlag &flag) {
    AVCodecTrace trace(std::string(__FUNCTION__));
    AVCODEC_LOGD("adapter QueueInputBuffer enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter queue input buffer error, call back not initlized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    if (index < 0) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL);
        AVCODEC_LOGE("index=%{public}d error", index);
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }

    auto result = worker_->GetInputBufferInfo(index);
    if (result == nullptr) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        AVCODEC_LOGE("getMemory failed");
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }

    if (result->GetStatus() != BufferStatus::OWNE_BY_CLIENT) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("GetStatus failed");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    result->SetBufferAttr(info);
    if (flag == AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_EOS) {
        result->SetEos(true);
    }
    worker_->PushInputData(index);
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

std::shared_ptr<AVSharedMemory> AudioFFMpegAdapter::GetOutputBuffer(size_t index) {
    AVCodecTrace trace(std::string(__FUNCTION__));
    AVCODEC_LOGD("adapter GetOutputBuffer enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter get output buffer error, call back not initlized .");
        return nullptr;
    }

    if (index < 0) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL);
        AVCODEC_LOGE("index=%{public}d error", index);
        return nullptr;
    }

    auto result = worker_->GetOutputBufferInfo(index);
    if (result == nullptr) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        AVCODEC_LOGE("getMemory failed");
        return nullptr;
    }

    if (result->GetStatus() == BufferStatus::IDEL) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        return nullptr;
    }

    return result->GetBuffer();
}

int32_t AudioFFMpegAdapter::ReleaseOutputBuffer(size_t index) {
    AVCodecTrace trace(std::string(__FUNCTION__));
    AVCODEC_LOGD("adapter ReleaseOutputBuffer enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter release output buffer error, call back not initlized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    if (index < 0) {
        AVCODEC_LOGE("index=%{public}d error", index);
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL);
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }

    bool isEos = worker_->GetOutputBufferInfo(index)->CheckIsEos();

    bool result = worker_->GetOutputBuffer()->RelaseBuffer(index);

    if (!result) {
        AVCODEC_LOGE("RelaseBuffer failed");
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }

    if (isEos) {
        NotifyEos();
    }

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAdapter::doInit() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    if (name_.empty()) {
        state_ = CodecState::RELEASED;
        AVCODEC_LOGE("doInit failed, because name is empty");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    audioCodec = IAudioFFMpegBaseCodec::make_sharePtr(name_);

    if (audioCodec == nullptr) {
        state_ = CodecState::RELEASED;
        AVCODEC_LOGE("Initlize failed, because create codec failed. name: %{public}s.", name_.data());
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("adapter doInit, state from %{public}s to INITLIZED", stateToString(state_).data());
    state_ = CodecState::INITLIZED;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAdapter::doConfigure(const Format &format) {
    AVCodecTrace trace(std::string(__FUNCTION__));
    if (state_ != CodecState::INITLIZED) {
        AVCODEC_LOGE("adapter configure failed because state is incrrect,state:%{public}d.",
                     static_cast<int>(state_.load()));
        state_ = CodecState::RELEASED;
        if (callback_) {
            callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        }
        AVCODEC_LOGE("state_=%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }
    int32_t ret = audioCodec->init(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("configure failed, because codec init failed,error:%{public}d.", static_cast<int>(ret));
        state_ = CodecState::RELEASED;
        if (callback_) {
            callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, ret);
        }
        return ret;
    }
    return ret;
}

int32_t AudioFFMpegAdapter::doStart() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    if (state_ != CodecState::STARTING) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("doStart failed, state = %{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }

    AVCODEC_LOGI("adapter doStart, state from %{public}s to RUNNING", stateToString(state_).data());
    state_ = CodecState::RUNNING;
    worker_ = std::make_shared<AudioCodecWorker>(audioCodec, callback_);
    worker_->Start();
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAdapter::doResume() {
    AVCODEC_LOGI("adapter doResume, state from %{public}s to RESUMING", stateToString(state_).data());
    state_ = CodecState::RESUMING;
    worker_->Resume();
    AVCODEC_LOGI("adapter doResume, state from %{public}s to RUNNING", stateToString(state_).data());
    state_ = CodecState::RUNNING;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAdapter::doStop() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    if (state_ == CodecState::RRELEASING) {
        AVCODEC_LOGW("adapter doStop, state is not completely correct, state_=%{public}s .",
                     stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }

    if (state_ != CodecState::STOPPING) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("doStop failed, state =%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }
    worker_->Stop();
    if (state_ == CodecState::STOPPING) {
        AVCODEC_LOGI("adapter doStop, state from %{public}s to INITLIZED", stateToString(state_).data());
        state_ = CodecState::INITLIZED;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAdapter::doFlush() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    if (state_ != CodecState::FLUSHING) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("doFlush failed, state_=%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    int32_t status = audioCodec->flush();

    worker_->Pause();

    state_ = CodecState::FLUSHED;
    if (status != AVCodecServiceErrCode::AVCS_ERR_OK) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("status=%{public}d", static_cast<int>(status));
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAdapter::doRelease() {
    AVCodecTrace trace(std::string(__FUNCTION__));
    if (state_ == CodecState::RELEASED) {
        AVCODEC_LOGW("adapter doRelease, state is not completely correct, state_=%{public}s .",
                     stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }
    audioCodec->release();
    worker_->Release();
    AVCODEC_LOGI("adapter doRelease, state from %{public}s to RELEASED", stateToString(state_).data());
    state_ = CodecState::RELEASED;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

std::string_view AudioFFMpegAdapter::stateToString(CodecState state) {
    std::map<CodecState, std::string_view> stateStrMap = {
        {CodecState::RELEASED, " RELEASED"},     {CodecState::INITLIZED, " INITLIZED"},
        {CodecState::FLUSHED, " FLUSHED"},       {CodecState::RUNNING, " RUNNING"},
        {CodecState::INITLIZING, " INITLIZING"}, {CodecState::STARTING, " STARTING"},
        {CodecState::STOPPING, " STOPPING"},     {CodecState::FLUSHING, " FLUSHING"},
        {CodecState::RESUMING, " RESUMING"},     {CodecState::RRELEASING, " RRELEASING"},
    };
    return stateStrMap[state];
}

} // namespace Media
} // namespace OHOS