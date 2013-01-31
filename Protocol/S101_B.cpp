#include <boost/bind.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>
#include "S101_B.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../FileSystem/Markup.h"
#include "../DataBase/PriStation.h"
#include "../DataBase/YkPoint.h"
#include "../DataBase/YxPoint.h"
#include "../DataBase/CosPoint.h"
#include "../DataBase/SoePoint.h"
#include "../DataBase/YcVarPoint.h"

namespace Protocol{

const std::string strDefaultCfg = "S101_BCfg.xml";
size_t CS101_B::S101_BObjectCounter_ = 1;

//针对101规约的YK功能码
const unsigned char DYK_OPEN_NEGATIVE = 0;
const unsigned char DYK_TYPE_OPEN = 0x01;
const unsigned char DYK_TYPE_CLOSE = 0x02;
const unsigned char SYK_TYPE_OPEN = 0;
const unsigned char SYK_TYPE_CLOSE = 0x01;
const unsigned char DYK_CLOSE_NEGATIVE = 0x03;

//各种报文信息体元素的最大数量，一般情况下其最大值是127（信息体数目元素所占字节数为1的情况)
const int INFONUM_LIMIT_ALLYXFRAME = 64;
const int INFONUM_LIMIT_ALLYCFRAME = 48;
const int INFONUM_LIMIT_ALLDDFRAME = 28;
const int INFONUM_LIMIT_COSFRAME = 32;
const int INFONUM_LIMIT_SOEFRAME = 16;
const int INFONUM_LIMIT_YCVARFRAME = 24;

const unsigned char DIR_PRM_N = 0x80;
const unsigned char DIR_PRM_A = 0xC0;
const unsigned char DIR_PRM_B = 0x40;
const unsigned char ACT_FCV = 0x10;
const unsigned char NACK_FCV = 0;
const unsigned char ACT_ACD = 0x20;
const unsigned char NACK_ACD = 0;
const unsigned char ACT_FCB = 0x20;
const unsigned char NACK_FCB = 0;

//针对101规约的传送原因定义
//const unsigned char trans_cyc = 0x01;
//const unsigned char trans_back = 0x02;
const unsigned char trans_spont = 0x03;
const unsigned char trans_init = 0x04;
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

//针对101规约的报文类型标识定义
const unsigned char M_SP_NA_1 = 0x01;
const unsigned char M_DP_NA_1 = 0x03;
//const unsigned char M_BO_NA_1 = 0x07;
const unsigned char M_ME_NA_1 = 0x09;
//const unsigned char M_ME_NB_1 = 0x0b;
//const unsigned char M_ME_NC_1 = 0x0d;
const unsigned char M_IT_NA_1 = 0x0f;
//const unsigned char M_PS_NA_1 = 0x14;
const unsigned char M_ME_ND_1 = 0x15;
const unsigned char M_SP_TB_1 = 0x1e;
const unsigned char M_DP_TB_1 = 0x1f;
//const unsigned char M_ME_TD_1 = 0x22;
//const unsigned char M_ME_TE_1 = 0x23;
//const unsigned char M_ME_TF_1 = 0x24;
//const unsigned char M_IT_TB_1 = 0x25;
const unsigned char M_SC_NA_1 = 0x2d;
const unsigned char M_DC_NA_1 = 0x2e;
const unsigned char M_EI_NA_1 = 0x46;
const unsigned char M_IC_NA_1 = 0x64;
const unsigned char M_CI_NA_1 = 0x65;
const unsigned char C_RD_NA_1 = 0x66;
const unsigned char M_CS_NA_1 = 0x67;
const unsigned char C_TS_NA_1 = 0x68;
const unsigned char C_RP_NA_1 = 0x69;

//针对101规约的功能码定义-监视方向
const unsigned char M_CON_NA_3 = 0x00;                 // <0> ：=确认帧     肯定认可
const unsigned char M_BY_NA_3 = 0x01;                  // <1> ：=确认帧     否定认可
const unsigned char M_AV_NA_3 = 0x08;                  // <8> ：=响应帧     以数据响应请求帧
const unsigned char M_NV_NA_3 = 0x09;                  // <0> ：=响应帧     无所召唤的数据
const unsigned char M_LKR_NA_3 = 0x0b;                 // <11> ：=确认帧    链路状态

//针对101规约的功能码定义-控制方向
const unsigned char C_RCU_NA_3 = 0x00;                 // <0> ：=发送/确认帧   复位通信单元（CU）
const unsigned char C_REQ_NA_3 = 0x03;                 // <3> ：=发送/确认帧   传送数据
const unsigned char C_NEQ_NA_3 = 0x04;                 // <4> ：=发送/无回答帧 传送数据 
const unsigned char C_RFB_NA_3 = 0x07;                 // <7> ：=发送/确认帧   复位帧计数位（FCB）
const unsigned char C_PLK_NA_3 = 0x09;
const unsigned char C_PL1_NA_3 = 0x0a;                 // <10> ：=请求/响应     召唤1级用户数据
const unsigned char C_PL2_NA_3 = 0x0b;                 // <11> ：=请求/响应     召唤2级用户数据

const unsigned char Local_Power = 0x00;
const unsigned char Local_Switch = 0x01;
const unsigned char Remote_Control = 0x02;

const unsigned char EnableISQ = 0x00;
const unsigned char DisableISQ = 0x80;

CS101_B::CS101_B(boost::asio::io_service & io_service)
	:CProtocol(io_service)
{
	SynCharNum_ = 4;
	iFrameRepeatSum_ = 1;
	bActiveRepeatFrame_ = true;
	bActiveDataUp_ = true;

	QOI_ = 0x14;

	InitObjectIndex();
	InitDefaultStartAddr();
	InitDefaultFrameElem();
	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);
}

CS101_B::~CS101_B(void)
{
	S101_BObjectCounter_--;
}

int CS101_B::ConnectSubYkSig(share_commpoint_ptr point)
{
	if (point)
	{
		if (point->getCommPointType() == PRISTATION_NODE)
		{
			return point->ConnectCmdRelaySig(boost::bind(&CS101_B::ProcessSubYkSig,this,_1,_2,_3,_4));
		}
	}


	return -1;
}

int CS101_B::DisconnectSubYkSig( share_commpoint_ptr point,bool bForceClose )
{
	if (point)
	{
		return point->DisconnectCmdRelaySig(bForceClose);
	}

	return -1;
}

void CS101_B::ProcessSubYkSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
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
	case YK_SEL_CON:
		{
			try
			{
				int index =boost::any_cast<int>(val);
				int PriIndex = pristationPtr->SubykToPriyk(index);
				if (index >= 0 && index < (int)pristationPtr->getYkSum() && ReturnCode != RETURN_CODE_TIMEOUT)
				{
					DataBase::stYkCmdPara ykPara(PriIndex,ReturnCode);
					if (bActiveDataUp_)
					{
						AddSendCmdVal(YK_SEL_CON,YK_SEL_CON_PRIORITY,pristationPtr,ykPara);
					}
					else
					{
						AddWaitCmdVal(YK_SEL_CON,YK_SEL_CON_PRIORITY,pristationPtr,ykPara);
						AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,pristationPtr);
					}
				}
			}
			catch(boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控选择确认消息的参数错误:"<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
		}
		break;

	case YK_EXE_CON:
		{
			try
			{
				int index =boost::any_cast<int>(val);
				int PriIndex = pristationPtr->SubykToPriyk(index);
				if (index >= 0 && index < (int)pristationPtr->getYkSum() && ReturnCode != RETURN_CODE_TIMEOUT)
				{
					DataBase::stYkCmdPara ykPara(PriIndex,ReturnCode);
					if (bActiveDataUp_)
					{
						AddSendCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,pristationPtr,ykPara);
					}
					else
					{
						AddWaitCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,pristationPtr,ykPara);
						AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,pristationPtr);
					}
				}
			}
			catch(boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控执行确认消息的参数错误:"<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
		}
		break;

	case YK_CANCEL_CON:
		{
			try
			{
				int index =boost::any_cast<int>(val);
				int PriIndex = pristationPtr->SubykToPriyk(index);
				if (index >= 0 && index < (int)pristationPtr->getYkSum() && ReturnCode != RETURN_CODE_TIMEOUT)
				{
					DataBase::stYkCmdPara ykPara(PriIndex,ReturnCode);
					if (bActiveDataUp_)
					{
						AddSendCmdVal(YK_CANCEL_CON,YK_CANCEL_CON_PRIORITY,pristationPtr,ykPara);
					}
					else
					{
						AddWaitCmdVal(YK_CANCEL_CON,YK_CANCEL_CON_PRIORITY,pristationPtr,ykPara);
						AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,pristationPtr);
					}
				}
			}
			catch(boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控取消确认消息的参数错误:"<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
		}
		break;

	default:
		break;
	}
}

int CS101_B::LoadXmlCfg(std::string filename)
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

	xml.ResetMainPos();
	if (xml.FindElem(strInfoAddr))
	{
		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem(strSYxStartAddr))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short addr = boost::lexical_cast<unsigned short>(strTmp);
				setSYX_START_ADDR(addr);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setSYX_START_ADDR(DEFAULT_SYX_START_ADDR);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strDYxStartAddr))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short addr = boost::lexical_cast<unsigned short>(strTmp);
				setDYX_START_ADDR(addr);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setDYX_START_ADDR(DEFAULT_DYX_START_ADDR);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strYcStartAddr))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short addr = boost::lexical_cast<unsigned short>(strTmp);
				setYC_START_ADDR(addr);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setYC_START_ADDR(DEFAULT_YC_START_ADDR);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strSYkStartAddr))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short addr = boost::lexical_cast<unsigned short>(strTmp);
				setSYK_START_ADDR(addr);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setSYK_START_ADDR(DEFAULT_SYK_START_ADDR);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strDYkStartAddr))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short addr = boost::lexical_cast<unsigned short>(strTmp);
				setDYK_START_ADDR(addr);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setDYK_START_ADDR(DEFAULT_DYK_START_ADDR);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strYmStartAddr))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short addr = boost::lexical_cast<unsigned short>(strTmp);
				setYM_START_ADDR(addr);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setYM_START_ADDR(DEFAULT_YM_START_ADDR);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strHisStartAddr))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short addr = boost::lexical_cast<unsigned short>(strTmp);
				setHIS_START_ADDR(addr);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setHIS_START_ADDR(DEFAULT_HIS_START_ADDR);
			}
		}

		xml.OutOfElem();
	}

	xml.ResetMainPos();
	if (xml.FindElem(strFrameElemLength))
	{
		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem(strFrameTypeLength))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short len = boost::lexical_cast<unsigned short>(strTmp);
				setFrameTypeLength(len);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setFrameTypeLength(DEFAULT_FrameTypeLength);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strInfoNumLength))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short len = boost::lexical_cast<unsigned short>(strTmp);
				setInfoNumLength(len);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setInfoNumLength(DEFAULT_InfoNumLength);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTransReasonLength))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short len = boost::lexical_cast<unsigned short>(strTmp);
				setTransReasonLength(len);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTransReasonLength(DEFAULT_TransReasonLength);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strAsduAddrLength))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short len = boost::lexical_cast<unsigned short>(strTmp);
				setAsduAddrLength(len);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setAsduAddrLength(DEFAULT_AsduAddrLength);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strInfoAddrLength))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short len = boost::lexical_cast<unsigned short>(strTmp);
				setInfoAddrLength(len);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setInfoAddrLength(DEFAULT_InfoAddrLength);
			}
		}

		xml.OutOfElem();
	}

	xml.ResetMainPos();
	if (xml.FindElem(strTimer))
	{
		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutYkSel))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutYkSel(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutYkSel(DEFAULT_timeOutYkSel);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutYkExe))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutYkExe(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutYkExe(DEFAULT_timeOutYkExe);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutYkCancel))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutYkCancel(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutYkCancel(DEFAULT_timeOutYkCancel);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutCallPara))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutCallPara(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutCallPara(DEFAULT_timeOutCallPara);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutSetPara))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutSetPara(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutSetPara(DEFAULT_timeOutSetPara);
			}
		}

		xml.OutOfElem();
	}

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CS101_B::SaveXmlCfg(std::string filename)
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

	if (SYX_START_ADDR_ != DEFAULT_SYX_START_ADDR)
	{
		xml.AddElem(strSYxStartAddr,SYX_START_ADDR_);
		bSave = true;
	}

	if (DYX_START_ADDR_ != DEFAULT_DYX_START_ADDR)
	{
		xml.AddElem(strDYxStartAddr,DYX_START_ADDR_);
		bSave = true;
	}

	if (YC_START_ADDR_ != DEFAULT_YC_START_ADDR)
	{
		xml.AddElem(strYcStartAddr,YC_START_ADDR_);
		bSave = true;
	}

	if (SYK_START_ADDR_ != DEFAULT_SYK_START_ADDR)
	{
		xml.AddElem(strSYkStartAddr,SYK_START_ADDR_);
		bSave = true;
	}

	if (DYK_START_ADDR_ != DEFAULT_DYK_START_ADDR)
	{
		xml.AddElem(strDYkStartAddr,DYK_START_ADDR_);
		bSave = true;
	}

	if (YM_START_ADDR_ != DEFAULT_YM_START_ADDR)
	{
		xml.AddElem(strYmStartAddr,YM_START_ADDR_);
		bSave = true;
	}

	if (HIS_START_ADDR_ != DEFAULT_HIS_START_ADDR)
	{
		xml.AddElem(strHisStartAddr,HIS_START_ADDR_);
		bSave = true;
	}

	xml.OutOfElem();
	if (!bSave)
	{
		xml.RemoveElem();
	}

	xml.AddElem(strFrameElemLength);
	bSave = false;
	xml.IntoElem();

	if (FrameTypeLength_ != DEFAULT_FrameTypeLength)
	{
		xml.AddElem(strFrameTypeLength,FrameTypeLength_);
		bSave = true;
	}

	if (InfoNumLength_ != DEFAULT_InfoNumLength)
	{
		xml.AddElem(strInfoNumLength,InfoNumLength_);
		bSave = true;
	}

	if (TransReasonLength_ != DEFAULT_TransReasonLength)
	{
		xml.AddElem(strTransReasonLength,TransReasonLength_);
		bSave = true;
	}

	if (AsduAddrLength_ != DEFAULT_AsduAddrLength)
	{
		xml.AddElem(strAsduAddrLength,AsduAddrLength_);
		bSave = true;
	}

	if (InfoAddrLength_ != DEFAULT_InfoAddrLength)
	{
		xml.AddElem(strInfoAddrLength,InfoAddrLength_);
		bSave = true;
	}

	xml.OutOfElem();
	if (!bSave)
	{
		xml.RemoveElem();
	}

	xml.AddElem(strTimer);
	bSave = false;
	xml.IntoElem();

	if (timeOutYkSel_ != DEFAULT_timeOutYkSel)
	{
		xml.AddElem(strTimeOutYkSel,timeOutYkSel_);
		bSave = true;
	}

	if (timeOutYkExe_ != DEFAULT_timeOutYkExe)
	{
		xml.AddElem(strTimeOutYkExe,timeOutYkExe_);
		bSave = true;
	}

	if (timeOutYkCancel_ != DEFAULT_timeOutYkCancel)
	{
		xml.AddElem(strTimeOutYkCancel,timeOutYkCancel_);
		bSave = true;
	}

	if (timeOutCallPara_ != DEFAULT_timeOutCallPara)
	{
		xml.AddElem(strTimeOutCallPara,timeOutCallPara_);
		bSave = true;
	}

	if (timeOutSetPara_ != DEFAULT_timeOutSetPara)
	{
		xml.AddElem(strTimeOutSetPara,timeOutSetPara_);
		bSave = true;
	}

	xml.OutOfElem();
	if (!bSave)
	{
		xml.RemoveElem();
	}

	xml.OutOfElem();

	xml.Save(filename);
}

void CS101_B::InitObjectIndex()
{
	ProtocolObjectIndex_ = S101_BObjectCounter_++;
}

int CS101_B::InitProtocol()
{
	//ConnectSubAliveSig();

	CProtocol::InitProtocol();

	InitFrameLocation(5 + AsduAddrLength_);

	AddStatusLogWithSynT("S101_B规约的通道打开成功。\n");

	return 0;
}

void CS101_B::UninitProtocol()
{
	DisconnectSubAliveSig();

	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("S101_B规约的通道关闭成功。\n");
}

int CS101_B::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	if ((buf[0] == 0x68) && (buf[3] == 0x68) && (buf[1] == buf[2]))
	{
		exceptedBytes = buf[1] + 6;
		return 0;
	}
	else if (buf[0] == 0x10)
	{
		exceptedBytes = 4 + AsduAddrLength_;
		return 0;
	}

	return -1;
}

int CS101_B::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	size_t sum = 0;
	if ((exceptedBytes == 4 + AsduAddrLength_) && (buf[0] == 0x10))
	{
		sum = CalcCheckSumByte(&buf[1],exceptedBytes - 3);
	}
	else if ((exceptedBytes > 4 + AsduAddrLength_) && (buf[0] == 0x68) && (buf[3] == 0x68))
	{
		sum = CalcCheckSumByte(&buf[4],exceptedBytes - 6);
	}

	if ((buf[exceptedBytes -1] == 0x16) && (buf[exceptedBytes - 2] == sum))
	{
		return 0;
	}

	return -1;
}

int CS101_B::ParseShortFrame(unsigned char * buf, share_pristation_ptr pristationPtr)
{
	unsigned char FCV = buf[1] & ACT_FCV;
	unsigned char FCB = buf[1] & ACT_FCB;

	unsigned char ContrCode = buf[1] & 0x0f;
	unsigned char DIR_PRM = buf[1] & 0xc0;

	if(DIR_PRM == DIR_PRM_A)
	{
		switch(ContrCode)
		{
		case C_RCU_NA_3:                                                        //复位链路
			if (CheckFCB(FCV,FCB,RESET_LINK,pristationPtr))
			{
				return 0;
			}
			ParseResetLink(buf,pristationPtr);
			break;

		case C_PLK_NA_3:                                                     //请求链路
			if (CheckFCB(FCV,FCB,REQUIRE_LINK,pristationPtr))
			{
				return 0;
			}
			ParseRequireLink(buf,pristationPtr);
			break;

		case C_PL1_NA_3:                                                     //请求一级数据
			if (CheckFCB(FCV,FCB,CALL_PRIMARY_DATA,pristationPtr))
			{
				return 0;
			}
			ParseCallPrimaryDatas(buf,pristationPtr);
			break;

		case C_PL2_NA_3:                                                     //请求二级数据
			if (CheckFCB(FCV,FCB,CALL_SECONDARY_DATA,pristationPtr))
			{
				return 0;
			}
			ParseCallSecondaryDatas(buf,pristationPtr);
			break;

		default:
			{
				std::ostringstream ostr;
				ostr<<"接收报文错误，未定义的报文类型 Control_Code ="<<ContrCode<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;
		}
	}
	else
	{
		switch(ContrCode)
		{
		case M_LKR_NA_3: //链路状态
			if (getLastSendCmd() == REQUIRE_LINK)
			{
				AddSendCmdVal(RESET_LINK,REQUIRE_LINK_PRIORITY,pristationPtr);
			}
			break;

		case M_CON_NA_3: //肯定认可
			switch (getLastSendCmd())
			{
			case RESET_LINK:
				AddSendCmdVal(END_INIT,END_INIT_PRIORITY,pristationPtr);
				break;

			default:
				break;
			}
			break;
		}
	}

	return 0;
}

int CS101_B::ParseLongFrame(unsigned char * buf, share_pristation_ptr pristationPtr)
{
	unsigned char FCV = buf[4] & ACT_FCV;
	unsigned char FCB = buf[4] & ACT_FCB;

	size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识
	size_t TransReason = BufToVal(&buf[TransReasonLocation_],TransReasonLength_);  //传送原因
	unsigned char Data_Code = buf[DataLocation_] & 0x80;

	switch (FrameType)
	{
	case M_SC_NA_1: //single yk
		switch (TransReason)
		{
		case trans_act:
			if (Data_Code == 0x80)
			{
				if (CheckFCB(FCV,FCB,YK_SEL_ACT,pristationPtr))
				{
					return 0;
				}
				ParseSingleYKSel(buf,pristationPtr);
			}
			else if (!Data_Code)
			{
				if (CheckFCB(FCV,FCB,YK_EXE_ACT,pristationPtr))
				{
					return 0;
				}
				ParseSingleYKExe(buf,pristationPtr);
			}
			else
			{
				std::ostringstream ostr;
				ostr<<"YK选择或执行报文错误，未定义的YK_CODE ="<<Data_Code<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;

		case trans_deact:
			if (CheckFCB(FCV,FCB,YK_CANCEL_ACT,pristationPtr))
			{
				return 0;
			}
			ParseSingleYKCancel(buf,pristationPtr);
			break;

		default:
			{
				std::ostringstream ostr;
				ostr<<"YK报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;
		}
		break;

	case M_DC_NA_1: //double yk
		switch (TransReason)
		{
		case trans_act:
			if (Data_Code == 0x80)
			{
				if (CheckFCB(FCV,FCB,YK_SEL_ACT,pristationPtr))
				{
					return 0;
				}
				ParseDoubleYkSel(buf,pristationPtr);
			}
			else if (!Data_Code)
			{
				if (CheckFCB(FCV,FCB,YK_EXE_ACT,pristationPtr))
				{
					return 0;
				}
				ParseDoubleYkExe(buf,pristationPtr);
			}
			else
			{
				std::ostringstream ostr;
				ostr<<"YK选择或执行报文错误，未定义的YK_CODE ="<<Data_Code<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;

		case trans_deact:
			if (CheckFCB(FCV,FCB,YK_CANCEL_ACT,pristationPtr))
			{
				return 0;
			}
			ParseDoubleYkCancel(buf,pristationPtr);
			break;

		default:
			{
				std::ostringstream ostr;
				ostr<<"YK报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;
		}
		break;

	case M_IC_NA_1:
		if (TransReason = trans_act)
		{
			if (CheckFCB(FCV,FCB,CALL_ALL_DATA_ACT,pristationPtr))
			{
				return 0;
			}
			ParseCallAllData(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"总召唤报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_CI_NA_1:
		if (TransReason = trans_act)
		{
			if (CheckFCB(FCV,FCB,CALL_ALL_DD_ACT,pristationPtr))
			{
				return 0;
			}
			ParseCallAllYMData(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"召唤电度报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_CS_NA_1:
		if (TransReason = trans_act)
		{
			if (CheckFCB(FCV,FCB,SYN_TIME_ACT,pristationPtr))
			{
				return 0;
			}
			ParseSynTime(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"对时报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case C_RP_NA_1:
		if (TransReason == trans_act)
		{
			ParseResetCMD(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"复位命令报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case C_TS_NA_1:
		if (TransReason == trans_act)
		{
			ParseTestCMD(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"测试命令报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case C_RD_NA_1:
		ParseReadCMD(buf,pristationPtr);
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

int CS101_B::ParseSingleYKSel(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	pristationPtr->setbSYkDouble(yk_no,false);

	if (yk_no < 0 || yk_no >= (int)pristationPtr->getYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控选择报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (/*pristationPtr->getSubTempSigConnection() ||*/ (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus(yk_no),DataBase::YkSelRecv)))
	if(/*pristationPtr->getSubTempSigConnection() ||*/ (pristationPtr->getYkPointPtr(yk_no))->RecvSelEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<pristationPtr->getYkStatus(yk_no)<<"NextStatus:"<<DataBase::YkSelRecv<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控选择报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if (yk_type == SYK_TYPE_OPEN)
	{
		//pristationPtr->setYkStatus(yk_no,DataBase::YkSelRecv);
		if(pristationPtr->AddYkSelCmd(yk_no,false))
		{
			//pristationPtr->setYkStatus(yk_no,DataBase::YkReady);
			(pristationPtr->getYkPointPtr(yk_no))->ClearYkState();
			AddStatusLogWithSynT("不能下发遥控选择命令，退出处理该报文。\n");
			return -1;
		}
		ConnectSubYkSig(pristationPtr);
		ResetTimerYkSel(pristationPtr,yk_no,true);
	}
	else if (yk_type == SYK_TYPE_CLOSE)
	{
		//pristationPtr->setYkStatus(yk_no,DataBase::YkSelRecv);
		if(pristationPtr->AddYkSelCmd(yk_no,true))
		{
			//pristationPtr->setYkStatus(yk_no,DataBase::YkReady);
			(pristationPtr->getYkPointPtr(yk_no))->ClearYkState();
			AddStatusLogWithSynT("不能下发遥控选择命令，退出处理该报文。\n");
			return -1;
		}
		ConnectSubYkSig(pristationPtr);
		ResetTimerYkSel(pristationPtr,yk_no,true);
	}
	else
	{
		AddStatusLogWithSynT("遥控选择报文的遥控类型非法。\n");
		return -1;
	}

	AddSendCmdVal(CONFIRM_ACK,YK_SEL_CON_PRIORITY + 1,pristationPtr);

	return 0;
}

int CS101_B::ParseSingleYKExe(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)pristationPtr->getYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控执行报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (/*pristationPtr->getSubTempSigConnection() ||*/ (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus(yk_no),DataBase::YkExeRecv)))
	if(/*pristationPtr->getSubTempSigConnection() ||*/ (pristationPtr->getYkPointPtr(yk_no))->RecvExeEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<pristationPtr->getYkStatus(yk_no)<<"NextStatus:"<<DataBase::YkExeRecv<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控执行报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if (yk_type == SYK_TYPE_OPEN)
	{
		if(pristationPtr->getYkType(yk_no) != DataBase::YkOpen)
		{
			AddStatusLogWithSynT("遥控执行报文的遥控类型与预期不符。\n");
			return -1;
		}
		else
		{
			//pristationPtr->setYkStatus(yk_no,DataBase::YkExeRecv);
			if(pristationPtr->AddYkExeCmd(yk_no,false))
			{
				//pristationPtr->setYkStatus(yk_no,DataBase::YkReady);
				(pristationPtr->getYkPointPtr(yk_no))->ClearYkState();
				AddStatusLogWithSynT("不能下发遥控执行命令，退出处理该报文。\n");
				return -1;
			}
			ConnectSubYkSig(pristationPtr);
			ResetTimerYkExe(pristationPtr,yk_no,true);
		}
	}
	else if (yk_type == SYK_TYPE_CLOSE)
	{
		if (pristationPtr->getYkType(yk_no) != DataBase::YkClose)
		{
			AddStatusLogWithSynT("遥控执行报文的遥控类型与预期不符。\n");
			return -1;
		}
		else
		{
			//pristationPtr->setYkStatus(yk_no,DataBase::YkExeRecv);
			if(pristationPtr->AddYkExeCmd(yk_no,true))
			{
				//pristationPtr->setYkStatus(yk_no,DataBase::YkReady);
				(pristationPtr->getYkPointPtr(yk_no))->ClearYkState();
				AddStatusLogWithSynT("不能下发遥控执行命令，退出处理该报文。\n");
				return -1;
			}
			ConnectSubYkSig(pristationPtr);
			ResetTimerYkExe(pristationPtr,yk_no,true);
		}
	}
	else
	{
		//CmdConSig_(YK_EXE_ACT,RETURN_CODE_NEGATIVE,pristationPtr,yk_no);
		AddStatusLogWithSynT("遥控执行报文的遥控类型与预期不符。\n");
		return -1;
	}

	ResetTimerYkSelToExe(pristationPtr,yk_no,false);
	AddSendCmdVal(CONFIRM_ACK,YK_EXE_CON_PRIORITY + 1,pristationPtr);

	return 0;
}

int CS101_B::ParseSingleYKCancel(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)pristationPtr->getYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控取消报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus(yk_no),DataBase::YkCancelRecv))
	if((pristationPtr->getYkPointPtr(yk_no))->RecvCancelEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<pristationPtr->getYkStatus(yk_no)<<"NextStatus:"<<DataBase::YkCancelRecv<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控取消报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if (yk_type == SYK_TYPE_OPEN)
	{
		//pristationPtr->setYkStatus(yk_no,DataBase::YkCancelRecv);
		if(pristationPtr->AddYkCancelCmd(yk_no,false))
		{
			//pristationPtr->setYkStatus(yk_no,DataBase::YkReady);
			(pristationPtr->getYkPointPtr(yk_no))->ClearYkState();
			AddStatusLogWithSynT("不能下发遥控取消命令，退出处理该报文。\n");
			return -1;
		}
		ConnectSubYkSig(pristationPtr);
		ResetTimerYkSel(pristationPtr,yk_no,false);
		ResetTimerYkExe(pristationPtr,yk_no,false);
		ResetTimerYkSelToExe(pristationPtr,yk_no,false);
		ResetTimerYkCancel(pristationPtr,yk_no,true);
	}
	else if (yk_type == SYK_TYPE_CLOSE)
	{
		//pristationPtr->setYkStatus(yk_no,DataBase::YkCancelRecv);
		if(pristationPtr->AddYkCancelCmd(yk_no,true))
		{
			//pristationPtr->setYkStatus(yk_no,DataBase::YkReady);
			(pristationPtr->getYkPointPtr(yk_no))->ClearYkState();
			AddStatusLogWithSynT("不能下发遥控取消命令，退出处理该报文。\n");
			return -1;
		}
		ConnectSubYkSig(pristationPtr);
		ResetTimerYkSel(pristationPtr,yk_no,false);
		ResetTimerYkExe(pristationPtr,yk_no,false);
		ResetTimerYkSelToExe(pristationPtr,yk_no,false);
		ResetTimerYkCancel(pristationPtr,yk_no,true);
	}
	else
	{
		//CmdConSig_(YK_CANCEL_CON,RETURN_CODE_NEGATIVE,pristationPtr,yk_no);
		AddStatusLogWithSynT("遥控取消报文的遥控类型与预期不符。\n");
		return -1;
	}

	AddSendCmdVal(CONFIRM_ACK,YK_CANCEL_CON_PRIORITY + 1,pristationPtr);

	return 0;
}

int CS101_B::ParseDoubleYkSel(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	pristationPtr->setbSYkDouble(yk_no,true);

	if (yk_no < 0 || yk_no >= (int)pristationPtr->getYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控选择报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (/*pristationPtr->getSubTempSigConnection() ||*/ (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus(yk_no),DataBase::YkSelRecv)))
	if(/*pristationPtr->getSubTempSigConnection() ||*/ (pristationPtr->getYkPointPtr(yk_no))->RecvSelEvent())
	{
		DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
		AddSendCmdVal(YK_SEL_CON,YK_SEL_CON_PRIORITY,pristationPtr,ykPara);

		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<pristationPtr->getYkStatus(yk_no)<<"NextStatus:"<<DataBase::YkSelRecv<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控选择报文，但是当前遥控状态不符合，返回否定确认报文。\n");
		return -1;
	}

	if (yk_type == DYK_TYPE_OPEN)
	{
		//pristationPtr->setYkStatus(yk_no,DataBase::YkSelRecv);
		if(pristationPtr->AddYkSelCmd(yk_no,false))
		{
			DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
			AddSendCmdVal(YK_SEL_CON,YK_SEL_CON_PRIORITY,pristationPtr,ykPara);
			AddStatusLogWithSynT("不能下发遥控选择命令，返回否定确认报文。\n");
			return -1;
		}
		ConnectSubYkSig(pristationPtr);
		ResetTimerYkSel(pristationPtr,yk_no,true);
	}
	else if (yk_type == DYK_TYPE_CLOSE)
	{
		//pristationPtr->setYkStatus(yk_no,DataBase::YkSelRecv);
		if(pristationPtr->AddYkSelCmd(yk_no,true))
		{
			DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
			AddSendCmdVal(YK_SEL_CON,YK_SEL_CON_PRIORITY,pristationPtr,ykPara);
			AddStatusLogWithSynT("不能下发遥控选择命令，返回否定确认报文。\n");
			return -1;
		}
		ConnectSubYkSig(pristationPtr);
		ResetTimerYkSel(pristationPtr,yk_no,true);
	}
	else
	{
		DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
		AddSendCmdVal(YK_SEL_CON,YK_SEL_CON_PRIORITY,pristationPtr,ykPara);
		//CmdConSig_(YK_SEL_ACT,RETURN_CODE_NEGATIVE,pristationPtr,yk_no);
		AddStatusLogWithSynT("遥控选择报文的遥控类型非法，返回否定确认报文。\n");
		return -1;
	}

	AddSendCmdVal(CONFIRM_ACK,YK_SEL_CON_PRIORITY + 1,pristationPtr);

	return 0;
}

int CS101_B::ParseDoubleYkExe(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)pristationPtr->getYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控执行报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (/*pristationPtr->getSubTempSigConnection() ||*/ (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus(yk_no),DataBase::YkExeRecv)))
	if(/*pristationPtr->getSubTempSigConnection() ||*/ (pristationPtr->getYkPointPtr(yk_no))->RecvExeEvent())
	{
		DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
		AddSendCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,pristationPtr,ykPara);

		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<pristationPtr->getYkStatus(yk_no)<<"NextStatus:"<<DataBase::YkExeRecv<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控执行报文，但是当前遥控状态不符合，返回否定确认报文。\n");
		return -1;
	}

	if (yk_type == DYK_TYPE_OPEN)
	{
		if(pristationPtr->getYkType(yk_no) != DataBase::YkOpen)
		{
			DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
			AddSendCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,pristationPtr,ykPara);
			//CmdConSig_(YK_EXE_ACT,RETURN_CODE_NEGATIVE,pristationPtr,yk_no);
			AddStatusLogWithSynT("遥控执行报文的遥控类型与预期不符，返回否定确认报文。\n");
			return -1;
		}
		else
		{
			//pristationPtr->setYkStatus(yk_no,DataBase::YkExeRecv);
			if(pristationPtr->AddYkExeCmd(yk_no,false))
			{
				DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
				AddSendCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,pristationPtr,ykPara);
				AddStatusLogWithSynT("不能下发遥控执行命令，返回否定确认报文。\n");
				return -1;
			}
			ConnectSubYkSig(pristationPtr);
			ResetTimerYkExe(pristationPtr,yk_no,true);
		}
	}
	else if (yk_type == DYK_TYPE_CLOSE)
	{
		if (pristationPtr->getYkType(yk_no) != DataBase::YkClose)
		{
			DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
			AddSendCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,pristationPtr,ykPara);
			//CmdConSig_(YK_EXE_ACT,RETURN_CODE_NEGATIVE,pristationPtr,yk_no);
			AddStatusLogWithSynT("遥控执行报文的遥控类型与预期不符，返回否定确认报文。\n");
			return -1;
		}
		else
		{
			//pristationPtr->setYkStatus(yk_no,DataBase::YkExeRecv);
			if(pristationPtr->AddYkExeCmd(yk_no,true))
			{
				DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
				AddSendCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,pristationPtr,ykPara);
				AddStatusLogWithSynT("不能下发遥控执行命令，返回否定确认报文。\n");
				return -1;
			}
			ConnectSubYkSig(pristationPtr);
			ResetTimerYkExe(pristationPtr,yk_no,true);
		}
	}
	else
	{
		DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
		AddSendCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,pristationPtr,ykPara);
		//CmdConSig_(YK_EXE_ACT,RETURN_CODE_NEGATIVE,pristationPtr,yk_no);
		AddStatusLogWithSynT("遥控执行报文的遥控类型与预期不符,，返回否定确认报文。\n");
		return -1;
	}

	ResetTimerYkSelToExe(pristationPtr,yk_no,false);
	AddSendCmdVal(CONFIRM_ACK,YK_EXE_CON_PRIORITY + 1,pristationPtr);

	return 0;
}

int CS101_B::ParseDoubleYkCancel(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)pristationPtr->getYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控取消报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus(yk_no),DataBase::YkCancelRecv))
	if((pristationPtr->getYkPointPtr(yk_no))->RecvCancelEvent())
	{
		DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
		AddSendCmdVal(YK_CANCEL_CON,YK_CANCEL_CON_PRIORITY,pristationPtr,ykPara);

		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<pristationPtr->getYkStatus(yk_no)<<"NextStatus:"<<DataBase::YkCancelRecv<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控取消报文，但是当前遥控状态不符合，返回否定确认报文。\n");
		return -1;
	}

	if (yk_type == DYK_TYPE_OPEN)
	{
		//pristationPtr->setYkStatus(yk_no,DataBase::YkCancelRecv);
		if(pristationPtr->AddYkCancelCmd(yk_no,false))
		{
			DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
			AddSendCmdVal(YK_CANCEL_CON,YK_CANCEL_CON_PRIORITY,pristationPtr,ykPara);
			AddStatusLogWithSynT("不能下发遥控取消命令，返回否定确认报文。\n");
			return -1;
		}
		ConnectSubYkSig(pristationPtr);
		ResetTimerYkSel(pristationPtr,yk_no,false);
		ResetTimerYkExe(pristationPtr,yk_no,false);
		ResetTimerYkSelToExe(pristationPtr,yk_no,false);
		ResetTimerYkCancel(pristationPtr,yk_no,true);
	}
	else if (yk_type == DYK_TYPE_CLOSE)
	{
		//pristationPtr->setYkStatus(yk_no,DataBase::YkCancelRecv);
		if(pristationPtr->AddYkCancelCmd(yk_no,true))
		{
			DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
			AddSendCmdVal(YK_CANCEL_CON,YK_CANCEL_CON_PRIORITY,pristationPtr,ykPara);
			AddStatusLogWithSynT("不能下发遥控取消命令，返回否定确认报文。\n");
			return -1;
		}
		ConnectSubYkSig(pristationPtr);
		ResetTimerYkSel(pristationPtr,yk_no,false);
		ResetTimerYkExe(pristationPtr,yk_no,false);
		ResetTimerYkSelToExe(pristationPtr,yk_no,false);
		ResetTimerYkCancel(pristationPtr,yk_no,true);
	}
	else
	{
		DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_NEGATIVE);
		AddSendCmdVal(YK_CANCEL_CON,YK_CANCEL_CON_PRIORITY,pristationPtr,ykPara);
		//CmdConSig_(YK_CANCEL_CON,RETURN_CODE_NEGATIVE,pristationPtr,yk_no);
		AddStatusLogWithSynT("遥控取消报文的遥控类型与预期不符，返回否定确认报文。\n");
		return -1;
	}

	AddSendCmdVal(CONFIRM_ACK,YK_CANCEL_CON_PRIORITY + 1,pristationPtr);

	return 0;
}

int CS101_B::ParseCallAllData(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	QOI_ = buf[DataLocation_];

	AddSendCmdVal(CONFIRM_ACK,CALL_ALL_DATA_CON_PRIORITY + 1,pristationPtr);
	AddSendCmdVal(CALL_ALL_DATA_CON,CALL_ALL_DATA_CON_PRIORITY,pristationPtr,QOI_);
	
	return 0;
}

int CS101_B::ParseCallAllYMData(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	QOI_ = buf[DataLocation_];

	AddSendCmdVal(CONFIRM_ACK,CALL_ALL_DD_CON_PRIORITY + 1,pristationPtr);
	AddSendCmdVal(CALL_ALL_YM_CON,CALL_ALL_DD_CON_PRIORITY,pristationPtr,QOI_);

	return 0;
}

int CS101_B::ParseResetCMD(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	unsigned char QRP = buf[DataLocation_];

	AddSendCmdVal(CONFIRM_ACK,RESET_CMD_PRIORITY + 1,pristationPtr);
	AddSendCmdVal(RESET_CMD,RESET_CMD_PRIORITY,pristationPtr,QRP);

	return 0;
}

int CS101_B::ParseTestCMD(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	unsigned short FBP = BufToVal(&buf[DataLocation_],2);

	AddSendCmdVal(CONFIRM_ACK,TEST_CMD_PRIORITY + 1,pristationPtr);
	AddSendCmdVal(TEST_CMD,TEST_CMD_PRIORITY,pristationPtr,FBP);

	return 0;
}

int CS101_B::ParseReadCMD(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_);

	if ((info_addr >= (int)SYX_START_ADDR_ || info_addr >= (int)DYX_START_ADDR_) && info_addr < (int)YC_START_ADDR_)
	{
		int index = ((info_addr - SYX_START_ADDR_) < (info_addr - DYX_START_ADDR_)) ? (info_addr - SYX_START_ADDR_) : (info_addr - DYX_START_ADDR_);
		if (index >= 0 && index < (int)pristationPtr->getYxSum())
		{
			AddSendCmdVal(CONFIRM_ACK,SINGLE_YX_DATA_PRIORITY + 1,pristationPtr);
			AddSendCmdVal(SINGLE_YX_DATA,SINGLE_YX_DATA_PRIORITY,pristationPtr,index);
		}
		else
		{
			AddSendCmdVal(CONFIRM_NACK,CONFIRM_NACK_PRIORITY,pristationPtr);
		}
		
	}
	else if (info_addr >= (int)YC_START_ADDR_ && (info_addr < (int)SYK_START_ADDR_ || info_addr < (int)DYK_START_ADDR_))
	{
		int index = info_addr - YC_START_ADDR_;
		if(index >= 0 && index < (int)pristationPtr->getYcSum())
		{
			AddSendCmdVal(CONFIRM_ACK,SINGLE_YC_DATA_PRIORITY + 1,pristationPtr);
			AddSendCmdVal(SINGLE_YC_DATA,SINGLE_YC_DATA_PRIORITY,pristationPtr,index);
		}
		else
		{
			AddSendCmdVal(CONFIRM_NACK,CONFIRM_NACK_PRIORITY,pristationPtr);
		}
	}
	else if (info_addr >= (int)YM_START_ADDR_)
	{
		int index = info_addr - YM_START_ADDR_;
		if(index >= 0 && index < (int)pristationPtr->getYmSum())
		{
			AddSendCmdVal(CONFIRM_ACK,SINGLE_YM_DATA_PRIORITY + 1,pristationPtr);
			AddSendCmdVal(SINGLE_YM_DATA,SINGLE_YM_DATA_PRIORITY,pristationPtr,index);
		}
		else
		{
			AddSendCmdVal(CONFIRM_NACK,CONFIRM_NACK_PRIORITY,pristationPtr);
		}
	}
	else
	{
		AddSendCmdVal(CONFIRM_NACK,CONFIRM_NACK_PRIORITY,pristationPtr);
	}

	return 0;
}

int CS101_B::ParseSynTime(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();

	unsigned short millisecond = BufToVal(&buf[DataLocation_],2);
	unsigned char minute = buf[DataLocation_ + 2] & 0x3f;
	unsigned char Hour = buf[DataLocation_ + 3] & 0x1f;
	unsigned char Day = buf[DataLocation_ + 4] & 0x1f;
	unsigned char Month = buf[DataLocation_ + 5] & 0x0f;
	unsigned short Year = ByteYearToWord(buf[DataLocation_ + 6] & 0x7f);

	//time_duration td(hours(Hour) + minutes(minute) + seconds(millisecond / 1000) + milliseconds(millisecond % 1000));
	//boost::gregorian::date dt(Year,Month,Day);
	//ptime timeVal(dt,td);

	pristationPtr->WriteSysTime(Year,Month,Day,Hour,minute,(millisecond / 1000),(millisecond % 1000));

	AddSendCmdVal(CONFIRM_ACK,SYN_TIME_CON_PRIORITY + 1,pristationPtr);
	AddSendCmdVal(SYN_TIME_CON,SYN_TIME_CON_PRIORITY,pristationPtr);
	
	return 0;

}

int CS101_B::ParseResetLink(unsigned char * buf, share_pristation_ptr pristationPtr)
{
	setSendFCB(pristationPtr,NACK_FCB);

	AddSendCmdVal(CONFIRM_ACK,REQUIRE_LINK_PRIORITY + 1,pristationPtr);

	AddSendCmdVal(REQUIRE_LINK,REQUIRE_LINK_PRIORITY,pristationPtr);

	return 0;
}

int CS101_B::ParseRequireLink(unsigned char * buf, share_pristation_ptr pristationPtr)
{
	AddSendCmdVal(LINK_STATUS,LINK_STATUS_PRIORITY,pristationPtr);

	return 0;
}

int CS101_B::ParseCallPrimaryDatas(unsigned char * buf, share_pristation_ptr pristationPtr)
{
	CCmd waitVal;
	DataBase::CCosPoint cosVal;

	if (!getMaxPriopriyWaitCmdByPointPtr(pristationPtr,waitVal))
	{
		AddSendCmdVal(waitVal);
	}
	else if (pristationPtr->getFirstCosPoint(cosVal) >= 0)
	{
		AddSendCmdVal(COS_DATA_UP,COS_DATA_UP_PRIORITY,pristationPtr);
	}
	else
	{
		AddSendCmdVal(CONFIRM_NACK,CONFIRM_NACK_PRIORITY,pristationPtr);
	}

	return 0;
}

int CS101_B::ParseCallSecondaryDatas(unsigned char * buf, share_pristation_ptr pristationPtr)
{
	CCmd waitVal;
	DataBase::CCosPoint cosVal;
	DataBase::CSoePoint soeVal;
	DataBase::CYcVarPoint ycvarVal;

	if (!getMaxPriopriyWaitCmdByPointPtr(pristationPtr,waitVal))
	{
		AddSendCmdVal(waitVal);
	}
	else if (pristationPtr->getFirstCosPoint(cosVal) >= 0)
	{
		AddSendCmdVal(COS_DATA_UP,COS_DATA_UP_PRIORITY,pristationPtr);
	}
	else if (pristationPtr->getFirstSoePoint(soeVal) >= 0)
	{
		AddSendCmdVal(SOE_DATA_UP,SOE_DATA_UP_PRIORITY,pristationPtr);
	}
	else if (pristationPtr->getFirstYcvarPoint(ycvarVal) >= 0)
	{
		AddSendCmdVal(YCVAR_DATA_UP,YCVAR_DATA_UP_PRIORITY,pristationPtr);
	}
	else
	{
		AddSendCmdVal(CONFIRM_NACK,CONFIRM_NACK_PRIORITY,pristationPtr);
	}

	return 0;
}

int CS101_B::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	int ret = 0;

	unsigned char funcType = buf[0];

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
		//std::ostringstream ostr;
		//ostr<<"S101_B规约不能根据接收报文中的地址匹配pristation ptr,这帧报文将不会被解析。"<<std::endl;
		//AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (funcType)
	{
	case 0x10:
		ret = ParseShortFrame(buf,pristationPtr);
		break;

	case 0x68:
		ret = ParseLongFrame(buf,pristationPtr);
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

	return pristationIndex;
}

unsigned char CS101_B::getACD(share_pristation_ptr pristationPtr)
{
	if (getWaitCmdQueueCount(pristationPtr) > 0)
	{
		return ACT_ACD;
	}

	DataBase::CCosPoint cosTmp;
	if (pristationPtr->getFirstCosPoint(cosTmp) >= 0)
	{
		return ACT_ACD;
	}

	return NACK_ACD;
}

int CS101_B::CheckACD(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	//buf[bufIndex] |= getACD(pristationPtr);

	return 0;
}

unsigned char CS101_B::getFCB(share_pristation_ptr pristationPtr)
{
	if (pristationPtr->getLastFcbFlag())
	{
		return ACT_FCB;
	}
	else
	{
		return NACK_FCB;
	}
}

unsigned char CS101_B::getSendFCB(share_pristation_ptr pristationPtr)
{
	if (pristationPtr->getSendFcbFlag())
	{
		return ACT_FCB;
	}
	else
	{
		return NACK_FCB;
	}
}

int CS101_B::setFCB(share_pristation_ptr pristationPtr,unsigned char val)
{
	val = val & ACT_FCB;
	if (val == ACT_FCB)
	{
		pristationPtr->setLastFcbFlag(true);
	}
	else if(val == NACK_FCB)
	{
		pristationPtr->setLastFcbFlag(false);
	}

	return 0;
}

int CS101_B::setSendFCB(share_pristation_ptr pristationPtr,unsigned char val)
{
	val = val & ACT_FCB;
	if (val == ACT_FCB)
	{
		pristationPtr->setSendFcbFlag(true);
	}
	else if(val == NACK_FCB)
	{
		pristationPtr->setSendFcbFlag(false);
	}

	return 0;
}

int CS101_B::CheckFCB( unsigned char FCV,unsigned char FCB,typeCmd CurCmd,share_pristation_ptr pristationPtr )
{
	int ret = 0;

	if (FCV)
	{
		if ((FCB == getFCB(pristationPtr)) && (getLastRecvCmd() == CurCmd))
		{
			bRepeatLastFrame_ = true;
			ret = 1;
		}

		setFCB(pristationPtr,FCB);
	}

	setLastRecvCmd(CurCmd);

	return ret;
}

int CS101_B::getAddrByRecvFrame(unsigned char * buf)
{
	unsigned char funcType = buf[0];
	unsigned char ContrCode = 0;
	int addr = -1;
	switch (funcType)
	{
	case 0x10:
		ContrCode = buf[1];
		addr = BufToVal(&buf[2],AsduAddrLength_);
		break;

	case 0x68:
		ContrCode = buf[4];
		if (BufToVal(&buf[5],AsduAddrLength_) == BufToVal(&buf[AsduAddrLocation_],AsduAddrLength_))
		{
			addr = BufToVal(&buf[AsduAddrLocation_],AsduAddrLength_);
		}
		else
		{
			addr = -1;
		}

		break;

	default:
		addr = -1;
		break;
	}

	if ((ContrCode & 0x80) == 0x80) //检查报文方向位标志
	{
		std::ostringstream ostr;
		ostr<<"控制域解析发现报文方向标志位错误,这帧报文将不会被解析。Control_Code ="<<ContrCode<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	return addr;
}

int CS101_B::AssembleLinkStatus(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_LKR_NA_3 | DIR_PRM_N,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CS101_B::AssembleConfirmNack(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_NV_NA_3 | DIR_PRM_N,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CS101_B::AssembleConfirmAck(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_CON_NA_3 | DIR_PRM_N,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CS101_B::AssembleRequireLink(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_PLK_NA_3 | DIR_PRM_A,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CS101_B::AssembleResetLink(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_RCU_NA_3 | DIR_PRM_A,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CS101_B::AssembleDoubleYKSelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CS101_B::AssembleDoubleYKExeCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CS101_B::AssembleDoubleYKCancelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_deactcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CS101_B::AssembleDoubleYKOver(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actterm,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CS101_B::AssembleSingleYKSelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CS101_B::AssembleSingleYKExeCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CS101_B::AssembleSingleYKCancelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_deactcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CS101_B::AssembleSingleYKOver(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actterm,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CS101_B::AssembleSynTimeCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,boost::posix_time::ptime time)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_CS_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	boost::posix_time::time_duration td = time.time_of_day();
	count += ValToBuf(&buf[count],td.total_milliseconds() % MinutesRemainderMillisecs,2);
	buf[count++] = td.minutes() & 0x3f;
	buf[count++] = td.hours() & 0x1f;
	boost::gregorian::date::ymd_type ymd = time.date().year_month_day();
	//buf[count++] = ymd.day & 0x1f;
	buf[count++] = ((time.date().day_of_week()<<5) & 0xe0) | (ymd.day & 0x1f);
	buf[count++] = ymd.month & 0x0f;
	buf[count++] = ymd.year % 100;

	return count - bufIndex;
}

int CS101_B::AssembleCallDataCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_IC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI_;

	return count - bufIndex;
}

int CS101_B::AssembleCallDataOver(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_IC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actterm,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI_;

	return count - bufIndex;
}

int CS101_B::AssembleAllSingleYX( size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num )
{
	size_t count = bufIndex;


	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYxSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYXFRAME)
	{
		info_num = INFONUM_LIMIT_ALLYXFRAME;
	}

	if (startIndex + info_num > pristationPtr->getYxSum())
	{
		info_num = pristationPtr->getYxSum() - startIndex;
	}

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SP_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],(info_num | DisableISQ<<((InfoNumLength_ - 1) * 8)),InfoNumLength_);
	count += ValToBuf(&buf[count],trans_all,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],startIndex + SYX_START_ADDR_,InfoAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		unsigned char byteVal = (pristationPtr->getYxQuality(startIndex) & 0xf0) | (pristationPtr->getFinalYxVal(startIndex) & 0x01);
		count += ValToBuf(&buf[count],byteVal,1);
		startIndex++;
	}

	return count - bufIndex;
}

int CS101_B::AssembleAllDoubleYX( size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num )
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYxSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYXFRAME)
	{
		info_num = INFONUM_LIMIT_ALLYXFRAME;
	}

	if (startIndex + info_num > pristationPtr->getYxSum())
	{
		info_num = pristationPtr->getYxSum() - startIndex;
	}

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DP_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],(info_num | DisableISQ<<((InfoNumLength_ - 1) * 8)),InfoNumLength_);
	count += ValToBuf(&buf[count],trans_all,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],startIndex + DYX_START_ADDR_,InfoAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		unsigned char byteVal = (pristationPtr->getYxQuality(startIndex) & 0xf0) | (pristationPtr->getFinalYxVal(startIndex) & 0x03);
		count += ValToBuf(&buf[count],byteVal,1);
		startIndex++;
	}

	return count - bufIndex;
}

int CS101_B::AssembleAllYCWithVaild( size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num )
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYcSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYCFRAME)
	{
		info_num = INFONUM_LIMIT_ALLYCFRAME;
	}

	if (startIndex + info_num > pristationPtr->getYcSum())
	{
		info_num = pristationPtr->getYcSum() - startIndex;
	}

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_ME_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],(info_num | DisableISQ<<((InfoNumLength_ - 1) * 8)),InfoNumLength_);
	count += ValToBuf(&buf[count],trans_all,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],startIndex + YC_START_ADDR_,InfoAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		count += ValToBuf(&buf[count],pristationPtr->getFinalYcVal(startIndex),2);
		count += ValToBuf(&buf[count],pristationPtr->getYcQuality(startIndex),1);
		startIndex++;
	}

	return count - bufIndex;
}

int CS101_B::AssembleCallYMCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_CI_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI_;

	return count - bufIndex;
}

int CS101_B::AssembleCallYMOver(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_CI_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actterm,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI_;

	return count - bufIndex;
}

int CS101_B::AssembleAllYM( size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num )
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYmSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLDDFRAME)
	{
		info_num = INFONUM_LIMIT_ALLDDFRAME;
	}

	if (startIndex + info_num > pristationPtr->getYmSum())
	{
		info_num = pristationPtr->getYmSum() - startIndex;
	}

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_IT_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],(info_num | DisableISQ<<((InfoNumLength_ - 1) * 8)),InfoNumLength_);
	count += ValToBuf(&buf[count],trans_all,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],startIndex + YM_START_ADDR_,InfoAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		count += ValToBuf(&buf[count],pristationPtr->getOriYmVal(startIndex),4);
		count += ValToBuf(&buf[count],pristationPtr->getYmQuality(startIndex),1);
		startIndex++;
	}

	return count - bufIndex;
}

int CS101_B::AssembleSingleCOS(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CCosPoint * cosBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SP_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		unsigned char byteVal = (cosBuf[i].getYxQuality() & 0xf0);
		if (pristationPtr->getYxPolar(cosBuf[i].getYxIndex()))
		{
			byteVal |= (cosBuf[i].getYxVal() & 0x01);
		}
		else
		{

			byteVal |= ~(cosBuf[i].getYxVal() & 0x01);
		}

		count += ValToBuf(&buf[count],cosBuf[i].getYxIndex() + SYX_START_ADDR_,InfoAddrLength_);
		count += ValToBuf(&buf[count],byteVal,1);
	}

	return count - bufIndex;
}

int CS101_B::AssembleDoubleCOS(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CCosPoint * cosBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DP_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		unsigned char byteVal = (cosBuf[i].getYxQuality() & 0xf0);
		if (pristationPtr->getYxPolar(cosBuf[i].getYxIndex()))
		{
			byteVal |= (cosBuf[i].getYxVal() & 0x03);
		}
		else
		{
			byteVal |= ~(cosBuf[i].getYxVal() & 0x03);
		}

		count += ValToBuf(&buf[count],cosBuf[i].getYxIndex() + DYX_START_ADDR_,InfoAddrLength_);
		count += ValToBuf(&buf[count],byteVal,1);
	}

	return count - bufIndex;
}

int CS101_B::AssembleSingleSOE(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CSoePoint * soeBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SP_TB_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		unsigned char byteVal = (soeBuf[i].getYxQuality() & 0xf0);
		if (pristationPtr->getYxPolar(soeBuf[i].getYxIndex()))
		{
			byteVal |= (soeBuf[i].getYxVal() & 0x01);
		}
		else
		{
			byteVal |= ~(soeBuf[i].getYxVal() & 0x01);
		}

		count += ValToBuf(&buf[count],soeBuf[i].getYxIndex() + SYX_START_ADDR_,InfoAddrLength_);
		count += ValToBuf(&buf[count],byteVal,1);

		boost::posix_time::time_duration td = (soeBuf[i].getYxTime()).time_of_day();
		count += ValToBuf(&buf[count],td.total_milliseconds() % MinutesRemainderMillisecs,2);
		buf[count++] = td.minutes() & 0x3f;
		buf[count++] = td.hours() & 0x1f;
		boost::gregorian::date::ymd_type ymd = (soeBuf[i].getYxTime().date()).year_month_day();
		buf[count++] = ymd.day & 0x1f;
		buf[count++] = ymd.month & 0x0f;
		buf[count++] = ymd.year % 100;
	}

	return count - bufIndex;
}

int CS101_B::AssembleDoubleSOE(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CSoePoint * soeBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DP_TB_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		unsigned char byteVal = (soeBuf[i].getYxQuality() & 0xf0);
		if (pristationPtr->getYxPolar(soeBuf[i].getYxIndex()))
		{
			byteVal |= (soeBuf[i].getYxVal() & 0x03);
		}
		else
		{
			byteVal |= ~(soeBuf[i].getYxVal() & 0x03);
		}

		count += ValToBuf(&buf[count],soeBuf[i].getYxIndex() + DYX_START_ADDR_,InfoAddrLength_);
		count += ValToBuf(&buf[count],byteVal,1);

		boost::posix_time::time_duration td = (soeBuf[i].getYxTime()).time_of_day();
		count += ValToBuf(&buf[count],td.total_milliseconds() % MinutesRemainderMillisecs,2);
		buf[count++] = td.minutes() & 0x3f;
		buf[count++] = td.hours() & 0x1f;
		boost::gregorian::date::ymd_type ymd = (soeBuf[i].getYxTime().date()).year_month_day();
		buf[count++] = ymd.day & 0x1f;
		buf[count++] = ymd.month & 0x0f;
		buf[count++] = ymd.year % 100;
	}

	return count - bufIndex;
}

int CS101_B::AssembleYcVar(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CYcVarPoint * ycvarBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_ME_ND_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		//typeFinalYcval ycVal = ycvarBuf[i].getYcVal() + pristationPtr->getYcPlus(ycvarBuf[i].getYcIndex());
		typeFinalYcval ycVal = pristationPtr->getFinalYcVarVal(ycvarBuf[i]);

		count += ValToBuf(&buf[count],ycvarBuf[i].getYcIndex() + YC_START_ADDR_,InfoAddrLength_);
		count += ValToBuf(&buf[count],ycVal,2);
	}

	return count - bufIndex;
}

int CS101_B::AssembleYcVarWithVaild(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CYcVarPoint * ycvarBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_ME_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		//typeFinalYcval ycVal = ycvarBuf[i].getYcVal() + pristationPtr->getYcPlus(ycvarBuf[i].getYcIndex());
		typeFinalYcval ycVal = pristationPtr->getFinalYcVarVal(ycvarBuf[i]);

		count += ValToBuf(&buf[count],ycvarBuf[i].getYcIndex() + YC_START_ADDR_,InfoAddrLength_);
		count += ValToBuf(&buf[count],ycVal,2);
		count += ValToBuf(&buf[count],ycvarBuf[i].getYcQuality(),1);
	}

	return count - bufIndex;
}

int CS101_B::AssembleEndInit(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_EI_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_init,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = Remote_Control;

	return count - bufIndex;
}

int CS101_B::AssembleResetCmdCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,unsigned char QRP)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],C_RP_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QRP;

	return count - bufIndex;
}

int CS101_B::AssembleTestCmdCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,unsigned short FBP)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],C_TS_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	count += ValToBuf(&buf[count],FBP,2);

	return count - bufIndex;
}

int CS101_B::AssembleSingleSYxData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num)
{
	size_t count = bufIndex;


	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYxSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYXFRAME)
	{
		info_num = INFONUM_LIMIT_ALLYXFRAME;
	}

	if (startIndex + info_num > (int)pristationPtr->getYxSum())
	{
		info_num = pristationPtr->getYxSum() - startIndex;
	}

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SP_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],(info_num | DisableISQ<<((InfoNumLength_ - 1) * 8)),InfoNumLength_);
	count += ValToBuf(&buf[count],trans_req,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],startIndex + SYX_START_ADDR_,InfoAddrLength_);
	for (int i=0;i<info_num;i++)
	{
		unsigned char byteVal = (pristationPtr->getYxQuality(startIndex) & 0xf0) | (pristationPtr->getFinalYxVal(startIndex) & 0x01);
		count += ValToBuf(&buf[count],byteVal,1);
	}

	return count - bufIndex;
}

int CS101_B::AssembleSingleDYxData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num)
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYxSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYXFRAME)
	{
		info_num = INFONUM_LIMIT_ALLYXFRAME;
	}

	if (startIndex + info_num > (int)pristationPtr->getYxSum())
	{
		info_num = pristationPtr->getYxSum() - startIndex;
	}

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DP_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],(info_num | DisableISQ<<((InfoNumLength_ - 1) * 8)),InfoNumLength_);
	count += ValToBuf(&buf[count],trans_req,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],startIndex + DYX_START_ADDR_,InfoAddrLength_);
	for (int i=0;i<info_num;i++)
	{
		unsigned char byteVal = (pristationPtr->getYxQuality(startIndex) & 0xf0) | (pristationPtr->getFinalYxVal(startIndex) & 0x03);
		count += ValToBuf(&buf[count],byteVal,1);
	}

	return count - bufIndex;
}

int CS101_B::AssembleSingleYcData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num)
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYcSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYCFRAME)
	{
		info_num = INFONUM_LIMIT_ALLYCFRAME;
	}

	if (startIndex + info_num > (int)pristationPtr->getYcSum())
	{
		info_num = pristationPtr->getYcSum() - startIndex;
	}

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_ME_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],(info_num | DisableISQ<<((InfoNumLength_ - 1) * 8)),InfoNumLength_);
	count += ValToBuf(&buf[count],trans_req,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],startIndex + YC_START_ADDR_,InfoAddrLength_);
	for (int i=0;i<info_num;i++)
	{
		count += ValToBuf(&buf[count],pristationPtr->getFinalYcVal(startIndex),2);
		count += ValToBuf(&buf[count],pristationPtr->getYcQuality(startIndex),1);
	}

	return count - bufIndex;
}

int CS101_B::AssembleSingleYmData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num)
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYmSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLDDFRAME)
	{
		info_num = INFONUM_LIMIT_ALLDDFRAME;
	}

	if (startIndex + info_num > (int)pristationPtr->getYmSum())
	{
		info_num = pristationPtr->getYmSum() - startIndex;
	}

	count += ValToBuf(&buf[count],C_REQ_NA_3 | DIR_PRM_A | getSendFCB(pristationPtr) | ACT_FCV,1);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_IT_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],(info_num | DisableISQ<<((InfoNumLength_ - 1) * 8)),InfoNumLength_);
	count += ValToBuf(&buf[count],trans_req,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],startIndex + YM_START_ADDR_,InfoAddrLength_);
	for (int i=0;i<info_num;i++)
	{
		count += ValToBuf(&buf[count],pristationPtr->getOriYmVal(startIndex),4);
		count += ValToBuf(&buf[count],pristationPtr->getYmQuality(startIndex),1);
	}

	return count - bufIndex;
}

int CS101_B::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	switch (cmd.getCmdType())
	{
	case LINK_STATUS:
		buf[count++] = 0x10;
		break;

	case CONFIRM_ACK:
		buf[count++] = 0x10;
		break;

	case CONFIRM_NACK:
		buf[count++] = 0x10;
		break;

	case REQUIRE_LINK:
		buf[count++] = 0x10;
		break;

	case RESET_LINK:
		buf[count++] = 0x10;
		break;

	default:
		buf[count++] = 0x68;
		buf[count++] = 0;
		buf[count++] = 0;
		buf[count++] = 0x68;
		break;
	}

	return count - bufIndex;
}

int CS101_B::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
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
		ostr<<"S101_B规约不能从发送命令中获得pristation ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (cmd.getCmdType())
	{
	case LINK_STATUS:
		bytesAssemble = AssembleLinkStatus(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			CheckACD(bufIndex,buf,pristationPtr);
		}
		break;

	case CONFIRM_ACK:
		bytesAssemble = AssembleConfirmAck(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			CheckACD(bufIndex,buf,pristationPtr);
		}
		break;

	case CONFIRM_NACK:
		bytesAssemble = AssembleConfirmNack(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			CheckACD(bufIndex,buf,pristationPtr);
		}
		break;

	case REQUIRE_LINK:
		bytesAssemble = AssembleRequireLink(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			CheckACD(bufIndex,buf,pristationPtr);
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case RESET_LINK:
		bytesAssemble = AssembleResetLink(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			CheckACD(bufIndex,buf,pristationPtr);
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case YK_SEL_CON:
		{
			DataBase::stYkCmdPara yk_para;
			try
			{
				yk_para = boost::any_cast<DataBase::stYkCmdPara>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控选择返校命令的遥控点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			typeYktype yk_type = DataBase::YkOtherType;
			try
			{
				yk_type = pristationPtr->getYkType((yk_para).YkNo_);
			}
			catch(PublicSupport::dat2def_exception & err)
			{
				std::ostringstream ostr;
				ostr<<"遥控选择返校命令的遥控点号参数错误："<<err.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			if (yk_type == DataBase::YkClose)
			{
				if (pristationPtr->getbSYkDouble((yk_para).YkNo_))
				{
					if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleDoubleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_TYPE_CLOSE);
					}
					else
					{
						bytesAssemble = AssembleDoubleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_CLOSE_NEGATIVE);
					}

				}
				else
				{
					if((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleSingleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_CLOSE,trans_actcon);
					}
					else
					{
						bytesAssemble = AssembleSingleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_CLOSE,trans_deactcon);
					}
				}

			}
			else if (yk_type == DataBase::YkOpen)
			{
				if (pristationPtr->getbSYkDouble((yk_para).YkNo_))
				{
					if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleDoubleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_TYPE_OPEN);
					}
					else
					{
						bytesAssemble = AssembleDoubleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_OPEN_NEGATIVE);
					}

				}
				else
				{
					//if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					//{
					if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleSingleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_OPEN,trans_actcon);
					}
					else
					{
						bytesAssemble = AssembleSingleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_OPEN,trans_deactcon);
					}

					//}
					//else
					//{
					//	bytesAssemble = 0;
					//	DisconnectSubYkSig(pristationPtr,false);
					//	return -1;
					//}
				}

			}
			else
			{
				AddStatusLogWithSynT("遥控选择返校命令的遥控类型参数非法。\n");
				return -1;
			}

			if (bytesAssemble > 0)
			{
				//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus((yk_para).YkNo_),DataBase::YkSelBack))
				if ((pristationPtr->getYkPointPtr(yk_para.YkNo_))->BackSelEvent())
				{
					//std::ostringstream ostr;
					//ostr<<"curStatus:"<<(int)pristationPtr->getYkStatus((yk_para).YkNo_)<<"NextStatus:"<<(int)DataBase::YkSelBack<<std::endl;
					//AddStatusLogWithSynT(ostr.str());
					AddStatusLogWithSynT("解析遥控选择返校命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
					return -1;
				}

				ResetTimerYkSel(pristationPtr,(yk_para).YkNo_,false);
				DisconnectSubYkSig(pristationPtr,false);
				//pristationPtr->setYkStatus((yk_para).YkNo_,DataBase::YkSelBack);
				CheckACD(bufIndex,buf,pristationPtr);
				ResetTimerYkSelToExe(pristationPtr,yk_para.YkNo_,true);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case YK_EXE_CON:
		{
			DataBase::stYkCmdPara yk_para;
			try
			{
				yk_para = boost::any_cast<DataBase::stYkCmdPara>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控执行返校命令的遥控点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			typeYktype yk_type = DataBase::YkOtherType;
			try
			{
				yk_type = pristationPtr->getYkType((yk_para).YkNo_);
			}
			catch(PublicSupport::dat2def_exception & err)
			{
				std::ostringstream ostr;
				ostr<<"遥控执行返校命令的遥控点号参数错误："<<err.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			if (yk_type == DataBase::YkClose)
			{
				if (pristationPtr->getbSYkDouble((yk_para).YkNo_))
				{
					if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleDoubleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_TYPE_CLOSE);
					}
					else
					{
						bytesAssemble = AssembleDoubleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_CLOSE_NEGATIVE);
					}

				}
				else
				{
					if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleSingleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_CLOSE,trans_actcon);
					}
					else
					{
						bytesAssemble = AssembleSingleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_CLOSE,trans_deactcon);
					}

				}

			}
			else if (yk_type == DataBase::YkOpen)
			{
				if (pristationPtr->getbSYkDouble((yk_para).YkNo_))
				{
					if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleDoubleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_TYPE_OPEN);
					}
					else
					{
						bytesAssemble = AssembleDoubleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_OPEN_NEGATIVE);
					}

				}
				else
				{
					//if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					//{
					if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleSingleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_OPEN,trans_actcon);
					}
					else
					{
						bytesAssemble = AssembleSingleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_OPEN,trans_deactcon);
					}

					//}
					//else
					//{
					//	bytesAssemble = 0;
					//	DisconnectSubYkSig(pristationPtr,false);
					//	return -1;
					//}
				}

			}
			else
			{
				AddStatusLogWithSynT("遥控执行返校命令的遥控类型参数非法。\n");
				return -1;
			}

			if (bytesAssemble > 0)
			{
				//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus((yk_para).YkNo_),DataBase::YkExeBack))
				if((pristationPtr->getYkPointPtr(yk_para.YkNo_))->BackExeEvent())
				{
					//std::ostringstream ostr;
					//ostr<<"curStatus:"<<(int)pristationPtr->getYkStatus((yk_para).YkNo_)<<"NextStatus:"<<(int)DataBase::YkExeBack<<std::endl;
					//AddStatusLogWithSynT(ostr.str());
					AddStatusLogWithSynT("解析遥控执行返校命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
					return -1;
				}

				ResetTimerYkExe(pristationPtr,(yk_para).YkNo_,false);
				DisconnectSubYkSig(pristationPtr,false);
				//pristationPtr->setYkStatus((yk_para).YkNo_,DataBase::YkExeBack);
				AddSendCmdVal(YK_OVER_CON,YK_OVER_PRIORITY,pristationPtr,(int)(yk_para).YkNo_);
				CheckACD(bufIndex,buf,pristationPtr);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case YK_CANCEL_CON:
		{
			DataBase::stYkCmdPara yk_para;
			try
			{
				yk_para = boost::any_cast<DataBase::stYkCmdPara>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控取消返校命令的遥控点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			typeYktype yk_type = DataBase::YkOtherType;
			try
			{
				yk_type = pristationPtr->getYkType((yk_para).YkNo_);
			}
			catch(PublicSupport::dat2def_exception & err)
			{
				std::ostringstream ostr;
				ostr<<"遥控取消返校命令的遥控点号参数错误："<<err.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			if (yk_type == DataBase::YkClose)
			{
				if (pristationPtr->getbSYkDouble((yk_para).YkNo_))
				{
					if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleDoubleYKCancelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_TYPE_CLOSE);
					}
					else
					{
						bytesAssemble = AssembleDoubleYKCancelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_CLOSE_NEGATIVE);
					}

				}
				else
				{
					bytesAssemble = AssembleSingleYKCancelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_CLOSE);

				}

			}
			else if (yk_type == DataBase::YkOpen)
			{
				if (pristationPtr->getbSYkDouble((yk_para).YkNo_))
				{
					if ((yk_para).YkCode_ == RETURN_CODE_ACTIVE)
					{
						bytesAssemble = AssembleDoubleYKCancelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_TYPE_OPEN);
					}
					else
					{
						bytesAssemble = AssembleDoubleYKCancelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,DYK_OPEN_NEGATIVE);
					}

				}
				else
				{
					bytesAssemble = AssembleSingleYKCancelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,SYK_TYPE_OPEN);
				}

			}
			else
			{
				AddStatusLogWithSynT("遥控取消返校命令的遥控类型参数非法。\n");
				return -1;
			}

			if (bytesAssemble > 0)
			{
				//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus((yk_para).YkNo_),DataBase::YkCancelBack))
				if((pristationPtr->getYkPointPtr(yk_para.YkNo_))->BackCancelEvent())
				{
					//std::ostringstream ostr;
					//ostr<<"curStatus:"<<(int)pristationPtr->getYkStatus((yk_para).YkNo_)<<"NextStatus:"<<(int)DataBase::YkCancelBack<<std::endl;
					//AddStatusLogWithSynT(ostr.str());
					AddStatusLogWithSynT("解析遥控取消返校命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
					return -1;
				}

				ResetTimerYkCancel(pristationPtr,(yk_para).YkNo_,false);
				DisconnectSubYkSig(pristationPtr,false);
				//pristationPtr->setYkStatus((yk_para).YkNo_,DataBase::YkCancelBack);
				CheckACD(bufIndex,buf,pristationPtr);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case YK_OVER_CON:
		{
			int yk_no;
			try
			{
				yk_no = boost::any_cast<int>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控结束返校命令的遥控点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			typeYktype yk_type = DataBase::YkOtherType;
			try
			{
				yk_type = pristationPtr->getYkType(yk_no);
			}
			catch(PublicSupport::dat2def_exception & err)
			{
				std::ostringstream ostr;
				ostr<<"遥控结束返校命令的遥控点号参数错误："<<err.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			if (yk_type == DataBase::YkClose)
			{
				if (pristationPtr->getbSYkDouble(yk_no))
				{
					bytesAssemble = AssembleDoubleYKOver(bufIndex,buf,pristationPtr,yk_no,DYK_TYPE_CLOSE);
				}
				else
				{
					bytesAssemble = AssembleSingleYKOver(bufIndex,buf,pristationPtr,yk_no,SYK_TYPE_CLOSE);
				}

			}
			else if (yk_type == DataBase::YkOpen)
			{
				if (pristationPtr->getbSYkDouble(yk_no))
				{
					bytesAssemble = AssembleDoubleYKOver(bufIndex,buf,pristationPtr,yk_no,DYK_TYPE_OPEN);
				}
				else
				{
					bytesAssemble = AssembleSingleYKOver(bufIndex,buf,pristationPtr,yk_no,SYK_TYPE_OPEN);
				}

			}
			else
			{
				AddStatusLogWithSynT("遥控结束返校命令的遥控类型参数非法。\n");
				return -1;
			}

			if (bytesAssemble > 0)
			{
				//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(pristationPtr->getYkStatus(yk_no),DataBase::YkOver))
				if((pristationPtr->getYkPointPtr(yk_no))->OverYkEvent())
				{
					//std::ostringstream ostr;
					//ostr<<"curStatus:"<<pristationPtr->getYkStatus(yk_no)<<"NextStatus:"<<DataBase::YkOver<<std::endl;
					//AddStatusLogWithSynT(ostr.str());
					AddStatusLogWithSynT("解析遥控结束命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
					return -1;
				}

				//DisconnectSubYkSig(pristationPtr,false);
				//pristationPtr->setYkStatus(yk_no,DataBase::YkOver);
				CheckACD(bufIndex,buf,pristationPtr);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case SYN_TIME_CON:
		bytesAssemble = AssembleSynTimeCon(bufIndex,buf,pristationPtr,boost::posix_time::microsec_clock::local_time());
		if (bytesAssemble > 0)
		{
			//CheckACD(bufIndex,buf,pristationPtr);
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_ALL_DATA_CON:
		{
			bytesAssemble = AssembleCallDataCon(bufIndex,buf,pristationPtr);
			if (bytesAssemble > 0)
			{
				if (pristationPtr->getYxSum() > 0)
				{
					AddSendCmdVal(ALL_YX_DATA,ALL_YX_DATA_PRIORITY,pristationPtr,(int)0);
				}
				else if (pristationPtr->getYcSum() > 0)
				{
					AddSendCmdVal(ALL_YC_DATA,ALL_YC_DATA_PRIORITY,pristationPtr,(int)0);
				}
				else
				{
					AddSendCmdVal(CALL_ALL_DATA_OVER,CALL_ALL_DATA_OVER_PRIORITY,pristationPtr);
				}

				CheckACD(bufIndex,buf,pristationPtr);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case CALL_ALL_DATA_OVER:
		{
			bytesAssemble = AssembleCallDataOver(bufIndex,buf,pristationPtr);
			{
				CheckACD(bufIndex,buf,pristationPtr);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case CALL_ALL_YM_CON:
		{
			bytesAssemble = AssembleCallYMCon(bufIndex,buf,pristationPtr);
			if (bytesAssemble > 0)
			{
				if (pristationPtr->getYmSum() > 0)
				{
					AddSendCmdVal(ALL_YM_DATA,ALL_YM_DATA_PRIORITY,pristationPtr,(int)0);
				}
				else
				{
					AddSendCmdVal(CALL_ALL_YM_OVER,CALL_ALL_YM_OVER_PRIORITY,pristationPtr);
				}

				CheckACD(bufIndex,buf,pristationPtr);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case CALL_ALL_YM_OVER:
		{
			bytesAssemble = AssembleCallYMOver(bufIndex,buf,pristationPtr);
			{
				CheckACD(bufIndex,buf,pristationPtr);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case ALL_YX_DATA:
		{
			int startIndex;
			try
			{
				startIndex = boost::any_cast<int>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"全YX命令的起始YX点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());

				if(pristationPtr->getYcSum() > 0)
				{
					AddSendCmdVal(ALL_YC_DATA,ALL_YC_DATA_PRIORITY,pristationPtr,(int)0);
				}
				else
				{
					AddSendCmdVal(CALL_ALL_DATA_OVER,CALL_ALL_DATA_OVER_PRIORITY,pristationPtr);
				}

				return -1;
			}

			int leftNum = pristationPtr->getYxSum() - startIndex;
			if (leftNum > 0)
			{
				typeYxtype yxType = pristationPtr->getYxType(startIndex);
				size_t count = 0;
				for (size_t i=startIndex;i<pristationPtr->getYxSum();i++)
				{
					if (pristationPtr->getYxType(i) != yxType || count >= INFONUM_LIMIT_ALLYXFRAME)
					{
						break;
					}
					count++;
				}

				if (yxType == DataBase::double_yx_point)
				{
					bytesAssemble = AssembleAllDoubleYX(bufIndex,buf,pristationPtr,startIndex,count);
				}
				else
				{
					bytesAssemble = AssembleAllSingleYX(bufIndex,buf,pristationPtr,startIndex,count);
				}

				if (bytesAssemble > 0)
				{
					if (startIndex < (int)pristationPtr->getYxSum())
					{
						AddSendCmdVal(ALL_YX_DATA,ALL_YX_DATA_PRIORITY,pristationPtr,startIndex);
					}
					else
					{
						if(pristationPtr->getYcSum() > 0)
						{
							AddSendCmdVal(ALL_YC_DATA,ALL_YC_DATA_PRIORITY,pristationPtr,(int)0);
						}
						else
						{
							AddSendCmdVal(CALL_ALL_DATA_OVER,CALL_ALL_DATA_OVER_PRIORITY,pristationPtr);
						}
					}

					CheckACD(bufIndex,buf,pristationPtr);
					setWaitingForAnswer(cmd.getCommPoint());
				}
				else
				{
					if(pristationPtr->getYcSum() > 0)
					{
						AddSendCmdVal(ALL_YC_DATA,ALL_YC_DATA_PRIORITY,pristationPtr,(int)0);
					}
					else
					{
						AddSendCmdVal(CALL_ALL_DATA_OVER,CALL_ALL_DATA_OVER_PRIORITY,pristationPtr);
					}
				}
			}
			else
			{
				if(pristationPtr->getYcSum() > 0)
				{
					AddSendCmdVal(ALL_YC_DATA,ALL_YC_DATA_PRIORITY,pristationPtr,(int)0);
				}
				else
				{
					AddSendCmdVal(CALL_ALL_DATA_OVER,CALL_ALL_DATA_OVER_PRIORITY,pristationPtr);
				}
			}
		}
		break;

	case ALL_YC_DATA:
		{
			int startIndex;
			try
			{
				startIndex = boost::any_cast<int>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"全YC命令的起始YC点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());

				AddSendCmdVal(CALL_ALL_DATA_OVER,CALL_ALL_DATA_OVER_PRIORITY,pristationPtr);

				return -1;
			}

			int leftNum = pristationPtr->getYcSum() - startIndex;
			if (leftNum > 0)
			{
				if (leftNum > INFONUM_LIMIT_ALLYCFRAME)
				{
					bytesAssemble = AssembleAllYCWithVaild(bufIndex,buf,pristationPtr,startIndex,INFONUM_LIMIT_ALLYCFRAME);
				}
				else
				{
					bytesAssemble = AssembleAllYCWithVaild(bufIndex,buf,pristationPtr,startIndex,leftNum);
				}

				if (bytesAssemble > 0)
				{
					if (startIndex < (int)pristationPtr->getYcSum())
					{
						AddSendCmdVal(ALL_YC_DATA,ALL_YC_DATA_PRIORITY,pristationPtr,startIndex);
					}
					else
					{
						AddSendCmdVal(CALL_ALL_DATA_OVER,CALL_ALL_DATA_OVER_PRIORITY,pristationPtr);
					}

					CheckACD(bufIndex,buf,pristationPtr);
					setWaitingForAnswer(cmd.getCommPoint());
				}
				else
				{
					AddSendCmdVal(CALL_ALL_DATA_OVER,CALL_ALL_DATA_OVER_PRIORITY,pristationPtr);
				}
			}
			else
			{
				AddSendCmdVal(CALL_ALL_DATA_OVER,CALL_ALL_DATA_OVER_PRIORITY,pristationPtr);
			}
		}
		break;

	case ALL_YM_DATA:
		{
			int startIndex;
			try
			{
				startIndex = boost::any_cast<int>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"全YM命令的起始YM点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());

				AddSendCmdVal(CALL_ALL_YM_OVER,CALL_ALL_YM_OVER_PRIORITY,pristationPtr);

				return -1;
			}

			int leftNum = pristationPtr->getYmSum() - startIndex;
			if (leftNum > 0)
			{
				if (leftNum > INFONUM_LIMIT_ALLDDFRAME)
				{
					bytesAssemble = AssembleAllYM(bufIndex,buf,pristationPtr,startIndex,INFONUM_LIMIT_ALLDDFRAME);
				}
				else
				{
					bytesAssemble = AssembleAllYM(bufIndex,buf,pristationPtr,startIndex,leftNum);
				}

				if (bytesAssemble > 0)
				{
					if (startIndex < (int)pristationPtr->getYmSum())
					{
						AddSendCmdVal(ALL_YM_DATA,ALL_YM_DATA_PRIORITY,pristationPtr,startIndex);
					}
					else
					{
						AddSendCmdVal(CALL_ALL_YM_OVER,CALL_ALL_YM_OVER_PRIORITY,pristationPtr);
					}

					CheckACD(bufIndex,buf,pristationPtr);
					setWaitingForAnswer(cmd.getCommPoint());
				}
				else
				{
					AddSendCmdVal(CALL_ALL_YM_OVER,CALL_ALL_YM_OVER_PRIORITY,pristationPtr);
				}
			}
			else
			{
				AddSendCmdVal(CALL_ALL_YM_OVER,CALL_ALL_YM_OVER_PRIORITY,pristationPtr);
			}
		}
		break;

	case COS_DATA_UP:
		{
			DataBase::CCosPoint firstCos;
			int ret = pristationPtr->getFirstCosPoint(firstCos);
			if (ret >= 0)
			{
				int CountAssemblePoint = 0;
				boost::scoped_array<DataBase::CCosPoint> temp_buf(new DataBase::CCosPoint[INFONUM_LIMIT_COSFRAME + 1]);
				typeYxtype yxType = firstCos.getYxType();
				int leftPointNum = pristationPtr->getCosPointNum();
				while (CountAssemblePoint < INFONUM_LIMIT_COSFRAME && leftPointNum > 0 && yxType == firstCos.getYxType())
				{
					CountAssemblePoint += pristationPtr->loadCosPoints(&(temp_buf.get())[CountAssemblePoint],(leftPointNum <= (INFONUM_LIMIT_COSFRAME - CountAssemblePoint) ? leftPointNum : (INFONUM_LIMIT_COSFRAME - CountAssemblePoint)),yxType);
					leftPointNum = pristationPtr->getCosPointNum();
				}

				if (CountAssemblePoint > 0)
				{
					if (firstCos.getYxType() == DataBase::double_yx_point)
					{
						bytesAssemble = AssembleDoubleCOS(bufIndex,buf,pristationPtr,temp_buf.get(),CountAssemblePoint);
					}
					else
					{
						bytesAssemble = AssembleSingleCOS(bufIndex,buf,pristationPtr,temp_buf.get(),CountAssemblePoint);
					}

					if (bytesAssemble > 0)
					{
						CheckACD(bufIndex,buf,pristationPtr);
					}
				}
			}
		}
		break;

	case SOE_DATA_UP:
		{
			DataBase::CSoePoint firstSoe;
			int ret = pristationPtr->getFirstSoePoint(firstSoe);
			if (ret >= 0)
			{
				int CountAssemblePoint = 0;
				boost::scoped_array<DataBase::CSoePoint> temp_buf(new DataBase::CSoePoint[INFONUM_LIMIT_SOEFRAME + 1]);
				typeYxtype yxType = firstSoe.getYxType();
				int leftPointNum = pristationPtr->getSoePointNum();
				while (CountAssemblePoint < INFONUM_LIMIT_SOEFRAME && leftPointNum > 0 && yxType == firstSoe.getYxType())
				{
					CountAssemblePoint += pristationPtr->loadSoePoints(&(temp_buf.get())[CountAssemblePoint],(leftPointNum <= (INFONUM_LIMIT_SOEFRAME - CountAssemblePoint) ? leftPointNum : (INFONUM_LIMIT_SOEFRAME - CountAssemblePoint)),yxType);
					leftPointNum = pristationPtr->getSoePointNum();
				}

				if (CountAssemblePoint > 0)
				{
					if (firstSoe.getYxType() == DataBase::double_yx_point)
					{
						bytesAssemble = AssembleDoubleSOE(bufIndex,buf,pristationPtr,temp_buf.get(),CountAssemblePoint);
					}
					else
					{
						bytesAssemble = AssembleSingleSOE(bufIndex,buf,pristationPtr,temp_buf.get(),CountAssemblePoint);
					}

					if (bytesAssemble > 0)
					{
						CheckACD(bufIndex,buf,pristationPtr);
					}
				}
			}
		}
		break;

	case YCVAR_DATA_UP:
		{
			int leftPointNum = pristationPtr->getYcvarPointNum();
			if (leftPointNum > 0)
			{
				int CountAssemblePoint = 0;
				boost::scoped_array<DataBase::CYcVarPoint> temp_buf(new DataBase::CYcVarPoint[INFONUM_LIMIT_YCVARFRAME + 1]);
				while (CountAssemblePoint < INFONUM_LIMIT_YCVARFRAME && leftPointNum > 0)
				{
					CountAssemblePoint += pristationPtr->loadYcvarPoints(&(temp_buf.get())[CountAssemblePoint],(leftPointNum <= (INFONUM_LIMIT_YCVARFRAME - CountAssemblePoint) ? leftPointNum : (INFONUM_LIMIT_YCVARFRAME - CountAssemblePoint)));
					leftPointNum = pristationPtr->getYcvarPointNum();
				}

				if (CountAssemblePoint > 0)
				{
					bytesAssemble = AssembleYcVarWithVaild(bufIndex,buf,pristationPtr,temp_buf.get(),CountAssemblePoint);
					if (bytesAssemble > 0)
					{
						CheckACD(bufIndex,buf,pristationPtr);
					}
				}
			}
		}
		break;

	case END_INIT:
		bytesAssemble = AssembleEndInit(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			ConnectSubAliveSig();
			//CheckACD(bufIndex,buf,pristationPtr);
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case RESET_CMD:
		{
			unsigned char QRP = 1;
			try
			{
				QRP = boost::any_cast<unsigned char>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"复位命令返校的限定词参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}

			bytesAssemble = AssembleResetCmdCon(bufIndex,buf,pristationPtr,QRP);
			if (bytesAssemble > 0)
			{
				CheckACD(bufIndex,buf,pristationPtr);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case TEST_CMD:
		{
			unsigned short FBP = 0;
			try
			{
				FBP = boost::any_cast<unsigned short>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"测试命令返校的限定词参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}

			bytesAssemble = AssembleTestCmdCon(bufIndex,buf,pristationPtr,FBP);
			if (bytesAssemble > 0)
			{
				CheckACD(bufIndex,buf,pristationPtr);
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case SINGLE_YX_DATA:
		{
			try
			{
				int index = boost::any_cast<int>(cmd.getVal());
				if (pristationPtr->getYxType(index) == DataBase::double_yx_point)
				{
					bytesAssemble = AssembleSingleDYxData(bufIndex,buf,pristationPtr,index,1);
				}
				else
				{
					bytesAssemble = AssembleSingleSYxData(bufIndex,buf,pristationPtr,index,1);
				}

				if (bytesAssemble > 0)
				{
					CheckACD(bufIndex,buf,pristationPtr);
					setWaitingForAnswer(cmd.getCommPoint());
				}
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"读遥信命令的信息体点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}
		}
		break;

	case SINGLE_YC_DATA:
		{
			try
			{
				int index = boost::any_cast<int>(cmd.getVal());
				bytesAssemble = AssembleSingleYcData(bufIndex,buf,pristationPtr,index,1);
				if (bytesAssemble > 0)
				{
					CheckACD(bufIndex,buf,pristationPtr);
					setWaitingForAnswer(cmd.getCommPoint());
				}
				
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"读遥测命令的信息体点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}
		}
		break;

	case SINGLE_YM_DATA:
		{
			try
			{
				int index = boost::any_cast<int>(cmd.getVal());
				bytesAssemble = AssembleSingleYmData(bufIndex,buf,pristationPtr,index,1);
				if (bytesAssemble > 0)
				{
					CheckACD(bufIndex,buf,pristationPtr);
					setWaitingForAnswer(cmd.getCommPoint());
				}

			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"读遥脉命令的信息体点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}
		}
		break;

	default:
		{
			std::ostringstream ostr; 
			ostr<<"未定义的命令类型 cmdType ="<<cmd.getCmdType()<<"，丢弃该命令。"<<std::endl;
			AddStatusLogWithSynT(ostr.str());
			return -1;
		}
	}

	return bytesAssemble;
}

int CS101_B::AssembleFrameTail( size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd )
{
	int length = bufIndex - bufBegin;
	if (length <= 1)
	{
		return -1;
	}

	size_t count = bufIndex;

	switch (cmd.getCmdType())
	{
	case LINK_STATUS:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;

	case CONFIRM_ACK:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;

	case CONFIRM_NACK:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;
		
	case REQUIRE_LINK:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;

	case RESET_LINK:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;

	default:
		{
			int framelength = length - 4;
			if (framelength <= 0 || framelength > max_frame_length_)
			{
				return -1;
			}

			buf[count++] = CalcCheckSumByte(&buf[bufBegin + 4],framelength);
			buf[count++] = 0x16;
			buf[bufBegin + 1] = framelength & 0xff;
			buf[bufBegin + 2] = framelength & 0xff;
		}
		break;
	}

	return count - bufIndex;
}

void CS101_B::InitDefaultStartAddr()
{
	SYX_START_ADDR_ = DEFAULT_SYX_START_ADDR;                              //单点yx起始地址
	DYX_START_ADDR_ = DEFAULT_DYX_START_ADDR;                              //双点yx起始地址
	YC_START_ADDR_ =  DEFAULT_YC_START_ADDR;                               //yc起始地址
	SYK_START_ADDR_ = DEFAULT_SYK_START_ADDR;                              //单点yk起始地址
	DYK_START_ADDR_ = DEFAULT_DYK_START_ADDR;                              //双点yk起始地址
	YM_START_ADDR_ =  DEFAULT_YM_START_ADDR;                               //ym起始地址
	HIS_START_ADDR_ = DEFAULT_HIS_START_ADDR;                              //历史数据起始地址
}

void CS101_B::InitDefaultFrameElem()
{
	FrameTypeLength_ = DEFAULT_FrameTypeLength;                             //报文类型标识的字节长度
	InfoNumLength_ =   DEFAULT_InfoNumLength;                               //信息体数目标识的字节长度
	TransReasonLength_ = DEFAULT_TransReasonLength;                         //传送原因标识的字节长度
	AsduAddrLength_ = DEFAULT_AsduAddrLength;                               //装置地址标识的字节长度
	InfoAddrLength_ = DEFAULT_InfoAddrLength;                               //信息体地址标识的字节长度
}

void CS101_B::InitFrameLocation(size_t frameHead)
{
	FrameTypeLocation_ = frameHead;
	InfoNumLocation_ = FrameTypeLocation_ + FrameTypeLength_;
	TransReasonLocation_ = InfoNumLocation_ + InfoNumLength_;
	AsduAddrLocation_ = TransReasonLocation_ + TransReasonLength_;
	InfoAddrLocation_ = AsduAddrLocation_ + AsduAddrLength_;
	DataLocation_ = InfoAddrLocation_ + InfoAddrLength_;
}

void CS101_B::InitDefaultTimeOut()
{
	timeOutYkSel_ = DEFAULT_timeOutYkSel;
	timeOutYkExe_ = DEFAULT_timeOutYkExe;
	timeOutYkCancel_ = DEFAULT_timeOutYkCancel;
	timeOutCallPara_ = DEFAULT_timeOutCallPara;
	timeOutSetPara_ = DEFAULT_timeOutSetPara;
	timeOutYkSelToExe_ = DEFAULT_timeOutYkSelToExe;
	timeOutCirCle_ = DEFAULT_timeOutCirCle;
}

void CS101_B::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	timerYkSel_.reset(new deadline_timer(io_service,seconds(timeOutYkSel_)));
	AddTimer(timerYkSel_);

	timerYkExe_.reset(new deadline_timer(io_service,seconds(timeOutYkExe_)));
	AddTimer(timerYkExe_);

	timerYkCancel_.reset(new deadline_timer(io_service,seconds(timeOutYkCancel_)));
	AddTimer(timerYkCancel_);

	timerCallPara_.reset(new deadline_timer(io_service,seconds(timeOutCallPara_)));
	AddTimer(timerCallPara_);

	timerSetPara_.reset(new deadline_timer(io_service,seconds(timeOutSetPara_)));
	AddTimer(timerSetPara_);

	timerYkSelToExe_.reset(new deadline_timer(io_service,seconds(timeOutYkSelToExe_)));
	AddTimer(timerYkSelToExe_);

	timerCirCle_.reset(new deadline_timer(io_service,seconds(timeOutCirCle_)));
	AddTimer(timerCirCle_);
}

void CS101_B::handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
{
	if (!error)
	{
		if (point)
		{
			AddStatusLogWithSynT("S101_B规约遥控选择命令超时。\n");

			if (point->getCommPointType() == PRISTATION_NODE)
			{
				share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(point);
				if (pristationPtr)
				{
					DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_TIMEOUT);
					AddWaitCmdVal(YK_SEL_CON,YK_SEL_CON_PRIORITY,point,ykPara);
					AddStatusLogWithSynT("返回遥控选择否定确认报文。\n");

					//pristationPtr->setYkStatus(yk_no,DataBase::YkSelTimeOut);
					(pristationPtr->getYkPointPtr(yk_no))->TimeOutEvent();
					DisconnectSubYkSig(pristationPtr,true);
				}
			}				
		}
	}
}

void CS101_B::handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
{
	if (!error)
	{
		if (point)
		{
			AddStatusLogWithSynT("S101_B规约遥控执行命令超时。\n");

			if (point->getCommPointType() == PRISTATION_NODE)
			{
				share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(point);
				if (pristationPtr)
				{
					DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_TIMEOUT);
					AddWaitCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,point,ykPara);
					AddStatusLogWithSynT("返回遥控执行否定确认报文。\n");

					//pristationPtr->setYkStatus(yk_no,DataBase::YkExeTimeOut);
					(pristationPtr->getYkPointPtr(yk_no))->TimeOutEvent();
					DisconnectSubYkSig(pristationPtr,true);
				}
			}
		}
	}
}

void CS101_B::handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
{
	if (!error)
	{
		if (point)
		{
			AddStatusLogWithSynT("S101_B规约遥控取消命令超时。\n");

			if (point->getCommPointType() == PRISTATION_NODE)
			{
				share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(point);
				if (pristationPtr)
				{
					DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_TIMEOUT);
					AddWaitCmdVal(YK_CANCEL_CON,YK_CANCEL_CON_PRIORITY,point,ykPara);
					AddStatusLogWithSynT("返回遥控取消否定确认报文。\n");

					//pristationPtr->setYkStatus(yk_no,DataBase::YkCancelTimeOut);
					(pristationPtr->getYkPointPtr(yk_no))->TimeOutEvent();
					DisconnectSubYkSig(pristationPtr,true);
				}
			}
		}
	}
}

void CS101_B::handle_timerCallPara(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		if (point)
		{
			CmdConSig_(CALL_EXTEND_PARA,RETURN_CODE_TIMEOUT,point,0);
		}
	}
}

void CS101_B::handle_timerSetPara(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		if (point)
		{
			CmdConSig_(SET_EXTEND_PARA,RETURN_CODE_TIMEOUT,point,0);
		}
	}
}

void CS101_B::handle_timerYkSelToExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
{
	if (!error)
	{
		if (point)
		{
			if (point->getCommPointType() == PRISTATION_NODE)
			{
				share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(point);
				if (pristationPtr)
				{
					//pristationPtr->setYkStatus(yk_no,DataBase::YkReady);
					(pristationPtr->getYkPointPtr(yk_no))->TimeOutEvent();
					//DisconnectSubYkSig(pristationPtr,true);
				}
			}
		}
	}
}

void CS101_B::handle_timerCirCle(const boost::system::error_code& error,share_commpoint_ptr point)
{

}

void CS101_B::ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerYkSel_->expires_from_now(boost::posix_time::seconds(timeOutYkSel_));
		}
		else
		{
			timerYkSel_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerYkSel_->async_wait(boost::bind(&CS101_B::handle_timerYkSel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkSel_->cancel();
	}
}

void CS101_B::ResetTimerYkExe(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkExe_->async_wait(boost::bind(&CS101_B::handle_timerYkExe,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkExe_->cancel();
	}
}

void CS101_B::ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerYkCancel_->expires_from_now(boost::posix_time::seconds(timeOutYkCancel_));
		}
		else
		{
			timerYkCancel_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerYkCancel_->async_wait(boost::bind(&CS101_B::handle_timerYkCancel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkCancel_->cancel();
	}
}

void CS101_B::ResetTimerCallPara(share_commpoint_ptr point,bool bContinue /*= false*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerCallPara_->expires_from_now(boost::posix_time::seconds(timeOutCallPara_));
		}
		else
		{
			timerCallPara_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerCallPara_->async_wait(boost::bind(&CS101_B::handle_timerCallPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallPara_->cancel();
	}
}

void CS101_B::ResetTimerSetPara(share_commpoint_ptr point,bool bContinue /*= false*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerSetPara_->expires_from_now(boost::posix_time::seconds(timeOutSetPara_));
		}
		else
		{
			timerSetPara_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerSetPara_->async_wait(boost::bind(&CS101_B::handle_timerSetPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerSetPara_->cancel();
	}
}

void CS101_B::ResetTimerYkSelToExe(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerYkSelToExe_->expires_from_now(boost::posix_time::seconds(timeOutYkSelToExe_));
		}
		else
		{
			timerYkSelToExe_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerYkSelToExe_->async_wait(boost::bind(&CS101_B::handle_timerYkSelToExe,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkSelToExe_->cancel();
	}
}

void CS101_B::ResetTimerCirCle(share_commpoint_ptr point,size_t yk_no,bool bContinue /* = false */,unsigned short val /* = 0 */)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerCirCle_->expires_from_now(boost::posix_time::seconds(timeOutCirCle_));
		}
		else
		{
			timerCirCle_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerCirCle_->async_wait(boost::bind(&CS101_B::handle_timerCirCle,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCirCle_->cancel();
	}
}

//para api
int CS101_B::setSYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	SYX_START_ADDR_ = val;

	return 0;
}

int CS101_B::setDYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	DYX_START_ADDR_ = val;

	return 0;
}

int CS101_B::setYC_START_ADDR(size_t val)
{
	if (val < 0x101 || val >= 0x6001)
	{
		return -1;
	}

	YC_START_ADDR_ = val;

	return 0;
}

int CS101_B::setSYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	SYK_START_ADDR_ = val;

	return 0;
}

int CS101_B::setDYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	DYK_START_ADDR_ = val;

	return 0;
}

int CS101_B::setYM_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	YM_START_ADDR_ = val;

	return 0;
}

int CS101_B::setHIS_START_ADDR(size_t val)
{
	HIS_START_ADDR_ = val;

	return 0;
}

int CS101_B::setFrameTypeLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameTypeLength_ = val;

	return 0;
}

int CS101_B::setInfoNumLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	InfoNumLength_ = val;

	return 0;
}

int CS101_B::setTransReasonLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	TransReasonLength_ = val;

	return 0;
}

int CS101_B::setAsduAddrLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	AsduAddrLength_ = val;

	return 0;
}

int CS101_B::setInfoAddrLength(unsigned short val)
{
	if (val <= 0 || val > 12)
	{
		return -1;
	}

	InfoAddrLength_ = val;

	return 0;
}

int CS101_B::setTimeOutYkSel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkSel_ = val;

	return 0;
}

int CS101_B::setTimeOutYkExe(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkExe_ = val;

	return 0;
}

int CS101_B::setTimeOutYkCancel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkCancel_ = val;

	return 0;
}

int CS101_B::setTimeOutCallPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutCallPara_ = val;

	return 0;
}

int CS101_B::setTimeOutSetPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutSetPara_ = val;

	return 0;
}

};//namespace Protocol
