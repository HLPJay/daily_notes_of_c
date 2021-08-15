// #include <libavcodec/avcodec.h>       //各种类型声音/图像编解码
// #include <libavformat/avformat.h>     //各种音视频封装格式的生成和解析，解码需要的相关信息以及上下文结构
// // #include <libswscale/swscale.h>    //图像像素格式转换
// #include <libswresample/swresample.h> //重采样

// #include <stdio.h>
// #include <signal.h>   //处理ctrl+c和相关异常终止的情况
// #include <assert.h>

// #include <SDL2/SDL.h>
// #include <SDL2/SDL_thread.h>
#include <unistd.h>
#include <fcntl.h>
#include "MP3_AAC_controller.h"
namespace AUDIO_PLAY{

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

//相关上下文参数 
struct SwrContext * swr_ctx = NULL;
AVFormatContext *formatContext;
AVCodecContext *codecContext;
AVCodec *codec;
AVFrame frame;

int audioIndex;
AudioParams audio_hw_params_target;
AudioParams audio_hw_params_src;
PacketQueue audio_queue;

int preparFFmpeg(const char* url);
void freeFFmpeg();
//打开设备并且获取相关的参数
int OpenHwAudioAndSetParams();
//处理SDL的回调函数
void audio_callback(void *userdata, Uint8 *stream, int len);

//队列的相关处理
void packet_queue_init(PacketQueue *q);
int packet_queue_put(PacketQueue *q, AVPacket *pkt);
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);

// static void sigterm_handler(int signum)
// {
// 	printf("Get signal: %d, Jump out ...", signum);
// 	exit(1);
// }

//真正的入口函数
int startPlayAudio(const char* url)
{
	//应该对入参进行判断
	if(access(url, F_OK) == -1)
	{
		printf("file not find \n");
		return -1;
	}

	// signal(SIGINT, sigterm_handler);
	// signal(SIGTERM, sigterm_handler);

	// //允许事件？
	// if (SDL_Init(SDL_INIT_AUDIO) < 0)
	// {
	// 	printf("init SDL error - %s\n", SDL_GetError());
	// 	return ;
	// }
	//准备ffmpeg
	int retcode = -1;
	retcode = preparFFmpeg(url);
	switch(retcode)
	{
		case -2:
			avcodec_close(codecContext);
		case -1:
			freeFFmpeg();
			return -1;
	}
	printf("ffmpeg sucess \n");
	//设置参数并打开设备
	if(OpenHwAudioAndSetParams() < 0)
	{
		printf("OpenHwAudioAndSetParams error \n ");
		avcodec_close(codecContext);
		freeFFmpeg();
		return -1;
	}
	//队列的初始化
	packet_queue_init(&audio_queue);
	//启动播放
	SDL_PauseAudio(0);
	AVPacket packet;
	while(av_read_frame(formatContext, &packet) >= 0)
	{
		if(packet.stream_index == audioIndex)
		{
			packet_queue_put(&audio_queue, &packet);
		}
		else
		{
			av_free_packet(&packet); //取出后播放完也会释放
		}
	}
	while(audio_queue.nb_packets > 0)
	{
		printf("start %d \n", audio_queue.nb_packets);
		SDL_Delay(1000);
	} 
	printf(" end of play \n");
	avcodec_close(codecContext);
	freeFFmpeg();
	SDL_CloseAudio();
    SDL_Quit();
	return 0;
}


//根据音频文件准备ffmpeg 使用新版的接口
int preparFFmpeg(const char* url)
{
	int retcode ; 
	 //初始化FormatContext
	av_register_all();

	formatContext = avformat_alloc_context();
	if(!formatContext){
		printf(" AudioController::preparFFmpeg avformat_alloc_context error \n");
		return -1;
	}

	//打开输入流
	retcode = avformat_open_input(&formatContext, url,  NULL, NULL);
	if(retcode != 0){
		printf(" AudioController::preparFFmpeg avformat_open_input error \n");
		return -1;
	}

	av_dump_format(formatContext, 0, url, 0);
	//读取媒体文件信息
    retcode = avformat_find_stream_info(formatContext, NULL);
    if (retcode != 0) {
        printf("AudioController::preparFFmpeg avformat_find_stream_info error \n");
        return -1;
    }
    //这里用新版的，可能有问题
    codecContext = avcodec_alloc_context3(NULL);
    if(!codecContext){
    	printf("AudioController::preparFFmpeg avcodec_alloc_context3 error \n");
        return -1;
    }

    //获取音频流的下标，把相关音频信息拷贝到m_pCodecCtx
    audioIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if(audioIndex < 0){
    	printf("AudioController::preparFFmpeg av_find_best_stream error [%d] \n", audioIndex);
    	return -1;
    }
     //将视频流的的编解码信息拷贝到codecContext中
    printf("get stream index is %d \n", audioIndex);
    retcode = avcodec_parameters_to_context(codecContext, formatContext->streams[audioIndex]->codecpar);
    if (retcode != 0) {
        printf("AudioController::preparFFmpeg avcodec_parameters_to_context error [%d]\n", retcode);
        return -1;
    }
    //查找解码器
    codec = avcodec_find_decoder(codecContext->codec_id);
    if(codec == NULL){
    	printf("AudioController::preparFFmpeg avcodec_find_decoder error \n");
        return -1;
    }
	//打开解码器
    retcode = avcodec_open2(codecContext, codec, NULL);
    if (retcode != 0) {
        printf("AudioController::preparFFmpeg avcodec_open2 error \n");
        return -2;
    }
    return 0;
}

void freeFFmpeg()
{
	//avcodec_open2 对应的释放先处理
	if (formatContext != NULL)
	{
		avformat_close_input(&formatContext);
	}
		
	if (codecContext != NULL)
	{
		avcodec_free_context(&codecContext);
	}
}

int OpenHwAudioAndSetParams()
{
	printf("OpenHwAudioAndSetParams start \n");
	int sample_rate;
	int nb_channels;
	int64_t channel_layout;
	sample_rate = codecContext->sample_rate;
	nb_channels = codecContext->channels;
	channel_layout = codecContext->channel_layout;
	printf("channel_layout=%" PRId64 "\n", channel_layout);
	printf("nb_channels=%d\n", nb_channels);
	printf("freq=%d\n", sample_rate);
	if(!channel_layout || nb_channels!= av_get_channel_layout_nb_channels(channel_layout))
	{
		channel_layout = av_get_default_channel_layout(nb_channels);
		channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
		printf("correction  channel_layout=%" PRId64 "\n", channel_layout);
	}
	//定义相关的参数   入参 返回实际的参数 
	SDL_AudioSpec wanted_spec, spec;  
	wanted_spec.freq = sample_rate;        //每个样本音频频率
	wanted_spec.format = AUDIO_S16SYS;     //音频格式
	wanted_spec.channels = nb_channels;    //声道
	wanted_spec.silence = 0;		       //用于将缓冲区设为静音的值
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE; //音频缓冲区期望大小，有一定的计算方式
	wanted_spec.callback = audio_callback;       //回调函数，应该加保护
	wanted_spec.userdata = codecContext;            //作为第一个参数传给回调
	//wanted_spec.size    = 1024;   //音频缓冲区大小，SDL_OpenAudio计算

	if(SDL_OpenAudio(&wanted_spec, &spec) < 0)
	{
		printf("AudioController::OpenHwAudio SDL_OpenAudio error \n");
		return -1;
	}

	//设置上下文需要的参数：
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
		printf("AudioController::OpenHwAudio set params size error. \n");
		return -1;
	}
	audio_hw_params_src = audio_hw_params_target;
	return 0;
}

void packet_queue_init(PacketQueue *q)
{
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = NULL;
	q->cond = NULL;
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
	AVPacketList *pkt1;
	//如果未分配内存 则分配该数据包
	if(av_dup_packet(pkt) < 0) 
	{
		return -1;
	}
	//定义队列中的变量  队列的下一个由list控制
	pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
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
	SDL_UnlockMutex(q->mutex); 
	return 0;
}

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;

	//加锁去取数据
	SDL_LockMutex(q->mutex);
	for(;;)
	{
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

int audio_decode_packet(AVCodecContext *aCodecCtx, uint8_t *audio_buf,unsigned int buf_size);
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
			//获取采样后的buffer进行播放 写入流stream中
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
	return ;
}
int resample(AVFrame * aframe, uint8_t * audio_buf,unsigned int * audio_buf_size);
int audio_decode_packet(AVCodecContext *aCodecCtx, uint8_t *audio_buf,unsigned int buf_size)
{
	static AVPacket pkt;
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;

	int len1, data_size = 0;

	for(;;)
	{
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

		if(packet_queue_get(&audio_queue, &pkt, 1) < 0)
		{
			return -1;
		}
		audio_pkt_size = pkt.size;
	}
	return 0;
}

//对解码后的frame进行重采样并返回采样后的buffer
int resample(AVFrame * aframe, uint8_t * audio_buf,unsigned int * audio_buf_size)
{
	int data_size = 0;
	int resampled_data_size = 0; //重采样数据的大小
	int64_t dec_channel_layout;

	//获取当前的buffer大小 这里frame相关的结构体和函数需要整理
	data_size = av_samples_get_buffer_size(NULL, av_frame_get_channels(aframe),
						aframe->nb_samples, (AVSampleFormat)aframe->format, 1);
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
			(AVSampleFormat)aframe->format,				//输入声音格式
			aframe->sample_rate,        //输入的采样率
			0, NULL);
		
		//保护判断
		if(!swr_ctx || swr_init(swr_ctx) < 0)
		{
			av_log(NULL, AV_LOG_ERROR,
                   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                    aframe->sample_rate, av_get_sample_fmt_name((AVSampleFormat)aframe->format), av_frame_get_channels(aframe),
                    audio_hw_params_target.freq, av_get_sample_fmt_name(audio_hw_params_target.fmt), audio_hw_params_target.channels);
            swr_free(&swr_ctx);
            return -1;
		}
		printf("swr init success. \n");
		//这里没有必要重新获取原包吧。。。
		audio_hw_params_src.channels = av_frame_get_channels(aframe);
		audio_hw_params_src.fmt = (AVSampleFormat)aframe->format;
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
        if(len2 == out_count) //每个通道可用于输出的空间量
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
}

//  using namespace AUDIO_PLAY;

// int main(int argc, char* argv[])
// {
// 	if(argc != 2)
// 	{
// 		printf("usage ./play <filename> \n");
// 		return -1;
// 	}
// 	startPlayAudio(argv[1]);
// 	return 0;
// }