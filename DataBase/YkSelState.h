#pragma once
#include "YkState.h"

namespace DataBase {

class CYkSelState :
	public CYkState
{
public:
	CYkSelState(void);
	virtual ~CYkSelState(void);

	virtual int SendSelEvent(CYkPoint & ykPoint);
	virtual int BackSelEvent(CYkPoint & ykPoint);
	virtual int SelResponEvent(CYkPoint & ykPoint);

	virtual int RecvExeEvent(CYkPoint & ykPoint);

	virtual int RecvCancelEvent(CYkPoint & ykPoint);
};

};//namespace DataBase 
