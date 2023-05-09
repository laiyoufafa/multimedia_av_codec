#include "audio_ffmpeg_aac_encoder_plugin.h"
#include "avcodec_errors.h"
#include "media_description.h"
#include <iostream>
namespace OHOS {
namespace Media {
AudioFFMpegAacEncoderPlugin::AudioFFMpegAacEncoderPlugin() : basePlugin(std::make_unique<AudioFfmpegEncoderPlugin>()) {}

AudioFFMpegAacEncoderPlugin::~AudioFFMpegAacEncoderPlugin() {
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

static int32_t GetAdtsHeader(std::string &adtsHeader, uint32_t &headerSize, std::shared_ptr<AVCodecContext> ctx, int aacLength)
{
    uint8_t freqIdx = 0;    //0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
    switch (ctx->sample_rate) {
    case 96000: freqIdx = 0; break;
    case 88200: freqIdx = 1; break;
    case 64000: freqIdx = 2; break;
    case 48000: freqIdx = 3; break;
    case 44100: freqIdx = 4; break;
    case 32000: freqIdx = 5; break;
    case 24000: freqIdx = 6; break;
    case 22050: freqIdx = 7; break;
    case 16000: freqIdx = 8; break;
    case 12000: freqIdx = 9; break;
    case 11025: freqIdx = 10; break;
    case 8000: freqIdx = 11; break;
    case 7350: freqIdx = 12; break;
    default: freqIdx = 4; break;
    }
    uint8_t chanCfg = ctx->channels;
    uint32_t frameLength = aacLength + 7;
    adtsHeader += 0xFF;
    adtsHeader += 0xF1;
    adtsHeader += ((ctx->profile) << 6) + (freqIdx << 2) + (chanCfg >> 2);
    adtsHeader += (((chanCfg & 3) << 6) + (frameLength >> 11));
    adtsHeader += ((frameLength & 0x7FF) >> 3);
    adtsHeader += (((frameLength & 7) << 5) + 0x1F);
    adtsHeader += 0xFC;
    headerSize = 7;

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

static bool CheckSampleFormat(const std::shared_ptr<AVCodec> &codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat* p = codec->sample_fmts;
    while (*p != AV_SAMPLE_FMT_NONE) { // 通过AV_SAMPLE_FMT_NONE作为结束符
        if (*p == sample_fmt)
            return true;
        p++;
    }
    return false;
}

static bool CheckSampleRate(const std::shared_ptr<AVCodec> &codec, const int sample_rate)
{
    const int* p = codec->supported_samplerates;
    while (*p != 0) {// 0作为退出条件，比如libfdk-aacenc.c的aac_sample_rates
        if (*p == sample_rate)
            return true;
        p++;
    }
    return false;
}

static bool CheckChannelLayout(const std::shared_ptr<AVCodec> &codec, const uint64_t channel_layout)
{
    // 不是每个codec都给出支持的channel_layout
    const uint64_t* p = codec->channel_layouts;
    if (!p) {
        std::cout << "The codec " << codec->name << " no set channel_layouts" << std::endl;
        return true;
    }
    while (*p != 0) { // 0作为退出条件，比如libfdk-aacenc.c的aac_channel_layout
        if (*p == channel_layout)
            return true;
        p++;
    }
    return false;
}

bool AudioFFMpegAacEncoderPlugin::CheckFormat(const Format &format) const
{
    if (!format.ContainKey(MediaDescriptionKey::MD_KEY_SAMPLE_FORMAT) ||
        !format.ContainKey(MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT) ||
        !format.ContainKey(MediaDescriptionKey::MD_KEY_SAMPLE_RATE)) {
        std::cout << "parameter missing" << std::endl;
        return false;
    }

    auto avCodec = basePlugin->GetAVCodec();
    int sampleFormat;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_FORMAT, sampleFormat);
    if (!CheckSampleFormat(avCodec, (AVSampleFormat)sampleFormat)) {
        std::cout << "sample format is not supported" << std::endl;
        return false;
    }

    int sampleRate;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, sampleRate);
    if (!CheckSampleRate(avCodec, sampleRate)) {
        std::cout << "sample rate is not supported" << std::endl;
        return false;
    }

    int64_t channelLayout;
    format.GetLongValue(MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT, channelLayout);
    if (!CheckChannelLayout(avCodec, channelLayout)) {
        std::cout << "channel layout is not supported" << std::endl;
        return false;
    }

    return true;
}

int32_t AudioFFMpegAacEncoderPlugin::init(const Format &format) {
    int32_t ret = basePlugin->AllocateContext("aac");
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        std::cout << "init 1 OH error:" << ret << "\n";
        return ret;
    }
    if (!CheckFormat(format)) {
        std::cout << "format not supported" << std::endl;
        return AVCodecServiceErrCode::AVCS_ERR_UNSUPPORT_AUD_PARAMS;
    }
    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        std::cout << "init 2 OH error:" << ret << "\n";
        return ret;
    }
    std::cout << "AudioFFMpegAacEncoderPlugin::init done." << std::endl;
    ret = basePlugin->OpenContext();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        std::cout << "init 3 OH error:" << ret << "\n";
        return ret;
    }

    ret = basePlugin->InitFrame();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        std::cout << "init 4 OH error:" << ret << "\n";
        return ret;
    }

    basePlugin->RegisterHeaderFunc(GetAdtsHeader);

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAacEncoderPlugin::processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer) {
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegAacEncoderPlugin::processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer) {
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegAacEncoderPlugin::reset() {
    return basePlugin->Reset();
}

int32_t AudioFFMpegAacEncoderPlugin::release() {
    return basePlugin->Release();
}

int32_t AudioFFMpegAacEncoderPlugin::flush() {
    return basePlugin->Flush();
}

uint32_t AudioFFMpegAacEncoderPlugin::getInputBufferSize() const {
    return 4 * 1024 * 8;
}

uint32_t AudioFFMpegAacEncoderPlugin::getOutputBufferSize() const {
    return 8192;
}

Format AudioFFMpegAacEncoderPlugin::GetFormat() const noexcept {
    return basePlugin->GetFormat();
}

} // namespace Media
} // namespace OHOS