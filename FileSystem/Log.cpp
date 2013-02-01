//#define BOOST_FILESYSTEM_VERSION 2

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <iostream>
#include "Log.h"

namespace FileSystem {

CLog::CLog(std::string filename,std::string OpenMode)
{
	strLogPath_ = filename;
	strMode_ = OpenMode;

	GreateFilePath(strLogPath_);
}

CLog::~CLog(void)
{
}

std::string CLog::getLogPath()
{
	return strLogPath_;
}

std::string CLog::getFileType()
{
	return "";
}

int CLog::GreateFilePath(std::string strFilePath)
{
	std::vector<std::string> splitVec; 
	boost::array<char, 2>separator = {'/','\\'};
	boost::algorithm::split(splitVec,strFilePath,boost::algorithm::is_any_of(separator));

	if(splitVec.size() > 1)
	{
		std::string strPath;
		for (size_t i=0;i<splitVec.size() - 1;i++)
		{
			strPath += splitVec[i];
			strPath += "/";
			
			//std::cout<<strPath<<std::endl;
		}

		namespace fs = boost::filesystem;
		//路径的可移植
		fs::path full_path( fs::initial_path() );
		full_path = fs::system_complete( fs::path(strPath, fs::native ) );

		//判断各级子目录是否存在，不存在则需要创建
		if ( !fs::exists( full_path ) )
		{
			// 创建多层子目录
			if(!fs::create_directories(full_path))
			{
				return -1;
			}

		}
	}

	//strFilePath = full_path.native_directory_string();

	return 0;

}


}; //namespace FileSystem 

