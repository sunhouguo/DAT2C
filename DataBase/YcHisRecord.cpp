#include <boost/lexical_cast.hpp>
#include "YcHisRecord.h"
#include "YCPoint.h"

namespace DataBase {

const std::string strYcIndex = "YcIndex";
const std::string strYcVal = "YcVal";

CYcHisRecord::CYcHisRecord(std::string id,int limit)
						:CDataRecord(id,limit)
{
}

CYcHisRecord::CYcHisRecord(std::string id)
:CDataRecord(id)
{
}

CYcHisRecord::~CYcHisRecord(void)
{
}

int CYcHisRecord::AddDataRecord(PublicSupport::CDynamicArrayDataBase<CYcPoint> & ycDB)
{
	using namespace std;

	FileSystem::name_val_set attrib_set;
	FileSystem::name_val_set val_set;
	for (int i=0;i<(int)ycDB.getDataBaseSum();i++)
	{
		string YcIndex = boost::lexical_cast<string>(i);
		string YcVal = boost::lexical_cast<string>(ycDB.getPointDataPtr(i)->getFinalYcVal());

		attrib_set.push_back(FileSystem::name_val(strYcIndex,YcIndex));
		val_set.push_back(FileSystem::name_val(strYcVal,YcVal));
	}

	return CDataRecord::AddDataRecord(attrib_set,val_set);
}

//int CYcHisRecord::AddDataRecord(CSubStation & sub)
//{
//	using namespace std;
//
//	FileSystem::name_val_set attrib_set;
//	FileSystem::name_val_set val_set;
//	for (int i=0;i<(int)sub.getYcSum();i++)
//	{
//		string YcIndex = boost::lexical_cast<string>(i);
//		string YcVal = boost::lexical_cast<string>(sub.getFinalYcVal(i));
//
//		attrib_set.push_back(FileSystem::name_val(strYcIndex,YcIndex));
//		val_set.push_back(FileSystem::name_val(strYcVal,YcVal));
//	}
//
//	return CDataRecord::AddDataRecord(attrib_set,val_set);
//}

};//namespace DataBase
