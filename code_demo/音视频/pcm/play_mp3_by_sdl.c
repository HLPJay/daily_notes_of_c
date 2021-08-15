//实现sdl播放MP3的功能，
//基本方案和实现队列的功能
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//ffmpeg使用时要定义该宏 C++C99相关的
#define __STDC_CONSTANT_MACROS

//ffmpeg相关的头文件
#include <libavcodec/avcodec.h>  //编解码相关
#include <libavformat/avformat.h> //device相关的AVInputForamt和AVOutputFormat
#include <libswresample/swresample.h> //重采样相关
#include <SDL2/SDL.h>


#define MAX_AUDIO_FRAME_SIZE 192000 //48000 *(32/4) 每秒48Khz 32bit的音频

unsigned int audio_play_len = 0;
unsigned char * audio_chunk = NULL;
unsigned char * audio_pos = NULL;

/**************************************
AVIOContext ：输入输出对应的结构体，用于输入输出（读写文件，RTMP协议等）。
AVFormatContext ：统领全局的基本结构体。主要用于处理封装格式（FLV/MKV/RMVB等）。
AVStream, AVCodecContext ：视音频流对应的结构体，用于视音频编解码。
AVFrame ：存储非压缩的数据（视频对应RGB/YUV像素数据，音频对应PCM采样数据）
AVPacket ：存储压缩数据（视频对应H.264等码流数据，音频对应AAC/MP3等码流数据）
***************************************/
void sdl_audio_callback(void *udata, Uint8 *stream, int len);
int main(int argc, char * argv[])
{

	if(argc != 2)
	{
		printf("use ./play mp3FileNname\n");
		exit(1);
	}
	const char* inFile = argv[1];
	//定义相关需要的结构体
	AVFormatContext * pFortCtx = NULL; //基础结构体
	AVCodecContext * pCodecCtx = NULL; //一般是编码时存数据
	AVCodec * pCodex = NULL;           //存储编解码信息
	AVPacket * pPkt = NULL; 		   //存储压缩编码数据的信息
	AVFrame * pFrame = NULL;           //存储原始数据， 未压缩的数据信息 ==》PCM
	
	struct SwrContext * pSwrCtx = NULL; //存储重采样相关信息
	SDL_AudioSpec spec;     //sdl相关函数参数信息

	int ret = 0; //对返回值进行处理
	//首先解码，从文件中获取到相关播放需要的参数
	//ffmpeg解码相关过程
	//注册相关编解码信息    这是以前的接口
	av_register_all();
	pFortCtx = avformat_alloc_context(); //基础结构体分配内存
	//打开输入的音频流
	if((avformat_open_input(&pFortCtx, inFile, NULL, NULL))<0)
	{
		printf("avformat_open_input open inputFile failed. \n");
		ret = -1;
		goto ret1;
	}
	//获取相关的流
	if(avformat_find_stream_info(pFortCtx, NULL) < 0)
	{
		printf("avformat_find_stream_info find stream from inputFileFd error. \n");
		ret = -1;
		goto ret2;
	}
	int audoId = 0;
	//查找我们需要的对应的流，并根据流的类型返回解码器的编号，根据编号找解码器
	if((audoId = av_find_best_stream(pFortCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0))<0)
	{
		printf("av_find_best_stream find stream from inputFile error. \n");
		ret = -1;
		goto ret2;
	}
	//根据编号找到对应的解码器
	pCodex = avcodec_find_decoder(pFortCtx->streams[audoId]->codecpar->codec_id);
	if(pCodex == NULL)
	{
		printf("avcodec_find_decoder find decodec error. \n");
		ret = -1;
		goto ret2;
	}
	//存储编解码的结构体,初始化
	// pCodecCtx = avcodec_alloc_context3(pCodex);
	// if(pCodecCtx == NULL)
	// {
	// 	printf("avcodec_alloc_context3 init codec_ctx by codec error \n");
	// 	ret = -1;
	// 	goto ret2;
	// }
	pCodecCtx = pFortCtx->streams[audoId]->codec;
	//打开对应的编码器
	if(avcodec_open2(pCodecCtx, pCodex, NULL) < 0)
	{
		printf("avcodec_open2 open codec error. \n");
		ret = -1;
		goto ret3;
	}

	//编码和为编码的结构体的初始化：
	//为存储编码的信息申请内存
	pPkt = av_packet_alloc();
	if(pPkt == NULL)
	{
		printf("av_packet_alloc init package error \n");
		ret = -1;
		goto ret4;
	}
	//存储未编码的信息，并申请空间
	pFrame = av_frame_alloc();
	if(pFrame == NULL)
	{
		printf("av_frame_alloc init frame error. \n");
		ret = -1;
		goto ret5;
	}
	//采样率
	int out_sample_rate = 44100;
	uint64_t out_chn_layout = AV_CH_LAYOUT_STEREO; //通道布局，双声道
	int out_channels = av_get_channel_layout_nb_channels(out_chn_layout);
	int out_nb_samples = pCodecCtx->frame_size;
	//获取音频占用的字节数 
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16; //声音格式
	int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
	//重采样 中间buffer参数，全局的一个buffer供播放时使用
	unsigned char *outBuff = NULL;
	outBuff = (unsigned char *)av_malloc(MAX_AUDIO_FRAME_SIZE *2);// 每秒48Khz  32位音频 （48000*（32/4）） 双声道
	//获取当前的通道布局
	int in_chn_layout = av_get_default_channel_layout(pCodecCtx->channels);

	//设置视频播放需要的相关参数
	spec.freq    = out_sample_rate; //采样率
	spec.format  = AUDIO_S16SYS;
	spec.channels= out_channels;     //声道
	spec.silence = 0;
	spec.samples = out_nb_samples; //样本帧中音频缓冲区大小（样本总数除以通道数）
	//spec.padding =
	//spec.size    = 1024;
	//回调函数和回调函数对应的入参
	spec.callback= sdl_audio_callback;
	spec.userdata= NULL; //传入pCodecCtx 没啥用
	//打开音频设备
	if(SDL_OpenAudio(&spec, NULL) < 0)
	{
		printf("failed to open audio device: %s \n",SDL_GetError());
		ret = -1;
		goto ret7;
	}


	//重采样相关的设置   重采样初始化有两种方式
/*******************************************************
swr_alloc() ：创建SwrContext对象。
av_opt_set_*() ： 设置输入和输出音频的信息。
swr_init() ： 初始化SwrContext。
av_samples_alloc_array_and_samples ：根据音频格式分配相应大小的内存空间。
av_samples_alloc ：根据音频格式分配相应大小的内存空间。用于转换过程中对输出内存大小进行调整。
swr_convert ：进行重采样转换。
********************************************************/

	// //Swr创建并设置相关参数 把解码后的数据重采样成和元数据一致的格式
	// pSwrCtx = swr_alloc_set_opts(NULL, //如果为NULL则创建一个新的SwrContext，否则对已有的SwrContext进行参数设置
	// 		out_chn_layout, //输出的声道格式
	// 		out_sample_fmt, //输出声音格式
	// 		out_sample_rate,  //输出采样率
	// 		in_chn_layout,    //输入的声道格式
	// 		pCodecCtx->sample_fmt,  //输入的声音格式
	// 		pCodecCtx->sample_rate, //输入的采样率
	// 		0,
	// 		NULL
	// 	);
	// //应该是把解码后的数据重采样成和元数据一致的格式
	
	// //Swr的初始化
	// swr_init(pSwrCtx);
	//重采样获取到数据，同时使用SDL进行播放
	SDL_PauseAudio(0);
	//通过基础结构体，读取到数据
	int get_result = 0;
	while(av_read_frame(pFortCtx, pPkt) == 0) //正确读取
	{
		//取到对应的格式
		if(audoId == pPkt->stream_index)
		{
			//保存解码后的PCM数据
			if(ret = avcodec_decode_audio4(pCodecCtx, pFrame, &get_result, pPkt)<0)
			{
				printf(" avcodec_decode_audio4 decode audo to pcm error %d.\n",ret);
				ret = -1;
				break;
			}

			if(pSwrCtx == NULL)
			{
				swr_free(&pSwrCtx);
				//重采样设置应该加在这里  解码后进行重采样，对应的入参和出参
				pSwrCtx = swr_alloc_set_opts(NULL, //如果为NULL则创建一个新的SwrContext，否则对已有的SwrContext进行参数设置
					pCodecCtx->channel_layout, //输出的声道格式
					pCodecCtx->sample_fmt, //输出声音格式
					pCodecCtx->sample_rate,  //输出采样率
					pFrame->channel_layout,    //输入的声道格式
					pFrame->format,				//输入声音格式
					pFrame->sample_rate,        //输入的采样率
					0, NULL);
				if(pSwrCtx == NULL || swr_init(pSwrCtx) < 0)
				{
					printf("swr init error. \n");
					break;
				}
			}

			//如果获取到数据，则进行重采样
			if(get_result > 0)
			{
				//根据获取到的PCM进行重采样，采样数据保存在outBuff 返回每个通道输出的样本数
				int result = 0; //解码后的实际采样数
				if((result = swr_convert(pSwrCtx, &outBuff, MAX_AUDIO_FRAME_SIZE,(const uint8_t **)pFrame->data , pFrame->nb_samples))<0)
				{
					printf(" swr_convert error %d. \n",result);
					// ret = -1;
					// break;
				}
				while(audio_play_len > 0) 
				{
					SDL_Delay(1);
				}
				//转码后的长度 根据计算buffer的长度 计算buffer的长度
				//audio_play_len = av_samples_get_buffer_size( NULL, out_channels, result, out_sample_fmt, 1);
				//对获取到的buffer进行处理，相关sdl播放的流程
				//如果没有播放完，就让sdl进行等待
				//获取到转码后的数据
				audio_chunk = (unsigned char *)outBuff;
				//实际处理的转码块的数据
				audio_pos = audio_chunk;
				audio_play_len = out_buffer_size;
				
				
//dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,result, dst_sample_fmt, 1);
				
			}
		}
		av_free_packet(pPkt);
	}
	SDL_CloseAudio();
    SDL_Quit();

	av_free_packet(pPkt);
	swr_free(&pSwrCtx);
ret7:
	av_free(outBuff);
ret6:
	av_frame_free(&pFrame);
ret5:
	av_packet_free(&pPkt);
ret4:
	avcodec_close(pCodecCtx);
ret3:
	avcodec_free_context(&pCodecCtx);
ret2:
	avformat_close_input(&pFortCtx);
ret1:
	avformat_free_context(pFortCtx);
	return ret;
}

//SDL播放的回调函数
void sdl_audio_callback(void *udata, Uint8 *stream, int len)
{
	SDL_memset(stream, 0, len);
	printf("buffer len is %u, %d\n", audio_play_len, len);
	if(audio_play_len == 0)
		return ;
	len = (len > audio_play_len? audio_play_len : len);
	//给声卡传数据进行播放
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_play_len -= len;
}