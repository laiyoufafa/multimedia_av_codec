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
#include <vector>
#include "avcodec_dfx.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "codec_factory.h"
#include "avcodec_dump_utils.h"
#include "media_description.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecServer"};
    constexpr uint32_t DUMP_CODEC_INFO_INDEX = 0x01010000;
    constexpr uint32_t DUMP_STATUS_INDEX = 0x01010100;
    constexpr uint32_t DUMP_LAST_ERROR_INDEX = 0x01010200;
    constexpr uint32_t DUMP_OFFSET_8 = 8;

    const std::map<OHOS::Media::CodecServer::CodecStatus, std::string> CODEC_STATE_MAP = {
        {OHOS::Media::CodecServer::UNINITIALIZED, "uninitialized"},
        {OHOS::Media::CodecServer::INITIALIZED, "initialized"},
        {OHOS::Media::CodecServer::CONFIGURED, "configured"},
        {OHOS::Media::CodecServer::RUNNING, "running"},
        {OHOS::Media::CodecServer::FLUSHED, "flushed"},
        {OHOS::Media::CodecServer::END_OF_STREAM, "end of stream"},
        {OHOS::Media::CodecServer::ERROR, "error"},
    };

    const std::vector<std::pair<std::string_view, const std::string>> DEFAULT_DUMP_TABLE = {
        { OHOS::Media::MediaDescriptionKey::MD_KEY_CODEC_NAME, "Codec_Name" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_BITRATE, "Bit_Rate" },
    };
            
    const std::vector<std::pair<std::string_view, const std::string>> VIDEO_DUMP_TABLE = {
        { OHOS::Media::MediaDescriptionKey::MD_KEY_CODEC_NAME, "Codec_Name" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_WIDTH, "Width" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_HEIGHT, "Height" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_FRAME_RATE, "Frame_Rate" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_BITRATE, "Bit_Rate" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, "Pixel_Format" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_SCALE_TYPE, "Scale_Type" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, "Rotation_Angle" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE, "Max_Input_Size" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_MAX_INPUT_BUFFER_COUNT, "Max_Input_Buffer_Count" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_MAX_OUTPUT_BUFFER_COUNT, "Max_Output_Buffer_Count" },
    };

    const std::vector<std::pair<std::string_view, const std::string>> AUDIO_DUMP_TABLE = {
        { OHOS::Media::MediaDescriptionKey::MD_KEY_CODEC_NAME, "Codec_Name" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, "Channel_Count" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_BITRATE, "Bit_Rate" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_SAMPLE_RATE, "Sample_Rate" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_CODEC_CONFIG, "Codec_Config" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE, "Max_Input_Size" },
    };

    const std::map<OHOS::Media::CodecServer::CodecType, 
        std::vector<std::pair<std::string_view, const std::string>>> CODEC_DUMP_TABLE = {
        { OHOS::Media::CodecServer::CodecType::CODEC_TYPE_DEFAULT, DEFAULT_DUMP_TABLE },
        { OHOS::Media::CodecServer::CodecType::CODEC_TYPE_VIDEO, VIDEO_DUMP_TABLE },
        { OHOS::Media::CodecServer::CodecType::CODEC_TYPE_AUDIO, AUDIO_DUMP_TABLE },
    };
}

namespace OHOS {
namespace Media {
std::shared_ptr<ICodecService> CodecServer::Create()
{
    std::shared_ptr<CodecServer> server = std::make_shared<CodecServer>();

    int32_t ret = server->InitServer();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Codec server init failed!");
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
    codecBase_ = nullptr;
}

int32_t CodecServer::InitServer()
{
    return AVCS_ERR_OK;
}

int32_t CodecServer::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (isMimeType) {
        bool isEncoder = (type == AVCODEC_TYPE_VIDEO_ENCODER) || (type == AVCODEC_TYPE_AUDIO_ENCODER);
        codecBase_ = CodecFactory::Instance().CreateCodecByMime(isEncoder, name);
    } else {
        codecBase_ = CodecFactory::Instance().CreateCodecByName(name);
    }
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "CodecBase is nullptr");
    codecName_ = name;
    std::shared_ptr<AVCodecCallback> callback = std::make_shared<CodecBaseCallback>(shared_from_this());
    int32_t ret = codecBase_->SetCallback(callback);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_INVALID_OPERATION, "CodecBase SetCallback failed");
    status_ = INITIALIZED;
    AVCODEC_LOGI("Codec server in %{public}s status", GetStatusDescription(status_).data());
    return AVCS_ERR_OK;
}

int32_t CodecServer::Configure(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == INITIALIZED, AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    config_ = format;
    int32_t ret = codecBase_->Configure(format);

    status_ = (ret == AVCS_ERR_OK ? CONFIGURED : ERROR);
    AVCODEC_LOGI("Codec server in %{public}s status", GetStatusDescription(status_).data());
    return ret;
}

int32_t CodecServer::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == FLUSHED || status_ == CONFIGURED,
        AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    int32_t ret = codecBase_->Start();
    if (codecCb_) {
        status_ = (ret == AVCS_ERR_OK ? RUNNING : ERROR);
    } else {
        status_ = (ret == AVCS_ERR_OK ? FLUSHED : ERROR);
    }
    AVCODEC_LOGI("Codec server in %{public}s status", GetStatusDescription(status_).data());
    return ret;
}

int32_t CodecServer::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING || status_ == END_OF_STREAM ||
        status_ == FLUSHED, AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    int32_t ret = codecBase_->Stop();
    status_ = (ret == AVCS_ERR_OK ? CONFIGURED : ERROR);
    AVCODEC_LOGI("Codec server in %{public}s status", GetStatusDescription(status_).data());
    return ret;
}

int32_t CodecServer::Flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING || status_ == END_OF_STREAM,
        AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    int32_t ret = codecBase_->Flush();
    status_ = (ret == AVCS_ERR_OK ? FLUSHED : ERROR);
    AVCODEC_LOGI("Codec server in %{public}s status", GetStatusDescription(status_).data());
    return ret;
}

int32_t CodecServer::NotifyEos()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING, AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    int32_t ret = codecBase_->NotifyEos();
    if (ret == AVCS_ERR_OK) {
        status_ = END_OF_STREAM;
        AVCODEC_LOGI("Codec server in %{public}s status", GetStatusDescription(status_).data());
        AVCODEC_LOGI("EOS state");
    }
    return ret;
}

int32_t CodecServer::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    int32_t ret = codecBase_->Reset();
    status_ = (ret == AVCS_ERR_OK ? INITIALIZED : ERROR);
    AVCODEC_LOGI("Codec server in %{public}s status", GetStatusDescription(status_).data());
    lastErrMsg_.clear();
    return ret;
}

int32_t CodecServer::Release()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::unique_ptr<std::thread> thread = std::make_unique<std::thread>(&CodecServer::ExitProcessor, this);
    if (thread != nullptr && thread->joinable()) {
        thread->join();
    }
    return AVCS_ERR_OK;
}

sptr<Surface> CodecServer::CreateInputSurface()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == CONFIGURED, nullptr, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, nullptr, "Codecbase is nullptr");
    sptr<Surface> surface = codecBase_->CreateInputSurface();
    firstFrameTraceId_ = FAKE_POINTER(surface.GetRefPtr());
    return surface;
}

int32_t CodecServer::SetOutputSurface(sptr<Surface> surface)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == CONFIGURED, AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    return codecBase_->SetOutputSurface(surface);
}

std::shared_ptr<AVSharedMemory> CodecServer::GetInputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING, nullptr, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, nullptr, "Codec base is nullptr");
    return codecBase_->GetInputBuffer(index);
}

int32_t CodecServer::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    std::lock_guard<std::mutex> lock(mutex_);
    firstFrameTraceId_ = FAKE_POINTER(this);
    if (isFirstFrameIn_) {
        AVCodecTrace::TraceBegin("CodecServer::FirstFrame", firstFrameTraceId_);
        isFirstFrameIn_ = false;
    }
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING, AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    int32_t ret = codecBase_->QueueInputBuffer(index, info, flag);
    if (flag & AVCODEC_BUFFER_FLAG_EOS) {
        if (ret == AVCS_ERR_OK) {
            status_ = END_OF_STREAM;
            AVCODEC_LOGI("Codec server in %{public}s status", GetStatusDescription(status_).data());
        }
    }
    return ret;
}

std::shared_ptr<AVSharedMemory> CodecServer::GetOutputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING || status_ == END_OF_STREAM,
        nullptr, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, nullptr, "Codecbase is nullptr");
    return codecBase_->GetOutputBuffer(index);
}

int32_t CodecServer::GetOutputFormat(Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ != UNINITIALIZED, AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    return codecBase_->GetOutputFormat(format);
}

int32_t CodecServer::ReleaseOutputBuffer(uint32_t index, bool render)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ == RUNNING || status_ == END_OF_STREAM,
        AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    
    int32_t ret;
    if (render) {
        ret = codecBase_->RenderOutputBuffer(index);
    } else {
        ret = codecBase_->ReleaseOutputBuffer(index);
    }
    return ret;
}

int32_t CodecServer::SetParameter(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(status_ != INITIALIZED && status_ != CONFIGURED,
        AVCS_ERR_INVALID_STATE, "In invalid state");
    CHECK_AND_RETURN_RET_LOG(codecBase_ != nullptr, AVCS_ERR_NO_MEMORY, "Codecbase is nullptr");
    return codecBase_->SetParameter(format);
}

int32_t CodecServer::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    {
        std::lock_guard<std::mutex> cbLock(cbMutex_);
        codecCb_ = callback;
    }

    return AVCS_ERR_OK;
}

int32_t CodecServer::DumpInfo(int32_t fd)
{
    Format codecFormat;
    int32_t ret = codecBase_->GetOutputFormat(codecFormat);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Get codec format failed.");
    CodecType codecType = GetCodecType();
    auto it = CODEC_DUMP_TABLE.find(codecType);
    auto &dumpTable = 
        it != CODEC_DUMP_TABLE.end() ? it->second : DEFAULT_DUMP_TABLE;
    AVCodecDumpControler dumpControler;
    std::string codecInfo;

    switch (codecType) {
    case CODEC_TYPE_VIDEO:
        codecInfo = "Video_Codec_Info";
        break;
    case CODEC_TYPE_DEFAULT:
        codecInfo = "Codec_Info";
        break;
    default:
        codecInfo = "Audio_Codec_Info";
        break;
    }

    dumpControler.AddInfo(DUMP_CODEC_INFO_INDEX, codecInfo);
    dumpControler.AddInfo(DUMP_STATUS_INDEX, 
        "Status", CODEC_STATE_MAP.find(status_)->second);
    dumpControler.AddInfo(DUMP_LAST_ERROR_INDEX, 
        "Last_Error", lastErrMsg_.size() ? lastErrMsg_ : "Null");

    int32_t dumpIndex = 3;
    for (auto iter : dumpTable) {
        dumpControler.AddInfoFromFormat(
            DUMP_CODEC_INFO_INDEX + (dumpIndex << DUMP_OFFSET_8), 
            codecFormat, iter.first, iter.second);
        dumpIndex++;
    }
        
    std::string dumpString;
    dumpControler.GetDumpString(dumpString);
    write(fd, dumpString.c_str(), dumpString.size());
    return AVCS_ERR_OK;
}

const std::string &CodecServer::GetStatusDescription(OHOS::Media::CodecServer::CodecStatus status)
{
    static const std::string ILLEGAL_STATE = "CODEC_STATUS_ILLEGAL";
    if (status < OHOS::Media::CodecServer::UNINITIALIZED ||
        status > OHOS::Media::CodecServer::ERROR) {
        return ILLEGAL_STATE;
    }

    return CODEC_STATE_MAP.find(status)->second;
}

void CodecServer::OnError(int32_t errorType, int32_t errorCode)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    lastErrMsg_ = AVCSErrorToOHAVErrCodeString(static_cast<AVCodecServiceErrCode>(errorCode));
    FaultEventWrite(errorCode, lastErrMsg_, "Codec");
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
    codecCb_->OnInputBufferAvailable(index);
}

void CodecServer::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (isFirstFrameOut_) {
        AVCodecTrace::TraceEnd("CodecServer::FirstFrame", firstFrameTraceId_);
        isFirstFrameOut_ = false;
    } else {
        AVCodecTrace::TraceEnd("CodecServer::Frame", FAKE_POINTER(this));
    }

    if (flag == AVCODEC_BUFFER_FLAG_EOS) {
        ResetTrace();
    } else {
        AVCodecTrace::TraceBegin("CodecServer::Frame", FAKE_POINTER(this));
    }

    if (codecCb_ == nullptr) {
        return;
    }
    codecCb_->OnOutputBufferAvailable(index, info, flag);
}

void CodecServer::ResetTrace()
{
    isFirstFrameIn_ = true;
    isFirstFrameOut_ = true;
    AVCodecTrace::TraceEnd("CodecServer::Frame", FAKE_POINTER(this));
    AVCodecTrace::TraceEnd("CodecServer::FirstFrame", firstFrameTraceId_);
}

CodecBaseCallback::CodecBaseCallback(const std::shared_ptr<CodecServer> &codec)
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

CodecServer::CodecType CodecServer::GetCodecType()
{
    CodecType codecType;
    
    if (codecName_.find("video") != codecName_.npos) {
        codecType = CodecType::CODEC_TYPE_VIDEO;
    } else if (codecName_.find("audio") != codecName_.npos) {
        codecType = CodecType::CODEC_TYPE_AUDIO;
    } else {
        codecType = CodecType::CODEC_TYPE_DEFAULT;
    }

    return codecType;
}
} // namespace Media
} // namespace OHOS