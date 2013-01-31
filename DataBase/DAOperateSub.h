#pragma once
#include "DAOperate.h"

namespace DataBase
{
	class CSubStation;
}

namespace DataBase {

class CDAOperateSub :
	public CDAOperate
{
public:
	CDAOperateSub(DataBase::CSubStation & sub);
	virtual ~CDAOperateSub(void);

	virtual size_t getYxSum();
	virtual typeYxval getYxVal(size_t index);
	virtual typeYxtype getYxType(size_t index);

	virtual size_t getYkSum();
	virtual int AddYkSelCmd(size_t index,bool bCloseOrOpen);
	virtual int AddYkExeCmd(size_t index,bool bCloseOrOpen);

	virtual size_t getYcSum();
	virtual typeYcval getYcVal(size_t index);

	virtual SigConnection ConnectYkSig(CmdRecallSlotType slot);
	virtual void TrigCosEvent(size_t index,typeYxval val,bool bSingleType);
	//virtual int SaveOriYxVal(size_t index,typeYxval val,bool bSingleType);

private:
	DataBase::CSubStation & sub_;
};

};//namespace DataBase 



