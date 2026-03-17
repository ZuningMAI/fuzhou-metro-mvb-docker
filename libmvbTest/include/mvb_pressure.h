#ifndef _MVB_PRESSURE_H_
#define _MVB_PRESSURE_H_

#include "stdint.h"

int MvbPress_Init(uint8_t u8DevNo);

int MvbPress_GetSnkPortNum(void);
int MvbPress_GetSrcPortNum(void);
int MvbPress_GetPortParam(uint16_t index,uint8_t u8PortType,uint16_t* pu16PortAddr,uint8_t* pu8PortSize,uint16_t* pu16PortCycle);

int MvbPress_ProcessSnkPortData(uint16_t u16TickDif,uint16_t u16PortAddr,uint8_t* pu8InData,uint8_t u8InLen,uint8_t u8IsFresh);
int MvbPress_RefreshSrcPortData(uint16_t u16PortAddr,uint8_t* pu8OutData,uint8_t u8Outlen);

#endif
