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

#include <vector>
#include <memory>
#include <string>
#include "avcodec_common.h"
#include "format.h"

namespace OHOS {
namespace Media {
class AVSourceTrack;

class AVSource {
public:
    virtual ~AVSource() = default;

    /**
     * @brief Count number of the track in source.
     * @return Returns the tracks's count.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t GetTrackCount(uint32_t &trackCount) = 0;

    /**
     * @brief Return a {@link AVSourceTrack} object.
     * @param trackIndex The index of the track.
     * @return Returns {@link AVSourceTrack} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual std::shared_ptr<AVSourceTrack> GetSourceTrackByID(uint32_t trackIndex) = 0;

    /**
     * @brief Gets the parameters of the source.
     * @param format The empty parameters for storing data.
     * @return Returns {@link Format} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t GetSourceFormat(Format &format) = 0;

    /**
     * @brief Gets the address of the source.
     * @return Returns {@link Format} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t GetSourceAddr(uintptr_t &addr) = 0;

    std::string sourceUri;

private:
    std::vector<AVSourceTrack*> tracks_ {};
};

class __attribute__((visibility("default"))) AVSourceFactory {
public:
#ifdef UNSUPPORT_SOURCE
    static std::shared_ptr<AVSource> CreateWithURI(const std::string &uri)
    {
        (void)uri;
        return nullptr;
    }

    static std::shared_ptr<AVSource> CreateWithFD(int32_t fd, int64_t offset, int64_t size)
    {
        (void)uri;
        return nullptr;
    }

#else
    /**
     * @brief Instantiate the preferred source of the uri.
     * @param uri The file's uri.
     * @return Returns the preferred source.
     * @since 4.0
     * @version 4.0
     */
    static std::shared_ptr<AVSource> CreateWithURI(const std::string &uri);

    /**
     * @brief Instantiate the preferred source of the fd.
     * @param fd The fileDescriptor data source.
     * @param offset The offset into the file where the data be read starts.
     * @param size the length in bytes of the data to be read.
     * @return Returns the preferred source.
     * @since 4.0
     * @version 4.0
     */
    static std::shared_ptr<AVSource> CreateWithFD(int32_t fd, int64_t offset, int64_t size);

#endif
private:
    AVSourceFactory() = default;
    ~AVSourceFactory() = default;
};

class AVSourceTrack {
public:
    virtual ~AVSourceTrack() = default;

    /**
     * @brief Sets the parameters to the track.
     * @param format The parameters to set.
     * @return Returns {@link MSEER_OK} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t SetTrackFormat(const Format &format) = 0;

    /**
     * @brief Gets the parameters of the track.
     * @param format The empty parameters for storing data.
     * @return Returns {@link Format} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t GetTrackFormat(Format &format) = 0;
};
} // namespace Media
} // namespace OHOS
#endif // AVSOURCE_H

