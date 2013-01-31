
#ifndef  _ZKTY_LIB_LED_HEADER_
#define  _ZKTY_LIB_LED_HEADER_

#ifdef __cplusplus
extern "C" 
{
#endif
#include <sys/types.h>


  long          zkty_led_open(long DevNum);
  void          zkty_led_close(long hdevice);

  int           zkty_ledread(long hdevice,int port,int num,unsigned char* pbuf);
  int           zkty_ledwrite(long hdevice,int port,unsigned char* pbuf);
  

#ifdef __cplusplus
}
#endif


#endif
