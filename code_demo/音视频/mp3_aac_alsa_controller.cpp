#include <iostream>
#include "mp3_aac_alsa_controller.h"

static char *device = "default";			/* playback device */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;	/* sample format */
static unsigned int rate = 44100;			/* stream rate */
static unsigned int channels = 2;			/* count of channels */
static unsigned int buffer_time = 500000;		/* ring buffer length in us */
static unsigned int period_time = 100000;		/* period time in us */
static int resample = 1;				/* enable alsa-lib resampling */
static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;
snd_pcm_access_t mode = SND_PCM_ACCESS_RW_INTERLEAVED;
static snd_output_t *output = NULL;

/*配置参数*/
static int set_hwparams(snd_pcm_t *handle,snd_pcm_hw_params_t *params,snd_pcm_access_t access)
{
	unsigned int rrate;
	snd_pcm_uframes_t size;
	int err, dir;

	/* choose all parameters */
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
		return err;
	}
	/* set hardware resampling */
	err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
	if (err < 0) {
		printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
		return err;
	}

	/* set the interleaved read/write format */
	/*访问格式*/
	err = snd_pcm_hw_params_set_access(handle, params, mode);
	if (err < 0) {
		printf("Access type not available for playback: %s\n", snd_strerror(err));
		return err;
	}

	/* set the sample format */
	/*采样格式*/
	err = snd_pcm_hw_params_set_format(handle, params, format);
	if (err < 0) {
		printf("Sample format not available for playback: %s\n", snd_strerror(err));
		return err;
	}

	/* set the count of channels */
	/*音频声道*/
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0) {
		printf("Channels count (%u) not available for playbacks: %s\n", channels, snd_strerror(err));
		return err;
	}

	/* set the stream rate */
	/*采样率*/
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
	if (err < 0) {
		printf("Rate %uHz not available for playback: %s\n", rate, snd_strerror(err));
		return err;
	}
	if (rrate != rate) {
		printf("Rate doesn't match (requested %uHz, get %iHz)\n", rate, err);
		return -EINVAL;
	}

	/* set the buffer time */
	/*底层buffer区间,以时间为单位，500000=0.5s*/
	err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
	if (err < 0) {
		printf("Unable to set buffer time %u for playback: %s\n", buffer_time, snd_strerror(err));
		return err;
	}
	err = snd_pcm_hw_params_get_buffer_size(params, &size);
	if (err < 0) {
		printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
		return err;
	}

	buffer_size = size;
	printf("buffer_size=%ld\n",buffer_size);
	/* set the period time */
	/*底层period区间,以时间为单位,100000=0.1s*/
	err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
	if (err < 0) {
		printf("Unable to set period time %u for playback: %s\n", period_time, snd_strerror(err));
		return err;
	}
	/*底层period区间，以字节为单位,44100*0.1=4410*/
	err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
	if (err < 0) {
		printf("Unable to get period size for playback: %s\n", snd_strerror(err));
		return err;
	}
	period_size = size;
	printf("period_size=%ld\n",period_size);
	/* write the parameters to device */
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
		return err;
	}
	return 0;
}

int StartPlayAudioByAlsa(const char* in_name)
{
	int rc;
	int size;
	
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hwparams;

	snd_pcm_hw_params_alloca(&hwparams);

	printf("Playback device is %s\n", device);
	printf("Stream parameters are %uHz, %s, %u channels\n", rate, snd_pcm_format_name(format), channels);
    int err;

    err = snd_output_stdio_attach(&output, stdout, 0);
	if (err < 0) {
		printf("Output failed: %s\n", snd_strerror(err));
		return 0;
	}
    /*设置播放模式*/
	err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) 
	{
		printf("Playback open error: %s\n", snd_strerror(err));
		return 0;
	}
    /*设置参数*/
    err = set_hwparams(handle, hwparams, mode);
	if (err < 0) {
		printf("Setting of hwparams failed: %s\n", snd_strerror(err));
		return 0;
	}
     
	//period_size大概是采样点数/帧——4410点/帧
	//s16位代表两个字节，再加上双声道
	//size公式=period_size*channels*16/8
	size = (period_size * channels * snd_pcm_format_physical_width(format)) / 8; /* 2 bytes/sample, 1 channels */
	printf("size:%d\n",size);

	char *buffer;
	buffer = (char *) malloc(size);
    memset(buffer,0,size);

	// const char *in_name=fileName;

    int ret;
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
        fprintf(stderr, "Could not open codec: %s\n", ret);
        return -1;
    }
	
	//查看输入封装内容
	av_dump_format(infmt_ctx, 0, in_name,0);
    
    AVPacket *dec_pkt;
	dec_pkt = (AVPacket *)av_malloc(sizeof(AVPacket)); 
    
	AVFrame *pframePCM;
	pframePCM = av_frame_alloc();

	pframePCM->format = AV_SAMPLE_FMT_S16;
    pframePCM->channel_layout = AV_CH_LAYOUT_STEREO;
    pframePCM->sample_rate = rate;
    pframePCM->nb_samples = period_size;
    pframePCM->channels = channels;
    av_frame_get_buffer(pframePCM, 0);

	AVFrame *pframeSRC;
	pframeSRC = av_frame_alloc();

	struct SwrContext *pcm_convert_ctx  = swr_alloc();
    if (!pcm_convert_ctx) 
    {
        fprintf(stderr, "Could not allocate resampler context\n");
        return -1;
    }
    
	swr_alloc_set_opts(pcm_convert_ctx, 
	                   AV_CH_LAYOUT_STEREO, 
	                   AV_SAMPLE_FMT_S16, 
	                   pframePCM->sample_rate, 
					   av_get_default_channel_layout(decodec_ctx->channels), 
	                   decodec_ctx->sample_fmt,
	                   decodec_ctx->sample_rate, 
	                   0, 
	                   NULL);
    
	ret = swr_init(pcm_convert_ctx);
	if (ret<0) 
	{
        fprintf(stderr, "Failed to initialize the resampling context\n");
        return -1;
    }

    int got_picture;
	int nb_data;

	while (1) 
	{
		ret=av_read_frame(infmt_ctx,dec_pkt);
		if (ret != 0)
        {    
            printf("fail to read_frame\n");
            break;
        }
        
		//解码获取初始音频
        ret = avcodec_decode_audio4(decodec_ctx, pframeSRC, &got_picture, dec_pkt);
        if(!got_picture)
        {
            printf("456\n");
            continue;
        }
        
		//MP3->PCM，
        ret=swr_convert(pcm_convert_ctx,pframePCM->data, pframePCM->nb_samples,(const uint8_t **)pframeSRC->data, pframeSRC->nb_samples);
        if (ret <= 0)
		{
		    printf("123\n");
			continue;
		}

        nb_data=ret;
		//向硬件写入音频数据
		rc = snd_pcm_writei(handle, pframePCM->data[0], nb_data);
		if (rc == -EPIPE) {
			fprintf(stderr, "underrun occurred\n");
			err=snd_pcm_prepare(handle);
			if(err<0)
			{
				fprintf(stderr, "can not recover from underrun: %s\n",snd_strerror(err));
			}
		
		} 
		else if (rc < 0) {
			fprintf(stderr,"error from writei: %s\n",snd_strerror(rc));
		}  
		else if (rc != (int)nb_data) {
			fprintf(stderr,"short write, write %d frames\n", rc);
		}
	}
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);
	
	return 0;
}
