#include <boost/bind.hpp>
//#include <boost/scoped_array.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/exception/all.hpp>
#include <boost/thread.hpp>
//#include <fstream>

#include "S104_518.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../FileSystem/Markup.h"
#include "../DataBase/PriStation.h"
//#include "../DataBase/YkPoint.h"
//#include "../DataBase/YxPoint.h"
//#include "../DataBase/SoePoint.h"
//#include "../DataBase/YcVarPoint.h"


namespace Protocol{

const unsigned int CRC_Code = 0x000A;

const std::string strDefaultCfg = "S104_518Cfg.xml";

#define      BFProtectValPara               "ProtectValPara.xml"
#define      BFBasePara                     "BasePara.xml"
#define FileLen     512  //召唤文件时每帧报文所包含的长度

//针对104规约的YK功能码
//const unsigned char DYK_OPEN_NEGATIVE = 0;
//const unsigned char DYK_TYPE_OPEN = 0x01;
//const unsigned char DYK_TYPE_CLOSE = 0x02;
//const unsigned char SYK_TYPE_OPEN = 0;
//const unsigned char SYK_TYPE_CLOSE = 0x01;
//const unsigned char DYK_CLOSE_NEGATIVE = 0x03;

//各种报文信息体元素的最大数量，一般情况下其最大值是127（信息体数目元素所占字节数为1的情况)
//const int INFONUM_LIMIT_ALLYXFRAME = 64;
//const int INFONUM_LIMIT_ALLYCFRAME = 48;
//const int INFONUM_LIMIT_ALLDDFRAME = 28;
//const int INFONUM_LIMIT_COSFRAME = 32;
//const int INFONUM_LIMIT_SOEFRAME = 16;
//const int INFONUM_LIMIT_YCVARFRAME = 24;

//针对BF533定义
//const unsigned char M_SG_RE_1 = 0xA6; //远方信号复归
const unsigned char M_FI_RE_3 = 0xA0;  //召唤文件类型标志
const unsigned char M_FI_SE_3 = 0xA1;  //发送文件类型标志
const unsigned char M_LV_VR_3 = 0xA2;  //定值校验类型标志
const unsigned char M_VA_VR_3 = 0xA3;  //召唤定值系数类型标志
const unsigned char M_VA_DL_3 = 0xA4;  //下装定值系数类型标志
const unsigned char M_ZD_RE_3 = 0xA5;  //装置复位类型标志
const unsigned char M_SG_RE_3 = 0xA6;  //远方信号复归类型标志
const unsigned char M_HA_MO_3 = 0xA7;  //召唤谐波分量类型标志
const unsigned char M_CA_TM_3 = 0xA9;  //召唤时钟类型标志
const unsigned char M_BD_RQ_3 = 0xB0;  //板件查询类型标志
const unsigned char M_AC_BT_3 = 0xB1;  //启动电池活化类型标志
const unsigned char M_OV_BT_3 = 0xB2;  //退出电池活化类型标志
const unsigned char M_DL_PA_3 = 0xB3;  //下装参数成功
const unsigned char M_RE_VR_3 = 0xB4;  //CPU版本查询
const unsigned char M_DL_BV_3 = 0xB5;  //保护电流校验
const unsigned char M_DC_VR_3 = 0xB6;  //直流量校验类型标志
const unsigned char M_PM_CA_3 = 0xB7;  //绝对相角查询类型标志
const unsigned char M_CA_CR_3 = 0xB8;  //召唤CRC类型标志

const unsigned char C_SG_RE_3 = 0x03;  //远方信号复归功能码


//针对104规约的传送原因定义
//const unsigned char trans_cyc = 0x01;
//const unsigned char trans_back = 0x02;
const unsigned char trans_spont = 0x03;
//const unsigned char trans_init = 0x04;
const unsigned char trans_req = 0x05;
const unsigned char trans_act = 0x06;
const unsigned char trans_actcon = 0x07;
const unsigned char trans_deact = 0x08;
const unsigned char trans_deactcon = 0x09;
const unsigned char trans_actterm = 0x0a;
//const unsigned char trans_retrem = 0x0b;
//const unsigned char trans_retloc = 0x0c;
//const unsigned char trans_file = 0x0d;
const unsigned char trans_all = 0x14;

const unsigned char trans_other = 0x47;
const unsigned char trans_no = 0x87;

//针对104规约的报文类型标识定义
//const unsigned char M_SP_NA_1 = 0x01;
//const unsigned char M_DP_NA_1 = 0x03;
////const unsigned char M_BO_NA_1 = 0x07;
//const unsigned char M_ME_NA_1 = 0x09;
//const unsigned char M_ME_NB_1 = 0x0b; //11
//const unsigned char M_ME_NC_1 = 0x0d; //13
//const unsigned char M_IT_NA_1 = 0x0f; //15
////const unsigned char M_PS_NA_1 = 0x14; //20
//const unsigned char M_ME_ND_1 = 0x15; //21
//const unsigned char M_SP_TB_1 = 0x1e; //30
//const unsigned char M_DP_TB_1 = 0x1f; //31
//const unsigned char M_ME_TD_1 = 0x22;
//const unsigned char M_ME_TE_1 = 0x23;
//const unsigned char M_ME_TF_1 = 0x24;
//const unsigned char M_IT_TB_1 = 0x25;

const unsigned char M_EP_TB_1 = 0x26; //38  带时标CP56Time2a的继电保护设备事件
const unsigned char M_EP_TE_1 = 0x27; //39  带时标CP56Time2a的继电保护设备成组启动事件
const unsigned char M_EP_TF_1 = 0x28; //40  带时标CP56Time2a的继电保护设备成组输出电路信息
const unsigned char M_EP_TJ_1 = 0x29; //41   带时标CP56Time2a和模拟量参数的继电保护设备事件
const unsigned char M_EP_TZ_1 = 0x6C; //108  召唤继电保护定值
const unsigned char M_EP_TX_1 = 0x72; //114  响应继电保护定值
const unsigned char M_EP_TS_1 = 0x73; //115  下装继电保护定值
const unsigned char M_EP_TH_1 = 0x74; //116  激活继电保护定值
const unsigned char M_EP_TG_1 = 0x35; //53   激活继电保护信号复归 //0x34修改为0x35

//const unsigned char M_SC_NA_1 = 0x2d; //45
//const unsigned char M_DC_NA_1 = 0x2e; //46
//const unsigned char C_SE_NA_1 = 0x30; //48
//const unsigned char C_SE_NB_1 = 0x31; //49
//const unsigned char C_SE_NC_1 = 0x32; //50
//const unsigned char M_EI_NA_1 = 0x46; //70
//const unsigned char M_IC_NA_1 = 0x64; //100
//const unsigned char M_CI_NA_1 = 0x65; //101
//const unsigned char M_RD_NA_1 = 0x66; //102?
//const unsigned char M_CS_NA_1 = 0x67; //103
//const unsigned char M_RP_NA_1 = 0x69; //105
const unsigned char P_ME_NA_1 = 0x6E; //110
//const unsigned char P_ME_NB_1 = 0x6F; //111
//const unsigned char P_ME_NC_1 = 0x70; //112

const unsigned char Local_Power = 0x00;
const unsigned char Local_Switch = 0x01;
const unsigned char Remote_Control = 0x02;

const unsigned char EnableISQ = 0x00;
const unsigned char DisableISQ = 0x80;

//unsigned char InitOverFlag=100;

#define strProtectPara   "Protectpara"
#define strJBEventStartAddr "JB_EVENT_START_ADDR"
#define strJBParaStartAddr "JB_PARA_START_ADDR"
#define strJBEventSet   "JB_EVENT_SET"
#define strJBEventNum   "JB_EVENT_NUM"
#define strJBParaSet    "JB_PARA_SET"
#define strJBParaNum    "JB_PARA_NUM"
#define strDstNode "DstNode"
#define strDstIndex "TabNo"

#define FileNameLen 50

CS104_518::CS104_518(boost::asio::io_service & io_service)
	:CS104(io_service),
	CBF533_CfgFile(io_service)
{
	InitFileOpt();

	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);

}

CS104_518::~CS104_518(void)
{
}

void CS104_518::InitFileOpt()
{
	Total_NUM = 0;
	This_N0 = 0;

	JB_EVENT_START_ADDR_=DEFAULT_JB_EVENT_START_ADDR;
	JB_PARA_START_ADDR_=DEFAULT_JB_PARA_START_ADDR;

	for (int i=0;i<100;i++)
	{
		JB_Event[i]=i;
		JB_Event_Original[i]=i;
		JB_Paratab[i]=i;
		JB_Paratab_Original[i]=i;
		JB_Tabnum[i]=i;
	}
	JB_PARA_NUM_=42;
	ReadProtectValue();
}

void CS104_518::ProcessSubYkSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any indexd)
{
	CS104::ProcessSubYkSig(cmdType,ReturnCode,point,indexd);

	if (!point)
	{
		return;
	}

	if (point->getCommPointType() != PRISTATION_NODE)
	{
		return;
	}
  
	share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(point);
	if (!pristationPtr)
	{
		return;
	}

	switch (cmdType)
	{
	case CALL_EQU_PARA_CON:
		{
			AddSendCmdVal(CALL_EQU_PARA_CON,CALL_EQU_PARA_CON_PRIORITY,pristationPtr,indexd);
		}
		break;

	case CALL_PROVAL_CON:
		{
			//std::cout<<"S104收到CALL_PROVAL_CON指令"<<std::endl;
			if(getLastRecvCmd()==M_EP_TZ_1)
			{
				//std::cout<<"本次由主站召唤... .，添加CALL_JB_PARA_CON指令"<<std::endl;
				AddSendCmdVal(CALL_JB_PARA_CON,CALL_JB_PARA_CON_PRIORITY,pristationPtr,0);
			}
			else if(getLastRecvCmd()==M_FI_RE_3)
			{
				//std::cout<<"本次由后台召唤... .，添加CALL_PROVAL_CON指令"<<std::endl;
				AddSendCmdVal(CALL_PROVAL_CON,CALL_PROVAL_CON_PRIORITY,pristationPtr,indexd);
			}
			else
			{

			}
			setLastRecvCmd(0xff);
		}
		break;
	case CALL_CHTYPE_CON:
		{
			AddSendCmdVal(CALL_CHTYPE_CON,CALL_CHTYPE_CON_PRIORITY,pristationPtr,indexd);
		}
		break;
	case CALL_LINEPARA_CON:
		{
			AddSendCmdVal(CALL_LINEPARA_CON,CALL_LINEPARA_CON_PRIORITY,pristationPtr,indexd);
		}
		break;
	case CALL_INTERFACE_PARA_CON:
		{
			AddSendCmdVal(CALL_INTERFACE_PARA_CON,CALL_INTERFACE_PARA_CON_PRIORITY,pristationPtr,indexd);
		}
		break;

  
	default:
		break;
	}
}

void CS104_518::ProcessSubAliveSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	CProtocol::ProcessSubAliveSig(cmdType,ReturnCode,point,val);

	switch(cmdType)
	{
	case LINE_VAL_VER_QYC:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(LINE_VAL_VER_QYC,LINE_VAL_VER_QYC_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case LINE_DCVAL_VER_SUCESS:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(LINE_DCVAL_VER_SUCESS,LINE_DCVAL_VER_SUCESS_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case HARMONIC_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(HARMONIC_CON,HARMONIC_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case CALL_VALCOEF_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(CALL_VALCOEF_CON,CALL_VALCOEF_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case BOARD_REQ_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(BOARD_REQ_CON,CALL_VALCOEF_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case SIGNAL_RESET_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					if(getLastRecvCmd()==M_EP_TG_1)
					{
                        AddSendCmdVal(JB_SIGNAL_RESET_CON,JB_SIGNAL_RESET_CON_PRIORITY,commPoint,val);
					}
					else if(getLastRecvCmd()==M_SG_RE_3)
					{
						AddSendCmdVal(SIGNAL_RESET_CON,SIGNAL_RESET_CON_PRIORITY,commPoint,val);
					}
					else
					{

					}
					setLastRecvCmd(0xff);
				}
			}
		}
		break;

	case BATTERY_ACTIVE_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(BATTERY_ACTIVE_CON,BATTERY_ACTIVE_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case BATTERY_ACTIVE_OVER_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(BATTERY_ACTIVE_OVER_CON,BATTERY_ACTIVE_OVER_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case LINE_BVAL_VER_QYC:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(LINE_BVAL_VER_QYC,LINE_BVAL_VER_QYC_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case LINE_BVAL_VER_SUCESS:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(LINE_BVAL_VER_SUCESS,LINE_BVAL_VER_SUCESS_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case DOWNLOAD_PARA_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					if(getLastRecvCmd()==M_EP_TH_1)
					{
                       AddSendCmdVal(ACT_JB_PARA_CON,ACT_JB_PARA_CON_PRIORITY,commPoint,val);
					   
					}
					else if(getLastRecvCmd()==M_FI_SE_3)
					{
                        AddSendCmdVal(DOWNLOAD_PARA_CON,DOWNLOAD_PARA_CON_PRIORITY,commPoint,val);
					}
				    else
				    {

				    }
				    setLastRecvCmd(0xff);
				}
			}
		}
		break;

	case CALL_PM_ANG_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(CALL_PM_ANG_CON,CALL_PM_ANG_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;
		
	case DSP_VERSION_INQ_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(DSP_VERSION_INQ_CON,DSP_VERSION_INQ_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;
		
	case EVENT_MESSAGE:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(EVENT_MESSAGE,LINE_BVAL_VER_SUCESS_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case CALL_JB_PARA_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(CALL_JB_PARA_CON,CALL_JB_PARA_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case SEND_JB_PARA_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(SEND_JB_PARA_CON,SEND_JB_PARA_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case ACT_JB_PARA_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(ACT_JB_PARA_CON,ACT_JB_PARA_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case DEACT_JB_PARA_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(DEACT_JB_PARA_CON,DEACT_JB_PARA_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	case JB_SIGNAL_RESET_CON:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					AddSendCmdVal(JB_SIGNAL_RESET_CON,JB_SIGNAL_RESET_CON_PRIORITY,commPoint,val);
				}
			}
		}
		break;
	}
}


int CS104_518::LoadXmlCfg(std::string filename)
{

	CS104::LoadXmlCfg(filename);

	FileSystem::CMarkup xml;

	if (!xml.Load(filename))
	{
		return -1;
	}

	xml.ResetMainPos();
	xml.FindElem();  //root strProtocolRoot
	xml.IntoElem();  //enter strProtocolRoot

	xml.ResetMainPos();
	if (xml.FindElem(strProtectPara))
	{
		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem(strJBEventStartAddr))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short addr = boost::lexical_cast<unsigned short>(strTmp);
				setJB_EVENT_START_ADDR(addr);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setJB_EVENT_START_ADDR(DEFAULT_JB_EVENT_START_ADDR);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strJBParaStartAddr))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short addr = boost::lexical_cast<unsigned short>(strTmp);
				setJB_PARA_START_ADDR(addr);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setJB_PARA_START_ADDR(DEFAULT_JB_PARA_START_ADDR);
			}
		}
		
		
	xml.ResetMainPos();
	if (xml.FindElem(strJBEventSet))
	{
		int sum = 0;
		std::string strTmp = xml.GetAttrib(strJBEventNum);
		boost::algorithm::trim(strTmp);
		try
		{
			sum = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			//do nothing, just ignore strTabDstNum attrib
			std::ostringstream ostr;
			ostr<<e.what();
			sum = 0;
		}

		for (int i=0;i<100;i++)
		{
			JB_Event[i]=0xff;
			JB_Event_Original[i]=0xff;
		}
        
//		if(sum>0)
		{
            xml.IntoElem();
			int count = 0;
			int no=0;
			while (xml.FindElem(strDstNode) /*&& count < sum*/)
			{
				std::string strTmp = xml.GetAttrib(strDstIndex);
				boost::algorithm::trim(strTmp);
				try
				{
					no = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					//do nothing, just ignore strTabDstNum attrib
					std::ostringstream ostr;
					ostr<<e.what();
					no = 0;
				}

				strTmp = xml.GetData();
				short val = -1;
				try
				{
					boost::algorithm::trim(strTmp);
					val = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					xml.OutOfElem();

					std::ostringstream ostr;
					ostr<< "不能将"<<strDstNode<<"转化为数字:"<<e.what()<<std::endl;
				}

				JB_Event[no]=val;
				JB_Event_Original[val]=no;

				count++;
			}
		}
		xml.OutOfElem();
	}

	xml.ResetMainPos();
	if (xml.FindElem(strJBParaSet))
	{
		int sum = 0;
		std::string strTmp = xml.GetAttrib(strJBParaNum);
		boost::algorithm::trim(strTmp);
		try
		{
			sum = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			//do nothing, just ignore strTabDstNum attrib
			std::ostringstream ostr;
			ostr<<e.what();
			sum = 0;
		}

		//		if(sum>0)
		{
			xml.IntoElem();
			int count = 0;
			int no=0;
			while (xml.FindElem(strDstNode) /*&& count < sum*/)
			{
				std::string strTmp = xml.GetAttrib(strDstIndex);
				boost::algorithm::trim(strTmp);
				try
				{
					no = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					//do nothing, just ignore strTabDstNum attrib
					std::ostringstream ostr;
					ostr<<e.what();
					no = 0;
				}

				strTmp = xml.GetData();
				short val = -1;
				try
				{
					boost::algorithm::trim(strTmp);
					val = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					xml.OutOfElem();

					std::ostringstream ostr;
					ostr<< "不能将"<<strDstNode<<"转化为数字:"<<e.what()<<std::endl;
				}

				JB_Paratab[count]=val;//no
				JB_Paratab_Original[no]=count;
				JB_Tabnum[count]=no;

				count++;
			}
			JB_PARA_NUM_=count;
		}
		xml.OutOfElem();
	}

	xml.OutOfElem();
	}

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CS104_518::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();

	CProtocol::SaveXmlCfg(xml);

	xml.AddElem(strInfoAddr);
	bool bSave = false;
	xml.IntoElem();

	xml.OutOfElem();
	if (!bSave)
	{
		xml.RemoveElem();
	}

	xml.OutOfElem();

	xml.Save(filename);
}

//recv frame parse
int CS104_518::ParseFrame_I(unsigned char * buf,size_t exceptedBytes)
{
	//std::cout<<"recv I gram"<<std::endl;

	unsigned short OppositeSendCounter = (buf[2]&0xfe) + (buf[3]*0x100); //对端I格式报文发送计数器
	unsigned short OppositeRecvCounter = (buf[4]&0xfe) + (buf[5]*0x100); //对端I格式报文接收计数器

	if (CheckIFrameRecvCounter(OppositeSendCounter) || CheckIFrameSendCounter(OppositeRecvCounter))
	{
		ReConnnectChannel();
		return -1;
	}

	size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识
	size_t TransReason = BufToVal(&buf[TransReasonLocation_],TransReasonLength_) & 0xff;  //传送原因
	size_t Addr = BufToVal(&buf[AsduAddrLocation_],AsduAddrLength_);               //RTU地址
	unsigned char Data_Code = buf[DataLocation_] & 0x80;

	int pristationIndex = getCommPointIndexByAddrCommType(PRISTATION_NODE,Addr);
	share_pristation_ptr pristationPtr;
	if (pristationIndex >= 0)
	{
		setLastRecvPointIndex(pristationIndex);
		pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(getCommPoint(pristationIndex).lock());
	}

	if (!pristationPtr)
	{
		std::ostringstream ostr;
		ostr<<"S104规约不能根据接收报文中地址匹配pristation ptr,这帧报文将不会被解析。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (FrameType)
	{
		/*	case C_SE_NA_1:
		if (TransReason = trans_act)
		{
		//			ParseSynTime(buf,pristationPtr);
		}
		else
		{
		std::ostringstream ostr;
		ostr<<"设点命令报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		}
		break;

		case C_SE_NB_1:
		if (TransReason = trans_act)
		{
		//			ParseSynTime(buf,pristationPtr);
		}
		else
		{
		std::ostringstream ostr;
		ostr<<"设点命令报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		}
		break;

		case C_SE_NC_1:
		if (TransReason = trans_act)
		{
		//			ParseSynTime(buf,pristationPtr);
		}
		else
		{
		std::ostringstream ostr;
		ostr<<"设点命令报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		}
		break;
		*/
		
	case M_LV_VR_3://定值校验
		{
			ParseLineValVer(buf,pristationPtr);
		}
		break;

	case M_DC_VR_3://直流量定值校验
		{
			ParseDcValVer(buf,pristationPtr);
		}
		break;
	case M_PM_CA_3://召唤绝对相角
		{
			ParseCallPM(buf,pristationPtr);
		}
		break;
	case M_CA_CR_3://0xB8 召唤CRC
		{
			ParseCallCRC(buf,pristationPtr);
		}
		break;
	case M_HA_MO_3://召唤谐波
		{

			ParseHarmonicAck(buf,pristationPtr);
		}
		break;
	case M_VA_VR_3://召唤定值系数 0xA2
		{
//			std::cout<<"收到召唤定值系数指令"<<std::endl;
			ParseCallVallCoef(buf,pristationPtr);
		}
		break;
	case M_ZD_RE_3://装置复位
		{
			//         std::cout<<"S101装置复位指令... "<<std::endl; 
			ParseRbootAck(buf,pristationPtr);
		}
		break;

	case M_SG_RE_3://远方信号复归
		{
			//	     std::cout<<"S101远方信号复归指令... "<<std::endl; 
			//int line_no=Addr;//调试用
			//if (Addr>=10) line_no=255;//tiaoshi
			//CCmd cmd(SIGNAL_RESET,SIGNAL_RESET_PRIORITY,pristationPtr);
			//pristationPtr->AddBF533Cmd(0,cmd);
			//ConnectSubYkSig(pristationPtr);//与消息进行连接
			ParseRemoteSingalReset(buf,pristationPtr);
			setLastRecvCmd(M_SG_RE_3);
		}
		break;
		
	case M_CA_TM_3://时钟校核
		{
			AddSendCmdVal(CALL_TIME_CON,CALL_TIME_CON_PRIORITY,pristationPtr);
		}
		break;
		
	case M_VA_DL_3://下装定值系数
		{
			ParseDownLoadLineHVal(buf,pristationPtr);//下装定值系数
		}
		break;
	case M_BD_RQ_3://板件查询
		{

			CCmd cmd(BOARD_REQ_ACK,BOARD_REQ_ACK_PRIORITY,pristationPtr);
			pristationPtr->AddBF533Cmd(0,cmd);
			//ConnectSubYkSig(pristationPtr);//与消息进行连接
		}
		break;

	case M_AC_BT_3://电池活化
		{
			//pristationPtr->AddNormlCmd( 0,BATTERY_ACTIVE_,0);//添加指令到队列,Val为结构体

			CCmd cmd(BATTERY_ACTIVE,BATTERY_ACTIVE_PRIORITY,pristationPtr);
			pristationPtr->AddBF533Cmd(0,cmd);

			//ConnectSubYkSig(pristationPtr);//与消息进行连接
		}
		break;

	case M_OV_BT_3://电池活化结束
		{
			//pristationPtr->AddNormlCmd( 0,BATTERY_ACTIVE_OVER_,0);//添加指令到队列,Val为结构体

			CCmd cmd(BATTERY_ACTIVE_OVER,BATTERY_ACTIVE_OVER_PRIORITY,pristationPtr);
			pristationPtr->AddBF533Cmd(0,cmd);

			//ConnectSubYkSig(pristationPtr);//与消息进行连接
		}
		break;

	case M_RE_VR_3:  //0xB4
		{
			ParseCpuVerReq(buf,pristationPtr);
		}
		break;

	case M_DL_BV_3://0xB5 保护电流校验定值下装
		{
			ParseBValVer(buf,pristationPtr);
		}
		break;

	case M_EP_TZ_1:
//		if (TransReason == trans_req)
		{
			ParseCallJBPara(buf,pristationPtr);
			setLastRecvCmd(M_EP_TZ_1);
		}
		/*else
		{
			std::ostringstream ostr;
			ostr<<"召唤继电保护定值，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}*/
		break;

	case M_EP_TS_1:
//		if (TransReason == trans_act)
		{
			ParseSendJBPara(buf,pristationPtr);
		}
		/*else
		{
			std::ostringstream ostr;
			ostr<<"下装继电保护定值，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}*/
		break;

	case M_EP_TH_1:
		if (TransReason == trans_act)
		{
			ParseActALLPara(buf,pristationPtr);
			setLastRecvCmd(M_EP_TH_1);
		}
		else if (TransReason == trans_deact)
		{
			ParseDeacALLtPara(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"激活继电保护定值，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_EP_TG_1:
		//if (TransReason == trans_act)
		{
			ParseSignalReset(buf,pristationPtr);
			setLastRecvCmd(M_EP_TG_1);
		}
		/*else
		{
			std::ostringstream ostr;
			ostr<<"激活继电保护信号复归，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}*/
		break;

	case P_ME_NA_1:
		if (TransReason == trans_act)
		{
			//ParseYcValuePara(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"归一化测量值参数报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	default:
		{
			return CS104::ParseFrame_I(buf,exceptedBytes);
		}
		break;
	}

	IGramFrameRecv();
	ClearCounterKI();
	ResetTimerIGramFrameRecv(pristationPtr,true);

	if (CheckCounterWI())
	{
		AddSendCmdVal(S_GRAM_FRAME,S_GRAM_FRAME_PRIORITY,pristationPtr);
	}

	return pristationIndex;
}

int CS104_518::ParseFileFrame(unsigned char * buf, share_pristation_ptr pristationPtr)
{
	//    std::cout<<"进入文件处理接口... "<<std::endl; 
	
	IGramFrameRecv();
	ClearCounterKI();
	ResetTimerIGramFrameRecv(pristationPtr,true);

	if (CheckCounterWI())
	{
		AddSendCmdVal(S_GRAM_FRAME,S_GRAM_FRAME_PRIORITY,pristationPtr);
	}

	size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识
	size_t TransReason = BufToVal(&buf[TransReasonLocation_],TransReasonLength_);  //传送原因
	unsigned char Data_Code = buf[DataLocation_] & 0x80;

	switch (FrameType)
	{
	case M_FI_SE_3:  //发送文件
		{
			setLastRecvCmd(M_FI_SE_3);
			if(TransReason == trans_act)//发送文件激活 0x06
			{
				 
				//		   std::cout<<"发送文件名... .."<<std::endl; 
				ParseSendFileAck(buf,pristationPtr);
			}

			else if(TransReason == trans_deact)//传送原因为0x08
			{
				 
				//		  std::cout<<"发送文件体... ..."<<std::endl; 
				ParseRecvFile(buf,pristationPtr);
			}
		}
		break;
	case M_FI_RE_3:
		{
			setLastRecvCmd(M_FI_RE_3);
			if(TransReason == trans_act)//召唤文件激活 0x06
			{
				 
//				std::cout<<"处理召唤文件名称指令. ..."<<std::endl; 
				ParseCallFileAck(buf,pristationPtr);
			}
		}
		break;


	default :
		{
			std::ostringstream ostr;
			ostr<<"接收报文错误，未定义的报文类型 FRAME_TYPE ="<<FrameType<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;
	}
	return 0;
}

int CS104_518::ParseLineValVer(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 15;//12;  //线路号所在位置

	DataBase::stLine_Val Val;  

	Val.Line_no = BufToVal(&buf[Location ++],1);  //读取线路号

	{
		Val.Flag_Ua = BufToVal(&buf[Location ++],1);
		Val.D_Value_Ua = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_Ub = BufToVal(&buf[Location ++],1);
		Val.D_Value_Ub = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_Uc = BufToVal(&buf[Location ++],1);
		Val.D_Value_Uc = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_CIa = BufToVal(&buf[Location ++],1);
		Val.D_Value_CIa = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_CIb = BufToVal(&buf[Location ++],1);
		Val.D_Value_CIb = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_CIc = BufToVal(&buf[Location ++],1);
		Val.D_Value_CIc = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_BIa = BufToVal(&buf[Location ++],1);
		Val.D_Value_BIa = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_BIb = BufToVal(&buf[Location ++],1);
		Val.D_Value_BIb = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_BIc = BufToVal(&buf[Location ++],1);
		Val.D_Value_BIc = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_U0 = BufToVal(&buf[Location ++],1);
		Val.D_Value_U0 = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_I0 = BufToVal(&buf[Location ++],1);
		Val.D_Value_I0 = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_Angle_UaIa = BufToVal(&buf[Location ++],1);
		Val.Angle_UaIa = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_Angle_UbIb = BufToVal(&buf[Location ++],1);
		Val.Angle_UbIb = BufToVal(&buf[Location],2); 
		Location += 2;
		Val.Flag_Angle_UcIc = BufToVal(&buf[Location ++],1);
		Val.Angle_UcIc = BufToVal(&buf[Location],2); 
	}   

	//pristationPtr->AddNormlCmd( 0,DOWNLOAD_LINE_VAL_,Val);//添加指令到队列,Val为结构体

	CCmd cmd(DOWNLOAD_LINE_VAL,DOWNLOAD_LINE_VAL_PRIORITY,pristationPtr,Val);
	pristationPtr->AddBF533Cmd(0,cmd);

	//ConnectSubYkSig(pristationPtr);//与消息进行连接
	ResetTimerValVer(pristationPtr,true,0);

	return 0;
}

int CS104_518::ParseHarmonicAck(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 15;  //线路号所在位置
	DataBase::stHarmonic Val;

	Val.Line_no = BufToVal(&buf[Location ++],1);  //读取线路号
	Val.harmonic_no = BufToVal(&buf[Location ++],1);

	//pristationPtr->AddNormlCmd( 0,HARMONIC_ACK_,Val);//添加指令到队列,Val为结构体

	CCmd cmd(HARMONIC_ACK,HARMONIC_ACK_PRIORITY,pristationPtr,Val);
	pristationPtr->AddBF533Cmd(0,cmd);

	//ConnectSubYkSig(pristationPtr);//与消息进行连接

	return 0;
}

int CS104_518::ParseSendFileAck(unsigned char * buf,share_pristation_ptr pristationPtr)//收到文件名报文后回复确认
{
	int Location = 15;   //文件名第一个字节
	char Name[FileNameLen];
	int Namelen_;
	memset(Name,0,FileNameLen);

	Namelen_ = (BufToVal(&buf[0],2) - 16);//文件名所占的字节数
	//memset(NameBuf,0,20);

	//for(int i = 0;i < Namelen_ ;i ++)//读出文件名
	//{
	//	NameBuf[i] = buf[Location ++];
	//}

	for(int i = 0;i < Namelen_ ;i ++)//读出文件名
	{
		Name[i] = buf[Location ++];
	}

	std::ostringstream filename;
	filename<<Name;
	FileHandleBegain(filename.str());//开始调用CFileHandle类进行文件处理

	//FileLength_ = (BufToVal(&buf[Location],3));

	FileHandleSetTotalLength((BufToVal(&buf[Location],3))); //写入总文件长度

//	std::cout<<"文件名和长度分别为："<< "文件名："<< Name <<",长度："<< FileLength_<<",文件名长度NameLength_："<< Namelen_<<std::endl; 

	AddSendCmdVal(SEND_FILENAME_CON,SEND_FILENAME_CON_PRIORITY,pristationPtr);
	return 0;
}

int CS104_518::ParseRecvFile(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 12; //文件第一个字节
	int val;

	Total_NUM = buf[Location ++];
	This_N0 = buf[Location ++];

	This_Length_ = (BufToVal(&buf[0],2) - 13);
    Location ++;

    //调用CFileHandle对文件近处理
	FileHandleOutFile(&buf[Location],This_Length_);

	//for(i = 0;i< This_Length_;i ++)
	//{
	//	FileDataBuf[FileDataPtr_ ++] = buf[Location ++];
	//}

	std::ostringstream ostr;
	ostr<<"收到文件，共"<< Total_NUM <<"帧，本次为"<< This_N0 << "帧，数据长度" << This_Length_ <<std::endl; 
	AddStatusLogWithSynT(ostr.str());

	if(Total_NUM == This_N0)
	{
		val = 15;//0x0F表示传送成功
//		std::cout<<FileDataBuf<<std::endl; 
		//SaveFile(pristationPtr);

		//调用CFileHandle对文件近处理
		FileHandleWrite();

		AddDownloadCmd(pristationPtr);

		FileHandleFinish();

		//FileDataPtr_ = 0;
	}
	else 
	{
		val = 10; //0x0a表示确认
	}
	//  std::cout<<"开始添加指令到队列 ."<<std::endl; 
	AddSendCmdVal(SEND_FILEBODY_CON,SEND_FILEBODY_CON_PRIORITY,pristationPtr,val);
	return 0;
}

int CS104_518::ParseCallFileAck(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 15;   //文件名第一个字节
	char Val=0x05;
	char Name[FileNameLen];
	int Namelen_;
	memset(Name,0,FileNameLen);

	Namelen_ = (BufToVal(&buf[0],2) - 13);//文件名所占的字节数
	//memset(NameBuf,0,20);
	for(int i = 0;i < Namelen_ ;i ++)//读出文件名
	{
		Name[i] = buf[Location ++];
	}

	std::ostringstream filename;
	filename<<Name;
	FileHandleBegain(filename.str());//开始调用CFileHandle类进行文件处理

	std::ostringstream ostr;
	ostr<<"需要召唤的文件名为："<< Name <<",文件名长度为："<<Namelen_<<std::endl; 
	AddStatusLogWithSynT(ostr.str());


	if((strcmp(BFBasePara,Name)==0)||(strcmp(BFDevSanyaoPara,Name)==0))
	{
		//pristationPtr->AddNormlCmd( 0,CALL_EQU_PARA_ACT_,0);//添加指令到队列,通知对下规约召唤装置参数

		CCmd cmd(CALL_EQU_PARA_ACT,CALL_EQU_PARA_ACT_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);

		ConnectSubYkSig(pristationPtr);//与消息进行连接
		ResetTimerCallFileDisConect(pristationPtr,true,0);
	}
	else if(strcmp(BFProtectValPara,Name)==0)//召唤保护定值
	{
		//   std::cout<<"开始向BF533添加指令 ... "<<std::endl; 
		//pristationPtr->AddNormlCmd( 0,CALL_PROVAL_ACK_,0);//添加指令到队列,通知对下规约召唤装置参数

		CCmd cmd(CALL_PROVAL_ACK,CALL_PROVAL_ACK_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);

		ConnectSubYkSig(pristationPtr);//与消息进行连接
		ResetTimerCallFileDisConect(pristationPtr,true,60);
	}
	else if(strcmp(BFChanneltypePara,Name)==0)
	{
		//   std::cout<<"开始向BF533添加指令 ... "<<std::endl; 
		//pristationPtr->AddNormlCmd( 0,CALL_CHTYPE_ACK_,0);//添加指令到队列,通知对下规约召唤装置参数

		CCmd cmd(CALL_CHTYPE_ACK,CALL_CHTYPE_ACK_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);

		ConnectSubYkSig(pristationPtr);//与消息进行连接
		ResetTimerCallFileDisConect(pristationPtr,true,0);
	}
	else if(strcmp(BFLinePara,Name)==0)
	{
		//   std::cout<<"开始向BF533添加指令 ... "<<std::endl; 
		//pristationPtr->AddNormlCmd( 0,CALL_LINEPARA_ACK_,0);//添加指令到队列,通知对下规约召唤装置参数

		CCmd cmd(CALL_LINEPARA_ACK,CALL_LINEPARA_ACK_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);

		ConnectSubYkSig(pristationPtr);//与消息进行连接
		ResetTimerCallFileDisConect(pristationPtr,true,0);
	}
	else if(strcmp(BFInterfacePara,Name)==0)
	{
		//   std::cout<<"开始向BF533添加指令 ... "<<std::endl; 
		CCmd cmd(CALL_INTERFACE_PARA,CALL_INTERFACE_PARA_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);//添加指令到队列,通知对下规约召唤装置参数

		ConnectSubYkSig(pristationPtr);//与消息进行连接
		ResetTimerCallFileDisConect(pristationPtr,true,0);
	}
	else
	{
         //调用CFileHandle对文件近处理
		if (FileHandleRead() > 0)
		{
			Val = 0x07;
			Total_NUM = ((FileHandleGetTotalLength() - 1) / FileLen) + 1;
		}
		else
		{
			Val = 0x05;
		}

		//if(GetFileData(pristationPtr) == 0)
		//{
		//	Val = 0x07;
		//}
		//else
		//{
		//	Val = 0x05;
		//}
		AddSendCmdVal(CALL_FILENAME_CON,CALL_FILENAME_CON_PRIORITY,pristationPtr,Val);//添加回复指令
	}

	return 0;
}

int CS104_518::ParseCallVallCoef(unsigned char * buf,share_pristation_ptr pristationPtr)//召唤定值系数
{
	int Location = 15;   //文件名第一个字节
	DataBase::stLine_ValCoef Val;

	Val.Line_no = BufToVal(&buf[Location],1);  //读取线路号

//	pristationPtr->AddNormlCmd( 0,CALL_VALCOEF_ACK_,Val);//添加指令到队列,Val为结构体

	CCmd cmd(CALL_VALCOEF_ACK,CALL_VALCOEF_ACK_PRIORITY,pristationPtr,Val);
	pristationPtr->AddBF533Cmd(0,cmd);

	//ConnectSubYkSig(pristationPtr);//与消息进行连接

	ResetTimerCallHVal(pristationPtr,true,0);
	return 0;

}

int CS104_518::ParseDownLoadLineHVal(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 15;   //文件名第一个字节
	DataBase::stLine_ValCoef HVal;

	HVal.Line_no = BufToVal(&buf[Location ++],1);  //读取线路号
	HVal.Flag_Ua = BufToVal(&buf[Location ++],1);
	HVal.H_Value_Ua = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_Ub = BufToVal(&buf[Location ++],1);
	HVal.H_Value_Ub = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_Uc = BufToVal(&buf[Location ++],1);
	HVal.H_Value_Uc = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_CIa = BufToVal(&buf[Location ++],1);
	HVal.H_Value_CIa = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_CIb = BufToVal(&buf[Location ++],1);
	HVal.H_Value_CIb = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_CIc = BufToVal(&buf[Location ++],1);
	HVal.H_Value_CIc = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_BIa = BufToVal(&buf[Location ++],1);
	HVal.H_Value_BIa = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_BIb = BufToVal(&buf[Location ++],1);
	HVal.H_Value_BIb = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_BIc = BufToVal(&buf[Location ++],1);
	HVal.H_Value_BIc = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_U0 = BufToVal(&buf[Location ++],1);
	HVal.H_Value_U0 = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_I0 = BufToVal(&buf[Location ++],1);
	HVal.H_Value_I0 = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_Angle_UaIa = BufToVal(&buf[Location ++],1);
	HVal.Angle_UaIa = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_Angle_UbIb = BufToVal(&buf[Location ++],1);
	HVal.Angle_UbIb = BufToVal(&buf[Location ],2);
	Location += 2;
	HVal.Flag_Angle_UcIc = BufToVal(&buf[Location ++],1);
	HVal.Angle_UcIc = BufToVal(&buf[Location ],2);

//	pristationPtr->AddNormlCmd( 0,DOWNLOAD_LINE_COEF_,HVal);//添加指令到队列,Val为结构体

	CCmd cmd(DOWNLOAD_LINE_COEF,DOWNLOAD_LINE_COEF_PRIORITY,pristationPtr,HVal);
	pristationPtr->AddBF533Cmd(0,cmd);


	//ConnectSubYkSig(pristationPtr);//与消息进行连接
	AddSendCmdVal(DOWNLOAD_LINE_CON,DOWNLOAD_LINE_CON_PRIORITY,pristationPtr,HVal);
	return 0;
}

int CS104_518::ParseRbootAck(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	AddSendCmdVal(REBOOT_CON,REBOOT_CON_PRIORITY,pristationPtr);
	return 0;
}

int CS104_518::ParseCpuVerReq(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	CCmd cmd(DSP_VERSION_INQ,DSP_VERSION_INQ_PRIORITY,pristationPtr,0);
	pristationPtr->AddBF533Cmd(0,cmd);//添加指令到队列,Val为结构体
	//AddSendCmdVal(DSP_VERSION_INQ_CON,DSP_VERSION_INQ_CON_PRIORITY,pristationPtr);//AddWaitCmdVal
	return 0;
}

int CS104_518::ParseBValVer(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 15;  
	DataBase::stLine_BCurVal Val;
	Val.Line_No = BufToVal(&buf[Location],1);
	Location += 1;
	Val.D_Val_BI1 = BufToVal(&buf[Location],2);
	Location += 2;
	Val.D_Val_BI2 = BufToVal(&buf[Location ],2);
	Location += 2;
	Val.D_Val_BI3 = BufToVal(&buf[Location ],2);

	//std::cout<<"S101收到的保护电流定值为："<<"Val.D_Val_BI1:"<<Val.D_Val_BI1<<" Val.D_Val_BI2:"<<Val.D_Val_BI2<<" Val.D_Val_BI3:"<<Val.D_Val_BI3<<std::endl;

//	pristationPtr->AddNormlCmd( 0,DOWNLOAD_LINE_BVAL_,Val);//添加指令到队列,Val为结构体

	CCmd cmd(DOWNLOAD_LINE_BVAL,DOWNLOAD_LINE_BVAL_PRIORITY,pristationPtr,Val);
	pristationPtr->AddBF533Cmd(0,cmd);

	//ConnectSubYkSig(pristationPtr);//与消息进行连接

	ResetTimerBCurVer(pristationPtr,true,0);

	return 0;
}

int CS104_518::ParseDcValVer(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 15;  
	DataBase::stDCVerVal Val;
	Val.ChannelNo = BufToVal(&buf[Location],1);
	Location += 1;
	Val.V_Val_DC = BufToVal(&buf[Location],2);

//	pristationPtr->AddNormlCmd( 0,LINE_DCVAL_VER_,Val);//添加指令到队列,Val为结构体

	CCmd cmd(LINE_DCVAL_VER,LINE_DCVAL_VER_PRIORITY,pristationPtr,Val);
	pristationPtr->AddBF533Cmd(0,cmd);

	//ConnectSubYkSig(pristationPtr);//与消息进行连接

	ResetTimerDcVer(pristationPtr,true,0);
	return 0;
}

int CS104_518::ParseCallPM(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 15;

	DataBase::stPMPara Val;

	int LineNo = BufToVal(&buf[Location],1);

	Val.LineNo = LineNo;

	CCmd cmd(CALL_PM_ANG,CALL_PM_ANG_PRIORITY,pristationPtr,Val);
	pristationPtr->AddBF533Cmd(0,cmd);//添加指令到队列,Val为结构体

	ResetTimerCallPm(pristationPtr,true,0);


	return 0;
}

int CS104_518::ParseCallCRC(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	AddSendCmdVal(CALL_CRC_CON,CALL_CRC_CON_PRIORITY,pristationPtr);
	return 0;
}

int CS104_518::ParseRemoteSingalReset(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 15;
	int LineNo_ = BufToVal(&buf[Location],1);
	CCmd cmd(SIGNAL_RESET,SIGNAL_RESET_PRIORITY,pristationPtr,LineNo_);//255 reset all
	pristationPtr->AddBF533Cmd(0,cmd);
	return 0;
}

void CS104_518::AddDownloadCmd(share_pristation_ptr pristationPtr)
{
	//char Name[FileNameLen];
	//memset(Name,0,FileNameLen);
	std::string Name;
	Name = FileHandleGetFileName();

	if(strcmp(BFBasePara,Name.c_str())==0)
	{
		////std::cout<<"收到文件为：BasePara.xml" <<std::endl; 
		//pristationPtr->AddNormlCmd( 0,DOWNLOAD_EQU_PARA_,0);//添加下装装置参数指令到队列
		//ConnectSubYkSig(pristationPtr);//与消息进行连接
		//ResetTimerDowloadPara(pristationPtr,true,0);
	}
	else if(strcmp(BFDevSanyaoPara,Name.c_str())==0)
	{
		//   std::cout<<"收到文件为：DevSanyaoPara.xml,开始添加指令... ..."<<std::endl; 
//		pristationPtr->AddNormlCmd( 0,DOWNLOAD_EQU_PARA_,0);//添加下装装置参数指令到队列

		CCmd cmd(DOWNLOAD_EQU_PARA,DOWNLOAD_EQU_PARA_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);

		//ConnectSubYkSig(pristationPtr);//与消息进行连接
		ResetTimerDowloadPara(pristationPtr,true,0);
	}
	else if(strcmp(BFChanneltypePara,Name.c_str())==0)
	{
		//   std::cout<<"收到文件为：ChanneltypePara.xml" <<std::endl;
//		pristationPtr->AddNormlCmd( 0,DOWNLOAD_CHANNEL_PARA_,0);

		CCmd cmd(DOWNLOAD_CHANNEL_PARA,DOWNLOAD_CHANNEL_PARA_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);

		//ConnectSubYkSig(pristationPtr);//与消息进行连接
		ResetTimerDowloadPara(pristationPtr,true,0);
	}
	else if(strcmp(BFInterfacePara,Name.c_str())==0)
	{
		//  std::cout<<"收到文件为：InterfacePara.xml"<<std::endl; 
//		pristationPtr->AddNormlCmd( 0,SET_BF518_PARA_,0);

		CCmd cmd(SET_BF518_PARA,SET_BF518_PARA_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);

		//ConnectSubYkSig(pristationPtr);//与消息进行连接
		//	ResetTimerDowloadPara(pristationPtr,true,0);
	}
	else if(strcmp(BFLinePara,Name.c_str())==0)
	{
		//  std::cout<<"收到文件为：LinePara.xml"<<std::endl; 
//		pristationPtr->AddNormlCmd( 0,DOWNLOAD_CHANNEL_COMREL_,0);

		CCmd cmd(DOWNLOAD_CHANNEL_COMREL,DOWNLOAD_CHANNEL_COMREL_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);

		//ConnectSubYkSig(pristationPtr);//与消息进行连接
		ResetTimerDowloadPara(pristationPtr,true,0);
	}
	else if(strcmp(BFMaster1YcZFTab,Name.c_str())==0)
	{
		//  std::cout<<"收到文件为：Master1YcZFTab.xml" <<std::endl; 
	}
	else if(strcmp(BFMaster1YxZFTab,Name.c_str())==0)
	{
		//  std::cout<<"收到文件为：Master1YxZFTab.xml"<<std::endl; 
	}
	else if(strcmp(BFMaster2YcZFTab,Name.c_str())==0)
	{
		//std::cout<<"收到文件为：Master2YcZFTab.xml" <<std::endl; 
	}
	else if(strcmp(BFMaster2YxZFTab,Name.c_str())==0)
	{
		//  std::cout<<"收到文件为：Master2YxZFTab.xml"<<std::endl; 
	}
	else if(strcmp(BFProtectValPara,Name.c_str())==0)
	{
		//   std::cout<<"收到文件为：ProtectValPara.xml"<<std::endl; 
//		pristationPtr->AddNormlCmd( 0,DOWNLOAD_DELAY_VAL_,0);

		CCmd cmd(DOWNLOAD_DELAY_VAL,DOWNLOAD_DELAY_VAL_PRIORITY,pristationPtr,0xFF);//0=>0xff ZHANGZHIHUA 20100508 此处必须传0
		pristationPtr->AddBF533Cmd(0,cmd);

		//ConnectSubYkSig(pristationPtr);//与消息进行连接
		ResetTimerDowloadPara(pristationPtr,true,0);
	}
}

//召唤保护定值
int CS104_518::ParseCallJBPara(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 12;  
	
	if(BufToVal(&buf[Location],2)==0xffff) JB_Para_No_=0xffff;
	else JB_Para_No_ = BufToVal(&buf[Location],2)-JB_PARA_START_ADDR_;
	Location += 2;
	JB_Line_No_ = BufToVal(&buf[Location],1);
	
	//   std::cout<<"开始向BF533添加指令 ... "<<std::endl; 
//		pristationPtr->AddNormlCmd( 0,CALL_PROVAL_ACK_,0);//添加指令到队列,通知对下规约召唤装置参数
   if(JB_Line_No_<BF533Base.LineNum)
   {
	CCmd cmd(CALL_PROVAL_ACK,CALL_PROVAL_ACK_PRIORITY,pristationPtr);
	pristationPtr->AddBF533Cmd(0,cmd);
   }
   else
   {
	   DataBase::stJBPARA Val;
	   Val.ParaNo[0]=JB_Para_No_;
       Val.LineNo[0]=JB_Line_No_-BF533Base.LineNum;
	   CCmd cmd(CALL_JB_PARA_ACT,CALL_JB_PARA_ACT_PRIORITY,pristationPtr,Val/*(JB_Line_No_-BF533Base.LineNum)*/);
	   //pristationPtr->AddBF533Cmd(0,cmd);
	   pristationPtr->AddSpeCmd(0,cmd);
   }
		
	ConnectSubYkSig(pristationPtr);//与消息进行连接
//	DisconnectSubYkSig(pristationPtr);//与消息进行连接

//	std::cout<<"ParseCallJBPara..."<<std::endl;
//	AddSendCmdVal(CALL_JB_PARA_CON,CALL_JB_PARA_CON_PRIORITY,pristationPtr);

	return 0;
}

//下装保护定值
int CS104_518::ParseSendJBPara(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	ResetTimerSetJBPara(pristationPtr,true,0);
	int Location = 12;  
	int num=BufToVal(&buf[7],1);

	JB_Para_No_=num;
	int value = 0;

	JB_Line_No_ = BufToVal(&buf[Location+2],1);

	for(int i=0;i<num;i++)
	{
		value = BufToVal(&buf[Location],2)-JB_PARA_START_ADDR_;
		JB_Parasave[i]=value;
		Location += 2;
		//	JB_Line_No_ = BufToVal(&buf[Location],1);
		Location++;
		JB_Para[JB_Line_No_][JB_Paratab[JB_Paratab_Original[value]]]=BufToVal(&buf[Location],2);
		Location+=2;
	}

	if(JB_Line_No_<BF533Base.LineNum)
	{
    
//	for(int i=0;i<num;i++)
//	{
//	value = BufToVal(&buf[Location],2)-JB_PARA_START_ADDR_;
//	JB_Parasave[i]=value;
//	Location += 2;
////	JB_Line_No_ = BufToVal(&buf[Location],1);
//    Location++;
//    JB_Para[JB_Line_No_][JB_Paratab[JB_Paratab_Original[value]]]=BufToVal(&buf[Location],2);
//	Location+=2;
//	}
	((BF533Base.Flag_Pro_Rst))=JB_Para[JB_Line_No_][41]*60;
	((BF533Base.Pro_Rst_Time))=JB_Para[JB_Line_No_][42]*60;
	WriteProtectValue();

	AddSendCmdVal(SEND_JB_PARA_CON,SEND_JB_PARA_CON_PRIORITY,pristationPtr);
	}

	else
	{
		Location =12;
		DataBase::stJBPARA Val;
		Val.Num=num;
		for(int i=0;i<num;i++)
		{
			Val.ParaNo[i]=BufToVal(&buf[Location],2);
			Location+=2;
			Val.LineNo[i]=BufToVal(&buf[Location],1)-BF533Base.LineNum;
			Location++;
			Val.Value[i] =BufToVal(&buf[Location],2);
			Location+=2;
		}

		CCmd cmd(SEND_JB_PARA_ACT,SEND_JB_PARA_ACT_PRIORITY,pristationPtr,Val);
		//pristationPtr->AddBF533Cmd(0,cmd);
		pristationPtr->AddSpeCmd(0,cmd);
	}

	return 0;
}

//激活保护定值
int CS104_518::ParseActALLPara(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	
  //   std::cout<<"开始向BF533添加指令 ... "<<std::endl; 
//	pristationPtr->AddNormlCmd( 0,DOWNLOAD_DELAY_VAL_,0);//添加指令到队列,通知对下规约召唤装置参数
	ResetTimerSetJBPara(pristationPtr,false,0);

	if(JB_Line_No_<BF533Base.LineNum)
	{
		CCmd cmd(DOWNLOAD_DELAY_VAL,DOWNLOAD_DELAY_VAL_PRIORITY,pristationPtr,JB_Line_No_);//JB_Line_No_=>0  ZHANGZHIHUA 20100508 此处必须传0
		pristationPtr->AddBF533Cmd(0,cmd);

	}
	else
	{
		CCmd cmd(ACT_JB_PARA_ACT,ACT_JB_PARA_ACT_PRIORITY,pristationPtr,(JB_Line_No_-BF533Base.LineNum));
		//pristationPtr->AddBF533Cmd(0,cmd);
		pristationPtr->AddSpeCmd(0,cmd);
	}
		
//    Write_ProtectValPara();
//	ConnectSubYkSig(pristationPtr);//与消息进行连接
//	DisconnectSubYkSig(pristationPtr);//与消息进行连接


//	AddSendCmdVal(ACT_JB_PARA_CON,ACT_JB_PARA_CON_PRIORITY,pristationPtr);

	return 0;
}

//撤销保护定值
int CS104_518::ParseDeacALLtPara(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	
  //   std::cout<<"开始向BF533添加指令 ... "<<std::endl; 
//		pristationPtr->AddNormlCmd( 0,CALL_PROVAL_ACK_,0);//添加指令到队列,通知对下规约召唤装置参数
	ResetTimerSetJBPara(pristationPtr,false,0);

	if(JB_Line_No_<BF533Base.LineNum)
	{
		CCmd cmd(CALL_PROVAL_ACK,CALL_PROVAL_ACK_PRIORITY,pristationPtr);
		pristationPtr->AddBF533Cmd(0,cmd);

	}
	else
	{
		CCmd cmd(DEACT_JB_PARA_ACT,DEACT_JB_PARA_ACT_PRIORITY,pristationPtr,(JB_Line_No_-BF533Base.LineNum));
		//pristationPtr->AddBF533Cmd(0,cmd);
		pristationPtr->AddSpeCmd(0,cmd);
	}
	
//	ReadProtectValue();

	//ConnectSubYkSig(pristationPtr);//与消息进行连接
	//DisconnectSubYkSig(pristationPtr);//与消息进行连接

	AddSendCmdVal(DEACT_JB_PARA_CON,DEACT_JB_PARA_CON_PRIORITY,pristationPtr);

	return 0;
}

//
int CS104_518::ParseSignalReset(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int Location = 12;  
	
//	JB_Para_No_ = BufToVal(&buf[Location],2)-JB_PARA_START_ADDR_;
	Location += 2;//3
	JB_Line_No_ = BufToVal(&buf[Location],1);
	
	//   std::cout<<"开始向BF533添加指令 ... "<<std::endl; 
//		pristationPtr->AddNormlCmd( 0,SIGNAL_RESET_,0);//添加指令到队列,通知对下规约召唤装置参数

	if(JB_Line_No_<BF533Base.LineNum)
	{
		CCmd cmd(SIGNAL_RESET,SIGNAL_RESET_PRIORITY,pristationPtr,JB_Line_No_);
		pristationPtr->AddBF533Cmd(0,cmd);

	}
	else
	{
		CCmd cmd(JB_SIGNAL_RESET_ACT,JB_SIGNAL_RESET_ACT_PRIORITY,pristationPtr,(JB_Line_No_-BF533Base.LineNum));
		//pristationPtr->AddBF533Cmd(0,cmd);
		pristationPtr->AddSpeCmd(0,cmd);
	}
		

	//ConnectSubYkSig(pristationPtr);//与消息进行连接
	//DisconnectSubYkSig(pristationPtr);//与消息进行连接

//	AddSendCmdVal(JB_SIGNAL_RESET_CON,JB_SIGNAL_RESET_CON_PRIORITY,pristationPtr);

	return 0;
}


int CS104_518::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	if ((buf[0] == 0x88)  )
	{
		exceptedBytes = BufToVal(&buf[1],2)+3;

		return 0;
	}

	return CS104::CheckFrameHead(buf,exceptedBytes);
}

int CS104_518::getAddrByRecvFrame(unsigned char * buf)
{
	int Addr_ = BufToVal(&buf[11],AsduAddrLength_);

	return Addr_;
}

int CS104_518::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	int ret = 0;

	ResetTimerAnyFrameRecv(true);
	share_commpoint_ptr nextPoint = getNextCommPoint(PRISTATION_NODE,boost::logic::indeterminate,getLastSendPointIndex());
	if (nextPoint)
	{
		ResetTimerHeartFrame(nextPoint);
	}

	if (buf[0] == 0x88) 
	{ 
		int Addr = getAddrByRecvFrame(buf);
		if (Addr < 0)
		{
			return Addr;
		}
		int pristationIndex = getCommPointIndexByAddrCommType(PRISTATION_NODE,Addr);
		share_pristation_ptr pristationPtr;
		if (pristationIndex >= 0)
		{
			setLastRecvPointIndex(pristationIndex);
			pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(getCommPoint(pristationIndex).lock());
		}

		if (!pristationPtr)
		{
			return -1;
		}

		//memcpy(tempbuf,buf+1,exceptedBytes-1);
		//memcpy(buf,tempbuf,exceptedBytes-1);
		ret = ParseFileFrame(buf+1,pristationPtr);
	}
	else if((buf[0] == 0x68) && ((buf[2] & 0x01) == 0))
	{
   		ret = ParseFrame_I(buf,exceptedBytes);
	}
	else
	{
		ret = CS104::ParseFrameBody(buf,exceptedBytes);
	}

	return ret;
}

//send frame assemble
int CS104_518::AssembleLineValVerSucess(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
  size_t count = bufIndex;
  DataBase::stLine_Val VerQYC;
  VerQYC = boost::any_cast<DataBase::stLine_Val>(cmd.getVal());

	count += ValToBuf(&buf[count],M_LV_VR_3,FrameTypeLength_); //类型标志0xA2
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],VerQYC.timeout_flag,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	
	count += ValToBuf(&buf[count],VerQYC.Line_no,1);
	count += ValToBuf(&buf[count],VerQYC.D_Value_Ua,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_Ub,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_Uc,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_CIa,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_CIb,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_CIc,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_BIa,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_BIb,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_BIc,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_U0,2);
	count += ValToBuf(&buf[count],VerQYC.D_Value_I0,2);
	count += ValToBuf(&buf[count],VerQYC.Angle_UaIa,2);
	count += ValToBuf(&buf[count],VerQYC.Angle_UbIb,2);
	count += ValToBuf(&buf[count],VerQYC.Angle_UcIc,2);
	count += ValToBuf(&buf[count],VerQYC.Fre_Val,2);
	count += ValToBuf(&buf[count],VerQYC.Cos_Val,2);
	count += ValToBuf(&buf[count],VerQYC.P_Val,2);
	count += ValToBuf(&buf[count],VerQYC.Q_Val,2);

	return count - bufIndex;
}

int CS104_518::AssembleHarmonicCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	size_t count = bufIndex;

	DataBase::stHarmonic Harmonic;
	Harmonic = boost::any_cast<DataBase::stHarmonic>(cmd.getVal());

	count += ValToBuf(&buf[count],M_HA_MO_3,FrameTypeLength_); //类型标志0xA7
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],0x07,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);  //信息体地址 

	count += ValToBuf(&buf[count],Harmonic.Line_no,1); 
	count += ValToBuf(&buf[count],Harmonic.harmonic_no,1); 
	count += ValToBuf(&buf[count],Harmonic.Value_Ua,2); 
	count += ValToBuf(&buf[count],Harmonic.Value_Ub,2); 
	count += ValToBuf(&buf[count],Harmonic.Value_Uc,2); 
	count += ValToBuf(&buf[count],Harmonic.Value_Ia,2); 
	count += ValToBuf(&buf[count],Harmonic.Value_Ib,2);
	count += ValToBuf(&buf[count],Harmonic.Value_Ic,2);
	count += ValToBuf(&buf[count],Harmonic.Value_U0,2);
	count += ValToBuf(&buf[count],Harmonic.Value_I0,2);
	count += ValToBuf(&buf[count],Harmonic.P_Val,2);
	count += ValToBuf(&buf[count],Harmonic.Q_Val,2);
	count += ValToBuf(&buf[count],Harmonic.Per_Val,2);

	return count - bufIndex;

}

//int CS104_518::AssembleCallValCoefCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
//{
//	size_t count = bufIndex;
//
//	DataBase::stLine_ValCoef ValCoef;
//	ValCoef = boost::any_cast<DataBase::stLine_ValCoef>(cmd.getVal());
//
//	count += ValToBuf(&buf[count],M_VA_VR_3,FrameTypeLength_); //类型标志0xA3
//	count += ValToBuf(&buf[count],1,InfoNumLength_);
//	count += ValToBuf(&buf[count],ValCoef.timeout_flag,TransReasonLength_);
//	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
//	count += ValToBuf(&buf[count],0,InfoAddrLength_);  //信息体地址 
//
//	count += ValToBuf(&buf[count],ValCoef.Line_no,1); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_Ua,1); 
//	count += ValToBuf(&buf[count],ValCoef.H_Value_Ua,2); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_Ub,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_Ub,2); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_Uc,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_Uc,2); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_CIa,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_CIa,2); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_CIb,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_CIb,2);
//	count += ValToBuf(&buf[count],ValCoef.Flag_CIc,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_CIc,2); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_BIa,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_BIa,2); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_BIb,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_BIb,2);
//	count += ValToBuf(&buf[count],ValCoef.Flag_BIc,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_BIc,2);
//	count += ValToBuf(&buf[count],ValCoef.Flag_U0,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_U0,2); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_I0,1);
//	count += ValToBuf(&buf[count],ValCoef.H_Value_I0,2); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_Angle_UaIa,1);
//	count += ValToBuf(&buf[count],ValCoef.Angle_UaIa,2); 
//	count += ValToBuf(&buf[count],ValCoef.Flag_Angle_UbIb,1);
//	count += ValToBuf(&buf[count],ValCoef.Angle_UbIb,2);
//	count += ValToBuf(&buf[count],ValCoef.Flag_Angle_UcIc,1);
//	count += ValToBuf(&buf[count],ValCoef.Angle_UcIc,2);
//
//	return count - bufIndex;
//}

int CS104_518::AssembleCallValCoefCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	size_t count = bufIndex;

	DataBase::stLine_ValCoef ValCoef;
	ValCoef = boost::any_cast<DataBase::stLine_ValCoef>(cmd.getVal());

	count += ValToBuf(&buf[count],M_VA_VR_3,FrameTypeLength_); //类型标志0xA3
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],ValCoef.timeout_flag,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);  //信息体地址 

	count += ValToBuf(&buf[count],ValCoef.Line_no,1); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_Ua,1); 
	count += ValToBuf(&buf[count],ValCoef.H_Value_Ua,2); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_Ub,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_Ub,2); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_Uc,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_Uc,2); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_CIa,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_CIa,2); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_CIb,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_CIb,2);
	//count += ValToBuf(&buf[count],ValCoef.Flag_CIc,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_CIc,2); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_BIa,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_BIa,2); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_BIb,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_BIb,2);
	//count += ValToBuf(&buf[count],ValCoef.Flag_BIc,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_BIc,2);
	//count += ValToBuf(&buf[count],ValCoef.Flag_U0,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_U0,2); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_I0,1);
	count += ValToBuf(&buf[count],ValCoef.H_Value_I0,2); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_Angle_UaIa,1);
	count += ValToBuf(&buf[count],ValCoef.Angle_UaIa,2); 
	//count += ValToBuf(&buf[count],ValCoef.Flag_Angle_UbIb,1);
	count += ValToBuf(&buf[count],ValCoef.Angle_UbIb,2);
	//count += ValToBuf(&buf[count],ValCoef.Flag_Angle_UcIc,1);
	count += ValToBuf(&buf[count],ValCoef.Angle_UcIc,2);

	return count - bufIndex;
}

int CS104_518::AssembleSendFlieCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	char Name[FileNameLen];
	std::string filename;
	size_t count = bufIndex;
	int Namelen_;

	memset(Name,0,FileNameLen);

	filename = FileHandleGetFileName();

    Namelen_ = filename.length();
	//Name = filename.c_str();
	memcpy(Name,filename.c_str(),Namelen_);

	

	count += ValToBuf(&buf[count],M_FI_SE_3,FrameTypeLength_); //类型标志0xA1
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);//传送原因为0x07
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);  //信息体地址 

	for(int i = 0;i < Namelen_;i ++)
	{
		count += ValToBuf(&buf[count],Name[i],1);
	}

	count += ValToBuf(&buf[count],FileHandleGetTotalLength(),3);

	return count - bufIndex;
}

int CS104_518::AssembleSendFlieBodyCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	size_t count = bufIndex;
	int Val =  boost::any_cast<int>(cmd.getVal());

	count += ValToBuf(&buf[count],M_FI_SE_3,FrameTypeLength_); //类型标志0xA1
	count += ValToBuf(&buf[count],(This_Length_ & 0x80),InfoNumLength_);
	count += ValToBuf(&buf[count],Val,TransReasonLength_);//传送原因为0x0a表示确认，0x0F表示完成
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],Total_NUM,1);  //信息体地址 
	count += ValToBuf(&buf[count],This_N0,1);  //分帧数
	count += ValToBuf(&buf[count],0x00,1);  //0x14
	count += ValToBuf(&buf[count],0x14,1);  //0x14

	return count - bufIndex;
}

int CS104_518::AssembleCallFlieNameCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	char Name[FileNameLen];
	std::string filename;
	size_t count = bufIndex;
	char Val =  0x05;
	int Namelen_;

     Val =  boost::any_cast<char>(cmd.getVal());
	 memset(Name,0,FileNameLen);

	 filename = FileHandleGetFileName();

	 Namelen_ = filename.length();

	 memcpy(Name,filename.c_str(),Namelen_);

	count += ValToBuf(&buf[count],M_FI_RE_3,FrameTypeLength_); //类型标志0xA0
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],Val,TransReasonLength_);//传送原因为0x07表示确认，0x05表示文件不存在
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	for(int i = 0;i < Namelen_;i ++)
	{
		count += ValToBuf(&buf[count],Name[i],1); 
	}

	count += ValToBuf(&buf[count],FileHandleGetTotalLength(),3);  //文件长度 

	if(Val == 0x07)
	{
		//std::cout<<"所召唤的文件存在，准备发送... ..."<<std::endl; 
		AddSendCmdVal(CALL_FILEBODY_ACK,CALL_FILEBODY_ACK_PRIORITY,pristationPtr,1);//AddWaitCmdVal
		//std::cout<<"添加CALL_FILEBODY_ACK完成... "<<std::endl; 
	}
	else
	{
		FileHandleFinish();
		std::ostringstream ostr;
		ostr<<"所召唤的文件不存在或者发送已完成0x07成功，0x0A则失败，返回值Val为："<< Val <<std::endl; 
		AddStatusLogWithSynT(ostr.str());
	}
	//std::cout<<"组装报文体完成后，发送文件count的值为："<<count<<std::endl;
	return count - bufIndex;
}

int CS104_518::AssembleCallFlieBody(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	size_t count = bufIndex;
	size_t len;
	char reason;
	//std::cout<<"开始组装报文准备发送文件... ..." <<std::endl; 
	if(FileHandleGetRemainLength() > FileLen)
	{
		len = FileLen;
		//SendFileLength_ -= FileLen;
	}
	else
	{
		len = FileHandleGetRemainLength();
	}

	int Val =  boost::any_cast<int>(cmd.getVal());

	This_N0 = Val;

	count += ValToBuf(&buf[count],M_FI_RE_3,FrameTypeLength_); //类型标志0xA0
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],0x14,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],Total_NUM,1);
	count += ValToBuf(&buf[count],This_N0,1);
	count += ValToBuf(&buf[count],0x00,1);
	//std::cout<<"现在指针所在的位置为："<< FileLoadPtr_ <<std::endl; 

	//for(size_t i = 0;i < len; i ++)
	//{
	//	//std::cout<<CallFileBuf[FileLoadPtr_]<<std::endl; 
	//	//count += ValToBuf(&buf[count],CallFileBuf[FileLoadPtr_ ++],1);
	//	buf[count ++] = CallFileBuf[FileLoadPtr_ ++];
	//}

	count += FileHandleGetFile(&buf[count],len);

	//count += ValToBuf(&buf[count],,len);

	// std::cout<<"现在指针所在的位置为："<< FileLoadPtr_ <<std::endl; 
	//std::cout<<"所召唤的文件存在，总帧数为："<< Total_NUM <<",本帧为："<<This_N0<<std::endl; 
	if(Total_NUM > This_N0)
	{
		Val ++;
		//  std::cout<<"开始添加继续发送指令. ..."<<std::endl; 
		AddSendCmdVal(CALL_FILEBODY_ACK,CALL_FILEBODY_ACK_PRIORITY,pristationPtr,Val);//AddWaitCmdVal
		//  std::cout<<"添加指令完成 ... "<<std::endl; 
	}
	else 
	{
		AddStatusLogWithSynT("文件发送完成，开始发送结束帧... ...\n");

		//std::cout<<"文件发送完成，开始发送结束帧... ..."<<std::endl; 
		reason = 0x0a;
//		AddSendCmdVal(CALL_FILEBODY_ACK,CALL_FILEBODY_ACK_PRIORITY,pristationPtr,Val);//AddWaitCmdVal
		AddSendCmdVal(CALL_FILENAME_CON,CALL_FILENAME_CON_PRIORITY,pristationPtr,reason);//AddWaitCmdVal
	}

	return count - bufIndex;
}

int CS104_518::AssembleRebootCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_ZD_RE_3,FrameTypeLength_); //类型标志0xA5
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],0x07,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	return count - bufIndex;
}

int CS104_518::AssembleCallTimeCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,boost::posix_time::ptime time)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_CA_TM_3,FrameTypeLength_); //类型标志0xA9
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],0x07,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);

	boost::posix_time::time_duration td = time.time_of_day();
	count += ValToBuf(&buf[count],td.total_milliseconds() % MinutesRemainderMillisecs,2);
	buf[count++] = td.minutes() & 0x3f;
	buf[count++] = td.hours() & 0x1f;
	boost::gregorian::date::ymd_type ymd = time.date().year_month_day();
	buf[count++] = ymd.day & 0x1f;
	buf[count++] = ymd.month & 0x0f;
	buf[count++] = ymd.year % 100;
	std::ostringstream ostr;
	ostr<<"现在时间是："<< ymd.year<<"年"<< ymd.month <<"月"<< ymd.day <<"日"<<std::endl; 
	AddStatusLogWithSynT(ostr.str());
	//std::cout<<"现在时间是："<< ymd.year<<"年"<< ymd.month <<"月"<< ymd.day <<"日"<<std::endl; 
	return count - bufIndex;
}

int CS104_518::AssembleDownLoadLineHValCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	size_t count = bufIndex;

	DataBase::stLine_ValCoef HVal;

	HVal =  boost::any_cast<DataBase::stLine_ValCoef>(cmd.getVal());

	count += ValToBuf(&buf[count],M_VA_DL_3,FrameTypeLength_); //类型标志0xA4
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],0x07,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);

	buf[count++] =  HVal.Line_no;
	buf[count++] =  HVal.Flag_Ua;
	buf[count++] = (HVal.H_Value_Ua & 0xff);
	buf[count++] = ((HVal.H_Value_Ua >> 8)& 0xff);
	buf[count++] =  HVal.Flag_Ub;
	buf[count++] = (HVal.H_Value_Ub & 0xff);
	buf[count++] = ((HVal.H_Value_Ub >> 8)& 0xff);
	buf[count++] =  HVal.Flag_Uc;
	buf[count++] = (HVal.H_Value_Uc & 0xff);
	buf[count++] = ((HVal.H_Value_Uc >> 8)& 0xff);
	buf[count++] =  HVal.Flag_CIa;
	buf[count++] = (HVal.H_Value_CIa & 0xff);
	buf[count++] = ((HVal.H_Value_CIa >> 8)& 0xff);
	buf[count++] =  HVal.Flag_CIb;
	buf[count++] = (HVal.H_Value_CIb & 0xff);
	buf[count++] = ((HVal.H_Value_CIb >> 8)& 0xff);
	buf[count++] =  HVal.Flag_CIc;
	buf[count++] = (HVal.H_Value_CIc & 0xff);
	buf[count++] = ((HVal.H_Value_CIc >> 8)& 0xff);
	buf[count++] =  HVal.Flag_BIa;
	buf[count++] = (HVal.H_Value_BIa & 0xff);
	buf[count++] = ((HVal.H_Value_BIa >> 8)& 0xff);
	buf[count++] =  HVal.Flag_BIb;
	buf[count++] = (HVal.H_Value_BIb & 0xff);
	buf[count++] = ((HVal.H_Value_BIb >> 8)& 0xff);
	buf[count++] =  HVal.Flag_BIc;
	buf[count++] = (HVal.H_Value_BIc & 0xff);
	buf[count++] = ((HVal.H_Value_BIc >> 8)& 0xff);
	buf[count++] =  HVal.Flag_U0;
	buf[count++] = (HVal.H_Value_U0 & 0xff);
	buf[count++] = ((HVal.H_Value_U0 >> 8)& 0xff);
	buf[count++] =  HVal.Flag_I0;
	buf[count++] = (HVal.H_Value_I0 & 0xff);
	buf[count++] = ((HVal.H_Value_I0 >> 8)& 0xff);
	buf[count++] =  HVal.Flag_Angle_UaIa;
	buf[count++] = (HVal.Angle_UaIa & 0xff);
	buf[count++] = ((HVal.Angle_UaIa >> 8)& 0xff);
	buf[count++] =  HVal.Flag_Angle_UbIb;
	buf[count++] = (HVal.Angle_UbIb & 0xff);
	buf[count++] = ((HVal.Angle_UbIb >> 8)& 0xff);
	buf[count++] =  HVal.Flag_Angle_UcIc;
	buf[count++] = (HVal.Angle_UcIc & 0xff);
	buf[count++] = ((HVal.Angle_UcIc >> 8)& 0xff);

	return count - bufIndex; 
}

int CS104_518::AssembleDownLoadParaCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	size_t count = bufIndex;
	char reason = boost::any_cast<char>(cmd.getVal());

	count += ValToBuf(&buf[count],M_DL_PA_3,FrameTypeLength_); //类型标志0xB2
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],reason,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);

	return count - bufIndex;
}

int CS104_518::AssembleBoardInq(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	size_t count = bufIndex;

	DataBase::stBoardInq Data;

	Data =  boost::any_cast<DataBase::stBoardInq>(cmd.getVal());

	count += ValToBuf(&buf[count],M_BD_RQ_3,FrameTypeLength_); //类型标志0xAA
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],0x07,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	count += ValToBuf(&buf[count],Data.YxBoardNum,1);
	count += ValToBuf(&buf[count],Data.YkBoardNum,1);
	count += ValToBuf(&buf[count],Data.YcBoardNum,1);

	return count - bufIndex; 

}

int CS104_518::AssembleBattryActiveCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_AC_BT_3,FrameTypeLength_); //类型标志0xB1
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],0x07,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	return count - bufIndex; 
}

int CS104_518::AssembleBattryActiveOverCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_OV_BT_3,FrameTypeLength_); //类型标志0xB2
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],0x07,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	return count - bufIndex; 
}

int CS104_518::AssembleSignalResetCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	size_t count = bufIndex;

	unsigned char Reason;
	int LineNo_ = boost::any_cast<int>(cmd.getVal());
	int LineNo;

	if (LineNo_ >= 0)
	{
		Reason = 0x07;
		LineNo = LineNo_;
	} 
	else
	{
		Reason = 0x0a;
		LineNo = 255;
	}


	count += ValToBuf(&buf[count],M_SG_RE_3,FrameTypeLength_); //类型标志0xA6
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],Reason,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	count += ValToBuf(&buf[count],LineNo,1);
	return count - bufIndex; 
}

int CS104_518::AssembleCpuVerInqCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	size_t count = bufIndex;
	
	int CpuVer = boost::any_cast<int>(cmd.getVal());

	count += ValToBuf(&buf[count],M_RE_VR_3,FrameTypeLength_); //类型标志0xB4
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],0x07,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	count += ValToBuf(&buf[count],CpuVer,2);//1
	
	return count - bufIndex; 
}

int CS104_518::AssembleLineBValVer(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	DataBase::stLine_BCurVerVal Val;
	Val = boost::any_cast<DataBase::stLine_BCurVerVal>(cmd.getVal());
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_DL_BV_3,FrameTypeLength_); //类型标志0xB5
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],Val.timeout_flag,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	count += ValToBuf(&buf[count],Val.Line_No,1);
	count += ValToBuf(&buf[count],Val.V_Val_BI1,2);
	count += ValToBuf(&buf[count],Val.V_Val_BI2,2);
	count += ValToBuf(&buf[count],Val.V_Val_BI3,2);
	//std::cout<<"S101发送的保护电流校验后值为："<<"Val.V_Val_BI1:"<<Val.V_Val_BI1<<" Val.V_Val_BI2:"<<Val.V_Val_BI2<<" Val.V_Val_BI3:"<<Val.V_Val_BI3<<std::endl;
	return count - bufIndex; 
}

int CS104_518::AssembleDcValVerSucess(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	DataBase::stDCVerVal Val;
	Val = boost::any_cast<DataBase::stDCVerVal>(cmd.getVal());
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_DC_VR_3,FrameTypeLength_); //类型标志0xB5
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],Val.timeout_flag,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	count += ValToBuf(&buf[count],Val.ChannelNo,1);
	count += ValToBuf(&buf[count],Val.V_Val_DC,2);
	//std::cout<<"S101发送的直流量校验后值为："<<Val.V_Val_DC<<std::endl;
	return count - bufIndex; 
}

int  CS104_518::AssembleCallCRCCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_CA_CR_3,FrameTypeLength_); //类型标志0xB8
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],0x07,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	count += ValToBuf(&buf[count],CRC_Code,2);  //

	return count - bufIndex; 
}

int CS104_518::AssembleCallPMCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd & cmd)
{
	DataBase::stPMPara Val;
	Val = boost::any_cast<DataBase::stPMPara>(cmd.getVal());
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_PM_CA_3,FrameTypeLength_); //类型标志0xB5
	count += ValToBuf(&buf[count],1,1);
	count += ValToBuf(&buf[count],Val.timeout_flag,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	count += ValToBuf(&buf[count],Val.LineNo,1);
	count += ValToBuf(&buf[count],Val.AngU1,2);
	count += ValToBuf(&buf[count],Val.AngU2,2);
	count += ValToBuf(&buf[count],Val.AngU3,2);
	count += ValToBuf(&buf[count],Val.AngCI1,2);
	count += ValToBuf(&buf[count],Val.AngCI2,2);
	count += ValToBuf(&buf[count],Val.AngCI3,2);
	count += ValToBuf(&buf[count],Val.AngBI1,2);
	count += ValToBuf(&buf[count],Val.AngBI2,2);
	count += ValToBuf(&buf[count],Val.AngBI3,2);
	count += ValToBuf(&buf[count],Val.AngU0,2);
	count += ValToBuf(&buf[count],Val.AngI0,2);
	//std::cout<<"S101发送的直流量校验后值为："<<Val.V_Val_DC<<std::endl;
	return count - bufIndex; 
}

//召唤保护定值
int CS104_518::AssembleCallJBParaCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,size_t info_num,int startIndex,CCmd & cmd)
{
	size_t count = bufIndex;
//	std::cout<<"AssembleCallJBParaCon"<<std::endl;
	ReadProtectValue();

	count += ValToBuf(&buf[count],M_EP_TX_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_req,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);

	if(info_num>48) info_num=48;

	if(JB_Line_No_<BF533Base.LineNum)
	{
    if(1==info_num)
	{	
		count += ValToBuf(&buf[count],JB_Para_No_ + JB_PARA_START_ADDR_,2);
		count += ValToBuf(&buf[count],JB_Line_No_,1);

        count += ValToBuf(&buf[count],JB_Para[JB_Line_No_][JB_Paratab[JB_Paratab_Original[JB_Para_No_]]],2);

	}
	else
	{
	for (size_t i=0;i<info_num;i++)
	{	
		count += ValToBuf(&buf[count],JB_Tabnum[i+startIndex] + JB_PARA_START_ADDR_,2);
		count += ValToBuf(&buf[count],JB_Line_No_,1);

        count += ValToBuf(&buf[count],JB_Para[JB_Line_No_][JB_Paratab[i]],2);
	}
     }
	}
	else
	{
		DataBase::stJBPARA val;
	    val = boost::any_cast<DataBase::stJBPARA>(cmd.getVal());

		if(1==info_num)
		{	
			count += ValToBuf(&buf[count],val.ParaNo[0],2);
			count += ValToBuf(&buf[count],val.LineNo[0]+BF533Base.LineNum,1);

			count += ValToBuf(&buf[count],val.Value[0],2);

		}
		else
		{
			for (size_t i=0;i<info_num;i++)
			{	
				count += ValToBuf(&buf[count],val.ParaNo[i],2);
				count += ValToBuf(&buf[count],val.LineNo[i]+BF533Base.LineNum,1);

				count += ValToBuf(&buf[count],val.Value[i],2);
			}
		}
	}
	return count - bufIndex;

}

//下装保护定值
int CS104_518::AssembleSendJBParaCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,size_t info_num)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EP_TS_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	
		for (size_t i=0;i<info_num;i++)
		{	
			count += ValToBuf(&buf[count],JB_Parasave[i] + JB_PARA_START_ADDR_,2);
			count += ValToBuf(&buf[count],JB_Line_No_,1);

			count += ValToBuf(&buf[count],JB_Para[JB_Line_No_][JB_Paratab[JB_Paratab_Original[JB_Parasave[i]]]],2);
		}

	return count - bufIndex;

}

//激活保护定值
int CS104_518::AssembleActJBParaCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EP_TH_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);


	return count - bufIndex;
}

//撤销保护定值
int CS104_518::AssembleDeactJBParaCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EP_TH_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_deactcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);


	return count - bufIndex;
}

//复归保护信号
int CS104_518::AssembleJBSignalResetCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EP_TG_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
//	count += ValToBuf(&buf[count],0,InfoAddrLength_);
  
  count += ValToBuf(&buf[count],0,2);
	count += ValToBuf(&buf[count],JB_Line_No_,1);
  
  count += ValToBuf(&buf[count],0,1);
	return count - bufIndex;
}

//保护事件
int CS104_518::AssemJBSOE(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,size_t info_num,CCmd & cmd)
{
	DataBase::stEVENT Val;
	Val = boost::any_cast<DataBase::stEVENT>(cmd.getVal());

	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],M_EP_TJ_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		if(Val.Type==1)
		{
        int type  =JB_Event_Original[((Val.No)>>4)&0xff];//需要转发

		if(type==0xff)
		{
			return -1;
		}

		int lineno=((Val.No)>>12)&0x0f;
		int phase=(Val.No)&0x0f;

		int spe = 0;

		int oritype =(((Val.No)>>4)&0xff) ;
		if(oritype==0x01||oritype==0x02||oritype==0x03||oritype==0x04||oritype==0x06||oritype==0x07
			||oritype==0x0b||oritype==0x0c||oritype==0x0d||oritype==0x10||oritype==0x11)
		{
			Val.EventValue=(Val.EventValue)/10;
		}
		//if(oritype==0x06||oritype==0x07||oritype==0x0a) spe |= 0x80;
		//else /*if(type==0x01||type==0x03||type==0x05)*/ 
		{
			if(phase==0x00) spe|=0x40;
			else if(phase==0x01) spe|=0x20;
			else if(phase==0x02) spe|=0x10;
			else if(phase==0x04) spe|=0x80;
		}

//		using namespace boost::posix_time;
//		ptime lt = boost::posix_time::microsec_clock::local_time();

			unsigned long Time = Val.EventTime;

			unsigned short millisecond = ((Time % (1000*60*60))%(1000*60));
			unsigned char  minute = ((Time % (1000*60*60))/(1000*60));
			unsigned char  Hour = Time / (1000*60*60);
//			time_duration td(minutes(minute) + seconds(millisecond/1000) + milliseconds(millisecond%1000));

			//   std::cout<<"现在时间是（BF533）：Time ="<<Time<<",换算之后是"<<Hour<<"时，"<<minute<<"分,"<<(millisecond/1000)<<"秒,"<<(millisecond%1000)<<"毫秒"<<std::endl;

		count += ValToBuf(&buf[count],type + DEFAULT_JB_EVENT_START_ADDR,2);
		count += ValToBuf(&buf[count],lineno,1);

		count += ValToBuf(&buf[count],spe,1);

		count +=ValToBuf(&buf[count],Val.EventValue,2);
		
		count += ValToBuf(&buf[count],millisecond,2);
		buf[count++] = minute & 0x3f;
		buf[count++] = Hour & 0x1f;

		boost::posix_time::ptime todaytime = boost::posix_time::microsec_clock::local_time();
		boost::gregorian::date::ymd_type ymd = todaytime.date().year_month_day();
		buf[count++] = ymd.day & 0x1f;
		buf[count++] = ymd.month & 0x0f;
		buf[count++] = ymd.year % 100;
		}

		else
		{
			count += ValToBuf(&buf[count],Val.FaultNo /*+ DEFAULT_JB_EVENT_START_ADDR*/,2);
			count += ValToBuf(&buf[count],(Val.LineNo+BF533Base.LineNum),1);

			count += ValToBuf(&buf[count],Val.SPE,1);

			count +=ValToBuf(&buf[count],Val.YcValue,2);

			count += ValToBuf(&buf[count],Val.millisecond,2);
			count += ValToBuf(&buf[count],Val.min,1);
			count += ValToBuf(&buf[count],Val.hour,1);
			count += ValToBuf(&buf[count],Val.day,1);
			count += ValToBuf(&buf[count],Val.month,1);
			count += ValToBuf(&buf[count],(Val.year%100),1);
		}
	}

	return count - bufIndex;
}

int CS104_518::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	switch (cmd.getCmdType())
	{
	case SEND_FILENAME_CON:
		{
			buf[count++] = 0x88;
			buf[count++] = 0;
			buf[count++] = 0;
			count += ValToBuf(&buf[count],IFrameSendCounter_,2);
		  count += ValToBuf(&buf[count],IFrameRecvCounter_,2);
		}
		break;

	case SEND_FILEBODY_CON:
		{
			buf[count++] = 0x88;
			buf[count++] = 0;
			buf[count++] = 0;
			count += ValToBuf(&buf[count],IFrameSendCounter_,2);
		  count += ValToBuf(&buf[count],IFrameRecvCounter_,2);
		}
		break;

	case CALL_FILENAME_CON:
		{
			buf[count++] = 0x88;
			buf[count++] = 0;
			buf[count++] = 0;
			count += ValToBuf(&buf[count],IFrameSendCounter_,2);
		  count += ValToBuf(&buf[count],IFrameRecvCounter_,2);
		}
		break;

	case CALL_FILEBODY_ACK:
		{
			buf[count++] = 0x88;
			buf[count++] = 0;
			buf[count++] = 0;
			count += ValToBuf(&buf[count],IFrameSendCounter_,2);
		  count += ValToBuf(&buf[count],IFrameRecvCounter_,2);
		}
		break;	

	default:
		return CS104::AssembleFrameHead(bufIndex,buf,cmd);
		break;
	}

	return count - bufIndex;
}

int CS104_518::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	//std::ostringstream ostrDebug;

	if (CheckCounterKU())
	{
		ReConnnectChannel();

		return -1;
	}

	if (CheckCounterKI())
	{
		ReConnnectChannel();

		return -1;
	}

	int bytesAssemble = 0;

	share_pristation_ptr pristationPtr;

	if (cmd.getCommPoint())
	{
		if (cmd.getCommPoint()->getCommPointType() == PRISTATION_NODE)
		{
			pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(cmd.getCommPoint());
		}
	}

	if (!pristationPtr)
	{
		std::ostringstream ostr;
		ostr<<"S104规约不能从发送命令中获得pristation ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		
		return -1;
	}

	switch (cmd.getCmdType())
	{
	case LINE_VAL_VER_QYC:
		{
			bytesAssemble = AssembleLineValVerSucess(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				DisconnectSubYkSig(pristationPtr,false);//与消息断开
				ResetTimerValVer(pristationPtr,false,0);
				IGramFrameSend();
			}
		}
		break;

	case LINE_DCVAL_VER_SUCESS:
		{
			bytesAssemble = AssembleDcValVerSucess(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				ResetTimerDcVer(pristationPtr,false,0);
				IGramFrameSend();
				}
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
		}
		break;

	case HARMONIC_CON:
		{
			bytesAssemble = AssembleHarmonicCon(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
	//				std::cout<<"开始发送... ..."<<std::endl; 
				DisconnectSubYkSig(pristationPtr,false);//与消息断开
				IGramFrameSend();
			}
		}
		break;

	case CALL_VALCOEF_CON:
		{
			bytesAssemble = AssembleCallValCoefCon(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				DisconnectSubYkSig(pristationPtr,false);//与消息断开
				IGramFrameSend();
			}
			ResetTimerCallHVal(pristationPtr,false,0);
		}
		break;
	case SIGNAL_RESET_CON:
		{
			bytesAssemble = AssembleSignalResetCon(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				DisconnectSubYkSig(pristationPtr,false);//与消息断开
				IGramFrameSend();
			}
		}
		break;

	case SEND_FILENAME_CON:
		{
			bytesAssemble = AssembleSendFlieCon(bufIndex,buf,pristationPtr);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
		}
		break;

	case SEND_FILEBODY_CON:
		{
			bytesAssemble = AssembleSendFlieBodyCon(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
		}
		break;

	case CALL_FILENAME_CON:
		{

			bytesAssemble = AssembleCallFlieNameCon(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
	//				AddSendCmdVal(CALL_FILEBODY_ACK,CALL_FILEBODY_ACK_PRIORITY,pristationPtr,1);//添加回复指令
				IGramFrameSend();
			}
		}
		break;

	case CALL_FILEBODY_ACK:
		{     
			//std::cout<<"收到指令CALL_FILEBODY_ACK... "<<std::endl; 
			bytesAssemble = AssembleCallFlieBody(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
		}
		break;

	case REBOOT_CON:
		{

			bytesAssemble = AssembleRebootCon(bufIndex,buf,pristationPtr);
			if (bytesAssemble > 0)
			{
				ResetTimerReboot(pristationPtr,true,0);
				IGramFrameSend();
			}	 
		}
		break;

	case CALL_TIME_CON:
		{
			bytesAssemble = AssembleCallTimeCon(bufIndex,buf,pristationPtr,boost::posix_time::microsec_clock::local_time());
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}	 
		}
		break;

	case DOWNLOAD_LINE_CON:
		{
			bytesAssemble = AssembleDownLoadLineHValCon(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}	 
		}
		break;

	case DOWNLOAD_PARA_CON:
		{
	//			std::cout<<"S101收到DOWNLOAD_PARA_CON... ..."<<std::endl;
			bytesAssemble = AssembleDownLoadParaCon(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				ResetTimerDowloadPara(pristationPtr,false,0);
				DisconnectSubYkSig(pristationPtr,false);//与消息断开
				IGramFrameSend();
			}	 
		}
		break;

	case BOARD_REQ_CON:
		{
			bytesAssemble = AssembleBoardInq(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				DisconnectSubYkSig(pristationPtr,false);//与消息断开
				IGramFrameSend();
			}	 
		}
		break;

	case CALL_EQU_PARA_CON:
		{
			char Val;
			//调用CFileHandle对文件近处理
			if (FileHandleRead() > 0)
			{ 
				Val = 0x07;
				Total_NUM = ((FileHandleGetTotalLength() - 1) / FileLen) + 1;
			}
			else
			{
				Val = 0x05;
			}
			AddSendCmdVal(CALL_FILENAME_CON,CALL_FILENAME_CON_PRIORITY,pristationPtr,Val);
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
			ResetTimerCallFileDisConect(pristationPtr);
	//			IGramFrameSend();
		}
		break;

	case CALL_INTERFACE_PARA_CON:
		{
			char Val;
			//调用CFileHandle对文件近处理
			if (FileHandleRead() > 0)
			{
				Val = 0x07;
				Total_NUM = ((FileHandleGetTotalLength() - 1) / FileLen) + 1;
			}
			else
			{
				Val = 0x05;
			}
			AddSendCmdVal(CALL_FILENAME_CON,CALL_FILENAME_CON_PRIORITY,pristationPtr,Val);
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
			ResetTimerCallFileDisConect(pristationPtr);
		}
		break;

	case CALL_CHTYPE_CON:
		{
	//			std::cout<<"S101收到召唤通道类型成功指令... ..."<<std::endl; 
			char Val;
			//调用CFileHandle对文件近处理
			if (FileHandleRead() > 0)
			{
				Val = 0x07;
				Total_NUM = ((FileHandleGetTotalLength() - 1) / FileLen) + 1;
			}
			else
			{
				Val = 0x05;
			}
			AddSendCmdVal(CALL_FILENAME_CON,CALL_FILENAME_CON_PRIORITY,pristationPtr,Val);
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
			ResetTimerCallFileDisConect(pristationPtr);
	//			IGramFrameSend();
		}
		break;

	case CALL_PROVAL_CON:
		{
	//			std::cout<<"S101收到召唤保护定值及保护控制字成功指令... ..."<<std::endl; 
			char Val;
			//调用CFileHandle对文件近处理
			if (FileHandleRead() > 0)
			{
				Val = 0x07;
				Total_NUM = ((FileHandleGetTotalLength() - 1) / FileLen) + 1;
			}
			else
			{
				Val = 0x05;
			}
			AddSendCmdVal(CALL_FILENAME_CON,CALL_FILENAME_CON_PRIORITY,pristationPtr,Val);
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
			ResetTimerCallFileDisConect(pristationPtr);
	//			IGramFrameSend();
		}
		break;

	case CALL_LINEPARA_CON:
		{
			//        std::cout<<"S101收到召唤通道组合关系成功指令... ..."<<std::endl; 
			char Val;
			//调用CFileHandle对文件近处理
			if (FileHandleRead() > 0)
			{
				Val = 0x07;
				Total_NUM = ((FileHandleGetTotalLength() - 1) / FileLen) + 1;
			}
			else
			{
				Val = 0x05;
			}
			AddSendCmdVal(CALL_FILENAME_CON,CALL_FILENAME_CON_PRIORITY,pristationPtr,Val);
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
			ResetTimerCallFileDisConect(pristationPtr);
	//			IGramFrameSend();
		}
		break;

	case BATTERY_ACTIVE_CON:
		{
			//std::cout<<"S101收到电池活化成功指令... ..."<<std::endl; 
			bytesAssemble = AssembleBattryActiveCon(bufIndex,buf,pristationPtr);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
			//DisconnectSubYkSig(pristationPtr);//与消息断开
		}
		break;

	case CALL_PM_ANG_CON:
		{
			bytesAssemble = AssembleCallPMCon(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				ResetTimerCallPm(pristationPtr,false,0);
				IGramFrameSend();
			}
			//DisconnectSubYkSig(pristationPtr);//与消息断开
		}
		break;

	case CALL_CRC_CON:
		bytesAssemble = AssembleCallCRCCon(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
		}
		break;

	case BATTERY_ACTIVE_OVER_CON:
		{
	//			std::cout<<"S101收到电池活化退出成功指令... ..."<<std::endl; 
			bytesAssemble = AssembleBattryActiveOverCon(bufIndex,buf,pristationPtr);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
			//DisconnectSubYkSig(pristationPtr);//与消息断开
		}
		break;

	case DSP_VERSION_INQ_CON:
		{
			bytesAssemble = AssembleCpuVerInqCon(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
		}
		break;

	case LINE_BVAL_VER_QYC:
		{
			bytesAssemble = AssembleLineBValVer(bufIndex,buf,pristationPtr,cmd);
			if (bytesAssemble > 0)
			{
				ResetTimerBCurVer(pristationPtr,false,0);
				IGramFrameSend();
			}
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
		}
		break;
		
	case CALL_JB_PARA_CON:
		{
			//std::cout<<"本次为CALL_JB_PARA_CON指令，开始组装主站报文"<<std::endl;
			int startIndex;
			try
			{
				startIndex = boost::any_cast<int>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				return -1;
			}

			if(0xffff!=JB_Para_No_) 
			{
				//std::cout<<"本次为0xffff!=JB_Para_No_"<<std::endl;
				bytesAssemble = AssembleCallJBParaCon(bufIndex,buf,pristationPtr,1,startIndex,cmd);
			}
			else 
			{
				//std::cout<<"本次为0xffff=JB_Para_No_"<<std::endl;
				if((JB_PARA_NUM_-startIndex)>48)
				{
					//std::cout<<"本次为JB_PARA_NUM_-startIndex)>48"<<std::endl;
					bytesAssemble = AssembleCallJBParaCon(bufIndex,buf,pristationPtr,48,startIndex,cmd);
					AddSendCmdVal(CALL_JB_PARA_CON,CALL_JB_PARA_CON_PRIORITY,pristationPtr,(startIndex+48));
				}
				else
				{
					//std::cout<<"本次不是JB_PARA_NUM_-startIndex)>48"<<std::endl;
					bytesAssemble = AssembleCallJBParaCon(bufIndex,buf,pristationPtr,JB_PARA_NUM_,startIndex,cmd);
	//					AddSendCmdVal(CALL_JB_PARA_OVER,CALL_JB_PARA_OVER_PRIORITY,pristationPtr);
				}
			}
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
		}
		break;
		
	case SEND_JB_PARA_CON:
		{
			bytesAssemble = AssembleSendJBParaCon(bufIndex,buf,pristationPtr,JB_Para_No_);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
		}
		break;
		
	case ACT_JB_PARA_CON:
		{
			bytesAssemble = AssembleActJBParaCon(bufIndex,buf,pristationPtr);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
		}
		break;
		
	case DEACT_JB_PARA_CON:
		{
			bytesAssemble = AssembleDeactJBParaCon(bufIndex,buf,pristationPtr);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
		}
		break;
		
	case JB_SIGNAL_RESET_CON:
		{
			bytesAssemble = AssembleJBSignalResetCon(bufIndex,buf,pristationPtr);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
			DisconnectSubYkSig(pristationPtr,false);//与消息断开
		}
		break;
		
	case EVENT_MESSAGE: //JB_SOE_CON
		{
	//			std::cout<<"EVENT_MESSAGE happend..."<<std::endl;
			bytesAssemble = AssemJBSOE(bufIndex,buf,pristationPtr,1,cmd);

			if (bytesAssemble > 0)
			{
				IGramFrameSend();
			}
		}
		break;

	default:
		return CS104::AssembleFrameBody(bufIndex,buf,cmd);
		break;
	}

	return bytesAssemble;
}

int CS104_518::AssembleFrameTail( size_t bufBegin,size_t bufIndex, unsigned char * buf, CCmd & cmd )
{
	size_t count = bufIndex;

	int length = count - bufBegin - 2;

	switch (cmd.getCmdType())
	{
	case SEND_FILENAME_CON:
		{
			if (length <= 0 )
		{
			return -1;
		}
		length = count - bufBegin - 3;
		buf[bufBegin + 1] = length & 0xff;
		buf[bufBegin + 2] = (length/0x100 )& 0xff;
		}
		break;

	case SEND_FILEBODY_CON:
		{
			if (length <= 0 )
		{
			return -1;
		}
		length = count - bufBegin - 3;
		buf[bufBegin + 1] = length & 0xff;
		buf[bufBegin + 2] = (length/0x100 )& 0xff;
		}
		break;

	case CALL_FILENAME_CON:
		{
			if (length <= 0 )
		{
			return -1;
		}
		length = count - bufBegin - 3;
		buf[bufBegin + 1] = length & 0xff;
		buf[bufBegin + 2] = (length/0x100 )& 0xff;
		}
		break;

	case CALL_FILEBODY_ACK:
		{
			if (length <= 0 )
		{
			return -1;
		}
		length = count - bufBegin - 3;
		buf[bufBegin + 1] = length & 0xff;
		buf[bufBegin + 2] = (length/0x100 )& 0xff;
		}
		break;

	default:
		return CS104::AssembleFrameTail(bufBegin,bufIndex,buf,cmd);
		break;
	}

	return count - bufIndex;
}

void CS104_518::InitDefaultTimeOut()
{

	timeOutValVer_ = DEFAULT_timeOutValVer;
	timeOutReboot_ = DEFAULT_timeOutReboot;
	timeOutCallHVal_ = DEFAULT_timeOutCallHVal;
	timeOutDownloadPara_ = DEFAULT_timeOutDownloadPara;
	timeOutBCurVer_  = DEFAULT_timeOutBCurVer;
	timeOutDcVer_    = DEFAULT_timeOutDcVer;
	timeOutCallPm_   = DEFAULT_timeOutCallPm;
	timeOutSetJBPara_=DEFAULT_timeOutSetJBPara;
	timeOutCallFileDisConect_ = DEFAULT_timeOutCallFileDisConect;
}

void CS104_518::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	ValVerTimer_.reset(new deadline_timer(io_service,seconds(timeOutValVer_)));
	AddTimer(ValVerTimer_);

	RebootTimer_.reset(new deadline_timer(io_service,seconds(timeOutReboot_)));
	AddTimer(RebootTimer_);

	CallHvalTimer_.reset(new deadline_timer(io_service,seconds(timeOutCallHVal_)));
	AddTimer(CallHvalTimer_);

	timerDownloadPara_.reset(new deadline_timer(io_service,seconds(timeOutDownloadPara_)));
	AddTimer(timerDownloadPara_);

	timerBCurVer_.reset(new deadline_timer(io_service,seconds(timeOutBCurVer_)));
	AddTimer(timerBCurVer_);

	timerDcVer_.reset(new deadline_timer(io_service,seconds(timeOutDcVer_)));
	AddTimer(timerDcVer_);

	timerCallPm_.reset(new deadline_timer(io_service,seconds(timeOutCallPm_)));
	AddTimer(timerCallPm_);

	timerCallFileDisConect_.reset(new deadline_timer(io_service,seconds(timeOutCallFileDisConect_)));
	AddTimer(timerCallFileDisConect_);
	
}

void CS104_518::handle_timerSetJBPara(const boost::system::error_code& error,share_pristation_ptr point)
{
	if (!error)
	{
		if (point)
		{
			CCmd cmd(CALL_PROVAL_ACK,CALL_PROVAL_ACK_PRIORITY,point);
			point->AddBF533Cmd(0,cmd);
		}
	}
}

void CS104_518::handle_timerValVer(const boost::system::error_code& error,share_commpoint_ptr point)
{

	DataBase::stLine_Val VerTimeOut;
	VerTimeOut.timeout_flag = 0x0a;
	if (!error)
	{
		if (point)
		{
			AddSendCmdVal(LINE_VAL_VER_QYC,LINE_VAL_VER_QYC_PRIORITY,point,VerTimeOut);
		}
	}
}

void CS104_518::handle_timerReboot(const boost::system::error_code& error,share_commpoint_ptr point)
{
	// std::cout<<"复位定时器超时... "<<std::endl; 
	system("reboot");
}

void CS104_518::handle_timerCallHVal(const boost::system::error_code& error,share_commpoint_ptr point)
{
	 //std::cout<<"召唤定值系数定时器超时... ..."<<std::endl; 
	DataBase::stLine_ValCoef  ValCoefTimeOut;
	ValCoefTimeOut.timeout_flag = 0x0a;
	if (!error)
	{
		if (point)
		{
			AddSendCmdVal(CALL_VALCOEF_CON,CALL_VALCOEF_CON_PRIORITY,point,ValCoefTimeOut);
		}
	}
}

void CS104_518::handle_timerDownloadPara(const boost::system::error_code& error,share_commpoint_ptr point)
{
	//std::cout<<"下装参数定时器超时... "<<std::endl; 
	char Val = 0x05;
	if (!error)
	{
		if (point)
		{
			AddSendCmdVal(DOWNLOAD_PARA_CON,DOWNLOAD_PARA_CON_PRIORITY,point,Val);//AddWaitCmdVal
		}
	}
}

void CS104_518::handle_timerBCurVer(const boost::system::error_code& error,share_commpoint_ptr point)
{
	//std::cout<<"TimerBCurVer超时... ..."<<std::endl;
	DataBase::stLine_BCurVerVal VerTimeOut;
	VerTimeOut.timeout_flag = 0x0a;
	if (!error)
	{
		if (point)
		{
			AddSendCmdVal(LINE_BVAL_VER_QYC,LINE_BVAL_VER_QYC_PRIORITY,point,VerTimeOut);
		}
	}
}

void CS104_518::handle_timerCallPm(const boost::system::error_code& error,share_commpoint_ptr point)
{
	//std::cout<<"TimerBCurVer超时... ..."<<std::endl;
	DataBase::stPMPara CallPmTimeOut;
	CallPmTimeOut.timeout_flag = 0x0a;
	if (!error)
	{
		if (point)
		{
			AddSendCmdVal(CALL_PM_ANG_CON,CALL_PM_ANG_CON_PRIORITY,point,CallPmTimeOut);
		}
	}
}

void CS104_518::handle_timerCallFileDisConect(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		if (point)
		{
			DisconnectSubYkSig(point,false);
		}
	}

	ResetTimerCallFileDisConect(point);
}

void CS104_518::handle_timerDcVer(const boost::system::error_code& error,share_commpoint_ptr point)
{
	//std::cout<<"TimerBCurVer超时... ..."<<std::endl;
	DataBase::stDCVerVal VerTimeOut;
	VerTimeOut.timeout_flag = 0x0a;
	if (!error)
	{
		if (point)
		{
			AddSendCmdVal(LINE_DCVAL_VER_SUCESS,LINE_DCVAL_VER_SUCESS_PRIORITY,point,VerTimeOut);
		}
	}
}

void CS104_518::ResetTimerBCurVer(share_commpoint_ptr point,bool bContinue/* = false*/,unsigned short val/* = 0*/)
{
	//std::cout<<"开始初始化ResetTimerBCurVer... "<<std::endl;
	if (bContinue)
	{
		if (val == 0)
		{
			timerBCurVer_->expires_from_now(boost::posix_time::seconds(timeOutBCurVer_));
		}
		else
		{
			timerBCurVer_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerBCurVer_->async_wait(boost::bind(&CS104_518::handle_timerBCurVer,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerBCurVer_->cancel();
	}
}

void CS104_518::ResetTimerCallPm(share_commpoint_ptr point,bool bContinue/* = false*/,unsigned short val/* = 0*/)
{
	//std::cout<<"开始初始化ResetTimerDcVer... "<<std::endl;
	if (bContinue)
	{
		if (val == 0)
		{
			timerCallPm_->expires_from_now(boost::posix_time::seconds(timeOutCallPm_));
		}
		else
		{
			timerCallPm_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerCallPm_->async_wait(boost::bind(&CS104_518::handle_timerCallPm,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallPm_->cancel();
	}
}

void CS104_518::ResetTimerCallFileDisConect(share_commpoint_ptr point,bool bContinue/* = false*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerCallFileDisConect_->expires_from_now(boost::posix_time::seconds(timeOutCallFileDisConect_));
		}
		else
		{
			timerCallFileDisConect_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerCallFileDisConect_->async_wait(boost::bind(&CS104_518::handle_timerCallFileDisConect,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallFileDisConect_->cancel();
	}
}


void CS104_518::ResetTimerDcVer(share_commpoint_ptr point,bool bContinue/* = false*/,unsigned short val/* = 0*/)
{
	//std::cout<<"开始初始化ResetTimerDcVer... "<<std::endl;
	if (bContinue)
	{
		if (val == 0)
		{
			timerDcVer_->expires_from_now(boost::posix_time::seconds(timeOutDcVer_));
		}
		else
		{
			timerDcVer_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerDcVer_->async_wait(boost::bind(&CS104_518::handle_timerDcVer,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerDcVer_->cancel();
	}
}

void CS104_518::ResetTimerSetJBPara(share_pristation_ptr point,bool bContinue /*= false*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerSetPara_->expires_from_now(boost::posix_time::seconds(timeOutSetJBPara_));
		}
		else
		{
			timerSetPara_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerSetPara_->async_wait(boost::bind(&CS104_518::handle_timerSetJBPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerSetPara_->cancel();
	}
}

void CS104_518::ResetTimerValVer(share_commpoint_ptr point,bool bContinue,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			ValVerTimer_->expires_from_now(boost::posix_time::seconds(timeOutValVer_));
		}
		else
		{
			ValVerTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		ValVerTimer_->async_wait(boost::bind(&CS104_518::handle_timerValVer,this,boost::asio::placeholders::error,point));
	}
	else
	{
		ValVerTimer_->cancel();
	}
}

void  CS104_518::ResetTimerReboot(share_commpoint_ptr point,bool bContinue,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			RebootTimer_->expires_from_now(boost::posix_time::seconds(timeOutReboot_));
		}
		else
		{
			RebootTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		RebootTimer_->async_wait(boost::bind(&CS104_518::handle_timerReboot,this,boost::asio::placeholders::error,point));
	}
	else
	{
		RebootTimer_->cancel();
	}
}

void CS104_518::ResetTimerCallHVal(share_commpoint_ptr point,bool bContinue,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			CallHvalTimer_->expires_from_now(boost::posix_time::seconds(timeOutCallHVal_));
		}
		else
		{
			CallHvalTimer_->expires_from_now(boost::posix_time::seconds(val));
		}
		CallHvalTimer_->async_wait(boost::bind(&CS104_518::handle_timerCallHVal,this,boost::asio::placeholders::error,point));
	}
	else
	{
		CallHvalTimer_->cancel();
	}
}

void CS104_518::ResetTimerDowloadPara(share_commpoint_ptr point,bool bContinue /*= false*/,unsigned short val /*= 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerDownloadPara_->expires_from_now(boost::posix_time::seconds(timeOutDownloadPara_));
		}
		else
		{
			timerDownloadPara_->expires_from_now(boost::posix_time::seconds(val));
		}
		timerDownloadPara_->async_wait(boost::bind(&CS104_518::handle_timerDownloadPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerDownloadPara_->cancel();
	}
}

//para api
int CS104_518::setJB_EVENT_START_ADDR(size_t val)
{
	if (val < 0x101 || val >= 0x6001)
	{
		return -1;
	}

	JB_EVENT_START_ADDR_ = val;

	return 0;
}

int CS104_518::setJB_PARA_START_ADDR(size_t val)
{
	if (val < 0x101 || val >= 0x6001)
	{
		return -1;
	}

	JB_PARA_START_ADDR_ = val;

	return 0;
}

void CS104_518::ReadProtectValue()
{
	int ret=Read_ProtectValPara();

	if(ret<0) return;

	for(int i=0;i<BF533Base.LineNum;i++) ///*BF533Base.LineNum*/
	{
		int count = 0;
		JB_Para[i][count++]=((ProVal[i].I_PRO)&0x01);
		JB_Para[i][count++]=((ProVal[i].OF_Alarm_F)&0x01);

		JB_Para[i][count++]=((ProVal[i].OverLoad)&0x01);
		JB_Para[i][count++]=((ProVal[i].OL_Alarm_F)&0x01);

		JB_Para[i][count++]=((ProVal[i].I0_PRO)&0x01);
		JB_Para[i][count++]=((ProVal[i].I0_Alarm_F)&0x01);

    JB_Para[i][count++]=((ProVal[i].Reclose_PRO)&0x01);

		JB_Para[i][count++]=((ProVal[i].Low_PRO)&0x01);

		JB_Para[i][count++]=((ProVal[i].Accel_F)&0x01);

		JB_Para[i][count++]=((ProVal[i].OverU_PRO_F)&0x01);
		JB_Para[i][count++]=((ProVal[i].OverU_Alarm_F)&0x01);

		JB_Para[i][count++]=((ProVal[i].I_PROValue)/10);
		JB_Para[i][count++]=((ProVal[i].I_PROTime));
		JB_Para[i][count++]=((ProVal[i].I_PROTZWait));

		JB_Para[i][count++]=((ProVal[i].OverLoadValue)/10);
		JB_Para[i][count++]=((ProVal[i].OverLoadTime));
		JB_Para[i][count++]=((ProVal[i].OverLoadTZWait));

		JB_Para[i][count++]=((ProVal[i].I0_PROValue)/10);
		JB_Para[i][count++]=((ProVal[i].I0_PROTime));
		JB_Para[i][count++]=((ProVal[i].I0_PROTZWait));

		JB_Para[i][count++]=((ProVal[i].Reclose_PROTime));

		JB_Para[i][count++]=((ProVal[i].Low_Lock));

		JB_Para[i][count++]=((ProVal[i].Accel_T));

		JB_Para[i][count++]=((ProVal[i].OverU_P_Val));
		JB_Para[i][count++]=((ProVal[i].OverU_Ck_T));
		JB_Para[i][count++]=((ProVal[i].OverU_PROTZWait));

		JB_Para[i][count++]=((ProVal[i].II_PRO));
		JB_Para[i][count++]=((ProVal[i].II_Alarm_F));
		JB_Para[i][count++]=((ProVal[i].II_PROValue)/10);
		JB_Para[i][count++]=((ProVal[i].II_PROTime));
		JB_Para[i][count++]=((ProVal[i].II_PROTZWait));

		JB_Para[i][count++]=((ProVal[i].InverseFlag));
		JB_Para[i][count++]=((ProVal[i].InverseType));

		JB_Para[i][count++]=((ProVal[i].I0_II_PRO));
		JB_Para[i][count++]=((ProVal[i].I0_II_Alarm_F));
		JB_Para[i][count++]=((ProVal[i].I0_II_PROValue)/10);
		JB_Para[i][count++]=((ProVal[i].I0_II_PROTime));
		JB_Para[i][count++]=((ProVal[i].I0_II_PROTZWait));

		JB_Para[i][count++]=((ProVal[i].U0_PRO));
		JB_Para[i][count++]=((ProVal[i].U0_PROValue));
		JB_Para[i][count++]=((ProVal[i].U0_PROTime));

		JB_Para[i][count++]=((BF533Base.Flag_Pro_Rst)/60);
		JB_Para[i][count++]=((BF533Base.Pro_Rst_Time)/60);
	}
}

void CS104_518::WriteProtectValue()
{
	int ret=Read_ProtectValPara();

	if(ret<0) return;

	for (int i=0;i<BF533Base.LineNum;i++)
	{
		int count = 0;
		((ProVal[i].I_PRO))=JB_Para[i][count++];//0
		((ProVal[i].OF_Alarm_F))=JB_Para[i][count++];//1

		((ProVal[i].OverLoad)=JB_Para[i][count++]);//2
		((ProVal[i].OL_Alarm_F))=JB_Para[i][count++];//3

		((ProVal[i].I0_PRO))=JB_Para[i][count++];//4
		((ProVal[i].I0_Alarm_F))=JB_Para[i][count++];//5

		((ProVal[i].Reclose_PRO))=JB_Para[i][count++];

		((ProVal[i].Low_PRO))=JB_Para[i][count++];

		((ProVal[i].Accel_F))=JB_Para[i][count++];

		((ProVal[i].OverU_PRO_F))=JB_Para[i][count++];
		((ProVal[i].OverU_Alarm_F))=JB_Para[i][count++];

		((ProVal[i].I_PROValue))=JB_Para[i][count++]*10;//11
		((ProVal[i].I_PROTime))=JB_Para[i][count++];
		((ProVal[i].I_PROTZWait))=JB_Para[i][count++];

		((ProVal[i].OverLoadValue))=JB_Para[i][count++]*10;
		((ProVal[i].OverLoadTime))=JB_Para[i][count++];
		((ProVal[i].OverLoadTZWait))=JB_Para[i][count++];

		((ProVal[i].I0_PROValue))=JB_Para[i][count++]*10;
		((ProVal[i].I0_PROTime))=JB_Para[i][count++];
		((ProVal[i].I0_PROTZWait))=JB_Para[i][count++];

		((ProVal[i].Reclose_PROTime))=JB_Para[i][count++];

		((ProVal[i].Low_Lock))=JB_Para[i][count++];//21

		((ProVal[i].Accel_T))=JB_Para[i][count++];

		((ProVal[i].OverU_P_Val))=JB_Para[i][count++];
		((ProVal[i].OverU_Ck_T))=JB_Para[i][count++];
		((ProVal[i].OverU_PROTZWait))=JB_Para[i][count++];

		((ProVal[i].II_PRO))=JB_Para[i][count++];
		((ProVal[i].II_Alarm_F))=JB_Para[i][count++];
		((ProVal[i].II_PROValue))=JB_Para[i][count++]*10;
		((ProVal[i].II_PROTime))=JB_Para[i][count++];
		((ProVal[i].II_PROTZWait))=JB_Para[i][count++];

		((ProVal[i].InverseFlag))=JB_Para[i][count++];//31
		((ProVal[i].InverseType))=JB_Para[i][count++];

		((ProVal[i].I0_II_PRO))=JB_Para[i][count++];
		((ProVal[i].I0_II_Alarm_F))=JB_Para[i][count++];
		((ProVal[i].I0_II_PROValue))=JB_Para[i][count++]*10;
		((ProVal[i].I0_II_PROTime))=JB_Para[i][count++];
		((ProVal[i].I0_II_PROTZWait))=JB_Para[i][count++];

		((ProVal[i].U0_PRO))=JB_Para[i][count++];
		((ProVal[i].U0_PROValue))=JB_Para[i][count++];
		((ProVal[i].U0_PROTime))=JB_Para[i][count++];

		((BF533Base.Flag_Pro_Rst))=JB_Para[JB_Line_No_][count++]*60;//41
		((BF533Base.Pro_Rst_Time))=JB_Para[JB_Line_No_][count++]*60;//42
	}

	Write_ProtectValPara();
}

int CS104_518::Read_ProtectValPara(void)
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
	}//end protect

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

		/*xml.ResetMainPos();
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
		}*/

		xml.OutOfElem();
	}//end devsaoyao.xml



	return 0;
}

int CS104_518::Write_ProtectValPara(void)
{
	int sum;
	int Index = 0;
	int count;
	int LineSum = BF533Base.LineNum;//读取召唤上来的线路总数
	//std::cout<<"Write_ProtectValPara 线路号为："<<LineSum<<std::endl;
	FileSystem::CMarkup xml;
	if (!xml.Load(BFProtectValPara))
	{
		Creat_ProtectValPara();
		xml.Load(BFProtectValPara);
		//		return -1;
	}

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

			if(LineSum > sum)
			{
				count = sum;
			}
			else
			{
				count = LineSum;
			}

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
					xml.SetData(ProVal[i].Low_Lock); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OL_P_Val"))
				{
					xml.SetData(ProVal[i].OverLoadValue); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OL_Ck_T"))
				{
					xml.SetData(ProVal[i].OverLoadTime); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OL_TZ_Delay"))
				{
					xml.SetData(ProVal[i].OverLoadTZWait); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OF_I_P_Val"))
				{
					xml.SetData(ProVal[i].I_PROValue); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OF_I_Ck_T"))
				{
					xml.SetData(ProVal[i].I_PROTime); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OF_I_TZ_Delay"))
				{
					xml.SetData(ProVal[i].I_PROTZWait); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_I_P_val"))
				{
					xml.SetData(ProVal[i].I0_PROValue); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_I_Ck_T"))
				{
					xml.SetData(ProVal[i].I0_PROTime); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_I_TZ_Delay"))
				{
					xml.SetData(ProVal[i].I0_PROTZWait); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_II_P_val"))
				{
					xml.SetData(ProVal[i].I0_II_PROValue); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_II_Ck_T"))
				{
					xml.SetData(ProVal[i].I0_II_PROTime); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_II_TZ_Delay"))
				{
					xml.SetData(ProVal[i].I0_II_PROTZWait); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("ReClose_T"))
				{
					xml.SetData(ProVal[i].Reclose_PROTime); 
				}
				//xml.ResetMainPos();
				//if (xml.FindElem("U0_P_val"))
				//{
				//	xml.SetData(ProVal[i].U0_PROValue); 
				//}
				//xml.ResetMainPos();
				//if (xml.FindElem("U0_P_T"))
				//{
				//	xml.SetData(ProVal[i].U0_PROTime); 
				//}

				xml.ResetMainPos();
				if (xml.FindElem("Accel_T"))
				{
					xml.SetData(ProVal[i].Accel_T); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OverU_P_Val"))
				{
					xml.SetData(ProVal[i].OverU_P_Val); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OverU_Ck_T"))
				{
					xml.SetData(ProVal[i].OverU_Ck_T); 
				}

				//xml.ResetMainPos();
				//if (xml.FindElem("NetFA_Ck_T"))
				//{
				//	xml.SetData(ProVal[i].FACheckTime); 
				//}

				//xml.ResetMainPos();
				//if (xml.FindElem("NetFA_TZ_Delay"))
				//{
				//	xml.SetData(ProVal[i].FAOpenDalayTime); 
				//}
				xml.ResetMainPos();
				if (xml.FindElem("NetFA_PS_Addr"))
				{
					xml.SetData(ProVal[i].FAPowerSideAddr); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("NetFA_LS_Addr"))
				{
					xml.SetData(ProVal[i].FALoadSideAddr); 
				}

				xml.ResetMainPos();
				if (xml.FindElem("FA_Lose_Sum"))
				{
					xml.SetData(ProVal[i].FAOverLoadLoseVolSum); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("FA_Rst_T"))
				{
					xml.SetData(ProVal[i].FAReturnToZeroTime); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("LowVol_F"))
				{
					xml.SetData(ProVal[i].Low_PRO); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("ReClose_F"))
				{
					xml.SetData(ProVal[i].Reclose_PRO); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OL_TZ_F"))
				{
					xml.SetData(ProVal[i].OverLoad); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OL_Alarm_F"))
				{
					xml.SetData(ProVal[i].OL_Alarm_F); 
				}

				xml.ResetMainPos();
				if (xml.FindElem("OF_I_TZ_F"))
				{
					xml.SetData(ProVal[i].I_PRO); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OF_I_Alarm_F"))
				{
					xml.SetData(ProVal[i].OF_Alarm_F); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_I_TZ_F"))
				{
					xml.SetData(ProVal[i].I0_PRO); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_I_Alarm_F"))
				{
					xml.SetData(ProVal[i].I0_Alarm_F); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_II_TZ_F"))
				{
					xml.SetData(ProVal[i].I0_II_PRO); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("I0_II_Alarm_F"))
				{
					xml.SetData(ProVal[i].I0_II_Alarm_F); 
				}
				//xml.ResetMainPos();
				//if (xml.FindElem("U0_P_F"))
				//{
				//	xml.SetData(ProVal[i].U0_PRO); 
				//}
				xml.ResetMainPos();
				if (xml.FindElem("NetFA_F"))
				{
					xml.SetData(ProVal[i].FAFlag); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("Accel_F"))
				{
					xml.SetData(ProVal[i].Accel_F); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OverU_Alarm_F"))
				{
					xml.SetData(ProVal[i].OverU_Alarm_F); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("FA_F"))
				{
					xml.SetData(ProVal[i].Local_FAFlag); 
				}

				xml.ResetMainPos();
				if (xml.FindElem("OverU_TZ_F"))
				{
					xml.SetData(ProVal[i].OverU_PRO_F); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OverU_TZ_T"))
				{
					xml.SetData(ProVal[i].OverU_PROTZWait); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OF_II_TZ_F"))
				{
					xml.SetData(ProVal[i].II_PRO); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OF_II_Alarm_F"))
				{
					xml.SetData(ProVal[i].II_Alarm_F); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OF_II_P_Val"))
				{
					xml.SetData(ProVal[i].II_PROValue); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OF_II_Ck_T"))
				{
					xml.SetData(ProVal[i].II_PROTime); 
				}
				xml.ResetMainPos();
				if (xml.FindElem("OF_II_TZ_Delay"))
				{
					xml.SetData(ProVal[i].II_PROTZWait); 
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
					xml.AddChildElem( "LowVol_Val", ProVal[Index].Low_Lock );
					xml.AddChildElem( "OL_P_Val", ProVal[Index].OverLoadValue );
					xml.AddChildElem( "OL_Ck_T",ProVal[Index].OverLoadTime );
					xml.AddChildElem( "OL_TZ_Delay",ProVal[Index].OverLoadTZWait );
					xml.AddChildElem( "OF_I_P_Val", ProVal[Index].I_PROValue );
					xml.AddChildElem( "OF_II_P_Val",ProVal[Index].II_PROValue);
					xml.AddChildElem( "OF_I_Ck_T", ProVal[Index].I_PROTime );
					xml.AddChildElem( "OF_I_TZ_Delay", ProVal[Index].I_PROTZWait );
					xml.AddChildElem( "OF_II_Ck_T",ProVal[Index].II_PROTime);
					xml.AddChildElem( "OF_II_TZ_Delay",ProVal[Index].II_PROTZWait);
					xml.AddChildElem( "I0_I_P_val", ProVal[Index].I0_PROValue );
					xml.AddChildElem( "I0_I_Ck_T", ProVal[Index].I0_PROTime );  
					xml.AddChildElem( "I0_I_TZ_Delay", ProVal[Index].I0_PROTZWait );
					xml.AddChildElem( "I0_II_P_val", ProVal[Index].I0_II_PROValue );
					xml.AddChildElem( "I0_II_Ck_T", ProVal[Index].I0_II_PROTime );  
					xml.AddChildElem( "I0_II_TZ_Delay", ProVal[Index].I0_II_PROTZWait );
					xml.AddChildElem( "ReClose_T", ProVal[Index].Reclose_PROTime );
					xml.AddChildElem( "U0_P_val", ProVal[Index].U0_PROValue );
					xml.AddChildElem( "U0_P_T", ProVal[Index].U0_PROTime );
					xml.AddChildElem( "Accel_T", ProVal[Index].Accel_T );
					xml.AddChildElem( "OverU_P_Val", ProVal[Index].OverU_P_Val );
					xml.AddChildElem( "OverU_Ck_T", ProVal[Index].OverU_Ck_T );
					xml.AddChildElem( "OverU_TZ_T",ProVal[Index].OverU_PROTZWait);
					xml.AddChildElem( "NetFA_Ck_T", ProVal[Index].FACheckTime);
					xml.AddChildElem( "NetFA_TZ_Delay", ProVal[Index].FAOpenDalayTime);
					xml.AddChildElem( "NetFA_PS_Addr", "0" );
					xml.AddChildElem( "NetFA_LS_Addr", "0" );
					xml.AddChildElem( "FA_Lose_Sum", ProVal[Index].FAOverLoadLoseVolSum);
					xml.AddChildElem( "FA_Rst_T", ProVal[Index].FAReturnToZeroTime);
					xml.AddChildElem( "LowVol_F",ProVal[Index].Low_Lock);
					xml.AddChildElem( "ReClose_F", ProVal[Index].Reclose_PRO );
					xml.AddChildElem( "OL_TZ_F", ProVal[Index].OverLoad ); 
					xml.AddChildElem( "OL_Alarm_F", ProVal[Index].OL_Alarm_F );    
					xml.AddChildElem( "OF_I_TZ_F", ProVal[Index].I_PRO );
					xml.AddChildElem( "OF_I_Alarm_F", ProVal[Index].OF_Alarm_F );
					xml.AddChildElem( "OF_II_TZ_F",ProVal[Index].II_PRO);
					xml.AddChildElem( "OF_II_Alarm_F",ProVal[Index].II_Alarm_F);
					xml.AddChildElem( "I0_I_TZ_F", ProVal[Index].I0_PRO );
					xml.AddChildElem( "I0_I_Alarm_F", ProVal[Index].I0_Alarm_F );
					xml.AddChildElem( "I0_II_TZ_F", ProVal[Index].I0_II_PRO );
					xml.AddChildElem( "I0_II_Alarm_F", ProVal[Index].I0_II_Alarm_F );
					xml.AddChildElem( "U0_P_F", ProVal[Index].U0_PRO );
					xml.AddChildElem( "NetFA_F",ProVal[Index].FAFlag);
					xml.AddChildElem( "Accel_F", ProVal[Index].Accel_F );
					xml.AddChildElem( "OverU_Alarm_F",ProVal[Index].OverU_Alarm_F);
					xml.AddChildElem( "OverU_TZ_F",ProVal[Index].OverU_PRO_F);
					xml.AddChildElem( "FA_F",ProVal[Index].Local_FAFlag);

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

	xml.Save(BFProtectValPara);//end protect


	//PrintMessage("开始写文件DevSanyaoPara... ...");
	int ChannelSum = BF533Base.ChannelSum;
	/*int */LineSum    = BF533Base.LineNum;
	int YKCloseTime= BF533Base.YKHZTime;
	int YKOpenTime = BF533Base.YKTZTime;
	int YXLvBoTime = BF533Base.YXLvBoTime;
	int HuoHua_AutoFlag=BF533Base.HuoHua_AutoFlag;
	int HuoHua_Day = BF533Base.HuoHua_Day;
	int AutoRstProtectTime= BF533Base.Pro_Rst_Time;
	int AutoRstProtectFlag= BF533Base.Flag_Pro_Rst;
	int I_Rated = BF533Base.I_Rated;

//	FileSystem::CMarkup xml;
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
		/*xml.ResetMainPos();
		if (xml.FindElem("U_SwitchFlag"))
		{
			xml.SetData(BF533HBase.U_SwitchFlag);
		}*/

		xml.OutOfElem();
	}
	xml.Save(BFDevSanyaoPara);//end devsaoyao.xml




	return 0;
}



};//namespace Protocol






