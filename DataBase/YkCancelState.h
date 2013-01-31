#pragma once
#include "YkState.h"

namespace DataBase {

class CYkCancelState :
	public CYkState
{
public:
	CYkCancelState(void);
	virtual ~CYkCancelState(void);

	virtual int SendCancelEvent(CYkPoint & ykPoint);
	virtual int BackCancelEvent(CYkPoint & ykPoint);
	virtual int CancelResponEvent(CYkPoint & ykPoint);
};

}; //namespace DataBase
