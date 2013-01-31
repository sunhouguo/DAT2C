#include <boost/bind.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include "TcpClient.h"
#include "../Protocol/Protocol.h"
#include "CommPara.h"

namespace CommInterface {

CTcpClient::CTcpClient(CCommPara & para, boost::asio::io_service& io_service)
					   :CCommInterface(para,io_service),
					   socket_(io_service),
					   timerRecv_(io_service,boost::posix_time::minutes(recv_time_out)),
					   timerReconnect_(io_service,boost::posix_time::minutes(reconnect_time_out))
{
	ReconnectTimes_ = 0;
	remoteEndPointIndex_  = 0;
}

CTcpClient::~CTcpClient(void)
{
	if (getConnectAlive())
	{
		CloseConnect();
	}

	UninitConnect();
}

int CTcpClient::InitConnect(void)
{
	EnableLog();

	int ret = 0;

	int EndPointNum = (para_.getRemoteIPSum() < para_.getRemotePortSum()) ? para_.getRemoteIPSum() : para_.getRemotePortSum();

	remoteEndPoints_.clear();

	for (int i=0;i<EndPointNum;i++)
	{
		tcp::resolver resolver(io_service_);
		std::string ip = para_.getRemoteIP(i);
		std::string port = para_.getRemotePort(i);

		if (CCommPara::AssertIPFormat(ip) && CCommPara::AssertNetPortFormat(port))
		{
			tcp::resolver::query query(tcp::v4(), ip, port);
			boost::system::error_code error;
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query,error);
			if (!error)
			{
				tcp::endpoint remote_point = *endpoint_iterator;
				remoteEndPoints_.push_back(remote_point);
			}
		}
	}

	if (remoteEndPoints_.size() > 0)
	{
		AddStatusLogWithSynT("TcpClient解析endpoint成功。\n");
	}
	else
	{
		AddStatusLogWithSynT("TcpClient解析endpoint失败。\n");

		return -1;
	}

	socket_.open(tcp::v4());

	if (CCommPara::AssertIPFormat(para_.getLocalIP()) && CCommPara::AssertNetPortFormat(para_.getLocalPort()))
	{
		try
		{
			size_t LocalPort = boost::lexical_cast<size_t>(para_.getLocalPort());
			socket_.bind(tcp::endpoint(boost::asio::ip::address::from_string(para_.getLocalIP()),LocalPort));

			std::ostringstream ostr;
			ostr<<"绑定TcpClient的Socket至IP:"<<para_.getLocalIP()<<"Port:"<<para_.getLocalPort()<<"成功！"<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		catch(...)
		{
			std::ostringstream ostr;
			ostr<<"绑定TcpClient的Socket至IP:"<<para_.getLocalIP()<<"Port:"<<para_.getLocalPort()<<"失败！"<<std::endl;
			AddStatusLogWithSynT(ostr.str());

		}
	}
	
	ret = EnableProtocol();
	if (ret)
	{
		AddStatusLogWithSynT("TcpClient初始化规约失败！\n");
		return ret;
	}

	AddStatusLogWithSynT("初始化TcpClient通道成功！\n");

	return ret;
}

void CTcpClient::UninitConnect(void)
{
	//resolver_.cancel();

	DisableProtocol();

	remoteEndPoints_.clear();

	DisableLog();

	AddStatusLogWithSynT("反初始化TcpClient通道！\n");
}

tcp::endpoint CTcpClient::getNextEndPoint()
{
	if (remoteEndPoints_.size() == 0)
	{
		return tcp::endpoint();
	}

	if (remoteEndPointIndex_ < 0 || remoteEndPointIndex_ >= (int)remoteEndPoints_.size())
	{
		remoteEndPointIndex_ = 0;
	}

	tcp::endpoint ret = remoteEndPoints_[remoteEndPointIndex_];

	remoteEndPointIndex_ = (++remoteEndPointIndex_) % remoteEndPoints_.size();

	return ret;
}

int CTcpClient::OpenConnect(void)
{
	if (remoteEndPoints_.size() == 0)
	{
		AddStatusLogWithSynT("TcpClient通道未能解析出对端通讯点\n");
		return -1;
	}

	tcp::endpoint point = getNextEndPoint();

	socket_.async_connect(point,	
		boost::bind(&CTcpClient::handle_connect, 
		this,//shared_from_this(), 
		boost::asio::placeholders::error));

	std::ostringstream ostr;
	ostr<<"TcpClient通道尝试连接socket，"<<point.address().to_string()<<":"<<point.port()<<std::endl;
	AddStatusLogWithSynT(ostr.str());
		
	return 0;
}

int CTcpClient::CloseConnect(void)
{
	timerRecv_.cancel();

	boost::system::error_code ec;
	socket_.cancel(ec);
	socket_.shutdown(tcp::socket::shutdown_both,ec);

	socket_.close();

	UninitProtocol(para_.getbDisableCommpointByCloseConnect());

	AddStatusLogWithSynT("TcpClient通道关闭socket。\n");
				
	return 0;
}

int CTcpClient::RetryConnect(void)
{
	timerReconnect_.cancel();

	if (getConnectAlive())
	{
		CloseConnect();
	}
	
	OpenConnect();

	return 0;
}
int CTcpClient::ReadFromConnect( unsigned char * buf,size_t bytes_transferred /*= 0*/ )
{
	if (getConnectAlive())
	{
		socket_.async_read_some(boost::asio::buffer(buf,max_length),
			boost::bind(&CTcpClient::handle_read,
				this,//shared_from_this(),
				buf,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

		timerRecv_.expires_from_now(boost::posix_time::minutes(recv_time_out));
		timerRecv_.async_wait(boost::bind(&CTcpClient::handle_timerRecv,
			this,//shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		RetryConnect();

		AddStatusLogWithSynT("TcpClient通道尝试读数据时发现通道未通讯，尝试重连接通道。\n");

		return -1;
	}
	
	
	return 0;
}

int CTcpClient::WriteToConnect( const unsigned char * buf,size_t bytes_transferred )
{
	if (getConnectAlive())
	{
		RecordFrameData(buf,bytes_transferred,false);

		socket_.async_send(boost::asio::buffer(buf, bytes_transferred),
			boost::bind(&CTcpClient::handle_write, 
				this,//shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		AddStatusLogWithSynT("TcpClient通道尝试写数据时发现通道未通讯，等待重连接通道。\n");

		return -1;
	}
	

	return 0;
}

/*
void CTcpClient::handle_resolve(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
{
	if (!error) 
	{
		//成功解析出地址，保存解析数组，打开连接
		endpoint_ = *endpoint_iterator;
		//OpenConnect();
	}
	else
	{
		//解析出错
		std::cout << "Error: " << error.message() << "\n";
	}

}
*/

void CTcpClient::handle_connect( const boost::system::error_code& err )
{
	if (!err)
	{
		// connect连接成功,开始读数据
		ReconnectTimes_ = 0;
		timerReconnect_.cancel();

		InitProtocol();
		
		AddStatusLogWithSynT("TcpClient通道连接socket成功！\n");

		ReadFromConnect(recv_data_);
	}
	else
	{
		//所有的点都连接失败了，重新连接
		if(ReconnectTimes_++ <= reconnect_times)
		{
			timerReconnect_.expires_from_now(boost::posix_time::seconds(retry_time_out));
			timerReconnect_.async_wait(boost::bind(&CTcpClient::handle_timerReconnect,
				this,//shared_from_this(),
				boost::asio::placeholders::error));

			std::ostringstream ostr;
			ostr<<"TcpClient通道连接socket失败第"<<ReconnectTimes_<<"次，"<<retry_time_out<<"秒后将尝试重连通道。"<<std::endl;
			AddStatusLogWithSynT(ostr.str());

			//RetryConnect();
		}
		else
		{
			//都重连了ReconnectTimes次了，决定罢工了,启动一个定时器每reconnect_time_out时间重新建立一次连接
			timerReconnect_.expires_from_now(boost::posix_time::minutes(reconnect_time_out));
			timerReconnect_.async_wait(boost::bind(&CTcpClient::handle_timerReconnect,
				this,//shared_from_this(),
				boost::asio::placeholders::error));

			UninitProtocol(true);
			
			std::ostringstream ostr;
			ostr<<"TcpClient通道连接socket失败了"<<ReconnectTimes_<<"次，将等待"<<reconnect_time_out<<"分钟再尝试重连接通道。"<<std::endl;
			AddStatusLogWithSynT(ostr.str());

			ReconnectTimes_ = 0;
		}
		
	}

}

void CTcpClient::handle_read( unsigned char * buf,const boost::system::error_code& error,size_t bytes_transferred )
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
		AddStatusLogWithSynT("TcpClient通道读数据过程中发现通道被关闭，等待重连接。\n");
		return;
	}
	else if(error == boost::asio::error::timed_out)
	{
		//socket连接超时
		AddStatusLogWithSynT("TcpClient通道读数据过程中发生错误，socket层连接超时，尝试重连接通道。\n");
		//RetryConnect();
		timerReconnect_.expires_from_now(boost::posix_time::seconds(retry_time_out));
		timerReconnect_.async_wait(boost::bind(&CTcpClient::handle_timerReconnect,
			this,//shared_from_this(),
			boost::asio::placeholders::error));
	}
	else if (error == boost::asio::error::eof)
	{
		//对端关闭socket连接
		AddStatusLogWithSynT("TcpClient通道读数据过程中发生错误，对端关闭socket，尝试重连接通道。\n");
		//RetryConnect();
		timerReconnect_.expires_from_now(boost::posix_time::seconds(retry_time_out));
		timerReconnect_.async_wait(boost::bind(&CTcpClient::handle_timerReconnect,
			this,//shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		//或者是其他系统抛出的错误，没分类处理，还是重连吧
		AddStatusLogWithSynT("TcpClient通道读数据过程中发生一个未分类的错误,尝试重连接通道。\n");
		//RetryConnect();
		timerReconnect_.expires_from_now(boost::posix_time::seconds(retry_time_out));
		timerReconnect_.async_wait(boost::bind(&CTcpClient::handle_timerReconnect,
			this,//shared_from_this(),
			boost::asio::placeholders::error));
	}
}

void CTcpClient::handle_write( const boost::system::error_code& error,size_t bytes_transferred)
{
	if(error == boost::asio::error::operation_aborted)
	{
		//通道被其他操作关闭了,谁关的谁负责重连，do nothing
		AddStatusLogWithSynT("TcpClient通道写数据过程中发现通道被关闭，等待重连接。\n");
		return;
	}
	else if(error == boost::asio::error::timed_out)
	{
		//socket连接超时
		AddStatusLogWithSynT("TcpClient通道写数据过程中发生错误，socket层连接超时，尝试重连接通道。\n");
		//RetryConnect();
		timerReconnect_.expires_from_now(boost::posix_time::seconds(retry_time_out));
		timerReconnect_.async_wait(boost::bind(&CTcpClient::handle_timerReconnect,
			this,//shared_from_this(),
			boost::asio::placeholders::error));
	}
	else if (error == boost::asio::error::eof)
	{
		//对端关闭socket连接
		AddStatusLogWithSynT("TcpClient通道写数据过程中发生错误，对端关闭socket，尝试重连接通道。\n");
		//RetryConnect();
		timerReconnect_.expires_from_now(boost::posix_time::seconds(retry_time_out));
		timerReconnect_.async_wait(boost::bind(&CTcpClient::handle_timerReconnect,
			this,//shared_from_this(),
			boost::asio::placeholders::error));
	}
	else if(error)
	{
		//或者是其他系统抛出的错误，没分类处理，还是不管吧
		AddStatusLogWithSynT("TcpClient通道写数据过程中发生一个未分类的错误,等待重连接通道。\n");
		//RetryConnect();
		//timerReconnect_.expires_from_now(boost::posix_time::seconds(retry_time_out));
		//timerReconnect_.async_wait(boost::bind(&CTcpClient::handle_timerReconnect,
		//	this,//shared_from_this(),
		//	boost::asio::placeholders::error));
	}

}

bool CTcpClient::getConnectAlive(void)
{
	return socket_.is_open();
}

/*
tcp::socket& CTcpClient::socket(void)
{
	return socket_;
}
*/

void CTcpClient::handle_timerRecv(const boost::system::error_code& error)
{
	if (!error)
	{
		//超过recv_time_out时间没有收到任何报文，重新连接通道。
		std::ostringstream ostr;
		ostr<<"TcpClient通道已经"<<recv_time_out<<"分钟时间未收到任何报文，尝试重连接通道。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		RetryConnect();
	}
}

void CTcpClient::handle_timerReconnect(const boost::system::error_code& error)
{
	if (!error)
	{
		RetryConnect();
	}
}

} //namespace CommInterface


