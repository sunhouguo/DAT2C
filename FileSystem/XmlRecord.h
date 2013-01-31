#pragma once
#include <vector>
#include <boost/tuple/tuple.hpp>
#include "XmlLog.h"

namespace FileSystem {

typedef boost::tuple<std::string,std::string> name_val;
typedef std::vector<name_val> name_val_set;
typedef std::vector<name_val>::iterator name_val_it;

class CXmlRecord :
	public CXmlLog
{
public:
	CXmlRecord(std::string filename,int limit);
	CXmlRecord(std::string filename);
	virtual ~CXmlRecord(void);

	int AddRecord(name_val_set & val);
	int AddRecord(name_val_set & attrib,name_val_set & val);
	int AddRecordWithSynT(name_val_set & val);
	int AddRecordWithSynT(name_val_set & attrib,name_val_set & val);

protected:
	int RecordData(name_val_set & val,bool strSynT,CMarkup & xml);
	int RecordData(name_val_set & attrib,name_val_set & val,bool strSynT,CMarkup & xml);
};

}; //namespace FileSystem 
