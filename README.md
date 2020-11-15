# 标题

​		该项目是基于s3c2440的文件管理器，实现具体的电子书浏览，以及相册浏览功能。该应用程序主要分为8个模块：

​    	1.页面模块page：包含7个通用菜单页面，包括主页面main_page，图片浏览页面browse_page，连播时间间隔设置页面interval_page，连续播放模式页面auto_page，图片操作菜单manal_page，设置页面setting_page。
​    	2.渲染模块render：format子模块解析jpeg以及bmp格式图片；operation子模块，实现图标的缩放，合并入主页面；以及查看图片的缩放和拖拽挪动。
​    	3.文件模块file：用作文件映射到虚拟内存。
​    	4.编码模块encoding：模块获取文字编码。
   	 5.点阵模块fonts：通过编码的到位图点阵。
​    	6.输入模块input：模块绑定按键的输入事件。
​    	7.调试模块debug：通过socket编程，实现远程打印调试。
​    	8.显示模块display：用作页面显示。

# 创作背景

​		出于学习的目的，偶然发现了这样一个项目，感觉对于自己成长很有帮助，本着追求进步、实事求是、天天向上的理念（xian de dan teng），最终完成了本鸿篇巨作之数码相册以及电子书，正所谓：学好数理化以及嵌入式，走遍天下全不怕。瞎扯到此为止，接下来开始认真的记录开发过程。项目不完善，还有很多不准确的地方，望各位不吝赐教 :D，最后，提前感谢各位的阅读，谢谢。



# 依赖安装

​		这个项目依赖于：freetype2获取字符编码的矢量字体；libjpeg用作libjpeg用作图片解析；tslib用于驱动触摸屏以及获取触摸屏输入事件获取，需交叉编译并安装到单板，将动态库以及头文件放入交叉编译工具链。



# 项目结构

│  list.txt
│  log.txt
│  main.c
│  Makefile
│  Makefile.build
│  netprint_client.c
│  说明.txt
│  
├─debug
│      debug_manager.c
│      Makefile
│      netprint.c
│      stdout.c
│      
├─display
│      disp_manager.c
│      fb.c
│      Makefile
│      
├─encoding
│      ascii.c
│      encoding_manager.c
│      Makefile
│      utf-16be.c
│      utf-16le.c
│      utf-8.c
│      
├─file
│      file.c
│      Makefile
│      
├─fonts
│      ascii.c
│      fonts_manager.c
│      freetype.c
│      gbk.c
│      Makefile
│      
├─include
│      config.h
│      debug_manager.h
│      disp_manager.h
│      encoding_manager.h
│      file.h
│      fonts_manager.h
│      input_manager.h
│      page_manager.h
│      picfmt_manager.h
│      pic_operation.h
│      render.h
│      
├─input
│      input_manager.c
│      Makefile
│      stdin.c
│      touchscreen.c
│      
├─page
│      auto_page.c
│      browse_page.c
│      interval_page.c
│      main_page.c
│      Makefile
│      manual_page.c
│      page_manager.c
│      setting_page.c
│      
└─render
    │  Makefile
    │  render.c
    │  
    ├─format
    │      bmp.c
    │      jpg.c
    │      Makefile
    │      picfmt_manager.c
    │      
    └─operation
            Makefile
            merge.c
            zoom.c



# 详细介绍

​		首先对各模块的功能进行详细的说明，模块排序按照从底层到顶层的顺序介绍。

## 1、文件模块file

​		该模块主要用于文件操作，其他模块通过调用函数的到文件句柄，得到图片以及图标的数据。提供如下API供其他模块使用：

​		int MapFile(PT_FileMap ptFileMap)：使用mmap函数映射一个文件到内存,以后就可以直接通过内存来访问文件

​		int GetDirContents(char *strDirName, PT_DirContent **pptDirContents, int *piNumber) ：把某目录下所含的顶层子目录、顶层文件都记录下来，并且按名字排序



## 2、编码模块encoding

​		该模块的功能为：获取字符的编码，从兼容性的角度出发，该模块提供对4种编码的支持，分别为：ascii/utf8/utf16le/uft16be。通过encoding_manager管理四种编码获得方式：分别将ascii/utf8/utf16le/uft16be的操作结构体注册入管理链表。编码操作结构体如下构造：

```c
typedef struct EncodingOpr {
	char *name;    /* 编码模块的名字 */
	int iHeadLen;  /* 文件头的长度: 一般在文件的开始用几个字节来表示它的编码方式 */
	PT_FontOpr ptFontOprSupportedHead;  /* 把能支持这种编码的"字体模块", 放在这个链表里 */
	int (*isSupport)(unsigned char *pucBufHead);  /* 用这个函数来判断是否支持某个文件 */
	int (*GetCodeFrmBuf)(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);  /* 取出一个字符的编码值 */
	struct EncodingOpr *ptNext;  /* 链表 */
}T_EncodingOpr, *PT_EncodingOpr;
```

​		为编码绑定支持的字体模块，用于字体模块解析。



## 3、字体模块fonts

​		一个字符的显示，依赖于三个步骤：首先获得字符的编码，其次根据字符编码获得字符的位图，最后将位图放入LCD的framebuffer，在屏幕显示。

​		本模块的作用，就是将编码模块输入字体编码，输出字体位图。有三种选择，分别是ascii、gbk、freetype。

本项目支持ascii以及gbk的宽字符点阵，但是主要使用freetype矢量字体。

```c
typedef struct FontOpr {
	char *name;             /* 字体模块的名字 */
	int (*FontInit)(char *pcFontFile, unsigned int dwFontSize);  /* 字体模块的初始化函数 */
	int (*GetFontBitmap)(unsigned int dwCode, PT_FontBitMap ptFontBitMap);  /* 根据编码值获得字符的位图 */
	void (*SetFontSize)(unsigned int dwFontSize);   /* 设置字体尺寸(单位:象素) */
	struct FontOpr *ptNext;  /* 链表 */
}T_FontOpr, *PT_FontOpr;
```

​		

### 3.1 矢量字体的说明

​		矢量字体需要用到freetype库，通过字符编码，以及一个字体文件（本项目使用WRYH.ttf）。调用freetype库提供的API，得到字体的位图点阵，最后，矢量字体最大的优势是，得到的点阵可以缩放旋转，相比于普通的字符点阵灵活许多。

​		简单的展示一下freetype提供的api

```c
static FT_Library g_tLibrary;
static FT_Face g_tFace;
static FT_GlyphSlot g_tSlot;

	iError = FT_Init_FreeType(&g_tLibrary );			   		/* initialize library */
	iError = FT_New_Face(g_tLibrary, pcFontFile, 0, &g_tFace); /* create face object */
	g_tSlot = g_tFace->glyph;
	iError = FT_Set_Pixel_Sizes(g_tFace, dwFontSize, 0);

	iError = FT_Load_Char(g_tFace, dwCode, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
```



## 4、输入事件模块input

​		input_manager用于管理两个事件输入子模块，stdin用作标准输入，使用键盘定义上一页（u），下一页（n），退出（q）操作，控制电子书。TS用作触摸屏输入，该模块使用tslib，获取触摸屏采样数据，记录采样数据的结构体如下所示：

### 4.1 触摸屏采样数据

```c
typedef struct InputEvent {
	struct timeval tTime;   /* 发生这个输入事件时的时间 */
	int iType;  /* 类别: stdin, touchsceen */
	int iX;     /* X/Y座标 */
	int iY;
	int iKey;   	/* 按键值 */
	int iPressure; /* 压力值 */
}T_InputEvent, *PT_InputEvent;
```

​		定义触摸屏上发生从左往右滑动，滑动幅度超过屏幕x分辨率的1/5，则认定为翻页信号。



### 4.2 创建子线程获取事件

​		考虑条件变量与互斥锁的结合。

​		本应用在input_manager的初始化int AllInputDevicesInit(void)中，创建子线程。在获取事件输入函数int GetInputEvent(PT_InputEvent ptInputEvent)中，休眠，子进程中读取输入，并唤醒父进程。以下是子进程的部分代码：

```c
static void *InputEventThreadFunction(void *pVoid)
{
	T_InputEvent tInputEvent;
	
	/* 定义函数指针 */
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	GetInputEvent = (int (*)(PT_InputEvent))pVoid;

	while (1)
	{
		if(0 == GetInputEvent(&tInputEvent))
		{
			/* 唤醒主线程, 把tInputEvent的值赋给一个全局变量 */
			/* 访问临界资源前，先获得互斥量 */
			pthread_mutex_lock(&g_tMutex);
			g_tInputEvent = tInputEvent;

			/*  唤醒主线程 */
			pthread_cond_signal(&g_tConVar);

			/* 释放互斥量 */
			pthread_mutex_unlock(&g_tMutex);
		}
	}

	return NULL;
}
```



### 4.3 终端输入一个值后立即返回

​		本应用想要实现的是，有一个按键输入以后立即返回，但系统的缺省处理需要按下某键，并回车确认，才会输入。所以参考APUE作如下处理：

```c
static int StdinDevInit(void)
{
    struct termios tTTYState;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);
 
    //turn off canonical mode
    tTTYState.c_lflag &= ~ICANON;
    //minimum of number input read.
    tTTYState.c_cc[VMIN] = 1;   /* 有一个数据时就立刻返回 */

    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);

	return 0;
}
```



## 5、显示模块display

​		该模块定义了缓存结构体VideoMem，为了加速显示，提前构造好若干个显示缓冲区，需要数据时刷新到显存即可。VideoMem结构体如下所示：

```c
typedef struct VideoMem {
	int iID;                        /* ID值,用于标识不同的页面 */
	int bDevFrameBuffer;            /* 1: 这个VideoMem是显示设备的显存; 0: 只是一个普通缓存 */
	E_VideoMemState eVideoMemState; /* 这个VideoMem的状态 */
	E_PicState ePicState;           /* VideoMem中内存里图片的状态 */
	T_PixelDatas tPixelDatas;       /* 内存: 用来存储图像 */
	struct VideoMem *ptNext;        /* 链表 */
}T_VideoMem, *PT_VideoMem;
```

​		考虑到内存较小的设备，可能无法分配缓冲区。将设备显存FB放到缓存结构体所在队列的头部，不分配缓存也可直接操作FB。



### 5.1 缓存状态

​		定义了两个枚举型变量，用来表示缓存状态。如下所示：

```c
/* VideoMem的状态:
 * 空闲/用于预先准备显示内容/用于当前线程
 */
typedef enum {
	VMS_FREE = 0,
	VMS_USED_FOR_PREPARE,
	VMS_USED_FOR_CUR,	
}E_VideoMemState;

/* VideoMem中内存里图片的状态:
 * 空白/正在生成/已经生成
 */
typedef enum {
	PS_BLANK = 0,
	PS_GENERATING,
	PS_GENERATED,	
}E_PicState;
```

​		在页面显示模块，当输入事件发生，并判断触摸屏采样点在某个某个图标，如在主页面点击“连播模式”，此时应当将“连播模式”的页面刷新到FB。由于缓冲区的存在，被刷新的“主页面”数据不会立刻丢失，会更改状态为VMS_USED_FOR_PREPARE，下次通过ID选择页面时，可直接返回到主页面，当缓冲区满，需要写入新数据时，才会被擦除。



## 6、页面渲染render

​		分为两个子模块：format以及operation。render整合所有对图标以及图片的操作。除此以外，render还实现了按住图标，将图标颜色取反的功能，以此来表示按键按下。

### 6.1 格式转换模块format

​		支持两种格式的图片的解析：bmp与jpeg。jpeg的解析使用libjpeg库提供的API实现，将jpeg图片数据按照行为单位解析，并存入缓冲。

​		bmp的数据结构为：BMP = 文件信息头（14byte） + 位图信息头（40byte） + RGB数据

​		也就是说只需要从54字节开始的地方读取，就可以得到图像的RGB数据。还有一点，bmp数据是从左下角开始存储的，并且一行最多4个RGB数据，在解析的时候要进行转换。



### 6.2 图片操作模块operation

​		分为两个部分的操作：合并与缩放

​		merge用于将小图标合并入背景，或大图标下。

```c
/**********************************************************************
 * 函数名称： PicMergeRegion
 * 功能描述： 把新图片的某部分, 合并入老图片的指定区域
 * 输入参数： iStartXofNewPic, iStartYofNewPic : 从新图片的(iStartXofNewPic, iStartYofNewPic)座标处开始读出数据用于合并
 *            iStartXofOldPic, iStartYofOldPic : 合并到老图片的(iStartXofOldPic, iStartYofOldPic)座标去
 *            iWidth, iHeight                  : 合并区域的大小
 *            ptNewPic                         : 新图片
 *            ptOldPic                         : 老图片
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int PicMergeRegion(int iStartXofNewPic, int iStartYofNewPic, int iStartXofOldPic, int iStartYofOldPic, int iWidth, int iHeight, PT_PixelDatas ptNewPic, PT_PixelDatas ptOldPic)
```

​		zoom用于图片的缩放。

```c
/**********************************************************************
 * 函数名称： PicZoom
 * 功能描述： 近邻取样插值方法缩放图片
 *            注意该函数会分配内存来存放缩放后的图片,用完后要用free函数释放掉
 *            "近邻取样插值"的原理请参考网友"lantianyu520"所著的"图像缩放算法"
 * 输入参数： ptOriginPic - 内含原始图片的象素数据
 *            ptBigPic    - 内含缩放后的图片的象素数据
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int PicZoom(PT_PixelDatas ptOriginPic, PT_PixelDatas ptZoomPic)
```



## 7、调试模块debug

​		设置打印级别g_iDbgLevelLimit: 级别范围0~7, 数字越小级别越高 ，高于或等于g_iDbgLevelLimit的调试信息才会打印出来。

​		开关打印频道，本应用采用了两种打印频道：标准输出（串口）和网络打印（socket客户端）。 输入参数stdout=0/1 ，关闭、打开标准输出；netprint=0/1 ，关闭、打开网络输出。

​		程序里用DBG_PRINTF来打印, 它就是DebugPrint。在config.h中的定义如下：

```c
//#define DBG_PRINTF(...)  
#define DBG_PRINTF DebugPrint
```

​		该方法将应用分为了两种运行状态，用户使用以及调试模式。当需要调试的时候，注销下面的定义，打开上面的定义即可。



### 7.1 基于UDP协议的网络打印模块

​		为完整的学习网络编程，特意添加该模块。

​		首先创建环形缓冲区充当消息队列，当有消息输出时，先将消息存入环形缓冲区。

​		在网络输出调试通道的初始化函数NetDbgInit中，将s3c2440作为服务器端，创建socket连接，bind绑定端口ID信息，并分配环形缓冲区。还绑定创建2个子线程, 一个用来接收控制命令, 比如打开/关闭某个打印通道, 设置打印级别，如下所示：

```c
	/* 创建netprint发送线程: 它用来发送打印信息给客户端 */
	pthread_create(&g_tSendTreadID, NULL, NetDbgSendTreadFunction, NULL);			
	
	/* 创建netprint接收线程: 用来接收控制信息,比如修改打印级别,打开/关闭打印 */
	pthread_create(&g_tRecvTreadID, NULL, NetDbgRecvTreadFunction, NULL);			
```

​			发送进程在NetDbgPrint函数中休眠，当其他模块调用NetDbgPrint函数时，唤醒发送子进程，将环形缓冲区的数据发送。



### 7.2 网络打印的客户端

​		将PC作为客户端，建立UDP连接，并向socket发送消息。以下为使用说明：

```c
./netprint_client <server_ip> dbglevel=<0-9>
./netprint_client <server_ip> stdout=0|1
./netprint_client <server_ip> netprint=0|1
./netprint_client <server_ip> show // setclient,并且接收打印信息
```



## 8、页面模块page

​		页面用到了7个通用的菜单模块，对应7个菜单页面，分别拥有各自的输入事件。它们通过page_manager控制，简单介绍以下page_manager的接口：

### 8.1 页面管理单元page_manager

```c
/* 从图标文件中解析出图像数据并放在指定区域,从而生成页面数据，构造的数据存入缓冲区 */
int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem)
    
/* 根据名字取出指定的"页面模块" */
PT_PageAction Page(char *pcName)

/* 两个时间点的间隔:单位ms，连播模式的间隔时间设定 */
int TimeMSBetween(struct timeval tTimeStart, struct timeval tTimeEnd)  
/* 针对连播模式，设置两个参数：播放目录和时间间隔 */
void GetPageCfg(PT_PageCfg ptPageCfg)
    
    
/* 读取输入数据,并判断它位于哪一个图标上 */
int GenericGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
```



### 8.2 图标结构体的定义

​		所有页面都是由背景图 + 图标合并而成，并且这些图标会根据输入事件的不同，具有不同的功能。所以图标结构体的构造显得尤为重要。图标Layout的定义如下：

```c
typedef struct PageLayout {
	int iTopLeftX;        /* 这个区域的左上角、右下角坐标 */
	int iTopLeftY;
	int iBotRightX;
	int iBotRightY;
	int iBpp;             /* 一个象素用多少位来表示 */
	int iMaxTotalBytes;
	PT_Layout atLayout;  /* 数组: 这个区域分成好几个小区域 */
}T_PageLayout, *PT_PageLayout;
```

​		上述的PT_Layout atLayout代表页面的图标数组，根据图标的左上角坐标，与右下角坐标，可以定义该图标上点击的事件。



# 作者

​		chen shi

​		18380459913@163.com

​		

# 问题与解决

## 1、bmp格式转换的字节对齐

​		由于bmp有一个14字节的文件信息头，所以解析数据的时候，为方便，定义了一个bmp信息头的结构体，如下所示：

```c
typedef struct tagBITMAPFILEHEADER { /* bmfh */
	unsigned short bfType; 
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long  bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER { /* bmih */
	unsigned long  biSize;
	unsigned long  biWidth;
	unsigned long  biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long  biCompression;
	unsigned long  biSizeImage;
	unsigned long  biXPelsPerMeter;
	unsigned long  biYPelsPerMeter;
	unsigned long  biClrUsed;
	unsigned long  biClrImportant;
} BITMAPINFOHEADER;
```

​		但是，当需要读取54字节后的bmp真实图片数据时，结构体自动对齐，导致读到的实际是56字节后的数据。需要设置系统1字节对齐，具体如下：

```c
	#pragma pack(push) /* 将当前pack设置压栈保存 */
	#pragma pack(1)    /* 必须在结构体定义之前使用,这是为了让结构体中各成员按1字节对齐 */
	/* 结构体定义 */
	#pragma pack(pop) /* 恢复先前的pack设置 */
```


## 2、网络打印的其他方案——fork创建子进程

​		网络打印创建两个线程，分别用于服务器端的发送与接收。在使用该方案以前，本应用考虑的是使用fork函数创建子线程，但是考虑到需要读写2个子进程，并且可能频繁切换，弃用该方案。该方案实现方法如下：

```c
			if (!fork())
			{
				/* 子进程的源码 */
				while (1)
				{
					/* 接收客户端发来的数据并显示出来 */
					iRecvLen = recv(iSocketClient, ucRecvBuf, 999, 0);
					if (iRecvLen <= 0)
					{
						close(iSocketClient);
						return -1;
					}
					else
					{
						ucRecvBuf[iRecvLen] = '\0';
						printf("Get Msg From Client %d: %s\n", iClientNum, ucRecvBuf);
					}
				}				
			}

```

​		fork使用还有一个需要注意的点，就是子进程结束以后，父进程需要调用wait回收子进程的task_struct，否则产生僵尸进程。



## 3、页面的ID标识

​		了解到一种较为简单，并且使用的唯一ID标识方式，在需要区别的个体较少时，比较好用。

```c
int ID(char *strName)
{
	return (int)(strName[0]) + (int)(strName[1]) + (int)(strName[2]) + (int)(strName[3]);
}

```



## 4、输入事件获取方式的选择

​		输入事件获取的最终方案为：在输入模块的初始化中创建子进程，在获取事件函数int GetInputEvent(PT_InputEvent ptInputEvent)进程中休眠，

```c
int GetInputEvent(PT_InputEvent ptInputEvent)
{
	/* 休眠 */
	pthread_mutex_lock(&g_tMutex);
	pthread_cond_wait(&g_tConVar, &g_tMutex);	

	/* 被唤醒后,返回数据 */
	*ptInputEvent = g_tInputEvent;
	pthread_mutex_unlock(&g_tMutex);
	return 0;	
}
```

​		在子进程服务程序中，获取事件，并唤醒主进程：

```c
static void *InputEventThreadFunction(void *pVoid)
{
	T_InputEvent tInputEvent;
	
	/* 定义函数指针 */
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	GetInputEvent = (int (*)(PT_InputEvent))pVoid;

	while (1)
	{
		if(0 == GetInputEvent(&tInputEvent))
		{
			/* 唤醒主线程, 把tInputEvent的值赋给一个全局变量 */
			/* 访问临界资源前，先获得互斥量 */
			pthread_mutex_lock(&g_tMutex);
			g_tInputEvent = tInputEvent;

			/*  唤醒主线程 */
			pthread_cond_signal(&g_tConVar);

			/* 释放互斥量 */
			pthread_mutex_unlock(&g_tMutex);
		}
	}

	return NULL;
}
```

​		其他的备选方案如下：

### 4.1 查询方式

​		该方式不可取，阻塞获取，CPU占用率极高

```c
int GetInputEvent(PT_InputEvent ptInputEvent)
{
	/* 把链表中的InputOpr的GetInputEvent都调用一次,一旦有数据即返回 */	
	PT_InputOpr ptTmp = g_ptInputOprHead;
	while (ptTmp)
	{
		if (0 == ptTmp->GetInputEvent(ptInputEvent))
		{
			return 0;
		}
		ptTmp = ptTmp->ptNext;
	}
	return -1;
}
```



### 4.2 select非阻塞监听

​		通过select实现非阻塞方式获取标准终端（APUE）
​		初始化：获取终端状态，修改标志为有一个数据就立即返回，设置终端状态
​		获取事件：select设置等待时间为0（非阻塞）

​		

### 4.3 select休眠等待监听

​		还可以通过设置timeval = NULL，来实现select的休眠，通过FD_ISSET查询监听集合中的哪一个触发。

