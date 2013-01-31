#include "YkReadyState.h"
#include "YkPoint.h"

namespace DataBase {

CYkState::CYkState(void)
{
}

CYkState::~CYkState(void)
{
}

void CYkState::ChangeState( CYkPoint & point,CYkState * state )
{
	point.ChangeState(state);
}

int CYkState::RecvSelEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::SendSelEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::BackSelEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::SelResponEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::RecvExeEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::SendExeEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::BackExeEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::ExeResponEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::RecvCancelEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::SendCancelEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::BackCancelEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::CancelResponEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::OverYkEvent(CYkPoint & ykPoint)
{
	ChangeState(ykPoint,new CYkReadyState());

	return -1;
}

int CYkState::TimeOutEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkReadyState());

	return 0;
}

}; //namespace DataBase 
