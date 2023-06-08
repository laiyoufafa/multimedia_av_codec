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

#include "audio_ffmpeg_vorbis_decoder_plugin.h"
#include "avcodec_dfx.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "media_description.h"
#include "securec.h"
#include "avcodec_mime_type.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegVorbisEncoderPlugin"};
constexpr uint8_t EXTRADATA_FIRST_CHAR = 2;
constexpr int COMMENT_HEADER_LENGTH = 16;
constexpr int COMMENT_HEADER_PADDING_LENGTH = 8;
constexpr uint8_t COMMENT_HEADER_FIRST_CHAR = '\x3';
constexpr uint8_t COMMENT_HEADER_LAST_CHAR = '\x1';
constexpr std::string_view VORBIS_STRING = "vorbis";
constexpr int NUMBER_PER_BYTES = 255;
}

namespace OHOS {
namespace Media {
static constexpr int32_t INPUT_BUFFER_SIZE_DEFAULT = 8192;
static constexpr int32_t OUTPUT_BUFFER_SIZE_DEFAULT = 4 * 1024 * 8;
constexpr std::string_view AUDIO_CODEC_NAME = "vorbis";

AudioFFMpegVorbisDecoderPlugin::AudioFFMpegVorbisDecoderPlugin()
    : basePlugin(std::make_unique<AudioFfmpegDecoderPlugin>())
{
}

AudioFFMpegVorbisDecoderPlugin::~AudioFFMpegVorbisDecoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

bool AudioFFMpegVorbisDecoderPlugin::CheckFormat(const Format &format) const
{
    if (!format.ContainKey(MediaDescriptionKey::MD_KEY_IDENTIFICATION_HEADER.data())) {
        AVCODEC_LOGE("Identification header missing.");
        return false;
    }
    if (!format.ContainKey(MediaDescriptionKey::MD_KEY_SETUP_HEADER.data())) {
        AVCODEC_LOGE("setup header missing.");
        return false;
    }
    return true;
}

void AudioFFMpegVorbisDecoderPlugin::GetExtradataSize(size_t idSize, size_t setupSize) const
{
    auto codecCtx = basePlugin->GetCodecContext();
    codecCtx->extradata_size = 1 + (1 + idSize/NUMBER_PER_BYTES + idSize) +
                                   (1 + COMMENT_HEADER_LENGTH/NUMBER_PER_BYTES + COMMENT_HEADER_LENGTH) +
                                   setupSize;
}

int AudioFFMpegVorbisDecoderPlugin::PutHeaderLength(uint8_t *p, size_t value) const
{
    int n = 0;
    while (value >= 0xff) {
        *p++ = 0xff;
        value -= 0xff;
        n++;
    }
    *p = value;
    n++;
    return n;
}

void AudioFFMpegVorbisDecoderPlugin::PutCommentHeader(int offset) const
{
    auto codecCtx = basePlugin->GetCodecContext();
    auto data = codecCtx->extradata;
    int pos = offset;
    data[pos++] = COMMENT_HEADER_FIRST_CHAR;
    for (size_t i = 0; i < VORBIS_STRING.size(); i++) {
        data[pos++] = VORBIS_STRING[i];
    }
    for (int i = 0; i < COMMENT_HEADER_PADDING_LENGTH; i++) {
        data[pos++] = '\0';
    }
    data[pos] = COMMENT_HEADER_LAST_CHAR;
}

int32_t AudioFFMpegVorbisDecoderPlugin::GenExtradata(const Format &format) const
{
    AVCODEC_LOGD("GenExtradata start");
    size_t idSize;
    uint8_t *idHeader;
    if (!format.GetBuffer(MediaDescriptionKey::MD_KEY_IDENTIFICATION_HEADER.data(), &idHeader, idSize)) {
        AVCODEC_LOGE("identification header not available.");
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    size_t setupSize;
    uint8_t *setupHeader;
    if (!format.GetBuffer(MediaDescriptionKey::MD_KEY_SETUP_HEADER.data(), &setupHeader, setupSize)) {
        AVCODEC_LOGE("setup header not available.");
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }

    GetExtradataSize(idSize, setupSize);
    auto codecCtx = basePlugin->GetCodecContext();
    codecCtx->extradata = static_cast<uint8_t*>(av_mallocz(codecCtx->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));

    int offset = 0;
    codecCtx->extradata[0] = EXTRADATA_FIRST_CHAR;
    offset = 1;

    // put identification header size and comment header size
    offset += PutHeaderLength(codecCtx->extradata + offset, idSize);
    codecCtx->extradata[offset++] = COMMENT_HEADER_LENGTH;

    // put identification header
    int ret = memcpy_s(codecCtx->extradata + offset, codecCtx->extradata_size - offset, idHeader, idSize);
    if (ret != 0) {
        AVCODEC_LOGE("memory copy failed: %{public}d", ret);
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    offset += idSize;

    // put comment header
    PutCommentHeader(offset);
    offset += COMMENT_HEADER_LENGTH;

    // put setup header
    ret =memcpy_s(codecCtx->extradata + offset, codecCtx->extradata_size - offset, setupHeader, setupSize);
    if (ret != 0) {
        AVCODEC_LOGE("memory copy failed: %{public}d", ret);
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    offset += setupSize;

    if (offset != codecCtx->extradata_size) {
        AVCODEC_LOGW("extradata write length mismatch extradata size");
    }

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegVorbisDecoderPlugin::Init(const Format &format)
{
    int32_t ret = basePlugin->AllocateContext("vorbis");
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("AllocateContext failed, ret=%{public}d", ret);
        return ret;
    }
    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("InitContext failed, ret=%{public}d", ret);
        return ret;
    }
    auto codecCtx = basePlugin->GetCodecContext();
    if (!basePlugin->HasExtraData()) {
        ret = GenExtradata(format);
        if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
            AVCODEC_LOGE("Generate extradata failed, ret=%{public}d", ret);
            return ret;
        }
    }
    return basePlugin->OpenContext();
}

int32_t AudioFFMpegVorbisDecoderPlugin::ProcessSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegVorbisDecoderPlugin::ProcessRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegVorbisDecoderPlugin::Reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegVorbisDecoderPlugin::Release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegVorbisDecoderPlugin::Flush()
{
    return basePlugin->Flush();
}

int32_t AudioFFMpegVorbisDecoderPlugin::GetInputBufferSize() const
{
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > INPUT_BUFFER_SIZE_DEFAULT) {
        maxSize = INPUT_BUFFER_SIZE_DEFAULT;
    }
    return maxSize;
}

int32_t AudioFFMpegVorbisDecoderPlugin::GetOutputBufferSize() const
{
    return OUTPUT_BUFFER_SIZE_DEFAULT;
}

Format AudioFFMpegVorbisDecoderPlugin::GetFormat() const noexcept
{
    auto format = basePlugin->GetFormat();
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_VORBIS);
    return format;
}
std::string_view AudioFFMpegVorbisDecoderPlugin::GetCodecType() const noexcept
{
    return AUDIO_CODEC_NAME;
}
} // namespace Media
} // namespace OHOS