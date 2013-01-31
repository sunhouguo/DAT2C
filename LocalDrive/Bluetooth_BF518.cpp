

#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>
//#include <boost/bind.hpp>
#include "Bluetooth_BF518.h"

#if defined(_BF518_)	
#include <termios.h>
#endif

//#include <fstream>


unsigned char NameBuf[40];//={0x01,0xFC,0x12,0x0B,0x53,0x75,0x6e,0x53,0x68,0x75,0x79,0x75,0x6e,0x5f,0x33};

namespace LocalDrive {

#define GOTOSET      0
#define OUTSET       1
#define NameHead     "PDZ800-"

CBluetooth_BF518::CBluetooth_BF518(/*boost::asio::io_service & io_service,*/bool  bEnableLog,/*DataBase::CSubStation & sub*/typeAddr addr)
	:CBluetooth(bEnableLog)
//	io_service_(io_service),
//	serial_port_(io_service),
//	sub_(sub)
{

	memset(NameBuf,0,40);
	memset(Name,0,15);

	id_ = boost::lexical_cast<std::string>(addr);
}

CBluetooth_BF518::~CBluetooth_BF518(void)
{
}

int CBluetooth_BF518::run()
{
	return OpenBluetooth();
}

int CBluetooth_BF518::stop()
{
	CloseBluetooth();

	return 0;
}

int CBluetooth_BF518::reset()
{
	return 0;
}

int CBluetooth_BF518::Set_ttyBF0(int fd, int speed)
{
#if defined(_BF518_)	
	int   status;
	struct termios Opt;

	tcgetattr(fd, &Opt); 

	cfsetispeed(&Opt, speed);
	cfsetospeed(&Opt, speed);

	Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
	Opt.c_iflag &= ~(IXON | IXOFF | IXANY); 
	Opt.c_iflag &= ~(INLCR | IGNCR | ICRNL);
	Opt.c_oflag &= ~(INLCR|IGNCR|ICRNL);
	Opt.c_oflag &= ~(ONLCR|OCRNL); 

	status = tcsetattr(fd, TCSANOW, &Opt); 
	if  (status != 0)
	{
		perror("tcsetattr fd1");
		return -1;
	}
#endif //defined(_BF518_)	

	return 0;
}

//int CBluetooth_BF518::OpenBluetooth()
//{
//#if defined(_BF518_)
//	int NameLength = GetNameBuf();
//
//	BluetoothHandle_ = open("/dev/Buletooth",O_WRONLY);
//
//	if (BluetoothHandle_ > 0)
//	{
//	  ioctl(BluetoothHandle_,GOTOSET);
//	  
//	  int fd = open("/dev/ttyBF0",O_RDWR);
//	  if (fd > 0)
//	  {
//		  Set_ttyBF0(fd,B9600);
//		  //使用id_写数据
//
//		  usleep(5000);
//		  write(fd,NameBuf,NameLength);
//		  usleep(5000);
//
//		  close(fd);
//	  }
//
//	  ioctl(BluetoothHandle_,OUTSET);
//	  return 0;
//	}
//#endif //defined(_BF518_)	
//
//	return -1;
//}

int CBluetooth_BF518::OpenBluetooth()
{
#if defined(_BF518_)
	int NameLength = GetNameBuf();

	BluetoothHandle_ = open("/dev/Buletooth",O_WRONLY);

	if (BluetoothHandle_ > 0)
	{
		ioctl(BluetoothHandle_,GOTOSET);

		int fd = open("/dev/ttyBF0",O_RDWR);
		if (fd > 0)
		{
			Set_ttyBF0(fd,B9600);
			//使用id_写数据

			usleep(5000);
			write(fd,NameBuf,NameLength);
			usleep(5000);

			close(fd);
		}

		ioctl(BluetoothHandle_,OUTSET);
		close(BluetoothHandle_);

		BluetoothHandle_ = open("/dev/Buletooth",O_WRONLY);
		ioctl(BluetoothHandle_,GOTOSET);
		usleep(5000);

		return 0;
	}
#endif //defined(_BF518_)	

	return -1;
}

int CBluetooth_BF518::GetNameBuf()
{
	int count = 0;
	int LengthLocation = 3;
	char * Data = 0;

	 std::stringstream ss;
	 std::string str;

	ss<<NameHead<<id_;
	ss>>str;

	strcpy(Name,str.c_str());

	int Lenth = str.length();

	//std::cout<<"现在的buf为："<<Name<<std::endl;

	NameBuf[count ++] = 0x01;
	NameBuf[count ++] = 0xFC;
	NameBuf[count ++] = 0x12;
	NameBuf[count ++] = Lenth;//数据长度

	for (int i = 0; i < Lenth; i ++)
	{
		NameBuf[count ++ ] = Name[i];
	}

	return count;
}

//unsigned int CBluetooth_BF518::TransStringToInt(std::string id)
//{
//    unsigned int ID_;
//	std::stringstream ss;
//	ss<<id;
//	ss>>ID_;
//	return ID_;
//}

void CBluetooth_BF518::CloseBluetooth()
{
	if(BluetoothHandle_ >= 0)
	{
#if defined(_BF518_)
		close(BluetoothHandle_);
#endif //defined(_BF518_)	

		BluetoothHandle_ = -1;
	}
}

//void CBluetooth_BF518::handle_write(const boost::system::error_code& error,size_t bytes_transferred)
//{
//	if(error == boost::asio::error::operation_aborted)
//	{
//		//通道被其他操作关闭了,谁关的谁负责重连，do nothing
//		//AddStatusLogWithSynT("Serial通道写数据过程中发现通道被关闭，等待重连接。\n");
//		return;
//	}
//	else if(error)
//	{
//		//或者是其他系统抛出的错误，没分类处理，还是重连吧
//		//AddStatusLogWithSynT("Serial通道写数据过程中发生一个未知错误，尝试重连接通道。\n");
//	}
//}

}//namespace LocalDrive



