#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "H103_DigiproII.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/Terminal.h"
#include "../DataBase/YxPoint.h"
#include "../DataBase/YkPoint.h"

namespace Protocol {

const std::string strDefaultCfg = "H103_DigiproIICfg.xml";
size_t CH103_DigiproII::H103ObjectCounter_ = 1;

const unsigned char contrl = 0x40;

//自定义类型标识
const unsigned char C_SYN_TA_3 = 0x00;
const unsigned char C_IGI_NA_3 = 0x01;
const unsigned char EVENT = 0x02;
const unsigned char YK_SEL = 0x05;
const unsigned char YK_EXE = 0x06;

CH103_DigiproII::CH103_DigiproII(boost::asio::io_service & io_service)
								:CProtocol(io_service)
{
	bActiveRepeatFrame_ = true;
	SynCharNum_ = 5;

	InitObjectIndex();
	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);

}

CH103_DigiproII::~CH103_DigiproII(void)
{
	H103ObjectCounter_--;
}

int CH103_DigiproII::LoadXmlCfg(std::string filename)
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

void CH103_DigiproII::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();   //enter strProtocolRoot

	CProtocol::SaveXmlCfg(xml);

	xml.OutOfElem();  //out strProtocolRoot

	xml.Save(filename); 
}

int CH103_DigiproII::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	if ((buf[0] == 0x68) && (buf[3] == 0x68) && (buf[1] == buf[2]))
	{
		exceptedBytes = buf[1] + 6;
		return 0;
	}

	return -1;
}

int CH103_DigiproII::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	size_t sum = 0;

	if ((exceptedBytes > 5) && (buf[0] == 0x68) && (buf[3] == 0x68))
	{
		sum = CalcCheckSumByte(&buf[4],exceptedBytes - 6);
	}

	if ((buf[exceptedBytes -1] == 0x16) && (buf[exceptedBytes - 2] == sum))
	{
		return 0;
	}

	return -1;
}

int CH103_DigiproII::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	int ret = 0;

	int Addr = buf[5];

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
		ostr<<"H103_DigiproII规约不能根据接收报文中的地址匹配terminal ptr,这帧报文将不会被解析。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	size_t FrameType = BufToVal(&buf[6],1);        //类型标识
	switch(FrameType)
	{
	case C_IGI_NA_3:
		ret = ParseAllData(buf,terminalPtr);

		break;

	case EVENT:
		ret = ParseAllEvent(buf,terminalPtr);
		AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
		if (ret > 0)
		{
			setWaitingForAnswer(terminalPtr);
			ResetTimerCallAllEvent(terminalPtr,true);
		}
		break;

	case YK_SEL:
		ret = ParseYkSelCon(buf,terminalPtr);
		break;

	default:
		{
			std::ostringstream ostr;
			ostr<<"接收报文错误，未定义的报文类型 FRAME_TYPE ="<<FrameType<<std::endl;
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

int CH103_DigiproII::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	buf[count++] = 0x68;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0x68;
	buf[count++] = contrl;

	return count - bufIndex;
}

int CH103_DigiproII::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
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
		ostr<<"H103_DigiproII规约不能从发送命令中获得terminal ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch(cmd.getCmdType())
	{
	case CALL_ALL_DATA_ACT:
		bytesAssemble = AssembleCallAllData(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerCallAllData(terminalPtr,true);
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

	case CONFIRM_ACK:
		bytesAssemble = AssembleEventCon(bufIndex,buf,terminalPtr);
		break;

	//case YK_SEL:
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

			unsigned char yk_obj;
			unsigned char yk_code;
			if (TransYkCmdToPara(yk_no,yk_type,yk_obj,yk_code) != 0)
			{
				std::ostringstream ostr;
				ostr<<"遥控点号:"<<yk_no<<"遥控分合位:"<<yk_type<<"，不能将该遥控选择命令参数转换为规约报文参数。"<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			bytesAssemble = AssembleYKSel(bufIndex,buf,terminalPtr,yk_obj,yk_code);

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

	//case YK_EXE:
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

			unsigned char yk_obj;
			unsigned char yk_code;
			if (TransYkCmdToPara(yk_no,yk_type,yk_obj,yk_code) != 0)
			{
				std::ostringstream ostr;
				ostr<<"遥控点号:"<<yk_no<<"遥控分合位:"<<yk_type<<"，不能将该遥控执行命令参数转换为规约报文参数。"<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			bytesAssemble = AssembleYKExe(bufIndex,buf,terminalPtr,yk_obj,yk_code);

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
				//terminalPtr->setYkStatus(yk_no,DataBase::YkExeCon);

				CmdConSig_(YK_EXE_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);
			}
		}
		break;
	}

	return bytesAssemble;
}

int CH103_DigiproII::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	int length = bufIndex - bufBegin;
	if (length <= 1)
	{
		return -1;
	}

	int framelength = length - 4;
	if (framelength <= 0 || framelength > max_frame_length_)
	{
		return -1;
	}

	size_t count = bufIndex;

	buf[count++] = CalcCheckSumByte(&buf[bufBegin + 4],framelength);
	buf[count++] = 0x16;
	buf[bufBegin + 1] = framelength & 0xff;
	buf[bufBegin + 2] = framelength & 0xff;

	return count - bufIndex;
}

int CH103_DigiproII::InitProtocol()
{
	CProtocol::InitProtocol();

	if(getCommPointSum() > 0)
	{
		share_commpoint_ptr nextPoint = getFirstCommPoint();
		if (nextPoint)
		{
			AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,nextPoint);
			AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,nextPoint);
		}
	}

	AddStatusLogWithSynT("H103_DigiproII规约的通道打开成功。\n");

	return 0;
}

void CH103_DigiproII::UninitProtocol()
{
	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("H103_DigiproII规约的通道关闭成功。\n");
}

void CH103_DigiproII::InitObjectIndex()
{
	ProtocolObjectIndex_ = H103ObjectCounter_++;
}

void CH103_DigiproII::InitDefaultTimeOut()
{
	timeOutCallAllData_ = DEFAULT_timeOutCallAllData;
	timeOutSynTime_ = DEFAULT_timeOutSynTime;
	timeOutYkSel_ = DEFAULT_timeOutYkSel;
	timeOutCallAllEvent_ = DEFAULT_timeOutCallAllEvent;
}

void CH103_DigiproII::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	timerCallAllData_.reset(new deadline_timer(io_service,seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllData,timeOutCallAllData_))));
	AddTimer(timerCallAllData_);

	timerSynTime_.reset(new deadline_timer(io_service,seconds(getMeanvalueOfPointsSum(MIN_timeOutSynTime,timeOutSynTime_))));
	AddTimer(timerSynTime_);

	timerYkSel_.reset(new deadline_timer(io_service,seconds(timeOutYkSel_)));
	AddTimer(timerYkSel_);

	timerCallAllEvent_.reset(new deadline_timer(io_service,seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllEvent,timeOutCallAllEvent_))));
	AddTimer(timerCallAllEvent_);
}

int CH103_DigiproII::AssembleSynTime(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr, boost::posix_time::ptime time)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],BroadCastAddr,1);
	count += ValToBuf(&buf[count],C_SYN_TA_3,1);
	boost::posix_time::time_duration td = time.time_of_day();
	buf[count++] = td.hours() & 0x1f;
	buf[count++] = td.minutes() & 0x3f;
	buf[count++] = td.seconds() & 0xff;
	boost::gregorian::date::ymd_type ymd = time.date().year_month_day();
	buf[count++] = ymd.year % 100;
	buf[count++] = ymd.month & 0x0f;
	buf[count++] = ymd.day & 0x1f;

	return count - bufIndex;
}

int CH103_DigiproII::AssembleCallAllData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],terminalPtr->getAddr(),1);
	count += ValToBuf(&buf[count],C_IGI_NA_3,1);

	return count -bufIndex;
}

int CH103_DigiproII::AssembleYKSel( size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char yk_obj, unsigned char yk_code )
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],terminalPtr->getAddr(),1);
	count += ValToBuf(&buf[count],YK_SEL,1);
	buf[count++] = yk_obj;
	buf[count++] = yk_code;

	return count - bufIndex;
}

int CH103_DigiproII::AssembleYKExe( size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char yk_obj, unsigned char yk_code )
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],terminalPtr->getAddr(),1);
	count += ValToBuf(&buf[count],YK_EXE,1);
	buf[count++] = yk_obj;
	buf[count++] = yk_code;

	return count - bufIndex;
}

int CH103_DigiproII::AssembleEventCon(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],terminalPtr->getAddr(),1);
	count += ValToBuf(&buf[count],EVENT,1);

	return count - bufIndex;
}

int CH103_DigiproII::ParseAllData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int DataLocation = 8;
	//int frameLength = buf[1] - 2;
	int frameLength = buf[1] + 4;

	size_t count = 0;
	if (frameLength - DataLocation >= 26*2)
	{
		for (int i=0;i<26;i++)
		{
			if (i < (int)terminalPtr->getYcSum())
			{
				unsigned short ycVal = BufToVal(&buf[DataLocation],2);
				unsigned char yc_valid = 0x00;                         //此处添加品质描述
				//bool bCheckYcVar = true;
				terminalPtr->SaveYcQuality(i,yc_valid);        //此处保存品质描述

				//int ret = terminalPtr->SaveOriYcVal(i,ycVal,terminalPtr->getInitCommPointFlag());
				int ret = terminalPtr->SaveOriYcVal(i,ycVal,true);
				if (ret == DataBase::CauseActiveData)
				{
					count++;
				}
			}
			
			DataLocation += 2;
		}
		if (count > 0)
		{
			std::ostringstream ostr;
			ostr<<"YC报文产生了ycvar，数目："<<count<<std::endl;
			AddStatusLogWithSynT(ostr.str());
			CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
		}
	}
	

	if(frameLength - DataLocation >= 6*2)
	{
		for (int i=0;i<6;i++)
		{
			if(i < (int)terminalPtr->getYmSum())
			{
				unsigned short ymVal = BufToVal(&buf[DataLocation],2);
				int ret = terminalPtr->SaveOriYmVal(i,ymVal);
			}

			DataLocation += 2;
		}
	}

	count = 0;
	if (frameLength - DataLocation >= 8)
	{
		for (int i=0;i<8;i++)
		{
			unsigned char YxByteVal = buf[DataLocation];
			for (int j=0;j<8;j++)
			{
				int index = i*8 + j;
				if((index >= 0) && (index < (int)terminalPtr->getYxSum()))
				{
					unsigned char yx_quality = 0x00;
					terminalPtr->SaveYxQuality(index,yx_quality);
					terminalPtr->SaveYxType(index,DataBase::single_yx_point);
					if ((YxByteVal & BYTE_CHECK_TRUE[j]) > 0)
					{
						int ret = terminalPtr->SaveOriYxVal(index,1,true);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
					else
					{
						int ret = terminalPtr->SaveOriYxVal(index,0,true);
						if (ret == DataBase::CauseActiveData)
						{
							count++;
						}
					}
				}
			}

			DataLocation++;
		}

		if (count > 0)
		{
			std::ostringstream ostr;
			ostr<<"全YX字报文产生了COS，数目："<<count<<std::endl;
			AddStatusLogWithSynT(ostr.str());
			CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,count);
		}
	}

	if (frameLength - DataLocation >= 4)
	{
		int hour = buf[DataLocation++];
		int min = buf[DataLocation++];
		int sec = buf[DataLocation++];
		int mecs = buf[DataLocation++] * 10;
	}

	return 0;
}

int CH103_DigiproII::ParseAllEvent(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	
	int DataLocation = 8;
	//int frameLength = buf[1] - 2;
	int frameLength = buf[1] + 4;
	int DataLength = 10;

	using namespace boost::posix_time;
	ptime lt = boost::posix_time::microsec_clock::local_time();

	int info_num = (frameLength - DataLocation) / DataLength;
	if (info_num > 8)
	{
		info_num = 8;
	}

	for (int i=0;i<info_num;i++)
	{
		int KZ_Tpy = BufToVal(&buf[DataLocation + i*10 +7],1);
		int EventData = BufToVal(&buf[DataLocation + i*10 +8],1);
		int EventTpy = BufToVal(&buf[DataLocation + i*10 +6],1);
		int index = 0x00;
		unsigned char YxBitVal = 0x00;

		unsigned char YxQuality = 0x00; 
		unsigned short Year = (buf[DataLocation + i*10 + 1] & 0xf0) + (((lt.date()).year() / 100) * 100);
		unsigned char Month = buf[DataLocation + i*10 + 1] & 0x0f;
		unsigned char Hour = buf[DataLocation + i*10 + 2] & 0x1f;
		unsigned char Day = buf[DataLocation + i*10 + 3] & 0x1f;
		unsigned int second = BufToVal(&buf[DataLocation + i*10 + 4],1);
		unsigned int minute = BufToVal(&buf[DataLocation + i*10 + 5],1);
		unsigned int millisecond = BufToVal(&buf[DataLocation + i*10],1);

		time_duration td(Hour,minute,second,millisecond);

		std::ostringstream ostr;

		boost::gregorian::date dt(Year,Month,Day);
		ptime timeVal(dt,td);

		if((index >= 0) && (index < (int)terminalPtr->getYxSum()))
		{
			if (KZ_Tpy == 0x0a)
			{
				if (EventData == 0x01)
				{
					index = 0x00;//00为遥信点号0的；01为第一屏，遥信点号为65；2=66,3=67,4=68,5=69,6=70,7=71,8=72,9=73,
				}
				else if (EventData == 0x02)
				{
					index = 0x03;
				}

				if (EventTpy == 0x01)
				{
					YxBitVal = 0x01;
				}
				else if (EventTpy == 0x00)
				{
					YxBitVal = 0x00;
				}
				terminalPtr->SaveYxQuality(index,YxQuality);
				terminalPtr->SaveYxType(index,DataBase::single_yx_point);
				terminalPtr->SaveSoePoint(index,YxBitVal,DataBase::single_yx_point,timeVal,YxQuality);
				ostr<<"收到事件添加单点SOE，点号"<<index<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				AddStatusLogWithSynT(ostr.str());

			}
			else
			{
				index = BufToVal(&buf[DataLocation + i*10 +7],1) + 64;
				YxBitVal = 0x01;

				terminalPtr->SaveYxQuality(index,YxQuality);
				terminalPtr->SaveYxType(index,DataBase::single_yx_point);
				terminalPtr->SaveSoePoint(index,YxBitVal,DataBase::single_yx_point,timeVal,YxQuality);
				terminalPtr->SaveCosPoint(index,YxBitVal,DataBase::single_yx_point,YxQuality);

				ostr<<"收到事件添加单点COS，点号"<<index<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<std::endl;
				AddStatusLogWithSynT(ostr.str());

				ostr<<"收到事件添加单点SOE，点号"<<index<<"，YX值"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)YxBitVal<<"："<<to_simple_string(timeVal)<<std::endl;
				AddStatusLogWithSynT(ostr.str());

			}

			DataLocation += DataLength;
		}
	}

	if (info_num > 0)
	{
		CmdConSig_(SOE_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);
		CmdConSig_(COS_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,info_num);

	}

	return 0;
}

int CH103_DigiproII::ParseYkSelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int DataLocation = 8;

	unsigned char yk_obj = buf[DataLocation++];
	unsigned char yk_code = buf[DataLocation++];
	unsigned char yk_con = buf[DataLocation++];

	int yk_no;
	typeYktype yk_type;
	if (TransYkParaToCmd(yk_obj,yk_code,yk_no,yk_type) != 0)
	{
		std::ostringstream ostr;
		ostr<<"遥控对象号:"<<yk_obj<<"遥控码:"<<yk_code<<"，不能将该规约报文参数参数转换为遥控选择命令参数。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	//if (!DataBase::CYkPoint::CheckYkStatusDevelopWithSel(terminalPtr->getYkStatus(yk_no),DataBase::YkSelCon))
	if((terminalPtr->loadYkPointPtr(yk_no))->SelResponEvent())
	{
		//std::ostringstream ostr;
		//ostr<<"curStatus:"<<(int)terminalPtr->getYkStatus(yk_no)<<"NextStatus:"<<(int)DataBase::YkSelCon<<"收到遥控选择返校报文，但是遥控当前遥控状态不符合，退出不处理该帧报文。"<<std::endl;
		//AddStatusLogWithSynT(ostr.str());
		AddStatusLogWithSynT("收到遥控选择返校报文，但是遥控当前遥控状态不符合，退出不处理该帧报文。\n");
		return -1;
	}

	if(terminalPtr->getYkType(yk_no) != yk_type)
	{
		ResetTimerYkSel(terminalPtr,yk_no,false);
		//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
		(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
		CmdConSig_(YK_SEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);
		AddStatusLogWithSynT("遥控选择返校报文的遥控类型与预期不符。\n");
		return -1;
	}

	if (yk_con != 0xff)
	{
		ResetTimerYkSel(terminalPtr,yk_no,false);
		//terminalPtr->setYkStatus(yk_no,DataBase::YkReady);
		(terminalPtr->loadYkPointPtr(yk_no))->ClearYkState();
		CmdConSig_(YK_SEL_CON,RETURN_CODE_NEGATIVE,terminalPtr,yk_no);

		std::ostringstream ostr;
		ostr<<"遥控选择返校报文确认字为："<<yk_con<<"，对端通知遥控选择失败。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}
	
	ResetTimerYkSel(terminalPtr,yk_no,false);
	//terminalPtr->setYkStatus(yk_no,DataBase::YkSelCon);
	CmdConSig_(YK_SEL_CON,RETURN_CODE_ACTIVE,terminalPtr,yk_no);

	return 0;
}

void CH103_DigiproII::handle_timerCallAllData(const boost::system::error_code& error,share_commpoint_ptr point)
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

void CH103_DigiproII::handle_timerSynTime(const boost::system::error_code& error,share_commpoint_ptr point)
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

void CH103_DigiproII::handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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
			AddStatusLogWithSynT("H103_DigiproII规约遥控选择命令超时。\n");
		}
	}
}

void CH103_DigiproII::handle_timerCallAllEvent(const boost::system::error_code& error,share_terminal_ptr terminalPtr)
{
	if (!error)
	{
		//share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,true,point);
			
		unsigned char yx_val = 0x00;
		unsigned char yx_quality = 0x00;
		for (int i=0;i<=11;i++)
		{
			int index = i + 64;
			terminalPtr->SaveYxQuality(index,yx_quality);
			terminalPtr->SaveYxType(index,DataBase::single_yx_point);
			int ret = terminalPtr->SaveOriYxVal(index,yx_val,true);
		}
		
	}
}

void CH103_DigiproII::ResetTimerCallAllData(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerCallAllData_->expires_from_now(boost::posix_time::seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllData,timeOutCallAllData_)));
		}
		else
		{
			timerCallAllData_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerCallAllData_->async_wait(boost::bind(&CH103_DigiproII::handle_timerCallAllData,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallAllData_->cancel();
	}
}

void CH103_DigiproII::ResetTimerSynTime(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerSynTime_->expires_from_now(boost::posix_time::seconds(getMeanvalueOfPointsSum(MIN_timeOutSynTime,timeOutSynTime_)));
		}
		else
		{
			timerSynTime_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerSynTime_->async_wait(boost::bind(&CH103_DigiproII::handle_timerSynTime,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerSynTime_->cancel();
	}
}

void CH103_DigiproII::ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkSel_->async_wait(boost::bind(&CH103_DigiproII::handle_timerYkSel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkSel_->cancel();
	}
}

void CH103_DigiproII::ResetTimerCallAllEvent(share_terminal_ptr terminalPtr,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerCallAllEvent_->expires_from_now(boost::posix_time::seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllEvent,timeOutCallAllEvent_)));
		}
		else
		{
			timerCallAllEvent_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerCallAllEvent_->async_wait(boost::bind(&CH103_DigiproII::handle_timerCallAllEvent,this,boost::asio::placeholders::error,terminalPtr));
	}
	else
	{
		timerCallAllEvent_->cancel();
	}
}

int CH103_DigiproII::TransYkCmdToPara(int yk_no,typeYktype yk_type,unsigned char & yk_obj,unsigned char & yk_code)
{
	int ret = -1;

	if (yk_no == 0 && yk_type == DataBase::YkOpen)
	{
		yk_obj = 0x11;
		yk_code = 0x11;
		ret = 0;
	}
	else if (yk_no == 0 && yk_type == DataBase::YkClose)
	{
		yk_obj = 0x22;
		yk_code = 0x22;
		ret = 0;
	}
	else if (yk_no == 1 && yk_type == DataBase::YkOpen)
	{
		yk_obj = 0x33;
		yk_code = 0x11;
		ret = 0;
	}
	else if (yk_no == 1 && yk_type == DataBase::YkClose)
	{
		yk_obj = 0x44;
		yk_code = 0x22;
		ret = 0;
	}
	else if (yk_no == 2 && yk_type == DataBase::YkOpen)
	{
		yk_obj = 0x55;
		yk_code = 0x11;
		ret = 0;
	}
	else if (yk_no == 2 && yk_type == DataBase::YkClose)
	{
		yk_obj = 0x66;
		yk_code = 0x22;
		ret = 0;
	}

	return ret;
}

int CH103_DigiproII::TransYkParaToCmd(unsigned char yk_obj,unsigned char yk_code,int & yk_no,typeYktype & yk_type)
{
	int ret = -1;

	if (yk_obj == 0x11 && yk_code == 0x11)
	{
		yk_no = 0;
		yk_type = DataBase::YkOpen;
		ret = 0;
	}
	else if (yk_obj == 0x22 && yk_code == 0x22)
	{
		yk_no = 0;
		yk_type = DataBase::YkClose;
		ret = 0;
	}
	else if (yk_obj == 0x33 && yk_code == 0x11)
	{
		yk_no = 1;
		yk_type = DataBase::YkOpen;
		ret = 0;
	}
	else if (yk_obj == 0x44 && yk_code == 0x22)
	{
		yk_no = 1;
		yk_type = DataBase::YkClose;
		ret = 0;
	}
	else if (yk_obj == 0x55 && yk_code == 0x11)
	{
		yk_no = 2;
		yk_type = DataBase::YkOpen;
		ret = 0;
	}
	else if (yk_obj == 0x66 && yk_code == 0x22)
	{
		yk_no = 2;
		yk_type = DataBase::YkClose;
		ret = 0;
	}

	return ret;
}

}; //namespace Protocol 
