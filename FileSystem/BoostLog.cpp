#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/filesystem/path.hpp>

#include <boost/log/common.hpp>
#include <boost/log/filters.hpp>
#include <boost/log/formatters.hpp>
#include <boost/log/attributes.hpp>
//#include <boost/log/utility/init/to_file.hpp>
//#include <boost/log/utility/init/common_attributes.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/channel_logger.hpp>

#include "BoostLog.h"

namespace FileSystem {

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace flt = boost::log::filters;
namespace fmt = boost::log::formatters;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;

using boost::shared_ptr;

CBoostLog::CBoostLog(std::string filename,std::string OpenMode,int limit)
	:CLog(filename,OpenMode)
{
	InitSink(filename);
}

CBoostLog::CBoostLog(std::string filename,std::string OpenMode)
	:CLog(filename,OpenMode)
{
	InitSink(filename);
}

CBoostLog::~CBoostLog(void)
{
}

std::string CBoostLog::getFileType()
{
	return strBoostLog;
}

int CBoostLog::AddRecord(std::string strval)
{
	// Let's do a quick test and output something. We have to create a logger for this.
	//src::logger lg;
	src::channel_logger_mt<> lg(keywords::channel = strLogPath_);

	// And output...
	BOOST_LOG(lg) << strval;

	return 0;
}

int CBoostLog::AddRecordWithSynT(std::string strval)
{
	// Let's do a quick test and output something. We have to create a logger for this.
	//src::logger lg;
	src::channel_logger_mt<> lg(keywords::channel = strLogPath_);

	// And output...
	BOOST_LOG(lg) << strval;

	return 0;
}

int CBoostLog::InitSink(std::string log_id)
{
	try
	{
		boost::filesystem::path log_path(log_id);

		std::string extension_name = log_path.extension().string();
		std::string parent_path = log_path.parent_path().string();
		std::string stem = log_path.stem().string();

		std::ostringstream ostr;
		if (parent_path.empty())
		{
			ostr<<stem<<"_%Y%m%d_%H%M%S_%5N"<<extension_name;
		}
		else
		{
			ostr<<parent_path<<"//"<<stem<<"_%Y%m%d_%H%M%S_%5N"<<extension_name;
		}

		// Create a text file sink
		typedef sinks::synchronous_sink< sinks::text_file_backend > file_sink;
		boost::shared_ptr<file_sink> sink(new file_sink(
			keywords::file_name = ostr.str(),					// file name pattern
			keywords::rotation_size = 16384                     // rotation size, in characters
			));

		// Set up where the rotated files will be stored
		sink->locked_backend()->set_file_collector(sinks::file::make_collector(
			keywords::target = "logs",                          // where to store rotated files
			keywords::max_size = 16 * 1024 * 1024,              // maximum total size of the stored files, in bytes
			keywords::min_free_space = 100 * 1024 * 1024        // minimum free space on the drive, in bytes
			));

		// Upon restart, scan the target directory for files matching the file_name pattern
		sink->locked_backend()->scan_for_files();

		sink->locked_backend()->set_formatter(
			fmt::format("%1%: [%2%] - %3%")
			% fmt::attr< unsigned int >("Line #")
			% fmt::date_time< boost::posix_time::ptime >("TimeStamp")
			% fmt::message()
			);

		// Add it to the core
		logging::core::get()->add_sink(sink);

		// Add some attributes too
		boost::shared_ptr< logging::attribute > attr(new attrs::local_clock);
		logging::core::get()->add_global_attribute("TimeStamp", attr);
		attr.reset(new attrs::counter<unsigned int>);
		logging::core::get()->add_global_attribute("Line #", attr);

		return 0;
	}
	catch (std::exception& e)
	{
		std::cout << "FAILURE: " << e.what() << std::endl;
	}

	return -1;
}

}; //namespace FileSystem
