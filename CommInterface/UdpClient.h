#pragma once
#include "CommInterface.h"

namespace CommInterface {

using boost::asio::ip::udp;

class CUdpClient:
	//public boost::enable_shared_from_this<CUdpClient>,
	public CCommInterface
{
public:
	CUdpClient(CCommPara & para,boost::asio::io_service & io_service);
	virtual ~CUdpClient(void);

	virtual int InitConnect(void);
	virtual void UninitConnect(void);
	virtual int OpenConnect(void);
	virtual int CloseConnect(void);
	virtual int RetryConnect(void);
	virtual int WriteToConnect(const unsigned char * buf,size_t bytes_transferred);
	virtual bool getConnectAlive(void);

	int WriteToBroadCast(const unsigned char * buf,size_t bytes_transferred);

private:
	virtual int ReadFromConnect(unsigned char * buf,size_t bytes_transferred = 0);
	void handle_read(unsigned char * buf,const boost::system::error_code& error,size_t bytes_transferred);
	void handle_write(const boost::system::error_code& error,size_t bytes_transferred);
	udp::endpoint getNextEndPoint();

	void StartBind();

private:
	enum
	{
		max_length = MAX_IP_MTU,
	};
	//unsigned short port_;
	udp::socket socket_;

	std::vector<udp::endpoint> remoteEndPoints_;
	int remoteEndPointIndex_;

	boost::scoped_ptr<udp::endpoint> broadcast_endpoint_;

	unsigned char recv_data_[max_length];
};

}; //namespace CommInterface 
