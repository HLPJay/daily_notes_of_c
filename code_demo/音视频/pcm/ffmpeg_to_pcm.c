//ffmpeg练习，实现将（mp3/aac）转为pcm格式, 但是不能实现WAV的转换
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//ffmpeg相关头文件
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h> //一些相关接口
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>

static void decode(AVCodecContext *codec_ctx, AVFrame *frame, AVPacket * package, FILE *fp_out);
void decode_audio(const char* inputFile, const char* outputFile);
int main(int argc, char * argv[])
{
	if(argc < 3)
	{
		printf("use: ./play <input file(aac/mp3)> <output file(pcm)>\n");
		exit(0);
	}
	decode_audio(argv[1], argv[2]);
	return 0;
}
/**********************************************************
对输入文件进行解码，写入到输出文件中
avformat_open_input()：打开输入的音频流
avformat_find_stream_info()：查找流的信息，填充AVFormatContext
av_find_best_stream()：找到输入流中想要的流，返回流的编号
avcodec_find_decoder()：通过解码器ID查找解码器
avcodec_alloc_context3()：初始化AVCodecContext
avcodec_open2()：打开解码器
av_packet_alloc()：初始化AVPacket
av_frame_alloc()：初始化AVFrame
av_read_frame()：从流中读取一个AVPacket包数据
avcodec_send_packet()：向解码器发生一个AVPacket包
av_get_bytes_per_sample()：获取一个采样的字节数
avcodec_receive_frame()：从解码器中读取一帧音频数据
************************************************************/
void decode_audio(const char* inputFile, const char* outputFile)
{
	//定义相关的结构体
	AVFormatContext * fmt_ctx = NULL; //基础结构体

	//打开输入的音频流
	if((avformat_open_input(&fmt_ctx, inputFile, NULL, NULL))<0)
	{
		printf("avformat_open_input open inputFile failed. \n");
		goto ret1;
	}

	//查找流的信息，给基础结构体中填值
	if(avformat_find_stream_info( fmt_ctx, NULL) < 0)
	{
		printf("avformat_find_stream_info find stream from inputFileFd error. \n");
		goto ret2;
	}
	int ret = 0;
	//根据输入，查找输入文件中的流，返回编号 这里查找音频对应的流AVMEDIA_TYPE_AUDIO
	if((ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0))<0)
	{
		printf("av_find_best_stream find stream from inputFile error. \n");
		goto ret2;
	}

	//根据上文找到的流编号，根据对应的解码id找到对应的解码器
	AVCodec * codec = NULL;
	codec = avcodec_find_decoder(fmt_ctx->streams[ret]->codecpar->codec_id);
	if(codec == NULL)
	{
		printf("avcodec_find_decoder find decodec error. \n");
		goto ret2;
	}
	//定义并初始化编码信息，根据上文的解码器
	AVCodecContext * codec_ctx = NULL;
	codec_ctx = avcodec_alloc_context3(codec);
	if(codec_ctx == NULL)
	{
		printf("avcodec_alloc_context3 init codec_ctx by codec error \n");
		goto ret2;
	}
	//打开对应的编码器
	if(avcodec_open2(codec_ctx, codec, NULL) < 0)
	{
		printf("avcodec_open2 open codec error. \n");
		goto ret3;
	}
	//为存储编码的信息申请内存
	AVPacket * pPkg = NULL;
	pPkg = av_packet_alloc();
	if(pPkg == NULL)
	{
		printf("av_packet_alloc init package error \n");
		goto ret4;
	}
	//存储未编码的信息，并申请空间
	AVFrame* frame = NULL;
	frame = av_frame_alloc();
	if(frame == NULL)
	{
		printf("av_frame_alloc init frame error. \n");
		goto ret5;
	}

	//打开输出的文件
	FILE *fp_out;
	fp_out = fopen(outputFile, "wb");//以写的方式
	if(fp_out == NULL)
	{
		printf("fopen outputFile error. \n");
		goto ret6;
	}
	//读取输入中的流 这里是解码 读一个package包, 进行解码处理
	while((ret = av_read_frame(fmt_ctx, pPkg)) == 0)
	{
		//如果读到数据，则进行解码
		if(pPkg->size > 0)
		{
			printf("get package size is %d \n", pPkg->size);
			decode(codec_ctx, frame, pPkg, fp_out); //解码器，未编码文件，编码文件，和输出文件的fd
		}
	}

	decode(codec_ctx, frame, NULL, fp_out); //结束标志

//一一对应释放相关的资源：
	fclose(fp_out);
	av_frame_free(&frame);
	av_packet_free(&pPkg);
	avcodec_close(codec_ctx);
	avcodec_free_context(&codec_ctx);
	avformat_close_input(&fmt_ctx);
	return ;
ret6:
	av_frame_free(&frame);
ret5:
	av_packet_free(&pPkg);
ret4:
	avcodec_close(codec_ctx);
ret3:
	avcodec_free_context(&codec_ctx);
ret2:
	avformat_close_input(&fmt_ctx);
ret1:
	exit(1);

}

//解码信息，为编码的包，编码的包，输出文件
/**********************************************************
avcodec_send_packet()：向解码器发生一个AVPacket包
av_get_bytes_per_sample()：获取一个采样的字节数
avcodec_receive_frame()：从解码器中读取一帧音频数据
************************************************************/
static void decode(AVCodecContext *codec_ctx, AVFrame *frame, AVPacket * package, FILE *fp_out)
{
	int ret = 0;

	if((ret = avcodec_send_packet(codec_ctx, package)) < 0)
	{
		printf("avcodec_send_packet send package by codec ctx error. \n");
		exit(1);
	}

	int data_size = 0;
	int i, ch;
	//获取一个采样的字节数 根据样本格式，然后根据大小读出数据
	data_size = av_get_bytes_per_sample(codec_ctx->sample_fmt);
	//从解码器中读数据并写入文件 
	while((ret = avcodec_receive_frame(codec_ctx, frame)) >= 0)
	{
		printf("write 1 frame. channels is %d %d\n", codec_ctx->channels,  frame->nb_samples);
		//这一帧数据中音频的样本数
		for(i = 0; i < frame->nb_samples; i++)
		{
			//判断音频通道数，进行写入文件
			for(ch = 0; ch < codec_ctx->channels; ch++)
			{
				fwrite(frame->data[ch] + data_size * i, 1, data_size, fp_out);
			}
		}
	}
	//eagain
	if((ret != AVERROR(EAGAIN)) && (ret != AVERROR_EOF))
	{
		printf("avcodec_receive_frame  receive frame from codec, error\n");
		exit(1);
	}
}