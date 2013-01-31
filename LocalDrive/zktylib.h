
#ifndef  _ZKTY_LIB_HEADER_
#define  _ZKTY_LIB_HEADER_

#ifdef __cplusplus
extern "C" 
{
#endif
#include <sys/types.h>


  long          zkty_deviceopen(long DevNum);
  void          zkty_deviceclose(long hdevice);

  int           zkty_diread(long hdevice,int port,int num,unsigned char* pbuf);
  int           zkty_doread(long hdevice,int port,int num,unsigned char* pbuf);
  int           zkty_dowrite(long hdevice,int port,int num,unsigned char* pbuf);
  
  unsigned char zkty_getkey(long hdevice);
  void          zkty_initscr(long hdevice);
  void          zkty_clrscr(long hdevice);
  void          zkty_setcursor(long hdevice,unsigned char data);
  void          zkty_scrmove(long hdevice,unsigned short x, unsigned short y);
  void          zkty_scrdrawbyte(long hdevice,unsigned char data);
  void          zkty_scrplotbyte(long hdevice,unsigned short x,unsigned short y,unsigned char data);
  
/*   int           zkty_counterreset(long hdevice,int counter); */
/*   int           zkty_counterstart(long hdevice,int counter,int mode); */
/*   int           zkty_counterread(long hdevice,int counter); */

/*   int           zkty_enableintr(long hdevice,unsigned long  eventtype); */
/*   int           zkty_disableintr(long hdevice,unsigned long eventtype); */
/*   int           zkty_checkintr(long hdevice,unsigned long * peventtype); */
  

#ifdef __cplusplus
}
#endif


#endif
