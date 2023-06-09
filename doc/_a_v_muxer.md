# AVMuxer


## 概述

AVMuxer模块提供用于音视频封装功能的函数。

\@syscap SystemCapability.Multimedia.Avcodec.AVMuxer

**Since:**

10



## 汇总


### 文件

| 名称 | 描述 | 
| -------- | -------- |
| [native_avmuxer.h](native__avmuxer_8h.md) | 声明用于音视频封装的Native API。 | 


### 函数

| 名称 | 描述 | 
| -------- | -------- |
| [OH_AVMuxer_Create](#oh_avmuxer_create) (int32_t fd, OH_AVOutputFormat format) | 通过文件描述符fd和封装格式创建OH_AVMuxer实例。 | 
| [OH_AVMuxer_SetLocation](#oh_avmuxer_setlocation) (OH_AVMuxer \*muxer, float latitude, float longitude) | 设置输出文件的经纬度。 | 
| [OH_AVMuxer_SetRotation](#oh_avmuxer_setrotation) (OH_AVMuxer \*muxer, int32_t rotation) | 设置视频的旋转角度。 | 
| [OH_AVMuxer_AddTrack](#oh_avmuxer_addtrack) (OH_AVMuxer \*muxer, int32_t \*trackIndex, OH_AVFormat \*trackFormat) | 向封装器添加媒体轨。 | 
| [OH_AVMuxer_Start](#oh_avmuxer_start) (OH_AVMuxer \*muxer) | 开始封装。 | 
| [OH_AVMuxer_WriteSampleBuffer](#oh_avmuxer_writesamplebuffer) (OH_AVMuxer \*muxer, uint32_t trackIndex, uint8_t \*sampleBuffer, OH_AVCodecBufferAttr info) | 将数据写入封装器。 | 
| [OH_AVMuxer_Stop](#oh_avmuxer_stop) (OH_AVMuxer \*muxer) | 停止封装。 | 
| [OH_AVMuxer_Destroy](#oh_avmuxer_destroy) (OH_AVMuxer \*muxer) | 清理内部资源，销毁OH_AVMuxer实例。 | 


## 函数说明


### OH_AVMuxer_AddTrack()

  
```
OH_AVErrCode OH_AVMuxer_AddTrack (OH_AVMuxer * muxer, int32_t * trackIndex, OH_AVFormat * trackFormat )
```

**描述:**

向封装器添加媒体轨。

该接口必须在OH_AVMuxer_Start后调用。

\@syscap SystemCapability.Multimedia.Media.AVMuxer

**参数:**

| 名称 | 描述 | 
| -------- | -------- |
| muxer | 指向OH_AVMuxer实例的指针 | 
| trackIndex | 用于获取该轨的索引。该值在OH_AVMuxer_WriteSampleBuffer接口中使用 如果媒体轨添加成功，该值大于或等于0，否则小于0 | 
| trackFormat | 指向OH_AVFormat实例的指针 | 

**返回:**

执行成功返回AV_ERR_OK。

执行失败返回具体错误码，参考[OH_AVErrCode](_core.md#oh_averrcode-1)。


### OH_AVMuxer_Create()

  
```
OH_AVMuxer* OH_AVMuxer_Create (int32_t fd, OH_AVOutputFormat format )
```

**描述:**

通过文件描述符fd和封装格式创建OH_AVMuxer实例。

\@syscap SystemCapability.Multimedia.Media.AVMuxer

**参数:**

| 名称 | 描述 | 
| -------- | -------- |
| fd | fd用读写方式打开（O_RDWR），由使用者关闭该fd | 
| format | 封装输出的文件格式，参考**OH_AVOutputFormat** | 

**返回:**

返回一个指向OH_AVMuxer实例的指针, 需要调用OH_AVMuxer_Destroy销毁


### OH_AVMuxer_Destroy()

  
```
OH_AVErrCode OH_AVMuxer_Destroy (OH_AVMuxer * muxer)
```

**描述:**

清理内部资源，销毁OH_AVMuxer实例。

\@syscap SystemCapability.Multimedia.Media.AVMuxer

**参数:**

| 名称 | 描述 | 
| -------- | -------- |
| codec | Pointer to an OH_AVMuxer instance | 

**返回:**

执行成功返回AV_ERR_OK，指向成功需要调用者将muxer置空

执行失败返回具体错误码，参考[OH_AVErrCode](_core.md#oh_averrcode-1)


### OH_AVMuxer_SetLocation()

  
```
OH_AVErrCode OH_AVMuxer_SetLocation (OH_AVMuxer * muxer, float latitude, float longitude )
```

**描述:**

设置输出文件的经纬度。

这个接口必须在OH_AVMuxer_Start前调用。

\@syscap SystemCapability.Multimedia.Media.AVMuxer

**参数:**

| 名称 | 描述 | 
| -------- | -------- |
| muxer | 指向OH_AVMuxer实例的指针 | 
| latitude | 纬度，范围必须是[-90, 90] | 
| longitude | 经度，范围必须是[-180, 180] | 

**返回:**

执行成功返回AV_ERR_OK

执行失败返回具体错误码，参考[OH_AVErrCode](_core.md#oh_averrcode-1)


### OH_AVMuxer_SetRotation()

  
```
OH_AVErrCode OH_AVMuxer_SetRotation (OH_AVMuxer * muxer, int32_t rotation )
```

**描述:**

设置视频的旋转角度。

这个接口必须在OH_AVMuxer_Start前调用。

\@syscap SystemCapability.Multimedia.Media.AVMuxer

**参数:**

| 名称 | 描述 | 
| -------- | -------- |
| muxer | 指向OH_AVMuxer实例的指针 | 
| rotation | 角度，必须为0, 90, 180, 或 270，参考**OH_VideoRotation** | 

**返回:**

执行成功返回AV_ERR_OK

执行失败返回具体错误码，参考[OH_AVErrCode](_core.md#oh_averrcode-1)


### OH_AVMuxer_Start()

  
```
OH_AVErrCode OH_AVMuxer_Start (OH_AVMuxer * muxer)
```

**描述:**

开始封装。

该接口必须在OH_AVMuxer_AddTrack后， 再OH_AVMuxer_WriteSampleBuffer前调用。

\@syscap SystemCapability.Multimedia.Media.AVMuxer

**参数:**

| 名称 | 描述 | 
| -------- | -------- |
| muxer | 指向OH_AVMuxer实例的指针 | 

**返回:**

执行成功返回AV_ERR_OK

执行失败返回具体错误码，参考[OH_AVErrCode](_core.md#oh_averrcode-1)


### OH_AVMuxer_Stop()

  
```
OH_AVErrCode OH_AVMuxer_Stop (OH_AVMuxer * muxer)
```

**描述:**

停止封装。

封装器一旦停止，不能重新开始。

\@syscap SystemCapability.Multimedia.Media.AVMuxer

**参数:**

| 名称 | 描述 | 
| -------- | -------- |
| muxer | 指向OH_AVMuxer实例的指针 | 

**返回:**

执行成功返回AV_ERR_OK

执行失败返回具体错误码，参考[OH_AVErrCode](_core.md#oh_averrcode-1)


### OH_AVMuxer_WriteSampleBuffer()

  
```
OH_AVErrCode OH_AVMuxer_WriteSampleBuffer (OH_AVMuxer * muxer, uint32_t trackIndex, uint8_t * sampleBuffer, OH_AVCodecBufferAttr info )
```

**描述:**

将数据写入封装器。

该接口必须在OH_AVMuxer_Start后，OH_AVMuxer_Stop前调用。调用者需要保证数据写入正确的轨道。

调用者需要保证数据写入的顺序是按时间递增的。

\@syscap SystemCapability.Multimedia.Media.AVMuxer

**参数:**

| 名称 | 描述 | 
| -------- | -------- |
| muxer | 指向OH_AVMuxer实例的指针 | 
| trackIndex | 数据对应的媒体轨的索引 | 
| sampleBuffer | 写入数据的地址，一般是编码后的数据 | 
| info | 写入数据的信息 ，参考**OH_AVCodecBufferAttr** | 

**返回:**

执行成功返回AV_ERR_OK

执行失败返回具体错误码，参考[OH_AVErrCode](_core.md#oh_averrcode-1)
