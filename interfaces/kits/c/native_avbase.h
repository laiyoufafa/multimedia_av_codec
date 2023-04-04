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

#ifndef NATIVE_AVBASE_H
#define NATIVE_AVBASE_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* mime type */
static const char *OH_AV_MIME_AUDIO_MPEG = "audio/mpeg";
static const char *OH_AV_MIME_AUDIO_FLAC = "audio/flac";
static const char *OH_AV_MIME_AUDIO_RAW = "audio/raw";
static const char *OH_AV_MIME_AUDIO_APE = "audio/ape";
static const char *OH_AV_MIME_AUDIO_WAV = "audio/wav";
static const char *OH_AV_MIME_AUDIO_AAC = "audio/mp4a-latm";
static const char *OH_AV_MIME_AUDIO_AAC_LATM = "audio/aac-latm";
static const char *OH_AV_MIME_AUDIO_VORBIS = "audio/vorbis";
static const char *OH_AV_MIME_AUDIO_OPUS = "audio/opus";
static const char *OH_AV_MIME_AUDIO_AC3 = "audio/ac3";
static const char *OH_AV_MIME_AUDIO_EAC3 = "audio/eac3";
static const char *OH_AV_MIME_AUDIO_WMA = "audio/x-ms-wma";
static const char *OH_AV_MIME_AUDIO_AMR_NB = "audio/3gpp";
static const char *OH_AV_MIME_AUDIO_AMR_WB = "audio/amr-wb";

static const char *OH_AV_MIME_VIDEO_RAW = "video/raw";
static const char *OH_AV_MIME_VIDEO_H264 = "video/avc";
static const char *OH_AV_MIME_VIDEO_MPEG4 = "video/mp4v-es";

/* format keys */
static const char *OH_AV_KEY_MIME = "mime"; ///< mime type
static const char *OH_AV_KEY_BIT_RATE = "bitrate"; ///< int64_t
static const char *OH_AV_KEY_CODEC_CONFIG = "codec-config"; ///< uint8_t* codec specific data buffer

/* audio format keys */
static const char *OH_AV_KEY_AUDIO_SAMPLE_FORMAT = "audio-sample-format"; ///< @OH_AudioSampleFormat int32_t
static const char *OH_AV_KEY_AUDIO_CHANNELS = "audio-channel-count"; ///< int32_t
static const char *OH_AV_KEY_AUDIO_SAMPLE_RATE = "audio-sample-rate"; ///< int32_t
static const char *OH_AV_KEY_AUDIO_CHANNEL_MASK = "audio-channel-mask"; ///< @OH_AudioChannelMask uint64_t
static const char *OH_AV_KEY_AUDIO_SAMPLE_PER_FRAME = "audio-frame-size"; ///< int32_t
static const char *OH_AV_KEY_AUDIO_AAC_PROFILE = "audio-aac-profile"; ///< int32_t

/* video format keys */
static const char *OH_AV_KEY_VIDEO_PIXEL_FORMAT = "video-pixel-format"; ///< OH_VideoPixelFormat int32_t
static const char *OH_AV_KEY_VIDEO_WIDTH = "video-width"; ///< int32_t
static const char *OH_AV_KEY_VIDEO_HEIGHT = "video-height"; ///< int32_t
static const char *OH_AV_KEY_VIDEO_FRAME_RATE = "video-frame-rate"; ///< int32_t
static const char *OH_AV_KEY_VIDEO_H264_PROFILE = "video-h264-profile"; ///< int32_t
static const char *OH_AV_KEY_VIDEO_H264_LEVEL = "video-h264-level"; ///< int32_t

/* metadata format keys */
static const char *OH_AV_KEY_TITLE = "metadata-title"; ///< const char *
static const char *OH_AV_KEY_ARTIST = "metadata-artist"; ///< const char *
static const char *OH_AV_KEY_LYRICIST = "metadata-lyricist"; ///< const char *
static const char *OH_AV_KEY_ALBUM = "metadata-album"; ///< const char *
static const char *OH_AV_KEY_ALBUM_ARTIST = "metadata-album-artist"; ///< const char *
static const char *OH_AV_KEY_DATE = "metadata-date"; ///< const char *
static const char *OH_AV_KEY_COMMENT = "metadata-comment"; ///< const char *
static const char *OH_AV_KEY_GENRE = "metadata-genre"; ///< const char *
static const char *OH_AV_KEY_COPYRIGHT = "metadata-copyright"; ///< const char *
static const char *OH_AV_KEY_LANGUAGE = "metadata-language"; ///< const char *
static const char *OH_AV_KEY_DESCRIPTION = "metadata-description"; ///< const char *
static const char *OH_AV_KEY_LYRICS = "metadata-lyrics"; ///< const char *
static const char *OH_AV_KEY_AUTHOR = "metadata-author"; ///< const char *
static const char *OH_AV_KEY_COMPOSER = "metadata-composer"; ///< const char *
static const char *OH_AV_KEY_MAKE = "metadata-make"; ///< const char *
static const char *OH_AV_KEY_MODEL = "metadata-model"; ///< const char *


/**
 * @enum Audio sample formats
 *
 * 'S' is signed, 'U' is unsigned, and 'F' is a floating-point number.
 * 'P' is planes， default is interleaved.
 *
 * @since 10
 * @version 1.0
 */
typedef enum OH_AudioSampleFormat {
    A_SAMPLE_FMT_NONE = 0,
    A_SAMPLE_FMT_U8,       ///< unsigned 8 bits
    A_SAMPLE_FMT_U8P,      ///< unsigned 8 bits, planar
    A_SAMPLE_FMT_S16,      ///< signed 16 bits
    A_SAMPLE_FMT_S16P,     ///< signed 16 bits, planar
    A_SAMPLE_FMT_S32,      ///< signed 32 bits
    A_SAMPLE_FMT_S32P,     ///< signed 32 bits, planar
    A_SAMPLE_FMT_S64,      ///< signed 64 bits
    A_SAMPLE_FMT_S64P,     ///< signed 64 bits, planar
    A_SAMPLE_FMT_F32,      ///< float
    A_SAMPLE_FMT_F32P,     ///< signed 32 bits, planar
    A_SAMPLE_FMT_F64,      ///< double
    A_SAMPLE_FMT_F64P,     ///< signed 64 bits, planar
} OH_AudioSampleFormat;

/**
 * @enum Audio Channel Set
 *
 * A 64-bit integer with bits set for each channel.
 *
 * @since 10
 * @version 1.0
 */
typedef enum OH_AudioChannelSet : uint64_t {
    A_CH_SET_FRONT_LEFT = 1ULL << 0U,
    A_CH_SET_FRONT_RIGHT = 1ULL << 1U,
    A_CH_SET_FRONT_CENTER = 1ULL << 2U,
    A_CH_SET_LOW_FREQUENCY = 1ULL << 3U,
    A_CH_SET_BACK_LEFT = 1ULL << 4U,
    A_CH_SET_BACK_RIGHT = 1ULL << 5U,
    A_CH_SET_FRONT_LEFT_OF_CENTER = 1ULL << 6U,
    A_CH_SET_FRONT_RIGHT_OF_CENTER = 1ULL << 7U,
    A_CH_SET_BACK_CENTER = 1ULL << 8U,
    A_CH_SET_SIDE_LEFT = 1ULL << 9U,
    A_CH_SET_SIDE_RIGHT = 1ULL << 10U,
    A_CH_SET_TOP_CENTER = 1ULL << 11U,
    A_CH_SET_TOP_FRONT_LEFT = 1ULL << 12U,
    A_CH_SET_TOP_FRONT_CENTER = 1ULL << 13U,
    A_CH_SET_TOP_FRONT_RIGHT = 1ULL << 14U,
    A_CH_SET_TOP_BACK_LEFT = 1ULL << 15U,
    A_CH_SET_TOP_BACK_CENTER = 1ULL << 16U,
    A_CH_SET_TOP_BACK_RIGHT = 1ULL << 17U,
    A_CH_SET_STEREO_LEFT = 1ULL << 29U,
    A_CH_SET_STEREO_RIGHT = 1ULL << 30U,
    A_CH_SET_WIDE_LEFT = 1ULL << 31U,
    A_CH_SET_WIDE_RIGHT = 1ULL << 32U,
    A_CH_SET_SURROUND_DIRECT_LEFT = 1ULL << 33U,
    A_CH_SET_SURROUND_DIRECT_RIGHT = 1ULL << 34U,
    A_CH_SET_LOW_FREQUENCY_2 = 1ULL << 35U,
    A_CH_SET_TOP_SIDE_LEFT = 1ULL << 36U,
    A_CH_SET_TOP_SIDE_RIGHT = 1ULL << 37U,
    A_CH_SET_BOTTOM_FRONT_CENTER = 1ULL << 38U,
    A_CH_SET_BOTTOM_FRONT_LEFT = 1ULL << 39U,
    A_CH_SET_BOTTOM_FRONT_RIGHT = 1ULL << 40U,

    // Ambisonics ACN formats

    // 0th and first order ambisonics ACN
    A_CH_SET_AMBISONICS_ACN0 = 1ULL << 41U,  /** 0th ambisonics channel number 0. */
    A_CH_SET_AMBISONICS_ACN1 = 1ULL << 42U,  /** first-order ambisonics channel number 1. */
    A_CH_SET_AMBISONICS_ACN2 = 1ULL << 43U,  /** first-order ambisonics channel number 2. */
    A_CH_SET_AMBISONICS_ACN3 = 1ULL << 44U,  /** first-order ambisonics channel number 3. */
    A_CH_SET_AMBISONICS_W = A_CH_SET_AMBISONICS_ACN0, /** same as 0th ambisonics channel number 0. */
    A_CH_SET_AMBISONICS_Y = A_CH_SET_AMBISONICS_ACN1, /** same as first-order ambisonics channel number 1. */
    A_CH_SET_AMBISONICS_Z = A_CH_SET_AMBISONICS_ACN2, /** same as first-order ambisonics channel number 2. */
    A_CH_SET_AMBISONICS_X = A_CH_SET_AMBISONICS_ACN3, /** same as first-order ambisonics channel number 3. */

    // second order ambisonics ACN
    A_CH_SET_AMBISONICS_ACN4 = 1ULL << 45U, /** second-order ambisonics channel number 4. */
    A_CH_SET_AMBISONICS_ACN5 = 1ULL << 46U, /** second-order ambisonics channel number 5. */
    A_CH_SET_AMBISONICS_ACN6 = 1ULL << 47U, /** second-order ambisonics channel number 6. */
    A_CH_SET_AMBISONICS_ACN7 = 1ULL << 48U, /** second-order ambisonics channel number 7. */
    A_CH_SET_AMBISONICS_ACN8 = 1ULL << 49U, /** second-order ambisonics channel number 8. */

    // third order ambisonics ACN
    A_CH_SET_AMBISONICS_ACN9 = 1ULL << 50U,  /** third-order ambisonics channel number 9. */
    A_CH_SET_AMBISONICS_ACN10 = 1ULL << 51U, /** third-order ambisonics channel number 10. */
    A_CH_SET_AMBISONICS_ACN11 = 1ULL << 52U, /** third-order ambisonics channel number 11. */
    A_CH_SET_AMBISONICS_ACN12 = 1ULL << 53U, /** third-order ambisonics channel number 12. */
    A_CH_SET_AMBISONICS_ACN13 = 1ULL << 54U, /** third-order ambisonics channel number 13. */
    A_CH_SET_AMBISONICS_ACN14 = 1ULL << 55U, /** third-order ambisonics channel number 14. */
    A_CH_SET_AMBISONICS_ACN15 = 1ULL << 56U, /** third-order ambisonics channel number 15. */
} OH_AudioChannelSet;

/**
 * @enum Audio AudioChannel Mask
 *
 * Indicates that the channel order in which the user requests decoder output
 * is the native codec channel order.
 *
 * @since 10
 * @version 1.0
 */
typedef enum OH_AudioChannelMask : uint64_t {
    A_CH_MASK_UNKNOWN = 0,
    A_CH_MASK_MONO = (A_CH_SET_FRONT_CENTER),
    A_CH_MASK_STEREO = (A_CH_SET_FRONT_LEFT | A_CH_SET_FRONT_RIGHT),
    A_CH_MASK_CH_2POINT1 = (A_CH_MASK_STEREO | A_CH_SET_LOW_FREQUENCY),
    A_CH_MASK_CH_2_1 = (A_CH_MASK_STEREO | A_CH_SET_BACK_CENTER),
    A_CH_MASK_SURROUND = (A_CH_MASK_STEREO | A_CH_SET_FRONT_CENTER),
    A_CH_MASK_CH_3POINT1 = (A_CH_MASK_SURROUND | A_CH_SET_LOW_FREQUENCY),
    A_CH_MASK_CH_4POINT0 = (A_CH_MASK_SURROUND | A_CH_SET_BACK_CENTER),
    A_CH_MASK_CH_4POINT1 = (A_CH_MASK_CH_4POINT0 | A_CH_SET_LOW_FREQUENCY),
    A_CH_MASK_CH_2_2 = (A_CH_MASK_STEREO | A_CH_SET_SIDE_LEFT | A_CH_SET_SIDE_RIGHT),
    A_CH_MASK_QUAD = (A_CH_MASK_STEREO | A_CH_SET_BACK_LEFT | A_CH_SET_BACK_RIGHT),
    A_CH_MASK_CH_5POINT0 = (A_CH_MASK_SURROUND | A_CH_SET_SIDE_LEFT | A_CH_SET_SIDE_RIGHT),
    A_CH_MASK_CH_5POINT1 = (A_CH_MASK_CH_5POINT0 | A_CH_SET_LOW_FREQUENCY),
    A_CH_MASK_CH_5POINT0_BACK = (A_CH_MASK_SURROUND | A_CH_SET_BACK_LEFT | A_CH_SET_BACK_RIGHT),
    A_CH_MASK_CH_5POINT1_BACK = (A_CH_MASK_CH_5POINT0_BACK | A_CH_SET_LOW_FREQUENCY),
    A_CH_MASK_CH_6POINT0 = (A_CH_MASK_CH_5POINT0 | A_CH_SET_BACK_CENTER),
    A_CH_MASK_CH_6POINT0_FRONT = (A_CH_MASK_CH_2_2 | A_CH_SET_FRONT_LEFT_OF_CENTER | A_CH_SET_FRONT_RIGHT_OF_CENTER),
    A_CH_MASK_HEXAGONAL = (A_CH_MASK_CH_5POINT0_BACK | A_CH_SET_BACK_CENTER),
    A_CH_MASK_CH_6POINT1 = (A_CH_MASK_CH_5POINT1 | A_CH_SET_BACK_CENTER),
    A_CH_MASK_CH_6POINT1_BACK = (A_CH_MASK_CH_5POINT1_BACK | A_CH_SET_BACK_CENTER),
    A_CH_MASK_CH_6POINT1_FRONT = (A_CH_MASK_CH_6POINT0_FRONT | A_CH_SET_LOW_FREQUENCY),
    A_CH_MASK_CH_7POINT0 = (A_CH_MASK_CH_5POINT0 | A_CH_SET_BACK_LEFT | A_CH_SET_BACK_RIGHT),
    A_CH_MASK_CH_7POINT0_FRONT = (A_CH_MASK_CH_5POINT0 | A_CH_SET_FRONT_LEFT_OF_CENTER | A_CH_SET_FRONT_RIGHT_OF_CENTER),
    A_CH_MASK_CH_7POINT1 = (A_CH_MASK_CH_5POINT1 | A_CH_SET_BACK_LEFT | A_CH_SET_BACK_RIGHT),
    A_CH_MASK_CH_7POINT1_WIDE = (A_CH_MASK_CH_5POINT1 | A_CH_SET_FRONT_LEFT_OF_CENTER | A_CH_SET_FRONT_RIGHT_OF_CENTER),
    A_CH_MASK_CH_7POINT1_WIDE_BACK = (A_CH_MASK_CH_5POINT1_BACK | A_CH_SET_FRONT_LEFT_OF_CENTER | A_CH_SET_FRONT_RIGHT_OF_CENTER),
    A_CH_MASK_CH_3POINT1POINT2 = (A_CH_MASK_CH_3POINT1 | A_CH_SET_TOP_FRONT_LEFT | A_CH_SET_TOP_FRONT_RIGHT),
    A_CH_MASK_CH_5POINT1POINT2 = (A_CH_MASK_CH_5POINT1 | A_CH_SET_TOP_SIDE_LEFT | A_CH_SET_TOP_SIDE_RIGHT),
    A_CH_MASK_CH_5POINT1POINT4 = (A_CH_MASK_CH_5POINT1 | A_CH_SET_TOP_FRONT_LEFT | A_CH_SET_TOP_FRONT_RIGHT |
            A_CH_SET_TOP_BACK_LEFT | A_CH_SET_TOP_BACK_RIGHT),
    A_CH_MASK_CH_7POINT1POINT2 = (A_CH_MASK_CH_7POINT1 | A_CH_SET_TOP_SIDE_LEFT | A_CH_SET_TOP_SIDE_RIGHT),
    A_CH_MASK_CH_7POINT1POINT4 = (A_CH_MASK_CH_7POINT1 | A_CH_SET_TOP_FRONT_LEFT | A_CH_SET_TOP_FRONT_RIGHT |
            A_CH_SET_TOP_BACK_LEFT | A_CH_SET_TOP_BACK_RIGHT),
    A_CH_MASK_CH_9POINT1POINT4 = (A_CH_MASK_CH_7POINT1POINT4 | A_CH_SET_WIDE_LEFT | A_CH_SET_WIDE_RIGHT),
    A_CH_MASK_CH_9POINT1POINT6 = (A_CH_MASK_CH_9POINT1POINT4 | A_CH_SET_TOP_SIDE_LEFT | A_CH_SET_TOP_SIDE_RIGHT),
    A_CH_MASK_CH_10POINT2 = (A_CH_SET_FRONT_LEFT | A_CH_SET_FRONT_RIGHT | A_CH_SET_FRONT_CENTER |
            A_CH_SET_TOP_FRONT_LEFT | A_CH_SET_TOP_FRONT_RIGHT | A_CH_SET_BACK_LEFT |
            A_CH_SET_BACK_RIGHT | A_CH_SET_BACK_CENTER | A_CH_SET_SIDE_LEFT |
            A_CH_SET_SIDE_RIGHT | A_CH_SET_WIDE_LEFT | A_CH_SET_WIDE_RIGHT),
    A_CH_MASK_CH_22POINT2 = (A_CH_MASK_CH_7POINT1POINT4 | A_CH_SET_FRONT_LEFT_OF_CENTER | A_CH_SET_FRONT_RIGHT_OF_CENTER |
            A_CH_SET_BACK_CENTER | A_CH_SET_TOP_CENTER | A_CH_SET_TOP_FRONT_CENTER |
            A_CH_SET_TOP_BACK_CENTER | A_CH_SET_TOP_SIDE_LEFT | A_CH_SET_TOP_SIDE_RIGHT |
            A_CH_SET_BOTTOM_FRONT_LEFT | A_CH_SET_BOTTOM_FRONT_RIGHT |
            A_CH_SET_BOTTOM_FRONT_CENTER | A_CH_SET_LOW_FREQUENCY_2),
    A_CH_MASK_OCTAGONAL = (A_CH_MASK_CH_5POINT0 | A_CH_SET_BACK_LEFT | A_CH_SET_BACK_CENTER | A_CH_SET_BACK_RIGHT),
    A_CH_MASK_HEXADECAGONAL = (A_CH_MASK_OCTAGONAL | A_CH_SET_WIDE_LEFT | A_CH_SET_WIDE_RIGHT | A_CH_SET_TOP_BACK_LEFT |
            A_CH_SET_TOP_BACK_RIGHT | A_CH_SET_TOP_BACK_CENTER | A_CH_SET_TOP_FRONT_CENTER |
            A_CH_SET_TOP_FRONT_LEFT | A_CH_SET_TOP_FRONT_RIGHT),
    A_CH_MASK_STEREO_DOWNMIX = (A_CH_SET_STEREO_LEFT | A_CH_SET_STEREO_RIGHT),
    A_CH_MASK_HOA_FIRST = A_CH_SET_AMBISONICS_ACN0 | A_CH_SET_AMBISONICS_ACN1 | A_CH_SET_AMBISONICS_ACN2 |
            A_CH_SET_AMBISONICS_ACN3,
    A_CH_MASK_HOA_SECOND = A_CH_MASK_HOA_FIRST | A_CH_SET_AMBISONICS_ACN4 | A_CH_SET_AMBISONICS_ACN5 |
            A_CH_SET_AMBISONICS_ACN6 | A_CH_SET_AMBISONICS_ACN7 | A_CH_SET_AMBISONICS_ACN8,
    A_CH_MASK_HOA_THIRD = A_CH_MASK_HOA_SECOND | A_CH_SET_AMBISONICS_ACN9 | A_CH_SET_AMBISONICS_ACN10 | A_CH_SET_AMBISONICS_ACN11 | 
            A_CH_SET_AMBISONICS_ACN12 |A_CH_SET_AMBISONICS_ACN13 | A_CH_SET_AMBISONICS_ACN14 | A_CH_SET_AMBISONICS_ACN15,
} OH_AudioChannelMask;

/**
 * @enum Audio AAC Profile。
 *
 * AAC mode type.  Note that the term profile is used with the MPEG-2
 * standard and the term object type and profile is used with MPEG-4
 *
 * @since 10
 * @version 1.0
 */
typedef enum OH_AudioAacProfile {
    AAC_PROFILE_NONE = 0,           ///< Null, not used
    AAC_PROFILE_MAIN = 1,           ///< AAC Main object
    AAC_PROFILE_LC,                 ///< AAC Low Complexity object (AAC profile)
    AAC_PROFILE_SSR,                ///< AAC Scalable Sample Rate object
    AAC_PROFILE_LTP,                ///< AAC Long Term Prediction object
    AAC_PROFILE_HE,                 ///< AAC High Efficiency (object type SBR, HE-AAC profile)
    AAC_PROFILE_SCALABLE,           ///< AAC Scalable object
    AAC_PROFILE_ERLC = 17,          ///< ER AAC Low Complexity object (Error Resilient AAC-LC)
    AAC_PROFILE_ER_SCALABLE = 20,   ///< ER AAC scalable object
    AAC_PROFILE_LD = 23,            ///< AAC Low Delay object (Error Resilient)
    AAC_PROFILE_HE_PS = 29,         ///< AAC High Efficiency with Parametric Stereo coding (HE-AAC v2, object type PS)
    AAC_PROFILE_ELD = 39,           ///< AAC Enhanced Low Delay. NOTE: Pending Khronos standardization
    AAC_PROFILE_XHE = 42,           ///< extended High Efficiency AAC. NOTE: Pending Khronos standardization
} OH_AudioAacProfile;

/**
 * @enum Video Pixel Format.
 *
 * @since 10
 * @version 1.0
 */
typedef enum OH_VideoPixelFormat {
    V_PIX_FMT_UNKNOWN = 0,
    V_PIX_FMT_YUV410P,   ///< planar YUV 4:1:0, 1 Cr & Cb sample per 4x4 Y samples
    V_PIX_FMT_YUV411P,   ///< planar YUV 4:1:1, 1 Cr & Cb sample per 4x1 Y samples
    V_PIX_FMT_YUV420P,   ///< planar YUV 4:2:0, 1 Cr & Cb sample per 2x2 Y samples
    V_PIX_FMT_NV12,      ///< semi-planar YUV 4:2:0, UVUV...
    V_PIX_FMT_NV21,      ///< semi-planar YUV 4:2:0, VUVU...
    V_PIX_FMT_YUYV422,   ///< packed YUV 4:2:2, Y0 Cb Y1 Cr
    V_PIX_FMT_YUV422P,   ///< planar YUV 4:2:2, 1 Cr & Cb sample per 2x1 Y samples
    V_PIX_FMT_YUV444P,   ///< planar YUV 4:4:4, 1 Cr & Cb sample per 1x1 Y samples
    V_PIX_FMT_RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    V_PIX_FMT_ARGB,      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    V_PIX_FMT_ABGR,      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    V_PIX_FMT_BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
    V_PIX_FMT_RGB24,     ///< packed RGB 8:8:8, RGBRGB...
    V_PIX_FMT_BGR24,     ///< packed RGB 8:8:8, BGRBGR...
    V_PIX_FMT_PAL8,      ///< 8 bit with AV_PIX_FMT_RGB32 palette
    V_PIX_FMT_GRAY8,     ///< Y
    V_PIX_FMT_MONOWHITE, ///< Y, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    V_PIX_FMT_MONOBLACK, ///< Y, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    V_PIX_FMT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG)
    V_PIX_FMT_YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG)
    V_PIX_FMT_YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG)
} OH_VideoPixelFormat;

/**
 * @enum Video H264/AVC profile.
 *
 * @since 10
 * @version 1.0
 */
typedef enum OH_VideoH264Profile {
    H264_PROFILE_UNKNOWN = 0,
    H264_PROFILE_BASELINE,  ///< Baseline profile
    H264_PROFILE_MAIN,      ///< Main profile
    H264_PROFILE_EXTENDED,  ///< Extended profile
    H264_PROFILE_HIGH,      ///< High profile
    H264_PROFILE_HIGH10,    ///< High 10 profile
    H264_PROFILE_HIGH422,   ///< High 4:2:2 profile
    H264_PROFILE_HIGH444,   ///< High 4:4:4 profile
} OH_VideoH264Profile;

/**
 * @brief Enumerate the categories of OH_AVCodec's Buffer tags
 *
 * @since 10
 * @version 1.0
 */
typedef enum OH_AVCodecBufferFlags : uint32_t {
    AVCODEC_BUFFER_FLAGS_NONE = 0,
    /* Indicates that the Buffer is an End-of-Stream frame */
    AVCODEC_BUFFER_FLAGS_EOS = 1 << 0,
    /* Indicates that the Buffer contains keyframes */
    AVCODEC_BUFFER_FLAGS_SYNC_FRAME = 1 << 1,
    /* Indicates that the data contained in the Buffer is only part of a frame */
    AVCODEC_BUFFER_FLAGS_PARTIAL_FRAME = 1 << 2,
} OH_AVCodecBufferFlags;

/**
 * @brief Define the Buffer description information of OH_AVCodec
 *
 * @since 10
 * @version 1.0
 */
typedef struct OH_AVCodecBufferInfo {
    /* Presentation timestamp of this Buffer in microseconds */
    int64_t pts;
    /* Decoding timestamp of this Buffer in microseconds */
    int64_t dts;
    /* Duration in time of this Buffer data in microseconds */
    int64_t duration;
    /* The size of the data contained in the Buffer in bytes */
    int32_t size;
    /* The starting offset of valid data in this Buffer */
    int32_t offset;
    /* The flags this Buffer has, which is also a combination of multiple {@link OH_AVCodecBufferFlags}. */
    uint32_t flags;
} OH_AVCodecBufferInfo;

/**
 * @brief Enumerates the muxer ouputfile format
 * 
 * @since 10
 * @version 1.0
 */
typedef enum OH_AVOutputFormat {
    AV_OUTPUT_FORMAT_UNKNOWN = 0,
    AV_OUTPUT_FORMAT_MPEG_4 = 1,
    AV_OUTPUT_FORMAT_M4A = 2,
} OH_AVOutputFormat;

/**
 * @brief Enumerates the video rotation.
 *
 * @since 10
 * @version 3.2
 */
typedef enum OH_VideoRotation {
    /**
     * Video without rotation
     */
    V_ROTATION_0 = 0,
    /**
     * Video rotated 90 degrees
     */
    V_ROTATION_90 = 90,
    /**
     * Video rotated 180 degrees
     */
    V_ROTATION_180 = 180,
    /**
     * Video rotated 270 degrees
     */
    V_ROTATION_270 = 270,
} OH_VideoRotation;

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVBASE_H