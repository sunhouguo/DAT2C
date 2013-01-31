#pragma once
#include "Protocol.h"
#include "BF533_CfgFile.h"

namespace Protocol {

	class CBF533
		:public CProtocol,
		public CBF533_CfgFile
	{
	public:
		CBF533(boost::asio::io_service & io_service);
		virtual ~CBF533(void);

		virtual int LoadXmlCfg(std::string filename);
		virtual void SaveXmlCfg(std::string filename);



		virtual int CheckFrameHead(unsigned char * buf,size_t & exceptedBytes);
		virtual int CheckFrameTail(unsigned char * buf,size_t exceptedBytes);
		virtual int ParseFrameBody(unsigned char * buf,size_t exceptedBytes);                  //返回通讯节点在容器中的序号

		virtual int AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd);
		virtual int AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd);
		virtual int AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd);

		virtual int InitProtocol();// 初始化规约


	private:
#define SYN_TIME         1200   //自动发送对时报文时间，单位s
#define BodyAddr         1      //计算校验和时的起始地址
#define CHANNELMAX       40     //最大通道数量
#define LINEMAX          10     //最大线路数

#define BF518Ver         133    //BF518版本号 1.33
#define DSPVer           133    //DSP版本号 1.33

#define TimerError        "My God!"

	public:
		size_t SOEStroeNum_ ;      //存储SOE的最大条数
		bool   SOEStroeFlag_;
		size_t FaultRecordMax_ ;     //存储故障事件最大条数
		bool   FaultRecordFlag_;
		char   YMDataRecordType_;       //积分电度的统计方式
		bool   YMDataFlag_ ;
		size_t YMDataRecordNum_;        //积分电度的记录最大条数
		size_t YMDataCount_;            //每一次记录的最大条数
		size_t DcChannelNo_;

		bool AutoDownloadCfgFlag_;
		std::string AutoDownloadCfgType_;

		unsigned short  UMaxLimit_;
		unsigned short  UMinLimit_;
		unsigned short  IMaxLimit_;
		unsigned short  IMinLimit_;
		unsigned short  ResetYcOverIndex_;
		unsigned short  NoCurDeadVal_;
		bool            NoCurRecord_;
		bool            YcOverAlarm_;






	public:

		struct stBF533Base     //装置参数下装时结构体
		{
			unsigned int       BoardType;
			unsigned int       Type        ;    //装置类型
			unsigned int       Version      ;    //DSP程序版本号
			unsigned int       YKHZTime  ;  	//开关合闸时间
			unsigned int       YKTZTime  ;  	//开关跳闸时间
			unsigned int       YXLvBoTime  ;  	//遥信滤波时间
			unsigned int       ChannelSum ;     //通道数量
			unsigned int       SYXNUM;
			unsigned int       LineNum;
			unsigned int       AotuRstYxTime ;   //故障遥信自动复归时间
			unsigned short     AotuRstYxFlag ;   	//故障遥信自动复归投退标志
			unsigned short     HuoHua_AutoFlag; // 自动活化投退标志
			unsigned short     HuoHua_Day  ;    // 自动活化启动间隔天数
			unsigned short     Pro_Rst_Time;    //保护动作自动复归时间     ZHANGZHIHUA
			unsigned short     Flag_Pro_Rst;    //保护动作自动复归投退标志 ZHANGZHIHUA
			unsigned int       I_Rated;         //额定电流值
			unsigned short     U_SwitchFlag;//电压切换投退字
			unsigned short     PT_L_Arlam;//PT断线投退字
		} BF533Base;	

		struct stBF533HBase     //装置参数召唤时结构体
		{
			unsigned int       BoardType;
			unsigned int       Type        ;    //装置类型
			unsigned int       Version      ;    //DSP程序版本号
			unsigned int       YKHZTime  ;  	//开关合闸时间
			unsigned int       YKTZTime  ;  	//开关跳闸时间
			unsigned int       YXLvBoTime  ;  	//遥信滤波时间
			unsigned int       ChannelSum ;     //通道数量
			unsigned int       SYXNUM;
			unsigned int       LineNum;
			unsigned int       AotuRstYxTime ;   //故障遥信自动复归时间
			unsigned short     AotuRstYxFlag ;   	//故障遥信自动复归投退标志
			unsigned short     HuoHua_AutoFlag; // 自动活化投退标志
			unsigned short     HuoHua_Day  ;   // 自动活化启动间隔天数
			unsigned short     Pro_Rst_Time;//保护动作自动复归时间     ZHANGZHIHUA
			unsigned short     Flag_Pro_Rst;//保护动作自动复归投退标志 ZHANGZHIHUA
			unsigned int       I_Rated;       //额定电流
            unsigned short     U_SwitchFlag;//电压切换投退字
			unsigned short     PT_L_Arlam;//PT断线投退字
		} BF533HBase;	

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
		}ExtYcVal[LINEMAX];

		struct  stYCChannel//通道参数下装时所用结构体
		{
			unsigned short Type    ;   //通道类型 0：电压 1：电流 2：直流 3：CVT
			unsigned int para[2]   ;   //通道校验参数
		}YcChannel[CHANNELMAX];

		struct  stHYCChannel//通道参数下装时所用结构体
		{
			unsigned short Type    ;   //通道类型 0：电压 1：电流 2：直流 3：CVT
			unsigned int para[2]   ;   //通道校验参数
		}HYcChannel[CHANNELMAX];


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
		}ProVal[LINEMAX];

		struct stProVal HProVal[LINEMAX];


		struct  stLinePara       //线路通道组合关系
		{
			unsigned short Vol_Chan[3]   ;  //电压通道
			unsigned short Cur_Chan[3]   ;  //电流通道
			unsigned short BCur_Chan[3]; //保护电流通道
			unsigned short U0_Chan   ;  //零序电压通道
			unsigned short I0_Chan   ;  //零序电流通道	

			int            YxStart; //虚遥信起始地址
			unsigned short YkNo;//YK点号back
			unsigned short IO_Flage;//线路是否为进线标志
		}LinePara[LINEMAX];


		struct  stHLinePara       //线路通道组合关系（召唤）
		{
			unsigned short Vol_Chan[3]   ;  //电压通道
			unsigned short Cur_Chan[3]   ;  //电流通道
			unsigned short BCur_Chan[3]   ;  //保护电流通道
			unsigned short U0_Chan   ;  //零序电压通道
			unsigned short I0_Chan   ;  //零序电流通道	

			int            YxStart; //虚遥信起始地址
			unsigned short YkNo;//YK点号back
			unsigned short IO_Flage;//线路是否为进线标志
		}HLinePara[LINEMAX];

		struct YMDATA
		{
			double YMDataVal;
		}YMData[LINEMAX];

		struct stNoCurVal 
		{
            unsigned short Ua;
			unsigned short Ub;
			unsigned short Uc;
			unsigned short Ia;
			unsigned short Ib;
			unsigned short Ic;
			bool           NoCurFlage;
			bool           Author_;
		}NoCurVal[LINEMAX];

		struct stYcOverVal 
		{
			bool            YcOveredFlage_;
		}YcOverVal[LINEMAX];


	private:
		//读取文件处理接口
		int Read_BasePara(void);
		int Read_DevSanyaoPara(void);
		int Read_ChanneltypePara(void);
		int Read_ProtectValPara(void);
		int Read_InterfacePara(void);
		int Read_LinePara(void);
		int Read_Master1YcZFTab(void);
		int Read_Master1YxZFTab(void);
		int Read_Master2YcZFTab(void);
		int Read_Master2YxZFTab(void);
		int LoadRecodFileCfg(void);
		//写入文件接口
		int Write_BasePara(void);//根据收到的装置参数报文解析出装置类型和版本号存入XML文件
		int Write_DevSanyaoPara(void);//根据收到的装置参数报文解析出的相关装置参数存入XML文件
		int Write_ChanneltypePara(void);//根据收到的通道类型报文将通道类型存入XML文件
		int Write_ProtectValPara(void);//根据收到的保护定值及保护控制字报文存入XML文件
		int Write_LinePara(void);//根据收到的通道组合关系报文将通道组合关系存入XML文件
		int Write_InterfacePara(void);

		void XML_Text(void);
		void XmlReadTest(void);
		void XmlWriteTest(void);

		int SaveHistoricalData(share_terminal_ptr terminalPtr);
		int SaveExtremum(share_terminal_ptr terminalPtr);
		int SaveYMData(int LineNum);
		int SaveFaultRecord(unsigned char * buf);
		int SaveSOERecord(int No,unsigned char Val,unsigned long Time);
		int SaveNoCurRecord(unsigned short LineNo);

		int SetBF518Para(void);
		int ModfiySetipSh(char Eth1IP[24],char Eth2IP[24],char Eth1MASK[24],char Eth2MASK[24]);
		std::string TransDoubleToString(double Data); //将传入的double数据转化为字符型数据
		std::string TransIntToString(int Data); //将传入的int数据转化为字符型数据
		char TransStringToDate(std::string Str);//将配置文件中的字符串转换成时间标志
		bool TransStringTobool(std::string Str);
		unsigned int  TransStringToInt(std::string Str);//将文件里的十六进制数转化成十进制
		std::string TransIntToHexString(unsigned int Data);//将收到的数值转化成十六进制标表示的字符串

		void setAutoDownloadCfgVal(std::string val);
		std::string getAutoDownloadCfgVal(void);
		void AutoDownloadCfg(share_terminal_ptr terminalPtr);

		void setUMaxLimit(unsigned short val);
		void setUMinLimit(unsigned short val);
		void setIMaxLimit(unsigned short val);
		void setIMinLimit(unsigned short val);
		void setResetYcOverIndex(unsigned short val);
		void setNoCurDeadVal(unsigned short val);
		//void setYcOveredFlage(bool val);
		unsigned short getUMaxLimit(void);
		unsigned short getUMinLimit(void);
		unsigned short getIMaxLimit(void);
		unsigned short getIMinLimit(void);
		unsigned short getResetYcOverIndex(void);
		unsigned short getNoCurDeadVal(void);
		//bool getYcOveredFlage(void);

	private:
		//报文发送处理函数
		int AssembleCallAllData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//组装总召报文
		int AssembleCallEquPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//召唤装置参数
		int AssembleSingleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code);   //下发遥控执行指令
		int AssembleYkSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//点遥控选择指示灯
		int AssembleYkCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//灭遥控选择指示灯
		int AssembleDownEquPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//下装装置参数
		int AssembleDownEquParaSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//下装装置参数成功
		int AssembleDownDelayVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int line_no);//下装保护定值
		int AssembleDownDelayValSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//下装保护定值成功
		int AssembleDownDelayContrl(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int line_no);//下装保护控制字
		int AssembleDownDelayContrlSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//下装保护控制字成功
		int AssembleSignalRest(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int line_no);//信号复归
		int AssembleSynTime(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,boost::posix_time::ptime time);//广播对时
		int AssembleDownChannelPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//下装通道参数
		int AssembleDownChannelParaSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//下装通道参数成功
		int AssembleDownChannelComrel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//下装通道组合关系
		int AssembleDownChannelComrelSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//下装通道组合关系成功
		int AssembleCallLineData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//召唤线路数据
		int AssembleDownLineVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd);//下装线路定值
		int AssembleLineValVer(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//线路定值校验
		int AssembleDspVersionInq(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//DSP版本查询
		int AssembleHarmonicAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd);
		int AssembleCallValCoef(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd);//召唤定系数
		int AssembleDownLoadLineHVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd);
		int AssembleBoardInquiry(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleCallProVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleCallChType(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleCallLinePara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleCallYMDataAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleBatteryActiveAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleBatteryActiveOverAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//电池活化退出
		int AssembleDownLineBVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd);//下装线路保护电流校验定值
		int AssembleLineValBVer(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);//线路保护电流定值校验
		int AssembleDcValBVer(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd);
		int AssembleCallPm(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd);
		int AssembleRebootBf533(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleTbootBf533(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleEraseBf533(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleDownBf533Pro(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd);


		//报文接收处理函数
	private:
		int getAddrByRecvFrame(unsigned char * buf);
		int ParseLongFrame(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseSingleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr);
		int ParseSingleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr);//YK 执行确认
		int ParseSingleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr);
		int ParseDownloadEquCeck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//装置参数下装反校
		int ParseDownloadDelayValCeck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//保护定值下装反校
		int ParseDownloadDelayContrlCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//保护控制字下装反校
		int ParseDownloadChannelParaCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//通道参数下装反校
		int ParseDownloadChannelComrelCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//通道组合关系下装反校
		int ParseDownloadLineValCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//线路定值下装反校
		int ParseDownloadLineCoefCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//线路通道系数下装反校
		int ParseAllYCData(unsigned char * buf, share_terminal_ptr terminalPtr);//全遥测报文
		int ParseLineCycTime(unsigned char * buf, share_terminal_ptr terminalPtr);//单线路全遥测定时发送报文
		int ParseAllYxCon(unsigned char * buf, share_terminal_ptr terminalPtr);//全遥信测报文
		int ParseSaveAllYX(int info_addr,share_terminal_ptr terminalPtr,unsigned char Data,unsigned char div,int offset);//存储全遥信报文
		int ParseSingleSOE(unsigned char * buf, share_terminal_ptr terminalPtr);//解析SOE报文
		int ParseLineValVerQyc(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//处理定值校验后全遥测
		int ParseHarmonicCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes); //召唤谐波回复
		int ParseDownloadDownDelayValSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseDownloadDownDelayContrlSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallValCoefCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseDownLoadLineCoef(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseBoardInqCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallEquCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallEquSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallProValCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallProVal(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallProContrl(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallProValSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallChTypeSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallChTypeAck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallLinePara(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallLineParaSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallYMDataCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseDownloadLineBValCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//线路保护电流校验定值下装反校
		int ParseLineBValVer(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//处理定值校验后全遥测
		int ParseFaultEvent(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCycDcIT(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);//直流量及温度定时上送
		int ParseDcValVerSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallPm(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseSingalReset(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseCallDspVersion(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseRebootBf533(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseTbootBf533(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseEraseBf533(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);
		int ParseDownBf533Pro(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes);

		int YcValOverAlarm(unsigned char * buf, share_terminal_ptr terminalPtr);
		int RecordLineLoad(unsigned char * buf, share_terminal_ptr terminalPtr);

		//定时器处理函数
	private:
		void InitDefaultTimer(boost::asio::io_service & io_service);

		//	void ResetTimerCallAllData (share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
		void ResetTimerSynTime(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
		void ResetTimerYkExe(share_commpoint_ptr point,int yk_no,bool bContinue = true ,unsigned short val = 0);
		void ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue = true ,unsigned short val = 0);
		void ResetTimerChannelError(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
		void ResetTimerSaveHis(share_terminal_ptr point,bool bContinue = true,unsigned short val = 0);
		void ResetTimerCallYMData(share_terminal_ptr point,bool bContinue = true,unsigned short val = 0);
		void ResetTimerAllData(share_terminal_ptr point,bool bContinue = true,unsigned short val = 0);
		void ResetTimerBattryActive(share_terminal_ptr point,bool bContinue = true,unsigned short val = 0);
		void ResetTimerBattryActiveOver(share_terminal_ptr point,bool bContinue = true,unsigned short val = 0);	
		void ResetTimerReboot(share_commpoint_ptr point,bool bContinue = false,unsigned short val = 0);
		void ResetTimerSingalReset(share_commpoint_ptr point,bool bContinue = false,unsigned short val = 0);
		void ResetTimerRebootBf533(share_terminal_ptr terminalPtr,bool bContinue = false,unsigned short val = 0);
		void ResetTimerTBootBf533(share_terminal_ptr terminalPtr,bool bContinue = false,unsigned short val = 0);

		//	void handle_timerCallAllDataTime(const boost::system::error_code& error,share_commpoint_ptr point);
		void handle_timerSynTime(const boost::system::error_code& error,share_commpoint_ptr point);
		void handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,int yk_no);
		void handle_timerChannelError(const boost::system::error_code& error,share_commpoint_ptr point);
		void handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
		void handle_timerSaveHis(const boost::system::error_code& error,share_terminal_ptr point);
		void handle_timerCallYMData(const boost::system::error_code& error,share_terminal_ptr point);
		void handle_timerAllData(const boost::system::error_code& error,share_terminal_ptr point);
		void handle_timerBattryActive(const boost::system::error_code& error,share_terminal_ptr point);
		void handle_timerBattryActiveOver(const boost::system::error_code& error,share_terminal_ptr point);
		void handle_timerReboot(const boost::system::error_code& error,share_commpoint_ptr point);
		void handle_timerSingalReset(const boost::system::error_code& error,share_commpoint_ptr point);
		void handle_timerRebootBf533(const boost::system::error_code& error,share_terminal_ptr terminalPtr);
		void handle_timerTBootBf533(const boost::system::error_code& error,share_terminal_ptr terminalPtr);

		//初始化相关函数
		void InitDefaultFrameElem();
		void InitDefaultTimeOut();
		void InitDefaultConfig();
		void InitDefaultFileData();//初始化文件存储相关默认参数
		void InitDefaultPara();
		void InitDefaultBF533Download();

		//打印信息接口
		void PrintMessage(const char PrintBuf[]);

	private:
		void setBf533ProName(std::string str);
		void setBf533AutoloadFlage(bool flage);
		bool getBf533AutoloadFlage(void);
		std::string getBf533ProName(void);

		//BF533变量
	private:
		unsigned short FrameTypeLocation_;                           //报文类型标识的字节定位
		unsigned short FrameTypeLength_;                             //报文类型标识的字节长度

	public:
		int  Line_No_ ;
		bool    FristFlag;
		size_t  SendTimes;
		size_t  Check_Sum;
		unsigned char Check_BF533Sum;
		size_t  ReSendTime;
		unsigned int Line_NUM;
		bool    bAssembleDelayValAll_;
		bool    bAllowTBoot_;
		bool    bAllowReboot_;
		bool    bYcAddRand_;//YC值随机增益
	private:
		bool    bBF533AutoDownload_;
		std::string BF533ProName;
		int This_Length_;
		int Total_NUM;
		int This_N0;

		//定时器处理函数
	private:
		//unsigned short timeOutCallAllDataTime_;
		//timerPtr CallAllDataTimer_;
		unsigned short timeOutSynTime_;
		timerPtr timerSynTime_;
		unsigned short timeOutYkExe_;
		timerPtr timerYkExe_;
		unsigned short timeOutChannelError_;
		timerPtr ChannelErrorTimer_;           //判断通道是否正常，不正常则总召
		unsigned short timeOutYkCancel_;
		timerPtr YkCancelTimer_;           //判断遥控选择之后是否收到遥控执行，超时则自动取消遥控
		unsigned short timeOutSaveHis_;
		timerPtr SaveHisTimer_; 
		unsigned short timeOutCallYMData_;
		timerPtr CallYMDataTimer_;            //定时召唤电度值
		unsigned short timeOutAllData_;
		timerPtr AllDataTimer_;            //定时召唤电度值
		unsigned short timeOutBattryActive_;
		timerPtr BattryActiveTimer_;            //定时启动电池活化
		unsigned short timeOutBattryActiveOver_;
		timerPtr BattryActiveOverTimer_;            //定时启动电池活化退出
		unsigned short timeOutSingalReset_;
		timerPtr SingalResetTimer_;            //定时启动电池活化退出
		unsigned short timeOutRebootBf533_;
		timerPtr ResetRebootBf533Timer_;
		unsigned short timeOutTBootBf533_;
		timerPtr ResetTBootBf533Timer_;      

		//定时器相关变量
	private:
		enum BF533Default
		{
			//DEFAULT_timeOutCallAllDataTime = 5,
			DEFAULT_timeOutYkCancel        = 10,
			DEFAULT_timeOutSaveHis         = 300,//300秒
			//MIN_timeOutCallAllData         = 1,
			DEFAULT_timeOutSynTime         = 1800,
			MIN_timeOutSynTime             = 300,
			DEFAULT_timeOutYkExe           = 5,
			DEFAULT_timeOutChannelError    = 10,
			MIN_timeOutChannelError        = 2,
			DEFAULT_timeOutCallYMData      = 300,   //整点召唤及存储电度数据 300秒
			DEFAULT_timeOutAllData         = 10,
			DEFAULT_SOEStroeNum            = 40,
			DEFAULT_timeOutReboot          = 3,             //3s之后复位
			DEFAULT_FaultRecordMax         = 20,
			DEFAULT_LineNum                = 10,//默认线路条数，主要作为获得直流量的入库地址
			DEFAULT_timeOutBattryActive    = 480,//7*24  //电池活化启动时间，单位为小时,20天
			DEFAULT_timeOutBattryActiveOver    = 120, //电池自动活化退出时间，单位为秒
			DEFAULT_timeOutSingalReset         = 3,
			DEFAULT_timeOutResetYcOver         = 10,    //遥测越限告警清零
			DEFAULT_timeOutRebootBf533         = 3,
			DEFAULT_timeOutTBootBf533          = 100
		};

	};//BF533

}; //Protocol 
