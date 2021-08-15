//实现ffmpeg重采样功能，对重采样相关的接口进行整理
//只能重采样到pcm吗？ 如果重采样为其他格式呢？

//ffmpeg相关接口
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
//ffmpeg重采样的接口
#include <libswresample/swresample.h>

static int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);
static void fill_samples(double *dst, int nb_samples, int nb_channels, int sample_rate, double *t);
//重采样的目标文件中
int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("usage %s outputfile \n", argv[0]);
		exit(1);
	}
	const char* inputFileName = argv[1];

	//打开文件
	FILE * dst_file;
	dst_file = fopen(inputFileName, "wb");
	if(dst_file == NULL) //!dst_file
	{
		printf("fopen (open dst file error: %s) \n", inputFileName);
		exit(1);
	}
	int ret = 0; //返回码
	/**************************
	swr_alloc() ：创建SwrContext对象。
	av_opt_set_*()：设置输入和输出音频的信息。
	swr_init()： 初始化SwrContext。
	av_samples_alloc_array_and_samples：根据音频格式分配相应大小的内存空间。
	av_samples_alloc：根据音频格式分配相应大小的内存空间。用于转换过程中对输出内存大小进行调整。
	swr_convert：进行重采样转换
	***************************/
	struct SwrContext * swr_ctx;
	swr_ctx = swr_alloc();
	if(swr_ctx == NULL)
	{
		printf("swr_alloc error");
		ret = AVERROR(ENOMEM);
		goto ret1;
	}
	//定义相关参数的值
	int64_t src_ch_layout = AV_CH_LAYOUT_STEREO, dst_ch_layout = AV_CH_LAYOUT_SURROUND; //声道布局 立体声 环绕声
	int src_rate = 48000, dst_rate = 44100; //采样率
	enum AVSampleFormat src_sample_fmt = AV_SAMPLE_FMT_DBL, dst_sample_fmt = AV_SAMPLE_FMT_S16;
	//对重采样的录入和输出进行参数配置
	av_opt_set_int(swr_ctx, "in_channel_layout",    src_ch_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate",       src_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", src_sample_fmt, 0);

    av_opt_set_int(swr_ctx, "out_channel_layout",    dst_ch_layout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate",       dst_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", dst_sample_fmt, 0);

    //初始化swr
    if((ret = swr_init(swr_ctx)) < 0)
    {
    	printf("swr_init init swr error. \n");
    	ret = -1;
    	goto ret2;
    }

    //分配音频内存块,根据src_sample_fmt、src_nb_samples、src_nb_channels为src_data分配内存空间，和设置对应的的linesize的值；返回分配的总内存的大小
    int src_nb_channels = 0, dst_nb_channels = 0;
    uint8_t **src_data = NULL, **dst_data = NULL;
    int src_linesize, dst_linesize;
    int src_nb_samples = 1024, dst_nb_samples, max_dst_nb_samples;
    src_nb_channels = av_get_channel_layout_nb_channels(src_ch_layout);//获取声道
    ret = av_samples_alloc_array_and_samples(&src_data, &src_linesize, src_nb_channels, src_nb_samples, src_sample_fmt, 0);
    if(ret <= 0)
    {
    	printf("could not allocate src samples. \n");
    	ret = -1;
    	goto ret2;
    }
    //计算缓冲后的样本数
    //根据src_nb_samples*dst_rate/src_rate公式初步估算重采样后音频的nb_samples大小
    max_dst_nb_samples = dst_nb_samples = av_rescale_rnd(src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
    //缓冲区将直接写入原始音频文件 不对齐
    dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
    ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels, dst_nb_samples, dst_sample_fmt, 0);
    if(ret <= 0)
    {
    	printf("could not allocate dst samples. \n");
    	ret = -1;
    	goto ret3;
    }

    double t = 0;
    int dst_buff_size = 0;
    //合成音频  进行转码 
    do{
    	//合成音频
    	fill_samples((double*)src_data[0], src_nb_samples, src_nb_channels, src_rate, &t);
    	//计算样本目的样本数,并进行保护重置
    	dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, src_rate) +
                                        src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
    	if(dst_nb_samples > max_dst_nb_samples)
    	{
    		av_freep(&dst_data[0]);
            ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels,
                                   dst_nb_samples, dst_sample_fmt, 1);
            if (ret < 0)
                break;
            max_dst_nb_samples = dst_nb_samples;
    	}
    	//开始转码 返回每个通道输出的样本数
    	ret = swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t **)src_data, src_nb_samples);
        if (ret < 0) {
            fprintf(stderr, "Error while converting\n");
            goto ret4;
        }
        //获取目标buff的大小，并且写入文件
        dst_buff_size = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels, 
        			ret, dst_sample_fmt, 1);
        if(dst_buff_size < 0)
        {
        	printf("could not get sample buffer size. \n");
        	goto ret4;
        }
        printf("t:%f in:%d out:%d\n", t, src_nb_samples, ret);
        //写入文件
        fwrite(dst_data[0], 1, dst_buff_size, dst_file);
    }while(t<10); 

    //获取一些必要的信息进行打印
    const char *fmt;
    if ((ret = get_format_from_sample_fmt(&fmt, dst_sample_fmt)) < 0)
        goto ret4;
    fprintf(stderr, "Resampling succeeded. Play the output file with the command:\n"
            "ffplay -f %s -channel_layout %"PRId64" -channels %d -ar %d %s\n",
            fmt, dst_ch_layout, dst_nb_channels, dst_rate, inputFileName);


ret4:
    if (dst_data)
        av_freep(&dst_data[0]);
    av_freep(&dst_data);
ret3:
    if (src_data)
        av_freep(&src_data[0]);
    av_freep(&src_data);
ret2:
	swr_free(&swr_ctx);
ret1:
	fclose(dst_file);
	return ret;
}

void fill_samples(double *dst, int nb_samples, int nb_channels, int sample_rate, double *t)
{
    int i, j;
    double tincr = 1.0 / sample_rate, *dstp = dst;
    const double c = 2 * M_PI * 440.0;

    /* generate sin tone with 440Hz frequency and duplicated channels */
    for (i = 0; i < nb_samples; i++) {
        *dstp = sin(c * *t);
        for (j = 1; j < nb_channels; j++)
            dstp[j] = dstp[0];
        dstp += nb_channels;
        *t += tincr;
    }
}

//获取到设置的格式 进行正确播放  固定的
int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt)
{
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt;
        const char *fmt_be;
        const char *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };

    int i;
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "Sample format %s not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return AVERROR(EINVAL);
}