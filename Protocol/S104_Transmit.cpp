#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>
#include "S104_Transmit.h"
#include "../FileSystem/Markup.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/PriStation.h"
#include "H104_Transmit.h"

namespace Protocol {

const std::string strDefaultCfg = "S104_TransmitCfg.xml";

//针对104规约的报文类型标识定义
const unsigned char M_SC_NA_1 = 0x2d;
const unsigned char M_DC_NA_1 = 0x2e;
const unsigned char M_CS_NA_1 = 0x67;

//针对104规约的传送原因定义
const unsigned char trans_ykfailed = 0x30;

const std::string strIGramCounterLength = "IGramCounterLength";

const unsigned char SYN_HEAD_LENGTH = 1;
const size_t NormalYkFrameLength = 16;

CS104_Transmit::CS104_Transmit(boost::asio::io_service & io_service)
	:CProtocol(io_service)
{
	SynCharNum_ = 6;

	InitDefaultFrameElem();

	LoadXmlCfg(strDefaultCfg);

	EnablePubKey();
}

CS104_Transmit::~CS104_Transmit(void)
{
}

void CS104_Transmit::InitDefaultFrameElem()
{
	FrameLenLength_ = DEFAULT_FrameLenLength;                               //报文长度标识的字节长度
	IGramCounterLength_ = DEFAULT_IGramCounterLength;
	FrameTypeLength_ = DEFAULT_FrameTypeLength;                             //报文类型标识的字节长度
	InfoNumLength_ =   DEFAULT_InfoNumLength;                               //信息体数目标识的字节长度
	TransReasonLength_ = DEFAULT_TransReasonLength;                         //传送原因标识的字节长度
	AsduAddrLength_ = DEFAULT_AsduAddrLength;                               //装置地址标识的字节长度
	InfoAddrLength_ = DEFAULT_InfoAddrLength;                               //信息体地址标识的字节长度
}

void CS104_Transmit::InitFrameLocation(size_t frameHead)
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

int CS104_Transmit::MatchFrameHead( size_t & exceptedBytes )
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

int CS104_Transmit::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	if(CheckPubKey())
	{
		if (buf[0] == 0x68)
		{
			int len = BufToVal(&buf[FrameLenLocation_],FrameLenLength_);
			if(len >= 4)
			{
				exceptedBytes = len + FrameLenLength_ + SYN_HEAD_LENGTH;

				size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识
				if (FrameType == M_SC_NA_1 || FrameType == M_DC_NA_1)
				{
					exceptedBytes += CalcSecretDataLength(buf,exceptedBytes);
				}

				return 0;
			}
		}
		else if(CheckPlusHead(buf,exceptedBytes) == 0)
		{
			return 0;
		}
	}
	else
	{
		exceptedBytes = getFrameBufLeft();

		return 0;
	}

	return -1;
}

int CS104_Transmit::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

int CS104_Transmit::ParseSynTime(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();

	unsigned short millisecond = BufToVal(&buf[DataLocation_],2);
	unsigned char minute = buf[DataLocation_ + 2] & 0x3f;
	unsigned char Hour = buf[DataLocation_ + 3] & 0x1f;
	unsigned char Day = buf[DataLocation_ + 4] & 0x1f;
	unsigned char Month = buf[DataLocation_ + 5] & 0x0f;
	unsigned short Year = ByteYearToWord(buf[DataLocation_ + 6] & 0x7f);

	pristationPtr->WriteSysTime(Year,Month,Day,Hour,minute,(millisecond / 1000),(millisecond % 1000),false);

	return 0;
}

int CS104_Transmit::ParseYkSign(unsigned char * buf,share_pristation_ptr pristationPtr,size_t exceptedBytes)
{
	int OriFrameLength = BufToVal(&buf[FrameLenLocation_],FrameLenLength_) + FrameLenLength_ + SYN_HEAD_LENGTH;

	if (decrypt(getSignStartIndex(),exceptedBytes - getSignStartIndex(),OriFrameLength - getSignStartIndex(),buf))
	{
		std::ostringstream ostr;
		ostr<<"YK报文错误，未认证的数字签名"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		boost::scoped_array<unsigned char> temp_buf(new unsigned char[exceptedBytes + 1]);
		memcpy(temp_buf.get(),buf,exceptedBytes);

		stSynTimePara para;
		para.IFrameSendCounter_ = ~(~(BufToVal(&buf[RecvCounterLocation_],IGramCounterLength_)) | 0x01); //对端I格式报文发送计数器
		para.IFrameRecvCounter_ = ~(~(BufToVal(&buf[SendCounterLocation_],IGramCounterLength_)) | 0x01); //对端I格式报文接收计数器
		para.addr_ = BufToVal(&buf[AsduAddrLocation_],AsduAddrLength_);               //RTU地址

		CCmd SynTimeCmd(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,pristationPtr,para);
		pristationPtr->AddBF533Cmd(0,SynTimeCmd);

		typeFrameCounter isend = para.IFrameRecvCounter_;
		typeFrameCounter irecv = CH104_Transmit::FrameCounterPlus(para.IFrameSendCounter_,1);
		ValToBuf(&(temp_buf.get()[TransReasonLocation_]) ,trans_ykfailed,TransReasonLength_);
		ValToBuf(&(temp_buf.get()[RecvCounterLocation_]),isend,IGramCounterLength_);
		ValToBuf(&(temp_buf.get()[SendCounterLocation_]),irecv,IGramCounterLength_);

		myframe frame;
		for (int i=0;i < OriFrameLength;i++)
		{
			frame.push_back(temp_buf[i]);
		}
		
		CCmd TransmitCmd(TRANSMIT_FRAME,TRANSMIT_FRAME_PRIORITY,pristationPtr,frame);
		AddSendCmdVal(TransmitCmd);
	}
	else
	{
		myframe frame;

		for (int i=0;i < OriFrameLength;i++)
		{
			frame.push_back(buf[i]);
		}

		CCmd TransmitCmd(TRANSMIT_FRAME,TRANSMIT_FRAME_PRIORITY,pristationPtr,frame);

		pristationPtr->AddBF533Cmd(0,TransmitCmd);
	}

	return 0;
}

int CS104_Transmit::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(getFirstCommPoint());

	if (!pristationPtr)
	{
		return -1;
	}

	if(CheckPubKey())
	{
		size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识

		if (FrameType == M_CS_NA_1)
		{
			ParseSynTime(buf,pristationPtr);
		}
		else if (FrameType == M_SC_NA_1 || FrameType == M_DC_NA_1)
		{
			return ParseYkSign(buf,pristationPtr,exceptedBytes);
		}
		else if((buf[0] == 0x16) && (buf[3] == 0x16) && (buf[1] >= buf[2]))
		{
			return ParseFrame_Plus(buf,exceptedBytes);
		}
	}

	myframe frame;

	for (size_t i=0;i < exceptedBytes;i++)
	{
		frame.push_back(buf[i]);
	}

	CCmd TransmitCmd(TRANSMIT_FRAME,TRANSMIT_FRAME_PRIORITY,pristationPtr,frame);

	pristationPtr->AddBF533Cmd(0,TransmitCmd);

	return 0;
}

int CS104_Transmit::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CS104_Transmit::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	switch(cmd.getCmdType())
	{
	case CHECK_PUB_KEY:
		try
		{
			bool bConAct = boost::any_cast<bool>(cmd.getVal());
			count = AssembleCheckPubKeyCon(bufIndex,buf,cmd.getCommPoint(),bConAct);
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
			count = AssembleUpdatePubKeyCon(bufIndex,buf,cmd.getCommPoint(),bConAct);
		}
		catch(const boost::bad_any_cast & e)
		{
			std::ostringstream ostr;
			ostr<<"更新公钥回应命令的参数非法："<<e.what()<<std::endl;
			AddStatusLogWithSynT(ostr.str());
			return -1;
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
				std::cerr<<"CS104_Transmit::AssembleFrameBody "<<e.what()<<std::endl;
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

int CS104_Transmit::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CS104_Transmit::LoadXmlCfg(std::string filename)
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

void CS104_Transmit::SaveXmlCfg(std::string filename)
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

int CS104_Transmit::setFrameLenLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameLenLength_ = val;

	return 0;
}

int CS104_Transmit::setIGramCounterLength(unsigned short val)
{
	if (val < 0 || val > 4)
	{
		return -1;
	}

	IGramCounterLength_ = val;

	return 0;
}

int CS104_Transmit::setFrameTypeLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameTypeLength_ = val;

	return 0;
}

int CS104_Transmit::setInfoNumLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	InfoNumLength_ = val;

	return 0;
}

int CS104_Transmit::setTransReasonLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	TransReasonLength_ = val;

	return 0;
}

int CS104_Transmit::setAsduAddrLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	AsduAddrLength_ = val;

	return 0;
}

int CS104_Transmit::setInfoAddrLength(unsigned short val)
{
	if (val <= 0 || val > 12)
	{
		return -1;
	}

	InfoAddrLength_ = val;

	return 0;
}

void CS104_Transmit::ProcessSubAliveSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	switch (cmdType)
	{
	case TRANSMIT_FRAME:
		for (int i=0;i<getCommPointSum();i++)
		{
			share_commpoint_ptr commPoint = getCommPoint(i).lock();
			if (commPoint)
			{
				if (commPoint->getCommPointType() == PRISTATION_NODE)
				{
					AddSendCmdVal(TRANSMIT_FRAME,TRANSMIT_FRAME_PRIORITY,commPoint,val);
				}
			}
		}
		break;

	default:
		break;
	}
}

int CS104_Transmit::InitProtocol()
{
	CProtocol::InitProtocol();

	InitFrameLocation(SYN_HEAD_LENGTH);

	share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(getFirstCommPoint());

	pristationPtr->AddBF533Cmd(1,CCmd(RESET_CMD,RESET_CMD_PRIORITY,pristationPtr));

	ConnectSubAliveSig();

	AddStatusLogWithSynT("S104_Transmit规约的通道打开成功。\n");

	return 0;
}

void CS104_Transmit::UninitProtocol()
{
	DisconnectSubAliveSig();

	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("S104_Transmit规约的通道关闭成功。\n");
}

};//namespace Protocol
