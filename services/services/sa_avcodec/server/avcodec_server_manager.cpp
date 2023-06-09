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

#include "avcodec_server_manager.h"
#include <thread>
#include <unistd.h>
#include <unordered_set>
#include "avcodec_dfx.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_log_dump.h"
#include "avcodec_xcollie.h"
#ifdef SUPPORT_MUXER
#include "muxer_service_stub.h"
#endif

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecServerManager"};
}


namespace OHOS {
namespace Media {
constexpr uint32_t SERVER_MAX_NUMBER = 16;
AVCodecServerManager &AVCodecServerManager::GetInstance()
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
        if (needDetail && iter.entry_(fd) != AVCS_ERR_OK) {
            return OHOS::INVALID_OPERATION;
        }
    }
    if (fd != -1) {
        write(fd, dumpString.c_str(), dumpString.size());
    } else {
        AVCODEC_LOGI("%{public}s", dumpString.c_str());
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

#ifdef SUPPORT_MUXER
    dumpString += "------------------MuxerServer------------------\n";
    if (WriteInfo(fd, dumpString, dumperTbl_[StubType::MUXER],
        argSets.find(u"muxer") != argSets.end()) != OHOS::NO_ERROR) {
        AVCODEC_LOGW("Failed to write MuxerServer information");
        return OHOS::INVALID_OPERATION;
    }
#endif

    if (AVCodecXCollie::GetInstance().Dump(fd) != OHOS::NO_ERROR) {
        AVCODEC_LOGW("Failed to write xcollie dump information");
        return OHOS::INVALID_OPERATION;
    }

    return OHOS::NO_ERROR;
}

AVCodecServerManager::AVCodecServerManager()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecServerManager::~AVCodecServerManager()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

sptr<IRemoteObject> AVCodecServerManager::CreateStubObject(StubType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    switch (type) {
#ifdef SUPPORT_MUXER
        case MUXER: {
            return CreateMuxerStubObject();
        }
#endif
        default: {
            AVCODEC_LOGE("default case, av_codec server manager failed");
            return nullptr;
        }
    }
}

#ifdef SUPPORT_MUXER
sptr<IRemoteObject> AVCodecServerManager::CreateMuxerStubObject()
{
    if (muxerStubMap_.size() >= SERVER_MAX_NUMBER) {
        AVCODEC_LOGE("The number of muxer services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", muxerStubMap_.size());
        return nullptr;
    }
    sptr<MuxerServiceStub> muxerStub = MuxerServiceStub::Create();
    if (muxerStub == nullptr) {
        AVCODEC_LOGE("Create MuxerServiceStub failed");
        return nullptr;
    }
    sptr<IRemoteObject> object = muxerStub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        muxerStubMap_[object] = pid;

        Dumper dumper;
        dumper.entry_ = [muxer = muxerStub](int32_t fd) -> int32_t {
            return muxer->DumpInfo(fd);
        };
        dumper.pid_ = pid;
        dumper.uid_ = IPCSkeleton::GetCallingUid();
        dumper.remoteObject_ = object;
        dumperTbl_[StubType::MUXER].emplace_back(dumper);
        AVCODEC_LOGD("The number of muxer services(%{public}zu).", muxerStubMap_.size());
        if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
            AVCODEC_LOGW("Failed to call InstanceDump");
        }
    }
    return object;
}
#endif

void AVCodecServerManager::DestroyStubObject(StubType type, sptr<IRemoteObject> object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    DestroyDumper(type, object);
    
    auto compare_func = [object](std::pair<sptr<IRemoteObject>, pid_t> objectPair) ->
        bool { return objectPair.first == object; };
    switch (type) {
        case MUXER: {
            auto it = find_if(muxerStubMap_.begin(), muxerStubMap_.end(), compare_func);
            if (it != muxerStubMap_.end()) {
                AVCODEC_LOGD("destroy muxer stub services(%{public}zu) pid(%{public}d).",
                    muxerStubMap_.size(), pid);
                (void)muxerStubMap_.erase(it);
                return;
            }
            AVCODEC_LOGE("find muxer object failed, pid(%{public}d).", pid);
            break;
        }
        default: {
            AVCODEC_LOGE("default case, av_codec server manager failed, pid(%{public}d).", pid);
            break;
        }
    }
}

void AVCodecServerManager::DestroyStubObjectForPid(pid_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AVCODEC_LOGD("muxer stub services(%{public}zu) pid(%{public}d).", muxerStubMap_.size(), pid);
    for (auto it = muxerStubMap_.begin(); it != muxerStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = muxerStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AVCODEC_LOGD("muxer stub services(%{public}zu).", muxerStubMap_.size());

    executor_.Clear();
}

void AVCodecServerManager::DestroyDumper(StubType type, sptr<IRemoteObject> object)
{
    for (auto it = dumperTbl_[type].begin(); it != dumperTbl_[type].end(); it++) {
        if (it->remoteObject_ == object) {
            (void)dumperTbl_[type].erase(it);
            AVCODEC_LOGD("AVCodecServerManager::DestroyDumper");
            if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
                AVCODEC_LOGW("failed to call InstanceDump");
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
                AVCODEC_LOGD("AVCodecServerManager::DestroyDumperForPid");
            } else {
                it++;
            }
        }
    }
    if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
        AVCODEC_LOGW("failed to call InstanceDump");
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
} // namespace Media
} // namespace OHOS
