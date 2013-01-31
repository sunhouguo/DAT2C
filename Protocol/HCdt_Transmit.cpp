#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include "HCdt_Transmit.h"
#include "../FileSystem/Markup.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/Terminal.h"

namespace Protocol {

const std::string strDefaultCfg = "HCdt_TransmitCfg.xml";

//同步头值
const unsigned char SYN_HEAD_L = 0xeb;
const unsigned char SYN_HEAD_H = 0x90;

//报文元素长度
const unsigned char SYN_HEAD_LEN = 6;
const unsigned char CONTROL_CODE_LEN = 6;
const unsigned char INFO_DATA_LEN = 6;

CHCdt_Transmit::CHCdt_Transmit(boost::asio::io_service & io_service)
	:CProtocol(io_service)
{
	SynCharNum_ = SYN_HEAD_LEN + CONTROL_CODE_LEN;

	LoadXmlCfg(strDefaultCfg);

	InitFrameLocation(SYN_HEAD_LEN);
}

CHCdt_Transmit::~CHCdt_Transmit(void)
{
}

int CHCdt_Transmit::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	//exceptedBytes = getFrameBufLeft();

	int count = 0;
	if ((buf[count++] == SYN_HEAD_L) && (buf[count++] == SYN_HEAD_H)
		&&(buf[count++] == SYN_HEAD_L) && (buf[count++] == SYN_HEAD_H)
		&&(buf[count++] == SYN_HEAD_L) && (buf[count++] == SYN_HEAD_H))
	{
		size_t control_byte = BufToVal(&buf[ControlByteLocation_],1);
		size_t frame_type = BufToVal(&buf[FrameTypeLocation_],1);
		size_t info_num = BufToVal(&buf[InfoNumLocation_],1);

		exceptedBytes = SYN_HEAD_LEN + CONTROL_CODE_LEN + INFO_DATA_LEN * info_num;

		return 0;
	}

	return -1;
}

int CHCdt_Transmit::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

int CHCdt_Transmit::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
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

int CHCdt_Transmit::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CHCdt_Transmit::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
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
				std::cerr<<"CHCdt_Transmit::AssembleFrameBody "<<e.what()<<std::endl;
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

int CHCdt_Transmit::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CHCdt_Transmit::LoadXmlCfg(std::string filename)
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

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CHCdt_Transmit::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();

	CProtocol::SaveXmlCfg(xml);
}

void CHCdt_Transmit::InitFrameLocation(size_t frameHead)
{
	ControlByteLocation_ = frameHead;
	FrameTypeLocation_ = ControlByteLocation_ + 1;
	InfoNumLocation_ = FrameTypeLocation_ + 1;
	SrcAddrLocation_ = InfoNumLocation_ + 1;
	DstAddrLocation_ = SrcAddrLocation_ + 1;
	ControlCrcLocation_ = DstAddrLocation_ + 1;
}

};//namespace Protocol
