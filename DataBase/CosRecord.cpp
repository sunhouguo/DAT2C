#include <boost/lexical_cast.hpp>
#include "CosRecord.h"
#include "CosPoint.h"

namespace DataBase {

const std::string strYxIndex = "YxIndex";
const std::string strYxVal = "YxVal";

CCosRecord::CCosRecord(std::string id,int limit)
	:CDataRecord(id,limit)
{
}

CCosRecord::CCosRecord(std::string id)
	:CDataRecord(id)
{
}

CCosRecord::~CCosRecord(void)
{
}

int CCosRecord::AddDataRecord(const CCosPoint & cos)
{
	using namespace std;

	string YxIndex = boost::lexical_cast<string>(cos.getYxIndex());
	string YxVal = boost::lexical_cast<string>((int)cos.getYxVal());


	FileSystem::name_val_set set;
	set.push_back(FileSystem::name_val(strYxIndex,YxIndex));
	set.push_back(FileSystem::name_val(strYxVal,YxVal));

	return CDataRecord::AddDataRecord(set);
}

}; //namespace DataBase

