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

#ifndef I_MEDIA_SERVICE_H
#define I_MEDIA_SERVICE_H

#include <memory>

#ifdef SUPPORT_CODEC
#include "i_avcodec_service.h"
#include "i_avcodeclist_service.h"
#endif

namespace OHOS {
namespace AVCodec {
class IMediaService {
public:
    virtual ~IMediaService() = default;

#ifdef SUPPORT_CODEC
    /**
     * @brief Create a codeclist service.
     *
     * All player functions must be created and obtained first.
     *
     * @return Returns a valid pointer if the setting is successful;
     * @since 1.0
     * @version 1.0
     */
    virtual std::shared_ptr<IAVCodecListService> CreateAVCodecListService() = 0;

    /**
     * @brief Destroy a codeclist service.
     *
     * call the API to destroy the codeclist service.
     *
     * @param pointer to the codeclist service.
     * @return Returns a valid pointer if the setting is successful;
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t DestroyAVCodecListService(std::shared_ptr<IAVCodecListService> avCodecList) = 0;

    /**
     * @brief Create an avcodec service.
     *
     * All player functions must be created and obtained first.
     *
     * @return Returns a valid pointer if the setting is successful;
     * @since 3.1
     * @version 3.1
     */
    virtual std::shared_ptr<IAVCodecService> CreateAVCodecService() = 0;

    /**
     * @brief Destroy a avcodec service.
     *
     * call the API to destroy the avcodec service.
     *
     * @param pointer to the avcodec service.
     * @return Returns a valid pointer if the setting is successful;
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t DestroyAVCodecService(std::shared_ptr<IAVCodecService> avCodec) = 0;
#endif

#ifdef SUPPORT_METADATA
    /**
     * @brief Create an avmetadatahelper service.
     *
     * All player functions must be created and obtained first.
     *
     * @return Returns a valid pointer if the setting is successful;
     * @since 1.0
     * @version 1.0
     */
    virtual std::shared_ptr<IAVMetadataHelperService> CreateAVMetadataHelperService() = 0;

    /**
     * @brief Destroy a avmetadatahelper service.
     *
     * call the API to destroy the avmetadatahelper service.
     *
     * @param pointer to the avmetadatahelper service.
     * @return Returns a valid pointer if the setting is successful;
     * @since 1.0
     * @version 1.0
     */
    virtual int32_t DestroyAVMetadataHelperService(std::shared_ptr<IAVMetadataHelperService> avMetadataHelper) = 0;
#endif
};

class __attribute__((visibility("default"))) MediaServiceFactory {
public:
    /**
     * @brief IMediaService singleton
     *
     * Create Recorder Service and Player Service Through the Media Service.
     *
     * @return Returns IMediaService singleton;
     * @since 1.0
     * @version 1.0
     */
    static IMediaService &GetInstance();
private:
    MediaServiceFactory() = delete;
    ~MediaServiceFactory() = delete;
};
} // namespace AVCodec
} // namespace OHOS
#endif // I_MEDIA_SERVICE_H
