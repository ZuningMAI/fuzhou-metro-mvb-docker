/**
 * 说明：MVB压力测试逻辑模块
 * Author： Hjian
 * 使用：
 * 1.调用MvbPress_Init()初始化模块
 * 2.调用MvbPress_GetSnkPortNum()和MvbPress_GetSnkPortNum()及MvbPress_GetPortParam获取端口参数；
 * 3.根据端口周期调用MvbPress_ProcessSnkPortData()和MvbPress_RefreshSrcPortData()刷新源/宿端口数据
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mvb_pressure.h"

#define MVB_REPORT_ERROR_PORT_SIZE        8  	// MVB错误报告端口大小 
  
#define MVB_REPORT_ERROR_PORT_ADDR        0x02  // MVB错误报告端口地址
  
/* MVB端口接收错误 */ 
#define MVB_PORT_RECV_ERROR_NORMAL          1  // 正常接收错误  
#define MVB_PORT_RECV_ERROR_ADDR            2  // 地址错误  
#define MVB_PORT_RECV_ERROR_CRC             3  // CRC校验错误  
#define MVB_PORT_RECV_ERROR_NODATA_10S      4  // 10秒内无数据接收错误  
#define MVB_PORT_RECV_ERROR_NOCHANGE_10S    5  // 10秒内数据无变化错误  
  
/* MVB端口发送错误*/ 
#define MVB_PORT_SEND_ERROR_NORMAL          10 // 正常发送错误  
#define MVB_PORT_SEND_ERROR_ERROR           11 // 发送错误  
   
#define MVB_SRC_POART_MAX_NUM 50  				// MVB源端口最大数量
#define MVB_SNK_POART_MAX_NUM 60  				// MVB宿端口最大数量

static uint16_t s_u16SrcPortNum = 0;  			// 当前已使用的源端口数量
static uint16_t s_u16SnkPortNum = 0;  			// 当前已使用的宿端口数量
  
/*MVB的压力测试端口信息*/   
typedef struct _T_MVB_PRESSURE  
{  
	uint8_t u8Enable;         // 端口是否启用  
	uint8_t u8PortType;       // 端口类型  
	uint8_t u8PortSize;       // 端口大小  
	uint8_t u8PortSinkHb;     // 可能是接收端口的心跳值 
	uint16_t u16PortAddr;     // 端口地址  
	uint16_t u16PortCycle;    // 端口通信周期  
	uint32_t u32SinkPortLostTime; // 接收端口丢失时间  
	uint8_t u8SinkCycleCnt;   // 宿端口的循环周期计数
	uint8_t u8Working;        // 端口工作状态  
}T_MVB_PRESSURE;  
  
static T_MVB_PRESSURE s_tMvbPressureSnkPortTable[MVB_SNK_POART_MAX_NUM];  // 存储宿端口的信息
static T_MVB_PRESSURE s_tMvbPressureSrcPortTable[MVB_SRC_POART_MAX_NUM];  // 存储源端口的信息

#define MVBPRESS_PORTTYPE_SNK 0 // 宿端口
#define MVBPRESS_PORTTYPE_SRC 1 // 源端口

static int s_i32MvbLocAddr = 0; // MVB的本地地址 

static unsigned char s_acReportErrorBuf[MVB_REPORT_ERROR_PORT_SIZE];	//存储错误信息


/**  
 * @brief 获取校验和  
 *   
 * 根据给定的数据头和数据长度，计算数据的校验和。  
 *   
 * @param pHead 数据头指针  
 * @param len 数据长度  
 * @return 校验和  
 */ 
static unsigned char getCheckSum(const unsigned char *pHead, unsigned char len)
{
    if ( NULL == pHead || len <= 0)
    {
        return 0;
    }
    unsigned char i = 0;
    unsigned short sum = 0;
    for (i = 0; i < len; i++ )
    {
        sum += pHead[i];
    }
    return (unsigned char)(sum & 0x00FF);
}


/*记录宿端口数据处理错误计数*/
static void ReportErrorToMaster(unsigned short nPortAddr, unsigned short nErrorNo)
{
    static unsigned char    s_iErrorNum = 0;
    
    if(nErrorNo != 0)
    {
        s_iErrorNum++;
		printf("s_iErrorNum %d \n",s_iErrorNum);
    }
    memset(s_acReportErrorBuf, 0, MVB_REPORT_ERROR_PORT_SIZE);
    s_acReportErrorBuf[0] = 0xff & (s_i32MvbLocAddr * 2 >> 8);
    s_acReportErrorBuf[1] = 0xff & s_i32MvbLocAddr * 2;
    s_acReportErrorBuf[2] = 0xff & (nPortAddr >> 8);
    s_acReportErrorBuf[3] = 0xff & nPortAddr;
    s_acReportErrorBuf[4] = s_iErrorNum;
    s_acReportErrorBuf[5] = 0xff & (nErrorNo >> 8);
    s_acReportErrorBuf[6] = 0xff & nErrorNo;
    s_acReportErrorBuf[MVB_REPORT_ERROR_PORT_SIZE - 1] = getCheckSum(s_acReportErrorBuf, MVB_REPORT_ERROR_PORT_SIZE - 1);
    printf("port %d report error %d\n",nPortAddr,nErrorNo);
}

/*加入端口*/
static int MvbPress_AddPort(uint8_t u8PortType, uint16_t u16PortAddr, uint8_t u8PortSize, uint16_t u16PortCycle)
{
	T_MVB_PRESSURE* ptMvbPort = NULL;
	uint16_t* pu16PortIndex = NULL;
	if(u8PortType == MVBPRESS_PORTTYPE_SNK)
	{
		if(s_u16SnkPortNum > MVB_SNK_POART_MAX_NUM)
		{
			return -1;
		}
		pu16PortIndex = &s_u16SnkPortNum;
		ptMvbPort = &s_tMvbPressureSnkPortTable[*pu16PortIndex];
	}
	else if(u8PortType == MVBPRESS_PORTTYPE_SRC)
	{
		if(s_u16SrcPortNum > MVB_SRC_POART_MAX_NUM)
		{
			return -1;
		}
		pu16PortIndex = &s_u16SrcPortNum;
		ptMvbPort = &s_tMvbPressureSrcPortTable[*pu16PortIndex];
	}
	else
	{
		return -1;
	}
	ptMvbPort->u8PortType = MVBPRESS_PORTTYPE_SNK;
	ptMvbPort->u16PortAddr = u16PortAddr;
	ptMvbPort->u8PortSize = u8PortSize;
	ptMvbPort->u16PortCycle = u16PortCycle;
	ptMvbPort->u8Enable = 1;
	*pu16PortIndex += 1;
	return 0;
}

/*构建端口table*/
int MvbPress_CreatePortTable(void)
{
	int i = 0;
	memset((uint8_t*)&s_tMvbPressureSnkPortTable,0,sizeof(T_MVB_PRESSURE)*MVB_SNK_POART_MAX_NUM);
	memset((uint8_t*)&s_tMvbPressureSrcPortTable,0,sizeof(T_MVB_PRESSURE)*MVB_SRC_POART_MAX_NUM);
#if 0	
	for(i = 0; i < 5; i++)
	{
		MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 31 + i, 2, 32);
	}

	for(i = 0; i < 5; i++)
	{
		MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 61 + i, 4, 64);//4, 32
	}

	for(i = 0; i < 8; i++)
	{
		MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 101 + i, 8, 128);//8, 32
	}

	for(i = 0; i < 10; i++)
	{
		MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 201 + i, 16, 128);//16, 64
	}

	for(i = 0; i < 10; i++)
	{
		MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 301 + i, 32, 256);//32, 128
	}

	for(i = 0; i < 10; i++)
	{
		MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 401 + i, 8, 512);//8, 32
	}
#endif
#if 1
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 31, 2, 32);
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 61, 4, 64);
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 101, 8, 128);
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 201, 16, 128);
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 301, 32, 256);
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 401, 8, 512);
	for(i = 0; i < 12; i++)
	{
		MvbPress_AddPort(MVBPRESS_PORTTYPE_SRC, 40 * (s_i32MvbLocAddr - 1) + 1041 + i, 8, 64);
	}
	/* report error port*/
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SRC, s_i32MvbLocAddr * 2, MVB_REPORT_ERROR_PORT_SIZE, 32);
#endif
	/*MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 1, 4, 32);
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SNK, 5, 32, 32);
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SRC, 2, 2, 8);
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SRC, 3, 8, 1024);
	MvbPress_AddPort(MVBPRESS_PORTTYPE_SRC, 4, 2, 4);*/

	printf("mvb pressure sinkport : %d,srcport : %d\n",s_u16SnkPortNum,s_u16SrcPortNum);
	return 0;
}


/*获取宿端口数量*/
int MvbPress_GetSnkPortNum(void)
{
	return s_u16SnkPortNum;
}

/*获取源端口数量*/
int MvbPress_GetSrcPortNum(void)
{
	return s_u16SrcPortNum;
}


/*获取端口参数*/
int MvbPress_GetPortParam(uint16_t index,uint8_t u8PortType,uint16_t* pu16PortAddr,uint8_t* pu8PortSize,uint16_t* pu16PortCycle)
{
	uint16_t u16AssignPortNum;
	T_MVB_PRESSURE* ptMvbPort = NULL;
	
	if(u8PortType == MVBPRESS_PORTTYPE_SNK)
	{
		u16AssignPortNum = MvbPress_GetSnkPortNum();
		ptMvbPort = &s_tMvbPressureSnkPortTable[0];
	}
	else if (u8PortType == MVBPRESS_PORTTYPE_SRC)
	{
		u16AssignPortNum = MvbPress_GetSrcPortNum();
		ptMvbPort = &s_tMvbPressureSrcPortTable[0];
	}
	else
	{
		return -1;
	}
	if(index >= u16AssignPortNum)
	{
		return -1;
	}
	ptMvbPort = &ptMvbPort[index];
	
	if(!ptMvbPort->u8Enable)
	{
		//return -1;
	}
	
	*pu16PortAddr = ptMvbPort->u16PortAddr;
	*pu8PortSize = ptMvbPort->u8PortSize;
	*pu16PortCycle = ptMvbPort->u16PortCycle;
	return 0;
}

/*查找端口索引 */
static int FindPortIndex(uint8_t u8PortType,uint16_t u16PortAddr)
{
	uint16_t u16AssignPortNum;
	T_MVB_PRESSURE* ptMvbPortList = NULL;
	int i = 0;
	
	if(u8PortType == MVBPRESS_PORTTYPE_SNK)
	{
		u16AssignPortNum = MvbPress_GetSnkPortNum();
		ptMvbPortList = &s_tMvbPressureSnkPortTable[0];
	}
	else if (u8PortType == MVBPRESS_PORTTYPE_SRC)
	{
		u16AssignPortNum = MvbPress_GetSrcPortNum();
		ptMvbPortList = &s_tMvbPressureSrcPortTable[0];
	}
	else
	{
		return -1;
	}
	
	for(i = 0;i < u16AssignPortNum;i++)
	{
		if(ptMvbPortList[i].u16PortAddr == u16PortAddr)
		{
			return i;
		}
	}
	return -1;
}

static void printf_array( unsigned char* pDat, unsigned int len)
{
    int i = 0, line = 0;

    printf(">>>>> %s [%d]:", __FUNCTION__,  len);
    printf("\n  [%03d]   ", line++);
    for ( i = 0; i < len; i++ )
    {
        if ((i != 0) && (i % 16 == 0))
        {
            printf("\n  [%03d]   ", line++);
        }
        printf("%02x ", pDat[i]);

    }
    printf("\n");
    return ;
}



/*  
 * 处理宿端口数据  
 * 参数：  
 *   u16TickDif - 时间差  
 *   u16PortAddr - 端口地址  
 *   pu8InData - 输入数据指针  
 *   u8InLen - 输入数据长度  
 *   u8IsFresh - 数据是否新鲜（即是否为新接收的数据）  
 * 返回值：  
 *   成功处理返回0，否则返回错误码  
 */
int MvbPress_ProcessSnkPortData(uint16_t u16TickDif,uint16_t u16PortAddr,uint8_t* pu8InData,uint8_t u8InLen,uint8_t u8IsFresh)
{
	uint16_t u16CalcPortAddr = 0; // 用于计算端口地址的临时变量  
	int i32PortIndex = 0; 		// 端口索引  
	T_MVB_PRESSURE* ptMvbPort = NULL; // 指向MVB端口结构体的指针 

	// 查找对应宿端口的索引 
	i32PortIndex = FindPortIndex(MVBPRESS_PORTTYPE_SNK,u16PortAddr);
	if(i32PortIndex < 0)
	{
		return i32PortIndex;
	}
	// 获取对应端口的结构体指针
	ptMvbPort = &s_tMvbPressureSnkPortTable[i32PortIndex];

	// 数据是否新鲜
	if(u8IsFresh)
	{
		// 检查端口是否启用，以及输入数据长度是否与端口大小匹配 
		if((!ptMvbPort->u8Enable) || (u8InLen != ptMvbPort->u8PortSize))
		{
			return -1;
		}
		
		ptMvbPort->u8Working = 1;
		ptMvbPort->u32SinkPortLostTime = 0;

		// 根据端口大小计算端口地址
		if(ptMvbPort->u8PortSize == 2)
		{
			u16CalcPortAddr = pu8InData[0];
		}
		else
		{
			u16CalcPortAddr = pu8InData[0] | ( pu8InData[1] << 8 );
		}

		// 检查计算出的端口地址是否与预期的端口地址匹配  
		if( u16CalcPortAddr != ptMvbPort->u16PortAddr )
		{
			ReportErrorToMaster(ptMvbPort->u16PortAddr, MVB_PORT_RECV_ERROR_ADDR);
			printf_array(pu8InData,ptMvbPort->u8PortSize);
			return 1;
		}
		
		// 检查数据的CRC校验和
		if(pu8InData[ptMvbPort->u8PortSize - 1] != getCheckSum(pu8InData,ptMvbPort->u8PortSize - 1))
		{
			printf("port %04x error crc %02x / %02x\n",ptMvbPort->u16PortAddr,pu8InData[ptMvbPort->u8PortSize - 1],getCheckSum(pu8InData, ptMvbPort->u8PortSize - 1));
			printf_array(pu8InData,ptMvbPort->u8PortSize);
			ReportErrorToMaster(ptMvbPort->u16PortAddr, MVB_PORT_RECV_ERROR_CRC);
			return 1;
		}

		// 如果端口大小大于或等于8字节 
		if( (ptMvbPort->u8PortSize >= 8))
		{
			if(ptMvbPort->u8PortSinkHb != pu8InData[4])
			{
				ptMvbPort->u8PortSinkHb = pu8InData[4];	// 更新心跳字节 
				ptMvbPort->u8SinkCycleCnt = 0;			// 重置循环计数器
			}
		} 
	}
	else	
	{
		ptMvbPort->u32SinkPortLostTime += ptMvbPort->u16PortCycle;
	}
	
	// 如果端口大小大于或等于8字节  
	if((ptMvbPort->u8PortSize >= 8))  
	{  
		ptMvbPort->u8SinkCycleCnt++;  		
		if((ptMvbPort->u8SinkCycleCnt > 100)&&(ptMvbPort->u8Working))  		
		{  
			ptMvbPort->u8SinkCycleCnt = 0;  
			// 报告给主设备，端口在10秒内没有变化  
			ReportErrorToMaster(ptMvbPort->u16PortAddr, MVB_PORT_RECV_ERROR_NOCHANGE_10S);  
		}  
	}  
	
	if((ptMvbPort->u32SinkPortLostTime > 2000) && (ptMvbPort->u8Working))  
	{  
		// 标记端口为非工作状态  
		ptMvbPort->u8Working = 0;  
		ptMvbPort->u32SinkPortLostTime =  0;  
		// 报告给主设备，端口在10秒内没有接收到数据  
		ReportErrorToMaster(ptMvbPort->u16PortAddr, MVB_PORT_RECV_ERROR_NODATA_10S);  
	}  
  
	
	return 0;
}

/*刷新源端口数据*/
int MvbPress_RefreshSrcPortData(uint16_t u16PortAddr,uint8_t* pu8OutData,uint8_t u8Outlen)
{
	uint16_t u16CalcPortAddr = 0;
	int i32PortIndex = 0;
	T_MVB_PRESSURE* ptMvbPort = NULL;
	
	i32PortIndex = FindPortIndex(MVBPRESS_PORTTYPE_SRC,u16PortAddr);
	if(i32PortIndex < 0)
	{
		return i32PortIndex;
	}
	ptMvbPort = &s_tMvbPressureSrcPortTable[i32PortIndex];

	//检查端口是否启用，并且输出长度是否与端口大小一致  
	if((!ptMvbPort->u8Enable) || (u8Outlen != ptMvbPort->u8PortSize))
	{
		return -1;
	}

	//检查是否是错误输出端口
	if((s_i32MvbLocAddr * 2) == ptMvbPort->u16PortAddr)
	{
		memcpy(pu8OutData,s_acReportErrorBuf,ptMvbPort->u8PortSize);
		return 0;
	}

	//其他端口根据端口大小设置输出数据 
	if(ptMvbPort->u8PortSize == 2)
	{
		pu8OutData[0] = (0xff & ptMvbPort->u16PortAddr);
		pu8OutData[1] = pu8OutData[0];
	}
	else if(ptMvbPort->u8PortSize == 4)
	{
		pu8OutData[0] = 0xff & ( ptMvbPort->u16PortAddr);
		pu8OutData[1] = 0xff & ( ptMvbPort->u16PortAddr >> 8);
		pu8OutData[2] = i32PortIndex;
		pu8OutData[3] = getCheckSum(pu8OutData, ptMvbPort->u8PortSize - 1);
	}
	else
	{
		pu8OutData[0] = 0xff & ( ptMvbPort->u16PortAddr);
		pu8OutData[1] = 0xff & ( ptMvbPort->u16PortAddr >> 8);
		pu8OutData[2] = i32PortIndex;
		pu8OutData[3] = 0;
		pu8OutData[4] = ptMvbPort->u8PortSinkHb++;
		pu8OutData[ptMvbPort->u8PortSize - 1] = getCheckSum(pu8OutData, ptMvbPort->u8PortSize - 1);
	}
	return 0;
}


/*MVB压力测试模块初始化*/
int MvbPress_Init(uint8_t u8DevNo)
{
	s_i32MvbLocAddr = u8DevNo;
	return MvbPress_CreatePortTable();
}

