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
#include "source.h"
#include <iostream>
#include <dlfcn.h>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include "avcodec_errors.h"
#include "avcodec_dfx.h"
#include "media_description.h"
#include "avcodec_log.h"
#include "avcodec_info.h"
#include "avcodec_common.h"
#include "media_description.h"
#include "media_source.h"
#include "format.h"

static std::string g_libFileHead = "libhistreamer_plugin_";
static std::string g_fileSeparator = "/";
static std::string g_libFileTail = ".z.so";

namespace OHOS {
namespace Media {
namespace Plugin {
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "Source"};

    inline bool FileIsExists (const char* name)
    {
        struct stat buffer;
        return (stat(name, &buffer) == 0);
    }

    static std::map<std::string, std::string> g_pluginMap = {
        {"http", "libhistreamer_plugin_HttpSource.z.so"},
        {"https", "libhistreamer_plugin_HttpSource.z.so"},
        {"fd", "libhistreamer_plugin_FileFdSource.z.so"}
    };

    static std::map<AVCodecID, std::string_view> g_codecIdToMime = {
        {AV_CODEC_ID_MP3, CodecMimeType::AUDIO_MPEG},
        {AV_CODEC_ID_FLAC, CodecMimeType::AUDIO_FLAC},
        {AV_CODEC_ID_PCM_S16LE, CodecMimeType::AUDIO_RAW},
        {AV_CODEC_ID_AAC, CodecMimeType::AUDIO_AAC},
        {AV_CODEC_ID_VORBIS, CodecMimeType::AUDIO_VORBIS},
        {AV_CODEC_ID_OPUS, CodecMimeType::AUDIO_OPUS},
        {AV_CODEC_ID_AMR_NB, CodecMimeType::AUDIO_AMR_NB},
        {AV_CODEC_ID_AMR_WB, CodecMimeType::AUDIO_AMR_WB},
        {AV_CODEC_ID_H264, CodecMimeType::VIDEO_AVC},
        {AV_CODEC_ID_MPEG4, CodecMimeType::VIDEO_MPEG4},
        {AV_CODEC_ID_MJPEG, CodecMimeType::IMAGE_JPG},
        {AV_CODEC_ID_PNG, CodecMimeType::IMAGE_PNG},
        {AV_CODEC_ID_BMP, CodecMimeType::IMAGE_BMP},
        {AV_CODEC_ID_H263, CodecMimeType::VIDEO_H263},
        {AV_CODEC_ID_MPEG2TS, CodecMimeType::VIDEO_MPEG2},
        {AV_CODEC_ID_MPEG2VIDEO, CodecMimeType::VIDEO_MPEG2},
        {AV_CODEC_ID_HEVC, CodecMimeType::VIDEO_HEVC},
        {AV_CODEC_ID_VP8, CodecMimeType::VIDEO_VP8},
        {AV_CODEC_ID_VP9, CodecMimeType::VIDEO_VP9},
    };

    static std::map<AVMediaType, int32_t> g_convertFfmpegType = {
        {AVMEDIA_TYPE_VIDEO, MEDIA_TYPE_VID},
        {AVMEDIA_TYPE_AUDIO, MEDIA_TYPE_AUD},
        {AVMEDIA_TYPE_SUBTITLE, MEDIA_TYPE_SUBTITLE},
    };

    std::map<std::string, std::shared_ptr<AVInputFormat>> g_pluginInputFormat;

    std::string GetUriSuffix(const std::string& uri)
    {
        AVCODEC_LOGD("GetUriSuffix, input: uri=%{public}s", uri.c_str());
        std::string suffix;
        auto const pos = uri.find_last_of('.');
        if (pos != std::string::npos) {
            suffix = uri.substr(pos + 1);
        }
        AVCODEC_LOGD("suffix: %{public}s", suffix.c_str());
        return suffix;
    }

    int32_t ParseProtocol(const std::string& uri, std::string& protocol)
    {
        AVCODEC_LOGD("ParseProtocol, input: uri=%{public}s, protocol=%{public}s", uri.c_str(), protocol.c_str());
        int32_t ret;
        auto const pos = uri.find("://");
        if (pos != std::string::npos) {
            auto prefix = uri.substr(0, pos);
            protocol.append(prefix);
            ret = AVCS_ERR_OK;
        }
        
        if (protocol.empty()) {
            AVCODEC_LOGE("ERROR:Invalid protocol: %{public}s", protocol.c_str());
            ret = AVCS_ERR_INVALID_OPERATION;
        }
        return ret;
    }

    RegisterFunc OpenFilePlugin(const std::string& path, const std::string& name, void* handler)
    {
        AVCODEC_LOGD("OpenFilePlugin, input: path=%{public}s, name=%{public}s", path.c_str(), name.c_str());
        if (FileIsExists(path.c_str())) {
            handler = ::dlopen(path.c_str(), RTLD_NOW);
            if (handler == nullptr) {
                AVCODEC_LOGE("dlopen failed due to %{public}s", ::dlerror());
            }
        }
        if (handler) {
            std::string registerFuncName = "register_" + name;
            RegisterFunc registerFunc = nullptr;
            registerFunc = (RegisterFunc)(::dlsym(handler, registerFuncName.c_str()));
            if (registerFunc) {
                return registerFunc;
            } else {
                AVCODEC_LOGE("register is not found in %{public}s", registerFuncName.c_str());
            }
        } else {
            AVCODEC_LOGE("dlopen failed: %{public}s", path.c_str());
        }
        return {};
    }

    bool IsInputFormatSupported(const char* name)
    {
        if (!strcmp(name, "audio_device") || !strncmp(name, "image", 5) ||
            !strcmp(name, "mjpeg") || !strcmp(name, "redir") || !strncmp(name, "u8", 2) ||
            !strncmp(name, "u16", 3) || !strncmp(name, "u24", 3) ||
            !strncmp(name, "u32", 3) ||
            !strncmp(name, "s8", 2) || !strncmp(name, "s16", 3) ||
            !strncmp(name, "s24", 3) ||
            !strncmp(name, "s32", 3) || !strncmp(name, "f32", 3) ||
            !strncmp(name, "f64", 3) ||
            !strcmp(name, "mulaw") || !strcmp(name, "alaw")) {
            return false;
        }
        if (!strcmp(name, "sdp") || !strcmp(name, "rtsp") || !strcmp(name, "applehttp")) {
            return false;
        }
        return true;
    }

    void ReplaceDelimiter(const std::string& delmiters, char newDelimiter, std::string& str)
    {
        for (auto it = str.begin(); it != str.end(); ++it) {
            if (delmiters.find(newDelimiter) != std::string::npos) {
                *it = newDelimiter;
            }
        }
    };
}

Status SourceRegister::AddPlugin(const PluginDefBase& def)
{
    auto& tmpDef = (SourcePluginDef&) def;
    sourcePlugin = (tmpDef.creator)(tmpDef.name);
    return Status::OK;
}

Status SourceRegister::AddPackage(const PackageDef& def)
{
    packageDef = std::make_shared<PackageDef>(def);
    return Status::OK;
}
constexpr size_t DEFAULT_READ_SIZE = 4096;

Source::Source()
    :formatContext_(nullptr), inputFormat_(nullptr)
{
    AVCODEC_LOGI("Source::Source is on call");
}

Source::~Source()
{
    formatContext_ = nullptr;
    inputFormat_ = nullptr;
    sourcePlugin_ = nullptr;
    register_ = nullptr;
    avioContext_ = nullptr;
    ::dlclose(handler_);
    AVCODEC_LOGI("Source::~Source is on call");
}

int32_t Source::GetTrackCount(uint32_t &trackCount)
{
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "call getTrackcount failed, because create source failed!");
    trackCount = static_cast<uint32_t>(formatContext_->nb_streams);
    return AVCS_ERR_OK;
}

void Source::GetStringFormatFromMetadata(std::string key, std::string_view formatName, Format &format)
{
    AVDictionaryEntry *valPtr = nullptr;
    valPtr = av_dict_get(formatContext_->metadata, key.c_str(), nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss %{public}s info in file", key.c_str());
    } else {
        bool ret = format.PutStringValue(formatName, valPtr->value);
        if (!ret) {
            AVCODEC_LOGW("Put track info failed: miss %{public}s info in file", key.c_str());
        }
    }
}

int32_t Source::GetSourceFormat(Format &format)
{
    AVCODEC_LOGI("Source::GetFormat is on call");
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_INVALID_OPERATION, "formatContext_ is nullptr!");
    Format::FormatDataMap formatMap = format.GetFormatMap();

    GetStringFormatFromMetadata("title", AVSourceFormat::SOURCE_TITLE, format);
    GetStringFormatFromMetadata("artist", AVSourceFormat::SOURCE_ARTIST, format);
    GetStringFormatFromMetadata("album", AVSourceFormat::SOURCE_ALBUM, format);
    GetStringFormatFromMetadata("album_artist", AVSourceFormat::SOURCE_ALBUM_ARTIST, format);
    GetStringFormatFromMetadata("date", AVSourceFormat::SOURCE_DATE, format);
    GetStringFormatFromMetadata("comment", AVSourceFormat::SOURCE_COMMENT, format);
    GetStringFormatFromMetadata("genre", AVSourceFormat::SOURCE_GENRE, format);
    GetStringFormatFromMetadata("copyright", AVSourceFormat::SOURCE_COPYRIGHT, format);
    GetStringFormatFromMetadata("language", AVSourceFormat::SOURCE_LANGUAGE, format);
    GetStringFormatFromMetadata("description", AVSourceFormat::SOURCE_DESCRIPTION, format);
    GetStringFormatFromMetadata("media_type", AVSourceFormat::SOURCE_TYPE, format);
    GetStringFormatFromMetadata("lyrics", AVSourceFormat::SOURCE_LYRICS, format);

    bool ret = format.PutLongValue(MediaDescriptionKey::MD_KEY_DURATION, formatContext_->duration);
    if (!ret) {
        AVCODEC_LOGW("Put track info failed: miss album_artist info in file");
    }

    ret = format.PutIntValue(
        MediaDescriptionKey::MD_KEY_TRACK_COUNT, static_cast<uint32_t>(formatContext_->nb_streams));
    if (!ret) {
        AVCODEC_LOGW("Put track info failed: miss track count info in file");
    }
    return AVCS_ERR_OK;
}


int32_t Source::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    AVCODEC_LOGI("Source::GetTrackFormat is on call");
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_INVALID_OPERATION,
                             "GetTrackFormat failed, formatContext_ is nullptr!");
    if (trackIndex < 0 || trackIndex >= static_cast<uint32_t>(formatContext_->nb_streams)) {
        AVCODEC_LOGE("trackIndex is invalid!");
        return AVCS_ERR_INVALID_VAL;
    }
    auto avStream = formatContext_->streams[trackIndex];

    int32_t media_type = -1;
    if (g_convertFfmpegType.count(avStream->codecpar->codec_type) > 0) {
        media_type = static_cast<int32_t>(g_convertFfmpegType[avStream->codecpar->codec_type]);
    }
    bool ret = format.PutIntValue(MediaDescriptionKey::MD_KEY_TRACK_TYPE, media_type);
    if (!ret) {
        AVCODEC_LOGW("Get track info failed:  miss track type info in track %{public}d", trackIndex);
    }

    if (g_codecIdToMime.count(avStream->codecpar->codec_id) != 0) {
        ret = format.PutStringValue(
            MediaDescriptionKey::MD_KEY_CODEC_MIME, g_codecIdToMime[avStream->codecpar->codec_id]);
    } else {
        ret = false;
    }
    if (!ret) {
        AVCODEC_LOGW("Get track info failed:  miss mime type info in track %{public}d", trackIndex);
    }

    if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        ret = format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, avStream->codecpar->width);
        if (!ret) {
            AVCODEC_LOGW("Get track info failed:  miss width info in track %{public}d", trackIndex);
        }
        ret = format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, avStream->codecpar->height);
        if (!ret) {
            AVCODEC_LOGW("Get track info failed:  miss height info in track %{public}d", trackIndex);
        }
    } else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        ret = format.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, avStream->codecpar->sample_rate);
        if (!ret) {
            AVCODEC_LOGW("Get track info failed:  miss sample rate info in track %{public}d", trackIndex);
        }
        ret = format.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, avStream->codecpar->channels);
        if (!ret) {
            AVCODEC_LOGW("Get track info failed:  miss channel count info in track %{public}d", trackIndex);
        }
        ret = format.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, avStream->codecpar->bit_rate);
        if (!ret) {
            AVCODEC_LOGW("Get track info failed:  miss bitrate info in track %{public}d", trackIndex);
        }
        if (avStream->codecpar->codec_id == AV_CODEC_ID_AAC) {
            ret = format.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, 1);
            if (!ret) {
                AVCODEC_LOGW("Get track info failed:  miss bitrate info in track %{public}d", trackIndex);
            }
        }
        if (avStream->codecpar->codec_id == AV_CODEC_ID_AAC_LATM) {
            ret = format.PutIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, 0);
            if (!ret) {
                AVCODEC_LOGW("Get track info failed:  miss bitrate info in track %{public}d", trackIndex);
            }
        }
    }
    return AVCS_ERR_OK;
}

int32_t Source::Create(std::string& uri)
{
    AVCODEC_LOGI("Source::Create is called");
    int32_t ret = LoadDynamicPlugin(uri);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
                             "create source failed when load source plugin!");
    std::shared_ptr<MediaSource> mediaSource = std::make_shared<MediaSource>(uri);
    AVCODEC_LOGD("mediaSource Init: %{public}s", mediaSource->GetSourceUri().c_str());
    if (sourcePlugin_ == nullptr) {
        AVCODEC_LOGE("load sourcePlugin_ fail !");
        return AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED;
    }
    Status pluginRet = sourcePlugin_->SetSource(mediaSource);
    size_t size;
    sourcePlugin_->GetSize(size);
    AVCODEC_LOGD("mp4 fileSize: %{public}d", size);
    CHECK_AND_RETURN_RET_LOG(pluginRet == Status::OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
                             "create source failed when set data source for plugin!");
    ret = LoadInputFormatList();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
                             "create source failed when load demuxerlist!");
    ret = SniffInputFormat(uri);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
                             "create source failed when find input format!");
    CHECK_AND_RETURN_RET_LOG(inputFormat_ != nullptr, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
                             "create source failed when find input format, cannnot match any input format!");
    ret = InitAVFormatContext();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
                             "create source failed when parse source info!");
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
                             "create source failed when init AVFormatContext!");
    return AVCS_ERR_OK;
}


int32_t Source::LoadInputFormatList()
{
    const AVInputFormat* plugin = nullptr;
    constexpr size_t strMax = 4;
    void* i = nullptr;
    while ((plugin = av_demuxer_iterate(&i))) {
        if (plugin->long_name != nullptr) {
            if (!strncmp(plugin->long_name, "pcm ", strMax)) {
                continue;
            }
        }
        if (!IsInputFormatSupported(plugin->name)) {
            continue;
        }
        std::string pluginName = "avdemux_" + std::string(plugin->name);
        ReplaceDelimiter(".,|-<> ", '_', pluginName);
        g_pluginInputFormat[pluginName] =
            std::shared_ptr<AVInputFormat>(const_cast<AVInputFormat*>(plugin), [](void*) {});
    }
    if (g_pluginInputFormat.empty()) {
        AVCODEC_LOGW("cannot load any format demuxer");
        return AVCS_ERR_INVALID_OPERATION;
    }
    return AVCS_ERR_OK;
}


int32_t Source::LoadDynamicPlugin(const std::string& path)
{
    AVCODEC_LOGI("LoadDynamicPlugin: %{public}s", path.c_str());
    std::string protocol;
    if (ParseProtocol(path, protocol) != AVCS_ERR_OK) {
        AVCODEC_LOGE("Couldn't find valid protocol for %{public}s", path.c_str());
        return AVCS_ERR_INVALID_OPERATION;
    }
    if (g_pluginMap.count(protocol) == 0) {
        AVCODEC_LOGE("Unsupport protocol: %{public}s", protocol.c_str());
        return AVCS_ERR_INVALID_OPERATION;
    }
    std::string libFileName = g_pluginMap[protocol];
    std::string filePluginPath = OH_FILE_PLUGIN_PATH + g_fileSeparator + libFileName;
    std::string pluginName =
        libFileName.substr(g_libFileHead.size(), libFileName.size() - g_libFileHead.size() - g_libFileTail.size());
    RegisterFunc registerFunc = OpenFilePlugin(filePluginPath, pluginName, handler_);
    if (registerFunc) {
        register_ = std::make_shared<SourceRegister>();
        registerFunc(register_);
        sourcePlugin_ = register_->sourcePlugin;
        AVCODEC_LOGD("regist source plugin successful");
        return AVCS_ERR_OK;
    } else {
        AVCODEC_LOGD("regist source plugin failed, sourcePlugin path: %{public}s", filePluginPath.c_str());
        return AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED;
    }
}

int32_t Source::GuessInputFormat(const std::string& uri, std::shared_ptr<AVInputFormat> &bestInputFormat)
{
    std::string uriSuffix = GetUriSuffix(uri);
    if (uriSuffix.empty()) {
        AVCODEC_LOGW("can't found suffix ,please check the file %{public}s's suffix", uri.c_str());
        return AVCS_ERR_INVALID_OPERATION;
    }
    std::map<std::string, std::shared_ptr<AVInputFormat>>::iterator iter;
    for (iter = g_pluginInputFormat.begin(); iter != g_pluginInputFormat.end(); iter++) {
        std::shared_ptr<AVInputFormat> inputFormat = iter->second;
        if (uriSuffix == inputFormat->name) {
            bestInputFormat = inputFormat;
            AVCODEC_LOGD("find input fromat successful: %{public}s", inputFormat->name);
            break;
        }
    }
    return AVCS_ERR_OK;
}

int32_t Source::SniffInputFormat(const std::string& uri)
{
    size_t bufferSize = DEFAULT_READ_SIZE;
    size_t fileSize = 0;
    if (!static_cast<int>(sourcePlugin_->GetSize(fileSize))) {
        bufferSize = (bufferSize < fileSize) ? bufferSize : fileSize;
    }
    std::vector<uint8_t> buff(bufferSize);
    auto bufferInfo = std::make_shared<Buffer>();
    auto bufferMemory = bufferInfo->WrapMemory(buff.data(), bufferSize, 0);
    if (bufferMemory==nullptr) {
        return AVCS_ERR_NO_MEMORY;
    }
    auto ret = static_cast<int>(sourcePlugin_->Read(bufferInfo, bufferSize));
    CHECK_AND_RETURN_RET_LOG(ret >= 0, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
        "create source service failed when probe source format!");
    AVProbeData probeData = {"", buff.data(), static_cast<int>(bufferSize), ""};
    constexpr int probThresh = 50;
    int maxProb = 0;
    std::map<std::string, std::shared_ptr<AVInputFormat>>::iterator iter;
    for (iter = g_pluginInputFormat.begin(); iter != g_pluginInputFormat.end(); iter++) {
        std::string vtype = iter->first;
        std::shared_ptr<AVInputFormat> inputFormat = iter -> second;
        if (inputFormat->read_probe) {
            auto prob = inputFormat->read_probe(&probeData);
            if (prob > probThresh) {
                inputFormat_ = inputFormat;
                break;
            }
            if (prob > maxProb) {
                maxProb = prob;
                inputFormat_ = inputFormat;
            }
        }
    }
    if (inputFormat_ == nullptr) {
        AVCODEC_LOGW("sniff input format failed, will guess one!");
        return GuessInputFormat(uri, inputFormat_);
    }
    return AVCS_ERR_OK;
}

void Source::InitAVIOContext(int flags)
{
    constexpr int bufferSize = 4096;
    customIOContext_.sourcePlugin = sourcePlugin_.get();
    Status pluginRet = sourcePlugin_->GetSize(customIOContext_.fileSize);
    if (pluginRet != Status::OK) {
        AVCODEC_LOGE("get file size failed when set data source for plugin!");
        return;
    }
    pluginRet = sourcePlugin_->SeekTo(0);
    if (pluginRet != Status::OK) {
        AVCODEC_LOGE("seek to 0 failed when set data source for plugin!");
        return;
    }
    customIOContext_.offset=0;
    customIOContext_.eof=false;
    auto buffer = static_cast<unsigned char*>(av_malloc(bufferSize));
    auto bufferVector = std::make_shared<Buffer>();
    customIOContext_.bufMemory = bufferVector;
    auto bufMemory = bufferVector->WrapMemory(buffer, bufferSize, 0);
    if (buffer == nullptr) {
        AVCODEC_LOGE("AllocAVIOContext failed to av_malloc...");
        return;
    }
    avioContext_ = avio_alloc_context(buffer, bufferSize, flags & AVIO_FLAG_WRITE,
                                    (void*)(&customIOContext_), AVReadPacket, NULL, AVSeek);
    customIOContext_.avioContext = avioContext_;
    if (avioContext_ == nullptr) {
        AVCODEC_LOGE("AllocAVIOContext failed to avio_alloc_context...");
        av_free(buffer);
        return;
    }
    Seekable seekable = sourcePlugin_->GetSeekable();
    AVCODEC_LOGD("seekable_ is %{public}d", (int)seekable);
    avioContext_->seekable = (seekable == Seekable::SEEKABLE) ? AVIO_SEEKABLE_NORMAL : 0;
    if (!(static_cast<uint32_t>(flags) & static_cast<uint32_t>(AVIO_FLAG_WRITE))) {
        avioContext_->buf_ptr = avioContext_->buf_end;
        avioContext_->write_flag = 0;
    }
}

int64_t Source::AVSeek(void *opaque, int64_t offset, int whence)
{
    auto customIOContext = static_cast<CustomIOContext*>(opaque);
    uint64_t newPos = 0;
    switch (whence) {
        case SEEK_SET:
            newPos = static_cast<uint64_t>(offset);
            customIOContext->offset = newPos;
            break;
        case SEEK_CUR:
            newPos = customIOContext->offset + offset;
            break;
        case SEEK_END:
        case AVSEEK_SIZE: {
            size_t mediaDataSize = 0;
            customIOContext->sourcePlugin->GetSize(mediaDataSize);
            if (mediaDataSize > 0) {
                newPos = mediaDataSize + offset;
            }
            break;
        }
        default:
            AVCODEC_LOGW("AVSeek unexpected whence: %{oublic}d", whence);
            break;
    }
    if (whence != AVSEEK_SIZE) {
        customIOContext->offset = newPos;
    }
    return newPos;
}

int Source::AVReadPacket(void *opaque, uint8_t *buf, int bufSize)
{
    int rtv = -1;
    auto readSize = bufSize;
    auto customIOContext = static_cast<CustomIOContext*>(opaque);
    auto bufferVector = customIOContext->bufMemory;
    if ((customIOContext->avioContext->seekable == (int) Seekable::SEEKABLE)&&(customIOContext->fileSize!=0)) {
        if (customIOContext->offset > customIOContext->fileSize) {
            AVCODEC_LOGW("ERROR: offset: %{public}zu is larger than totalSize: %{public}zu",
                         customIOContext->offset, customIOContext->fileSize);
            return AVCS_ERR_SEEK_FAILED;
        }
        if (static_cast<size_t>(customIOContext->offset+bufSize) > customIOContext->fileSize) {
            readSize = customIOContext->fileSize - customIOContext->offset;
        }
        if (buf!=nullptr && bufferVector->GetMemory()!=nullptr) {
            auto memSize = static_cast<int>(bufferVector->GetMemory()->GetCapacity());
            readSize = (readSize > memSize) ? memSize : readSize;
        }
        if (customIOContext->position != customIOContext->offset) {
            int32_t err = static_cast<int32_t>(customIOContext->sourcePlugin->SeekTo(customIOContext->offset));
            if (err < 0) {
                AVCODEC_LOGD("ERROR: Seek to %{public}zu fail,err=%{public}d\n", customIOContext->offset, err);
                return AVCS_ERR_SEEK_FAILED;
            }
            customIOContext->position = customIOContext->offset;
        }
        int32_t result = static_cast<int32_t>(
                    customIOContext->sourcePlugin->Read(bufferVector, static_cast<size_t>(readSize)));
        AVCODEC_LOGD("AVReadPacket read data size = %{public}d",
                     static_cast<int32_t>(bufferVector->GetMemory()->GetSize()));
        if (result == 0) {
            rtv = bufferVector->GetMemory()->GetSize();
            customIOContext->offset += rtv;
            customIOContext->position += rtv;
        } else if (static_cast<int>(result) == 1) {
            customIOContext->eof=true;
            rtv = AVERROR_EOF;
        } else {
            AVCODEC_LOGE("AVReadPacket failed with rtv = %{public}d", static_cast<int>(result));
        }
    }
    return rtv;
}

int32_t Source::InitAVFormatContext()
{
    AVFormatContext *formatContext = avformat_alloc_context();
    if (formatContext == nullptr) {
        AVCODEC_LOGE("InitAVFormatContext failed, because  alloc AVFormatContext failed.");
        return AVCS_ERR_INVALID_OPERATION;
    }
    InitAVIOContext(AVIO_FLAG_READ);
    if (avioContext_ == nullptr) {
        AVCODEC_LOGE("InitAVFormatContext failed, because  init AVIOContext failed.");
        return AVCS_ERR_INVALID_OPERATION;
    }
    formatContext->pb = avioContext_;
    formatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
    int32_t ret = -1;
    ret = static_cast<int32_t>(avformat_open_input(&formatContext, nullptr, inputFormat_.get(), nullptr));
    if (ret == 0) {
        formatContext_ = std::shared_ptr<AVFormatContext>(formatContext, [](AVFormatContext* ptr) {
            if (ptr) {
                auto ctx = ptr->pb;
                if (ctx) {
                    av_freep(&ctx->buffer);
                    av_free(ctx);
                }
            }
        });
    } else {
        AVCODEC_LOGE("avformat_open_input failed by %{public}s", inputFormat_->name);
        return AVCS_ERR_INVALID_OPERATION;
    }
    return AVCS_ERR_OK;
}

uintptr_t Source::GetSourceAddr()
{
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_INVALID_OPERATION,
                             "GetSourceAddr failed, formatContext_ is nullptr!");
    return (uintptr_t)(formatContext_.get());
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS