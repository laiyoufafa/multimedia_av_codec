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
#ifndef AVCODEC_COMMOM_H
#define AVCODEC_COMMOM_H

#include <string>
#include "format.h"
#include "avsharedmemory.h"

namespace OHOS {
namespace AVCodec {
/**
 * @brief Error type of AVCodec
 *
 * @since 4.0
 * @version 4.0
 */
enum AVCodecErrorType : int32_t {
    /* internal errors, error code passed by the errorCode, and definition see "MediaServiceErrCode" */
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
    /* The flag of the available buffer. For details, see {@link AVCodecBufferFlag} */
    uint32_t flags = 0;
    /* The index of the track which this Buffer belongs. */
    uint32_t trackId;
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
     * @since 4.0
     * @version 4.0
     */
    virtual void OnError(AVCodecErrorType errorType, int32_t errorCode) = 0;

    /**
     * Called when the output format has changed.
     *
     * @param format The new output format.
     * @since 4.0
     * @version 4.0
     */
    virtual void OnOutputFormatChanged(const Format &format) = 0;

    /**
     * Called when an input buffer becomes available.
     *
     * @param index The index of the available input buffer.
     * @since 4.0
     * @version 4.0
     */
    virtual void OnInputBufferAvailable(uint32_t index) = 0;

    /**
     * Called when an output buffer becomes available.
     *
     * @param index The index of the available output buffer.
     * @param info The info of the available output buffer. For details, see {@link AVCodecBufferInfo}
     * @since 4.0
     * @version 4.0
     */
    virtual void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info) = 0;
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
} // namespace AVCodec
} // namespace OHOS
#endif // AVCODEC_COMMOM_H
