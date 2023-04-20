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

#include "avcodec_local.h"
#include "media_errors.h"
#include "av_log.h"
#include "demuxer_server.h"

namespace OHOS {
namespace Media {
IAVCodecService &AVCodecServiceFactory::GetInstance()
{
    static AVCodecLocal instance;
    return instance;
}

std::shared_ptr<IAVDemuxer> AVCodecLocal::CreateDemuxerService()
{
    return AVDemuxerServer::Create();
}

int32_t AVCodecLocal::DestroyDemuxerService(std::shared_ptr<IAVDemuxer> demuxer)
{
    return AVCS_ERR_OK;
}

} // namespace Media
} // namespace OHOS
