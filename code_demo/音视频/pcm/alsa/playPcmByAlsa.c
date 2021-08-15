//使用alsa播放pcm文件

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PCM_FORMAT SND_PCM_FORMAT_S16_LE
#define CHANNEL 1
#define SAMPLE_RATE 16000

int main(int argc,char *argv[])
{
	/* Handle for the PCM device */ 
    	snd_pcm_t *handle;          
    	/* Playback stream */
    	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
    	snd_pcm_hw_params_t *hwparams;
    	char *pcm_name = "default";
    	int err;
	int dir = 0;
	snd_pcm_format_t format = PCM_FORMAT;
	unsigned int nChannels = CHANNEL;
	unsigned int rate = SAMPLE_RATE;
	snd_pcm_uframes_t psize_frames = 32;
	snd_pcm_uframes_t bsize_frames = 64;

	char *filename = NULL;
	int fd;
	char *buffer = NULL;
	int count;
	unsigned int buffer_time;

	if(argc < 2)
	{
		printf("Usage: ./playback pcmfile\n");
		return -1;
	}
	filename = argv[1];
	fd = open(filename,O_RDONLY);
	if(fd < 0)
	{
		perror("open file failed!\n");
		return -1;
	}

	//open PCM
    	if (snd_pcm_open(&handle, pcm_name, stream, 0) < 0) 
    	{
      		printf(stderr, "Error opening PCM device %s\n", pcm_name);
      		return -1;
    	}

    	//Allocate the snd_pcm_hw_params_t structure on the stack. 
    	err = snd_pcm_hw_params_malloc(&hwparams);
	if(err < 0)
	{
		perror("malloc space for snd_pcm_hw_params_t structure failed !\n");
		return -1;
	}

	//initial hwparams
    	err = snd_pcm_hw_params_any(handle, hwparams);
    	if (err < 0) 
    	{
		printf("Broken configuration for this PCM\n");
		return err;
    	}

	//set access type
    	err = snd_pcm_hw_params_set_access(handle, hwparams,SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) 
	{
		printf("Access type not available\n");
		return err;
	}

	//set pcm format
	err = snd_pcm_hw_params_set_format(handle, hwparams, format);
	if (err < 0) 
	{
		printf("Sample format non available\n");
		return err;
	}

	//set pcm channels
	err = snd_pcm_hw_params_set_channels(handle, hwparams, 1);
	if (err < 0) 
	{
		printf("Channels count non available\n");
		return err;
	}

	//set pcm sample rate
	err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &rate, &dir);
	if (err < 0) 
	{
		printf("Set rate failed\n");
		return err;
	}

	//set buffer size
	err = snd_pcm_hw_params_set_buffer_size_near(handle,hwparams,&bsize_frames);
	if(err < 0)
	{
		printf("Set period size failed\n");
		return err;
	}

	//set period size
	err = snd_pcm_hw_params_set_period_size_near(handle,hwparams,&psize_frames,0);
	if(err < 0)
	{
		printf("Set buffer size failed\n");
		return err;
	}

	//set the hwparams to driver
	err = snd_pcm_hw_params(handle,hwparams);
	if(err<0)
	{
		printf("failed to set hardware parameters:%s\n",snd_strerror(err));
		return err;
	}

	//Print information about buffer and period
	printf("Print information about buffer and period:\n");

	err = snd_pcm_hw_params_get_period_size_min(hwparams,&psize_frames,&dir);
	if(err < 0)
	{
		printf("get period size min falied!\n");
		return -1;
	}
	else
	{
		printf("period size min is %d.\n",psize_frames);
	}

	err = snd_pcm_hw_params_get_period_size(hwparams, &psize_frames, &dir);
	if (err < 0) 
	{
		printf("get period size fail\n");
		return err;
	}
	else
	{
		printf("period size is %d frames.\n",psize_frames);
	}

	err = snd_pcm_hw_params_get_buffer_size_min(hwparams,&bsize_frames);
	if(err < 0)
	{
		printf("get buffer size min falied!\n");
		return -1;
	}
	else
	{
		printf("buffer size min is %d.\n",bsize_frames);
	}

	err = snd_pcm_hw_params_get_buffer_size(hwparams, &bsize_frames);
	if (err < 0) 
	{
		printf("get buffer size failed\n");
		return err;
	}
	else
	{
		printf("buffer size is %d frames.\n",bsize_frames);
	}

	err = snd_pcm_hw_params_get_buffer_time_max(hwparams,&buffer_time,0);
	if(err < 0)
	{
		printf("Get buffer time failed!\n");
		return err;
	}
	else
	{
		printf("buffer time is %d us.\n",buffer_time);
	}

	
	//play
	buffer = (char*)malloc(psize_frames*2);//1 channel,sizeof(frame) = 2*1 byte;
	while(1)
	{
		count = read(fd,buffer,psize_frames*2);
		if(count <= 0)
			break;
		
		while(err = snd_pcm_writei(handle,buffer,psize_frames)<0)
		{

			if(err == -EAGAIN)
			{
				printf("again");
				snd_pcm_wait(handle,1000);
			}
			else if(err == -EPIPE)
			{
				snd_pcm_prepare(handle);
				printf("buffer underrun!!!!\n");
			}
			else if(err == -ESTRPIPE)
			{
				printf("Need suspend!\n");
			}
			else if(err < 0)
			{
				printf("snd_pcm_writei error:%s\n",snd_strerror(err));
			}
		}
	}

	snd_pcm_drain(handle);
  	snd_pcm_close(handle);
	free(buffer);

	return 0;
}

