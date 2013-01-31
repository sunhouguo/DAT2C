

namespace DataBase {
	//以下为BF518/BF533通讯相关结构体
	struct PDZ101_STRUCT
	{
		unsigned char   CMD_IMAG;             //报文镜像,用于ACK、NAK指令时
		unsigned long int Check_Sum; //校验和
		unsigned int    SendTimes;
		unsigned int    ReSendTime;
	};

	struct LINEPPARA
	{
		unsigned int LineNUM;
	};
    
	struct stEVENT
	{
		unsigned char Type; //1:BF533 2:other

		unsigned int  No;
		unsigned long EventTime;
		unsigned char EventD_Type;
		unsigned char EventC_Type;
		unsigned int EventValue;

		unsigned char LineNo;
		unsigned int  FaultNo;
		unsigned char SPE;
		unsigned int  YcValue;
		unsigned int  millisecond;
		unsigned char min;
		unsigned char hour;
		unsigned char day;
		unsigned char month;
		unsigned int  year;
	};

	struct stJBPARA
	{
		unsigned char  Num;
		unsigned char  LineNo[48];
		unsigned int ParaNo[48];
		unsigned int Value[48];
	};

	struct stPMPara
	{
		unsigned char timeout_flag;
		unsigned int  LineNo;
		unsigned int  AngU1;
		unsigned int  AngU2;
		unsigned int  AngU3;
		unsigned int  AngCI1;
		unsigned int  AngCI2;
		unsigned int  AngCI3;
		unsigned int  AngBI1;
		unsigned int  AngBI2;
		unsigned int  AngBI3;
		unsigned int  AngU0;
		unsigned int  AngI0;
	};

	struct stLine_Val
	{
		unsigned char timeout_flag;
		size_t       Line_no;
		unsigned int Flag_Ua;
		unsigned int Flag_Ub;
		unsigned int Flag_Uc;
		unsigned int Flag_CIa;
		unsigned int Flag_CIb;
		unsigned int Flag_CIc;
		unsigned int Flag_BIa;
		unsigned int Flag_BIb;
		unsigned int Flag_BIc;
		unsigned int Flag_U0;
		unsigned int Flag_I0;
		unsigned int Flag_Angle_UaIa;
		unsigned int Flag_Angle_UbIb;
		unsigned int Flag_Angle_UcIc;
		unsigned int Flag_Fre_Val;
		unsigned int Flag_Cos_Val;
		unsigned int Flag_P_Val;
		unsigned int Flag_Q_Val;

		unsigned int D_Value_Ua;
		unsigned int D_Value_Ub;
		unsigned int D_Value_Uc;
		unsigned int D_Value_CIa;
		unsigned int D_Value_CIb;
		unsigned int D_Value_CIc;
		unsigned int D_Value_BIa;
		unsigned int D_Value_BIb;
		unsigned int D_Value_BIc;
		unsigned int D_Value_U0;
		unsigned int D_Value_I0;
		unsigned int Angle_UaIa;
		unsigned int Angle_UbIb;
		unsigned int Angle_UcIc;
		unsigned int Fre_Val;
		unsigned int Cos_Val;
		unsigned int P_Val;
		unsigned int Q_Val;
	};

	struct stHarmonic
	{
		size_t       Line_no;
		size_t       harmonic_no;

		unsigned int Value_Ua;
		unsigned int Value_Ub;
		unsigned int Value_Uc;
		unsigned int Value_Ia;
		unsigned int Value_Ib;
		unsigned int Value_Ic;
		unsigned int Value_U0;
		unsigned int Value_I0;
		unsigned int P_Val;
		unsigned int Q_Val;
		unsigned int Per_Val;
	};

	struct stLine_ValCoef  //线路定值系数
	{
		unsigned char timeout_flag;
		size_t       Line_no;

		unsigned int Flag_Ua;
		unsigned int Flag_Ub;
		unsigned int Flag_Uc;
		unsigned int Flag_CIa;
		unsigned int Flag_CIb;
		unsigned int Flag_CIc;
		unsigned int Flag_BIa;
		unsigned int Flag_BIb;
		unsigned int Flag_BIc;
		unsigned int Flag_U0;
		unsigned int Flag_I0;
		unsigned int Flag_Angle_UaIa;
		unsigned int Flag_Angle_UbIb;
		unsigned int Flag_Angle_UcIc;
		unsigned int Flag_Fre_Val;
		unsigned int Flag_Cos_Val;
		unsigned int Flag_P_Val;
		unsigned int Flag_Q_Val;

		unsigned int H_Value_Ua;
		unsigned int H_Value_Ub;
		unsigned int H_Value_Uc;
		unsigned int H_Value_CIa;
		unsigned int H_Value_CIb;
		unsigned int H_Value_CIc;
		unsigned int H_Value_BIa;
		unsigned int H_Value_BIb;
		unsigned int H_Value_BIc;
		unsigned int H_Value_U0;
		unsigned int H_Value_I0;
		unsigned int Angle_UaIa;
		unsigned int Angle_UbIb;
		unsigned int Angle_UcIc;
	};

	struct stBoardInq
	{
		unsigned int YkBoardNum;
		unsigned int YxBoardNum;
		unsigned int YcBoardNum;
	};

	struct stLine_BCurVal  //保护电流定值系数
	{
		size_t       Line_No;
		unsigned int D_Val_BI1;
		unsigned int D_Val_BI2;
        unsigned int D_Val_BI3;
	};

	struct stLine_BCurVerVal  //保护电流定值系数
	{
		size_t       timeout_flag;
		size_t       Line_No;
		unsigned int V_Val_BI1;
		unsigned int V_Val_BI2;
		unsigned int V_Val_BI3;
	};

	struct stDCVerVal //直流量校验
	{
	   unsigned char timeout_flag; //此处用于通知PDA校验成功或者失败
       size_t  ChannelNo;
	   unsigned int V_Val_DC;
	};

	struct stDCValCoef //直流量校验系数查询
	{
		size_t  ChannelNo;
		unsigned int V_HVal_DC;
	};

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
	} ;	//BF533Base

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
};//namespace DataBase

