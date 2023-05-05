/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef AVCODEC_COMMOM_H
#define AVCODEC_COMMOM_H

#include <vector>
#include <string>
#include "av_common.h"
#include "format.h"
#include "avsharedmemory.h"

namespace OHOS {
namespace Media {
/**
 * @brief Error type of AVCodec
 *
 * @since 3.1
 * @version 3.1
 */
enum AVCodecErrorType : int32_t {
    /* internal errors, error code passed by the errorCode, and definition see "AVCodecServiceErrCode" */
    AVCODEC_ERROR_INTERNAL,
    /* extend error start. The extension error code agreed upon by the plug-in and
       the application will be transparently transmitted by the service. */
    AVCODEC_ERROR_EXTEND_START = 0X10000,
};

enum AVCodecBufferFlag : uint32_t {
    AVCODEC_BUFFER_FLAG_NONE = 0,
    /* This signals the end of stream */
    AVCODEC_BUFFER_FLAG_EOS = 1 << 0,
    /* This indicates that the buffer contains the data for a sync frame */
    AVCODEC_BUFFER_FLAG_SYNC_FRAME = 1 << 1,
    /* This indicates that the buffer only contains part of a frame */
    AVCODEC_BUFFER_FLAG_PARTIAL_FRAME = 1 << 2,
    /* This indicated that the buffer contains codec specific data */
    AVCODEC_BUFFER_FLAG_CODEC_DATA = 1 << 3,
};

struct AVCodecBufferInfo {
    /* The presentation timestamp in microseconds for the buffer */
    int64_t presentationTimeUs = 0;
    /* The amount of data (in bytes) in the buffer */
    int32_t size = 0;
    /* The start-offset of the data in the buffer */
    int32_t offset = 0;
    /* The flags this Buffer has, which is also a combination of multiple {@link OH_AVCodecBufferFlags}. */
    uint32_t flags = 0;
};

struct AVBufferElement {
    std::shared_ptr<AVSharedMemory> buffer;
    std::shared_ptr<AVSharedMemory> metaData;
};

class AVCodecCallback {
public:
    virtual ~AVCodecCallback() = default;
    /**
     * Called when an error occurred.
     *
     * @param errorType Error type. For details, see {@link AVCodecErrorType}.
     * @param errorCode Error code.
     * @since 3.1
     * @version 3.1
     */
    virtual void OnError(AVCodecErrorType errorType, int32_t errorCode) = 0;

    /**
     * Called when the output format has changed.
     *
     * @param format The new output format.
     * @since 3.1
     * @version 3.1
     */
    virtual void OnOutputFormatChanged(const Format &format) = 0;

    /**
     * Called when an input buffer becomes available.
     *
     * @param index The index of the available input buffer.
     * @since 3.1
     * @version 3.1
     */
    virtual void OnInputBufferAvailable(uint32_t index) = 0;

    /**
     * Called when an output buffer becomes available.
     *
     * @param index The index of the available output buffer.
     * @param info The info of the available output buffer. For details, see {@link AVCodecBufferInfo}
     * @param flag The flag of the available output buffer. For details, see {@link AVCodecBufferFlag}
     * @since 3.1
     * @version 3.1
     */
    virtual void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) = 0;
};

class SurfaceBufferExtratDataKey {
public:
    /**
     * Key for timeStamp in surface's extraData, value type is int64
     */
    static constexpr std::string_view ED_KEY_TIME_STAMP = "timeStamp";

    /**
     * Key for endOfStream in surface's extraData, value type is bool
     */
    static constexpr std::string_view ED_KEY_END_OF_STREAM = "endOfStream";

private:
    SurfaceBufferExtratDataKey() = delete;
    ~SurfaceBufferExtratDataKey() = delete;
};

class AVSourceFormat {
public:
    static constexpr std::string_view SOURCE_TITLE         = "title";            //< string
    static constexpr std::string_view SOURCE_ARTIST        = "artist";           //< std::string, artist
    static constexpr std::string_view SOURCE_ALBUM         = "album";            //< std::string, album
    static constexpr std::string_view SOURCE_ALBUM_ARTIST  = "album_artist";     //< std::string, album artist
    static constexpr std::string_view SOURCE_DATE          = "date";             //< std::string, media date, format：YYYY-MM-DD
    static constexpr std::string_view SOURCE_COMMENT       = "comment";          //< std::string, comment
    static constexpr std::string_view SOURCE_GENRE         = "genre";            //< std::string, genre
    static constexpr std::string_view SOURCE_COPYRIGHT     = "copyright";        //< std::string, copyright
    static constexpr std::string_view SOURCE_LANGUAGE      = "language";         //< std::string, language
    static constexpr std::string_view SOURCE_DESCRIPTION   = "description";      //< std::string, description
    static constexpr std::string_view SOURCE_LYRICS        = "lyrics";           //< std::string, cyrics
    static constexpr std::string_view SOURCE_DURATION      = "duration";         //< int64_t, duration based on {@link HST_TIME_BASE}
    static constexpr std::string_view SOURCE_TYPE          = "type";             //< std::string, sourece type
private:
    AVSourceFormat() = delete;
    ~AVSourceFormat() = delete;
};


class AVSourceTrackFormat {
public:
    static constexpr std::string_view TRACK_INDEX               = "track_index";
    static constexpr std::string_view TRACK_SAMPLE_COUNT        = "track_sample_count";
    static constexpr std::string_view TRACK_TYPE                = "track_type";
    static constexpr std::string_view TRACK_DURATION            = "duration";
    static constexpr std::string_view TRACK_BITRATE             = "bitrate";
    static constexpr std::string_view VIDEO_TRACK_ROTATION      = "rotation_angle";
    static constexpr std::string_view VIDEO_TRACK_WIDTH         = "width";
    static constexpr std::string_view VIDEO_TRACK_HEIGHT        = "height";
    static constexpr std::string_view VIDEO_PIXEL_FORMAT        = "pixel_format";
    static constexpr std::string_view VIDEO_BIT_STREAM_FORMAT   = "bit_stream_format";
private:
    AVSourceTrackFormat() = delete;
    ~AVSourceTrackFormat() = delete;
};

enum VideoBitStreamFormat {
    UNKNOWN = 0,
    AVCC,
    HVCC,
    ANNEXB
};

} // namespace Media
} // namespace OHOS
#endif // AVCODEC_COMMOM_H
