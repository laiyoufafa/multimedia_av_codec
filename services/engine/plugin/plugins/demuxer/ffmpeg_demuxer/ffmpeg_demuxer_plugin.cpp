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
#include <cinttypes>
#include "securec.h"
#include "avcodec_errors.h"
#include "native_avcodec_base.h"
#include "plugin_definition.h"
#include "avcodec_log.h"
#include "avcodec_dfx.h"
#include "ffmpeg_demuxer_plugin.h"

#if defined(LIBAVFORMAT_VERSION_INT) && defined(LIBAVFORMAT_VERSION_INT)
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 78, 0) and LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(58, 64, 100)
#if LIBAVFORMAT_VERSION_INT != AV_VERSION_INT(58, 76, 100)
#include "libavformat/internal.h"
#endif
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
static const std::map<AVSeekMode, int32_t>  g_seekModeToFFmpegSeekFlags = {
    { AVSeekMode::SEEK_MODE_PREVIOUS_SYNC, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD },
    { AVSeekMode::SEEK_MODE_NEXT_SYNC, AVSEEK_FLAG_FRAME },
    { AVSeekMode::SEEK_MODE_CLOSEST_SYNC, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_ANY }
};

constexpr int32_t TIME_INTERNAL = 100;
constexpr int32_t MAX_CONFIDENCE = 100;

int32_t Sniff(const std::string& pluginName)
{
    return MAX_CONFIDENCE;
}

Status RegisterDemuxerPlugins(const std::shared_ptr<Register>& reg)
{
    DemuxerPluginDef def;
    constexpr int32_t rankScore = 100;
    def.name = "ffmpegDemuxer";
    def.description = "ffmpeg demuxer";
    def.rank = rankScore;
    def.creator = []() -> std::shared_ptr<DemuxerPlugin> {
        return std::make_shared<FFmpegDemuxerPlugin>();
    };
    def.sniffer = Sniff;
    return reg->AddPlugin(def);
}

PLUGIN_DEFINITION(FFmpegDemuxer, LicenseType::GPL, RegisterDemuxerPlugins, [] {})
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
    AVCodecBufferFlag flags = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
    if (pkt->flags == 0x0001) {
        flags = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_SYNC_FRAME;
    } else {
        flags = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
    }

    return flags;
}

int32_t FFmpegDemuxerPlugin::Create(uintptr_t sourceAddr)
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::Create");
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
    :blockQueue_("cache_que")
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::FFmpegDemuxerPlugin");
}

FFmpegDemuxerPlugin::~FFmpegDemuxerPlugin()
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::~FFmpegDemuxerPlugin");
    selectedTrackIds_.clear();
}

int32_t FFmpegDemuxerPlugin::SetBitStreamFormat()
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::SetBitStreamFormat");
    uint32_t trackCount = formatContext_->nb_streams;
    for (uint32_t i = 0; i < trackCount; i++) {
        if (formatContext_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            InitBitStreamContext(*(formatContext_->streams[i]));
        }
    }
    return AVCS_ERR_OK;
}

int32_t FFmpegDemuxerPlugin::SelectTrackByID(uint32_t trackIndex)
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::SelectTrackByID: trackIndex=%{public}u", trackIndex);
    std::unique_lock<std::mutex> lock(mutex_);
    std::stringstream selectedTracksString;
    for (const auto &index : selectedTrackIds_) {
        selectedTracksString << index << " | ";
    }
    AVCODEC_LOGI("Total track in file: %{public}d | add track index: %{public}u",
        formatContext_.get()->nb_streams, trackIndex);
    AVCODEC_LOGI("Selected tracks in file: %{public}s.", selectedTracksString.str().c_str());

    if (trackIndex >= static_cast<uint32_t>(formatContext_.get()->nb_streams)) {
        AVCODEC_LOGE("trackIndex is invalid! Just have %{public}d tracks in file", formatContext_.get()->nb_streams);
        return AVCS_ERR_INVALID_VAL;
    }
    auto index = std::find_if(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                              [trackIndex](uint32_t selectedId) {return trackIndex == selectedId;});
    if (index == selectedTrackIds_.end()) {
        selectedTrackIds_.push_back(trackIndex);
        if (trackIsEnd_.count(trackIndex) == 0) {
            trackIsEnd_.insert(std::make_pair(trackIndex, false));
        }
        return blockQueue_.AddTrackQueue(trackIndex);
    } else {
        AVCODEC_LOGW("track %{public}u is already in selected list!", trackIndex);
    }
    return AVCS_ERR_OK;
}

int32_t FFmpegDemuxerPlugin::UnselectTrackByID(uint32_t trackIndex)
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::UnselectTrackByID: trackIndex=%{public}u", trackIndex);
    std::unique_lock<std::mutex> lock(mutex_);
    std::stringstream selectedTracksString;
    for (const auto &index : selectedTrackIds_) {
        selectedTracksString << index << " | ";
    }
    AVCODEC_LOGI("Selected track in file: %{public}s | remove track: %{public}u",
        selectedTracksString.str().c_str(), trackIndex);

    auto index = std::find_if(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                              [trackIndex](uint32_t selectedId) {return trackIndex == selectedId; });
    if (index != selectedTrackIds_.end()) {
        selectedTrackIds_.erase(index);
        return blockQueue_.RemoveTrackQueue(trackIndex);
    } else {
        AVCODEC_LOGW("Unselect track failed, track %{public}u is not in selected list!", trackIndex);
    }
    return AVCS_ERR_OK;
}

std::vector<uint32_t> FFmpegDemuxerPlugin::GetSelectedTrackIds()
{
    AVCODEC_LOGD("FFmpegDemuxerPlugin::GetSelectedTrackIds");
    std::vector<uint32_t> trackIds;
    trackIds = selectedTrackIds_;
    return trackIds;
}

bool FFmpegDemuxerPlugin::IsInSelectedTrack(uint32_t trackIndex)
{
    return std::any_of(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                       [trackIndex](uint32_t id) { return id == trackIndex; });
}

void FFmpegDemuxerPlugin::InitBitStreamContext(const AVStream& avStream)
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::InitBitStreamContext");
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
    (void)av_bsf_send_packet(avbsfContext_.get(), &pkt);
    (void)av_packet_unref(&pkt);
    (void)av_bsf_receive_packet(avbsfContext_.get(), &pkt);
}

int32_t FFmpegDemuxerPlugin::ConvertAVPacketToSample(AVStream* avStream, std::shared_ptr<AVSharedMemory> sample,
    AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag, std::shared_ptr<SamplePacket> samplePacket)
{
    if (samplePacket == nullptr || samplePacket->pkt == nullptr) {
        return AVCS_ERR_INVALID_OPERATION;
    }
    uint64_t frameSize = 0;
    if (avStream->duration <= (samplePacket->pkt->pts + samplePacket->pkt->duration)) {
        SetEndStatus(samplePacket->pkt->stream_index);
    }
    bufferInfo.presentationTimeUs = AvTime2Ms(ConvertTimeFromFFmpeg(samplePacket->pkt->pts, avStream->time_base));
    flag = ConvertFlagsFromFFmpeg(samplePacket->pkt, avStream);
    CHECK_AND_RETURN_RET_LOG(samplePacket->pkt->size >= 0, AVCS_ERR_DEMUXER_FAILED,
        "the sample size is must be positive");
    if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        frameSize = static_cast<uint64_t>(samplePacket->pkt->size);
    } else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        if (avStream->codecpar->codec_id == AV_CODEC_ID_RAWVIDEO) {
            AVCODEC_LOGE("unsupport raw video");
            return AVCS_ERR_UNSUPPORT_STREAM;
        }
        if (avbsfContext_) {
            ConvertAvcOrHevcToAnnexb(*(samplePacket->pkt));
        }
        frameSize = static_cast<uint64_t>(samplePacket->pkt->size);
    } else {
        AVCODEC_LOGE("unsupport stream type");
        return AVCS_ERR_UNSUPPORT_VID_PARAMS;
    }
  
    bufferInfo.size = static_cast<int32_t>(frameSize);
    bufferInfo.offset = 0;
    auto copyFrameSize = static_cast<uint64_t>(frameSize) - samplePacket->offset;
    auto copySize = copyFrameSize;
    if (copySize > static_cast<uint64_t>(sample->GetSize())) {
        copySize = static_cast<uint64_t>(sample->GetSize());
    }
    errno_t rs = memset_s(sample->GetBase(), copySize, 0, copySize);
    CHECK_AND_RETURN_RET_LOG(rs == EOK, AVCS_ERR_UNKNOWN, "memset_s failed");
    errno_t rc = memcpy_s(sample->GetBase(), copySize, samplePacket->pkt->data+samplePacket->offset, copySize);
    CHECK_AND_RETURN_RET_LOG(rc == EOK, AVCS_ERR_UNKNOWN, "memcpy_s failed");

    if (copySize != copyFrameSize) {
        samplePacket->offset += copySize;
        return AVCS_ERR_NO_MEMORY;
    }
    av_packet_free(&(samplePacket->pkt));
    return AVCS_ERR_OK;
}

int32_t FFmpegDemuxerPlugin::GetNextPacket(uint32_t trackIndex, std::shared_ptr<SamplePacket> *samplePacket)
{
    int32_t ffmpegRet;
    do {
        AVPacket* pkt = av_packet_alloc();
        ffmpegRet = av_read_frame(formatContext_.get(), pkt);
        CHECK_AND_RETURN_RET_LOG(pkt->stream_index >= 0, AVCS_ERR_DEMUXER_FAILED, "the stream_index must be positive");
        uint32_t streamIndex = static_cast<uint32_t>(pkt->stream_index);
        if (ffmpegRet >= 0 && IsInSelectedTrack(streamIndex)) {
            std::shared_ptr<SamplePacket> cacheSamplePacket = std::make_shared<SamplePacket>();
            cacheSamplePacket->offset = 0;
            cacheSamplePacket->pkt = pkt;
            if (streamIndex == trackIndex) {
                *samplePacket = cacheSamplePacket;
                break;
            }
            blockQueue_.Push(streamIndex, cacheSamplePacket);
        }
    } while (ffmpegRet >= 0);
    return ffmpegRet;
}

int32_t FFmpegDemuxerPlugin::ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
                                        AVCodecBufferInfo &info, AVCodecBufferFlag &flag)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (selectedTrackIds_.empty() || std::count(selectedTrackIds_.begin(), selectedTrackIds_.end(), trackIndex) == 0) {
        AVCODEC_LOGE("read frame failed, track %{public}u has not been selected", trackIndex);
        return AVCS_ERR_INVALID_OPERATION;
    }
    AVStream* avStream = formatContext_->streams[trackIndex];
    if (blockQueue_.HasCache(trackIndex)) {
        int32_t ret = ConvertAVPacketToSample(avStream, sample, info, flag, blockQueue_.Front(trackIndex));
        if (ret == AVCS_ERR_OK) {
            blockQueue_.Pop(trackIndex);
        }
        return ret;
    }
    if (trackIsEnd_.count(trackIndex) != 0 && trackIsEnd_[trackIndex]) {
        SetEosBufferInfo(info, flag);
        return AVCS_ERR_OK;
    }

    std::shared_ptr<SamplePacket> samplePacket = std::make_shared<SamplePacket>();
    int32_t ffmpegRet = GetNextPacket(trackIndex, &samplePacket);
    if (ffmpegRet == AVERROR_EOF) {
        SetEosBufferInfo(info, flag);
        return AVCS_ERR_OK;
    }
    if (ffmpegRet < 0) {
        AVCODEC_LOGE("read frame failed, ffmpeg error:%{public}d", ffmpegRet);
        av_packet_free(&(samplePacket->pkt));
        return AVCS_ERR_DEMUXER_FAILED;
    }
    int32_t ret = ConvertAVPacketToSample(avStream, sample, info, flag, samplePacket);
    if (ret == AVCS_ERR_NO_MEMORY) {
        blockQueue_.Push(trackIndex, samplePacket);
    }
    return ret;
}

int64_t FFmpegDemuxerPlugin::CalculateTimeByFrameIndex(AVStream* avStream, int keyFrameIdx)
{
#if defined(LIBAVFORMAT_VERSION_INT) && defined(LIBAVFORMAT_VERSION_INT)
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(58, 78, 0)
    return avformat_index_get_entry(avStream, keyFrameIdx)->timestamp;
#elif LIBAVFORMAT_VERSION_INT == AV_VERSION_INT(58, 76, 100)
    return avStream->index_entries[keyFrameIdx].timestamp;
#elif LIBAVFORMAT_VERSION_INT > AV_VERSION_INT(58, 64, 100)
    return avStream->internal->index_entries[keyFrameIdx].timestamp;
#else
    return avStream->index_entries[keyFrameIdx].timestamp;
#endif
#else
    return avStream->index_entries[keyFrameIdx].timestamp
#endif
}

int32_t FFmpegDemuxerPlugin::SeekToTime(int64_t millisecond, AVSeekMode mode)
{
    AVCODEC_LOGI("FFmpegDemuxerPlugin::SeekToTime: millisecond=%{public}" PRId64 ", mode=%{public}d",
        millisecond, mode);
    std::unique_lock<std::mutex> lock(mutex_);
    if (!g_seekModeToFFmpegSeekFlags.count(mode)) {
        AVCODEC_LOGE("unsupported seek mode: %{public}d", static_cast<uint32_t>(mode));
        return AVCS_ERR_INVALID_OPERATION;
    }
    int flags = g_seekModeToFFmpegSeekFlags.at(mode);
    if (selectedTrackIds_.empty()) {
        AVCODEC_LOGW("no track has been selected");
        return AVCS_ERR_INVALID_OPERATION;
    }
    for (size_t i = 0; i < selectedTrackIds_.size(); i++) {
        int trackIndex = static_cast<int>(selectedTrackIds_[i]);
        auto avStream = formatContext_->streams[trackIndex];
        int64_t ffTime = ConvertTimeToFFmpeg(millisecond*1000*1000, avStream->time_base);
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (ffTime > avStream->duration) {
                AVCODEC_LOGE("seek to timestamp = %{public}" PRId64 " failed, max = %{public}" PRId64,
                             ffTime, avStream->duration);
                return AVCS_ERR_INVALID_OPERATION;
            }
            if (AvTime2Ms(ConvertTimeFromFFmpeg(avStream->duration, avStream->time_base) - millisecond) <= TIME_INTERNAL
                && mode == AVSeekMode::SEEK_MODE_NEXT_SYNC) {
                flags = g_seekModeToFFmpegSeekFlags.at(AVSeekMode::SEEK_MODE_PREVIOUS_SYNC);
            }
            if (ffTime < 0) {
                AVCODEC_LOGW("invalid ffmpeg time: %{public}" PRId64 " ms, will be set to 0", ffTime);
                ffTime = 0;
            }
            int keyFrameIdx = av_index_search_timestamp(avStream, ffTime, flags);
            if (keyFrameIdx < 0) {
                flags = g_seekModeToFFmpegSeekFlags.at(AVSeekMode::SEEK_MODE_CLOSEST_SYNC);
                keyFrameIdx = av_index_search_timestamp(avStream, ffTime, flags);
            }
            if (keyFrameIdx >= 0) {
                ffTime = CalculateTimeByFrameIndex(avStream, keyFrameIdx);
            }
        }
        auto rtv = av_seek_frame(formatContext_.get(), trackIndex, ffTime, flags);
        if (rtv < 0) {
            AVCODEC_LOGE("seek failed, return value: ffmpeg error:%{public}d", rtv);
            return AVCS_ERR_SEEK_FAILED;
        }
    }
    ResetStatus();
    return AVCS_ERR_OK;
}

void FFmpegDemuxerPlugin::ResetStatus()
{
    for (size_t i = 0; i < selectedTrackIds_.size(); ++i) {
        blockQueue_.ResetQueue(selectedTrackIds_[i]);
        if (trackIsEnd_.count(selectedTrackIds_[i]) != 0) {
            trackIsEnd_[selectedTrackIds_[i]] = false;
        }
    }
}

void FFmpegDemuxerPlugin::SetEndStatus(uint32_t trackIndex)
{
    if (trackIsEnd_.count(trackIndex) != 0) {
        trackIsEnd_[trackIndex] = true;
    } else {
        trackIsEnd_.insert(std::make_pair(trackIndex, true));
    }
}

void FFmpegDemuxerPlugin::SetEosBufferInfo(AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag)
{
    bufferInfo.presentationTimeUs = 0;
    bufferInfo.size = 0;
    bufferInfo.offset = 0;
    flag = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_EOS;
}
} // FFmpeg
} // Plugin
} // Media
} // OHOS
