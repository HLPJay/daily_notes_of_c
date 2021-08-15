/***********************************
实现该目录下可直接运行编译的一个环境：
	1:拷贝ffmpeg相关头文件和需要的so
	2:拷贝SDL相关的头文件和需要的so
	3:配置Makefile,实现可直接编译
	4:或者写个脚本直接对相关的环境进行配置
************************************/

#include <libavcodec/avcodec.h>       //各种类型声音/图像编解码
#include <libavformat/avformat.h>     //各种音视频封装格式的生成和解析，解码需要的相关信息以及上下文结构
// #include <libswscale/swscale.h>    //图像像素格式转换
#include <libswresample/swresample.h> //重采样

#include <stdio.h>
#include <signal.h>   //处理ctrl+c和相关异常终止的情况
#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

typedef struct AudioParams {
    int freq;            //采样率
    int channels;		 //声道
    int64_t channel_layout;  //声道布局
    enum AVSampleFormat fmt; //音频格式
    int frame_size;			 //一帧的大小
    int bytes_per_sec;       //一个采样里的大小
} AudioParams;

AudioParams audio_hw_params_target; 
AudioParams audio_hw_params_src; //原来存储的信息

static void sigterm_handler(int signum)
{
	printf("Get signal: %d, Jump out ...", signum);
	exit(1);
}

//packet队列的实现以及初始化和函数定义
//使用库中的list来实现队列功能
typedef struct PacketQueue{
	AVPacketList * first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
}PacketQueue;
//通过队列管理播放包的存和取，通过参数控制播放结束
PacketQueue audio_queue;
int quit = 0;

void packet_queue_init(PacketQueue *q);
int packet_queue_put(PacketQueue *q, AVPacket *pkt);
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);



//处理SDL的回调函数
void audio_callback(void *userdata, Uint8 *stream, int len);

//相关实际的解码函数
AVFrame frame;
//解码器上下文  和存储转码后的数据包
int audio_decode_packet(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size);

//重采样，把数据写入buff进行播放
struct SwrContext * swr_ctx = NULL;
int resample(AVFrame * af, uint8_t * audio_buf, int * audio_buf_size);


//采用队列的方式，实现音频数据的读和取
int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("Usage: %s <playfile> \n", argv[0]);
		exit(1); //exit是系统级的， return是函数级的
	}

	signal(SIGINT, sigterm_handler);
	signal(SIGTERM, sigterm_handler);
/**************************************
AVFormatContext ：统领全局的基本结构体。主要用于处理封装格式（FLV/MKV/RMVB等）。
AVIOContext ：输入输出对应的结构体，用于输入输出（读写文件，RTMP协议等）。
AVStream, AVCodecContext ：视音频流对应的结构体，用于视音频编解码。
AVFrame ：存储非压缩的数据（视频对应RGB/YUV像素数据，音频对应PCM采样数据）
AVPacket ：存储压缩数据（视频对应H.264等码流数据，音频对应AAC/MP3等码流数据）
***************************************/

	//编解码有两种方案，1：老版的方案，全部注册
	av_register_all();

	//SDL的初始化 ==》音频初始化
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("init SDL error - %s\n", SDL_GetError());
		return -1;
	}
	//打开输入的文件,获取结构体信息
	//根据输入的文件查找对应的流
	//打印找到的相关流信息
	//根据流照对应的解码器
	//打开对应的编辑码器
	//编解码器，编码和未编码包 对应内存的申请
	//从包中读取文件  进行编解码处理
		//发送包  获取包的大小  获取编解码后的包  然后处理
	AVFormatContext * pFormatCtx = NULL;

	int ret = 0;
	ret = avformat_open_input(&pFormatCtx, argv[1], NULL, NULL);
	if(ret != 0)
	{
		printf("avformat_open_input Get format context error. \n");
		goto ret1;
	}

	ret = avformat_find_stream_info(pFormatCtx, NULL);
	if(ret < 0)
	{
		printf("avformat_find_stream_info find stream error. \n");
		goto ret2;
	}

	//打印相关的输入信息
	av_dump_format(pFormatCtx, 0, argv[1], 0);
	//查找音频最佳的流，根据流编号找到最合适的解码器
	int audiostream = -1;
	audiostream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if(audiostream < 0)
	{
		printf("av_find_best_stream: get best audio stream error. \n");
		ret = -1;
		goto ret2;
	}
	printf("first get the best stream is %d \n", audiostream);
	audiostream = -1;
	for(int i=0; i<pFormatCtx->nb_streams; i++)
	{
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO && audiostream < 0) 
		{
			audiostream=i;
		}
	} 
	printf("second get the best stream is %d \n", audiostream);
	if(audiostream==-1)
	{
		printf("error of get stream %d \n",  audiostream);
		goto ret2;
	}

	AVCodec *pCodex = NULL;
	pCodex = avcodec_find_decoder(pFormatCtx->streams[audiostream]->codecpar->codec_id);
	if(pCodex == NULL)
	{
		printf("avcodec_find_decoder error \n");
		goto ret2;
	}

	//解码器上下文分配内存，初始化
	AVCodecContext * pCodecCtx = NULL;
	pCodecCtx = avcodec_alloc_context3(pCodex);
	if(pCodecCtx == NULL || avcodec_copy_context(pCodecCtx, pFormatCtx->streams[audiostream]->codec) != 0)
	{
		printf("avcodec_alloc_context3 alloc codec context error. \n");
		goto ret2;
	}
	//这里应该是没有问题的  可能要加个copy
	ret = avcodec_open2(pCodecCtx, pCodex, NULL);
	if(ret != 0 ) 
	{
		printf("avcodec_open2 init codecContext error. \n");
   		goto ret3;
	}
    
	// if(ret != 0)
	// {
	// 	printf("avcodec_open2 init codecContext error. \n");
	// 	goto ret3;
	// }

	//声道布局 声道 采样率
	int sample_rate, nb_channels;
	int64_t channel_layout;

	sample_rate = pCodecCtx->sample_rate;
	nb_channels = pCodecCtx->channels;
	channel_layout = pCodecCtx->channel_layout;

	printf("channel_layout=%" PRId64 "\n", channel_layout);
	printf("nb_channels=%d\n", nb_channels);
	printf("freq=%d\n", sample_rate);
	//修改布局 
	if(!channel_layout || nb_channels!= av_get_channel_layout_nb_channels(channel_layout))
	{
		channel_layout = av_get_default_channel_layout(nb_channels);
		channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
		printf("correction channel layout. \n");
	}

	printf("correction channel_layout=%" PRId64 "\n", channel_layout);
	printf("correction nb_channels=%d\n", nb_channels);
	printf("correction freq=%d\n", sample_rate);

	//根据已知信息，定义sdl要播放的结构体参数
	SDL_AudioSpec wanted_spec, spec;  
	wanted_spec.freq = sample_rate;        //每个样本音频频率
	wanted_spec.format = AUDIO_S16SYS;     //音频格式
	wanted_spec.channels = nb_channels;    //声道
	wanted_spec.silence = 0;		       //用于将缓冲区设为静音的值
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE; //音频缓冲区期望大小，有一定的计算方式
	wanted_spec.callback = audio_callback;       //回调函数，应该加保护
	wanted_spec.userdata = pCodecCtx;            //作为第一个参数传给回调
	//wanted_spec.size    = 1024;   //音频缓冲区大小，SDL_OpenAudio计算

	//传入期望的参数，可以获得实际的参数
	ret = SDL_OpenAudio(&wanted_spec, &spec);
	if(ret < 0)
	{
		printf("SDL_OpenAudio %s \n", SDL_GetError());
		goto ret4;
	}
	printf("freq : %d \t channels: %d \t buffer size: %u \n",  spec.freq, spec.channels, spec.size);
	
	//定义一个保存参数的结构体，方便取用相关参数
	audio_hw_params_target.freq          = spec.freq; 
	audio_hw_params_target.channels      = spec.channels;
	audio_hw_params_target.channel_layout= channel_layout;
	audio_hw_params_target.fmt           = AV_SAMPLE_FMT_S16;
	audio_hw_params_target.frame_size    = av_samples_get_buffer_size(
		NULL, audio_hw_params_target.channels, 1, audio_hw_params_target.fmt, 1);
	audio_hw_params_target.bytes_per_sec = av_samples_get_buffer_size(
		NULL, audio_hw_params_target.channels, audio_hw_params_target.freq, audio_hw_params_target.fmt, 1);

	if(audio_hw_params_target.frame_size < 0 || audio_hw_params_target.bytes_per_sec < 0)
	{
		printf("Get size of frame buffer error. \n");
		goto ret4;
	}

	audio_hw_params_src = audio_hw_params_target;

	// AVPacket packet;
	// 这里先初始化packet的队列：==>这个有点不合理把
	packet_queue_init(&audio_queue);
	SDL_PauseAudio(0); //暂停音频处理回调函数
	int i = 0;

	SDL_Event       event;
	AVPacket packet;
	//从上下文中读包,读到则塞进队列，否则释放packet，av_read_frame会alloc
	//获取到停止事件，然后进行停止
	int flag = 0;
	while(av_read_frame(pFormatCtx, &packet) >= 0)
	{
		if(packet.stream_index == audiostream)
		{
			packet_queue_put(&audio_queue, &packet);
		}else
		{
			av_free_packet(&packet); //取出后播放完也会释放
		}

		//从事件队列读取相应的事件
		SDL_PollEvent(&event);
	    switch(event.type) {
		    case SDL_QUIT:
				quit = 1;
				SDL_Quit();
				exit(0);
				break;
		    default:
				break;
	    }
	}

	while(1) SDL_Delay(1000);
ret4:
	avcodec_close(pCodecCtx);
ret3:
	avcodec_free_context(&pCodecCtx);
ret2:
	avformat_close_input(&pFormatCtx);
ret1:
	SDL_Quit();
	return ret;
}

void packet_queue_init(PacketQueue *q)
{
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = NULL;
	q->cond = NULL;
}

//往队列中存包
int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
	AVPacketList *pkt1;
	//如果未分配内存 则分配该数据包
	if(av_dup_packet(pkt) < 0) 
	{
		return -1;
	}
	//定义队列中的变量  队列的下一个由list控制
	pkt1 = av_malloc(sizeof(AVPacketList));
	if(pkt1 == NULL)
		return -1;
	pkt1->pkt = *pkt; //这个应该是拷贝构造
	pkt1->next = NULL;

	//加锁去塞数据 
	SDL_LockMutex(q->mutex);
	if(q->last_pkt == NULL)
		q->first_pkt = pkt1;
	else 
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets ++;
	q->size += pkt1->pkt.size;

	SDL_CondSignal(q->cond);
	SDL_UnlockMutex(q->mutex); //注意 l小写
}

//从队列中取包 
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;

	//加锁去取数据
	SDL_LockMutex(q->mutex);
	for(;;)
	{
		if(quit) //这是控制sdl结束用的
			return -1;
		pkt1 = q->first_pkt;
		if(pkt1 != NULL)
		{
			q->first_pkt = q->first_pkt->next;
			if(q->first_pkt == NULL)
				q->last_pkt == NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			SDL_UnlockMutex(q->mutex);
			return 1;
		}else if(!block)
		{
			SDL_UnlockMutex(q->mutex);
			return 0;
		}else
		{
			SDL_CondWait(q->cond, q->mutex);
		}
	}

	SDL_UnlockMutex(q->mutex);
	return 0;
}

// #define SDL_AUDIO_BUFFER_SIZE 1024
// #define MAX_AUDIO_FRAME_SIZE 192000
//SDL回调函数的处理 音频缓冲区指针  该缓冲区大小
void audio_callback(void *userdata, Uint8 *stream, int len)
{
	AVCodecContext *paCodecCtx = (AVCodecContext*) userdata;

	static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3)/2]; //1.5倍的缓冲区
	static unsigned int audio_buff_size = 0;
	static unsigned int audio_buff_index = 0;

	int len1, audio_size;
	while(len > 0)
	{
		if(audio_buff_index >= audio_buff_size)
		{
			audio_size = audio_decode_packet(paCodecCtx, audio_buf, sizeof(audio_buf));
			if(audio_size <0)
			{
				audio_buff_size = 1024;
				memset(audio_buf, 0, audio_buff_size);
			}
			else
			{
				audio_buff_size = audio_size;
			}
			audio_buff_index = 0;
		}
		//确定处理完
		len1 = audio_buff_size - audio_buff_index;
		if(len1 >len)
			len1 = len;
		//len1是缓冲区大小 audio_buff_index是已经播放的buffer长度 audio_buff_size是buffer总长度
		memcpy(stream, (uint8_t *)audio_buf + audio_buff_index, len1);
		len -= len1;
		stream += len1;
		audio_buff_index += len1;
	}
}

//实际的解码
// AVFrame frame;
//解码器上下文  和存储转码后的数据包
int audio_decode_packet(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size)
{
	static AVPacket pkt;
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;

	int len1, data_size = 0;

	for(;;)
	{
		//packet 中有数据
		while(audio_pkt_size > 0)
		{
			int get_frame;
			len1 = avcodec_decode_audio4(aCodecCtx, &frame, &get_frame, &pkt);
			if(len1<0)
			{
				//如果解析错误，直接跳出这个包
				printf("decode packet error %d.\n", len1);
				audio_pkt_size = 0;
				break;
			}
			//判断解析包的进度，没有必要保存数据位置把，avcodec_decode_audio4会处理吗？
			audio_pkt_size -= len1;
			data_size = 0;
			if(get_frame)
			{
				//把解析后的包写入到buffer中
				data_size = resample(&frame, audio_buf, &buf_size);
				assert(data_size <= buf_size);
			}
			if(data_size <= 0)
			{
				continue;
			}
			return data_size;
		}
		//解析错误的情况下，应该有包没有释放
		if(pkt.data)
			av_free_packet(&pkt);

		if(quit) {
			return -1;
		}

		//获取一个包, 下一次就可用了
		if(packet_queue_get(&audio_queue, &pkt, 1) < 0)
		{
			return -1;
		}

		//获取包的大小，以便解析
		audio_pkt_size = pkt.size;
	}
	return 0;
}

// struct SwrContext * swr_ctx = NULL;
// 对获取到的frame进行重采样并且放入到buffer中
// 一个frame 目标buffer和大小
int resample(AVFrame * aframe, uint8_t * audio_buf, int * audio_buf_size)
{
	int data_size = 0;
	int resampled_data_size = 0; //重采样数据的大小
	int64_t dec_channel_layout;


	//获取当前的buffer大小 这里frame相关的结构体和函数需要整理
	data_size = av_samples_get_buffer_size(NULL, av_frame_get_channels(aframe),
						aframe->nb_samples, aframe->format, 1);
	//取声道布局，或者用默认的
	dec_channel_layout = ((aframe->channel_layout && av_frame_get_channels(aframe))
			== av_get_channel_layout_nb_channels(aframe->channel_layout)) ?
			aframe->channel_layout : av_get_default_channel_layout(av_frame_get_channels(aframe));

	//判断解码后的数据的头是否一致
	//设置重采样参数 并初始化

	if(aframe->format   != audio_hw_params_src.fmt ||
			aframe->sample_rate != audio_hw_params_src.freq ||
			dec_channel_layout != audio_hw_params_src.channel_layout ||
			!swr_ctx )
	{
		swr_free(&swr_ctx);
		// channels  为 音频的 通道数 1 2 3 4 5.....
		// channel_layout  为音频 通道格式类型 如 单通道 双通道 .....
		swr_ctx = swr_alloc_set_opts(NULL, 
			audio_hw_params_target.channel_layout,  //输出通道格式，单声道还是双声道
			audio_hw_params_target.fmt,             //输出的声音格式，PCM几种声音格式可以直接播放
			audio_hw_params_target.freq,            //输出采样率
			dec_channel_layout,			//输入声道格式 ==》frame的
			aframe->format,				//输入声音格式
			aframe->sample_rate,        //输入的采样率
			0, NULL);
		
		//保护判断
		if(!swr_ctx || swr_init(swr_ctx) < 0)
		{
			av_log(NULL, AV_LOG_ERROR,
                   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                    aframe->sample_rate, av_get_sample_fmt_name(aframe->format), av_frame_get_channels(aframe),
                    audio_hw_params_target.freq, av_get_sample_fmt_name(audio_hw_params_target.fmt), audio_hw_params_target.channels);
            swr_free(&swr_ctx);
            return -1;
		}
		printf("swr init success. \n");
		//这里没有必要重新获取原包吧。。。
		audio_hw_params_src.channels = av_frame_get_channels(aframe);
		audio_hw_params_src.fmt = aframe->format;
		audio_hw_params_src.freq = aframe->sample_rate;
	}

	//开始重采样处理
	if(swr_ctx)
	{
		const uint8_t **in = (const uint8_t **)aframe->extended_data;
        uint8_t **out = &audio_buf;
        //每个通道可用与输出的样本数 计算要输出的buffer的大小
        int out_count = (int64_t)aframe->nb_samples * audio_hw_params_target.freq / aframe->sample_rate + 256;
        int out_size  = av_samples_get_buffer_size(NULL, audio_hw_params_target.channels, out_count, audio_hw_params_target.fmt, 0);

        int len2;
        if(out_size < 0)  //
        {
        	av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            return -1;
        }

        //分配缓冲区 够用和不够用的场景
        av_fast_malloc(&audio_buf, audio_buf_size, out_size);
        if(audio_buf == NULL)
        	return AVERROR(ENOMEM);

        //开始重采样处理 out_count aframe->nb_samples 输出和输入每个通道可输的样本数
        //返回每个通道输出的样本数
        len2 = swr_convert(swr_ctx, out, out_count, in, aframe->nb_samples);
        if(len2 < 0)
        {
        	av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
            return -1;
        }

        //这个应该也没必要吧？
        if(len2 == out_count) //这个代表只处理了一个通道？
        {
        	av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if (swr_init(swr_ctx) < 0)
                swr_free(&swr_ctx);
        }
        resampled_data_size = len2 * audio_hw_params_target.channels * 
        	av_get_bytes_per_sample(audio_hw_params_target.fmt);
	}
	else //直接返回原数据？
	{
		printf("swr_ctx is null. \n ");
		audio_buf = aframe->data[0];
        resampled_data_size = data_size;
	}
	return resampled_data_size;
}