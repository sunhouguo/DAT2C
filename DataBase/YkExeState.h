#pragma once
#include "YkState.h"

namespace DataBase {

class CYkExeState :
	public CYkState
{
public:
	CYkExeState(void);
	virtual ~CYkExeState(void);

	virtual int SendExeEvent(CYkPoint & ykPoint);
	virtual int BackExeEvent(CYkPoint & ykPoint);
	virtual int ExeResponEvent(CYkPoint & ykPoint);

	virtual int RecvCancelEvent(CYkPoint & ykPoint);

	//virtual int OverYkEvent(CYkPoint & ykPoint);
};

}; //namespace DataBase
