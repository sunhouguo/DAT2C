#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include "CommFactory.h"
#include "CommPara.h"

#include "TcpClient.h"
#include "TcpServer.h"
#include "UdpClient.h"
#include "UdpServer.h"
#include "SerialPort.h"
#include "UartPort.h"

namespace CommInterface {

//自定义通道类型
const char COMMTYPE_TCP_CLIENT = 1;
const char COMMTYPE_TCP_SERVER = 2;
const char COMMTYPE_SERIAL_PORT = 3;
const char COMMTYPE_UDP_CLIENT = 4;
const char COMMTYPE_UDP_SERVER = 5;
const char COMMTYPE_UART_PORT = 6;

#define strTcpClienChannelVal "TcpClientChannel"
#define strTcpServerChannelVal "TcpServerChannel"
#define strUdpClientChannelVal "UdpClientChannel"
#define strUdpServerChannelVal "UdpServerChannel"
#define strSerialChannelVal "SerialPortChannel"
#define strUartChannelVal "UartPortChannel"

CCommFactory::CCommFactory(void)
{
}

CCommFactory::~CCommFactory(void)
{
}

std::string CCommFactory::TransCommChannelTypeToString(unsigned char val)
{
	std::string strTmp;
	switch (val)
	{
	case COMMTYPE_TCP_CLIENT:
		strTmp = strTcpClienChannelVal;
		break;

	case COMMTYPE_TCP_SERVER:
		strTmp = strTcpServerChannelVal;
		break;

	case COMMTYPE_SERIAL_PORT:
		strTmp = strSerialChannelVal;
		break;

	case COMMTYPE_UDP_CLIENT:
		strTmp = strUdpClientChannelVal;
		break;

	case COMMTYPE_UDP_SERVER:
		strTmp = strUdpServerChannelVal;
		break;

	case COMMTYPE_UART_PORT:
		strTmp = strUartChannelVal;
		break;

	default:
		strTmp = "";
		break;
	}

	return strTmp;
}

int CCommFactory::TransCommChannelTypeFromString( std::string val )
{
	char ret = -1;

	if (boost::iequals(strTcpClienChannelVal,val))
	{
		ret = COMMTYPE_TCP_CLIENT;
	}
	else if (boost::iequals(strTcpServerChannelVal,val))
	{
		ret = COMMTYPE_TCP_SERVER;
	}
	else if (boost::iequals(strSerialChannelVal,val))
	{
		ret = COMMTYPE_SERIAL_PORT;
	}
	else if (boost::iequals(strUdpClientChannelVal,val))
	{
		ret = COMMTYPE_UDP_CLIENT;
	}
	else if (boost::iequals(strUdpServerChannelVal,val))
	{
		ret = COMMTYPE_UDP_SERVER;
	}
	else if (boost::iequals(strUartChannelVal,val))
	{
		ret = COMMTYPE_UART_PORT;
	}

	return ret;
}

CCommInterface * CCommFactory::CreatCommInterface(CCommPara & para,boost::asio::io_service & io_service)
{
	int CommType = TransCommChannelTypeFromString(para.getCommChannelType());

	switch(CommType)
	{
	case COMMTYPE_TCP_CLIENT:
		return new CTcpClient(para,io_service);
		break;

	case COMMTYPE_UDP_CLIENT:
		return new CUdpClient(para,io_service);
		break;

	case COMMTYPE_SERIAL_PORT:
		return new CSerialPort(para,io_service);
		break;

	case COMMTYPE_UART_PORT:
		return new CUartPort(para,io_service);
		break;

	default:
		return NULL;
		break;
	}

	return NULL;

}

CServerInterface * CCommFactory::CreateServerInterface(CCommPara & para,boost::asio::io_service & io_service)
{
	try
	{
		int port = boost::lexical_cast<int>(para.getLocalPort());
		int CommType = TransCommChannelTypeFromString(para.getCommChannelType());
		switch(CommType)
		{
		case COMMTYPE_TCP_SERVER:
			return new CTcpServer(para,io_service,port);
			break;

		case COMMTYPE_UDP_SERVER:
			return new CUdpServer(para,io_service,port);
			break;

		default:
			return NULL;
			break;
		}
	}
	catch(boost::bad_lexical_cast & e)
	{
		std::cerr<<"CCommFactory::CreateServerInterface "<<e.what()<<std::endl;
		return NULL;
	}
}

bool CCommFactory::CompareCommPara(CCommPara & srcPara,CCommPara & dstPara,bool & bSever)
{
	int srcType = TransCommChannelTypeFromString(srcPara.getCommChannelType());
	int dstType = TransCommChannelTypeFromString(dstPara.getCommChannelType());

	if (srcType < 0 || dstType < 0 || srcType != dstType)
	{
		return false;
	}

	bool bRet = false;

	switch(srcType)
	{
	case COMMTYPE_TCP_CLIENT:
		bSever = false;
		if (boost::iequals(srcPara.getRemotePort(0),dstPara.getRemotePort(0)) && boost::iequals(srcPara.getRemoteIP(0),dstPara.getRemoteIP(0)))
		{
			bRet = true;
		}
		break;

	case COMMTYPE_UDP_CLIENT:
		bSever = false;
		if (boost::iequals(srcPara.getLocalPort(),dstPara.getLocalPort()) && (!srcPara.getLocalPort().empty()) && boost::iequals(srcPara.getRemotePort(0),dstPara.getRemotePort(0)) && boost::iequals(srcPara.getRemoteIP(0),dstPara.getRemoteIP(0)))
		{
			bRet = true;
		}
		break;

	case COMMTYPE_SERIAL_PORT:
		bSever = false;
		if (boost::iequals(srcPara.getLocalPort(),dstPara.getLocalPort()))
		{
			bRet = true;
		}
		break;

	case COMMTYPE_UART_PORT:
		bSever = false;
		if (boost::iequals(srcPara.getLocalPort(),dstPara.getLocalPort()))
		{
			bRet = true;
		}
		break;

	case COMMTYPE_TCP_SERVER:
		bSever = true;
		if (boost::iequals(srcPara.getLocalPort(),dstPara.getLocalPort()))
		{
			bRet = true;
		}
		break;

	case COMMTYPE_UDP_SERVER:
		bSever = true;
		if (boost::iequals(srcPara.getLocalPort(),dstPara.getLocalPort()))
		{
			bRet = true;
		}
		break;

	default:
		break;
	}

	return bRet;
}

};//namespace CommInterface 

