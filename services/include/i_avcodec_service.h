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

#ifndef I_AVCODEC_SERVICE_H
#define I_AVCODEC_SERVICE_H

#include <memory>

#ifdef SUPPORT_CODEC
#include "i_codec_service.h"
#endif

#ifdef SUPPORT_CODECLIST
#include "i_codeclist_service.h"
#endif

#ifdef SUPPORT_DEMUXER
#include "i_demuxer_service.h"
#endif

#ifdef SUPPORT_MUXER
#include "i_muxer_service.h"
#endif

#ifdef SUPPORT_SOURCE
#include "i_source_service.h"
#endif

namespace OHOS {
namespace Media {
class IAVCodecService {
public:
    virtual ~IAVCodecService() = default;

#ifdef SUPPORT_CODECLIST
    /**
     * @brief Create a codeclist service.
     *
     * All player functions must be created and obtained first.
     *
     * @return Returns a valid pointer if the setting is successful;
     * @since 4.0
     * @version 4.0
     */
    virtual std::shared_ptr<ICodecListService> CreateCodecListService() = 0;

    /**
     * @brief Destroy a codeclist service.
     *
     * call the API to destroy the codeclist service.
     *
     * @param pointer to the codeclist service.
     * @return Returns a valid pointer if the setting is successful;
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t DestroyCodecListService(std::shared_ptr<ICodecListService> avCodecList) = 0;
#endif

#ifdef SUPPORT_CODEC
    /**
     * @brief Create an avcodec service.
     *
     * All player functions must be created and obtained first.
     *
     * @return Returns a valid pointer if the setting is successful;
     * @since 4.0
     * @version 4.0
     */
    virtual std::shared_ptr<ICodecService> CreateCodecService() = 0;

    /**
     * @brief Destroy a avcodec service.
     *
     * call the API to destroy the avcodec service.
     *
     * @param pointer to the avcodec service.
     * @return Returns a valid pointer if the setting is successful;
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t DestroyCodecService(std::shared_ptr<ICodecService> codec) = 0;
#endif
#ifdef SUPPORT_DEMUXER
    virtual std::shared_ptr<IDemuxerService> CreateDemuxerService() = 0;
    virtual int32_t DestroyDemuxerService(std::shared_ptr<IDemuxerService> demuxer) = 0;
#endif

#ifdef SUPPORT_MUXER
    /**
     * @brief Create an muxer service.
     *
     * All muxer functions must be created and obtained first.
     *
     * @return Returns a valid pointer if the setting is successful;
     * @since 10
     * @version 4.0
     */
    virtual std::shared_ptr<IMuxerService> CreateMuxerService() = 0;

    /**
     * @brief Destroy a muxer service.
     *
     * call the API to destroy the muxer service.
     *
     * @param pointer to the muxer service.
     * @return Returns a valid pointer if the setting is successful;
     * @since 10
     * @version 4.0
     */
    virtual int32_t DestroyMuxerService(std::shared_ptr<IMuxerService> muxer) = 0;
#endif

#ifdef SUPPORT_SOURCE
    virtual std::shared_ptr<ISourceService> CreateSourceService() = 0;
    virtual int32_t DestroySourceService(std::shared_ptr<ISourceService> source) = 0;
#endif
};

class __attribute__((visibility("default"))) AVCodecServiceFactory {
public:
    /**
     * @brief IAVCodecService singleton
     *
     * Create Muxer and Demuxer Service Through the AVCodec Service.
     *
     * @return Returns IAVCodecService singleton;
     * @since 4.0
     * @version 4.0
     */
    static IAVCodecService &GetInstance();
private:
    AVCodecServiceFactory() = delete;
    ~AVCodecServiceFactory() = delete;
};
} // namespace Media
} // namespace OHOS
#endif // I_AVCODEC_SERVICE_H
