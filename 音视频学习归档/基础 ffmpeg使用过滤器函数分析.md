# 1：过滤器简介：

​	ffmpeg的filter⽤起来是和Gstreamer的plugin是⼀样的概念，

​			通过avfilter_link，将各个创建好的filter按 ⾃⼰想要的次序链接到⼀起，然后avfilter_graph_config之后，就可以正常使⽤。

​	⽐较常⽤的滤镜有：scale、trim、overlay、rotate、movie、yadif。

​			scale 滤镜⽤于缩放，

​			trim 滤镜⽤ 于帧级剪切，

​			overlay 滤镜⽤于视频叠加，

​			rotate 滤镜实现旋转，

​			movie 滤镜可以加载第三⽅的视频， 

​			yadif 滤镜可以去隔⾏。

# 2：主要结构体：

## 2.1 对filters系统的整体管理

```c
//对filters系统的整体管理
struct AVFilterGraph
{
	AVFilterContext **filters;
	unsigned nb_filters;
}


 // 对filters系统的整体管理
typedef struct AVFilterGraph {
	const AVClass *av_class;
	AVFilterContext **filters;
	unsigned nb_filters;
	char *scale_sws_opts; 
	#if FF_API_LAVR_OPTS
		attribute_deprecated char *resample_lavr_opts; 
	#endif

	int thread_type;
	int nb_threads;

	AVFilterGraphInternal *internal;
	void *opaque;

	avfilter_execute_func *execute;
	char *aresample_swr_opts; 
    
	AVFilterLink **sink_links;
	int sink_links_count;
	unsigned disable_auto_convert;
} AVFilterGraph;
```

## 2.2 定义filter本身的能⼒:

```c
const char *name; // overlay
const AVFilterPad *inputs;
const AVFilterPad *outputs;


AVFilter ff_vf_overlay = {
	.name = "overlay",
	.description = NULL_IF_CONFIG_SMALL("Overlay a video source ontop of the input."),
	.preinit = overlay_framesync_preinit,
	.init = init,
	.uninit = uninit,
	.priv_size = sizeof(OverlayContext),
	.priv_class = &overlay_class,
	.query_formats = query_formats,
	.activate = activate,
	.process_command = process_command,
	.inputs = avfilter_vf_overlay_inputs,
	.outputs = avfilter_vf_overlay_outputs,
	.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL |AVFILTER_FLAG_SLICE_THREADS,
};

//定义filter本身的能⼒，拥有的pads，回调函数接⼝定义

typedef struct AVFilter {
	const char *name;

	const char *description;

	const AVFilterPad *inputs;
	const AVFilterPad *outputs;
	const AVClass *priv_class;

	int flags;

	int (*preinit)(AVFilterContext *ctx);
	int (*init)(AVFilterContext *ctx);
	int (*init_dict)(AVFilterContext *ctx, AVDictionary **options);
	void (*uninit)(AVFilterContext *ctx);
	int (*query_formats)(AVFilterContext *);
    
	int priv_size;
	int flags_internal;

	struct AVFilter *next;

	int (*process_command)(AVFilterContext *, const char *cmd, const char *arg, char *res, int res_len, int flags);
	int (*init_opaque)(AVFilterContext *ctx, void *opaque);
	int (*activate)(AVFilterContext *ctx);
} AVFilter;
```

## 2.3 AVFilterContext-filter实例，管理filter与外部的联系:

```
struct AVFilterContext
{
	const AVFilter *filter;
	char *name;
	
	AVFilterPad *input_pads;
	AVFilterLink **inputs;
	
	unsigned nb_inputs
	
	AVFilterPad *output_pads;
	AVFilterLink **outputs;
	
	unsigned nb_outputs;
	struct AVFilterGraph *graph; // 从属于哪个AVFilterGraph
}

//完整结构体：
struct AVFilterContext {
	const AVClass *av_class; 
	const AVFilter *filter; 
	char *name; 
	
	AVFilterPad *input_pads; 
	AVFilterLink **inputs; 
	
	unsigned nb_inputs;
	
	AVFilterPad *output_pads;
	AVFilterLink **outputs; 
	unsigned nb_outputs; 
	void *priv; 
	struct AVFilterGraph *graph; 
	int thread_type;
	AVFilterInternal *internal;

	struct AVFilterCommand *command_queue;

	char *enable_str;
	void *enable;
	double *var_values;

	int is_disabled; 
	AVBufferRef *hw_device_ctx;

	int nb_threads;

	unsigned ready;
	int extra_hw_frames;
};

```

## 2.4 AVFilterLink-定义两个filters之间的联接

```
struct AVFilterLink
{
	AVFilterContext *src;
	AVFilterPad *srcpad;
	AVFilterContext *dst;
	AVFilterPad *dstpad;
	struct AVFilterGraph *graph;
}
```

## 2.5 AVFilterPad-定义filter的输⼊/输出接⼝

```c
struct AVFilterPad
{
	const char *name;
	AVFrame *(*get_video_buffer)(AVFilterLink *link, int w, int h);
	AVFrame *(*get_audio_buffer)(AVFilterLink *link, int nb_samples);
	int (*filter_frame)(AVFilterLink *link, AVFrame *frame);
	int (*request_frame)(AVFilterLink *link);
}

typedef struct AVFilterInOut {
	char *name;
	AVFilterContext *filter_ctx;
	int pad_idx;
	struct AVFilterInOut *next;
} AVFilterInOut;

//在AVFilter模块中定义了AVFilter结构，很个AVFilter都是具有独⽴功能的节点，
//如scale filter的作⽤就是进⾏图像尺⼨变换，overlay filter的作⽤就是进⾏图像的叠加。
	//这⾥需要重点提的是两个特别的filter，⼀个是buffer，⼀个是buffersink，
	//滤波器buffer代表filter graph中的源头，原始数据就往这个filter节点输⼊的；
		//⽽滤波器buffersink代表filter graph中的输出节点，处理完成的数据从这个filter节点输出。
```

# 3: 函数使用：

```c
// 获取FFmpeg中定义的filter，调⽤该⽅法前需要先调⽤avfilter_register_all();进⾏滤波器注册
AVFilter avfilter_get_by_name(const char name);
// 往源滤波器buffer中输⼊待处理的数据
int av_buffersrc_add_frame(AVFilterContext ctx, AVFrame frame);
// 从⽬的滤波器buffersink中获取处理完的数据
int av_buffersink_get_frame(AVFilterContext ctx, AVFrame frame);
// 创建⼀个滤波器图filter graph
AVFilterGraph *avfilter_graph_alloc(void);
// 创建⼀个滤波器实例AVFilterContext，并添加到AVFilterGraph中
int avfilter_graph_create_filter(AVFilterContext **filt_ctx, const AVFilter *filt, const char name, const char args, void *opaque, AVFilterGraph *graph_ctx);
// 连接两个滤波器节点
int avfilter_link(AVFilterContext *src, unsigned srcpad, AVFilterContext *dst, unsigned dstpad);

```

# 4: filter例子

```
⾸先使⽤split滤波器将input流分成两路流（main和tmp），然后分别对两路流进⾏处理。
	对于tmp流，先经过crop滤波器进⾏裁剪处理，再经过flip滤波器进⾏垂直⽅向上的翻转操作，输出的结果命名为flip流。
	再将main流和flip流输⼊到overlay滤波器进⾏合成操作。

	buffer源滤波器，output就是上⾯的提过的buffersink滤波器。
	上图中每个节点都是⼀个AVFilterContext，每个连线就是AVFliterLink。
	所有这些信息都统⼀由AVFilterGraph来管理

```

![image-20210802000312979](..\md文档相关图片\ffmpeg AVfilter主体框架流程.png)

# 5:filter补充命令相关

## 5.0 基础查询命令

```bash
#⽀持的filter的列表可以通过以下命令获得：
	ffmpeg -filters
#filter的⼀个简单的应⽤示例，对视频的宽和⾼减半
	 ffmpeg -i input -vf scale=iw/2:ih/2 output
```

## 5.2 filter的使用语法：

### 5.2.1 FFmpeg中filter包含三个层次**，filter->filterchain->filtergraph。**

![image-20210802000750574](..\md文档相关图片\ffmpeg 中filter使用语法.png)

```
说明：
	第⼀层是 filter 的语法。
	第⼆层是 filterchain的语法。
	第三层是 filtergraph的语法。
filtergraph可以⽤⽂本形式表示，
	可以作为ffmpeg中的-filter/-vf/-af和-filter_complex选项
		以及ffplay中的-vf/-af和libavfilter/avfilter.h中定义的avfilter_graph_parse2()函数的参数。
为了说明可能的情况，我们考虑下⾯的例⼦“把视频的上部分镜像到下半部分”。
	处理流程如下：
		1. 使⽤split filter将输⼊流分割为两个流[main]和[temp]。
		2. 其中⼀个流[temp]通过crop filter把下半部分裁剪掉。
		3. 步骤2中的输出再经过vflip filter对视频进⾏和垂直翻转，输出[flip]。
		4. 把步骤3中输出[flip]叠加到[main]的下半部分。
	以下整个处理过程的⼀个图示，也就是对filtergraph的⼀个描述[2]。
```

可以⽤以下的命令来实现这个流程：

```bash
#把视频的上部分镜像到下半部分：
ffmpeg -i INPUT -vf "split [main][tmp]; [tmp] crop=iw:ih/2:0:0, vflip [flip]; [main][flip] overlay=0:H/2" OUTPUT
```

### 5.2.2  filter的语法：

⽤⼀个字符串描述filter的组成，形式如下：

```bash
[in_link_1]…[in_link_N]filter_name=parameters[out_link_1]…[out_link_M]
参数说明：
	1. [in_link_N]、[out_link_N]：⽤来标识输⼊和输出的标签。
			in_link_N是标签名，标签名可以任意命名，需使⽤⽅括号括起来。
			在filter_name的前⾯的标签⽤于标识输⼊，在filter_name后⾯的⽤于标识输出。
		⼀个filter可以有多个输⼊和多个输出，没有输⼊的filter称为source filter，没有输出的filter称为sink filter。
		对输⼊或输出打标签是可选的，打上标签是为了连接其他filter时使⽤。
	2. filter_name：		filter的名称。
	3.  “=parameters”：	包含初始化filter的参数，是可选的。
		“=parameters”	有以下⼏种形式
			1. 使⽤':'字符分隔的⼀个“键=值”对列表。如下所示。
					ffmpeg -i input -vf scale=w=iw/2:h=ih/2 output
					ffmpeg -i input -vf scale=h=ih/2:w=iw/2 output
			2：使⽤':'字符分割的“值”的列表。
					在这种情况下，键按照声明的顺序被假定为选项名。
					例如，scale filter的前两个选项分别是w和h，当参数列表为“iw/2:ih/2”时，iw/2的值赋给w，ih/2的值赋给h。
				如下所示：
					ffmpeg -i input -vf scale=iw/2:ih/2 output
			3： 使⽤':' 字符分隔混合“值”和“键=值”对的列表。
					“值”必须位于“键=值”对之前，并遵循与前⼀点相同的约束顺序。
					之后的“键=值”对的顺序不受约束。
				如下所示：
					ffmpeg -i input -vf scale=iw/2:h=ih/2 output
				filter类定义了filter的特性以及输⼊和输出的数量，某个filter的使⽤⽅式可以通过以下命令获知：
					 ffmpeg -h filter=filter_name
		
```

以下是使⽤到fiter的标签名的⼀个示例：抽取视频Y、U、V分量到不同的⽂件：

```bash
ffmpeg -i input.mp4 -filter_complex "extractplanes=y+u+v[y][u][v]" -map "[y]" input_y.mp4 -map "[u]" input_u.mp4 -map "[v]" input_v.mp4
   #extractplanes filter指定了三个输出，分别是 [y][u][v]，抽取后，将不同的输出保存到不同的⽂件中
```

### 5.2.3  filterchain的语法

⽤⼀个字符串描述filterchain的组成，形式如下：

```
"filter1, filter2, ... filterN-1, filterN"
```

说明： 

 	1. 由⼀个或多个filter的连接⽽成，filter之间以逗号“,”分隔。
 	2. 每个filter都连接到序列中的前⼀个filter，即前⼀个filter的输出是后⼀个filter的输⼊。 

比如实例：

```bash
ffmpeg -i INPUT -vf "split [main][tmp]; [tmp] crop=iw:ih/2:0:0, vflip [flip]; [main][flip] overlay=0:H/2" OUTPUT

# crop、vflip在同⼀个filterchain中
```

### 5.2.4 filtergraph的语法

⽤⼀个字符串描述filtergraph的组成，

​		形式如下 "filterchain1;filterchain2;...filterchainN-1;fiterchainN" 

说明：

   1. 由⼀个或多个filter的组合⽽成，filterchain之间⽤分号";"分隔。

   2.  filtergraph是连接filter的有向图。它可以包含循环，⼀对filter之间可以有多个连接。

   3. 当在filtergraph中找到两个相同名称的标签时，将创建相应输⼊和输出之间的连接。 

   4.  如果输出没有被打标签，则默认将其连接到filterchain中下⼀个filter的第⼀个未打标签的输⼊。

例如 以下filterchain中：

```bash
nullsrc, split[L1], [L2]overlay, nullsink

#split filter有两个输出，overlay filter有两个输⼊。
#split的第⼀个输出标记为“L1”，overlay的第⼀个输⼊pad标记为“L2”。 
#split的第⼆个输出将连接到overlay的第⼆个输⼊。
```

5. 在⼀个filter描述中，如果没有指定第⼀个filter的输⼊标签，则假定为“In”。

   如果没有指定最后⼀个 filter的输出标签，则假定为“out”。 

6.  在⼀个完整的filterchain中，所有没有打标签的filter输⼊和输出必须是连接的。

   如果所有filterchain的 所有filter输⼊和输出pad都是连接的，则认为filtergraph是有效的[2]。

   ⽐如示例：

   ```bash
   ffmpeg -i INPUT -vf "split [main][tmp]; [tmp] crop=iw:ih/2:0:0, vflip [flip]; [main][flip] overlay=0:H/2" OUTPUT
   其中有三个filterchain, 分别是：
   	1. "split [main][tmp]"。
   			它只有⼀个filter，即 split，它有⼀个默认的输⼊，即INPUT解码后的frame。
   			有两个输出, 以 [main], [tmp] 标识。
   	2. "[tmp] crop=iw:ih/2:0:0, vflip [flip]"。
   			它由两个filter组成，crop和vflip，crop的输⼊ 为[tmp]，
   			vflip的输出标识为[flip]。
   	3. "[main][flip] overlay=0:H/2"。
   			它由⼀个filter组成，即overlay。
   			有两个输⼊，[main]和[flip]。
   			有⼀个默认的输出。
   ```





# 6：滤波过程，结构：

## 6.1: 滤波结构

把⼀整个滤波的流程称为滤波过程。下⾯是⼀个滤波过程的结构：

![image-20210802002540153](..\md文档相关图片\ffmpeg  滤波过程结构图.png)

滤波中涉及到的各个结构简介：

​	

```c
AVFilterGraph 			//⽤于统合这整个滤波过程的结构体。
AVFilter 				//滤波器，滤波器的实现是通过AVFilter以及位于其下的结构体/函数来维护的。
AVFilterContext			//⼀个滤波器实例，
				//即使是同⼀个滤波器，但是在进⾏实际的滤波时，也会由于输⼊的参数不同⽽有不同的滤波效果，
				//AVFilterContext就是在实际进⾏滤波时⽤于维护滤波相关信息的实体。
AVFilterLink 			//滤波器链，作⽤主要是⽤于连接相邻的两个AVFilterContext。
				//为了实现⼀个滤波过程，可能会需要多个滤波器协同完成，即⼀个滤波器的输出可能会是另⼀个滤波器的输⼊，
				//AVFilterLink的作⽤是串联两个相邻的滤波器实例，形成两个滤波器之间的通道。
AVFilterPad 			//滤波器的输⼊输出端⼝，⼀个滤波器可以有多个输⼊以及多个输出端⼝，
				//相邻滤波器之间是通过AVFilterLink来串联的，
				//位于AVFilterLink两端的分别就是前⼀个滤波器的输出端⼝以及后⼀个滤波器的输⼊端⼝。
buffersrc 				//⼀个特殊的滤波器，这个滤波器的作⽤就是充当整个滤波过程的⼊⼝，
    			//通过调⽤该滤波器提供的函数（如av_buffersrc_add_frame）可以把需要滤波的帧传输进⼊滤波过程。
    			//在创建该滤波器实例的时候需要提供⼀些关于所输⼊的帧的格式的必要参数（如：time_base、图像的宽⾼、图像像素格式等）。
buffersink 				//⼀个特殊的滤波器，这个滤波器的作⽤就是充当整个滤波过程的出⼝
    			//通过调⽤该滤波器提供的函数（如av_buffersink_get_frame）可以提取出被滤波过程滤波完成后的帧.
```

## 6.2：简单滤波过程：

```c
//⾸先需要得到整个滤波过程所需的滤波器（AVFilter），其中buffersrc以及buffersink是作为输⼊以及输出所必须的两个滤波器。
	const AVFilter *buffersrc = avfilter_get_by_name("buffer");
	const AVFilter *buffersink = avfilter_get_by_name("buffersink");
	const AVFilter *myfilter = avfilter_get_by_name("myfilter");

//创建统合整个滤波过程的滤波图结构体（AVFilterGraph）
	filter_graph = avfilter_graph_alloc();

//创建⽤于维护滤波相关信息的滤波器实例（AVFilterContext）
	AVFilterContext *in_video_filter = NULL;
	AVFilterContext *out_video_filter = NULL;
	AVFilterContext *my_video_filter = NULL;
	avfilter_graph_create_filter(&in_video_filter, buffersrc, "in", args, NULL, filter_graph);
	avfilter_graph_create_filter(&out_video_filter, buffersink, "out", NULL, NULL, filter_graph);
	avfilter_graph_create_filter(&my_video_filter, myfilter, "myfilter", NULL, NULL, filter_graph);

//⽤AVFilterLink把相邻的两个滤波实例连接起来
	avfilter_link(in_video_filter, 0, my_video_filter, 0);
	avfilter_link(my_video_filter, 0, out_video_filter, 0);

//提交整个滤波图
	avfilter_graph_config(filter_graph, NULL);
```

## 6.3: 创建复杂的滤波过程

​	即需要多个滤波器进⾏复杂的连接来实现整个滤波过程.

​	对于复杂的滤波过程，ffmpeg提供了⼀个 更为⽅便的滤波过程创建⽅式。

```bash
#这种复杂的滤波器过程创建⽅式要求⽤户以字符串的⽅式描述各个滤波器之间的关系。
#如下是⼀个描述复杂滤波过程的字符串的例⼦：

[0]trim=start_frame=10:end_frame=20[v0];\
[0]trim=start_frame=30:end_frame=40[v1];\
[v0][v1]concat=n=2[v2];\
[1]hflip[v3];\
[v2][v3]overlay=eof_action=repeat[v4];\
[v4]drawbox=50:50:120:120:red:t=5[v5]

#以上是⼀个连续的字符串，为了⽅便分析我们把该字符串进⾏了划分，每⼀⾏都是⼀个滤波器实例，
#对于⼀⾏：
#	1. 开头是⼀对中括号，中括号内的是输⼊的标识名0。
#	2. 中括号后⾯接着的是滤波器名称trim。
#	3. 名称后的第⼀个等号后⾯是滤波器参数start_frame=10:end_frame=20，这⾥有两组参数，两组参数⽤冒号分开。
#	4. 第⼀组参数名称为start_frame，参数值为10，中间⽤等号分开。
#	5. 第⼆组参数名称为end_frame，参数值为20，中间⽤等号分开。
#	6. 最后也有⼀对中括号，中括号内的是输出的标识名v0。
#	7. 如果⼀个滤波实例的输⼊标识名与另⼀个滤波实例的输出标识名相同，则表示这两个滤波实例构成滤波链。
#	8. 如果⼀个滤波实例的输⼊标识名或者输出标识名⼀直没有与其它滤波实例的输出标识名或者输⼊标识名相同，则表明这些为外部的输⼊输出，通常我们会为其接上buffersrc以及buffersink。

```

按照这种规则，上⾯的滤波过程可以被描绘成以下滤波图：

![image-20210802003623532](..\md文档相关图片\ffmpeg 复杂滤波图流程.png)

```c
//ffmpeg提供⼀个函数⽤于解析这种字符串：avfilter_graph_parse2。
	//这个函数会把输⼊的字符串⽣成如上⾯的滤波图，
	//不过我们需要⾃⾏⽣成buffersrc以及buffersink的实例，并通过该函数提供的输⼊以及输出接⼝把buffersrc、buffersink与该滤波图连接起来。
	//整个流程包含以下步骤：
		//创建统合整个滤波过程的滤波图结构体（AVFilterGraph）:
			 filter_graph = avfilter_graph_alloc();
		//解析字符串，并构建该字符串所描述的滤波图
			 avfilter_graph_parse2(filter_graph, graph_desc, &inputs, &outputs);
		//其中inputs与outputs分别为输⼊与输出的接⼝集合，我们需要为这些接⼝接上输⼊以及输出
			for (cur = inputs, i = 0; cur; cur = cur->next, i++) {
				const AVFilter *buffersrc = avfilter_get_by_name("buffer");
				avfilter_graph_create_filter(&filter, buffersrc, name, args, NULL, filter_graph);
                avfilter_link(filter, 0, cur->filter_ctx, cur->pad_idx);
			}
			avfilter_inout_free(&inputs);
			for (cur = outputs, i = 0; cur; cur = cur->next, i++) {
				const AVFilter *buffersink = avfilter_get_by_name("buffersink");
				avfilter_graph_create_filter(&filter, buffersink, name, NULL, NULL, filter_graph);
				avfilter_link(cur->filter_ctx, cur->pad_idx, filter, 0);
			}
			avfilter_inout_free(&outputs);
		//提交整个滤波图
			avfilter_graph_config(filter_graph, NULL);

```

## 6.4 滤波API

```c
//buffersrc提供了向滤波过程输⼊帧的API：av_buffersrc_add_frame。
	//向指定的buffersrc实例输⼊想要进⾏滤波的帧就可以把帧传⼊滤波过程。
	av_buffersrc_add_frame(c->in_filter, pFrame);
//buffersink提供了从滤波过程提取帧的API：av_buffersink_get_frame。
	//可以从指定的buffersink实例提取滤波完成的帧。
	av_buffersink_get_frame(c->out_filter, pFrame);

//当av_buffersink_get_frame返回值⼤于0则表示提取成功。
```

