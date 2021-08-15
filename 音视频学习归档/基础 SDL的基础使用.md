SDL：可以控制视频播放，声音播放，响应键盘鼠标的一些事件。

​	实际上就是使用SDL播放YUV（视频）, PCM（音频）。



1：windows上使用SDL：

​	下载必要的库，把库拷贝到我们的qt项目目录下。

​	去：[Simple DirectMedia Layer - SDL version 2.0.14 (stable) (libsdl.org)](https://www.libsdl.org/download-2.0.php)

​	下载：[SDL2-devel-2.0.14-VC.zip](https://www.libsdl.org/release/SDL2-devel-2.0.14-VC.zip) (Visual C++ 32/64-bit)

2：linux环境上使用SDL：

​	下载地址：https://www.libsdl.org/download-2.0.php

​	下载源码库：SDL2-2.0.10.tar.gz

​	依次执行解压安装：

```bash
/configure
make
sudo make install
```

​		如果出现Could not initialize SDL - No available video device (Did you set the DISPLAY variable?)错误 

​		说明系统中没有安装x11的库文件，因此编译出来的SDL库实际上不能用。

```bash
sudo apt-get install libx11-dev
sudo apt-get install xorg-dev
```

3：SDL各个子系统：

```bash
SDL将功能分成下列数个子系统（subsystem）：
	◼ SDL_INIT_TIMER：		定时器
	◼ SDL_INIT_AUDIO：		音频
	◼ SDL_INIT_VIDEO：		视频
	◼ SDL_INIT_JOYSTICK：	摇杆
	◼ SDL_INIT_HAPTIC：		触摸屏
	◼ SDL_INIT_GAMECONTROLLER：	游戏控制器
	◼ SDL_INIT_EVENTS：			事件
	◼ SDL_INIT_EVERYTHING：		包含上述所有选项
```

4：SDL视频显示函数介绍：

```
◼ SDL_Init()：			初始化SDL系统
◼ SDL_CreateWindow()：	创建窗口SDL_Window
◼ SDL_CreateRenderer()：	创建渲染器SDL_Renderer
◼ SDL_CreateTexture()：	创建纹理SDL_Texture
◼ SDL_UpdateTexture()：	设置纹理的数据
◼ SDL_RenderCopy()：		将纹理的数据拷贝给渲染器
◼ SDL_RenderPresent()：	显示
◼ SDL_Delay()：			工具函数，用于延时
◼ SDL_Quit()：			退出SDL系统
```

​	SDL数据结构简介：

```bash
◼ SDL_Window 		代表了一个“窗口”
◼ SDL_Renderer 		代表了一个“渲染器”
◼ SDL_Texture 		代表了一个“纹理”
◼ SDL_Rect 			一个简单的矩形结构
```

存储RGB和存储纹理的区别： 

​		比如一个从左到右由红色渐变到蓝色的矩形，用 存储RGB的话就需要把矩形中每个点的具体颜色 值存储下来；

​		而纹理只是一些描述信息，比如记 录了矩形的大小、起始颜色、终止颜色等信息， 显卡可以通过这些信息推算出矩形块的详细信息。 

​		所以相对于存储RGB而已，存储纹理占用的内存要少的多。

5：SDL事件：

```bash
◼ 函数
	• SDL_WaitEvent()：		等待一个事件
	• SDL_PushEvent()：		发送一个事件
	• SDL_PumpEvents()：		将硬件设备产生的事件放入事件队列，用于读取事件，在调用该函数之前，必须调用SDL_PumpEvents搜集键盘等事件
	• SDL_PeepEvents()：		从事件队列提取一个事件
◼ 数据结构
	• SDL_Event：			代表一个事件
```

6：SDL线程：

```bash
SDL多线程
	◼ SDL线程创建：					SDL_CreateThread
	◼ SDL线程等待：					SDL_WaitThead
	◼ SDL互斥锁：		 			 SDL_CreateMutex/SDL_DestroyMutex
	◼ SDL锁定互斥：		 			 SDL_LockMutex/SDL_UnlockMutex
	◼ SDL条件变量(信号量)：			   SDL_CreateCond/SDL_DestoryCond
	◼ SDL条件变量(信号量)等待/通知：	SDL_CondWait/SDL_CondSingal
```

7：YUV 视频显示流程：

![image-20210801141226307](..\md文档相关图片\SDL yuv视频显示流程.png)

8：SDL播放音频PCM流程：

```c
//打开音频设备：
	int SDLCALL SDL_OpenAudio(SDL_AudioSpec * desired, SDL_AudioSpec * obtained); 
		// desired：期望的参数。
		// obtained：实际音频设备的参数，一般情况下设置为NULL即可。
//SDL_AudioSpec 参数：
	typedef struct SDL_AudioSpec {
		int freq; 					// 音频采样率
		SDL_AudioFormat format; 	// 音频数据格式
		Uint8 channels; 			// 声道数: 1 单声道, 2 立体声
		Uint8 silence; 				// 设置静音的值，因为声音采样是有符号的，所以0当然就是这个值
		Uint16 samples; 			// 音频缓冲区中的采样个数，要求必须是2的n次
		Uint16 padding; 			// 考虑到兼容性的一个参数
		Uint32 size; 				// 音频缓冲区的大小，以字节为单位
		SDL_AudioCallback callback; // 填充音频缓冲区的回调函数
		void *userdata; 			// 用户自定义的数据
	} SDL_AudioSpec;
//SDL_AudioCallback
	void (SDLCALL * SDL_AudioCallback) (void *userdata, Uint8 *stream, int len);
		// userdata：SDL_AudioSpec结构中的用户自定义数据，一般情况下可以不用。
		// stream：该指针指向需要填充的音频缓冲区。
		// len：音频缓冲区的大小（以字节为单位）1024*2*2。
//播放音频数据
	void SDLCALL SDL_PauseAudio(int pause_on)
		// 当pause_on设置为0的时候即可开始播放音频数据。设置为1的时候，将会播放静音的值。
```

