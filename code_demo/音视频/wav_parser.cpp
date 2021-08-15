#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> 

#include "wav_parser.h"

namespace AUDIO_PLAY{
	//初始化的时候其实啥也不用做
	WAVParser::WAVParser()
	{}

	WAVParser::~WAVParser()
	{}

	//从fd中读取相关的头信息
	int WAVParser::wavReadHeader(int fd)
	{
		assert((fd>=0));
		if(read(fd, &(m_wav_container.header), sizeof(m_wav_container.header)) != sizeof(m_wav_container.header))
		{
			printf("wav read header failed. \n");
			return -1;
		}

		if(read(fd, &(m_wav_container.format), sizeof(m_wav_container.format)) != sizeof(m_wav_container.format))
		{
			printf("wav read format failed. \n");
			return -1;
		}

		if(read(fd, &(m_wav_container.data), sizeof(m_wav_container.data)) != sizeof(m_wav_container.data))
		{
			printf("wav  read data failed. \n");
			return -1;
		}

		PrintWAVHeader();
		if(checkWAVHeader() < 0)
		{
			printf("check wav header failed \n");
			return -1;
		}
		return 0;
	}

	//把pcm转为wav格式会用到
	int WAVParser::wavWriteHeader(int fd)
	{
		return 0;
	}

	//校验wav的头是否符合条件
	int WAVParser::checkWAVHeader()
	{
		int ret = 0;
		if(m_wav_container.header.magic != WAV_RIFF)
		{
			printf(" check WAV header magic error. [\"RIFF\"]  \n");
			ret = -1;
		}
		if(m_wav_container.header.type != WAV_WAVE)
		{
			printf("check WAV header type error.  [\"WAVE\"]   \n");
			ret = -1;
		}
		if(m_wav_container.format.magic != WAV_FMT)
		{
			printf("check WAV header format magic error. [\"FMT \"]   \n");
			ret = -1;
		}
		if(m_wav_container.format.fmt_size != LE_INT(16) &&
			m_wav_container.format.fmt_size != LE_INT(18))
		{
			printf("check WAV header format fmt_size error. [16 or 18] \n");
			ret = -1;
		}
		if(m_wav_container.format.channels != LE_SHORT(1) &&
			m_wav_container.format.channels != LE_SHORT(2))
		{
			printf("check WAV header format channels error. [ 1 or 2]\n");
			ret = -1;
		}
		if(m_wav_container.data.type != WAV_DATA)
		{
			printf("check WAV header data type error. \n");
		}
		printf("check WAV header success. \n");
		return ret;
	}
	const char * WAVParser::wavFmtToString(uint16_t fmt)
	{
		switch (fmt) {
			case WAV_FMT_PCM:
				return "PCM";
				break;
			case WAV_FMT_IEEE_FLOAT:
				return "IEEE FLOAT";
				break;
			case WAV_FMT_DOLBY_AC3_SPDIF:
				return "DOLBY AC3 SPDIF";
				break;
			case WAV_FMT_EXTENSIBLE:
				return "EXTENSIBLE";
				break;
			default:
				break;
		}
	 
		return "NON Support Fmt";
	}

	void WAVParser::PrintWAVHeader()
	{
		printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		printf("\n");

		printf("File Magic:         [%c%c%c%c]\n", 
				(char)(m_wav_container.header.magic), 
				(char)(m_wav_container.header.magic>>8), 
				(char)(m_wav_container.header.magic>>16), 
				(char)(m_wav_container.header.magic>>24));
		printf("File Length:        [%d]\n", m_wav_container.header.length);
		printf("File Type:          [%c%c%c%c]\n",
				(char)(m_wav_container.header.type), 
				(char)(m_wav_container.header.type>>8), 
				(char)(m_wav_container.header.type>>16), 
				(char)(m_wav_container.header.type>>24));
		printf("\n");
	 
		printf("Fmt Magic:          [%c%c%c%c]\n",
				(char)(m_wav_container.format.magic), 
				(char)(m_wav_container.format.magic>>8), 
				(char)(m_wav_container.format.magic>>16), 
				(char)(m_wav_container.format.magic>>24));
		printf("Fmt Size:           [%d]\n", m_wav_container.format.fmt_size);
		printf("Fmt Format:         [%s]\n", wavFmtToString(m_wav_container.format.format));
		printf("Fmt Channels:       [%d]\n", m_wav_container.format.channels);
		printf("Fmt Sample_rate:    [%d](HZ)\n", m_wav_container.format.sample_rate);
		printf("Fmt Bytes_p_second: [%d]\n", m_wav_container.format.bytes_p_second);
		printf("Fmt Blocks_align:   [%d]\n", m_wav_container.format.blocks_align);
		printf("Fmt Sample_length:  [%d]\n", m_wav_container.format.sample_length);
		printf("\n");
	 
		printf("Chunk Type:         [%c%c%c%c]\n",
				(char)(m_wav_container.data.type), 
				(char)(m_wav_container.data.type>>8), 
				(char)(m_wav_container.data.type>>16), 
				(char)(m_wav_container.data.type>>24));
		printf("Chunk Length:       [%d]\n", m_wav_container.data.length);
		
		printf("\n");
		printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		return;
	}


	int WAVParser::getWavFormat(snd_pcm_format_t * format)
	{
		if (LE_SHORT(m_wav_container.format.format) != WAV_FMT_PCM)
			return -1;
		
		switch (LE_SHORT(m_wav_container.format.sample_length)) {
			case 16:
				*format = SND_PCM_FORMAT_S16_LE;
				break;
			case 8:
				*format = SND_PCM_FORMAT_U8;
				break;
			default:
				*format = SND_PCM_FORMAT_UNKNOWN;
				break;
		}
	 
		return 0;
	}

	int WAVParser::getWavFormatChannels(int *channels)
	{
		*channels = LE_SHORT(m_wav_container.format.channels);
		return 0;
	}

	int WAVParser::getWavSampleRate(uint32_t *rate)
	{
		*rate = LE_INT(m_wav_container.format.sample_rate);
		return 0;
	}

	int WAVParser::getWavDataLen(uint32_t *len)
	{
		*len = LE_INT(m_wav_container.data.length);
		return 0;
	}
}


// using namespace AUDIO_PLAY;
// int main(int argc, char* argv[])
// {
// 	if(argc != 2)
// 	{
// 		printf("usage ./play <filename> \n");
// 		return -1;
// 	}
// 	const char* filename = argv[1];
// 	int fd = open(filename, O_RDONLY);
// 	if(fd < 0)
// 	{
// 		printf("open file failed. \n");
// 		return -1;
// 	}

// 	WAVParser wav_parser;
// 	wav_parser.wavReadHeader(fd);
// 	close(fd);
// 	return 0;
// }