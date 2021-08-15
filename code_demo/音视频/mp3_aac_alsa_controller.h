#ifndef PLAY_MP3_AAC_ALSA_CONTROLLER_NOT_CLASS_H__
#define PLAY_MP3_AAC_ALSA_CONTROLLER_NOT_CLASS_H__

#include <stdio.h>
#include <signal.h>   //处理ctrl+c和相关异常终止的情况
#include <assert.h>
#include <alsa/asoundlib.h>
extern "C"
{
	#include <libavutil/time.h>
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavdevice/avdevice.h>
	#include <libswresample/swresample.h>
	#include <libswscale/swscale.h>
}




int StartPlayAudioByAlsa(const char* fileName);

#endif //PLAY_MP3_AAC_ALSA_CONTROLLER_NOT_CLASS_H__