#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "XmlRecord.h"
#include "Markup.h"

namespace FileSystem {

const std::string strRecord = "Record";
const std::string strClock = "Time";

CXmlRecord::CXmlRecord(std::string filename,int limit)
					:CXmlLog(filename,"a",limit)
{

}
CXmlRecord::CXmlRecord(std::string filename)
					:CXmlLog(filename,"a")
{

}

CXmlRecord::~CXmlRecord(void)
{
}

int CXmlRecord::AddRecord(name_val_set & val)
{
	if (strLogPath_.empty())
	{
		return -1;
	}

	return RecordData(val,false,xml_);
}

int CXmlRecord::AddRecord(name_val_set & attrib,name_val_set & val)
{
	if (strLogPath_.empty())
	{
		return -1;
	}

	return RecordData(attrib,val,false,xml_);
}

int CXmlRecord::AddRecordWithSynT(name_val_set & val)
{
	if (strLogPath_.empty())
	{
		return -1;
	}

	return RecordData(val,true,xml_);
}

int CXmlRecord::AddRecordWithSynT(name_val_set & attrib,name_val_set & val)
{
	if (strLogPath_.empty())
	{
		return -1;
	}

	return RecordData(attrib,val,true,xml_);
}

int CXmlRecord::RecordData(name_val_set & val,bool strSynT,CMarkup & xml)
{
	CheckXmlLogExsit(strLogPath_,xml);

	xml.FindElem(); 
	xml.IntoElem(); //into root

	xml.AddElem(strRecord);			
	if (strSynT)
	{
		boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
		std::string strTime = to_simple_string(now);
		xml.AddAttrib(strClock,strTime);
	}

	xml.IntoElem();

	for (name_val_it it = val.begin();it != val.end();it++)
	{
		xml.AddElem(boost::tuples::get<0>(*it),boost::tuples::get<1>(*it));
	}

	xml.OutOfElem();

	return ResetRecordCounter(xml);
}

int CXmlRecord::RecordData(name_val_set & attrib,name_val_set & val,bool strSynT,CMarkup & xml)
{
	CheckXmlLogExsit(strLogPath_,xml);

	xml.FindElem(); 
	xml.IntoElem(); //into root

	xml.AddElem(strRecord);			
	if (strSynT)
	{
		boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
		std::string strTime = to_simple_string(now);
		xml.AddAttrib(strClock,strTime);
	}

	xml.IntoElem();

	name_val_it itAttrib = attrib.begin();
	for (name_val_it itVal = val.begin();itVal != val.end();itVal++)
	{
		xml.AddElem(boost::tuples::get<0>(*itVal),boost::tuples::get<1>(*itVal));
		if (itAttrib != attrib.end())
		{
			xml.AddAttrib(boost::tuples::get<0>(*itAttrib),boost::tuples::get<1>(*itAttrib));
		}
		itAttrib ++;
	}

	xml.OutOfElem();

	return ResetRecordCounter(xml);
}

}; //namespace FileSystem 

