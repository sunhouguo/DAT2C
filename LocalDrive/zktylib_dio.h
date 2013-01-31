
#ifndef  _ZKTY_LIB_DIO_HEADER_
#define  _ZKTY_LIB_DIO_HEADER_

#ifdef __cplusplus
extern "C" 
{
#endif
#include <sys/types.h>

#define zkty_evt_diif0     0x1000
#define zkty_evt_diif1     (zkty_evt_diif0 +1)
#define zkty_evt_diif2     (zkty_evt_diif0 +2)
#define zkty_evt_diif3     (zkty_evt_diif0 +3)
#define zkty_evt_diif4     (zkty_evt_diif0 +4)
#define zkty_evt_diif5     (zkty_evt_diif0 +5)
#define zkty_evt_extif     (zkty_evt_diif0 +6)
#define zkty_evt_irigb     (zkty_evt_diif0 +7)

long   zkty_dio_open(long DevNum);
void   zkty_dio_close(long hdevice);

int    zkty_diread(long hdevice,unsigned char* pbuf);
int    zkty_dowrite(long hdevice,unsigned char* pbuf);
int    zkty_enable_event(long hdevice,unsigned char* enable,unsigned char* threshold);
int    zkty_check_event(long hdevice,unsigned int EventType,unsigned int Milliseconds);
  
#ifdef __cplusplus
}
#endif


#endif
