#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "YkRecord.h"

namespace DataBase {

const std::string strYkEvent = "YkEvent";
const std::string strYkSelEvent = "YkSelEvent";
const std::string strYkExeEvent = "YkExeEvent";
const std::string strYkCancelEvent = "YkCancelEvent";
const std::string strYkOverEvent = "YkOverEvent";
const std::string strYkSelFailEvent = "YkSelFailEvent";
const std::string strYkExeFailEvent = "YkExeFailEvent";
const std::string strYkCancelFailEvent = "YkCancelFailEvent";
const std::string strYkSelConEvent = "YkSelConEvent";
const std::string strYkExeConEvent = "YkExeConEvent";
const std::string strYkCancelConEvent = "YkCanceConlEvent";
const std::string strUndefineEvent = "UndefineEvent";
const std::string strYkNO = "YkNO";
const std::string strYkVal = "YkVal";
const std::string strYkClose = "Close";
const std::string strYkOpen = "Open";

CYkRecord::CYkRecord(std::string id,int limit)
					:CDataRecord(id,limit)
{
}

CYkRecord::CYkRecord(std::string id)
					:CDataRecord(id)
{
}

CYkRecord::~CYkRecord(void)
{
}

int CYkRecord::AddDataRecord(unsigned char yk_event,int yk_no,bool bCloseOrOpen)
{
	using namespace std;

	string ykEvent = strUndefineEvent;
	switch (yk_event)
	{
	case yk_sel:
		ykEvent = strYkSelEvent;
		break;

	case yk_exe:
		ykEvent = strYkExeEvent;
		break;

	case yk_cancel:
		ykEvent = strYkCancelEvent;
		break;

	case yk_over:
		ykEvent = strYkOverEvent;
		break;

	case yk_sel_fail:
		ykEvent = strYkSelFailEvent;
		break;

	case yk_exe_fail:
		ykEvent = strYkExeFailEvent;
		break;

	case yk_cancel_fail:
		ykEvent = strYkCancelFailEvent;
		break;

	case yk_sel_con:
		ykEvent = strYkSelConEvent;
		break;

	case yk_exe_con:
		ykEvent = strYkExeConEvent;
		break;

	case yk_cancel_con:
		ykEvent = strYkCancelConEvent;
		break;

	default:
		ykEvent = strUndefineEvent;
		break;
	}

	string no = boost::lexical_cast<string>(yk_no);
	string YkVal;
	if (bCloseOrOpen)
	{
		YkVal = strYkClose;
	}
	else
	{
		YkVal = strYkOpen;
	}
	
	FileSystem::name_val_set set;
	set.push_back(FileSystem::name_val(strYkEvent,ykEvent));
	set.push_back(FileSystem::name_val(strYkNO,no));
	set.push_back(FileSystem::name_val(strYkVal,YkVal));

	return CDataRecord::AddDataRecord(set);
}

}; //namespace DataBase 
