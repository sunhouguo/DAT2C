#pragma once
#include "CommInterface.h"

namespace CommInterface {

class CCommPara;

const unsigned short IP_MATCHING = 0;
const unsigned short ID_MATCHING = 1;

#define strMatchByID "MatchByID"
#define strMatchByIP "MatchByIP"

class CServerInterface
{
public:
	CServerInterface(CCommPara & para,boost::asio::io_service& io_service);
	virtual ~CServerInterface(void);
	virtual int StartServer() = 0;
	virtual int InitServer(share_commpoint_ptr comm_point = share_commpoint_ptr());
	
	int setCmdRecallSlot(CmdRecallSlotType slotVal);

	int AddCommPoint(share_commpoint_ptr val);

protected:
	int DelCommPoint(int index);
	weak_commpoint_ptr getCommPoint(int index);
	int getCommPointSum();
	void ClearCommPoint();

	int TransMatchTypeFromString(std::string val);
	std::string TransMatchTypeToString(int val);

protected:
	CCommPara & para_;
	boost::asio::io_service& io_service_;

	boost::shared_ptr<CmdRecallSlotType> CmdRecallSlot_;

private:
	std::vector<weak_commpoint_ptr> commPoints_;     //属于本通道的通讯结点指针集合
	
};

}; //namespace CommInterface 
