
#ifndef _DEBUG_MANAGER_H
#define _DEBUG_MANAGER_H

/* 信息的调试级别,数值起小级别越高 */
#define	APP_EMERG	"<0>"	/* system is unusable			*/
#define	APP_ALERT	"<1>"	/* action must be taken immediately	*/
#define	APP_CRIT	"<2>"	/* critical conditions			*/
#define	APP_ERR	    "<3>"	/* error conditions			*/
#define	APP_WARNING	"<4>"	/* warning conditions			*/
#define	APP_NOTICE	"<5>"	/* normal but significant condition	*/
#define	APP_INFO	"<6>"	/* informational			*/
#define	APP_DEBUG	"<7>"	/* debug-level messages			*/

/* 信息的默认调试级别 */
#define DEFAULT_DBGLEVEL  4

typedef struct DebugOpr {
	char *name;
	int isCanUse;
	int (*DebugInit)(void);   /* 调试模块的初始化函数 */
	int (*DebugExit)(void);   /* 退出函数 */
	int (*DebugPrint)(char *strData);  /* 输出函数 */
	struct DebugOpr *ptNext;
}T_DebugOpr, *PT_DebugOpr;

/**********************************************************************
 * 函数名称： RegisterDispOpr
 * 功能描述： 注册"调试通道", 把PT_DebugOpr结构体放入链表中
 * 输入参数： ptDebugOpr - 一个结构体,表示调试通道
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int RegisterDebugOpr(PT_DebugOpr ptDebugOpr);

/**********************************************************************
 * 函数名称： ShowDebugOpr
 * 功能描述： 显示本程序能支持的"调试模块"
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void ShowDebugOpr(void);

/**********************************************************************
 * 函数名称： GetDebugOpr
 * 功能描述： 根据名字取出指定的"调试模块"
 * 输入参数： pcName - 名字
 * 输出参数： 无
 * 返 回 值： NULL   - 失败,没有指定的模块, 
 *            非NULL - 显示模块的PT_DebugOpr结构体指针
 ***********************************************************************/
PT_DebugOpr GetDebugOpr(char *pcName);

/**********************************************************************
 * 函数名称： SetDbgLevel
 * 功能描述： 设置打印级别g_iDbgLevelLimit: 级别范围0~7, 数字越小级别越高
 *            高于或等于g_iDbgLevelLimit的调试信息才会打印出来
 * 输入参数： strBuf - 类似"dbglevel=3"
 * 输出参数： 无
 * 返 回 值： 0   - 成功
 ***********************************************************************/
int SetDbgLevel(char *strBuf);

/**********************************************************************
 * 函数名称： SetDbgChanel
 * 功能描述： 开/关某个调试通道
 * 输入参数： strBuf - 类似于以下字符串
 *                     stdout=0			   : 关闭stdout打印
 *                     stdout=1			   : 打开stdout打印
 *                     netprint=0		   : 关闭netprint打印
 *                     netprint=1		   : 打开netprint打印
 * 输出参数： 无
 * 返 回 值： 0   - 成功
 *            -1  - 失败
 ***********************************************************************/
int SetDbgChanel(char *strBuf);

/**********************************************************************
 * 函数名称： DebugInit
 * 功能描述： 注册调试通道,就是把PT_DebugOpr注册到链表中
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int DebugInit(void);

/**********************************************************************
 * 函数名称： DebugPrint
 * 功能描述： 打印信息的总入口函数
 *            程序里用DBG_PRINTF来打印, 它就是DebugPrint
 *            在config.h里有这样的宏定义: #define DBG_PRINTF DebugPrint
 * 输入参数： 可变参数,用法和printf完全一样
 * 输出参数： 无
 * 返 回 值： 0   - 成功
 *            -1  - 失败
 ***********************************************************************/
int DebugPrint(const char *pcFormat, ...);

/**********************************************************************
 * 函数名称： InitDebugChanel
 * 功能描述： 有些打印通道需要进行一些初始化, 比如网络打印需要绑定端口等等
 *            本函数用于执行这些初始化
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int InitDebugChanel(void);

/**********************************************************************
 * 函数名称： StdoutInit
 * 功能描述： 注册"标准输出调试通道", 把g_tStdoutDbgOpr结构体放入链表中
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int StdoutInit(void);

/**********************************************************************
 * 函数名称： StdoutInit
 * 功能描述： 注册"网络输出调试通道", 把g_tNetDbgOpr结构体放入链表中
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int NetPrintInit(void);

#endif /* _DEBUG_MANAGER_H */
  
