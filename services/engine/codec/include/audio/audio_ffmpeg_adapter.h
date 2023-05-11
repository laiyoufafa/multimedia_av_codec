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

#ifndef CODEC_EENGIN_AUDIO_FFMPEG_ADAPTER_H
#define CODEC_EENGIN_AUDIO_FFMPEG_ADAPTER_H

#include "audio_codec_worker.h"
#include "audio_ffmpeg_base_codec.h"
#include "avcodec_common.h"
#include "codecbase.h"
#include "nocopyable.h"

namespace OHOS {
namespace Media {

class AudioFFMpegAdapter : public CodecBase, public NoCopyable {
private:
    std::atomic<CodecState> state_;
    const std::string name_;
    std::shared_ptr<AVCodecCallback> callback_;
    std::shared_ptr<IAudioFFMpegBaseCodec> audioCodec;
    std::shared_ptr<AudioCodecWorker> worker_;

public:
    explicit AudioFFMpegAdapter(const std::string &name);

    ~AudioFFMpegAdapter() override;

    int32_t SetCallback(const std::shared_ptr<AVCodecCallback> &callback) override;

    int32_t Configure(const Format &format) override;

    int32_t Start() override;

    int32_t Stop() override;

    int32_t Flush() override;

    int32_t Reset() override;

    int32_t Release() override;

    int32_t Pause() override;

    int32_t Resume() override;

    int32_t NotifyEos() override;

    int32_t SetParameter(const Format &format) override;

    int32_t GetOutputFormat(Format &format) override;

    std::shared_ptr<AVSharedMemoryBase> GetInputBuffer(size_t index) override;

    int32_t QueueInputBuffer(size_t index, const AVCodecBufferInfo &info, AVCodecBufferFlag &flag) override;

    std::shared_ptr<AVSharedMemoryBase> GetOutputBuffer(size_t index) override;

    int32_t ReleaseOutputBuffer(size_t index) override;

private:
    int32_t doFlush();
    int32_t doStart();
    int32_t doStop();
    int32_t doResume();
    int32_t doRelease();
    int32_t doInit();
    int32_t doConfigure(const Format &format);
    std::string_view stateToString(CodecState state);
};

} // namespace Media
} // namespace OHOS
#endif