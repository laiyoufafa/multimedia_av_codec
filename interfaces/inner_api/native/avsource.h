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

#ifndef AVSOURCE_H
#define AVSOURCE_H

#include <stdint>
#include <memory>
#include "native_avcodec_base.h"
#include "libavformat/avformat.h"

namespace OHOS {
namespace AVCodec{

class __attribute__((visibility("default"))) AVSource {
public:
    /**
     * @brief Instantiate the source of the given uri.
     * 
     * @param uri The source model for demuxer.
     * @return Returns the designated demuxer.
     * @since 4.0
     * @version 4.0
     */
    static Source(const std::string &uri);
    virtual ~Source() = default;

    /**
     * @brief Count number of the track in source.
     * 
     * @return Returns the tracks's count.
     * @since 4.0
     * @version 4.0
     */
    virtual uint32_t GetTrackCount() = 0;

    /**
     * @brief Return a {@link AVSourceTrack} object.
     * 
     * @param trackId The index of the track.
     * @return Returns {@link AVSourceTrack} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual std::shared_ptr<SourceTrack> LoadSourceTrackByID(uint32_t trackId) = 0;

private:
    std::shared_ptr<AVFormatContext> formatContext_ = nullptr;

};

class __attribute__((visibility("default"))) AVSourceTrack {
public:
    /**
     * @brief Instantiate the sourceTrack of the given trackId from formatContext.
     * 
     * @param formatContext The ffmpeg source model.
     * @param trackId The index of the track.
     * @since 4.0
     * @version 4.0
     */
    SourceTrack(const AVFormatContext &formatContext, uint32_t trackId);
    virtual ~SourceTrack() = default;

    /**
     * @brief Sets the parameters to the track.
     * 
     * @param format The parameters.
     * @return Returns {@link MSEER_OK} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual uint32_t SetParameter(const Format &param) = 0;

    /**
     * @brief Gets the parameters of the track.
     * 
     * @return Returns {@link Format} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual std::shared_ptr<Format> GetTrackFormat() = 0;
private:
    uint32_t trackId_;
    OH_MediaType trackType_;
    std::unique_ptr<Format> trackFormat_;
    std::shared_ptr<AVStream> stream_;
};
} // namespace AVCodec
} // namespace OHOS
#endif // AVSOURCE_H

