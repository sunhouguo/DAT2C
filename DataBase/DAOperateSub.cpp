#include "DAOperateSub.h"
#include "SubStation.h"
#include "YkPoint.h"

namespace DataBase {

CDAOperateSub::CDAOperateSub(DataBase::CSubStation & sub)
							:sub_(sub)
{
}

CDAOperateSub::~CDAOperateSub(void)
{
}

size_t CDAOperateSub::getYxSum()
{
	return sub_.getYxSum();
}

typeYxval CDAOperateSub::getYxVal(size_t index)
{
	return sub_.getFinalYxVal(index);
}

typeYxtype CDAOperateSub::getYxType(size_t index)
{
	return sub_.getYxType(index);
}

size_t CDAOperateSub::getYkSum()
{
	return sub_.getYkSum();
}

int CDAOperateSub::AddYkSelCmd(size_t index,bool bCloseOrOpen)
{
	if(!sub_.getYkPointPtr(index)->RecvSelEvent())
	{
		return sub_.AddYkSelCmd(index,bCloseOrOpen);
	}

	return -1;
}

int CDAOperateSub::AddYkExeCmd(size_t index,bool bCloseOrOpen)
{
	if(!sub_.getYkPointPtr(index)->RecvExeEvent())
	{
		return sub_.AddYkExeCmd(index,bCloseOrOpen);
	}

	return -1;
}

SigConnection CDAOperateSub::ConnectYkSig(CmdRecallSlotType slot)
{
	return sub_.ConnectSubTempSig(slot);
}

void CDAOperateSub::TrigCosEvent(size_t index,typeYxval val,bool bSingleType)
{
	return sub_.TrigCosEvent(index,val,bSingleType);
}

//int CDAOperateSub::SaveOriYxVal(size_t index,typeYxval val,bool bSingleType)
//{
//	return sub_.setOriYxValDateUp(index,val);
//}

size_t CDAOperateSub::getYcSum()
{
	return sub_.getYcSum();
}

typeYcval CDAOperateSub::getYcVal(size_t index)
{
	return sub_.getFinalYcVal(index);
}

}; //namespace DataBase 
