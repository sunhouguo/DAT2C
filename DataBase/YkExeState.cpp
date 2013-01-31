#include "YkExeState.h"
#include "YkCancelState.h"
#include "YkReadyState.h"

namespace DataBase {

CYkExeState::CYkExeState(void)
{
}

CYkExeState::~CYkExeState(void)
{
}

int CYkExeState::SendExeEvent(CYkPoint & ykPoint)
{
	return 0;
}

int CYkExeState::BackExeEvent(CYkPoint & ykPoint)
{
	ChangeState(ykPoint,new CYkReadyState());

	return 0;
}

int CYkExeState::ExeResponEvent(CYkPoint & ykPoint)
{
	return 0;
}

int CYkExeState::RecvCancelEvent(CYkPoint & ykPoint)
{
	ChangeState(ykPoint,new CYkCancelState());

	return 0;
}

//int CYkExeState::OverYkEvent(CYkPoint & ykPoint)
//{
//	//ChangeState(ykPoint,new CYkReadyState());
//
//	return 0;
//}

}; //namespace DataBase
