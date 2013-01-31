#include "YkCancelState.h"
#include "YkReadyState.h"

namespace DataBase {

CYkCancelState::CYkCancelState(void)
{
}

CYkCancelState::~CYkCancelState(void)
{
}

int CYkCancelState::SendCancelEvent(CYkPoint & ykPoint)
{
	return 0;
}

int CYkCancelState::BackCancelEvent(CYkPoint & ykPoint)
{
	ChangeState(ykPoint,new CYkReadyState());

	return 0;
}

int CYkCancelState::CancelResponEvent(CYkPoint & ykPoint)
{
	return 0;
}

}; //namespace DataBase
