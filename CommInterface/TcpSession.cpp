#include <boost/bind.hpp>
#include "TcpSession.h"
#include "../Protocol/Protocol.h"

namespace CommInterface {

using boost::asio::ip::tcp;

CTcpSession::CTcpSession(CCommPara & para,boost::asio::io_service& io_service)
						:CCommInterface(para,io_service),
						socket_(io_service),
						timerRecv_(io_service,boost::posix_time::minutes(recv_time_out))
						
{
	
}

CTcpSession::~CTcpSession(void)
{
	if (getConnectAlive())
	{
		CloseConnect();
	}

	UninitConnect();
}

tcp::socket& CTcpSession::socket(void)
{
	return socket_;
}

int CTcpSession::InitConnect(void)
{
	EnableLog();

	int ret = 0;

	ret = EnableProtocol();
	if (ret)
	{
		AddStatusLogWithSynT("TcpSession通道初始化规约失败！\n");
		return ret;
	}

	AddStatusLogWithSynT("初始化TcpSession通道成功！\n");

	return ret;
}

void CTcpSession::UninitConnect(void)
{
	DisableProtocol();

	DisableLog();

	AddStatusLogWithSynT("反初始化TcpSession通道！\n");
}

int CTcpSession::OpenConnect(void)
{
	InitProtocol();

	std::ostringstream ostr;
	ostr<<"一个新的TcpSession通道建立成功，对端信息："<<socket_.remote_endpoint().address().to_string()<<":"<<socket_.remote_endpoint().port()<<std::endl;

	AddStatusLogWithSynT(ostr.str());

	ReadFromConnect(recv_data_);

	return 0;
}

int CTcpSession::CloseConnect(void)
{
	timerRecv_.cancel();

	boost::system::error_code ec;
	socket_.cancel(ec);
	socket_.shutdown(tcp::socket::shutdown_both,ec);

	socket_.close();

	UninitProtocol();

	AddStatusLogWithSynT("TcpSession通道关闭连接。\n");

	return 0;
}

int CTcpSession::ReadFromConnect(unsigned char * buf, size_t bytes_transferred /* = 0 */)
{
	if (getConnectAlive())
	{
		socket_.async_read_some(boost::asio::buffer(buf,max_length),
			boost::bind(&CTcpSession::handle_read,
			shared_from_this(),
			buf,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

		timerRecv_.expires_from_now(boost::posix_time::minutes(recv_time_out));
		timerRecv_.async_wait(boost::bind(&CTcpSession::handle_timerRecv,
			shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		CloseConnect();

		AddStatusLogWithSynT("TcpSession通道尝试读数据时发现通道未通讯，尝试重连接通道。\n");

		return -1;
	}

	return 0;
}

int CTcpSession::WriteToConnect(const unsigned char * buf, size_t bytes_transferred /* = 0 */)
{
	if (getConnectAlive())
	{
		RecordFrameData(buf,bytes_transferred,false);

		socket_.async_send(boost::asio::buffer(buf, bytes_transferred),
			boost::bind(&CTcpSession::handle_write, 
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		AddStatusLogWithSynT("TcpSession通道尝试写数据时发现通道未通讯，尝试重连接通道。\n");

		return -1;
	}

	return 0;
}

bool CTcpSession::getConnectAlive(void)
{
	return socket_.is_open();
}

int CTcpSession::RetryConnect(void)
{
	CloseConnect();
	
	return 0;
}

void CTcpSession::handle_read( unsigned char * buf,const boost::system::error_code& error,size_t bytes_transferred )
{
	if(!error)
	{
		timerRecv_.cancel();

		RecordFrameData(buf,bytes_transferred,true);

		//调用CProtocol模块处理recvbuf中bytes_transferred字节的报文
		if (protocol_)
		{
			protocol_->RecvProcess(buf,bytes_transferred);
		}
		
		//再继续读数据
		ReadFromConnect(buf);
	}
	else if (error == boost::asio::error::operation_aborted)
	{
		//socket被其他操作关闭了,谁关的谁负责重连，do nothing
		AddStatusLogWithSynT("TcpSession通道读数据过程中发现通道被关闭，等待重连接。\n");
	}
	else if(error == boost::asio::error::timed_out)
	{
		//socket连接超时
		AddStatusLogWithSynT("TcpSession通道读数据过程中发生错误，socket层连接超时，尝试关闭通道。\n");
		CloseConnect();
	}
	else if (error == boost::asio::error::eof)
	{
		//对端关闭socket连接
		AddStatusLogWithSynT("TcpSession通道读数据过程中发生错误，对端关闭socket，尝试关闭通道。\n");
		CloseConnect();
	}
	else
	{
		//或者是其他系统抛出的错误，没分类处理，还是重连吧
		AddStatusLogWithSynT("TcpSession通道读数据过程中发生一个未分类的错误,尝试关闭通道。\n");
		CloseConnect();
	}
}

void CTcpSession::handle_write(const boost::system::error_code& error,size_t bytes_transferred)
{
	if(error == boost::asio::error::operation_aborted)
	{
		//通道被其他操作关闭了,谁关的谁负责重连，do nothing
		AddStatusLogWithSynT("TcpSession通道写数据过程中发现通道被关闭，等待重连接。\n");
		return;
	}
	else if(error == boost::asio::error::timed_out)
	{
		//socket连接超时
		AddStatusLogWithSynT("TcpSession通道写数据过程中发生错误，socket层连接超时，尝试关闭通道。\n");
		CloseConnect();
	}
	else if (error == boost::asio::error::eof)
	{
		//对端关闭socket连接
		AddStatusLogWithSynT("TcpSession通道写数据过程中发生错误，对端关闭socket，尝试关闭通道。\n");
		CloseConnect();
	}
	else if(error)
	{
		//或者是其他系统抛出的错误，没分类处理，还是重连吧
		AddStatusLogWithSynT("TcpSession通道写数据过程中发生一个未分类的错误,尝试关闭通道。\n");
		CloseConnect();
	}
}

void CTcpSession::handle_timerRecv(const boost::system::error_code& error)
{
	if (!error)
	{
		//超过recv_time_out时间没有收到任何报文，重新连接通道。
		std::ostringstream ostr;
		ostr<<"TcpSession通道已经"<<recv_time_out<<"分钟时间未收到任何报文，尝试重连接通道。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		CloseConnect();
	}
}

int CTcpSession::setCommPara(CCommPara val)
{
	para_ = val;

	return 0;
}

} //namespace CommInterface


