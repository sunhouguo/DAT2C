#pragma once
#include <stdio.h>

namespace FileSystem {

class CLog;

class CLogFactory
{
public:
	~CLogFactory(void);
	static CLog * CreateLog(std::string log_id,std::string log_type,std::string log_limit = "0");
	static std::string TransLogTypeToString(unsigned short val);

private:
	CLogFactory(void);
	static unsigned char TransLogTypeFromString( std::string val );
};

};//namespace FileSystem
