#include "YcStatRecord.h"
#include "YCPoint.h"

namespace DataBase {

const std::string strYcIndex = "YcIndex";
const std::string strMaxYcVal = "MaxYcVal";
const std::string strMinYcVal = "MinYcVal";

CYcStatRecord::CYcStatRecord(std::string id,int limit)
	:CDataRecord(id,limit)
{
}

CYcStatRecord::CYcStatRecord(std::string id)
	:CDataRecord(id)
{
}

CYcStatRecord::~CYcStatRecord(void)
{
}

int CYcStatRecord::AddDataRecord(PublicSupport::CDynamicArrayDataBase<CYcPoint> & ycDB)
{
	using namespace std;

	FileSystem::name_val_set max_attrib_set;
	FileSystem::name_val_set max_val_set;
	FileSystem::name_val_set min_attrib_set;
	FileSystem::name_val_set min_val_set;

	for (int i=0;i<(int)ycDB.getDataBaseSum();i++)
	{
		string YcIndex = boost::lexical_cast<string>(i);
		string MaxYcVal = boost::lexical_cast<string>(ycDB.getPointDataPtr(i)->getYcMaxVal());
		string MinYcVal = boost::lexical_cast<string>(ycDB.getPointDataPtr(i)->getYcMinVal());
		
		max_attrib_set.push_back(FileSystem::name_val(strYcIndex,YcIndex));
		max_val_set.push_back(FileSystem::name_val(strMaxYcVal,MaxYcVal));
		min_attrib_set.push_back(FileSystem::name_val(strYcIndex,YcIndex));
		min_val_set.push_back(FileSystem::name_val(strMinYcVal,MinYcVal));

		ycDB.getPointDataPtr(i)->ResetYcStatVal();
	}

	CDataRecord::AddDataRecord(max_attrib_set,max_val_set);
	CDataRecord::AddDataRecord(min_attrib_set,min_val_set);

	return 0;
}

}; //namespace DataBase
