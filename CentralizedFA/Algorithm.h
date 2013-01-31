#pragma once
#include <vector>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include "MyBTree.h"
#include "SwitchNode.h"

#include <boost/asio.hpp>

namespace FileSystem
{
	class CLog;
}

namespace DataBase
{
	class CDAOperate;
}
namespace CentralizedFA {

typedef boost::shared_ptr<CSwitchNode> share_switchnode_ptr;

class CCentralizedDA
{
public:
	CCentralizedDA(/*boost::asio::io_service & io_service,*/std::string id,DataBase::CDAOperate & da_op);
	~CCentralizedDA(void);
	void ProcessAlgorithmSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);

	int InitAlgorithm();
	void UninitAlgorithm();

	int start(CmdRecallSignalType & sig);
	void stop();

	share_switchnode_ptr getWitchNodeByID(typeID val);

	std::string getID();

	int LoadXmlCfg(std::string filename);
	void SaveXmlCfg(std::string filename);

private:
	//sig api
	int ConnectSubAlgorithmSig(CmdRecallSignalType & sig);
	int DisconnectSubAlgorithmSig();

	int AddOutLetSwitch(share_outletswitch_ptr outlet);

	//tree api
	int BuildForest();
	int BuildLeafQueue(std::queue<share_switchnode_ptr> & leafQueue);

	int CheckFault();
	int SearchFaultNode(std::queue<share_switchnode_ptr> & powerQueue);
	int Insulate(std::queue<share_switchnode_ptr> & insulateQueue); //隔离
	int Resume(std::queue<share_switchnode_ptr> & resumeQueue);     //恢复
	int OutPutYk();

	//thread api
	void thread_fun_da();

	void ClearAlgorithmData();

	//log api
	int AddLog(std::string val);

	//in out
	int CheckInput();
	bool CheckInputYxSet();
	int WriteOutputYx(int outputindex,typeYxval val,bool bSingleType  = true);
	int WriteInputYx(stIn st,typeYxval val ,bool bSingleType = true);

	//fault deal
	bool CheckFaultAlreadyExist(share_switchnode_ptr node);
	bool CheckNewFaultNode();

	//yk api
	int OpenSwitch(share_switchnode_ptr node);
	int CloseSwitch(share_switchnode_ptr node);
	int OpenOutlet(share_outletswitch_ptr node);
	int CloseOutlet(share_outletswitch_ptr node);
	int WaitYkResult(share_outletswitch_ptr node, boost::posix_time::ptime begin_time,bool excepted_swichposition);

	//void handle_timerSignReset(const boost::system::error_code& error);
	//void ResetTimerSignReset(bool bContinue,unsigned short timeVal);
	//void handle_timerGetLoad(const boost::system::error_code& error);
	//void ResetTimerGetLoad(bool bContinue,unsigned short timeVal);
	//void GetLoad();
	//int CheckLoadOver();
	//int GetSwitchLoad();
	//bool CheckFASwitchDisable();

	//bool ResetOutputYx(typeYxval val,bool bSingleType);

	//init
	void ResetCfg();

	//out put yx
	void FaAct(bool act);
	void FaMulFault(bool act);

private:
	enum
	{
		wait_yxcon_timeout = 20,//单位：秒
	};

	//boost::posix_time::ptime lastTime_;

	DataBase::CDAOperate & da_op_;

	//log
	boost::scoped_ptr<FileSystem::CLog> log_;

	SigConnection SubAlgorithmSigConnection_;
	
	//输入
	CMyBTree<share_switchnode_ptr> tree_;
	std::vector<share_switchnode_ptr> OriSwitchNodes_;

	//输出
	std::vector<share_switchnode_ptr> faultSet_;
	std::vector<share_switchnode_ptr> ykOpenSet_;
	std::vector<share_switchnode_ptr> ykCloseSet_;
	std::vector<share_outletswitch_ptr> outLetswitchSet_;

	//da配置文件名字
	std::string id_;

	//size_t YcOverLoadI_;
	//size_t YcOverLoadII_;
 //   size_t YcOverLoadValue1_;
	//size_t YcOverLoadValue2_;

	//size_t CheckOverLoad_;
	//bool   bCheckOverLoad_;
    int   CheckFADisable_;

	//size_t OverLoadValue_;

	//bool  bSwitchRefuseSign_;
	//bool  bDisableOutLed_;

	size_t WaitYxTime_;
	size_t WaitYkTime_;

	//thread
	boost::mutex yxMutex_;
	boost::condition_variable yxCond_;
	bool thread_run_;

	//in out
	std::vector<stIn> input_set_;
	std::vector<stOut> output_set_;

	//int SignResetTime_;
	//boost::asio::io_service & io_service_;
	//boost::asio::deadline_timer TimerSignReset_;
	//boost::asio::deadline_timer TimerGetLoad_;
	
	bool bReset_;
};

}; //namespace FeederAutomation 
