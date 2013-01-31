#include "YkReadyState.h"
#include "YkSelState.h"

namespace DataBase {

CYkReadyState::CYkReadyState(void)
{
}

CYkReadyState::~CYkReadyState(void)
{
}

int CYkReadyState::RecvSelEvent( CYkPoint & ykPoint )
{
	ChangeState(ykPoint,new CYkSelState());

	return 0;
}

int CYkReadyState::BackExeEvent(CYkPoint & ykPoint)
{
	ChangeState(ykPoint,new CYkReadyState());

	return 0;
}

int CYkReadyState::OverYkEvent(CYkPoint & ykPoint)
{
	ChangeState(ykPoint,new CYkReadyState());

	return 0;
}

}; //namespace DataBase 
