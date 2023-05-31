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

#ifndef AVCODEC_VIDEO_DECODER_INNER_DEMO_H
#define AVCODEC_VIDEO_DECODER_INNER_DEMO_H

#include <atomic>
#include <fstream>
#include <thread>
#include <queue>
#include <string>
#include "surface/window.h"
#include "avcodec_common.h"
#include "avcodec_video_decoder.h"
#include "nocopyable.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace Media {
namespace InnerVideoDemo {
class TestConsumerListener : public IBufferConsumerListener {
public:
    TestConsumerListener(sptr<Surface> cs, std::string_view name);
    ~TestConsumerListener();
    void OnBufferAvailable() override;

private:
    int64_t timestamp_ = 0;
    OHOS::Rect damage_ = {};
    sptr<Surface> cs_ = nullptr;
    std::unique_ptr<std::ofstream> outFile_;
};

class VDecSignal {
public:
    std::mutex inMutex_;
    std::mutex outMutex_;
    std::condition_variable inCond_;
    std::condition_variable outCond_;
    std::queue<uint32_t> inQueue_;
    std::queue<uint32_t> outQueue_;
    std::queue<AVCodecBufferInfo> infoQueue_;
    std::queue<AVCodecBufferFlag> flagQueue_;
};

class VDecDemoCallback : public AVCodecCallback, public NoCopyable {
public:
    explicit VDecDemoCallback(std::shared_ptr<VDecSignal> signal);
    virtual ~VDecDemoCallback() = default;

    void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    void OnOutputFormatChanged(const Format &format) override;
    void OnInputBufferAvailable(uint32_t index) override;
    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;

private:
    std::shared_ptr<VDecSignal> signal_;
};

class VDecInnerDemo : public NoCopyable {
public:
    VDecInnerDemo();
    virtual ~VDecInnerDemo();
    void RunCase(std::string &mode);

private:
    int32_t CreateDec();
    int32_t Configure(const Format &format);
    int32_t SetOutputSurface(sptr<Surface> surface);
    sptr<Surface> GetSurface(std::string &mode);
    int32_t Start();
    int32_t Stop();
    int32_t Flush();
    int32_t Reset();
    int32_t Release();
    void InputFunc();
    void OutputFunc();
    void HandleInputEOS(const uint32_t &index);
    int32_t HandleNormalInput(const uint32_t &index, const int64_t &pts, const size_t &size);
    int32_t ExtractPacket();

    std::atomic<bool> isRunning_ = false;
    std::unique_ptr<std::ifstream> inputFile_;
    std::unique_ptr<std::ofstream> outFile_;
    std::unique_ptr<std::thread> inputLoop_;
    std::unique_ptr<std::thread> outputLoop_;
    std::shared_ptr<AVCodecVideoDecoder> videoDec_;
    std::shared_ptr<VDecSignal> signal_;
    std::shared_ptr<VDecDemoCallback> cb_;

    // Extract packet
    static constexpr int32_t VIDEO_INBUF_SIZE = 10240;
    static constexpr int32_t VIDEO_REFILL_THRESH = 4096;
    const AVCodec *codec_ = nullptr;
    AVCodecParserContext *parser_ = nullptr;
    AVCodecContext *codec_ctx_ = nullptr;
    AVPacket *pkt_ = nullptr;
    size_t data_size_ = 0;
    uint8_t *data_ = nullptr;
    uint8_t inbuf_[VIDEO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    bool file_end_ = false;

    std::string mode_ = "0";
};
} // namespace InnerVideoDemo
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_VIDEO_DECODER_INNER_DEMO_H
