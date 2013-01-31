#pragma once
#include "OperateNode.h"
#include "../DataBase/CommPoint.h"

namespace DistributedFA {

/**
* 邻接的开关结点类
*/
class CLinkNode
	:public COperateNode,
	public DataBase::CCommPoint
{
public:
	CLinkNode(boost::asio::io_service & io_service,DataBase::CDAOperate & da_op);
	virtual ~CLinkNode(void);

	virtual int getPosition();
	virtual int getProtect();
	virtual int OpenSwitchSel();
	virtual int OpenSwitchExe();
	virtual int CloseSwitchSel();
	virtual int CloseSwitchExe();

	//xml api
	int LoadXmlCfg(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);

	//虚函数接口
	virtual share_commpoint_ptr getSelfPtr();
	virtual SigConnection ConnectSubAliveSig(CmdRecallSlotType slotVal);
	virtual SigConnection ConnectSubTempSig(CmdRecallSlotType slotVal);

private:
	//int getCommIndex();
	//int setCommIndex(int val);
	//bool getCommAlive();

private:
	//int CommIndex_;
	DataBase::CDAOperate & da_op_;
};

}; //namespace DistributedFA
