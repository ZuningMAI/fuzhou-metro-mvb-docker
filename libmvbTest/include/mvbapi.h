#ifndef __UART_MVB_API_H__
#define __UART_MVB_API_H__
#include "mvbmsg.h"

extern "C" {

typedef struct _T_PORT_INFO
{
	unsigned short usPortAddr;	//端口地址
	unsigned char ucPortType;		//端口类型
	unsigned char ucPortLen;		//端口长度
	unsigned short usPortCycle;	//端口周期
}T_PORT_INFO;



int MVB_Init(const char * devName);
int MVB_Uninit();
void PortData_printf( char * str, char* pDat, T_PORT_INFO* pPortPram, int iProtid);
void Frame_printf( char * str, char* acBuf, int ilen);

static char* MVB_DevType(unsigned char DevType);

int MVB_FormatFrame(unsigned char * pFrame, unsigned char cmd, unsigned char ctrl, unsigned char * pdata, int datasize);
/* 获取MVb版本号 */
int MVB_GetVersion(void);
/*下发MVB配置*/
int MVB_DeviceConfig(unsigned short usDeviceAddr);
/*注册MVb端口*/
int MVB_RegisterPort(unsigned        short usPortNum, T_PORT_INFO *ptPort);
int MVB_PortCtrl(unsigned      short usPortAddr, unsigned char ucOperateMode, unsigned char ucCtrlMode);
/* 开始MVB通信 */
int MVB_CommCtrl(void);
//char MVB_GETSrcPD(unsigned short usPortAddr, unsigned char ucPortSize, char *pcData, char* acBuf, unsigned char offset );
/* 将端口缓存数据写入获取源端口数据请求帧 */
int MVB_WriteSrcPD(unsigned short usPortAddr,unsigned char ucPortLen , char* pcData);
/* 读取宿端口数据请求帧 */
int MVB_ReadSinkPD(unsigned short usPortAddr, char *pcData, unsigned char *pu8Freshness);
int MVB_Reset(void);
}

#endif
