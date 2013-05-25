#pragma once
//edit
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include "CommInterface.h"

namespace CommInterface {

class CTcpSession:
	public boost::enable_shared_from_this<CTcpSession>,
	public CCommInterface
{
public:
	CTcpSession(CCommPara & para,boost::asio::io_service& io_service);
	virtual ~CTcpSession(void);

	virtual int InitConnect(void);
	virtual void UninitConnect(void);
	virtual int OpenConnect(void);
	virtual int CloseConnect(void);
	virtual int RetryConnect(void);
	virtual int WriteToConnect(const unsigned char * buf, size_t bytes_transferred);
	virtual bool getConnectAlive(void);
	boost::asio::ip::tcp::socket& socket(void);

	int setCommPara(CCommPara val);

private:
	virtual int ReadFromConnect(unsigned char * buf, size_t bytes_transferred = 0);
	void handle_read(unsigned char * buf,const boost::system::error_code& error,size_t bytes_transferred);
	void handle_write(const boost::system::error_code& error,size_t bytes_transferred);
	void handle_timerRecv(const boost::system::error_code& error);

private:
	enum
	{
		recv_time_out = 7,           //接收超时(单位:分钟)

		max_length = MAX_TCP_MTU,
	};
	boost::asio::ip::tcp::socket socket_;
	boost::asio::deadline_timer timerRecv_;

	unsigned char recv_data_[max_length];
};

typedef boost::shared_ptr<CTcpSession> tcpsession_ptr;

} //namespace CommInterface
