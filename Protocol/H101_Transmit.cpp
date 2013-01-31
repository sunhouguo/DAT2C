#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include "H101_Transmit.h"
#include "../FileSystem/Markup.h"
//#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/Terminal.h"

namespace Protocol {

const std::string strDefaultCfg = "H101_TransmitCfg.xml";

CH101_Transmit::CH101_Transmit(boost::asio::io_service & io_service)
	:CProtocol(io_service)
{
	SynCharNum_ = 5;

	InitDefaultFrameElem();

	LoadXmlCfg(strDefaultCfg);

	InitFrameLocation(5 + AsduAddrLength_);
}

CH101_Transmit::~CH101_Transmit(void)
{
}

void CH101_Transmit::InitDefaultFrameElem()
{
	FrameTypeLength_ = DEFAULT_FrameTypeLength;                             //报文类型标识的字节长度
	InfoNumLength_ =   DEFAULT_InfoNumLength;                               //信息体数目标识的字节长度
	TransReasonLength_ = DEFAULT_TransReasonLength;                         //传送原因标识的字节长度
	AsduAddrLength_ = DEFAULT_AsduAddrLength;                               //装置地址标识的字节长度
	InfoAddrLength_ = DEFAULT_InfoAddrLength;                               //信息体地址标识的字节长度
}

void CH101_Transmit::InitFrameLocation(size_t frameHead)
{
	FrameTypeLocation_ = frameHead;
	InfoNumLocation_ = FrameTypeLocation_ + FrameTypeLength_;
	TransReasonLocation_ = InfoNumLocation_ + InfoNumLength_;
	AsduAddrLocation_ = TransReasonLocation_ + TransReasonLength_;
	InfoAddrLocation_ = AsduAddrLocation_ + AsduAddrLength_;
	DataLocation_ = InfoAddrLocation_ + InfoAddrLength_;
}

int CH101_Transmit::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	exceptedBytes = getFrameBufLeft();

	return 0;
}

int CH101_Transmit::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

int CH101_Transmit::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	share_terminal_ptr terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(getFirstCommPoint());

	if (!terminalPtr)
	{
		return -1;
	}

	myframe frame;

	for (size_t i=0;i < exceptedBytes;i++)
	{
		frame.push_back(buf[i]);
	}

	CmdConSig_(TRANSMIT_FRAME,RETURN_CODE_ACTIVE,terminalPtr,frame);

	return 0;
}

int CH101_Transmit::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CH101_Transmit::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	switch(cmd.getCmdType())
	{
	case TRANSMIT_FRAME:
		{
			myframe frame;

			try
			{
				frame = boost::any_cast<myframe>(cmd.getVal());
			}
			catch(const boost::bad_any_cast & e)
			{
				std::cerr<<"CH101_Transmit::AssembleFrameBody "<<e.what()<<std::endl;
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

int CH101_Transmit::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CH101_Transmit::LoadXmlCfg(std::string filename)
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

void CH101_Transmit::SaveXmlCfg(std::string filename)
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

int CH101_Transmit::setFrameTypeLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameTypeLength_ = val;

	return 0;
}

int CH101_Transmit::setInfoNumLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	InfoNumLength_ = val;

	return 0;
}

int CH101_Transmit::setTransReasonLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	TransReasonLength_ = val;

	return 0;
}

int CH101_Transmit::setAsduAddrLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	AsduAddrLength_ = val;

	return 0;
}

int CH101_Transmit::setInfoAddrLength(unsigned short val)
{
	if (val <= 0 || val > 12)
	{
		return -1;
	}

	InfoAddrLength_ = val;

	return 0;
}

};//namespace Protocol
