#pragma once
#include "YkState.h"

namespace DataBase {

class CYkReadyState :
	public CYkState
{
public:
	CYkReadyState(void);
	virtual ~CYkReadyState(void);

	virtual int RecvSelEvent(CYkPoint & ykPoint);
	virtual int BackExeEvent(CYkPoint & ykPoint);
	virtual int OverYkEvent(CYkPoint & ykPoint);
};

};//namespace DataBase
