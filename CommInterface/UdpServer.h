#pragma once
#include <boost/asio.hpp>
#include "ServerInterface.h"

namespace CommInterface {

using boost::asio::ip::udp;

class CUdpSession;

typedef boost::shared_ptr<CUdpSession> udpsession_ptr;

class CUdpServer
	:public CServerInterface
{
public:
	CUdpServer(CCommPara & para,boost::asio::io_service& io_service,unsigned short port);
	virtual ~CUdpServer(void);

	virtual int StartServer();
	virtual int InitServer(share_commpoint_ptr comm_point  = share_commpoint_ptr());
	int StartReadConnect();

private:
	void StartBind();
	void CloseSever();
	int AddSession(udpsession_ptr val);
	int DelSession(int index);
	udpsession_ptr getSession(int index);
	int getSessionSum();
	void ClearSessions();

	void handle_read(unsigned char * buf,const boost::system::error_code& error,size_t bytes_transferred/*,boost::asio::ip::udp::endpoint & remote_endpoint*/);
	udpsession_ptr getSessionByIP(boost::asio::ip::address ipVal);

private:
	enum
	{
		max_length = MAX_IP_MTU,
	};

	size_t port_;
	udp::socket socket_;
	udp::endpoint remote_endpoint_;
	std::vector<udpsession_ptr> sessions_;

	unsigned char recv_data_[max_length];


};

}; //namespace CommInterface 
