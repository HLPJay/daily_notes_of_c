//因为涉及回调函数相关，有关设计优化待定
#ifndef PLAY_MP3_AAC_CONTROLLER_NOT_CLASS_H__
#define PLAY_MP3_AAC_CONTROLLER_NOT_CLASS_H__


#include <stdio.h>
#include <signal.h>   //处理ctrl+c和相关异常终止的情况
#include <assert.h>
extern "C"
{
	#include <libavcodec/avcodec.h>       //各种类型声音/图像编解码
	#include <libavformat/avformat.h>     //各种音视频封装格式的生成和解析，解码需要的相关信息以及上下文结构
	#include <libswresample/swresample.h> //重采样
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_thread.h>
}

namespace AUDIO_PLAY{
	
typedef struct AudioParams {
    int freq;            //采样率
    int channels;		 //声道
    int64_t channel_layout;  //声道布局
    enum AVSampleFormat fmt; //音频格式
    int frame_size;			 //一帧的大小
    int bytes_per_sec;       //一个采样里的大小
} AudioParams;

typedef struct PacketQueue{
	AVPacketList * first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
}PacketQueue;

	
	//提供一个真正的入口函数
int startPlayAudio(const char* url);
}
#endif //PLAY_MP3_AAC_CONTROLLER_NOT_CLASS_H__