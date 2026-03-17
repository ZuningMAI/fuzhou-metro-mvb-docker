#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "mvbapi.h"
#include "mvbapp.h"
#include "mvbmsg.h"

// //自定义端口信息
// T_PORT_INFO g_atPortMap[] =
// {
// 	{0x202, 0, 32, 64 },
// 	{0x201, 0, 32, 64 },
// 	{0x101, 1, 16, 64 },
// 	{0x102, 1, 32, 64 },
// };

//自定义端口信息 [端口地址，端口类型，数据大小[位],周期]
// T_PORT_INFO g_atPortMap[] =
// {
//     {0x151, 1, 32, 1024 },
//     {0x152, 1, 32, 1024 },
//     {0xb1,  0, 16, 128 },
//     {0x1b0, 0, 32, 256 },
// };
T_PORT_INFO g_atPortMap[] =
    {
        {0xa1, 1, 32, 1024 },
        {0xff, 0, 8, 512 },
        {0xf1, 0, 8, 1024 },
        {0xf2, 0, 8, 1024 },
        {0xf3, 0, 8, 1024 },
        {0xf4, 0, 8, 1024 },
        {0xa0,  0, 32, 128 },
        {0x80,0,32,128},
        {0x01,0,32,128},
        {0x0d,0,32,128},
        {0x0e,0,32,128},
        {0x0f,0,32,128},
        {0x10,0,32,128}
    };

static char g_acPortData[4096][32];
static char g_cMvbStatus = E_MVB_UNREADY;

static pthread_mutex_t g_tMvbMutex;

//端口数量
unsigned short g_usMaxPortNum = sizeof(g_atPortMap) / sizeof(T_PORT_INFO);



int MVBAPP_GetMvbData(unsigned short usPortAddr, char *pcData, int iLen)
{
    if ((usPortAddr >= 4096) || (NULL == pcData) || (iLen > 32) || (iLen <= 0))
    {
        return -1;
    }

    pthread_mutex_lock(&g_tMvbMutex);
    memcpy(pcData, g_acPortData[usPortAddr], iLen);


    pthread_mutex_unlock(&g_tMvbMutex);

    return 0;
}

int                     MVBAPP_SetMvbData(unsigned short usPortAddr, char *pcData, int iLen)
{
    if ((usPortAddr >= 4096) || (NULL == pcData) || (iLen > 32) || (iLen <= 0))
    {
        return -1;
    }

    pthread_mutex_lock(&g_tMvbMutex);
    memcpy(g_acPortData[usPortAddr], pcData, iLen);
    pthread_mutex_unlock(&g_tMvbMutex);

    return 0;
}

char MVBAPP_GetMvbStatus()
{
    return g_cMvbStatus;
}

int MVBAPP_SetMvbStatus(char cStatus)
{
    g_cMvbStatus = cStatus;
    return 0;
}

void *MvbProcThread(void *argv)
{
    char acData[32];	//端口数据
    T_PORT_INFO *ptPortMap = g_atPortMap;			//源端口映射

    unsigned short usSrcMaxPortNum = 0;		//源端口数量
    unsigned short usSinkMaxPortNum = 0;	//宿端口数量
    int iTryCount = 3;	//尝试次数
    int i = 0;
    int iRet = 0;
    unsigned char u8Freshness = 0;
    char cMvbStatus = 0;
    //char acProtDataBuf[MVB_DATA_LEN_MAX];		//端口数据缓存
    //unsigned char offset;		//写入源端口数据段，端口参数偏移量

    printf("start mvb app\r\n");

    iRet = MVB_Init("/dev/ttyS4");//MVB板卡连接端口初始化
    if(iRet < 0)
    {
        printf("MVB_Init error iRet:%d\n",iRet);
        return NULL;
    }

    //MVB_Reset();
    usleep(600000);

     /*获取MVB版本信息*/
    iTryCount = 10;
    while(iTryCount--)
    {
        iRet = MVB_GetVersion();
        if(iRet < 0)
        {
            printf("**** MVB_GetVersion error. TryCount %d , iRet:%d\n",10-iTryCount,iRet);
            continue;
        }
        else
        {
            printf("MVB_GetVersion suc\n");
            break;
        }
    }

    /*下发MVB设备配置*/
    iTryCount = 10;
    while(iTryCount--)
    {
        iRet = MVB_DeviceConfig(MVB_DEV_ADDR);
        if(iRet < 0)
        {
            printf("**** MVB_DeviceConfig error. TryCount %d , iRet:%d\n",10-iTryCount,iRet);
            continue;
        }
        else
        {
            break;
        }
    }

    /*MVB端口注册*/
    iTryCount = 3;
    while(iTryCount--)
    {
        iRet = MVB_RegisterPort(g_usMaxPortNum, ptPortMap);
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
        MVBAPP_SetMvbStatus(E_MVB_REGISTER_FAILER);
        return NULL;
    }
    printf("MVB_RegisterPort success\n");
    sleep(2);
    MVBAPP_SetMvbStatus(E_MVB_STATUS_OK);

    /*开始MVB通信*/
    iRet = MVB_CommCtrl();
    if(iRet < 0)
    {
        printf("MVB_CommCtrl error iRet:%d\n",iRet);
        return NULL;
    }

    while(1)
    {
          cMvbStatus = MVBAPP_GetMvbStatus();//获取MVB状态
          if (cMvbStatus != E_MVB_STATUS_OK)
          {
            //MVB_Reset();
            usleep(600000);

            /*下发MVB设备配置*/
            iRet = MVB_DeviceConfig(MVB_DEV_ADDR);
            if(iRet < 0)
            {
                printf("MVB_DeviceConfig error iRet:%d\n",iRet);
                continue;
            }

            iRet = MVB_RegisterPort(g_usMaxPortNum, ptPortMap);
            if (iRet < 0)
            {
                printf("mvb register failed\n");
                sleep(2);
                continue;
            }
            sleep(2);
            MVBAPP_SetMvbStatus(E_MVB_STATUS_OK);
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
        for(i = 0; i < g_usMaxPortNum ; i++)
        {
            if(ptPortMap[i].ucPortType == 1)
            {
                //刷新源端口数据
                MVBAPP_GetMvbData(ptPortMap[i].usPortAddr, acData, sizeof(acData));
                //PortData_printf( "MVB Write PortData",acData, ptPortMap,i);
                //将端口缓存数据写入获取源端口数据请求帧
                iRet = MVB_WriteSrcPD(ptPortMap[i].usPortAddr, ptPortMap[i].ucPortLen,acData);
                if (iRet < 0)
                {
                    printf("***** mvbwirte error %d, 0x%x len %d, ret %d\n", __LINE__, ptPortMap[i].usPortAddr,ptPortMap[i].ucPortLen, iRet);
                    MVBAPP_SetMvbStatus(E_MVB_WRITE_FAILER);
                    break;
                }
            }
        }

        /*遍历端口，找到宿端口，读取数量与地址并写入缓存*/
        for (i = 0; i < g_usMaxPortNum; i++)
        {
            if(ptPortMap[i].ucPortType == 0)
            {
                iRet = MVB_ReadSinkPD(ptPortMap[i].usPortAddr, acData, &u8Freshness);
                //PortData_printf( "MVB_ReadSinkPD",acData, ptPortMap,i);
                if (iRet < 0)
                {
                    printf("***** mvbread error %d, 0x%x len %d, ret %d\n", __LINE__, ptPortMap[i].usPortAddr,ptPortMap[i].ucPortLen, iRet);
                    MVBAPP_SetMvbStatus(E_MVB_READ_FAILER);
                    break;
                }
                MVBAPP_SetMvbData(ptPortMap[i].usPortAddr, acData, sizeof(acData));
            }

        }
        usleep(256000);
    }
    printf("__NULL____________________\n");
    return NULL;
}

int MVBAPP_Init()
{
    pthread_t tid;
    pthread_create(&tid, NULL, MvbProcThread, NULL);
    return 0;
}

int MVBAPP_Uninit()
{
}
