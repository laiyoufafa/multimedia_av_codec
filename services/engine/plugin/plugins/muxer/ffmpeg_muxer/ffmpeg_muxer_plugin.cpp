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

#include "ffmpeg_muxer_plugin.h"
#include <functional>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include "securec.h"
#include "ffmpeg_utils.h"
#include "avcodec_log.h"
#include "avcodec_common.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "FfmpegMuxerPlugin"};
}

namespace {
using namespace OHOS::Media;
using namespace Plugin;
using namespace Ffmpeg;

std::map<std::string, std::shared_ptr<AVOutputFormat>> g_pluginOutputFmt;

std::map<std::string, uint32_t> g_supportedMuxer = {{"mp4", OUTPUT_FORMAT_MPEG_4}, {"ipod", OUTPUT_FORMAT_M4A}};

bool IsMuxerSupported(const char *name)
{
    auto it = g_supportedMuxer.find(name);
    if (it != g_supportedMuxer.end()) {
        return true;
    }
    return false;
}

int32_t Sniff(const std::string& pluginName, uint32_t outputFormat)
{
    constexpr int32_t ffmpegConfidence = 60;
    if (pluginName.empty()) {
        return 0;
    }
    auto plugin = g_pluginOutputFmt[pluginName];
    int32_t confidence = 0;
    auto it = g_supportedMuxer.find(plugin->name);
    if (it != g_supportedMuxer.end() && it->second == outputFormat) {
        confidence = ffmpegConfidence;
    }

    return confidence;
}

Status RegisterMuxerPlugins(const std::shared_ptr<Register>& reg)
{
    const AVOutputFormat *outputFormat = nullptr;
    void *ite = nullptr;
    while ((outputFormat = av_muxer_iterate(&ite))) {
        if (!IsMuxerSupported(outputFormat->name)) {
            continue;
        }
        if (outputFormat->long_name != nullptr) {
            if (!strncmp(outputFormat->long_name, "raw ", 4)) { // 4
                continue;
            }
        }
        std::string pluginName = "ffmpegMux_" + std::string(outputFormat->name);
        ReplaceDelimiter(".,|-<> ", '_', pluginName);
        MuxerPluginDef def;
        def.name = pluginName;
        def.description = "ffmpeg muxer";
        def.rank = 100; // 100
        def.creator = [](const std::string& name, int32_t fd) -> std::shared_ptr<MuxerPlugin> {
            return std::make_shared<FFmpegMuxerPlugin>(name, fd);
        };
        def.sniffer = Sniff;
        if (reg->AddPlugin(def) != Status::NO_ERROR) {
            continue;
        }
        g_pluginOutputFmt[pluginName] = std::shared_ptr<AVOutputFormat>(
            const_cast<AVOutputFormat*>(outputFormat), [](AVOutputFormat *ptr) {}); // do not delete
    }
    return Status::NO_ERROR;
}

PLUGIN_DEFINITION(FFmpegMuxer, LicenseType::LGPL, RegisterMuxerPlugins, [] {g_pluginOutputFmt.clear();})

void ResetCodecParameter(AVCodecParameters *par)
{
    av_freep(&par->extradata);
    (void)memset_s(par, sizeof(*par), 0, sizeof(*par));
    par->codec_type = AVMEDIA_TYPE_UNKNOWN;
    par->codec_id = AV_CODEC_ID_NONE;
    par->format = -1;
    par->profile = FF_PROFILE_UNKNOWN;
    par->level = FF_LEVEL_UNKNOWN;
    par->field_order = AV_FIELD_UNKNOWN;
    par->color_range = AVCOL_RANGE_UNSPECIFIED;
    par->color_primaries = AVCOL_PRI_UNSPECIFIED;
    par->color_trc = AVCOL_TRC_UNSPECIFIED;
    par->color_space = AVCOL_SPC_UNSPECIFIED;
    par->chroma_location = AVCHROMA_LOC_UNSPECIFIED;
    par->sample_aspect_ratio = AVRational {0, 1};
}
}


namespace OHOS {
namespace Media {
namespace Plugin {
namespace Ffmpeg {
FFmpegMuxerPlugin::FFmpegMuxerPlugin(std::string name, int32_t fd)
    : MuxerPlugin(std::move(name)), fd_(dup(fd)), isWriteHeader_(false)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
    if (fd_ < 0) {
        AVCODEC_LOGE("fd_ %{public}d is error!", fd_);
    }
    uint32_t fdPermission = static_cast<uint32_t>(fcntl(fd_, F_GETFL, 0));
    if ((fdPermission & O_RDWR) != O_RDWR) {
        AVCODEC_LOGE("No permission to read and write fd");
    }
    if (lseek(fd_, 0, SEEK_SET) < 0) {
        AVCODEC_LOGE("The fd is not seekable");
    }
    mallopt(M_SET_THREAD_CACHE, M_THREAD_CACHE_DISABLE);
    mallopt(M_DELAYED_FREE, M_DELAYED_FREE_DISABLE);
    auto pkt = av_packet_alloc();
    cachePacket_ = std::shared_ptr<AVPacket> (pkt, [] (AVPacket *packet) {av_packet_free(&packet);});
    outputFormat_ = g_pluginOutputFmt[pluginName_];
    auto fmt = avformat_alloc_context();
    fmt->pb = InitAvIoCtx(fd_, 1);
    fmt->oformat = outputFormat_.get();
    fmt->flags = static_cast<uint32_t>(fmt->flags) | static_cast<uint32_t>(AVFMT_FLAG_CUSTOM_IO);
    fmt->io_open = IoOpen;
    fmt->io_close = IoClose;
    formatContext_ = std::shared_ptr<AVFormatContext>(fmt, [](AVFormatContext *ptr) {
        if (ptr) {
            DeInitAvIoCtx(ptr->pb);
            avformat_free_context(ptr);
        }
    });
}

FFmpegMuxerPlugin::~FFmpegMuxerPlugin()
{
    AVCODEC_LOGD("Destory");
    outputFormat_.reset();
    cachePacket_.reset();
    formatContext_.reset();
    CloseFd();
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    mallopt(M_FLUSH_THREAD_CACHE, 0);
}

Status FFmpegMuxerPlugin::SetRotation(int32_t rotation)
{
    rotation_ = rotation;
    return Status::NO_ERROR;
}

Status FFmpegMuxerPlugin::SetCodecParameterOfTrack(AVStream *stream, const MediaDescription &trackDesc)
{
    uint8_t *extraData = nullptr;
    size_t extraDataSize = 0;

    AVCodecParameters *par = stream->codecpar;
    if (trackDesc.ContainKey(MediaDescriptionKey::MD_KEY_BITRATE)) {
        trackDesc.GetLongValue(MediaDescriptionKey::MD_KEY_BITRATE, par->bit_rate); // bit rate
    }
    if (trackDesc.ContainKey(MediaDescriptionKey::MD_KEY_CODEC_CONFIG) &&
        trackDesc.GetBuffer(MediaDescriptionKey::MD_KEY_CODEC_CONFIG, &extraData, extraDataSize)) { // codec config
        par->extradata = static_cast<uint8_t *>(av_mallocz(extraDataSize + AV_INPUT_BUFFER_PADDING_SIZE));
        CHECK_AND_RETURN_RET_LOG(par->extradata != nullptr, Status::ERROR_NO_MEMORY, "codec config malloc failed!");
        par->extradata_size = static_cast<int32_t>(extraDataSize);
        errno_t rc = memcpy_s(par->extradata, par->extradata_size, extraData, extraDataSize);
        CHECK_AND_RETURN_RET_LOG(rc == EOK, Status::ERROR_UNKNOWN, "memcpy_s failed");
    }

    return Status::NO_ERROR;
}

Status FFmpegMuxerPlugin::AddAudioTrack(int32_t &trackIndex, const MediaDescription &trackDesc, AVCodecID codeID)
{
    int sampleRate = 0;
    int channels = 0;
    bool ret = trackDesc.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, sampleRate); // sample rate
    CHECK_AND_RETURN_RET_LOG((ret && sampleRate > 0), Status::ERROR_MISMATCHED_TYPE,
        "get audio sample_rate failed! sampleRate:%{public}d", sampleRate);
    ret = trackDesc.GetIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, channels); // channels
    CHECK_AND_RETURN_RET_LOG((ret && channels > 0), Status::ERROR_MISMATCHED_TYPE,
        "get audio channels failed! channels:%{public}d", channels);

    auto st = avformat_new_stream(formatContext_.get(), nullptr);
    CHECK_AND_RETURN_RET_LOG(st != nullptr, Status::ERROR_NO_MEMORY, "avformat_new_stream failed!");
    ResetCodecParameter(st->codecpar);
    st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codecpar->codec_id = codeID;
    st->codecpar->sample_rate = sampleRate;
    st->codecpar->channels = channels;
    int32_t frameSize = 0;
    if (trackDesc.ContainKey(MediaDescriptionKey::MD_KEY_AUDIO_SAMPLES_PER_FRAME) &&
        trackDesc.GetIntValue(MediaDescriptionKey::MD_KEY_AUDIO_SAMPLES_PER_FRAME, frameSize) &&
        frameSize > 0) {
        st->codecpar->frame_size = frameSize;
    }
    trackIndex = st->index;
    return SetCodecParameterOfTrack(st, trackDesc);
}

Status FFmpegMuxerPlugin::AddVideoTrack(int32_t &trackIndex, const MediaDescription &trackDesc,
                                        AVCodecID codeID, bool isCover)
{
    constexpr int maxLength = 65535;
    int width = 0;
    int height = 0;
    bool ret = trackDesc.GetIntValue(MediaDescriptionKey::MD_KEY_WIDTH, width); // width
    CHECK_AND_RETURN_RET_LOG((ret && width > 0 && width <= maxLength), Status::ERROR_MISMATCHED_TYPE,
        "get video width failed! width:%{public}d", width);
    ret = trackDesc.GetIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, height); // height
    CHECK_AND_RETURN_RET_LOG((ret && height > 0 && height <= maxLength), Status::ERROR_MISMATCHED_TYPE,
        "get video height failed! height:%{public}d", height);

    auto st = avformat_new_stream(formatContext_.get(), nullptr);
    CHECK_AND_RETURN_RET_LOG(st != nullptr, Status::ERROR_NO_MEMORY, "avformat_new_stream failed!");
    ResetCodecParameter(st->codecpar);
    st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    st->codecpar->codec_id = codeID;
    st->codecpar->width = width;
    st->codecpar->height = height;
    int32_t videoDelay = 0;
    if (trackDesc.ContainKey(MediaDescriptionKey::MD_KEY_VIDEO_DELAY) &&
        trackDesc.GetIntValue(MediaDescriptionKey::MD_KEY_VIDEO_DELAY, videoDelay) &&
        videoDelay > 0) {
        st->codecpar->video_delay = videoDelay;
    }

    trackIndex = st->index;
    if (isCover) {
        st->disposition = AV_DISPOSITION_ATTACHED_PIC;
    }
    double frameRate = 0;
    if (trackDesc.ContainKey(MediaDescriptionKey::MD_KEY_FRAME_RATE) &&
        trackDesc.GetDoubleValue(MediaDescriptionKey::MD_KEY_FRAME_RATE, frameRate) &&
        frameRate > 0) {
        st->avg_frame_rate = {static_cast<int>(frameRate), 1};
    }
    return SetCodecParameterOfTrack(st, trackDesc);
}

Status FFmpegMuxerPlugin::AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc)
{
    CHECK_AND_RETURN_RET_LOG(!isWriteHeader_, Status::ERROR_WRONG_STATE, "AddTrack failed! muxer has start!");
    CHECK_AND_RETURN_RET_LOG(outputFormat_ != nullptr, Status::ERROR_NULL_POINTER, "AVOutputFormat is nullptr");
    constexpr int32_t mimeTypeLen = 5;
    Status ret = Status::NO_ERROR;
    std::string mimeType = {};
    AVCodecID codeID = AV_CODEC_ID_NONE;
    CHECK_AND_RETURN_RET_LOG(trackDesc.GetStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, mimeType),
        Status::ERROR_MISMATCHED_TYPE, "get mimeType failed!"); // mime
    AVCODEC_LOGD("mimeType is %{public}s", mimeType.c_str());
    CHECK_AND_RETURN_RET_LOG(Mime2CodecId(mimeType, codeID), Status::ERROR_INVALID_DATA,
        "this mimeType do not support! mimeType:%{public}s", mimeType.c_str());
    if (!mimeType.compare(0, mimeTypeLen, "audio")) {
        ret = AddAudioTrack(trackIndex, trackDesc, codeID);
        CHECK_AND_RETURN_RET_LOG(ret == Status::NO_ERROR, ret, "AddAudioTrack failed!");
    } else if (!mimeType.compare(0, mimeTypeLen, "video")) {
        ret = AddVideoTrack(trackIndex, trackDesc, codeID, false);
        CHECK_AND_RETURN_RET_LOG(ret == Status::NO_ERROR, ret, "AddVideoTrack failed!");
    } else if (!mimeType.compare(0, mimeTypeLen, "image")) {
        ret = AddVideoTrack(trackIndex, trackDesc, codeID, true);
        CHECK_AND_RETURN_RET_LOG(ret == Status::NO_ERROR, ret, "AddCoverTrack failed!");
    } else {
        AVCODEC_LOGD("mimeType %{public}s is unsupported", mimeType.c_str());
        return Status::ERROR_UNSUPPORTED_FORMAT;
    }
    uint32_t flags = static_cast<uint32_t>(formatContext_->flags);
    formatContext_->flags = static_cast<int32_t>(flags | AVFMT_TS_NONSTRICT);
    return Status::NO_ERROR;
}

Status FFmpegMuxerPlugin::Start()
{
    CHECK_AND_RETURN_RET_LOG(!isWriteHeader_, Status::ERROR_WRONG_STATE, "Start failed! muxer has start!");
    if (rotation_ != 0) {
        std::string rotate = std::to_string(rotation_);
        for (uint32_t i = 0; i < formatContext_->nb_streams; i++) {
            if (formatContext_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                av_dict_set(&formatContext_->streams[i]->metadata, "rotate", rotate.c_str(), 0);
            }
        }
    }
    AVDictionary *options = nullptr;
    av_dict_set(&options, "movflags", "faststart", 0);
    int ret = avformat_write_header(formatContext_.get(), &options);
    if (ret < 0) {
        AVCODEC_LOGE("write header failed, %{public}s", AVStrError(ret).c_str());
        return Status::ERROR_UNKNOWN;
    }
    isWriteHeader_ = true;
    return Status::NO_ERROR;
}

Status FFmpegMuxerPlugin::Stop()
{
    CHECK_AND_RETURN_RET_LOG(isWriteHeader_, Status::ERROR_WRONG_STATE, "Stop failed! Did not write header!");
    int ret = av_write_frame(formatContext_.get(), nullptr); // flush out cache data
    if (ret < 0) {
        AVCODEC_LOGE("write trailer failed, %{public}s", AVStrError(ret).c_str());
    }
    ret = av_write_trailer(formatContext_.get());
    if (ret != 0) {
        AVCODEC_LOGE("write trailer failed, %{public}s", AVStrError(ret).c_str());
    }
    avio_flush(formatContext_->pb);

    CloseFd();
    return Status::NO_ERROR;
}

Status FFmpegMuxerPlugin::WriteSample(uint32_t trackIndex, const uint8_t *sample,
        AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    CHECK_AND_RETURN_RET_LOG(isWriteHeader_, Status::ERROR_WRONG_STATE, "WriteSample failed! Did not write header!");
    CHECK_AND_RETURN_RET_LOG(sample != nullptr, Status::ERROR_NULL_POINTER,
        "av_write_frame sample is null!");
    CHECK_AND_RETURN_RET_LOG(trackIndex < formatContext_->nb_streams,
        Status::ERROR_INVALID_PARAMETER, "track index is invalid!");
    av_init_packet(cachePacket_.get());
    cachePacket_->data = const_cast<uint8_t *>(sample);
    cachePacket_->size = info.size;
    cachePacket_->stream_index = static_cast<int>(trackIndex);
    cachePacket_->pts = ConvertTimeToFFmpeg(info.presentationTimeUs, formatContext_->streams[trackIndex]->time_base);
    if (formatContext_->streams[trackIndex]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        cachePacket_->dts = cachePacket_->pts;
    }
    cachePacket_->flags = 0;
    if (flag & AVCODEC_BUFFER_FLAG_SYNC_FRAME) {
        AVCODEC_LOGD("It is key frame");
        cachePacket_->flags = AV_PKT_FLAG_KEY;
    }
    auto ret = av_write_frame(formatContext_.get(), cachePacket_.get());
    av_packet_unref(cachePacket_.get());
    if (ret < 0) {
        AVCODEC_LOGE("write sample buffer failed, %{public}s", AVStrError(ret).c_str());
        return Status::ERROR_UNKNOWN;
    }
    return Status::NO_ERROR;
}

AVIOContext *FFmpegMuxerPlugin::InitAvIoCtx(int32_t fd, int writeFlags)
{
    IOContext *ioContext = new IOContext();
    ioContext->fd_ = fd;
    ioContext->pos_ = 0;
    ioContext->end_ = 0;

    constexpr int bufferSize = 4 * 1024; // 4096
    auto buffer = static_cast<unsigned char*>(av_malloc(bufferSize));
    AVIOContext *avioContext = avio_alloc_context(buffer, bufferSize, writeFlags, static_cast<void*>(ioContext),
                                                  IoRead, IoWrite, IoSeek);
    if (avioContext == nullptr) {
        delete ioContext;
        av_free(buffer);
        return nullptr;
    }
    avioContext->seekable = AVIO_SEEKABLE_NORMAL;
    return avioContext;
}

void FFmpegMuxerPlugin::DeInitAvIoCtx(AVIOContext *ptr)
{
    if (ptr != nullptr) {
        delete static_cast<IOContext*>(ptr->opaque);
        ptr->opaque = nullptr;
        av_freep(&ptr->buffer);
        av_opt_free(ptr);
        avio_context_free(&ptr);
    }
}

void FFmpegMuxerPlugin::CloseFd()
{
    if (fd_ != -1) {
        AVCODEC_LOGD("close fd");
        close(fd_);
        fd_ = -1;
    }
}

int32_t FFmpegMuxerPlugin::IoRead(void *opaque, uint8_t *buf, int bufSize)
{
    auto ioCtx = static_cast<IOContext*>(opaque);
    if (ioCtx && ioCtx->fd_ != -1) {
        int64_t ret = lseek(ioCtx->fd_, ioCtx->pos_, SEEK_SET);
        if (ret != -1) {
            ssize_t size = read(ioCtx->fd_, buf, bufSize);
            if (size < 0) {
                return -1;
            }
            ioCtx->pos_ += size;
            if (ioCtx->pos_ > ioCtx->end_) {
                ioCtx->end_ = ioCtx->pos_;
            }
            return size;
        }
        return 0;
    }
    return -1;
}

int32_t FFmpegMuxerPlugin::IoWrite(void *opaque, uint8_t *buf, int bufSize)
{
    auto ioCtx = static_cast<IOContext*>(opaque);
    if (ioCtx && ioCtx->fd_ != -1) {
        int64_t ret = lseek(ioCtx->fd_, ioCtx->pos_, SEEK_SET);
        if (ret != -1) {
            ssize_t size = write(ioCtx->fd_, buf, bufSize);
            if (size < 0) {
                return -1;
            }
            ioCtx->pos_ += size;
            if (ioCtx->pos_ > ioCtx->end_) {
                ioCtx->end_ = ioCtx->pos_;
            }
            return size;
        }
        return 0;
    }
    return -1;
}

int64_t FFmpegMuxerPlugin::IoSeek(void *opaque, int64_t offset, int whence)
{
    auto ioContext = static_cast<IOContext*>(opaque);
    uint64_t newPos = 0;
    switch (whence) {
        case SEEK_SET:
            newPos = static_cast<uint64_t>(offset);
            ioContext->pos_ = newPos;
            break;
        case SEEK_CUR:
            newPos = ioContext->pos_ + offset;
            break;
        case SEEK_END:
        case AVSEEK_SIZE:
            newPos = ioContext->end_ + offset;
            break;
        default:
            break;
    }
    if (whence != AVSEEK_SIZE) {
        ioContext->pos_ = newPos;
    }
    return newPos;
}

int32_t FFmpegMuxerPlugin::IoOpen(AVFormatContext *s, AVIOContext **pb,
                                  const char *url, int flags, AVDictionary **options)
{
    AVCODEC_LOGD("IoOpen flags %{public}d", flags);
    *pb = InitAvIoCtx(static_cast<IOContext*>(s->pb->opaque)->fd_, 0);
    return 0;
}

void FFmpegMuxerPlugin::IoClose(AVFormatContext *s, AVIOContext *pb)
{
    avio_flush(pb);
    DeInitAvIoCtx(pb);
}
} // Ffmpeg
} // Plugin
} // Media
} // OHOS
