#pragma once

namespace DistributedFA {

const int PositionClose = 1;
const int PositionOpen = 0;
const int ProtectedPositive = 1;
const int ProtectedNagetive = 0;

/**
* 开关结点的接口定义
*/
class CSWNode
{
public:
	CSWNode(void);
	virtual ~CSWNode(void);

	virtual int getPosition() = 0;
	virtual int getProtect() = 0;
};

}; //namespace DistributedFA
