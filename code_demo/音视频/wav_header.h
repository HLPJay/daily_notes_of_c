#ifndef PLAY_WAV_HEADER_H__
#define PLAY_WAV_HEADER_H__

namespace AUDIO_PLAY{
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	typedef unsigned int   uint32_t;

	//WAV总共由三部分头组成
	//文件标识
	typedef struct WAVHeader{
		uint32_t magic;		/* 'RIFF' */
		uint32_t length;	/* filelen */
		uint32_t type;		/* 'WAVE' */
	}WAVHeader_t;

	//采样率，通道数，码率等参数
	typedef struct WAVFmt{
		uint32_t magic; 		 /* 'FMT '*/ //波形格式
		uint32_t fmt_size; 		 /* 16 or 18 */  //过滤字节
		uint16_t format;		 /* see WAV_FMT_* */ //字节格式种类
		uint16_t channels;       //通道数
		uint32_t sample_rate;	 /* frequence of sample */ //采样率
		uint32_t bytes_p_second; //比特率Byte率=采样频率*音频通道数*每次采样得到的样本位数/8
		uint16_t blocks_align;	 /* samplesize; 1 or 2 bytes */ //2字节数据块长度(每个样本的字节数=通道数*每次采样得到的样本位数/8
		uint16_t sample_length;	 /* 8, 12 or 16 bit */ //2字节每个采样点的位数
	}WAVFmt_t;
	//数据和长度
	typedef struct WAVChunkHeader{
		uint32_t type;			/* 'data' */
		uint32_t length;		/* samplecount */
	}WAVChunkHeader_t;
	//构造
	typedef struct WAVContainer
	{
		WAVHeader_t header;
		WAVFmt_t format;
		WAVChunkHeader_t data;
	}WAVContainer_t;

	//大端字节序和小端字节序的处理 bswap_16 16位大小端转换
	#if __BYTE_ORDER == __LITTLE_ENDIAN
		#define COMPOSE_ID(a,b,c,d)	  ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
		#define LE_SHORT(v)		      (v)
		#define LE_INT(v)		      (v)
		#define BE_SHORT(v)		      bswap_16(v)
		#define BE_INT(v)		      bswap_32(v)
	#elif __BYTE_ORDER == __BIG_ENDIAN
		#define COMPOSE_ID(a,b,c,d)	 ((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
		#define LE_SHORT(v)		      bswap_16(v)
		#define LE_INT(v)		      bswap_32(v)
		#define BE_SHORT(v)		      (v)
		#define BE_INT(v)		      (v)
	#else
		#error "Wrong endian ."
	#endif

	//定义校验wav头的相关信息并做大小端处理
	#define WAV_RIFF		COMPOSE_ID('R','I','F','F')
	#define WAV_WAVE		COMPOSE_ID('W','A','V','E')
	#define WAV_FMT			COMPOSE_ID('f','m','t',' ')
	#define WAV_DATA		COMPOSE_ID('d','a','t','a')

	#define WAV_FMT_PCM             0x0001
	#define WAV_FMT_IEEE_FLOAT      0x0003
	#define WAV_FMT_DOLBY_AC3_SPDIF 0x0092
	#define WAV_FMT_EXTENSIBLE      0xfffe
}

#endif //PLAY_WAV_HEADER_H__