#pragma once
#include "DataRecord.h"

namespace DataBase {

const unsigned char yk_sel = 1;
const unsigned char yk_exe = 2;
const unsigned char yk_cancel = 3;
const unsigned char yk_over = 4;
const unsigned char yk_sel_fail = 5;
const unsigned char yk_exe_fail = 6;
const unsigned char yk_cancel_fail = 7;
const unsigned char yk_sel_con = 8;
const unsigned char yk_exe_con = 9;
const unsigned char yk_cancel_con = 10;

class CYkRecord :
	public CDataRecord
{
public:
	CYkRecord(std::string id,int limit);
	CYkRecord(std::string id);
	virtual ~CYkRecord(void);

	int AddDataRecord(unsigned char yk_event,int yk_no,bool bCloseOrOpen);
};

}; //namespace DataBase
