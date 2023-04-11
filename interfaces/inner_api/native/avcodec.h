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

#ifndef AVCODEC_H
#define AVCODEC_H

#include "avcodec_common.h"
#include "avcodec_info.h"
#include "avsharedmemory.h"
#include "format.h"
#include "surface.h"

namespace OHOS {
namespace Media {
using PersistentSurface = Surface; // not achieve PersistentSurface, this code just for compile.
class AVCodec {
public:
    virtual ~AVCodec() = default;

    /**
     * @brief Configure the codec.
     *
     * @param format The format of the input data and the desired format of the output data.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t Configure(const Format &format) = 0;

    /**
     * @brief Start codec.
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t Start() = 0;

    /**
     * @brief Stop codec.
     *
     * This function must be called during running
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t Stop() = 0;

    /**
     * @brief Flush both input and output buffers of the codec.
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t Flush() = 0;

    /**
     * @brief Notify eos of the encoder.
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t NotifyEos() = 0;

    /**
     * @brief Restores the codec to the initial state.
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t Reset() = 0;

    /**
     * @brief Releases codec resources. All methods are unavailable after calling this.
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t Release() = 0;

    /**
     * @brief Obtains the surface from codec.
     *
     * This function can only be called after {@link Configure} and before {@link Start}
     *
     * @return Returns the pointer to the surface.
     * @since 4.0
     * @version 4.0
     */
    virtual sptr<Surface> CreateInputSurface() = 0;

    /**
     * @brief Sets a persistent surface that can be used as the input to encoder
     *
     * This function must be called before {@link Start}
     *
     * @param index The index of the output buffer.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    // virtual int32_t SetInputSurface(sptr<PersistentSurface> surface) = 0;

    /**
     * @brief Sets the surface on which to render the output of this codec.
     *
     * This function must be called before {@link Start}
     *
     * @param index The index of the output buffer.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t SetOutputSurface(sptr<Surface> surface) = 0;

    /**
     * @brief Returns a {@link AVBufferElement} object for a input buffer index that contains the data.
     *
     * This function must be called during running
     *
     * @param index The index of the input buffer.
     * @return Returns {@link AVBufferElement} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual std::shared_ptr<AVBufferElement> GetInputBuffer(uint32_t index) = 0;

    /**
     * @brief Submits input buffer to codec.
     *
     * This function must be called during running
     *
     * @param index The index of the input buffer.
     * @param info The info of the input buffer. For details, see {@link AVCodecBufferInfo}
     * @param flag The flag of the input buffer. For details, see {@link AVCodecBufferFlag}
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) = 0;

    /**
     * @brief Get the index of the next ready input buffer.
     *
     * This function must be called during running
     *
     * @param index The index of the input buffer.
     * @param timeUs timeoutUs
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t DequeueInputBuffer(uint32_t *index, int64_t timetUs) = 0;

    /**
     * @brief Get the index of the next ready Output buffer.
     *
     * This function must be called during running
     *
     * @param index The index of the output buffer.
     * @param attr Pointer to an OH_AVCodecBufferAttr instance
     * @param timeUs timeoutUs
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t DequeueOutputBuffer(uint32_t *index, AVCodecBufferInfo *attr, int64_t timetUs) = 0;

    /**
     * @brief Returns a {@link AVBufferElement} object for a output buffer index that contains the data.
     *
     * This function must be called during running
     *
     * @param index The index of the output buffer.
     * @return Returns {@link AVBufferElement} if success; returns nullptr otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual std::shared_ptr<AVBufferElement> GetOutputBuffer(uint32_t index) = 0;

    /**
     * @brief Gets the format of the output data.
     *
     * This function must be called after {@link Configure}
     *
     * @param format
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t GetOutputFormat(Format &format) = 0;

    /**
     * @brief Returns the output buffer to the codec.
     *
     * This function must be called during running
     *
     * @param index The index of the output buffer.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t ReleaseOutputBuffer(uint32_t index, bool render) = 0;

    /**
     * @brief Sets the parameters to the codec.
     *
     * This function must be called after {@link Configure}
     *
     * @param format The parameters.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t SetParameter(const Format &format) = 0;

    /**
     * @brief Registers a codec listener.
     *
     * This function must be called before {@link Configure}
     *
     * @param callback Indicates the codec listener to register. For details, see {@link AVCodecCallback}.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t SetCallback(const std::shared_ptr<AVCodecCallback> &callback) = 0;

};

class __attribute__((visibility("default"))) CodecFactory {
public:
#ifdef UNSUPPORT_CODEC
    static std::shared_ptr<AVCodec> CreateByMime(const std::string &mime, bool encoder)
    {
        (void)mime;
        return nullptr;
    }

    static std::shared_ptr<AVCodec> CreateByName(const std::string &name)
    {
        (void)name;
        return nullptr;
    }
#else
    /**
     * @brief Instantiate the preferred codec of the given mime type.
     *
     * @param mime The mime type.
     * @param encoder true for encoder and false for decoder.
     * @return Returns the preferred codec.
     * @since 4.0
     * @version 4.0
     */
    static std::shared_ptr<AVCodec> CreateByMime(const std::string &mime, bool encoder);

    /**
     * @brief Instantiates the designated codec.
     *
     * @param name The codec's name.
     * @return Returns the designated codec.
     * @since 4.0
     * @version 4.0
     */
    static std::shared_ptr<AVCodec> CreateByName(const std::string &name);
#endif
private:
    CodecFactory() = default;
    ~CodecFactory() = default;
};
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_H