#ifndef PLAY_WAV_CONTROLLER_H__
#define PLAY_WAV_CONTROLLER_H__

#include "wav_parser.h"
//定义播放上下文需要的一些信息
namespace AUDIO_PLAY{
	typedef long long off64_t;
	typedef struct SndPcmContainer{
		snd_pcm_t *handle; //打开音频设备的结构体
		snd_output_t *log; //重定向输出的结构体
		snd_pcm_uframes_t chunk_size; //unsigned long
		snd_pcm_uframes_t buffer_size;
		snd_pcm_format_t format; //PCM的采样格式

		uint16_t channels;
		size_t chunk_bytes;
		size_t bits_per_sample;
		size_t bits_per_frame;
	 
		uint8_t *data_buf;
	}SndPcmContainer_t;

	//对wav格式的音频进行解析+播放控制
	class WAVController{
	public:
		WAVController(const char* filename);
		~WAVController();
	public:
		//根据fd校验头文件
		int checkAndGetWAVFile();
		int startPlayWavfile();
		//获取到相关的上下文结构体信息
		int setDeviceParams();
	private:
		void reallyPlayWavfile();
		int ReadToBuffer(int offset, int remain);
		int WriteToPcm(int chunk_size);

	private:
		// const char* m_filename ;
		int fd;
		const char* m_devicename = "default";

		WAVParser m_wav_parser; //校验以及提取相关有用的信息
		SndPcmContainer_t m_pcm_container;
	};
}

#endif