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

#ifndef CODEC_SERVER_H
#define CODEC_SERVER_H

#include <queue>
#include "i_codec_service.h"
#include "nocopyable.h"

namespace OHOS {
namespace AVCodec {

class CodecBaseCallback : public AVCodecCallback, public NoCopyable {
public:
    explicit CodecBaseCallback(const sptr<CodecServer> &codec);
    virtual ~CodecBaseCallback();

    void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    void OnOutputFormatChanged(const Format &format) override;
    void OnInputBufferAvailable(uint32_t index) override;
    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
private:
    sptr<CodecServer> codec_ = nullptr;
};


class CodecServer : public ICodecService, public NoCopyable {
public:
    static std::shared_ptr<ICodecService> Create();
    CodecServer();
    virtual ~CodecServer();

    enum CodecStatus {
        CODEC_UNINITIALIZED = 0,
        CODEC_INITIALIZED,
        CODEC_CONFIGURED,
        CODEC_RUNNING,
        CODEC_FLUSHED,
        CODEC_END_OF_STREAM,
        CODEC_ERROR,
    };

    int32_t Init(AVCodecType type, bool isMimeType, const std::string &name) override;
    int32_t Configure(const Format &format) override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Flush() override;
    int32_t Reset() override;
    int32_t Release() override;
    int32_t NotifyEos() override;
    sptr<Surface> CreateInputSurface() override;
    int32_t SetOutputSurface(sptr<Surface> surface) override;
    std::shared_ptr<AVBufferElement> GetInputBuffer(uint32_t index) override;
    int32_t QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
    std::shared_ptr<AVBufferElement> GetOutputBuffer(uint32_t index) override;
    int32_t GetOutputFormat(Format &format) override;
    int32_t ReleaseOutputBuffer(uint32_t index, bool render) override;
    int32_t SetParameter(const Format &format) override;
    int32_t SetCallback(const std::shared_ptr<AVCodecCallback> &callback) override;
    int32_t SetInputSurface(sptr<PersistentSurface> surface) override;
    int32_t DequeueInputBuffer(uint32_t *index, int64_t timetUs) override;
    int32_t DequeueOutputBuffer(uint32_t *index, int64_t timetUs) override;
    // int32_t SetRenderedListener(const std::shared_ptr<AVCodecFrameRenderedListener> &listener) override;

    int32_t DumpInfo(int32_t fd);

    void OnError(int32_t errorType, int32_t errorCode) override;
    void OnOutputFormatChanged(const Format &format) override;
    void OnInputBufferAvailable(uint32_t index) override;
    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
    void ResetTrace();

private:
    int32_t InitServer();
    void ExitProcessor();
    const std::string &GetStatusDescription(OHOS::Media::CodecServer::CodecStatus status);

    CodecStatus status_ = CODEC_UNINITIALIZED;
    
    // std::unique_ptr<IAVCodecEngine> codecEngine_;
    std::unique_ptr<AVCodecBase> codecBase_;
    std::shared_ptr<AVCodecCallback> codecCb_;
    std::mutex mutex_;
    std::mutex cbMutex_;
    std::queue<uint32_t> inQueue_;
    std::queue<uint32_t> outQueue_;

    Format config_;
    std::string lastErrMsg_;
    int32_t firstFrameTraceId_ = 0;
    bool isFirstFrameIn_ = true;
    bool isFirstFrameOut_ = true;
};
} // namespace AVCodec
} // namespace OHOS
#endif // CODEC_SERVER_H