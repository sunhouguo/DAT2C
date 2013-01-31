#pragma once
#include "OperateNode.h"

namespace DistributedFA {

class CLoadSwtich :
	public COperateNode
{
public:
	CLoadSwtich(DataBase::CDAOperate & da_op);
	virtual ~CLoadSwtich(void);
};

};//namespace DistributedFA
