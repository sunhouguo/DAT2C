#pragma once
#include <boost/scoped_ptr.hpp>

namespace DataBase
{
	class CDAOperate;
}

namespace DistributedFA {

class COperateNode;
class CLinkNode;

/**
* 描述本地开关
*/
class CSelfNode
{
public:
	CSelfNode(DataBase::CDAOperate & da_op);
	~CSelfNode(void);

private:
	bool CheckTieSwitch();
	bool CheckSelfFault();

	int getPosition();
	int getProtect();
	int OpenSwitchSel();
	int OpenSwitchExe();
	int CloseSwitchSel();
	int CloseSwitchExe();

	void ChangeNodeType(COperateNode * nodeType);

private:
	//self attrib
	boost::scoped_ptr<COperateNode> selfNodeType_;

	//link attrib
	boost::scoped_ptr<CLinkNode> powerNode_;
	boost::scoped_ptr<CLinkNode> loadNode_;
};

}; //namespace DistributedFA

