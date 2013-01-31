#include <boost/bind.hpp>
//#include <boost/lambda/lambda.hpp>
//#include <boost/algorithm/string/trim.hpp>
//#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "H101.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/Terminal.h"
#include "../DataBase/YkPoint.h"
#include "../DataBase/YxPoint.h"
#include "../DataBase/YCPoint.h"

namespace Protocol{

const std::string strDefaultCfg = "H101Cfg.xml";
size_t CH101::H101ObjectCounter_ = 1;

//针对101规约的YK功能码
const unsigned char DYK_TYPE_OPEN = 0x01;
const unsigned char DYK_TYPE_CLOSE = 0x02;
const unsigned char SYK_TYPE_OPEN = 0;
const unsigned char SYK_TYPE_CLOSE = 0x01;

//针对101规约的控制域定义
const unsigned char DIR_PRM = 0x40;
const unsigned char ACT_FCB = 0x20;
const unsigned char NACK_FCB = 0;
const unsigned char ACT_FCV = 0x10;
const unsigned char NACK_FCV = 0;
const unsigned char ACT_ACD = 0x20;
const unsigned char NACK_ACD = 0;

//针对101规约的传送原因定义
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
const unsigned char trans_file = 0x0d;
const unsigned char trans_all = 0x14;

//针对101规约的报文类型标识定义
const unsigned char M_SP_NA_1 = 0x01;
const unsigned char M_SP_TA_1 = 0x02;
const unsigned char M_DP_NA_1 = 0x03;
//const unsigned char M_BO_NA_1 = 0x07;
const unsigned char M_ME_NA_1 = 0x09;
const unsigned char M_ME_NB_1 = 0x0b;
//const unsigned char M_ME_NC_1 = 0x0d;
const unsigned char M_IT_NA_1 = 0x0f;
const unsigned char M_PS_NA_1 = 0x14;
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
const unsigned char M_CS_NA_1 = 0x67;
const unsigned char M_CD_NA_1 = 0x6a;

//针对101规约的功能码定义-监视方向
const unsigned char M_CON_NA_3 = 0x00;                 // <0> ：=确认帧     确认
const unsigned char M_BY_NA_3 = 0x01;                  // <1> ：=确认帧     链路忙，未收到报文
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

const unsigned char EnableISQ = 0x00;
const unsigned char DisableISQ = 0x80;
const unsigned char QOI = 0x14;

const std::string strTransDelay = "TransDelay";

//类成员函数
CH101::CH101(boost::asio::io_service & io_service)
			:CProtocol(io_service)
{
	SynCharNum_ = 5;
	bInitQuality_ = true;
	bActiveRepeatFrame_ = true;

	bTransDelay_ = false;

	InitObjectIndex();
	InitDefaultStartAddr();
	InitDefaultFrameElem();
	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);

	EnablePriKey();
}

CH101::~CH101(void)
{
	H101ObjectCounter_--;
}

int CH101::LoadXmlCfg(std::string filename)
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
		if (xml.FindElem(strTimeOutQueryUnActivePoint))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				if(!setTimeOutQueryUnActivePoint(timeout))
				{
					bUseTimeOutQueryUnActivePoint_ = true;
				}
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutQueryUnActivePoint(DEFAULT_timeOutQueryUnActivePoint);
				bUseTimeOutQueryUnActivePoint_ = false;
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutRequireLink))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutRequrieLink(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutRequrieLink(DEFAULT_timeOutRequireLink);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutCallAllData))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutCallAllData(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutCallAllData(DEFAULT_timeOutCallAllData);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutCallAllDD))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutCallAllDD(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutCallAllDD(DEFAULT_timeOutCallAllDD);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutSynTime))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutSynTime(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutSynTime(DEFAULT_timeOutSynTime);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutHeartFrame))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutHeartFrame(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutHeartFrame(DEFAULT_timeOutHeartFrame);
			}
		}

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

	xml.ResetMainPos();
	if (xml.FindElem(strTransDelay))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			bTransDelay_ = true;
		}
		else
		{
			bTransDelay_ = false;
		}
	}

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CH101::SaveXmlCfg(std::string filename)
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

	if (bUseTimeOutQueryUnActivePoint_ && (timeOutQueryUnActivePoint_ != DEFAULT_timeOutQueryUnActivePoint))
	{
		xml.AddElem(strTimeOutQueryUnActivePoint,timeOutQueryUnActivePoint_);
		bSave = true;
	}

	if (timeOutRequireLink_ != DEFAULT_timeOutRequireLink)
	{
		xml.AddElem(strTimeOutRequireLink,timeOutRequireLink_);
		bSave = true;
	}

	if (timeOutCallAllData_ != DEFAULT_timeOutCallAllData)
	{
		xml.AddElem(strTimeOutCallAllData,timeOutCallAllData_);
		bSave = true;
	}

	if (timeOutCallAllDD_ != DEFAULT_timeOutCallAllDD)
	{
		xml.AddElem(strTimeOutCallAllDD,timeOutCallAllDD_);
		bSave = true;
	}

	if (timeOutSynTime_ != DEFAULT_timeOutSynTime)
	{
		xml.AddElem(strTimeOutSynTime,timeOutSynTime_);
		bSave = true;
	}

	if (timeOutHeartFrame_ != DEFAULT_timeOutHeartFrame)
	{
		xml.AddElem(strTimeOutHeartFrame,timeOutHeartFrame_);
		bSave = true;
	}

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


	if (bTransDelay_)
	{
		xml.AddElem(strTransDelay,strboolTrue);
	}

	xml.OutOfElem();

	xml.Save(filename);
}

void CH101::InitObjectIndex()
{
	ProtocolObjectIndex_ = H101ObjectCounter_++;
}

int CH101::InitProtocol()
{

	CProtocol::InitProtocol();

	InitFrameLocation(5 + AsduAddrLength_);

	if(getCommPointSum() > 0)
	{
		share_commpoint_ptr nextPoint = getFirstCommPoint();
		if (nextPoint)
		{
			AddSendCmdVal(REQUIRE_LINK,REQUIRE_LINK_PRIORITY,nextPoint);
		}
	}
	
	AddStatusLogWithSynT("H101规约的通道打开成功。\n");

	return 0;
}

void CH101::UninitProtocol()
{
	CProtocol::UninitProtocol();

	DisableAllCommPoints();

	AddStatusLogWithSynT("H101规约的通道关闭成功。\n");
}

int CH101::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
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
	else if(buf[0] == 0xe5)
	{
		exceptedBytes = 1;
		return 0;
	}
	
	return -1;
}

int CH101::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	size_t sum = 0;

	if ((exceptedBytes == 1) && (buf[0] == 0xe5))
	{
		return 0;
	}

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

int CH101::ParseShortFrame(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	unsigned char ACD = buf[1] & ACT_ACD;
	if (ACD == ACT_ACD)
	{
		AddOnlySendCmdWithoutVal(CALL_PRIMARY_DATA,CALL_PRIMARY_DATA_PRIORITY,terminalPtr,boost::any());
	}

	unsigned char ContrCode = buf[1] & 0x0f;

	switch (ContrCode)
	{
	case M_CON_NA_3: //肯定认可
		switch (getLastSendCmd())
		{
		case RESET_LINK:
			if (bTransDelay_)
			{
				AddOnlySendCmdByCmdType(TRANS_DELAY_ACT,TRANS_DELAY_ACT_PRIORITY,terminalPtr,boost::any());
				AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,terminalPtr);
				AddOnlySendCmdByCmdType(CALL_SECONDARY_DATA,CALL_SECONDARY_DATA_PRIORITY,terminalPtr,boost::any());
			}
			else
			{
				AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,terminalPtr);
				AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,terminalPtr);
				AddOnlySendCmdByCmdType(CALL_SECONDARY_DATA,CALL_SECONDARY_DATA_PRIORITY,terminalPtr,boost::any());
			}

			if (ACD == ACT_ACD)
			{
				AddOnlySendCmdWithoutVal(CALL_PRIMARY_DATA,HIGH_PRIORITY,terminalPtr,boost::any());
			}
			break;

		case YK_SEL_ACT:
			if (ACD == 0)
			{
				AddSendCmdVal(CALL_SECONDARY_DATA,CALL_SECONDARY_DATA_PRIORITY,terminalPtr);
			}
			break;

		case YK_EXE_ACT:
			if (ACD == 0)
			{
				AddSendCmdVal(CALL_SECONDARY_DATA,CALL_SECONDARY_DATA_PRIORITY,terminalPtr);
			}
			break;

		case YK_CANCEL_ACT:
			if (ACD == 0)
			{
				AddSendCmdVal(CALL_SECONDARY_DATA,CALL_SECONDARY_DATA_PRIORITY,terminalPtr);
			}
			break;

		case CALL_ALL_DATA_ACT:
			if (ACD == 0)
			{
				AddSendCmdVal(CALL_SECONDARY_DATA,CALL_SECONDARY_DATA_PRIORITY,terminalPtr);
			}
			break;

		case CALL_ALL_DD_ACT:
			if (ACD == 0)
			{
				AddSendCmdVal(CALL_SECONDARY_DATA,CALL_SECONDARY_DATA_PRIORITY,terminalPtr);
			}
			break;

		case SYN_TIME_ACT:
			if (ACD == 0)
			{
				AddSendCmdVal(CALL_SECONDARY_DATA,CALL_SECONDARY_DATA_PRIORITY,terminalPtr);
			}
			break;

		case TRANS_DELAY_DOWNLOAD:
			terminalPtr->setDelayCommPointFlag(true);
			AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,terminalPtr);
			break;

		default:
			break;
		}
		break;

	case M_BY_NA_3: //否定认可
		{
			terminalPtr->setCommActive(false);
			AddSendCmdVal(REQUIRE_LINK,REQUIRE_LINK_PRIORITY,terminalPtr);
			return -1;
		}
		break;

	case M_NV_NA_3: //无所请求的数据
		if (getLastSendCmd() == RESET_LINK)
		{
			terminalPtr->setCommActive(false);
			AddSendCmdVal(REQUIRE_LINK,REQUIRE_LINK_PRIORITY,terminalPtr);
			return -1;
		}
		break;

	case M_LKR_NA_3: //链路状态
		if (getLastSendCmd() == REQUIRE_LINK)
		{
			AddSendCmdVal(RESET_LINK,REQUIRE_LINK_PRIORITY,terminalPtr);
		}
		break;

	default:
		{
			std::ostringstream ostr;
			ostr<<"接收报文错误，未定义的报文类型 Control_Code ="<<ContrCode<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;
	}

	return 0;
}

int CH101::ParseLongFrame(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	unsigned char ACD = buf[4] & ACT_ACD;
	if (ACD == ACT_ACD)
	{
		AddOnlySendCmdWithoutVal(CALL_PRIMARY_DATA,CALL_PRIMARY_DATA_PRIORITY,terminalPtr,boost::any());
	}

	size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识
	size_t TransReason = BufToVal(&buf[TransReasonLocation_],TransReasonLength_);  //传送原因
	unsigned char Data_Code = buf[DataLocation_] & 0x80;

	switch (FrameType)
	{
	case M_SC_NA_1:
		switch (TransReason)
		{
		case trans_actcon: //single yk
			if (Data_Code == 0x80)
			{
				ParseSingleYKSelCon(buf,terminalPtr);
			}
			else if (!Data_Code)
			{
				ParseSingleYKExeCon(buf,terminalPtr);
			}
			else
			{
				std::ostringstream ostr;
				ostr<<"YK选择或执行返校报文错误，未定义的YK_CODE ="<<Data_Code<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;

		case trans_deactcon:
			ParseSingleYKCancelCon(buf,terminalPtr);
			break;

		case trans_actterm:
			ParseSingleYKOverCon(buf,terminalPtr);
			break;

		default:
			{
				std::ostringstream ostr;
				ostr<<"YK返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;
		}
		break;

	case M_DC_NA_1:
		switch (TransReason)
		{
		case trans_actcon: //double yk
			if (Data_Code == 0x80)
			{
				ParseDoubleYKSelCon(buf,terminalPtr);
			}
			else if (!Data_Code)
			{
				ParseDoubleYKExeCon(buf,terminalPtr);
			}
			else
			{
				std::ostringstream ostr;
				ostr<<"YK选择或执行返校报文错误，未定义的YK_CODE ="<<Data_Code<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;

		case trans_deactcon:
			ParseDoubleYKCancelCon(buf,terminalPtr);
			break;

		case trans_actterm:
			ParseDoubleYKOverCon(buf,terminalPtr);
			break;

		default:
			{
				std::ostringstream ostr;
				ostr<<"YK返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;
		}
		break;

	case M_IC_NA_1: //call all data 
		switch (TransReason)
		{
		case trans_actcon:
			ParseAllDataCallCon(buf,terminalPtr);
			break;

		case trans_actterm:
			ParseAllDataCallOver(buf,terminalPtr);
			break;

		default:
			{
				std::ostringstream ostr;
				ostr<<"总召唤返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;
		}
		break;

	case M_SP_NA_1: //single yx
		if (TransReason == trans_all)
		{
			ParseAllSingleYX(buf,terminalPtr);
		}
		else if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseSingleCOS(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"单点YX返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_DP_NA_1: //double yx
		if (TransReason == trans_all)
		{
			ParseAllDoubleYX(buf,terminalPtr);
		}
		else if(TransReason == trans_spont || TransReason == trans_req)
		{
			ParseDoubleCOS(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"双点YX返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_PS_NA_1: //byte yx
		if (TransReason == trans_all)
		{
			ParseAllYXByte(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"YX字返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_ME_ND_1: //yc
		if (TransReason == trans_all)
		{
			ParseAllYCData(buf,terminalPtr);
		} 
		else if(TransReason == trans_spont || TransReason == trans_req)
		{
			ParseYCCH(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"YC返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_ME_NA_1: //yc with yc_valid
		if (TransReason == trans_all)
		{
			ParseAllYCDataWithValid(buf,terminalPtr);
		}
		else if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseYCCHWithValid(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"带校验的YC返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_ME_NB_1: //yc with yc_valid 标度化值
		if (TransReason == trans_all)
		{
			ParseAllYCDataWithValid(buf,terminalPtr);
		}
		else if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseYCCHWithValid(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"带校验的YC返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_CS_NA_1: //time
		ParseSynTimeCon(buf,terminalPtr);
		break;

	case M_CD_NA_1: //trans delay
		if (TransReason == trans_actcon)
		{
			ParseTransDelayCon(buf,terminalPtr);
		}
		break;

	case M_SP_TA_1: //short synt single soe
		if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseSingleSOE(buf,terminalPtr);
		} 
		else
		{
			std::ostringstream ostr;
			ostr<<"短时标单点SOE返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_SP_TB_1: //long synt single soe
		if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseSingleSOE(buf,terminalPtr);
		} 
		else
		{
			std::ostringstream ostr;
			ostr<<"长时标单点SOE返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_DP_TB_1: //long synt double soe
		if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseDoubleSOE(buf,terminalPtr);
		} 
		else
		{
			std::ostringstream ostr;
			ostr<<"长时标双点SOE返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_CI_NA_1: //call all dd
		switch (TransReason)
		{
		case trans_actcon:
			ParseAllYMCallCon(buf,terminalPtr);
			break;

		case trans_actterm:
			ParseAllYMCallOver(buf,terminalPtr);
			break;

		default:
			{
				std::ostringstream ostr;
				ostr<<"召唤电度返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;
		}
		break;

	case M_IT_NA_1: //dd data
		if (TransReason == trans_all)
		{
			ParseAllYMData(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"电度数据返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_EI_NA_1: //初始化结束报文
		ParseEndInit(buf,terminalPtr);
		break;

		/********************自定义104报文*****************/
	case 0x81: //参数数据报文
		if (TransReason == trans_all)
		{
			ParseAllParaData(buf,terminalPtr);
		} 
		else
		{
			std::ostringstream ostr;
			ostr<<"召唤装置参数返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case 0x82: //参数设置报文
		if (TransReason == trans_actcon) //肯定响应
		{
			ParseSetParaCon(buf,terminalPtr);
		} 
		else if(TransReason == trans_actterm) //否定响应
		{
			ParseSetParaErr(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"设置装置参数返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case 0x83: //二级装置信息报文
		if (TransReason == trans_all)
		{
			ParseExtendRTUInfo(buf,terminalPtr);
		} 
		else
		{
			std::ostringstream ostr;
			ostr<<"召唤装置信息返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case 0x91: //历史数据
		if (TransReason == trans_all)
		{
			ParseHistoryData(buf,terminalPtr);
		} 
		else if(TransReason == trans_req)
		{
			ParseNoHistoryData(buf,terminalPtr);
		}
		else if(TransReason == trans_actterm)
		{
			ParseEndHistoryData(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"召唤历史数据返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case 0x94: //yx record
		if (TransReason == trans_all)
		{
			ParseYxRecordData(buf,terminalPtr);
		} 
		else if(TransReason == trans_actcon)
		{
			ParseYxRecordEnd(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"召唤YX记录返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case 0x95: //yk record
		if (TransReason == trans_all)
		{
			ParseYkRecordData(buf,terminalPtr);
		}
		else if(TransReason == trans_actcon)
		{
			ParseYkRecordEnd(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"召唤YK记录返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case 0x96: //fault record
		if (TransReason == trans_all)
		{
			ParseFaultRecordData(buf,terminalPtr);
		}
		else if (TransReason == trans_actcon)
		{
			ParseFaultRecordEnd(buf,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"召唤故障记录返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
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

int CH101::ParseDoubleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)terminalPtr->getRecvYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控选择返校报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkSelCon))
	if((terminalPtr->loadYkPointPtr(yk_no))->SelResponEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkSelCon<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控选择返校报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if (yk_type == DYK_TYPE_OPEN)
	{
		if(terminalPtr->getYkType(yk_no) != DataBase::YkOpen)
		{
			ResetTimerYkSel(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_SEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控选择返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else if (yk_type == DYK_TYPE_CLOSE)
	{
		if (terminalPtr->getYkType(yk_no) != DataBase::YkClose)
		{
			ResetTimerYkSel(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_SEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控选择返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else
	{
		ResetTimerYkSel(terminalPtr,yk_no,false);
		//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
		(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
		CmdConSig_(YK_SEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
		AddStatusLogWithSynT("遥控选择返校报文的遥控类型与预期不符。\n");
		return -1;
	}

	ResetTimerYkSel(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkSelCon);
	CmdConSig_(YK_SEL_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);

	return 0;


}

int CH101::ParseDoubleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)terminalPtr->getRecvYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控执行返校报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkExeCon))
	if((terminalPtr->loadYkPointPtr(yk_no))->ExeResponEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkExeCon<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控执行返校报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if (yk_type == DYK_TYPE_OPEN)
	{
		if(terminalPtr->getYkType(yk_no) != DataBase::YkOpen)
		{
			ResetTimerYkExe(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_EXE_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控执行返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else if (yk_type == DYK_TYPE_CLOSE)
	{
		if (terminalPtr->getYkType(yk_no) != DataBase::YkClose)
		{
			ResetTimerYkExe(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_EXE_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控执行返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else
	{
		ResetTimerYkExe(terminalPtr,yk_no,false);
		//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
		(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
		CmdConSig_(YK_EXE_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
		AddStatusLogWithSynT("遥控执行返校报文的遥控类型与预期不符。\n");
		return -1;
	}

	ResetTimerYkExe(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkExeCon);
	CmdConSig_(YK_EXE_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);

	return 0;
}

int CH101::ParseDoubleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)terminalPtr->getRecvYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控取消返校报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkCancelCon))
	if((terminalPtr->loadYkPointPtr(yk_no))->CancelResponEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkCancelCon<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控取消返校报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if (yk_type == DYK_TYPE_OPEN)
	{
		if(terminalPtr->getYkType(yk_no) != DataBase::YkOpen)
		{
			ResetTimerYkCancel(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_CANCEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控取消返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else if (yk_type == DYK_TYPE_CLOSE)
	{
		if (terminalPtr->getYkType(yk_no) != DataBase::YkClose)
		{
			ResetTimerYkCancel(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_CANCEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控取消返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else
	{
		ResetTimerYkCancel(terminalPtr,yk_no,false);
		//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
		(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
		CmdConSig_(YK_CANCEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
		AddStatusLogWithSynT("遥控取消返校报文的遥控类型与预期不符。\n");
		return -1;
	}

	ResetTimerYkCancel(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkCancelCon);
	CmdConSig_(YK_CANCEL_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);

	return 0;
}

int CH101::ParseDoubleYKOverCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	ResetTimerYkSel(terminalPtr,yk_no,false);
	ResetTimerYkExe(terminalPtr,yk_no,false);
	ResetTimerYkCancel(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
	(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();

	return 0;
}

int CH101::ParseSingleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)terminalPtr->getRecvYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控选择返校报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkSelCon))
	if((terminalPtr->loadYkPointPtr(yk_no))->SelResponEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkSelCon<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控选择返校报文，但是遥控当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if (yk_type == SYK_TYPE_OPEN)
	{
		if(terminalPtr->getYkType(yk_no) != DataBase::YkOpen)
		{
			ResetTimerYkSel(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_SEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控选择返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else if (yk_type == SYK_TYPE_CLOSE)
	{
		if (terminalPtr->getYkType(yk_no) != DataBase::YkClose)
		{
			ResetTimerYkSel(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_SEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控选择返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else
	{
		ResetTimerYkSel(terminalPtr,yk_no,false);
		//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
		(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
		CmdConSig_(YK_SEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
		AddStatusLogWithSynT("遥控选择返校报文的遥控类型与预期不符。\n");
		return -1;
	}

	ResetTimerYkSel(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkSelCon);
	CmdConSig_(YK_SEL_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);

	return 0;


}

int CH101::ParseSingleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)terminalPtr->getRecvYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控执行返校报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkExeCon))
	if((terminalPtr->loadYkPointPtr(yk_no))->ExeResponEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkExeCon<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控执行返校报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if (yk_type == SYK_TYPE_OPEN)
	{
		if(terminalPtr->getYkType(yk_no) != DataBase::YkOpen)
		{
			ResetTimerYkExe(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_EXE_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控执行返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else if (yk_type == SYK_TYPE_CLOSE)
	{
		if (terminalPtr->getYkType(yk_no) != DataBase::YkClose)
		{
			ResetTimerYkExe(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_EXE_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控执行返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else
	{
		ResetTimerYkExe(terminalPtr,yk_no,false);
		//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
		(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
		CmdConSig_(YK_EXE_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
		AddStatusLogWithSynT("遥控执行返校报文的遥控类型与预期不符。\n");
		return -1;
	}

	ResetTimerYkExe(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkExeCon);
	CmdConSig_(YK_EXE_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);

	return 0;
}

int CH101::ParseSingleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	if (yk_no < 0 || yk_no >= (int)terminalPtr->getRecvYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 收到遥控取消返校报文，但是遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkCancelCon))
	if((terminalPtr->loadYkPointPtr(yk_no))->CancelResponEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkCancelCon<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控取消返校报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if (yk_type == SYK_TYPE_OPEN)
	{
		if(terminalPtr->getYkType(yk_no) != DataBase::YkOpen)
		{
			ResetTimerYkCancel(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_CANCEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控取消返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else if (yk_type == SYK_TYPE_CLOSE)
	{
		if (terminalPtr->getYkType(yk_no) != DataBase::YkClose)
		{
			ResetTimerYkCancel(terminalPtr,yk_no,false);
			//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
			(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
			CmdConSig_(YK_CANCEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
			AddStatusLogWithSynT("遥控取消返校报文的遥控类型与预期不符。\n");
			return -1;
		}
	}
	else
	{
		ResetTimerYkCancel(terminalPtr,yk_no,false);
		//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
		(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
		CmdConSig_(YK_CANCEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
		AddStatusLogWithSynT("遥控取消返校报文的遥控类型与预期不符。\n");
		return -1;
	}

	ResetTimerYkCancel(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkCancelCon);
	CmdConSig_(YK_CANCEL_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);

	return 0;
}

int CH101::ParseSingleYKOverCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYK_START_ADDR_;
	unsigned char yk_type = buf[DataLocation_] & 0x03;

	ResetTimerYkSel(terminalPtr,yk_no,false);
	ResetTimerYkExe(terminalPtr,yk_no,false);
	ResetTimerYkCancel(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
	(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();

	return 0;
}

int CH101::ParseAllDataCallCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	//terminalPtr->setInitCommPointFlag(true);

	ResetTimerCallAllData(terminalPtr,true);

	return 0;
}

int CH101::ParseAllDataCallOver(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	terminalPtr->setInitCommPointFlag(true);

	ResetTimerCallAllData(terminalPtr,true);

	return 0;
}

int CH101::ParseAllSingleYX(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;
	int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYX_START_ADDR_;

	if (info_addr < 0 || info_addr > (int)terminalPtr->getRecvYxSum())
	{
		AddStatusLogWithSynT( "收到单点全YX报文，但是信息体地址错误，不处理该报文。\n" );
		return -1;
	}

	if (info_num <= 0)
	{
		AddStatusLogWithSynT( "收到单点全YX报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	if(info_num > (int)terminalPtr->getRecvYxSum())
	{
		info_num = terminalPtr->getRecvYxSum();
	}

	int count = 0;

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 1;

		for (int i=0;i<info_num;i++)
		{
			if (info_addr+i >= 0 && info_addr+i < (int)terminalPtr->getRecvYxSum())
			{
				unsigned char yx_quality = buf[DataLocation_ + i*InfoDataLength] & 0xf0;
				unsigned char yx_val = buf[DataLocation_ + i*InfoDataLength] & 0x01;

				terminalPtr->SaveYxQuality(info_addr + i,yx_quality);
				terminalPtr->SaveYxType(info_addr + i,DataBase::single_yx_point);
				int ret = terminalPtr->SaveOriYxVal(info_addr + i,yx_val,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	}
	else if (info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 1;

		for (int i=0;i<info_num;i++)
		{
			info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - SYX_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYxSum())
			{
				unsigned char yx_quality = buf[DataLocation_ + i*InfoDataLength] & 0xf0;
				unsigned char yx_val = buf[DataLocation_ + i*InfoDataLength] & 0x01;

				terminalPtr->SaveYxQuality(info_addr,yx_quality);
				terminalPtr->SaveYxType(info_addr,DataBase::single_yx_point);
				int ret = terminalPtr->SaveOriYxVal(info_addr,yx_val,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
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

int CH101::ParseAllDoubleYX(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;
	int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYX_START_ADDR_;

	if (info_addr < 0 || info_addr > (int)terminalPtr->getRecvYxSum())
	{
		AddStatusLogWithSynT( "收到双点全YX报文，但是信息体地址错误，不处理该报文。\n" );
		return -1;
	}

	if (info_num <= 0)
	{
		AddStatusLogWithSynT( "收到双点全YX报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	if (info_num > (int)terminalPtr->getRecvYxSum())
	{
		info_num = (int)terminalPtr->getRecvYxSum();
	}

	int count = 0;

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 1;

		for (int i=0;i<info_num;i++)
		{
			if (info_addr+i >= 0 && info_addr+i < (int)terminalPtr->getRecvYxSum())
			{
				unsigned char yx_quality = buf[DataLocation_ + i*InfoDataLength] & 0xf0;
				unsigned char yx_val = buf[DataLocation_ + i*InfoDataLength] & 0x03;

				terminalPtr->SaveYxQuality(info_addr + i,yx_quality);
				terminalPtr->SaveYxType(info_addr + i,DataBase::double_yx_point);
				int ret = terminalPtr->SaveOriYxVal(info_addr + i,yx_val,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	}
	else if (info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 1;

		for (int i=0;i<info_num;i++)
		{
			info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - DYX_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYxSum())
			{
				unsigned char yx_quality = buf[DataLocation_ + i*InfoDataLength] & 0xf0;
				unsigned char yx_val = buf[DataLocation_ + i*InfoDataLength] & 0x03;

				terminalPtr->SaveYxQuality(info_addr,yx_quality);
				terminalPtr->SaveYxType(info_addr,DataBase::double_yx_point);
				int ret = terminalPtr->SaveOriYxVal(info_addr,yx_val,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	}

	if (count > 0)
	{
		std::ostringstream ostr;
		ostr<<"双点全YX报文产生了COS，数目："<<count<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CH101::ParseAllYXByte(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	if (info_num == 0 || (info_num - 1)*16 > (int)terminalPtr->getRecvYxSum())
	{
		AddStatusLogWithSynT( "收到全YX字报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	int count = 0;

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 5;
		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYX_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			unsigned char yxQuality = buf[DataLocation_ + 4 + i*InfoDataLength] & 0xf0;

			unsigned char yx_val = buf[DataLocation_ + i*InfoDataLength];
			for (int j=0;j<8;j++)
			{
				int index = info_addr + i*16 + j;
				if (index >= 0 && index < (int)terminalPtr->getRecvYxSum())
				{
					if ((yx_val & BYTE_CHECK_TRUE[j]) > 0)
					{
						terminalPtr->SaveYxType(index,DataBase::single_yx_point);
						int ret = terminalPtr->SaveOriYxVal(index,1);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
					else
					{
						terminalPtr->SaveYxType(index,DataBase::single_yx_point);
						int ret = terminalPtr->SaveOriYxVal(index,0);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
				}

				terminalPtr->SaveYxQuality(index,yxQuality);
			}

			yx_val = buf[DataLocation_ + 1 + i*InfoDataLength];
			for (int j=0;j<8;j++)
			{
				int index = info_addr + i*16 + 8 + j;
				if((index >= 0) && (index < (int)terminalPtr->getRecvYxSum()))
				{
					if ((yx_val & BYTE_CHECK_TRUE[j]) > 0)
					{
						terminalPtr->SaveYxType(index,DataBase::single_yx_point);
						int ret = terminalPtr->SaveOriYxVal(index,1);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
					else
					{
						terminalPtr->SaveYxType(index,DataBase::single_yx_point);
						int ret = terminalPtr->SaveOriYxVal(index,0);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
				}

				terminalPtr->SaveYxQuality(index,yxQuality);
			}
		}
	}
	else if (info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 5;

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - SYX_START_ADDR_;
			unsigned char yxQuality = buf[DataLocation_ + 4 + i*InfoDataLength] & 0xf0;

			unsigned char yx_val = buf[DataLocation_ + i*InfoDataLength];
			for (int j=0;j<8;j++)
			{
				int index = info_addr + j;
				if (index >= 0 && index < (int)terminalPtr->getRecvYxSum())
				{
					if ((yx_val & BYTE_CHECK_TRUE[j]) > 0)
					{
						terminalPtr->SaveYxType(index,DataBase::single_yx_point);
						int ret = terminalPtr->SaveOriYxVal(index,1);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
					else
					{
						terminalPtr->SaveYxType(index,DataBase::single_yx_point);
						int ret = terminalPtr->SaveOriYxVal(index,0);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
				}

				terminalPtr->SaveYxQuality(index,yxQuality);
			}

			yx_val = buf[DataLocation_ + 1 + i*InfoDataLength];
			for (int j=0;j<8;j++)
			{
				int index = info_addr + 8 + j;
				if((index >= 0) && (index < (int)terminalPtr->getRecvYxSum()))
				{
					if ((yx_val & BYTE_CHECK_TRUE[j]) > 0)
					{
						terminalPtr->SaveYxType(index,DataBase::single_yx_point);
						int ret = terminalPtr->SaveOriYxVal(index,1);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
					else
					{
						terminalPtr->SaveYxType(index,DataBase::single_yx_point);
						int ret = terminalPtr->SaveOriYxVal(index,0);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
				}

				terminalPtr->SaveYxQuality(index,yxQuality);
			}
		}
	}

	if (count > 0)
	{
		std::ostringstream ostr;
		ostr<<"全YX字报文产生了COS，数目："<<count<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CH101::ParseSingleCOS(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	if (info_num <= 0)
	{
		AddStatusLogWithSynT("收到单点COS报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	/*
	if (info_ISQ == DisableISQ)
	{
		AddStatusLogWithSynT( "收到单点COS报文，但是信息体ISQ位错误，不处理该报文。\n" );
		return -1;
	}
	*/

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 1;
		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYX_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if ((info_addr + i >= 0) && (info_addr + i < (int)terminalPtr->getRecvYxSum()))
			{
				unsigned char YxQuality = buf[DataLocation_ + i*InfoDataLength] & 0xf0;
				unsigned char YxBitVal = buf[DataLocation_ + i*InfoDataLength] & 0x01;

				terminalPtr->SaveYxQuality(info_addr+i,YxQuality);
				terminalPtr->SaveYxType(info_addr+i,DataBase::single_yx_point);
				terminalPtr->SaveCosPoint(info_addr+i,YxBitVal,DataBase::single_yx_point,YxQuality);

				std::ostringstream ostr;
				ostr<<"收到单点COS，点号"<<info_addr+i<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			} 
			else
			{
				AddStatusLogWithSynT("收到单点COS，但是信息体地址错误，不处理该点。\n");
			}
		}
	} 
	else if(info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 1;

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - SYX_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYxSum())
			{
				unsigned char YxQuality = buf[DataLocation_ + i*InfoDataLength] & 0xf0;
				unsigned char YxBitVal = buf[DataLocation_ + i*InfoDataLength] & 0x01;

				terminalPtr->SaveYxQuality(info_addr,YxQuality);
				terminalPtr->SaveYxType(info_addr,DataBase::single_yx_point);
				terminalPtr->SaveCosPoint(info_addr,YxBitVal,DataBase::single_yx_point,YxQuality);

				std::ostringstream ostr;
				ostr<<"收到单点COS，点号"<<info_addr<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			} 
			else
			{
				AddStatusLogWithSynT("收到单点COS，但是信息体地址错误，不处理该点。\n");
			}
		}
	}
	
	if (info_num > 0)
	{
		CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
	}

	return 0;
}

int CH101::ParseDoubleCOS(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	if (info_num <= 0)
	{
		AddStatusLogWithSynT("收到双点COS报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	/*
	if (info_ISQ == DisableISQ)
	{
		AddStatusLogWithSynT( "收到双点COS报文，但是信息体ISQ位错误，不处理该报文。\n" );
		return -1;
	}
	*/

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 1;
		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYX_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if ((info_addr + i >= 0) && (info_addr + i < (int)terminalPtr->getRecvYxSum()))
			{
				unsigned char YxQuality = buf[DataLocation_ + i*InfoDataLength] & 0xf0;
				unsigned char YxBitVal = buf[DataLocation_ + i*InfoDataLength] & 0x03;

				terminalPtr->SaveYxQuality(info_addr+i,YxQuality);
				terminalPtr->SaveYxType(info_addr+i,DataBase::double_yx_point);
				terminalPtr->SaveCosPoint(info_addr+i,YxBitVal,DataBase::double_yx_point,YxQuality);

				std::ostringstream ostr;
				ostr<<"收到双点COS，点号"<<info_addr+i<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			} 
			else
			{
				AddStatusLogWithSynT("收到双点COS，但是信息体地址错误，不处理该点\n");
			}
		}
	}
	else if (info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 1;

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - DYX_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYxSum())
			{
				unsigned char YxQuality = buf[DataLocation_ + i*InfoDataLength] & 0xf0;
				unsigned char YxBitVal = buf[DataLocation_ + i*InfoDataLength] & 0x03;

				terminalPtr->SaveYxQuality(info_addr,YxQuality);
				terminalPtr->SaveYxType(info_addr,DataBase::double_yx_point);
				terminalPtr->SaveCosPoint(info_addr,YxBitVal,DataBase::double_yx_point,YxQuality);

				std::ostringstream ostr;
				ostr<<"收到双点COS，点号"<<info_addr<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			} 
			else
			{
				AddStatusLogWithSynT("收到双点COS，但是信息体地址错误，不处理该点\n");
			}
		}
	}

	if (info_num > 0)
	{
		CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
	}

	return 0;
}

int CH101::ParseAllYCData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;
	

	/*
	if (info_addr < 0 || info_addr > (int)terminalPtr->getRecvYcSum())
	{
		AddStatusLogWithSynT( "收到全YC报文，但是信息体地址错误，不处理该报文。\n" );
		return -1;
	}
	*/

	if (info_num <= 0)
	{
		AddStatusLogWithSynT( "收到全YC报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	if (info_num > (int)terminalPtr->getRecvYcSum())
	{
		info_num = (int)terminalPtr->getRecvYcSum();
	}

	/*
	if (info_ISQ == EnableISQ)
	{
		AddStatusLogWithSynT( "收到全YC报文，但是信息体ISQ位错误，不处理该报文。\n" );
		return -1;
	}
	*/

	int count = 0;

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 2;
		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - YC_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if ((info_addr + i >= 0) && (info_addr + i < (int)terminalPtr->getRecvYcSum()))
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				terminalPtr->SaveYcQuality(info_addr + i,DataBase::CYcPoint::QualityActive);
				int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	} 
	else if(info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 2;

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - YC_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				terminalPtr->SaveYcQuality(info_addr,DataBase::CYcPoint::QualityActive);
				int ret = terminalPtr->SaveOriYcVal(info_addr,YcVal,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	}
	

	if (count > 0)
	{
		std::ostringstream ostr;
		ostr<<"全YC报文产生了ycvar，数目："<<count<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CH101::ParseAllYCDataWithValid(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;
	
	/*
	if (info_addr < 0 || info_addr > (int)terminalPtr->getRecvYcSum())
	{
		AddStatusLogWithSynT( "收到带校验位的全YC报文，但是信息体地址错误，不处理该报文。\n" );
		return -1;
	}
	*/

	if (info_num <= 0)
	{
		AddStatusLogWithSynT( "收到带校验位的全YC报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	if (info_num > (int)terminalPtr->getRecvYcSum())
	{
		info_num = (int)terminalPtr->getRecvYcSum();
	}

	/*
	if (info_ISQ == EnableISQ)
	{
		AddStatusLogWithSynT( "收到带校验位的全YC报文，但是信息体ISQ位错误，不处理该报文。\n" );
		return -1;
	}
	*/

	int count = 0;

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 3;
		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - YC_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if (info_addr+i >= 0 && info_addr+i < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				unsigned char yc_valid = buf[DataLocation_ + 2 + i*InfoDataLength];

				terminalPtr->SaveYcQuality(info_addr + i,yc_valid);
				int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	} 
	else if(info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 3;

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - YC_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				unsigned char yc_valid = buf[DataLocation_ + 2 + i*InfoDataLength];

				terminalPtr->SaveYcQuality(info_addr,yc_valid);
				int ret = terminalPtr->SaveOriYcVal(info_addr,YcVal,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	}
	
	if (count > 0)
	{
		std::ostringstream ostr;
		ostr<<"带校验位的全YC报文产生了ycvar，数目："<<count<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CH101::ParseAllYCDataWithValidTE(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	/*
	if (info_addr < 0 || info_addr > (int)terminalPtr->getRecvYcSum())
	{
	AddStatusLogWithSynT( "收到带校验位的全YC报文，但是信息体地址错误，不处理该报文。\n" );
	return -1;
	}
	*/

	if (info_num <= 0)
	{
		AddStatusLogWithSynT( "收到带校验位的全YC报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	if (info_num > (int)terminalPtr->getRecvYcSum())
	{
		info_num = (int)terminalPtr->getRecvYcSum();
	}

	/*
	if (info_ISQ == EnableISQ)
	{
	AddStatusLogWithSynT( "收到带校验位的全YC报文，但是信息体ISQ位错误，不处理该报文。\n" );
	return -1;
	}
	*/

	int count = 0;

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 10;
		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - YC_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if (info_addr+i >= 0 && info_addr+i < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				unsigned char yc_valid = buf[DataLocation_ + 2 + i*InfoDataLength];

				terminalPtr->SaveYcQuality(info_addr + i,yc_valid);
				int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	} 
	else if(info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 10;

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - YC_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				unsigned char yc_valid = buf[DataLocation_ + 2 + i*InfoDataLength];

				terminalPtr->SaveYcQuality(info_addr,yc_valid);
				int ret = terminalPtr->SaveOriYcVal(info_addr,YcVal,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	}

	if (count > 0)
	{
		std::ostringstream ostr;
		ostr<<"带时标和校验位的全YC报文产生了ycvar，数目："<<count<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CH101::ParseYCCH(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	if (info_num <= 0 || info_num > (int)terminalPtr->getRecvYxSum())
	{
		AddStatusLogWithSynT("收到YC变化报文，但是信息体地址错误，不处理该报文。\n" );
		return -1;
	}

	/*
	if (info_ISQ == DisableISQ)
	{
		AddStatusLogWithSynT( "收到YC变化报文，但是信息体ISQ位错误，不处理该报文。\n" );
		return -1;
	}
	*/

	int count = 0;

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 2;
		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - YC_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if ((info_addr + i >= 0) && (info_addr + i < (int)terminalPtr->getRecvYcSum()))
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				terminalPtr->SaveYcQuality(info_addr + i,DataBase::CYcPoint::QualityActive);
				int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	} 
	else if(info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 2;

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - YC_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				terminalPtr->SaveYcQuality(info_addr,DataBase::CYcPoint::QualityActive);
				int ret = terminalPtr->SaveOriYcVal(info_addr,YcVal,true);
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	}
	
	if (count > 0)
	{
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CH101::ParseYCCHWithValid(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	if (info_num <= 0 || info_num > (int)terminalPtr->getRecvYxSum())
	{
		AddStatusLogWithSynT("收到带校验的YC变化报文，但是信息体地址错误，不处理该报文。\n" );
		return -1;
	}

	/*
	if (info_ISQ == DisableISQ)
	{
		AddStatusLogWithSynT( "收到带校验YC变化报文，但是信息体ISQ位错误，不处理该报文。\n" );
		return -1;
	}
	*/

	int count = 0;

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength = 3;
		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - YC_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if (info_addr+i >= 0 && info_addr+i < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				unsigned char yc_valid = buf[DataLocation_ + 2 + i*InfoDataLength];

				terminalPtr->SaveYcQuality(info_addr + i,yc_valid);
				int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	} 
	else if(info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 3;

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - YC_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				unsigned char yc_valid = buf[DataLocation_ + 2 + i*InfoDataLength];

				terminalPtr->SaveYcQuality(info_addr,yc_valid);
				int ret = terminalPtr->SaveOriYcVal(info_addr,YcVal,true);
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	}
	
	if (count > 0)
	{
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CH101::ParseYCCHWithValidTE(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	if (info_num <= 0 || info_num > (int)terminalPtr->getRecvYxSum())
	{
		AddStatusLogWithSynT("收到带校验的YC变化报文，但是信息体地址错误，不处理该报文。\n" );
		return -1;
	}

	/*
	if (info_ISQ == DisableISQ)
	{
	AddStatusLogWithSynT( "收到带校验YC变化报文，但是信息体ISQ位错误，不处理该报文。\n" );
	return -1;
	}
	*/

	int count = 0;

	if (info_ISQ == DisableISQ)
	{
		unsigned char frame_length = buf[1] + 4;
		int InfoDataLength = (frame_length - DataLocation_) / info_num;
		if ( InfoDataLength >= 10)
		{
			InfoDataLength = 10;
		}
		else if (InfoDataLength < 10 && InfoDataLength >= 6)
		{
			InfoDataLength = 6;
		}

		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - YC_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if (info_addr + i>= 0 && info_addr + i < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				unsigned char yc_valid = buf[DataLocation_ + 2 + i*InfoDataLength];
				
				terminalPtr->SaveYcQuality(info_addr + i,yc_valid);
				int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,terminalPtr->getInitCommPointFlag());
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	} 
	else if(info_ISQ == EnableISQ)
	{
		unsigned char frame_length = buf[1] + 4;
		unsigned char InfoDataLength = (frame_length - InfoAddrLocation_) / info_num;
		if ( InfoDataLength >= InfoAddrLength_ + 10)
		{
			InfoDataLength = InfoAddrLength_ + 10;
		}
		else if (InfoDataLength < InfoAddrLength_ + 10 && InfoDataLength >= InfoAddrLength_ + 6)
		{
			InfoDataLength = InfoAddrLength_ + 6;
		}

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - YC_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
			{
				unsigned short YcVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],2);
				unsigned char yc_valid = buf[DataLocation_ + 2 + i*InfoDataLength];

				terminalPtr->SaveYcQuality(info_addr,yc_valid);
				int ret = terminalPtr->SaveOriYcVal(info_addr,YcVal,true);
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
		}
	}

	if (count > 0)
	{
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
	}

	return 0;
}

int CH101::ParseSynTimeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	terminalPtr->setSynTCommPointFlag(true);

	ResetTimerSynTime(terminalPtr,true);

	return 0;
}

int CH101::ParseTransDelayCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	boost::posix_time::ptime lt_now = boost::posix_time::microsec_clock::local_time();
	unsigned short lt_millisecond = (lt_now.time_of_day().total_milliseconds()) % MinutesRemainderMillisecs;

	unsigned short frame_millisecond = BufToVal(&buf[DataLocation_],2);

	short diff_millisecond = lt_millisecond - frame_millisecond;

	if (diff_millisecond > 0)
	{
		AddSendCmdVal(TRANS_DELAY_DOWNLOAD,TRANS_DELAY_DOWNLOAD_PRIORITY,terminalPtr,diff_millisecond);
	}
	else
	{
		AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,terminalPtr);
	}

	terminalPtr->setDelayCommPointFlag(true);

	return 0;
}

int CH101::ParseSingleSOE(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	if (info_num <= 0)
	{
		AddStatusLogWithSynT("收到单点SOE报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	/*
	if (info_ISQ == DisableISQ)
	{
		AddStatusLogWithSynT( "收到单点SOE报文，但是信息体ISQ位错误，不处理该报文。\n" );
		return -1;
	}
	*/

	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();

	if (info_ISQ == DisableISQ)
	{
		unsigned char frame_length = buf[1] + 4;
		unsigned char InfoDataLength = (frame_length - DataLocation_) / info_num;
		if ( InfoDataLength >= 8)
		{
			InfoDataLength = 8;
		}
		else if (InfoDataLength < 8 && InfoDataLength >= 4)
		{
			InfoDataLength = 4;
		}

		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYX_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if (info_addr + i>= 0 && info_addr + i< (int)terminalPtr->getRecvYxSum())
			{
				unsigned char YxQuality = buf[DataLocation_ + i*InfoDataLength] & 0xf0; 
				unsigned char YxBitVal = buf[DataLocation_ + i*InfoDataLength] & 0x01;
				unsigned short millisecond = BufToVal(&buf[DataLocation_ + 1 + i*InfoDataLength],2);
				unsigned char minute = buf[DataLocation_ + 3 + i*InfoDataLength] & 0x3f;
				time_duration td(minutes(minute) + seconds(millisecond / 1000) + milliseconds(millisecond % 1000));

				std::ostringstream ostr;

				if (InfoDataLength >= 8)
				{
					unsigned char Hour = buf[DataLocation_ + 4 + i*InfoDataLength] & 0x1f;
					unsigned char Day = buf[DataLocation_ + 5 + i*InfoDataLength] & 0x1f;
					unsigned char Month = buf[DataLocation_ + 6 + i*InfoDataLength] & 0x0f;
					unsigned short Year = (buf[DataLocation_ + 7 + i*InfoDataLength] & 0x7f) + (((lt.date()).year() / 100) * 100);
					td += hours(Hour);
					boost::gregorian::date dt(Year,Month,Day);
					ptime timeVal(dt,td);

					terminalPtr->SaveYxQuality(info_addr+i,YxQuality);
					terminalPtr->SaveYxType(info_addr+i,DataBase::single_yx_point);
					terminalPtr->SaveSoePoint(info_addr+i,YxBitVal,DataBase::single_yx_point,timeVal,YxQuality);

					ostr<<"收到单点SOE，点号"<<info_addr+i<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				}
				else
				{
					td += hours((lt.time_of_day()).hours());
					ptime timeVal(lt.date(),td);

					terminalPtr->SaveYxQuality(info_addr+i,YxQuality);
					terminalPtr->SaveYxType(info_addr+i,DataBase::single_yx_point);
					terminalPtr->SaveSoePoint(info_addr+i,YxBitVal,DataBase::single_yx_point,timeVal,YxQuality);

					ostr<<"收到单点SOE，点号"<<info_addr+i<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				}

				AddStatusLogWithSynT(ostr.str());
			}
		}
	} 
	else if(info_ISQ == EnableISQ)
	{
		unsigned char frame_length = buf[1] + 4;
		unsigned char InfoDataLength = (frame_length - InfoAddrLocation_) / info_num;
		if ( InfoDataLength >= InfoAddrLength_ + 8)
		{
			InfoDataLength = InfoAddrLength_ + 8;
		}
		else if (InfoDataLength < InfoAddrLength_ + 8 && InfoDataLength >= InfoAddrLength_ + 4)
		{
			InfoDataLength = InfoAddrLength_ + 4;
		}

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - SYX_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYxSum())
			{
				unsigned char YxQuality = buf[DataLocation_ + i*InfoDataLength] & 0xf0; 
				unsigned char YxBitVal = buf[DataLocation_ + i*InfoDataLength] & 0x01;
				unsigned short millisecond = BufToVal(&buf[DataLocation_ + 1 + i*InfoDataLength],2);
				unsigned char minute = buf[DataLocation_ + 3 + i*InfoDataLength] & 0x3f;
				time_duration td(minutes(minute) + seconds(millisecond / 1000) + milliseconds(millisecond % 1000));

				std::ostringstream ostr;

				if (InfoDataLength >= InfoAddrLength_ + 8)
				{
					unsigned char Hour = buf[DataLocation_ + 4 + i*InfoDataLength] & 0x1f;
					unsigned char Day = buf[DataLocation_ + 5 + i*InfoDataLength] & 0x1f;
					unsigned char Month = buf[DataLocation_ + 6 + i*InfoDataLength] & 0x0f;
					unsigned short Year = (buf[DataLocation_ + 7 + i*InfoDataLength] & 0x7f) + (((lt.date()).year() / 100) * 100);
					td += hours(Hour);
					boost::gregorian::date dt(Year,Month,Day);
					ptime timeVal(dt,td);

					terminalPtr->SaveYxQuality(info_addr,YxQuality);
					terminalPtr->SaveYxType(info_addr,DataBase::single_yx_point);
					terminalPtr->SaveSoePoint(info_addr,YxBitVal,DataBase::single_yx_point,timeVal,YxQuality);

					ostr<<"收到单点SOE，点号"<<info_addr<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				}
				else
				{
					td += hours((lt.time_of_day()).hours());
					ptime timeVal(lt.date(),td);

					terminalPtr->SaveYxQuality(info_addr,YxQuality);
					terminalPtr->SaveYxType(info_addr,DataBase::single_yx_point);
					terminalPtr->SaveSoePoint(info_addr,YxBitVal,DataBase::single_yx_point,timeVal,YxQuality);

					ostr<<"收到单点SOE，点号"<<info_addr<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				}

				AddStatusLogWithSynT(ostr.str());
			} 
			else
			{
				AddStatusLogWithSynT("收到单点SOE，但是信息体地址错误，不处理该点。\n");
			}
		}
	}

	if (info_num > 0)
	{
		CmdConSig_(SOE_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
	}

	return 0;
}

int CH101::ParseDoubleSOE(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	if (info_num <= 0)
	{
		AddStatusLogWithSynT("收到双点SOE报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	/*
	if (info_ISQ == DisableISQ)
	{
		AddStatusLogWithSynT( "收到双点SOE报文，但是信息体ISQ位错误，不处理该报文。\n" );
		return -1;
	}
	*/

	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();

	if (info_ISQ == DisableISQ)
	{
		unsigned char frame_length = buf[1] + 4;
		unsigned char InfoDataLength = (frame_length - DataLocation_) / info_num;
		if ( InfoDataLength >= 8)
		{
			InfoDataLength = 8;
		}
		else if (InfoDataLength < 8 && InfoDataLength >= 4)
		{
			InfoDataLength = 4;
		}

		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYX_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if (info_addr + i>= 0 && info_addr + i< (int)terminalPtr->getRecvYxSum())
			{
				unsigned char YxQuality = buf[DataLocation_ + i*InfoDataLength] & 0xf0; 
				unsigned char YxBitVal = buf[DataLocation_ + i*InfoDataLength] & 0x03;
				unsigned short millisecond = BufToVal(&buf[DataLocation_ + 1 + i*InfoDataLength],2);
				unsigned char minute = buf[DataLocation_ + 3 + i*InfoDataLength] & 0x3f;
				time_duration td(minutes(minute) + seconds(millisecond / 1000) + milliseconds(millisecond % 1000));

				std::ostringstream ostr;

				if (InfoDataLength >= InfoAddrLength_ + 8)
				{
					unsigned char Hour = buf[DataLocation_ + 4 + i*InfoDataLength] & 0x1f;
					unsigned char Day = buf[DataLocation_ + 5 + i*InfoDataLength] & 0x1f;
					unsigned char Month = buf[DataLocation_ + 6 + i*InfoDataLength] & 0x0f;
					unsigned short Year = (buf[DataLocation_ + 7 + i*InfoDataLength] & 0x7f) + (((lt.date()).year() / 100) * 100);
					td += hours(Hour);
					boost::gregorian::date dt(Year,Month,Day);
					ptime timeVal(dt,td);

					terminalPtr->SaveYxQuality(info_addr+i,YxQuality);
					terminalPtr->SaveYxType(info_addr+i,DataBase::double_yx_point);
					terminalPtr->SaveSoePoint(info_addr+i,YxBitVal,DataBase::double_yx_point,timeVal,YxQuality);

					ostr<<"收到双点SOE，点号"<<info_addr+i<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				}
				else
				{
					td += hours((lt.time_of_day()).hours());
					ptime timeVal(lt.date(),td);

					terminalPtr->SaveYxQuality(info_addr+i,YxQuality);
					terminalPtr->SaveYxType(info_addr+i,DataBase::double_yx_point);
					terminalPtr->SaveSoePoint(info_addr+i,YxBitVal,DataBase::double_yx_point,timeVal,YxQuality);

					ostr<<"收到双点SOE，点号"<<info_addr+i<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				}

				AddStatusLogWithSynT(ostr.str());
			}
		}
	} 
	else if(info_ISQ == EnableISQ)
	{
		unsigned char frame_length = buf[1] + 4;
		unsigned char InfoDataLength = (frame_length - InfoAddrLocation_) / info_num;
		if ( InfoDataLength >= InfoAddrLength_ + 8)
		{
			InfoDataLength = InfoAddrLength_ + 8;
		}
		else if (InfoDataLength < InfoAddrLength_ + 8 && InfoDataLength >= InfoAddrLength_ + 4)
		{
			InfoDataLength = InfoAddrLength_ + 4;
		}

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - DYX_START_ADDR_;
			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYxSum())
			{
				unsigned char YxQuality = buf[DataLocation_ + i*InfoDataLength] & 0xf0; 
				unsigned char YxBitVal = buf[DataLocation_ + i*InfoDataLength] & 0x03;
				unsigned short millisecond = BufToVal(&buf[DataLocation_ + 1 + i*InfoDataLength],2);
				unsigned char minute = buf[DataLocation_ + 3 + i*InfoDataLength] & 0x3f;
				time_duration td(minutes(minute) + seconds(millisecond / 1000) + milliseconds(millisecond % 1000));

				std::ostringstream ostr;

				if (InfoDataLength >= InfoAddrLength_ + 8)
				{
					unsigned char Hour = buf[DataLocation_ + 4 + i*InfoDataLength] & 0x1f;
					unsigned char Day = buf[DataLocation_ + 5 + i*InfoDataLength] & 0x1f;
					unsigned char Month = buf[DataLocation_ + 6 + i*InfoDataLength] & 0x0f;
					unsigned short Year = (buf[DataLocation_ + 7 + i*InfoDataLength] & 0x7f) + (((lt.date()).year() / 100) * 100);
					td += hours(Hour);
					boost::gregorian::date dt(Year,Month,Day);
					ptime timeVal(dt,td);

					terminalPtr->SaveYxQuality(info_addr,YxQuality);
					terminalPtr->SaveYxType(info_addr,DataBase::double_yx_point);
					terminalPtr->SaveSoePoint(info_addr,YxBitVal,DataBase::double_yx_point,timeVal,YxQuality);

					ostr<<"收到双点SOE，点号"<<info_addr<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				}
				else
				{
					td += hours((lt.time_of_day()).hours());
					ptime timeVal(lt.date(),td);

					terminalPtr->SaveYxQuality(info_addr,YxQuality);
					terminalPtr->SaveYxType(info_addr,DataBase::double_yx_point);
					terminalPtr->SaveSoePoint(info_addr,YxBitVal,DataBase::double_yx_point,timeVal,YxQuality);

					ostr<<"收到双点SOE，点号"<<info_addr<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				}

				AddStatusLogWithSynT(ostr.str());
			} 
			else
			{
				AddStatusLogWithSynT("收到双点SOE，但是信息体地址错误，不处理该点。\n");
			}
		}
	}
	
	if (info_num > 0)
	{
		CmdConSig_(SOE_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
	}

	return 0;
}

int CH101::ParseAllYMCallCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseAllYMCallOver(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseAllYMData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;

	if (info_num <= 0)
	{
		AddStatusLogWithSynT( "收到全YM报文，但是信息体数目错误，不处理该报文。\n" );
		return -1;
	}

	if (info_num > (int)terminalPtr->getRecvYmSum())
	{
		info_num = (int)terminalPtr->getRecvYmSum();
	}

	if (info_ISQ == DisableISQ)
	{
		int InfoDataLength  = 5;
		int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - YM_START_ADDR_;

		for (int i=0;i<info_num;i++)
		{
			if (info_addr + i>= 0 && info_addr + i< (int)terminalPtr->getRecvYmSum())
			{
				unsigned int YmVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],4);
				unsigned char YmQuality = BufToVal(&buf[DataLocation_ + 4 + i*InfoDataLength],1);

				terminalPtr->SaveYmQuality(info_addr + i,YmQuality);
				terminalPtr->SaveOriYmVal(info_addr + i,YmVal);
			}
		}
	}
	else if (info_ISQ == EnableISQ)
	{
		int InfoDataLength = InfoAddrLength_ + 5;

		for (int i=0;i<info_num;i++)
		{
			int info_addr = BufToVal(&buf[InfoAddrLocation_ + i*InfoDataLength],InfoAddrLength_) - YM_START_ADDR_;

			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYmSum())
			{
				unsigned int YmVal = BufToVal(&buf[DataLocation_ + i*InfoDataLength],4);
				unsigned char YmQuality = BufToVal(&buf[DataLocation_ + 4 + i*InfoDataLength],1);

				terminalPtr->SaveYmQuality(info_addr + i,YmQuality);
				terminalPtr->SaveOriYmVal(info_addr + i,YmVal);
			}
		}
	}

	return 0;
}

int CH101::ParseEndInit(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseSetParaCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseSetParaErr(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseAllParaData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseExtendRTUInfo(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseHistoryData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseNoHistoryData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseEndHistoryData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseFaultRecordData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseFaultRecordEnd(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseYkRecordData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseYkRecordEnd(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseYxRecordData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseYxRecordEnd(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	int ret = 0;

	unsigned char funcType = buf[0];

	if (exceptedBytes == 1 && funcType == 0xe5)
	{
		setLastRecvPointIndex(getLastSendPointIndex());
		return getLastSendPointIndex();
	}

	int Addr = getAddrByRecvFrame(buf);
	if (Addr < 0)
	{
		return Addr;
	}

	int terminalIndex = getCommPointIndexByAddrCommType(TERMINAL_NODE,Addr);
	share_terminal_ptr terminalPtr;
	if (terminalIndex >= 0)
	{
		setLastRecvPointIndex(terminalIndex);
		terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(getCommPoint(terminalIndex).lock());
	}

	if (!terminalPtr)
	{
		std::ostringstream ostr;
		ostr<<"H101规约不能根据接收报文中的地址匹配terminal ptr,这帧报文将不会被解析。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (funcType)
	{
	case 0x10:
		ret = ParseShortFrame(buf,terminalPtr);
		break;

	case 0x68:
		ret = ParseLongFrame(buf,terminalPtr);
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

	return terminalIndex;
}

int CH101::getFCB(share_terminal_ptr terminalPtr)
{
	if(terminalPtr->getCurFcbFlag())
	{
		return ACT_FCB;
	}
	else
	{
		return NACK_FCB;
	}
}

int CH101::getAddrByRecvFrame(unsigned char * buf)
{
	unsigned char funcType = buf[0];
	unsigned char dirPrmByte = 0;
	int addr = -1;
	switch (funcType)
	{
	case 0x10:
		dirPrmByte = buf[1];
		addr = BufToVal(&buf[2],AsduAddrLength_);
		break;

	case 0x68:
		dirPrmByte = buf[4];
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

	if ((dirPrmByte & DIR_PRM) == DIR_PRM) //检查报文方向位标志
	{
		return -1;
	}

	return addr;
}

int CH101::AssemblePrimaryData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_PL1_NA_3 | getFCB(terminalPtr) | DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH101::AssembleSecondaryData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_PL2_NA_3 | getFCB(terminalPtr) | DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	
	return count - bufIndex;
}

int CH101::AssembleRequireLink(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_PLK_NA_3 |  DIR_PRM | NACK_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	
	return count - bufIndex;
}

int CH101::AssembleResetLink(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_RCU_NA_3 | DIR_PRM | NACK_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	
	return count - bufIndex;
}

int CH101::AssembleCallAllData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_IC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI;

	return count - bufIndex;
}

int CH101::AssembleCallAllDD(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_CI_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI;

	return count - bufIndex;
}

int CH101::AssembleSynTime(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr, boost::posix_time::ptime time)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_CS_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
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

int CH101::AssembleTransDelay(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,boost::posix_time::ptime time)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_CD_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	boost::posix_time::time_duration td = time.time_of_day();
	count += ValToBuf(&buf[count],td.total_milliseconds() % MinutesRemainderMillisecs,2);
	
	return count - bufIndex;
}

int CH101::AssembleTransDownLoad(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,short time_diff)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_CD_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	count += ValToBuf(&buf[count],time_diff,2);

	return count - bufIndex;
}

int CH101::AssembleDoubleYKSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH101::AssembleDoubleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CH101::AssembleDoubleYKCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no,unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH101::AssembleSingleYKSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH101::AssembleSingleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CH101::AssembleSingleYKCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no,unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH101::AssembleCallPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int secondaryIndex)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CH101::AssembleSetPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int secondaryIndex)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CH101::AssembleCallExtendRTUInfo(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0x83,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI;

	return count - bufIndex;
}

int CH101::AssembleCallHistoryData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,boost::posix_time::ptime time)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0x91,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0 + HIS_START_ADDR_,InfoAddrLength_);
	boost::posix_time::time_duration td = time.time_of_day();
	buf[count++] = td.minutes() & 0x3f;
	buf[count++] = td.hours() & 0x1f;
	boost::gregorian::date::ymd_type ymd = time.date().year_month_day();
	buf[count++] = ymd.day & 0x1f;
	buf[count++] = ymd.month & 0x0f;

	return count - bufIndex;
}

int CH101::AssembleCallHistoryDatas(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,boost::posix_time::ptime startTime,boost::posix_time::ptime endTime)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0x91,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x02,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_file,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0 + HIS_START_ADDR_,InfoAddrLength_);
	boost::posix_time::time_duration td = startTime.time_of_day();
	buf[count++] = td.minutes() & 0x3f;
	buf[count++] = td.hours() & 0x1f;
	boost::gregorian::date::ymd_type ymd = startTime.date().year_month_day();
	buf[count++] = ymd.day & 0x1f;
	buf[count++] = ymd.month & 0x0f;
	td = endTime.time_of_day();
	buf[count++] = td.minutes() & 0x3f;
	buf[count++] = td.hours() & 0x1f;
	ymd = endTime.date().year_month_day();
	buf[count++] = ymd.day & 0x1f;
	buf[count++] = ymd.month & 0x0f;

	return count - bufIndex;
}

int CH101::AssembleCallFaultRecordData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0x96,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH101::AssembleCallYkRecordData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0x95,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH101::AssembleCallYxRecordData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PRM | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0x94,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH101::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	switch (cmd.getCmdType())
	{
	case REQUIRE_LINK:
		buf[count++] = 0x10;
		break;

	case RESET_LINK:
		buf[count++] = 0x10;
		break;

	case CALL_PRIMARY_DATA:
		buf[count++] = 0x10;
		break;

	case CALL_SECONDARY_DATA:
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

int CH101::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	int bytesAssemble = 0;

	share_terminal_ptr terminalPtr;

	if (cmd.getCommPoint())
	{
		if (cmd.getCommPoint()->getCommPointType() == TERMINAL_NODE)
		{
			terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(cmd.getCommPoint());
		}
	}

	if (!terminalPtr)
	{
		std::ostringstream ostr;
		ostr<<"H101规约不能从发送命令中获得terminal ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (cmd.getCmdType())
	{
	case CALL_PRIMARY_DATA:
		bytesAssemble = AssemblePrimaryData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_SECONDARY_DATA:
		bytesAssemble = AssembleSecondaryData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerHeartFrame(terminalPtr,true);
		}
		break;

	case REQUIRE_LINK:
		bytesAssemble = AssembleRequireLink(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerRequireLink(terminalPtr,true);
		}
		break;

	case RESET_LINK:
		bytesAssemble = AssembleResetLink(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_ALL_DATA_ACT:
		bytesAssemble = AssembleCallAllData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerCallAllData(terminalPtr,true);
		}
		break;

	case CALL_ALL_DD_ACT:
		bytesAssemble = AssembleCallAllDD(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerCallAllDD(terminalPtr,true);
		}
		break;

	case SYN_TIME_ACT:
		bytesAssemble = AssembleSynTime(bufIndex,buf,terminalPtr,boost::posix_time::microsec_clock::local_time());
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerSynTime(terminalPtr,true);
		}
		break;

	case TRANS_DELAY_ACT:
		bytesAssemble = AssembleTransDelay(bufIndex,buf,terminalPtr,boost::posix_time::microsec_clock::local_time());
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerDelay(terminalPtr,true);
		}
		break;

	case TRANS_DELAY_DOWNLOAD:
		{
			short time_diff = 0;
			try
			{
				time_diff = boost::any_cast<short>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				time_diff = 0;

				std::ostringstream ostr;
				ostr<<"装载延时命令参数转换失败："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			bytesAssemble = AssembleTransDownLoad(bufIndex,buf,terminalPtr,time_diff);
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case YK_SEL_ACT:
		{
			int yk_no;
			try
			{
				yk_no = boost::any_cast<int>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控选择命令的遥控点号参数非法："<<e.what()<<std::endl;
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
				ostr<<"遥控选择命令的遥控点号参数错误："<<err.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			size_t trans_reason = trans_act;
			if (terminalPtr->getEncryptOutside())
			{
				trans_reason |= (0x80 << (8 * (TransReasonLength_ - 1)));
			}

			if (yk_type == DataBase::YkClose)
			{
				if (terminalPtr->getbHYkDouble(yk_no))
				{
					bytesAssemble = AssembleDoubleYKSel(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_CLOSE,trans_reason);
				}
				else
				{
					bytesAssemble = AssembleSingleYKSel(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_CLOSE,trans_reason);
				}

			}
			else if (yk_type == DataBase::YkOpen)
			{
				if (terminalPtr->getbHYkDouble(yk_no))
				{
					bytesAssemble = AssembleDoubleYKSel(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_OPEN,trans_reason);
				}
				else
				{
					bytesAssemble = AssembleSingleYKSel(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_OPEN,trans_reason);
				}

			}
			else
			{
				AddStatusLogWithSynT("遥控选择命令的遥控类型参数非法。\n");
				return -1;
			}

			if (bytesAssemble > 0)
			{
				//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkSelSend))
				if((terminalPtr->loadYkPointPtr(yk_no))->SendSelEvent())
				{
					//std::ostringstream ostr;
					//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkSelSend<<std::endl;
					//AddStatusLogWithSynT(ostr.str());
					AddStatusLogWithSynT("解析遥控选择命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
					return -1;
				}
				//terminalPtr->setYkStatus(yk_no,DataBase::YkSelSend);

				setWaitingForAnswer(cmd.getCommPoint());
				ResetTimerYkSel(terminalPtr,yk_no,true);
			}
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

			size_t trans_reason = trans_act;
			if (terminalPtr->getEncryptOutside())
			{
				trans_reason |= (0x80 << (8 * (TransReasonLength_ - 1)));
			}

			if (yk_type == DataBase::YkClose)
			{
				if (terminalPtr->getbHYkDouble(yk_no))
				{
					bytesAssemble = AssembleDoubleYKExe(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_CLOSE,trans_reason);
				}
				else
				{
					bytesAssemble = AssembleSingleYKExe(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_CLOSE,trans_reason);
				}

			}
			else if (yk_type == DataBase::YkOpen)
			{
				if (terminalPtr->getbHYkDouble(yk_no))
				{
					bytesAssemble = AssembleDoubleYKExe(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_OPEN,trans_reason);
				}
				else
				{
					bytesAssemble = AssembleSingleYKExe(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_OPEN,trans_reason);
				}

			}
			else
			{
				AddStatusLogWithSynT("遥控执行命令的遥控类型参数非法。\n");
				return -1;
			}

			if (bytesAssemble > 0)
			{
				//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkExeSend))
				if((terminalPtr->loadYkPointPtr(yk_no))->SendExeEvent())
				{
					//std::ostringstream ostr;
					//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkExeSend<<std::endl;
					//AddStatusLogWithSynT(ostr.str());
					AddStatusLogWithSynT("解析遥控执行命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
					return -1;
				}
				//terminalPtr->setYkStatus(yk_no,DataBase::YkExeSend);

				setWaitingForAnswer(cmd.getCommPoint());
				ResetTimerYkExe(terminalPtr,yk_no,true);
			}
		}
		break;

	case YK_CANCEL_ACT:
		{
			int yk_no;
			try
			{
				yk_no = boost::any_cast<int>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控取消命令的遥控点号参数非法："<<e.what()<<std::endl;
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
				ostr<<"遥控取消命令的遥控点号参数错误："<<err.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			size_t trans_reason = trans_deact;
			if (terminalPtr->getEncryptOutside())
			{
				trans_reason |= (0x80 << (8 * (TransReasonLength_ - 1)));
			}

			if (yk_type == DataBase::YkClose)
			{
				if (terminalPtr->getbHYkDouble(yk_no))
				{
					bytesAssemble = AssembleDoubleYKCancel(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_CLOSE,trans_reason);
				}
				else
				{
					bytesAssemble = AssembleSingleYKCancel(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_CLOSE,trans_reason);
				}

			}
			else if (yk_type == DataBase::YkOpen)
			{
				if (terminalPtr->getbHYkDouble(yk_no))
				{
					bytesAssemble = AssembleDoubleYKCancel(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_OPEN,trans_reason);
				}
				else
				{
					bytesAssemble = AssembleSingleYKCancel(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_OPEN,trans_reason);
				}

			}
			else
			{
				AddStatusLogWithSynT("遥控取消命令的遥控类型参数非法。\n");
				return -1;
			}

			if (bytesAssemble > 0)
			{
				//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkCancelSend))
				if ((terminalPtr->loadYkPointPtr(yk_no))->SendCancelEvent())
				{
					//std::ostringstream ostr;
					//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkCancelSend<<std::endl;
					//AddStatusLogWithSynT(ostr.str());
					AddStatusLogWithSynT("解析遥控取消命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
					return -1;
				}
				//terminalPtr->setYkStatus(yk_no,DataBase::YkCancelSend);

				setWaitingForAnswer(cmd.getCommPoint());
				ResetTimerYkSel(terminalPtr,yk_no,false);
				ResetTimerYkExe(terminalPtr,yk_no,false);
				ResetTimerYkCancel(terminalPtr,yk_no,true);
			}
		}
		break;

	case CALL_HISTORY_DATA:
		{
			boost::posix_time::ptime time_para;
			try
			{
				time_para = boost::any_cast<boost::posix_time::ptime>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"召唤单点历史数据命令的时间参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}
			bytesAssemble = AssembleCallHistoryData(bufIndex,buf,terminalPtr,time_para);
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case CALL_HISTORY_DATAS:
		{
			DataBase::stSE_TIMEPOINT time_para;
			try
			{
				time_para = boost::any_cast<DataBase::stSE_TIMEPOINT>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"召唤单点历史数据命令的时间参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			bytesAssemble = AssembleCallHistoryDatas(bufIndex,buf,terminalPtr,time_para.startTime,time_para.endTime);
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case CALL_FAULT_RECORD:
		bytesAssemble = AssembleCallFaultRecordData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_YK_RECORD:
		bytesAssemble = AssembleCallYkRecordData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_YX_RECORD:
		bytesAssemble = AssembleCallYxRecordData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_EXTEND_CPUINFO:
		bytesAssemble = AssembleCallExtendRTUInfo(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_EXTEND_PARA:
		bytesAssemble = AssembleCallPara(bufIndex,buf,terminalPtr,0);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case SET_EXTEND_PARA:
		break;

	default:
		break;
	}

	return bytesAssemble;
}

int CH101::AssembleFrameTail( size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	int length = bufIndex - bufBegin;
	if (length <= 1)
	{
		return -1;
	}

	size_t count = bufIndex;

	switch (cmd.getCmdType())
	{
	case REQUIRE_LINK:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;

	case RESET_LINK:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;

	case CALL_PRIMARY_DATA:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;

	case CALL_SECONDARY_DATA:
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

			if (cmd.getCommPoint()->getEncryptOutside() && ((cmd.getCmdType() == YK_SEL_ACT) || (cmd.getCmdType() == YK_EXE_ACT) || (cmd.getCmdType() == YK_CANCEL_ACT)))
			{
				unsigned short trans_reason = BufToVal(&buf[bufBegin + TransReasonLocation_],TransReasonLength_); //获取本来的传送原因
				unsigned short NoEncryptVal = ~((~trans_reason) | (0x80 << (8 * (TransReasonLength_ - 1))));      //把最高位的1改成0
				
				ValToBuf(&buf[bufBegin + TransReasonLocation_],NoEncryptVal,TransReasonLength_);                  //把buf中的传送原因字节改掉
				buf[count++] = CalcCheckSumByte(&buf[bufBegin + 4],framelength);                                  //用没有高位1的传送原因计算校验和
				ValToBuf(&buf[bufBegin + TransReasonLocation_],trans_reason,TransReasonLength_);                  //把传送原因再改回来
			}
			else
			{
				buf[count++] = CalcCheckSumByte(&buf[bufBegin + 4],framelength);
			}

			buf[count++] = 0x16;
			buf[bufBegin + 1] = framelength & 0xff;
			buf[bufBegin + 2] = framelength & 0xff;
		}
		break;
	}

	int tailLength = count - bufIndex;

	//数字签名
	if(CheckPriKey())
	{
		if (cmd.getCmdType() == YK_SEL_ACT || cmd.getCmdType() == YK_EXE_ACT || cmd.getCmdType() == YK_CANCEL_ACT)
		{
			int oriframeLength = count - bufBegin;
			int signlen = encrypt(bufBegin  + getSignStartIndex(),oriframeLength  - getSignStartIndex(),buf) - oriframeLength + getSignStartIndex();
			if(signlen >= 0)
			{
				return tailLength + signlen;
			}
		}
	}

	return tailLength;
}

void CH101::InitDefaultStartAddr()
{
	SYX_START_ADDR_ = DEFAULT_SYX_START_ADDR;                              //单点yx起始地址
	DYX_START_ADDR_ = DEFAULT_DYX_START_ADDR;                              //双点yx起始地址
	YC_START_ADDR_ =  DEFAULT_YC_START_ADDR;                               //yc起始地址
	SYK_START_ADDR_ = DEFAULT_SYK_START_ADDR;                              //单点yk起始地址
	DYK_START_ADDR_ = DEFAULT_DYK_START_ADDR;                              //双点yk起始地址
	YM_START_ADDR_ =  DEFAULT_YM_START_ADDR;                               //ym起始地址
	HIS_START_ADDR_ = DEFAULT_HIS_START_ADDR;                              //历史数据起始地址
}

void CH101::InitDefaultFrameElem()
{
	FrameTypeLength_ =   DEFAULT_FrameTypeLength;                           //报文类型标识的字节长度
	InfoNumLength_ =     DEFAULT_InfoNumLength;                             //信息体数目标识的字节长度
	TransReasonLength_ = DEFAULT_TransReasonLength;                         //传送原因标识的字节长度
	AsduAddrLength_ =    DEFAULT_AsduAddrLength;                            //装置地址标识的字节长度
	InfoAddrLength_ =    DEFAULT_InfoAddrLength;                            //信息体地址标识的字节长度
}

void CH101::InitFrameLocation(size_t frameHead)
{
	FrameTypeLocation_ = frameHead;
	InfoNumLocation_ = FrameTypeLocation_ + FrameTypeLength_;
	TransReasonLocation_ = InfoNumLocation_ + InfoNumLength_;
	AsduAddrLocation_ = TransReasonLocation_ + TransReasonLength_;
	InfoAddrLocation_ = AsduAddrLocation_ + AsduAddrLength_;
	DataLocation_ = InfoAddrLocation_ + InfoAddrLength_;
}

void CH101::InitDefaultTimeOut()
{
	bUseTimeOutQueryUnActivePoint_ = false;
	timeOutQueryUnActivePoint_ = DEFAULT_timeOutQueryUnActivePoint;
	timeOutRequireLink_ = DEFAULT_timeOutRequireLink;
	timeOutCallAllData_ = DEFAULT_timeOutCallAllData;
	timeOutCallAllDD_ = DEFAULT_timeOutCallAllDD;
	timeOutSynTime_ = DEFAULT_timeOutSynTime;
	timeOutHeartFrame_ = DEFAULT_timeOutHeartFrame;
	timeOutYkSel_ = DEFAULT_timeOutYkSel;
	timeOutYkExe_ = DEFAULT_timeOutYkExe;
	timeOutYkCancel_ = DEFAULT_timeOutYkCancel;
	timeOutCallPara_ = DEFAULT_timeOutCallPara;
	timeOutSetPara_ = DEFAULT_timeOutSetPara;
	timeOutDelay_ = DEFAULT_timeOutDelay_;

}

void CH101::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	timerRequireLink_.reset(new deadline_timer(io_service,seconds(timeOutRequireLink_)));
	AddTimer(timerRequireLink_);

	timerCallAllData_.reset(new deadline_timer(io_service,seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllData,timeOutCallAllData_))));
	AddTimer(timerCallAllData_);

	timerCallAllDD_.reset(new deadline_timer(io_service,seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllDD,timeOutCallAllDD_))));
	AddTimer(timerCallAllDD_);

	timerSynTime_.reset(new deadline_timer(io_service,seconds(getMeanvalueOfPointsSum(MIN_timeOutSynTime,timeOutSynTime_))));
	AddTimer(timerSynTime_);

	timerHeartFrame_.reset(new deadline_timer(io_service,seconds(timeOutHeartFrame_)));
	AddTimer(timerHeartFrame_);

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

	timerDelay_.reset(new deadline_timer(io_service,seconds(timeOutDelay_)));
	AddTimer(timerDelay_);

}

int CH101::QueryUnAliveCommPoint(share_commpoint_ptr point)
{
	if (point)
	{
		if (bUseTimeOutQueryUnActivePoint_)
		{
			timeOutRequireLink_ = timeOutQueryUnActivePoint_;
		}

		ResetTimerRequireLink(point,true);

		return 0;
	}

	return -1;
}

void CH101::handle_timerRequireLink(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,false,point);
		if (val)
		{
			AddSendCmdVal(REQUIRE_LINK,REQUIRE_LINK_PRIORITY,val);
		}
	}
}

void CH101::handle_timerCallAllData(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,true,point);
		if (val)
		{
			AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,val);
		}
	}
}

void CH101::handle_timerCallAllDD(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,true,point);
		if (val)
		{
			AddSendCmdVal(CALL_ALL_DD_ACT,CALL_ALL_DD_ACT_PRIORITY,val);
		}
	}
}

bool CH101::CheckSynPoint(share_commpoint_ptr point)
{
	return (point->getCommActive() && point->getDelayCommPointFlag());
}

void CH101::handle_timerSynTime(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr NextPoint;
		if (bTransDelay_)
		{
			NextPoint = getNextCommPointBySelfDef(TERMINAL_NODE,boost::bind(&CH101::CheckSynPoint,this,_1),point);
		}
		else
		{
			NextPoint = getNextCommPoint(TERMINAL_NODE,true,point);
		}
		
		if (NextPoint)
		{
			AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,NextPoint);
		}
	}
}

bool CH101::CheckDelayPoint(share_commpoint_ptr point)
{
	return (point->getCommActive() && (!point->getDelayCommPointFlag()));
}

void CH101::handle_timerDelay(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr NextPoint = getNextCommPointBySelfDef(TERMINAL_NODE,boost::bind(&CH101::CheckDelayPoint,this,_1),point);
		if (NextPoint)
		{
			AddSendCmdVal(TRANS_DELAY_ACT,TRANS_DELAY_ACT_PRIORITY,NextPoint);
		}
	}
}

void CH101::handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr NextPoint = getNextCommPoint(TERMINAL_NODE,true,point);
		if (NextPoint)
		{
			AddOnlySendCmdByCmdType(CALL_SECONDARY_DATA,CALL_SECONDARY_DATA_PRIORITY,NextPoint,boost::any());
		}
	}
}

void CH101::handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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
					//terminalPtr->setYkStatus(yk_no,DataBase::YkSelTimeOut);
					(terminalPtr->loadYkPointPtr(yk_no))->TimeOutEvent();
				}
			}

			CmdConSig_(YK_SEL_CON,RETURN_CODE_TIMEOUT,point,(int)yk_no);
			AddStatusLogWithSynT("H101规约遥控选择命令超时。\n");
		}
	}
}

void CH101::handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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

			CmdConSig_(YK_EXE_CON,RETURN_CODE_TIMEOUT,point,(int)yk_no);
			AddStatusLogWithSynT("H101规约遥控执行命令超时。\n");
		}
	}
}

void CH101::handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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
					//terminalPtr->setYkStatus(yk_no,DataBase::YkCancelTimeOut);
					(terminalPtr->loadYkPointPtr(yk_no))->TimeOutEvent();
				}
			}

			CmdConSig_(YK_CANCEL_CON,RETURN_CODE_TIMEOUT,point,(int)yk_no);
			AddStatusLogWithSynT("H101规约遥控取消命令超时。\n");
		}
	}
}

void CH101::handle_timerCallPara(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		if (point)
		{
			CmdConSig_(CALL_EXTEND_PARA,RETURN_CODE_TIMEOUT,point,0);
		}
	}
}

void CH101::handle_timerSetPara(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		if (point)
		{
			CmdConSig_(SET_EXTEND_PARA,RETURN_CODE_TIMEOUT,point,0);
		}
	}
}

void CH101::ResetTimerRequireLink(share_commpoint_ptr point,bool bContinue /*= true*/, unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerRequireLink_->expires_from_now(boost::posix_time::seconds(timeOutRequireLink_));
		}
		else
		{
			timerRequireLink_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerRequireLink_->async_wait(boost::bind(&CH101::handle_timerRequireLink,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerRequireLink_->cancel();
	}
}

void CH101::ResetTimerCallAllData(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,true,point);
		if(nextPoint)
		{
			if(nextPoint->getInitCommPointFlag())
			{
				if (val == 0)
				{
					timerCallAllData_->expires_from_now(boost::posix_time::seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllData,timeOutCallAllData_)));
				}
				else
				{
					timerCallAllData_->expires_from_now(boost::posix_time::seconds(val));
				}
			}
			else
			{
				timerCallAllData_->expires_from_now(boost::posix_time::seconds(MIN_timeOutCallAllData));
			}

			timerCallAllData_->async_wait(boost::bind(&CH101::handle_timerCallAllData,this,boost::asio::placeholders::error,point));
		}
	}
	else
	{
		timerCallAllData_->cancel();
	}
}

void CH101::ResetTimerCallAllDD(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerCallAllDD_->expires_from_now(boost::posix_time::seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllDD,timeOutCallAllDD_)));
		}
		else
		{
			timerCallAllDD_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerCallAllDD_->async_wait(boost::bind(&CH101::handle_timerCallAllDD,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallAllDD_->cancel();
	}
}

void CH101::ResetTimerSynTime(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,true,point);
		if(nextPoint)
		{
			if(nextPoint->getSynTCommPointFlag())
			{
				if (val == 0)
				{
					timerSynTime_->expires_from_now(boost::posix_time::seconds(getMeanvalueOfPointsSum(MIN_timeOutSynTime,timeOutSynTime_)));
				}
				else
				{
					timerSynTime_->expires_from_now(boost::posix_time::seconds(val));
				}
			}
			else
			{
				timerSynTime_->expires_from_now(boost::posix_time::seconds(MIN_timeOutSynTime));
			}

			timerSynTime_->async_wait(boost::bind(&CH101::handle_timerSynTime,this,boost::asio::placeholders::error,point));
		}
	}
	else
	{
		timerSynTime_->cancel();
	}
}

void CH101::ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerHeartFrame_->expires_from_now(boost::posix_time::seconds(timeOutHeartFrame_));
		}
		else
		{
			timerHeartFrame_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerHeartFrame_->async_wait(boost::bind(&CH101::handle_timerHeartFrame,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerHeartFrame_->cancel();
	}
}

void CH101::ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkSel_->async_wait(boost::bind(&CH101::handle_timerYkSel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkSel_->cancel();
	}
}

void CH101::ResetTimerYkExe(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkExe_->async_wait(boost::bind(&CH101::handle_timerYkExe,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkExe_->cancel();
	}
}

void CH101::ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkCancel_->async_wait(boost::bind(&CH101::handle_timerYkCancel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkCancel_->cancel();
	}
}

void CH101::ResetTimerCallPara(share_commpoint_ptr point,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerCallPara_->async_wait(boost::bind(&CH101::handle_timerCallPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallPara_->cancel();
	}
}

void CH101::ResetTimerSetPara(share_commpoint_ptr point,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerSetPara_->async_wait(boost::bind(&CH101::handle_timerSetPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerSetPara_->cancel();
	}
}

void CH101::ResetTimerDelay(share_commpoint_ptr point,bool bContinue /* = false */,unsigned short val /* = 0 */)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerDelay_->expires_from_now(boost::posix_time::seconds(timeOutDelay_));
		}
		else
		{
			timerDelay_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerDelay_->async_wait(boost::bind(&CH101::handle_timerDelay,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerSetPara_->cancel();
	}
}

//para api
int CH101::setSYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	SYX_START_ADDR_ = val;

	return 0;
}

int CH101::setDYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	DYX_START_ADDR_ = val;

	return 0;
}

int CH101::setYC_START_ADDR(size_t val)
{
	if (val < 0x101 || val >= 0x6001)
	{
		return -1;
	}

	YC_START_ADDR_ = val;

	return 0;
}

int CH101::setSYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	SYK_START_ADDR_ = val;

	return 0;
}

int CH101::setDYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	DYK_START_ADDR_ = val;

	return 0;
}

int CH101::setYM_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	YM_START_ADDR_ = val;

	return 0;
}

int CH101::setHIS_START_ADDR(size_t val)
{
	HIS_START_ADDR_ = val;

	return 0;
}

int CH101::setFrameTypeLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameTypeLength_ = val;

	return 0;
}

int CH101::setInfoNumLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	InfoNumLength_ = val;

	return 0;
}

int CH101::setTransReasonLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	TransReasonLength_ = val;

	return 0;
}

int CH101::setAsduAddrLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	AsduAddrLength_ = val;

	return 0;
}

int CH101::setInfoAddrLength(unsigned short val)
{
	if (val <= 0 || val > 12)
	{
		return -1;
	}

	InfoAddrLength_ = val;

	return 0;
}

int CH101::setTimeOutQueryUnActivePoint(unsigned short val)
{
	if (val < MIN_timeOutQueryUnActivePoint)
	{
		return -1;
	}

	timeOutQueryUnActivePoint_ = val;

	return 0;
}

int CH101::setTimeOutRequrieLink(unsigned short val)
{
	if (val < MIN_timeOutRequireLink || val > 60)
	{
		return -1;
	}

	timeOutRequireLink_ = val;

	return 0;
}

int CH101::setTimeOutCallAllData(unsigned short val)
{
	if (val < 60 || val > 12000)
	{
		return -1;
	}

	timeOutCallAllData_ = val;

	return 0;
}

int CH101::setTimeOutCallAllDD(unsigned short val)
{
	if (val < 60 || val > 12000)
	{
		return -1;
	}

	timeOutCallAllDD_ = val;

	return 0;
}

int CH101::setTimeOutSynTime(unsigned short val)
{
	if (val < 60 || val > 12000)
	{
		return -1;
	}

	timeOutSynTime_ = val;

	return 0;
}

int CH101::setTimeOutHeartFrame(unsigned short val)
{
	if (val <= 0 || val > 60) 
	{
		return -1;
	}

	timeOutHeartFrame_ = val;

	return 0;
}

int CH101::setTimeOutYkSel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkSel_ = val;

	return 0;
}

int CH101::setTimeOutYkExe(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkExe_ = val;

	return 0;
}

int CH101::setTimeOutYkCancel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkCancel_ = val;

	return 0;
}

int CH101::setTimeOutCallPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutCallPara_ = val;

	return 0;
}

int CH101::setTimeOutSetPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutSetPara_ = val;

	return 0;
}

};//namespace Protocol
