#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "FaultRecord.h"
#include "FaultPoint.h"

namespace DataBase {

const std::string strFaultNO = "FaultNO";
const std::string strFaultVal = "FaultVal";
const std::string strFaultTime = "FaultTime";

CFaultRecord::CFaultRecord(std::string id,int limit)
						:CDataRecord(id,limit)
{
}

CFaultRecord::CFaultRecord(std::string id)
						:CDataRecord(id)
{
}

CFaultRecord::~CFaultRecord(void)
{
}

int CFaultRecord::AddDataRecord(const CFaultPoint & fault)
{
	using namespace std;

	string FaultNO = boost::lexical_cast<string>(fault.getFaultNO());
	string FaultVal = boost::lexical_cast<string>(fault.getFaultVal());
	string FaultTime = boost::posix_time::to_simple_string(fault.getFaultTime());

	FileSystem::name_val_set set;
	set.push_back(FileSystem::name_val(strFaultNO,FaultNO));
	set.push_back(FileSystem::name_val(strFaultVal,FaultVal));
	set.push_back(FileSystem::name_val(strFaultTime,FaultTime));

	return CDataRecord::AddDataRecord(set);
}

}; //namespace DataBase

