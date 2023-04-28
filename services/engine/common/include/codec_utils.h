#ifndef CODEC_UTILS_H
#define CODEC_UTILS_H

#include "plugin_video_tags.h"
#include "plugin_types.h"
#include "plugin_buffer.h"
#include "surface/surface.h"
#include "ffmpeg_utils.h"
#include "avcodec_errors.h"
#include "share_memory.h"

namespace OHOS { namespace Media { namespace Codec {

enum SurfaceRotation : uint32_t {
    SURFACE_ROTATION_0 = 0,
    SURFACE_ROTATION_90 = 90,
    SURFACE_ROTATION_180 = 180,
    SURFACE_ROTATION_270 = 270,
};

GraphicTransformType TranslateSurfaceRotation(const SurfaceRotation& rotation);
PixelFormat TranslateSurfaceFormat(const VideoPixelFormat& surfaceFormat);

VideoPixelFormat TranslateSurfaceFormat(PixelFormat surfaceFormat);
int32_t ConvertVideoFrame(std::shared_ptr<Scale> scale, std::shared_ptr<AVFrame> frame, uint8_t **dstData,
                       int32_t *dstLineSize, AVPixelFormat dstPixFmt);
int32_t WriteRgbDataStride(const std::shared_ptr<Buffer> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                          VideoPixelFormat pixFmt, int32_t stride, int32_t height);
int32_t WriteYuvData(const std::shared_ptr<ShareMemory> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                    VideoPixelFormat pixFmt, int32_t height, int32_t width);
int32_t WriteRgbData(const std::shared_ptr<Buffer> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                    VideoPixelFormat pixFmt, int32_t height, int32_t width);

}}} // namespace OHOS::Media::Codec
#endif