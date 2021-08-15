/********************************************
对wav音频格式的头文件进行封装，
	校验wav格式的头文件是否正确
	打印相关的文件信息
*********************************************/

#ifndef PLAY_WAV_PARSER_H__
#define PLAY_WAV_PARSER_H__

#include <iostream>
#include "wav_header.h"
#include <alsa/asoundlib.h>

//定义相关的头
namespace AUDIO_PLAY{
	// struct snd_pcm_format_t;
	class WAVParser
	{
	public:
		WAVParser();
		~WAVParser();
	public:
		//相关处理函数 写入头 读头 校验头信息   打印头信息
		int wavReadHeader(int fd); //从fd中读相关头信息
		int wavWriteHeader(int fd);//

		//提供一些获取的api
		int getWavFormat(snd_pcm_format_t * format);
		int getWavFormatChannels(int *channels);
		int getWavSampleRate(uint32_t *rate);
		int getWavDataLen(uint32_t *len);
	private:
		int checkWAVHeader();
		const char * wavFmtToString(uint16_t fmt);
		void PrintWAVHeader();
	private:
		AUDIO_PLAY::WAVContainer_t m_wav_container;
	};
}
#endif //PLAY_WAV_PARSER_H__
