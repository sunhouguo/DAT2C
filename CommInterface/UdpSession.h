#pragma once
#include "CommInterface.h"

namespace CommInterface {

using boost::asio::ip::udp;

class CUdpSession :
	//public boost::enable_shared_from_this<CUdpSession>,
	public CCommInterface
{
public:
	CUdpSession(CCommPara & para,boost::asio::io_service& io_service,udp::socket & socket);

	virtual ~CUdpSession(void);

	virtual int InitConnect(void);
	virtual void UninitConnect(void);
	virtual int OpenConnect(void);
	virtual int CloseConnect(void);
	virtual int RetryConnect(void);
	virtual int WriteToConnect(const unsigned char * buf, size_t bytes_transferred);
	virtual int ReadFromConnect(unsigned char * buf, size_t bytes_transferred = 0);
	virtual bool getConnectAlive(void);

	int WriteToBroadCast(const unsigned char * buf,size_t bytes_transferred);

	bool MatchRemoteIP(boost::asio::ip::address ipVal);

private:
	void handle_write(const boost::system::error_code& error,size_t bytes_transferred);
	udp::endpoint getNextEndPoint();
	
private:
	udp::socket & socket_;
	//boost::asio::ip::udp::endpoint remote_endpoint_;
	std::vector<udp::endpoint> remoteEndPoints_;
	int remoteEndPointIndex_;

	boost::scoped_ptr<udp::endpoint> broadcast_endpoint_;
};

}; //namespace CommInterface 
