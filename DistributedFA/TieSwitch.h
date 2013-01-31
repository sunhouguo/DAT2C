#pragma once
#include "OperateNode.h"

namespace DistributedFA {

class CTieSwitch :
	public COperateNode
{
public:
	CTieSwitch(DataBase::CDAOperate & da_op);
	virtual ~CTieSwitch(void);
};

};//namespace DistributedFA
