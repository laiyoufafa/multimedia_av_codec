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

#ifndef FFMPEG_DEMUXER_PLUGIN_H
#define FFMPEG_DEMUXER_PLUGIN_H

#include <memory>
#include <vector>
#include "avcodec_common.h"
#include "demuxer_plugin.h"
#include "plugin/osal/thread/mutex.h"
#include "source.h"
#include "libavformat/avformat.h"

namespace OH{
namespace Media{
namespace Plugin{
namespace FFmpeg{
class FFmpegDemuxerPlugin : public DemuxerPlugin{
public:
    FFmpegDemuxerPlugin(size_t sourceAttr);
    ~FFmpegDemuxerPlugin() override;

    Status AddSourceTrackByID(uint32_t index) override;
    Status RemoveSourceTrackByID(uint32_t index) override;
    Status CopyCurrentSampleToBuf(AVCodecBufferElement *buffer, AVCodecBufferInfo *bufferInfo) override;
    Status SeekToTimeStamp(int64_t mSeconds, SeekMode mode) override;
private:
    vector<int32_t> selectedTrackIds_;
    OSAL::Mutex mutex_ {};
    std::shared_ptr<AVFormatContext> formatContext_;
};
} // namespace FFmpeg
} // namespace Plugin
} // namespace Media
} // namespace OH
#endif // FFMPEG_DEMUXER_PLUGIN_H