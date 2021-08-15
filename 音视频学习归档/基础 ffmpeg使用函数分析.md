## 1：ffmpeg一些基础概念：

```
• 容器／文件（Conainer/File）：		即特定格式的多媒体文件，比如mp4、flv、mkv等。
• 媒体流（Stream）：				 表示时间轴上的一段连续数据，如一段声音数据、一段视频数据或一段字幕数据，
		可以是压缩的，也可以是非压缩的，压缩的数据需要关联特定的编解码器（有些码流音频他是纯PCM）。
• 数据帧／数据包（Frame/Packet）：	通常，一个媒体流是由大量的数据帧组成的，
		对于压缩数据，帧对应着编解码器的最小处理单元，分属于不同媒体流的数据帧交错存储于容器之中。
• 编解码器：							编解码器是以帧为单位实现压缩数据和原始数据之间的相互转换的。

• 复用器：			音频流，视频流，字幕流，其他成分按照一定的规则组合后的视频文件，如MP4，flv等
• 解复用器			把视频文件（Mp4,flv）按一定的规则拆分成音频流，视频流，字幕流等。
```

FFMPEG有8个常用库：

```bash
• AVUtil：		核心工具库，下面的许多其他模块都会依赖该库做一些基本的音视频处理操作。
• AVFormat：		文件格式和协议库，该模块是最重要的模块之一，封装了Protocol层和Demuxer、Muxer层，使得协议和格式对于开发者来说是透明的。
• AVCodec：		编解码库，封装了Codec层，但是有一些Codec是具备自己的License的，FFmpeg是不会默认添加像libx264、FDK-AAC等库的，
		但是FFmpeg就像一个平台一样，可以将其他的第三方的Codec以插件的方式添加进来，然后为开发者提供统一的接口。
• AVFilter：		音视频滤镜库，该模块提供了包括音频特效和视频特效的处理，
		在使用FFmpeg的API进行编解码的过程中，直接使用该模块为音视频数据做特效处理是非常方便同时也非常高效的一种方式
		
• AVDevice：		输入输出设备库，
		比如，需要编译出播放声音或者视频的工具ffplay，就需要确保该模块是打开的，同时也需要SDL的预先编译，因为该设备模块播放声音与播放视频使用的都是SDL库。
• SwrRessample：		该模块可用于音频重采样，可以对数字音频进行声道数、数据格式、采样率等多种基本信息的转换。
• SWScale：			该模块是将图像进行格式转换的模块，比如，可以将YUV的数据转换为RGB的数据，缩放尺寸由1280*720变为800*480。
• PostProc：			该模块可用于进行后期处理，当我们使用AVFilter的时候需要打开该模块的开关，因为Filter中会使用到该模块的一些基础函数。

```



## 2：ffmpeg注册相关函数介绍：

```
◼ av_register_all()：			注册所有组件,4.0已经弃用
◼ avdevice_register_all()		对设备进行注册，比如V4L2等。
◼ avformat_network_init();		初始化网络库以及网络加密协议相关的库（比如openssl）
```



## 3：ffmpeg封装格式函数介绍：

```
◼ avformat_alloc_context();			负责申请一个AVFormatContext结构的内存,并进行简单初始化
◼ avformat_free_context();			释放该结构里的所有东西以及该结构本身

◼ avformat_open_input();			打开输入视频文件
◼ avformat_close_input();			关闭解复用器。关闭后就不再需要使用avformat_free_context 进行释放。

◼ avformat_find_stream_info()：		获取视频文件信息
◼ av_read_frame(); 					读取音视频包
◼ avformat_seek_file(); 			定位文件
◼ av_seek_frame():					定位文件
```

![image-20210801142924355](..\md文档相关图片\ffmpeg封装使用流程.png)

## 4：解码器相关函数

```
• avcodec_alloc_context3(): 		分配解码器上下文
• avcodec_find_decoder()：			根据ID查找解码器
• avcodec_find_decoder_by_name():	根据解码器名字查找解码器

• avcodec_open2()： 					打开编解码器
	• avcodec_decode_video2()：			解码一帧视频数据
	• avcodec_decode_audio4()：			解码一帧音频数据

• avcodec_send_packet(): 			发送编码数据包
• avcodec_receive_frame():		 	接收解码后数据

• avcodec_free_context():			释放解码器上下文，包含了avcodec_close()
• avcodec_close():					关闭解码器
```

解码相关流程：

​	![image-20210801143231983](..\md文档相关图片\ffmpeg解码流程.png)

## 5：组件注册相关：

​	1：使用ffmpeg，首先要执行**av_register_al**l，把全局的解码器、编码器等结 构体注册到各自全局的对象链表里，以便后面查找调用。

![image-20210801143443887](..\md文档相关图片\ffmpeg注册组件理解.png)

2：4.0.2 组件注册方式:

​	FFmpeg内部去做，不需要用户调用API去注册

以codec编解码器为例： 

 1. 在configure的时候生成要注册的组件 

    ./configure:7203:print_enabled_components **libavcodec/codec_list.c**  AVCodec codec_list $CODEC_LIST 

    ​	这里会生成一个codec_list.c 文件，里面只有static const AVCodec *  const codec_list[]数组。 

 2. 在libavcodec/allcodecs.c中，

    ​	将static const AVCodec * const codec_list[] 的编解码器用链表的方式组织起来。

 

例如：对于demuxer/muxer（解复用器，也称容器)，对应文件 libavformat/muxer_list.c和libavformat/**demuxer_list.c**

 1. 对应 libavformat/**muxer_list.c** libavformat/**demuxer_list.c** 这两个文件

    ​	也是在configure的时候生成， 也就是说直接下载源码是没有这两个文件的。

​	 2. 在libavformat/allformats.c将demuxer_list[]和muexr_list[]以链表的方 式组织。

其他组件类似。

## 6：ffmpeg数据结构

### 6.1：ffmpeg数据结构介绍：

```
AVFormatContext			封装格式上下文结构体，也是统领全局的结构体，保存了视频文件封装格式相关信息。
AVInputFormat demuxer	每种封装格式（例如FLV, MKV, MP4, AVI）对应一个该结构体。

AVStream				视频文件中每个视频（音频）流对应一个该结构体。
AVCodecContext			编解码器上下文结构体，保存了视频（音频）编解码相关信息。
AVCodec					每种视频（音频）编解码器(例如H.264解码器)对应一个该结构体。
	
AVPacket				存储一帧压缩编码数据。
AVFrame					存储一帧解码后像素（采样）数据。
```

数据结构之间的关系：

```c
AVFormatContext和AVInputFormat之间的关系
	==》AVFormatContext API调用AVInputFormat 
	==》主要是FFMPEG内部调用

//AVFormatContext 封装格式上下文结构体
	struct AVInputFormat *iformat;
//相关使用方法（可重入）：
		AVInputFormat 每种封装格式（例如FLV, MKV, MP4）
	int (*read_header)(struct AVFormatContext * );
	int (*read_packet)(struct AVFormatContext *, AVPacket *pkt);

int avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options);
```



```c
AVCodecContext和AVCodec之间的关系:
	//AVCodecContext 编码器上下文结构体
		struct AVCodec *codec;

//AVCodec 每种视频（音频）编解码器
	int (*decode)(AVCodecContext *, void *outdata, int *outdata_size, AVPacket *avpkt);
	int (*encode2)(AVCodecContext *avctx, AVPacket *avpkt, const AVFrame *frame, int *got_packet_ptr);
```

数据结构之间的关系：

​	**AVFormatContext**, **AVStream**和**AVCodecContext**之间的关系

![image-20210801152225361](..\md文档相关图片\ffmpeg数据结构关系.png)

区分/查找不同的码流:

```c
// AVMEDIA_TYPE_VIDEO视频流
	video_index = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,-1,-1, NULL, 0);
// AVMEDIA_TYPE_AUDIO音频流
	audio_index = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,-1,-1, NULL, 0);
```

AVPacket和AVFrame之间的关系:

​	AVPacket 存储编码前的数据（如H264帧， AAC帧）

​	AVFrame存储解码后的数据 （如 YUV数据，PCM数据）

### 6.2：ffmpeg数据结构分析：

```c
◼ AVFormatContext
	• iformat：		输入媒体的AVInputFormat，比如指向AVInputFormat ff_flv_demuxer
	• nb_streams：	输入媒体的AVStream 个数
	• streams：		输入媒体的AVStream []数组
	• duration：		输入媒体的时长（以微秒为单位），计算方式可以参考av_dump_format()函数。
	• bit_rate：		输入媒体的码率
◼ AVInputFormat
	• name：			封装格式名称
	• extensions：	封装格式的扩展名
	• id：			封装格式ID
	• 一些封装格式处理的接口函数,比如read_packet()
    
◼ AVStream
	• index：			标识该视频/音频流
	• time_base：		该流的时基，PTS*time_base=真正的时间（秒）
	• avg_frame_rate： 	该流的帧率
	• duration：			该视频/音频流长度
	• codecpar：			编解码器参数属性
◼ AVCodecParameters
	• codec_type：	媒体类型AVMEDIA_TYPE_VIDEO/AVMEDIA_TYPE_AUDIO等
	• codec_id：		编解码器类型，AV_CODEC_ID_H264/AV_CODEC_ID_AAC等
	
◼ AVCodecContext
	• codec：			编解码器的AVCodec，比如指向AVCodec ff_aac_latm_decoder
	• width, height：	图像的宽高（只针对视频）
	• pix_fmt：			像素格式（只针对视频）
	• sample_rate：		采样率（只针对音频）
	• channels：			声道数（只针对音频）
	• sample_fmt：		采样格式（只针对音频）
◼ AVCodec
	• name：			编解码器名称
	• type：			编解码器类型
	• id：			编解码器ID
	• 一些编解码的接口函数，比如int (*decode)()
    
◼ AVPacket
	• pts：				显示时间戳
	• dts：				解码时间戳
	• data：				压缩编码数据
	• size：				压缩编码数据大小
	• pos:			 	 数据的偏移地址
	• stream_index：		所属的AVStream
◼ AVFrame
	• data：				解码后的图像像素数据（音频采样数据）
	• linesize：			对视频来说是图像中一行像素的大小；对音频来说是整个音频帧的大小
	• width, height：	图像的宽高（只针对视频）
	• key_frame：		是否为关键帧（只针对视频） 。
	• pict_type：		帧类型（只针对视频） 。例如I， P， B
	• sample_rate：		音频采样率（只针对音频）
	• nb_samples：		音频每通道采样数（只针对音频）
	• pts：				显示时间戳
```

## 7：ffmpeg内存模型：

### 7.1：内存模型：

![image-20210801161826452](..\md文档相关图片\ffmpeg内存模型.png)

◼ 从现有的Packet拷贝一个新Packet的时候，有两种情况： 

​	• ①两个Packet的buf引用的是同一数据缓存空间，这时 候要注意数据缓存空间的释放问题；

​	 • ②两个Packet的buf引用不同的数据缓存空间，每个 Packet都有数据缓存空间的copy；

◼ 对于多个AVPacket共享同一个缓存空间，FFmpeg使用的引 用计数的机制（reference-count）： 

​		◼ 初始化引用计数为0，只有真正分配AVBuffer的时候， 引用计数初始化为1; 

​		◼ 当有新的Packet引用共享的缓存空间时，就将引用计数 +1； 

​		◼ 当释放了引用共享空间的Packet，就将引用计数-1；

​		◼ 引 用计数为0时，就释放掉引用的缓存空间AVBuffer。 

◼ AVFrame也是采用同样的机制

### 7.2: AVPacket常用API:

```c
AVPacket *av_packet_alloc(void); 				//分配AVPacket这个时候和buffer没有关系
void av_packet_free(AVPacket **pkt); 			//释放AVPacket和_alloc对应
void av_init_packet(AVPacket *pkt); 			//初始化AVPacket只是单纯初始化pkt字段
int av_new_packet(AVPacket *pkt, int size); 	//给AVPacket的buf分配内存，引用计数初始化为1
int av_packet_ref(AVPacket *dst, const AVPacket *src); 	//增加引用计数
void av_packet_unref(AVPacket *pkt); 					//减少引用计数
void av_packet_move_ref(AVPacket *dst, AVPacket *src); 	//转移引用计数
AVPacket *av_packet_clone(const AVPacket *src); 		//等于av_packet_alloc()+av_packet_ref()
```

### 7.3：AVFrame常用API：

```c
AVFrame *av_frame_alloc(void); 							//分配AVFrame
void av_frame_free(AVFrame **frame); 					//释放AVFrame
int av_frame_ref(AVFrame *dst, const AVFrame *src); 	//增加引用计数
void av_frame_unref(AVFrame *frame); 					//减少引用计数
void av_frame_move_ref(AVFrame *dst, AVFrame *src); 	//转移引用计数
int av_frame_get_buffer(AVFrame *frame, int align); 	//根据AVFrame分配内存
AVFrame *av_frame_clone(const AVFrame *src); 			//等于av_frame_alloc()+av_frame_ref()
```

## 8：采样率：采样设备每秒抽取样本的次数

### 8.1 通过重采样，我们可以对：

	 1. sample rate(采样率)
	 2. sample format(采样格式)
	 3. channel layout(通道布局，可以通过此参数获取声道数。

### 8.2 采样格式和量化精度（位宽）

```c
FFMpeg中⾳频格式有以下⼏种，每种格式有其占⽤的字节数信息（libavutil/samplefmt.h）：
enum AVSampleFormat {
	AV_SAMPLE_FMT_NONE = -1,
	AV_SAMPLE_FMT_U8, 			///< unsigned 8 bits
	AV_SAMPLE_FMT_S16, 			///< signed 16 bits
	AV_SAMPLE_FMT_S32, 			///< signed 32 bits
	AV_SAMPLE_FMT_FLT, 			///< float
	AV_SAMPLE_FMT_DBL, 			///< double
	AV_SAMPLE_FMT_U8P, 			///< unsigned 8 bits, planar
	AV_SAMPLE_FMT_S16P, 		///< signed 16 bits, planar
	AV_SAMPLE_FMT_S32P, 		///< signed 32 bits, planar
	AV_SAMPLE_FMT_FLTP, 		///< float, planar
	AV_SAMPLE_FMT_DBLP,		 	///< double, planar
	AV_SAMPLE_FMT_S64, 			///< signed 64 bits
	AV_SAMPLE_FMT_S64P, 		///< signed 64 bits, planar
	AV_SAMPLE_FMT_NB 			///< Number of sample formats. DO NOT USE if linking dynamically
};
```

### 8.3 分⽚（plane）和打包（packed）

​		以双声道为例，带P（plane）的数据格式在存储时，其左声道和右声道的数据是分开存储的，左声道的 数据存储在data[0]，右声道的数据存储在data[1]，每个声道的所占⽤的字节数为linesize[0]和 linesize[1]； 

​		不带P（packed）的⾳频数据在存储时，是按照LRLRLR...的格式交替存储在data[0]中，linesize[0] 表示总的数据量。

### 8.4 声道分布（channel_layout)

​		声道分布在**FFmpeg\libavutil\channel_layout.h**中有定义，

​		⼀般来说⽤的⽐较多的是 AV_CH_LAYOUT_STEREO（双声道）和AV_CH_LAYOUT_SURROUND（三声道），

这两者的定义如 下：

```c
#define AV_CH_LAYOUT_STEREO    (AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT)
#define AV_CH_LAYOUT_SURROUND  (AV_CH_LAYOUT_STEREO|AV_CH_FRONT_CENTER)
```

### 8.5 ⾳频帧的数据量计算

```
	⼀帧⾳频的数据量（字节）=channel数 * nb_samples样本数 * 每个样本占⽤的字节数
	如果该⾳频帧是FLTP格式的PCM数据，包含1024个样本，双声道，那么该⾳频帧包含的⾳频数据量是2*1024*4=8192字节。
	
	AV_SAMPLE_FMT_DBL ： 2*1024*8 = 16384
```

### 8.6 音频播放时间计算

```
	以采样率44100Hz来计算，每秒44100个sample，⽽正常⼀帧为1024个sample：
		可知每帧播放时间/1024=1000ms/44100，得到每帧播放时间=1024*1000/44100=23.2ms （更精确的是23.21995464852608）。
		⼀帧播放时间（毫秒） = nb_samples样本数 *1000/采样率 =
	（1）1024*1000/44100=23.21995464852608ms ->约等于 23.2ms，精度损失了0.011995464852608ms，
		如果累计10万帧，误差>1199毫秒，如果有视频⼀起的就会有⾳视频同步的问题。 
		如果按着23.2去计算pts（0 23.2 46.4 ）就会有累积误差。
	（2）1024*1000/48000=21.33333333333333ms
```

## 9：FFmpeg重采样API:

### 9.1 重采样api

```c
//分配⾳频重采样的上下⽂
	struct SwrContext *swr_alloc(void);
//当设置好相关的参数后，使⽤此函数来初始化SwrContext结构体
	int swr_init(struct SwrContext *s);
//分配SwrContext并设置/重置常⽤的参数。
	struct SwrContext *swr_alloc_set_opts(struct SwrContext *s, // ⾳频重采样上下⽂
				int64_t out_ch_layout, 					// 输出的layout, 如：5.1声道
				enum AVSampleFormat out_sample_fmt, 	// 输出的采样格式。Float, S16,⼀般选⽤是s16 绝⼤部分声卡⽀持
				int out_sample_rate, 					//输出采样率
				int64_t in_ch_layout, 					// 输⼊的layout
				enum AVSampleFormat in_sample_fmt, 		// 输⼊的采样格式
				int in_sample_rate, 					// 输⼊的采样率
				int log_offset, 						// ⽇志相关，不⽤管先，直接为0
				void *log_ctx 							// ⽇志相关，不⽤管先，直接为NULL
			);
//将输⼊的⾳频按照定义的参数进⾏转换并输出
	int swr_convert(struct SwrContext *s, 		// ⾳频重采样的上下⽂
				uint8_t **out, 					// 输出的指针。传递的输出的数组
				int out_count, 					//输出的样本数量，不是字节数。单通道的样本数量。
				const uint8_t **in , 			//输⼊的数组，AVFrame解码出来的DATA
				int in_count 					// 输⼊的单通道的样本数量。
			);
		//返回值 <= out_count		in和in_count可以设置为0，以最后刷新最后⼏个样本。
//释放掉SwrContext结构体并将此结构体置为NULL;
	void swr_free(struct SwrContext **s);

	
```

⾳频重采样，采样格式转换和混合库。

```c
/*与lswr的交互是通过SwrContext完成的，SwrContext被分配给swr_alloc（）或swr_alloc_set_opts（）。
	它是不透明的，所以所有参数必须使⽤AVOptions API设置。
为了使⽤lswr，你需要做的第⼀件事就是分配SwrContext。 
	这可以使⽤swr_alloc（）或swr_alloc_set_opts（）来完成。 
		如果您使⽤前者，则必须通过AVOptions API设置选项。
    	后⼀个函数提供了相同的功能，但它允许您在同⼀语句中设置⼀些常⽤选项。
	例如，以下代码将设置从平⾯浮动样本格式到交织的带符号16位整数的转换，从48kHz到44.1kHz的下采样，以及从5.1声道到⽴体声的下混合（使⽤默认混合矩阵）。 
*/

//这是使⽤swr_alloc（）函数。
	SwrContext *swr = swr_alloc();
	av_opt_set_channel_layout(swr, "in_channel_layout", AV_CH_LAYOUT_5POINT1, 0);
	av_opt_set_channel_layout(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(swr, "in_sample_rate", 48000, 0);
	av_opt_set_int(swr, "out_sample_rate", 44100, 0);
	av_opt_set_sample_fmt(swr, "in_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
	av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

//同样的⼯作也可以使⽤swr_alloc_set_opts（）：
	SwrContext *swr = swr_alloc_set_opts(NULL, 		// we're allocating a new context
		AV_CH_LAYOUT_STEREO, 						// out_ch_layout
		AV_SAMPLE_FMT_S16, 							// out_sample_fmt
		44100, 										// out_sample_rate
		AV_CH_LAYOUT_5POINT1, 						// in_ch_layout
		AV_SAMPLE_FMT_FLTP, 						// in_sample_fmt
		48000, 										// in_sample_rate
		0, 											// log_offset
		NULL); 										// log_ctx
//⼀旦设置了所有值，它必须⽤swr_init（）初始化。 
//如果需要更改转换参数，可以使⽤AVOptions来更改参数，如上⾯第⼀个例⼦所述; 
//或者使⽤swr_alloc_set_opts（），但是第⼀个参数是分配的上下⽂。 您必须再次调⽤swr_init（）。

//转换本身通过重复调⽤swr_convert（）来完成。 
//请注意，如果提供的输出空间不⾜或采样率转换完成后，样本可能会在swr中缓冲，这需要“未来”样本。
//可以随时通过使⽤swr_convert（）（in_count可以设置为0）来检索不需要将来输⼊的样本。
//在转换结束时，可以通过调⽤具有NULL in和in incount的swr_convert（）来刷新重采样缓冲区。
```

### 9.2 重采样Demo：

​	FFmpeg\doc\examples\resampling_audio.c

​	**ffmpeg 命查找重定向令**

```bash
	⽐如我们在-f fmt打算指定格式时，怎么知道什么样的格式才是适合的format？
		可以通过ffmpeg -formats | findstr xx的⽅式去查找。
		对于findstr，/i是忽略⼤⼩写
⽐如：
	查找Audio的裸流解复⽤器：ffmpeg -formats | findstr /i audio
	查找Video的裸流解复⽤器：ffmpeg -formats | findstr /i video
```



## 10：ffmpeg AVIO内存输⼊模式

```c
AVIOContext *avio_alloc_context(
			unsigned char *buffer, 
			int buffer_size,	//是 read_packet / write_packet 的第⼆个和第三个参数，是供FFmpeg使⽤的数据区。
						//buffer ⽤作FFmpeg输⼊时，由⽤户负责向 buffer 中填充数据，FFmpeg取⾛数据。
						//buffer ⽤作FFmpeg输出时，由FFmpeg负责向 buffer 中填充数据，⽤户取⾛数据。
			int write_flag,   	//是缓冲区读写标志，读写的主语是指FFmpeg。
    					//write_flag 为1时， buffer ⽤于写，即作为FFmpeg输出。
						//write_flag 为0时， buffer ⽤于读，即作为FFmpeg输⼊。
			void *opaque,    	//是 read_packet / write_packet 的第⼀个参数，指向⽤户数据。
			int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),	//函数指针，指向⽤户编写的回调函数
			int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
			int64_t (*seek)(void *opaque, int64_t offset, int whence));     //函数指针，需要⽀持seek时使⽤。 可以类⽐fseek的机制
```

## 11 ：ffmpeg视频编解码流程：

### 11.1 视频编解码流程图：

![image-20210801171424322](..\md文档相关图片\ffmpeg视频编解码流程.png)

### 11.2 关键函数：

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

### 11.3 关键数据结构：

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

### 11.4 编解码API介绍

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

编解码API流程说明：

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

### 11.5 avcodec_send_packet和avcodec_receive_frame

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

## 12 ffmpeg音频编解码流程

### 12.1 音频编解码流程：

![image-20210801181114283](..\md文档相关图片\ffmpeg音频编解码流程.png)

### 12.2 音频编解码相关函数

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

### 12.3 音频编解码相关函数：

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



### 12.4 音频编解码API介绍：

​	参考视频编解码api介绍

## 13 ffmpeg练习：  MP4 FLV H264和AAC编码分析
