/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "avcodec_server_manager.h"
#include <unordered_set>
#include "codec_service_stub.h"
#include "codeclist_service_stub.h"
#include "muxer_service_stub.h"
#include "demuxer_service_stub.h"
#include "source_service_stub.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "avcodec_dfx.h"
#include "service_dump_manager.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecServerManager"};
}

namespace OHOS {
namespace AVCodec {
constexpr uint32_t SERVER_MAX_NUMBER = 16;
AVCodecServerManager &AVCOdecServerManager::GetInstance()
{
    static AVCodecServerManager instance;
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
        AVCodec_LOGI("%{public}s", dumpString.c_str());
    }
    dumpString.clear();

    return OHOS::NO_ERROR;
}

int32_t AVCodecServerManager::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    std::string dumpString;
    std::unordered_set<std::u16string> argSets;
    for (decltype(args.size()) index = 0; index < args.size(); ++index) {
        argSets.insert(args[index]);
    }

    dumpString += "------------------CodecServer------------------\n";
    if (WriteInfo(fd, dumpString, dumperTbl_[StubType::CODEC],
        argSets.find(u"codec") != argSets.end()) != OHOS::NO_ERROR) {
        AV_CODEC_LOGW("Failed to write CodecServer information");
        return OHOS::INVALID_OPERATION;
    }

    dumpString += "------------------MuxerServer------------------\n";
    if (WriteInfo(fd, dumpString, dumperTbl_[StubType::MUXER],
        argSets.find(u"muxer") != argSets.end()) != OHOS::NO_ERROR) {
        AV_CODEC_LOGW("Failed to write MuxerServer information");
        return OHOS::INVALID_OPERATION;
    }

    dumpString += "------------------DemuxerServer------------------\n";
    if (WriteInfo(fd, dumpString, dumperTbl_[StubType::DEMUXER],
        argSets.find(u"demuxer") != argSets.end()) != OHOS::NO_ERROR) {
        AV_CODEC_LOGW("Failed to write DemuxerServer information");
        return OHOS::INVALID_OPERATION;
    }

    dumpString += "------------------SourceServer------------------\n";
    if (WriteInfo(fd, dumpString, dumperTbl_[StubType::SOURCE],
        argSets.find(u"source") != argSets.end()) != OHOS::NO_ERROR) {
        AV_CODEC_LOGW("Failed to write SourceServer information");
        return OHOS::INVALID_OPERATION;
    }

    if (ServiceDumpManager::GetInstance().Dump(fd, argSets) != OHOS::NO_ERROR) {
        AV_CODEC_LOGW("Failed to write dfx dump information");
        return OHOS::INVALID_OPERATION;
    }

    return OHOS::NO_ERROR;
}

AVCodecServerManager::AVCodecServerManager()
{
    AV_CODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecServerManager::~AVCodecServerManager()
{
    AV_CODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

sptr<IRemoteObject> AVCodecServerManager::CreateStubObject(StubType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    switch (type) {
        case CODECLIST: {
            return CreateCodecListStubObject();
        }
        case CODEC: {
            return CreateCodecStubObject();
        }
        case MUXER: {
            return CreateMuxerStubObject();
        }
        case DEMUXER: {
            return CreateDemuxerStubObject();
        }
        case SOURCE: {
            return CreateSourceStubObject();
        }
        default: {
            AV_CODEC_LOGE("default case, av_codec server manager failed");
            return nullptr;
        }
    }
}

sptr<IRemoteObject> AVCodecServerManager::CreateCodecListStubObject()
{
    if (codecListStubMap_.size() >= SERVER_MAX_NUMBER) {
        AV_CODEC_LOGE("The number of codeclist services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", codecListStubMap_.size());
        return nullptr;
    }
    sptr<CodecListServiceStub> codecListStub = codecListServiceStub::Create();
    if (codecListStub == nullptr) {
        AV_CODEC_LOGE("failed to create CodecListServiceStub");
        return nullptr;
    }
    sptr<IRemoteObject> object = codecListStub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        codecListStubMap_[object] = pid;
        AV_CODEC_LOGD("The number of codeclist services(%{public}zu).", codecListStubMap_.size());
    }
    return object;
}

sptr<IRemoteObject> MediaServerManager::CreateCodecStubObject()
{
    if (codecStubMap_.size() >= SERVER_MAX_NUMBER) {
        AV_CODEC_LOGE("The number of codec services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", codecStubMap_.size());
        return nullptr;
    }
    sptr<CodecServiceStub> codecHelperStub = CodecServiceStub::Create();
    if (CodecHelperStub == nullptr) {
        MEDIA_LOGE("failed to create CodecServiceStub");
        return nullptr;
    }
    sptr<IRemoteObject> object = codecHelperStub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        codecStubMap_[object] = pid;

        Dumper dumper;
        dumper.entry_ = [codec = codecHelperStub](int32_t fd) -> int32_t {
            return codec->DumpInfo(fd);
        };
        dumper.pid_ = pid;
        dumper.uid_ = IPCSkeleton::GetCallingUid();
        dumper.remoteObject_ = object;
        dumperTbl_[StubType::CODEC].emplace_back(dumper);
        MEDIA_LOGD("The number of codec services(%{public}zu).", codecStubMap_.size());
        if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
            AV_CODEC_LOGW("failed to call InstanceDump");
        }
    }
    return object;
}

void AVCodecServerManager::DestroyStubObject(StubType type, sptr<IRemoteObject> object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    DestroyDumper(type, object);
    switch (type) {
        case CODEC: {
            for (auto it = codecStubMap_.begin(); it != codecStubMap_.end(); it++) {
                if (it->first == object) {
                    AV_CODEC_LOGD("destroy codec stub services(%{public}zu) pid(%{public}d).",
                        codecStubMap_.size(), pid);
                    (void)codecStubMap_.erase(it);
                    return;
                }
            }
            AV_CODEC_LOGE("find codec object failed, pid(%{public}d).", pid);
            break;
        }
        case CODECLIST: {
            for (auto it = codecListStubMap_.begin(); it != codecListStubMap_.end(); it++) {
                if (it->first == object) {
                    AV_CODEC_LOGD("destroy codeclist stub services(%{public}zu) pid(%{public}d).",
                        codecListStubMap_.size(), pid);
                    (void)codecListStubMap_.erase(it);
                    return;
                }
            }
            AV_CODEC_LOGE("find codeclist object failed, pid(%{public}d).", pid);
            break;
        }
        case MUXER: {
            for (auto it = muxerStubMap_.begin(); it != muxerStubMap_.end(); it++) {
                if (it->first == object) {
                    AV_CODEC_LOGD("destroy muxer stub services(%{public}zu) pid(%{public}d).",
                        muxerStubMap_.size(), pid);
                    (void)muxerStubMap_.erase(it);
                    return;
                }
            }
            AV_CODEC_LOGE("find muxer object failed, pid(%{public}d).", pid);
            break;
        }
        case DEMUXER: {
            for (auto it = demuxerStubMap_.begin(); it != demuxerStubMap_.end(); it++) {
                if (it->first == object) {
                    AV_CODEC_LOGD("destroy demuxer stub services(%{public}zu) pid(%{public}d).",
                        demuxerStubMap_.size(), pid);
                    (void)demuxerStubMap_.erase(it);
                    return;
                }
            }
            AV_CODEC_LOGE("find demuxer object failed, pid(%{public}d).", pid);
            break;
        }
        case SOURCE: {
            for (auto it = sourceStubMap_.begin(); it != sourceStubMap_.end(); it++) {
                if (it->first == object) {
                    AV_CODEC_LOGD("destroy source stub services(%{public}zu) pid(%{public}d).",
                        sourceStubMap_.size(), pid);
                    (void)sourceStubMap_.erase(it);
                    return;
                }
            }
            AV_CODEC_LOGE("find demuxer object failed, pid(%{public}d).", pid);
            break;
        }
        default: {
            AV_CODEC_LOGE("default case, av_codec server manager failed, pid(%{public}d).", pid);
            break;
        }
    }
}

void AVCodecServerManager::DestroyStubObjectForPid(pid_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AV_CODEC_LOGD("codec stub services(%{public}zu) pid(%{public}d).", codecStubMap_.size(), pid);
    for (auto it = codecStubMap_.begin(); it != codecStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = codecStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AV_CODEC_LOGD("codec stub services(%{public}zu).", codecStubMap_.size());

    AV_CODEC_LOGD("codeclist stub services(%{public}zu) pid(%{public}d).", codecListStubMap_.size(), pid);
    for (auto it = codecListStubMap_.begin(); it != codecListStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = codecListStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AV_CODEC_LOGD("codeclist stub services(%{public}zu).", codecListStubMap_.size());

    AV_CODEC_LOGD("muxer stub services(%{public}zu) pid(%{public}d).", muxerStubMap_.size(), pid);
    for (auto it = muxerStubMap_.begin(); it != muxerStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = muxerStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AV_CODEC_LOGD("muxer stub services(%{public}zu).", muxerStubMap_.size());

    AV_CODEC_LOGD("demuxer stub services(%{public}zu) pid(%{public}d).", demuxerStubMap_.size(), pid);
    for (auto it = demuxerStubMap_.begin(); it != demuxerStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = demuxerStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AV_CODEC_LOGD("demuxer stub services(%{public}zu).", demuxerStubMap_.size());

    AV_CODEC_LOGD("source stub services(%{public}zu) pid(%{public}d).", sourceStubMap_.size(), pid);
    for (auto it = sourceStubMap_.begin(); it != sourceStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = sourceStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AV_CODEC_LOGD("source stub services(%{public}zu).", sourceStubMap_.size());

    executor_.Clear();
}

void AVCodecServerManager::DestroyDumper(StubType type, sptr<IRemoteObject> object)
{
    for (auto it = dumperTbl_[type].begin(); it != dumperTbl_[type].end(); it++) {
        if (it->remoteObject_ == object) {
            (void)dumperTbl_[type].erase(it);
            AV_CODEC_LOGD("AVCodecServerManager::DestroyDumper");
            if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
                AV_CODEC_LOGW("failed to call InstanceDump");
            }
            return;
        }
    }
}

void AVCodecServerManager::DestroyDumperForPid(pid_t pid)
{
    for (auto &dumpers : dumperTbl_) {
        for (auto it = dumpers.second.begin(); it != dumpers.second.end();) {
            if (it->pid_ == pid) {
                it = dumpers.second.erase(it);
                AV_CODEC_LOGD("AVCodecServerManager::DestroyDumperForPid");
            } else {
                it++;
            }
        }
    }
    if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
        MEDIA_LOGW("failed to call InstanceDump");
    }
}

void AVCodecServerManager::AsyncExecutor::Commit(sptr<IRemoteObject> obj)
{
    std::lock_guard<std::mutex> lock(listMutex_);
    freeList_.push_back(obj);
}

void AVCodecServerManager::AsyncExecutor::Clear()
{
    std::thread(&AVCodecServerManager::AsyncExecutor::HandleAsyncExecution, this).detach();
}

void AVCodecServerManager::AsyncExecutor::HandleAsyncExecution()
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
