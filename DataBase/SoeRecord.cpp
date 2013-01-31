#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "SoeRecord.h"
#include "SoePoint.h"

namespace DataBase {

const std::string strYxIndex = "YxIndex";
const std::string strYxVal = "YxVal";
const std::string strSoeTime = "SoeTime";

CSoeRecord::CSoeRecord(std::string id,int limit)
					:CDataRecord(id,limit)
{
}

CSoeRecord::CSoeRecord(std::string id)
					:CDataRecord(id)
{
}

CSoeRecord::~CSoeRecord(void)
{
}

int CSoeRecord::AddDataRecord(const CSoePoint & soe)
{
	using namespace std;

	string YxIndex = boost::lexical_cast<string>(soe.getYxIndex());
	string YxVal = boost::lexical_cast<string>((int)soe.getYxVal());
	string SoeTime = boost::posix_time::to_simple_string(soe.getYxTime());

	FileSystem::name_val_set set;
	set.push_back(FileSystem::name_val(strYxIndex,YxIndex));
	set.push_back(FileSystem::name_val(strYxVal,YxVal));
	set.push_back(FileSystem::name_val(strSoeTime,SoeTime));

	return CDataRecord::AddDataRecord(set);
}

}; //namespace DataBase
