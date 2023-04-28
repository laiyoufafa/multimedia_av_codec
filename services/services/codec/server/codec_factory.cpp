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

#include "codec_factory.h"
#include <limits>
#include <cinttypes>
#include <dlfcn.h>
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "audio_ffmpeg_adapter.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecFactory"};
}

namespace OHOS {
namespace Media {

CodecFactory &CodecFactory::Instance()
{
    static CodecFactory inst;
    return inst;
}

CodecFactory::~CodecFactory()
{
}

std::shared_ptr<CodecBase> CodecFactory::CreateCodecByMime(bool isEncoder, const std::string &mime)
{
    (void)isEncoder;
    (void)mime;
    AVCODEC_LOGD("CreateCodecByMime is not support");
    return nullptr;
}

std::shared_ptr<CodecBase> CodecFactory::CreateCodecByName(const std::string &name)
{
    std::shared_ptr<CodecBase> codec = nullptr;
    codec = std::make_shared<AudioFFMpegAdapter>(name);
    AVCODEC_LOGD("Create %{public}s", name.c_str());
    return codec;
}
} // namespace Media
} // namespace OHOS
