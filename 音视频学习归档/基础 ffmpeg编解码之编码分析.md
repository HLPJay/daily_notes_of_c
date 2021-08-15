理解：h264白皮书，相关协议



## 1：ffmpeg 合成FLV格式流程：

### 1.1 流程图简介：

![image-20210801183754058](..\md文档相关图片\ffmpeg 合成flv流程.png)

### 1.2 相关函数分析：

```c
ffmpeg 的 Mux 主要分为 三步操作：
	avformat_write_header ： 					写⽂件头
	av_write_frame/av_interleaved_write_frame： 	写packet
	av_write_trailer ： 							写⽂件尾
    
    avcodec_parameters_from_context：
    		将AVCodecContext结构体中码流参数拷⻉到AVCodecParameters结构体中，和avcodec_parameters_to_context刚好相反。
```

### 1.3 FFmpeg函数：avformat_alloc_output_context2

```c
int avformat_alloc_output_context2(AVFormatContext **ctx,  AVOutputFormat *oformat, const char *format_name, const char *filename);
函数参数的介绍：
	ctx:			需要创建的context，返回NULL表示失败。
	oformat:		指定对应的AVOutputFormat，如果不指定，可以通过后⾯format_name、filename两个参数进⾏指定，让ffmpeg⾃⼰推断。
	format_name: 	指定⾳视频的格式，⽐如“flv”，“mpeg”等，如果设置为NULL，则由filename进⾏指定，让ffmpeg⾃⼰推断。
	filename: 		指定⾳视频⽂件的路径，
		如果oformat、format_name为NULL，则ffmpeg内部根据filename后缀名选择合适的复⽤器，⽐如xxx.flv则使⽤flv复⽤器。
        
        
  avformat_alloc_output_context2函数中：
        ⾥⾯最主要的就两个函数，avformat_alloc_context和av_guess_format，
        	⼀个是申请内存分配上下⽂，
        	⼀个是通过后⾯两个参数获取AVOutputFormat。
		av_guess_format这个函数会通过filename和short_name来和所有的编码器进⾏⽐对，找出最接近的编码器然后返回。
```

### 1.4 FFmpeg结构体：AVOutputFormat

​		AVOutpufFormat		表示输出⽂件容器格式，

```c
AVOutputFormat 		//结构主要包含的信息有：封装名称描述，编码格式信息(video/audio 默认编码格式，⽀持的编码格式列表)，⼀些对封装的操作函数 (write_header,write_packet,write_tailer等)。
 					//ffmpeg⽀持各种各样的输出⽂件格式，MP4，FLV，3GP等等。⽽ AVOutputFormat 结构体则保存了这些格式的信息和⼀些常规设置。
```

每⼀种封装对应⼀个 AVOutputFormat 结构，ffmpeg将AVOutputFormat 按照链表存储：

![image-20210801185033368](..\md文档相关图片\ffmpeg视频输出文件格式链表存储.png)

结构体定义分析：

```c
typedef struct AVOutputFormat {
	const char *name;    			// 复⽤器名称
	const char *long_name;			//格式的描述性名称，易于阅读。
	const char *mime_type;
	const char *extensions; 		//逗号分隔的文件扩展名 秒 
    
	enum AVCodecID audio_codec; 			//默认的⾳频编解码器
	enum AVCodecID video_codec; 			//默认的视频编解码器
	enum AVCodecID subtitle_codec;			//默认的字幕编解码器
		//⼤部分复⽤器都有默认的编码器，所以⼤家如果要调整编码器类型则需要⾃⼰⼿动指定
		/*比如：
    ⽐如AVOutputFormat ff_flv_muxer
		AVOutputFormat ff_flv_muxer = {
			.name = "flv",
			.audio_codec = CONFIG_LIBMP3LAME ? AV_CODEC_ID_MP3 :AV_CODEC_ID_ADPCM_SWF, // 默认了MP3
			.video_codec = AV_CODEC_ID_FLV1,
			....
		};
		AVOutputFormat ff_mpegts_muxer = {
			.name = "mpegts",
			.extensions = "ts,m2t,m2ts,mts",
			.audio_codec = AV_CODEC_ID_MP2,
			.video_codec = AV_CODEC_ID_MPEG2VIDEO,
			....
		};*/
    int flags;
	const struct AVCodecTag * const *codec_tag;
	const AVClass *priv_class;
    
	#if FF_API_AVIOFORMAT
	#define ff_const59
	#else
	#define ff_const59 const
	#endif
    
	ff_const59 struct AVOutputFormat *next;

	int priv_data_size;

	int (*write_header)(struct AVFormatContext *);
	int (*write_packet)(struct AVFormatContext *, AVPacket *pkt);
    	//写⼀个数据包。 如果在标志中设置AVFMT_ALLOW_FLUSH，则pkt可以为NULL。
	int (*write_trailer)(struct AVFormatContext *);

	int (*interleave_packet)(struct AVFormatContext *, AVPacket *out, AVPacket *in, int flush);
	int (*query_codec)(enum AVCodecID id, int std_compliance);
	void (*get_output_timestamp)(struct AVFormatContext *s, int stream, int64_t *dts, int64_t *wall);

	int (*control_message)(struct AVFormatContext *s, int type, void *data, size_t data_size);
    	//允许从应⽤程序向设备发送消息。
	int (*write_uncoded_frame)(struct AVFormatContext *, int stream_index, AVFrame **frame, unsigned flags);
    	//写⼀个未编码的AVFrame。

	int (*get_device_list)(struct AVFormatContext *s, struct AVDeviceInfoList *device_list);
	int (*create_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);
	int (*free_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);
	
    enum AVCodecID data_codec; /**< default data codec */

	int (*init)(struct AVFormatContext *);
    	//初始化格式。 可以在此处分配数据，并设置在发送数据包之前需要设置的任何AVFormatContext或AVStream参数。
	void (*deinit)(struct AVFormatContext *);
    	//取消初始化格式
	int (*check_bitstream)(struct AVFormatContext *, const AVPacket*pkt);
    	//设置任何必要的⽐特流过滤，并提取全局头部所需的任何额外数据。
} AVOutputFormat;
```

### 1.5 FFmpeg函数：avformat_new_stream

​		AVStream 即是流通道。

​		例如我们将 H264 和 AAC 码流存储为MP4⽂件的时候，

​				就需要在 MP4⽂件中 增加两个流通道，⼀个存储Video：H264，⼀个存储Audio：AAC。（假设H264和AAC只包含单个流通 道）。

```c
AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c);
		//avformat_new_stream 在 AVFormatContext 中创建 Stream 通道。

关联的结构体
	AVFormatContext ：
		unsigned int nb_streams; 	记录stream通道数⽬。
		AVStream **streams; 		存储stream通道。
	AVStream ：
		int index; 					在AVFormatContext 中所处的通道索引
                
      //avformat_new_stream之后便在 AVFormatContext ⾥增加了 AVStream 通道（相关的index已经被设置了）。
      //之后，我们就可以⾃⾏设置 AVStream 的⼀些参数信息。
            //例如 : codec_id , format ,bit_rate, width , height
```

### 1.6 FFmpeg函数：av_interleaved_write_frame

```c
//函数原型：
	int av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt);
	//说明：将数据包写⼊输出媒体⽂件，并确保正确的交织（保持packet dts的增⻓性）。
	//该函数会在内部根据需要缓存packet，以确保输出⽂件中的packet按dts递增的顺序正确交织。
		//如果⾃⼰进⾏交织则应调⽤av_write_frame()。
//参数：
	s 			媒体⽂件句柄
	pkt 		要写⼊的packet。
			如果packet使⽤引⽤参考计数的内存⽅式，则此函数将获取此引⽤权(可以理解为move了reference)，并在内部在合适的时候进⾏释放。
        	此函数返回后，调⽤者不得通过此引⽤访问数据。如果packet没有引⽤计数，libavformat将进⾏复制。
			此参数可以为NULL（在任何时候，不仅在结尾），以刷新交织队列。
			Packet的stream_index字段必须设置为s-> streams中相应流的索引。
			时间戳记（pts，dts）必须设置为stream's timebase中的正确值（除⾮输出格式⽤AVFMT_NOTIMESTAMPS标志标记，然后可以将其设置为AV_NOPTS_VALUE）。
			同⼀stream后续packet的dts必须严格递增（除⾮输出格式⽤AVFMT_TS_NONSTRICT标记，则它们只必须不减少）。duration也应设置(如果已知)。
```

### 1.7 FFmpeg函数：av_compare_ts

```c
int av_compare_ts(int64_t ts_a, AVRational tb_a, int64_t ts_b, AVRational tb_b);
返回值：
	-1	 ts_a 在ts_b之前
	1 	 ts_a 在ts_b之后
	0 	 ts_a 在ts_b同⼀位置
⽤伪代码：return ts_a == ts_b ? 0 : ts_a < ts_b ? -1 : 1
```



## 2：ffmpeg 音频 编码分析：

### 2.1：PCM存储成AAC流程分析：

从本地⽂件读取PCM数据进⾏AAC格式编码，然后将编码后的AAC数据存储到本地⽂件:

![image-20210801205708962](..\md文档相关图片\ffmpeg pcm转aac编码流程.png)

关键函数说明：

```c
关键函数说明：
	avcodec_find_encoder：		//根据指定的AVCodecID查找注册的编码器。
	avcodec_alloc_context3：		//为AVCodecContext分配内存。
	avcodec_open2：				//打开编码器。
	avcodec_send_frame：			//将AVFrame⾮压缩数据给编码器。
	avcodec_receive_packet：		//获取到编码后的AVPacket数据，收到的packet需要⾃⼰释放内存。
	av_frame_get_buffer: 		//为⾳频或视频帧分配新的buffer。
			//在调⽤这个函数之前，必须在AVFame上设置好以下属性：format(视频为像素格式，⾳频为样本格式)、nb_samples(样本个数，针对⾳频)、
	channel_layout				//(通道类型，针对⾳频)、width/height(宽⾼，针对视频）。
	av_frame_make_writable：		//确保AVFrame是可写的，
        //使⽤av_frame_make_writable()的问题是，
        //在最坏的情况下，它会在您使⽤encode再次更改整个输⼊frame之前复制它. 
        //如果frame不可写，av_frame_make_writable()将分配新的缓冲区，并复制这个输⼊input frame数据，避免和编码器需要缓存该帧时造成冲突。
	av_samples_fill_arrays 填充⾳频帧
```

对于 flush encoder的操作：

```
编码器通常的冲洗⽅法：
	调⽤⼀次 avcodec_send_frame(NULL)(返回成功)，
		然后不停调⽤avcodec_receive_packet() 直到其返回 AVERROR_EOF，取出所有缓存帧，
	avcodec_receive_packet() 返回 AVERROR_EOF 这⼀次是没有有效数据的，仅仅获取到⼀个结束标志
```

### 2.2 PCM样本格式分析：

#### 	2.2.1 **PCM(Pulse Code Modulation，脉冲编码调制)**⾳频数据是未经压缩的⾳频采样数据裸流，

​		它是由模拟信 号经过采样、量化、编码转换成的**标准数字⾳频数据**。

```
描述PCM数据的6个参数：
	1. Sample Rate : 			采样频率。8kHz(电话)、44.1kHz(CD)、48kHz(DVD)。
	2. Sample Size :		 	量化位数。通常该值为16-bit。
	3. Number of Channels : 	通道个数。常⻅的⾳频有⽴体声(stereo)和单声道(mono)两种类型，
				⽴体声包含左声道和右声道。另外还有环绕⽴体声等其它不太常⽤的类型。
	4. Sign : 					表示样本数据是否是有符号位，
				⽐如⽤⼀字节表示的样本数据，有符号的话表示范围为-128 ~127，⽆符号是0 ~ 255。有符号位16bits数据取值范围为-32768~32767。
	5. Byte Ordering : 			字节序。
				字节序是little-endian还是big-endian。通常均为little-endian。
	6. Integer Or Floating Point : 整形或浮点型。⼤
				多数格式的PCM样本数据使⽤整形表示，⽽在⼀些对精度要求⾼的应⽤⽅⾯，使⽤浮点类型表示PCM样本数据（浮点数 float值域为 [-1.0, 1.0]）。
```

#### 	2.2.2 推荐的PCM数据播放⼯具：

```c
//播放格式为f32le，双声道，采样频率48000Hz的PCM数据
	ffplay -f f32le -ac 2 -ar 48000 pcm_audio

//Audacity：			⼀款免费开源的跨平台⾳频处理软件。
//Adobe Auditon:	导⼊原始数据，打开的时候需要选择采样率、格式和字节序。
```

#### 	2.2.3 FFmpeg⽀持的PCM数据格式:

使⽤**ffmpeg -formats**命令，获取ffmpeg⽀持的⾳视频格式，其中我们可以找到⽀持的PCM格式:

```bash
#s是有符号，u是⽆符号，f是浮点数。
#be是⼤端，le是⼩端。	
	DE alaw PCM A-law
	DE f32be PCM 32-bit floating-point big-endian
	DE f32le PCM 32-bit floating-point little-endian
	DE f64be PCM 64-bit floating-point big-endian
	DE f64le PCM 64-bit floating-point little-endian
	DE mulaw PCM mu-law
	DE s16be PCM signed 16-bit big-endian
	DE s16le PCM signed 16-bit little-endian
	DE s24be PCM signed 24-bit big-endian
	DE s24le PCM signed 24-bit little-endian
	DE s32be PCM signed 32-bit big-endian
	DE s32le PCM signed 32-bit little-endian
	DE s8 PCM signed 8-bit
	DE u16be PCM unsigned 16-bit big-endian
	DE u16le PCM unsigned 16-bit little-endian
	DE u24be PCM unsigned 24-bit big-endian
	DE u24le PCM unsigned 24-bit little-endian
	DE u32be PCM unsigned 32-bit big-endian
	DE u32le PCM unsigned 32-bit little-endian
	DE u8 PCM unsigned 8-bit
```

#### 		2.2.4 FFmpeg中Packed和Planar的PCM数据区别

```c
FFmpeg中⾳视频数据基本上都有Packed和Planar两种存储⽅式，
	对于双声道⾳频来说:
			Packed⽅式为两个声道的数据交错存储；
			Planar⽅式为两个声道分开存储。
	假设⼀个L/R为⼀个采样点，数据存储的⽅式如下所示：
		Packed: L R L R L R L R
		Planar: L L L L ... R R R R..

//packed格式
	1 AV_SAMPLE_FMT_U8, 	///< unsigned 8 bits
	2 AV_SAMPLE_FMT_S16, 	///< signed 16 bits
	3 AV_SAMPLE_FMT_S32, 	///< signed 32 bits
	4 AV_SAMPLE_FMT_FLT, 	///< float
	5 AV_SAMPLE_FMT_DBL, 	///< double
//只能保存在AVFrame的		uint8_t *data[0];
//⾳频保持格式如下：		  LRLRLR ...


//planar格式
//planar为FFmpeg内部存储⾳频使⽤的采样格式，所有的Planar格式后⾯都有字⺟P标识
	1 AV_SAMPLE_FMT_U8P, 		///< unsigned 8 bits, planar
	2 AV_SAMPLE_FMT_S16P, 		///< signed 16 bits, planar
	3 AV_SAMPLE_FMT_S32P, 		///< signed 32 bits, planar
	4 AV_SAMPLE_FMT_FLTP, 		///< float, planar
	5 AV_SAMPLE_FMT_DBLP, 		///< double, planar
	6 AV_SAMPLE_FMT_S64, 		///< signed 64 bits
	7 AV_SAMPLE_FMT_S64P, 		///< signed 64 bits, planar

//plane 0: LLLLLLLLLLLLLLLLLLLLLLLLLL...
//plane 1: RRRRRRRRRRRRRRRRRRRR....

//plane 0对应  uint8_t *data[0];
//plane 1对应  uint8_t *data[1];
```

​		FFmpeg默认的**AAC编码器不⽀持AV_SAMPLE_FMT_S16**格式的编码，只⽀持 AV_SAMPLE_FMT_FLTP，

​		这种格式是按平⾯存储，样点是float类型，所谓平⾯也就是：

​				每个声道单独存储，⽐如左声道存储到data[0]中，右声道存储到data[1]中。

#### 		2.2.5 AVFrame结构存储PCM：

​	FFmpeg⾳频解码后和编码前的数据是存放在AVFrame结构中的：

```
Packed格式，	frame.data[0]或frame.extended_data[0]包含所有的⾳频数据中。
Planar格式，	frame.data[i]或者frame.extended_data[i]表示第i个声道的数据。
	（假设声道0是第⼀个）, AVFrame.data数组⼤⼩固定为8，如果声道数超过8，需要从frame.extended_data获取声道数据。
```

#### 		2.2.6 补充说明：

```
Planar模式是ffmpeg内部存储模式，我们实际使⽤的⾳频⽂件都是Packed模式的。
	FFmpeg解码不同格式的⾳频输出的⾳频采样格式不是⼀样。
	测试发现，其中AAC解码输出的数据为浮点型的 AV_SAMPLE_FMT_FLTP 格式，MP3解码输出的数据为 AV_SAMPLE_FMT_S16P 格式（使⽤的mp3⽂件为16位）。
	具体采样格式可以查看解码后的AVFrame中的format成员或编解码器的AVCodecContext中的sample_fmt成员。
		
Planar或者Packed模式直接影响到保存⽂件时写⽂件的操作，操作数据的时候⼀定要先检测⾳频采样格式。
```

​		PCM字节序：

```
	谈到字节序的问题，必然牵涉到两⼤CPU派系。
	那就是Motorola的PowerPC系列CPU和Intel的x86系列CPU。
		PowerPC系列采⽤big endian⽅式存储数据，⽽x86系列则采⽤little endian⽅式存储数据。
		那么究竟什么是big endian，什么⼜是little endian？
	big endian是指低地址存放最⾼有效字节（MSB，Most Significant Bit），
	⽽little endian则是低地址存放最低有效字节（LSB，Least Significant Bit）。

下⾯⽤图像加以说明。⽐如数字0x12345678在两种不同字节序CPU中的存储顺序如下所示：
Big Endian
低地址 ⾼地址
---------------------------------------------------------------------------
-->
| 12 | 34 | 56 | 78 |

Little Endian
低地址 ⾼地址
---------------------------------------------------------------------------
-->
| 78 | 56 | 34 | 12 |
所有⽹络协议都是采⽤big endian的⽅式来传输数据的。所以也把big endian⽅式称之为⽹络字节序。
	当两台采⽤不同字节序的主机通信时，在发送数据之前都必须经过字节序的转换成为⽹络字节序后再进⾏传输。
```

## 3：ffmpeg 视频 编码分析：

### 3.1 从本地读取YUV数据编码为h264格式的数据，然后再存⼊到本地，编码后的数据有带startcode：

![image-20210801212833859](..\md文档相关图片\ffmpeg YUV转h264流程图.png)

### 3.2 相关流程函数说明，与FFmpeg 示例⾳频编码的流程基本⼀致：

```c
//函数说明：
	avcodec_find_encoder_by_name：		//根据指定的编码器名称查找注册的编码器。
	avcodec_alloc_context3：				//为AVCodecContext分配内存。
	avcodec_open2：						//打开编解码器。
	avcodec_send_frame：					//将AVFrame⾮压缩数据给编码器。。
	avcodec_receive_packet：				//获取到编码后的AVPacket数据。
	av_frame_get_buffer: 				//为⾳频或视频数据分配新的buffer。
			//在调⽤这个函数之前，必须在AVFame上设置好以下属性：format(视频为像素格式，⾳频为样本格式)、nb_samples(样本个数，针对⾳频)、
	channel_layout						//(通道类型，针对⾳频)、width/height(宽⾼，针对视频）。
	av_frame_make_writable：				//确保AVFrame是可写的，尽可能避免数据的复制。
			//如果AVFrame不是是可写的，将分配新的buffer和复制数据。
	av_image_fill_arrays: 				//存储⼀帧像素数据存储到AVFrame对应的data buffer。

//编码出来的h264数据可以直接使⽤ffplay播放，也可以使⽤VLC播放。
```

### 3.3 涉及到的相关函数介绍：

```c
int av_image_get_buffer_size(enum AVPixelFormat pix_fmt, int width, int height, int align);
	//函数的作⽤是通过指定像素格式、图像宽、图像⾼来计算所需的内存⼤⼩
	//重点说明⼀个参数align:	此参数是设定内存对⻬的对⻬数，也就是按多⼤的字节进⾏内存对⻬：
	//		⽐如设置为1，表示按1字节对⻬，那么得到的结果就是与实际的内存⼤⼩⼀样。
	//		再⽐如设置为4，表示按4字节对⻬。也就是内存的起始地址必须是4的整倍数。


//	此函数的功能是按照指定的宽、⾼、像素格式来分配图像内存。
int av_image_alloc(uint8_t *pointers[4], int linesizes[4], int w, int h, enum AVPixelFormat pix_fmt, int align);
	//参数：
		//pointers[4]：		保存图像通道的地址。
		//		如果是RGB，则前三个指针分别指向R,G,B的内存地址。第四个指针保留不⽤ 
		//linesizes[4]：		保存图像每个通道的内存对⻬的步⻓，即⼀⾏的对⻬内存的宽度，此值⼤⼩等于图像宽度。
		//w: 				要申请内存的图像宽度。
		//h: 				要申请内存的图像⾼度。
		//pix_fmt: 			要申请内存的图像的像素格式。
		//align: 			⽤于内存对⻬的值。
	//返回值：所申请的内存空间的总⼤⼩。如果是负值，表示申请失败。


int av_image_fill_arrays(uint8_t *dst_data[4], int dst_linesize[4], const uint8_t *src, enum AVPixelFormat pix_fmt, int width, int height, int align);
	//av_image_fill_arrays()函数⾃身不具备内存申请的功能，此函数类似于格式化已经申请的内存，
    //		即通过av_malloc()函数申请的内存空间，或者av_frame_get_buffer()函数申请的内存空间。
	//再者，av_image_fill_arrays()中参数具体说明：
	//			dst_data[4]： 		[out]	对申请的内存格式化为三个通道后，分别保存其地址
	//			dst_linesize[4]: 	[out]	格式化的内存的步⻓（即内存对⻬后的宽度)
	//			*src: 				[in]	av_alloc()函数申请的内存地址。
	//			pix_fmt: 			[in] 	申请 src内存时的像素格式
	//			width: 				[in]	申请src内存时指定的宽度
	//			height: 			[in]	申请scr内存时指定的⾼度
	//			align:				[in]	申请src内存时指定的对⻬字节数。
```

### 3.4 H.264 码率设置

#### 3.4.1 什么是视频码率 

​		视频码率是视频数据（包含视频⾊彩量、亮度量、像素量）每秒输出的位数。⼀般⽤的单位是kbps。

#### 3.4.2 设置视频码率的必要性 

​		在⽹络视频应⽤中，视频质量和⽹络带宽占⽤是相⽭盾的。

​		通常情况下，视频流占⽤的带宽越⾼则视频 质量也越⾼，需要的⽹络带宽也越⼤，解决这⼀⽭盾的钥匙当然是**视频编解码技术**。

​				评判⼀种视频编解 码技术的优劣，是⽐较在相同的带宽条件下，哪个视频质量更好；

​				在相同的视频质量条件下，哪个占⽤ 的⽹络带宽更少（⽂件体积⼩）。 

​		是不是视频码率越⾼，质量越好呢？理论上是这样的。然⽽在我们⾁眼分辨的范围内，当码率⾼到⼀定 程度时，就没有什么差别了。

​		所以码率设置有它的最优值，



#### 3.4.3 H.264（也叫AVC或X264）的⽂件中，视频 的建议码率如下：

![image-20210801214316464](..\md文档相关图片\ffmpeg h264视频推荐码率.png)

#### 3.4.4 ⼿机设置码率建议:

![image-20210801214423496](..\md文档相关图片\ffmpeg 手机码率推荐.png)

#### 3.4.5 H264 参数介绍：

​	⼿动设定的参数会覆盖preset和tune⾥的参数。

​	使⽤**ffmpeg -h encoder=libx264** 命令查询相关⽀持的参数

#### 3.4.6 H264 两种码率控制及参数

​		对于普通⽤户通常有两种码率控制模式：**CRF（Constant Rate Factor)和Two pass ABR。**

​		码率控制是⼀ 种决定为每⼀个视频帧分配多少⽐特数的⽅法，它将决定⽂件的⼤⼩和质量的分配。

**CRF（Constant Rate Factor)：**

```
选择⼀个CRF值：
	量化⽐例的范围为0~51，其中0为⽆损模式，23为缺省值，51可能是最差的。
	该数字越⼩，图像质量越好。
	
从主观上讲，18~28是⼀个合理的范围。
	18往往被认为从视觉上看是⽆损的，它的输出视频质量和输⼊视频⼀模⼀样或者说相差⽆⼏。但从技术的⻆度来讲，它依然是有损压缩。
		若CRF值加6，输出码率⼤概减少⼀半；若CRF值减6，输出码率翻倍。
	通常是在保证可接受视频质量的前提下选择⼀个最⼤的CRF值，
		如果输出视频质量很好，那就尝试⼀个更⼤的值，如果看起来很糟，那就尝试⼀个⼩⼀点值。
注释：本⽂所提到的量化⽐例只适⽤于8-bit x264（10-bit x264的量化⽐例 为0~63），
		你可以使⽤x264--help命令在Output bit depth选项查看输出位深，在各种版本中，8bit是最常⻅的。
```

**选择⼀个preset和tune：**

​	preset：预设是⼀系列参数的集合，这个集合能够在编码速度和压缩率之间做出⼀个权衡。

​	tune：是x264中重要性仅次于preset的选项，

​			它是视觉优化的参数，tune可以理解为视频偏好（或者视频类 型），tune不是⼀个单⼀的参数，⽽是由⼀组参数构成-tune来改变参数设置。

​	profile：参数是-profile:v，它可以将你的输出限制到⼀个特定的 H.264 profile

​			⼀些⾮常⽼的或者 要被淘汰的设备仅⽀持有限的选项，⽐如只⽀持baseline或者main。

相关参数介绍：

```c
preset ： 预设是⼀系列参数的集合，这个集合能够在编码速度和压缩率之间做出⼀个权衡。
	⼀个编码速度稍慢的预设会提供更⾼的压缩效率（压缩效率是以⽂件⼤⼩来衡量的)。
	这就是说，假如你想得到⼀个指定⼤⼩的⽂件或者采⽤恒定⽐特率编码模式，你可以采⽤⼀个较慢的预设来获得更好的质量。
		同样的，对于恒定质量编码模式，你可以通过选择⼀个较慢的预设轻松地节省⽐特率。
	如果你很有耐⼼，通常的建议是使⽤最慢的预设。⽬前所有的预设按照编码速度降序排列为：
		ultrafast
		superfast
		veryfast
		faster
		fast
		medium – default preset
		slow
		slower
		veryslow
		placebo - ignore this as it is not useful (see FAQ)
    
	默认为medium级别。
 你可以使⽤--preset来查看预设列表，也可以通过x264 --fullhelp来查看预设所采⽤的参数配置。

tune是x264中重要性仅次于preset的选项，
    	它是视觉优化的参数，tune可以理解为视频偏好（或者视频类型），
    	tune不是⼀个单⼀的参数，⽽是由⼀组参数构成-tune来改变参数设置。
    当前的 tune包括：
		film：			电影类型，对视频的质量⾮常严格时使⽤该选项
		animation：		动画⽚，压缩的视频是动画⽚时使⽤该选项
		grain：			颗粒物很重，该选项适⽤于颗粒感很重的视频
		stillimage：		静态图像，该选项主要⽤于静⽌画⾯⽐较多的视频
		psnr：			提⾼psnr，该选项编码出来的视频psnr⽐较⾼
    	ssim：			提⾼ssim，该选项编码出来的视频ssim⽐较⾼
		fastdecode：		快速解码，该选项有利于快速解码
		zerolatency：	零延迟，该选项主要⽤于视频直播
	如果你不确定使⽤哪个选项或者说你的输⼊与所有的tune皆不匹配，你可以忽略--tune 选项。
	你可以使⽤-tune来查看tune列表，也可以通过x264 --fullhelp来查看tune所采⽤的参数配置。    

profile:另外⼀个可选的参数是-profile:v，它可以将你的输出限制到⼀个特定的 H.264 profile。
    	⼀些⾮常⽼的或者要被淘汰的设备仅⽀持有限的选项，⽐如只⽀持baseline或者main。
    所有的profile 包括：
		1. baseline profile：		基本画质。⽀持I/P 帧，只⽀持⽆交错（Progressive）和CAVLC；
		2. extended profile：		进阶画质。⽀持I/P/B/SP/SI 帧，只⽀持⽆交错（Progressive）和CAVLC；
		3. main profile：			主流画质。提供I/P/B 帧，⽀持⽆交错（Progressive）和交错（Interlaced），也⽀持CAVLC 和CABAC 的⽀持；
		4. high profile：			⾼级画质。在main Profile 的基础上增加了8x8内部预测、⾃定义量化、 ⽆损视频编码和更多的YUV 格式；
    
想要说明H.264 high profile与H.264 main profile的区别就要讲到H.264的技术发展了。
    JVT于2003年完成H.264基本部分标准制定⼯作，包含baseline profile、extended profile和main profile，分别包括不同的编码⼯具。
    之后JVT⼜完成了H.264 FRExt（即：Fidelity Range Extensions）扩展部分（Amendment）的制定⼯作，
    	包括high profile（HP）、high 10 profile（Hi10P）、high 4:2:2、profile（Hi422P）、high 4:4:4 profile（Hi444P）4个profile。
	
    H.264 baseline profile、extended profile和main profile都是针对8位样本数据、4:2:0格式的视频序列，
    FRExt将其扩展到8～12位样本数据，视频格式可以为4:2:0、4:2:2、4:4:4，设⽴了highprofile（HP）、high 10 profile（Hi10P）、high 4:2:2 profile（Hi422P）、high 4:4:4、profile（Hi444P） 4个profile，这4个profile都以main profile为基础。
        
	在相同配置情况下，High profile（HP）可以⽐Main profile（MP）节省10%的码流量，
        ⽐MPEG-2MP节省60%的码流量，具有更好的编码性能。
    
    根据应⽤领域的不同：
		baseline profile	多应⽤于实时通信领域；
		main profile		多应⽤于流媒体领域；
		high profile		则多应⽤于⼴电和存储领域。
```

#### 3.4.7 其他：低延迟，兼容

```c
低延迟：x264提供了⼀个 -tune zerolatency 选项
兼容性：-profile:v baseline  这将会关闭很多⾼级特性，但是它会提供很好的兼容性。

	要牢记apple quick time 对于x264编码的视频只⽀持 YUV 420颜⾊空间，⽽且不⽀持任何⾼于 mian profile编码档次。
	这样对于quick time 只留下了两个兼容选项baseline和 main。
	其他的编码档次qucik time均不⽀持，虽然它们均可以在其它的播放设备上回放。
	
zerolatency参数的分析：加⼊zerolatency之后，转码路数会明显降低。
	是为了降低在线转码的编码延迟。
	
	zerolatency如何影响转码速率？
	代码中编码延迟影响因素：
		h->frames.i_delay = max(h->param.i_bframe, h->param.rc.i_lookahead)
			+ h->i_thread_frames - 1
			+ h->param.i_sync_lookahead
			+ h->param.b_vfr_input
	设置zerolatency后，相应的参数配置如下：
		if( !strncasecmp( s, "zerolatency", 11 ) )
		{
			param->rc.i_lookahead = 0;
			param->i_sync_lookahead = 0;
			param->i_bframe = 0;
			param->b_sliced_threads = 1;
			param->b_vfr_input = 0;
			param->rc.b_mb_tree = 0;
		}
	zerolatency设置中各个参数的意义：
		rc_lookahead:	该参数为mb-tree码率控制和vbv-lookahead设置可⽤的帧数量，最⼤值为250。
            	对于mbi-tree来说，rc_lookahead值越⼤，会得到更准确的结果，但编码速度也会更慢，
            		因为编码器需要缓存慢rc_lookahead帧数据后，才会开始第⼀帧编码，增加编码延时，因此在实时视频通信中将其设置为0。
        sync_lookahead: 设置⽤于线程预测的帧缓存⼤⼩，最⼤值为250。
            		在第⼆遍及更多遍编码或基于分⽚线程时⾃动关闭。
            		sync_lookahead = 0为关闭线程预测，可减⼩延迟，但也会降低性能。
        bframes: I帧和P帧或者两个P帧之间可⽤的最⼤连续B帧数量，默认值为3。
            	B帧可使⽤双向预测，从⽽显著提⾼压缩率，
            	但由于需要缓存更多的帧数以及重排序的原因，会降低编码速度，增加编码延迟，
            	因此在实时编码时也建议将该值设置为0。
		sliced_threads: 基于分⽚的线程，默认值为off，
            	开启该⽅法在压缩率和编码效率上都略低于默认⽅法，但没有编码延时。
            	除⾮在编码实时流或者对低延迟要求较⾼的场合开启该⽅法，⼀般情况下建议设为off。
        vfr_input： 与force-cfr选项相对应：
				OPT("force-cfr")	p->b_vfr_input = !atobool(value);
				vfr_input= 1时，为可变帧率，使⽤timebase和timestamps做码率控制；
                vfr_input = 0时，为固定帧率，使⽤fps做码率控制.
        mb_tree: 基于宏块树的码率控制。
            	对于每个MB，向前预测⼀定数量的帧(该数量由rc_lookahead和keyint中的较⼩值决定)，
            		计算该MB被引⽤的次数，根据被引⽤次数的多少决定为该MB分配的量化QP值。
            	该⽅法会⽣成⼀个临时stats⽂件，记录了每个P帧中每个MB被参考的情况。
            	使⽤mb_tree的⽅法能够节约⼤概30%的码率，但同时也会增加编码延迟，因此实时流编码时也将其关闭。

      从这⾥分析可以看出来，不单是B帧导致延迟.
```

