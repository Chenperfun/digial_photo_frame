#include <config.h>
#include <encoding_manager.h>
#include <string.h>
#include <stdlib.h>

static PT_EncodingOpr g_ptEncodingOprHead;

/**********************************************************************
 * 函数名称： RegisterEncodingOpr
 * 功能描述： 注册"编码模块", 一行文字可能采用ASCII编码,也可能用UTF8等编码.
 *            怎么从一行文字中取出一个字,不同的编码取法不一样,
 *            每一种编码方式都调用RegisterEncodingOpr把相关结构体放入g_ptEncodingOprHead链表
 * 输入参数： ptEncodingOpr - 一个结构体,内含某种编码方式的操作函数
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr)
{
	PT_EncodingOpr ptTmp;

	if (!g_ptEncodingOprHead)
	{
		g_ptEncodingOprHead   = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptEncodingOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	      = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}

	return 0;
}


/**********************************************************************
 * 函数名称： Encode
 * 功能描述： 根据名字取出指定的"编码模块"
 * 输入参数： pcName - 名字
 * 输出参数： 无
 * 返 回 值： NULL   - 失败,没有指定的模块, 
 *            非NULL - "页面模块"的PT_EncodingOpr结构体指针
 ***********************************************************************/
PT_EncodingOpr Encode(char *pcName)
{
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


/**********************************************************************
 * 函数名称： ShowEncodingOpr
 * 功能描述： 显示本程序能支持的"编码方式"
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void ShowEncodingOpr(void)
{
	int i = 0;
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/**********************************************************************
 * 函数名称： SelectEncodingOprForFile
 * 功能描述： 给文件选择一个编码模块
 * 输入参数： pucFileBufHead - 文件头部, 即调用mmap映射文件所得到内存指针
 * 输出参数： 无
 * 返 回 值： NULL   - 现有的编码方式都不支持此文件
 *            非NULL - 支持此文件的编码方式对应的结构体指针
 ***********************************************************************/
PT_EncodingOpr SelectEncodingOprForFile(unsigned char *pucFileBufHead)
{
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;

	while (ptTmp)
	{	
		if (ptTmp->isSupport(pucFileBufHead))
			return ptTmp;
		else
			ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

/**********************************************************************
 * 函数名称： GetCodeFrmBuf
 * 功能描述： 从字符串中取出UTF8格式的字符串的UNICODE编码值
 * 输入参数： pucBufStart - 字符串起始位置
 *            pucBufEnd   - 字符串结束位置(这个位置的字符不处理)
 * 输出参数： pdwCode     - 所取出的编码值存在这里
 * 返 回 值： 使用了多少个字符才得到这个编码值
 ***********************************************************************/
int GetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{
    return Encode("utf-8")->GetCodeFrmBuf(pucBufStart, pucBufEnd, pdwCode);
}


/**********************************************************************
 * 函数名称： AddFontOprForEncoding
 * 功能描述：给某个编码方式(ptEncodingOpr)提供字符的处理方法(ptFontOpr),
 *        ptFontOpr将被放入"ptEncodingOpr->ptFontOprSupportedHead"链表中
 * 输入参数： ptEncodingOpr - 编码方式
 *            ptFontOpr     - 字符处理结构体,内含"GetFontBitmap"函数(即根据编码值获得位图)
 * 输出参数： 无
 * 返 回 值： 0   - 成功
 *            -1  - 失败
 ***********************************************************************/
int AddFontOprForEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptFontOprCpy;
	
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
        /* 分配一个新的T_FontOpr结构体 */
		ptFontOprCpy = malloc(sizeof(T_FontOpr));
		if (!ptFontOprCpy)
		{
			return -1;
		}
		else
		{
            /* 复制结构体里的内容 */
			memcpy(ptFontOprCpy, ptFontOpr, sizeof(T_FontOpr));

            /* 把新结构体放入ptEncodingOpr->ptFontOprSupportedHead链表 
             * 为何不直接把原来的ptFontOpr放入链表?
             * 因为原来的ptFontOpr->ptNext已经在RegisterFontOpr函数中被占用了
             */
			ptFontOprCpy->ptNext = ptEncodingOpr->ptFontOprSupportedHead;
			ptEncodingOpr->ptFontOprSupportedHead = ptFontOprCpy;
			return 0;
		}		
	}
}


/**********************************************************************
 * 函数名称： DelFontOprFrmEncoding
 * 功能描述： AddFontOprForEncoding的反函数,从PT_EncodingOpr->ptFontOprSupportedHead链表中把某个PT_FontOpr去掉
 * 输入参数： ptEncodingOpr - 编码方式
 *            ptFontOpr     - 字符处理结构体,内含"GetFontBitmap"函数(即根据编码值获得位图)
 * 输出参数： 无
 * 返 回 值： 0  - 成功
 *            -1 - 失败
 ***********************************************************************/
int DelFontOprFrmEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTmp;
	PT_FontOpr ptPre;
		
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptTmp = ptEncodingOpr->ptFontOprSupportedHead;
		if (strcmp(ptTmp->name, ptFontOpr->name) == 0)
		{
			/* 删除头节点 */
			ptEncodingOpr->ptFontOprSupportedHead = ptTmp->ptNext;
			free(ptTmp);
			return 0;
		}

		ptPre = ptEncodingOpr->ptFontOprSupportedHead;
		ptTmp = ptPre->ptNext;
		while (ptTmp)
		{
			if (strcmp(ptTmp->name, ptFontOpr->name) == 0)
			{
				/* 从链表里取出、释放 */
				ptPre->ptNext = ptTmp->ptNext;
				free(ptTmp);
				return 0;
			}
			else
			{
				ptPre = ptTmp;
				ptTmp = ptTmp->ptNext;
			}
		}

		return -1;
	}
}

/**********************************************************************
 * 函数名称： EncodingInit
 * 功能描述： 调用各种编码方式的初始化函数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int EncodingInit(void)
{
	int iError;

	iError = AsciiEncodingInit();
	if (iError)
	{
		DBG_PRINTF("AsciiEncodingInit error!\n");
		return -1;
	}

	iError = Utf16leEncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf16leEncodingInit error!\n");
		return -1;
	}
	
	iError = Utf16beEncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf16beEncodingInit error!\n");
		return -1;
	}
	
	iError = Utf8EncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf8EncodingInit error!\n");
		return -1;
	}

	return 0;
}

