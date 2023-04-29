#include "codec_utils.h"
#include "avcodec_log.h"

namespace OHOS { namespace Media { namespace Codec {

static const uint32_t VIDEO_ALIGN_SIZE = 16; // 16字节对齐
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "FCodec"};
}

GraphicTransformType TranslateSurfaceRotation(const SurfaceRotation& rotation)
{
    switch (rotation) {
        case SurfaceRotation::SURFACE_ROTATION_90: {
            return GRAPHIC_ROTATE_270;
        }
        case SurfaceRotation::SURFACE_ROTATION_180: {
            return GRAPHIC_ROTATE_180;
        }
        case SurfaceRotation::SURFACE_ROTATION_270: {
            return GRAPHIC_ROTATE_90;
        }
        default:
            return GRAPHIC_ROTATE_NONE;
    }
}

PixelFormat TranslateSurfaceFormat(const VideoPixelFormat& surfaceFormat)
{
    switch (surfaceFormat) {
        case VideoPixelFormat::YUV420P: {
            return PixelFormat::PIXEL_FMT_YCBCR_420_P;
        }
        case VideoPixelFormat::RGBA: {
            return PixelFormat::PIXEL_FMT_RGBA_8888;
        }
        case VideoPixelFormat::BGRA: {
            return PixelFormat::PIXEL_FMT_BGRA_8888;
        }
        case VideoPixelFormat::NV12: {
            return PixelFormat::PIXEL_FMT_YCBCR_420_SP;
        }
        case VideoPixelFormat::NV21: {
            return PixelFormat::PIXEL_FMT_YCRCB_420_SP;
        }
        default:
            return PixelFormat::PIXEL_FMT_BUTT;
    }
}
int32_t ConvertVideoFrame(std::shared_ptr<Scale> scale, std::shared_ptr<AVFrame> frame, uint8_t **dstData,
                                  int32_t *dstLineSize, AVPixelFormat dstPixFmt)
{
    if (scale == nullptr) {
        scale = std::make_shared<Scale>();
        ScalePara scalePara{static_cast<int32_t>(frame->width),
                            static_cast<int32_t>(frame->height),
                            static_cast<AVPixelFormat>(frame->format), // src fmt
                            static_cast<int32_t>(frame->width),
                            static_cast<int32_t>(frame->height),
                            dstPixFmt, // dst fmt
                            VIDEO_ALIGN_SIZE};
        CHECK_AND_RETURN_RET_LOG(scale->Init(scalePara, dstData, dstLineSize) == AVCS_ERR_OK, AVCS_ERR_UNKNOWN,
                                 "Scale init error");
    }
    return scale->Convert(frame->data, frame->linesize, dstData, dstLineSize);
}


int32_t WriteRgbDataStride(const std::shared_ptr<SurfaceMemory> &frameBuffer, uint8_t **scaleData,
                                     int32_t *scaleLineSize, VideoPixelFormat pixFmt, int32_t stride, int32_t height)
{
    if (pixFmt == VideoPixelFormat::RGBA || pixFmt == VideoPixelFormat::BGRA) {
        size_t srcPos = 0;
        size_t dstPos = 0;
        int32_t writeSize = scaleLineSize[0];
        for (uint32_t colNum = 0; colNum < height; colNum++) {
            frameBuffer->Write(scaleData[0] + srcPos, writeSize, dstPos);
            dstPos += stride;
        }
    } else {
        return AVCS_ERR_UNSUPPORT;
    }
    AVCODEC_LOGD("WriteRgbDataStride success");
    return AVCS_ERR_OK;
}

int32_t WriteYuvData(const std::shared_ptr<ShareMemory> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                               VideoPixelFormat pixFmt, int32_t height, int32_t width)
{
    size_t ySize = static_cast<size_t>(scaleLineSize[0] * height);      // yuv420: 411 nv21
    size_t uvSize = static_cast<size_t>(scaleLineSize[1] * height / 2); // 2
    size_t frameSize = 0;
    if (pixFmt == VideoPixelFormat::YUV420P) {
        frameSize = ySize + (uvSize * 2); // 2
    } else if (pixFmt == VideoPixelFormat::NV21 || pixFmt == VideoPixelFormat::NV12) {
        frameSize = ySize + uvSize;
    }
    CHECK_AND_RETURN_RET_LOG(frameBuffer->GetSize() >= frameSize, AVCS_ERR_NO_MEMORY,
                             "output buffer size is not enough: real[%{public}zu], need[%{public}zu]",
                             frameBuffer->GetSize(), frameSize);

    if (pixFmt == VideoPixelFormat::YUV420P) {
        frameBuffer->Write(scaleData[0], ySize);
        frameBuffer->Write(scaleData[1], uvSize);
        frameBuffer->Write(scaleData[2], uvSize); // 2
    } else if ((pixFmt == VideoPixelFormat::NV12) || (pixFmt == VideoPixelFormat::NV21)) {
        frameBuffer->Write(scaleData[0], ySize);
        frameBuffer->Write(scaleData[1], uvSize);
    } else {
        return AVCS_ERR_UNSUPPORT;
    }
    AVCODEC_LOGD("WriteYuvData success");
    return AVCS_ERR_OK;
}

int32_t WriteRgbData(const std::shared_ptr<SurfaceMemory> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                               VideoPixelFormat pixFmt, int32_t height, int32_t width)
{
    uint32_t stride = frameBuffer->GetSurfaceBufferStride();
    if (stride % width) {
        return WriteRgbDataStride(frameBuffer, scaleData, scaleLineSize, pixFmt, stride, height);
    }
    

    size_t frameSize = static_cast<size_t>(scaleLineSize[0] * height);
    CHECK_AND_RETURN_RET_LOG(frameBuffer->GetSize() >= frameSize, AVCS_ERR_NO_MEMORY,
                             "output buffer size is not enough: real[%{public}zu], need[%{public}zu]",
                             frameBuffer->GetSize(), frameSize);
    if (pixFmt == VideoPixelFormat::RGBA || pixFmt == VideoPixelFormat::BGRA) {
        frameBuffer->Write(scaleData[0], frameSize);
    } else {
        return AVCS_ERR_UNSUPPORT;
    }
    AVCODEC_LOGD("WriteRgbData success");
    return AVCS_ERR_OK;
}

}}} // namespace OHOS::Media::Codec