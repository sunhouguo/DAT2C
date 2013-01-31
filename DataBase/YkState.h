#pragma once
#ifndef YkState_H
#define YkState_H

namespace DataBase {

class CYkPoint;

class CYkState
{
public:
	CYkState(void);
	virtual ~CYkState(void);

	virtual int RecvSelEvent(CYkPoint & ykPoint);
	virtual int SendSelEvent(CYkPoint & ykPoint);
	virtual int BackSelEvent(CYkPoint & ykPoint);
	virtual int SelResponEvent(CYkPoint & ykPoint);

	virtual int RecvExeEvent(CYkPoint & ykPoint);
	virtual int SendExeEvent(CYkPoint & ykPoint);
	virtual int BackExeEvent(CYkPoint & ykPoint);
	virtual int ExeResponEvent(CYkPoint & ykPoint);

	virtual int RecvCancelEvent(CYkPoint & ykPoint);
	virtual int SendCancelEvent(CYkPoint & ykPoint);
	virtual int BackCancelEvent(CYkPoint & ykPoint);
	virtual int CancelResponEvent(CYkPoint & ykPoint);

	virtual int OverYkEvent(CYkPoint & ykPoint);

	virtual int TimeOutEvent(CYkPoint & ykPoint);

protected:
	void ChangeState(CYkPoint & point,CYkState * state);
};

};//namespace DataBase

#endif //#ifndef YkState_H
