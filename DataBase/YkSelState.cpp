#include "YkSelState.h"
#include "YkCancelState.h"
#include "YkExeState.h"

namespace DataBase {

CYkSelState::CYkSelState(void)
{
}

CYkSelState::~CYkSelState(void)
{
}

int CYkSelState::SendSelEvent(CYkPoint & ykPoint)
{
	return 0;
}

int CYkSelState::BackSelEvent(CYkPoint & ykPoint)
{
	return 0;
}

int CYkSelState::SelResponEvent(CYkPoint & ykPoint)
{
	return 0;
}

int CYkSelState::RecvExeEvent(CYkPoint & ykPoint)
{
	ChangeState(ykPoint,new CYkExeState());

	return 0;
}

int CYkSelState::RecvCancelEvent(CYkPoint & ykPoint)
{
	ChangeState(ykPoint,new CYkCancelState());

	return 0;
}

}; //namespace DataBase

