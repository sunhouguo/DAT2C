//#include <boost/algorithm/string/trim.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/scoped_array.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "SCdt_Transmit.h"
#include "../FileSystem/Markup.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/PriStation.h"

namespace Protocol {

const std::string strDefaultCfg = "SCdt_TransmitCfg.xml";

//同步头值
const unsigned char SYN_HEAD_L = 0xeb;
const unsigned char SYN_HEAD_H = 0x90;

//报文元素长度
const unsigned char SYN_HEAD_LEN = 6;
const unsigned char CONTROL_CODE_LEN = 6;
const unsigned char INFO_DATA_LEN = 6;

//帧类型
const unsigned char YK_SEL_FRAME = 0x61;
const unsigned char YK_EXE_FRAME = 0xc2;
const unsigned char YK_CANCEL_FRAME = 0xb3;
const unsigned char SYN_TIME_FRAME = 0x7a;

//功能码
const unsigned char SYN_TIME_CODE_L = 0xee;
const unsigned char SYN_TIME_CODE_H = 0xef;


const size_t NormalYkFrameLength = 30;

CSCdt_Transmit::CSCdt_Transmit(boost::asio::io_service & io_service)
	:CProtocol(io_service)
{
	SynCharNum_ = SYN_HEAD_LEN + CONTROL_CODE_LEN;

	LoadXmlCfg(strDefaultCfg);

	EnablePubKey();
}

CSCdt_Transmit::~CSCdt_Transmit(void)
{
}

void CSCdt_Transmit::InitFrameLocation(size_t frameHead)
{
	ControlByteLocation_ = frameHead;
	FrameTypeLocation_ = ControlByteLocation_ + 1;
	InfoNumLocation_ = FrameTypeLocation_ + 1;
	SrcAddrLocation_ = InfoNumLocation_ + 1;
	DstAddrLocation_ = SrcAddrLocation_ + 1;
	ControlCrcLocation_ = DstAddrLocation_ + 1;
}

int CSCdt_Transmit::MatchFrameHead( size_t & exceptedBytes )
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

int CSCdt_Transmit::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	if(CheckPubKey())
	{
		int count = 0;
		if ((buf[count++] == SYN_HEAD_L) && (buf[count++] == SYN_HEAD_H)
			&&(buf[count++] == SYN_HEAD_L) && (buf[count++] == SYN_HEAD_H)
			&&(buf[count++] == SYN_HEAD_L) && (buf[count++] == SYN_HEAD_H))
		{
			size_t control_byte = BufToVal(&buf[ControlByteLocation_],1);
			size_t frame_type = BufToVal(&buf[FrameTypeLocation_],1);
			size_t info_num = BufToVal(&buf[InfoNumLocation_],1);

			exceptedBytes = SYN_HEAD_LEN + CONTROL_CODE_LEN + INFO_DATA_LEN * info_num;

			if (frame_type == YK_SEL_FRAME || frame_type == YK_EXE_FRAME || frame_type == YK_CANCEL_FRAME)
			{
				exceptedBytes += CalcSecretDataLength(buf,exceptedBytes,false);
			}

			return 0;
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

int CSCdt_Transmit::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

int CSCdt_Transmit::ParseSynTime(unsigned char * buf,share_pristation_ptr pristationPtr)
{
	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();

	boost::posix_time::time_duration td = lt.time_of_day();
	unsigned short millisecond = td.total_milliseconds() % 1000;
	unsigned char Second = td.seconds() & 0x3f;
	unsigned char minute = td.minutes() & 0x3f;
	unsigned char Hour = td.hours() & 0x1f;

	boost::gregorian::date::ymd_type ymd = lt.date().year_month_day();
	unsigned char Day = ymd.day & 0x1f;
	unsigned char Month = ymd.month & 0x0f;
	unsigned short Year = ymd.year;

	bool bRecvTime = false;
	size_t info_num = BufToVal(&buf[InfoNumLocation_],1);
	for (size_t i=0;i<info_num;i++)
	{
		unsigned short DataLocation = SYN_HEAD_LEN + CONTROL_CODE_LEN + i*INFO_DATA_LEN;
		unsigned char CalcCRC = CRC8_CDT(&buf[DataLocation],INFO_DATA_LEN - 1);
		unsigned char FrameCRC = BufToVal(&buf[DataLocation + INFO_DATA_LEN - 1],1);
		if (CalcCRC == FrameCRC)
		{
			size_t funCode = BufToVal(&buf[DataLocation],1);
			if (funCode == SYN_TIME_CODE_L)
			{
				unsigned char count = 1;
				unsigned char millisecond_l = buf[DataLocation + count++];
				unsigned char millisecond_h = buf[DataLocation + count++] & 0x03;
				millisecond = millisecond_l + (millisecond_h * 0x100);
				Second = buf[DataLocation + count++] & 0x3f;
				minute = buf[DataLocation + count++] & 0x3f;

				bRecvTime = true;
			}
			else if(funCode == SYN_TIME_CODE_H)
			{
				unsigned char count = 1;
				Hour = buf[DataLocation + count++] & 0x1f;
				Day = buf[DataLocation + count++] & 0x1f;
				Month = buf[DataLocation + count++] & 0x0f;
				Year = ByteYearToWord(buf[DataLocation + count++] & 0x7f);

				bRecvTime = true;
			}
		}
	}

	if (bRecvTime)
	{
		pristationPtr->WriteSysTime(Year,Month,Day,Hour,minute,Second,millisecond,false);
	}

	return 0;
}

int CSCdt_Transmit::ParseYkSign(unsigned char * buf,share_pristation_ptr pristationPtr,size_t exceptedBytes)
{
	size_t info_num = BufToVal(&buf[InfoNumLocation_],1);
	int OriFrameLength = SYN_HEAD_LEN + CONTROL_CODE_LEN + INFO_DATA_LEN * info_num;

	if (decrypt(getSignStartIndex(),exceptedBytes - getSignStartIndex(),OriFrameLength - getSignStartIndex(),buf))
	{
		std::ostringstream ostr;
		ostr<<"YK报文错误，未认证的数字签名"<<std::endl;
		std::cerr<<ostr.str();

		//boost::scoped_array<unsigned char> temp_buf(new unsigned char[exceptedBytes + 1]);
		//memcpy(temp_buf.get(),buf,exceptedBytes); //拷贝一个报文镜像

		//ValToBuf(&temp_buf.get()[4],M_AV_NA_3 | DIR_PRM,1); //修改控制域
		//ValToBuf(&(temp_buf.get()[TransReasonLocation_]) ,trans_ykfailed,TransReasonLength_);//修改传送原因
		//ValToBuf(&(temp_buf.get()[OriFrameLength - 2]),CalcCheckSumByte(&temp_buf.get()[4],OriFrameLength - 6),1); //修改效验和
		//
		//myframe frame;

		//for (int i=0;i < OriFrameLength;i++)
		//{
		//	frame.push_back(temp_buf[i]);
		//}

		//CCmd TransmitCmd(TRANSMIT_FRAME,TRANSMIT_FRAME_PRIORITY,pristationPtr,frame);

		//AddSendCmdVal(TransmitCmd);

		return -1;
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

int CSCdt_Transmit::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	share_pristation_ptr pristationPtr = boost::dynamic_pointer_cast<DataBase::CPriStation>(getFirstCommPoint());
	
	if (!pristationPtr)
	{
		return -1;
	}

	if(CheckPubKey())
	{
		size_t frame_type = BufToVal(&buf[FrameTypeLocation_],1);

		if (frame_type == SYN_TIME_FRAME)
		{
			ParseSynTime(buf,pristationPtr);
		}
		else if (frame_type == YK_SEL_FRAME || frame_type == YK_EXE_FRAME || frame_type == YK_CANCEL_FRAME)
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

int CSCdt_Transmit::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CSCdt_Transmit::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
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
				std::cerr<<"CSCdt_Transmit::AssembleFrameBody "<<e.what()<<std::endl;
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

int CSCdt_Transmit::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	return count - bufIndex;
}

int CSCdt_Transmit::LoadXmlCfg(std::string filename)
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

void CSCdt_Transmit::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();

	CProtocol::SaveXmlCfg(xml);
}

void CSCdt_Transmit::ProcessSubAliveSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
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

int CSCdt_Transmit::InitProtocol()
{
	ConnectSubAliveSig();

	CProtocol::InitProtocol();

	InitFrameLocation(SYN_HEAD_LEN);

	AddStatusLogWithSynT("SCdt_Transmit规约的通道打开成功。\n");

	return 0;
}

void CSCdt_Transmit::UninitProtocol()
{
	DisconnectSubAliveSig();

	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("SCdt_Transmit规约的通道关闭成功。\n");
}

};//namespace Protocol
