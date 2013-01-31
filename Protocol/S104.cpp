#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "S104.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../FileSystem/Markup.h"
#include "../DataBase/PriStation.h"
#include "../DataBase/YkPoint.h"
#include "../DataBase/YxPoint.h"
#include "../DataBase/SoePoint.h"
#include "../DataBase/YcVarPoint.h"

namespace Protocol{

const std::string strDefaultCfg = "S104Cfg.xml";
size_t CS104::S104ObjectCounter_ = 1;
#define strYcSendByTimer  "YcSendByTimer"

//针对104规约的YK功能码
const unsigned char DYK_OPEN_NEGATIVE = 0;
const unsigned char DYK_TYPE_OPEN = 0x01;
const unsigned char DYK_TYPE_CLOSE = 0x02;
const unsigned char SYK_TYPE_OPEN = 0;
const unsigned char SYK_TYPE_CLOSE = 0x01;
const unsigned char DYK_CLOSE_NEGATIVE = 0x03;

//各种报文信息体元素的最大数量，一般情况下其最大值是127（信息体数目元素所占字节数为1的情况)
//const int INFONUM_LIMIT_ALLYXFRAME = 64;
//const int INFONUM_LIMIT_ALLYCFRAME = 48;
//const int INFONUM_LIMIT_ALLDDFRAME = 28;
//const int INFONUM_LIMIT_COSFRAME = 32;
//const int INFONUM_LIMIT_SOEFRAME = 16;
//const int INFONUM_LIMIT_YCVARFRAME = 24;

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
const unsigned char trans_ykfailed = 0x30;

//针对104规约的报文类型标识定义
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
const unsigned char M_IC_NA_1 = 0x64; //100
const unsigned char M_CI_NA_1 = 0x65; //101
const unsigned char M_RD_NA_1 = 0x66; //102?
const unsigned char M_CS_NA_1 = 0x67; //103
const unsigned char M_RP_NA_1 = 0x69; //105
//const unsigned char P_ME_NA_1 = 0x6E; //110
//const unsigned char P_ME_NB_1 = 0x6F; //111
//const unsigned char P_ME_NC_1 = 0x70; //112

const unsigned char M_KY_NA_1 = 0x6C;  //108 ECC公钥传输报文

const unsigned char Local_Power = 0x00;
const unsigned char Local_Switch = 0x01;
const unsigned char Remote_Control = 0x02;

const unsigned char EnableISQ = 0x00;
const unsigned char DisableISQ = 0x80;

const size_t NormalYkFrameLength = 16;

const std::string PubKeyName = "104pubecc.txt"; //长沙公钥名称
const std::string strCheckRecvI = "CheckRecvI";
const std::string strCheckSendI = "CheckSendI";
const std::string strSUM_K = "SUM_K";
const std::string strSUM_W = "SUM_W";
const std::string strTimeOutIGramRecv = "TimeOutIGramRecv";

const std::string strInfoNumLimit = "InfoNumLimit";
const std::string strInfoNumLimitAllYxFrame = "InfoNumLimitAllYxFrame";
const std::string strInfoNumLimitAllYcFrame = "InfoNumLimitAllYcFrame";
const std::string strInfoNumLimitAllDDFrame = "InfoNumLimitAllDDFrame";
const std::string strInfoNumLimitCosFrame = "InfoNumLimitCosFrame";
const std::string strInfoNumLimitSoeFrame = "InfoNumLimitSoeFrame";
const std::string strInfoNumLimitYcVarFrame = "InfoNumLimitYcVarFrame";

const std::string strIGramCounterLength = "IGramCounterLength";

const unsigned char SYN_HEAD_LENGTH = 1;

CS104::CS104(boost::asio::io_service & io_service)
	:CProtocol(io_service)
{
	bActiveDataUp_ = true;
	SynCharNum_ = 6;
	QOI_ = 0x14;

	bCheckIFrameRecvCounter_ = true;
	bCheckIFrameSendCounter_ = false;
	bYcSendByTimer_ = false;

	S104_SUM_K_ = DEFAULT_SUM_K;
	S104_SUM_W_ = DEFAULT_SUM_W;

	InitObjectIndex();
	InitDefaultStartAddr();
	InitDefaultInfoNumLimit();
	InitDefaultFrameElem();
	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);

	EnablePubKey();

}

CS104::~CS104(void)
{
	S104ObjectCounter_--;
}

int CS104::ConnectSubYkSig(share_commpoint_ptr point)
{
	if (point)
	{
		if (point->getCommPointType() == PRISTATION_NODE)
		{
			return point->ConnectCmdRelaySig(boost::bind(&CS104::ProcessSubYkSig,this,_1,_2,_3,_4));
		}
	}

	return -1;
}

int CS104::DisconnectSubYkSig( share_commpoint_ptr point,bool bForceClose )
{
	if (point)
	{
		return point->DisconnectCmdRelaySig(bForceClose);
	}

	return -1;
}

void CS104::ProcessSubYkSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
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
				int index = boost::any_cast<int>(val);
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

int CS104::LoadXmlCfg(std::string filename)
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

		xml.ResetMainPos();
		if (xml.FindElem(strYcSendByTimer))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
				setTimeOutYcSend(timeout);
				bYcSendByTimer_ = true;
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				setTimeOutYcSend(DEFAULT_timeOutYcSendTime);
			}
		}

		xml.OutOfElem();
	}

	xml.ResetMainPos();
	if (xml.FindElem(strInfoNumLimit))
	{
		xml.IntoElem();

		xml.ResetMainPos();
		if (xml.FindElem(strInfoNumLimitAllYxFrame))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				INFONUM_LIMIT_ALLYXFRAME_ = boost::lexical_cast<typeInfoNumLimit>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				INFONUM_LIMIT_ALLYXFRAME_ = DEFAULT_INFONUM_LIMIT_ALLYXFRAME;
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strInfoNumLimitAllYcFrame))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				INFONUM_LIMIT_ALLYCFRAME_ = boost::lexical_cast<typeInfoNumLimit>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				INFONUM_LIMIT_ALLYCFRAME_ = DEFAULT_INFONUM_LIMIT_ALLYCFRAME;
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strInfoNumLimitAllDDFrame))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				INFONUM_LIMIT_ALLDDFRAME_ = boost::lexical_cast<typeInfoNumLimit>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				INFONUM_LIMIT_ALLDDFRAME_ = DEFAULT_INFONUM_LIMIT_ALLDDFRAME;
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strInfoNumLimitCosFrame))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				INFONUM_LIMIT_COSFRAME_ = boost::lexical_cast<typeInfoNumLimit>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				INFONUM_LIMIT_COSFRAME_ = DEFAULT_INFONUM_LIMIT_COSFRAME;
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strInfoNumLimitSoeFrame))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				INFONUM_LIMIT_SOEFRAME_ = boost::lexical_cast<typeInfoNumLimit>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				INFONUM_LIMIT_SOEFRAME_ = DEFAULT_INFONUM_LIMIT_SOEFRAME;
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strInfoNumLimitYcVarFrame))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				INFONUM_LIMIT_YCVARFRAME_ = boost::lexical_cast<typeInfoNumLimit>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();

				INFONUM_LIMIT_YCVARFRAME_ = DEFAULT_INFONUM_LIMIT_YCVARFRAME;
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
			S104_SUM_K_ = boost::lexical_cast<unsigned short>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();

			S104_SUM_K_ = DEFAULT_SUM_K;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strSUM_W))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			S104_SUM_W_ = boost::lexical_cast<unsigned short>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();

			S104_SUM_W_ = DEFAULT_SUM_K;
		}
	}

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CS104::SaveXmlCfg(std::string filename)
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

	if (timeOutYcSend_ != DEFAULT_timeOutYcSendTime)
	{
		xml.AddElem(strYcSendByTimer,timeOutYcSend_);
	}

	xml.OutOfElem();
	if (!bSave)
	{
		xml.RemoveElem();
	}

	xml.AddElem(strInfoNumLimit);
	bSave = false;
	xml.IntoElem();

	if (INFONUM_LIMIT_ALLYXFRAME_ != DEFAULT_INFONUM_LIMIT_ALLYXFRAME)
	{
		xml.AddElem(strInfoNumLimitAllYxFrame,INFONUM_LIMIT_ALLYXFRAME_);
		bSave = true;
	}

	if (INFONUM_LIMIT_ALLYCFRAME_ != DEFAULT_INFONUM_LIMIT_ALLYCFRAME)
	{
		xml.AddElem(strInfoNumLimitAllYcFrame,INFONUM_LIMIT_ALLYCFRAME_);
		bSave = true;
	}

	if (INFONUM_LIMIT_ALLDDFRAME_ != DEFAULT_INFONUM_LIMIT_ALLDDFRAME)
	{
		xml.AddElem(strInfoNumLimitAllDDFrame,INFONUM_LIMIT_ALLDDFRAME_);
		bSave = true;
	}

	if (INFONUM_LIMIT_COSFRAME_ != DEFAULT_INFONUM_LIMIT_COSFRAME)
	{
		xml.AddElem(strInfoNumLimitCosFrame,INFONUM_LIMIT_COSFRAME_);
		bSave = true;
	}

	if (INFONUM_LIMIT_SOEFRAME_ != DEFAULT_INFONUM_LIMIT_SOEFRAME)
	{
		xml.AddElem(strInfoNumLimitSoeFrame,INFONUM_LIMIT_SOEFRAME_);
		bSave = true;
	}

	if (INFONUM_LIMIT_YCVARFRAME_ != DEFAULT_INFONUM_LIMIT_YCVARFRAME)
	{
		xml.AddElem(strInfoNumLimitYcVarFrame,INFONUM_LIMIT_YCVARFRAME_);
		bSave = true;
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

	if (S104_SUM_K_ != DEFAULT_SUM_K)
	{
		xml.AddElem(strSUM_K,S104_SUM_K_);
	}

	if (S104_SUM_W_ != DEFAULT_SUM_W)
	{
		xml.AddElem(strSUM_W,S104_SUM_W_);
	}

	xml.OutOfElem();

	xml.Save(filename);
}

void CS104::InitObjectIndex()
{
	ProtocolObjectIndex_ = S104ObjectCounter_++;
}

int CS104::InitProtocol()
{
	ClearIFrameCounter();
	ClearCounterKI();
	ClearCounterKU();
	ClearCounterWI();

	CProtocol::InitProtocol();

	InitFrameLocation(SYN_HEAD_LENGTH);
	InitFrameLength();

	AddStatusLogWithSynT("S104规约的通道打开成功。\n");

	return 0;
}

void CS104::UninitProtocol()
{
	DisconnectSubAliveSig();

	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("S104规约的通道关闭成功。\n");
}

int CS104::MatchFrameHead( size_t & exceptedBytes )
{
	unsigned char tempChar;

	if (SynCharNum_ <= 0)
	{
		return 0;
	}

	size_t charLeft = getFrameBufLeft();
	while (charLeft >= SynCharNum_)
	{
		int addLength = 1;
		if ((int)charLeft > NormalYkFrameLength)
		{
			addLength = NormalYkFrameLength;
		}

		boost::scoped_array<unsigned char> temp_buf(new unsigned char[SynCharNum_ + addLength]);
		recv_buf_->copyBuf(temp_buf.get(),SynCharNum_ + addLength);
		int ret = CheckFrameHead(temp_buf.get(),exceptedBytes);
		if (!ret)
		{
			return 0;
		}
		else
		{
			recv_buf_->popChar(tempChar);
		}
		charLeft = getFrameBufLeft();
	}

	return NO_SYN_HEAD;
}

int CS104::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	if (buf[0] == 0x68)
	{
		int len = BufToVal(&buf[FrameLenLocation_],FrameLenLength_);
		if(len >= 4)
		{
			exceptedBytes = len + FrameLenLength_ + SYN_HEAD_LENGTH;

			if (CheckPubKey())
			{
				size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识

				if (FrameType == M_SC_NA_1 || FrameType == M_DC_NA_1)
				{
					exceptedBytes += CalcSecretDataLength(buf,exceptedBytes);
				}
			}

			return 0;
		}
	}
	else if (CheckPubKey())
	{
		return CheckPlusHead(buf,exceptedBytes);
	}

	return -1;
}

int CS104::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

//recv frame parse
int CS104::ParseFrame_S(unsigned char * buf)
{
	//std::cout<<"recv S gram"<<std::endl;

	int ret = 0;

	setLastRecvPointIndex(getLastSendPointIndex());

	unsigned short OppositeRecvCounter = (buf[4]&0xfe) + (buf[5]*0x100);
	if (CheckIFrameSendCounter(OppositeRecvCounter))
	{
		ReConnnectChannel();
		return -1;
	}

	ClearCounterKI();

	return ret;
}

int CS104::ParseFrame_U(unsigned char * buf)
{
	//std::cout<<"recv U gram"<<std::endl;

	int ret = 0;

	setLastRecvPointIndex(getLastSendPointIndex());

	ClearCounterKU();

	switch (buf[RecvCounterLocation_])
	{
	case 0x07:
		{
			share_commpoint_ptr nextPoint = getNextCommPoint(PRISTATION_NODE,boost::logic::indeterminate,getLastRecvPointIndex());
			if (nextPoint)
			{
				AddSendCmdVal(START_CONFIRM,START_CONFIRM_PRIORITY,nextPoint);
				ResetTimerHeartFrame(nextPoint,true);
			}
			//else
			//{
			//	std::cout<<"no active point"<<std::endl;
			//}
		}
		break;

	case 0x0b:
		ClearIFrameCounter();
		ClearCounterKI();
		ClearCounterKU();
		ClearCounterWI();
		break;

	case 0x13:
		{
			share_commpoint_ptr nextPoint = getNextCommPoint(PRISTATION_NODE,true,getLastRecvPointIndex());
			if (nextPoint)
			{
				AddSendCmdVal(STOP_CONFIRM,START_CONFIRM_PRIORITY,nextPoint);
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
			share_commpoint_ptr nextPoint = getNextCommPoint(PRISTATION_NODE,true,getLastRecvPointIndex());
			if (nextPoint)
			{
				AddSendCmdVal(TEST_CONFIRM,TEST_CONFIRM_PRIORITY,nextPoint);
			}
			//else
			//{
			//	std::cout<<"no active point"<<std::endl;
			//}
		}
		break;

	case 0x83:
		{
			share_commpoint_ptr nextPoint = getNextCommPoint(PRISTATION_NODE,true,getLastRecvPointIndex());
			ResetTimerHeartFrame(nextPoint,true);
		}
		break;

	default:
		//std::cout<<"undefined U gram type"<<std::endl;
		ret = -1;
		break;
	}

	return ret;
}

int CS104::ParseFrame_I(unsigned char * buf,size_t exceptedBytes)
{
	//std::cout<<"recv I gram"<<std::endl;

	unsigned short OppositeSendCounter = ~(~(BufToVal(&buf[RecvCounterLocation_],IGramCounterLength_)) | 0x01); //对端I格式报文发送计数器
	unsigned short OppositeRecvCounter = ~(~(BufToVal(&buf[SendCounterLocation_],IGramCounterLength_)) | 0x01); //对端I格式报文接收计数器

	std::cout<<OppositeSendCounter<<" "<<OppositeRecvCounter<<std::endl;

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

	IGramFrameRecv();
	ClearCounterKI();
	ResetTimerIGramFrameRecv(pristationPtr,true);

	if (CheckCounterWI())
	{
		AddSendCmdVal(S_GRAM_FRAME,S_GRAM_FRAME_PRIORITY,pristationPtr);
	}

	switch (FrameType)
	{
	case M_SC_NA_1: //single yk
		{
			if (CheckPubKey())
			{
				int len = BufToVal(&buf[FrameLenLocation_],FrameLenLength_) + FrameLenLength_ + SYN_HEAD_LENGTH;
				if (decrypt(getSignStartIndex(),exceptedBytes - getSignStartIndex(),len - getSignStartIndex(),buf))
				{
					std::ostringstream ostr;
					ostr<<"YK报文错误，未认证的数字签名"<<std::endl;
					AddStatusLogWithSynT(ostr.str());

					ParseYKKeyError(buf,pristationPtr,TransReason,Data_Code,false);

					return -1;
				}
			}

			switch (TransReason)
			{
			case trans_act:
				if (Data_Code == 0x80)
				{
					ParseSingleYKSel(buf,pristationPtr);
				}
				else if (!Data_Code)
				{
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
		}
		break;

	case M_DC_NA_1: //double yk
		{
			if (CheckPubKey())
			{
				int len = BufToVal(&buf[FrameLenLocation_],FrameLenLength_) + FrameLenLength_ + SYN_HEAD_LENGTH;
				if (decrypt(getSignStartIndex(),exceptedBytes - getSignStartIndex(),len - getSignStartIndex(),buf))
				{
					std::ostringstream ostr;
					ostr<<"YK报文错误，未认证的数字签名"<<std::endl;
					AddStatusLogWithSynT(ostr.str());

					ParseYKKeyError(buf,pristationPtr,TransReason,Data_Code,true);

					return -1;
				}
			}

			switch (TransReason)
			{
			case trans_act:
				if (Data_Code == 0x80)
				{
					ParseDoubleYkSel(buf,pristationPtr);
				}
				else if (!Data_Code)
				{
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
		}
		break;

	case M_IC_NA_1:
		if (TransReason = trans_act)
		{
			ParseCallAllData(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"总召唤报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_KY_NA_1://收到公钥
		if (TransReason = trans_act)
		{
			ParseTransPubKey(buf,pristationPtr,exceptedBytes);
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
			ParseSynTime(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"对时报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_RD_NA_1://102
		if (TransReason = trans_act)
		{
			ParseReadCMD(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"读命令报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_RP_NA_1:
		if (TransReason = trans_act)
		{
			ParseResetCMD(buf,pristationPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"复位进程报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
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

	return pristationIndex;
}

//遥控数字签名验证相关接口
int CS104::ParseYKKeyError(unsigned char * buf,share_pristation_ptr pristationPtr,size_t TransReason,unsigned char Data_Code,bool DoubleFlag)
{
	//在这里进行遥控加密异常的处理

	int yk_no = 0;

	if (DoubleFlag)
	{
		yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - DYK_START_ADDR_;
	} 
	else
	{
		yk_no = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_) - SYK_START_ADDR_;
	}

	unsigned char yk_type = buf[DataLocation_] & 0x03;

	DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_KEYERROR);

	if (yk_no < 0 || yk_no >= (int)pristationPtr->getYkSum())
	{
		std::ostringstream ostr;
		ostr<<"yk_no = "<<yk_no<<" 遥控点号不符合，退出不处理该帧报文。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	switch (TransReason)
	{
	case trans_act:
		if (Data_Code == 0x80)
		{
			if(/*pristationPtr->getSubTempSigConnection() ||*/ (pristationPtr->getYkPointPtr(yk_no))->RecvSelEvent())
			{

				AddStatusLogWithSynT("收到遥控选择报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
				return -1;
			}

			if(yk_type == SYK_TYPE_OPEN)
			{
				pristationPtr->setYkType(yk_no,DataBase::YkOpen);
			}
			else
			{
				pristationPtr->setYkType(yk_no,DataBase::YkClose);
			}
			AddSendCmdVal(YK_SEL_CON,YK_SEL_CON_PRIORITY,pristationPtr,ykPara);
		}
		else if (!Data_Code)
		{
			if(/*pristationPtr->getSubTempSigConnection() ||*/ (pristationPtr->getYkPointPtr(yk_no))->RecvExeEvent())
			{
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
			}
			else if (yk_type == SYK_TYPE_CLOSE)
			{
				if (pristationPtr->getYkType(yk_no) != DataBase::YkClose)
				{
					AddStatusLogWithSynT("遥控执行报文的遥控类型与预期不符。\n");
					return -1;
				}
			}
			else
			{
				AddStatusLogWithSynT("遥控执行报文的遥控类型与预期不符。\n");
				return -1;
			}

			AddSendCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,pristationPtr,ykPara);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"YK选择或执行报文错误，未定义的YK_CODE ="<<Data_Code<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case trans_deact:
		{
			if((pristationPtr->getYkPointPtr(yk_no))->RecvCancelEvent())
			{
				AddStatusLogWithSynT("收到遥控取消报文，但是当前遥控状态不符合，退出不处理该帧报文。\n");
				return -1;
			}
			AddSendCmdVal(YK_CANCEL_CON,YK_CANCEL_CON_PRIORITY,pristationPtr,ykPara);
		}
		break;
	}

	return 0;
}

int CS104::ParseSingleYKSel(unsigned char * buf,share_pristation_ptr pristationPtr)
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

	return 0;
}

int CS104::ParseSingleYKExe(unsigned char * buf,share_pristation_ptr pristationPtr)
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

	return 0;
}

int CS104::ParseSingleYKCancel(unsigned char * buf,share_pristation_ptr pristationPtr)
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

	return 0;
}

int CS104::ParseDoubleYkSel(unsigned char * buf,share_pristation_ptr pristationPtr)
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

	return 0;
}

int CS104::ParseDoubleYkExe(unsigned char * buf,share_pristation_ptr pristationPtr)
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

	return 0;
}

int CS104::ParseDoubleYkCancel(unsigned char * buf,share_pristation_ptr pristationPtr)
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

	return 0;
}

int CS104::ParseCallAllData(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	QOI_ = buf[DataLocation_];

	AddSendCmdVal(CALL_ALL_DATA_CON,CALL_ALL_DATA_CON_PRIORITY,pristationPtr);

	pristationPtr->CallAllData();

	return 0;
}

int CS104::ParseCallAllYMData(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	QOI_ = buf[DataLocation_];

	AddSendCmdVal(CALL_ALL_YM_CON,CALL_ALL_DD_CON_PRIORITY,pristationPtr);

	return 0;
}

int CS104::ParseSynTime(unsigned char * buf,share_pristation_ptr pristationPtr)
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

	//std::cout<<(short)Year<<"-"<<(short)Month<<"-"<<(short)Day<<" "<<(short)Hour<<":"<<(short)minute<<":"<<(short)(millisecond / 1000)<<std::endl;
	pristationPtr->WriteSysTime(Year,Month,Day,Hour,minute,(millisecond / 1000),(millisecond % 1000));

	AddSendCmdVal(SYN_TIME_CON,SYN_TIME_CON_PRIORITY,pristationPtr);

	pristationPtr->SynAllTime();

	return 0;
}

//读命令
int CS104::ParseReadCMD(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	int info_addr = BufToVal(&buf[InfoAddrLocation_],InfoAddrLength_);

	if ((info_addr >= (int)SYX_START_ADDR_ || info_addr >= (int)DYX_START_ADDR_) && info_addr < (int)YC_START_ADDR_)
	{
		int index = ((info_addr - SYX_START_ADDR_) < (info_addr - DYX_START_ADDR_)) ? (info_addr - SYX_START_ADDR_) : (info_addr - DYX_START_ADDR_);
		if (index >= 0 && index < (int)pristationPtr->getYxSum())
		{
			AddSendCmdVal(SINGLE_YX_DATA,SINGLE_YX_DATA_PRIORITY,pristationPtr,index);
		}
	}
	else if (info_addr >= (int)YC_START_ADDR_ && (info_addr < (int)SYK_START_ADDR_ || info_addr < (int)DYK_START_ADDR_))
	{
		int index = info_addr - YC_START_ADDR_;
		if(index >= 0 && index < (int)pristationPtr->getYcSum())
		{
			AddSendCmdVal(SINGLE_YC_DATA,SINGLE_YC_DATA_PRIORITY,pristationPtr,index);
		}
	}
	else if (info_addr >= (int)YM_START_ADDR_)
	{
		int index = info_addr - YM_START_ADDR_;
		if(index >= 0 && index < (int)pristationPtr->getYmSum())
		{
			AddSendCmdVal(SINGLE_YM_DATA,SINGLE_YM_DATA_PRIORITY,pristationPtr,index);
		}
	}

	return 0;
}

//复位进程
int CS104::ParseResetCMD(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	unsigned char QRP = buf[DataLocation_];

	AddSendCmdVal(RESET_CMD,RESET_CMD_PRIORITY,pristationPtr,QRP);

	ReConnnectChannel();

	return 0;
}

int CS104::ParseTransPubKey(unsigned char * buf,share_pristation_ptr pristationPtr,size_t exceptedBytes)
{
	size_t TotalNum_ = 0;
	size_t ThisNo_ = 0;
	size_t FileLocation = 16;
	size_t FileNumLocation = 13;//密钥文件总帧数所在位置
	size_t Filelen = exceptedBytes - FileLocation;

	QOI_ = buf[DataLocation_];
	TotalNum_ = buf[FileNumLocation++];
	ThisNo_ = buf[FileNumLocation++];

	std::ostringstream ostr;
	ostr<<"TotalNum_ is :"<<TotalNum_<<",ThisNo_ is :"<<ThisNo_<<std::endl;
	AddStatusLogWithSynT(ostr.str());


	if (ThisNo_ == 1)
	{
		AddStatusLogWithSynT("收到第一帧密钥传送报文！开始初始化文件处理指针... ...\n");
		FileHandleBegain(PubKeyName);
		AddStatusLogWithSynT("初始化文件处理指针成功... ...\n");
	} 

	if (TotalNum_ == ThisNo_)
	{
		AddStatusLogWithSynT("该帧为密钥最后一帧... ...\n");
		FileHandleOutFile(&buf[FileLocation],Filelen);
		AddStatusLogWithSynT("输出到缓冲区完成... ...\n");

		FileHandleWriteByByte();
		AddStatusLogWithSynT("生成文件完成... ...\n");

		FileHandleFinish();
		AddStatusLogWithSynT("关闭文件指针完成... ...\n");

		AddSendCmdVal(THRANS_PUKEY_CON,THRANS_PUKEY_CON_PRIORITY,pristationPtr);
	} 
	else
	{
		FileHandleOutFile(&buf[FileLocation],Filelen);
	}

	return 0;
}

int CS104::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	int ret = 0;

	ResetTimerAnyFrameRecv(true);
	share_commpoint_ptr nextPoint = getNextCommPoint(PRISTATION_NODE,boost::logic::indeterminate,getLastSendPointIndex());
	if (nextPoint)
	{
		ResetTimerHeartFrame(nextPoint);
	}
	
	if(buf[0] == 0x68)
	{
		if (exceptedBytes == (FrameLenLength_ + IGramCounterLength_ * 2 + SYN_HEAD_LENGTH))
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
			ret = ParseFrame_I(buf,exceptedBytes);
		}
	}
	else if((buf[0] == 0x16) && (buf[3] == 0x16) && (buf[1] >= buf[2]))
	{
		ret = ParseFrame_Plus(buf,exceptedBytes);
	}

	return ret;
}

//send frame assemble
int CS104::AssembleSGram(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x01;
	buf[count++] = 0;
	count += ValToBuf(&buf[count],IFrameRecvCounter_,IGramCounterLength_);

	return count - bufIndex;
}

int CS104::AssembleStartDTAct(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x07;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CS104::AssembleStartDTCon(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x0b;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CS104::AssembleStopDTAct(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x13;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CS104::AssembleStopDTCon(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x23;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CS104::AssembleTestFRAct(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x43;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CS104::AssembleTestFRCon(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0x83;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;

	return count - bufIndex;
}

int CS104::AssembleDoubleYKSelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CS104::AssembleDoubleYKExeCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CS104::AssembleDoubleYKCancelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code,unsigned char trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CS104::AssembleDoubleYKOver(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actterm,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CS104::AssembleSingleYKSelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CS104::AssembleSingleYKExeCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_reason,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CS104::AssembleSingleYKCancelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code,unsigned char trans_reason)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_deactcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CS104::AssembleSingleYKOver(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actterm,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}
int CS104::AssembleSynTimeCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,boost::posix_time::ptime time)
{
	//std::cout<<"before assemble:"<<boost::posix_time::to_simple_string(time)<<std::endl;

	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_CS_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	boost::posix_time::time_duration td = time.time_of_day();

	//std::cout<<"total_milliseconds = "<<td.total_milliseconds()<<std::endl;
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

int CS104::AssembleCallDataCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_IC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI_;

	return count - bufIndex;
}

int CS104::AssembleCallDataOver(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_IC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actterm,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI_;

	return count - bufIndex;
}

int CS104::AssembleAllSingleYX( size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num )
{
	size_t count = bufIndex;


	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYxSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYXFRAME_)
	{
		info_num = INFONUM_LIMIT_ALLYXFRAME_;
	}

	if (startIndex + info_num > pristationPtr->getYxSum())
	{
		info_num = pristationPtr->getYxSum() - startIndex;
	}

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

int CS104::AssembleAllDoubleYX( size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num )
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYxSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYXFRAME_)
	{
		info_num = INFONUM_LIMIT_ALLYXFRAME_;
	}

	if (startIndex + info_num > pristationPtr->getYxSum())
	{
		info_num = pristationPtr->getYxSum() - startIndex;
	}

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

int CS104::AssembleAllYCWithVaild( size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num )
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYcSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYCFRAME_)
	{
		info_num = INFONUM_LIMIT_ALLYCFRAME_;
	}

	if (startIndex + info_num > pristationPtr->getYcSum())
	{
		info_num = pristationPtr->getYcSum() - startIndex;
	}

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

int CS104::AssembleCallYMCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_CI_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI_;

	return count - bufIndex;
}

int CS104::AssembleCallYMOver(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_CI_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actterm,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI_;

	return count - bufIndex;
}

int CS104::AssembleAllYM( size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num )
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYmSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLDDFRAME_)
	{
		info_num = INFONUM_LIMIT_ALLDDFRAME_;
	}

	if (startIndex + info_num > pristationPtr->getYmSum())
	{
		info_num = pristationPtr->getYmSum() - startIndex;
	}

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

int CS104::AssembleSingleCOS(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CCosPoint * cosBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

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

int CS104::AssembleDoubleCOS(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CCosPoint * cosBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

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

int CS104::AssembleSingleSOE(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CSoePoint * soeBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

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

int CS104::AssembleDoubleSOE(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CSoePoint * soeBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

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

int CS104::AssembleYcVar(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CYcVarPoint * ycvarBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

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

int CS104::AssembleYcVarWithVaild(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CYcVarPoint * ycvarBuf,size_t info_num)
{
	size_t count = bufIndex;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],M_ME_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		typeFinalYcval ycVal = pristationPtr->getFinalYcVarVal(ycvarBuf[i]);

		count += ValToBuf(&buf[count],ycvarBuf[i].getYcIndex() + YC_START_ADDR_,InfoAddrLength_);
		count += ValToBuf(&buf[count],ycVal,2);
		count += ValToBuf(&buf[count],ycvarBuf[i].getYcQuality(),1);
	}

	return count - bufIndex;
}

int CS104::AssembleEndInit(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_EI_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);

	count += ValToBuf(&buf[count],Remote_Control,1);//reason 0

	return count - bufIndex;
}

int CS104::AssembleResetCMDCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,unsigned char QRP)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_RP_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QRP;

	return count - bufIndex;
}

int CS104::AssembleSingleSYxData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num)
{
	size_t count = bufIndex;


	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYxSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYXFRAME_)
	{
		info_num = INFONUM_LIMIT_ALLYXFRAME_;
	}

	if (startIndex + info_num > (int)pristationPtr->getYxSum())
	{
		info_num = pristationPtr->getYxSum() - startIndex;
	}

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

int CS104::AssembleSingleDYxData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num)
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYxSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYXFRAME_)
	{
		info_num = INFONUM_LIMIT_ALLYXFRAME_;
	}

	if (startIndex + info_num > (int)pristationPtr->getYxSum())
	{
		info_num = pristationPtr->getYxSum() - startIndex;
	}

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

int CS104::AssembleSingleYcData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num)
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYcSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLYCFRAME_)
	{
		info_num = INFONUM_LIMIT_ALLYCFRAME_;
	}

	if (startIndex + info_num > (int)pristationPtr->getYcSum())
	{
		info_num = pristationPtr->getYcSum() - startIndex;
	}

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

int CS104::AssembleSingleYmData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num)
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYmSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_ALLDDFRAME_)
	{
		info_num = INFONUM_LIMIT_ALLDDFRAME_;
	}

	if (startIndex + info_num > (int)pristationPtr->getYmSum())
	{
		info_num = pristationPtr->getYmSum() - startIndex;
	}

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

int CS104::AssembleThansPubKeyCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],M_KY_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],1,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_actcon,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI_;

	return count - bufIndex;
}

int CS104::AssembleSendYcByTimer(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num)
{
	size_t count = bufIndex;

	if (startIndex < 0 || startIndex >= (int)pristationPtr->getYcSum())
	{
		return -1;
	}

	if (info_num > INFONUM_LIMIT_YCVARFRAME_)
	{
		info_num = INFONUM_LIMIT_YCVARFRAME_;
	}

	if (startIndex + info_num > pristationPtr->getYcSum())
	{
		info_num = pristationPtr->getYcSum() - startIndex;
	}

	//std::cout<<"开始组装报文，收到的参数为info_num="<<info_num<<",startIndex="<<startIndex<<std::endl;

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],M_ME_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);
	for (size_t i=0;i<info_num;i++)
	{
		count += ValToBuf(&buf[count],startIndex + YC_START_ADDR_,InfoAddrLength_);
		count += ValToBuf(&buf[count],pristationPtr->getFinalYcVal(startIndex),2);
		count += ValToBuf(&buf[count],pristationPtr->getYcQuality(startIndex),1);
		startIndex++;
	}

	return count - bufIndex;
}

int CS104::AssembleSendYcIByTimer(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd cmd)
{
	size_t count = bufIndex;
	int info_num = boost::any_cast<int> (cmd.getVal()); 
	int startIndex = 0;

	if (info_num > INFONUM_LIMIT_YCVARFRAME_)
	{
		AddSendCmdVal(YCI_SEND_BYTIME,YCI_SEND_BYTIME_PRIORITY,pristationPtr,info_num - INFONUM_LIMIT_YCVARFRAME_);
		info_num = INFONUM_LIMIT_YCVARFRAME_;
	}

	info_num &= (~(0x80<<(InfoNumLength_ - 1) * 8));
	info_num |= EnableISQ<<((InfoNumLength_ - 1) * 8);

	count += ValToBuf(&buf[count],M_ME_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],info_num,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_spont,TransReasonLength_);
	count += ValToBuf(&buf[count],pristationPtr->getAddr(),AsduAddrLength_);

	for (size_t i=0;i<info_num;i++)
	{
		startIndex = pristationPtr->getTerminalYcINum(i);
		count += ValToBuf(&buf[count],startIndex + YC_START_ADDR_,InfoAddrLength_);
		count += ValToBuf(&buf[count],pristationPtr->getFinalYcVal(startIndex),2);
		count += ValToBuf(&buf[count],pristationPtr->getYcQuality(startIndex),1);
	}

	return count - bufIndex;
}

int CS104::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
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

	case CHECK_PUB_KEY:
		break;

	case UPDATE_PUB_KEY:
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

int CS104::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
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

	//ostrDebug<<this<<"命令参数通讯点指针转换OK！";
	//PrintDebug(ostrDebug.str());

	if (!pristationPtr)
	{
		std::ostringstream ostr;
		ostr<<"S104规约不能从发送命令中获得pristation ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
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
			//ResetTimerRequireLink(pristationPtr,true);
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case START_CONFIRM:
		bytesAssemble = AssembleStartDTCon(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			ClearIFrameCounter();
			ConnectSubAliveSig();
			AddSendCmdVal(END_INIT,END_INIT_PRIORITY,pristationPtr);
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
			ResetTimerHeartFrame(pristationPtr,true);
		}
		break;

	case TEST_CONFIRM:
		bytesAssemble = AssembleTestFRCon(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			ResetTimerHeartFrame(pristationPtr,true);
		}
		break;

	case S_GRAM_FRAME:
		bytesAssemble = AssembleSGram(bufIndex,buf);
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

			unsigned char yk_code;
			unsigned char trans_reason;
			bool bDouble = pristationPtr->getbSYkDouble((yk_para).YkNo_);

			if (CreateYkFramePara(false,bDouble,yk_type,yk_para.YkCode_,yk_code,trans_reason) == 0)
			{
				if (bDouble)
				{
					bytesAssemble = AssembleDoubleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,yk_code,trans_reason);
				}
				else
				{
					bytesAssemble = AssembleSingleYKSelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,yk_code,trans_reason);
				}
			}
			else
			{
				AddStatusLogWithSynT("遥控选择返校命令的遥控类型参数非法。\n");
				return -1;
			}
			
			if (bytesAssemble > 0)
			{
				if (yk_para.YkCode_ != RETURN_CODE_KEYERROR)
				{

					if ((pristationPtr->getYkPointPtr(yk_para.YkNo_))->BackSelEvent())
					{
						AddStatusLogWithSynT("解析遥控选择返校命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
						return -1;
					}

					ResetTimerYkSelToExe(pristationPtr,yk_para.YkNo_,true);

				}
				else
				{
					(pristationPtr->getYkPointPtr(yk_para.YkNo_))->ClearYkState();
				}

				IGramFrameSend();
				ResetTimerYkSel(pristationPtr,(yk_para).YkNo_,false);
				DisconnectSubYkSig(pristationPtr,false);
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

			unsigned char yk_code;
			unsigned char trans_reason;
			bool bDouble = pristationPtr->getbSYkDouble((yk_para).YkNo_);

			if (CreateYkFramePara(false,bDouble,yk_type,yk_para.YkCode_,yk_code,trans_reason) == 0)
			{
				if (bDouble)
				{
					bytesAssemble = AssembleDoubleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,yk_code,trans_reason);
				}
				else
				{
					bytesAssemble = AssembleSingleYKExeCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,yk_code,trans_reason);
				}
			}
			else
			{
				AddStatusLogWithSynT("遥控执行返校命令的遥控类型参数非法。\n");
				return -1;
			}

			if (bytesAssemble > 0)
			{
				if (yk_para.YkCode_ != RETURN_CODE_KEYERROR)
				{
					if((pristationPtr->getYkPointPtr(yk_para.YkNo_))->BackExeEvent())
					{
						AddStatusLogWithSynT("解析遥控执行返校命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
						return -1;
					}

					AddSendCmdVal(YK_OVER_CON,YK_OVER_PRIORITY,pristationPtr,(int)(yk_para).YkNo_);
				}
				else
				{
					(pristationPtr->getYkPointPtr(yk_para.YkNo_))->ClearYkState();
				}

				IGramFrameSend();
				ResetTimerYkExe(pristationPtr,(yk_para).YkNo_,false);
				DisconnectSubYkSig(pristationPtr,false);
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

			unsigned char yk_code;
			unsigned char trans_reason;
			bool bDouble = pristationPtr->getbSYkDouble((yk_para).YkNo_);

			if (CreateYkFramePara(true,bDouble,yk_type,yk_para.YkCode_,yk_code,trans_reason) == 0)
			{
				if (bDouble)
				{
					bytesAssemble = AssembleDoubleYKCancelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,yk_code,trans_reason);
				}
				else
				{
					bytesAssemble = AssembleSingleYKCancelCon(bufIndex,buf,pristationPtr,(yk_para).YkNo_,yk_code,trans_reason);
				}
			}
			else
			{
				AddStatusLogWithSynT("遥控取消返校命令的遥控类型参数非法。\n");
				return -1;
			}
			
			if (bytesAssemble > 0)
			{
				if (yk_para.YkCode_ != RETURN_CODE_KEYERROR)
				{
					if((pristationPtr->getYkPointPtr(yk_para.YkNo_))->BackCancelEvent())
					{
						AddStatusLogWithSynT("解析遥控取消返校命令，但是遥控当前遥控状态不符合，退出不发送该命令。\n");
						return -1;
					}
				}
				else
				{
					(pristationPtr->getYkPointPtr(yk_para.YkNo_))->ClearYkState();
				}

				IGramFrameSend();
				ResetTimerYkCancel(pristationPtr,(yk_para).YkNo_,false);
				DisconnectSubYkSig(pristationPtr,false);
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

				IGramFrameSend();
				//DisconnectSubYkSig(pristationPtr,false);
				//pristationPtr->setYkStatus(yk_no,DataBase::YkOver);
			}
		}
		break;

	case SYN_TIME_CON:
		bytesAssemble = AssembleSynTimeCon(bufIndex,buf,pristationPtr,boost::posix_time::microsec_clock::local_time());
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
		}
		break;

	case THRANS_PUKEY_CON:
		bytesAssemble = AssembleThansPubKeyCon(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
		}
		break;


	case CALL_ALL_DATA_CON:
		bytesAssemble = AssembleCallDataCon(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
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
		}
		break;

	case CALL_ALL_DATA_OVER:
		bytesAssemble = AssembleCallDataOver(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			//AddSendCmdVal(COS_DATA_UP,COS_DATA_UP_PRIORITY,pristationPtr);
			//AddSendCmdVal(SOE_DATA_UP,SOE_DATA_UP_PRIORITY,pristationPtr);
			//AddSendCmdVal(YCVAR_DATA_UP,YCVAR_DATA_UP_PRIORITY,pristationPtr);
			if (bYcSendByTimer_)
			{
				ResetTimerYcSend(pristationPtr,pristationPtr->getYcSum(),0,true,5);
			}
			IGramFrameSend();
		}
		break;

	case CALL_ALL_YM_CON:
		bytesAssemble = AssembleCallYMCon(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
			if (pristationPtr->getYmSum() > 0)
			{
				AddSendCmdVal(ALL_YM_DATA,ALL_YM_DATA_PRIORITY,pristationPtr,(int)0);
			}
			else
			{
				AddSendCmdVal(CALL_ALL_YM_OVER,CALL_ALL_YM_OVER_PRIORITY,pristationPtr);
			}
		}
		break;

	case CALL_ALL_YM_OVER:
		bytesAssemble = AssembleCallYMOver(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
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
					if (pristationPtr->getYxType(i) != yxType || count >= INFONUM_LIMIT_ALLYXFRAME_)
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
					IGramFrameSend();
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
				if (leftNum > INFONUM_LIMIT_ALLYCFRAME_)
				{
					bytesAssemble = AssembleAllYCWithVaild(bufIndex,buf,pristationPtr,startIndex,INFONUM_LIMIT_ALLYCFRAME_);
				}
				else
				{
					bytesAssemble = AssembleAllYCWithVaild(bufIndex,buf,pristationPtr,startIndex,leftNum);
				}

				if (bytesAssemble > 0)
				{
					IGramFrameSend();
					if (startIndex < (int)pristationPtr->getYcSum())
					{
						AddSendCmdVal(ALL_YC_DATA,ALL_YC_DATA_PRIORITY,pristationPtr,startIndex);
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
				if (leftNum > INFONUM_LIMIT_ALLDDFRAME_)
				{
					bytesAssemble = AssembleAllYM(bufIndex,buf,pristationPtr,startIndex,INFONUM_LIMIT_ALLDDFRAME_);
				}
				else
				{
					bytesAssemble = AssembleAllYM(bufIndex,buf,pristationPtr,startIndex,leftNum);
				}

				if (bytesAssemble > 0)
				{
					IGramFrameSend();
					if (startIndex < (int)pristationPtr->getYmSum())
					{
						AddSendCmdVal(ALL_YM_DATA,ALL_YM_DATA_PRIORITY,pristationPtr,startIndex);
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
				boost::scoped_array<DataBase::CCosPoint> temp_buf(new DataBase::CCosPoint[INFONUM_LIMIT_COSFRAME_ + 1]);
				typeYxtype yxType = firstCos.getYxType();
				int leftPointNum = pristationPtr->getCosPointNum();
				while (CountAssemblePoint < INFONUM_LIMIT_COSFRAME_ && leftPointNum > 0 && yxType == firstCos.getYxType())
				{
					CountAssemblePoint += pristationPtr->loadCosPoints(&(temp_buf.get())[CountAssemblePoint],(leftPointNum <= (INFONUM_LIMIT_COSFRAME_ - CountAssemblePoint) ? leftPointNum : (INFONUM_LIMIT_COSFRAME_ - CountAssemblePoint)),yxType);
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
						IGramFrameSend();
						if (pristationPtr->getCosPointNum() > 0)
						{
							//AddSendCmdVal(COS_DATA_UP,COS_DATA_UP_PRIORITY,pristationPtr);
							AddOnlySendCmdByCmdType(COS_DATA_UP,COS_DATA_UP_PRIORITY,pristationPtr,boost::any());
						}
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
				boost::scoped_array<DataBase::CSoePoint> temp_buf(new DataBase::CSoePoint[INFONUM_LIMIT_SOEFRAME_ + 1]);
				typeYxtype yxType = firstSoe.getYxType();
				int leftPointNum = pristationPtr->getSoePointNum();
				while (CountAssemblePoint < INFONUM_LIMIT_SOEFRAME_ && leftPointNum > 0 && yxType == firstSoe.getYxType())
				{
					CountAssemblePoint += pristationPtr->loadSoePoints(&(temp_buf.get())[CountAssemblePoint],(leftPointNum <= (INFONUM_LIMIT_SOEFRAME_ - CountAssemblePoint) ? leftPointNum : (INFONUM_LIMIT_SOEFRAME_ - CountAssemblePoint)),yxType);
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
						IGramFrameSend();
						if (pristationPtr->getSoePointNum() > 0)
						{
							//AddSendCmdVal(SOE_DATA_UP,SOE_DATA_UP_PRIORITY,pristationPtr);
							AddOnlySendCmdByCmdType(SOE_DATA_UP,SOE_DATA_UP_PRIORITY,pristationPtr,boost::any());
						}
					}
				}
			}
		}
		break;

	case YCVAR_DATA_UP:
		{
			//ostrDebug.str("");
			//ostrDebug<<this<<"变化遥测开始！";
			//PrintDebug(ostrDebug.str());

			int leftPointNum = pristationPtr->getYcvarPointNum();
			if (leftPointNum > 0)
			{
				//ostrDebug.str("");
				//ostrDebug<<this<<"变化遥测数量"<<leftPointNum;
				//PrintDebug(ostrDebug.str());

				int CountAssemblePoint = 0;
				boost::scoped_array<DataBase::CYcVarPoint> temp_buf(new DataBase::CYcVarPoint[INFONUM_LIMIT_YCVARFRAME_ + 1]);
				while (CountAssemblePoint < INFONUM_LIMIT_YCVARFRAME_ && leftPointNum > 0)
				{
					//ostrDebug.str("");
					//ostrDebug<<this<<"统计变化遥测数量 已经统计数量="<<CountAssemblePoint<<"剩余数量="<<leftPointNum;
					//PrintDebug(ostrDebug.str());

					CountAssemblePoint += pristationPtr->loadYcvarPoints(&(temp_buf.get())[CountAssemblePoint],(leftPointNum <= (INFONUM_LIMIT_YCVARFRAME_ - CountAssemblePoint) ? leftPointNum : (INFONUM_LIMIT_YCVARFRAME_ - CountAssemblePoint)));
					leftPointNum = pristationPtr->getYcvarPointNum();
				}

				if (CountAssemblePoint > 0)
				{
					bytesAssemble = AssembleYcVarWithVaild(bufIndex,buf,pristationPtr,temp_buf.get(),CountAssemblePoint);
					if (bytesAssemble > 0)
					{
						IGramFrameSend();
						if (pristationPtr->getYcvarPointNum() > 0)
						{
							//AddSendCmdVal(YCVAR_DATA_UP,YCVAR_DATA_UP_PRIORITY,pristationPtr);
							AddOnlySendCmdByCmdType(YCVAR_DATA_UP,YCVAR_DATA_UP_PRIORITY,pristationPtr,boost::any());
						}
					}
				}
			}
		}
		break;

	case YC_SEND_BYTIME:
		{
			//std::cout<<"收到定时上送遥测指令！"<<std::endl;
			int startIndex;
			try
			{
				startIndex = boost::any_cast<int>(cmd.getVal());
				//std::cout<<"startIndex的值为："<<startIndex<<std::endl;
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"全YC命令的起始YC点号参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				//std::cout<<"开始继续初始化定时器！"<<std::endl;
				ResetTimerYcSend(pristationPtr,pristationPtr->getYcSum(),0,true);

				return -1;
			}

			int leftNum = pristationPtr->getYcSum() - startIndex;
			if (leftNum > 0)
			{
				if (leftNum > INFONUM_LIMIT_YCVARFRAME_)
				{
					//std::cout<<"满足条件leftNum > INFONUM_LIMIT_ALLYCFRAME！，传入参数为："<<INFONUM_LIMIT_YCVARFRAME<<"及"<<startIndex<<std::endl;
					bytesAssemble = AssembleSendYcByTimer(bufIndex,buf,pristationPtr,startIndex,INFONUM_LIMIT_YCVARFRAME_);
				}
				else
				{
					//std::cout<<"不满足条件leftNum > INFONUM_LIMIT_ALLYCFRAME！传入参数为："<<INFONUM_LIMIT_YCVARFRAME<<startIndex<<std::endl;
					bytesAssemble = AssembleSendYcByTimer(bufIndex,buf,pristationPtr,startIndex,leftNum);
				}

				if (bytesAssemble > 0)
				{
					IGramFrameSend();
					//std::cout<<"开始继续初始化定时器！startIndex值为"<<startIndex<<std::endl;
					ResetTimerYcSend(pristationPtr,pristationPtr->getYcSum(),startIndex,true);
					//AddSendCmdVal(YC_SEND_BYTIME,YC_SEND_BYTIME_PRIORITY,pristationPtr,startIndex);//修改为2s全部上送一次而不是2s循环上送
				}
			}
			else
			{
				//std::cout<<"开始继续初始化定时器！"<<std::endl;
				ResetTimerYcSend(pristationPtr,pristationPtr->getYcSum(),0,true);
			}
		}
		break;

	case END_INIT:
		bytesAssemble = AssembleEndInit(bufIndex,buf,pristationPtr);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
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

			bytesAssemble = AssembleResetCMDCon(bufIndex,buf,pristationPtr,QRP);
			if (bytesAssemble > 0)
			{
				IGramFrameSend();
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
					IGramFrameSend();
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
					IGramFrameSend();
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
					IGramFrameSend();
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

	case CHECK_PUB_KEY:
		try
		{
			bool bConAct = boost::any_cast<bool>(cmd.getVal());
			bytesAssemble = AssembleCheckPubKeyCon(bufIndex,buf,pristationPtr,bConAct);
		}
		catch(const boost::bad_any_cast & e)
		{
			std::ostringstream ostr;
			ostr<<"检查公钥回应命令的参数非法："<<e.what()<<std::endl;
			AddStatusLogWithSynT(ostr.str());
			return -1;
		}
		break;

	case UPDATE_PUB_KEY:
		try
		{
			bool bConAct = boost::any_cast<bool>(cmd.getVal());
			bytesAssemble = AssembleUpdatePubKeyCon(bufIndex,buf,pristationPtr,bConAct);
		}
		catch(const boost::bad_any_cast & e)
		{
			std::ostringstream ostr;
			ostr<<"更新公钥回应命令的参数非法："<<e.what()<<std::endl;
			AddStatusLogWithSynT(ostr.str());
			return -1;
		}
		break;

	case YCI_SEND_BYTIME:
		bytesAssemble = AssembleSendYcIByTimer(bufIndex,buf,pristationPtr,cmd);
		if (bytesAssemble > 0)
		{
			IGramFrameSend();
		}
		break;

	default:
		{
			std::ostringstream ostr; 
			ostr<<"未定义的命令类型 cmdType ="<<cmd.getCmdType()<<"，丢弃该命令。"<<std::endl;
			AddStatusLogWithSynT(ostr.str());
			return -1;
		}
		break;
	}

	return bytesAssemble;
}

int CS104::AssembleFrameTail( size_t bufBegin,size_t bufIndex, unsigned char * buf, CCmd & cmd )
{
	size_t count = bufIndex;

	if(!((cmd.getCmdType() == CHECK_PUB_KEY) || (cmd.getCmdType() == UPDATE_PUB_KEY)))
	{
		int length = count - bufBegin - (FrameLenLength_ + SYN_HEAD_LENGTH);
		if (length <= 0 || length > max_frame_length_)
		{
			return -1;
		}

		ValToBuf(&buf[FrameLenLocation_],length,FrameLenLength_);
	}

	return count - bufIndex;
}

void CS104::IGramFrameRecv()
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

void CS104::IGramFrameSend()
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

void CS104::UGramFrameSend()
{
	count_K_U_++;
}

bool CS104::CheckIFrameRecvCounter(unsigned short sendVal)
{
	if(bCheckIFrameRecvCounter_)
	{
		if (IFrameRecvCounter_ == sendVal)
		{
			return false;
		}

		std::ostringstream ostr;
		ostr<<"S104规约的I格式报文接收计数器 = "<<IFrameRecvCounter_<<",接收报文中对端的发送计数器 = "<<sendVal<<": 计数器错误！"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return true;
	}

	return false;
}

bool CS104::CheckIFrameSendCounter(unsigned short recvVal)
{
	if(bCheckIFrameSendCounter_) //默认把报文计数器的判断条件放松，自己的发送和接收计数器都作判断，但是自己的发送接收计数器出错不重连通道，交给对端的接收去判断。
	{
		if (IFrameSendCounter_ >= recvVal)
		{
			return false;
		}

		std::ostringstream ostr;
		ostr<<"S104规约的I格式报文发送计数器 = "<<IFrameRecvCounter_<<",接收报文中对端的接收计数器 = "<<recvVal<<": 计数器错误！"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return true;
	}

	return false;
}

void CS104::ClearIFrameCounter()
{
	IFrameRecvCounter_ = 0;
	IFrameSendCounter_ = 0;
}

bool CS104::CheckCounterKU()
{
	if(count_K_U_ > S104_SUM_K_)
	{
		std::ostringstream ostr;
		ostr<<count_K_U_<<"帧U格式报文发送未收到回应。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		ClearCounterKU();

		return true;
	}

	return false;
}

void CS104::ClearCounterKU()
{
	count_K_U_ = 0;
}

bool CS104::CheckCounterKI()
{
	if(count_K_I_ > S104_SUM_K_)
	{
		std::ostringstream ostr;
		ostr<<count_K_I_<<"帧I格式报文发送未收到回应。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		ClearCounterKI();
		return true;
	}

	return false;
}

void CS104::ClearCounterKI()
{
	count_K_I_ = 0;
}

bool CS104::CheckCounterWI()
{
	if(count_W_I_ > S104_SUM_W_)
	{
		ClearCounterWI();

		return true;
	}

	return false;
}

void CS104::ClearCounterWI()
{
	count_W_I_ = 0;
}

void CS104::InitDefaultStartAddr()
{
	SYX_START_ADDR_ = DEFAULT_SYX_START_ADDR;                              //单点yx起始地址
	DYX_START_ADDR_ = DEFAULT_DYX_START_ADDR;                              //双点yx起始地址
	YC_START_ADDR_ =  DEFAULT_YC_START_ADDR;                               //yc起始地址
	SYK_START_ADDR_ = DEFAULT_SYK_START_ADDR;                              //单点yk起始地址
	DYK_START_ADDR_ = DEFAULT_DYK_START_ADDR;                              //双点yk起始地址
	YM_START_ADDR_ =  DEFAULT_YM_START_ADDR;                               //ym起始地址
	HIS_START_ADDR_ = DEFAULT_HIS_START_ADDR;                              //历史数据起始地址
}

void CS104::InitDefaultFrameElem()
{
	FrameLenLength_ = DEFAULT_FrameLenLength;                               //报文长度标识的字节长度
	IGramCounterLength_ = DEFAULT_IGramCounterLength;
	FrameTypeLength_ = DEFAULT_FrameTypeLength;                             //报文类型标识的字节长度
	InfoNumLength_ =   DEFAULT_InfoNumLength;                               //信息体数目标识的字节长度
	TransReasonLength_ = DEFAULT_TransReasonLength;                         //传送原因标识的字节长度
	AsduAddrLength_ = DEFAULT_AsduAddrLength;                               //装置地址标识的字节长度
	InfoAddrLength_ = DEFAULT_InfoAddrLength;                               //信息体地址标识的字节长度
}

void CS104::InitDefaultInfoNumLimit()
{
	INFONUM_LIMIT_ALLYXFRAME_ = DEFAULT_INFONUM_LIMIT_ALLYXFRAME;
	INFONUM_LIMIT_ALLYCFRAME_ = DEFAULT_INFONUM_LIMIT_ALLYCFRAME;
	INFONUM_LIMIT_ALLDDFRAME_ = DEFAULT_INFONUM_LIMIT_ALLDDFRAME;
	INFONUM_LIMIT_COSFRAME_ = DEFAULT_INFONUM_LIMIT_COSFRAME;
	INFONUM_LIMIT_SOEFRAME_ = DEFAULT_INFONUM_LIMIT_SOEFRAME;
	INFONUM_LIMIT_YCVARFRAME_ = DEFAULT_INFONUM_LIMIT_YCVARFRAME;
}

void CS104::InitFrameLocation(size_t frameHead)
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

void CS104::InitFrameLength()
{
	if (FrameLenLength_ > 1)
	{
		//if (InfoNumLength_ > 1)
		//{
		//	max_frame_length_ = 0x7fff;
		//	send_buf_.reset(new unsigned char[max_frame_length_]);
		//	recv_buf_.reset(new PublicSupport::CLoopBuf(max_frame_length_));
		//}
		//else
		//{
		max_frame_length_ = MAX_IP_MTU;
		send_buf_.reset(new unsigned char[max_frame_length_]);
		recv_buf_.reset(new PublicSupport::CLoopBuf(max_frame_length_ * 4));
		//}
	}
}

void CS104::InitDefaultTimeOut()
{
	timeOutAnyFrameRecv_ = DEFAULT_timeOutAnyFrameRecv;
	timeOutIGramFrameRecv_ = DEFAULT_timeOutIGramFrameRecv;
	timeOutHeartFrame_ = DEFAULT_timeOutHeartFrame;
	timeOutYkSel_ = DEFAULT_timeOutYkSel;
	timeOutYkExe_ = DEFAULT_timeOutYkExe;
	timeOutYkCancel_ = DEFAULT_timeOutYkCancel;
	timeOutCallPara_ = DEFAULT_timeOutCallPara;
	timeOutSetPara_ = DEFAULT_timeOutSetPara;
	timeOutYkSelToExe_ = DEFAULT_timeOutYkSelToExe;
	timeOutYcSend_     = DEFAULT_timeOutYcSendTime;
}

void CS104::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	timerAnyFrameRecv_.reset(new deadline_timer(io_service,seconds(timeOutAnyFrameRecv_)));
	AddTimer(timerAnyFrameRecv_);

	timerIGramFrameRecv_.reset(new deadline_timer(io_service,seconds(timeOutIGramFrameRecv_)));
	AddTimer(timerIGramFrameRecv_);

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

	timerYkSelToExe_.reset(new deadline_timer(io_service,seconds(timeOutYkSelToExe_)));
	AddTimer(timerYkSelToExe_);

	timerYcSend_.reset(new deadline_timer(io_service,seconds(timeOutYcSend_)));
	AddTimer(timerYcSend_);
}

void CS104::handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(PRISTATION_NODE,true,point);
		if (val)
		{
			AddSendCmdVal(TEST_ACT,TEST_ACT_PRIORITY,point);
		}
	}
}

void CS104::handle_timerAnyFrameRecv(const boost::system::error_code& error)
{
	if (!error)
	{
		ReConnnectChannel();
	}
}

void CS104::handle_timerIGramFrameRecv(const boost::system::error_code &error,share_commpoint_ptr point)
{
	if (!error)
	{
		ClearCounterWI();
		AddSendCmdVal(S_GRAM_FRAME,S_GRAM_FRAME_PRIORITY,point);
	}
}

void CS104::handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
{
	if (!error)
	{
		if (point)
		{
			AddStatusLogWithSynT("S104规约遥控选择命令超时。\n");

			if (point->getCommPointType() == PRISTATION_NODE)
			{
				share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(point);
				if (pristationPtr)
				{
					//DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_TIMEOUT);
					//AddSendCmdVal(YK_SEL_CON,YK_SEL_CON_PRIORITY,point,ykPara);
					//AddStatusLogWithSynT("返回遥控选择否定确认报文。\n");

					//pristationPtr->setYkStatus(yk_no,DataBase::YkSelTimeOut);
					(pristationPtr->getYkPointPtr(yk_no))->TimeOutEvent();
					DisconnectSubYkSig(pristationPtr,true);
				}
			}



		}
	}
}

void CS104::handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
{
	if (!error)
	{
		if (point)
		{
			AddStatusLogWithSynT("S104规约遥控执行命令超时。\n");

			if (point->getCommPointType() == PRISTATION_NODE)
			{
				share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(point);
				if (pristationPtr)
				{
					//DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_TIMEOUT);
					//AddSendCmdVal(YK_EXE_CON,YK_EXE_CON_PRIORITY,point,ykPara);
					//AddStatusLogWithSynT("返回遥控执行否定确认报文。\n");

					//pristationPtr->setYkStatus(yk_no,DataBase::YkSelTimeOut);
					(pristationPtr->getYkPointPtr(yk_no))->TimeOutEvent();
					DisconnectSubYkSig(pristationPtr,true);
				}
			}			
		}
	}
}

void CS104::handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
{
	if (!error)
	{
		if (point)
		{
			AddStatusLogWithSynT("S104规约遥控取消命令超时。\n");

			if (point->getCommPointType() == PRISTATION_NODE)
			{
				share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(point);
				if (pristationPtr)
				{
					//DataBase::stYkCmdPara ykPara(yk_no,RETURN_CODE_TIMEOUT);
					//AddSendCmdVal(YK_CANCEL_CON,YK_CANCEL_CON_PRIORITY,point,ykPara);
					//AddStatusLogWithSynT("返回遥控取消否定确认报文。\n");

					//pristationPtr->setYkStatus(yk_no,DataBase::YkSelTimeOut);
					(pristationPtr->getYkPointPtr(yk_no))->TimeOutEvent();
					DisconnectSubYkSig(pristationPtr,true);
				}
			}			
		}
	}
}

void CS104::handle_timerYkSelToExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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

void CS104::handle_timerYcSend(const boost::system::error_code& error,share_commpoint_ptr point,int startIndex)
{
	if (!error)
	{
		if (point)
		{
			//std::cout<<"定时器时间到，开始添加指令！"<<std::endl;
			if (bYcSendByTimer_)
			{
				AddSendCmdVal(YC_SEND_BYTIME,YC_SEND_BYTIME_PRIORITY,point,startIndex);
			}
		}
	}
}

void CS104::handle_timerCallPara(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		if (point)
		{
			CmdConSig_(CALL_EXTEND_PARA,RETURN_CODE_TIMEOUT,point,0);
		}
	}
}

void CS104::handle_timerSetPara(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		if (point)
		{
			CmdConSig_(SET_EXTEND_PARA,RETURN_CODE_TIMEOUT,point,0);
		}
	}
}

void CS104::ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
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

		timerHeartFrame_->async_wait(boost::bind(&CS104::handle_timerHeartFrame,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerHeartFrame_->cancel();
	}
}

void CS104::ResetTimerAnyFrameRecv( bool bContinue /*= true*/,unsigned short val /*= 0*/ )
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

		timerAnyFrameRecv_->async_wait(boost::bind(&CS104::handle_timerAnyFrameRecv,this,boost::asio::placeholders::error));
	}
	else
	{
		timerAnyFrameRecv_->cancel();
	}
}

void CS104::ResetTimerIGramFrameRecv(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
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

		timerIGramFrameRecv_->async_wait(boost::bind(&CS104::handle_timerIGramFrameRecv,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerIGramFrameRecv_->cancel();
	}
}

void CS104::ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkSel_->async_wait(boost::bind(&CS104::handle_timerYkSel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkSel_->cancel();
	}
}

void CS104::ResetTimerYkExe(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkExe_->async_wait(boost::bind(&CS104::handle_timerYkExe,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkExe_->cancel();
	}
}

void CS104::ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkCancel_->async_wait(boost::bind(&CS104::handle_timerYkCancel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkCancel_->cancel();
	}
}

void CS104::ResetTimerCallPara(share_commpoint_ptr point,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerCallPara_->async_wait(boost::bind(&CS104::handle_timerCallPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallPara_->cancel();
	}
}

void CS104::ResetTimerSetPara(share_commpoint_ptr point,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerSetPara_->async_wait(boost::bind(&CS104::handle_timerSetPara,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerSetPara_->cancel();
	}
}

void CS104::ResetTimerYkSelToExe(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkSelToExe_->async_wait(boost::bind(&CS104::handle_timerYkSelToExe,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkSelToExe_->cancel();
	}
}

void CS104::ResetTimerYcSend(share_commpoint_ptr point,int YcSum,int startIndex,bool bContinue /*= false*/,unsigned short val/* = 0*/)
{
	//std::cout<<"开始初始化定时器！"<<std::endl;
	if (bContinue)
	{
		if (val == 0)
		{
			int sendcount_,timeVal;
			if (YcSum%INFONUM_LIMIT_YCVARFRAME_ == 0)
			{
				sendcount_ = YcSum/INFONUM_LIMIT_YCVARFRAME_;
			} 
			else
			{
				sendcount_ = YcSum/INFONUM_LIMIT_YCVARFRAME_ + 1;
			}

			timeVal = (timeOutYcSend_ * 1000)/sendcount_;

			timerYcSend_->expires_from_now(boost::posix_time::millisec(timeVal));
		}
		else
		{
			timerYcSend_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerYcSend_->async_wait(boost::bind(&CS104::handle_timerYcSend,this,boost::asio::placeholders::error,point,startIndex));
	}
	else
	{
		timerYcSend_->cancel();
	}
}

//para api
int CS104::setSYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	SYX_START_ADDR_ = val;

	return 0;
}

int CS104::setDYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	DYX_START_ADDR_ = val;

	return 0;
}

int CS104::setYC_START_ADDR(size_t val)
{
	if (val < 0x101 || val >= 0x6001)
	{
		return -1;
	}

	YC_START_ADDR_ = val;

	return 0;
}

int CS104::setSYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	SYK_START_ADDR_ = val;

	return 0;
}

int CS104::setDYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	DYK_START_ADDR_ = val;

	return 0;
}

int CS104::setYM_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	YM_START_ADDR_ = val;

	return 0;
}

int CS104::setHIS_START_ADDR(size_t val)
{
	HIS_START_ADDR_ = val;

	return 0;
}

int CS104::setFrameLenLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameLenLength_ = val;

	return 0;
}

int CS104::setIGramCounterLength(unsigned short val)
{
	if (val < 0 || val > 4)
	{
		return -1;
	}

	IGramCounterLength_ = val;

	return 0;
}

int CS104::setFrameTypeLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameTypeLength_ = val;

	return 0;
}

int CS104::setInfoNumLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	InfoNumLength_ = val;

	return 0;
}

int CS104::setTransReasonLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	TransReasonLength_ = val;

	return 0;
}

int CS104::setAsduAddrLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	AsduAddrLength_ = val;

	return 0;
}

int CS104::setInfoAddrLength(unsigned short val)
{
	if (val <= 0 || val > 12)
	{
		return -1;
	}

	InfoAddrLength_ = val;

	return 0;
}

int CS104::setTimeOutAnyFrameRecv(unsigned short val)
{
	if (val < 60 || val > 300)
	{
		return -1;
	}

	timeOutAnyFrameRecv_ = val;

	return 0;
}

int CS104::setTimeOutIGramFrameRecv(unsigned short val)
{
	if (val < 1 || val > 60)
	{
		return -1;
	}

	timeOutIGramFrameRecv_ = val;

	return 0;
}

int CS104::setTimeOutHeartFrame(unsigned short val)
{
	if (val <= 0 || val > 60) 
	{
		return -1;
	}

	timeOutHeartFrame_ = val;

	return 0;
}

int CS104::setTimeOutYkSel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkSel_ = val;

	return 0;
}

int CS104::setTimeOutYkExe(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkExe_ = val;

	return 0;
}

int CS104::setTimeOutYkCancel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkCancel_ = val;

	return 0;
}

int CS104::setTimeOutCallPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutCallPara_ = val;

	return 0;
}

int CS104::setTimeOutSetPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutSetPara_ = val;

	return 0;
}

int CS104::setTimeOutYcSend(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYcSend_ = val;
	return 0;
}

int CS104::CreateYkFramePara(bool bCancel,bool bDouble,unsigned char yk_type,unsigned char return_code,unsigned char & yk_code,unsigned char & trans_reason)
{
	unsigned char actcon;
	unsigned char deactcon;
	unsigned char keyfail;

	if (!bCancel)
	{
		actcon = trans_actcon;
		deactcon = trans_deactcon;
		keyfail = trans_ykfailed;
	}
	else
	{
		actcon = trans_deactcon;
		deactcon = trans_deactcon;
		keyfail = trans_ykfailed;
	}

	if (bDouble)
	{
		if (yk_type == DataBase::YkClose)
		{
			if (return_code == RETURN_CODE_ACTIVE)
			{
				yk_code = DYK_TYPE_CLOSE;
				trans_reason = actcon;
			}
			else if(return_code == RETURN_CODE_KEYERROR)
			{
				yk_code = DYK_CLOSE_NEGATIVE;
				trans_reason = keyfail;
			}
			else
			{
				yk_code = DYK_CLOSE_NEGATIVE;
				trans_reason = actcon;
			}
		}
		else if(yk_type == DataBase::YkOpen)
		{
			if (return_code == RETURN_CODE_ACTIVE)
			{
				yk_code = DYK_TYPE_OPEN;
				trans_reason = actcon;
			}
			else if(return_code == RETURN_CODE_KEYERROR)
			{
				yk_code = DYK_OPEN_NEGATIVE;
				trans_reason = keyfail;
			}
			else
			{
				yk_code = DYK_OPEN_NEGATIVE;
				trans_reason = actcon;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		if (yk_type == DataBase::YkClose)
		{
			if (return_code == RETURN_CODE_ACTIVE)
			{
				yk_code = SYK_TYPE_CLOSE;
				trans_reason = actcon;
			}
			else if(return_code == RETURN_CODE_KEYERROR)
			{
				yk_code = SYK_TYPE_CLOSE;
				trans_reason = keyfail;
			}
			else
			{
				yk_code = SYK_TYPE_CLOSE;
				trans_reason = deactcon;
			}
		}
		else if(yk_type == DataBase::YkOpen)
		{
			if (return_code == RETURN_CODE_ACTIVE)
			{
				yk_code = SYK_TYPE_OPEN;
				trans_reason = actcon;
			}
			else if(return_code == RETURN_CODE_KEYERROR)
			{
				yk_code = SYK_TYPE_OPEN;
				trans_reason = keyfail;
			}
			else
			{
				yk_code = SYK_TYPE_OPEN;
				trans_reason = deactcon;
			}
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

};//namespace Protocol
