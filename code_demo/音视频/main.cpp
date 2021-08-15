#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mp3_aac_alsa_controller.h"
#include "wav_controller.h"

//需要修改makefile，把相关的ffmpeg和sdl相关的头文件拷贝过来
//mp3/aac有两个入口，分别是直接用alsa和sdl进行播放，
typedef enum
{
	AUDIO_TYPE_WAC,
	AUDIO_TYPE_AAC_OR_MP3,
	NOT_AUDIO_FILE
}AUDIO_FILE_TYPE;

//文件判断
int is_audio_file(const char* str1, const char* str2);
AUDIO_FILE_TYPE get_audio_file_type(const char* str1);
//wac 播放测试
using namespace AUDIO_PLAY;
int play_wac_file_by_alsa(const char* file)
{
	//传入文件名，打开文件，提取文件中信息
	WAVController wac_controller(file);
	//根据fd，解析验证头信息
	if(wac_controller.checkAndGetWAVFile() < 0)
	{
		printf("\n\n****check file error**** \n\n");
		return -1;
	}
	//打开设备,设置日志，设置参数，开始播放，触发日志
	if(wac_controller.startPlayWavfile() < 0)
	{
		printf("\n\n****play file error**** \n\n");
		return -1;
	}
}

void AudioController()
{
	char file_name[160];
	printf("please input audio filename:\n");
	scanf("%s", file_name);
	AUDIO_FILE_TYPE audio_type;
	while((audio_type = get_audio_file_type(file_name)) ==  NOT_AUDIO_FILE)
	{
		printf("Please enter aac, mp3.wav format file. please input:\n");
		scanf("%s", file_name);
		return;
	}

	if(audio_type == AUDIO_TYPE_WAC)
	{
		printf("this is wav file.\n");
		play_wac_file_by_alsa(file_name);
	}
	
	if(audio_type == AUDIO_TYPE_AAC_OR_MP3)
	{
		printf("this is mp3 or aac file. \n");
		// play_mp3_or_aac_file_by_sdl(file_name);
		StartPlayAudioByAlsa(file_name);
	}

}


int is_audio_file(const char* str1, const char* str2)
{
	if(str1 == NULL || str2 == NULL)
		return -1;
	
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	if(len1 <= len2 || len1 == 0 || len2 == 0)
	{
		return -1;
	}

	for(int i= 1; i <= len2; i++)
	{
		if( str1[len1 - i] != str2[ len2 -i])
		{
			return -1;
		}
	}
	return 0;
}
//获取输入文件的类型
AUDIO_FILE_TYPE get_audio_file_type(const char* str1)
{
	AUDIO_FILE_TYPE audio_type = NOT_AUDIO_FILE;
	if(str1 == NULL || strlen(str1) <= 4)
	{
		audio_type = NOT_AUDIO_FILE;
		return audio_type;
	}
	if(!is_audio_file(str1, ".wav"))
	{
		printf("this is wav file. \n");
		audio_type = AUDIO_TYPE_WAC;
	}
	if(!is_audio_file(str1, ".aac") || !is_audio_file(str1, ".mp3"))
	{
		printf("this is aac or mp3 file. \n");
		audio_type = AUDIO_TYPE_AAC_OR_MP3;
	}
	return audio_type;
}

int main()
{
	AudioController();
	return 0;
}