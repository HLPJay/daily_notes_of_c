//实现用sdl对音频数据的播放, 播放pcm，wac
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#define BLOCK_SIZE 4096000
static Uint8 *input_buf = NULL;
static Uint8 *output_buf = NULL;
static size_t buffer_len = 0;

//SDL回调函数，真正的写入声卡并播放
void read_and_play_callback(void * data, Uint8 * stream, int len);

int main(int argc, char* argv[])
{
	//对输入进行校验
	if(argc != 2)
	{
		printf("use ./play pcmFileNname\n");
		exit(1);
	}
	//SDL进行初始化
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)<0)
	{
		printf("init SDL error \n");
		return -1;
	}

	//获取输入文件的指针，方便后面读文件
	FILE * pcm_fp = NULL;
	pcm_fp = fopen(argv[1], "rb");
	if(pcm_fp == NULL)
	{
		printf("open pcm file error. \n");
		exit(1);
	}
	/***************************
	对SDL_OpenAudio函数中音频参数进行进行定义及设置
	typedef struct SDL_AudioSpec
	{
	    int freq;                   < DSP frequency -- samples per second 
	    SDL_AudioFormat format;     < Audio data format 
	    Uint8 channels;             < Number of channels: 1 mono, 2 stereo 
	    Uint8 silence;              < Audio buffer silence value (calculated) 
	    Uint16 samples;             < Audio buffer size in sample FRAMES (total samples divided by channel count) 
	    Uint16 padding;             < Necessary for some compile environments 
	    Uint32 size;                < Audio buffer size in bytes (calculated) 
	    SDL_AudioCallback callback; < Callback that feeds the audio device (NULL to use SDL_QueueAudio()). 
	    void *userdata;             < Userdata passed to callback (ignored for NULL callbacks). 
	} SDL_AudioSpec;
	****************************/
	SDL_AudioSpec spec;//这里直接定义，因为是pcm的，其他的应该去获取。
	spec.freq    = 44100; //采样率
	spec.format  = AUDIO_S16SYS;
	spec.channels= 2;     //声道
	spec.silence = 0;
	//spec.padding =
	spec.size    = 1024;
	spec.callback= read_and_play_callback;
	spec.userdata= NULL;

	//打开音频设备
	if(SDL_OpenAudio(&spec, NULL) < 0)
	{
		printf("failed to open audio device: %s \n",SDL_GetError());
		goto __FAIL;
	}
	//先设置暂停播放 1是循环播放
	SDL_PauseAudio(0);
	input_buf = (char *) malloc(BLOCK_SIZE);
	//采用循环的方式，从打开文件中读buffer
	//因为这里回调函数去写入声卡，所以相关参数要传递
	do{
		//memset(input_buf, 0, BLOCK_SIZE);
		buffer_len = fread(input_buf, 1, BLOCK_SIZE, pcm_fp);
		printf("read from pcm file size is %zu \n", buffer_len);

		output_buf = input_buf;

		while(output_buf <(input_buf + buffer_len))
		{
			//暂停等待时间
			SDL_Delay(1);
		}
	}while(buffer_len != 0);
	//关闭音频播放
	SDL_CloseAudio();
	//失败场景下，需要释放资源
__FAIL:
	if(input_buf) //没必要吧？
	{
		free(input_buf);
	}

	if(pcm_fp)
	{
		fclose(pcm_fp);
	}

	//关闭SDL
	SDL_Quit();
	return 0;
}

//len是传入的固定长度？ 1024吗
void read_and_play_callback(void * data, Uint8 * stream, int len)
{
	//buffer_len 是读取到的数据
	if(buffer_len == 0) return;
	SDL_memset(stream, 0, len);
	len = (len <buffer_len)? len: buffer_len;
	//给声卡传数据 SDL_MIX_MAXVOLUME = 128 (最大音量 0~128)
	SDL_MixAudio(stream, output_buf, len, SDL_MIX_MAXVOLUME);

	//移动指针和改变未读的长度
	output_buf += len;
	buffer_len -= len;
}
