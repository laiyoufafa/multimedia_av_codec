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

#include <memory>
#include <string>
#include "avcodec_common.h"
#include "format.h"

namespace OHOS {
namespace Media{

class AVSource {
public:
    virtual ~AVSource() = default;

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
};

class __attribute__((visibility("default"))) SourceFactory {
#ifdef UNSUPPORT_SOURCE
    static std::shared_ptr<AVSource> CreateWithURI(const std::string &uri)
    {
        (void)uri;
        return nullptr;
    }

#else
    /**
     * @brief Instantiate the preferred source of the uri.
     *
     * @param uri The file's uri.
     * @return Returns the preferred source.
     * @since 4.0
     * @version 4.0
     */
    static std::shared_ptr<AVSource> CreateWithURI(const std::string &uri);

#endif
private:
    SourceFactory() = default;
    ~SourceFactory() = default;
};

class AVSourceTrack {
public:
    virtual ~AVSourceTrack() = default;

    /**
     * @brief Sets the parameters to the track.
     * 
     * @param format The parameters.
     * @return Returns {@link MSEER_OK} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t SetParameter(const Format &param) = 0;

    /**
     * @brief Gets the parameters of the track.
     * 
     * @return Returns {@link Format} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual std::shared_ptr<Format> GetTrackFormat(Format &format) = 0;
private:
    uint32_t trackId_;
};
} // namespace Media
} // namespace OHOS
#endif // AVSOURCE_H

