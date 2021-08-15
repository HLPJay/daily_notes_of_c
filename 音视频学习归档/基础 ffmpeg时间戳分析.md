# 1：  I帧/P帧/B帧

```
	I帧：I帧(Intra-coded picture, 帧内编码帧，常称为关键帧)包含⼀幅完整的图像信息，属于帧内编码图像，不含运动⽮量，在解码时不需要参考其他帧图像。
		因此在I帧图像处可以切换频道，⽽不会导致图像丢失或⽆法解码。
		I帧图像⽤于阻⽌误差的累积和扩散。
		在闭合式GOP中，每个GOP的第⼀个帧⼀定是I帧，且当前GOP的数据不会参考前后GOP的数据。
	P帧：P帧(Predictive-coded picture, 预测编码图像帧)是帧间编码帧，
		利⽤之前的I帧或P帧进⾏预测编码。

	B帧：B帧(Bi-directionally predicted picture, 双向预测编码图像帧)是帧间编码帧，利⽤之前和(或)之后的I帧或P帧进⾏双向预测编码。
		B帧不可以作为参考帧。
		B帧具有更⾼的压缩率，但需要更多的缓冲时间以及更⾼的CPU占⽤率，
			B帧适合本地存储以及视频点播，⽽不适⽤对实时性要求较⾼的直播系统。
```

# 2： DTS和PTS

```
	DTS(Decoding Time Stamp, 解码时间戳)，表示压缩帧的解码时间。
	PTS(Presentation Time Stamp, 显示时间戳)，表示将压缩帧解码后得到的原始帧的显示时间。

	⾳频中DTS和PTS是相同的。
	视频中由于B帧需要双向预测，B帧依赖于其前和其后的帧，
		因此含B帧的视频解码顺序与显示顺序不同，即DTS与PTS不同。
		当然，不含B帧的视频，其DTS和PTS是相同的。
```

下图以⼀个开放式GOP示意图为例，说明视频流的解码顺序和显示顺序：

![image-20210801200203051](..\md文档相关图片\ffmpegI帧P帧B帧GOP示意图.png)

```bash
采集顺序指图像传感器采集原始信号得到图像帧的顺序。
编码顺序指编码器编码后图像帧的顺序。
	存储到磁盘的本地视频⽂件中图像帧的顺序与编码顺序相同。
传输顺序指编码后的流在⽹络中传输过程中图像帧的顺序。
解码顺序指解码器解码图像帧的顺序。
显示顺序指图像帧在显示器上显示的顺序。

采集顺序与显示顺序相同。编码顺序、传输顺序和解码顺序相同。
	以图中“B[1]”帧为例进⾏说明，“B[1]”帧解码时需要参考“I[0]”帧和“P[3]”帧，因此“P[3]”帧必须⽐“B[1]”帧先解码。
	这就导致了解码顺序和显示顺序的不⼀致，后显示的帧需要先解码。
```

# 3: FFmpeg中的时间基与时间戳

## 3.1 时间基与时间戳的概念:

```
	在FFmpeg中，时间基(time_base)是时间戳(timestamp)的单位，时间戳值乘以时间基，可以得到实际的时刻值(以秒等为单位)。
		例如，如果⼀个视频帧的dts是40，pts是160，其time_base是1/1000秒，那么可以计算出此视频帧的解码时刻是40毫秒(40/1000)，显示时刻是160毫秒(160/1000)。
		FFmpeg中时间戳(pts/dts)的类型是int64_t类型，把⼀个time_base看作⼀个时钟脉冲，则可把dts/pts看作时钟脉冲的计数。
```

## 3.2 三种时间基tbr、tbn和tbc

```
不同的封装格式具有不同的时间基。
	在FFmpeg处理⾳视频过程中的不同阶段，也会采⽤不同的时间基。
FFmepg中有三种时间基，命令⾏中tbr、tbn和tbc的打印值就是这三种时间基的倒数：
	tbn：	对应容器中的时间基。		 值是AVStream.time_base的倒数
	tbc：	对应编解码器中的时间基。	值是AVCodecContext.time_base的倒数
	tbr：	从视频流中猜算得到，可能是帧率或场率(帧率的2倍)
```

测试⽂件下载(右键另存为)：tnmil3.flv
使⽤ffprobe探测媒体⽂件格式，可以了解文件中使用的相关编码，如下：

```bash
1 think@opensuse> ffprobe tnmil3.flv
2 ffprobe version 4.1 Copyright (c) 2007-2018 the FFmpeg developers
3 Input #0, flv, from 'tnmil3.flv':
4 Metadata:
5 encoder : Lavf58.20.100
6 Duration: 00:00:03.60, start: 0.017000, bitrate: 513 kb/s
7 Stream #0:0: Video: h264 (High), yuv420p(progressive), 784x480, 2
5 fps, 25 tbr, 1k tbn, 50 tbc
8 Stream #0:1: Audio: aac (LC), 44100 Hz, stereo, fltp, 128 kb/s
```

## 3.3 内部时间基AV_TIME_BASE

除以上三种时间基外，FFmpeg还有⼀个**内部时间基AV_TIME_BASE**(以及分数形式的 **AV_TIME_BASE_Q**）

```c
1 // Internal time base represented as integer
2 #define AV_TIME_BASE 1000000 //微妙
3 // Internal time base represented as fractional value
4 #define AV_TIME_BASE_Q (AVRational){1, AV_TIME_BASE}
```

AV_TIME_BASE及AV_TIME_BASE_Q⽤于FFmpeg内部函数处理，使⽤此时间基计算得到时间值表示的 是微秒。

## 3.4 时间值形式转换

​		**av_q2d()**将时间从AVRational形式转换为double形式。

​		AVRational是分数类型，double是双精度浮点数 类型，转换的结果单位是秒。

​		转换前后的值基于同⼀时间基，仅仅是数值的表现形式不同⽽已。 

av_q2d()实现如下：

```c
static inline double av_q2d(AVRational a){
	return a.num / (double) a.den;
}

//av_q2d()使⽤⽅法如下：
AVStream stream;
AVPacket packet;
packet播放时刻值：	timestamp(单位秒) = packet.pts * av_q2d(stream.time_base);
packet播放时⻓值：	duration(单位秒) = packet.duration * av_q2d(stream.time_base);
```

## 3.5 时间基转换函数

​		**av_rescale_q()**⽤于不同时间基的转换，⽤于将时间值从⼀种时间基转换为另⼀种时间基。

​			 将a数值由 bq时间基转成 cq的时间基，

​			通过返回结果获取以cq时间基表示的新数值。

```c
int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq) av_const;

```

​		**av_rescale_rnd()**  是计算 "a * b / c" 的值并分五种⽅式来取整

```c

int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd)
//它的作⽤是计算 "a * b / c" 的值并分五种⽅式来取整:

AV_ROUND_ZERO = 0, 		// Round toward zero. 趋近于0， round(2.5) 为 2, ⽽round(-2.5) 为 -2
AV_ROUND_INF = 1, 		// Round away from zero. 趋远于0 round(3.5)=4, round(-3.5)=-4
AV_ROUND_DOWN = 2, 		// Round toward -infinity.向负⽆穷⼤⽅向 [-2.9, -1.2, 2.4, 5.6,7.0, 2.4] -> [-3, -2, 2, 5, 7, 2]
AV_ROUND_UP = 3, 		// Round toward +infinity. 向正⽆穷⼤⽅向[-2.9, -1.2, 2.4, 5.6,7.0, 2.4] -> [-2, -1, 3, 6, 7, 3]
AV_ROUND_NEAR_INF = 5, 	// Round to nearest and halfway cases away from zero. 
						// 四舍五⼊,⼩于0.5取值趋向0,⼤于0.5取值趋远于0
```

​	**av_packet_rescale_ts()**⽤于将**AVPacket**中各种时间值从⼀种时间基转换为另⼀种时间基。

```c
void av_packet_rescale_ts(AVPacket *pkt, AVRational tb_src, AVRational tb_dst);
	//将AVPacket中各种时间值从⼀种时间基转换为另⼀种时间基。
```

## 3.6 转封装过程中的时间基转换

容器中的时间基(AVStream.time_base，3.2节中的tbn)定义如下：

```c
typedef struct AVStream {
	......

	AVRational time_base;
	......
	转封装过程中的时间基转换
}

AVStream.time_base是AVPacket中pts和dts的时间单位，
 输⼊流与输出流中time_base按如下⽅式确定：
	对于输⼊流：打开输⼊⽂件后，调⽤avformat_find_stream_info()可获取到每个流中的time_base
	对于输出流：打开输出⽂件后，调⽤avformat_write_header()可根据输出⽂件封装格式确定每个流的time_base并写⼊输出⽂件中
```

**不同封装格式具有不同的时间基**，在转封装(**将⼀种封装格式转换为另⼀种封装格式**)过程中，时间基转换相 关代码如下：

```c
//在转封装过程中，时间基转换相关代码如下:	
	av_read_frame(ifmt_ctx, &pkt);
	pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
	pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
	pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);

//下⾯的代码具有和上⾯代码相同的效果：
	// 从输⼊⽂件中读取packet
	av_read_frame(ifmt_ctx, &pkt);
	// 将packet中的各时间值从输⼊流封装格式时间基转换到输出流封装格式时间基
	av_packet_rescale_ts(&pkt, in_stream->time_base, out_stream->time_base);
		// in_stream->time_base 和 out_stream->time_base ，是容器中的时间基
```

例如：flv封装格式的time_base为{1,1000}，ts封装格式的time_base为{1,90000}

我们编写程序将flv封装格式转换为ts封装格式，抓取原⽂件(flv)的前四帧显示时间戳：

```bash
#抓取原⽂件(flv)的前四帧显示时间戳：
think@opensuse> ffprobe -show_frames -select_streams v tnmil3.flv | grep pkt_pts
ffprobe version 4.1 Copyright (c) 2007-2018 the FFmpeg developers
Input #0, flv, from 'tnmil3.flv':
Metadata:
encoder : Lavf58.20.100
Duration: 00:00:03.60, start: 0.017000, bitrate: 513 kb/s
Stream #0:0: Video: h264 (High), yuv420p(progressive), 784x480,25 fps, 25 tbr, 1k tbn, 50 tbc
Stream #0:1: Audio: aac (LC), 44100 Hz, stereo, fltp, 128 kb/s
pkt_pts=80
pkt_pts_time=0.080000
pkt_pts=120
pkt_pts_time=0.120000
pkt_pts=160
pkt_pts_time=0.160000
pkt_pts=200
pkt_pts_time=0.200000

#再抓取转换的⽂件(ts)的前四帧显示时间戳：
think@opensuse> ffprobe -show_frames -select_streams v tnmil3.ts | grep pkt_pts
ffprobe version 4.1 Copyright (c) 2007-2018 the FFmpeg developers
Input #0, mpegts, from 'tnmil3.ts':
Duration: 00:00:03.58, start: 0.017000, bitrate: 619 kb/s
Program 1
Metadata:
service_name : Service01
service_provider: FFmpeg
Stream #0:0[0x100]: Video: h264 (High) ([27][0][0][0] / 0x001B),yuv420p(progressive), 784x480, 25 fps, 25 tbr, 90k tbn, 50 tbc
Stream #0:1[0x101]: Audio: aac (LC) ([15][0][0][0] / 0x000F), 44100 Hz, stereo, fltp, 127 kb/s
pkt_pts=7200
pkt_pts_time=0.080000
pkt_pts=10800
pkt_pts_time=0.120000
pkt_pts=14400
pkt_pts_time=0.160000
pkt_pts=18000
pkt_pts_time=0.200000

#可以发现，对于同⼀个视频帧，它们时间基(tbn)不同因此时间戳(pkt_pts)也不同，但是计算出来的时刻值(pkt_pts_time)是相同的。
#看第⼀帧的时间戳，计算关系：80×{1,1000} == 7200×{1,90000} == 0.080000
```

## 3.7 转码过程中的时间基转换

编解码器中的时间基(AVCodecContext.time_base，3.2节中的tbc)定义如下：

```c
typedef struct AVCodecContext {
	......

	AVRational time_base;
	......
}

//AVCodecContext.time_base是帧率(视频帧)的倒数，每帧时间戳递增1，那么tbc就等于帧率。
//		编码过程中，应由⽤户设置好此参数。
//		解码过程中，此参数已过时，建议直接使⽤帧率倒数⽤作时间基。

//	这⾥有⼀个问题：按照此处注释说明，帧率为25的视频流，tbc理应为25，但实际值却为50，不知作何解释？是否tbc已经过时，不具参考意义

//	根据注释中的建议，
//实际使⽤时，在视频解码过程中，我们不使⽤AVCodecContext.time_base，⽽⽤帧率倒数作时间基
//			在视频编码过程中，我们将AVCodecContext.time_base设置为帧率的倒数。
```

### 3.7.1 视频流

​		视频按帧播放，所以解码后的原始视频帧时间基为 1/framerate。 

​		**视频解码过程中的时间基转换处理**（darren注：该段没有参考意义，packet的pts到底什么，要看实际的 情况，从av_read_frame读取的packet，是以AVSteam->time_base，送给解码器之前没有必要转成 AVcodecContext->time_base， 需要注意的是avcodec_receive_frame后以AVSteam->time_base为 单位即可。）：

```c
//视频解码时间基转换处理：
	AVFormatContext *ifmt_ctx;
	AVStream *in_stream;
	AVCodecContext *dec_ctx;
	AVPacket packet;
	AVFrame *frame;
	// 从输⼊⽂件中读取编码帧
	av_read_frame(ifmt_ctx, &packet);
	// 时间基转换
	int raw_video_time_base = av_inv_q(dec_ctx->framerate);
	av_packet_rescale_ts(packet, in_stream->time_base, raw_video_time_base);
 	// 解码
	avcodec_send_packet(dec_ctx, packet)
	avcodec_receive_frame(dec_ctx, frame);
```

​		**视频编码过程中的时间基转换处理**（darren：编码的时候frame如果以AVstream为time_base送编码器， 则avcodec_receive_packet读取的时候也是以转成AVSteam->time_base，本质来讲就是具体情况具体 分析，没必要硬套流程）：

```c
//视频编码时间基转换处理：	
	AVFormatContext *ofmt_ctx;
	AVStream *out_stream;
	AVCodecContext *dec_ctx;
	AVCodecContext *enc_ctx;
	AVPacket packet;
	AVFrame *frame;
	// 编码
	avcodec_send_frame(enc_ctx, frame);
	avcodec_receive_packet(enc_ctx, packet);
	// 时间基转换
	packet.stream_index = out_stream_idx;
	enc_ctx->time_base = av_inv_q(dec_ctx->framerate);
	av_packet_rescale_ts(&opacket, enc_ctx->time_base, out_stream->time_base);
	// 将编码帧写⼊输出媒体⽂件
	av_interleaved_write_frame(o_fmt_ctx, &packet);
```

### 3.7.2 ⾳频流

​		darren：**对于⾳频流也是类似的**，本质来讲就是具体情况具体分析，没必要硬套流程，

​				⽐如**ffplay 解码播放**时就是AVSteam的time_base为基准的packet进⼊到编码器，然后出来的frame再⽤AVSteam的 time_base讲对应的pts转成秒，

​				但是要注意的是ffplay做了⼀个⽐较隐秘的设置：avctx- >pkt_timebase = ic->streams[stream_index]->time_base; 

​				即是对应的codeccontext⾃⼰对 pkt_timebase设置和AVStream⼀样的time_base。

​	

​	⾳频按采样点播放，所以解码后的原始⾳频帧时间基为 1/sample_rate：

​	⾳频解码过程中的时间基转换处理：

```c
//⾳频解码过程中的时间基转换处理：
	AVFormatContext *ifmt_ctx;
	AVStream *in_stream;
	AVCodecContext *dec_ctx;
	AVPacket packet;
	AVFrame *frame;
	// 从输⼊⽂件中读取编码帧
	av_read_frame(ifmt_ctx, &packet);
	// 时间基转换
	int raw_audio_time_base = av_inv_q(dec_ctx->sample_rate);
	av_packet_rescale_ts(packet, in_stream->time_base, raw_audio_time_base);
	// 解码
	avcodec_send_packet(dec_ctx, packet)
	avcodec_receive_frame(dec_ctx, frame);
```

⾳频编码过程中的时间基转换处理：

```c
//⾳频编码过程中的时间基转换处理：
	AVFormatContext *ofmt_ctx;
	AVStream *out_stream;
	AVCodecContext *dec_ctx;
	AVCodecContext *enc_ctx;
	AVPacket packet;
	AVFrame *frame;
	// 编码
	avcodec_send_frame(enc_ctx, frame);
	avcodec_receive_packet(enc_ctx, packet);
	// 时间基转换
	packet.stream_index = out_stream_idx;
	enc_ctx->time_base = av_inv_q(dec_ctx->sample_rate);
	av_packet_rescale_ts(&opacket, enc_ctx->time_base, out_stream->time_base);
	// 将编码帧写⼊输出媒体⽂件
	av_interleaved_write_frame(o_fmt_ctx, &packet);
```

