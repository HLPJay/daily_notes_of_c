## 1 ：ffmpeg视频 解码流程：

### 1.1 视频 解码流程图：

![image-20210801171424322](C:\Users\yun68\Desktop\md文档整理_待完成\daily_notes\md文档相关图片\ffmpeg视频编解码流程.png)

### 1.2 关键函数：

```c
关键函数说明：
	avcodec_find_decoder：		根据指定的AVCodecID查找注册的解码器。
	av_parser_init：				初始化AVCodecParserContext。
	avcodec_alloc_context3：		为AVCodecContext分配内存。
	avcodec_open2：				打开解码器。
	av_parser_parse2：			解析获得⼀个Packet。
	avcodec_send_packet：		将AVPacket压缩数据给解码器。
	avcodec_receive_frame：		获取到解码后的AVFrame数据。
	av_get_bytes_per_sample: 	 获取每个sample中的字节数。
```

### 1.3 关键数据结构：

​	AVCodecParser：⽤于解析输⼊的数据流并把它分成⼀帧⼀帧的压缩编码数据。

​	⽐较形象 的说法就是把⻓⻓的⼀段连续的数据“切割”成⼀段段的数据。

```c
//⽐如H264 aac_parser  ⽐如H264 aac_parser
AVCodecParser ff_h264_parser = {
	.codec_ids = { AV_CODEC_ID_H264 },
	.priv_data_size = sizeof(H264ParseContext),
	.parser_init = init,
	.parser_parse = h264_parse,
	.parser_close = h264_close,
	.split = h264_split,
};
//从AVCodecParser结构的实例化我们可以看出来，不同编码类型的parser是和CODE_ID进⾏绑定的。
//所以也就可以解释:
	parser = av_parser_init(AV_CODEC_ID_H264);
//可以通过CODE_ID查找到对应的码流 parser。
```

### 1.4 解码API介绍

```
FFmpeg提供了两组函数，分别⽤于编码和解码：
	解码：avcodec_send_packet()、avcodec_receive_frame()。
	解码：avcodec_send_frame()、avcodec_receive_packet()。
```

API的编解码流程，使用流程：

```
建议的使⽤流程如下：
	1. 像以前⼀样设置并打开AVCodecContext。
	2. 输⼊有效的数据：
			解码：调⽤avcodec_send_packet()给解码器传⼊包含原始的压缩数据的AVPacket对象。
			编码：调⽤ avcodec_send_frame()给编码器传⼊包含解压数据的AVFrame对象。
		两种情况下推荐AVPacket和AVFrame都使⽤refcounted（引⽤计数）的模式，否则libavcodec可能不得不对输⼊的数据进⾏拷⻉。
	3. 在⼀个循环体内去接收codec的输出，即周期性地调⽤avcodec_receive_*()来接收codec输出的数据：
			解码：调⽤avcodec_receive_frame()，如果成功会返回⼀个包含未压缩数据的AVFrame。
			编码：调⽤avcodec_receive_packet()，如果成功会返回⼀个包含压缩数据的AVPacket。
		反复地调⽤avcodec_receive_packet()直到返回 AVERROR(EAGAIN)或其他错误。
			返回AVERROR(EAGAIN)错误表示codec需要新的输⼊来输出更多的数据。
			对于每个输⼊的packet或frame，codec⼀般会输出⼀个frame或packet，但是也有可能输出0个或者多于1个。
	4. 流处理结束的时候需要flush（冲刷） codec。
		因为codec可能在内部缓冲多个frame或packet，出于性能或其他必要的情况（如考虑B帧的情况）。
        处理流程如下：
			调⽤avcodec_send_*()传⼊的AVFrame或AVPacket指针设置为NULL。 
			这将进⼊draining mode（排⽔模式）。
	反复地调⽤avcodec_receive_*()直到返回AVERROR_EOF，该⽅法在draining mode时不会返回AVERROR(EAGAIN)的错误，除⾮你没有进⼊draining mode。
	当重新开启codec时，需要先调⽤ avcodec_flush_buffers()来重置codec。
```

解码API流程说明：

```
说明：
	1. 编码或者解码刚开始的时候，codec可能接收了多个输⼊的frame或packet后还没有输出数据，直到内部的buffer被填充满。上⾯的使⽤流程可以处理这种情况。
	2. 理论上，只有在输出数据没有被完全接收的情况调⽤avcodec_send_*()的时候才可能会发⽣AVERROR(EAGAIN)的错误。
		你可以依赖这个机制来实现区别于上⾯建议流程的处理⽅式，
		⽐如每次循环都调⽤avcodec_send_*()，在出现AVERROR(EAGAIN)错误的时候再去调⽤avcodec_receive_*()。
	3. 并不是所有的codec都遵循⼀个严格、可预测的数据处理流程，
		唯⼀可以保证的是 “调⽤avcodec_send_*()/avcodec_receive_*()返回AVERROR(EAGAIN)的时候去avcodec_receive_*()/avcodec_send_*()会成功，否则不应该返回AVERROR(EAGAIN)的错误。
		”⼀般来说，任何codec都不允许⽆限制地缓存输⼊或者输出。
	4. 在同⼀个AVCodecContext上混合使⽤新旧API是不允许的，这将导致未定义的⾏为。
```

### 1.5 avcodec_send_packet和avcodec_receive_frame

```c
//函数：
    int avcodec_send_packet(AVCodecContext *avctx, const AVPacket *avpkt);
//作⽤：⽀持将裸流数据包送给解码器
警告：
	输⼊的avpkt-data缓冲区必须⼤于AV_INPUT_PADDING_SIZE，因为优化的字节流读取器必须⼀次读取32或者64⽐特的数据
	不能跟之前的API(例如avcodec_decode_video2)混⽤，否则会返回不可预知的错误
备注：
	在将包发送给解码器的时候，AVCodecContext必须已经通过avcodec_open2打开
参数：
	avctx：		解码上下⽂
	avpkt：		输⼊AVPakcet.
    	通常情况下，输⼊数据是⼀个单⼀的视频帧或者⼏个完整的⾳频帧。
    		1:调⽤者保留包的原有属性，解码器不会修改包的内容。
    		2:解码器可能创建对包的引⽤。
			3:如果包没有引⽤计数将拷⻉⼀份。跟以往的API不⼀样，输⼊的包的数据将被完全地消耗，
			4:如果包含有多个帧，要求多次调⽤avcodec_recvive_frame，直到avcodec_recvive_frame返回VERROR(EAGAIN)或AVERROR_EOF。
    		5:输⼊参数可以为NULL，或者AVPacket的data域设置为NULL或者size域设置为0，表示将刷新所有的包，意味着数据流已经结束了。
    第⼀次发送刷新会总会成功，第⼆次发送刷新包是没有必要的，并且返回AVERROR_EOF,
		如果×××缓存了⼀些帧，返回⼀个刷新包，将会返回所有的解码包
返回值：
	0: 					 表示成功
	AVERROR(EAGAIN)：	当前状态不接受输⼊，⽤户必须先使⽤avcodec_receive_frame() 读取数据帧；
	AVERROR_EOF：		解码器已刷新，不能再向其发送新包；
	AVERROR(EINVAL)：	没有打开解码器，或者这是⼀个编码器，或者要求刷新；
	AVERRO(ENOMEN)：		⽆法将数据包添加到内部队列。
```

```c
//函数：
	int avcodec_receive_frame ( AVCodecContext * avctx, AVFrame * frame )
//作⽤：从解码器返回已解码的输出数据。
参数：
	avctx: 			编解码器上下⽂
	frame: 			获取使⽤reference-counted机制的audio或者video帧（取决于解码器类型）。
        	请注意，在执⾏其他操作之前，函数内部将始终先调⽤av_frame_unref(frame)。
返回值：
	0: 						成功，返回⼀个帧
	AVERROR(EAGAIN):	 	该状态下没有帧输出，需要使⽤avcodec_send_packet发送新的packet到解码器
	AVERROR_EOF: 			解码器已经被完全刷新，不再有输出帧
	AVERROR(EINVAL): 		编解码器没打开其他<0的值: 具体查看对应的错误码
```

## 2 ffmpeg音频 解码流程

### 2.1 音频编解码流程：

![image-20210801181114283](C:\Users\yun68\Desktop\md文档整理_待完成\daily_notes\md文档相关图片\ffmpeg音频编解码流程.png)

### 2.2 音频解码相关函数

```
关键函数说明：
	avcodec_find_decoder：		根据指定的AVCodecID查找注册的解码器。
	av_parser_init：				初始化AVCodecParserContext。
	avcodec_alloc_context3：		为AVCodecContext分配内存。
	
	avcodec_open2：				打开解码器。
	av_parser_parse2：			解析获得⼀个Packet。
	avcodec_send_packet：		将AVPacket压缩数据给解码器。
	avcodec_receive_frame：		获取到解码后的AVFrame数据。
	av_get_bytes_per_sample:	获取每个sample中的字节数。
```

### 2.3 音频解码相关函数：

关键数据结构说明： 

​		AVCodecParser：⽤于解析输⼊的数据流并把它分成⼀帧⼀帧的压缩编码数据。

​				⽐较形象 的说法就是把⻓⻓的⼀段连续的数据“切割”成⼀段段的数据。

​				 ⽐如AAC aac_parser

ffmpeg-4.2.1\libavcodec\aac_parser.c

```c
AVCodecParser ff_aac_parser = {
	.codec_ids = { AV_CODEC_ID_AAC },
	.priv_data_size = sizeof(AACAC3ParseContext),
	.parser_init = aac_parse_init,
	.parser_parse = ff_aac_ac3_parse,
	.parser_close = ff_parse_close,
};
//从AVCodecParser结构的实例化我们可以看出来，不同编码类型的parser是和CODE_ID进⾏绑定的。
//所以也就可以解释：
	parser = av_parser_init(codec->id);
//可以通过CODE_ID查找到对应的码流 parser。
```



### 2.4 音频解码API介绍：

​	参考视频编解码api介绍

## 3：解码练习：

### 3.1： 目标YUV,PCM格式

### 3.2： MP4， flv， AAC格式解码分析

