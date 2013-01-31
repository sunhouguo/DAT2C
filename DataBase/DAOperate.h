#pragma once
#include "../PublicSupport/Dat2cTypeDef.h"

namespace DataBase {

class CDAOperate
{
public:
	CDAOperate();
	virtual ~CDAOperate(void);

	virtual size_t getYxSum() = 0;
	virtual typeYxval getYxVal(size_t index) = 0;
	virtual typeYxtype getYxType(size_t index) = 0;

	virtual size_t getYkSum() = 0;
	virtual int AddYkSelCmd(size_t index,bool bCloseOrOpen) = 0;
	virtual int AddYkExeCmd(size_t index,bool bCloseOrOpen) = 0;

	virtual size_t getYcSum() = 0;
	virtual typeYcval getYcVal(size_t index) = 0;

	virtual SigConnection ConnectYkSig(CmdRecallSlotType slot) = 0;
	virtual void TrigCosEvent(size_t index,typeYxval val,bool bSingleType) = 0;
	//virtual int SaveOriYxVal(size_t index,typeYxval val,bool bSingleType) =0;
};

};//namespace DataBase

