//获取音频  并使用pcm进行播放
#include <alsa/asoundlib.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>


#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FSIZE 2*CHANNELS

int main(int argc, char *argv[])
{
	int rc;
	int size;
	
	int ret;
	
	unsigned int val;

	int dir;
	snd_pcm_uframes_t frames;
	char *buffer;
	char *inFile;

	int fd;
	int err;

	inFile = "output.raw";

	fd = open(inFile,O_RDONLY);
	
	snd_pcm_t *handle;
    //以播放模式打开设备
	rc = snd_pcm_open(&handle, "default",SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) 
	{
		fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(rc));
		exit(1);
	}
    //配置硬件参数结构体
	snd_pcm_hw_params_t *params;
	
	//params申请内存
    snd_pcm_hw_params_malloc(&params);

	 //使用pcm设备初始化hwparams
	err=snd_pcm_hw_params_any(handle, params);
	if (err < 0)
	{
		fprintf(stderr, "Can not configure this PCM device: %s\n",snd_strerror(err));
		exit(1);
	}
	
	//设置多路数据在buffer中的存储方式
	//SND_PCM_ACCESS_RW_INTERLEAVED每个周期(period)左右声道的数据交叉存放
	err=snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) 
	{
		fprintf(stderr,"Failed to set PCM device to interleaved: %s\n",snd_strerror(err));
		exit(1);
	}
	
	//设置16位采样格式，S16为有符号16位,LE是小端模式
	err=snd_pcm_hw_params_set_format(handle, params,SND_PCM_FORMAT_S16_LE);
	if (err < 0)
	{
		fprintf(stderr,"Failed to set PCM device to 16-bit signed PCM: %s\n",snd_strerror(err));
		exit(1);
	}
	
	//设置声道数,双声道
	err=snd_pcm_hw_params_set_channels(handle, params, CHANNELS);
	if (err < 0)
	{
		fprintf(stderr, "Failed to set PCM device to mono: %s\n",snd_strerror(err));
		exit(1);
	}
	
	//采样率48000
	val = SAMPLE_RATE;
	//设置采样率,如果采样率不支持，会用硬件支持最接近的采样率
	err=snd_pcm_hw_params_set_rate_near(handle, params,&val, &dir);
	if (err < 0) 
	{
		fprintf(stderr, "Failed to set PCM device to sample rate =%d: %s\n",val,snd_strerror(err));
		exit(1);
	}
	
	unsigned int buffer_time,period_time;
	//获取最大的缓冲时间,buffer_time单位为us,500000us=0.5s
	snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
	if ( buffer_time >500000)
		buffer_time = 500000;
	
    //设置缓冲时间
	err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, 0);
	if (err < 0) 
	{
		fprintf(stderr, "Failed to set PCM device to buffer time =%d: %s\n",buffer_time,snd_strerror(err));
		exit(1);
	}
	
    period_time = 26315;
    //设置周期时间
	err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
	if (err < 0) 
	{
		fprintf(stderr, "Failed to set PCM device to period time =%d: %s\n",period_time,snd_strerror(err));
		exit(1);
	}

    //让这些参数作用于PCM设备
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0) 
	{
		fprintf(stderr,"unable to set hw parameters: %s\n",snd_strerror(rc));
		exit(1);
	}

	snd_pcm_hw_params_get_period_size(params, &frames,&dir);
	// 1 frame = channels * sample_size.
	size = frames * FSIZE; /* 2 bytes/sample, 1 channels */
	buffer = (char *) malloc(size);
    

    //服务器地址
    char *in_name="rtmp://127.0.0.1/live/stream";
   
    AVFormatContext* infmt_ctx = NULL;
    //创建输入封装器
    ret=avformat_open_input(&infmt_ctx, in_name, NULL, NULL);
    if (ret != 0) 
    {
        printf("failed alloc output context\n");
        return -1;
    }
	
	//读取一部分视音频流并且获得一些相关的信息
    avformat_find_stream_info(infmt_ctx, NULL);
    
	//视频流和音频流的标志
	int audioindex=-1;
	//查找视频||音频流
	for(int i=0; i<infmt_ctx->nb_streams; i++) 
	{
		if(infmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
		{
			audioindex=i;
			break;
	    }

	}
    
	if (audioindex == -1) 
	{
        printf("input video stream not exist\n");
        return -1;
    }
    
	AVCodec* decodec = NULL;

	AVCodecContext* decodec_ctx = NULL;

	decodec_ctx=infmt_ctx->streams[audioindex]->codec;
	//找到解码器
	decodec = avcodec_find_decoder(decodec_ctx->codec_id);
	if (!decodec) 
	{
		printf("not find decoder\n");
		avformat_close_input(&infmt_ctx);
		return -1;
	}

	//打开解码器
	ret = avcodec_open2(decodec_ctx, decodec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
        return -1;
    }
	
	//查看输入封装内容
	av_dump_format(infmt_ctx, 0, in_name,0);
    
	//拉流包裹
    AVPacket *dec_pkt;
	dec_pkt = (AVPacket *)av_malloc(sizeof(AVPacket)); 
    
    //用于播放的音频帧
	AVFrame *pframePCM;
	pframePCM = av_frame_alloc();

	pframePCM->format = AV_SAMPLE_FMT_S16;
    pframePCM->channel_layout = AV_CH_LAYOUT_STEREO;
    pframePCM->sample_rate = 48000;
    pframePCM->nb_samples = frames;
    pframePCM->channels = CHANNELS;
    av_frame_get_buffer(pframePCM, 0);
    
    //拉流获得的音频帧
	AVFrame *pframeAAC;
	pframeAAC = av_frame_alloc();

	pframeAAC->format = AV_SAMPLE_FMT_FLTP;
    pframeAAC->channel_layout = AV_CH_LAYOUT_STEREO;
    pframeAAC->sample_rate = 44100;
    pframeAAC->nb_samples = 1024;
    pframeAAC->channels = CHANNELS;
    av_frame_get_buffer(pframeAAC, 0);

    
    //作用：拉流获得的音频帧-->用于播放的音频帧
    //音频帧转换器
    struct SwrContext *pcm_convert_ctx  = swr_alloc();
    if (!pcm_convert_ctx) 
    {
        fprintf(stderr, "Could not allocate resampler context\n");
        return -1;
    }
    
	swr_alloc_set_opts(pcm_convert_ctx, 
	                   AV_CH_LAYOUT_STEREO,     //目标音频帧
	                   AV_SAMPLE_FMT_S16, 
	                   48000, 
					   AV_CH_LAYOUT_STEREO,     //初始音频帧
	                   AV_SAMPLE_FMT_FLTP,
	                   44100, 
	                   0, 
	                   NULL);
    
	//音频帧转换器初始化
	if ((ret = swr_init(pcm_convert_ctx)) < 0) 
	{
        fprintf(stderr, "Failed to initialize the resampling context\n");
        return -1;
    }
    
	int got_picture = 0;
	while (1) 
	{
		//获取音频包裹
	    ret=av_read_frame(infmt_ctx,dec_pkt);
		if (ret != 0)
        {    
            printf("fail to read_frame\n");
            break;
        }
        
		//解码获取初始音频帧
        ret = avcodec_decode_audio4(decodec_ctx, pframeAAC, &got_picture, dec_pkt);
        if(!got_picture)
        {
            printf("获取初始音频帧失败\n");
            continue;
        }
        
		//aac-->pcm格式转换
        ret=swr_convert(pcm_convert_ctx,pframePCM->data, pframePCM->nb_samples,(const uint8_t **)pframeAAC->data, pframeAAC->nb_samples);
        if (ret <= 0)
		{
		    printf("音频帧转化失败\n");
			continue;
		}

		memcpy(buffer,pframePCM->data[0],size);
        
        //播放音频
		rc = snd_pcm_writei(handle, buffer, frames);
		if (rc == -EPIPE) 
		{
			fprintf(stderr, "underrun occurred\n");
			err=snd_pcm_prepare(handle);
			if(err <0)
			{
				fprintf(stderr, "can not recover from underrun: %s\n",snd_strerror(err));
			}
		} 
		else if (rc < 0) 
		{
			fprintf(stderr,"error from writei: %s\n",snd_strerror(rc));
		}  
		else if (rc != (int)frames) 
		{
			fprintf(stderr,"short write, write %d frames\n", rc);
		}
		
        av_free_packet(dec_pkt);
	}
	avformat_close_input(&infmt_ctx);
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);
	close(fd);
	return 0;
}

