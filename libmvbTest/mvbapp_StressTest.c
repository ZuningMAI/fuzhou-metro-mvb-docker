#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "mvb_pressure.h"

#include "mvbapi.h"
#include "mvbapp.h"
#include "mvbmsg.h"

#define MVB_SRC_POART_MAX_NUM 50  				// MVB源端口最大数量
#define MVB_SNK_POART_MAX_NUM 60  				// MVB宿端口最大数量

static T_PORT_INFO s_tMvbSnkPortTable[MVB_SNK_POART_MAX_NUM];  // 存储宿端口的信息
static T_PORT_INFO s_tMvbSrcPortTable[MVB_SRC_POART_MAX_NUM];  // 存储源端口的信息

unsigned short usSrcMaxPortNum = 0; 	//源端口数量
unsigned short usSinkMaxPortNum = 0;	//宿端口数量


static char g_cMvbStatus = E_MVB_UNREADY;



int GetPressMvbPortTable(void)
{
	uint16_t i = 0;
	int ret = 0;
	memset((uint8_t*)&s_tMvbSnkPortTable,0,sizeof(T_PORT_INFO) * MVB_SNK_POART_MAX_NUM);
	memset((uint8_t*)&s_tMvbSrcPortTable,0,sizeof(T_PORT_INFO) * MVB_SRC_POART_MAX_NUM);

	/*源端口数据获取*/
	for(i = 0 ; i < usSrcMaxPortNum ; i++)
	{
		s_tMvbSrcPortTable[i].ucPortType = 1;
		ret = MvbPress_GetPortParam(i,s_tMvbSrcPortTable[i].ucPortType,
							&s_tMvbSrcPortTable[i].usPortAddr,&s_tMvbSrcPortTable[i].ucPortLen,&s_tMvbSrcPortTable[i].usPortCycle);
		if(ret != 0)
		{
			return  ret;
		}
	}

	/*宿端口数据获取*/
	for(i = 0 ; i < usSinkMaxPortNum ; i++)
	{
		s_tMvbSnkPortTable[i].ucPortType = 0;
		ret = MvbPress_GetPortParam(i,s_tMvbSnkPortTable[i].ucPortType,
							&s_tMvbSnkPortTable[i].usPortAddr,&s_tMvbSnkPortTable[i].ucPortLen,&s_tMvbSnkPortTable[i].usPortCycle);
		if(ret != 0)
		{
			return  ret;
		}
	}

	printf("SrcPortTable: \n");
	for(int j = 0;j < usSrcMaxPortNum;j++)
	{
		printf(" Port: %d,ADDR %02x ",j,s_tMvbSrcPortTable[j].usPortAddr);
	}
	printf("\n");
	for(int j = 0;j < usSinkMaxPortNum;j++)
	{
		printf(" Port: %d,ADDR %02x ",j,s_tMvbSnkPortTable[j].usPortAddr);
	}
	printf("\n");
	
	return ret;
}

char MVBStress_GetMvbStatus()
{
    return g_cMvbStatus;
}

int MVBStress_SetMvbStatus(char cStatus)
{
    g_cMvbStatus = cStatus;
    return 0;
}

void *MvbProcThread_Stress(void *argv)
{
    char acData[32];	//端口数据
    int iTryCount;
    int i = 0;
    int iRet = 0;
	int Tmp = 0;
    unsigned char u8Freshness = 0;
    char cMvbStatus = 0;

	usSrcMaxPortNum = MvbPress_GetSrcPortNum();
	usSinkMaxPortNum = MvbPress_GetSnkPortNum();
	printf("usSrcMaxPortNum: %d ,usSinkMaxPortNum: %d\r\n",usSrcMaxPortNum,usSinkMaxPortNum);

	/*获取端口配置信息*/
	GetPressMvbPortTable();
	
	printf("start mvb app\r\n");

    iRet = MVB_Init("/dev/ttyS4");//MVB板卡连接端口初始化
    if(iRet < 0)
    {
        printf("MVB_Init error iRet:%d\n",iRet);
        return NULL;
    }
	
	MVB_Reset();
	usleep(600000);
	
    /*获取MVB版本信息*/
	iTryCount = 10;
	while(iTryCount--)
    {
		iRet = MVB_GetVersion();
		if(iRet < 0)
		{
			printf("MVB_GetVersion error. TryCount %d , iRet:%d\n",10-iTryCount,iRet);
			continue;
		}
		else
		{
			break;
		}
	}
    
	/*下发MVB设备配置*/
	iRet = MVB_DeviceConfig(2);
	if(iRet < 0)
	{
		printf("MVB_DeviceConfig error iRet:%d\n",iRet);
		return NULL;
	}
	
	/*端口注册*/
	iTryCount = 3;
    while(iTryCount--)
    {
		iRet += MVB_RegisterPort(usSrcMaxPortNum, s_tMvbSrcPortTable);
		iRet += MVB_RegisterPort(usSinkMaxPortNum, s_tMvbSnkPortTable);
		usleep(256000);
        
		printf("MVB_RegisterPort Times : %d\n",iTryCount);
        if (iRet < 0)
        {
            printf("MVB_RegisterPort error : %d\n",iRet);
            continue;
        }
        else
        {
            break;
        }
    }
    if(iRet < 0)
    {
        printf("MVB_RegisterPort failed\n");
        MVBStress_SetMvbStatus(E_MVB_REGISTER_FAILER);
        return NULL;
    }
	MVBStress_SetMvbStatus(E_MVB_STATUS_OK);
    printf("MVB_RegisterPort success\n");
    sleep(2);
   
	/*开始MVB通信*/
	iRet = MVB_CommCtrl();
	if(iRet < 0)
	{
		printf("MVB_CommCtrl error iRet:%d\n",iRet);
		return NULL;
	}
	
    while(1)
    {
    	  cMvbStatus = MVBStress_GetMvbStatus();//获取MVB状态
    	  if (cMvbStatus != E_MVB_STATUS_OK)
    	  {
		  	MVB_Reset();
			usleep(600000);
			
		  	/*下发MVB设备配置*/
			iRet = MVB_DeviceConfig(2);
			if(iRet < 0)
			{
				printf("MVB_DeviceConfig error iRet:%d\n",iRet);
				continue;
			}
			
            iRet += MVB_RegisterPort(usSrcMaxPortNum, s_tMvbSrcPortTable);
			iRet += MVB_RegisterPort(usSinkMaxPortNum, s_tMvbSnkPortTable);
            if (iRet < 0)
            {
                printf("mvb register failed\n");
                sleep(2);
                continue;
            }
            sleep(2);
            MVBStress_SetMvbStatus(E_MVB_STATUS_OK);
            printf("mvb register restore success\n");

			iRet = MVB_CommCtrl();
			if(iRet < 0)
			{
				printf("MVB_CommCtrl error iRet:%d\n",iRet);
				continue;;
			}
    	  }     

		/*遍历源端口，读取数据并写入（每次只写入一个端口数据）*/
		memset(acData, 0, sizeof(acData));
		for(i = 0; i < usSrcMaxPortNum ; i++)
		{
			//刷新源端口数据
			MvbPress_RefreshSrcPortData(s_tMvbSrcPortTable[i].usPortAddr, acData,s_tMvbSrcPortTable[i].ucPortLen);
			PortData_printf( "MVB Write PortData",acData, s_tMvbSrcPortTable,i);
			
			iRet = MVB_WriteSrcPD(s_tMvbSrcPortTable[i].usPortAddr, s_tMvbSrcPortTable[i].ucPortLen,acData);
	    	if (iRet < 0)
	        {
	            printf("mvbwirte error \n");
	            MVBStress_SetMvbStatus(E_MVB_WRITE_FAILER);
	            break;
	   	    }
		}

		/*遍历宿端口，读取数量与地址并写入缓存*/
		memset(acData, 0, sizeof(acData));
        for (i = 0; i < usSinkMaxPortNum; i++)
        {
			iRet = MVB_ReadSinkPD(s_tMvbSnkPortTable[i].usPortAddr, acData, &u8Freshness);
			PortData_printf( "MVB Read PortData",acData, s_tMvbSnkPortTable,i);
			
        	if (iRet < 0)
            {
               printf("mvbread error %d, 0x%x len %d, ret %d\n", __LINE__, s_tMvbSnkPortTable[i].usPortAddr,s_tMvbSnkPortTable[i].ucPortLen, iRet);
               MVBStress_SetMvbStatus(E_MVB_WRITE_FAILER);
               break;
       	    }
			iRet = MvbPress_ProcessSnkPortData(0,s_tMvbSnkPortTable[i].usPortAddr,acData,s_tMvbSnkPortTable[i].usPortAddr,u8Freshness);
			if (iRet < 0)
			{
				printf("  MvbPress_ProcessSnkPortData EEROR ADDR: %04x \n",s_tMvbSnkPortTable[i].usPortAddr);
			}
        }
    
        usleep(256000);
    }
    
    return NULL;
}

int MVBAPP_Stress_Init()
{
    pthread_t tid;
    pthread_create(&tid, NULL, MvbProcThread_Stress, NULL);
    return 0;
}


