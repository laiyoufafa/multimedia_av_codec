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
#include "media_server_manager.h"
#include <unordered_set>
#ifdef SUPPORT_DEMUXER
#include "demuxer_service_stub.h"
#endif
#ifdef SUPPORT_CODEC
#include "codec_service_stub.h"
#endif

#include "av_log.h"
#include "media_errors.h"
// #include "service_dump_manager.h"
// #include "player_xcollie.h"
#include <thread>
#include <unistd.h>
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MediaServerManager"};
}


namespace OHOS {
namespace AVCodec {
constexpr uint32_t SERVER_MAX_NUMBER = 16;
MediaServerManager &MediaServerManager::GetInstance()
{
    static MediaServerManager instance;
    return instance;
}

int32_t WriteInfo(int32_t fd, std::string &dumpString, std::vector<Dumper> dumpers, bool needDetail)
{
    int32_t i = 0;
    for (auto iter : dumpers) {
        dumpString += "-----Instance #" + std::to_string(i) + ": ";
        dumpString += "pid = ";
        dumpString += std::to_string(iter.pid_);
        dumpString += " uid = ";
        dumpString += std::to_string(iter.uid_);
        dumpString += "-----\n";
        if (fd != -1) {
            write(fd, dumpString.c_str(), dumpString.size());
            dumpString.clear();
        }
        i++;
        if (needDetail && iter.entry_(fd) != MSERR_OK) {
            return OHOS::INVALID_OPERATION;
        }
    }
    if (fd != -1) {
        write(fd, dumpString.c_str(), dumpString.size());
    } else {
        MEDIA_LOGI("%{public}s", dumpString.c_str());
    }
    dumpString.clear();

    return OHOS::NO_ERROR;
}

int32_t MediaServerManager::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    std::string dumpString;
    std::unordered_set<std::u16string> argSets;
    for (decltype(args.size()) index = 0; index < args.size(); ++index) {
        argSets.insert(args[index]);
    }


    return OHOS::NO_ERROR;
}

MediaServerManager::MediaServerManager()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

MediaServerManager::~MediaServerManager()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

sptr<IRemoteObject> MediaServerManager::CreateStubObject(StubType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    switch (type) {
#ifdef SUPPORT_DEMUXER
        case DEMUXER: {
            return CreateDemuxerStubObject();
        }
#endif
#ifdef SUPPORT_CODEC
        case AVCODEC: {
            return CreateAVCodecStubObject();
        }
#endif
        default: {
            MEDIA_LOGE("default case, media server manager failed");
            return nullptr;
        }
    }
}

#ifdef SUPPORT_DEMUXER
sptr<IRemoteObject> MediaServerManager::CreateDemuxerStubObject()
{
    if (demuxerStubMap_.size() >= SERVER_MAX_NUMBER) {
        MEDIA_LOGE("The number of demuxer services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", demuxerStubMap_.size());
        return nullptr;
    }
    sptr<DemuxerServiceStub> demuxerHelperStub = DemuxerServiceStub::Create();
    if (demuxerHelperStub == nullptr) {
        MEDIA_LOGE("failed to create DemuxerServiceStub");
        return nullptr;
    }
    sptr<IRemoteObject> object = demuxerHelperStub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        demuxerStubMap_[object] = pid;

        Dumper dumper;
        // dumper.entry_ = [demuxer = demuxerHelperStub](int32_t fd) -> int32_t {
        //     return demuxer->DumpInfo(fd);
        // };
        dumper.pid_ = pid;
        dumper.uid_ = IPCSkeleton::GetCallingUid();
        dumper.remoteObject_ = object;
        dumperTbl_[StubType::DEMUXER].emplace_back(dumper);
        MEDIA_LOGD("The number of demuxer services(%{public}zu).", demuxerStubMap_.size());
        if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
            MEDIA_LOGW("failed to call InstanceDump");
        }
    }
    return object;
}
#endif

#ifdef SUPPORT_CODEC
sptr<IRemoteObject> MediaServerManager::CreateAVCodecStubObject()
{
    if (avCodecStubMap_.size() >= SERVER_MAX_NUMBER) {
        av_logE("The number of avcodec services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", avCodecStubMap_.size());
        return nullptr;
    }
    sptr<AVCodecServiceStub> avCodecHelperStub = AVCodecServiceStub::Create();
    if (avCodecHelperStub == nullptr) {
        av_logE("failed to create AVCodecServiceStub");
        return nullptr;
    }
    sptr<IRemoteObject> object = avCodecHelperStub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        avCodecStubMap_[object] = pid;

        Dumper dumper;
        dumper.entry_ = [avcodec = avCodecHelperStub](int32_t fd) -> int32_t {
            return avcodec->DumpInfo(fd);
        };
        dumper.pid_ = pid;
        dumper.uid_ = IPCSkeleton::GetCallingUid();
        dumper.remoteObject_ = object;
        dumperTbl_[StubType::AVCODEC].emplace_back(dumper);
        av_logD("The number of avcodec services(%{public}zu).", avCodecStubMap_.size());
        if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
            av_logW("failed to call InstanceDump");
        }
    }
    return object;
}
#endif

void MediaServerManager::DestroyStubObject(StubType type, sptr<IRemoteObject> object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    DestroyDumper(type, object);
    switch (type) {
        case DEMUXER: {
            for (auto it = demuxerStubMap_.begin(); it != demuxerStubMap_.end(); it++) {
                if (it->first == object) {
                    MEDIA_LOGD("destroy demuxer stub services(%{public}zu) pid(%{public}d).",
                        demuxerStubMap_.size(), pid);
                    (void)demuxerStubMap_.erase(it);
                    return;
                }
            }
            MEDIA_LOGE("find demuxer object failed, pid(%{public}d).", pid);
            break;
        }
        default: {
            MEDIA_LOGE("default case, media server manager failed, pid(%{public}d).", pid);
            break;
        }
    }
}

void MediaServerManager::DestroyStubObjectForPid(pid_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    MEDIA_LOGD("demuxer stub services(%{public}zu) pid(%{public}d).", demuxerStubMap_.size(), pid);
    for (auto itDemuxer = demuxerStubMap_.begin(); itDemuxer != demuxerStubMap_.end();) {
        if (itDemuxer->second == pid) {
            executor_.Commit(itDemuxer->first);
            itDemuxer = demuxerStubMap_.erase(itDemuxer);
        } else {
            itDemuxer++;
        }
    }
    MEDIA_LOGD("demuxer stub services(%{public}zu).", demuxerStubMap_.size());

    executor_.Clear();
}

void MediaServerManager::DestroyDumper(StubType type, sptr<IRemoteObject> object)
{
    for (auto it = dumperTbl_[type].begin(); it != dumperTbl_[type].end(); it++) {
        if (it->remoteObject_ == object) {
            (void)dumperTbl_[type].erase(it);
            MEDIA_LOGD("MediaServerManager::DestroyDumper");
            if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
                MEDIA_LOGW("failed to call InstanceDump");
            }
            return;
        }
    }
}

void MediaServerManager::DestroyDumperForPid(pid_t pid)
{
    for (auto &dumpers : dumperTbl_) {
        for (auto it = dumpers.second.begin(); it != dumpers.second.end();) {
            if (it->pid_ == pid) {
                it = dumpers.second.erase(it);
                MEDIA_LOGD("MediaServerManager::DestroyDumperForPid");
            } else {
                it++;
            }
        }
    }
    if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
        MEDIA_LOGW("failed to call InstanceDump");
    }
}

void MediaServerManager::AsyncExecutor::Commit(sptr<IRemoteObject> obj)
{
    std::lock_guard<std::mutex> lock(listMutex_);
    freeList_.push_back(obj);
}

void MediaServerManager::AsyncExecutor::Clear()
{
    std::thread(&MediaServerManager::AsyncExecutor::HandleAsyncExecution, this).detach();
}

void MediaServerManager::AsyncExecutor::HandleAsyncExecution()
{
    std::list<sptr<IRemoteObject>> tempList;
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        freeList_.swap(tempList);
    }
    tempList.clear();
}
} // namespace AVCodec
} // namespace OHOS
