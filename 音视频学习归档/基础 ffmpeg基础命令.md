基于windows环境安装好必要的ffmpeg后，对ffmpeg基础命令进行一些了解：

## 1：ffmpeg查看版本：

```
ffmpeg -version
```

## 2：ffmpeg查询命令：

```bash
基本信息：ffmpeg -h
高级信息：ffmpeg -h long
所有信息：ffmpeg -h full

ffplay查看帮助： ffplay -h
ffprobe查看帮助： ffprobe -h

ffmpeg具体分类支持的参数查看帮助： ffmepg -h type=name
比如：		
		ffmpeg -h muxer=flv
		ffmpeg -h filter=atempo (atempo调整音频播放速率)
		ffmpeg -h encoder=libx264
```

## 3：ffmpeg分类查询命令：

![ffmpeg分类查询](..\md文档相关图片\ffmpeg分类查询.png)

```bash
	-version 	显示版本 
	-buildconf 	显示编译配置 
	-protocols 	显示可用的协议 
	-formats 	显示可用格式 (muxers+demuxers) 
	
	-filters 	显示可用的过滤器 
	-muxers 	显示可用复用器 

	-demuxers 	显示可用解复用器 
	-codecs 	显示可用编解码器 (decoders+encoders)
	-decoders 	显示可用解码器
	-encoders 	显示可用编码器
	-layouts 	显示标准声道名称 
	
	-pix_fmts 		显示可用的像素格式 
	-sample_fmts 	显示可用的音频采样 格式 
	-bsfs 			显示可用比特流filter 
	-colors 		显示可用的颜色名称 
```

## 4：ffmpeg基础命令：

​	更多参考： http://www.ffmpeg.org/ffmpeg.html

```bash
 主要参数：
	◼ -i 设定输入流
	◼ -f 设定输出格式(format)
	◼ -ss 开始时间
	◼ -t 时间长度
	
音频参数：
	◼ -aframes 		设置要输出的音频帧数
	◼ -b:a 			音频码率
	◼ -ar 			设定采样率
	◼ -ac			设定声音的Channel数
	◼ -acodec 		设定声音编解码器，如果用copy表示原始编解码数据必须被拷贝。
	◼ -an 			不处理音频
	◼ -af 			音频过滤器
ffmpeg -i test.mp4 -b:a 192k -ar 48000 -ac 2 -acodec libmp3lame -aframes 200 out2.mp3

视频参数：
	◼ -vframes 			设置要输出的视频帧数
	◼ -b 				设定视频码率
	◼ -b:v 				视频码率
	◼ -r 				设定帧速率
	◼ -s 				设定画面的宽与高
	◼ -vn 				不处理视频
	◼ -aspect aspect 	设置横纵比 4:3 16:9 或 1.3333 1.7777
	◼ -vcodec 			设定视频编解码器，如果用copy表示原始编解码数据必须被拷贝。
	◼ -vf 				视频过滤器
ffmpeg -i test.mp4 -vframes 300 -b:v 300k -r 30 -s 640x480 -aspect 16:9 -vcodec libx265
```

## 5：ffmepg使用命令提取音频或视频：

```bash
提取音视频数据：
◼ 保留封装格式
	ffmpeg -i test.mp4 -acodec copy -vn audio.mp4
	ffmpeg -i test.mp4 -vcodec copy -an video.mp4
◼ 提取视频
	保留编码格式：ffmpeg -i test.mp4 -vcodec copy -an test_copy.h264
	强制格式：ffmpeg -i test.mp4 -vcodec libx264 -an test.h264
◼ 提取音频
	保留编码格式：ffmpeg -i test.mp4 -acodec copy -vn test.aac
	强制格式：ffmpeg -i test.mp4 -acodec libmp3lame -vn test.mp3
	
提取像素格式和PCM数据：
◼ 提取YUV
	◼ 提取3秒数据，分辨率和源视频一致
		ffmpeg -i test_1280x720.mp4 -t 3 -pix_fmt yuv420p yuv420p_orig.yuv
	◼ 提取3秒数据，分辨率转为320x240
		ffmpeg -i test_1280x720.mp4 -t 3 -pix_fmt yuv420p -s 320x240 yuv420p_320x240.yuv
◼ 提取RGB
	◼ 提取3秒数据，分辨率转为320x240
		ffmpeg -i test.mp4 -t 3 -pix_fmt rgb24 -s 320x240 rgb24_320x240.rgb
◼ RGB和YUV之间的转换
	ffmpeg -s 320x240 -pix_fmt yuv420p -i yuv420p_320x240.yuv -pix_fmt rgb24 rgb24_320x240_2.rgb
	
◼ 提取PCM
	ffmpeg -i buweishui.mp3 -ar 48000 -ac 2 -f s16le 48000_2_s16le.pcm
	ffmpeg -i buweishui.mp3 -ar 48000 -ac 2 -sample_fmt s16 out_s16.wav
	ffmpeg -i buweishui.mp3 -ar 48000 -ac 2 -codec:a pcm_s16le out2_s16le.wav
	ffmpeg -i buweishui.mp3 -ar 48000 -ac 2 -f f32le 48000_2_f32le.pcm
	ffmpeg -i test.mp4 -t 10 -vn -ar 48000 -ac 2 -f f32le 48000_2_f32le_2.pcm
```

## 6：ffmpeg命令转封装：

```bash
◼ 保持编码格式：
	ffmpeg -i test.mp4 -vcodec copy -acodec copy test_copy.ts
	ffmpeg -i test.mp4 -codec copy test_copy2.ts
◼ 改变编码格式：
	ffmpeg -i test.mp4 -vcodec libx265 -acodec libmp3lame out_h265_mp3.mkv
	
◼ 修改帧率：
	ffmpeg -i test.mp4 -r 15 -codec copy output.mp4 (错误命令)
	ffmpeg -i test.mp4 -r 15 output2.mp4
◼ 修改视频码率：
	ffmpeg -i test.mp4 -b 400k output_b.mkv （此时音频也被重新编码）
◼ 修改视频码率：
	ffmpeg -i test.mp4 -b:v 400k output_bv.mkv
	
◼ 修改音频码率：
	ffmpeg -i test.mp4 -b:a 192k output_ba.mp4
	如果不想重新编码video，需要加上-vcodec copy
◼ 修改音视频码率：
	ffmpeg -i test.mp4 -b:v 400k -b:a 192k output_bva.mp4
	
◼ 修改视频分辨率：
	ffmpeg -i test.mp4 -s 480x270 output_480x270.mp4
◼ 修改音频采样率: 
	ffmpeg -i test.mp4 -ar 44100 output_44100hz.mp4
```

## 7：ffmpeg过滤器命令：

```bash
1：生成测试文件

◼ 找三个不同的视频每个视频截取10秒内容
	ffmpeg -i 沙海02.mp4 -ss 00:05:00 -t 10 -codec copy 1.mp4
	ffmpeg -i 复仇者联盟3.mp4 -ss 00:05:00 -t 10 -codec copy 2.mp4
	ffmpeg -i 红海行动.mp4 -ss 00:05:00 -t 10 -codec copy 3.mp4
		如果音视频格式不统一则强制统一为 -vcodec libx264 -acodec aac
◼ 将上述1.mp4/2.mp4/3.mp4转成ts格式
	ffmpeg -i 1.mp4 -codec copy -vbsf h264_mp4toannexb 1.ts
	ffmpeg -i 2.mp4 -codec copy -vbsf h264_mp4toannexb 2.ts
	ffmpeg -i 3.mp4 -codec copy -vbsf h264_mp4toannexb 3.ts
◼ 转成flv格式
	ffmpeg -i 1.mp4 -codec copy 1.flv
	ffmpeg -i 2.mp4 -codec copy 2.flv
	ffmpeg -i 3.mp4 -codec copy 3.flv
分离某些封装格式（例如MP4/FLV/MKV等）中的H.264的时候，需要首先写入SPS和PPS，否则会导致分离出来的数据没有SPS、PPS而无法播放。
	H.264码流的SPS和PPS信息存储在AVCodecContext结构体的extradata中。
	需要使用ffmpeg中名称为“h264_mp4toannexb”的bitstream filter处理
	
2：开始拼接文件：
◼ 以MP4格式进行拼接
	方法1：ffmpeg -i "concat:1.mp4|2.mp4|3.mp4" -codec copy out_mp4.mp4 
	方法2：ffmpeg -f concat -i mp4list.txt -codec copy out_mp42.mp4
	自己新建 mp4list.txt,在其中实现 file '1.mp4'
	
	
◼ 以TS格式进行拼接
	方法1：ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out_ts.mp4 
	方法2：ffmpeg -f concat -i tslist.txt -codec copy out_ts2.mp4
	自己新建 tslist.txt,在其中实现 file '1.ts'
	
◼ 以FLV格式进行拼接
	方法1：ffmpeg -i "concat:1.flv|2.flv|3.flv" -codec copy out_flv.mp4 
	方法2：ffmpeg -f concat -i flvlist.txt -codec copy out_flv2.mp4

需要验证最后结果是否正常：
◼ 方法1只适用部分封装格式，比如TS
◼ 建议：
	（1）使用方法2进行拼接
	（2）转成TS格式再进行拼接

3：测试不同编码拼接：
◼ 修改音频编码
	ffmpeg -i 2.mp4 -vcodec copy -acodec ac3 -vbsf h264_mp4toannexb 2.ts
	ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out1.mp4 结果第二段没有声音
◼ 修改音频采样率
	ffmpeg -i 2.mp4 -vcodec copy -acodec aac -ar 96000 -vbsf h264_mp4toannexb 2.ts
	ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out2.mp4 第二段播放异常
◼ 修改视频编码格式
	ffmpeg -i 1.mp4 -acodec copy -vcodec libx265 1.ts
	ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out3.mp4 
◼ 修改视频分辨率
	ffmpeg -i 1.mp4 -acodec copy -vcodec libx264 -s 800x472 -vbsf h264_mp4toannexb 1.ts
	ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out4.mp4
◼ 注意：
	把每个视频封装格式也统一为ts，拼接输出的时候再输出你需要的封装格式，比如MP4
	视频分辨率可以不同，但是编码格式需要统一
	音频编码格式需要统一，音频参数(采样率/声道等)也需要统一
```

## 8：ffmpeg 图片与视频互转：

```bash
◼ 截取一张图片
	ffmpeg -i test.mp4 -y -f image2 -ss 00:00:02 -vframes 1 -s 640x360 test.jpg
	ffmpeg -i test.mp4 -y -f image2 -ss 00:00:02 -vframes 1 -s 640x360 test.bmp
		-i 输入
		-y 覆盖
		-f 格式
		image2 一种格式
		-ss 起始值
		-vframes 帧 如果大于1 那么 输出加%03d test%03d.jpg 
		-s 格式大小size
◼ 转换视频为图片（每帧一张图):
	ffmpeg -i test.mp4 -t 5 -s 640x360 -r 15 frame%03d.jpg 
◼ 图片转换为视频:
	ffmpeg -f image2 -i frame%03d.jpg -r 25 video.mp4


◼ 从视频中生成GIF图片
	ffmpeg -i test.mp4 -t 5 -r 1 image1.gif
	ffmpeg -i test.mp4 -t 5 -r 25 -s 640x360 image2.gif
◼ 将 GIF 转化为 视频
	ffmpeg -f gif -i image2.gif image2.mp4
```

## 9：ffmpeg 视频录制命令：

有时候写入MP4文件会不成功，改成flv就好

```bash
◼ 先安装dshow软件 Screen Capturer Recorder，
	项目地址：https://sourceforge.net/projects/screencapturer/files/ 
	然后查看可用设备名字：
		ffmpeg -list_devices true -f dshow -i dummy

◼ 录制视频（默认参数）
	桌面：		 ffmpeg -f dshow -i video="screen-capture-recorder" v-out.mp4 
	摄像头： 	 ffmpeg -f dshow -i video="Integrated Webcam" -y v-out2.flv (要根据自己摄像头名称)
◼ 录制声音（默认参数）
	系统声音：			ffmpeg -f dshow -i audio="virtual-audio-capturer" a-out.aac
	系统+麦克风声音：	  ffmpeg -f dshow -i audio="麦克风 (Realtek(R) Audio)" -f dshow -i audio="virtual-audio-capturer" -filter_complex amix=inputs=2:duration=first:dropout_transition=2 a-out2.aac

◼ 查看视频录制的可选参数
	ffmpeg -f dshow -list_options true -i video="screen-capture-recorder"	
◼ 查看音频录制的可选参数
	ffmpeg -f dshow -list_options true -i audio="virtual-audio-capturer"
	ffmpeg -f dshow -list_options true -i audio="麦克风 (Realtek(R) Audio)"

指定参数录制音视频：
	◼ ffmpeg -f dshow -i audio="麦克风 (Realtek(R) Audio)" -f dshow -i audio="virtual-audio-capturer" -filter_complex amix=inputs=2:duration=first:dropout_transition=2 -f dshow -video_size 1920x1080 -framerate 15 -pixel_format yuv420p -i video="screen-capture-recorder" -vcodec h264_qsv -b:v 3M -y av-out.flv
	◼ ffmpeg -f dshow -i audio="麦克风 (Realtek(R) Audio)" -f dshow -i audio="virtual-audio-capturer" -filter_complex amix=inputs=2:duration=first:dropout_transition=2 -f dshow -i video="screen-capture-recorder" -vcodec h264_qsv -b:v 3M -r 15 -y av-out2.mp4
	◼ ffmpeg -f dshow -i audio="麦克风 (Realtek(R) Audio)" -f dshow -i audio="virtual-audio-capturer" -filter_complex amix=inputs=2:duration=first:dropout_transition=2 -f dshow -framerate 15 -pixel_format yuv420p -i video="screen-capture-recorder" -vcodec h264_qsv -b:v 3M -r 15 -y av-out3.mp4
```

## 10：ffmpeg直播推流/拉流命令：

```bash
◼ 直播拉流
	ffplay rtmp://server/live/streamName 
	ffmpeg -i rtmp://server/live/streamName -c copy dump.flv
		对于不是rtmp的协议 -c copy要谨慎使用
◼ 可用地址
	HKS：rtmp://live.hkstv.hk.lxdns.com/live/hks2
	大熊兔(点播)：rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov
	CCTV1高清：http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8
		ffmpeg -i http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8 -c copy cctv1.ts
		ffmpeg -i http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8 cctv1.flv
		ffmpeg -i http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8 -acodec aac -vcodec libx264 cctv1-2.flv
	CCTV3高清：http://ivi.bupt.edu.cn/hls/cctv3hd.m3u8
	CCTV5高清：http://ivi.bupt.edu.cn/hls/cctv5hd.m3u8
	CCTV5+高清：http://ivi.bupt.edu.cn/hls/cctv5phd.m3u8
	CCTV6高清：http://ivi.bupt.edu.cn/hls/cctv6hd.m3u8

◼ 直播推流
	ffmpeg -re -i out.mp4 -c copy flv rtmp://server/live/streamName
		-re,表示按时间戳读取文件
```

## 11：ffmpeg filter过滤器应用：

### 	1：视频裁剪：

![image-20210801011734872](..\md文档相关图片\ffmpeg视频裁剪原理.png)

​	

```bash
变量 			   用于 ow 和 oh 参数的表达式中的可用变量
x, y			对 x 的计算值(从左上角水平方向的像素个数)和 y(垂直像素的数量)，对每个帧进行评估，x的默认值为(iw - ow)/2, y 的默认值为(ih - oh)/2
in_w, iw 		输入的宽度
in_h, ih 		输入的高度
out_w,ow		输出(裁剪)宽度，默认值= iw
out_h,oh 		输出(裁剪)高度，默认值= ih
a 				纵横比，与 iw/ih 相同
sar 			输入样本比例
dar 			输入显示宽比，等于表达式 a*sar
hsub, vsub 		水平和垂直的色度子样本值，对于像素格式 yuv422p, hsub 的值为 2,vsub 为 1
n 				输入帧的数目，从 0 开始
pos 			位置在输入框的文件中，如果不知道 NAN
t 				时间戳以秒表示，如果输入时间戳未知

ow 的值可以从 oh 得到，反之亦然，但不能从 x 和 y 中得到，因为这些值是在 ow 和 oh 之后进行的。
x 的值可以从 y 的值中得到，反之亦然。
	例如，在输入框的左三、中三和右三，我们可以使用命令(iw/3*2)是开始裁剪的位置:
		ffmpeg -i input -vf crop=iw/3:ih:0:0 output 
		ffmpeg -i input -vf crop=iw/3:ih:iw/3:0 output 
		ffmpeg -i input -vf crop=iw/3:ih:iw/3*2:0 output

（1）裁剪 100x100 的区域，起点为(12,34).
		crop=100:100:12:34
		相同效果:
		crop=w=100:h=100:x=12:y=34
（2）裁剪中心区域，大小为 100x100
		crop=100:100
（3）裁剪中心区域，大小为输入视频的 2/3
		crop=2/3*in_w:2/3*in_h
（4）裁剪中心区域的正方形，高度为输入视频的高
		crop=out_w=in_h
		crop=in_h
（5）裁剪偏移左上角 100 像素
		crop=in_w-100:in_h-100:100:100
（6）裁剪掉左右 10 像素，上下 20 像素
		crop=in_w-2*10:in_h-2*20
（7）裁剪右下角区域
		crop=in_w/2:in_h/2:in_w/2:in_h/2
```

### 	2：文字水印:	

编译的时候需要支持 FreeType、FontConfig、iconv，系统中需要有相关的子库，

在 FFmpeg 中增加纯字母水印可以使用 drawtext 滤镜进行支持:

```bash
drawtext 参数相关：
text 		字符串 	   文字
textfile 	字符串 	   文字文件
box 		布尔 			文字区域背景框(缺省 false)
boxcolor 	色彩 			展示字体区域块的颜色
font 		字符串 	   字体名称（默认为 Sans 字体）
fontsize 	整数 			显示字体的大小
x 			字符串 		缺省为 0
y 			字符串 		缺省为 0
alpha 		浮点数 		透明度(默认为 1)，值从 0~1

```

相关使用实例：

```bash
（1）将文字的水印加在视频的左上角：
	ffplay -i input.mp4 -vf "drawtext=fontsize=100:fontfile=FreeSerif.ttf:text='hello world':x=20:y=20"
将字体的颜色设置为绿色：
	ffplay -i input.mp4 -vf "drawtext=fontsize=100:fontfile=FreeSerif.ttf:text='hello world':fontcolor=green"
如果想调整文字水印显示的位置，调整 x 与 y 参数的数值即可。
	ffplay -i input.mp4 -vf "drawtext=fontsize=100:fontfile=FreeSerif.ttf:text='hello world':fontcolor=green:x=400:y=200"
修改透明度
	ffplay -i input.mp4 -vf "drawtext=fontsize=100:fontfile=FreeSerif.ttf:text='helloworld':fontcolor=green:x=400:y=200:alpha=0.5"
	
（2）文字水印还可以增加一个框，然后给框加上背景颜色：
	ffplay -i input.mp4 -vf "drawtext=fontsize=100:fontfile=FreeSerif.ttf:text='hello world':fontcolor=green:box=1:boxcolor=yellow"
	
（3）有些时候文字水印希望以本地时间作为水印内容，可以在 drawtext 滤镜中配合一些特殊用法来完成，在 text 中显示本地当前时间，格式为年月日时分秒的方式，
	ffplay -i input.mp4 -vf "drawtext=fontsize=60:fontfile=FreeSerif.ttf:text='%{localtime\:%Y\-%m\-%d %H-%M-%S}':fontcolor=gree
n:box=1:boxcolor=yellow"
在使用 ffmpeg 转码存储到文件时需要加上-re，否则时间不对。
	ffmpeg -re -i input.mp4 -vf "drawtext=fontsize=60:fontfile=FreeSerif.ttf:text='%{localtime\:%Y\-%m\-%d %H-%M-%S}':fontcolor=gree n:box=1:boxcolor=yellow" out.mp4
（4）在个别场景中，需要定时显示水印，定时不显示水印，这种方式同样可以配合 drawtext 滤镜进行处理，使用 drawtext 与 enable 配合即可，
	例如每 3 秒钟显示一次文字水印：
	ffplay -i input.mp4 -vf "drawtext=fontsize=60:fontfile=FreeSerif.ttf:text='test':fontcolor=green:box=1:boxcolor=yellow:enable=lt(mod(t\,3)\,1)" 
	在使用 ffmpeg 转码存储到文件时需要加上-re，否则时间不对。
	表达式参考：http://www.ffmpeg.org/ffmpeg-utils.html 3 Expression Evaluation 
		lt(x, y) Return 1 if x is lesser than y, 0 otherwise.
		mod(x, y) Compute the remainder of division of x by y.
		
（5）跑马灯效果
	ffplay -i input.mp4 -vf "drawtext=fontsize=100:fontfile=FreeSerif.ttf:text='helloworld':x=mod(100*t\,w):y=abs(sin(t))*h*0.7"
修改字体透明度，修改字体颜色
	ffplay -i input.mp4 -vf  "drawtext=fontsize=40:fontfile=FreeSerif.ttf:text='liaoqingfu':x=mod(50*t\,w):y=abs(sin(t))*h*0.7:alpha=0.5:fontcolor=white:enable=lt(mod(t\,3)\,1)"

```

### 	3：图片水印:

​	为视频添加图片水印可以使用 **movie** 滤镜，相关参数：

```bash
filename		 	字符串		输入的文件名，可以是文件，协议，设备
format_name, f 		字符串 	输入的封装格式
stream_index, si 	整数 		输入的流索引编号
seek_point, sp 		浮点数 	Seek 输入流的时间位置
streams, s 			字符串 	输入的多个流的流信息
loop 				整数 		循环次数
discontinuity 		时间差值   支持跳动的时间戳差值

例如：
ffmpeg -i input.mp4 -vf "movie=logo.png[watermark];[in][watermark]overlay=x=10:y=10[out]" output.mp4
	➢ 原始视频文件路径：input.mp4
	➢ 水印图片路径：logo.png
	➢ 水印位置：(x,y)=(10,10)<=(left,top)距离左侧、顶部各 10 像素；
	➢ 输出文件路径：output.mp4
		main_w 视频单帧图像宽度
		main_h 视频单帧图像高度
		overlay_w 水印图片的宽度
		overlay_h 水印图片的高度
	对应地可以将 overlay 参数设置成如下值来改变水印图片的位置：
		左上角 10:10
		右上角 main_w-overlay_w-10:10
		左下角 10:main_h-overlay_h-10
		右下角 main_w-overlay_w-10:main_h-overlay_h-10
```

使用实例：

```bash
在 FFmpeg 中加入图片水印有两种方式：
	一种是通过 movie 指定水印文件路径，
	另外一种方式是通过filter 读取输入文件的流并指定为水印。
读取 movie 图片文件作为水印：

（1）图片 logo.png 将会打入到 input.mp4 视频中，显示在 x 坐标 50、y 坐标 20 的位置
		ffplay -i input.mp4 -vf "movie=logo.png[logo];[in][logo]overlay=50:10[out]"
由于 logo.png 图片的背景色是白色，所以显示起来比较生硬，如果水印图片是透明背景的，效果会更好，
下面找一张透明背景色的图片试一下：
	ffplay -i input.mp4 -vf "movie=logo2.png[watermark];[in][watermark]overlay=50:10[out]"
（2）显示位置
		ffplay -i input.mp4 -vf "movie=logo.png[watermark];[in][watermark]overlay=10:10[out]"
		ffplay -i input.mp4 -vf "movie=logo.png[watermark];[in][watermark]overlay=main_w-overlay_w-10:10[out]"
		ffplay -i input.mp4 -vf "movie=logo.png[watermark];[in][watermark]overlay=10:main_h-overlay_h-10[out]"
		ffplay -i input.mp4 -vf "movie=logo.png[watermark];[in][watermark]overlay=main_w-overlay_w-10:main_h-overlay_h-10[out]"
（3）跑马灯效果
		ffplay -i input.mp4 -vf "movie=logo.png[watermark];[in][watermark]overlay=x=mod(50*t\,main_w):y=abs(sin(t))*h*0.7[out]"
```

### 4：生成画中画：

可以通过 **overlay** 将 多个视频流、多个多媒体采集设备、多个视频文件合并到一个界面中，生成画中画的效果。

相关参数介绍：

```bash
x 			字符串 		X 坐标
y 			字符串 		Y 坐标
eof_action 	整数 			遇到 eof 表示时的处理方式，默认为重复
							➢ repeat(值为 0)：重复前一帧
							➢ endcall(值为 1)：停止所有的流
							➢ pass(值为 2)：保留主图层
shortest 	布尔 			终止最短的视频时全部终止（默认 false）
format 		整数 			设置 output 的像素格式，默认为 yuv420
							➢ yuv420 (值为 0)
							➢ yuv422 (值为 1)
							➢ yuv444 (值为 2)
							➢ rgb (值为 3)
（1）显示画中画效果
		ffplay -i input.mp4 -vf "movie=sub_320x240.mp4[sub];[in][sub]overlay=x=20:y=20[out]"
		ffplay -i input.mp4 -vf "movie=sub_320x240.mp4[sub];[in][sub]overlay=x=20:y=20:eof_action=1[out]"
		ffplay -i input.mp4 -vf "movie=sub_320x240.mp4[sub];[in][sub]overlay=x=20:y=20:shortest =1[out]"
缩放子画面尺寸
	ffplay -i input.mp4 -vf "movie=sub_320x240.mp4,scale=640x480[sub];[in][sub]overlay=x=20:y=20[out]"
（2）跑马灯
	ffplay -i input.mp4 -vf "movie=sub_320x240.mp4[test];[in][test]overlay=x=mod(50*t\,main_w):y=abs(sin(t))*main_h*0.7[out]"

```

### 	5：多宫格处理：

从前文中可以看出进行视频图像处理时，overlay 滤镜为关键画布，可以通过 FFmpeg建立一个画布，也可以使用默认的画布。如果想以多宫格的方式展现，则可以自己建立一个足够大的画布， 下面就来看一下多宫格展示的例子：

```bash
ffmpeg -i 1.mp4 -i 2.mp4 -i 3.mp4 -i 4.mp4 -filter_complex "nullsrc=size=640x480[base];[0:v] setpts=PTS-STARTPTS,scale=320x240[upperleft];[1:v]setpts=PTS-STARTPTS,scale=320x240[upperright];[2:v]setpts=PTS-STARTPTS, scale=320x240[lowerleft];[3:v]setpts=PTS-STARTPTS,scale=320x240[lowerright];[base][upperleft]overlay=shortest=1[tmp1];[tmp1][upperright]overlay=shortest=1:x=320[tmp2];[tmp2][lowerleft]overlay=shortest=1:y=240[tmp3];[tmp3][lowerright]overlay=shortest=1:x=320:y=240" out.mp4

1.2.3.4.mp4 为文件路径，out.MP4 为输出文件路径，通过 nullsrc 创建 overlay 画布，画布大小 640:480,使用[0:v][1:v][2:v][3:v]将输入的 4 个视频流去除，分别进行缩放处理，然后基于 nullsrc 生成的画布进行视频平铺，命令中自定义 upperleft,upperright,lowerleft,lowerright 进行不同位置平铺。

只叠加左上右上的命令（输出到tmp1中，可以最后加[out]）：
ffmpeg -i 1.mp4 -i 2.mp4 -i 3.mp4 -i 4.mp4 -filter_complex "nullsrc=size=640x480[base];[0:v]setpts=PTS-STARTPTS,scale=320x240[upperleft];[1:v]setpts=PTS-STARTPTS,scale=320x240[upperright];[base][upperleft]overlay=shortest=1[tmp1];[tmp1][upperright]overlay=shortest=1:x=320" out2.mp4
```

### 	6：视频倒放相关控制：

```bash
通过ffmpeg命令行进行音视频倒放，android平台同样可以以将ffmpeg集成进去实现音视频的相关编辑

1.视频倒放，无音频
	ffmpeg.exe -i inputfile.mp4 -filter_complex [0:v]reverse[v] -map [v] -preset superfast reversed.mp4
 
2.视频倒放，音频不变
	ffmpeg.exe -i inputfile.mp4 -vf reverse reversed.mp4
 
3.音频倒放，视频不变
	ffmpeg.exe -i inputfile.mp4 -map 0 -c:v copy -af "areverse" reversed_audio.mp4
 
4.音视频同时倒放
	ffmpeg.exe -i test.mp4 -vf reverse -af areverse -preset superfast outtest.mp4
 
6、查看音视频实际时长
	ffprobe.exe  -v error -select_streams v:0 -show_entries stream=duration -of default=noprint_wrappers=1:nokey=1 input.mp4
	ffprobe.exe  -v error -select_streams a:0 -show_entries stream=duration -of default=noprint_wrappers=1:nokey=1 input.mp4
 
7、查看NV21
	ffplay.exe -i input.nv21 -pix_fmt nv21 -s 4624x3472
 
8、nv21转jpg
	ffmpeg -y -s 1920x1080 -pix_fmt nv21 -i image.nv21.yuv image.jpg
 
9.jpg转nv21
	ffmpeg -i input.jpg -pix_fmt nv21 output_1080x1920.yuv
 
10、rgba转png
	ffmpeg -f rawvideo -pixel_format rgba -video_size 1080x1920 -i input.raw output.png
 
11、jpg转rgba
	ffmpeg -i input.jpg -vcodec rawvideo -pix_fmt rgba raw1.rgb
 
12、剔除mp4中音频或视频（-map 0:0 -map 0:1）
	ffmpeg.exe -i input.mp4 -map 0:0 -vcodec copy -acodec copy output.mp4
		-map 0:0: 第1个输入文件的第一个流，也就是主要的视频流。
		-map 0:1: 第1个输入文件的第二个流，是视频的声音。
		-vcodec copy: 拷贝选择的视频流。
		-acodec copy: 拷贝选择的声音流
```



## 12：分离H264或mpeg2video视频格式数据命令：

```bash
#提取H264:
	ffmpeg -i source.200kbps.768x320_10s.flv -vcodec libx264 -an -f h264 source.200kbps.768x320_10s.h264
#提取MPEG2:
	ffmpeg -i source.200kbps.768x320_10s.flv -vcodec mpeg2video -an -f mpeg2video source.200kbps.768x320_10s.mpeg2v
	
#播放YUV
	ffplay -pixel_format yuv420p -video_size 768x320 -framerate 25 source.200kbps.768x320_10s.yuv
```

## 13：ffmpeg 命令查找重定向(-f fmt 对应的参数)

```bash
	⽐如我们在-f fmt打算指定格式时，怎么知道什么样的格式才是适合的format？
		可以通过ffmpeg -formats | findstr xx的⽅式去查找。
		对于findstr，/i是忽略⼤⼩写
⽐如：
	查找Audio的裸流解复⽤器：ffmpeg -formats | findstr /i audio
	查找Video的裸流解复⽤器：ffmpeg -formats | findstr /i video
```

## 14：ffmpeg其他基础命令：

```bash
#查看视频元信息  比如 编码格式和比特率
$ ffmpeg -i input.mp4
#只查看元信息
$ ffmpeg -i input.mp4 -hide_banner 

#将音频和视频合并到一个文件：
$ ffmpeg -i input.aac -i input.mp4 output.mp4

#截取一张图片：
$ffmpeg -ss 01:23:45 -i input -vframes 1 -q:v 2 output.jpg

#为音频添加封面：
$ ffmpeg -loop 1 -i cover.jpg -i input.mp3 -c:v libx264 -c:a aac -b:a 192k -shortest output.mp4
#	有两个输入文件，一个是封面图片cover.jpg，另一个是音频文件input.mp3。
#		-loop 1参数表示图片无限循环，-shortest参数表示音频文件结束，输出视频就结束。
```

## 15：花屏 绿屏

绿屏的主要是: 无法渲染的画面有些用黑色填充，有些用绿色填充，有些用上一帧画面填充。

​		视频参数改变， 而解码端的SPS&PPS信息未及时重新获取更新，会导致画面无法正常渲染，继而导致绿屏的现象出现。

全屏花屏：

​	正常花屏：

​			码率特别低的时候出现的大面积马赛克，编码器每秒产生的视频数据太少。

​			视频参数问题：

​					视频源修改过视频参数(如从720P修改1080P),此时客户端用于解码的SPS&PPS如果没有重新获取的话，就会出现整个画面花屏的现象。

​					不会恢复。

​	局部花屏：

​				SO_SNDBUF的Buffer太小，丢失P帧

​				P帧丢失。

**总结：视频播放时，相关SPS和PPS参数不匹配/丢帧**

