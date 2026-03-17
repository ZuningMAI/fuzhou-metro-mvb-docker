#ifndef __MVB_APP_H__
#define __MVB_APP_H__

enum _E_MVB_STATUS
{
    E_MVB_STATUS_OK       = 0,
    E_MVB_UNREADY         = 1,
    E_MVB_REGISTER_FAILER = 2,
    E_MVB_WRITE_FAILER    = 3,
    E_MVB_READ_FAILER     = 4,
};


int MVBAPP_Init();
int MVBAPP_Stress_Init();

int MVBAPP_Uninit();

int MVBAPP_GetMvbData(unsigned short usPortAddr, char *pcData, int iLen);
int MVBAPP_SetMvbData(unsigned short usPortAddr, char *pcData, int iLen);

char MVBAPP_GetMvbStatus();
int MVBAPP_SetMvbStatus(char cStatus);
#endif
