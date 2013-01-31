#include <boost/bind.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <stdio.h>
#include <fstream>
#include "BF533.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/Terminal.h"
#include "../DataBase/YkPoint.h"
#include "../DataBase/YxPoint.h"
#include "../DataBase/FaultPoint.h"
#include "BF533_DataBase.h"
//#if !defined(_WIN32)
//#include <time.h>
//#include <sys/time.h>
//#endif

#define  IFCONFIGETH1   "ifconfig eth1 "
#define  IFCONFIGETH2   "ifconfig eth2 "
#define  IP_SH_NAME     "setip.sh"        //设置IP脚本文件名
#define  SETNETMASK     " netmask "

namespace Protocol {

const std::string strDefaultCfg = "Bf533Cfg.xml";
const unsigned char Frist_ACT = 0x01;             //第一次上电总召标志

//针对BF533规约的YK功能码
const unsigned char DYK_TYPE_OPEN = 0x01;         //单点遥控分闸操作码
const unsigned char DYK_TYPE_CLOSE = 0x02;        //单点遥控合闸操作码
const unsigned char SYK_TYPE_OPEN  = 0x33;        //双点遥控分闸操作码
const unsigned char SYK_TYPE_CLOSE = 0xCC;        //双点遥控合闸操作码

#define DayRecordFlag         0   //电度按天统计
#define MonthRecordFlag       1   //电度按月统计
#define YearRecordFlag        2   //电度按年统计

#define FileLen               128
#define SYNTIMEMULTIPLE       1.6  //给BF533对时时乘以的倍数

#define strAutoDownloadCfg           "AutoDownloadCfg"
#define strDownloadDevSanyaoPara     "DevSanyaoPara"
#define strDownloadChanneltypePara   "ChanneltypePara"
#define strDownloadProtectValPara    "ProtectValPara"
#define strDownloadLinePara          "LinePara"
#define strDownloadAll               "All"
#define strYcOverAlarm               "YcOverAlarm"
#define strUMaxLimit                 "UMaxLimit"
#define strUMinLimit                 "UMinLimit"
#define strIMaxLimit                 "IMaxLimit"
#define strIMinLimit                 "IMinLimit"
#define strNoCurRecord               "NoCurRecord"
#define strNoCurDeadVal              "NoCurDeadVal"
#define strBf533AutoDownload         "Bf533AutoDownload"
#define strBf533ProName              "Bf533ProName"
#define strYcAddRand                 "YcAddRand"



//BF533规约类型标识定义
//下行方向
const unsigned char C_Call_AllData        = 0x20; //总召唤
const unsigned char C_Yk_Exe              = 0x21; //遥控执行
const unsigned char C_Call_EquPara        = 0x22; //召唤装置参数
const unsigned char C_Down_EquPara        = 0x23; //下装装置参数
const unsigned char C_DownSu_EquPara      = 0x24; //装置参数下装成功
const unsigned char C_Down_DelayVal       = 0x25; //下装保护定值
const unsigned char C_DownSu_DelayVal     = 0x26; //下装保护定值成功
const unsigned char C_Down_DelayContrl    = 0x27; //下装保护控制字
const unsigned char C_DownSu_DelayContrl  = 0x28; //下装保护控制字成功
const unsigned char C_Signal_Rest         = 0x29; //远方信号复归
const unsigned char C_DcValVer            = 0x33; 
const unsigned char C_Syn_Time            = 0xFF; //系统对时
const unsigned char C_Down_ChannelPara    = 0x2A; //通道类型下装
const unsigned char C_DownSu_ChannelPara  = 0x2B; //通道类型下装成功
const unsigned char C_Down_ChannelComrel  = 0x2C; //下装通道组合关系
const unsigned char C_DownSu_ChannelComrel= 0x2D; //下装通道组合关系成功
const unsigned char C_Call_LineData       = 0x2E; //召唤线路数据
const unsigned char C_Down_LineVal        = 0x2F; //线路定值下装
const unsigned char C_Line_ValVer         = 0x30; //线路定值校验
const unsigned char C_Down_LineHVal       = 0x31; //线路通道系数下装
const unsigned char C_Inq_DspVersion      = 0x32; //DSP版本查询
const unsigned char C_Call_Harmonic       = 0x37; //召唤谐波分量数据
const unsigned char C_BoardInquiry        = 0x39; //板件查询
const unsigned char C_Call_ProVal         = 0x3A; //保护定值查询
const unsigned char C_Call_ChType         = 0x3B; //通道类型查询
const unsigned char C_Call_LinePara       = 0x3C; //召唤通道组合关系
const unsigned char C_Call_YmData         = 0x3D; //召唤电度数据
const unsigned char C_BatteryActive       = 0x3E; //远方电池活化开始
const unsigned char C_BatteryActiveOver   = 0x3F; //电池活化退出
const unsigned char C_YkSel_Ack           = 0xA0; //遥控选择点灯
const unsigned char C_YkCancel_Ack        = 0xA1; //遥控取消灭灯
const unsigned char C_Down_LineBVal       = 0xA3; //保护电流定值校验定值下装
const unsigned char C_LineValBVer         = 0xA4; //保护电流定值校验
const unsigned char C_LineCallPm          = 0xA5; 
const unsigned char C_RebootBf533         = 0xE0; 
const unsigned char C_EraseBf533          = 0xE1; 
const unsigned char C_DownBf533Pro        = 0xE2; 


//上行方向
const unsigned char R_AllYc               = 0x40;//全遥测
const unsigned char R_Call_EquPara        = 0x41;//召唤装置参数，装置参数下装反校
const unsigned char R_SOE                 = 0x42;//SOE报文
const unsigned char R_Yk_Exe              = 0x43;//遥控执行成功
const unsigned char R_Down_EquPara        = 0x44;//装置参数下装成功
const unsigned char R_Down_DVal_Check     = 0x45;//保护定值下装反校
const unsigned char R_Down_DVal           = 0x46;//保护定值下装成功
const unsigned char R_Down_DContrl_Check  = 0x47;//下装保护控制字反校
const unsigned char R_Down_DContrl        = 0x48;//保护控制字下装成功
const unsigned char R_Signal_Rest         = 0x49;//远方复归成功
const unsigned char R_Down_ChPara_Check   = 0x4A;//通道类型下装反校
const unsigned char R_Down_ChannelPara    = 0x4B;//通道类型下装成功
const unsigned char R_Down_ChComrel_Check = 0x4C;//通道组合关系下装反校
const unsigned char R_Down_ChannelComrel  = 0x4D;//通道组合关系下装成功
const unsigned char R_Line_SynYc          = 0x4E;//单线路定时全遥测
const unsigned char R_AllYx               = 0x4F;//全遥信报文
const unsigned char R_Down_LineVal_Check  = 0x50;//线路定值下装反校
const unsigned char R_Qyc_ValVer          = 0x51;//定值校验后全遥测
const unsigned char R_Line_ValVer         = 0x52;//线路定值校验成功
const unsigned char R_Down_LineHVal_Check = 0x53;//线路定值系数下装
const unsigned char R_Fault_Event         = 0x54;//故障记录报文
const unsigned char R_Yk_Failed           = 0x55;//继电器跳闸失败
const unsigned char R_Timer_IT            = 0x56;//温度及直流量定时上送
const unsigned char R_DcVal_Ver           = 0x58;//直流量校验成功
const unsigned char R_Call_Harmonic       = 0x59;//召唤谐波分量回复
const unsigned char R_BoardInquiry        = 0x5B;//板件查询回复
const unsigned char R_Call_EquPara_Sucess = 0x5C;//召唤装置参数成功
const unsigned char R_Call_ProVal         = 0x5D;//召唤保护定值确认
const unsigned char R_Call_ProVal_Sucess  = 0x5E;//召唤保护定值成功
const unsigned char R_Call_ChType         = 0x5F;//召唤通道类型成功
const unsigned char R_Call_LinePara       = 0x61;//召唤通道组合关系成功
const unsigned char R_Call_YmData         = 0x62;//召唤电度量应答报文
const unsigned char R_FA_Event            = 0x63;//主动上送故障电流信息(FA)
const unsigned char R_BatteryActive       = 0x64;//远方电池活化确认
const unsigned char R_BatteryActiveOver   = 0x65;//远方活化退出确认
const unsigned char R_YkSel_Ack           = 0x66;//遥控选择点灯确认
const unsigned char R_YkCancel_Ack        = 0x67;//遥控撤消灭灯确认
const unsigned char R_Down_LineBVal_Check = 0x69;//线路保护电流校验定值下装反校
const unsigned char R_LineValBVer         = 0x6A;//保护电流定值校验后的电流值
const unsigned char R_LineValBVer_Sucess  = 0x6B;//保护电流定值校验成功
const unsigned char R_Call_Pm             = 0x6C;//
const unsigned char R_Call_DSPVer         = 0x60;
const unsigned char R_RebootBf533         = 0xF0; 
const unsigned char R_EraseBf533          = 0xF1; 
const unsigned char R_DownBf533Pro        = 0xF2; 


CBF533::CBF533(boost::asio::io_service & io_service)
	:CProtocol(io_service),CBF533_CfgFile(io_service)
{
	bActiveRepeatFrame_ = true;     //需要回复标志
	bYcAddRand_ = false;
	SynCharNum_ = 4;                //确定检查报文头是传入的字节数
	InitDefaultFrameElem();//初始化默认报文构成位置
	InitDefaultFileData();
	LoadRecodFileCfg();
	InitDefaultConfig();
	InitDefaultPara();
	InitDefaultBF533Download();
	InitDefaultTimer(io_service);
	
	LoadXmlCfg(strDefaultCfg);
	bAssembleDelayValAll_=false;
}

CBF533::~CBF533(void)
{

}

int CBF533::InitProtocol()
{
	CProtocol::InitProtocol();

	if(getCommPointSum() > 0)
	{
		share_commpoint_ptr nextPoint = getFirstCommPoint();

		if (nextPoint)
		{
			if (getBf533AutoloadFlage())
			{
				FileHandleBegain(getBf533ProName());
				if (FileHandleReadFor533Pro() > 0)
				{
					Total_NUM = ((FileHandleGetTotalLength() - 1) / FileLen) + 1;
				}
				AddSendCmdVal(REBOOT_BF533,REBOOT_BF533_PRIORITY,nextPoint);
				bAllowTBoot_ = true;
				bAllowReboot_ = true;
			}
			else
			{
				bAllowTBoot_ = false;
				bAllowReboot_ = false;
				AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,nextPoint);
			}
		}
	}

	PrintMessage("BF533打开规约通道成功！");

	return 0;
}

int CBF533::LoadXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;

	if (!xml.Load(filename))
	{
		return -1;
	}

	xml.ResetMainPos();
	xml.FindElem();  //root strProtocolRoot
	xml.IntoElem();  //enter strProtocolRoot

	CProtocol::LoadXmlCfg(xml);

	AutoDownloadCfgFlag_ = false;

	xml.ResetMainPos();
	if (xml.FindElem(strAutoDownloadCfg))
	{
		AutoDownloadCfgFlag_ = true;
		std::string strTmp = xml.GetData();
		setAutoDownloadCfgVal(strTmp);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYcOverAlarm))
	{
		YcOverAlarm_ = true;

		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem(strUMaxLimit))
		{
			try
			{
				std::string strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				setUMaxLimit(boost::lexical_cast<unsigned short>(strTmp));
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<"非法的遥测越限参数:"<<e.what();

				throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem(strUMinLimit))
		{
			try
			{
				std::string strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				setUMinLimit(boost::lexical_cast<unsigned short>(strTmp));
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<"非法的遥测越限参数:"<<e.what();

				throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem(strIMaxLimit))
		{
			try
			{
				std::string strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				setIMaxLimit(boost::lexical_cast<unsigned short>(strTmp));
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<"非法的遥测越限参数:"<<e.what();

				throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem(strIMinLimit))
		{
			try
			{
				std::string strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				setIMinLimit(boost::lexical_cast<unsigned short>(strTmp));
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<"非法的遥测越限参数:"<<e.what();

				throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
			}
		}

		xml.OutOfElem();
	}

	xml.ResetMainPos();
	if (xml.FindElem(strNoCurRecord))
	{
		NoCurRecord_ = true;

		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem(strNoCurDeadVal))
		{
			try
			{
				std::string strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				setNoCurDeadVal(boost::lexical_cast<unsigned short>(strTmp));
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<"非法的遥测越限参数:"<<e.what();

				throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
			}
		}

		xml.OutOfElem();
	}
	
	xml.ResetMainPos();
	if (xml.FindElem(strBf533AutoDownload))//BF533自动下载程序配置
	{
		setBf533AutoloadFlage(true);

		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem(strBf533ProName))
		{
			try
			{
				std::string strTmp = xml.GetData();
				setBf533ProName(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<"非法的程序自动下载参数:"<<e.what();

				throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
			}
		}
		xml.OutOfElem();
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYcAddRand))
	{
		try
		{
			std::string strTmp = xml.GetData();
			bYcAddRand_ = TransStringTobool(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的遥测随机数增益参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CBF533::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();

	CProtocol::SaveXmlCfg(xml);

	xml.OutOfElem();

	xml.Save(filename);
}


int CBF533::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	if ((buf[0] == 0xEB) && (buf[1] == 0x90))
	{
		exceptedBytes = (buf[2]+buf[3]*256) + 5;
		return 0;
	}
	return -1;
}

int CBF533::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	if (buf[exceptedBytes -1] == 0x03)
	{
		return 0;
	}
	return -1;
}

int CBF533::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	int ret = 0;

	unsigned char funcType = buf[0];

	int Addr = getAddrByRecvFrame(buf);//检查报文地址与终端地址是否匹配
	if (Addr < 0)
	{
		return Addr;
	}

	int terminalIndex =0;
	share_terminal_ptr terminalPtr;
	if (terminalIndex >= 0)
	{
		setLastRecvPointIndex(terminalIndex);
		terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(getCommPoint(terminalIndex).lock());
	}

	if (!terminalPtr)
	{
		std::ostringstream ostr;
		ostr<<"BF533规约不能根据接收报文中的地址匹配terminal ptr,这帧报文将不会被解析。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (funcType)
	{
	case 0xEB:
		ret = ParseLongFrame(buf,terminalPtr,exceptedBytes);
		break;

	default:
		{
			std::ostringstream ostr;
			ostr<<"未定义的报文头,buf[0] = "<<funcType<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;
	}

	if (ret < 0)
	{
		return ret;
	}

	if (FristFlag == true)
	{
		//PrintMessage("初始化历史数据定时器... ...");
		//ResetTimerSaveHis(terminalPtr,true,0);

		if (YMDataFlag_)
		{
			ResetTimerCallYMData(terminalPtr,true,0);
		}

		FristFlag = false;
	}
	return terminalIndex;
}

//组装报文头
int CBF533::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	switch (cmd.getCmdType())
	{
	case T_BOOT_BF533:
		//buf[count++] = 0x7E;
		break;

	default:
		buf[count++] = 0xEB;
		buf[count++] = 0x90;
		buf[count++] = 0x00;
		buf[count++] = 0x00;
		break;
	}
	return count - bufIndex;
}

//组装报文体
int CBF533::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	int bytesAssemble = 0;
	size_t count = bufIndex;

	share_terminal_ptr terminalPtr;

	if (cmd.getCommPoint()) //获得通信点
	{
		if (cmd.getCommPoint()->getCommPointType() == TERMINAL_NODE)
		{
			terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(cmd.getCommPoint());
		}
	}

	if (!terminalPtr)
	{
		std::ostringstream ostr;
		ostr<<"BF533规约不能从发送命令中获得terminal ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (cmd.getCmdType())
	{
	case CALL_ALL_DATA_ACT:
		{
			PrintMessage("下行：总召");
			bytesAssemble = AssembleCallAllData(bufIndex,buf,terminalPtr);
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
				ResetTimerAllData(terminalPtr,true,0);
				//ResetTimerCallAllData(terminalPtr,true,10);//启动等待总召回复定时器
			}
		}

		break;
	case YK_SEL_ACT:
		{

			int yk_no = boost::any_cast<int>(cmd.getVal());

			typeYktype yk_type = DataBase::YkOtherType;
			yk_type = terminalPtr->getYkType(yk_no);

			//terminalPtr->setYkStatus(yk_no,DataBase::YkSelCon);

			if((terminalPtr->loadYkPointPtr(yk_no))->SendSelEvent())
			{
				//std::ostringstream ostr;
				//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkSelSend<<std::endl;
				//AddStatusLogWithSynT(ostr.str());
				AddStatusLogWithSynT("解析遥控选择命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
				return -1;
			}

			CmdConSig_(YK_SEL_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);

			PrintMessage("下行：遥控选择点灯");

			bytesAssemble = AssembleYkSel(bufIndex,buf,terminalPtr);//开始点灯

			//ResetTimerYkCancel(terminalPtr,yk_no,true,0);

		}
		break;
	case YK_CANCEL_ACT:
		{

			//PrintMessage("下行：遥控选择灭灯1");
			int yk_no = boost::any_cast<int>(cmd.getVal());

			typeYktype yk_type = DataBase::YkOtherType;
			yk_type = terminalPtr->getYkType(yk_no);
			//PrintMessage("下行：遥控选择灭灯2");
			//terminalPtr->setYkStatus(yk_no,DataBase::YkCancelCon);

			//if((terminalPtr->loadYkPointPtr(yk_no))->SendSelEvent())
			//{
			//	//std::ostringstream ostr;
			//	//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkSelSend<<std::endl;
			//	//AddStatusLogWithSynT(ostr.str());
			//	AddStatusLogWithSynT("解析遥控选择命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
			//	return -1;
			//}
			//PrintMessage("下行：遥控选择灭灯3");
			CmdConSig_(YK_CANCEL_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);

			PrintMessage("下行：遥控选择灭灯");

			bytesAssemble = AssembleYkCancel(bufIndex,buf,terminalPtr);//开始灭灯
			//        ResetTimerYkCancel(terminalPtr,yk_no,false,0);
		}
		break;
	case YK_EXE_ACT:
		{
			int yk_no;
			try
			{
				yk_no = boost::any_cast<int>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控执行命令的遥控点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			typeYktype yk_type = DataBase::YkOtherType;
			try
			{
				yk_type = terminalPtr->getYkType(yk_no);
			}
			catch(PublicSupport::dat2def_exception & err)
			{
				std::ostringstream ostr;
				ostr<<"遥控执行命令的遥控点号参数错误："<<err.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			//if (!DataBase::CYkPoint::CheckYkStatusDevelopNoSel(terminalPtr->getYkStatus(yk_no),DataBase::YkExeSend))
			//{
			//	std::ostringstream ostr;
			//	ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkExeSend<<std::endl;
			//	AddStatusLogWithSynT(ostr.str());
			//	AddStatusLogWithSynT("解析遥控执行命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
			//	return -1;
			//}

			PrintMessage("下行：遥控执行");
			if (yk_type == DataBase::YkClose)
			{
				bytesAssemble = AssembleSingleYKExe(bufIndex,buf,terminalPtr, yk_no,SYK_TYPE_CLOSE);
			}
			else if (yk_type == DataBase::YkOpen)
			{
				bytesAssemble = AssembleSingleYKExe(bufIndex,buf,terminalPtr, yk_no,SYK_TYPE_OPEN);
			} 

			if (bytesAssemble > 0)
			{
				if((terminalPtr->loadYkPointPtr(yk_no))->SendExeEvent())
				{
					//std::ostringstream ostr;
					//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkExeSend<<std::endl;
					//AddStatusLogWithSynT(ostr.str());
					AddStatusLogWithSynT("解析遥控执行命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
					return -1;
				}

				//			PrintMessage("遥控执行完成！");
				//terminalPtr->setYkStatus(yk_no,DataBase::YkExeSend);
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
				ResetTimerYkExe(terminalPtr,yk_no,true);//启用定时器，若遥控执行超时则上送超时标志
			}

		}
		break;
	case DOWNLOAD_EQU_PARA:
		PrintMessage("下行：装置参数下装");
		Read_BasePara();
		Read_DevSanyaoPara();
		bytesAssemble = AssembleDownEquPara(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			SendTimes ++;
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case DOWNLOAD_EQU_PARA_SUCESS:
		PrintMessage("下行：装置参数下装成功");
		bytesAssemble = AssembleDownEquParaSucess(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case DOWNLOAD_DELAY_VAL:
		{
			PrintMessage("下行：保护定值下装");

			int line_no = boost::any_cast<int>(cmd.getVal());

			//PrintMessage("获取线路号成功... ...mm");
			//if(line_no == 0)
			//{
			//	Read_ProtectValPara();
			//	PrintMessage("读取配置文件成功... .mm");
			//}

			Read_ProtectValPara();

			if (!bAssembleDelayValAll_)
			{
				if(line_no==0xff)
				{
					bAssembleDelayValAll_=true;
					line_no=0;
				}
				else
				{
					bAssembleDelayValAll_=false;
				}
			}

			Line_No_ = line_no;                             //用于判断是否所有线路都下装完成
			//std::cout<<"保护定值下装,本次下装的是第"<< Line_No_ << "条线路，总线路数为："<< BF533Base.LineNum <<std::endl; 
			bytesAssemble = AssembleDownDelayVal(bufIndex,buf,terminalPtr,line_no);
			if (bytesAssemble > 0)
			{
				SendTimes ++;
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case DOWNLOAD_DELAY_VAL_SUCESS:
		PrintMessage("下行：保护定值下装成功");
		bytesAssemble = AssembleDownDelayValSucess(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case DOWNLOAD_DELAY_CONRL:
		{
			PrintMessage("下行：保护控制字下装");

			int line_no = boost::any_cast<int>(cmd.getVal());

			Line_No_ = line_no;
			std::cout<<"保护控制字下装,本次下装的是第"<< Line_No_ << "条线路，总线路数为："<< BF533Base.LineNum <<std::endl; 
			bytesAssemble = AssembleDownDelayContrl(bufIndex,buf,terminalPtr,Line_No_);
			if (bytesAssemble > 0)
			{
				SendTimes ++;
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case DOWNLOAD_DELAY_CONTRL_SUCESS:
		PrintMessage("下行：保护控制字下装成功");
		bytesAssemble = AssembleDownDelayContrlSucess(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case SIGNAL_RESET:
		{
			PrintMessage("下行：远方信号复归");
			int lineno = boost::any_cast<int>(cmd.getVal());
			bytesAssemble = AssembleSignalRest(bufIndex,buf,terminalPtr,lineno);
			if (bytesAssemble > 0)
			{
				ResetTimerSingalReset(terminalPtr,true,0);
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
			break;
		}
	case SYN_TIME_ACT:
		PrintMessage("下行：广播对时");
		bytesAssemble = AssembleSynTime(bufIndex,buf,terminalPtr,boost::posix_time::microsec_clock::local_time());
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case DOWNLOAD_CHANNEL_PARA:
		PrintMessage("下行：通道参数下装");

		Read_ChanneltypePara();

		bytesAssemble = AssembleDownChannelPara(bufIndex,buf,terminalPtr);

		if (bytesAssemble > 0)
		{
			SendTimes ++;
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case DOWNLOAD_CHANNEL_PARA_SUCESS:
		PrintMessage("下行：通道参数下装成功");
		bytesAssemble = AssembleDownChannelParaSucess(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case DOWNLOAD_CHANNEL_COMREL:
		PrintMessage("下行：通道组合关系下装");

		Read_LinePara();

		bytesAssemble = AssembleDownChannelComrel(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			SendTimes ++;
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case DOWNLOAD_CHANNEL_COMREL_SUCESS:
		PrintMessage("下行：通道组合关系下装成功");
		bytesAssemble = AssembleDownChannelComrelSucess(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case CALL_LINE_DATA_ACT:
		PrintMessage("下行：线路数据召唤");
		bytesAssemble = AssembleCallLineData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case DOWNLOAD_LINE_VAL:
		{

			PrintMessage("下行：线路定值下装");

			bytesAssemble = AssembleDownLineVal(bufIndex,buf,terminalPtr,cmd);
			if (bytesAssemble > 0)
			{
				SendTimes ++;
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}		
		break;
	case LINE_VAL_VER:
		PrintMessage("下行：线路定值校验");
		bytesAssemble = AssembleLineValVer(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;
	case HARMONIC_ACK:
		{
			PrintMessage("下行：召唤谐波数据");
			bytesAssemble = AssembleHarmonicAck(bufIndex,buf,terminalPtr,cmd);
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case DSP_VERSION_INQ:
		PrintMessage("下行：DSP版本查询");
		bytesAssemble = AssembleDspVersionInq(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}

		break;
	case CALL_VALCOEF_ACK:
		{
			PrintMessage("下行：召唤定值系数");
			bytesAssemble = AssembleCallValCoef(bufIndex,buf,terminalPtr,cmd);
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case DOWNLOAD_LINE_COEF:
		{
			PrintMessage("下行：下装定值系数");
			bytesAssemble = AssembleDownLoadLineHVal(bufIndex,buf,terminalPtr,cmd);
			if (bytesAssemble > 0)
			{
				SendTimes ++;
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case BOARD_REQ_ACK:
		{
			PrintMessage("下行：板件查询");
			bytesAssemble = AssembleBoardInquiry(bufIndex,buf,terminalPtr);//板件查询
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;

	case CALL_EQU_PARA_ACT:
		PrintMessage("下行：召唤装置参数");
		bytesAssemble = AssembleCallEquPara(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
		}
		break;

	case CALL_PROVAL_ACK:
		{
			PrintMessage("下行：召唤保护定值及保护控制字");
			bytesAssemble = AssembleCallProVal(bufIndex,buf,terminalPtr);//板件查询
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case CALL_CHTYPE_ACK:
		{
			PrintMessage("下行：召唤通道类型");
			bytesAssemble = AssembleCallChType(bufIndex,buf,terminalPtr);//板件查询
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case CALL_LINEPARA_ACK:
		{
			PrintMessage("下行：召唤通道组合关系");
			bytesAssemble = AssembleCallLinePara(bufIndex,buf,terminalPtr);//板件查询
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case CALL_YMDATA_ACK:
		{
			PrintMessage("下行：召唤积分电度数据");
			bytesAssemble = AssembleCallYMDataAck(bufIndex,buf,terminalPtr);//板件查询
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case BATTERY_ACTIVE:
		{
			PrintMessage("下行：启动电池活化");
			bytesAssemble = AssembleBatteryActiveAck(bufIndex,buf,terminalPtr);//电池活化启动
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case SET_BF518_PARA:
		{
			PrintMessage("收到接口参数文件，修改IP地址... ...");
			SetBF518Para();
		}
		break;
	case BATTERY_ACTIVE_OVER:
		{
			PrintMessage("下行：电池活化退出");
			bytesAssemble = AssembleBatteryActiveOverAck(bufIndex,buf,terminalPtr);//电池活化退出
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}	
		}
		break;
	case DOWNLOAD_LINE_BVAL:
		{
			PrintMessage("下行：保护电流校验定值下装");
			bytesAssemble = AssembleDownLineBVal(bufIndex,buf,terminalPtr,cmd);//电池活化退出

			if (bytesAssemble > 0)
			{
				SendTimes ++;
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case LINE_BVAL_VER:
		{
			PrintMessage("下行：保护电流校验定值");
			bytesAssemble = AssembleLineValBVer(bufIndex,buf,terminalPtr);//电池活化退出

			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case LINE_DCVAL_VER:
		{
			PrintMessage("下行：直流量电流校验定值");
			bytesAssemble = AssembleDcValBVer(bufIndex,buf,terminalPtr,cmd);//电池活化退出

			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case CALL_PM_ANG:
		{
			PrintMessage("下行：召唤绝对相角");
			bytesAssemble = AssembleCallPm(bufIndex,buf,terminalPtr,cmd);//电池活化退出

			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());//置需要回复标志
			}
		}
		break;
	case CALL_INTERFACE_PARA:
		{
			PrintMessage("召唤接口参数");
			Write_InterfacePara();
			CmdConSig_(CALL_INTERFACE_PARA_CON,RETURN_CODE_ACTIVE,terminalPtr,0);

		}
		break;
	case REBOOT_BF533:
		{
			PrintMessage("下行：BF533重启指令");
			bytesAssemble = AssembleRebootBf533(bufIndex,buf,terminalPtr);
			if (bytesAssemble > 0)
			{
				ResetTimerRebootBf533(terminalPtr,true);
			} 
		}
		break;
	case T_BOOT_BF533:
		{
			PrintMessage("下行：BF533 boot指令");
			bytesAssemble = AssembleTbootBf533(bufIndex,buf,terminalPtr);
			if (bytesAssemble >= 0)
			{
				if (bAllowTBoot_)
				{
					ResetTimerTBootBf533(terminalPtr,true);
				}	
			}
		}
		break;

	case ERASE_BF533:
		{
			PrintMessage("下行：BF533擦除指令");
			bytesAssemble = AssembleEraseBf533(bufIndex,buf,terminalPtr);
		}
		break;

	case DOWN_BF533_PRO:
		{
			PrintMessage("下行：BF533程序代码下装");
			bytesAssemble = AssembleDownBf533Pro(bufIndex,buf,terminalPtr,cmd);
		}
		break;

	default:
		{

		}
		break;
	}
	return bytesAssemble;
}
//组装报文尾
int CBF533::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	int length = bufIndex - bufBegin;
	int framelength ;//= length -4;
	size_t count = bufIndex;

	switch (cmd.getCmdType())
	{
	case T_BOOT_BF533:
		//buf[count++] = 0x7E;
		break;

	default:
		{
			framelength = length -4;

			buf[bufBegin + 2] = framelength & 0xff;
			buf[bufBegin + 3] = (framelength >> 8) & 0xff;

			buf[count++] = 0x03;
		}
		break;
	}

	return count - bufIndex;
}


int CBF533::AssembleCallAllData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)//组装总召报文
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_Call_AllData;
	return count - bufIndex;
}

int CBF533::AssembleCallEquPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)//召唤装置参数报文
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_Call_EquPara;
	return count - bufIndex;
}


int CBF533::AssembleSingleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code)//下发遥控执行指令
{
	size_t count = bufIndex;
	//buf[count++] = 0x08;
	//buf[count++] = 0x00;
	buf[count++] = C_Yk_Exe;
	buf[count++] = yk_code & 0xff;
	buf[count++] = yk_no & 0xff;
	buf[count++] = ~(yk_code & 0xff);
	buf[count++] = ~(yk_no & 0xff);
	buf[count++] = 0x00;
	buf[count++] = 0x00;
	buf[count++] = 0xFF;
	return count - bufIndex;
}

int CBF533::AssembleYkSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_YkSel_Ack;
	return count - bufIndex;
}

int CBF533::AssembleYkCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_YkCancel_Ack;
	return count - bufIndex;
}

int CBF533::AssembleDownEquPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)//下装装置参数
{
	size_t count = bufIndex;
	size_t i;

	//buf[count++] = 18;
	//buf[count++] = 0;
	buf[count++] = C_Down_EquPara;
	buf[count++] = (BF533Base.Type&0xFF);
	buf[count++] = ((BF533Base.Type >> 8)&0xFF);	
	//buf[count++] = (BF533Base.Version&0xFF);      //ZHANGZHIHUA 20110118
	//buf[count++] = ((BF533Base.Version >> 8)&0xFF);
	buf[count++] = (BF533Base.YKHZTime&0xFF);
	buf[count++] = ((BF533Base.YKHZTime >> 8)&0xFF);
	buf[count++] = (BF533Base.YKTZTime&0xFF);
	buf[count++] = ((BF533Base.YKTZTime >> 8)&0xFF);
	buf[count++] = (BF533Base.YXLvBoTime&0xFF);
	buf[count++] = ((BF533Base.YXLvBoTime >> 8)&0xFF);
	//buf[count++] = (BF533Base.SYXNUM&0xFF);
	//buf[count++] = ((BF533Base.SYXNUM >> 8)&0xFF);
	buf[count++] = BF533Base.LineNum;
	buf[count++] = BF533Base.HuoHua_AutoFlag;
	buf[count++] = (BF533Base.HuoHua_Day)/24;
	buf[count++] = (BF533Base.Pro_Rst_Time&0xFF);
	buf[count++] = ((BF533Base.Pro_Rst_Time >> 8)&0xFF);
	buf[count++] = BF533Base.Flag_Pro_Rst;
	buf[count++] = (BF533Base.BoardType&0xFF);
	buf[count++] = ((BF533Base.BoardType >> 8)&0xFF);
	//buf[count++] = (BF533Base.I_Rated&0xFF);
	//buf[count++] = ((BF533Base.I_Rated >> 8)&0xFF);
	//buf[count++] = 0xFF;
	buf[count++] = BF533Base.U_SwitchFlag;
	buf[count++] = BF533Base.PT_L_Arlam;//Modfiy by Zhangzhihua 20111220
	buf[count++] = BF533Base.ChannelSum;


	Check_Sum = 0;
	for(i = (bufIndex + BodyAddr);i < count;i ++)//计算校验和
	{
		Check_Sum += buf[i];
	}

	return count - bufIndex;
}

int CBF533::AssembleDownEquParaSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)//下装装置参数成功
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_DownSu_EquPara;
	return count - bufIndex;
}

int CBF533::AssembleDownDelayVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int line_no)//下装保护定值
{
	size_t count = bufIndex;
	size_t i;
	//buf[count++] = 80;
	//buf[count++] = 0x00;
	buf[count++] = C_Down_DelayVal;
	buf[count++]  = line_no;
	//	buf[count++]  = 0x03;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].Low_Lock&0xFF);
	buf[count++]  = ((ProVal[line_no].Low_Lock >> 8)&0xFF);//1
	//	buf[count++]  = 0x07;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].OverLoadValue&0xFF);
	buf[count++]  = ((ProVal[line_no].OverLoadValue >> 8)&0xFF);//2
	//	buf[count++]  = 0x08;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].OverLoadTime&0xFF);
	buf[count++]  = ((ProVal[line_no].OverLoadTime >> 8)&0xFF);//3
	//	buf[count++]  = 0x09;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].OverLoadTZWait&0xFF);
	buf[count++]  = ((ProVal[line_no].OverLoadTZWait >> 8)&0xFF);//4
	//	buf[count++]  = 0x10;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].Reclose_PROTime&0xFF);
	buf[count++]  = ((ProVal[line_no].Reclose_PROTime >> 8)&0xFF);//5
	//	buf[count++]  = 0x11;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].Accel_T&0xFF);
	buf[count++]  = ((ProVal[line_no].Accel_T >> 8)&0xFF);//6
	//	buf[count++]  = 0x12;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].I_PROValue&0xFF);
	buf[count++]  = ((ProVal[line_no].I_PROValue >> 8)&0xFF);//7
	//	buf[count++]  = 0x13;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].I_PROTime&0xFF);
	buf[count++]  = ((ProVal[line_no].I_PROTime >> 8)&0xFF);//8
	//	buf[count++]  = 0x14;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].I_PROTZWait&0xFF);
	buf[count++]  = ((ProVal[line_no].I_PROTZWait >> 8)&0xFF);//9
	//	buf[count++]  = 0x24;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].I0_PROValue&0xFF);
	buf[count++]  = ((ProVal[line_no].I0_PROValue >> 8)&0xFF);//10
	//	buf[count++]  = 0x25;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].I0_PROTime&0xFF);
	buf[count++]  = ((ProVal[line_no].I0_PROTime >> 8)&0xFF);//11
	//	buf[count++]  = 0x26;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].I0_PROTZWait&0xFF);
	buf[count++]  = ((ProVal[line_no].I0_PROTZWait >> 8)&0xFF);//12
	//	buf[count++]  = 0x27;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].OverU_P_Val&0xFF);
	buf[count++]  = ((ProVal[line_no].OverU_P_Val >> 8)&0xFF);//13
	//	buf[count++]  = 0x28;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].OverU_Ck_T&0xFF);
	buf[count++]  = ((ProVal[line_no].OverU_Ck_T >> 8)&0xFF);//14
	//buf[count++]  = 0x14;
	//buf[count++]  = 0x01;
	//buf[count++]  = (ProVal[line_no].FAOpenDalayTime&0xFF);
	//buf[count++]  = ((ProVal[line_no].FAOpenDalayTime >> 8)&0xFF);
	//	buf[count++]  = 0x15;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].FAReturnToZeroTime&0xFF);
	buf[count++]  = ((ProVal[line_no].FAReturnToZeroTime >> 8)&0xFF);//15
	//	buf[count++]  = 0x16;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].FAPowerSideAddr&0xFF);
	buf[count++]  = ((ProVal[line_no].FAPowerSideAddr >> 8)&0xFF);//16
	//	buf[count++]  = 0x17;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].FALoadSideAddr&0xFF);
	buf[count++]  = ((ProVal[line_no].FALoadSideAddr >> 8)&0xFF);//17

	//	buf[count++]  = 0x27;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].I0_II_PROValue&0xFF);
	buf[count++]  = ((ProVal[line_no].I0_II_PROValue >> 8)&0xFF);//18
	//	buf[count++]  = 0x28;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].I0_II_PROTime&0xFF);
	buf[count++]  = ((ProVal[line_no].I0_II_PROTime >> 8)&0xFF);//19
	//	buf[count++]  = 0x29;
	//	buf[count++]  = 0x01;
	buf[count++]  = (ProVal[line_no].I0_II_PROTZWait&0xFF);
	buf[count++]  = ((ProVal[line_no].I0_II_PROTZWait >> 8)&0xFF);//20
	buf[count++]  = (ProVal[line_no].OverU_PROTZWait&0xFF);
	buf[count++]  = ((ProVal[line_no].OverU_PROTZWait >> 8)&0xFF);//21
	buf[count++]  = (ProVal[line_no].II_PROValue&0xFF);
	buf[count++]  = ((ProVal[line_no].II_PROValue >> 8)&0xFF);//22
	buf[count++]  = (ProVal[line_no].II_PROTime&0xFF);
	buf[count++]  = ((ProVal[line_no].II_PROTime >> 8)&0xFF);//23buf[count++]  = ProVal[line_no].FAOverLoadLoseVolSum;
	buf[count++]  = (ProVal[line_no].II_PROTZWait&0xFF);
	buf[count++]  = ((ProVal[line_no].II_PROTZWait >> 8)&0xFF);//24
	buf[count++]  = (ProVal[line_no].FAOverLoadLoseVolSum&0xFF);
	buf[count++]  = ((ProVal[line_no].FAOverLoadLoseVolSum >> 8)&0xFF);//25

	Check_Sum = 0;

	for(i = (bufIndex + BodyAddr);i < count;i ++)//计算校验和
	{
		Check_Sum += buf[i];
	}

	return count - bufIndex;
}

int CBF533::AssembleDownDelayValSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)//下装保护定值成功
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_DownSu_DelayVal;
	return count - bufIndex;
}

int CBF533::AssembleDownDelayContrl(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int line_no) //下装保护控制字
{
	size_t count = bufIndex;
	size_t i;

	//buf[count++] = 37;
	//buf[count++] = 0x00;
	buf[count++] = C_Down_DelayContrl;
	buf[count++]  = line_no;
	//	buf[count++]  = 0x02;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].Low_PRO;//1
	//	buf[count++]  = 0x08;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].OverLoad;//2
	//	buf[count++]  = 0x09;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].OL_Alarm_F;//3
	//	buf[count++]  = 0x0F;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].Reclose_PRO;//4
	//	buf[count++]  = 0x0E;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].Accel_F;//5
	//	buf[count++]  = 0x10;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].I_PRO;//6
	//	buf[count++]  = 0x11;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].OF_Alarm_F;//7
	//	buf[count++]  = 0x4C;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].I0_PRO;//8
	//	buf[count++]  = 0x4D;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].I0_Alarm_F;//9
	//	buf[count++]  = 0x20;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].OverU_Alarm_F;//10
	//	buf[count++]  = 0x12;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].FAFlag;//11
	//buf[count++]  = 0x13;
	//buf[count++]  = 0x03;
	//buf[count++]  = ProVal[line_no].FAPowerSideAddr;
	//buf[count++]  = 0x14;
	//buf[count++]  = 0x03;
	//buf[count++]  = ProVal[line_no].FALoadSideAddr;
	//	buf[count++]  = 0x15;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].Local_FAFlag;//12
	//	buf[count++]  = 0x16;
	//	buf[count++]  = 0x03;
	//	buf[count++]  = ProVal[line_no].FAOverLoadLoseVolSum;//13

	//	buf[count++]  = 0x4E;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].I0_II_PRO;//4
	//	buf[count++]  = 0x4F;
	//	buf[count++]  = 0x03;
	buf[count++]  = ProVal[line_no].I0_II_Alarm_F;//15
	buf[count++]  = ProVal[line_no].OverU_PRO_F;//16
	buf[count++]  = ProVal[line_no].II_PRO;//17
	buf[count++]  = ProVal[line_no].II_Alarm_F;//18


	Check_Sum = 0;

	for(i = (bufIndex + BodyAddr);i < count;i ++)//计算校验和
	{
		Check_Sum += buf[i];
	}

	return count - bufIndex;
}

int CBF533::AssembleDownDelayContrlSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr) //下装保护控制字成功
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_DownSu_DelayContrl;
	return count - bufIndex;
}

int CBF533::AssembleSignalRest(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int line_no) //信号复归
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_Signal_Rest;
	buf[count++] = line_no;
	return count - bufIndex;
}

int CBF533::AssembleSynTime(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,boost::posix_time::ptime time) //广播对时
{
	size_t count = bufIndex;
	//buf[count++] = 0x05;
	//buf[count++] = 0x00;
	buf[count++] = C_Syn_Time;

	boost::posix_time::time_duration td = time.time_of_day();
	unsigned long milliseconds = (td.total_milliseconds())*SYNTIMEMULTIPLE;
	buf[count++] = (milliseconds&0xFF);
	buf[count++] = ((milliseconds>> 8)&0xFF);
	buf[count++] = ((milliseconds>>16)&0xFF);
	buf[count++] = ((milliseconds>>24)&0xFF);

	return count - bufIndex;
}

int CBF533::AssembleDownChannelPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr) //下装通道参数
{
	size_t count = bufIndex;
	size_t i;

	//buf[count++] = (BF533Base.ChannelSum + 2)&0xFF;;
	//buf[count++] = ((BF533Base.ChannelSum + 2) >> 8)&0xFF;
	buf[count++] = C_Down_ChannelPara;
	buf[count++] = BF533Base.ChannelSum;

	for(i = 0;i < BF533Base.ChannelSum;i ++)
	{
		buf[count + i] = YcChannel[i].Type;
	}

	count += i;

	Check_Sum = 0;

	for(i = (bufIndex + BodyAddr);i < count;i ++)//计算校验和
	{
		Check_Sum += buf[i];
	}

	return count - bufIndex;

}

int CBF533::AssembleDownChannelParaSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr) //下装通道参数成功
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_DownSu_ChannelPara;
	return count - bufIndex;
}

int CBF533::AssembleDownChannelComrel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr) //下装通道组合关系
{
	size_t count = bufIndex;
	size_t i;
	//std::cout<<"线路数量BF533Base.LineNum是："<<BF533Base.LineNum<<"/"<<((BF533Base.LineNum*12 + 1)&0xFF)<<"/"<<(((BF533Base.LineNum*12 + 1) >> 8)&0xFF)<<std::endl;
	//buf[count++] = ((BF533Base.LineNum*12 + 1)&0xFF);
	//buf[count++] = (((BF533Base.LineNum*12 + 1) >> 8)&0xFF);
	buf[count++] = C_Down_ChannelComrel;
	//    buf[count++] = BF533Base.LineNum;
	for(i=0;i < BF533Base.LineNum;i++)
	{
		buf[count++]  = i;
		buf[count++]  = LinePara[i].Vol_Chan[0];
		buf[count++]  = LinePara[i].Vol_Chan[1];
		buf[count++]  = LinePara[i].Vol_Chan[2];
		buf[count++]  = LinePara[i].Cur_Chan[0];
		buf[count++]  = LinePara[i].Cur_Chan[1];
		buf[count++]  = LinePara[i].Cur_Chan[2];
		buf[count++]  = LinePara[i].BCur_Chan[0];//保护电流通道
		buf[count++]  = LinePara[i].BCur_Chan[1];
		buf[count++]  = LinePara[i].BCur_Chan[2];
		buf[count++]  = LinePara[i].U0_Chan;
		buf[count++]  = LinePara[i].I0_Chan;
		buf[count++]  = (LinePara[i].YxStart) & 0xff;
		buf[count++]  = ((LinePara[i].YxStart)/256) & 0xff;
		buf[count++]  = LinePara[i].IO_Flage;//Modfiy by Zhangzhihua 20111220
	}

	Check_Sum = 0;

	for(i = (bufIndex + BodyAddr);i < count;i ++)//计算校验和
	{
		Check_Sum += buf[i];
	}

	return count - bufIndex;
}

int CBF533::AssembleDownChannelComrelSucess(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr) //下装通道组合关系成功
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_DownSu_ChannelComrel;
	return count - bufIndex;
}

int CBF533::AssembleCallLineData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr) //召唤线路数据
{
	return 0;
}

int CBF533::AssembleDownLineVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd) //下装线路定值
{
	size_t count = bufIndex;
	size_t i;


	DataBase::stLine_Val Val;
	Val = boost::any_cast<DataBase::stLine_Val>(cmd.getVal());

	size_t line_no = Val.Line_no;

	//buf[count++] = 44;
	//buf[count++] = 0x00;
	buf[count++] = C_Down_LineVal;

	buf[count++] = line_no;
	buf[count++] = Val.Flag_Ua;
	buf[count++] = (Val.D_Value_Ua &0xFF);
	buf[count++] = ((Val.D_Value_Ua >> 8)&0xFF);
	buf[count++] = Val.Flag_Ub;
	buf[count++] = (Val.D_Value_Ub &0xFF);
	buf[count++] = ((Val.D_Value_Ub >> 8)&0xFF);
	buf[count++] = Val.Flag_Uc;
	buf[count++] = (Val.D_Value_Uc &0xFF);
	buf[count++] = ((Val.D_Value_Uc >> 8)&0xFF);
	buf[count++] = Val.Flag_CIa;
	buf[count++] = (Val.D_Value_CIa &0xFF);
	buf[count++] = ((Val.D_Value_CIa >> 8)&0xFF);
	buf[count++] = Val.Flag_CIb;
	buf[count++] = (Val.D_Value_CIb &0xFF);
	buf[count++] = ((Val.D_Value_CIb >> 8)&0xFF);
	buf[count++] = Val.Flag_CIc;
	buf[count++] = (Val.D_Value_CIc &0xFF);
	buf[count++] = ((Val.D_Value_CIc >> 8)&0xFF);
	buf[count++] = Val.Flag_BIa;
	buf[count++] = (Val.D_Value_BIa &0xFF);
	buf[count++] = ((Val.D_Value_BIa >> 8)&0xFF);
	buf[count++] = Val.Flag_BIb;
	buf[count++] = (Val.D_Value_BIb &0xFF);
	buf[count++] = ((Val.D_Value_BIb >> 8)&0xFF);
	buf[count++] = Val.Flag_BIc;
	buf[count++] = (Val.D_Value_BIc &0xFF);
	buf[count++] = ((Val.D_Value_BIc >> 8)&0xFF);
	buf[count++] = Val.Flag_U0;
	buf[count++] = (Val.D_Value_U0 &0xFF);
	buf[count++] = ((Val.D_Value_U0 >> 8)&0xFF);
	buf[count++] = Val.Flag_I0;
	buf[count++] = (Val.D_Value_I0 &0xFF);
	buf[count++] = ((Val.D_Value_I0 >> 8)&0xFF);
	buf[count++] = Val.Flag_Angle_UaIa;
	buf[count++] = (Val.Angle_UaIa &0xFF);
	buf[count++] = ((Val.Angle_UaIa >> 8)&0xFF);
	buf[count++] = Val.Flag_Angle_UbIb;
	buf[count++] = (Val.Angle_UbIb &0xFF);
	buf[count++] = ((Val.Angle_UbIb >> 8)&0xFF);
	buf[count++] = Val.Flag_Angle_UcIc;
	buf[count++] = (Val.Angle_UcIc &0xFF);
	buf[count++] = ((Val.Angle_UcIc >> 8)&0xFF);

	Check_Sum = 0;

	for(i = (bufIndex + BodyAddr);i < count;i ++)//计算校验和
	{
		Check_Sum += buf[i];
	}

	return count - bufIndex;
}

int CBF533::AssembleHarmonicAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	size_t count = bufIndex;
	DataBase::stHarmonic Val;
	Val = boost::any_cast<DataBase::stHarmonic>(cmd.getVal());

	//buf[count++] = 0x03;
	//buf[count++] = 0x00;
	buf[count++] = C_Call_Harmonic;
	buf[count++] = Val.Line_no;
	buf[count++] = Val.harmonic_no;

	return count - bufIndex;

}

int CBF533::AssembleCallValCoef(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	size_t count = bufIndex;  
	DataBase::stLine_ValCoef Val;

	Val = boost::any_cast<DataBase::stLine_ValCoef>(cmd.getVal());

	//buf[count++] = 0x02;
	//buf[count++] = 0x00;
	buf[count++] = C_Call_LineData;
	buf[count++] = Val.Line_no;

	return count - bufIndex;
}

int CBF533::AssembleLineValVer(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr) //线路定值校验
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_Line_ValVer;
	return count - bufIndex;
}

int CBF533::AssembleDownLoadLineHVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	size_t count = bufIndex; 
	size_t i;
	DataBase::stLine_ValCoef HVal;

	HVal = boost::any_cast<DataBase::stLine_ValCoef>(cmd.getVal());

	//buf[count++] = 0x44;
	//buf[count++] = 0x00;
	buf[count++] = C_Down_LineHVal;
	buf[count++] = HVal.Line_no;
	buf[count++] = HVal.Flag_Ua;
	buf[count++] = (HVal.H_Value_Ua & 0xff);
	buf[count++] = ((HVal.H_Value_Ua >> 8)& 0xff);
	buf[count++] = HVal.Flag_Ub;
	buf[count++] = (HVal.H_Value_Ub & 0xff);
	buf[count++] = ((HVal.H_Value_Ub >> 8)& 0xff);
	buf[count++] = HVal.Flag_Uc;
	buf[count++] = (HVal.H_Value_Uc & 0xff);
	buf[count++] = ((HVal.H_Value_Uc >> 8)& 0xff);
	buf[count++] = HVal.Flag_CIa;
	buf[count++] = (HVal.H_Value_CIa & 0xff);
	buf[count++] = ((HVal.H_Value_CIa >> 8)& 0xff);
	buf[count++] = HVal.Flag_CIb;
	buf[count++] = (HVal.H_Value_CIb & 0xff);
	buf[count++] = ((HVal.H_Value_CIb >> 8)& 0xff);
	buf[count++] = HVal.Flag_CIc;
	buf[count++] = (HVal.H_Value_CIc & 0xff);
	buf[count++] = ((HVal.H_Value_CIc >> 8)& 0xff);
	buf[count++] = HVal.Flag_BIa;
	buf[count++] = (HVal.H_Value_BIa & 0xff);
	buf[count++] = ((HVal.H_Value_BIa >> 8)& 0xff);
	buf[count++] = HVal.Flag_BIb;
	buf[count++] = (HVal.H_Value_BIb & 0xff);
	buf[count++] = ((HVal.H_Value_BIb >> 8)& 0xff);
	buf[count++] = HVal.Flag_BIc;
	buf[count++] = (HVal.H_Value_BIc & 0xff);
	buf[count++] = ((HVal.H_Value_BIc >> 8)& 0xff);
	buf[count++] = HVal.Flag_U0;
	buf[count++] = (HVal.H_Value_U0 & 0xff);
	buf[count++] = ((HVal.H_Value_U0 >> 8)& 0xff);
	buf[count++] = HVal.Flag_I0;
	buf[count++] = (HVal.H_Value_I0 & 0xff);
	buf[count++] = ((HVal.H_Value_I0 >> 8)& 0xff);
	buf[count++] = HVal.Flag_Angle_UaIa;
	buf[count++] = (HVal.Angle_UaIa & 0xff);
	buf[count++] = ((HVal.Angle_UaIa >> 8)& 0xff);
	buf[count++] = HVal.Flag_Angle_UbIb;
	buf[count++] = (HVal.Angle_UbIb & 0xff);
	buf[count++] = ((HVal.Angle_UbIb >> 8)& 0xff);
	buf[count++] = HVal.Flag_Angle_UcIc;
	buf[count++] = (HVal.Angle_UcIc & 0xff);
	buf[count++] = ((HVal.Angle_UcIc >> 8)& 0xff);

	Check_Sum = 0;

	for(i = (bufIndex + BodyAddr);i < count;i ++)//计算校验和
	{
		Check_Sum += buf[i];
	}

	return count - bufIndex;
}

int CBF533::AssembleDspVersionInq(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr) //DSP版本查询
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_Inq_DspVersion;
	return count - bufIndex;
}


int CBF533::AssembleBoardInquiry(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_BoardInquiry;
	return count - bufIndex;
}

int CBF533::AssembleCallProVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_Call_ProVal;
	return count - bufIndex;
}

int CBF533::AssembleCallChType(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_Call_ChType;
	return count - bufIndex;
}

int CBF533::AssembleCallLinePara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_Call_LinePara;
	return count - bufIndex;
}

int CBF533::AssembleCallYMDataAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_Call_YmData;
	return count - bufIndex;
}

int CBF533::AssembleBatteryActiveAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_BatteryActive;
	return count - bufIndex;
}

int CBF533::AssembleBatteryActiveOverAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_BatteryActiveOver;
	return count - bufIndex;
}

int CBF533::AssembleDownLineBVal(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	size_t count = bufIndex;
	size_t i;


	DataBase::stLine_BCurVal Val;
	Val = boost::any_cast<DataBase::stLine_BCurVal>(cmd.getVal());

	size_t line_no = Val.Line_No;

	//buf[count++] = 0x08;
	//buf[count++] = 0x00;
	buf[count++] = C_Down_LineBVal;
	buf[count++] = line_no;
	buf[count++] = (Val.D_Val_BI1 & 0xff);
	buf[count++] = ((Val.D_Val_BI1 >> 8)& 0xff);
	buf[count++] = (Val.D_Val_BI2 & 0xff);
	buf[count++] = ((Val.D_Val_BI2 >> 8)& 0xff);
	buf[count++] = (Val.D_Val_BI3 & 0xff);
	buf[count++] = ((Val.D_Val_BI3 >> 8)& 0xff);

	Check_Sum = 0;

	//std::cout<<"BF533收到的保护电流定值为："<<"Val.D_Val_BI1:"<<Val.D_Val_BI1<<" Val.D_Val_BI2:"<<Val.D_Val_BI2<<" Val.D_Val_BI3"<<Val.D_Val_BI3<<std::endl;

	for(i = (bufIndex + BodyAddr);i < count;i ++)//计算校验和
	{
		Check_Sum += buf[i];
	}

	return count - bufIndex;
}

int CBF533::AssembleDcValBVer(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	DataBase::stDCVerVal Val;
	Val = boost::any_cast<DataBase::stDCVerVal>(cmd.getVal());
	DcChannelNo_ = Val.ChannelNo;
	size_t count = bufIndex;
	//buf[count++] = 0x04;
	//buf[count++] = 0x00;
	buf[count++] = C_DcValVer;
	buf[count++] = Val.ChannelNo;
	buf[count++] = (Val.V_Val_DC & 0xff);
	buf[count++] = ((Val.V_Val_DC >> 8)& 0xff);
	return count - bufIndex;
}

int CBF533::AssembleCallPm(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	DataBase::stPMPara Val;
	Val = boost::any_cast<DataBase::stPMPara>(cmd.getVal());
	int LineNo = Val.LineNo;
	size_t count = bufIndex;
	//buf[count++] = 0x02;
	//buf[count++] = 0x00;
	buf[count++] = C_LineCallPm;
	buf[count++] = Val.LineNo;

	return count - bufIndex;
}

int CBF533::AssembleRebootBf533(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	buf[count++] = C_RebootBf533;
	return count - bufIndex;
}

int CBF533::AssembleTbootBf533(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	buf[count++] = 0x7E;
	return count - bufIndex;
}

int CBF533::AssembleEraseBf533(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	buf[count++] = C_EraseBf533;
	return count - bufIndex;
}

int CBF533::AssembleDownBf533Pro(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	size_t count = bufIndex;
	size_t len;

	if(FileHandleGetRemainLength() > FileLen)
	{
		len = FileLen;
		//SendFileLength_ -= FileLen;
	}
	else
	{
		len = FileHandleGetRemainLength();
	}

	int Val = boost::any_cast<int>(cmd.getVal());

	This_N0 = Val;
	buf[count++] = C_DownBf533Pro;
	count += ValToBuf(&buf[count],This_N0,2);
	count += ValToBuf(&buf[count],Total_NUM,2);
	count += FileHandleGetFile(&buf[count],len);

	for(int i = (bufIndex + BodyAddr);i < count;i ++)//计算校验和
	{
		Check_BF533Sum += buf[i];
	}

	buf[count++] = Check_BF533Sum;

	std::ostringstream ostr;
	ostr<<"本次发送第 "<<This_N0<<" 帧，共 "<<Total_NUM<<" 帧！"<<std::endl;
	AddStatusLogWithSynT(ostr.str());

	return count - bufIndex;
}

int CBF533::AssembleLineValBVer(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	//buf[count++] = 0x01;
	//buf[count++] = 0x00;
	buf[count++] = C_LineValBVer;
	return count - bufIndex;
}

int CBF533::getAddrByRecvFrame(unsigned char * buf)
{
	unsigned char funcType = buf[0];
	int addr = -1;

	switch (funcType)
	{
	case 0xEB:
		addr = 1;
		break;

	default:
		addr = -1;
		break;
	}

	return addr;
}

int CBF533::ParseLongFrame(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识

	switch (FrameType)
	{
	case 0x06:
		PrintMessage("上行：ACK报文");
		if(getLastSendCmd() == SYN_TIME_ACT)
		{
			if (getLastRecvCmd() == ALL_QYX_CON)
			{
				PrintMessage("上电总召结束！");
				terminalPtr->setCommActive(true);
				ResetTimerSynTime(terminalPtr,true,SYN_TIME);//启动定时发送对时报文定时器
				if(BF533Base.AotuRstYxFlag)
				{
					ResetTimerBattryActive(terminalPtr,true,0);//启动定时电池活化定时器
				}

				if (AutoDownloadCfgFlag_)
				{
					AutoDownloadCfg(terminalPtr);
				}
			}
			else
			{
				ResetTimerSynTime(terminalPtr,true,SYN_TIME);//启动定时发送对时报文定时器
			}
		}
		setLastRecvCmd(ACK_CON);//设置收到指令为ACK
		break;
	case R_AllYc:
		{
			PrintMessage("上行：全遥测报文");
			setLastRecvCmd(ALL_QYC_CON);//设置收到指令为全遥测
			ParseAllYCData(buf,terminalPtr);
		}	  
		break;
	case R_Call_EquPara:
		{
			PrintMessage("上行：召唤装置参数应答报文");
			if ((getLastSendCmd() == CALL_EQU_PARA_ACT)/*&&(getLastRecvCmd() == ALL_QYX_CON)*/)//如果上一次为召唤装置参数，则对收到参数进行文件存储并通知对上规约
			{
				ParseCallEquCon(buf,terminalPtr,exceptedBytes);
				//AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,terminalPtr);
			}
			else if (getLastSendCmd() == DOWNLOAD_EQU_PARA)//如果上一次为下装装置参数，本次为反校
			{
				ParseDownloadEquCeck(buf,terminalPtr,exceptedBytes);//解析反校报文
			}
			setLastRecvCmd(CALL_EQU_PARA_CON);//设置收到指令为召唤装置参数回复
		}	 
		break;
	case R_SOE:
		{
			PrintMessage("上行：事件顺序记录SOE");
			ParseSingleSOE(buf,terminalPtr);
		}
		break;
	case R_Yk_Exe:
		{
			PrintMessage("上行：遥控执行确认报文");
			ParseSingleYKExeCon(buf,terminalPtr);//解析遥控执行确认报文
		}
		break;
	case R_Down_EquPara:
		{
			PrintMessage("上行：装置参数下装成功");
			char Val = 0x0A;//传递给对上规约，传送原因为0x0A
			CmdConSig_(DOWNLOAD_PARA_CON,RETURN_CODE_ACTIVE,terminalPtr,Val);
		}

		break;
	case R_Down_DVal_Check:
		{
			if(getLastSendCmd() == DOWNLOAD_DELAY_VAL)
			{
				PrintMessage("上行：保护定值下装反校");
				ParseDownloadDelayValCeck(buf,terminalPtr,exceptedBytes);//解析反校报文
			}
			else if(getLastSendCmd() == CALL_PROVAL_ACK)
			{
				PrintMessage("上行：召唤保护定值回复");
				ParseCallProVal(buf,terminalPtr,exceptedBytes);//解析反校报文
			}
		}

		break;
	case R_Down_DVal:
		{
			PrintMessage("上行：保护定值下装成功");
			ParseDownloadDownDelayValSucess(buf,terminalPtr,exceptedBytes);//处理下装成功报文
		}
		break;
	case R_Down_DContrl_Check:
		{
			if(getLastSendCmd() == DOWNLOAD_DELAY_CONRL)
			{
				PrintMessage("上行：保护控制字下装反校");
				ParseDownloadDelayContrlCheck(buf,terminalPtr,exceptedBytes);
			}
			else if(getLastSendCmd() == CALL_PROVAL_ACK)
			{
				PrintMessage("上行：召唤保护控制字回复");
				ParseCallProContrl(buf,terminalPtr,exceptedBytes);
			}
		}
		break;
	case R_Down_DContrl:
		{
			PrintMessage("上行：保护控制字下装成功");
			ParseDownloadDownDelayContrlSucess(buf,terminalPtr,exceptedBytes);
		}

		break;
	case R_Signal_Rest:
		{
			PrintMessage("上行：远方复归确认");
			ParseSingalReset(buf,terminalPtr,exceptedBytes);
		}

		break;
	case R_Down_ChPara_Check:
		{
			if(getLastSendCmd() == DOWNLOAD_CHANNEL_PARA)
			{
				PrintMessage("上行：通道参数下装反校");
				ParseDownloadChannelParaCheck(buf,terminalPtr,exceptedBytes);
			}
			else if(getLastSendCmd() == CALL_CHTYPE_ACK)
			{
				PrintMessage("上行：召唤通道参数回复");
				ParseCallChTypeAck(buf,terminalPtr,exceptedBytes);
			}
		}
		break;
	case R_Down_ChannelPara:
		{
			PrintMessage("上行：下装通道参数成功");
			char Val = 0x0A;//传递给对上规约，传送原因为0x0A
			CmdConSig_(DOWNLOAD_PARA_CON,RETURN_CODE_ACTIVE,terminalPtr,Val);
		}
		break;
	case R_Down_ChComrel_Check:
		{
			if(getLastSendCmd() == DOWNLOAD_CHANNEL_COMREL)
			{
				PrintMessage("上行：通道组合关系下装反校");
				ParseDownloadChannelComrelCheck(buf,terminalPtr,exceptedBytes);
			}
			else if(getLastSendCmd() == CALL_LINEPARA_ACK)
			{
				PrintMessage("上行：召唤通道组合关系回复");
				ParseCallLinePara(buf,terminalPtr,exceptedBytes);
			}
		}
		break;
	case R_Down_ChannelComrel:
		{
			PrintMessage("上行：通道组合关系下装成功");
			char Val = 0x0A;//传递给对上规约，传送原因为0x0A
			CmdConSig_(DOWNLOAD_PARA_CON,RETURN_CODE_ACTIVE,terminalPtr,Val);
		}

		break;
	case R_Line_SynYc:
		{
			//	  PrintMessage("上行：单线路全遥测定时上送");
			ParseLineCycTime(buf,terminalPtr);
		}
		break;
	case R_Timer_IT://直流量及温度定时上送
		{
			ParseCycDcIT(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_DcVal_Ver:
		{
			PrintMessage("上行：直流量校验成功");
			ParseDcValVerSucess(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_AllYx:
		{
			PrintMessage("上行：全遥信报文");

			ParseAllYxCon(buf,terminalPtr);

			if ((getLastSendCmd() == CALL_ALL_DATA_ACT)/*&&(getLastRecvCmd() == ALL_QYC_CON)*/)//上一次为总召且前一次收到全遥测，下一次开始召唤装置参数
			{
				//			  PrintMessage("开始清零定时器1... .");
				ResetTimerAllData(terminalPtr,false,0);
				//			  ResetTimerCallAllData(terminalPtr,false,10);//上电总召结束
				AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,terminalPtr);//总召结束后对时
				//			  AddSendCmdVal(CALL_EQU_PARA_ACT,CALL_EQU_PARA_ACT_PRIORITY,terminalPtr);//总召结束后召唤装置参数
			}
			setLastRecvCmd(ALL_QYX_CON);   //设置本次收到指令为全遥信
		}
		break;
	case R_Down_LineVal_Check:
		{
			PrintMessage("上行：线路定值下装反校");
			ParseDownloadLineValCheck(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Qyc_ValVer:
		{
			PrintMessage("上行：定值校验后全遥测");
			ParseLineValVerQyc(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Line_ValVer:
		{
			PrintMessage("上行：线路定值校验成功");
		}
		break;
	case R_Down_LineHVal_Check:
		{
			if (getLastSendCmd() == CALL_VALCOEF_ACK)
			{
				PrintMessage("上行：召唤定值系数回复");
				ParseCallValCoefCon(buf,terminalPtr,exceptedBytes);
			}
			else if (getLastSendCmd() == DOWNLOAD_LINE_COEF)
			{
				PrintMessage("上行：下装通道系数反校");
				ParseDownLoadLineCoef(buf,terminalPtr,exceptedBytes);
			}
		}
		break;
	case R_Fault_Event:
		{
			PrintMessage("上行：故障事件报文");
			ParseFaultEvent(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Call_Harmonic:
		{
			PrintMessage("上行：召唤谐波分量回复");
			ParseHarmonicCon(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_BoardInquiry:
		{
			PrintMessage("上行：板件查询回复");
			ParseBoardInqCon(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Call_EquPara_Sucess:
		{
			PrintMessage("上行：召唤装置参数成功");
			ParseCallEquSucess(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Call_ProVal:
		{
			PrintMessage("上行：召唤保护定值及保护控制字确认报文");
			ParseCallProValCon(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Call_ProVal_Sucess:
		{
			PrintMessage("上行：召唤保护定值及保护控制字成功");
			ParseCallProValSucess(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Call_ChType:
		{
			PrintMessage("上行：召唤通道类型成功");
			ParseCallChTypeSucess(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Call_DSPVer:
		{
			PrintMessage("上行：DSP版本查询回复");
			ParseCallDspVersion(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Call_LinePara:
		{
			PrintMessage("上行：召唤通道组合关系成功");
			ParseCallLineParaSucess(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Call_YmData:
		{
			PrintMessage("上行：召唤电度值回复");
			ParseCallYMDataCon(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_FA_Event:
		{
			PrintMessage("上行：主动上送故障电流信息(FA)");
		}
		break;
	case R_BatteryActive:
		{
			PrintMessage("上行：电池活化确认");
			CmdConSig_(BATTERY_ACTIVE_CON,RETURN_CODE_ACTIVE,terminalPtr,0);
		}
		break;
	case R_BatteryActiveOver:
		{
			PrintMessage("上行：电池活化退出确认");
			CmdConSig_(BATTERY_ACTIVE_OVER_CON,RETURN_CODE_ACTIVE,terminalPtr,0);
		}
		break;
	case R_YkSel_Ack:
		{
			PrintMessage("上行：遥控选择点灯确认");
		}
		break;
	case R_YkCancel_Ack:
		{
			PrintMessage("上行：遥控选择灭灯确认");
		}
		break;
	case R_Down_LineBVal_Check:
		{
			PrintMessage("上行：保护电流校验定值下装反校");
			ParseDownloadLineBValCheck(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_LineValBVer:
		{
			PrintMessage("上行：保护电流校验校验后保护电流值");
			ParseLineBValVer(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_Call_Pm:
		{
			PrintMessage("上行：召唤绝对相角回复");
			ParseCallPm(buf,terminalPtr,exceptedBytes);
		}
		break;
	case R_LineValBVer_Sucess:
		{
			PrintMessage("上行：保护电流校验校验成功");
			//CmdConSig_(LINE_BVAL_VER_SUCESS,RETURN_CODE_ACTIVE,terminalPtr,0);
		}
		break;

	case 0x70:

		break;

	case R_RebootBf533:
		{
			PrintMessage("上行：BF533重新启动报文");
			ParseRebootBf533(buf,terminalPtr,exceptedBytes);
		}
		break;

	case R_EraseBf533:
		{
			PrintMessage("上行：Flash擦除完成报文");
			ParseEraseBf533(buf,terminalPtr,exceptedBytes);
		}
		break;

	case R_DownBf533Pro:
		{
			PrintMessage("上行：发送文件反校报文");
			ParseDownBf533Pro(buf,terminalPtr,exceptedBytes);
		}
		break;

	default:
		{
			std::ostringstream ostr;
			ostr<<"接收报文错误，未定义的报文类型 FRAME_TYPE ="<<FrameType<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;
	}

	return 0;
}

int CBF533::ParseCallEquSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	Write_BasePara();
	Write_DevSanyaoPara();
	Read_DevSanyaoPara();
	CmdConSig_(CALL_EQU_PARA_CON,RETURN_CODE_ACTIVE,terminalPtr,0);//存成文件后通知对上规约进行文件发送
	return 0;
}

int CBF533::ParseCallProValSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	Write_ProtectValPara();
	CmdConSig_(CALL_PROVAL_CON,RETURN_CODE_ACTIVE,terminalPtr,0);//存成文件后通知对上规约进行文件发送
	//std::cout<<"BF533开始对上添加召唤保护定值成功指令"<<std::endl;
	return 0;
}


int CBF533::ParseCallLineParaSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	Write_LinePara();
	CmdConSig_(CALL_LINEPARA_CON,RETURN_CODE_ACTIVE,terminalPtr,0);//存成文件后通知对上规约进行文件发送
	return 0;
}

int CBF533::ParseCallYMDataCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 5; //线路数量所在位置
	int LineSum = BufToVal(&buf[Location ++],1);
	for (int i = 0;i < LineSum;i ++)
	{
		double Data = BufToVal(&buf[Location],4);
		double Data1 =  (Data /(3600*1000*100));
		YMData[i].YMDataVal = Data1;
		//      YMData[i].YMDataVal = (Data /(3600*1000*100));
		//		YMData[i].YMDataVal = (BufToVal(&buf[Location],4)/(3600*1000*100));
		Location += 4;
	}

	SaveYMData(LineSum);
	return 0;
}


int CBF533::ParseCallLinePara(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int LineNum = (BufToVal(&buf[2],2) -1 )/15/*12*/;
	int Location = 6;
	BF533HBase.LineNum  = LineNum;
	for(int i = 0;i < LineNum; i ++)
	{
		HLinePara[i].Vol_Chan[0] =  BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].Vol_Chan[1] =  BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].Vol_Chan[2] =  BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].Cur_Chan[0] =  BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].Cur_Chan[1] =  BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].Cur_Chan[2] =  BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].BCur_Chan[0] = BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].BCur_Chan[1] = BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].BCur_Chan[2] = BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].U0_Chan     =  BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].I0_Chan     =  BufToVal(&buf[Location],1);
		Location += 1;
		HLinePara[i].YxStart     =  BufToVal(&buf[Location],2);
		//Location +=3; //去掉进出线判断标识时使用
		Location +=2;//Modfiy by Zhangzhihua 20111220
		HLinePara[i].IO_Flage     =  BufToVal(&buf[Location],1);   //Modfiy by Zhangzhihua 20111220
		Location +=2;//Modfiy by Zhangzhihua 20111220
	}
	return 0;
}

int CBF533::ParseCallProValCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	BF533HBase.LineNum = BufToVal(&buf[5],1);
	//std::cout<<"ParseCallProValCon收到线路号为："<<BF533HBase.LineNum<<std::endl;
	return 0;
}

int CBF533::ParseCallChTypeAck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 6;
	BF533HBase.ChannelSum = BufToVal(&buf[5],1);
	for(size_t i = 0;i < BF533HBase.ChannelSum; i ++)
	{
		HYcChannel[i].Type = BufToVal(&buf[Location ++],1);
	}
	return 0;
}

int CBF533::ParseCallChTypeSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	Write_ChanneltypePara();
	CmdConSig_(CALL_CHTYPE_CON,RETURN_CODE_ACTIVE,terminalPtr,0);//存成文件后通知对上规约进行文件发送
	return 0;
}

int CBF533::ParseCallProContrl(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int line_no = BufToVal(&buf[5],1);
	int Location = 6;
	unsigned short length = 1;
	//std::cout<<"收到保护控制字，线路号为："<<line_no<<std::endl; 
	HProVal[line_no].Low_PRO = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].OverLoad = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].OL_Alarm_F = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].Reclose_PRO = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].Accel_F = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].I_PRO = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].OF_Alarm_F = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].I0_PRO = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].I0_Alarm_F = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].OverU_Alarm_F = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].FAFlag = BufToVal(&buf[Location],1);
	Location += length;
	//HProVal[line_no].FAPowerSideAddr = BufToVal(&buf[Location],1);
	//Location += length;
	//HProVal[line_no].FALoadSideAddr = BufToVal(&buf[Location],1);
	//Location += length;
	HProVal[line_no].Local_FAFlag = BufToVal(&buf[Location],1);
	Location += length;
	//HProVal[line_no].FAOverLoadLoseVolSum = BufToVal(&buf[Location],1);
	//Location += length;
	HProVal[line_no].I0_II_PRO = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].I0_II_Alarm_F = BufToVal(&buf[Location],1);

	Location += length;
	HProVal[line_no].OverU_PRO_F = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].II_PRO = BufToVal(&buf[Location],1);
	Location += length;
	HProVal[line_no].II_Alarm_F = BufToVal(&buf[Location],1);

	return 0;
}

int CBF533::ParseCallProVal(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int line_no = BufToVal(&buf[5],1);
	int Location = 6;
	unsigned short length = 2;
	//std::cout<<"收到保护定值，线路号为："<<line_no<<std::endl; 
	HProVal[line_no].Low_Lock = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].OverLoadValue = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].OverLoadTime = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].OverLoadTZWait = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].Reclose_PROTime = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].Accel_T = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].I_PROValue = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].I_PROTime = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].I_PROTZWait = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].I0_PROValue = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].I0_PROTime = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].I0_PROTZWait = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].OverU_P_Val = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].OverU_Ck_T = BufToVal(&buf[Location],2);
	Location += length;
	//HProVal[line_no].FAOpenDalayTime = BufToVal(&buf[Location],2);
	//Location += length;
	HProVal[line_no].FAReturnToZeroTime = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].FAPowerSideAddr = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].FALoadSideAddr = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].I0_II_PROValue = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].I0_II_PROTime = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].I0_II_PROTZWait = BufToVal(&buf[Location],2);
	Location += length;

	HProVal[line_no].OverU_PROTZWait = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].II_PROValue = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].II_PROTime = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].II_PROTZWait = BufToVal(&buf[Location],2);
	Location += length;
	HProVal[line_no].FAOverLoadLoseVolSum = BufToVal(&buf[Location],2);
	Location += length;

	return 0;
}

int CBF533::ParseCallEquCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 5;

	BF533HBase.Type = BufToVal(&buf[Location],2);
	Location += 2;
	//BF533HBase.Version = BufToVal(&buf[Location],2);
	//Location += 2;
	BF533HBase.YKHZTime = BufToVal(&buf[Location],2);
	Location += 2;
	BF533HBase.YKTZTime = BufToVal(&buf[Location],2);
	Location += 2;
	BF533HBase.YXLvBoTime = BufToVal(&buf[Location],2);
	Location += 2;
	BF533HBase.LineNum = BufToVal(&buf[Location],1);
	Location += 1;
	BF533HBase.HuoHua_AutoFlag = BufToVal(&buf[Location],1);
	Location += 1;
	BF533HBase.HuoHua_Day = BufToVal(&buf[Location],1);
	Location += 1;
	BF533HBase.Pro_Rst_Time = BufToVal(&buf[Location],2);
	Location += 2;
	BF533HBase.Flag_Pro_Rst = BufToVal(&buf[Location],1);
	Location += 1;
	BF533HBase.BoardType = BufToVal(&buf[Location],2);
	//BF533Base.I_Rated = BufToVal(&buf[Location],2);
	Location += 2;
	BF533HBase.U_SwitchFlag = BufToVal(&buf[Location],1);
	Location += 1;
	BF533HBase.PT_L_Arlam = BufToVal(&buf[Location],1);//Modfiy by Zhangzhihua 20111220
	Location += 1;//Modfiy by Zhangzhihua 20111220
	BF533HBase.ChannelSum = BufToVal(&buf[Location],1);


	//std::cout<<"The BF533HBase.BoardType is:"<<BF533HBase.BoardType<<std::endl; 

	return 0;
}

int CBF533::ParseSingleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int yk_no = BufToVal(&buf[6],1);
	unsigned char yk_type = BufToVal(&buf[5],1);

	if((terminalPtr->loadYkPointPtr(yk_no))->ExeResponEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkExeCon<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控执行返校报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	ResetTimerYkExe(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkExeCon);
	CmdConSig_(YK_EXE_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);//通知对上规约，遥控执行成功

	return 0;
}

int CBF533::ParseSingleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{

	return 0;
}


int CBF533::ParseSingleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CBF533::ParseDownloadEquCeck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	unsigned long int Sum = 0;
	size_t i;
	for (i = 5;i < exceptedBytes - 1;i ++)
	{
		Sum += buf[i];
	}
	if (Sum == Check_Sum)
	{
		SendTimes = 0;
		AddSendCmdVal(DOWNLOAD_EQU_PARA_SUCESS,DOWNLOAD_EQU_PARA_SUCESS_PRIORITY,terminalPtr);
	}
	else 
	{
		PrintMessage("下装装置参数反校失败！");
		std::cout<<"Sum："<<Sum<<"Check_Sum:"<< Check_Sum <<std::endl;
		if (SendTimes < ReSendTime)
		{
			AddSendCmdVal(DOWNLOAD_EQU_PARA,DOWNLOAD_EQU_PARA_PRIORITY,terminalPtr);
		}
		else 
		{
			SendTimes = 0;
		}
	}
	Check_Sum = 0;
	return 0;
}

int CBF533::ParseDownloadDelayValCeck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	unsigned long int Sum = 0;
	size_t i;
	for (i = 5;i < exceptedBytes -1;i ++)
	{
		Sum += buf[i];
	}
	if (Sum == Check_Sum)
	{
		SendTimes = 0;
		AddSendCmdVal(DOWNLOAD_DELAY_VAL_SUCESS,DOWNLOAD_DELAY_VAL_SUCESS_PRIORITY,terminalPtr);
	}
	else 
	{
		PrintMessage("下装保护定值反校失败！");
		if (SendTimes < ReSendTime)
		{
			AddSendCmdVal(DOWNLOAD_DELAY_VAL,DOWNLOAD_DELAY_VAL_PRIORITY,terminalPtr);
		}	
		else 
		{
			SendTimes = 0;
		}
		bAssembleDelayValAll_ = false;
	}
	Check_Sum = 0;
	return 0;
}

int CBF533::ParseDownloadDelayContrlCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	unsigned long int Sum = 0;
	size_t i;
	for (i = 5;i < exceptedBytes -1;i ++)
	{
		Sum += buf[i];
	}
	if (Sum == Check_Sum)
	{
		SendTimes = 0;
		AddSendCmdVal(DOWNLOAD_DELAY_CONTRL_SUCESS,DOWNLOAD_DELAY_CONTRL_SUCESS_PRIORITY,terminalPtr);
	}
	else 
	{
		PrintMessage("下装保护定值反校失败！");
		if (SendTimes < ReSendTime)
		{
			AddSendCmdVal(DOWNLOAD_DELAY_CONRL,DOWNLOAD_DELAY_CONTRL_PRIORITY,terminalPtr);
		}	
		else 
		{
			SendTimes = 0;
		}

		bAssembleDelayValAll_ = false;
	}

	Check_Sum = 0;

	return 0;
}

int CBF533::ParseDownloadChannelParaCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	unsigned long int Sum = 0;
	size_t i;
	for (i = 5;i < exceptedBytes -1;i ++)
	{
		Sum += buf[i];
	}
	if (Sum == Check_Sum)
	{
		SendTimes = 0;
		AddSendCmdVal(DOWNLOAD_CHANNEL_PARA_SUCESS,DOWNLOAD_CHANNEL_PARA_SUCESS_PRIORITY,terminalPtr);
	}
	else 
	{
		PrintMessage("下装通道参数反校失败！");
		if (SendTimes < ReSendTime)
		{
			AddSendCmdVal(DOWNLOAD_CHANNEL_PARA,DOWNLOAD_CHANNEL_PARA_PRIORITY,terminalPtr);
		}		
		else 
		{
			SendTimes = 0;
		}
	}

	Check_Sum = 0;

	return 0;
}

int CBF533::ParseDownloadChannelComrelCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	unsigned long int Sum = 0;
	size_t i;
	for (i = 5;i < exceptedBytes -1;i ++)
	{
		Sum += buf[i];
	}
	if (Sum == Check_Sum)
	{
		SendTimes = 0;
		AddSendCmdVal(DOWNLOAD_CHANNEL_COMREL_SUCESS,DOWNLOAD_CHANNEL_COMREL_SUCESS_PRIORITY,terminalPtr);
	}
	else 
	{
		PrintMessage("下装通道组合关系反校失败！");
		//std::cout<<"Sum is:"<<Sum<<",Check_Sum is:"<<Check_Sum<<std::endl;
		if (SendTimes < ReSendTime)
		{
			AddSendCmdVal(DOWNLOAD_CHANNEL_COMREL,DOWNLOAD_CHANNEL_COMREL_PRIORITY,terminalPtr);
		}		
		else 
		{
			SendTimes = 0;
		}
	}

	Check_Sum = 0;

	return 0;
}

int CBF533::ParseDownloadLineValCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	unsigned long int Sum = 0;
	size_t i;
	for (i = 5;i < exceptedBytes -1;i ++)
	{
		Sum += buf[i];
	}
	if (Sum == Check_Sum)//若反校成功则开始校验
	{
		SendTimes = 0;
		AddSendCmdVal(LINE_VAL_VER,LINE_VAL_VER_PRIORITY,terminalPtr);
	}
	else 
	{
		PrintMessage("下装线路定值反校失败！");
		std::cout<<"sum 的值为："<<Sum<< ",Check_Sum是："<<Check_Sum <<std::endl;
		if (SendTimes < ReSendTime)
		{
			AddSendCmdVal(DOWNLOAD_LINE_VAL,DOWNLOAD_LINE_VAL_PRIORITY,terminalPtr);
		}		
		else 
		{
			SendTimes = 0;
		}
	}

	Check_Sum = 0;

	return 0;
}

int CBF533::ParseDownloadLineBValCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	unsigned long int Sum = 0;
	size_t i;
	for (i = 5;i < exceptedBytes -1;i ++)
	{
		Sum += buf[i];
	}
	if (Sum == Check_Sum)//若反校成功则开始校验
	{
		SendTimes = 0;
		AddSendCmdVal(LINE_BVAL_VER,LINE_BVAL_VER_PRIORITY,terminalPtr);
	}
	else 
	{
		PrintMessage("下装保护电流定值反校失败！");
		std::cout<<"sum 的值为："<<Sum<< ",Check_Sum是："<<Check_Sum <<std::endl;
		if (SendTimes < ReSendTime)
		{
			AddSendCmdVal(DOWNLOAD_LINE_BVAL,DOWNLOAD_LINE_BVAL_PRIORITY,terminalPtr);
		}		
		else 
		{
			SendTimes = 0;
		}
	}

	Check_Sum = 0;

	return 0;
}

int CBF533::ParseLineBValVer(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	//std::cout<<"校验成功后存库... ."<<std::endl;
	DataBase::stLine_BCurVerVal Val;

	int Location = 18;//与定时全遥测使用同样的报文，故跳开前面无关的信息

	Val.timeout_flag = 0x07;

	Val.Line_No = BufToVal(&buf[5],1);

	Val.V_Val_BI1 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.V_Val_BI2 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.V_Val_BI3 = BufToVal(&buf[Location],2);
	//Location += 2;
	//std::cout<<"BF533收到的保护电流校验后值为："<<"Val.V_Val_BI1:"<<Val.V_Val_BI1<<" Val.V_Val_BI2:"<<Val.V_Val_BI2<<" Val.V_Val_BI3:"<<Val.V_Val_BI3<<std::endl;
	CmdConSig_(LINE_BVAL_VER_QYC,RETURN_CODE_ACTIVE,terminalPtr,Val);

	return 0;
}

int CBF533::ParseFaultEvent(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();

	int Location = 5;

	unsigned long Time;

	unsigned short FaultNO_;
	typeYcval FaultVal_;
	boost::posix_time::ptime FaultTime_;

	FaultNO_ = BufToVal(&buf[Location],2);
	Location += 2;
	//Time = (((buf[Location + 5]*256 + buf[Location + 4])*256 + buf[Location + 3])*256 + buf[Location + 2]) / 1.2;//BufToVal(&buf[2],2);
	Time = (BufToVal(&buf[Location],4) / SYNTIMEMULTIPLE);
	Location += 4;
	FaultVal_  = BufToVal(&buf[Location],2);

	//std::cout<<"现在时间是（SOE时标）：Time ="<<Time<<std::endl;

	unsigned short millisecond = ((Time % (1000*60*60))%(1000*60));
	unsigned char  minute = ((Time % (1000*60*60))/(1000*60));
	unsigned char  Hour = Time / (1000*60*60);
	time_duration td(minutes(minute) + seconds(millisecond/1000) + milliseconds(millisecond%1000));

	td += hours((lt.time_of_day()).hours());
	ptime timeVal(lt.date(),td);

	FaultTime_ = timeVal;


	terminalPtr->SaveFaultPoint(FaultNO_,FaultVal_,FaultTime_);


	DataBase::stEVENT Val;

	Location = 5;

	Val.No = BufToVal(&buf[Location],2);
	Location += 2;
	Val.EventTime = (BufToVal(&buf[Location],4) / SYNTIMEMULTIPLE);
	Location += 4;
	Val.EventValue  = BufToVal(&buf[Location],2);

	Val.Type=1;


	CmdConSig_(EVENT_MESSAGE,RETURN_CODE_ACTIVE,terminalPtr,Val);

	if (FaultRecordFlag_)
	{
		SaveFaultRecord(buf);
	}

	return 0;
}

int CBF533::ParseDownloadLineCoefCheck(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	unsigned long int Sum = 0;
	size_t i;
	for (i = 5;i < exceptedBytes -1;i ++)
	{
		Sum += buf[i];
	}
	if (Sum == Check_Sum)//若反校成功则开始校验
	{
		SendTimes = 0;
		AddSendCmdVal(LINE_VAL_VER,LINE_VAL_VER_PRIORITY,terminalPtr);
	}
	else 
	{
		PrintMessage("下装线路通道系数反校失败！");
		if (SendTimes < ReSendTime)
		{
			AddSendCmdVal(DOWNLOAD_LINE_COEF,DOWNLOAD_LINE_COEF_PRIORITY,terminalPtr);
		}		
		else 
		{
			SendTimes = 0;
		}
	}

	Check_Sum = 0;

	return 0;
}

int CBF533::ParseAllYCData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = ((BufToVal(&buf[2],2) - 1 - Line_NUM*1)/2);//计算遥测数
	int count = 0;
	int InfoDataLength = 2;
	int info_addr = 0;
	int Location = 5;
	int i;

	for(i = 0;i < info_num;i++)
	{
		if ((i%18) == 0)
		{
			Location += 1;
		}
		unsigned short YcVal = BufToVal(&buf[Location],2);

		if ((bYcAddRand_)&&((i%18 == 3)||(i%18 == 4)||(i%18 == 5)))
		{
			if (YcVal > 3)
			{
				YcVal = (YcVal * 10) + RandSeed(9);
			}
			else
			{
				YcVal = YcVal * 10;
			}
		}

		Location += 2;  
		int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,true/*terminalPtr->getInitCommPointFlag()*/);

		if (ret == DataBase::CauseActiveData)//判断是否产生变化遥测
		{
			count++;
		}
		//std::cout<<"遥测值："<<YcVal<<std::endl;
	}
	if (count > 0)
	{
		std::ostringstream ostr;
		ostr<<"全YC报文产生了ycvar，数目："<<count<<std::endl;
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CBF533::ParseLineCycTime(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = 18;//计算遥测数
	int count = 0;
	int InfoDataLength = 2;
	int info_addr = (BufToVal(&buf[5],1)*18);
	int Location = 6;
	int i = 0;

	for(i = 0;i < info_num;i++)
	{
		unsigned short YcVal = BufToVal(&buf[Location + i*InfoDataLength],2);

		terminalPtr->SaveYcQuality(info_addr + i,DataBase::CYxPoint::QualityActive);

		if ((bYcAddRand_)&&((i == 3)||(i == 4)||(i == 5)))
		{
			if (YcVal > 3)
			{
				YcVal = (YcVal * 10) + RandSeed(9);
			}
			else
			{
				YcVal = YcVal * 10;
			}
		}

		int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,true/*terminalPtr->getInitCommPointFlag()*/);

		if (ret == DataBase::CauseActiveData)//判断是否产生变化遥测
		{
			count++;
		}
	}

	if (count > 0)
	{
		std::ostringstream ostr;
		//std::cout<<"产生了变化遥测，个数count为："<<count<<std::endl;
		ostr<<"全YC报文产生了ycvar，数目："<<count<<std::endl;
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}
	if (BufToVal(&buf[5],1) == ((terminalPtr->getYcSum())/18 -1))
	{
		//SaveExtremum(terminalPtr);
	}

	if (YcOverAlarm_)
	{
		YcValOverAlarm(buf,terminalPtr);
	}

	if (NoCurRecord_)
	{
		RecordLineLoad(buf,terminalPtr);
	}

	return 0;
}

int CBF533::RecordLineLoad(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = 1;//计算遥测数
	int count = 0;
	int InfoDataLength = 2;
	int info_addr = (BufToVal(&buf[5],1));//获取线路号
	int Location = 6;

	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();

	time_duration td = (lt.time_of_day());
	ptime timeVal(lt.date(),td);

	unsigned short YcValUa = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValUb = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValUc = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValIa = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValIb = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValIc = BufToVal(&buf[Location],2);

	//	setResetYcOverIndex(info_addr + 70);

	//以一下用于记录断线时的馈线负荷
	if ((YcValIa < getNoCurDeadVal())&&(YcValIb < getNoCurDeadVal())&&(YcValIc < getNoCurDeadVal()))
	{
		if (!NoCurVal[info_addr].NoCurFlage)
		{
			NoCurVal[info_addr].NoCurFlage = true;
		}
	}
	else
	{
		if (!NoCurVal[info_addr].Author_)
		{
			NoCurVal[info_addr].Author_ = true;
		}
		NoCurVal[info_addr].NoCurFlage = false;
	}

	if ((NoCurVal[info_addr].NoCurFlage)&&(NoCurVal[info_addr].Author_))
	{
		SaveNoCurRecord(info_addr);
	} 
	else
	{
		NoCurVal[info_addr].Ua = YcValUa;
		NoCurVal[info_addr].Ub = YcValUb;
		NoCurVal[info_addr].Uc = YcValUc;
		NoCurVal[info_addr].Ia = YcValIa;
		NoCurVal[info_addr].Ib = YcValIb;
		NoCurVal[info_addr].Ic = YcValIc;
	}

	return 0;
}

int CBF533::YcValOverAlarm(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = 1;//计算遥测数
	int count = 0;
	int InfoDataLength = 2;
	int info_addr = (BufToVal(&buf[5],1));//获取线路号
	int Location = 6;
	unsigned char YxBitVal;

	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();

	time_duration td = (lt.time_of_day());
	ptime timeVal(lt.date(),td);

	unsigned short YcValUa = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValUb = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValUc = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValIa = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValIb = BufToVal(&buf[Location],2);
	Location += 2;
	unsigned short YcValIc = BufToVal(&buf[Location],2);

	//	setResetYcOverIndex(info_addr + 70);

	//以一下用于遥测越限告警
	if ((((YcValUa > getUMaxLimit())||(YcValUb > getUMaxLimit())||(YcValUc > getUMaxLimit())) || ((YcValUa < getUMinLimit())||(YcValUb < getUMinLimit())||(YcValUc < getUMinLimit()))) \
		|| (((YcValIa > getIMaxLimit())||(YcValIb > getIMaxLimit())||(YcValIc > getIMaxLimit())) || ((YcValIa < getIMinLimit())||(YcValIb < getIMinLimit())||(YcValIc < getIMinLimit()))))
	{
		if (!YcOverVal[info_addr].YcOveredFlage_)
		{
			YcOverVal[info_addr].YcOveredFlage_ = true;
			YxBitVal = 0x01;
			terminalPtr->SaveYxType(info_addr + 70,DataBase::single_yx_point);
			terminalPtr->SaveSoePoint(info_addr + 70,YxBitVal,DataBase::single_yx_point,timeVal);

			terminalPtr->SaveCosPoint(info_addr + 70,YxBitVal,DataBase::single_yx_point);

			CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
			CmdConSig_(SOE_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
		}

		//ResetTimerResetYcOver(terminalPtr,true);
	}
	else
	{
		if (YcOverVal[info_addr].YcOveredFlage_)
		{
			YcOverVal[info_addr].YcOveredFlage_ = false;
			YxBitVal = 0x00;
			terminalPtr->SaveYxType(info_addr + 70,DataBase::single_yx_point);
			terminalPtr->SaveSoePoint(info_addr + 70,YxBitVal,DataBase::single_yx_point,timeVal);

			terminalPtr->SaveCosPoint(info_addr + 70,YxBitVal,DataBase::single_yx_point);

			CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
			CmdConSig_(SOE_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
		}
	}

	return 0;
}

int CBF533::ParseCycDcIT(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int info_addr = BF533Base.LineNum * 18;
	int Location = 5;
	int info_num = ((BufToVal(&buf[2],2) - 1)/2);
	int InfoDataLength = 2;
	int count = 0;

	for (int i = 0; i < info_num;i ++)
	{
		unsigned short YcVal = BufToVal(&buf[Location + i*InfoDataLength],2);

		terminalPtr->SaveYcQuality(info_addr + i,DataBase::CYxPoint::QualityActive);

		int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,true/*terminalPtr->getInitCommPointFlag()*/);

		if (ret == DataBase::CauseActiveData)//判断是否产生变化遥测
		{
			count++;
		}
	}

	if (count > 0)
	{
		std::ostringstream ostr;

		ostr<<"全YC报文产生了ycvar，数目："<<count<<std::endl;
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CBF533::ParseDcValVerSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	DataBase::stDCVerVal Val; 
	int Location = 5;

	Val.V_Val_DC = BufToVal(&buf[Location],2);
	Val.ChannelNo = DcChannelNo_;
	Val.timeout_flag = 0x07;

	CmdConSig_(LINE_DCVAL_VER_SUCESS,RETURN_CODE_ACTIVE,terminalPtr,Val);

	return 0;
}

int CBF533::ParseCallPm(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	DataBase::stPMPara Val; 
	int Location = 5;

	Val.timeout_flag = 0x07;

	Val.LineNo = BufToVal(&buf[Location ++],1);
	Val.AngU1 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngU2 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngU3 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngCI1 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngCI2 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngCI3 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngBI1 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngBI2 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngBI3 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngU0 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.AngI0 = BufToVal(&buf[Location],2);

	CmdConSig_(CALL_PM_ANG_CON,RETURN_CODE_ACTIVE,terminalPtr,Val);

	return 0;
}

int CBF533::ParseSingalReset(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 5;
	int LineNo_ = BufToVal(&buf[Location],1);
	CmdConSig_(SIGNAL_RESET_CON,RETURN_CODE_ACTIVE,terminalPtr,LineNo_);//通知对上规约校验成功
	ResetTimerSingalReset(terminalPtr);
	return 0;
}

int CBF533::ParseCallDspVersion(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 5;
	int Val = BufToVal(&buf[Location],2);
	CmdConSig_(DSP_VERSION_INQ_CON,RETURN_CODE_ACTIVE,terminalPtr,Val);

	return 0;
}

int CBF533::ParseRebootBf533(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 5;

	char type_ = BufToVal(&buf[Location ++],1);
	int  ver = BufToVal(&buf[Location],2);

	if (type_ == 0x01)
	{
		std::ostringstream ostr;
		ostr<<"本次启动来自于PDZ_Flash应用程序，类型为:"<<type_<<"；版本号为:"<<ver<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		AddSendCmdVal(ERASE_BF533,ERASE_BF533_PRIORITY,terminalPtr);
		bAllowTBoot_ = false;
		ResetTimerTBootBf533(terminalPtr,false);
	} 
	else if (type_ == 0x02)
	{
		std::ostringstream ostr;
		ostr<<"本次启动来自于PDZ_DSP应用程序，类型为:"<<type_<<"；版本号为:"<<ver<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		AddSendCmdVal(T_BOOT_BF533,T_BOOT_BF533_PRIORITY,terminalPtr);
	}
	else
	{
		std::ostringstream ostr;
		ostr<<"本次启动来自于PDZ_DSP应用程序，类型为:"<<type_<<"；版本号为:"<<ver<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,terminalPtr);
	}

	bAllowReboot_ = false;
	ResetTimerRebootBf533(terminalPtr,false);

	return 0;
}

int CBF533::ParseTbootBf533(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	ResetTimerTBootBf533(terminalPtr,false);
	AddSendCmdVal(ERASE_BF533,ERASE_BF533_PRIORITY,terminalPtr);
	return 0;
}

int CBF533::ParseEraseBf533(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 5;

	char result = BufToVal(&buf[Location],1);

	if (result)
	{
		std::ostringstream ostr;
		ostr<<"Flash擦除成功，返回结果为："<<result<<"；开始下载程序代码！"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		AddSendCmdVal(DOWN_BF533_PRO,DOWN_BF533_PRO_PRIORITY,terminalPtr,1);
	}
	else
	{
		std::ostringstream ostr;
		ostr<<"Flash擦除失败，返回结果为："<<result<<"；重新擦除！"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		AddSendCmdVal(ERASE_BF533,ERASE_BF533_PRIORITY,terminalPtr);
	}
	return 0;
}

int CBF533::ParseDownBf533Pro(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 5;
	int this_No_;
	int total_Num_;
	char write_;
	char check_;
	int Rchecksum_;
	int Schecksum_;

	this_No_ = BufToVal(&buf[Location],2);
	Location += 2;
	total_Num_ = BufToVal(&buf[Location],2);
	Location += 2;
	check_ = BufToVal(&buf[Location],1);
	Location += 1;
	write_ = BufToVal(&buf[Location],1);
	Location += 1;
	Schecksum_ = BufToVal(&buf[Location],1);
	Location += 1;
	Rchecksum_ = BufToVal(&buf[Location],1);
	Location += 1;

	if (this_No_ == This_N0)
	{
		AddStatusLogWithSynT("检查序列号通过！\n");
	} 
	else
	{
		
		AddStatusLogWithSynT("两次发送序列号不同！\n");
		return -1;
	}

	if (write_)
	{
		AddStatusLogWithSynT("BF533程序写入正确！\n");
	} 
	else
	{
		AddStatusLogWithSynT("BF533程序写入失败！\n");
	}

	if (check_)
	{
		AddStatusLogWithSynT("BF533程序校验正确！\n");
	} 
	else
	{
		AddStatusLogWithSynT("BF533程序校验失败！\n");
		return -1;
	}

	if(Total_NUM > This_N0)
	{
		This_N0 ++;
		AddSendCmdVal(DOWN_BF533_PRO,DOWN_BF533_PRO_PRIORITY,terminalPtr,This_N0);
	}
	else 
	{
		AddStatusLogWithSynT("所有程序传输完成\n");
		FileHandleFinish();
	}
	return 0;
}

int CBF533::ParseAllYxCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[5],2);
	int info_addr = 0;
	int count = 0;
	int Location = 7;

	while (info_addr < info_num) //存放所有遥信
	{
		unsigned char Data = buf[Location ++];

		count += ParseSaveAllYX( info_addr,terminalPtr,Data,0x01,0);
		info_addr ++;
		if (info_addr == info_num)
		{
			break;
		}
		count += ParseSaveAllYX( info_addr, terminalPtr,Data,0x02,1);
		info_addr ++;
		if (info_addr == info_num)
		{
			break;
		}
		count += ParseSaveAllYX( info_addr, terminalPtr,Data,0x04,2);
		info_addr ++;
		if (info_addr == info_num)
		{
			break;
		}
		count += ParseSaveAllYX( info_addr, terminalPtr,Data,0x08,3);
		info_addr ++;
		if (info_addr == info_num)
		{
			break;
		}
		count += ParseSaveAllYX( info_addr, terminalPtr,Data,0x10,4);
		info_addr ++;
		if (info_addr == info_num)
		{
			break;
		}
		count += ParseSaveAllYX( info_addr, terminalPtr,Data,0x20,5);
		info_addr ++;
		if (info_addr == info_num)
		{
			break;
		}
		count += ParseSaveAllYX( info_addr, terminalPtr,Data,0x40,6);
		info_addr ++;
		if (info_addr == info_num)
		{
			break;
		}
		count += ParseSaveAllYX( info_addr, terminalPtr,Data,0x80,7);
		info_addr ++;
		if (info_addr == info_num)
		{
			break;
		}
	}

	if (count > 0)
	{
		std::ostringstream ostr;
		ostr<<"单点全YX报文产生了COS，数目："<<count<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}


	return 0;

}

int CBF533::ParseSaveAllYX(int info_addr,share_terminal_ptr terminalPtr,unsigned char Data,unsigned char div,int offset)
{
	unsigned char yx_val = ((Data & div) >> offset)&0x01;
	int count = 0;

	terminalPtr->SaveYxType(info_addr,DataBase::single_yx_point);
	int ret = terminalPtr->SaveOriYxVal(info_addr,yx_val,true/*terminalPtr->getInitCommPointFlag()*/);
	if (ret == DataBase::CauseActiveData)
	{
		count++;
	}
	return count;
}

int CBF533::ParseSingleSOE(unsigned char * buf, share_terminal_ptr terminalPtr)
{

	int info_num = BufToVal(&buf[5],2);
	int count = 0;
	int Location = 7;  //点号从第7个开始

	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();


	unsigned char InfoDataLength = 6;

	for (int i = 0;i < info_num;i ++)
	{
		int info_addr = (buf[Location+ i*InfoDataLength]) + (((buf[Location + i*InfoDataLength + 1])&0x7F)*256);

		unsigned char YxBitVal = ((buf[Location + i*InfoDataLength + 1]&0x80 ) >> 7)& 0x01;//获得编号最高位为遥信状态 先得到D7位，再右移7位，然后屏蔽高7位得到遥信状态


		unsigned long Time = (((buf[Location + i*InfoDataLength + 5]*256 + buf[Location + i*InfoDataLength + 4])*256 + buf[Location + i*InfoDataLength + 3])*256 + buf[Location + i*InfoDataLength + 2]) / SYNTIMEMULTIPLE;//BufToVal(&buf[2],2);
		//      /*******************************/
		//boost::posix_time::time_duration tdd = (boost::posix_time::microsec_clock::local_time()).time_of_day();
		//std::cout<<"现在时间是（SOE时标）：Time ="<<Time<<",系统当前时间是:Time="<<tdd.total_milliseconds()<<std::endl;
		///*******************************/
		unsigned short millisecond = ((Time % (1000*60*60))%(1000*60));
		unsigned char  minute = ((Time % (1000*60*60))/(1000*60));
		unsigned char  Hour = Time / (1000*60*60);
		time_duration td(minutes(minute) + seconds(millisecond/1000) + milliseconds(millisecond%1000));

		//   std::cout<<"现在时间是（BF533）：Time ="<<Time<<",换算之后是"<<Hour<<"时，"<<minute<<"分,"<<(millisecond/1000)<<"秒,"<<(millisecond%1000)<<"毫秒"<<std::endl;

		td += hours((lt.time_of_day()).hours());
		ptime timeVal(lt.date(),td);


		//std::cout<<"总个数为："<<info_num<<",Soe地址为："<<info_addr<<std::endl;

		terminalPtr->SaveYxType(info_addr,DataBase::single_yx_point);
		terminalPtr->SaveSoePoint(info_addr,YxBitVal,DataBase::single_yx_point,timeVal);

		terminalPtr->SaveCosPoint(info_addr,YxBitVal,DataBase::single_yx_point);

		if (SOEStroeFlag_)
		{
			SaveSOERecord(info_addr,YxBitVal,Time);
		}

		std::ostringstream ostr;
		ostr<<"收到单点COS，点号"<<info_addr<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<std::endl;
		AddStatusLogWithSynT(ostr.str());
	}


	if (info_num > 0)
	{
		CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
		CmdConSig_(SOE_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
	}

	return 0;
}

int CBF533::ParseLineValVerQyc(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{

	DataBase::stLine_Val VerQYC;    //定义在Ykpoint.h

	int Location = 5;  //线路号所在位置

	VerQYC.Line_no = BufToVal(&buf[Location ++],1);  //读取线路号

	//std::cout<<"线路号是："<< VerQYC.Line_no <<std::endl;

	{
		VerQYC.D_Value_Ua = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_Ub = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_Uc = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_CIa = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_CIb = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_CIc = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_BIa = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_BIb = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_BIc = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_U0 = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.D_Value_I0 = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.Angle_UaIa = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.Angle_UbIb = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.Angle_UcIc = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.Fre_Val = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.Cos_Val = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.P_Val = BufToVal(&buf[Location],2); 
		Location += 2;
		VerQYC.Q_Val = BufToVal(&buf[Location],2); 
	}  
	//std::cout<<"收到的定值校验后全遥测为："<<std::endl;
	//std::cout<<VerQYC.D_Value_Ua<<"/"<<VerQYC.D_Value_Ub<<"/"<<VerQYC.D_Value_Uc<<std::endl;
	//std::cout<<VerQYC.D_Value_CIa<<"/"<<VerQYC.D_Value_CIb<<"/"<<VerQYC.D_Value_CIc<<std::endl;
	//std::cout<<VerQYC.D_Value_U0<<"/"<<VerQYC.D_Value_I0<<std::endl;
	//std::cout<<VerQYC.Fre_Val<<"/"<<VerQYC.Cos_Val<<"/"<<VerQYC.P_Val<<std::endl;
	VerQYC.timeout_flag = 0x07;
	//std::cout<<"开始添加校验成功指令"<<LINE_VAL_VER_QYC<<std::endl;
	CmdConSig_(LINE_VAL_VER_QYC,RETURN_CODE_ACTIVE,terminalPtr,VerQYC);//通知对上规约校验成功

	return 0;

}


int CBF533::ParseHarmonicCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	DataBase::stHarmonic Harmonic;    //定义在Ykpoint.h
	int Location = 5;  //线路号所在位置
	Harmonic.Line_no = BufToVal(&buf[Location ++],1);  //读取线路号
	Harmonic.harmonic_no = BufToVal(&buf[Location ++],1);  //读取谐波次数

	Harmonic.Value_Ua = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.Value_Ub = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.Value_Uc = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.Value_Ia = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.Value_Ib = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.Value_Ic = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.Value_U0 = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.Value_I0 = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.P_Val = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.Q_Val = BufToVal(&buf[Location],2); 
	Location += 2;
	Harmonic.Per_Val = BufToVal(&buf[Location],2); 

	CmdConSig_(HARMONIC_CON,RETURN_CODE_ACTIVE,terminalPtr,Harmonic);//通知对上规约校验成功

	return 0;
}

int CBF533::ParseDownloadDownDelayValSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	if (bAssembleDelayValAll_)
	{
		if(Line_No_  <  (BF533Base.LineNum - 1))//判断是否所有的线路都下装完成
		{
			int line_no = Line_No_ + 1;
			AddSendCmdVal(DOWNLOAD_DELAY_VAL,DOWNLOAD_DELAY_VAL_PRIORITY,terminalPtr,line_no); //未下装完成则继续下装
			//std::cout<<",保护定值下装完成，即将下装的是第"<< line_no<< "条线路，总线路数为："<< BF533Base.LineNum <<std::endl; 
		}
		else    //全部线路定值下装完成，开始下装保护控制字
		{
			Line_No_ = 0;
			AddSendCmdVal(DOWNLOAD_DELAY_CONRL,DOWNLOAD_DELAY_CONTRL_PRIORITY,terminalPtr,0);
			//std::cout<<"开始下装的是第"<< Line_No_ << "条线路保护控制字，总线路数为："<< BF533Base.LineNum <<std::endl; 
		}
	} 
	else
	{
		AddSendCmdVal(DOWNLOAD_DELAY_CONRL,DOWNLOAD_DELAY_CONTRL_PRIORITY,terminalPtr,Line_No_);
	}

	return 0;
}

int CBF533::ParseDownloadDownDelayContrlSucess(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	if (bAssembleDelayValAll_)
	{
		if(Line_No_  <  (BF533Base.LineNum - 1))//判断是否所有的线路都下装完成
		{
			int line_no = Line_No_ + 1;
			AddSendCmdVal(DOWNLOAD_DELAY_CONRL,DOWNLOAD_DELAY_CONTRL_PRIORITY,terminalPtr,line_no); //未下装完成则继续下装
			//std::cout<<"即将下装的是第"<< (Line_No_ + 1 )<< "条线路，总线路数为："<< BF533Base.LineNum <<std::endl; 
		}
		else
		{
			Line_No_ = 0;
			bAssembleDelayValAll_=false;
			std::cout<<"保护控制字全部下装完成"<<std::endl; 
			char Val = 0x0A;//传递给对上规约，传送原因为0x0A
			CmdConSig_(DOWNLOAD_PARA_CON,RETURN_CODE_ACTIVE,terminalPtr,Val);
		}
	} 
	else
	{
		Line_No_ = 0;
		bAssembleDelayValAll_=false;
		std::cout<<"保护控制字全部下装完成"<<std::endl; 
		char Val = 0x0A;//传递给对上规约，传送原因为0x0A
		CmdConSig_(DOWNLOAD_PARA_CON,RETURN_CODE_ACTIVE,terminalPtr,Val);
	}

	//if((Line_No_  <  (BF533Base.LineNum - 1)))
	//{
	//	int line_no = Line_No_ + 1;
	//	AddSendCmdVal(DOWNLOAD_DELAY_VAL,DOWNLOAD_DELAY_VAL_PRIORITY,terminalPtr,line_no); //未下装完成则继续下装
	//	std::cout<<"继续下装的是第"<< (Line_No_ + 1 )<< "条线路，总线路数为："<< BF533Base.LineNum <<std::endl;
	//}
	//else
	//{
	//	bAssembleDelayValAll_=false;
	//	std::cout<<"保护控制字全部下装完成"<<std::endl; 
	//	char Val = 0x0A;//传递给对上规约，传送原因为0x0A
	//	CmdConSig_(DOWNLOAD_PARA_CON,RETURN_CODE_ACTIVE,terminalPtr,Val);
	//}

	return 0;
}

int CBF533::ParseCallValCoefCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 5;  //线路号所在位置
	DataBase::stLine_ValCoef Data;

	Data.timeout_flag = 0x07;

	Data.Line_no =  BufToVal(&buf[Location ++],1);  //读取线路号
	Data.Flag_Ua =  BufToVal(&buf[Location ++],1);
	Data.H_Value_Ua = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_Ub =  BufToVal(&buf[Location ++],1);
	Data.H_Value_Ub = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_Uc =  BufToVal(&buf[Location ++],1);
	Data.H_Value_Uc = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_CIa =  BufToVal(&buf[Location ++],1);
	Data.H_Value_CIa = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_CIb =  BufToVal(&buf[Location ++],1);
	Data.H_Value_CIb = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_CIc =  BufToVal(&buf[Location ++],1);
	Data.H_Value_CIc = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_BIa =  BufToVal(&buf[Location ++],1);
	Data.H_Value_BIa = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_BIb =  BufToVal(&buf[Location ++],1);
	Data.H_Value_BIb = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_BIc =  BufToVal(&buf[Location ++],1);
	Data.H_Value_BIc = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_U0 =  BufToVal(&buf[Location ++],1);
	Data.H_Value_U0 = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_I0 =  BufToVal(&buf[Location ++],1);
	Data.H_Value_I0 = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_Angle_UaIa =  BufToVal(&buf[Location ++],1);
	Data.Angle_UaIa = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_Angle_UbIb =  BufToVal(&buf[Location ++],1);
	Data.Angle_UbIb = BufToVal(&buf[Location],2);
	Location += 2;
	Data.Flag_Angle_UcIc =  BufToVal(&buf[Location ++],1);
	Data.Angle_UcIc = BufToVal(&buf[Location],2);
	Location += 2;

	CmdConSig_(CALL_VALCOEF_CON,RETURN_CODE_ACTIVE,terminalPtr,Data);//通知对上规约校验成功
	PrintMessage("定值校验成功添加对上消息成功... ...");
	return 0;
}

int CBF533::ParseDownLoadLineCoef(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	unsigned long int Sum = 0;
	size_t i;
	for (i = 5;i < exceptedBytes -1;i ++)
	{
		Sum += buf[i];
	}
	if (Sum == Check_Sum)//若反校成功则开始校验
	{
		SendTimes = 0;
		AddSendCmdVal(LINE_VAL_VER,LINE_VAL_VER_PRIORITY,terminalPtr);
	}
	else 
	{
		PrintMessage("下装线路系数反校失败！");
		std::cout<<"sum 的值为："<<Sum<< ",Check_Sum是："<<Check_Sum <<std::endl;
		if (SendTimes < ReSendTime)
		{
			AddSendCmdVal(DOWNLOAD_LINE_COEF,DOWNLOAD_LINE_COEF_PRIORITY,terminalPtr);
		}		
		else 
		{
			SendTimes = 0;
		}
	}

	Check_Sum = 0;

	return 0;
}

int CBF533::ParseBoardInqCon(unsigned char * buf, share_terminal_ptr terminalPtr,size_t exceptedBytes)
{
	int Location = 5;  //线路号所在位置
	DataBase::stBoardInq Data;

	Data.YxBoardNum =  BufToVal(&buf[Location ++],1);
	Data.YkBoardNum =  BufToVal(&buf[Location ++],1);
	Data.YcBoardNum =  BufToVal(&buf[Location ++],1);

	CmdConSig_(BOARD_REQ_CON,RETURN_CODE_ACTIVE,terminalPtr,Data);//通知对上规约校验成功

	return 0;
}

void CBF533::InitDefaultFrameElem()
{
	FrameTypeLength_ =   1;                           //报文类型标识的字节长度
	FrameTypeLocation_ = 4;                           //报文类型标识的字节定位
	Check_Sum = 0;                             //校验和
	ReSendTime = 3;                            //反校失败时的重发次数
}

void CBF533::InitDefaultTimeOut()
{
	FristFlag = true;
	//timeOutCallAllDataTime_ = DEFAULT_timeOutCallAllDataTime;
	timeOutYkCancel_        = DEFAULT_timeOutYkCancel;
	timeOutSynTime_         = DEFAULT_timeOutSynTime;
	timeOutYkExe_           = DEFAULT_timeOutYkExe;
	timeOutChannelError_    = DEFAULT_timeOutChannelError;
	timeOutSaveHis_         = DEFAULT_timeOutSaveHis;
	timeOutCallYMData_      = DEFAULT_timeOutCallYMData;
	timeOutAllData_         = DEFAULT_timeOutAllData;
	timeOutBattryActive_    = BF533Base.HuoHua_Day;
	timeOutSingalReset_     = DEFAULT_timeOutSingalReset;
	//timeOutResetYcOver_          = DEFAULT_timeOutResetYcOver;
	timeOutRebootBf533_     = DEFAULT_timeOutRebootBf533;
	timeOutTBootBf533_      = DEFAULT_timeOutTBootBf533;

}

void CBF533::InitDefaultConfig()
{
	BF533Base.LineNum = DEFAULT_LineNum;
	Read_BasePara();//读取存储历史数据的时间
	Read_DevSanyaoPara();
}

void CBF533::InitDefaultBF533Download()
{
	bBF533AutoDownload_ = false;
	BF533ProName        = "PDZ_DSP.bin";
	This_Length_        = 0;
	This_N0             = 0;
	Total_NUM           = 0;
}

void CBF533::InitDefaultPara()
{
	DcChannelNo_ = 0;

	for (int i = 0;i < LINEMAX; i ++)
	{
		NoCurVal[i].Author_ = false;
		NoCurVal[i].NoCurFlage = false;
		YcOverVal[i].YcOveredFlage_ = false;
	}

	setUMaxLimit(65533);
	setUMinLimit(0);
	setIMaxLimit(65533);
	setIMinLimit(0);
	setNoCurDeadVal(10);

	YcOverAlarm_ = false;
	NoCurRecord_ = false;

}

void CBF533::InitDefaultFileData()
{
	SOEStroeNum_     = DEFAULT_SOEStroeNum;
	FaultRecordMax_  = DEFAULT_FaultRecordMax;
	SOEStroeFlag_    = false;
	FaultRecordFlag_ = false;
	YMDataFlag_      = false;
	YMDataRecordType_= MonthRecordFlag;
	YMDataFlag_      = false;
	YMDataRecordNum_ = 5;            //积分电度的记录最大条数
	YMDataCount_     = 5;            //每一次记录的最大条数
	Line_NUM         = LINEMAX;
	BF533Base.HuoHua_AutoFlag = 0;
	BF533Base.HuoHua_Day      = DEFAULT_timeOutBattryActive;
	BF533HBase.Version        = 131;
}


void CBF533::ResetTimerSynTime(share_commpoint_ptr point,bool bContinue,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerSynTime_->expires_from_now(boost::posix_time::seconds(getMeanvalueOfPointsSum(MIN_timeOutSynTime,timeOutSynTime_)));
		}
		else
		{
			timerSynTime_->expires_from_now(boost::posix_time::seconds(val));
		}
		timerSynTime_->async_wait(boost::bind(&CBF533::handle_timerSynTime,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerSynTime_->cancel();
	}
}

void CBF533::ResetTimerYkExe(share_commpoint_ptr point,int yk_no,bool bContinue ,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerYkExe_->expires_from_now(boost::posix_time::seconds(timeOutYkExe_));
		}
		else
		{
			timerYkExe_->expires_from_now(boost::posix_time::seconds(val));
		}
		timerYkExe_->async_wait(boost::bind(&CBF533::handle_timerYkExe,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkExe_->cancel();
	}
}

void CBF533::ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue ,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			YkCancelTimer_->expires_from_now(boost::posix_time::seconds(timeOutYkCancel_));
		}
		else
		{
			YkCancelTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		YkCancelTimer_->async_wait(boost::bind(&CBF533::handle_timerYkCancel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		YkCancelTimer_->cancel();
	}
}

void CBF533::ResetTimerChannelError(share_commpoint_ptr point,bool bContinue ,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			ChannelErrorTimer_->expires_from_now(boost::posix_time::seconds(getMeanvalueOfPointsSum(MIN_timeOutChannelError,timeOutChannelError_)));
		}
		else
		{
			ChannelErrorTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		ChannelErrorTimer_->async_wait(boost::bind(&CBF533::handle_timerChannelError,this,boost::asio::placeholders::error,point));
	}
	else
	{
		ChannelErrorTimer_->cancel();
	}

}

void CBF533::ResetTimerSaveHis(share_terminal_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			SaveHisTimer_->expires_from_now(boost::posix_time::seconds(timeOutSaveHis_));
		}
		else
		{
			SaveHisTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		SaveHisTimer_->async_wait(boost::bind(&CBF533::handle_timerSaveHis,this,boost::asio::placeholders::error,point));
	}
	else
	{
		SaveHisTimer_->cancel();
	}
}

void CBF533::ResetTimerCallYMData(share_terminal_ptr point,bool bContinue/* = true*/,unsigned short val /*= 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			CallYMDataTimer_->expires_from_now(boost::posix_time::seconds(timeOutCallYMData_));
		}
		else
		{
			CallYMDataTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		CallYMDataTimer_->async_wait(boost::bind(&CBF533::handle_timerCallYMData,this,boost::asio::placeholders::error,point));
	}
	else
	{
		CallYMDataTimer_->cancel();
	}
}

void CBF533::ResetTimerAllData(share_terminal_ptr point,bool bContinue/* = true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			AllDataTimer_->expires_from_now(boost::posix_time::seconds(timeOutAllData_));
		}
		else
		{
			AllDataTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		AllDataTimer_->async_wait(boost::bind(&CBF533::handle_timerAllData,this,boost::asio::placeholders::error,point));
	}
	else
	{
		AllDataTimer_->cancel();
	}
}

void CBF533::ResetTimerBattryActive(share_terminal_ptr point,bool bContinue/* = true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			BattryActiveTimer_->expires_from_now(boost::posix_time::hours(timeOutBattryActive_));
		}
		else
		{
			BattryActiveTimer_->expires_from_now(boost::posix_time::hours(val));
		}
		BattryActiveTimer_->async_wait(boost::bind(&CBF533::handle_timerBattryActive,this,boost::asio::placeholders::error,point));
	}
	else
	{
		BattryActiveTimer_->cancel();
	}
}

void CBF533::ResetTimerBattryActiveOver(share_terminal_ptr point,bool bContinue/* = true*/,unsigned short val/* = 0*/)
{

	if (bContinue)
	{
		if (val == 0)
		{
			BattryActiveOverTimer_->expires_from_now(boost::posix_time::seconds(timeOutBattryActiveOver_));
		}
		else
		{
			BattryActiveOverTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		BattryActiveOverTimer_->async_wait(boost::bind(&CBF533::handle_timerBattryActiveOver,this,boost::asio::placeholders::error,point));
	}
	else
	{
		BattryActiveOverTimer_->cancel();
	}
}

void CBF533::ResetTimerSingalReset(share_commpoint_ptr point,bool bContinue/* = false*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			SingalResetTimer_->expires_from_now(boost::posix_time::seconds(timeOutSingalReset_));
		}
		else
		{
			SingalResetTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		SingalResetTimer_->async_wait(boost::bind(&CBF533::handle_timerSingalReset,this,boost::asio::placeholders::error,point));
	}
	else
	{
		BattryActiveOverTimer_->cancel();
	}
}

void CBF533::ResetTimerRebootBf533(share_terminal_ptr terminalPtr,bool bContinue,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			ResetRebootBf533Timer_->expires_from_now(boost::posix_time::seconds(timeOutRebootBf533_));
		}
		else
		{
			ResetRebootBf533Timer_->expires_from_now(boost::posix_time::seconds(val));
		}
		ResetRebootBf533Timer_->async_wait(boost::bind(&CBF533::handle_timerRebootBf533,this,boost::asio::placeholders::error,terminalPtr));
	}
	else
	{
		ResetRebootBf533Timer_->cancel();
	}
}

void CBF533::ResetTimerTBootBf533(share_terminal_ptr terminalPtr,bool bContinue,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			ResetTBootBf533Timer_->expires_from_now(boost::posix_time::milliseconds(timeOutTBootBf533_));
		}
		else
		{
			ResetTBootBf533Timer_->expires_from_now(boost::posix_time::milliseconds(val));
		}
		ResetTBootBf533Timer_->async_wait(boost::bind(&CBF533::handle_timerTBootBf533,this,boost::asio::placeholders::error,terminalPtr));
	}
	else
	{
		ResetTBootBf533Timer_->cancel();
	}
}

void CBF533::handle_timerAllData(const boost::system::error_code& error,share_terminal_ptr point)
{
	if (!error)
	{
		AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,point);
	}
}

void CBF533::handle_timerBattryActive(const boost::system::error_code& error,share_terminal_ptr point)
{
	if (!error)
	{
		AddSendCmdVal(BATTERY_ACTIVE,BATTERY_ACTIVE_PRIORITY,point);
		ResetTimerBattryActive(point,true,0);//定时下一次启动
		ResetTimerBattryActiveOver(point,true,0);//定时退出
	}
}

void CBF533::handle_timerBattryActiveOver(const boost::system::error_code& error,share_terminal_ptr point)
{
	if (!error)
	{
		AddSendCmdVal(BATTERY_ACTIVE_OVER,BATTERY_ACTIVE_OVER_PRIORITY,point);
		ResetTimerBattryActiveOver(point,false,0);
	}
}

void CBF533::handle_timerSingalReset(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		CmdConSig_(SIGNAL_RESET_CON,RETURN_CODE_ACTIVE,point,-1);
	}
}

void CBF533::handle_timerRebootBf533(const boost::system::error_code& error,share_terminal_ptr terminalPtr)
{
	if (bAllowReboot_)
	{
		AddSendCmdVal(REBOOT_BF533,REBOOT_BF533_PRIORITY,terminalPtr);
	}
}

void CBF533::handle_timerTBootBf533(const boost::system::error_code& error,share_terminal_ptr terminalPtr)
{
	AddSendCmdVal(T_BOOT_BF533,T_BOOT_BF533_PRIORITY,terminalPtr);
}

void CBF533::handle_timerCallYMData(const boost::system::error_code& error,share_terminal_ptr point)
{
	if (!error)
	{
		AddSendCmdVal(CALL_YMDATA_ACK,CALL_YMDATA_ACK_PRIORITY,point);
	}
	ResetTimerCallYMData(point,true,0);
}

void CBF533::handle_timerSaveHis(const boost::system::error_code& error,share_terminal_ptr point)
{
	if (!error)
	{
		//PrintMessage("记录历史数据... ...");
		//SaveHistoricalData(point);
	}
	ResetTimerSaveHis(point,true,0);
}

void CBF533::handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
{
	if (!error)
	{
		//PrintMessage("超时未收到遥控执行指令，自动撤销... ...");
		AddSendCmdVal(YK_CANCEL_ACT,YK_CANCEL_ACT_PRIORITY,point,yk_no);
	}
}

//void CBF533::handle_timerCallAllDataTime(const boost::system::error_code& error,share_commpoint_ptr point)
//{
//	if (!error)
//	{
//		AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,point);
//	}
//}

void CBF533::handle_timerSynTime(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,point);
	}
}

void CBF533::handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,int yk_no)
{
	if (!error)
	{
		if (point)
		{
			if (point->getCommPointType() == TERMINAL_NODE)
			{
				share_terminal_ptr terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(point);
				if (terminalPtr)
				{
					//terminalPtr->setYkStatus(yk_no,DataBase::YkExeTimeOut);
					(terminalPtr->loadYkPointPtr(yk_no))->TimeOutEvent();
				}
			}

			CmdConSig_(YK_EXE_CON,RETURN_CODE_TIMEOUT,point,yk_no);
			AddStatusLogWithSynT("规约遥控执行命令超时。\n");
		}
	}
}

void CBF533::handle_timerChannelError(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		PrintMessage("超时未收到定时全遥测，通道有误！");
		AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,point);
	}
}

void CBF533::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	//CallAllDataTimer_.reset(new deadline_timer(io_service,seconds(timeOutCallAllDataTime_)));
	//AddTimer(CallAllDataTimer_);

	timerSynTime_.reset(new deadline_timer(io_service,seconds(timeOutSynTime_)));
	AddTimer(timerSynTime_);

	timerYkExe_.reset(new deadline_timer(io_service,seconds(timeOutYkExe_)));
	AddTimer(timerYkExe_);

	ChannelErrorTimer_.reset(new deadline_timer(io_service,seconds(timeOutChannelError_)));
	AddTimer(ChannelErrorTimer_);

	YkCancelTimer_.reset(new deadline_timer(io_service,seconds(timeOutYkCancel_)));
	AddTimer(ChannelErrorTimer_);

	SaveHisTimer_.reset(new deadline_timer(io_service,seconds(timeOutSaveHis_)));
	AddTimer(SaveHisTimer_);

	CallYMDataTimer_.reset(new deadline_timer(io_service,seconds(timeOutCallYMData_)));
	AddTimer(CallYMDataTimer_);

	AllDataTimer_.reset(new deadline_timer(io_service,seconds(timeOutAllData_)));
	AddTimer(AllDataTimer_);

	BattryActiveTimer_.reset(new deadline_timer(io_service,hours(timeOutBattryActive_)));
	AddTimer(BattryActiveTimer_);

	BattryActiveOverTimer_.reset(new deadline_timer(io_service,hours(timeOutBattryActiveOver_)));
	AddTimer(BattryActiveOverTimer_);

	SingalResetTimer_.reset(new deadline_timer(io_service,hours(timeOutSingalReset_)));
	AddTimer(SingalResetTimer_);

	ResetRebootBf533Timer_.reset(new deadline_timer(io_service,hours(timeOutRebootBf533_)));
	AddTimer(ResetRebootBf533Timer_);

	ResetTBootBf533Timer_.reset(new deadline_timer(io_service,hours(timeOutTBootBf533_)));
	AddTimer(ResetTBootBf533Timer_);

}

//打印信息函数
void CBF533::PrintMessage(const char PrintBuf[])
{
	std::ostringstream ostr;
	ostr<<PrintBuf<<std::endl; 
	AddStatusLogWithSynT(ostr.str());
	//std::cout<<PrintBuf<<std::endl; 
}

int CBF533::LoadRecodFileCfg(void)
{
	FileSystem::CMarkup xml;
	if (!xml.Load(RecordFileCfg))
	{
		//PrintMessage("Load RecordFileCfg.xml不存在");
		//Creat_RecordFileCfg();
		//xml.Load(RecordFileCfg);
		return -1;
	}

	xml.ResetMainPos();
	if (xml.FindElem("RecordFileCfg"))
	{
		xml.IntoElem();
		xml.ResetMainPos();
		if (xml.FindElem("FaultRecord"))
		{
			if (xml.FindChildElem("SaveFlag")) 
			{
				std::string strTmp = xml.GetChildData();
				FaultRecordFlag_ = TransStringTobool(strTmp);
			}

			if (xml.FindChildElem("MaxRecords"))
			{
				std::string strTmp = xml.GetChildData();
				boost::algorithm::trim(strTmp);
				try
				{
					FaultRecordMax_ = boost::lexical_cast<unsigned int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();
				}
			}
		}

		if (xml.FindElem("SOEStroe"))
		{
			if (xml.FindChildElem("SaveFlag")) 
			{
				std::string strTmp = xml.GetChildData();
				SOEStroeFlag_ = TransStringTobool(strTmp);
			}

			if (xml.FindChildElem("MaxRecords"))
			{
				std::string strTmp = xml.GetChildData();
				boost::algorithm::trim(strTmp);
				try
				{
					SOEStroeNum_ = boost::lexical_cast<unsigned int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();
				}
			}
		}

		if (xml.FindElem("YMData"))
		{
			if (xml.FindChildElem("SaveFlag")) 
			{
				std::string strTmp = xml.GetChildData();
				YMDataFlag_ = TransStringTobool(strTmp);
			}

			if (xml.FindChildElem("Periord")) 
			{
				std::string strTmp = xml.GetChildData();
				//boost::algorithm::trim(strTmp);
				YMDataRecordType_ = TransStringToDate(strTmp);
			}

			if (xml.FindChildElem("MaxPeriords"))
			{
				std::string strTmp = xml.GetChildData();
				boost::algorithm::trim(strTmp);
				try
				{
					YMDataRecordNum_ = boost::lexical_cast<unsigned int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();
				}
			}

			if(xml.FindChildElem("MaxRecordsInPeriod"))
			{
				std::string strTmp = xml.GetChildData();
				boost::algorithm::trim(strTmp);
				try
				{
					YMDataCount_ = boost::lexical_cast<unsigned int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();
				}
			}
		}

		xml.OutOfElem();
	}//RecordFileCfg
	return 0;
}

int CBF533::Read_BasePara(void)
{

	FileSystem::CMarkup xml;
	if (!xml.Load(BFBasePara))
	{
		//PrintMessage("Load BasePara.xml出错");
		//Creat_BasePara();
		//xml.Load(BFBasePara);
		return -1;
	}

	//CProtocol::LoadXmlCfg(xml);

	xml.ResetMainPos();//将当前主位置复位为第一个兄弟位置之前。
	if (xml.FindElem("PDZ_BasePara_Set"))
	{

		xml.IntoElem();
		xml.ResetMainPos();
		if (xml.FindElem("PDZ_Name"))
		{		
			std::string strTmp = xml.GetData();
		}

		xml.ResetMainPos();
		if (xml.FindElem("PDZ_Addr"))
		{		
			std::string strTmp = xml.GetData();
		}
		xml.ResetMainPos();
		if (xml.FindElem("Version"))
		{	
			xml.IntoElem();

			xml.ResetMainPos();
			if (xml.FindElem("BF518"))
			{
				std::string strTmp = xml.GetData();
			}
			xml.ResetMainPos();
			if (xml.FindElem("DSP"))
			{
				std::string strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				try
				{
					BF533Base.Version = boost::lexical_cast<unsigned int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();
				}
			}
			xml.OutOfElem();
		}
		xml.ResetMainPos();
		if (xml.FindElem("PDZ_Type"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.Type = boost::lexical_cast<unsigned int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("PDZ_Storgtime"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				timeOutSaveHis_ = (boost::lexical_cast<unsigned int>(strTmp)*60);
				timeOutCallYMData_ = timeOutSaveHis_;
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("BoardType"))
		{
			std::string strTmp = xml.GetData();

			BF533Base.BoardType = TransStringToInt(strTmp);
		}
	}
	xml.OutOfElem();
	return 0;
}

int CBF533::Read_DevSanyaoPara(void)
{
	FileSystem::CMarkup xml;
	if (!xml.Load(BFDevSanyaoPara))
	{
		//PrintMessage("Load DevSanYaoPara.xml出错");
		//Creat_DevSanyaoPara();
		//xml.Load(BFDevSanyaoPara);
		return -1;
	}

	//CProtocol::LoadXmlCfg(xml);

	xml.ResetMainPos();
	if (xml.FindElem("PDZ_SanyaoPara_Set"))
	{
		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem("YkSum"))
		{
			std::string strTmp = xml.GetData();
		}
		xml.ResetMainPos();
		if (xml.FindElem("YxSum"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.SYXNUM = boost::lexical_cast<unsigned int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}

		}
		xml.ResetMainPos();
		if (xml.FindElem("ChannelSum"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.ChannelSum = boost::lexical_cast<unsigned int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}

		}
		xml.ResetMainPos();
		if (xml.FindElem("LineSum"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.LineNum = boost::lexical_cast<unsigned int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}

		}
		xml.ResetMainPos();
		if (xml.FindElem("YKCloseTime"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.YKHZTime = boost::lexical_cast<unsigned int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("YKOpenTime"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.YKTZTime = boost::lexical_cast<unsigned int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("YXLvBoTime"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.YXLvBoTime = boost::lexical_cast<unsigned int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("AutoRstYxTime"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.AotuRstYxTime = boost::lexical_cast<unsigned int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("AutoRstYxFlag"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.AotuRstYxFlag = boost::lexical_cast<unsigned short>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("HuoHua_AutoFlag"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.HuoHua_AutoFlag = boost::lexical_cast<unsigned short>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("HuoHua_Day"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.HuoHua_Day = (boost::lexical_cast<unsigned short>(strTmp)) * 24;
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("AutoRstProtectTime"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.Pro_Rst_Time = boost::lexical_cast<unsigned short>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("AutoRstProtectFlag"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.Flag_Pro_Rst = boost::lexical_cast<unsigned short>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}
		xml.ResetMainPos();
		if (xml.FindElem("I_Rated"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.I_Rated = boost::lexical_cast<unsigned short>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem("U_SwitchFlag"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.U_SwitchFlag = boost::lexical_cast<unsigned short>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}

		xml.ResetMainPos();//Modfiy by Zhangzhihua 20111220
		if (xml.FindElem("PT_L_Arlam"))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				BF533Base.PT_L_Arlam = boost::lexical_cast<unsigned short>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}

		xml.OutOfElem();
	}
	return 0;
}

int CBF533::Read_ChanneltypePara(void)
{
	int sum;
	FileSystem::CMarkup xml;
	if (!xml.Load(BFChanneltypePara))
	{
		//PrintMessage("Load DevSanYaoPara.xml出错");
		//Creat_ChanneltypePara();
		//xml.Load(BFChanneltypePara);
		return -1;
	}

	//CProtocol::LoadXmlCfg(xml);

	xml.ResetMainPos();
	if (xml.FindElem("PDZ_Channel_Type"))
	{
		xml.IntoElem();
		xml.ResetMainPos();
		if (xml.FindElem("ChannelSum"))
		{
			std::string strTmp = xml.GetAttrib("Sum");
			boost::algorithm::trim(strTmp);
			try
			{
				sum = boost::lexical_cast<int>(strTmp);
				BF533Base.ChannelSum = sum;
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}

			xml.IntoElem();
			xml.ResetMainPos();
			for(int i = 0;i < sum;i ++)
			{
				xml.FindElem();
				{
					std::string strTmp = xml.GetData();
					boost::algorithm::trim(strTmp);
					try
					{
						YcChannel[i].Type = boost::lexical_cast<unsigned short>(strTmp);
					}
					catch(boost::bad_lexical_cast& e)
					{
						std::ostringstream ostr;
						ostr<<e.what();
					}
				}
			}
			xml.OutOfElem();
		}
		xml.OutOfElem();
	}
	return 0;
}

int CBF533::Read_ProtectValPara(void)
{
	int sum;
	FileSystem::CMarkup xml;
	if (!xml.Load(BFProtectValPara))
	{
		//PrintMessage("读文件Load BFProtectValPara.xml出错");
		//Creat_ProtectValPara();
		//xml.Load(BFProtectValPara);
		return -1;
	}

	//CProtocol::LoadXmlCfg(xml); 

	xml.ResetMainPos();
	if (xml.FindElem("ProVal_Set"))
	{
		xml.IntoElem();
		xml.ResetMainPos();
		if (xml.FindElem("LineSum"))
		{
			std::string strTmp = xml.GetAttrib("Sum");
			boost::algorithm::trim(strTmp);
			try
			{
				sum = boost::lexical_cast<int>(strTmp);
				BF533Base.LineNum = sum;
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}

			xml.IntoElem();//进入<Line_Set LineNo="x">
			xml.ResetMainPos();

			for( int i = 0; i < sum; i ++)
			{
				xml.FindElem();
				xml.IntoElem();
				xml.ResetMainPos();
				{
					if (xml.FindElem("LowVol_Val"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].Low_Lock = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}

					}
					xml.ResetMainPos();
					if (xml.FindElem("OL_P_Val"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OverLoadValue = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OL_Ck_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OverLoadTime = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OL_TZ_Delay"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OverLoadTZWait = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OF_I_P_Val"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I_PROValue = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OF_I_Ck_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I_PROTime = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OF_I_TZ_Delay"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I_PROTZWait = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("I0_I_P_val"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_PROValue = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("I0_I_Ck_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_PROTime = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					if (xml.FindElem("I0_I_TZ_Delay"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_PROTZWait = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("I0_II_P_val"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_II_PROValue = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("I0_II_Ck_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_II_PROTime = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					if (xml.FindElem("I0_II_TZ_Delay"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_II_PROTZWait = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("ReClose_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].Reclose_PROTime = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					//xml.ResetMainPos();
					//if (xml.FindElem("U0_P_val"))
					//{
					//	std::string strTmp = xml.GetData();
					//	boost::algorithm::trim(strTmp);
					//	try
					//	{
					//		ProVal[i].U0_PROValue = boost::lexical_cast<unsigned int>(strTmp);
					//	}
					//	catch(boost::bad_lexical_cast& e)
					//	{
					//		std::ostringstream ostr;
					//		ostr<<e.what();
					//	}
					//}
					//xml.ResetMainPos();
					//if (xml.FindElem("U0_P_T"))
					//{
					//	std::string strTmp = xml.GetData();
					//	boost::algorithm::trim(strTmp);
					//	try
					//	{
					//		ProVal[i].U0_PROTime = boost::lexical_cast<unsigned int>(strTmp);
					//	}
					//	catch(boost::bad_lexical_cast& e)
					//	{
					//		std::ostringstream ostr;
					//		ostr<<e.what();
					//	}
					//}


					xml.ResetMainPos();
					if (xml.FindElem("Accel_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].Accel_T = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OverU_P_Val"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OverU_P_Val = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OverU_Ck_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OverU_Ck_T = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}

					//xml.ResetMainPos();
					//if (xml.FindElem("NetFA_Ck_T"))
					//{
					//	std::string strTmp = xml.GetData();
					//	boost::algorithm::trim(strTmp);
					//	try
					//	{
					//		ProVal[i].FACheckTime = boost::lexical_cast<unsigned int>(strTmp);
					//	}
					//	catch(boost::bad_lexical_cast& e)
					//	{
					//		std::ostringstream ostr;
					//		ostr<<e.what();
					//	}
					//}
					//xml.ResetMainPos();
					//if (xml.FindElem("NetFA_TZ_Delay"))
					//{
					//	std::string strTmp = xml.GetData();
					//	boost::algorithm::trim(strTmp);
					//	try
					//	{
					//		ProVal[i].FAOpenDalayTime = boost::lexical_cast<unsigned int>(strTmp);
					//	}
					//	catch(boost::bad_lexical_cast& e)
					//	{
					//		std::ostringstream ostr;
					//		ostr<<e.what();
					//	}
					//}
					xml.ResetMainPos();
					if (xml.FindElem("NetFA_PS_Addr"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].FAPowerSideAddr = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("NetFA_LS_Addr"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].FALoadSideAddr = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("FA_Lose_Sum"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].FAOverLoadLoseVolSum = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("FA_Rst_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].FAReturnToZeroTime = boost::lexical_cast<unsigned int>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("LowVol_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].Low_PRO = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("ReClose_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].Reclose_PRO = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OL_TZ_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OverLoad = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OL_Alarm_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OL_Alarm_F = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OF_I_TZ_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I_PRO = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OF_I_Alarm_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OF_Alarm_F = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("I0_I_TZ_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_PRO = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("I0_I_Alarm_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_Alarm_F = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("I0_II_TZ_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_II_PRO = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("I0_II_Alarm_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].I0_II_Alarm_F = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("U0_P_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].U0_PRO = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("NetFA_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].FAFlag = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("Accel_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].Accel_F = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("OverU_Alarm_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OverU_Alarm_F = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("FA_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].Local_FAFlag = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}

					xml.ResetMainPos();
					if (xml.FindElem("OverU_TZ_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OverU_PRO_F = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}

					xml.ResetMainPos();
					if (xml.FindElem("OverU_TZ_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].OverU_PROTZWait = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}

					xml.ResetMainPos();
					if (xml.FindElem("OF_II_TZ_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].II_PRO = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}

					xml.ResetMainPos();
					if (xml.FindElem("OF_II_Alarm_F"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].II_Alarm_F = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}

					xml.ResetMainPos();
					if (xml.FindElem("OF_II_P_Val"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].II_PROValue = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}

					xml.ResetMainPos();
					if (xml.FindElem("OF_II_Ck_T"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].II_PROTime = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}

					xml.ResetMainPos();
					if (xml.FindElem("OF_II_TZ_Delay"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							ProVal[i].II_PROTZWait = boost::lexical_cast<unsigned short>(strTmp);
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}

				}
				xml.OutOfElem();
			}
			xml.OutOfElem();
		}
		xml.OutOfElem();
	}
	return 0;
}

int CBF533::Read_LinePara(void)
{
	int sum;
	FileSystem::CMarkup xml;
	if (!xml.Load(BFLinePara))
	{
		//PrintMessage("Load BFLinePara.xml出错");
		//Creat_LinePara();
		//xml.Load(BFLinePara);
		return -1;
	}

	//CProtocol::LoadXmlCfg(xml); 

	xml.ResetMainPos();
	if (xml.FindElem("PDZ_LinePara_Set"))
	{
		xml.IntoElem();//进入<LineSum Sum="3">
		xml.ResetMainPos();
		if (xml.FindElem("LineSum"))
		{
			std::string strTmp = xml.GetAttrib("Sum");
			boost::algorithm::trim(strTmp);
			try
			{
				sum = boost::lexical_cast<int>(strTmp);
				BF533Base.LineNum = sum;
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
			//std::cout<<"所得值为"<< sum <<std::endl; 
			xml.IntoElem();//进入<Line_Set LineNo="x">
			xml.ResetMainPos();
			for(int i = 0;i < sum;i ++)
			{
				xml.FindElem();
				xml.IntoElem();//进入 <CTRate>1</CTRate>

				xml.ResetMainPos();
				if (xml.FindElem("CTRate"))
				{
					std::string strTmp = xml.GetData();
				}
				xml.ResetMainPos();
				if (xml.FindElem("PTRate"))
				{
					std::string strTmp = xml.GetData();
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0Rate"))
				{
					std::string strTmp = xml.GetData();
				}
				xml.ResetMainPos();
				if (xml.FindElem("U0Rate"))
				{
					std::string strTmp = xml.GetData();
				}
				xml.ResetMainPos();
				if (xml.FindElem("CTRatio"))
				{
					std::string strTmp = xml.GetData();
				}
				xml.ResetMainPos();
				if (xml.FindElem("PTRatio"))
				{
					std::string strTmp = xml.GetData();
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0Ratio"))
				{
					std::string strTmp = xml.GetData();
				}
				xml.ResetMainPos();
				if (xml.FindElem("U0Ratio"))
				{
					std::string strTmp = xml.GetData();
				}

				xml.ResetMainPos();
				if (xml.FindElem("YXStart"))
				{
					std::string strTmp = xml.GetData();
					boost::algorithm::trim(strTmp);
					try
					{
						LinePara[i].YxStart = boost::lexical_cast<unsigned short>(strTmp); 
					}
					catch(boost::bad_lexical_cast& e)
					{
						std::ostringstream ostr;
						ostr<<e.what();
					}
				}

				xml.ResetMainPos();//Modfiy by Zhangzhihua 20111220
				if (xml.FindElem("IO_Flage"))
				{
					std::string strTmp = xml.GetData();
					boost::algorithm::trim(strTmp);
					try
					{
						LinePara[i].IO_Flage = boost::lexical_cast<unsigned short>(strTmp); 
					}
					catch(boost::bad_lexical_cast& e)
					{
						std::ostringstream ostr;
						ostr<<e.what();
					}
				}

				xml.ResetMainPos();
				if (xml.FindElem("YKNO"))
				{
					std::string strTmp = xml.GetData();
					boost::algorithm::trim(strTmp);
					try
					{
						LinePara[i].YkNo = boost::lexical_cast<unsigned short>(strTmp); 
					}
					catch(boost::bad_lexical_cast& e)
					{
						std::ostringstream ostr;
						ostr<<e.what();
					}
				}


				xml.ResetMainPos();
				if (xml.FindElem("PTChannelNo"))
				{
					xml.IntoElem();//进入PTChannelNo
					xml.ResetMainPos();
					if (xml.FindElem("U1"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].Vol_Chan[0] = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Vol_Chan[0]是"<<LinePara[i]Vol_Chan[0]<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("U2"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].Vol_Chan[1] = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Vol_Chan[0]是"<<LinePara[i]Vol_Chan[1]<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("U3"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].Vol_Chan[2] = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Vol_Chan[0]是"<<LinePara[i]Vol_Chan[2]<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("U0"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].U0_Chan = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Vol_Chan[0]是"<<LinePara[i]U0_Chan<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.OutOfElem(); //退出PTChannelNo
				}

				xml.ResetMainPos();
				if (xml.FindElem("CTChannelNo"))
				{
					xml.IntoElem();//进入CTChannelNo
					xml.ResetMainPos();
					if (xml.FindElem("CI1"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].Cur_Chan[0] = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Cur_Chan[0]是"<<LinePara[i]Cur_Chan[0]<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("CI2"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].Cur_Chan[1] = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Cur_Chan[0]是"<<LinePara[i]Cur_Chan[1]<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("CI3"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].Cur_Chan[2] = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Cur_Chan[0]是"<<LinePara[i]Cur_Chan[2]<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("BI1"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].BCur_Chan[0] = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Cur_Chan[0]是"<<LinePara[i]Cur_Chan[0]<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("BI2"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].BCur_Chan[1] = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Cur_Chan[0]是"<<LinePara[i]Cur_Chan[1]<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("BI3"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].BCur_Chan[2] = boost::lexical_cast<unsigned short>(strTmp);
							//				std::cout<<"解析出LinePara[i].Cur_Chan[0]是"<<LinePara[i]Cur_Chan[2]<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.ResetMainPos();
					if (xml.FindElem("I0"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							LinePara[i].I0_Chan = boost::lexical_cast<unsigned short>(strTmp);
							//std::cout<<"解析出LinePara[i].Vol_Chan[0]是"<<LinePara[i].I0_Chan<<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
					}
					xml.OutOfElem(); //退出CTChannelNo
				}

				xml.OutOfElem(); //退出<CTRate>1</CTRate>
			}

			xml.OutOfElem();//退出<Line_Set LineNo="x">
		}
		xml.OutOfElem();//退出<LineSum Sum="3">
	}
	return 0;
}


int CBF533::Write_BasePara(void)
{
	//PrintMessage("开始写文件BasePara... ...");
	//int DSPVer = BF533HBase.Version;//读取DSP版本号
	int EQUType = BF533HBase.Type;//读取装置类型
	FileSystem::CMarkup xml;
	//if (!xml.Load(BFBasePara))
	//{
	//	//PrintMessage("Load BFBasePara出错");
	//	Creat_BasePara();
	//	xml.Load(BFBasePara);
	//	//		return -1;
	//}
	//CProtocol::LoadXmlCfg(xml); 
	Creat_BasePara();
	xml.Load(BFBasePara);

	xml.ResetMainPos();
	if (xml.FindElem("PDZ_BasePara_Set"))
	{
		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem("Version"))
		{	
			xml.IntoElem();
			xml.ResetMainPos();

			if (xml.FindElem("DSP"))
			{
				xml.SetData(DSPVer);
			}

			xml.ResetMainPos();
			if (xml.FindElem("BF518"))
			{
				xml.SetData(BF518Ver);
			}

			xml.OutOfElem();
		}
		xml.ResetMainPos();
		if (xml.FindElem("PDZ_Type"))
		{
			xml.SetData(EQUType);
		}
		xml.ResetMainPos();
		if (xml.FindElem("BoardType"))
		{		
			xml.SetData(TransIntToHexString(BF533HBase.BoardType));
		}
	}
	xml.OutOfElem();
	xml.Save(BFBasePara);
	return 0;
}

int CBF533::Write_DevSanyaoPara(void)
{
	//PrintMessage("开始写文件DevSanyaoPara... ...");
	int ChannelSum = BF533HBase.ChannelSum;
	int LineSum    = BF533HBase.LineNum;
	int YKCloseTime= BF533HBase.YKHZTime;
	int YKOpenTime = BF533HBase.YKTZTime;
	int YXLvBoTime = BF533HBase.YXLvBoTime;
	int HuoHua_AutoFlag=BF533HBase.HuoHua_AutoFlag;
	int HuoHua_Day = BF533HBase.HuoHua_Day;
	int AutoRstProtectTime= BF533HBase.Pro_Rst_Time;
	int AutoRstProtectFlag= BF533HBase.Flag_Pro_Rst;
	int I_Rated = BF533HBase.I_Rated;

	FileSystem::CMarkup xml;
	//if (!xml.Load(BFDevSanyaoPara))
	//{
	//	//PrintMessage("Load DevSanYaoPara.xml出错");
	//	Creat_DevSanyaoPara();
	//	xml.Load(BFDevSanyaoPara);
	//	//		return -1;
	//}

	//CProtocol::LoadXmlCfg(xml);

	Creat_DevSanyaoPara();
	xml.Load(BFDevSanyaoPara);

	xml.ResetMainPos();
	if (xml.FindElem("PDZ_SanyaoPara_Set"))
	{
		xml.IntoElem();

		xml.ResetMainPos();

		if (xml.FindElem("ChannelSum"))
		{
			xml.SetData(ChannelSum);
		}
		xml.ResetMainPos();
		if (xml.FindElem("LineSum"))
		{
			xml.SetData(LineSum);
		}
		xml.ResetMainPos();
		if (xml.FindElem("YKCloseTime"))
		{
			xml.SetData(YKCloseTime);
		}
		xml.ResetMainPos();
		if (xml.FindElem("YKOpenTime"))
		{
			xml.SetData(YKOpenTime);
		}
		xml.ResetMainPos();
		if (xml.FindElem("YXLvBoTime"))
		{
			xml.SetData(YXLvBoTime);
		}
		xml.ResetMainPos();
		if (xml.FindElem("HuoHua_AutoFlag"))
		{
			xml.SetData(HuoHua_AutoFlag);
		}
		xml.ResetMainPos();
		if (xml.FindElem("HuoHua_Day"))
		{
			xml.SetData(HuoHua_Day);
		}
		xml.ResetMainPos();
		if (xml.FindElem("AutoRstProtectTime"))
		{
			xml.SetData(AutoRstProtectTime);
		}
		xml.ResetMainPos();
		if (xml.FindElem("AutoRstProtectFlag"))
		{
			xml.SetData(AutoRstProtectFlag);
		}
		xml.ResetMainPos();
		if (xml.FindElem("I_Rated"))
		{
			xml.SetData(I_Rated);
		}
		xml.ResetMainPos();
		if (xml.FindElem("U_SwitchFlag"))
		{
			xml.SetData(BF533HBase.U_SwitchFlag);
		}
		xml.ResetMainPos();//Modfiy by Zhangzhihua 20111220
		if (xml.FindElem("PT_L_Arlam"))
		{
			xml.SetData(BF533HBase.PT_L_Arlam);
		}

		xml.OutOfElem();
	}
	xml.Save(BFDevSanyaoPara);
	return 0;
}

int CBF533::Write_ChanneltypePara(void)
{
	//PrintMessage("开始写文件BFChanneltypePara... ..");
	std::string str;
	int ChannelSum = BF533HBase.ChannelSum;
	int sum = 0;
	char Data[5];
	int Index = 0;
	int Location = 6;
	int count = 0;

	FileSystem::CMarkup xml;
	//if (!xml.Load(BFChanneltypePara))
	//{
	//	//PrintMessage("Load ChanneltypePara.xml出错");
	//	Creat_ChanneltypePara();
	//	xml.Load(BFChanneltypePara);
	//	//		return -1;
	//}

	//CProtocol::LoadXmlCfg(xml);

	Creat_ChanneltypePara();
	xml.Load(BFChanneltypePara);

	xml.ResetMainPos();
	if (xml.FindElem("PDZ_Channel_Type"))
	{
		xml.IntoElem();
		xml.ResetMainPos();
		if (xml.FindElem("ChannelSum"))
		{
			{
				std::string strTmp = xml.GetAttrib("Sum");
				boost::algorithm::trim(strTmp);
				try
				{
					sum = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();
				}
				xml.SetAttrib("Sum",ChannelSum,0);
			}

			count = sum > ChannelSum ? sum : ChannelSum;

			xml.ResetMainPos();

			for(int i = 0;i < count;i ++)
			{
				sprintf(Data,"%d",Index);				 
				str = Data;
				if(Index < 10)
				{
					xml.FindChildElem(("CH00"+str));
				}
				else if((Index < 100)&&(Index >= 10))
				{
					xml.FindChildElem(("CH0"+str));
				}
				else if(Index >= 100)
				{
					xml.FindChildElem(("CH"+str));
				}
				xml.SetChildData(HYcChannel[i].Type);
				Index ++;
			}
			//			std::cout<<"参数分别为：ChannelSum："<<ChannelSum<<",sum是："<<sum<<std::endl; 
			if(ChannelSum > sum)//如果比上次多，则添加节点
			{
				for(int i = 0; i < ChannelSum - sum;i ++)
				{
					sprintf(Data,"%d",Index);				 
					str = Data;
					if(Index < 10)
					{
						xml.AddChildElem(("CH00"+str),HYcChannel[Index].Type);
					}
					else if((Index < 100)&&(Index >= 10))
					{
						xml.AddChildElem(("CH0"+str),HYcChannel[Index].Type);
					}
					else if(Index >= 100)
					{
						xml.AddChildElem(("CH"+str),HYcChannel[Index].Type);
					}
					Index ++;
				}
			}
			else//如果没上次多，删除多余节点
			{
				std::string str;
				char Data[10];
				sprintf(Data,"%d",Index);				 
				str = Data;
				for(int i = 0; i < sum - ChannelSum;i ++)
				{  
					sprintf(Data,"%d",Index);				 
					str = Data;
					xml.ResetMainPos();

					if(Index < 10)
					{
						xml.FindChildElem("CH00"+str);
					}
					else if((Index <100)&&(Index >= 10))
					{
						xml.FindChildElem("CH0"+str);
					} 
					Index ++;
					xml.RemoveChildElem();
				}
			}

			xml.OutOfElem();
		}
		xml.OutOfElem();
	}
	xml.Save(BFChanneltypePara);
	return 0;
}

int CBF533::Write_LinePara(void)//将通道组合关系
{
	//PrintMessage("开始写文件BFLinePara... ...");

	int sum;
	int count;
	int Location = 6;
	int Index = 0;
	int LineSum = BF533HBase.LineNum;//读取召唤上来的线路总数

	FileSystem::CMarkup xml;
	//if (!xml.Load(BFLinePara))
	//{
	//	//PrintMessage("Load BFLinePara.xml出错");
	//	Creat_LinePara();
	//	xml.Load(BFLinePara);
	//	//		return -1;
	//}
	//CProtocol::LoadXmlCfg(xml); 

	Creat_LinePara();
	xml.Load(BFLinePara);

	xml.ResetMainPos();
	if (xml.FindElem("PDZ_LinePara_Set"))
	{
		xml.IntoElem();//进入<LineSum Sum="3">
		xml.ResetMainPos();
		if (xml.FindElem("LineSum"))
		{ 
			std::string strTmp = xml.GetAttrib("Sum");
			boost::algorithm::trim(strTmp);
			try
			{
				sum = boost::lexical_cast<int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
			xml.SetAttrib("Sum",LineSum,0);

			count = sum > LineSum ? sum : LineSum;

			xml.IntoElem();//进入<Line_Set LineNo="x">
			xml.ResetMainPos();
			for(int i = 0;i < count;i ++)
			{
				if(xml.FindElem("Line_Set"))
				{
					xml.SetAttrib("LineNo",i,0);//设置属性

					xml.IntoElem();//进入 <CTRate>1</CTRate>
					xml.ResetMainPos();
					{
						if(xml.FindElem("YXStart"))
						{
							xml.SetData(HLinePara[i].YxStart);
						}
						xml.ResetMainPos();

						if(xml.FindElem("YKNO"))
						{
							xml.SetData(HLinePara[i].YkNo);
						}
						xml.ResetMainPos();
						if(xml.FindElem("IO_Flage"))//Modfiy by Zhangzhihua 20111220
						{
							xml.SetData(HLinePara[i].IO_Flage);
						}
						xml.ResetMainPos();

						if (xml.FindElem("PTChannelNo"))
						{
							xml.IntoElem();//进入PTChannelNo
							xml.ResetMainPos();
							{
								if (xml.FindElem("U1"))
								{
									xml.SetData(HLinePara[i].Vol_Chan[0]);
								}//Ua
								xml.ResetMainPos();
								if (xml.FindElem("U2"))
								{
									xml.SetData(HLinePara[i].Vol_Chan[1]);
								}
								xml.ResetMainPos();
								if (xml.FindElem("U3"))
								{
									xml.SetData(HLinePara[i].Vol_Chan[2]);
								}
								xml.ResetMainPos();
								if (xml.FindElem("U0"))
								{
									xml.SetData(HLinePara[i].U0_Chan);
								}
							}
							xml.OutOfElem();//退出PTChannelNo 
						}//PTChannelNo
						xml.ResetMainPos();
						if (xml.FindElem("CTChannelNo"))
						{
							xml.IntoElem();//进入CTChannelNo
							xml.ResetMainPos();
							{
								if (xml.FindElem("CI1"))
								{
									xml.SetData(HLinePara[i].Cur_Chan[0]);
								}
								xml.ResetMainPos();
								if (xml.FindElem("CI2"))
								{
									xml.SetData(HLinePara[i].Cur_Chan[1]);
								}
								xml.ResetMainPos();
								if (xml.FindElem("CI3"))
								{
									xml.SetData(HLinePara[i].Cur_Chan[2]);
								}
								if (xml.FindElem("BI1"))
								{
									xml.SetData(HLinePara[i].BCur_Chan[0]);
								}
								xml.ResetMainPos();
								if (xml.FindElem("BI2"))
								{
									xml.SetData(HLinePara[i].BCur_Chan[1]);
								}
								xml.ResetMainPos();
								if (xml.FindElem("BI3"))
								{
									xml.SetData(HLinePara[i].BCur_Chan[2]);
								}
								xml.ResetMainPos();
								if (xml.FindElem("I0"))
								{
									xml.SetData(HLinePara[i].I0_Chan);
								}
							}
							xml.OutOfElem();//退出CTChannelNo 
						}//CTChannelNo
					}
					xml.OutOfElem();//退出<CTRate>1</CTRate>
				}//Line_Set
				Index ++;
			}//退出for循环

			xml.OutOfElem();//退出<Line_Set LineNo="x">

			if(LineSum > sum)//需要增加新的节点
			{
				for(int i = 0;i < LineSum - sum;i ++)
				{
					xml.AddChildElem( "Line_Set" );
					xml.SetChildAttrib( "LineNo",Index);
					xml.IntoElem();
					xml.AddChildElem( "CTRate", "5" );
					xml.AddChildElem( "PTRate", "100" );
					xml.AddChildElem( "I0CTRate", "5" );
					xml.AddChildElem( "U0PTRate", "100" );
					xml.AddChildElem( "CTRadio", "120" );
					xml.AddChildElem( "PTRadio", "1100" );
					xml.AddChildElem( "I0CTRadio", "50" );
					xml.AddChildElem( "U0PTRadio", "100" );
					xml.AddChildElem( "YXStart",  HLinePara[Index].YxStart );
					xml.AddChildElem( "YKNO",  HLinePara[Index].YkNo );
					xml.AddChildElem( "IO_Flage",  HLinePara[Index].IO_Flage);//Modfiy by Zhangzhihua 20111220
					xml.AddChildElem( "PTChannelNo" ); 
					xml.IntoElem();
					xml.AddChildElem( "Ua", HLinePara[Index].Vol_Chan[0]);
					xml.AddChildElem( "Ub", HLinePara[Index].Vol_Chan[1] );
					xml.AddChildElem( "Uc", HLinePara[Index].Vol_Chan[2] );
					xml.AddChildElem( "U0", HLinePara[Index].U0_Chan );
					xml.OutOfElem();
					xml.AddChildElem( "CTChannelNo" ); 
					xml.IntoElem();
					xml.AddChildElem( "CI1", HLinePara[i].Cur_Chan[0] );
					xml.AddChildElem( "CI2", HLinePara[i].Cur_Chan[1] );
					xml.AddChildElem( "CI3", HLinePara[i].Cur_Chan[2]);
					xml.AddChildElem( "BI1", HLinePara[i].BCur_Chan[0] );
					xml.AddChildElem( "BI2", HLinePara[i].BCur_Chan[1] );
					xml.AddChildElem( "BI3", HLinePara[i].BCur_Chan[2]);
					xml.AddChildElem( "I0", HLinePara[Index].I0_Chan );
					xml.OutOfElem();   
					xml.OutOfElem(); 
					Index ++;
				}//for循环退出
			}//LineSum > sum
			else
			{
				for(int i = 0;i < sum - LineSum;i ++)
				{
					xml.FindChildElem();
					xml.RemoveChildElem();
				}
			}//LineSum < sum

		}//LineSum
		xml.OutOfElem();//退出<LineSum Sum="3">

	}//PDZ_LinePara_Set

	xml.Save(BFLinePara);
	return 0;
}

int CBF533::Write_InterfacePara(void)
{
	int LanNo;
	char IP[24];
	char MASK[24];

	FileSystem::CMarkup xml;
	if (!xml.Load(BFInterfacePara))
	{
		//PrintMessage("写文件Load BFInterfacePara.xml出错");
		Creat_InterfacePara();
		xml.Load(BFInterfacePara);
	}

	xml.ResetMainPos();

	if (xml.FindElem("PDZ_InterfacePara_Set"))
	{
		xml.IntoElem();//进入<LineSum Sum="10">
		xml.ResetMainPos();
		for (int i = 0;i < 2;i ++)
		{
			if (xml.FindElem("PDZ_Lan_Set"))
			{
				std::string strTmp = xml.GetAttrib("LanNo");
				boost::algorithm::trim(strTmp);
				try
				{
					LanNo = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();
				}
			}
			if (LanNo == i + 1)
			{
				if (LanNo == 1)
				{
					//std::cout<<"开始获取eth1的IP地址... .."<<std::endl;
					getip("eth1",IP,MASK) ;
					// std::cout<<"返回的IP地址为："<<IP<<",MASK 是："<<MASK<<std::endl;
				} 
				else
				{
					//std::cout<<"开始获取eth1的IP地址... .."<<std::endl;
					getip("eth2",IP,MASK) ;
					// std::cout<<"返回的IP地址为："<<IP<<",MASK 是："<<MASK<<std::endl;
				}
				xml.IntoElem();//进入<LineSum Sum="10">
				xml.ResetMainPos();
				if (xml.FindElem("IP"))
				{
					xml.SetData(IP);
				}
				if (xml.FindElem("MASK"))
				{
					xml.SetData(MASK);
				}
				xml.OutOfElem();
			}
		}
		xml.OutOfElem();
	}
	xml.Save(BFInterfacePara);

	return 0;

}

int CBF533::Write_ProtectValPara(void)
{
	PrintMessage("开始写文件BFProtectValPara... ...");
	int sum;
	int Index = 0;
	int count;
	int LineSum = BF533HBase.LineNum;//读取召唤上来的线路总数
	//std::cout<<"Write_ProtectValPara 线路号为："<<LineSum<<std::endl;

	FileSystem::CMarkup xml;
	//if (!xml.Load(BFProtectValPara))
	//{
	//	PrintMessage("写文件Load BFProtectValPara.xml出错");
	//	Creat_ProtectValPara();
	//	xml.Load(BFProtectValPara);
	//	//		return -1;
	//}

	Creat_ProtectValPara();
	xml.Load(BFProtectValPara);

	//CProtocol::LoadXmlCfg(xml); 

	xml.ResetMainPos();

	if (xml.FindElem("ProVal_Set"))
	{
		xml.IntoElem();//进入<LineSum Sum="10">
		xml.ResetMainPos();
		if (xml.FindElem("LineSum"))
		{
			std::string strTmp = xml.GetAttrib("Sum");
			boost::algorithm::trim(strTmp);
			try
			{
				sum = boost::lexical_cast<int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
			xml.SetAttrib("Sum",LineSum);

			count = sum > LineSum ? sum : LineSum;

			xml.IntoElem();//进入<Line_Set LineNo="x">
			xml.ResetMainPos();
			for( int i = 0; i < count; i ++)
			{
				//		  std::cout<<"Index = :"<<Index<<std::endl; 
				if(xml.FindElem("Pro_Set"))
				{
					xml.SetAttrib("Line",i);
				}//Protect_Set
				xml.IntoElem();//进入具体保护定值设定
				xml.ResetMainPos();
				if (xml.FindElem("LowVol_Val"))
				{
					xml.SetData(HProVal[i].Low_Lock); 
				}
				else
				{
					xml.AddElem("LowVol_Val",HProVal[i].Low_Lock);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OL_P_Val"))
				{
					xml.SetData(HProVal[i].OverLoadValue); 
				}
				else
				{
					xml.AddElem("OL_P_Val",HProVal[i].OverLoadValue);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OL_Ck_T"))
				{
					xml.SetData(HProVal[i].OverLoadTime); 
				}
				else
				{
					xml.AddElem("OL_Ck_T",HProVal[i].OverLoadTime);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OL_TZ_Delay"))
				{
					xml.SetData(HProVal[i].OverLoadTZWait); 
				}
				else
				{
					xml.AddElem("OL_TZ_Delay",HProVal[i].OverLoadTZWait);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_I_P_Val"))
				{
					xml.SetData(HProVal[i].I_PROValue); 
				}
				else
				{
					xml.AddElem("OF_I_P_Val",HProVal[i].I_PROValue);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_I_Ck_T"))
				{
					xml.SetData(HProVal[i].I_PROTime); 
				}
				else
				{
					xml.AddElem("OF_I_Ck_T",HProVal[i].I_PROTime);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_I_TZ_Delay"))
				{
					xml.SetData(HProVal[i].I_PROTZWait); 
				}
				else
				{
					xml.AddElem("OF_I_TZ_Delay",HProVal[i].I_PROTZWait);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_I_P_val"))
				{
					xml.SetData(HProVal[i].I0_PROValue); 
				}
				else
				{
					xml.AddElem("I0_I_P_val",HProVal[i].I0_PROValue);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_I_Ck_T"))
				{
					xml.SetData(HProVal[i].I0_PROTime); 
				}
				else
				{
					xml.AddElem("I0_I_Ck_T",HProVal[i].I0_PROTime);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_I_TZ_Delay"))
				{
					xml.SetData(HProVal[i].I0_PROTZWait); 
				}
				else
				{
					xml.AddElem("I0_I_TZ_Delay",HProVal[i].I0_PROTZWait);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_II_P_val"))
				{
					xml.SetData(HProVal[i].I0_II_PROValue); 
				}
				else
				{
					xml.AddElem("I0_II_P_val",HProVal[i].I0_II_PROValue);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_II_Ck_T"))
				{
					xml.SetData(HProVal[i].I0_II_PROTime); 
				}
				else
				{
					xml.AddElem("I0_II_Ck_T",HProVal[i].I0_II_PROTime);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_II_TZ_Delay"))
				{
					xml.SetData(HProVal[i].I0_II_PROTZWait); 
				}
				else
				{
					xml.AddElem("I0_II_TZ_Delay",HProVal[i].I0_II_PROTZWait);
				}

				xml.ResetMainPos();
				if (xml.FindElem("ReClose_T"))
				{
					xml.SetData(HProVal[i].Reclose_PROTime); 
				}
				else
				{
					xml.AddElem("ReClose_T",HProVal[i].Reclose_PROTime);
				}

				//xml.ResetMainPos();
				//if (xml.FindElem("U0_P_val"))
				//{
				//	xml.SetData(HProVal[i].U0_PROValue); 
				//}
				//xml.ResetMainPos();
				//if (xml.FindElem("U0_P_T"))
				//{
				//	xml.SetData(HProVal[i].U0_PROTime); 
				//}

				xml.ResetMainPos();
				if (xml.FindElem("Accel_T"))
				{
					xml.SetData(HProVal[i].Accel_T); 
				}
				else
				{
					xml.AddElem("Accel_T",HProVal[i].Accel_T);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OverU_P_Val"))
				{
					xml.SetData(HProVal[i].OverU_P_Val); 
				}
				else
				{
					xml.AddElem("OverU_P_Val",HProVal[i].OverU_P_Val);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OverU_Ck_T"))
				{
					xml.SetData(HProVal[i].OverU_Ck_T); 
				}
				else
				{
					xml.AddElem("OverU_Ck_T",HProVal[i].OverU_Ck_T);
				}

				//xml.ResetMainPos();
				//if (xml.FindElem("NetFA_Ck_T"))
				//{
				//	xml.SetData(HProVal[i].FACheckTime); 
				//}

				//xml.ResetMainPos();
				//if (xml.FindElem("NetFA_TZ_Delay"))
				//{
				//	xml.SetData(HProVal[i].FAOpenDalayTime); 
				//}
				xml.ResetMainPos();
				if (xml.FindElem("NetFA_PS_Addr"))
				{
					xml.SetData(HProVal[i].FAPowerSideAddr); 
				}
				else
				{
					xml.AddElem("NetFA_PS_Addr",HProVal[i].FAPowerSideAddr);
				}

				xml.ResetMainPos();
				if (xml.FindElem("NetFA_LS_Addr"))
				{
					xml.SetData(HProVal[i].FALoadSideAddr); 
				}
				else
				{
					xml.AddElem("NetFA_LS_Addr",HProVal[i].FALoadSideAddr);
				}

				xml.ResetMainPos();
				if (xml.FindElem("FA_Lose_Sum"))
				{
					xml.SetData(HProVal[i].FAOverLoadLoseVolSum); 
				}
				else
				{
					xml.AddElem("FA_Lose_Sum",HProVal[i].FAOverLoadLoseVolSum);
				}

				xml.ResetMainPos();
				if (xml.FindElem("FA_Rst_T"))
				{
					xml.SetData(HProVal[i].FAReturnToZeroTime); 
				}
				else
				{
					xml.AddElem("FA_Rst_T",HProVal[i].FAReturnToZeroTime);
				}

				xml.ResetMainPos();
				if (xml.FindElem("LowVol_F"))
				{
					xml.SetData(HProVal[i].Low_PRO); 
				}
				else
				{
					xml.AddElem("LowVol_F",HProVal[i].Low_PRO);
				}

				xml.ResetMainPos();
				if (xml.FindElem("ReClose_F"))
				{
					xml.SetData(HProVal[i].Reclose_PRO); 
				}
				else
				{
					xml.AddElem("ReClose_F",HProVal[i].Reclose_PRO);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OL_TZ_F"))
				{
					xml.SetData(HProVal[i].OverLoad); 
				}
				else
				{
					xml.AddElem("OL_TZ_F",HProVal[i].OverLoad);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OL_Alarm_F"))
				{
					xml.SetData(HProVal[i].OL_Alarm_F); 
				}
				else
				{
					xml.AddElem("OL_Alarm_F",HProVal[i].OL_Alarm_F);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_I_TZ_F"))
				{
					xml.SetData(HProVal[i].I_PRO); 
				}
				else
				{
					xml.AddElem("OF_I_TZ_F",HProVal[i].I_PRO);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_I_Alarm_F"))
				{
					xml.SetData(HProVal[i].OF_Alarm_F); 
				}
				else
				{
					xml.AddElem("OF_I_Alarm_F",HProVal[i].OF_Alarm_F);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_I_TZ_F"))
				{
					xml.SetData(HProVal[i].I0_PRO); 
				}
				else
				{
					xml.AddElem("I0_I_TZ_F",HProVal[i].I0_PRO);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_I_Alarm_F"))
				{
					xml.SetData(HProVal[i].I0_Alarm_F); 
				}
				else
				{
					xml.AddElem("I0_I_Alarm_F",HProVal[i].I0_Alarm_F);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_II_TZ_F"))
				{
					xml.SetData(HProVal[i].I0_II_PRO); 
				}
				else
				{
					xml.AddElem("I0_II_TZ_F",HProVal[i].I0_II_PRO);
				}

				xml.ResetMainPos();
				if (xml.FindElem("I0_II_Alarm_F"))
				{
					xml.SetData(HProVal[i].I0_II_Alarm_F); 
				}
				else
				{
					xml.AddElem("I0_II_Alarm_F",HProVal[i].I0_II_Alarm_F);
				}

				//xml.ResetMainPos();
				//if (xml.FindElem("U0_P_F"))
				//{
				//	xml.SetData(HProVal[i].U0_PRO); 
				//}
				xml.ResetMainPos();
				if (xml.FindElem("NetFA_F"))
				{
					xml.SetData(HProVal[i].FAFlag); 
				}
				else
				{
					xml.AddElem("NetFA_F",HProVal[i].FAFlag);
				}

				xml.ResetMainPos();
				if (xml.FindElem("Accel_F"))
				{
					xml.SetData(HProVal[i].Accel_F); 
				}
				else
				{
					xml.AddElem("Accel_F",HProVal[i].Accel_F);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OverU_Alarm_F"))
				{
					xml.SetData(HProVal[i].OverU_Alarm_F); 
				}
				else
				{
					xml.AddElem("OverU_Alarm_F",HProVal[i].OverU_Alarm_F);
				}

				xml.ResetMainPos();
				if (xml.FindElem("FA_F"))
				{
					xml.SetData(HProVal[i].Local_FAFlag); 
				}
				else
				{
					xml.AddElem("FA_F",HProVal[i].Local_FAFlag);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OverU_TZ_F"))
				{
					xml.SetData(HProVal[i].OverU_PRO_F); 
				}
				else
				{
					xml.AddElem("OverU_TZ_F",HProVal[i].OverU_PRO_F);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OverU_TZ_T"))
				{
					xml.SetData(HProVal[i].OverU_PROTZWait); 
				}
				else
				{
					xml.AddElem("OverU_TZ_T",HProVal[i].OverU_PROTZWait);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_II_TZ_F"))
				{
					xml.SetData(HProVal[i].II_PRO); 
				}
				else
				{
					xml.AddElem("OF_II_TZ_F",HProVal[i].II_PRO);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_II_Alarm_F"))
				{
					xml.SetData(HProVal[i].II_Alarm_F); 
				}
				else
				{
					xml.AddElem("OF_II_Alarm_F",HProVal[i].II_Alarm_F);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_II_P_Val"))
				{
					xml.SetData(HProVal[i].II_PROValue); 
				}
				else
				{
					xml.AddElem("OF_II_P_Val",HProVal[i].II_PROValue);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_II_Ck_T"))
				{
					xml.SetData(HProVal[i].II_PROTime); 
				}
				else
				{
					xml.AddElem("OF_II_Ck_T",HProVal[i].II_PROTime);
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_II_TZ_Delay"))
				{
					xml.SetData(HProVal[i].II_PROTZWait); 
				}
				else
				{
					xml.AddElem("OF_II_TZ_Delay",HProVal[i].II_PROTZWait);
				}

				xml.OutOfElem();//退出具体保护定值设定
				Index ++;
			}//退出for循环
			xml.OutOfElem();//退出<Line_Set LineNo="x">
			//xml.ResetMainPos();
			if(LineSum > sum)//需要增加新的节点
			{
				for(int i = 0;i < LineSum - sum;i ++)
				{
					//std::cout<<"Index = :"<<Index<<std::endl; 
					xml.AddChildElem( "Pro_Set" );
					xml.SetChildAttrib( "Line",Index);
					xml.IntoElem();//进入具体值设定
					xml.AddChildElem( "LowVol_Val", HProVal[Index].Low_Lock );
					xml.AddChildElem( "OL_P_Val", HProVal[Index].OverLoadValue );
					xml.AddChildElem( "OL_Ck_T",HProVal[Index].OverLoadTime );
					xml.AddChildElem( "OL_TZ_Delay",HProVal[Index].OverLoadTZWait );
					xml.AddChildElem( "OF_I_P_Val", HProVal[Index].I_PROValue );
					xml.AddChildElem( "OF_II_P_Val",HProVal[Index].II_PROValue);
					xml.AddChildElem( "OF_I_Ck_T", HProVal[Index].I_PROTime );
					xml.AddChildElem( "OF_I_TZ_Delay", HProVal[Index].I_PROTZWait );
					xml.AddChildElem( "OF_II_Ck_T",HProVal[Index].II_PROTime);
					xml.AddChildElem( "OF_II_TZ_Delay",HProVal[Index].II_PROTZWait);
					xml.AddChildElem( "I0_I_P_val", HProVal[Index].I0_PROValue );
					xml.AddChildElem( "I0_I_Ck_T", HProVal[Index].I0_PROTime );  
					xml.AddChildElem( "I0_I_TZ_Delay", HProVal[Index].I0_PROTZWait );
					xml.AddChildElem( "I0_II_P_val", HProVal[Index].I0_II_PROValue );
					xml.AddChildElem( "I0_II_Ck_T", HProVal[Index].I0_II_PROTime );  
					xml.AddChildElem( "I0_II_TZ_Delay", HProVal[Index].I0_II_PROTZWait );
					xml.AddChildElem( "ReClose_T", HProVal[Index].Reclose_PROTime );
					xml.AddChildElem( "U0_P_val", HProVal[Index].U0_PROValue );
					xml.AddChildElem( "U0_P_T", HProVal[Index].U0_PROTime );
					xml.AddChildElem( "Accel_T", HProVal[Index].Accel_T );
					xml.AddChildElem( "OverU_P_Val", HProVal[Index].OverU_P_Val );
					xml.AddChildElem( "OverU_Ck_T", HProVal[Index].OverU_Ck_T );
					xml.AddChildElem( "OverU_TZ_T",HProVal[Index].OverU_PROTZWait);
					xml.AddChildElem( "NetFA_Ck_T", HProVal[Index].FACheckTime);
					xml.AddChildElem( "NetFA_TZ_Delay", HProVal[Index].FAOpenDalayTime);
					xml.AddChildElem( "NetFA_PS_Addr", "0" );
					xml.AddChildElem( "NetFA_LS_Addr", "0" );
					xml.AddChildElem( "FA_Lose_Sum", HProVal[Index].FAOverLoadLoseVolSum);
					xml.AddChildElem( "FA_Rst_T", HProVal[Index].FAReturnToZeroTime);
					xml.AddChildElem( "LowVol_F",HProVal[Index].Low_Lock);
					xml.AddChildElem( "ReClose_F", HProVal[Index].Reclose_PRO );
					xml.AddChildElem( "OL_TZ_F", HProVal[Index].OverLoad ); 
					xml.AddChildElem( "OL_Alarm_F", HProVal[Index].OL_Alarm_F );    
					xml.AddChildElem( "OF_I_TZ_F", HProVal[Index].I_PRO );
					xml.AddChildElem( "OF_I_Alarm_F", HProVal[Index].OF_Alarm_F );
					xml.AddChildElem( "OF_II_TZ_F",HProVal[Index].II_PRO);
					xml.AddChildElem( "OF_II_Alarm_F",HProVal[Index].II_Alarm_F);
					xml.AddChildElem( "I0_I_TZ_F", HProVal[Index].I0_PRO );
					xml.AddChildElem( "I0_I_Alarm_F", HProVal[Index].I0_Alarm_F );
					xml.AddChildElem( "I0_II_TZ_F", HProVal[Index].I0_II_PRO );
					xml.AddChildElem( "I0_II_Alarm_F", HProVal[Index].I0_II_Alarm_F );
					xml.AddChildElem( "U0_P_F", HProVal[Index].U0_PRO );
					xml.AddChildElem( "NetFA_F",HProVal[Index].FAFlag);
					xml.AddChildElem( "Accel_F", HProVal[Index].Accel_F );
					xml.AddChildElem( "OverU_Alarm_F",HProVal[Index].OverU_Alarm_F);
					xml.AddChildElem( "OverU_TZ_F",HProVal[Index].OverU_PRO_F);
					xml.AddChildElem( "FA_F",HProVal[Index].Local_FAFlag);

					xml.OutOfElem();//退出具体值设定
					Index ++;
				}
			}
			else//删除多余节点
			{
				for(int i = 0;i < sum - LineSum;i ++)
				{
					xml.FindChildElem();
					xml.RemoveChildElem();
				}
			}
		}//LineSum
		xml.OutOfElem();//退出<LineSum Sum="10">
	}//PDZ_ProtectVal_Set

	xml.Save(BFProtectValPara);
	return 0;
}



int CBF533::SaveExtremum(share_terminal_ptr terminalPtr)
{
	int linenum;
	int Index;
	int GetVal;
	int offset = 12;
	int UMinLimit = 50;  //计算极值时的门槛值  
	int IMinLimit = 50;
	int UMaxLimit = 50000;  //计算极值时的门槛值  500V
	int IMaxLimit = 150000; //150A
	int infonum = terminalPtr->getYcSum();
	//	std::cout<<"BF533获得遥测总数infonum为："<< infonum <<std::endl;
	std::stringstream ss2;

	std::string str2;

	int year,month,day,hour,min;

	using namespace boost::posix_time;
	boost::posix_time::time_duration td = (boost::posix_time::microsec_clock::local_time()).time_of_day();
	boost::gregorian::date::ymd_type ymd = (boost::posix_time::microsec_clock::local_time()).date().year_month_day();

	int Sec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) / (1000);
	int Msec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) % (1000);

	year = ymd.year;
	month = ymd.month;
	day = ymd.day;
	hour = td.hours();
	min = td.minutes();


	ss2 <<year<<"/"<<month<<"/"<<day<<"-"<<hour<<":"<<min<<":"<<Sec<<":"<<Msec;
	ss2 >> str2;

	linenum = infonum / offset;

	//  std::cout<<"现在时间是："<< str2 <<std::endl;
	//  std::cout<<"文件名为："<< str1 <<std::endl; 
	//  std::cout<<"遥测数量是："<<infonum<<"线路数量是："<<linenum<<std::endl; 

	for (int i = 0;i < linenum;i ++)
	{
		Index = 0;
		ExtYcVal[i].Ua  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Ub  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Uc  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Ia  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Ib  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Ic  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].U0  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].I0  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Fre = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Fac = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].AP  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].RP  = terminalPtr->getFinalYcVal(i*offset + (Index++));
	}
	FileSystem::CMarkup xml;
	if (!xml.Load(BFExtremum))
	{
		PrintMessage("Load 极值记录文件出错... .");
		Creat_Extremum(linenum, boost::posix_time::microsec_clock::local_time());
		xml.Load(BFExtremum);
		//	  return -1;
	}

	xml.ResetMainPos();
	if (xml.FindElem("HistoricalData"))
	{
		xml.IntoElem();//进入ExtremumRecordNode
		xml.ResetMainPos();
		if (xml.FindElem("ExtremumRecordNode"))
		{
			xml.IntoElem();//进入ExtremumVal
			xml.ResetMainPos();
			for (int i = 0;i < linenum;i ++)
			{

				if (xml.FindElem("ExtremumVal"))
				{
					xml.IntoElem();//进入MaxUa
					xml.ResetMainPos();
					/*********************************************************/
					if (xml.FindElem("MaxUa"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ua > GetVal)&&(ExtYcVal[i].Ua < UMaxLimit))
						{
							xml.SetData(ExtYcVal[i].Ua);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinUa"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ua < GetVal)&&(ExtYcVal[i].Uc > UMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ua);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxUb"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Ub > GetVal)
						{
							xml.SetData(ExtYcVal[i].Ub);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinUb"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ub < GetVal)&&(ExtYcVal[i].Ub > UMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ub);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxUc"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Uc > GetVal)
						{
							xml.SetData(ExtYcVal[i].Uc);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinUc"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Uc < GetVal)&&(ExtYcVal[i].Uc > UMinLimit))
						{
							xml.SetData(ExtYcVal[i].Uc);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxIa"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Ia > GetVal)
						{
							xml.SetData(ExtYcVal[i].Ia);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinIa"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ia < GetVal)&&(ExtYcVal[i].Ia > IMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ia);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxIb"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Ib > GetVal)
						{
							xml.SetData(ExtYcVal[i].Ib);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinIb"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ib < GetVal)&&(ExtYcVal[i].Ib > IMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ib);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxIc"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Ic > GetVal)
						{
							xml.SetData(ExtYcVal[i].Ic);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinIc"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ic < GetVal)&&(ExtYcVal[i].Ic > IMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ic);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					/*********************************************************/
					xml.OutOfElem();//退出MaxUa
				}

			}//for循环
			xml.OutOfElem();//退出ExtremumVal
		}//ExtremumRecordNode
		xml.OutOfElem();//退出ExtremumRecordNode
	}
	xml.Save(BFExtremum);	
	return 0;
}

int CBF533::SaveHistoricalData(share_terminal_ptr terminalPtr)
{
	//  PrintMessage("开始保存.. ...");

	int linenum;
	int Index;
	int GetVal;
	int offset = 12;
	int UMinLimit = 50;  //计算极值时的门槛值  
	int IMinLimit = 50;
	int UMaxLimit = 50000;  //计算极值时的门槛值  500V
	int IMaxLimit = 150000; //150A
	int infonum = terminalPtr->getYcSum();
	std::stringstream ss1,ss2;

	std::string str1;
	std::string str2;

	int year,month,day,hour,min;

	using namespace boost::posix_time;
	boost::posix_time::time_duration td = (boost::posix_time::microsec_clock::local_time()).time_of_day();
	boost::gregorian::date::ymd_type ymd = (boost::posix_time::microsec_clock::local_time()).date().year_month_day();

	int Sec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) / (1000);
	int Msec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) % (1000);

	year = ymd.year;
	month = ymd.month;
	day = ymd.day;
	hour = td.hours();
	min = td.minutes();

	ss1 <<"/mnt/"<<year<<month<<day<<".xml";
	ss1 >> str1;

	ss2 <<year<<"/"<<month<<"/"<<day<<"-"<<hour<<":"<<min<<":"<<Sec<<":"<<Msec;
	ss2 >> str2;

	linenum = infonum / offset;

	//  std::cout<<"现在时间是："<< str2 <<std::endl;
	//  std::cout<<"文件名为："<< str1 <<std::endl; 
	//  std::cout<<"遥测数量是："<<infonum<<"线路数量是："<<linenum<<std::endl; 

	for (int i = 0;i < linenum;i ++)
	{
		Index = 0;
		ExtYcVal[i].Ua  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Ub  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Uc  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Ia  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Ib  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Ic  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].U0  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].I0  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Fre = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].Fac = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].AP  = terminalPtr->getFinalYcVal(i*offset + (Index++));
		ExtYcVal[i].RP  = terminalPtr->getFinalYcVal(i*offset + (Index++));
	}


	FileSystem::CMarkup xml;
	if (!xml.Load(str1))
	{
		PrintMessage("Load 历史数据文件出错... .");
		Creat_HistoricalData(linenum, boost::posix_time::microsec_clock::local_time());
		xml.Load(str1);
		//	  return -1;
	}

	//以下为统计极值
	xml.ResetMainPos();
	if (xml.FindElem("HistoricalData"))
	{
		xml.IntoElem();//进入ExtremumRecordNode
		xml.ResetMainPos();
		if (xml.FindElem("ExtremumRecordNode"))
		{
			xml.IntoElem();//进入ExtremumVal
			xml.ResetMainPos();
			for (int i = 0;i < linenum;i ++)
			{

				if (xml.FindElem("ExtremumVal"))
				{
					xml.IntoElem();//进入MaxUa
					xml.ResetMainPos();
					/*********************************************************/
					if (xml.FindElem("MaxUa"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ua > GetVal)&&(ExtYcVal[i].Ua < UMaxLimit))
						{
							xml.SetData(ExtYcVal[i].Ua);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinUa"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ua < GetVal)&&(ExtYcVal[i].Uc > UMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ua);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxUb"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Ub > GetVal)
						{
							xml.SetData(ExtYcVal[i].Ub);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinUb"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ub < GetVal)&&(ExtYcVal[i].Ub > UMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ub);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxUc"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Uc > GetVal)
						{
							xml.SetData(ExtYcVal[i].Uc);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinUc"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Uc < GetVal)&&(ExtYcVal[i].Uc > UMinLimit))
						{
							xml.SetData(ExtYcVal[i].Uc);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxIa"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Ia > GetVal)
						{
							xml.SetData(ExtYcVal[i].Ia);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinIa"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ia < GetVal)&&(ExtYcVal[i].Ia > IMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ia);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxIb"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Ib > GetVal)
						{
							xml.SetData(ExtYcVal[i].Ib);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinIb"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ib < GetVal)&&(ExtYcVal[i].Ib > IMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ib);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					if (xml.FindElem("MaxIc"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if (ExtYcVal[i].Ic > GetVal)
						{
							xml.SetData(ExtYcVal[i].Ic);
							xml.SetAttrib("Time",str2);
						}
					}//MaxUa
					if (xml.FindElem("MinIc"))
					{
						std::string strTmp = xml.GetData();
						boost::algorithm::trim(strTmp);
						try
						{
							GetVal = boost::lexical_cast<int>(strTmp);
							//					   std::cout<<"所得值为："<< GetVal <<std::endl; 
						}
						catch(boost::bad_lexical_cast& e)
						{
							std::ostringstream ostr;
							ostr<<e.what();
						}
						if ((ExtYcVal[i].Ic < GetVal)&&(ExtYcVal[i].Ic > IMinLimit))
						{
							xml.SetData(ExtYcVal[i].Ic);
							xml.SetAttrib("Time",str2);
						}
					}//MinUa
					/*********************************************************/
					xml.OutOfElem();//退出MaxUa
				}

			}//for循环
			xml.OutOfElem();//退出ExtremumVal
		}//ExtremumRecordNode
		xml.OutOfElem();//退出ExtremumRecordNode

		//以下记录遥测数据
		if(xml.FindChildElem("HistoricalNode"))//一开始存在
		{
			std::cout<<"记录存在... ..."<<std::endl; 
			xml.IntoElem();
			for(int i = 0;i < linenum;i ++)
			{
				xml.AddChildElem( "HistoricalVal" );
				xml.SetChildAttrib( "LineNo",i);
				xml.IntoElem();
				xml.AddChildElem( "Ua");
				xml.SetChildData(ExtYcVal[i].Ua);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Ub");
				xml.SetChildData(ExtYcVal[i].Ub);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Uc");
				xml.SetChildData(ExtYcVal[i].Uc);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Ia");
				xml.SetChildData(ExtYcVal[i].Ia);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Ib");
				xml.SetChildData(ExtYcVal[i].Ib);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Ic");
				xml.SetChildData(ExtYcVal[i].Ic);
				xml.SetChildAttrib( "Time",str2);

				xml.AddChildElem( "U0");
				xml.SetChildData(ExtYcVal[i].U0);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "I0");
				xml.SetChildData(ExtYcVal[i].I0);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Fre");
				xml.SetChildData(ExtYcVal[i].Fre);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Fac");
				xml.SetChildData(ExtYcVal[i].Fac);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "AP");
				xml.SetChildData(ExtYcVal[i].AP);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "RP");
				xml.SetChildData(ExtYcVal[i].RP);
				xml.SetChildAttrib( "Time",str2);
				xml.OutOfElem();
			}
			xml.OutOfElem();
		}
		else
		{ 
			xml.AddChildElem( "HistoricalNode" ); 
			xml.IntoElem();
			for(int i = 0;i < linenum;i ++)
			{
				xml.AddChildElem( "HistoricalVal" );
				xml.SetChildAttrib( "LineNo",i);
				xml.IntoElem();
				xml.AddChildElem( "Ua");
				xml.SetChildData(ExtYcVal[i].Ua);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Ub");
				xml.SetChildData(ExtYcVal[i].Ub);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Uc");
				xml.SetChildData(ExtYcVal[i].Uc);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Ia");
				xml.SetChildData(ExtYcVal[i].Ia);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Ib");
				xml.SetChildData(ExtYcVal[i].Ib);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Ic");
				xml.SetChildData(ExtYcVal[i].Ic);
				xml.SetChildAttrib( "Time",str2);

				xml.AddChildElem( "U0");
				xml.SetChildData(ExtYcVal[i].U0);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "I0");
				xml.SetChildData(ExtYcVal[i].I0);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Fre");
				xml.SetChildData(ExtYcVal[i].Fre);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "Fac");
				xml.SetChildData(ExtYcVal[i].Fac);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "AP");
				xml.SetChildData(ExtYcVal[i].AP);
				xml.SetChildAttrib( "Time",str2);
				xml.AddChildElem( "RP");
				xml.SetChildData(ExtYcVal[i].RP);
				xml.SetChildAttrib( "Time",str2);
				xml.OutOfElem();
			}
			xml.OutOfElem();
		}

	}//HistoricalData

	xml.Save(str1);

	return 0;
}


int CBF533::SaveYMData(int LineNum)
{

	int ParentNum,ChildNum;
	int ParentNo,ChildNo;
	int ParentCurPtr,ChildCurPtr;
	double Temp = 0.0;

	std::stringstream ss_Time,ss1,ss_Title;

	std::string str_Time,str1,str_Title;

	int year,month,day,hour,min;

	using namespace boost::posix_time;
	boost::posix_time::time_duration td = (boost::posix_time::microsec_clock::local_time()).time_of_day();
	boost::gregorian::date::ymd_type ymd = (boost::posix_time::microsec_clock::local_time()).date().year_month_day();

	int Sec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) / (1000);
	int Msec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) % (1000);

	year = ymd.year;
	month = ymd.month;
	day = ymd.day;
	hour = td.hours();
	min = td.minutes();

	if (YMDataRecordType_ == YearRecordFlag)
	{
		ss_Title <<year;
		ss_Title >> str_Title; 
	} 
	else if (YMDataRecordType_ == MonthRecordFlag)
	{
		ss_Title <<year<<month;
		ss_Title >> str_Title; 
	}
	else
	{
		ss_Title <<year<<month<<day;
		ss_Title >> str_Title; 
	}

	ss_Time <<year<<"/"<<month<<"/"<<day<<"-"<<hour<<":"<<min<<":"<<Sec<<":"<<Msec;
	ss_Time >> str_Time;


	FileSystem::CMarkup xml;
	if (!xml.Load(BFYMData))
	{
		PrintMessage("Load 积分电度文件文件出错... ...");
		Creat_BFYMData(LineNum);
		xml.Load(BFYMData);
	}

	xml.ResetMainPos();
	if (xml.FindElem("YMData"))
	{
		std::string strTmp = xml.GetAttrib("Num");
		boost::algorithm::trim(strTmp);
		try
		{
			ParentNum = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}

		std::string strTmp1 = xml.GetAttrib("CurPtr");
		boost::algorithm::trim(strTmp1);
		try
		{
			ParentCurPtr = boost::lexical_cast<int>(strTmp1);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}

		if (ParentNum == 0)
		{
			xml.IntoElem();
			xml.ResetMainPos();
			xml.AddElem("YMDataNode");
			xml.SetAttrib("Title",str_Title);
			xml.SetAttrib("Num",1);
			xml.SetAttrib("LineSum",LineNum);
			xml.SetAttrib("CurPtr",1);
			xml.SetAttrib("No",0);
			xml.IntoElem();
			xml.ResetMainPos();
			xml.AddElem("YMDataNO");
			xml.SetAttrib("No",0);
			xml.SetAttrib("Time",str_Time);
			for (int i = 0;i < LineNum;i ++)
			{
				xml.IntoElem();
				xml.AddElem("YMDataVal");
				xml.SetAttrib("Lineno",i);
				xml.AddChildElem("Val");
				xml.SetChildData(TransDoubleToString(YMData[i].YMDataVal));
				xml.OutOfElem();
			}
			xml.OutOfElem();
			xml.OutOfElem();
			xml.SetAttrib("Num",1);
			xml.SetAttrib("CurPtr",1);
			xml.Save(BFYMData);
			return 0;
		}//if (ParentNum == 0)
		else
		{
			xml.IntoElem();//YMDataNode
			xml.ResetMainPos();
			for (int i = 0; i < ParentNum; i ++)
			{
				if (xml.FindElem("YMDataNode"))
				{
					std::string strTmp = xml.GetAttrib("No");
					boost::algorithm::trim(strTmp);
					try
					{
						ParentNo = boost::lexical_cast<int>(strTmp);
					}
					catch(boost::bad_lexical_cast& e)
					{
						std::ostringstream ostr;
						ostr<<e.what();
					} 

					if (ParentNo + 1 == ParentCurPtr)//找到最新一个节点
					{
						std::string Title = xml.GetAttrib("Title");
						if (Title == str_Title)//继续追加次数，在此判断次数是否循环
						{
							std::string Num = xml.GetAttrib("Num");
							boost::algorithm::trim(Num);
							try
							{
								ChildNum = boost::lexical_cast<int>(Num);
							}
							catch(boost::bad_lexical_cast& e)
							{
								std::ostringstream ostr;
								ostr<<e.what();
							}

							std::string CurPtr = xml.GetAttrib("CurPtr");
							boost::algorithm::trim(CurPtr);
							try
							{
								ChildCurPtr = boost::lexical_cast<int>(CurPtr);
							}
							catch(boost::bad_lexical_cast& e)
							{
								std::ostringstream ostr;
								ostr<<e.what();
							}

							//先取得上一次的值
							if (ChildNum < YMDataCount_)//直接追加时读取上一次值
							{
								xml.IntoElem();
								xml.ResetMainPos();
								for (int j = 0; j < ChildNum; j ++)
								{
									if (xml.FindElem("YMDataNO"))
									{
										std::string No = xml.GetAttrib("No");
										boost::algorithm::trim(No);
										try
										{
											ChildNo = boost::lexical_cast<int>(No);
										}
										catch(boost::bad_lexical_cast& e)
										{
											std::ostringstream ostr;
											ostr<<e.what();
										}

										if (ChildNo + 1 == ChildCurPtr)
										{
											xml.IntoElem();
											xml.ResetMainPos();
											for (int k = 0; k < LineNum; k ++)//读出值
											{

												if (xml.FindElem("YMDataVal"))
												{
													if (xml.FindChildElem("Val"))
													{
														std::string strTmp = xml.GetChildData();
														boost::algorithm::trim(strTmp);
														try
														{
															Temp = boost::lexical_cast<double>(strTmp);
															//std::cout<<"Data... ..."<<Temp<<std::endl;  
														}
														catch(boost::bad_lexical_cast& e)
														{
															std::ostringstream ostr;
															ostr<<e.what();
														}
														YMData[k].YMDataVal += Temp;
													}

												}
												//xml.OutOfElem();
											}
											xml.OutOfElem();
											break;
										}
									}

								}//for (int j = 0; j < ChildNum; j ++)
								xml.OutOfElem();
							}
							else//循环覆盖时读取值
							{
								int No_Temp;
								if (ChildCurPtr == 1)
								{
									No_Temp = YMDataCount_;
								} 
								else
								{
									No_Temp = ChildCurPtr - 1;
								}
								xml.IntoElem();
								xml.ResetMainPos();
								for (int j = 0; j < ChildNum; j ++)
								{
									if (xml.FindElem("YMDataNO"))
									{
										std::string No = xml.GetAttrib("No");
										boost::algorithm::trim(No);
										try
										{
											ChildNo = boost::lexical_cast<int>(No);
										}
										catch(boost::bad_lexical_cast& e)
										{
											std::ostringstream ostr;
											ostr<<e.what();
										}

										if (No_Temp == ChildNo + 1)
										{
											xml.IntoElem();
											xml.ResetMainPos();
											for (int k = 0; k < LineNum; k ++)//读出值
											{

												if (xml.FindElem("YMDataVal"))
												{
													if (xml.FindChildElem("Val"))
													{
														std::string strTmp = xml.GetChildData();
														boost::algorithm::trim(strTmp);
														try
														{
															Temp = boost::lexical_cast<double>(strTmp);
															//std::cout<<"Data... ..."<<Temp<<std::endl;  
														}
														catch(boost::bad_lexical_cast& e)
														{
															std::ostringstream ostr;
															ostr<<e.what();
														}
														YMData[k].YMDataVal += Temp;
													}

												}
												//xml.OutOfElem();
											}
											xml.OutOfElem();
											break;
										}
									}
								}
								xml.OutOfElem();
							}

							if (ChildNum < YMDataCount_)//直接追加新的记录
							{
								xml.IntoElem();
								xml.ResetMainPos();
								xml.AddElem("YMDataNO");
								xml.SetAttrib("No",ChildNum);
								xml.SetAttrib("Time",str_Time);
								for (int kk = 0;kk < LineNum;kk ++)
								{
									str1 = "0";
									xml.IntoElem();
									xml.AddElem("YMDataVal");
									xml.SetAttrib("Lineno",i);
									xml.AddChildElem("Val");
									xml.SetChildData(TransDoubleToString(YMData[kk].YMDataVal));
									xml.OutOfElem();
								}
								xml.OutOfElem();

								xml.SetAttrib("Num",ChildNum + 1);
								xml.SetAttrib("LineSum",LineNum);
								if (ChildCurPtr + 1 == YMDataCount_)
								{
									xml.SetAttrib("CurPtr",1);
								} 
								else
								{
									xml.SetAttrib("CurPtr",ChildCurPtr + 1);
								}
							} 
							else//开始循环覆盖次数
							{
								xml.IntoElem();
								xml.ResetMainPos();
								for (int kk = 0; kk < ChildNum; kk ++)
								{
									if (xml.FindElem("YMDataNO"))
									{
										std::string No_1 = xml.GetAttrib("No");
										boost::algorithm::trim(No_1);
										try
										{
											ChildNo = boost::lexical_cast<int>(No_1);
										}
										catch(boost::bad_lexical_cast& e)
										{
											std::ostringstream ostr;
											ostr<<e.what();
										}

										if (ChildNo + 1 == ChildCurPtr)
										{
											xml.RemoveElem();
											xml.AddElem("YMDataNO");
											xml.SetAttrib("No",ChildNo);
											xml.SetAttrib("Time",str_Time);
											for (int j = 0;j < LineNum;j ++)
											{
												str1 = "0";
												xml.IntoElem();
												xml.AddElem("YMDataVal");
												xml.SetAttrib("Lineno",j);
												xml.AddChildElem("Val");
												xml.SetChildData(TransDoubleToString(YMData[j].YMDataVal));
												xml.OutOfElem();
											}
											break;
										}
									}//(xml.FindElem("YMDataNO"))
								}
								xml.OutOfElem();

								if (ChildCurPtr == YMDataCount_)
								{
									xml.SetAttrib("CurPtr",1);
								} 
								else
								{
									xml.SetAttrib("CurPtr",ChildCurPtr + 1);
								}
							}//循环覆盖次数

						}//Title = str_Title 
						else//此时进入新的时间记录，在此判断是否需要循环覆盖按时间的记录
						{
							xml.OutOfElem();
							if (ParentNum < YMDataRecordNum_)//继续追加新的按时间记录
							{
								xml.SetAttrib("Num",ParentNum + 1);
								xml.SetAttrib("CurPtr",ParentCurPtr + 1);
								xml.IntoElem();
								xml.ResetMainPos();
								xml.AddElem("YMDataNode");
								xml.SetAttrib("Title",str_Title);
								xml.SetAttrib("Num",1);
								xml.SetAttrib("LineSum",LineNum);
								xml.SetAttrib("CurPtr",1);
								xml.SetAttrib("No",ParentNo + 1);
								xml.IntoElem();
								xml.ResetMainPos();
								xml.AddElem("YMDataNO");
								xml.SetAttrib("No",0);
								xml.SetAttrib("Time",str_Time);
								for (int i = 0;i < LineNum;i ++)
								{
									xml.IntoElem();
									xml.AddElem("YMDataVal");
									xml.SetAttrib("Lineno",i);
									xml.AddChildElem("Val");
									xml.SetChildData(TransDoubleToString(YMData[i].YMDataVal));
									xml.OutOfElem();
								}
								xml.OutOfElem();
							} 
							else//需要按时间循环覆盖
							{
								if (ParentCurPtr == YMDataRecordNum_)
								{
									ParentCurPtr = 0;
									xml.SetAttrib("CurPtr",ParentCurPtr + 1);
								} 
								else
								{
									xml.SetAttrib("CurPtr",ParentCurPtr + 1);
								}

								xml.IntoElem();
								xml.ResetMainPos();
								for (int j = 0; j < YMDataRecordNum_; j ++)
								{
									if (xml.FindElem("YMDataNode"))
									{
										std::string No_2 = xml.GetAttrib("No");
										boost::algorithm::trim(No_2);
										try
										{
											ParentNo = boost::lexical_cast<int>(No_2);
										}
										catch(boost::bad_lexical_cast& e)
										{
											std::ostringstream ostr;
											ostr<<e.what();
										}

										if (ParentNo == ParentCurPtr)//开始按时间循环覆盖
										{
											xml.RemoveElem();
											xml.AddElem("YMDataNode");
											xml.SetAttrib("Title",str_Title);
											xml.SetAttrib("Num",1);
											xml.SetAttrib("LineSum",LineNum);
											xml.SetAttrib("CurPtr",1);
											xml.SetAttrib("No",ParentNo);
											xml.IntoElem();
											xml.ResetMainPos();
											xml.AddElem("YMDataNO");
											xml.SetAttrib("No",0);
											xml.SetAttrib("Time",str_Time);
											for (int k = 0; k < LineNum; k ++)
											{
												xml.IntoElem();
												xml.AddElem("YMDataVal");
												xml.SetAttrib("Lineno",k);
												xml.AddChildElem("Val");
												xml.SetChildData(TransDoubleToString(YMData[k].YMDataVal));
												xml.OutOfElem();
											}
											xml.OutOfElem();
											break;
										}//(ParentNo + 1 == ParentCurPtr)
									}//xml.FindElem("YMDataNode")

								}//for (int j = 0; j < YMDataRecordNum_; j ++)
								xml.OutOfElem();

							}
						}//Title != str_Title
					}//ParentNo == ParentCurPtr
				} //if (xml.FindElem("YMDataNode"))
			}
			xml.OutOfElem();//YMDataNode
		}//if (ParentNum != 0)

	}

	xml.Save(BFYMData);
	return 0;
}

int CBF533::SaveFaultRecord(unsigned char * buf)
{
	int sum,CurPtr,No;
	int Location = 5;
	int FaultNo;
	int Val1/*,Val2,Val3*/;
	int year,month,day,hour,min,Sec,Msec;
	unsigned long int now_time;

	using namespace boost::posix_time;

	boost::gregorian::date::ymd_type ymd = (boost::posix_time::microsec_clock::local_time()).date().year_month_day();

	std::stringstream ss;

	std::string str;

	FaultNo = BufToVal(&buf[Location],2); 
	Location += 2;
	now_time = (BufToVal(&buf[Location],4)/SYNTIMEMULTIPLE); 
	Location += 4;
	Val1 = BufToVal(&buf[Location],2); 
	//Location += 2;
	//Val2 = BufToVal(&buf[Location],2); 
	//Location += 2;
	//Val3 = BufToVal(&buf[Location],2); 

	year  = ymd.year;
	month = ymd.month;
	day   = ymd.day;
	hour  =    now_time / (1000*60*60);
	min   = (  now_time % (1000*60*60)) / (1000*60);
	Sec   = (( now_time % (1000*60*60))%  (1000*60)) / (1000);
	Msec   = ((now_time % (1000*60*60))%  (1000*60)) % (1000);

	ss <<year<<"/"<<month<<"/"<<day<<"-"<<hour<<":"<<min<<":"<<Sec<<":"<<Msec;
	ss >> str;

	FileSystem::CMarkup xml;
	if (!xml.Load(BFFaultRecord))
	{
		PrintMessage("Load 故障记录文件出错... .");
		Creat_BFFaultRecord();
		xml.Load(BFFaultRecord);
		//		return -1;
	}

	xml.ResetMainPos();
	if (xml.FindElem("FaultmRecordNode"))
	{
		std::string strTmp = xml.GetAttrib("sum");
		boost::algorithm::trim(strTmp);
		try
		{
			sum = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}

		std::string strTmp1 = xml.GetAttrib("sum");
		boost::algorithm::trim(strTmp1);
		try
		{
			CurPtr = boost::lexical_cast<int>(strTmp1);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}

		if (sum < FaultRecordMax_)
		{
			xml.SetAttrib("sum",(sum + 1));
			xml.SetAttrib("CurPtr",(CurPtr + 1));

			xml.IntoElem();
			xml.AddElem("FaultmRecord");
			xml.SetAttrib("No",sum);
			xml.AddChildElem("FaultNo",FaultNo);
			xml.AddChildElem("FaultTime",str);
			xml.AddChildElem("Val1",Val1);
			//xml.AddChildElem("Val2",Val2);
			//xml.AddChildElem("Val3",Val3);
			xml.OutOfElem();
		} //sum < FaultRecordMax_
		else
		{
			if (CurPtr >= FaultRecordMax_)
			{
				xml.SetAttrib("CurPtr",2);
				CurPtr = 1;
			}
			else
			{
				xml.SetAttrib("CurPtr",CurPtr + 1);
			}

			xml.IntoElem();
			xml.ResetMainPos();
			for (int i = 0; i < CurPtr;i ++)
			{
				if (xml.FindElem("FaultmRecord"))
				{
					std::string strTmp = xml.GetAttrib("No");
					boost::algorithm::trim(strTmp);
					try
					{
						No = boost::lexical_cast<int>(strTmp);
					}
					catch(boost::bad_lexical_cast& e)
					{
						std::ostringstream ostr;
						ostr<<e.what();
					}

					if (No + 1 == CurPtr)
					{
						xml.IntoElem();
						if (xml.FindElem("FaultNo"))
						{
							xml.SetData(FaultNo);
						}
						if (xml.FindElem("FaultTime"))
						{
							xml.SetData(str);
						}
						if (xml.FindElem("Val1"))
						{
							xml.SetData(Val1);
						}
						//if (xml.FindElem("Val2"))
						//{
						//	xml.SetData(Val2);
						//}
						//if (xml.FindElem("Val3"))
						//{
						//	xml.SetData(Val3);
						//}
						xml.OutOfElem();
					}
				}
			}
			xml.OutOfElem();
		}

	}//FaultmRecordNode
	xml.Save(BFFaultRecord);
	return 0;
}

int CBF533::SaveNoCurRecord(unsigned short LineNo)
{
	int lineno_;
	int LineSum;
	int Count;
	int sum;
	std::stringstream ss2;


	std::string str2;

	int year,month,day,hour,min;

	using namespace boost::posix_time;
	boost::posix_time::time_duration td = (boost::posix_time::microsec_clock::local_time()).time_of_day();
	boost::gregorian::date::ymd_type ymd = (boost::posix_time::microsec_clock::local_time()).date().year_month_day();

	int Sec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) / (1000);
	int Msec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) % (1000);

	year = ymd.year;
	month = ymd.month;
	day = ymd.day;
	hour = td.hours();
	min = td.minutes();


	ss2 <<year<<"/"<<month<<"/"<<day<<"-"<<hour<<":"<<min<<":"<<Sec<<":"<<Msec;
	ss2 >> str2;

	LineSum = BF533Base.LineNum;

	FileSystem::CMarkup xml;
	if (!xml.Load(NoCurRecord))
	{
		Creat_NoCurRecord(BF533Base.LineNum);
		xml.Load(NoCurRecord);
	}

	if (xml.FindElem("NoCurRecord"))
	{

		std::string strTmpVal = xml.GetAttrib("Count");
		try
		{
			Count = boost::lexical_cast<int>(strTmpVal);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}

		sum = Count > LineSum ? Count : LineSum;

		xml.SetAttrib("Count",LineSum);

		xml.IntoElem();

		if (Count > LineSum)//删除多余节点
		{
			for(int i = 0;i < LineSum ;i ++)
			{
				if (xml.FindElem("RecordNode")){};
			}

			for (int i = 0;i < Count - LineSum;i ++)
			{
				xml.FindElem();
				xml.RemoveElem();
			}
		} 
		else
		{
			for (int i = 0;i < LineSum - Count;i ++)
			{
				xml.AddElem("RecordNode");
				xml.AddAttrib("LineNo",Count + i);
				xml.AddAttrib("Time",str2);
				xml.AddChildElem("Ua");
				xml.SetChildData(0);
				xml.AddChildElem("Ub");
				xml.SetChildData(0);
				xml.AddChildElem("Uc");
				xml.SetChildData(0);
				xml.AddChildElem("Ia");
				xml.SetChildData(0);
				xml.AddChildElem("Ib");
				xml.SetChildData(0);
				xml.AddChildElem("Ic");
				xml.SetChildData(0);
			}
		}

		xml.ResetMainPos();
		for(int i = 0;i < sum ;i ++)
		{
			if (xml.FindElem("RecordNode"))
			{
				std::string strTmp = xml.GetAttrib("LineNo");
				try
				{
					lineno_ = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();
				}

				if (lineno_ == LineNo)
				{
					xml.SetAttrib("Time",str2);
					xml.IntoElem();
					xml.ResetMainPos();
					if (xml.FindElem("Ua"))
					{
						xml.SetData(NoCurVal[LineNo].Ua);
					}
					xml.ResetMainPos();
					if (xml.FindElem("Ub"))
					{
						xml.SetData(NoCurVal[LineNo].Ub);
					}
					xml.ResetMainPos();
					if (xml.FindElem("Uc"))
					{
						xml.SetData(NoCurVal[LineNo].Uc);
					}
					xml.ResetMainPos();
					if (xml.FindElem("Ia"))
					{
						xml.SetData(NoCurVal[LineNo].Ia);
					}
					xml.ResetMainPos();
					if (xml.FindElem("Ib"))
					{
						xml.SetData(NoCurVal[LineNo].Ib);
					}
					xml.ResetMainPos();
					if (xml.FindElem("Ic"))
					{
						xml.SetData(NoCurVal[LineNo].Ic);
					}
					xml.OutOfElem();
				}

			}
		}


		xml.OutOfElem();
	}

	xml.Save(NoCurRecord);

	return 0;
}

int CBF533::SaveSOERecord(int No,unsigned char Val,unsigned long Time)
{
	//	PrintMessage("开始存储SOE... ...");
	int Sum,Index;
	int In;

	int year,month,day,hour,min,Sec,Msec;

	using namespace boost::posix_time;

	boost::gregorian::date::ymd_type ymd = (boost::posix_time::microsec_clock::local_time()).date().year_month_day();

	std::stringstream ss;

	std::string str;


	year  = ymd.year;
	month = ymd.month;
	day   = ymd.day;
	hour  =    Time / (1000*60*60);
	min   = (  Time % (1000*60*60)) / (1000*60);
	Sec   = (( Time % (1000*60*60))%  (1000*60)) / (1000);
	Msec   = ((Time % (1000*60*60))%  (1000*60)) % (1000);

	ss <<year<<"/"<<month<<"/"<<day<<"-"<<hour<<":"<<min<<":"<<Sec<<":"<<Msec;
	ss >> str;

	FileSystem::CMarkup xml;
	if (!xml.Load(BFSOERecord))
	{
		PrintMessage("Load SOE记录文件出错... ...");
		Creat_BFSOERecord();
		xml.Load(BFSOERecord);
		//		return -1;
	}

	xml.ResetMainPos();
	if (xml.FindElem("SOERecordNode"))
	{
		std::string strTmp0 = xml.GetAttrib("Sum");
		boost::algorithm::trim(strTmp0);
		try
		{
			Sum = boost::lexical_cast<int>(strTmp0);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}

		std::string strTmp1 = xml.GetAttrib("Index");
		boost::algorithm::trim(strTmp1);
		try
		{
			Index = boost::lexical_cast<int>(strTmp1);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}

		if (Sum < SOEStroeNum_)//不到规定条记录，增加新记录
		{
			//		  PrintMessage("Sum < 5 ... ...");
			xml.SetAttrib("Sum",(Sum + 1));
			xml.SetAttrib("Index",(Index + 1));

			xml.IntoElem();
			xml.AddElem("SOERecord");
			xml.SetAttrib("No",Sum);
			xml.AddChildElem("SOENo",No);
			xml.AddChildElem("SOEVal",Val);
			xml.AddChildElem("SOETime",str);
			xml.OutOfElem();
		}
		else if (Sum == SOEStroeNum_)
		{
			if (Index >= SOEStroeNum_)
			{
				Index = 0;
			}
			xml.SetAttrib("Index",(Index + 1));

			xml.IntoElem();
			for(int i = 0;i < Index + 1;i ++)
			{
				if (xml.FindElem("SOERecord"))
				{
					//                 PrintMessage("找到节点SOERecord... ...");
					std::string strTmp = xml.GetAttrib("No");
					boost::algorithm::trim(strTmp);
					try
					{
						In = boost::lexical_cast<int>(strTmp);
					}
					catch(boost::bad_lexical_cast& e)
					{
						std::ostringstream ostr;
						ostr<<e.what();
					}
					if (Index == In)
					{
						//				   std::cout<<"Index:"<<Index<<",In:"<<In<<std::endl;
						//                   PrintMessage("Index = In");
						xml.IntoElem();
						xml.ResetMainPos();
						if (xml.FindElem("SOENo"))
						{
							xml.SetData(No);
						}
						xml.ResetMainPos();
						if (xml.FindElem("SOEVal"))
						{
							xml.SetData(Val);
						}
						xml.ResetMainPos();
						if (xml.FindElem("SOETime"))
						{
							xml.SetData(str);
						}
						xml.OutOfElem();
					}
				}
			}
			xml.OutOfElem();	   	
		}

	}//FaultmRecordNode
	xml.Save(BFSOERecord);
	return 0;
}


int CBF533::SetBF518Para(void)
{
	char Eth1IP[24];
	char Eth1MASK[24];
	char Eth2IP[24];
	char Eth2MASK[24];
	int No1,No2;

	std::string str;

	FileSystem::CMarkup xml;
	if (!xml.Load(BFInterfacePara))
	{
		//PrintMessage("Load BFInterfacePara文件出错... ...");
		Creat_InterfacePara();
		xml.Load(BFInterfacePara);
		//		return -1;
	}
	//PrintMessage("开始设定IP地址... ...");
	xml.ResetMainPos();
	if (xml.FindElem("PDZ_InterfacePara_Set"))
	{
		//PrintMessage("找到PDZ_InterfacePara_Set... ...");
		xml.IntoElem();
		xml.ResetMainPos(); 
		if (xml.FindElem("PDZ_Lan_Set"))
		{
			//PrintMessage("找到PDZ_Lan_Set 1... ...");
			std::string strTmp = xml.GetAttrib("LanNo");
			boost::algorithm::trim(strTmp);
			try
			{
				No1 = boost::lexical_cast<int>(strTmp);
				//std::cout<<"所得属性值为："<<No1<<std::endl;
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
			if (No1 == 1)
			{
				xml.IntoElem();
				xml.ResetMainPos(); 
				if (xml.FindElem("IP"))
				{
					//PrintMessage("找到IP 1... ...");
					std::string strTmp = xml.GetData();

					const char *Temp = strTmp.c_str();
					strcpy(Eth1IP,Temp);
					//std::cout<<"Eth1 IP 地址为："<<Eth1IP<<std::endl;
				}
				xml.ResetMainPos(); 
				if (xml.FindElem("MASK"))
				{
					//PrintMessage("找到MASK 1... ...");
					std::string strTmp = xml.GetData();

					const char *Temp = strTmp.c_str();
					strcpy(Eth1MASK,Temp);
					//std::cout<<"Eth1MASK[24]  地址为："<<Eth1MASK<<std::endl;
				}
				//setip("eth1", Eth1IP,Eth1MASK);
				xml.OutOfElem();
			}
		}//PDZ_Lan_Set
		if (xml.FindElem("PDZ_Lan_Set"))
		{
			//PrintMessage("找到PDZ_Lan_Set 2... ...");
			std::string strTmp = xml.GetAttrib("LanNo");
			boost::algorithm::trim(strTmp);
			try
			{
				No2 = boost::lexical_cast<int>(strTmp);
				//std::cout<<"所得属性值为："<<No2<<std::endl;
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
			if (No2 == 2)
			{
				xml.IntoElem();
				xml.ResetMainPos(); 
				if (xml.FindElem("IP"))
				{
					//PrintMessage("找到IP 2... ...");
					std::string strTmp = xml.GetData();

					const char *Temp = strTmp.c_str();
					strcpy(Eth2IP,Temp);
					//std::cout<<"Eth2 IP 地址为："<<Eth2IP<<std::endl;
				}
				xml.ResetMainPos(); 
				if (xml.FindElem("MASK"))
				{
					//PrintMessage("找到MASK 2... ...");
					std::string strTmp = xml.GetData();

					const char *Temp = strTmp.c_str();
					strcpy(Eth2MASK,Temp);
					//std::cout<<"Eth2MASK[24]  地址为："<<Eth2MASK<<std::endl;
				}
				setip("eth2", Eth2IP,Eth2MASK);
				xml.OutOfElem();
			}
		}//PDZ_Lan_Set
		xml.OutOfElem();
	}//PDZ_InterfacePara_Set

	ModfiySetipSh(Eth1IP,Eth2IP,Eth1MASK,Eth2MASK);

	return 0;  
}

int CBF533::ModfiySetipSh(char Eth1IP[24],char Eth2IP[24],char Eth1MASK[24],char Eth2MASK[24])
{
	//开始修改脚本文件

	std::ofstream fout;

	fout.open(IP_SH_NAME,std::ios::out);

	if(fout.is_open())
	{
		fout.seekp(0,std::ios::beg);
		fout<<IFCONFIGETH1<<Eth1IP<<SETNETMASK<<Eth1MASK<<" up"<<"\n";
		fout<<IFCONFIGETH2<<Eth2IP<<SETNETMASK<<Eth2MASK<<" up"<<"\n";
		fout<<std::flush;
	}

	fout.close();
	fout.clear();

	return 0;
}

std::string CBF533::TransDoubleToString(double Data)
{
	std::stringstream ss;
	std::string str;
	ss << Data;
	ss >> str;
	return str;
}

std::string CBF533::TransIntToString(int Data)
{
	std::stringstream ss;
	std::string str;
	ss << Data;
	ss >> str;
	return str;
}

bool CBF533::TransStringTobool(std::string Str)
{
	if (Str == "true")
	{
		return true;
	} 
	else
	{
		return false;
	}
}

char CBF533::TransStringToDate(std::string Str)
{
	if (Str == "year")
	{
		return YearRecordFlag;
	}
	else if (Str == "month")
	{
		return   MonthRecordFlag;
	} 
	else
	{
		return  DayRecordFlag;
	}
}

unsigned int  CBF533::TransStringToInt(std::string Str)
{
	std::stringstream ss2;

	unsigned char Data[5];
	unsigned int nIndex = 1;
	unsigned int Temp = 0;
	memset(Data,0,5);
	ss2<<Str;
	ss2>>Data;
	for (int i = 3;i >= 0;i -- )
	{
		if (Data[i] < 58)
		{
			Data[i] = Data[i] - 48;
		}
		else if((Data[i] > 64)&&(Data[i] < 91))
		{
			Data[i] = Data[i] - 55;
		}
		else
		{
			Data[i] = Data[i] - 55  - 32;
		}
		Temp = Temp + Data[i] * nIndex;
		nIndex = nIndex * 16;
	}
	return Temp;
}

std::string CBF533::TransIntToHexString(unsigned int Data)
{
	std::string Val;
	std::stringstream ss2;
	unsigned char Temp[5];
	memset(Temp,0,5);
	int nIndex = 0;
	int j = 0;

	while(j < 4)
	{
		nIndex = Data % 16;
		if (nIndex < 10)
		{
			Temp[j] = nIndex + 48;
		} 
		else
		{
			Temp[j] = nIndex + 55;
		}
		j ++;
		Data = Data / 16;
	}

	ss2<<Temp[3]<<Temp[2]<<Temp[1]<<Temp[0];
	ss2>>Val;

	return Val;
}

void CBF533::setAutoDownloadCfgVal(std::string val)
{
	AutoDownloadCfgType_ = val;
}

std::string CBF533::getAutoDownloadCfgVal(void)
{
	return AutoDownloadCfgType_;
}

void CBF533::setUMaxLimit(unsigned short val)
{
	UMaxLimit_ = val;
}

void CBF533::setUMinLimit(unsigned short val)
{
	UMinLimit_ = val;
}

void CBF533::setIMaxLimit(unsigned short val)
{
	IMaxLimit_ = val;
}

void CBF533::setIMinLimit(unsigned short val)
{
	IMinLimit_ = val;
}

//void CBF533::setResetYcOverIndex(unsigned short val)
//{
//	ResetYcOverIndex_ = val;
//}

void CBF533::setNoCurDeadVal(unsigned short val)
{
	NoCurDeadVal_ = val;
}

//void CBF533::setYcOveredFlage(bool val)
//{
//   YcOveredFlage_ = val;
//}

unsigned short CBF533::getUMaxLimit(void)
{
	return UMaxLimit_;
}

unsigned short CBF533::getUMinLimit(void)
{
	return UMinLimit_;
}

unsigned short CBF533::getIMaxLimit(void)
{
	return IMaxLimit_;
}

unsigned short CBF533::getIMinLimit(void)
{
	return IMinLimit_;
}

unsigned short CBF533::getResetYcOverIndex(void)
{
	return ResetYcOverIndex_;
}

//bool CBF533::getYcOveredFlage(void)
//{
//	return YcOveredFlage_;
//}

unsigned short CBF533::getNoCurDeadVal(void)
{
	return NoCurDeadVal_;
}

void CBF533::AutoDownloadCfg(share_terminal_ptr terminalPtr)
{
	if (getAutoDownloadCfgVal() == strDownloadAll)
	{
		AddStatusLogWithSynT("开始自动下载所有参数 \n");
	} 
	else if(getAutoDownloadCfgVal() == strDownloadDevSanyaoPara)
	{
		AddStatusLogWithSynT("开始自动下载装置参数 \n");
		AddSendCmdVal(DOWNLOAD_EQU_PARA,DOWNLOAD_EQU_PARA_PRIORITY,terminalPtr);
	}
	else if(getAutoDownloadCfgVal() == strDownloadChanneltypePara)
	{
		AddStatusLogWithSynT("开始自动下载通道类型 \n");
		AddSendCmdVal(DOWNLOAD_CHANNEL_PARA,DOWNLOAD_CHANNEL_PARA_PRIORITY,terminalPtr);
	}
	else if(getAutoDownloadCfgVal() == strDownloadLinePara)
	{
		AddStatusLogWithSynT("开始自动下载线路参数 \n");
		AddSendCmdVal(DOWNLOAD_CHANNEL_COMREL,DOWNLOAD_CHANNEL_COMREL_PRIORITY,terminalPtr);
	}
	else if(getAutoDownloadCfgVal() == strDownloadProtectValPara)
	{
		AddStatusLogWithSynT("开始自动下载保护定值及保护控制字 \n");
		AddSendCmdVal(DOWNLOAD_DELAY_VAL,DOWNLOAD_DELAY_VAL_PRIORITY,terminalPtr,0);
	}
	else
	{
		AddStatusLogWithSynT("自动下载参数配置文件出错，没有对应的文件来下载 \n");
	}
}

void CBF533::setBf533ProName(std::string str)
{
	BF533ProName = str;
}

void CBF533::setBf533AutoloadFlage(bool flage)
{
	bBF533AutoDownload_ = flage;
}

bool CBF533::getBf533AutoloadFlage(void)
{
	return bBF533AutoDownload_;
}

std::string CBF533::getBf533ProName(void)
{
	return BF533ProName; 
}

}; //Protocol 
