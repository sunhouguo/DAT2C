#pragma once
#include <boost/asio.hpp>
#include "ServerInterface.h"

namespace CommInterface {

class CTcpSession;

class CTcpServer
	:public CServerInterface
{
public:
	CTcpServer(CCommPara & para,boost::asio::io_service& io_service,size_t port);
	virtual ~CTcpServer(void);

	virtual int StartServer();

private:
	void ResetServer();

	void handle_accept(const boost::system::error_code& error);
	void handle_read(unsigned char * buf,const boost::system::error_code& error,size_t bytes_transferred);
	void handle_timerMatching(const boost::system::error_code& error);
	void ResetTimerMatch(bool bContinue);

	share_commpoint_ptr MatchCommPointByIP(boost::asio::ip::address remote_ip);
	share_commpoint_ptr MatchCommPointByID(int ID);
	void OpenSession(share_commpoint_ptr commpoint);
	void CloseSession();
	int ParseIdByFrame(unsigned char *buf);

private:
	enum
	{
		defatult_timeout_match = 3, //等待匹配报文超时时间，单位：秒
		max_length = 256,
	};

	int MatchType_;
	size_t port_;
	
	unsigned char MatchFrameBuf[max_length];
	
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::shared_ptr<CTcpSession> new_session_;
	boost::asio::deadline_timer timerMatching_;
};

} //namespace CommInterface
