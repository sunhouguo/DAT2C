#pragma once
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>

namespace FileSystem
{
	class CLog;
}

namespace LocalDrive {

	class CBluetooth
	{
	public:
		CBluetooth(bool bEnableLog);
		virtual ~CBluetooth(void);

		virtual int run() = 0;
		virtual int stop() = 0;
		virtual int reset() = 0;

	protected:
		int AddLogWithSynT(std::string strVal);

	private:
		boost::scoped_ptr<FileSystem::CLog> Log_;
	};

};//namespace LocalDrive 


