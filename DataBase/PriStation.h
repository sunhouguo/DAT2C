#pragma once
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include "CommPoint.h"

namespace PublicSupport
{
	class CEfficientRouteTab;
}

namespace DataBase {

class CSubStation;
class CCosPoint;
class CSoePoint;
class CYcVarPoint;
class CYkPoint;
class CFaultPoint;
class CYkRecord;
class CCosRecord;
class CSoeRecord;
class CFaultRecord;
class CLine;

class CPriStation :
	public CCommPoint,
	public boost::enable_shared_from_this<CPriStation>
{
public:
	CPriStation(boost::asio::io_service & io_service,CSubStation & sub);
	virtual ~CPriStation(void);

	int InitPriStation(FileSystem::CMarkup & xml);
	void UninitPriStation();

	//cos api
	void InitCosLoadPtr();
	int getCosPointNum();                                //没做yx转发表点号检查前的cos数量
	int getFirstCosPoint(CCosPoint & outVal);                 //查找第一个符合转发表点号的点
	int loadCosPoints(CCosPoint * cosBuf,size_t count,typeYxtype & yxType);  //经过yx转发表点号检查并修改以后的结果

	//soe api
	void InitSoeLoadPtr();                                
	int getSoePointNum();                                 //没做yx转发表点号检查前的soe数量
	int getFirstSoePoint(CSoePoint & outVal);                  //查找第一个符合转发表点号的点
	int loadSoePoints(CSoePoint * soeBuf,size_t count,typeYxtype & yxType);   //经过yx转发表点号检查并修改以后的结果

	//ycvar api
	void InitYcvarLoadPtr();                              
	int getYcvarPointNum();	                                   //没做yc转发表点号检查前的ycvar数量
	int getFirstYcvarPoint(CYcVarPoint & outVal);              //查找第一个符合转发表点号的点
	int loadYcvarPoints(CYcVarPoint * ycvarBuf,size_t count);  //经过yc转发表点号检查并修改以后的结果
	typeFinalYcval getFinalYcVarVal(CYcVarPoint & var);        //这个接口是给对主站的通讯规约用的，它假设使用输入参数var.getYcIndex()获得是主站视图的点号（经过转发表转换），如果在CSubStation或者其他位置调用这个接口请自行留意点号的转换。

	//yx api
	size_t getYxSum();
	size_t getSubYxSum();
	typeYxval getOriYxVal(size_t index);
	int setOriYxVal(size_t index, typeYxval val);
	typeYxtype getYxType(size_t index);
	int setYxType(size_t index,typeYxtype val);
	bool getYxPolar(size_t index);
	int setYxPolar(size_t index,bool val);
	typeYxQuality getYxQuality(size_t index);
	int setYxQuality(size_t index,typeYxQuality val);
	typeFinalYcval getFinalYxVal(size_t index);

	//yc api
	size_t getYcSum();
	size_t getSubYcSum();
	typeYcval getOriYcVal(size_t index);
	int setOriYcVal(size_t index,typeYcval val);
	typeYcplus getYcPlus(size_t index);
	int setYcPlus(size_t index,typeYcplus val);
	typeYcmul getYcMul(size_t index);
	int setYcMul(size_t index,typeYcmul val);
	typeYcquality getYcQuality(size_t index);
	int setYcQuality(size_t index,typeYcquality val);
	typeYcdead getYcDeadLimit(size_t index);
	int setYcDeadLimit(size_t index,typeYcdead val);
	typeFinalYcval getFinalYcVal(size_t index,bool bUseMul = false);

	//yk api
	size_t getYkSum();
	size_t getSubYkSum();
	//typeYkstatus getYkStatus(size_t index);
	//int setYkStatus(size_t index,typeYkstatus val);
	typeYktype getYkType(size_t index);
	int setYkType(size_t index,typeYkstatus val);
	bool getbHYkDouble(size_t index);
	int setbHYkDouble(size_t index,bool val);
	bool getbSYkDouble(size_t index);
	int setbSYkDouble(size_t index,bool val);
	int AddYkSelCmd(size_t index,bool bCloseOrOpen);
	int AddYkExeCmd(size_t index,bool bCloseOrOpen);
	int AddYkCancelCmd(size_t index,bool bCloseOrOpen = true);
	int AddBF533Cmd(int tIndex,Protocol::CCmd cmdVal); //用于增加常规指令的传递
	int getTerminalYcINum(int tIndex);
	CYkPoint * getYkPointPtr(int index);

	//ym api
	size_t getYmSum();
	size_t getSubYmSum();
	typeYmval getOriYmVal(size_t index);
	int setOriYmVal(size_t index,typeYmval val);
	typeYcquality getYmQuality(size_t index);
	int setYmQuality(size_t index,typeYmquality val);

	//line api
	size_t getLineSum();
	CLine * getLinePtr(int index);

	//index transform
	int PriyxToSubyx(size_t val);
	int SubyxToPriyx(size_t val);
	int PriycToSubyc(size_t val);
	int SubycToPriyc(size_t val);
	int PriykToSubyk(size_t val);
	int SubykToPriyk(size_t val);
	int PriymToSubym(size_t val);
	int SubymToPriym(size_t val);

	//虚函数接口
	//virtual void AddSelfPointToProtocol();
	//virtual void AddSelfPointToTcpServerPtr();
	//virtual int ResetTimerRecv(size_t LostRecvTimeOut);
	//virtual void handle_timerRecv(const boost::system::error_code& error);
	//virtual int ConnectCmdRecallSig();
	virtual SigConnection ConnectSubAliveSig(CmdRecallSlotType slotVal);
	virtual SigConnection ConnectSubTempSig(CmdRecallSlotType slotVal);
	virtual share_commpoint_ptr getSelfPtr();
	
	//xml cfg api
	void SaveXmlCfg(FileSystem::CMarkup & xml);
	int LoadXmlCfg(FileSystem::CMarkup & xml);

	//protocol api
	bool getLastFcbFlag();
	int setLastFcbFlag(bool val);
	bool getSendFcbFlag();
	int setSendFcbFlag(bool val);

	//local api
	int WriteSysTime(unsigned short year,unsigned char month,unsigned char day,unsigned char hour,unsigned char minnutes,unsigned char seconds,unsigned short milliseconds,bool bSynTerminal = true);

	//tab api
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> getYxTabPtr();
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> getYcTabPtr();
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> getYkTabPtr();
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> getYmTabPtr();

	//comm active api
	virtual bool getCommActive();
	virtual void setCommActive(bool val);
	bool getCommActieBackup();
	void setCommActiveBackup(bool val);

	int setbAcceptSynSubTime(bool val);

	//frame api
	int CallAllData();
	int SynAllTime();

	int AddGeneralCmd(int tIndex,Protocol::CCmd cmdVal); //用于增加常规指令的传递
	int AddSpeCmd(int tIndex,Protocol::CCmd cmdVal); //用于增加常规指令的传递
private:
	bool getbAcceptSynSubTime();
	
	bool getbAuthor();
	int setbAuthor(bool val);
	void ResetAuthorisedFlagByLocalPort(std::string strPort);

	int AddYkRecord(unsigned char recordType,int yk_no,bool bCloseOrOpen);
	int AddCosRecord(const CCosPoint & cos);
	int AddSoeRecord(const CSoePoint & soe);
	int AddFaultRecord(const CFaultPoint & fault);

private:
	CSubStation & sub_;

	bool bAcceptSynSubTime_;
	bool bSynTimeImmediately_;
	bool bCallDataImmediately_;
	bool bAddGeneralCmd_;
	bool bAddSpeCmd_;

	bool bAuthorised_;
	bool bLastFcbFlag_;
	bool bSendFcbFlag_;

	boost::shared_ptr<PublicSupport::CEfficientRouteTab> yxTab_;
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> ycTab_;
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> ymTab_;
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> ykTab_;
	int UseYxTabIndex_;
	int UseYcTabIndex_;
	int UseYmTabIndex_;
	int UseYkTabIndex_;

	size_t cosLoadPtr_;
	size_t soeLoadPtr_;
	size_t ycvarLoadPtr_;

	//record data
	boost::scoped_ptr<CYkRecord> YkRecordPtr_;
	boost::scoped_ptr<CSoeRecord> SoeRecordPtr_;
	boost::scoped_ptr<CCosRecord> CosRecordPtr_;
	boost::scoped_ptr<CFaultRecord> FaultRecordPtr_;

	//comm active backup
	bool bCommActiveBackup_;

	bool bycmul_;
};

};//namespace DataBase


