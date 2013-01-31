#include <boost/scoped_array.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include "Protocol.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/Terminal.h"
#include "../DataBase/PriStation.h"
#include "../FileSystem/LogFactory.h"
#include "../FileSystem/Log.h"
#include "../FileSystem/Markup.h"
#include "../FileSystem/FileHandle.h"
#include "../DigitalSignature/KeyFactory.h"
#include "../DigitalSignature/PrivateKey.h"
#include "../DigitalSignature/PublicKey.h"
#include "../DigitalSignature/Sm2PublicKey.h"

namespace Protocol {

#define strEnableStatusLog "StatusLogName"
#define strEnableFrameLog "FrameLogName"
#define strPrintFrameTerm "PrintFrameTerm"
#define strPrintStatusTerm "PrintStatusTerm"
#define strSecretKey "SecretKey"
#define strKeyType "KeyType"
#define strSignStart "SignStart"
#define strKeyPath   "KeyPath"

#define strActiveDataUp "ActiveDataUp"                     //该规约是否是主动上送数据的
#define strActiveRepeatFrame "ActiveRepeatFrame"           //该规约是否需要重发请求报文
#define strFrameRepeatSum "FrameRepeatSum"                 //报文需要重发的总次数
#define strLostAnswerTimesOut "LostAnswerTimesOut"         //多少次无应答报文以后视为通讯中断
#define strWaitforanswerTimeout "WaitforanswerTimeout"     //等到应答报文超时的时间 单位：秒
#define strConsecutiveSendTimeOut "ConsecutiveSendTimeOut" //连续发送两帧报文之间需要的时间间隔 单位：毫秒
#define strActiveDataRecv "ActiveDataRecv"                 //该规约是否需要靠接收到的数据来判定结点的通讯状况
#define strLostRecvTimeOut "LostRecvTimeOut"               //多长时间未收到报文以后视为通讯中断，单位：秒
#define strClearDataBaseQuality "ClearQuality"             //初始化规约的时候是否将数据的品质描述项置为Negative
#define strClearEventLoadPtr "ClearEvent"                  //是否在规约初始化阶段重置CPristation的告警事件的Load指针，默认false

const unsigned char RecordRecvData = 1;                    //发送报文记录
const unsigned char RecordSendData = 2;                    //接收报文记录
const unsigned char RecordBroadCast = 3;                   //广播报文记录

CProtocol::CProtocol(boost::asio::io_service & io_service)
					:io_service_(io_service),
					timerWaitForAnswer_(io_service,boost::posix_time::seconds(WaitforanswerTimeOut)),
					timerConsecutiveSend_(io_service,boost::posix_time::milliseconds(ConsecutiveSendTimeOut))
{
	ProtocolObjectIndex_ = 0;

	//bWaitingForAnswer_ = false;
	bConsecutiveSend_ = false;
	bRepeatLastFrame_ = false;
	bActiveDataUp_ = false;
	bActiveDataRecv_ = false;
	bActiveRepeatFrame_ = false;
	bPrintFrame_ = false;
	bPrintStatus_ = false;
	bBroadCastSend_ = false;
	bClearDataBaseQuality_ = false;
	bClearEventLoadPtr_ = false;
	bInitQuality_ = false;
	
	SecretKeyType_ = "";
	KeyPath_ = "";
	SignStart_ = 0;

	//iCurCommpointIndex_ = 0;
	SynCharNum_ = 0;
	iLastFrameLength_ = 0;
	iFrameRepeatCount_ = 0;
	iFrameRepeatSum_ = frameRepeatSum;
	iLostAnswerTimesOut_ = LostAnswerTimesOut;
	timeOutWaitforAnswer_ = WaitforanswerTimeOut;
	timeOutConsecutiveSend_ = ConsecutiveSendTimeOut;
	timeOutLostRecv_ = LostRecvTimeOut;

	LastRecvPointIndex_ = 0;
	LastSendPointIndex_ = 0;

	recv_buf_.reset(new PublicSupport::CLoopBuf(max_recv_length));
	send_buf_.reset(new unsigned char[max_send_length]);

	iLastFrameLength_ = 0;
	max_frame_length_ = 0xff;
}

CProtocol::~CProtocol(void)
{
	StopAllTimer();
	DisconnectCmdRecallSig();
	//DisconnectSubAliveSig();
}


//xml api
int CProtocol::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	xml.ResetMainPos();
	if (xml.FindElem(strEnableStatusLog))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (!strTmp.empty())
		{
			std::string strType = xml.GetAttrib(strFileType);
			boost::algorithm::trim(strType);
			std::string strFileLimit = xml.GetAttrib(strLimit);
			boost::algorithm::trim(strFileLimit);

			EnableStatusLog(strTmp,strType,strFileLimit);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strEnableFrameLog))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (!strTmp.empty())
		{
			std::string strType = xml.GetAttrib(strFileType);
			boost::algorithm::trim(strType);
			std::string strFileLimit = xml.GetAttrib(strLimit);
			boost::algorithm::trim(strFileLimit);

			EnableFrameLog(strTmp,strType,strFileLimit);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strActiveDataUp))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		bActiveDataUp_ = false;
		if (boost::algorithm::iequals(strboolTrue,strTmp))
		{
			bActiveDataUp_ = true;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strActiveDataRecv))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		bActiveDataRecv_ = false;
		if (boost::algorithm::iequals(strboolTrue,strTmp))
		{
			bActiveDataRecv_ = true;
		}

		if (bActiveDataRecv_)
		{
			xml.ResetMainPos();
			if (xml.FindElem(strLostRecvTimeOut))
			{
				strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				try
				{
					unsigned short timerSec = boost::lexical_cast<unsigned short>(strTmp);
					timeOutLostRecv_ = timerSec;
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();

					timeOutLostRecv_ = LostRecvTimeOut;
				}
			}
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strActiveRepeatFrame))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		bActiveRepeatFrame_ = false;
		if (boost::algorithm::iequals(strboolTrue,strTmp))
		{
			bActiveRepeatFrame_ = true;
		}

		if (bActiveRepeatFrame_)
		{
			xml.ResetMainPos();
			if (xml.FindElem(strFrameRepeatSum))
			{
				std::string strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				try
				{
					unsigned short sum = boost::lexical_cast<unsigned short>(strTmp);
					iFrameRepeatSum_ = sum;
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();

					iFrameRepeatSum_ = frameRepeatSum;
				}
			}

			xml.ResetMainPos();
			if (xml.FindElem(strLostAnswerTimesOut))
			{
				std::string strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				try
				{
					unsigned short times = boost::lexical_cast<unsigned short>(strTmp);
					iLostAnswerTimesOut_ = times;
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::ostringstream ostr;
					ostr<<e.what();

					iLostAnswerTimesOut_ = LostRecvTimeOut;
				}
			}
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strWaitforanswerTimeout))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			unsigned short timerSec = boost::lexical_cast<unsigned short>(strTmp);
			timeOutWaitforAnswer_ = timerSec;
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();

			timeOutWaitforAnswer_ = WaitforanswerTimeOut;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strConsecutiveSendTimeOut))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			unsigned short timerSec = boost::lexical_cast<unsigned short>(strTmp);
			timeOutConsecutiveSend_ = timerSec;
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();

			timeOutConsecutiveSend_ = ConsecutiveSendTimeOut;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strPrintFrameTerm))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			setPrintFrameTerm(true);
		}
		else
		{
			setPrintFrameTerm(false);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strPrintStatusTerm))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			setPrintStatusTerm(true);
		}
		else
		{
			setPrintStatusTerm(false);
		}
	}
	
	xml.ResetMainPos();
	if (xml.FindElem(strSecretKey))
	{
		xml.IntoElem();

		xml.ResetMainPos();

		if (xml.FindElem(strKeyType))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			setSecretKeyType(strTmp);
		}

		xml.ResetMainPos();
		if (xml.FindElem(strSignStart))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			try
			{
				int singstart = boost::lexical_cast<int>(strTmp);
				setSignStart(singstart);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::ostringstream ostr;
				ostr<<e.what();
			}
		}

		xml.ResetMainPos();
		if (xml.FindElem(strKeyPath))
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			setSecretKeyPath(strTmp);
		}

		xml.OutOfElem();
	}

	xml.ResetMainPos();
	if (xml.FindElem(strClearDataBaseQuality))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			setClearQuality(true);
		}
		else
		{
			setClearQuality(false);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strClearEventLoadPtr))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			setClearEvent(true);
		}
		else
		{
			setClearEvent(false);
		}
	}

	return 0;
}

void CProtocol::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	if (statusLog_)
	{
		xml.AddElem(strEnableStatusLog,statusLog_->getLogPath());
		if (!statusLog_->getFileType().empty())
		{
			xml.AddAttrib(strFileType,statusLog_->getFileType());
		}
	}

	if (frameLog_)
	{
		xml.AddElem(strEnableFrameLog,frameLog_->getLogPath());
		if (frameLog_->getFileType().empty())
		{
			xml.AddAttrib(strFileType,frameLog_->getFileType());
		}
	}

	if (bActiveDataUp_)
	{
		xml.AddElem(strActiveDataUp,strboolTrue);
	}

	if (bActiveDataRecv_)
	{
		xml.AddElem(strActiveDataRecv,strboolTrue);

		if (timeOutLostRecv_ != LostRecvTimeOut)
		{
			xml.AddElem(strLostRecvTimeOut,timeOutLostRecv_);
		}
	}

	if (bActiveRepeatFrame_)
	{
		xml.AddElem(strActiveRepeatFrame,strboolTrue);

		if (iFrameRepeatSum_ != frameRepeatSum)
		{
			xml.AddElem(strFrameRepeatSum,iFrameRepeatSum_);
		}
		
		if (iLostAnswerTimesOut_ != LostAnswerTimesOut)
		{
			xml.AddElem(strLostAnswerTimesOut,iLostAnswerTimesOut_);
		}
	}

	if (timeOutWaitforAnswer_ != WaitforanswerTimeOut)
	{
		xml.AddElem(strWaitforanswerTimeout,timeOutWaitforAnswer_);
	}

	if (timeOutConsecutiveSend_ != ConsecutiveSendTimeOut)
	{
		xml.AddElem(strConsecutiveSendTimeOut,timeOutConsecutiveSend_);
	}

	if (getPrintFrameTerm())
	{
		xml.AddElem(strPrintFrameTerm,strboolTrue);
	}

	if (getPrintStatusTerm())
	{
		xml.AddElem(strPrintStatusTerm,strboolTrue);
	}
	
	if (!getSecretKeyPath().empty())
	{
		xml.AddElem(strSecretKey);
		xml.AddChildElem(strKeyType,getSecretKeyType());
		xml.AddChildElem(strSignStart,getSignStartIndex());
		xml.AddChildElem(strKeyPath,getSecretKeyPath());
	}

	if (getClearQuality())
	{
		xml.AddElem(strClearDataBaseQuality,strboolTrue);
	}

	if (getClearEvent())
	{
		xml.AddElem(strClearEventLoadPtr,strboolTrue);
	}
}

//key api
int CProtocol::CalcSecretDataLength(unsigned char * buf,size_t keyIndex,bool bCalcByFrame/* = true*/)
{
	if (CheckPubKey())
	{
		//std::ostringstream ostr;
		//ostr<<"keyindex = "<<keyIndex<<std::endl;
		//std::cout<<ostr.str();

		//std::cout<<(int)&buf[keyIndex]<<std::endl;

		return pubKey_->CalcSecretDataLength(&buf[keyIndex],bCalcByFrame);

		//ostr.str("");
		//ostr<<"CalcSecretDataLength = "<<ret<<std::endl;
		//std::cout<<ostr.str();

		//return ret;
	}

	return 0;
}

int CProtocol::getSignStartIndex()
{
	return SignStart_;
}

int CProtocol::setSignStart(int SignStart)
{
	SignStart_ = SignStart;
	return 0;
}

std::string CProtocol::getSecretKeyType()
{
	return SecretKeyType_;
}

int CProtocol::setSecretKeyType(std::string val)
{
	SecretKeyType_ = val;

	return 0;
}

std::string CProtocol::getSecretKeyPath()
{
	return KeyPath_;
}

int CProtocol::setSecretKeyPath(std::string val)
{
	KeyPath_ = val;

	return 0;
}

int CProtocol::EnablePubKey()
{
	pubKey_.reset(DigitalSignature::CKeyFactory::CreatePublicKey(getSecretKeyType(),getSecretKeyPath(),*this));

	if (CheckPubKey())
	{
		return 0;
	}

	return -1;
}

int CProtocol::EnablePriKey()
{
	priKey_.reset(DigitalSignature::CKeyFactory::CreatePrivateKey(getSecretKeyType(),getSecretKeyPath()));
	
	if (CheckPriKey())
	{
		return 0;
	}

	return -1;
}

bool CProtocol::CheckPubKey()
{
	if (pubKey_)
	{
		return pubKey_->ValidKey();
	}

	return false;
}

bool CProtocol::CheckPriKey()
{
	if (priKey_)
	{
		return priKey_->ValidKey();
	}

	return false;
}

int CProtocol::encrypt(size_t bufBegin,size_t bufCount,unsigned char * buf)
{
	if(CheckPriKey())
	{
		int bufLength = max_send_length;

		if(priKey_->AssembleSignature(&buf[bufBegin],bufCount,&buf[bufBegin],bufLength))
		{
			std::ostringstream ostr;
			ostr<<"使用私钥签名失败"<<std::endl;
			AddStatusLogWithSynT(ostr.str());

			return -1;
		}

		return bufLength;
	}

	std::ostringstream ostr;
	ostr<<"装载私钥失败"<<std::endl;
	AddStatusLogWithSynT(ostr.str());

	return -1;
}

int CProtocol::decrypt(size_t bufBegin,size_t bufCount,size_t dsStartIndex,unsigned char * buf)
{
	if (CheckPubKey())
	{
		int bufLength = max_send_length;

		if((int)bufCount <= pubKey_->getKeyLength())
		{
			std::ostringstream ostr;
			ostr<<"解密私钥时发现接收的报文长度不足，bufCount = "<<bufCount<<std::endl;
			AddStatusLogWithSynT(ostr.str());

			return -1;
		}

		return pubKey_->ParseSignature(&buf[bufBegin],bufCount,dsStartIndex,&buf[bufBegin],bufLength);
	}

	std::ostringstream ostr;
	ostr<<"装载公钥失败"<<std::endl;
	AddStatusLogWithSynT(ostr.str());

	return -1;
}

int CProtocol::AssembleCheckPubKeyCon( size_t bufIndex,unsigned char * buf,share_commpoint_ptr pristationPtr,bool bConAct )
{
	if (CheckPubKey())
	{
		boost::shared_ptr<DigitalSignature::CSm2PublicKey> sm2Key = boost::dynamic_pointer_cast<DigitalSignature::CSm2PublicKey>(pubKey_);
		if (sm2Key)
		{
			return sm2Key->AssembleCheckKeyCon(&buf[bufIndex],bConAct);
		}
	}

	return -1;
}

int CProtocol::AssembleUpdatePubKeyCon( size_t bufIndex,unsigned char * buf,share_commpoint_ptr pristationPtr,bool bConAct )
{
	if (CheckPubKey())
	{
		boost::shared_ptr<DigitalSignature::CSm2PublicKey> sm2Key = boost::dynamic_pointer_cast<DigitalSignature::CSm2PublicKey>(pubKey_);
		if (sm2Key)
		{
			return sm2Key->AssembleUpdateKeyCon(&buf[bufIndex],bConAct);
		}
	}

	return -1;
}

int CProtocol::ParseFrame_Plus(unsigned char * buf,size_t exceptedBytes)
{
	return decrypt(0,exceptedBytes,0,buf);
}

int CProtocol::CheckPlusHead(unsigned char * buf,size_t & exceptedBytes)
{
	if ((buf[0] == 0x16) && (buf[1] >= buf[2]) && buf[3] == 0x16)
	{
		exceptedBytes = CalcSecretDataLength(buf,0);

		return 0;
	}

	return -1;
}

int CProtocol::CheckPlusTail(unsigned char * buf,size_t exceptedBytes)
{
	if (CheckPubKey())
	{
		size_t sum = CalcCheckSumByte(&buf[4],exceptedBytes - 6);

		if ((buf[exceptedBytes -1] == 0x16) && (buf[exceptedBytes - 2] == sum))
		{
			return 0;
		}
	}

	return -1;
}

//cmd api
int CProtocol::AddSendCmdVal(CCmd val)
{
	int ret = CCmdQueue::AddSendCmdVal(val);

	NotifySendCmd();

	return ret;
}

int CProtocol::AddSendCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint)
{
	int ret = CCmdQueue::AddSendCmdVal(CmdType,priority,commpoint);

	NotifySendCmd();

	return ret;
}

int CProtocol::AddSendCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val)
{
	int ret = CCmdQueue::AddSendCmdVal(CmdType,priority,commpoint,val);

	NotifySendCmd();

	return ret;
}

int CProtocol::AddOnlySendCmdWithoutVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val)
{
	if (!SearchSendCmdQueue(CmdType,priority,commpoint))
	{
		return AddSendCmdVal(CmdType,priority,commpoint,val);
	}

	return 1;
}

int CProtocol::AddOnlySendCmdByCmdType(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val)
{
	if (!SearchSendCmdQueue(CmdType))
	{
		return AddSendCmdVal(CmdType,priority,commpoint,val);
	}

	return 1;
}

//commpoint api
int CProtocol::AddCommPoint(share_commpoint_ptr val)
{
	EnableCommPoint(val);

	commPoints_.push_back(weak_commpoint_ptr(val));

	return 0;
}

int CProtocol::AddCommPoint(weak_commpoint_ptr val)
{
	EnableCommPoint(val.lock());

	commPoints_.push_back(val);

	return 0;
}

int CProtocol::DelCommPoint(int index)
{
	if (index < 0 || index >= getCommPointSum())
	{
		return -1;
	}

	commPoints_.erase(commPoints_.begin() + index);

	return 0;
}

weak_commpoint_ptr CProtocol::getCommPoint(int index)
{
	if (index < 0 || index >= getCommPointSum())
	{
		//throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		return weak_commpoint_ptr();
	}

	return commPoints_.at(index);
}

int CProtocol::getCommPointIndexByAddrCommType(unsigned char commTypeVal,unsigned short addrVal,unsigned short addrLen/* = 0*/)
{
	typeAddr AddrMask = 0;

	if(addrLen > sizeof(typeAddr))
	{
		addrLen = sizeof(typeAddr);
	}
	
	for (size_t i=0;i<addrLen;i++)
	{
		AddrMask += (0xff << (i * 8));
	}

	for (int i=0;i<getCommPointSum();i++)
	{
		share_commpoint_ptr point = getCommPoint(i).lock();
		if (point)
		{
			if (point->getCommPointType() == commTypeVal)
			{
				if(AddrMask == 0)
				{
					if(point->getAddr() == addrVal || (point->getbAcceptBroadCast() && point->getBroadCastAddr() == addrVal))
					{
						return i;
					}
				}
				else
				{
					if((point->getAddr() & AddrMask) == addrVal || (point->getbAcceptBroadCast() && (point->getBroadCastAddr() & AddrMask) == addrVal))
					{
						return i;
					}

				}

			}
		}
	}

	return -1;
}
int CProtocol::getCommPointSum()
{
	return commPoints_.size();
}

void CProtocol::ClearCommPoint()
{
	commPoints_.clear();
}

/*
bool CProtocol::getDataActiveUp()
{
	return bActiveDataUp_;
}
*/

/*
int CProtocol::AddPriStation(share_pristation_ptr val)
{
	pristations_.push_back(weak_pristation_ptr(val));

	return 0;
}

int CProtocol::AddTerminal(share_terminal_ptr val)
{
	terminals_.push_back(weak_terminal_ptr(val));

	return 0;
}

int CProtocol::DelPriStation(unsigned short index)
{
	if (index < 0 || index >= getPriStationSum())
	{
		return -1;
	}

	pristations_.erase(pristations_.begin() + index);

	return 0;
}

int CProtocol::DelTerminal(unsigned short index)
{
	if (index < 0 || index >= getTerminalSum())
	{
		return -1;
	}

	terminals_.erase(terminals_.begin() + index);

	return 0;
}

int CProtocol::getPriStationSum()
{
	return pristations_.size();
}

int CProtocol::getTerminalSum()
{
	return terminals_.size();
}

void CProtocol::ClearPriStation()
{
	pristations_.clear();
}

void CProtocol::ClearTerminal()
{
	terminals_.clear();
}

weak_pristation_ptr CProtocol::getPriStationPtr(unsigned short index)
{
	if (index < 0 || index >= getPriStationSum())
	{
		return weak_pristation_ptr();
	}

	return pristations_[index];
}

weak_terminal_ptr CProtocol::getTerminalPtr(unsigned short index)
{
	if (index < 0 || index >= getTerminalSum())
	{
		return weak_terminal_ptr();
	}

	return terminals_[index];
}
*/

//frame api
int CProtocol::MatchFrameHead( size_t & exceptedBytes )
{
	unsigned char tempChar;
	
	if (SynCharNum_ <= 0)
	{
		return 0;
	}

	size_t charLeft = recv_buf_->charNum();
	while (charLeft >= SynCharNum_)
	{
		//temp_buf = new unsigned char[SynCharNum_ + 10];
		boost::scoped_array<unsigned char> temp_buf(new unsigned char[SynCharNum_ + 1]);
		recv_buf_->copyBuf(temp_buf.get(),SynCharNum_);
		int ret = CheckFrameHead(temp_buf.get(),exceptedBytes);
		if (!ret)
		{
			return 0;
		}
		else
		{
			recv_buf_->popChar(tempChar);
		}
		charLeft = recv_buf_->charNum();
	}

	return NO_SYN_HEAD;
}

int CProtocol::RecvProcess( unsigned char * buf, size_t count )
{
	int ret = 0;
	unsigned char tempChar;
	size_t exceptedRecvBytes = 0;

	//std::ostringstream ostrDebug;
	//ostrDebug<<"收到报文"<<count<<"字节"<<std::endl;
	//AddStatusLogWithSynT(ostrDebug.str());

	RecordFrameData(buf,count,RecordRecvData);

	recv_buf_->putBuf(buf,count);

	size_t charLeft = recv_buf_->charNum();
	
	while (charLeft >= SynCharNum_)
	{
		//std::ostringstream ostrDebug;
		//ostrDebug<<"charLeft"<<charLeft<<" SynCharNum_"<<SynCharNum_<<" loadPtr"<<loopbuf_->getLoadPtr()<<" savePtr"<<loopbuf_->getSavePtr()<<std::endl;
		//AddStatusLogWithSynT(ostrDebug.str());

		ret = MatchFrameHead(exceptedRecvBytes);
		if (!ret)
		{
			//ostrDebug.str("");
			//ostrDebug<<"check frame head ok"<<" loadPtr"<<loopbuf_->getLoadPtr()<<" savePtr"<<loopbuf_->getSavePtr()<<std::endl;
			//AddStatusLogWithSynT(ostrDebug.str());

			charLeft = recv_buf_->charNum();
			if ((exceptedRecvBytes > 0) && (charLeft >= exceptedRecvBytes))
			{
				//ostrDebug.str("");
				//ostrDebug<<"charLeft"<<charLeft<<" exceptedRecvBytes"<<exceptedRecvBytes<<" loadPtr"<<loopbuf_->getLoadPtr()<<" savePtr"<<loopbuf_->getSavePtr()<<std::endl;
				//AddStatusLogWithSynT(ostrDebug.str());

				boost::scoped_array<unsigned char> temp_buf(new unsigned char[exceptedRecvBytes + 10]);
				recv_buf_->copyBuf(temp_buf.get(),exceptedRecvBytes);

				//ostrDebug.str("");
				//ostrDebug<<"copy buf"<<exceptedRecvBytes<<" loadPtr"<<loopbuf_->getLoadPtr()<<" savePtr"<<loopbuf_->getSavePtr()<<std::endl;
				//AddStatusLogWithSynT(ostrDebug.str());

				ret = CheckFrameTail(temp_buf.get(),exceptedRecvBytes);
				if (!ret)
				{
					//ostrDebug.str("");
					//ostrDebug<<"check frame tail ok"<<" loadPtr"<<loopbuf_->getLoadPtr()<<" savePtr"<<loopbuf_->getSavePtr()<<std::endl;
					//AddStatusLogWithSynT(ostrDebug.str());

					recv_buf_->getBuf(temp_buf.get(),exceptedRecvBytes);
					int index = -1;
					try
					{
						//ostrDebug.str("");
						//ostrDebug<<"get buf"<<exceptedRecvBytes<<" loadPtr"<<loopbuf_->getLoadPtr()<<" savePtr"<<loopbuf_->getSavePtr()<<std::endl;
						//AddStatusLogWithSynT(ostrDebug.str());

						index = ParseFrameBody(temp_buf.get(),exceptedRecvBytes);
					}
					catch(PublicSupport::dat2def_exception & err)
					{
						std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(err);
						if (strPtr)
						{
							std::ostringstream ostr;
							ostr<<"解析报文体失败,";
							int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(err);
							if (indexPtr)
							{
								ostr<<"data no:"<<(*indexPtr)<<",";
							}
							ostr<<"error info:"<<(*strPtr)<<std::endl;

							AddStatusLogWithSynT(ostr.str());
						}

						exceptedRecvBytes = 0;
					}
					catch(...)
					{
						index = -1;
					}

					exceptedRecvBytes = 0;
					
					if (index >= 0)
					{
						share_commpoint_ptr commpoint = getCommPoint(index).lock();
						if (commpoint)
						{
							commpoint->setCommActive(true);
							commpoint->NotifyEventStatus(COMM_EVENT,RETURN_CODE_CMDRECV);

							if (bActiveDataRecv_)
							{
								commpoint->ResetTimerRecv(timeOutLostRecv_);
							}
						}
						
						if (getWaitingForAnswer())
						{
							if (getWaitingForAnswer() == commpoint)
							{
								ClearWaitAnswerFlag();
							}
						}
					}
				}
				else
				{
					recv_buf_->popChar(tempChar);
				}
			}
			else
			{
				return LESS_RECV_BYTE;
			}
		}
		charLeft = recv_buf_->charNum();
	}

	return ret;
}

int CProtocol::SendProcess( unsigned char * buf, CCmd & cmd )
{
	//std::ostringstream ostrDebug;

	size_t bytesAssembled = 0;
	size_t bufbegin = bytesAssembled;

	int ret = AssembleFrameHead(bytesAssembled,buf,cmd);
	if (ret < 0)
	{
		return ret;
	}
	bytesAssembled += ret; 

	//ostrDebug<<this<<"组装报文头OK！";
	//PrintDebug(ostrDebug.str());

	try
	{
		ret = AssembleFrameBody(bytesAssembled,buf,cmd);
	}
	catch(PublicSupport::dat2def_exception & err)
	{
		std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(err);
		if (strPtr)
		{
			std::ostringstream ostr;
			ostr<<"组装报文体失败,";
			int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(err);
			if (indexPtr)
			{
				ostr<<"data no:"<<(*indexPtr)<<",";
			}
			ostr<<"error info:"<<(*strPtr)<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}

		//ostrDebug.str("");
		//ostrDebug<<this<<"组装报文体失败："<<err.what();
		//PrintDebug(ostrDebug.str());

		return -1;
	}
	catch(...)
	{	
		//ostrDebug.str("");
		//ostrDebug<<this<<"组装报文体失败：未知错误";
		//PrintDebug(ostrDebug.str());

		return -1;
	}

	if (ret <= 0)
	{
		//ostrDebug.str("");
		//ostrDebug<<this<<"组装报文体失败，报文体长度为："<<ret;
		//PrintDebug(ostrDebug.str());

		return ret;
	}
	bytesAssembled += ret;

	//ostrDebug.str("");
	//ostrDebug<<this<<"组装报文体OK！";
	//PrintDebug(ostrDebug.str());

	ret = AssembleFrameTail(bufbegin,bytesAssembled,buf,cmd);
	if (ret < 0)
	{
		return ret;
	}
	bytesAssembled += ret;

	//ostrDebug.str("");
	//ostrDebug<<this<<"组装报文尾OK！";
	//PrintDebug(ostrDebug.str());

	if (bytesAssembled > 0)
	{
		setLastSendCmd(cmd.getCmdType());
		setLastSendPointIndex(getCommPointIndexByPtr(cmd.getCommPoint()));
		cmd.getCommPoint()->NotifyEventStatus(COMM_EVENT,RETURN_CODE_CMDSEND);
	}

	return bytesAssembled;
}

//protocol api
share_commpoint_ptr CProtocol::getWaitingForAnswer()
{
	return CurWaitCommpointPtr_;
}

void CProtocol::setWaitingForAnswer(share_commpoint_ptr val)
{
	CurWaitCommpointPtr_ = val;
}

bool CProtocol::getbConsecutiveSend()
{
	return bConsecutiveSend_;
}

void CProtocol::setbConsecutiveSend(bool val)
{
	bConsecutiveSend_ = val;
}

//commpoint api
int CProtocol::MathcCommPoint(weak_commpoint_ptr val)
{
	for (int i=0;i<getCommPointSum();i++)
	{
		if (getCommPoint(i).lock() == val.lock())
		{
			return i;
		}
	}

	return -1;
}

int CProtocol::DisableAllCommPoints()
{
	for (int i=0;i<getCommPointSum();i++)
	{
		share_commpoint_ptr point = getCommPoint(i).lock();
		if (point)
		{
			point->setCommActive(false);
		}
	}

	return 0;
}

int CProtocol::InitAllCommPoints()
{
	for (int i=0;i<getCommPointSum();i++)
	{
		share_commpoint_ptr point = getCommPoint(i).lock();
		
		EnableCommPoint(point);
	}

	return 0;
}

int CProtocol::EnableCommPoint(share_commpoint_ptr point)
{
	if (point)
	{
		point->setInitCommPointFlag(false);
		point->setSynTCommPointFlag(false);

		if (point->getCommPointType() == TERMINAL_NODE)
		{
			share_terminal_ptr terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(point);

			if(getClearQuality())
			{
				terminalPtr->ClearDataBaseQuality(false);
			}

			terminalPtr->setActiveInitQuality(bInitQuality_);

		}

		if ((getClearEvent()) && (point->getCommPointType() == PRISTATION_NODE))
		{
			share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(point);

			pristationPtr->InitCosLoadPtr();
			pristationPtr->InitSoeLoadPtr();
			pristationPtr->InitYcvarLoadPtr();
		}

		return 0;
	}

	return -1;
}

//int CProtocol::setCommPointsDataBaseQuality(bool active)
//{
//	for (int i=0;i<getCommPointSum();i++)
//	{
//		share_commpoint_ptr point = getCommPoint(i).lock();
//		if (point)
//		{
//			point->ClearDataBaseQuality(active);
//		}
//	}
//
//	return 0;
//}

/*
bool CProtocol::getAllCommPointAlive()
{
	for (int i=0;i<getCommPointSum();i++)
	{
		share_commpoint_ptr pointTmp = getCommPoint(i).lock();
		if (pointTmp)
		{
			if(!pointTmp->getCommActive())
			{
				return false;
			}
		}
	}

	return true;
}
*/

//send cmd api
int CProtocol::WriteDatas(unsigned char * buf,size_t length)
{
	if (CommSigConnection_.connected())
	{
		iLastFrameLength_ = length;
		RecordFrameData(send_buf_.get(),length,RecordSendData);
		CommSig_(WriteData,buf,length);
		
		return 0;
	}

	return -1;
	
}

int CProtocol::WriteBroadCast(unsigned char * buf,size_t length)
{
	if (CommSigConnection_.connected())
	{
		iLastFrameLength_ = length;
		RecordFrameData(send_buf_.get(),length,RecordBroadCast);
		CommSig_(BroadCast,buf,length);

		return 0;
	}

	return -1;
}

int CProtocol::ReConnnectChannel()
{
	if (CommSigConnection_.connected())
	{
		CommSig_(ReConnect,NULL,0);

		return 0;
	}

	return -1;
	
}

void CProtocol::ClearFrameRepeatFlag()
{
	bRepeatLastFrame_ = false;
	iFrameRepeatCount_ = 0;
}

void CProtocol::ClearWaitAnswerFlag()
{
	ClearFrameRepeatFlag();
	setWaitingForAnswer(share_commpoint_ptr());
	ResetTimerWaitForAnswer(share_commpoint_ptr(),false);
}

bool CProtocol::CheckFrameRepeatOutCount(size_t & count)
{
	if (count >= iFrameRepeatSum_)
	{
		count = 0;
		return false;
	}
	else
	{
		count++;
		return true;
	}
}

int CProtocol::getFrameBufLeft()
{
	return recv_buf_->charNum();
}

//timer api
void CProtocol::handle_timerWaitForAnswer( const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{	
		if (bActiveRepeatFrame_)
		{
			if (CheckFrameRepeatOutCount(iFrameRepeatCount_))
			{
				bRepeatLastFrame_ = true;
			}
			else
			{
				ClearFrameRepeatFlag();
				setWaitingForAnswer(share_commpoint_ptr());
				//share_commpoint_ptr commpointTmp = (getCommPoint(index)).lock();
				if (point)
				{
					if(point->CheckLostAnserTimesPlus(iLostAnswerTimesOut_))
					{
						QueryUnAliveCommPoint(point);
					}
				}
			}
		}
		else
		{
			setWaitingForAnswer(share_commpoint_ptr());
		}

		NotifySendCmd();
	}
	else if (error == boost::asio::error::operation_aborted )
	{
		NotifySendCmd();
	}
}

void CProtocol::handle_timerConsecutiveSend(const boost::system::error_code& error)
{
	setbConsecutiveSend(false);

	if (!error)
	{
		NotifySendCmd();
	}
}

int CProtocol::InitProtocol()
{
	InitAllCommPoints();

	return 0;
}

void CProtocol::UninitProtocol()
{
	ClearSendCmdQueue();
	StopAllTimer();
	//DisableAllCommPoints();

	setbConsecutiveSend(false);
	setWaitingForAnswer(share_commpoint_ptr());
}

//send cmd api
void CProtocol::handle_SendCmd()
{
	if (bActiveRepeatFrame_ && bRepeatLastFrame_)
	{
		if (getBroadCastSend())
		{
			WriteBroadCast(send_buf_.get(),iLastFrameLength_);
			setBroadCastSend(false);
		}
		else
		{
			WriteDatas(send_buf_.get(),iLastFrameLength_);
		}

		bRepeatLastFrame_ = false;

		share_commpoint_ptr waitpoint = getWaitingForAnswer();
		if (waitpoint)
		{
			if (MathcCommPoint(waitpoint) >= 0)
			{
				ResetTimerWaitForAnswer(waitpoint,true);
			}
		}
	}

	if (getbConsecutiveSend())
	{
		//std::ostringstream ostrDebug;
		//ostrDebug<<this<<"发送停顿，退出！";
		//PrintDebug(ostrDebug.str());

		return;
	}

	if (getWaitingForAnswer())
	{
		//std::ostringstream ostrDebug;
		//ostrDebug<<this<<"等待应答，退出！";
		//PrintDebug(ostrDebug.str());

		return;
	}

	CCmd cmdVal;
	if (!getMaxPriopriySendCmd(cmdVal))
	{
		//std::ostringstream ostrDebug;
		//ostrDebug<<this<<"获取命令："<<cmdVal.getCmdType();
		//PrintDebug(ostrDebug.str());

		//iCurCommpointIndex_ = MathcCommPoint(cmdVal.getCommPoint());
		//if (iCurCommpointIndex_ < 0)
		if(MathcCommPoint(cmdVal.getCommPoint()) < 0)
		{
			//ostrDebug.str("");
			//ostrDebug<<this<<"命令中通讯点参数非法，退出！";
			//PrintDebug(ostrDebug.str());
			return;
		}

		int length = SendProcess(send_buf_.get(),cmdVal);
		if (length > 0)
		{
			if (getBroadCastSend())
			{
				WriteBroadCast(send_buf_.get(),length);
				setBroadCastSend(false);
			}
			else
			{
				WriteDatas(send_buf_.get(),length);
			}

			if (getWaitingForAnswer())
			{
				//ResetTimerWaitForAnswer(iCurCommpointIndex_,true);
				ResetTimerWaitForAnswer(cmdVal.getCommPoint(),true);
			}

			ResetTimerConsecutiveSend(true);

			//ostrDebug.str("");
			//ostrDebug<<this<<"当前发送命令类型标识："<<cmdVal.getCmdType();
			//PrintDebug(ostrDebug.str());
		}

		//ostrDebug.str("");
		//ostrDebug<<this<<"还有"<<getSendCmdQueueSum()<<"个命令待发送！";
		//PrintDebug(ostrDebug.str());
		NotifySendCmd();
	}
}

typeCmd CProtocol::getLastSendCmd()
{
	return LastSendCmd_;
}

int CProtocol::setLastSendCmd(typeCmd val)
{
	LastSendCmd_ = val;

	return 0;
}

typeCmd CProtocol::getLastRecvCmd()
{
	return LastRecvCmd_;
}

int CProtocol::setLastRecvCmd(typeCmd val)
{
	LastRecvCmd_ = val;

	return 0;
}

int CProtocol::getLastSendPointIndex()
{
	return LastSendPointIndex_;
}

int CProtocol::getLastRecvPointIndex()
{
	return LastRecvPointIndex_;
}

int CProtocol::setLastSendPointIndex(int val)
{
	LastSendPointIndex_ = val;

	return 0;
}

int CProtocol::setLastRecvPointIndex(int val)
{
	LastRecvPointIndex_ = val;

	return 0;
}

int CProtocol::getLastFrameLength()
{
	return iLastFrameLength_;
}

//log api
int CProtocol::AddStatusLog(std::string strVal)
{
	if (getPrintStatusTerm())
	{
		PrintInfoToTerm(strVal);
	}

	if (statusLog_)
	{
		return statusLog_->AddRecord(strVal);
	}

	return -1;
}

int CProtocol::AddStatusLogWithSynT(std::string strVal)
{
	if (getPrintStatusTerm())
	{
		PrintInfoToTerm(strVal);
	}

	if (statusLog_)
	{
		return statusLog_->AddRecordWithSynT(strVal);
	}

	return -1;
}

int CProtocol::AddFrameLog(std::string strVal)
{
	if (frameLog_)
	{
		return frameLog_->AddRecord(strVal);
	}

	return -1;
}

int CProtocol::AddFrameLogWithSynT(std::string strVal)
{
	if (frameLog_)
	{
		return frameLog_->AddRecordWithSynT(strVal);
	}

	return -1;
}

int CProtocol::RecordFrameData( const unsigned char * buf,size_t count,unsigned char datatype)
{
	if (frameLog_ || getPrintFrameTerm())
	{
		if (count > 0)
		{
			std::ostringstream ostr;

			switch(datatype)
			{
			case RecordRecvData:
				ostr<<"Recv: ";
				break;

			case RecordSendData:
				ostr<<"Send: ";
				break;

			case RecordBroadCast:
				ostr<<"BroadCast: ";
				break;

			default:
				ostr<<"UnDefine: ";
				break;
			}

			for (size_t i=0;i<count;i++)
			{
				ostr<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)buf[i];
			}
			
			ostr<<std::endl;
			
			if (frameLog_)
			{
				AddFrameLogWithSynT(ostr.str());
			}

			if (bPrintFrame_)
			{
				PrintInfoToTerm(ostr.str());
			}

			return 0;
		}
	}

	return -1;
}

//log ptr enable
int CProtocol::EnableStatusLog( std::string filename,std::string filetype,std::string limit /*=""*/ )
{
	//std::ostringstream ostr;
	//ostr<<"_"<<ProtocolObjectIndex_<<".";
	//boost::algorithm::replace_last(filename,".",ostr.str());

	boost::filesystem::path log_path(filename);
	std::string extension_name = log_path.extension().string();
	std::string parent_path = log_path.parent_path().string();
	std::string stem = log_path.stem().string();
	std::ostringstream ostr;
	if (parent_path.empty())
	{
		ostr<<stem<<"_"<<ProtocolObjectIndex_<<extension_name;
	}
	else
	{
		ostr<<parent_path<<"//"<<stem<<"_"<<ProtocolObjectIndex_<<extension_name;
	}

	statusLog_.reset(FileSystem::CLogFactory::CreateLog(filename,filetype,limit));

	return 0;
}

int CProtocol::EnableFrameLog( std::string filename,std::string filetype,std::string limit /*=""*/ )
{
	//std::ostringstream ostr;
	//ostr<<"_"<<ProtocolObjectIndex_<<".";
	//boost::algorithm::replace_last(filename,".",ostr.str());

	boost::filesystem::path log_path(filename);
	std::string extension_name = log_path.extension().string();
	std::string parent_path = log_path.parent_path().string();
	std::string stem = log_path.stem().string();
	std::ostringstream ostr;
	if (parent_path.empty())
	{
		ostr<<stem<<"_"<<ProtocolObjectIndex_<<extension_name;
	}
	else
	{
		ostr<<parent_path<<"//"<<stem<<"_"<<ProtocolObjectIndex_<<extension_name;
	}

	frameLog_.reset(FileSystem::CLogFactory::CreateLog(filename,filetype,limit));

	return 0;
}

void CProtocol::DisableStatusLog()
{
	statusLog_.reset();
}

void CProtocol::DisableFrameLog()
{
	frameLog_.reset();
}

share_commpoint_ptr CProtocol::getNextCommPoint(unsigned char pointType,boost::logic::tribool bCommActive,size_t curIndex)
{
	curIndex++;

	for (int i=0;i<getCommPointSum();i++)
	{
		share_commpoint_ptr tmpPtr = getCommPoint((curIndex + i) % getCommPointSum()).lock();
		if(tmpPtr)
		{
			if (tmpPtr->getCommPointType() == pointType)
			{
				if (boost::logic::indeterminate(bCommActive))
				{
					return tmpPtr;
				}
				else
				{
					if (tmpPtr->getCommActive() == bCommActive)
					{
						return tmpPtr;
					}
				}
			}
		}
	}

	/*
	if(getCommPointSum() > 0)
	{
		return getCommPoint(0).lock();
	}
	*/

	return share_commpoint_ptr();
}

share_commpoint_ptr CProtocol::getNextCommPointBySelfDef(unsigned char pointType,boost::function<bool(share_commpoint_ptr)> CheckTrueFunC,size_t curIndex)
{
	curIndex++;

	for (int i=0;i<getCommPointSum();i++)
	{
		share_commpoint_ptr tmpPtr = getCommPoint((curIndex + i) % getCommPointSum()).lock();
		if(tmpPtr)
		{
			if (tmpPtr->getCommPointType() == pointType)
			{
				if (CheckTrueFunC(tmpPtr))
				{
					return tmpPtr;
				}
			}
		}
	}

	/*
	if(getCommPointSum() > 0)
	{
		return getCommPoint(0).lock();
	}
	*/

	return share_commpoint_ptr();
}

share_commpoint_ptr CProtocol::getNextCommPoint(unsigned char pointType,boost::logic::tribool bCommActive,share_commpoint_ptr point)
{
	//share_commpoint_ptr pointTmp = point.lock();

	if (point)
	{
		int curIndex = getCommPointIndexByPtr(point);
		if (curIndex >= 0)
		{
			return getNextCommPoint(pointType,bCommActive,curIndex);
		}
	}

	if(getCommPointSum() > 0)
	{
		return getCommPoint(0).lock();
	}

	return share_commpoint_ptr();
	
}

share_commpoint_ptr CProtocol::getNextCommPointBySelfDef(unsigned char pointType,boost::function<bool(share_commpoint_ptr)> CheckTrueFunC,share_commpoint_ptr point)
{
	//share_commpoint_ptr pointTmp = point.lock();

	if (point)
	{
		int curIndex = getCommPointIndexByPtr(point);
		if (curIndex >= 0)
		{
			return getNextCommPointBySelfDef(pointType,CheckTrueFunC,curIndex);
		}
	}

	if(getCommPointSum() > 0)
	{
		return getCommPoint(0).lock();
	}

	return share_commpoint_ptr();

}

share_commpoint_ptr CProtocol::getFirstCommPoint()
{
	if (getCommPointSum() > 0)
	{
		return getCommPoint(0).lock();
	}

	return share_commpoint_ptr();
}

int CProtocol::getCommPointIndexByPtr(share_commpoint_ptr point)
{
	for (int i=0;i<getCommPointSum();i++)
	{
		if (getCommPoint(i).lock() == point)
		{
			return i;
		}
	}

	return -1;
}

int CProtocol::getMeanvalueOfPointsSum(unsigned short minVal,unsigned short val)
{
	if (getCommPointSum() > 0)
	{
		unsigned short meanVal = val / getCommPointSum();

		return (minVal > meanVal) ? minVal : meanVal;
	}
	else
	{
		return val;
	}
}
//timer api
void CProtocol::StopAllTimer()
{
	for (size_t i=0;i<timers_.size();i++)
	{
		timers_[i]->cancel();
	}

	timerConsecutiveSend_.cancel();
	ResetTimerWaitForAnswer(share_commpoint_ptr(),false);
}

void CProtocol::AddTimer(timerPtr val)
{
	timers_.push_back(val);
}

void CProtocol::ResetTimerWaitForAnswer(share_commpoint_ptr point,bool bContinue/* = true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerWaitForAnswer_.expires_from_now(boost::posix_time::seconds(timeOutWaitforAnswer_));
		}
		else
		{
			timerWaitForAnswer_.expires_from_now(boost::posix_time::seconds(val));
		}

		timerWaitForAnswer_.async_wait(boost::bind(&CProtocol::handle_timerWaitForAnswer,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerWaitForAnswer_.cancel();
	}
}

void CProtocol::ResetTimerConsecutiveSend(bool bContinue /*= true*/,unsigned short val /*= 0*/)
{
	setbConsecutiveSend(bContinue);

	if (bContinue)
	{
		if (val == 0)
		{
			timerConsecutiveSend_.expires_from_now(boost::posix_time::milliseconds(timeOutConsecutiveSend_));
		}
		else
		{
			timerConsecutiveSend_.expires_from_now(boost::posix_time::milliseconds(val));
		}

		timerConsecutiveSend_.async_wait(boost::bind(&CProtocol::handle_timerConsecutiveSend,this,boost::asio::placeholders::error));
	}
	else
	{
		timerConsecutiveSend_.cancel();
	}
}

int CProtocol::QueryUnAliveCommPoint(share_commpoint_ptr point)
{
	return 0;
}

//sig api
int CProtocol::ConnectCmdRecallSig( CmdRecallSlotType slotVal )
{
	CmdConSigConnection_ = CmdConSig_.connect(slotVal);

	return 0;
}

int CProtocol::DisconnectCmdRecallSig()
{
	CmdConSigConnection_.disconnect();

	return 0;
}

int CProtocol::ConnectSubAliveSig()
{
	if (getCommPointSum() > 0)
	{
		share_commpoint_ptr ptrTmp = getCommPoint(0).lock();
		if (ptrTmp)
		{
			if (ptrTmp->getCommPointType() == PRISTATION_NODE)
			{
				SubAliveSigConnection_ = ptrTmp->ConnectSubAliveSig(boost::bind(&CProtocol::ProcessSubAliveSig,this,_1,_2,_3,_4));
				return 0;
			}
		}

	}

	return -1;
}
int CProtocol::DisconnectSubAliveSig()
{
	SubAliveSigConnection_.disconnect();

	return 0;
}

bool CProtocol::getSubAliveSigConnection()
{
	return SubAliveSigConnection_.connected();
}

void CProtocol::ProcessSubAliveSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	switch (cmdType)
	{
	case COS_DATA_UP:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					if (bActiveDataUp_)
					{
						//AddSendCmdVal(COS_DATA_UP,COS_DATA_UP_PRIORITY,commPoint);
						AddOnlySendCmdByCmdType(COS_DATA_UP,COS_DATA_UP_PRIORITY,commPoint,boost::any());
					}
				}
			}
		}
		break;

	case SOE_DATA_UP:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					if (bActiveDataUp_)
					{
						//AddSendCmdVal(SOE_DATA_UP,SOE_DATA_UP_PRIORITY,commPoint);
						AddOnlySendCmdByCmdType(SOE_DATA_UP,SOE_DATA_UP_PRIORITY,commPoint,boost::any());
					}
				}
			}
		}
		break;
		
	case YCVAR_DATA_UP:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					if (bActiveDataUp_)
					{
						//AddSendCmdVal(YCVAR_DATA_UP,YCVAR_DATA_UP_PRIORITY,commPoint);
						AddOnlySendCmdByCmdType(YCVAR_DATA_UP,YCVAR_DATA_UP_PRIORITY,commPoint,boost::any());
					}
				}
			}
		}
		break;

	case YCI_SEND_BYTIME://电流定时上送
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE && ReturnCode == RETURN_CODE_ACTIVE)
				{
					if (bActiveDataUp_)
					{
						AddSendCmdVal(YCI_SEND_BYTIME,YCI_SEND_BYTIME_PRIORITY,commPoint,val);
					}
				}
			}
		}
		break;


	default:
		break;
	}
}

SigConnection CProtocol::ConnectCommSingal(CommSlotType slotVal)
{
	CommSigConnection_ = CommSig_.connect(slotVal);
	
	return CommSigConnection_;
}

bool CProtocol::getPrintFrameTerm()
{
	return bPrintFrame_;
}

int CProtocol::setPrintFrameTerm(bool val)
{
	bPrintFrame_ = val;

	return 0;
}

bool CProtocol::getPrintStatusTerm()
{
	return bPrintStatus_;
}

int CProtocol::setPrintStatusTerm(bool val)
{
	bPrintStatus_ = val;

	return 0;
}

int CProtocol::PrintInfoToTerm(std::string strVal)
{
	if (getPrintFrameTerm() || getPrintStatusTerm())
	{
		std::cout<<strVal;

		return 0;
	}

	return -1;
}

bool CProtocol::getBroadCastSend()
{
	return bBroadCastSend_;
}

int CProtocol::setBroadCastSend(bool val)
{
	bBroadCastSend_ = val;

	return 0;
}

bool CProtocol::getClearQuality()
{
	return bClearDataBaseQuality_;
}

int CProtocol::setClearQuality(bool val)
{
	bClearDataBaseQuality_ = val;

	return 0;
}

bool CProtocol::getClearEvent()
{
	return bClearEventLoadPtr_;
}

int CProtocol::setClearEvent(bool val)
{
	bClearEventLoadPtr_ = val;

	return 0;
}

unsigned long CProtocol::FileHandleGetRemainLength(void)
{
	return FileHandle_->GetRemainLength();
}

unsigned long CProtocol::FileHandleGetTotalLength(void)
{
	return FileHandle_->GetTotalLength();
}

std::string CProtocol::FileHandleGetFileName(void)
{
   return FileHandle_->GetFileName();
}

unsigned int CProtocol::FileHandleSetTotalLength(unsigned long Totallen)
{
	return FileHandle_->SetTotalLength(Totallen);
}

int CProtocol::FileHandleWrite(void)
{
	return FileHandle_->HandleWrite();
}

int CProtocol::FileHandleWriteByByte(void)
{
	return FileHandle_->HandleWriteByByte();
}

int CProtocol::FileHandleRead(void)
{
	return FileHandle_->HandleRead();
}

int CProtocol::FileHandleReadFor533Pro(void)
{
	return FileHandle_->HandleReadFor533Pro();
}

int CProtocol::FileHandleOutFile(unsigned char * filedata,int length)
{
	return FileHandle_->OutFile(filedata,length);
}

int CProtocol::FileHandleGetFile(unsigned char * filedata,int length)
{
	return FileHandle_->GetFile(filedata,length);
}

int CProtocol::FileHandleBegain(std::string name)
{

	FileHandle_.reset(new FileSystem::CFileHandle(name));

	return 0;
}

int CProtocol::FileHandleFinish(void)
{
	FileHandle_.reset();

	return 0;
}

int CProtocol::NotifySendCmd()
{
	if (getbConsecutiveSend())
	{
		//std::ostringstream ostrDebug;
		//ostrDebug<<this<<"发送停顿，退出！";
		//PrintDebug(ostrDebug.str());

		return -1;
	}

	//if (getWaitingForAnswer())
	//{
	//	//std::ostringstream ostrDebug;
	//	//ostrDebug<<this<<"等待应答，退出！";
	//	//PrintDebug(ostrDebug.str());

	//	return -1;
	//}

	if(getSendCmdQueueSum() > 0)
	{
		io_service_.post(boost::bind(&CProtocol::handle_SendCmd,this));
	}

	return 0;
}

}; //Protocol

