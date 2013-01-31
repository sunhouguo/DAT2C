#include <boost/bind.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "UdpServer.h"
#include "UdpSession.h"
#include "../DataBase/CommPoint.h"

namespace CommInterface {

CUdpServer::CUdpServer(CCommPara & para,boost::asio::io_service& io_service,unsigned short port)
					   :CServerInterface(para,io_service),
					   port_(port),
					   socket_(io_service)
{
	StartBind();
}

CUdpServer::~CUdpServer(void)
{
	CloseSever();
}

void CUdpServer::CloseSever()
{
	boost::system::error_code ec;
	socket_.cancel(ec);
	socket_.shutdown(udp::socket::shutdown_both,ec);
	socket_.close();
}

void CUdpServer::StartBind()
{
	try
	{
		if (CCommPara::AssertIPFormat(para_.getLocalIP()))
		{
			boost::system::error_code ec;
			socket_.open(udp::v4());
			socket_.bind(udp::endpoint(boost::asio::ip::address::from_string(para_.getLocalIP()),port_),ec);
			if(ec)
			{
				std::ostringstream ostr;
				ostr<<"绑定UdpServer的Socket至IP:"<<para_.getLocalIP()<<"Port:"<<para_.getLocalPort()<<"失败 "<<ec<<std::endl;
				std::cerr<<ostr.str();
			}
		}
		else
		{
			socket_.open(udp::v4());
			socket_.bind(udp::endpoint(udp::v4(),port_));
		}

		return;
	}
	catch(boost::system::system_error & e)
	{
		std::ostringstream ostr;
		ostr<<"绑定UdpServer的Socket至IP:"<<para_.getLocalIP()<<"Port:"<<para_.getLocalPort()<<"失败 "<<e.what()<<std::endl;
		std::cerr<<ostr.str();
	}
}

int CUdpServer::InitServer(share_commpoint_ptr comm_point /*= share_commpoint_ptr()*/)
{
	if(comm_point)
	{
		udpsession_ptr sessionTmp(new CUdpSession(comm_point->getCommPara(),io_service_,socket_));
		AddSession(sessionTmp);

		comm_point->ResetCommPtr(sessionTmp);
		if (!sessionTmp->InitConnect())
		{
			sessionTmp->AddCommPoint(comm_point);
			if (CmdRecallSlot_)
			{
				sessionTmp->ConnectCmdRecallSig(*CmdRecallSlot_);
			}

			sessionTmp->OpenConnect();
		}
	}

	return 0;
}

int CUdpServer::StartServer()
{
	return StartReadConnect();
}

int CUdpServer::AddSession(udpsession_ptr val)
{
	sessions_.push_back(val);

	return 0;
}

int CUdpServer::DelSession(int index)
{
	if (index < 0 || index >= getSessionSum())
	{
		return -1;
	}

	sessions_.erase(sessions_.begin() + index);

	return 0;
}

udpsession_ptr CUdpServer::getSession(int index)
{
	if (index < 0 || index >= getSessionSum())
	{
		return udpsession_ptr();
	}

	return sessions_.at(index);
}

int CUdpServer::getSessionSum()
{
	return sessions_.size();
}

void CUdpServer::ClearSessions()
{
	sessions_.clear();
}

int CUdpServer::StartReadConnect()
{
	//boost::asio::ip::udp::endpoint remote_endpoint_;

	socket_.async_receive_from(boost::asio::buffer(recv_data_,max_length),remote_endpoint_,
		boost::bind(&CUdpServer::handle_read,
		this,
		recv_data_,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));


	return 0;
}

void CUdpServer::handle_read(unsigned char * buf,const boost::system::error_code& error,size_t bytes_transferred/*,udp::endpoint & remote_endpoint*/)
{
	if(!error)
	{
		udpsession_ptr tmpSession = getSessionByIP(remote_endpoint_.address());
		if (tmpSession)
		{
			tmpSession->ReadFromConnect(buf,bytes_transferred);
		}
	}

	//再继续读数据
	StartReadConnect();
}

udpsession_ptr CUdpServer::getSessionByIP(boost::asio::ip::address ipVal)
{
	for (int i=0;i<getSessionSum();i++)
	{
		if(sessions_[i])
		{
			if(sessions_[i]->MatchRemoteIP(ipVal))
			{
				return sessions_[i];
			}
		}
	}

	return udpsession_ptr();
}

}; //namespace CommInterface 
