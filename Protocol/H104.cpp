#include <boost/bind.hpp>
//#include <boost/algorithm/string/trim.hpp>
//#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "H104.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/Terminal.h"
#include "../DataBase/YkPoint.h"
#include "../DataBase/YxPoint.h"
#include "../DataBase/YcPoint.h"
#include "BF533_DataBase.h"

namespace Protocol{

const std::string strDefaultCfg = "H104Cfg.xml";
size_t CH104::H104ObjectCounter_ = 1;

//针对104规约的YK功能码
const unsigned char DYK_TYPE_OPEN = 0x01;
const unsigned char DYK_TYPE_CLOSE = 0x02;
const unsigned char SYK_TYPE_OPEN = 0;
const unsigned char SYK_TYPE_CLOSE = 0x01;

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
const unsigned char trans_file = 0x0d;
const unsigned char trans_all = 0x14;

//针对104规约的报文类型标识定义
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

const unsigned char M_EP_TJ_1 = 0x29; //41   带时标CP56Time2a和模拟量参数的继电保护设备事件
const unsigned char M_EP_TZ_1 = 0x6C; //108  召唤继电保护定值
const unsigned char M_EP_TX_1 = 0x72; //114  响应继电保护定值
const unsigned char M_EP_TS_1 = 0x73; //115  下装继电保护定值
const unsigned char M_EP_TH_1 = 0x74; //116  激活继电保护定值
const unsigned char M_EP_TG_1 = 0x35; //53   激活继电保护信号复归 //0x34修改为0x35

const unsigned char EnableISQ = 0x00;
const unsigned char DisableISQ = 0x80;
const unsigned char QOI = 0x14;

const std::string strCheckRecvI = "CheckRecvI";
const std::string strCheckSendI = "CheckSendI";
const std::string strSUM_K = "SUM_K";
const std::string strSUM_W = "SUM_W";
const std::string strTimeOutIGramRecv = "TimeOutIGramRecv";

const std::string strIGramCounterLength = "IGramCounterLength";

const unsigned char SYN_HEAD_LENGTH = 1;

CH104::CH104(boost::asio::io_service & io_service)
			:CProtocol(io_service)
{
	SynCharNum_ = 2;
	bInitQuality_ = true;

	bCheckIFrameRecvCounter_ = true;
	bCheckIFrameSendCounter_ = false;
	H104_SUM_K_ = DEFAULT_SUM_K;
	H104_SUM_W_ = DEFAULT_SUM_W;

	InitObjectIndex();
	InitDefaultStartAddr();
	InitDefaultFrameElem();
	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);

	EnablePriKey();
}

CH104::~CH104(void)
{
	H104ObjectCounter_--;
}

int CH104::LoadXmlCfg(std::string filename)
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
		if (xml.FindElem(strFrameLenLength))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short len = boost::lexical_cast<unsigned short>(strTmp);
				setFrameLenLength(len);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setFrameLenLength(DEFAULT_FrameLenLength);
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strIGramCounterLength))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short len = boost::lexical_cast<unsigned short>(strTmp);
				setIGramCounterLength(len);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setIGramCounterLength(DEFAULT_IGramCounterLength);
			}
		}

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
				setTimeOutQueryUnActivePoint(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutQueryUnActivePoint(DEFAULT_timeOutQueryUnActivePoint);
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

		xml.ResetMainPos();
		if (xml.FindElem(strTimeOutIGramRecv))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutIGramFrameRecv(timeout);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutIGramFrameRecv(DEFAULT_timeOutIGramFrameRecv);
			}
		}

		xml.OutOfElem();
	}

	xml.ResetMainPos();
	if (xml.FindElem(strCheckSendI))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			bCheckIFrameSendCounter_ = true;
		}
		else
		{
			bCheckIFrameSendCounter_ = false;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strCheckRecvI))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolFalse,strTmp))
		{
			bCheckIFrameRecvCounter_ = false;
		}
		else
		{
			bCheckIFrameRecvCounter_ = true;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strSUM_K))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			H104_SUM_K_ = boost::lexical_cast<unsigned short>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();

			H104_SUM_K_ = DEFAULT_SUM_K;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strSUM_W))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			H104_SUM_W_ = boost::lexical_cast<unsigned short>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();

			H104_SUM_W_ = DEFAULT_SUM_K;
		}
	}

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CH104::SaveXmlCfg(std::string filename)
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

	if (FrameLenLength_ != DEFAULT_FrameLenLength)
	{
		xml.AddElem(strFrameLenLength,FrameLenLength_);
		bSave = true;
	}

	if (IGramCounterLength_ != DEFAULT_IGramCounterLength)
	{
		xml.AddElem(strIGramCounterLength,IGramCounterLength_);
		bSave = true;
	}

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

	if (timeOutQueryUnActivePoint_ != DEFAULT_timeOutQueryUnActivePoint)
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

	if (timeOutIGramFrameRecv_ != DEFAULT_timeOutIGramFrameRecv)
	{
		xml.AddElem(strTimeOutIGramRecv,timeOutIGramFrameRecv_);
	}

	xml.OutOfElem();
	if (!bSave)
	{
		xml.RemoveElem();
	}

	if(bCheckIFrameSendCounter_)
	{
		xml.AddElem(strCheckSendI,bCheckIFrameSendCounter_);
	}

	if (!bCheckIFrameRecvCounter_)
	{
		xml.AddElem(strCheckRecvI,bCheckIFrameRecvCounter_);
	}

	if (H104_SUM_K_ != DEFAULT_SUM_K)
	{
		xml.AddElem(strSUM_K,H104_SUM_K_);
	}

	if (H104_SUM_W_ != DEFAULT_SUM_W)
	{
		xml.AddElem(strSUM_W,H104_SUM_W_);
	}

	xml.OutOfElem();

	xml.Save(filename);
}

void CH104::InitObjectIndex()
{
	ProtocolObjectIndex_ = H104ObjectCounter_++;
}

int CH104::InitProtocol()
{
	CProtocol::InitProtocol();

	InitFrameLocation(SYN_HEAD_LENGTH);
	InitFrameLength();

	ClearIFrameCounter();
	ClearCounterKI();
	ClearCounterKU();
	ClearCounterWI();

	if(getCommPointSum() > 0)
	{
		//share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,false,getCommPointSum() - 1);
		share_commpoint_ptr nextPoint = getFirstCommPoint();
		if (nextPoint)
		{
			AddSendCmdVal(START_ACT,START_ACT_PRIORITY,nextPoint);
		}
	}

	AddStatusLogWithSynT("H104规约的通道打开成功。\n");

	return 0;
}

void CH104::UninitProtocol()
{
	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("H104规约的通道关闭成功。\n");
}

int CH104::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	if (buf[0] == 0x68)
	{
		int len = BufToVal(&buf[FrameLenLocation_],FrameLenLength_);
		if(len >= 4)
		{
			exceptedBytes = len + FrameLenLength_ + SYN_HEAD_LENGTH;

			return 0;
		}
	}

	return -1;
}

int CH104::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

//recv frame parse
int CH104::ParseFrame_S(unsigned char * buf)
{
	int ret = 0;

	unsigned short OppositeRecvCounter = (buf[4]&0xfe) + (buf[5]*0x100);
	if (CheckIFrameSendCounter(OppositeRecvCounter))
	{
		ReConnnectChannel();
		return -1;
	}

	setLastRecvPointIndex(getLastSendPointIndex());

	ClearCounterKI();

	return ret;
}

int CH104::ParseFrame_U(unsigned char * buf)
{
	int ret = 0;

	setLastRecvPointIndex(getLastSendPointIndex());

	ClearCounterKU();

	switch (buf[RecvCounterLocation_])
	{
	case 0x07:
		{
			share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,boost::logic::indeterminate,getLastRecvPointIndex());
			if (nextPoint)
			{
				AddSendCmdVal(START_CONFIRM,START_CONFIRM_PRIORITY,nextPoint);
			}
		}
		break;

	case 0x0b:
		{
			ClearIFrameCounter();
			ClearCounterKI();
			ClearCounterKU();
			ClearCounterWI();

			share_commpoint_ptr nextUnlivePoint = getNextCommPoint(TERMINAL_NODE,false,getLastRecvPointIndex());
			share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,boost::logic::indeterminate,getLastRecvPointIndex());

			if (nextUnlivePoint)
			{
				ResetTimerRequireLink(nextUnlivePoint,false);
			}
			
			if (nextPoint)
			{
				AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,nextPoint);
				AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,nextPoint);
				ResetTimerHeartFrame(nextPoint,true);
			}
		}
		break;

	case 0x13:
		{
			share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,true,getLastRecvPointIndex());
			if (nextPoint)
			{
				AddSendCmdVal(STOP_CONFIRM,STOP_CONFIRM_PRIORITY,nextPoint);
			}
		}
		break;

	case 0x23:
		ClearIFrameCounter();
		ClearCounterKI();
		ClearCounterKU();
		ClearCounterWI();
		break;

	case 0x43:
		{
			share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,true,getLastRecvPointIndex());
			if (nextPoint)
			{
				AddSendCmdVal(TEST_CONFIRM,TEST_CONFIRM_PRIORITY,nextPoint);
			}
		}
		break;

	case 0x83:
		{
			share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,true,getLastRecvPointIndex());
			if (nextPoint)
			{
				ResetTimerHeartFrame(nextPoint,true);
			}
		}
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

int CH104::ParseFrame_I(unsigned char * buf)
{
	unsigned short OppositeSendCounter = ~(~(BufToVal(&buf[RecvCounterLocation_],IGramCounterLength_)) | 0x01); //对端I格式报文发送计数器
	unsigned short OppositeRecvCounter = ~(~(BufToVal(&buf[SendCounterLocation_],IGramCounterLength_)) | 0x01); //对端I格式报文接收计数器

	if (CheckIFrameRecvCounter(OppositeSendCounter) || CheckIFrameSendCounter(OppositeRecvCounter))
	{
		ReConnnectChannel();
		return -1;
	}

	size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识
	size_t TransReason = BufToVal(&buf[TransReasonLocation_],TransReasonLength_) & 0xff;  //传送原因
	size_t Addr = BufToVal(&buf[AsduAddrLocation_],AsduAddrLength_);               //RTU地址
	unsigned char Data_Code = buf[DataLocation_] & 0x80;

	//std::ostringstream ostrDebug;
	//ostrDebug<<"FrameType="<<FrameType<<" TransReason="<<TransReason<<" Addr="<<Addr<<std::endl;
	//AddStatusLogWithSynT(ostrDebug.str());

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
		ostr<<"H104规约不能根据接收报文中的地址匹配terminal ptr,这帧报文将不会被解析。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		PrintDebug(ostr.str());

		return -1;
	}

	IGramFrameRecv();
	ClearCounterKI();
	ResetTimerIGramFrameRecv(terminalPtr,true);

	if (CheckCounterWI())
	{
		AddSendCmdVal(S_GRAM_FRAME,S_GRAM_FRAME_PRIORITY,terminalPtr);
	}

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

	case M_ME_NA_1: //yc with yc_valid 归一化值
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

	case M_EP_TJ_1: //chengdu protect fault record
//		if (TransReason == trans_spont)
		{
			ParseJBSoe(buf,terminalPtr);
		}
		/*else
		{
			std::ostringstream ostr;
			ostr<<"保护记录事件报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}*/
		break;

		case M_EP_TX_1: //chengdu protect fault record
//		if (TransReason == trans_req)
		{
			ParseCallJBParaCon(buf,terminalPtr);
		}
		/*else
		{
			std::ostringstream ostr;
			ostr<<"保护记录事件报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}*/
		break;

		case M_EP_TS_1: //chengdu protect fault record
//		if (TransReason == trans_actcon)
		{
			ParseSendJBParaCon(buf,terminalPtr);
		}
		/*else
		{
			std::ostringstream ostr;
			ostr<<"保护记录事件报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}*/
		break;

		case M_EP_TH_1: //chengdu protect fault record
			if (TransReason == trans_actcon)
			{
				ParseActALLParaCon(buf,terminalPtr);
				setLastRecvCmd(M_EP_TH_1);
			}
			else if (TransReason == trans_deactcon)
			{
				ParseDeacALLtParaCon(buf,terminalPtr);
			}
			else
			{
				std::ostringstream ostr;
				ostr<<"激活继电保护定值，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;

		case M_EP_TG_1: //chengdu protect fault record
//		if (TransReason == trans_actcon)
		{
			ParseSignalResetCon(buf,terminalPtr);
		}
		/*else
		{
			std::ostringstream ostr;
			ostr<<"保护记录事件报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}*/
		break;

	default:
		{
			std::ostringstream ostr;
			ostr<<"接收报文错误，未定义的报文类型 FRAME_TYPE ="<<FrameType<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;
	}

	//ostrDebug.str("");
	//ostrDebug<<"TerminalIndex="<<terminalIndex<<std::endl;
	//AddStatusLogWithSynT(ostrDebug.str());

	return terminalIndex;
}

int CH104::ParseDoubleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseDoubleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseDoubleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseDoubleYKOverCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseSingleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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
		AddStatusLogWithSynT("收到遥控选择返校报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
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

int CH104::ParseSingleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseSingleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseSingleYKOverCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseAllDataCallCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	//terminalPtr->setInitCommPointFlag(true);

	ResetTimerCallAllData(terminalPtr,true);

	return 0;
}

int CH104::ParseAllDataCallOver(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	terminalPtr->setInitCommPointFlag(true);

	ResetTimerCallAllData(terminalPtr,true);

	return 0;
}

int CH104::ParseAllSingleYX(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseAllDoubleYX(unsigned char * buf, share_terminal_ptr terminalPtr)
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

				terminalPtr->SaveYxQuality(info_addr,yx_val);
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

int CH104::ParseAllYXByte(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseSingleCOS(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseDoubleCOS(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseAllYCData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;
	//int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - YC_START_ADDR_;

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

int CH104::ParseAllYCDataWithValid(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ = (BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;
	//int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - YC_START_ADDR_;

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

int CH104::ParseAllYCDataWithValidTE(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseYCCH(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseYCCHWithValid(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseYCCHWithValidTE(unsigned char * buf, share_terminal_ptr terminalPtr)
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
		unsigned char frame_length = BufToVal(&buf[FrameLenLocation_],FrameLenLength_) + FrameLenLength_ + SYN_HEAD_LENGTH;
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
			if (info_addr + i >= 0 && info_addr + i < (int)terminalPtr->getRecvYcSum())
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
		unsigned char frame_length = BufToVal(&buf[FrameLenLocation_],FrameLenLength_) + FrameLenLength_ + SYN_HEAD_LENGTH;
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

int CH104::ParseSynTimeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	terminalPtr->setSynTCommPointFlag(true);

	ResetTimerSynTime(terminalPtr,true);

	return 0;
}

int CH104::ParseSingleSOE(unsigned char * buf, share_terminal_ptr terminalPtr)
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
		unsigned char frame_length = BufToVal(&buf[FrameLenLocation_],FrameLenLength_) + FrameLenLength_ + SYN_HEAD_LENGTH;
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
		unsigned char frame_length = BufToVal(&buf[FrameLenLocation_],FrameLenLength_) + FrameLenLength_ + SYN_HEAD_LENGTH;
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

int CH104::ParseDoubleSOE(unsigned char * buf, share_terminal_ptr terminalPtr)
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
		unsigned char frame_length = BufToVal(&buf[FrameLenLocation_],FrameLenLength_) + FrameLenLength_ + SYN_HEAD_LENGTH;
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
			if (info_addr + i >= 0 && info_addr + i < (int)terminalPtr->getRecvYxSum())
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
		unsigned char frame_length = BufToVal(&buf[FrameLenLocation_],FrameLenLength_) + FrameLenLength_ + SYN_HEAD_LENGTH;
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

int CH104::ParseAllYMCallCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseAllYMCallOver(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseAllYMData(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH104::ParseEndInit(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseSetParaCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseSetParaErr(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseAllParaData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseExtendRTUInfo(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseHistoryData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseNoHistoryData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseEndHistoryData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseFaultRecordData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseFaultRecordEnd(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseYkRecordData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseYkRecordEnd(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseYxRecordData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH104::ParseYxRecordEnd(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int  CH104::ParseJBSoe(unsigned char * buf,share_terminal_ptr terminalPtr)
{
	int Location = 12;  

	DataBase::stEVENT Val;

	Val.FaultNo =BufToVal(&buf[Location],2)/*-JB_PARA_START_ADDR_*/;
	Val.LineNo = BufToVal(&buf[Location+2],1);
	Val.SPE = BufToVal(&buf[Location+3],1);
	Val.YcValue = BufToVal(&buf[Location+4],2);
	Val.millisecond= BufToVal(&buf[Location+6],2);
	Val.min = (BufToVal(&buf[Location+8],1)) & 0x3f;
	Val.hour = (BufToVal(&buf[Location+9],1)) & 0x1f;
	Val.day = (BufToVal(&buf[Location+10],1)) & 0x1f;
	Val.month = (BufToVal(&buf[Location+11],1)) & 0x0f;
	Val.year = (BufToVal(&buf[Location+12],1)) + 2000;

	Val.Type=2;

	CmdConSig_(EVENT_MESSAGE,RETURN_CODE_ACTIVE,terminalPtr,Val);

	return 0;
}

int  CH104::ParseCallJBParaCon(unsigned char * buf,share_terminal_ptr terminalPtr)
{
	int Location = 12;  

	DataBase::stJBPARA Val;

	for(int i=0;i<48;i++)
	{
		Val.LineNo[i]=BufToVal(&buf[Location],2);
		Val.LineNo[i]=BufToVal(&buf[Location+2],1);
		Val.Value[i] =BufToVal(&buf[Location+3],1);

	}

	CmdConSig_(CALL_JB_PARA_CON,CALL_JB_PARA_CON_PRIORITY,terminalPtr,Val);//通知对上规约校验成功

	return 0;
}

int  CH104::ParseSendJBParaCon(unsigned char * buf,share_terminal_ptr terminalPtr)
{
	CmdConSig_(SEND_JB_PARA_CON,SEND_JB_PARA_CON_PRIORITY,terminalPtr,0);//通知对上规约校验成功
	return 0;
}

int  CH104::ParseActALLParaCon(unsigned char * buf,share_terminal_ptr terminalPtr)
{
	CmdConSig_(ACT_JB_PARA_CON,ACT_JB_PARA_CON_PRIORITY,terminalPtr,0);//通知对上规约校验成功
	return 0;
}

int  CH104::ParseDeacALLtParaCon(unsigned char * buf,share_terminal_ptr terminalPtr)
{
	CmdConSig_(DEACT_JB_PARA_CON,DEACT_JB_PARA_CON_PRIORITY,terminalPtr,0);//通知对上规约校验成功

	return 0;
}

int  CH104::ParseSignalResetCon(unsigned char * buf,share_terminal_ptr terminalPtr)
{
	int Location = 14/*15*/;
	int LineNo_ = BufToVal(&buf[Location],1);
	CmdConSig_(SIGNAL_RESET_CON,RETURN_CODE_ACTIVE,terminalPtr,LineNo_);//通知对上规约校验成功

	return 0;
}

int CH104::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	int ret = -1;

	ResetTimerAnyFrameRecv(true);
	share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,boost::logic::indeterminate,getLastSendPointIndex());
	if (nextPoint)
	{
		ResetTimerHeartFrame(nextPoint);
	}
	
	if (exceptedBytes == (FrameLenLength_ + IGramCounterLength_ * 2 + 1))
	{
		if ((buf[RecvCounterLocation_] & 0x03) == 0x01)
		{
			ret = ParseFrame_S(buf);
		}
		else if ((buf[RecvCounterLocation_] & 0x03) == 0x03)
		{
			ret = ParseFrame_U(buf);
		}
	}
	else if ((buf[RecvCounterLocation_] & 0x01) == 0)
	{
		ret = ParseFrame_I(buf);
	}

	return ret;
}

//send frame assemble
int CH104::AssembleSGram(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x01;
	buf[count++] = 0;
	count += ValToBuf(&buf[count],IFrameRecvCounter_,IGramCounterLength_);

	return count - bufIndex;
}

int CH104::AssembleStartDTAct(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x07;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CH104::AssembleStartDTCon(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x0b;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CH104::AssembleStopDTAct(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x13;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CH104::AssembleStopDTCon(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x23;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CH104::AssembleTestFRAct(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x43;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CH104::AssembleTestFRCon(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x83;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CH104::AssembleCallAllData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_IC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI;

	return count - bufIndex;
}

int CH104::AssembleCallAllDD(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_CI_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI;

	return count - bufIndex;
}

int CH104::AssembleSynTime(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr, boost::posix_time::ptime time)
{
	size_t count = bufIndex;

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

int CH104::AssembleDoubleYKSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH104::AssembleDoubleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CH104::AssembleDoubleYKCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no,unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH104::AssembleSingleYKSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH104::AssembleSingleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CH104::AssembleSingleYKCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no,unsigned char yk_code,size_t trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH104::AssembleCallPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int secondaryIndex)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],0x81,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	buf[count++] = terminalPtr->getAddr() % 0xff;
	buf[count++] = 0x01;
	buf[count++] = 0;
	buf[count++] = QOI;

	return count - bufIndex;
}

int CH104::AssembleSetPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,int secondaryIndex)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CH104::AssembleCallExtendRTUInfo(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],0x83,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI;

	return count - bufIndex;
}

int CH104::AssembleCallHistoryData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,boost::posix_time::ptime time)
{
	size_t count = bufIndex;

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

int CH104::AssembleCallHistoryDatas(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,boost::posix_time::ptime startTime,boost::posix_time::ptime endTime)
{
	size_t count = bufIndex;

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

int CH104::AssembleCallFaultRecordData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],0x96,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH104::AssembleCallYkRecordData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],0x95,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH104::AssembleCallYxRecordData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],0x94,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH104::AssembleCallJBPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	DataBase::stJBPARA Val;
	Val = boost::any_cast<DataBase::stJBPARA>(cmd.getVal());

	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EP_TZ_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_req,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	count += ValToBuf(&buf[count],Val.ParaNo[0],2);
	count += ValToBuf(&buf[count],Val.LineNo[0],1);

	return count - bufIndex;
}

int CH104::AssembleSendJBPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	DataBase::stJBPARA Val;
	Val = boost::any_cast<DataBase::stJBPARA>(cmd.getVal());

	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EP_TS_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	for (int i =0;i<Val.Num;i++)
	{
	count += ValToBuf(&buf[count],Val.ParaNo[i],2);
	count += ValToBuf(&buf[count],Val.LineNo[i],1);
	count += ValToBuf(&buf[count],Val.Value[i],2);
	}

	return count - bufIndex;
}

int CH104::AssembleActJBPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	unsigned int line_no = boost::any_cast<unsigned int>(cmd.getVal());

	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EP_TH_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	count += ValToBuf(&buf[count],0,2);
	count += ValToBuf(&buf[count],line_no,1);

	return count - bufIndex;
}

int CH104::AssembleDeactJBPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
	/*DataBase::stJBPARA Val;
	Val = boost::any_cast<DataBase::stJBPARA>(cmd.getVal());*/
	unsigned int line_no = boost::any_cast<unsigned int>(cmd.getVal());

	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EP_TH_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_deact,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	count += ValToBuf(&buf[count],0,2);
	count += ValToBuf(&buf[count],line_no,1);

	return count - bufIndex;
}

int CH104::AssembleSignalResetJBPara(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,CCmd & cmd)
{
//	DataBase::stJBPARA Val;
//	Val = boost::any_cast<DataBase::stJBPARA>(cmd.getVal());
	unsigned int line_no = boost::any_cast<unsigned int>(cmd.getVal());

	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EP_TG_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	count += ValToBuf(&buf[count],0,2);
	count += ValToBuf(&buf[count],line_no/*Val.LineNo[0]*/,1);

	return count - bufIndex;
}

int CH104::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	switch (cmd.getCmdType())
	{
	case START_ACT:
		buf[count++] = 0x68;
		count += ValToBuf(&buf[count],0,FrameLenLength_);
		break;

	case START_CONFIRM:
		buf[count++] = 0x68;
		count += ValToBuf(&buf[count],0,FrameLenLength_);
		break;

	case STOP_ACT:
		buf[count++] = 0x68;
		count += ValToBuf(&buf[count],0,FrameLenLength_);
		break;

	case STOP_CONFIRM:
		buf[count++] = 0x68;
		count += ValToBuf(&buf[count],0,FrameLenLength_);
		break;

	case TEST_ACT:
		buf[count++] = 0x68;
		count += ValToBuf(&buf[count],0,FrameLenLength_);
		break;

	case TEST_CONFIRM:
		buf[count++] = 0x68;
		count += ValToBuf(&buf[count],0,FrameLenLength_);
		break;

	case S_GRAM_FRAME:
		buf[count++] = 0x68;
		count += ValToBuf(&buf[count],0,FrameLenLength_);
		break;

	default:
		buf[count++] = 0x68;
		count += ValToBuf(&buf[count],0,FrameLenLength_);
		count += ValToBuf(&buf[count],IFrameSendCounter_,IGramCounterLength_);
		count += ValToBuf(&buf[count],IFrameRecvCounter_,IGramCounterLength_);
		break;
	}

	return count - bufIndex;
}

int CH104::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
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
		ostr<<"H104规约不能从发送命令中获得terminal ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (cmd.getCmdType())
	{
	case START_ACT:
		bytesAssemble = AssembleStartDTAct(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			UGramFrameSend();
			ResetTimerRequireLink(terminalPtr,true);
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case START_CONFIRM:
		bytesAssemble = AssembleStartDTCon(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			ClearIFrameCounter();
		}
		break;

	case STOP_ACT:
		bytesAssemble = AssembleStopDTAct(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			UGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case STOP_CONFIRM:
		bytesAssemble = AssembleStopDTCon(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			ClearIFrameCounter();
		}
		break;

	case TEST_ACT:
		bytesAssemble = AssembleTestFRAct(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			UGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerHeartFrame(terminalPtr,true);
		}
		break;

	case TEST_CONFIRM:
		bytesAssemble = AssembleTestFRCon(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			ResetTimerHeartFrame(terminalPtr,true);
		}
		break;

	case S_GRAM_FRAME:
		bytesAssemble = AssembleSGram(bufIndex,buf);
		break;

	case CALL_ALL_DATA_ACT:
		bytesAssemble = AssembleCallAllData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerCallAllData(terminalPtr,true);
		}
		break;

	case CALL_ALL_DD_ACT:
		bytesAssemble = AssembleCallAllDD(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerCallAllDD(terminalPtr,true);
		}
		break;

	case SYN_TIME_ACT:
		bytesAssemble = AssembleSynTime(bufIndex,buf,terminalPtr,boost::posix_time::microsec_clock::local_time());
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerSynTime(terminalPtr,true);
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

				IGramFrameSend();
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

				IGramFrameSend();
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
				if((terminalPtr->loadYkPointPtr(yk_no))->SendCancelEvent())
				{
					//std::ostringstream ostr;
					//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkCancelSend<<std::endl;
					//AddStatusLogWithSynT(ostr.str());
					AddStatusLogWithSynT("解析遥控取消命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
					return -1;
				}

				IGramFrameSend();
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
				IGramFrameSend();
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
				IGramFrameSend();
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case CALL_FAULT_RECORD:
		bytesAssemble = AssembleCallFaultRecordData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_YK_RECORD:
		bytesAssemble = AssembleCallYkRecordData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_YX_RECORD:
		bytesAssemble = AssembleCallYxRecordData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_EXTEND_CPUINFO:
		bytesAssemble = AssembleCallExtendRTUInfo(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case CALL_EXTEND_PARA:
		bytesAssemble = AssembleCallPara(bufIndex,buf,terminalPtr,0);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case SET_EXTEND_PARA:
		break;

	case CALL_JB_PARA_ACT:
		bytesAssemble = AssembleCallJBPara(bufIndex,buf,terminalPtr,cmd);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case SEND_JB_PARA_ACT:
		bytesAssemble = AssembleSendJBPara(bufIndex,buf,terminalPtr,cmd);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case ACT_JB_PARA_ACT:
		bytesAssemble = AssembleActJBPara(bufIndex,buf,terminalPtr,cmd);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case DEACT_JB_PARA_ACT:
		bytesAssemble = AssembleDeactJBPara(bufIndex,buf,terminalPtr,cmd);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case JB_SIGNAL_RESET_ACT:
		bytesAssemble = AssembleSignalResetJBPara(bufIndex,buf,terminalPtr,cmd);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	default:
		break;
	}

	return bytesAssemble;
}

int CH104::AssembleFrameTail( size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	int length = count - bufBegin - (FrameLenLength_ + SYN_HEAD_LENGTH);
	if (length <= 0 || length > max_frame_length_)
	{
		return -1;
	}

	ValToBuf(&buf[FrameLenLocation_],length,FrameLenLength_);

	int tailLength = count - bufIndex;

	//数字签名
	if(CheckPriKey())
	{
		if (cmd.getCmdType() == YK_SEL_ACT || cmd.getCmdType() == YK_EXE_ACT || cmd.getCmdType() == YK_CANCEL_ACT)
		{
			int oriframeLength = count - bufBegin;
			int signlen = encrypt(bufBegin + getSignStartIndex(),oriframeLength - getSignStartIndex(),buf) - oriframeLength + getSignStartIndex();
			if(signlen >= 0)
			{
				return tailLength + signlen;
			}
		}
	}

	return tailLength;
}

void CH104::IGramFrameRecv()
{
	if (IFrameRecvCounter_ >= 0xfffe)
	{
		IFrameRecvCounter_ = 0;
	} 
	else
	{
		IFrameRecvCounter_ += 2;
	}

	count_W_I_++;
}

void CH104::IGramFrameSend()
{
	if (IFrameSendCounter_ >= 0xfffe)
	{
		IFrameSendCounter_ = 0;
	} 
	else
	{
		IFrameSendCounter_ += 2;
	}

	count_K_I_++;
}

void CH104::UGramFrameSend()
{
	count_K_U_++;
}

bool CH104::CheckIFrameRecvCounter(unsigned short sendVal)
{
	if(bCheckIFrameRecvCounter_)
	{
		if (IFrameRecvCounter_ == sendVal)
		{
			return false;
		}

		std::ostringstream ostr;
		ostr<<"H104规约的I格式报文接收计数器 = "<<IFrameRecvCounter_<<",接收报文中对端的发送计数器 = "<<sendVal<<": 计数器错误！"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return true;
	}

	return false;
}

bool CH104::CheckIFrameSendCounter(unsigned short recvVal)
{
	if(bCheckIFrameSendCounter_) //默认把报文计数器的判断条件放松，自己的发送和接收计数器都作判断，但是自己的发送接收计数器出错不重连通道，交给对端的接收去判断。
	{
		if (IFrameSendCounter_ >= recvVal)
		{
			return false;
		}

		std::ostringstream ostr;
		ostr<<"H104规约的I格式报文发送计数器 = "<<IFrameRecvCounter_<<",接收报文中对端的接收计数器 = "<<recvVal<<": 计数器错误！"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return true;
	}

	return false;
}

void CH104::ClearIFrameCounter()
{
	IFrameRecvCounter_ = 0;
	IFrameSendCounter_ = 0;
}

bool CH104::CheckCounterKU()
{
	if(count_K_U_ >= H104_SUM_K_)
	{
		std::ostringstream ostr;
		ostr<<count_K_U_<<"帧U格式报文发送未收到回应。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		ClearCounterKU();

		return true;
	}
	
	return false;
}

void CH104::ClearCounterKU()
{
	count_K_U_ = 0;
}

bool CH104::CheckCounterKI()
{
	if(count_K_I_ >= H104_SUM_K_)
	{
		std::ostringstream ostr;
		ostr<<count_K_I_<<"帧I格式报文发送未收到回应。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		ClearCounterKI();

		return true;
	}
	
	return false;
}

void CH104::ClearCounterKI()
{
	count_K_I_ = 0;
}

bool CH104::CheckCounterWI()
{
	if(count_W_I_ > H104_SUM_W_)
	{
		ClearCounterWI();

		return true;
	}
	
	return false;
}

void CH104::ClearCounterWI()
{
	count_W_I_ = 0;
}

void CH104::InitDefaultStartAddr()
{
	SYX_START_ADDR_ = DEFAULT_SYX_START_ADDR;                              //单点yx起始地址
	DYX_START_ADDR_ = DEFAULT_DYX_START_ADDR;                              //双点yx起始地址
	YC_START_ADDR_ =  DEFAULT_YC_START_ADDR;                               //yc起始地址
	SYK_START_ADDR_ = DEFAULT_SYK_START_ADDR;                              //单点yk起始地址
	DYK_START_ADDR_ = DEFAULT_DYK_START_ADDR;                              //双点yk起始地址
	YM_START_ADDR_ =  DEFAULT_YM_START_ADDR;                               //ym起始地址
	HIS_START_ADDR_ = DEFAULT_HIS_START_ADDR;                              //历史数据起始地址
}

void CH104::InitDefaultFrameElem()
{
	FrameLenLength_ = DEFAULT_FrameLenLength;                               //报文长度标识的字节长度
	IGramCounterLength_ = DEFAULT_IGramCounterLength;
	FrameTypeLength_ = DEFAULT_FrameTypeLength;                             //报文类型标识的字节长度
	InfoNumLength_ =   DEFAULT_InfoNumLength;                               //信息体数目标识的字节长度
	TransReasonLength_ = DEFAULT_TransReasonLength;                         //传送原因标识的字节长度
	AsduAddrLength_ = DEFAULT_AsduAddrLength;                               //装置地址标识的字节长度
	InfoAddrLength_ = DEFAULT_InfoAddrLength;                               //信息体地址标识的字节长度
}

void CH104::InitFrameLocation(size_t frameHead)
{
	FrameLenLocation_ = frameHead;
	RecvCounterLocation_ = FrameLenLocation_ + FrameLenLength_;
	SendCounterLocation_ = RecvCounterLocation_ + IGramCounterLength_;
	FrameTypeLocation_ = SendCounterLocation_ + IGramCounterLength_;
	InfoNumLocation_ = FrameTypeLocation_ + FrameTypeLength_;
	TransReasonLocation_ = InfoNumLocation_ + InfoNumLength_;
	AsduAddrLocation_ = TransReasonLocation_ + TransReasonLength_;
	InfoAddrLocation_ = AsduAddrLocation_ + AsduAddrLength_;
	DataLocation_ = InfoAddrLocation_ + InfoAddrLength_;
}

void CH104::InitFrameLength()
{
	if (FrameLenLength_ > 1)
	{
		max_frame_length_ = MAX_IP_MTU;
		send_buf_.reset(new unsigned char[max_frame_length_]);
		recv_buf_.reset(new PublicSupport::CLoopBuf(max_frame_length_ * 4));
	}
}

void CH104::InitDefaultTimeOut()
{
	bUseTimeOutQueryUnActivePoint_ = false;
	timeOutQueryUnActivePoint_ = DEFAULT_timeOutQueryUnActivePoint;
	timeOutRequireLink_ = DEFAULT_timeOutRequireLink;
	timeOutAnyFrameRecv_ = DEFAULT_timeOutAnyFrameRecv;
	timeOutIGramFrameRecv_ = DEFAULT_timeOutIGramFrameRecv;
	timeOutCallAllData_ = DEFAULT_timeOutCallAllData;
	timeOutCallAllDD_ = DEFAULT_timeOutCallAllDD;
	timeOutSynTime_ = DEFAULT_timeOutSynTime;
	timeOutHeartFrame_ = DEFAULT_timeOutHeartFrame;
	timeOutYkSel_ = DEFAULT_timeOutYkSel;
	timeOutYkExe_ = DEFAULT_timeOutYkExe;
	timeOutYkCancel_ = DEFAULT_timeOutYkCancel;
	timeOutCallPara_ = DEFAULT_timeOutCallPara;
	timeOutSetPara_ = DEFAULT_timeOutSetPara;

}

void CH104::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	timerRequireLink_.reset(new deadline_timer(io_service,seconds(timeOutRequireLink_)));
	AddTimer(timerRequireLink_);

	timerAnyFrameRecv_.reset(new deadline_timer(io_service,seconds(timeOutAnyFrameRecv_)));
	AddTimer(timerAnyFrameRecv_);

	timerIGramFrameRecv_.reset(new deadline_timer(io_service,seconds(timeOutIGramFrameRecv_)));
	AddTimer(timerIGramFrameRecv_);

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
}

int CH104::QueryUnAliveCommPoint(share_commpoint_ptr point)
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

void CH104::handle_timerRequireLink(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,false,point);
		if (val)
		{
			AddSendCmdVal(START_ACT,START_ACT_PRIORITY,val);
		}
	}
}

void CH104::handle_timerCallAllData(const boost::system::error_code& error,share_commpoint_ptr point)
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

void CH104::handle_timerCallAllDD(const boost::system::error_code& error,share_commpoint_ptr point)
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

void CH104::handle_timerSynTime(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,true,point);
		if (val)
		{
			AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,val);
		}
	}
}

void CH104::handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,true,point);
		if (val)
		{
			AddSendCmdVal(TEST_ACT,TEST_ACT_PRIORITY,point);
		}
	}
}

void CH104::handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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
			AddStatusLogWithSynT("H104规约遥控选择命令超时。\n");
		}
	}
}

void CH104::handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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
					//terminalPtr->setYkType(yk_no,DataBase::YkExeTimeOut);
					(terminalPtr->loadYkPointPtr(yk_no))->TimeOutEvent();
				}
			}

			CmdConSig_(YK_EXE_CON,RETURN_CODE_TIMEOUT,point,(int)yk_no);
			AddStatusLogWithSynT("H104规约遥控执行命令超时。\n");
		}
	}
}

void CH104::handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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
			AddStatusLogWithSynT("H104规约遥控取消命令超时。\n");
		}
	}
}

void CH104::handle_timerCallPara(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		if (point)
		{
			CmdConSig_(CALL_EXTEND_PARA,RETURN_CODE_TIMEOUT,point,0);
		}
	}
}

void CH104::handle_timerSetPara(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		if (point)
		{
			CmdConSig_(SET_EXTEND_PARA,RETURN_CODE_TIMEOUT,point,0);
		}
	}
}

void CH104::handle_timerAnyFrameRecv(const boost::system::error_code& error)
{
	if (!error)
	{
		ReConnnectChannel();
	}
}

void CH104::handle_timerIGramFrameRecv(const boost::system::error_code &error,share_commpoint_ptr point)
{
	if (!error)
	{
		ClearCounterWI();
		AddSendCmdVal(S_GRAM_FRAME,S_GRAM_FRAME_PRIORITY,point);
	}
}

void CH104::ResetTimerRequireLink(share_commpoint_ptr point,bool bContinue /*= true*/, unsigned short val/* = 0*/)
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

		timerRequireLink_->async_wait(boost::bind(&CH104::handle_timerRequireLink,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerRequireLink_->cancel();
	}
}

void CH104::ResetTimerCallAllData(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,true,point);
		if (nextPoint)
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

			timerCallAllData_->async_wait(boost::bind(&CH104::handle_timerCallAllData,this,boost::asio::placeholders::error,point));
		}
	}
	else
	{
		timerCallAllData_->cancel();
	}
}

void CH104::ResetTimerCallAllDD(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
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

		timerCallAllDD_->async_wait(boost::bind(&CH104::handle_timerCallAllDD,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallAllDD_->cancel();
	}
}

void CH104::ResetTimerSynTime(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
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

			timerSynTime_->async_wait(boost::bind(&CH104::handle_timerSynTime,this,boost::asio::placeholders::error,point));
		}
	}
	else
	{
		timerSynTime_->cancel();
	}
}

void CH104::ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
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

		timerHeartFrame_->async_wait(boost::bind(&CH104::handle_timerHeartFrame,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerHeartFrame_->cancel();
	}
}

void CH104::ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkSel_->async_wait(boost::bind(&CH104::handle_timerYkSel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkSel_->cancel();
	}
}

void CH104::ResetTimerYkExe(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkExe_->async_wait(boost::bind(&CH104::handle_timerYkExe,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkExe_->cancel();
	}
}

void CH104::ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkCancel_->async_wait(boost::bind(&CH104::handle_timerYkCancel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkCancel_->cancel();
	}
}

void CH104::ResetTimerCallPara(share_commpoint_ptr point,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerCallPara_->async_wait(boost::bind(&CH104::handle_timerCallPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallPara_->cancel();
	}
}

void CH104::ResetTimerSetPara(share_commpoint_ptr point,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerSetPara_->async_wait(boost::bind(&CH104::handle_timerSetPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerSetPara_->cancel();
	}
}

void CH104::ResetTimerAnyFrameRecv( bool bContinue /*= true*/,unsigned short val /*= 0*/ )
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerAnyFrameRecv_->expires_from_now(boost::posix_time::seconds(timeOutAnyFrameRecv_));
		}
		else
		{
			timerAnyFrameRecv_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerAnyFrameRecv_->async_wait(boost::bind(&CH104::handle_timerAnyFrameRecv,this,boost::asio::placeholders::error));
	}
	else
	{
		timerAnyFrameRecv_->cancel();
	}
}

void CH104::ResetTimerIGramFrameRecv(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerIGramFrameRecv_->expires_from_now(boost::posix_time::seconds(timeOutIGramFrameRecv_));
		}
		else
		{
			timerIGramFrameRecv_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerIGramFrameRecv_->async_wait(boost::bind(&CH104::handle_timerIGramFrameRecv,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerIGramFrameRecv_->cancel();
	}
}

//para api
int CH104::setSYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	SYX_START_ADDR_ = val;

	return 0;
}

int CH104::setDYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	DYX_START_ADDR_ = val;

	return 0;
}

int CH104::setYC_START_ADDR(size_t val)
{
	if (val < 0x101 || val >= 0x6001)
	{
		return -1;
	}

	YC_START_ADDR_ = val;

	return 0;
}

int CH104::setSYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	SYK_START_ADDR_ = val;

	return 0;
}

int CH104::setDYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	DYK_START_ADDR_ = val;

	return 0;
}

int CH104::setYM_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	YM_START_ADDR_ = val;

	return 0;
}

int CH104::setHIS_START_ADDR(size_t val)
{
	HIS_START_ADDR_ = val;

	return 0;
}

int CH104::setFrameLenLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameLenLength_ = val;

	return 0;
}

int CH104::setIGramCounterLength(unsigned short val)
{
	if (val < 0 || val > 4)
	{
		return -1;
	}

	IGramCounterLength_ = val;

	return 0;
}

int CH104::setFrameTypeLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameTypeLength_ = val;

	return 0;
}

int CH104::setInfoNumLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	InfoNumLength_ = val;

	return 0;
}

int CH104::setTransReasonLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	TransReasonLength_ = val;

	return 0;
}

int CH104::setAsduAddrLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	AsduAddrLength_ = val;

	return 0;
}

int CH104::setInfoAddrLength(unsigned short val)
{
	if (val <= 0 || val > 12)
	{
		return -1;
	}

	InfoAddrLength_ = val;

	return 0;
}

int CH104::setTimeOutQueryUnActivePoint(unsigned short val)
{
	if (val < MIN_timeOutQueryUnActivePoint)
	{
		return -1;
	}

	timeOutQueryUnActivePoint_ = val;

	return 0;
}

int CH104::setTimeOutRequrieLink(unsigned short val)
{
	if (val < MIN_timeOutRequireLink || val > 60)
	{
		return -1;
	}

	timeOutRequireLink_ = val;

	return 0;
}

int CH104::setTimeOutAnyFrameRecv(unsigned short val)
{
	if (val < 60 || val > 300)
	{
		return -1;
	}

	timeOutAnyFrameRecv_ = val;

	return 0;
}

int CH104::setTimeOutIGramFrameRecv(unsigned short val)
{
	if (val < 1 || val > 60)
	{
		return -1;
	}

	timeOutIGramFrameRecv_ = val;

	return 0;
}

int CH104::setTimeOutCallAllData(unsigned short val)
{
	if (val < 60 || val > 12000)
	{
		return -1;
	}

	timeOutCallAllData_ = val;

	return 0;
}

int CH104::setTimeOutCallAllDD(unsigned short val)
{
	if (val < 60 || val > 12000)
	{
		return -1;
	}

	timeOutCallAllDD_ = val;

	return 0;
}

int CH104::setTimeOutSynTime(unsigned short val)
{
	if (val < 60 || val > 12000)
	{
		return -1;
	}

	timeOutSynTime_ = val;

	return 0;
}

int CH104::setTimeOutHeartFrame(unsigned short val)
{
	if (val <= 0 || val > 60) 
	{
		return -1;
	}

	timeOutHeartFrame_ = val;

	return 0;
}

int CH104::setTimeOutYkSel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkSel_ = val;

	return 0;
}

int CH104::setTimeOutYkExe(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkExe_ = val;

	return 0;
}

int CH104::setTimeOutYkCancel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkCancel_ = val;

	return 0;
}

int CH104::setTimeOutCallPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutCallPara_ = val;

	return 0;
}

int CH104::setTimeOutSetPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutSetPara_ = val;

	return 0;
}

};//namespace Protocol
