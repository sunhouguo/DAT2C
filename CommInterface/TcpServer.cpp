#include <boost/bind.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include "TcpServer.h"
#include "TcpSession.h"
#include "../DataBase/CommPoint.h"

namespace CommInterface {

using boost::asio::ip::tcp;

CTcpServer::CTcpServer(CCommPara & para,boost::asio::io_service& io_service,size_t port)
					   :CServerInterface(para,io_service),
					   port_(port),
					   acceptor_(io_service_,tcp::endpoint(tcp::v4(),port)),
					   new_session_(new CTcpSession(para,io_service)),
					   timerMatching_(io_service)
{
	MatchType_ = TransMatchTypeFromString(para.getMatchType());
}

CTcpServer::~CTcpServer(void)
{
}

int CTcpServer::StartServer()
{
	acceptor_.async_accept(new_session_->socket(), 
		boost::bind(&CTcpServer::handle_accept,
		this,
		boost::asio::placeholders::error));

	return 0;
}

void CTcpServer::ResetServer()
{
	new_session_.reset(new CTcpSession(para_,io_service_));

	acceptor_.async_accept(new_session_->socket(), 
		boost::bind(&CTcpServer::handle_accept,
		this,
		boost::asio::placeholders::error));
}

share_commpoint_ptr CTcpServer::MatchCommPointByID( int ID )
{
	for (int i=0;i<getCommPointSum();i++)
	{
		share_commpoint_ptr tempPtr = getCommPoint(i).lock();
		if (tempPtr)
		{
			unsigned short iport;
			try
			{
				iport = boost::lexical_cast<unsigned short>(tempPtr->getLocalPort());
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<< "exception: " << e.what() <<std::endl;
				continue;
			}

			try
			{
				if ((ID == boost::lexical_cast<int>(tempPtr->getRemoteID())) && port_ == iport)	
				{
					return tempPtr; //匹配成功，返回匹配的通讯结点指针
				}
			}
			catch(boost::bad_lexical_cast & e)
			{
				std::cerr<<e.what()<<std::endl;
			}
		}
	}

	return share_commpoint_ptr();
}

share_commpoint_ptr CTcpServer::MatchCommPointByIP(boost::asio::ip::address remote_ip)
{
	for (int i = 0;i<getCommPointSum();i++)
	{
		share_commpoint_ptr tempPtr = getCommPoint(i).lock();
		if (tempPtr)
		{
			unsigned short iport;
			try
			{
				iport = boost::lexical_cast<unsigned short>(tempPtr->getLocalPort());
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<< "exception: " << e.what() <<std::endl;
				continue;
			}

			if(tempPtr->MatchRemoteIP(remote_ip.to_string()) && port_ == iport)
			{
				return tempPtr; //匹配成功，返回匹配的通讯结点指针
			}
		}
	}

	return share_commpoint_ptr(); //匹配失败，返回空指针。
}

void CTcpServer::OpenSession( share_commpoint_ptr commpoint )
{
	commpoint->ResetCommPtr(new_session_);
	new_session_->setCommPara(commpoint->getCommPara());

	if (!new_session_->InitConnect())
	{
		new_session_->AddCommPoint(commpoint);
		if (CmdRecallSlot_)
		{
			new_session_->ConnectCmdRecallSig(*CmdRecallSlot_);
		}

		new_session_->OpenConnect();
	}
	else
	{
		CloseSession();
	}
}

void CTcpServer::CloseSession()
{
	new_session_->CloseConnect();
	new_session_->UninitConnect();
}

void CTcpServer::handle_read(unsigned char * buf,const boost::system::error_code& error,size_t bytes_transferred)
{
	if (!error)
	{
		//解析报文 buf[0]~buf[bytes_transferred - 1],根据报文获得装置ID
		int id = ParseIdByFrame(buf);

		share_commpoint_ptr commpoint = MatchCommPointByID(id);

		if (commpoint) //如果匹配到一个commpoint，则取消匹配定时器，开始初始化session；否则等定时器去执行取消session操作即可
		{
			ResetTimerMatch(false);
			OpenSession(commpoint);

			ResetServer();
		}
	}
}

void CTcpServer::handle_timerMatching(const boost::system::error_code& error)
{
	if (!error)
	{
		CloseSession();

		ResetServer();
	}
}

void CTcpServer::ResetTimerMatch(bool bContinue)
{
	if (bContinue)
	{
		timerMatching_.expires_from_now(boost::posix_time::seconds(defatult_timeout_match));
		timerMatching_.async_wait(boost::bind(&CTcpServer::handle_timerMatching,
			this,
			boost::asio::placeholders::error));
	}
	else
	{
		timerMatching_.cancel();
	}
}

void CTcpServer::handle_accept(const boost::system::error_code& error)
{
	if (!error)
	{
		switch(MatchType_)
		{
		case ID_MATCHING:
			{
				new_session_->socket().async_read_some(boost::asio::buffer(MatchFrameBuf,max_length),
					boost::bind(&CTcpServer::handle_read,
					this,
					MatchFrameBuf,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

				ResetTimerMatch(true);
			}
			break;

		default:
			{
				boost::asio::ip::address remote_ip = ((new_session_->socket()).remote_endpoint()).address();
				share_commpoint_ptr commpoint = MatchCommPointByIP(remote_ip);
				if (commpoint)
				{
					OpenSession(commpoint);
				}
				else
				{
					CloseSession();
				}

				ResetServer();
			}
			break;
		}
	}
	else
	{
		ResetServer();
	}
}

int CTcpServer::ParseIdByFrame(unsigned char *buf)
{
	int ret = -1;

	if ((buf[0] == 0x58) && (buf[1] == 0x4d) && (buf[2] == 0x59) && (buf[3] == 0x4e))
	{
		char Tmpbuf [9];
		for (int i=4;i< 13;i++)
		{
			char strTmp = buf[i];
			switch (strTmp)
			{
			case 0x30:
				Tmpbuf [i-4] ='0';
				break;
			case 0x31:
				Tmpbuf [i-4] ='1';
				break;
			case 0x32:
				Tmpbuf [i-4] ='2';
				break;
			case 0x33:
				Tmpbuf [i-4] ='3';
				break;
			case 0x34:
				Tmpbuf [i-4] ='4';
				break;
			case 0x35:
				Tmpbuf [i-4] ='5';
				break;
			case 0x36:
				Tmpbuf [i-4] ='6';
				break;
			case 0x37:
				Tmpbuf [i-4] ='7';
				break;
			case 0x38:
				Tmpbuf [i-4] ='8';
				break;
			case 0x39:
				Tmpbuf [i-4] ='9';
				break;
			default:
				break;
			}
		}

		ret = 100*(Tmpbuf[6]-48)+10*(Tmpbuf[7]-48)+(Tmpbuf[8]-48);	
	}

	return ret;
}

} //namespace CommInterface
