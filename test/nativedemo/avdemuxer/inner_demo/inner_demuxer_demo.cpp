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

#include "native_avmemory.h"
#include "avdemuxer.h"
#include "inner_demuxer_demo.h"

namespace OHOS {
namespace Media {
InnerDemuxerDemo::InnerDemuxerDemo()
{
    printf("InnerDemuxerDemo is called\n");
}

InnerDemuxerDemo::~InnerDemuxerDemo()
{
    printf("~InnerDemuxerDemo is called\n");
}

int32_t InnerDemuxerDemo::CreateWithSource(AVSource &source)
{
    this->demuxer_ = AVDemuxerFactory::CreateWithSource(source);
    if (!demuxer_) {
        printf("AVDemuxerFactory::CreateWithSource is failed\n");
        return -1;
    }
    return 0;
}
void InnerDemuxerDemo::Destroy()
{
    printf("InnerDemuxerDemo::Destroy\n");
}

int32_t InnerDemuxerDemo::SelectTrackByID(uint32_t trackIndex)
{
    int32_t ret = this->demuxer_->SelectTrackByID(trackIndex);
    if (ret != 0) {
        printf("SelectTrackByID is failed\n");
    }
    return ret;
}

int32_t InnerDemuxerDemo::UnselectTrackByID(uint32_t trackIndex)
{
    int32_t ret = this->demuxer_->UnselectTrackByID(trackIndex);
    if (ret != 0) {
        printf("SelectTrackByID is failed\n");
    }
    return ret;
}

int32_t InnerDemuxerDemo::PrintInfo(int32_t tracks)
{
    for (int32_t i = 0; i < tracks; i++) {
        printf("streams[%d]==>total frames=%lld,KeyFrames=%lld\n", i,
            frames_[i], key_frames_[i]);
    }
    return 0;
}

int32_t InnerDemuxerDemo::ReadAllSamples(std::shared_ptr<AVSharedMemory> SampleMem, int32_t tracks)
{
    AVCodecBufferFlag bufferFlag = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
    for (int i = 0;i < tracks; i++) {
        frames_[i] = 0;
        key_frames_[i] = 0;
    }
    int32_t ret = -1;
    bool isEnd = false;
    while (!isEnd) {
        for (int32_t i = 0; i < tracks; i++) {
            ret = ReadSample(i, SampleMem, sampleInfo, bufferFlag);
            if (ret != 0) {
                printf("read finished\n");
                PrintInfo(tracks);
                isEnd = true;
            }
            if (bufferFlag == AVCODEC_BUFFER_FLAG_EOS) {
                frames_[i]++;
                printf("streams[%d]==>sampleInfo:pts=%lld,size=%d,offset=%d\n",
                    i, sampleInfo.presentationTimeUs, sampleInfo.size, sampleInfo.offset);
            }
            if (bufferFlag == AVCODEC_BUFFER_FLAG_SYNC_FRAME) {
                frames_[i]++;
                key_frames_[i]++;
            }
        }
    }
    return ret;
}

int32_t InnerDemuxerDemo::ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> mem,
                                     AVCodecBufferInfo &bufInfo, AVCodecBufferFlag &bufferFlag)
{
    int32_t ret = this->demuxer_->ReadSample(trackIndex, mem, bufInfo, bufferFlag);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

int32_t InnerDemuxerDemo::SeekToTime(int64_t mSeconds, AVSeekMode mode)
{
    int32_t ret = demuxer_->SeekToTime(mSeconds, mode);
    if (ret != 0) {
        printf("SeekToTime is failed\n");
    }
    return ret;
}
}
}
