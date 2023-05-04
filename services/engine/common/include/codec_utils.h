#ifndef CODEC_UTILS_H
#define CODEC_UTILS_H

#include "surface/surface.h"
#include "avcodec_errors.h"
#include "surface_memory.h"
#include "share_memory.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
};
#endif

namespace OHOS { namespace Media { namespace Codec {
enum SurfaceRotation : uint32_t {
    SURFACE_ROTATION_0 = 0,
    SURFACE_ROTATION_90 = 90,
    SURFACE_ROTATION_180 = 180,
    SURFACE_ROTATION_270 = 270,
};

enum struct VideoFormat : uint8_t {
    UNKNOWN = 0,
    H264 = 1,
    MPEG4 = 2,
};

enum struct Tag : uint32_t {
    INVALID = 0,
    REQUIRED_IN_BUFFER_CNT,   
    REQUIRED_OUT_BUFFER_CNT,          ///< uint32_t, required buffer count of plugin; read only tag
    VIDEO_SCALE_TYPE,                 ///< VideoScaleType, video scale type
    VIDEO_WIDTH, 
    VIDEO_HEIGHT,                                    ///< uint32_t, video height
    VIDEO_PIXEL_FORMAT,                              ///< @see VideoPixelFormat
    VIDEO_FRAME_RATE,                                ///< uint32_t, video frame rate
    MEDIA_BITRATE,
    SURFACE_PIXFORMAT,
    SURFACE_SCALE_TYPE,
    SURFACE_ROTATION,          
};

enum struct VideoPixelFormat : uint32_t {
    UNKNOWN,
    YUV410P,   ///< planar YUV 4:1:0, 1 Cr & Cb sample per 4x4 Y samples
    YUV411P,   ///< planar YUV 4:1:1, 1 Cr & Cb sample per 4x1 Y samples
    YUV420P,   ///< planar YUV 4:2:0, 1 Cr & Cb sample per 2x2 Y samples
    NV12,      ///< semi-planar YUV 4:2:0, UVUV...
    NV21,      ///< semi-planar YUV 4:2:0, VUVU...
    YUYV422,   ///< packed YUV 4:2:2, Y0 Cb Y1 Cr
    YUV422P,   ///< planar YUV 4:2:2, 1 Cr & Cb sample per 2x1 Y samples
    YUV444P,   ///< planar YUV 4:4:4, 1 Cr & Cb sample per 1x1 Y samples
    RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    ARGB,      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    ABGR,      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
    RGB24,     ///< packed RGB 8:8:8, RGBRGB...
    BGR24,     ///< packed RGB 8:8:8, BGRBGR...
    PAL8,      ///< 8 bit with AV_PIX_FMT_RGB32 palette
    GRAY8,     ///< Y
    MONOWHITE, ///< Y, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    MONOBLACK, ///< Y, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG)
    YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG)
    YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG)
};

struct ScalePara {
    int32_t srcWidth {0};
    int32_t srcHeight {0};
    AVPixelFormat srcFfFmt {AVPixelFormat::AV_PIX_FMT_NONE};
    int32_t dstWidth {0};
    int32_t dstHeight {0};
    AVPixelFormat dstFfFmt {AVPixelFormat::AV_PIX_FMT_RGBA};
    int32_t align {16};
};

struct Scale {
public:
    int32_t Init(const ScalePara& scalePara, uint8_t** dstData, int32_t* dstLineSize);
    int32_t Convert(uint8_t** srcData, const int32_t* srcLineSize, uint8_t** dstData, int32_t* dstLineSize);
private:
    ScalePara scalePara_ {};
    std::shared_ptr<SwsContext> swsCtx_ {nullptr};
};

PixelFormat TranslateSurfaceFormat(const VideoPixelFormat& surfaceFormat);
VideoPixelFormat ConvertPixelFormatFromFFmpeg(int32_t ffmpegPixelFormat);
AVPixelFormat ConvertPixelFormatToFFmpeg(VideoPixelFormat pixelFormat);
int32_t ConvertVideoFrame(std::shared_ptr<Scale> scale, std::shared_ptr<AVFrame> frame, uint8_t **dstData,
                       int32_t *dstLineSize, AVPixelFormat dstPixFmt);
int32_t WriteRgbDataStride(const std::shared_ptr<SurfaceMemory> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                          VideoPixelFormat pixFmt, int32_t stride, int32_t height);
int32_t WriteYuvData(const std::shared_ptr<ShareMemory> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                    VideoPixelFormat pixFmt, int32_t height, int32_t width);
int32_t WriteRgbData(const std::shared_ptr<SurfaceMemory> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                    VideoPixelFormat pixFmt, int32_t height, int32_t width);


std::string AVStrError(int errnum);
bool IsYuvFormat(AVPixelFormat format);
bool IsRgbFormat(AVPixelFormat format);

}}} // namespace OHOS::Media::Codec
#endif