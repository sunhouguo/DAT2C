#pragma once
#include "Bluetooth.h"
#include "../PublicSupport/Dat2cTypeDef.h"

//namespace DataBase
//{
//	class CSubStation;
//}

namespace LocalDrive {

	class CBluetooth_BF518 :
		public CBluetooth
	{
	public:
		CBluetooth_BF518(/*boost::asio::io_service & io_service,*/bool  bEnableLog,/*DataBase::CSubStation & sub*/typeAddr addr);
		virtual ~CBluetooth_BF518(void);

		int run();
		int stop();
		int reset();

	private:
		//int PortHandle_;
		//typeAddr Address_;
		//int NameLength_;

		int OpenBluetooth();
		void CloseBluetooth();
		int Set_ttyBF0(int fd, int speed);
		int GetNameBuf();
		//unsigned int TransStringToInt(std::string id);

		//int FormatNameBuf();
		//void handle_write(const boost::system::error_code& error,size_t bytes_transferred);

		
		//unsigned char NameBuf[12];//= {0x01,0xFC,0x12,0x0A,0x50,0x44,0x5a,0x38,0x30,0x30,0x2d,0x31};//PDZ800-0

		//boost::asio::io_service & io_service_;
		//boost::asio::serial_port serial_port_;

	private:
		//DataBase::CSubStation & sub_;
		std::string id_;
		char Name[15];
		int Addr_;
		int BluetoothHandle_;
	};

}; //namespace LocalDrive
