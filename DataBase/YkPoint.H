#pragma once
#include <boost/scoped_ptr.hpp>
#include "../PublicSupport/Dat2cTypeDef.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

#define strYkDouble "DoubleYk"

class CYkState;

//yk 状态定义
//const unsigned char YkReady = 0;                              //0
//const unsigned char YkSelTimeOut = YkReady + 1;               //1
//const unsigned char YkSelSend = YkSelTimeOut + 1;             //2
//const unsigned char YkSelCon = YkSelSend + 1;                 //3
//const unsigned char YkSelRecv = YkSelCon + 1;                 //4
//const unsigned char YkSelBack = YkSelRecv + 1;                //5
//const unsigned char YkExeTimeOut = YkSelBack + 1;             //6
//const unsigned char YkExeSend = YkExeTimeOut + 1;             //7
//const unsigned char YkExeCon = YkExeSend + 1;                 //8
//const unsigned char YkExeRecv = YkExeCon + 1;                 //9
//const unsigned char YkExeBack = YkExeRecv + 1;                //10
//const unsigned char YkCancelTimeOut = YkExeBack + 1;          //11
//const unsigned char YkCancelSend = YkCancelTimeOut + 1;       //12
//const unsigned char YkCancelCon = YkCancelSend + 1;           //13
//const unsigned char YkCancelRecv = YkCancelCon + 1;           //14
//const unsigned char YkCancelBack = YkCancelRecv + 1;          //15
//const unsigned char YkOver = YkCancelBack + 1;                //16

//yk 类型定义
const unsigned char YkClose = 1;
const unsigned char YkOpen = 0;
const char YkOtherType = -1;

struct stYkCmdPara
{
	unsigned short YkNo_;
	unsigned char YkCode_;

	stYkCmdPara(unsigned short yk_no,unsigned char yk_code)
		:YkNo_(yk_no),
		YkCode_(yk_code)
	{

	}

	stYkCmdPara()
	{

	}
};

class CYkPoint
{
public:
	CYkPoint();
	CYkPoint(CYkPoint const & rhs);
	CYkPoint & operator=(CYkPoint const & rhs);
	~CYkPoint(void);

	//typeYkstatus getYkStatus();
	//int setYkStatus(typeYkstatus val);
	typeYktype getYkType();
	int setYkType(typeYktype val);
	bool getbHYkDouble();
	int setbHYkDouble(bool val);
	bool getbSYkDouble();
	int setbSYkDouble(bool val);
	bool getbYkAllow();
	int setbYkAllow(bool val);

	//xml api
	void SaveXmlCfg(FileSystem::CMarkup & xml);
	int LoadXmlCfg(FileSystem::CMarkup & xml);
	std::string TransYkDoubleToString(bool val);
	bool TransYkDoulbeFromString(std::string val);

	//ykstate api
	int RecvSelEvent();
	int SendSelEvent();
	int BackSelEvent();
	int SelResponEvent();

	int RecvExeEvent();
	int SendExeEvent();
	int BackExeEvent();
	int ExeResponEvent();

	int RecvCancelEvent();
	int SendCancelEvent();
	int BackCancelEvent();
	int CancelResponEvent();

	int OverYkEvent();

	int TimeOutEvent();

	int ClearYkState();

	//static bool CheckYkStatusDevelopWithSel(typeYkstatus curStatus,typeYkstatus nextStatus);
	//static bool CheckYkStatusDevelopNoSel(typeYkstatus curStatus,typeYkstatus nextStatus);

private:
	void ChangeState(CYkState * state);

public:
	enum
	{
		DefaultYkDouble = true,
	};

protected:
	enum
	{	
		//默认值
		//DefaultYkStatus = YkReady,
		DefaultYkType = YkOtherType,

		//DefaultClearYkStatusTime = 60, //单位 秒
	};

	friend class CYkState;

	bool ykAllow_;
	bool ykHDouble_;
	bool ykSDouble_;
	//typeYkstatus ykStatus_;
	typeYktype ykType_;

	boost::scoped_ptr<CYkState> ykState_;
};

}; //namespace DataBase 




