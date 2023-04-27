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

#include "engine_factory_repo.h"
#include <limits>
#include <cinttypes>
#include <dlfcn.h>
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "audio_ffmpeg_adapter.h"

namespace {
    static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "EngineFactoryRepo"};
#ifdef __aarch64__
    static const std::string MEDIA_ENGINE_LIB_PATH = "/system/lib64/media";
#else
    static const std::string MEDIA_ENGINE_LIB_PATH = "/system/lib/media";
#endif
    static const std::string MEDIA_ENGINE_ENTRY_SYMBOL = "CreateEngineFactory";
}

namespace OHOS {
namespace Media {

EngineFactoryRepo &EngineFactoryRepo::Instance()
{
    static EngineFactoryRepo inst;
    return inst;
}

EngineFactoryRepo::~EngineFactoryRepo()
{
}

std::shared_ptr<CodecBase> EngineFactoryRepo::CreateCodecByName(const std::string &name)
{
    std::shared_ptr<CodecBase> target = nullptr;
    target = std::make_shared<AudioFFMpegAdapter>(name);
    AVCODEC_LOGD("Create %{public}s", name.c_str());
    return target;
}
} // namespace Media
} // namespace OHOS
