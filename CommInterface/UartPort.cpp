#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include "UartPort.h"
#include "../Protocol/Protocol.h"
#include "../FileSystem/Markup.h"
#include "CommPara.h"


namespace CommInterface {

const std::string strSerialCfgName = "SerialPort.xml";

////xml结点定义
#define strSerialRoot "SerialPort"
#define strBaudRate "BaudRate"
#define strDataBits "DataBits"
#define strStopBits "StopBits"
#define strStopBitOne "1"
#define strStopBitOnePointFive "1.5"
#define strStopBitTwo "2"
#define strParity "Parity"
#define strParityNone "none"
#define strParityOdd "odd"
#define strParityEven "even"
#define strFlowControl "FlowControl"
#define strFlowControlNone "none"
#define strFlowControlSoftware "software"
#define strFlowControlHardware "hardware"

//seiral prot default para
#define Default_BaudRate 9600
#define Default_DataBits 8
#define Default_StopBits boost::asio::serial_port::stop_bits::one
#define Default_Parity boost::asio::serial_port::parity::none
#define Default_FlowControl boost::asio::serial_port::flow_control::none
#define Default_LcrVal 0x03

CUartPort::CUartPort(CCommPara & para,boost::asio::io_service& io_service)
	:CCommInterface(para,io_service),
	//serial_port_(io_service),
	//timerRecv_(io_service,boost::posix_time::minutes(recv_time_out)),
	timerRetry_(io_service,boost::posix_time::seconds(retry_time_out))
{
	retryTimes_ = 0;
	port_no_ = para.getLocalPort();
    LCR_Val = Default_LcrVal;
	uart_fd = -1;
}

CUartPort::~CUartPort(void)
{
	if (getConnectAlive())
	{
		CloseConnect();
	}

	UninitConnect();
}

int CUartPort::InitSerialPort()
{
	if (getConnectAlive())
	{
#if defined(_BF518_)
		ioctl(uart_fd,Default_BaudRate,3);
#endif //#if defined(_BF518_)

		if (LoadSerialCfg(strSerialCfgName))
		{
			AddStatusLogWithSynT("未能从配置文件初始化串口，使用默认值初始化。\n");
		}

		return 0;
	}

	return -1;
}

void CUartPort::SaveSerialAttrib(FileSystem::CMarkup & xml)
{
	xml.AddElem(strBaudRate,BaudRate_);
}

void CUartPort::SaveSerialCfg(std::string fileName)
{
	std::vector<std::string> strVec;
	boost::algorithm::split(strVec,port_no_,boost::algorithm::is_any_of("/"));

	if (strVec.size() < 1)
	{
		return;
	}

	std::string port_node = *(strVec.end() - 1);

	FileSystem::CMarkup xml;

	if (xml.Load(fileName))
	{
		xml.ResetMainPos();
		xml.FindElem();  //root strSerialPort
		xml.IntoElem();  //enter strSerialPort

		xml.ResetMainPos();
		if (xml.FindElem(port_node))
		{
			xml.RemoveElem();

			xml.AddElem(port_node);
			xml.IntoElem();                  //enter port_no_

			SaveSerialAttrib(xml);

			xml.OutOfElem();                 //out port_no_
		}
		else
		{
			xml.AddElem(port_node);
			xml.IntoElem();                  //enter port_no_

			SaveSerialAttrib(xml);

			xml.OutOfElem();                 //out port_no_
		}


		xml.OutOfElem();  //out strSerialPort
	}
	else
	{
		xml.SetDoc(strXmlHead);
		xml.SetDoc(strSerialXsl);

		xml.AddElem(strSerialRoot);
		xml.IntoElem();                  //enter strSerialRoot

		xml.AddElem(port_node);
		xml.IntoElem();                  //enter port_no_

		SaveSerialAttrib(xml);

		xml.OutOfElem();                 //out port_no_

		xml.OutOfElem();                 //out strSerialRoot
	}
}

int CUartPort::LoadSerialCfg(std::string fileName)
{
	int ret = -1;

	FileSystem::CMarkup xml;

	if (!xml.Load(fileName))
	{
		return ret;
	}

	xml.ResetMainPos();
	xml.FindElem();  //root strSerialPort
	xml.IntoElem();  //enter strSerialPort

	std::vector<std::string> strVec;
	boost::algorithm::split(strVec,port_no_,boost::algorithm::is_any_of("/"));

	if (strVec.size() < 1)
	{
		return ret;
	}

	std::string port_node = *(strVec.end() - 1);

	xml.ResetMainPos();
	if (xml.FindElem(port_node))
	{
		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem(strDataBits))
		{
			//unsigned short databits = Default_DataBits;

			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short databits = boost::lexical_cast<unsigned short>(strTmp);
           
				if (databits >= 4 && databits <= 8)
				{
					LCR_Val = LCR_Val & 0xFC;

					if (databits == 5)
					{
						LCR_Val = (LCR_Val | 0x00);
					} 
					else if(databits == 6)
					{
						LCR_Val = (LCR_Val | 0x01);
					}
					else if(databits == 7)
					{
						LCR_Val = (LCR_Val | 0x02);
					}
					else if(databits == 8)
					{
						LCR_Val = (LCR_Val | 0x03);
					}
					ret = 0;
				}
				else
				{
					std::ostringstream ostr;
					ostr<<"非法的串口数据位参数："<<databits<<"，将使用默认值\n";
					AddStatusLogWithSynT(ostr.str());
				}
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<"非法的串口数据位参数："<<e.what()<<"，将使用默认值\n";
				AddStatusLogWithSynT(ostr.str());
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strStopBits))
		{
			//float stopbits = 1;
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				float stopbits = boost::lexical_cast<float>(strTmp);

				if (stopbits == 1)
				{
					LCR_Val = (LCR_Val | 0x00);
					ret = 0;
				}
				else if(stopbits == 1.5)
				{
					LCR_Val = (LCR_Val | 0x04);
					ret = 0;
				}
				else if (stopbits == 2)
				{
					LCR_Val = (LCR_Val | 0x04);
					ret = 0;
				}
				else
				{
					std::ostringstream ostr;
					ostr<<"非法的串口停止位参数："<<stopbits<<"，将使用默认值\n";
					AddStatusLogWithSynT(ostr.str());
				}
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<"非法的串口停止位参数："<<e.what()<<"，将使用默认值\n";
				AddStatusLogWithSynT(ostr.str());
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strParity))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);

			if (boost::iequals(strTmp,strParityNone))
			{
                LCR_Val = (LCR_Val | 0x00);
				ret = 0;
			}
			else if (boost::iequals(strTmp,strParityOdd))
			{
				LCR_Val = (LCR_Val | 0x08);
				ret = 0;
			}
			else if (boost::iequals(strTmp,strParityEven))
			{
			    LCR_Val = (LCR_Val | 0x18);
				ret = 0;
			}
			else
			{
				std::ostringstream ostr;
				ostr<<"非法的串口校验位参数："<<strTmp<<"，将使用默认值\n";
				AddStatusLogWithSynT(ostr.str());
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strBaudRate))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BaudRate_ = boost::lexical_cast<unsigned int>(strTmp);

				if (BaudRate_ >= 300 && BaudRate_ <= 921600)
				{
                    #if defined(_BF518_)
					ioctl(uart_fd,BaudRate_,LCR_Val);
					ret = 0;
                    #endif //#if defined(_BF518_)
				}
				else
				{
					std::ostringstream ostr;
					ostr<<"非法的串口波特率参数："<<BaudRate_<<"，将使用默认值\n";
					AddStatusLogWithSynT(ostr.str());
				}
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<"非法的串口波特率参数："<<e.what()<<"，将使用默认值\n";
				AddStatusLogWithSynT(ostr.str());
			}
		}

		xml.OutOfElem();
	}

	xml.OutOfElem();

	return ret;
}

int CUartPort::InitConnect(void)
{
	EnableLog();

	int ret = 0;

	ret = EnableProtocol();
	if (ret)
	{
		AddStatusLogWithSynT("Serial通道初始化规约失败！\n");
		return ret;
	}

	AddStatusLogWithSynT("初始化Serial通道成功！\n");

	return ret;
}

void CUartPort::UninitConnect( void )
{
	//timerRecv_.cancel();
	timerRetry_.cancel();

	DisableProtocol();

	DisableLog();

	AddStatusLogWithSynT("反初始化Serial通道！\n");
}

int CUartPort::OpenConnect(void)
{

# if defined (_BF518_)
	uart_fd = open(port_no_.c_str(),O_RDWR);
#endif //# if defined (_BF518_)

	if (!uart_fd)
	{
		std::ostringstream ostr;
		ostr<<"Serial通道尝试打开"<<port_no_<<"失败"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		ResetTimerRetry(true);

		return -1;
	}

	retryTimes_ = 0;
	ResetTimerRetry(false);

	if (InitSerialPort())
	{
		std::ostringstream ostr;
		ostr<<"Serial通道尝试初始化"<<port_no_<<"失败。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
	}

	InitProtocol();

	std::ostringstream ostr;
	ostr<<"Serial通道打开"<<port_no_<<"成功。"<<std::endl;
	AddStatusLogWithSynT(ostr.str());

	ReadFromConnect(recv_data_);

	return 0;
}

int CUartPort::CloseConnect(void)
{
	//timerRecv_.cancel();

#if defined(_BF518_)
	if(close(uart_fd))
	{
		std::ostringstream ostr;
		ostr<<"Serial通道尝试关闭"<<port_no_<<"失败"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}
#endif //#if defined(_BF518_)

	UninitProtocol();

	std::ostringstream ostr;
	ostr<<"Serial通道关闭"<<port_no_<<"成功。"<<std::endl;
	AddStatusLogWithSynT(ostr.str());

	return 0;
}

void CUartPort::UartRecvThread(unsigned char * buf)
{
	boost::asio::deadline_timer blockTimer(io_service_,boost::posix_time::milliseconds(40));//milliseconds  microseconds

#if defined (_BF518_)

	while (true)
	{
		usleep(30000);
		int bytes_transferred = read(uart_fd,buf,1024);

		if (bytes_transferred > 0)
		{
			RecordFrameData(buf,bytes_transferred,true);

			//调用CProtocol模块处理recvbuf中bytes_transferred字节的报文
			if (protocol_)
			{
				protocol_->RecvProcess(buf,bytes_transferred);
			}
		}
		else
		{
			blockTimer.wait();
		}
	}

#endif //#if defined (_BF518_)

}

int CUartPort::ReadFromConnect(unsigned char * buf, size_t bytes_transferred /* = 0 */)
{

	boost::thread thrd(boost::bind(&CUartPort::UartRecvThread,this,buf));
	//thrd.join();
	  
	return 0;
}

int CUartPort::WriteToConnect(const unsigned char * buf, size_t bytes_transferred /* = 0 */)
{
	if (getConnectAlive())
	{
		RecordFrameData(buf,bytes_transferred,false);
        
#if defined(_BF518_)

		write(uart_fd,buf,bytes_transferred);

#endif //#if defined(_BF518_)
	}

	return 0;
}

int CUartPort::RetryConnect(void)
{
	if (getConnectAlive())
	{
		CloseConnect();
	}

	OpenConnect();

	return 0;
}

//void CUartPort::handle_read( unsigned char * buf,const boost::system::error_code& error,size_t bytes_transferred )
//{
//	if(!error)
//	{
//		timerRecv_.cancel();
//
//		RecordFrameData(buf,bytes_transferred,true);
//
//		//调用CProtocol模块处理recvbuf中bytes_transferred字节的报文
//		if (protocol_)
//		{
//			protocol_->RecvProcess(buf,bytes_transferred);
//		}
//
//		//再继续读数据
//		ReadFromConnect(buf);
//	}
//	else if (error == boost::asio::error::operation_aborted)
//	{
//		//通道被其他操作关闭了,谁关的谁负责重连，do nothing
//		AddStatusLogWithSynT("Serial通道读数据过程中发现通道被关闭，等待重连接。\n");
//		return;
//	}
//	else
//	{
//		//或者是其他系统抛出的错误，没分类处理，还是重连吧
//		AddStatusLogWithSynT("Serial通道读数据过程中发生一个未知错误，尝试重连接通道。\n");
//		RetryConnect();
//	}
//}

void CUartPort::handle_write(const boost::system::error_code& error,size_t bytes_transferred)
{
	if(error == boost::asio::error::operation_aborted)
	{
		//通道被其他操作关闭了,谁关的谁负责重连，do nothing
		AddStatusLogWithSynT("Serial通道写数据过程中发现通道被关闭，等待重连接。\n");
		return;
	}
	else if(error)
	{
		//或者是其他系统抛出的错误，没分类处理，还是重连吧
		AddStatusLogWithSynT("Serial通道写数据过程中发生一个未知错误，尝试重连接通道。\n");
		RetryConnect();
	}
}

void CUartPort::handle_timerRecv(const boost::system::error_code& error)
{
	if (!error)
	{
		//超过recv_time_out时间没有收到任何报文，重新连接通道。
		std::ostringstream ostr;
		ostr<<"Serial通道已经"<<recv_time_out<<"分钟时间未收到任何报文，尝试重连接通道。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		RetryConnect();
	}
}

void CUartPort::handle_timerRetry(const boost::system::error_code& error)
{
	if (!error)
	{
		//超过retry_time_out时间,重新尝试打开串口
		std::ostringstream ostr;
		ostr<<"Serial通道尝试打开串口"<<port_no_<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		RetryConnect();
	}
}

void CUartPort::ResetTimerRetry(bool bContinue)
{
	if (bContinue)
	{
		if (retryTimes_++ > retry_times)
		{
			timerRetry_.expires_from_now(boost::posix_time::minutes(reconnect_time_out));
			retryTimes_ = 0;
		}
		else
		{
			timerRetry_.expires_from_now(boost::posix_time::seconds(retry_time_out));
		}

		timerRetry_.async_wait(boost::bind(&CUartPort::handle_timerRetry,
			this,//shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		timerRetry_.cancel();
	}
}

} //namespace CommInterface



