#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
// #include <alsa/asoundlib.h>
#include "wav_controller.h"

namespace AUDIO_PLAY{
	WAVController::WAVController(const char* filename):fd(-1)
	{
		fd = open(filename, O_RDONLY);
	}

	WAVController::~WAVController()
	{
		if(fd >= 0)
		{
			close(fd);
		}
	}
	//解析函数中保存着相关头信息
	int WAVController::checkAndGetWAVFile()
	{
		if(fd < 0)
		{
			printf(" open file failed: [%d]. \n", fd);
			return -1;
		}
		m_wav_parser.wavReadHeader(fd);
		return 0;
	}
	//对上下文结构体进行设置
	int WAVController::setDeviceParams()
	{
		//使用snd_pcm_hw_params_t 配置参数
		snd_pcm_hw_params_t * hwparams;
		snd_pcm_format_t format;

		snd_pcm_hw_params_alloca(&hwparams);
		//初始化hwparams handle在open的时候初始化
		if (snd_pcm_hw_params_any(m_pcm_container.handle, hwparams) < 0) {
			fprintf(stderr, "Error snd_pcm_hw_params_any\n");
			return -1 ;
		}

		//设置交错模式
		if (snd_pcm_hw_params_set_access(m_pcm_container.handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
			fprintf(stderr, "Error snd_pcm_hw_params_set_access\n");
			return -1 ;
		}
		//设置format 保存format
		if(m_wav_parser.getWavFormat(&format) <0)
		{
			fprintf(stderr, "Error getWavFormat\n");
			return -1;
		}

		if (snd_pcm_hw_params_set_format(m_pcm_container.handle, hwparams, format) < 0) {
			fprintf(stderr, "Error snd_pcm_hw_params_set_format\n");
			return -1;
		}
		m_pcm_container.format = format;
		//设置channels 保存channels
		int channels = -1;
		if((m_wav_parser.getWavFormatChannels(&channels)) <0)
		{
			fprintf(stderr, "Error getWavFormatChannels\n");
			return -1;
		}
		if (snd_pcm_hw_params_set_channels(m_pcm_container.handle, hwparams, channels) < 0) {
		//if (snd_pcm_hw_params_set_channels(m_pcm_container.handle, hwparams, LE_SHORT(channels)) < 0) {
			fprintf(stderr, "Error snd_pcm_hw_params_set_channels\n");
			return -1;
		}

		m_pcm_container.channels = channels;
		printf("XXXX %d, \n", m_pcm_container.channels);
		//获取实际采样率，获取最近采样率
		uint32_t exact_rate, temp_rate;
		if(m_wav_parser.getWavSampleRate(&exact_rate) <0)
		{
			fprintf(stderr, "Error getWavSampleRate\n");
			return -1;
		}
		printf("XXXX exact_rate %u, \n", exact_rate);
		//exact_rate = LE_INT(temp_rate);
		//获取最近采样率
		temp_rate = exact_rate;
		if (snd_pcm_hw_params_set_rate_near(m_pcm_container.handle, hwparams, &temp_rate, 0) < 0) {
			fprintf(stderr, "Error snd_pcm_hw_params_set_rate_near\n");
			return -1;
		}
		printf("XXXXtemp_rate  %u, \n", temp_rate);

		if(exact_rate != temp_rate)
		{
			fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n ==> Using %d Hz instead.\n", exact_rate, temp_rate);
		}
		//底层支持的最大buffer 换算成时间??
		uint32_t buffer_time, period_time;
		if (snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0) < 0) {
			fprintf(stderr, "Error snd_pcm_hw_params_get_buffer_time_max\n");
			return -1;
		}

		if(buffer_time > 500000)
		{
			buffer_time = 500000;
		}
		period_time = buffer_time / 4; //一个周期的大小？
		printf("XXXXperiod_time  %u, \n", period_time);
		//上文设置的大小可能不匹配 用最近的
		printf("XXXX set hwparams buffer_time: %u", buffer_time);
		if (snd_pcm_hw_params_set_buffer_time_near(m_pcm_container.handle, hwparams, &buffer_time, 0) < 0) {
			fprintf(stderr, "Error snd_pcm_hw_params_set_buffer_time_near %u\n", buffer_time);
			return -1;
		}
	 
		if (snd_pcm_hw_params_set_period_time_near(m_pcm_container.handle, hwparams, &period_time, 0) < 0) {
			fprintf(stderr, "Error snd_pcm_hw_params_set_period_time_near %u\n", period_time);
			return -1;
		}
	 	//调用使硬件参数生效
		if(snd_pcm_hw_params(m_pcm_container.handle, hwparams)<0)
		{
			fprintf(stderr, "Error snd_pcm_hw_params(handle, params)\n");
			return -1;
		}

		//获取块大小和buffer的大小
		snd_pcm_hw_params_get_period_size(hwparams, &m_pcm_container.chunk_size, 0);	
		snd_pcm_hw_params_get_buffer_size(hwparams, &m_pcm_container.buffer_size);
		if (m_pcm_container.chunk_size == m_pcm_container.buffer_size) {		
			fprintf(stderr, ("Can't use period equal to buffer size (%lu == %lu)\n"), m_pcm_container.chunk_size, m_pcm_container.buffer_size);		
			return -1;
		}

		m_pcm_container.bits_per_sample = snd_pcm_format_physical_width(format); //物理位宽
		m_pcm_container.bits_per_frame = m_pcm_container.bits_per_sample * channels;
		
		m_pcm_container.chunk_bytes = m_pcm_container.chunk_size * m_pcm_container.bits_per_frame / 8;
	 	printf("XXXXchunk_bytes  %u, \n", m_pcm_container.chunk_bytes);
	 	//申请音频buffer内存
	 	m_pcm_container.data_buf = (uint8_t *)malloc(m_pcm_container.chunk_bytes);
		if (m_pcm_container.data_buf == NULL)
		{
			fprintf(stderr, "Error malloc: [data_buf]\n");
			return -1;
		}
		return 0;
	}
	//总入口函数 ，设置参数，播放
	int WAVController::startPlayWavfile()
	{
		memset(&m_pcm_container, 0x0, sizeof(m_pcm_container));

		//重定向相关日志，触发日志的打印
		if(snd_output_stdio_attach(&m_pcm_container.log, stderr, 0) < 0)
		{
			fprintf(stderr, "Error snd_output_stdio_attach. \n");
			return -1;
		}
		//打开音频设备
		if (snd_pcm_open(&m_pcm_container.handle, m_devicename, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
			fprintf(stderr, "Error snd_pcm_open [ %s]\n", m_devicename);
			return -1;
		}

		//设置参数
		if(setDeviceParams() < 0)
		{
			fprintf(stderr, "\n\n****Error setDeviceParams. ****\n\n");
			snd_pcm_close(m_pcm_container.handle);
			return -1;
		}
		printf("*********************   start dump log   *********************\n");
		snd_pcm_dump(m_pcm_container.handle, m_pcm_container.log); //把相关信息导入到log

		printf("*********************    start play file  *********************\n");
		clock_t begin, duration;
		begin = clock();

		reallyPlayWavfile(); //真正的触发写入文件进行播放

		duration = clock() - begin;
		printf( "use time: %d ms \n", duration*1000 / CLOCKS_PER_SEC );
		printf("*********************    end play file  *********************\n");
		//对于播放，会先等待所有挂起没有传输完的数据帧先播完，才会去关闭PCM
		snd_pcm_drain(m_pcm_container.handle); 

		//需要一个接口对应set进行释放
		//释放申请的内存
		if(m_pcm_container.data_buf != NULL)
		{
			free(m_pcm_container.data_buf);
		}
		snd_output_close(m_pcm_container.log);
		snd_pcm_close(m_pcm_container.handle);
		return 0;
	}
	// 配置相关的设备
	//触发设备的设置
	//相关对应的资源释放
	//真正的播放：
	void WAVController::reallyPlayWavfile()
	{
		//真正的播放
		off64_t written = 0;
		off64_t temp;
		uint32_t count;

		int load = 0;
		int ret;
		if(m_wav_parser.getWavDataLen(&count) < 0)
		{
			printf(" get Wav Data Len error\n");
			return ;
		}
		//printf("XXXXXXXXXXX  reallyplay count =  %u \n",  count);
		//实现边写入，边读取 上文已经申请了空间  这里直接读数据  然后写数据
		while(written < count)
		{
			do{
				//从fd中读数据放入到buffer中
				temp = count - written;
				if(temp > m_pcm_container.chunk_bytes) //一个块大小，malloc的大小
				{
					temp = m_pcm_container.chunk_bytes;
				}

				//load是偏移量 temp是没有读的
				temp = temp - load; //剩余没有读
				if(temp <= 0) 
				{
					printf("has load end. [%lld] \n", temp);
					break;
				}
				//printf("XXXXXXXXXXXXXXXread to buff %d %d %dXXXXXXXXXXXXXXX \n",ret, temp, count);
				ret = ReadToBuffer(load, temp);
				//printf("XXXXXXXXXXXXXXXread to buff end [%d  %d]XXXXXXXXXXXXXXX \n",ret, m_pcm_container.chunk_bytes);
				if(ret < 0)
				{
					fprintf(stderr, "Error ReadToBuffer. \n");
					exit(-1);
				}

				if(ret == 0)
				{
					break;
				}
				load += ret;
			}while((size_t)load < m_pcm_container.chunk_bytes);//load最后写入一个块大小
			//转换成块的个数
			load = load * 8/m_pcm_container.bits_per_frame;
			//printf("XXXXXXXXXXX load = %d\n", load);
			ret = WriteToPcm(load);
			//printf("XXXXXXXXXXX ret = %d load = %d\n",ret, load);
			if(ret != load)
			{
				fprintf(stderr, "end to WriteToPcm. \n");
				break;
			}
			ret = ret * m_pcm_container.bits_per_frame / 8;
			written += ret; //获取到已经写入的大小
			load = 0;
		}
		return ;
	}

	//从fd中读取到数据，然后写入固定大小到buff中
	int WAVController::ReadToBuffer(int offset, int remain)
	{
		void * buf = (char*)m_pcm_container.data_buf + offset;
		ssize_t result = 0, res;
		while(remain > 0) //还有数据可以写
		{
			if((res = read(fd, buf, remain)) == 0)
			{
				printf("end to read to buffer :%d \n",res);
				break;
			}
			if(res < 0)
				return result > 0? result: res;
			remain -= res;
			result += res;
			buf = (char *) buf + res;
		}
		// printf("****************read to buffer [%d] addr:[%p,%p][%u]************** \n",
			// result, m_pcm_container.data_buf,buf,(*(uint32_t*)m_pcm_container.data_buf) );
		return result;
	}

	int WAVController::WriteToPcm(int chunk_size)
	{
		ssize_t r;
		ssize_t result = 0;
		uint8_t *data = m_pcm_container.data_buf;
	 	//printf("****************write to pcm [%d] addr:[%p,%p]************** \n",chunk_size, m_pcm_container.data_buf, *data);
		//printf("**************** chunk_size = %d, %d \n",chunk_size,m_pcm_container.chunk_size);
		if (chunk_size < m_pcm_container.chunk_size) {
			//将buffer中的数据转换为静音
			snd_pcm_format_set_silence(m_pcm_container.format, 
				data + chunk_size * m_pcm_container.bits_per_frame / 8, 
				(m_pcm_container.chunk_size - chunk_size) * m_pcm_container.channels);
			chunk_size = m_pcm_container.chunk_size;
		}
		//printf("**************** chunk_size = %d, %d \n",chunk_size,m_pcm_container.chunk_size);
		if(chunk_size > 0)
		{
			//chunk_size是采样的帧数
			r = snd_pcm_writei((m_pcm_container.handle), data, chunk_size);
			if (r == -EAGAIN || (r >= 0 && (size_t)r < chunk_size)) {
				snd_pcm_wait(m_pcm_container.handle, 1000);
			} else if (r == -EPIPE) {
				snd_pcm_prepare(m_pcm_container.handle);
				fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n");
			} else if (r == -ESTRPIPE) {			
				fprintf(stderr, "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>\n");		
			} else if (r < 0) {
				fprintf(stderr, "Error snd_pcm_writei: [%s] \n", snd_strerror(r));
				exit(-1);
			}
			if (r > 0) {
				result += r;
				chunk_size -= r;
				data += r * m_pcm_container.bits_per_frame / 8;
			}
		}
		return result;
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

// 	//传入文件名，打开文件，提取文件中信息
// 	WAVController wac_controller(argv[1]);
// 	//根据fd，解析验证头信息
// 	if(wac_controller.checkAndGetWAVFile() < 0)
// 	{
// 		printf("\n\n****check file error**** \n\n");
// 		return -1;
// 	}
// 	//打开设备,设置日志，设置参数，开始播放，触发日志
// 	if(wac_controller.startPlayWavfile() < 0)
// 	{
// 		printf("\n\n****play file error**** \n\n");
// 		return -1;
// 	}
// 	//显示pcm相关的设置

// 	return 0;
// }