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

#ifndef AUDIO_FFMPEG_DECODER_PLUGIN
#define AUDIO_FFMPEG_DECODER_PLUGIN

#include "audio_ffmpeg_base_codec.h"
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
};
#endif

namespace OHOS {
namespace Media {

class AudioFfmpegDecoderPlugin {
private:
    int64_t preBufferGroupPts_{0};
    int64_t curBufferGroupPts_{0};
    int32_t bufferNum_{1};
    int32_t bufferIndex_{1};
    int64_t bufferGroupPtsDistance{0};
    std::shared_ptr<AVCodec> avCodec_{};
    std::shared_ptr<AVCodecContext> avCodecContext_{};
    std::shared_ptr<AVFrame> cachedFrame_{};
    std::shared_ptr<AVPacket> avPacket_{};
    mutable std::mutex avMutext_{};
    std::mutex parameterMutex_{};
    Format format_;

public:
    AudioFfmpegDecoderPlugin();

    ~AudioFfmpegDecoderPlugin();

    int32_t ProcessSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer);

    int32_t ProcessRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer);

    int32_t Reset();

    int32_t Release();

    int32_t Flush();

    int32_t AllocateContext(const std::string &name);

    int32_t InitContext(const Format &format);

    int32_t OpenContext();

    Format GetFormat() const noexcept;

    std::shared_ptr<AVCodecContext> GetCodecContext() const noexcept;

    std::shared_ptr<AVPacket> GetCodecAVPacket() const noexcept;

    std::shared_ptr<AVFrame> GetCodecCacheFrame() const noexcept;

    int32_t CloseCtxLocked();

private:
    int32_t SendBuffer(const std::shared_ptr<AudioBufferInfo> &inputBuffer);
    int32_t ReceiveBuffer(std::shared_ptr<AudioBufferInfo> &outBuffer);
    int32_t ReceiveFrameSucc(std::shared_ptr<AudioBufferInfo> &outBuffer);
};

} // namespace Media
} // namespace OHOS

#endif