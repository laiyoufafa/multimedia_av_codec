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
#include "avmuxer_demo_base.h"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "avcodec_errors.h"
#include "avcodec_common.h"

namespace {
    constexpr int MODE_ZERO = 0;
    constexpr int MODE_ONE = 1;
    constexpr int MODE_TWO = 2;
    constexpr int MODE_THREE = 3;
    constexpr int CONFIG_BUFFER_SZIE = 100;
}

namespace OHOS {
namespace Media {
const AudioTrackParam *AVMuxerDemoBase::audioParams_ = nullptr;
const VideoTrackParam *AVMuxerDemoBase::videoParams_ = nullptr;
const VideoTrackParam *AVMuxerDemoBase::coverParams_ = nullptr;
std::string AVMuxerDemoBase::videoType_ = std::string("");
std::string AVMuxerDemoBase::audioType_ = std::string("");
std::string AVMuxerDemoBase::coverType_ = std::string("");
std::string AVMuxerDemoBase::format_ = std::string("");
OutputFormat AVMuxerDemoBase::outputFormat_ = OUTPUT_FORMAT_DEFAULT;
bool AVMuxerDemoBase::hasSetMode_ = false;

AVMuxerDemoBase::AVMuxerDemoBase()
{
}

std::shared_ptr<std::ifstream> OpenFile(const std::string &filePath)
{
    auto file = std::make_shared<std::ifstream>();
    file->open(filePath, std::ios::in | std::ios::binary);
    if (file->is_open()) {
        return file;
    }

    return nullptr;
}

void AVMuxerDemoBase::SelectFormatMode()
{
    int num;
    std::cout<<"\nplease select muxer type: 0.mp4 1.m4a"<<std::endl;
    std::cin>>num;
    switch (num) {
        case MODE_ZERO:
            format_ = "mp4";
            outputFormat_ = OUTPUT_FORMAT_MPEG_4;
            break;
        case MODE_ONE:
            format_ = "m4a";
            outputFormat_ = OUTPUT_FORMAT_M4A;
            break;
        default:
            format_ = "mp4";
            outputFormat_ = OUTPUT_FORMAT_MPEG_4;
            break;
    }
}

void AVMuxerDemoBase::SelectAudioVideoMode()
{
    int num;
    std::cout<<"\nplease select audio file: 0.noAudio 1.aac 2.mpeg"<<std::endl;
    std::cin>>num;
    switch (num) {
        case MODE_ZERO:
            audioType_ = "noAudio";
            audioParams_ = nullptr;
            break;
        case MODE_ONE:
            audioType_ = "aac";
            audioParams_ = &g_audioAacPar;
            break;
        case MODE_TWO:
            audioType_ = "mpeg";
            audioParams_ = &g_audioMpegPar;
            break;
        default:
            videoType_ = "noAudio";
            audioParams_ = nullptr;
            std::cout<<"do not support audio type index: "<<num<<", set to noAudio"<<std::endl;
            break;
    }

    std::cout<<"please select video file:0.noVideo 1.h264 2.mpeg4"<<std::endl;
    std::cin>>num;
    switch (num) {
        case MODE_ZERO:
            videoType_ = "noVideo";
            videoParams_ = nullptr;
            break;
        case MODE_ONE:
            videoType_ = "h264";
            videoParams_ = &g_videoH264Par;
            break;
        case MODE_TWO:
            videoType_ = "mpeg4";
            videoParams_ = &g_videoMpeg4Par;
            break;
        default:
            videoType_ = "noVideo";
            videoParams_ = nullptr;
            std::cout<<"do not support video type index: "<<", set to noVideo"<<num<<std::endl;
            break;
    }
}

void AVMuxerDemoBase::SelectCoverMode()
{
    int num;
    std::cout<<"please select cover file:0.NoCover 1.jpg 2.png 3.bmp"<<std::endl;
    std::cin>>num;
    switch (num) {
        case MODE_ZERO:
            coverType_ = "noCover";
            coverParams_ = nullptr;
            break;
        case MODE_ONE:
            coverType_ = "jpg";
            coverParams_ = &g_jpegCoverPar;
            break;
        case MODE_TWO:
            coverType_ = "png";
            coverParams_ = &g_pngCoverPar;
            break;
        case MODE_THREE:
            coverType_ = "bmp";
            coverParams_ = &g_bmpCoverPar;
            break;
        default:
            coverType_ = "noCover";
            coverParams_ = nullptr;
            std::cout<<"do not support cover type index: "<<", set to noCover"<<num<<std::endl;
            break;
    }
}

int AVMuxerDemoBase::SelectMode()
{
    if (hasSetMode_) {
        return 0;
    }
    SelectFormatMode();
    SelectAudioVideoMode();
    SelectCoverMode();

    hasSetMode_ = true;
    return 0;
}

int AVMuxerDemoBase::SelectModeAndOpenFile()
{
    if (SelectMode() != 0) {
        return -1;
    }

    if (audioParams_ != nullptr) {
        audioFile_ = OpenFile(audioParams_->fileName);
        if (audioFile_ == nullptr) {
            std::cout<<"open audio file failed! file name:"<<audioParams_->fileName<<std::endl;
            return -1;
        }
        std::cout<<"open audio file success! file name:"<<audioParams_->fileName<<std::endl;
    }

    if (videoParams_ != nullptr) {
        videoFile_ = OpenFile(videoParams_->fileName);
        if (videoFile_ == nullptr) {
            std::cout<<"open video file failed! file name:"<<videoParams_->fileName<<std::endl;
            Reset();
            return -1;
        }
        std::cout<<"video file success! file name:"<<videoParams_->fileName<<std::endl;
    }

    if (coverParams_ != nullptr) {
        coverFile_ = OpenFile(coverParams_->fileName);
        if (coverFile_ == nullptr) {
            std::cout<<"open cover file failed! file name:"<<coverParams_->fileName<<std::endl;
            Reset();
            return -1;
        }
        std::cout<<"cover file success! file name:"<<coverParams_->fileName<<std::endl;
    }
    return 0;
}

void AVMuxerDemoBase::Reset()
{
    if (outFd_ > 0) {
        close(outFd_);
        outFd_ = -1;
    }
    if (audioFile_ != nullptr) {
        audioFile_->close();
        audioFile_ = nullptr;
    }
    if (videoFile_ != nullptr) {
        videoFile_->close();
        videoFile_ = nullptr;
    }
    if (coverFile_ != nullptr) {
        coverFile_->close();
        coverFile_ = nullptr;
    }
}

void AVMuxerDemoBase::RunCase()
{
    if (SelectModeAndOpenFile() != 0) {
        return;
    }

    DoRunMuxer();

    Reset();
}

void AVMuxerDemoBase::RunMultiThreadCase()
{
    std::cout<<"==== start AVMuxerDemoBase::RunMultiThreadCase ==="<<std::endl;
    if (SelectModeAndOpenFile() != 0) {
        return;
    }

    DoRunMultiThreadCase();

    Reset();
}

void AVMuxerDemoBase::WriteSingleTrackSample(uint32_t trackId, std::shared_ptr<std::ifstream> file)
{
    if (file == nullptr) {
        std::cout<<"AVMuxerDemoBase::WriteTrackSample file si nullptr"<<std::endl;
        return;
    }
    uint32_t flags = 0;
    uint32_t dataSize = 0;
    std::shared_ptr<AVSharedMemoryBase> avMemBuffer = nullptr;
    uint32_t avMemBufferSize = 0;
    TrackSampleInfo info {trackId, 0, 0, 0, 0};
    while (1) {
        file->read((char *)&info.timeUs, sizeof(info.timeUs));
        if (file->eof()) {
            break;
        }

        file->read((char *)&flags, sizeof(flags));
        if (file->eof()) {
            break;
        }

        file->read((char *)&dataSize, sizeof(dataSize));
        if (file->eof()) {
            break;
        }

        if (avMemBuffer != nullptr && dataSize > avMemBufferSize) {
            avMemBuffer = nullptr;
            avMemBufferSize = 0;
        }
        if (avMemBuffer == nullptr) {
            avMemBuffer = std::make_shared<AVSharedMemoryBase>(dataSize,
                AVSharedMemory::FLAGS_READ_ONLY, "sampleData");;
            avMemBufferSize = dataSize;
            avMemBuffer->Init();
        }

        file->read((char *)avMemBuffer->GetBase(), dataSize);
        if (file->eof()) {
            break;
        }
        info.size = dataSize;

        info.flags = 0;
        if (flags != 0) {
            info.flags |= AVCODEC_BUFFER_FLAG_SYNC_FRAME;
        }

        if (DoWriteSample(avMemBuffer, info) != 0) {
            std::cout<<"DoWriteSample failed!"<<std::endl;
        }
    }
}

int AVMuxerDemoBase::ReadSampleDataInfo(std::shared_ptr<std::ifstream> &curFile,
    std::shared_ptr<AVSharedMemoryBase> &buffer, uint32_t &curSize, TrackSampleInfo &info)
{
    uint32_t dataSize = 0;
    uint32_t flags = 0;
    if (audioPts_ > videoPts_) {
        curFile = videoFile_;
        info.trackIndex = videoTrackId_;
        info.timeUs = videoPts_;
    } else {
        curFile = audioFile_;
        info.trackIndex = audioTrackId_;
        info.timeUs = audioPts_;
    }

    curFile->read((char *)&flags, sizeof(flags));
    if (curFile->eof()) {
        return -1;
    }
    info.flags = 0;
    if (flags != 0) {
        info.flags |= AVCODEC_BUFFER_FLAG_SYNC_FRAME;
    }

    curFile->read((char *)&dataSize, sizeof(dataSize));
    if (curFile->eof()) {
        return -1;
    }

    if (buffer != nullptr && dataSize > curSize) {
        buffer = nullptr;
        curSize = 0;
    }
    if (buffer == nullptr) {
        buffer = std::make_shared<AVSharedMemoryBase>(dataSize,
            AVSharedMemory::FLAGS_READ_ONLY, "sampleData");;
        curSize = dataSize;
        buffer->Init();
    }

    curFile->read((char *)buffer->GetBase(), dataSize);
    info.size = dataSize;
    return 0;
}

void AVMuxerDemoBase::WriteAvTrackSample()
{
    if (audioFile_ == nullptr || videoFile_ == nullptr) {
        return;
    }
    TrackSampleInfo info {0, 0, 0, 0, 0};
    std::shared_ptr<std::ifstream> curFile = nullptr;
    std::shared_ptr<AVSharedMemoryBase> avMemBuffer = nullptr;
    uint32_t avMemBufferSize = 0;
    audioFile_->read((char *)&audioPts_, sizeof(audioPts_));
    if (audioFile_->eof()) {
        return;
    }
    videoFile_->read((char *)&videoPts_, sizeof(videoPts_));
    if (videoFile_->eof()) {
        return;
    }
    while (1) {
        if (ReadSampleDataInfo(curFile, avMemBuffer, avMemBufferSize, info) != 0) {
            break;
        }
        if (DoWriteSample(avMemBuffer, info) != 0) {
            std::cout<<"DoWriteSample failed!"<<std::endl;
        }
        if (curFile == audioFile_) {
            audioFile_->read((char *)&audioPts_, sizeof(audioPts_));
            if (audioFile_->eof()) {
                break;
            }
        } else {
            videoFile_->read((char *)&videoPts_, sizeof(videoPts_));
            if (videoFile_->eof()) {
                break;
            }
        }
    }
}

void AVMuxerDemoBase::WriteTrackSample()
{
    if (audioFile_ != nullptr && videoFile_ != nullptr && audioTrackId_ >= 0 && videoTrackId_ >= 0) {
        std::cout<<"AVMuxerDemoBase::WriteTrackSample write AUDIO and VIDEO sample"<<std::endl;
        std::cout<<"audio trackId:"<<audioTrackId_<<" video trackId:"<<videoTrackId_<<std::endl;
        WriteAvTrackSample();
    } else if (audioFile_ != nullptr && audioTrackId_ >= 0) {
        std::cout<<"AVMuxerDemoBase::WriteTrackSample write AUDIO sample"<<std::endl;
        WriteSingleTrackSample(audioTrackId_, audioFile_);
    } else if (videoFile_ != nullptr && videoTrackId_ >= 0) {
        std::cout<<"AVMuxerDemoBase::WriteTrackSample write VIDEO sample"<<std::endl;
        WriteSingleTrackSample(videoTrackId_, videoFile_);
    } else {
        std::cout<<"AVMuxerDemoBase::WriteTrackSample don't write AUDIO and VIDEO track!!"<<std::endl;
    }
}

void AVMuxerDemoBase::MulThdWriteTrackSample(AVMuxerDemoBase *muxerBase, uint32_t trackId,
    std::shared_ptr<std::ifstream> file)
{
    muxerBase->WriteSingleTrackSample(trackId, file);
}

void AVMuxerDemoBase::WriteCoverSample()
{
    if (coverParams_ == nullptr) {
        return;
    }
    std::cout<<"AVMuxerDemoBase::WriteCoverSample"<<std::endl;
    if (coverFile_ == nullptr) {
        std::cout<<"AVMuxerDemoBase::WriteCoverSample coverFile_ is nullptr!"<<std::endl;
        return;
    }
    TrackSampleInfo info {coverTrackId_, 0, 0, 0, 0};
    coverFile_->seekg(0, std::ios::end);
    info.size = coverFile_->tellg();
    coverFile_->seekg(0, std::ios::beg);
    if (info.size <= 0) {
        std::cout<<"AVMuxerDemoBase::WriteCoverSample coverFile_ size is 0!"<<std::endl;
        return;
    }

    std::shared_ptr<AVSharedMemoryBase> avMemBuffer =
        std::make_shared<AVSharedMemoryBase>(info.size, AVSharedMemory::FLAGS_READ_ONLY, "sampleData");
    avMemBuffer->Init();
    coverFile_->read((char *)avMemBuffer->GetBase(), info.size);
    if (DoWriteSample(avMemBuffer, info) != AVCS_ERR_OK) {
        std::cout<<"WriteCoverSample error"<<std::endl;
    }
}

int AVMuxerDemoBase::AddVideoTrack(const VideoTrackParam *param)
{
    if (param == nullptr) {
        std::cout<<"AVMuxerDemoBase::AddVideoTrack video is not select!"<<std::endl;
        return -1;
    }
    MediaDescription videoParams;
    videoParams.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, param->mimeType);
    videoParams.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, param->width);
    videoParams.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, param->height);

    int extSize = 0;
    char buffer[CONFIG_BUFFER_SZIE] {0};
    videoFile_->read((char*)&extSize, sizeof(extSize));
    if (extSize > 0 && extSize < CONFIG_BUFFER_SZIE) {
        videoFile_->read((char*)buffer, extSize);
        videoParams.PutBuffer(MediaDescriptionKey::MD_KEY_CODEC_CONFIG, (uint8_t *)buffer, extSize);
    } else {
        std::cout<<"AVMuxerDemoBase::AddVideoTrack DoAddTrack failed!"<<std::endl;
    }

    if (DoAddTrack(videoTrackId_, videoParams) != AVCS_ERR_OK) {
        return -1;
    }
    std::cout << "AVMuxerDemoBase::AddVideoTrack video trackId is: " << videoTrackId_ << std::endl;
    return 0;
}

int AVMuxerDemoBase::AddAudioTrack(const AudioTrackParam *param)
{
    if (param == nullptr) {
        std::cout<<"AVMuxerDemoBase::AddAudioTrack audio is not select!"<<std::endl;
        return -1;
    }
    MediaDescription audioParams;
    audioParams.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, param->mimeType);
    audioParams.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, param->sampleRate);
    audioParams.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, param->channels);

    int extSize = 0;
    char buffer[CONFIG_BUFFER_SZIE] {0};
    audioFile_->read((char*)&extSize, sizeof(extSize));
    if (extSize > 0 && extSize < CONFIG_BUFFER_SZIE) {
        audioFile_->read((char*)buffer, extSize);
        audioParams.PutBuffer(MediaDescriptionKey::MD_KEY_CODEC_CONFIG, (uint8_t *)buffer, extSize);
    } else {
        std::cout<<"AVMuxerDemoBase::AddAudioTrack error extSize:"<<extSize<<std::endl;
    }

    if (DoAddTrack(audioTrackId_, audioParams) != 0) {
        std::cout<<"AVMuxerDemoBase::AddAudioTrack DoAddTrack failed!"<<std::endl;
        return -1;
    }
    std::cout << "AVMuxerDemoBase::AddAudioTrack audio trackId is: " << audioTrackId_ << std::endl;
    return 0;
}

int AVMuxerDemoBase::AddCoverTrack(const VideoTrackParam *param)
{
    if (param == nullptr) {
        std::cout<<"AVMuxerDemoBase::AddCoverTrack cover is not select!"<<std::endl;
        return -1;
    }
    MediaDescription coverParams;
    coverParams.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, param->mimeType);
    coverParams.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, param->width);
    coverParams.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, param->height);

    if (DoAddTrack(coverTrackId_, coverParams) != AVCS_ERR_OK) {
        return -1;
    }
    std::cout << "AVMuxerDemoBase::AddCoverTrack video trackId is: " << coverTrackId_ << std::endl;
    return 0;
}
} // Media
} // OHOS