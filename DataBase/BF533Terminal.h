#pragma once
#include "Terminal.h"

namespace DataBase {

const int CHANNELMAX = 40;     //最大通道数量
const int LINEMAX = 10;        //最大线路数

struct  stYCChannel//通道参数下装时所用结构体
{
	unsigned short Type    ;   //通道类型 0：电压 1：电流 2：直流 3：CVT
	unsigned int para[2]   ;   //通道校验参数
};

struct stBF533Base     //装置参数结构体
{
	unsigned int       Type        ;    //装置类型
	unsigned int       Version      ;   //DSP程序版本号
	unsigned int       YKHZTime  ;  	//开关合闸时间
	unsigned int       YKTZTime  ;  	//开关跳闸时间
	unsigned int       YXLvBoTime  ;  	//遥信滤波时间
	
	unsigned int       AotuRstYxTime ;  //故障遥信自动复归时间
	unsigned short     AotuRstYxFlag ;  //故障遥信自动复归投退标志
	unsigned short     HuoHua_AutoFlag; // 自动活化投退标志
	unsigned short     HuoHua_Day  ;    // 自动活化启动间隔天数
	unsigned short     Pro_Rst_Time;    //保护动作自动复归时间     ZHANGZHIHUA
	unsigned short     Flag_Pro_Rst;    //保护动作自动复归投退标志 ZHANGZHIHUA
	unsigned int       I_Rated;         //额定电流值

	unsigned int       ChannelSum ;     //通道数量
	unsigned int       SYXNUM;
	unsigned int       LineNum;
};

struct    stProVal  //保护定值及保护控制字结构体
{
	unsigned short OverLoad     ;      //过负荷投退字 0：退出 1：动作 
	unsigned short OL_Alarm_F     ;    //告警 ： 0：退出 1：告警
	unsigned int   OverLoadValue;         //过负荷定值  
	unsigned int   OverLoadTime;        //过负荷时间定值  
	unsigned int   OverLoadTZWait;        //过负荷跳闸延时  

	unsigned short   I_PRO     ;      //I段保护投退字 0：退出 1：动作  
	unsigned short   OF_Alarm_F     ; //告警 ： 0：退出 1：告警
	unsigned int  I_PROValue;         //I段保护定值  
	unsigned int  I_PROTime;        //I段保护时间定值  
	unsigned int  I_PROTZWait;       //跳闸延时  

	unsigned short   II_PRO;      //II段保护投退字 0：退出1：动作 
	unsigned short   II_Alarm_F     ; //告警 ： 0：退出 1：告警
	unsigned int  II_PROValue;         //II段保护定值  
	unsigned int  II_PROTime;        //II段保护时间定值  
	unsigned int  II_PROTZWait;       //跳闸延时  

	unsigned int  InverseFlag;    //反时限投退 0：退出1：投入
	unsigned int  InverseType;    //反时限类型 0：一般反时限,  1：非常反时限，2：极端反时限

	unsigned short   I0_PRO     ;      //零序电流投退字 0：退出1：告警2：动作 
	unsigned short   I0_Alarm_F ;
	unsigned int  I0_PROValue;         //零序电流定值  
	unsigned int  I0_PROTime;        //零序电流时间定值  
	unsigned int  I0_PROTZWait;

	unsigned short   I0_II_PRO     ;      //零序II电流投退字 0：退出1：告警2：动作 
	unsigned short   I0_II_Alarm_F ;
	unsigned int  I0_II_PROValue;         //零序II电流定值  
	unsigned int  I0_II_PROTime;        //零序II电流时间定值  
	unsigned int  I0_II_PROTZWait;

	unsigned short   U0_PRO     ;      //零序电压投退字 0：退出1：告警2：动作  
	unsigned int  U0_PROValue;         //零序电压定值  
	unsigned int  U0_PROTime;        //零序电压时间定值  

	unsigned int  Accel_T;        //后加速时间定值
	unsigned int  Accel_F;

	unsigned int   OverU_PRO_F; //过压动作  0：退出 1：告警
	unsigned int   OverU_Alarm_F; //过压告警  0：退出 1：告警
	unsigned int  OverU_P_Val;    //过压保护定值
	unsigned int  OverU_Ck_T;     //过压检故障时间
	unsigned int  OverU_PROTZWait;     //过压跳闸延时时间


	unsigned short   Reclose_PRO    ;    //重合闸投退字 0：退出1：告警  
	unsigned int  Reclose_PROTime;        //重合闸时间定值 

	unsigned short Low_PRO;          //低电压闭锁投退字 ZHANGZHIHUA
	unsigned int  Low_Lock;          //低电压闭锁定值   ZHANGZHIHUA 

	unsigned short FAFlag; 
	unsigned short Local_FAFlag;
	unsigned int   FACheckTime;
	unsigned int   FAOpenDalayTime;         
	unsigned int   FAPowerSideAddr;        
	unsigned int   FALoadSideAddr;   
	unsigned int   FAOverLoadLoseVolSum;
	unsigned int   FAReturnToZeroTime;
};
struct  stLinePara       //线路通道组合关系
{
	unsigned short Vol_Chan[3]   ;  //电压通道
	unsigned short Cur_Chan[3]   ;  //电流通道
	unsigned short BCur_Chan[3]; //保护电流通道
	unsigned short U0_Chan   ;  //零序电压通道
	unsigned short I0_Chan   ;  //零序电流通道	

	int            YxStart; //虚遥信起始地址
	unsigned short YkNo;//YK点号back
};

struct stEXTYCVAL 
{
	int Ua;
	int Ub;
	int Uc;
	int Ia;
	int Ib;
	int Ic;
	int U0;
	int I0;
	int Fre;
	int Fac;
	int AP;
	int RP;
};

class CBF533Terminal :
	public CTerminal
{
public:
	virtual ~CBF533Terminal(void);

	//xml cfg api
	virtual void SaveXmlCfg(FileSystem::CMarkup & xml);
	virtual int LoadXmlCfg(FileSystem::CMarkup & xml);

	int AddGeneralCmd(Protocol::CCmd cmdVal);
	int ActiveBattery(bool val);
	//int SynTime(void);
	int Reconnect();

	int getTerminalYcINum(int tIndex);
	

	static CBF533Terminal * getMyInstance(boost::asio::io_service & io_service,CSubStation & sub);

private:
	void setYcISendNum(int val);
	void setYcISendFeq(int val);
	int  getYcISendNum(void);
	void InitDefaultTimer(boost::asio::io_service & io_service);
	void InitDefaultTimeOut(void);

	void ResetTimerYcISend(bool bContinue = false,unsigned short val = 0);

	void handle_timerYcISend(const boost::system::error_code& error);

private:
	CBF533Terminal(boost::asio::io_service & io_service,CSubStation & sub);

private:
	enum BF533Default
	{
		DEFAULT_timeOutYcISend       = 4,
	};

	static CBF533Terminal * instance_;

	stBF533Base dev_para_;
	std::vector<stYCChannel> YcChannelSet_;//stYCChannel YcChannel_[CHANNELMAX];
	std::vector<stProVal> proValSet_; //stProVal proVal_[LINEMAX];
	std::vector<stLinePara> linePara_; //stLinePara linePara[LINEMAX];

	std::vector<int> TerminalYcINum_;

	bool YcISendFlage_;
	int YcISendNum_;
	int timeOutYcISendTime_;
	boost::scoped_ptr<boost::asio::deadline_timer> timerYcISend_;
};

};//namespace DataBase
