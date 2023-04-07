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

#ifndef NATIVE_AVCODEC_CODEC_H
#define NATIVE_AVCODEC_CODEC_H

#include <stdint.h>
#include <stdio.h>
#include "native_averrors.h"
#include "native_avformat.h"
#include "native_avcodec_base.h"
#include "native_avcapablility.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Find the supported video encoder name by format(must contains video MIME).
 * @param format Indicates a media description which contains required video encoder capability.
 * @return  Returns video encoder name, if not find, return null.
 * @since 10
 * @version 1.0
 */
char *OH_AVCodec_FindVideoDecoder(const OH_AVFormat *format);

/**
 * @brief Find the supported video decoder name by format(must contains video MIME).
 * @param format Indicates a media description which contains required video decoder capability.
 * @return  Returns video decoder name, if not find, return null.
 * @since 10
 * @version 1.0
 */
char *OH_AVCodec_FindVideoDecoder(const OH_AVFormat *format);

/**
 * @brief Find the supported audio encoder name by format(must contains audio MIME).
 * @param format Indicates a media description which contains required audio encoder capability.
 * @return  Returns audio encoder name, if not find, return null.
 * @since 10
 * @version 1.0
 */
char *OH_AVCodec_FindAudioEncoder(const OH_AVFormat *format);

/**
 * @brief Find the supported audio decoder name by format(must contains audio MIME).
 * @param format Indicates a media description which contains required audio decoder capability.
 * @return  Returns audio decoder name, if not find, return empty string.
 * @since 10
 * @version 1.0
 */
char *OH_AVCodec_FindAudioDecoder(const OH_AVFormat *format);

/**
 * @brief Get the capabilities by codec name
 * @param codeName Codec name
 * @return Returns an array of supported video decoder capability, if not find, return null.
 * @since 10
 * @version 1.0
 */
OH_AVCapability *OH_AVCodec_GetCapability(const char *name);

/**
 * @brief Creates encoder by mime type, which is recommended in most cases.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param mime mime type description string
 * @return Returns a Pointer to an OH_AVCodec instance
 * @since 10
 * @version 4.0
 */
OH_AVCodec *OH_AVCodec_CreateEncoderByMime(const char *mime);

/**
 * @brief Creates decoder by mime type, which is recommended in most cases.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param mime mime type description string
 * @return Returns a Pointer to an OH_AVCodec instance
 * @since 10
 * @version 4.0
 */
OH_AVCodec *OH_AVCodec_CreateDecoderByMime(const char *mime);

/**
 * @brief Creates codec by name.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param name name description string
 * @return Returns a Pointer to an OH_AVCodec instance
 * @since 10
 * @version 4.0
 */
OH_AVCodec *OH_AVCodec_CreateByName(const char *name);

/**
 * @brief destroy the codec and free its resources.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_Destroy(OH_AVCodec *codec);

/**
 * @brief Set the asynchronous callback function so that your application can respond to the events
 * generated by the codec. This interface must be called before Configure is called.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param callback A collection of all callback functions, see {@link OH_AVCodecCallback}
 * @param userData User specific data
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_SetCallback(OH_AVCodec *codec, OH_AVCodecCallback callback, void *userData);

/**
 * @brief To configure the codec, typically, you would get the fotmat from an extractor for decoding.
 * This interface must be called before Start is called.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param format A pointer to an OH_AVFormat to give the description of the video track to be decoded
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_Configure(OH_AVCodec *codec, OH_AVFormat *format);

/**
 * @brief Start the codec, this interface must be called after the OH_AVCodec_Configure is successful.
 * After being successfully started, the codec will start reporting InputDataReady events.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_Start(OH_AVCodec *codec);

/**
 * @brief Stop the codec. After stopping, you can re-enter the Started state through Start,
 * but it should be noted that if Codec-Specific-Data has been input to the decoder before, it needs to be input again.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_Stop(OH_AVCodec *codec);

/**
 * @brief Clear the input and output data buffered in the codec. After this interface is called, all the Buffer
 * indexes previously reported through the asynchronous callback will be invalidated, make sure not to access
 * the Buffers corresponding to these indexes.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_Flush(OH_AVCodec *codec);

/**
 * @brief Reset the codec. To continue decoding, you need to call the Configure interface again
 * to configure the codec instance.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_Reset(OH_AVCodec *codec);

/**
 * @brief Get the format information of the output data of the codec, refer to {@link OH_AVFormat}
 * It should be noted that the life cycle of the OH_AVFormat instance pointed to by the return value * needs
 * to be manually released by the caller.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @return Returns a pointer to an OH_AVFormat instance
 * @since 10
 * @version 4.0
 */
OH_AVFormat *OH_AVCodec_GetOutputFormat(OH_AVCodec *codec);

/**
 * @brief Set dynamic parameters to the decoder. Note: This interface can only be called after the codec is started.
 * At the same time, incorrect parameter settings may cause codec failure.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param format pointer to an OH_AVFormat instance
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_SetParameter(OH_AVCodec *codec, OH_AVFormat *format);

/**
 * @brief Get the index of the next ready input buffer. 
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param index Pointer to an uint32_t instance
 * @param timeoutUs timeoutUs
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns negtive value for invalid buffer index
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_DequeueInputBuffer(OH_AVCodec *codec, uint32_t *index, int64_t timeoutUs);

/**
 * @brief Get an input buffer. 
 * The buffer index must be odbtained from OH_AVCodec_DequeueInputBuffer, and not queued yet.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param index The index value corresponding to the input Buffer
 * @return Returns a pointer to an BufferElement instance
 * @since 10
 * @version 4.0
 */
OH_AVBufferElement* OH_AVCodec_GetInputBuffer(OH_AVCodec *codec, uint32_t index);

/**
 * @brief Submit the input buffer filled with data to the codec. The {@link OH_AVCodecOnInputDataReady} callback
 * will report the available input buffer and the corresponding index value. Once the buffer with the specified index
 * is submitted to the codec, the buffer cannot be accessed again until the {@link OH_AVCodecOnInputDataReady}
 * callback is received again reporting that the buffer with the same index is available. In addition, for some
 * decoders, it is required to input Codec-Specific-Data to the decoder at the beginning to initialize the decoding
 * process of the decoder, such as PPS/SPS data in H264 format.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param index Enter the index value corresponding to the Buffer
 * @param attr Information describing the data contained in the Buffer
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_QueueInputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVCodecBufferAttr attr);

/**
 * @brief Get the index of the next ready output buffer of processed data. 
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param index Pointer to an uint32_t instance
 * @param attr Pointer to an OH_AVCodecBufferAttr instance
 * @param timeoutUs timeoutUs
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns negtive value for invalid buffer index
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_DequeueOutputBuffer(OH_AVCodec *codec, uint32_t *index, OH_AVCodecBufferAttr *attr, int64_t timeoutUs);

/**
 * @brief Get an output buffer. 
 * The buffer index must be odbtained from OH_AVCodec_DequeueOutputBuffer.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param index The index value corresponding to the output Buffer
 * @return Returns a pointer to an BufferElement instance
 * @since 10
 * @version 4.0
 */
OH_AVBufferElement* OH_AVCodec_GetOutputBuffer(OH_AVCodec *codec, uint32_t index);

/**
 * @brief Return the processed output Buffer to the codec.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param index The index value corresponding to the output Buffer
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_ReleaseOutputData(OH_AVCodec *codec, uint32_t index);

/**
 * @brief Get the input Surface from the video encoder, this interface must be called before Configure is called.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param window A pointer to a OHNativeWindow instance, see {@link OHNativeWindow}
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_VideoEncoderGetSurface(OH_AVCodec *codec, OHNativeWindow **window);

/**
 * @brief Create a persistent surface that can be used as the input to encoder.
 * Persistent surface can also be set on a new instance via OH_AVCodec_VideoEncoderSetSurface().
 * A persistent surface can be connected to at most one instance of AVCodec.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param window A pointer to a OHNativeWindow instance, see {@link OHNativeWindow}
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_VideoEncoderGetPersistentSurface(OHNativeWindow **window);

/**
 * @brief Set a persistent surface that can be used as the input to encoder, inplaces of input buffers.
 * this can only be called after OH_AVCodec_Configure()
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param window A pointer to a OHNativeWindow instance, see {@link OHNativeWindow}, 
 * which must be a presistent surface created by OH_AVCodec_VideoEncoderGetPersistentSurface()
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_VideoEncoderSetSurface(OH_AVCodec *codec, OHNativeWindow *window);

/**
 * @brief Specify the output Surface to provide video decoding output,
 * this interface must be called before Configure is called
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param window A pointer to a OHNativeWindow instance, see {@link OHNativeWindow}
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_VideoDecoderSetSurface(OH_AVCodec *codec, OHNativeWindow *window);

/**
 * @brief Return the processed output Buffer to the decoder, and notify the decoder to finish rendering the
 * decoded data contained in the Buffer on the output Surface. If the output surface is not configured before,
 * calling this interface only returns the output buffer corresponding to the specified index to the decoder.
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @param index The index value corresponding to the output Buffer
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_VideoDecoderRenderFrame(OH_AVCodec *codec, uint32_t index);

/**
 * @brief Notifies the video encoder that the input stream has ended. It is recommended to use this interface to notify
 * the encoder of the end of the stream in surface mode
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
OH_AVCodecErrCode OH_AVCodec_VideoEncoderNotifyEndOfStream(OH_AVCodec *codec);

/**
 * @brief Is used to check whether the current codec instance is valid. It can be used fault recovery or app 
 * switchback from the background
 * @syscap SystemCapability.Multimedia.AVCodec.Codec
 * @param codec Pointer to an OH_AVCodec instance
 * @return Returns AVCODEC_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVCodecErrCode}
 * @since 10
 * @version 4.0
 */
bool OH_AVCodec_IsValid(OH_AVCodec *codec);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVCODEC_CODEC_H