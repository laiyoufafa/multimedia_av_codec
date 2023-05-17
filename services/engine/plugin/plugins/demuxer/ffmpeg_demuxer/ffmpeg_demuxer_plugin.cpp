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

#include <algorithm>
#include <string>
#include <sstream>
#include <type_traits>
#include <iostream>
#include "securec.h"
#include "avcodec_errors.h"
#include "native_avcodec_base.h"
#include "plugin_definition.h"
#include "avcodec_log.h"
#include "avcodec_dfx.h"
#include "ffmpeg_demuxer_plugin.h"

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 78, 0) and LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(58, 64, 100)
#if LIBAVFORMAT_VERSION_INT != AV_VERSION_INT(58, 76, 100)

#endif
#endif
#define AV_CODEC_TIME_BASE (static_cast<int64_t>(1))
#define AV_CODEC_NSECOND AV_CODEC_TIME_BASE
#define AV_CODEC_USECOND (static_cast<int64_t>(1000) * AV_CODEC_NSECOND)
#define AV_CODEC_MSECOND (static_cast<int64_t>(1000) * AV_CODEC_USECOND)
#define AV_CODEC_SECOND (static_cast<int64_t>(1000) * AV_CODEC_MSECOND)


namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "FFmpegDemuxerPlugin"};
}


namespace OHOS {
namespace Media {
namespace Plugin {
namespace FFmpeg {
namespace {
static const std::map<AVSeekMode, int32_t>  seekModeToFFmpegSeekFlags = {
    { AVSeekMode::SEEK_MODE_PREVIOUS_SYNC, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD },
    { AVSeekMode::SEEK_MODE_NEXT_SYNC, AVSEEK_FLAG_FRAME },
    { AVSeekMode::SEEK_MODE_CLOSEST_SYNC, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_ANY }
};

int32_t Sniff(const std::string& pluginName)
{
    return 100;
}

Status RegisterDemuxerPlugins(const std::shared_ptr<Register>& reg)
{
    DemuxerPluginDef def;
    constexpr int32_t RankScore = 100;
    def.name = "ffmpegDemuxer";
    def.description = "ffmpeg demuxer";
    def.rank = RankScore;
    def.creator = []() -> std::shared_ptr<DemuxerPlugin> {
        return std::make_shared<FFmpegDemuxerPlugin>();
    };
    def.sniffer = Sniff;
    return reg->AddPlugin(def);
}

PLUGIN_DEFINITION(FFmpegDemuxer, LicenseType::GPL, RegisterDemuxerPlugins, [] {})

static const std::map<std::string, VideoBitStreamFormat> videoBitStreamFormatStringMap = {
                                                            {"AVCC", VideoBitStreamFormat::AVCC},
                                                            {"HVCC", VideoBitStreamFormat::HVCC},
                                                            {"ANNEXB", VideoBitStreamFormat::ANNEXB},
};
}

inline int64_t AvTime2Ms(int64_t hTime)
{
    return hTime / AV_CODEC_MSECOND;
}

int64_t ConvertTimeToFFmpeg(int64_t timestampUs, AVRational base)
{
    int64_t result;
    if (base.num == 0) {
        result = AV_NOPTS_VALUE;
    } else {
        AVRational bq = {1, AV_CODEC_SECOND};
        result = av_rescale_q(timestampUs, bq, base);
    }
    return result;
}

int64_t ConvertTimeFromFFmpeg(int64_t pts, AVRational base)
{
    int64_t out;
    if (pts == AV_NOPTS_VALUE) {
        out = -1;
    } else {
        AVRational bq = {1, AV_CODEC_SECOND};
        out = av_rescale_q(pts, base, bq);
    }
    return out;
}

int64_t FFmpegDemuxerPlugin::GetTotalStreamFrames(int streamIndex)
{
    AVCODEC_LOGD("FFmpegDemuxerPlugin::GetTotalStreamFrames is called");
    return formatContext_->streams[streamIndex]->nb_frames;
}


AVCodecBufferFlag FFmpegDemuxerPlugin::ConvertFlagsFromFFmpeg(AVPacket* pkt,  AVStream* avStream)
{
    AVCODEC_LOGD("FFmpegDemuxerPlugin::ConvertFlagsFromFFmpeg is called");
    AVCodecBufferFlag flags = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
    if (pkt->flags == 0x0001) {
        if (abs((int)(avStream->duration-pkt->pts)) <= (pkt->duration)) {
            flags =  AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_EOS;
        } else {
            flags = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_SYNC_FRAME;
        }
    } else {
        flags = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
    }

    return flags;
}

int32_t FFmpegDemuxerPlugin::Create(uintptr_t sourceAddr)
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::Create is on call");
    if (std::is_object<decltype(sourceAddr)>::value) {
        formatContext_ = std::shared_ptr<AVFormatContext>((AVFormatContext*)sourceAddr);
        SetBitStreamFormat();
        AVCODEC_LOGD("create FFmpegDemuxerPlugin successful.");
    } else {
        formatContext_ = nullptr;
        AVCODEC_LOGW("create FFmpegDemuxerPlugin failed, becasue sourceAddr is not a class address.");
        return AVCS_ERR_INVALID_VAL;
    }
    return AVCS_ERR_OK;
}

FFmpegDemuxerPlugin::FFmpegDemuxerPlugin()
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::FFmpegDemuxerPlugin is on call");
}

FFmpegDemuxerPlugin::~FFmpegDemuxerPlugin()
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::~FFmpegDemuxerPlugin is on call");
    formatContext_ = nullptr;
    selectedTrackIds_.clear();
}

int32_t FFmpegDemuxerPlugin::SetBitStreamFormat()
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::SetBitStreamFormat is on call");
    uint32_t trackCount = formatContext_->nb_streams;
    for (uint32_t i = 0; i < trackCount; i++) {
        if (formatContext_->streams[i]->codecpar->codec_type== AVMEDIA_TYPE_VIDEO) {
            AVDictionary *trackMetadata = formatContext_->streams[i]->metadata;
            std::string key = std::string(AVSourceTrackFormat::VIDEO_BIT_STREAM_FORMAT);
            auto tag = av_dict_get(trackMetadata, key.c_str(), nullptr, AV_DICT_IGNORE_SUFFIX);
            if (tag == nullptr) {
                AVCODEC_LOGW("SetBitStreamFormat failed, Output stream format will not change!");
            } else if (auto value = videoBitStreamFormatStringMap.at(tag->value)) {
                videoBitStreamFormat_[i] = value;
                AVCODEC_LOGD("SetBitStreamFormat successful, track %{public}d, %{public}s", i, tag->value);
            } else {
                AVCODEC_LOGW("SetBitStreamFormat failed, %{public}s is not supported, \
                             Output stream format will not change!", tag->value);
            }
        }
    }
    std::string videoBitStreamFormatString = "";
    for (auto const& pair: videoBitStreamFormat_) {
        videoBitStreamFormatString += std::to_string(pair.first) + "=" + std::to_string(pair.second) + " | ";
    }
    AVCODEC_LOGD("videoBitStreamFormat: %{public}s", videoBitStreamFormatString.c_str());
    return AVCS_ERR_OK;
}

int32_t FFmpegDemuxerPlugin::SelectSourceTrackByID(uint32_t trackIndex)
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::SelectSourceTrackByID is on call");
    std::stringstream selectedTracksString;
    for (const auto &index : selectedTrackIds_) {
        selectedTracksString << index;
    }
    AVCODEC_LOGI("Total track in file: %{public}d | add track index: %{public}d",
        formatContext_.get()->nb_streams, trackIndex);
    AVCODEC_LOGI("Selected tracks in file: %{public}s.", selectedTracksString.str().c_str());

    if (trackIndex < 0 || trackIndex >= static_cast<uint32_t>(formatContext_.get()->nb_streams)) {
        AVCODEC_LOGE("trackIndex is invalid! Just have %{public}d tracks in file", formatContext_.get()->nb_streams);
        return AVCS_ERR_INVALID_VAL;
    }
    auto index = std::find_if(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                              [trackIndex](uint32_t selectedId) {return trackIndex == selectedId;});
    if (index == selectedTrackIds_.end()) {
        selectedTrackIds_.push_back(trackIndex);
    } else {
        AVCODEC_LOGW("track %{public}d is already in selected list!", trackIndex);
    }
    return AVCS_ERR_OK;
}

int32_t FFmpegDemuxerPlugin::UnselectSourceTrackByID(uint32_t trackIndex)
{
    std::stringstream selectedTracksString;
    for (const auto &index : selectedTrackIds_) {
        selectedTracksString << index;
    }
    AVCODEC_LOGI("Selected track in file: %{public}s | remove track: %{public}d",
        selectedTracksString.str().c_str(), trackIndex);

    auto index = std::find_if(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                              [trackIndex](uint32_t selectedId) {return trackIndex == selectedId; });
    if (index != selectedTrackIds_.end()) {
        selectedTrackIds_.erase(index);
    } else {
        AVCODEC_LOGW("Unselect track failed, track %{public}d is not in selected list!", trackIndex);
        return AVCS_ERR_INVALID_VAL;
    }
    return AVCS_ERR_OK;
}

std::vector<uint32_t> FFmpegDemuxerPlugin::GetSelectedTrackIds()
{
    AVCODEC_LOGD("FFmpegDemuxerPlugin::GetSelectedTrackIds is on call");
    std::vector<uint32_t> trackIds;
    trackIds = selectedTrackIds_;
    return trackIds;
}

bool FFmpegDemuxerPlugin::IsInSelectedTrack(uint32_t trackIndex)
{
    AVCODEC_LOGD("FFmpegDemuxerPlugin::IsInSelectedTrack is on call");
    return std::any_of(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                       [trackIndex](uint32_t id) { return id == trackIndex; });
}

void FFmpegDemuxerPlugin::InitBitStreamContext(const AVStream& avStream)
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::InitBitStreamContext is on call");
    const AVBitStreamFilter* avBitStreamFilter {nullptr};
    char codeTag[AV_FOURCC_MAX_STRING_SIZE] {0};
    av_fourcc_make_string(codeTag, avStream.codecpar->codec_tag);
    if (strncmp(codeTag, "avc1", strlen("avc1")) == 0) {
        AVCODEC_LOGD("codeTag is avc1, will convert avc1 to annexb");
        avBitStreamFilter = av_bsf_get_by_name("h264_mp4toannexb");
    } else if (strncmp(codeTag, "hevc", strlen("hevc")) == 0) {
        AVCODEC_LOGD("codeTag is hevc, will convert hevc to annexb");
        avBitStreamFilter = av_bsf_get_by_name("hevc_mp4toannexb");
    } else {
        AVCODEC_LOGW("Can not find valid bit stream filter for %{public}s, stream will not be converted", codeTag);
    }
    if (avBitStreamFilter && !avbsfContext_) {
        AVBSFContext* avbsfContext {nullptr};
        (void)av_bsf_alloc(avBitStreamFilter, &avbsfContext);
        (void)avcodec_parameters_copy(avbsfContext->par_in, avStream.codecpar);
        av_bsf_init(avbsfContext);
        avbsfContext_ = std::shared_ptr<AVBSFContext>(avbsfContext, [](AVBSFContext* ptr) {
            if (ptr) {
                av_bsf_free(&ptr);
            }
        });
    }
    if (avbsfContext_ == nullptr) {
        AVCODEC_LOGW("the video bit stream not support %{public}s convert to annexb format, \
                     stream will not be converted", codeTag);
    }
}

void FFmpegDemuxerPlugin::ConvertAvcOrHevcToAnnexb(AVPacket& pkt)
{
    AVCODEC_LOGD("FFmpegDemuxerPlugin::ConvertAvcOrHevcToAnnexb is on call");
    (void)av_bsf_send_packet(avbsfContext_.get(), &pkt);
    (void)av_packet_unref(&pkt);
    (void)av_bsf_receive_packet(avbsfContext_.get(), &pkt);
}

int32_t FFmpegDemuxerPlugin::ConvertAVPacketToSample(AVStream* avStream, uint8_t* buffer, AVCodecBufferInfo &bufferInfo,
                                                     AVCodecBufferFlag &flag, AVPacket* pkt)
{
    int frameSize = 0;
    bufferInfo.presentationTimeUs = AvTime2Ms(ConvertTimeFromFFmpeg(pkt->pts, avStream->time_base));
    flag = ConvertFlagsFromFFmpeg(pkt, avStream);
    if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        frameSize = pkt->size;
    } else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        if (avStream->codecpar->codec_id == AV_CODEC_ID_RAWVIDEO) {
            AVCODEC_LOGE("unsupport raw video");
            return AVCS_ERR_UNSUPPORT_STREAM;
        }
        if (videoBitStreamFormat_[pkt->stream_index] == VideoBitStreamFormat::ANNEXB) {
            if (!avbsfContext_) {
                InitBitStreamContext(*avStream);
            }
            if (avbsfContext_) {
                ConvertAvcOrHevcToAnnexb(*pkt);
            }
        }
        frameSize = pkt->size;
    } else {
        AVCODEC_LOGE("unsupport stream type");
        return AVCS_ERR_UNSUPPORT_VID_PARAMS;
    }
    if (buffer==nullptr) {
        AVCODEC_LOGW("the buffer is NULL, allocate buffer\n");
        buffer = (uint8_t*)malloc(frameSize);
    }
    
    bufferInfo.size = frameSize;
    bufferInfo.offset = 0;
    memset_s(buffer, frameSize, 0, frameSize);
    memcpy_s(buffer, frameSize, pkt->data, frameSize);
    return AVCS_ERR_OK;
}

int32_t FFmpegDemuxerPlugin::CopyNextSample(uint32_t &trackIndex, uint8_t* buffer,
                                            AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag)
{
    AVCODEC_LOGD("FFmpegDemuxerPlugin::CopyNextSample is on call");
    int ret = -1;
    AVPacket* pkt = av_packet_alloc();
    do {
        ret = av_read_frame(formatContext_.get(), pkt);
    } while (ret >= 0 && !selectedTrackIds_.empty() && !IsInSelectedTrack(pkt->stream_index));
    
    if (ret >= 0) {
        trackIndex = pkt->stream_index;
        AVStream* avStream = formatContext_->streams[pkt->stream_index];
        if (!sampleIndex_.count(pkt->stream_index)) {
            sampleIndex_[pkt->stream_index]=1;
        } else {
            sampleIndex_[pkt->stream_index]++;
        }
        ret = ConvertAVPacketToSample(avStream, buffer, bufferInfo, flag, pkt);
    } else {
        AVCODEC_LOGE("read frame failed, ffmpeg error: %{public}d", ret);
        ret = AVCS_ERR_DEMUXER_FAILED;
    }
    av_packet_free(&pkt);
    return ret;
}

int64_t FFmpegDemuxerPlugin::CalculateTimeByFrameIndex(AVStream* avStream, int keyFrameIdx)
{
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(58, 78, 0)
    return avformat_index_get_entry(avStream, keyFrameIdx)->timestamp;
#elif LIBAVFORMAT_VERSION_INT == AV_VERSION_INT(58, 76, 100)
    return avStream->index_entries[keyFrameIdx].timestamp;
#elif LIBAVFORMAT_VERSION_INT > AV_VERSION_INT(58, 64, 100)
    return avStream->internal->index_entries[keyFrameIdx].timestamp;
#else
    return avStream->index_entries[keyFrameIdx].timestamp;
#endif
}

int32_t FFmpegDemuxerPlugin::SeekToTime(int64_t mSeconds, AVSeekMode mode)
{
    if (!seekModeToFFmpegSeekFlags.count(mode)) {
        AVCODEC_LOGE("unsupported seek mode: %{public}d", static_cast<uint32_t>(mode));
        return AVCS_ERR_SEEK_FAILED;
    }
    int flags = seekModeToFFmpegSeekFlags.at(mode);
    std::vector<uint32_t> trackVec;
    if (selectedTrackIds_.empty()) {
        for (uint32_t trackIndex = 0; trackIndex < formatContext_.get()->nb_streams; trackIndex++) {
            trackVec.push_back(trackIndex);
        }
    } else {
        trackVec = selectedTrackIds_;
    }
    
    for (size_t i = 0; i < trackVec.size(); i++) {
        int trackIndex = trackVec[i];
        auto avStream = formatContext_->streams[trackIndex];
        int64_t ffTime = ConvertTimeToFFmpeg(mSeconds*1000*1000, avStream->time_base);
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (ffTime > avStream->duration) {
                AVCODEC_LOGE("ERROR: Seek to timestamp = %{public}" PRId64 " failed, max = %{public}" PRId64,
                             ffTime, avStream->duration);
                return AVCS_ERR_SEEK_FAILED;
            }
            if (AvTime2Ms(ConvertTimeFromFFmpeg(avStream->duration, avStream->time_base) - mSeconds) <= 100
                && mode == AVSeekMode::SEEK_MODE_NEXT_SYNC) {
                flags = seekModeToFFmpegSeekFlags.at(AVSeekMode::SEEK_MODE_PREVIOUS_SYNC);
            }
            if (ffTime < 0) {
                AVCODEC_LOGW("invalid ffmpeg time: %{public}" PRId64 " ms, will be set to 0", ffTime);
                ffTime = 0;
            }
            int keyFrameIdx = av_index_search_timestamp(avStream, ffTime, flags);
            if (keyFrameIdx < 0) {
                flags = seekModeToFFmpegSeekFlags.at(AVSeekMode::SEEK_MODE_CLOSEST_SYNC);
                keyFrameIdx = av_index_search_timestamp(avStream, ffTime, flags);
            }
            if (keyFrameIdx >= 0) {
                ffTime = CalculateTimeByFrameIndex(avStream, keyFrameIdx);
            }
        }
        int64_t realSeekTime = ConvertTimeFromFFmpeg(ffTime, avStream->time_base);
        AVCODEC_LOGD("realSeekTime: %{public}" PRId64, realSeekTime);
        AVCODEC_LOGD("seek param: trackIndex=%{public}d, ffTime=%{public}" PRId64 ", \
                     realSeekTime=%{public}" PRId64 ", flags=%{public}d",
                     trackIndex, ffTime, realSeekTime, flags);

        auto rtv = av_seek_frame(formatContext_.get(), trackIndex, ffTime, flags);
        if (rtv < 0) {
            AVCODEC_LOGE("seek failed, return value: %{public}d", rtv);
            return AVCS_ERR_SEEK_FAILED;
        }
    }
    return AVCS_ERR_OK;
}
} // FFmpeg
} // Plugin
} // Media
} // OHOS