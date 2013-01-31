#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include "H104_Transmit.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../FileSystem/Markup.h"
#include "../DataBase/Terminal.h"

namespace Protocol {

const std::string strDefaultCfg = "H104_TransmitCfg.xml";

//针对104规约的传送原因定义
const unsigned char trans_act = 0x06;

//针对104规约的报文类型标识定义
const unsigned char M_CS_NA_1 = 0x67;

const unsigned char SYN_HEAD_LENGTH = 1;

const std::string strIGramCounterLength = "IGramCounterLength";

CH104_Transmit::CH104_Transmit(boost::asio::io_service & io_service)
	:CProtocol(io_service)
{
	SynCharNum_ = 2;

	InitDefaultFrameElem();

	LoadXmlCfg(strDefaultCfg);

	InitFrameLocation(SYN_HEAD_LENGTH);
}

CH104_Transmit::~CH104_Transmit(void)
{
}

void CH104_Transmit::InitDefaultFrameElem()
{
	FrameLenLength_ = DEFAULT_FrameLenLength;                               //报文长度标识的字节长度
	IGramCounterLength_ = DEFAULT_IGramCounterLength;
	FrameTypeLength_ = DEFAULT_FrameTypeLength;                             //报文类型标识的字节长度
	InfoNumLength_ =   DEFAULT_InfoNumLength;                               //信息体数目标识的字节长度
	TransReasonLength_ = DEFAULT_TransReasonLength;                         //传送原因标识的字节长度
	AsduAddrLength_ = DEFAULT_AsduAddrLength;                               //装置地址标识的字节长度
	InfoAddrLength_ = DEFAULT_InfoAddrLength;                               //信息体地址标识的字节长度
}

void CH104_Transmit::InitFrameLocation(size_t frameHead)
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

stSynTimePara CH104_Transmit::getIgnorePara()
{
	return synPara_;
}

int CH104_Transmit::setIgnorePara(stSynTimePara val)
{
	synPara_ = val;

	return 0;
}

int CH104_Transmit::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	exceptedBytes = getFrameBufLeft();

	return 0;
}

int CH104_Transmit::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

int CH104_Transmit::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	share_terminal_ptr terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(getFirstCommPoint());

	if (!terminalPtr)
	{
		return -1;
	}

	if (getbIgnoreSynTime() && exceptedBytes > 6)
	{
		unsigned short OppositeSendCounter = ~(~(BufToVal(&buf[RecvCounterLocation_],IGramCounterLength_)) | 0x01); //对端I格式报文发送计数器
		unsigned short OppositeRecvCounter = ~(~(BufToVal(&buf[SendCounterLocation_],IGramCounterLength_)) | 0x01); //对端I格式报文接收计数器

		size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识

		setbIgnoreSynTime(false);

		if (FrameType == M_CS_NA_1)
		{
			return 0;
		}
		else
		{
			stSynTimePara para = getIgnorePara();

			if (((abs(OppositeRecvCounter - para.IFrameSendCounter_) % 0xfe) == 2) && (para.IFrameRecvCounter_ == OppositeSendCounter))
			{
				return 0;
			}
		}
	}

	myframe frame;

	for (size_t i=0;i < exceptedBytes;i++)
	{
		frame.push_back(buf[i]);
	}

	CmdConSig_(TRANSMIT_FRAME,RETURN_CODE_ACTIVE,terminalPtr,frame);

	return 0;
}

bool CH104_Transmit::getbIgnoreSynTime()
{
	return bIgnoreSynTime_;
}

int CH104_Transmit::setbIgnoreSynTime(bool val)
{
	bIgnoreSynTime_ = val;

	return 0;
}

int CH104_Transmit::AssembleSynTime(size_t bufIndex, unsigned char * buf, stSynTimePara para,boost::posix_time::ptime time)
{
	size_t count = bufIndex;

	buf[count++] = 0x68;
	count += ValToBuf(&buf[count],0,FrameLenLength_);
	count += ValToBuf(&buf[count],para.IFrameSendCounter_,2);
	count += ValToBuf(&buf[count],para.IFrameRecvCounter_,2);
	count += ValToBuf(&buf[count],M_CS_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],para.addr_,AsduAddrLength_);
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

	int len = count - bufIndex - (FrameLenLength_ + SYN_HEAD_LENGTH);
	ValToBuf(&buf[FrameLenLocation_],len,FrameLenLength_);

	return count - bufIndex;
}

int CH104_Transmit::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CH104_Transmit::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	switch(cmd.getCmdType())
	{
	case SYN_TIME_ACT:
		{
			stSynTimePara para;

			try
			{
				para = boost::any_cast<stSynTimePara>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::cerr<<"CH101_Transmit::AssembleFrameBody "<<e.what()<<std::endl;
				return -1;
			}

			int bytesAssemble = AssembleSynTime(bufIndex,buf,para,boost::posix_time::microsec_clock::local_time());
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());
				setbIgnoreSynTime(true);
				setIgnorePara(para);

				count = count + bytesAssemble;
			}
		}
		break;

	case TRANSMIT_FRAME:
		{
			myframe frame;

			try
			{
				frame = boost::any_cast<myframe>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::cerr<<"CH104_Transmit::AssembleFrameBody "<<e.what()<<std::endl;
				return -1;
			}

			for(myframe::iterator it = frame.begin();it != frame.end();it++)
			{
				buf[count++] = *it;
			}
		}
		break;

	default:
		break;
	}

	return count - bufIndex;
}

int CH104_Transmit::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CH104_Transmit::LoadXmlCfg(std::string filename)
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

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CH104_Transmit::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();

	CProtocol::SaveXmlCfg(xml);

	xml.AddElem(strFrameElemLength);
	xml.IntoElem();

	bool bSave = false;

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

	xml.OutOfElem();

	xml.Save(filename);
}

int CH104_Transmit::setFrameLenLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameLenLength_ = val;

	return 0;
}

int CH104_Transmit::setIGramCounterLength(unsigned short val)
{
	if (val < 0 || val > 4)
	{
		return -1;
	}

	IGramCounterLength_ = val;

	return 0;
}

int CH104_Transmit::setFrameTypeLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameTypeLength_ = val;

	return 0;
}

int CH104_Transmit::setInfoNumLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	InfoNumLength_ = val;

	return 0;
}

int CH104_Transmit::setTransReasonLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	TransReasonLength_ = val;

	return 0;
}

int CH104_Transmit::setAsduAddrLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	AsduAddrLength_ = val;

	return 0;
}

int CH104_Transmit::setInfoAddrLength(unsigned short val)
{
	if (val <= 0 || val > 12)
	{
		return -1;
	}

	InfoAddrLength_ = val;

	return 0;
}

typeFrameCounter CH104_Transmit::FrameCounterPlus(typeFrameCounter OriCounter,int plus_num)
{
	return ((OriCounter + plus_num * 2) % 0xfffe);
}

};//namespace Protocol
