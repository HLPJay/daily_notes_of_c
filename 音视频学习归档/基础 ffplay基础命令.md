查看网站：[ffplay Documentation (ffmpeg.org)](http://www.ffmpeg.org/ffplay.html)

## 1：ffplay基础命令：

```
ffplay -version
ffplay -h
```

## 2：ffplay播放控制：

![image-20210731174502053](C:\Users\yun68\Desktop\md文档整理_待完成\daily_notes\md文档相关图片\ffplay播放控制.png)

```bash
'q, ESC'            退出
'f'                 全屏
'p, SPC'            暂停
'w'                 切换显示模式(视频/音频波形/音频频带)
's'                 步进到下一帧
'left/right'        快退/快进 10 秒
'down/up'           快退/快进 1 分钟
'page down/page up' 跳转到前一章/下一章(如果没有章节，快退/快进 10 分钟)
'mouse click'       跳转到鼠标点击的位置(根据鼠标在显示窗口点击的位置计算百分比)
```

## 3：ffplay命令选项：

基本格式： **ffplay [选项] ['输入文件']**

相关主要选项：

### 1：主要操作基础命令：

```bash
-x width 					强制显示宽带
-y height 					强制显示高度
-video_size size 			设置帧尺寸 设置帧尺寸大小。
	适用于类似原始YUV等没有包含帧大小(WxH)的视频。
	例如：ffplay -pixel_format yuv420p -video_size 320x240 -framerate 5 yuv420p_320x240.yuv
-pixel_format format 		设置像素格式。
-volume vol 				设置起始音量。音量范围[0 ~100]
-window_title title 		设置窗口标题（默认为输入文件名）
-loop number 				设置播放循环次数
-showmode mode 				设置显示模式，可用的模式值：0 显示视频，1 显示音频波形，2 显示音频频谱。缺省为0，如果视频不存在则自动选择2
-vf filtergraph 			设置视频滤镜
-af filtergraph 			设置音频滤镜
-f fmt 						强制使用设置的格式进行解析。比如-f s16le

-fs 					以全屏模式启动
-an						禁用音频（不播放声音）
-vn 					禁用视频（不播放视频）
-sn 					禁用字幕（不显示字幕）
-nodisp 				关闭图形化显示窗口，视频将不显示
-noborder 				无边框窗口


-t duration 		设置播放视频/音频长度，时间单位如 -ss选项
-ss pos 			跳转到指定的位置，注意时间单位：
		比如：'55' 55 seconds, 
			'12:03:45' ,12 hours, 03 minutes and 45 seconds, 
			'23.189' 23.189 second


-bytes 							按字节进行跳转（0=off 1=on -1=auto）。
-seek_interval interval 		自定义左/右键定位拖动间隔（以秒为单位），默认值为10秒（代码没有看到实现）
```

### 2：较高级的命令：

```bash
-stats 					打印多个回放统计信息，包括显示流持续时间，编解码器参数，流中的当前位置，以及音频/视频同步差值。
	默认情况下处于启用状态，要显式禁用它则需要指定-nostats。。
-fast 					非标准化规范的多媒体兼容优化。
-genpts 				生成pts。
-sync type 				同步类型 将主时钟设置为audio（type=audio），video（type=video）或external（type=ext），默认是audio为主时钟。

-ast audio_stream_specifier 		指定音频流索引，比如-ast 3，播放流索引为3的音频流
-vst video_stream_specifier			指定视频流索引，比如-vst 4，播放流索引为4的视频流
-sst subtitle_stream_specifier 		指定字幕流索引，比如-sst 5，播放流索引为5的字幕流

-autoexit 				视频播放完毕后退出
-exitonkeydown			键盘按下任何键退出播放
-exitonmousedown 		鼠标按下任何键退出播放


◼ -codec:media_specifier codec_name 	强制使用设置的多媒体解码器，media_specifier可用值为a（音频）， v（视频）和s字幕。
		比如：codec:v h264_qsv 强制视频采用h264_qsv解码
◼ -acodec codec_name 					强制使用设置的音频解码器进行音频解码
◼ -vcodec codec_name 					强制使用设置的视频解码器进行视频解码
◼ -scodec codec_name 					强制使用设置的字幕解码器进行字幕解码

◼ -autorotate					根据文件元数据自动旋转视频。值为0或1 ，默认为1。
◼ -framedrop 					如果视频不同步则丢弃视频帧。当主时钟非视频时钟时默认开启。
		若需禁用则使用 -noframedrop
◼ -infbuf 						不限制输入缓冲区大小。尽可能快地从输入中读取尽可能多的数据。
		播放实时流时默认启用，如果未及时读取数据，则可能会丢弃数据。
		此选项将不限制缓冲区的大小。若需禁用则使用 -noinfbuf
```

### 3：ffplay 播放视频样例：

```bash
◼ 播放本地文件
	ffplay -window_title "test time" -ss 2 -t 10 -autoexit test.mp4
	ffplay buweishui.mp3
◼ 播放网络流
	ffplay -window_title "rtmp stream" rtmp://202.69.69.180:443/webcast/bshdlive-pc
◼ 强制解码器
	mpeg4解码器：ffplay -vcodec mpeg4 test.mp4
	h264解码器：ffplay -vcodec h264 test.mp4 
◼ 禁用音频或视频
	禁用音频：ffplay test.mp4 -an
	禁用视频：ffplay test.mp4 -vn
◼ 播放YUV数据
	ffplay -pixel_format yuv420p -video_size 320x240 -framerate 5 yuv420p_320x240.yuv
◼ 播放RGB数据
	ffplay -pixel_format rgb24 -video_size 320x240 -i rgb24_320x240.rgb
	ffplay -pixel_format rgb24 -video_size 320x240 -framerate 5 -i rgb24_320x240.rgb
◼ 播放PCM数据
	ffplay -ar 48000 -ac 2 -f f32le 48000_2_f32le.pcm
		-ar set audio sampling rate (in Hz) (from 0 to INT_MAX) (default 0)
		-ac set number of audio channels (from 0 to INT_MAX) (default 0)
```

### 4：ffplay简单过滤器：

```bash
◼ 视频旋转
	ffplay -i test.mp4 -vf transpose=1
◼ 视频反转
	ffplay test.mp4 -vf hflip
	ffplay test.mp4 -vf vflip
◼ 视频旋转和反转
	ffplay test.mp4 -vf hflip,transpose=1
◼ 音频变速播放
	ffplay -i test.mp4 -af atempo=2
◼ 视频变速播放
	ffplay -i test.mp4 -vf setpts=PTS/2
◼ 音视频同时变速
	ffplay -i test.mp4 -vf setpts=PTS/2 -af atempo=2
◼ 更多参考
	http://www.ffmpeg.org/ffmpeg-filters.html
```

