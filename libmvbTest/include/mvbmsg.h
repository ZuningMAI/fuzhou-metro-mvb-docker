#ifndef __UART_MVB_MSG_H__
#define __UART_MVB_MSG_H__



#define UART_MVB_START_TAG1  0xF1
#define UART_MVB_START_TAG2  0xF2

#define MVB_DATA_LEN_MAX  246	//支持长报文时：最长1400字节;不支持长报文时：最长246字节

/* 报文控制字段  bit0：并不需要确认0，否则1； bit2：主动发送0/被动响应1 */
#define CTRL_NEEDACK   0x01
#define CTRL_PASSIVE   0x02

#define MVB_DEV_ADDR   0x02		//自定义设备地址

enum E_MVB_MSG_TYPE
{
	E_MVB_GET_VERSION		= 0x01,		//设备信息
	E_MVB_GET_VERSION_ACK	= 0x81,
	E_MVB_CFG_DEV			= 0x02,		//设备配置
	E_MVB_CFG_DEV_ACK		= 0x82,
	E_MVB_PORT_REGISTER     = 0x03,		//端口注册  
	E_MVB_PORT_REGISTER_ACK = 0x83,		
	E_MVB_PORT_CTRL	    	= 0x04,		//端口控制
	E_MVB_PORT_CTRL_ACK 	= 0x84,	
	E_MVB_PORT_COMM_CTRL    = 0x05,		//通信控制  
	E_MVB_PORT_COMM_CTRL_ACK= 0x85,	
	E_MVB_SRC_WRITE_PD      = 0x06,		//写入源端口数据  
	E_MVB_SRC_WRITE_PD_ACK	= 0x86,
	E_MVB_SINK_READ_PD    	= 0x07,		//读取宿端口数据 
	E_MVB_SINK_READ_PD_ACK 	= 0x87,
	E_MVB_GET_DEV_STA      	= 0x08,		//设备状态
	E_MVB_GET_DEV_STA_ACK  	= 0x88,
	E_MVB_RESET				= 0x09,		//设备复位
	E_MVB_RESET_ACK			= 0x89,
	
	E_MVB_RESET_BOARD		= 0x90,
};

/*MVB下发设备参数数据段*/
typedef struct _T_MVB_DEVPARAM
{
	unsigned char ucDeviceAddr_Low;			//设备地址低位
	unsigned char ucDeviceAddr_High;		//设备地址高位
    unsigned char ucMvbDevType;            	//站点类型
    unsigned char ucComMode;				//通信模式
	unsigned char ucReportCycle_Low;		//上报周期低位
	unsigned char ucReportCycle_High;		//上报周期高位
	unsigned char aucDevParam[32];        	//设备参数
} __attribute__((packed)) T_MVB_DEVPARAM;

// 定义MVB端口配置的结构体  
typedef struct _T_MVB_PORT_CFG{
	unsigned char ucPortAddr_Low;        // 端口地址低位  
	unsigned char ucPortAddr_High;       // 端口地址高位  
	unsigned char ucPortParam;           // 端口参数   
} __attribute__((packed)) T_MVB_PORT_CFG;

// 定义MVB端口注册数据帧的结构体
typedef struct _T_MVB_REGISTER_FRAME{
	unsigned char ucPortNum_Low;			//端口数量低位	
	unsigned char ucPortNum_High;			//端口数量高位	
	T_MVB_PORT_CFG tPortCfg[50];			//最大50个端口  
} __attribute__((packed)) T_MVB_REGISTER_FRAME;

// MVB获取版本信息应答帧数据段的结构体
typedef struct _T_DEV_INFO_PKG
{
    unsigned short usSoftVer;             /* 固件版本号 */
    unsigned short usHardVer;             /* 硬件版本号 */   
    unsigned short usFpgaVer;             /* FPGA版本号 */
    unsigned char  ucProtoVer;            /* 协议版本号 */ 
    unsigned char  ucDevType;             /* 设备类型 */ 
    unsigned char  aucDevName[16];        /* 设备型号*/
    unsigned char  ucDevAbility;          /* 设备能力字 */  
}  __attribute__((packed))T_DEV_INFO_PKG;

#endif
