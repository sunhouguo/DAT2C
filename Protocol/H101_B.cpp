#include <boost/bind.hpp>
//#include <boost/algorithm/string/trim.hpp>
//#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "H101_B.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/Terminal.h"
#include "../DataBase/YkPoint.h"
#include "../DataBase/YxPoint.h"

namespace Protocol{

const std::string strDefaultCfg = "H101_BCfg.xml";
size_t CH101_B::H101_BObjectCounter_ = 1;

//针对101规约的YK功能码
const unsigned char DYK_TYPE_OPEN = 0x01;
const unsigned char DYK_TYPE_CLOSE = 0x02;
const unsigned char SYK_TYPE_OPEN = 0;
const unsigned char SYK_TYPE_CLOSE = 0x01;

//针对101规约的控制域定义
const unsigned char DIR_PTOT = 0x00;
const unsigned char DIR_TTOP = 0x80;
const unsigned char PRM_ACK  = 0x40;
const unsigned char PRM_CON  = 0x00;
const unsigned char NACK_FCB = 0;
const unsigned char NACK_FCV = 0;
const unsigned char ACT_FCB = 0x20;
const unsigned char ACT_FCV = 0x10;


//针对101规约的功能码定义-控制方向
const unsigned char C_RCU_NA_3 = 0x00;                 // <0> ：=发送/确认帧   复位通信单元（CU）
const unsigned char C_TEST_NA_3 = 0x02;
const unsigned char C_REQ_NA_3 = 0x03;                 // <3> ：=发送/确认帧   传送数据
const unsigned char C_PLK_NA_3 = 0x09;                 // <9> ：=请求链路状态
const unsigned char C_LKR_NA_3 = 0x0b;                 // <11> ：=确认帧    链路状态


//针对101规约的功能码定义-监视方向
const unsigned char M_CON_NA_3 = 0x00;                 // <0> ：=确认帧     确认
const unsigned char M_BY_NA_3  = 0x01;                  // <1> ：=确认帧     链路忙，未收到报文
const unsigned char M_NV_NA_3 = 0x09;                  // <0> ：=响应帧     无所召唤的数据
const unsigned char M_PLK_NA_3 = 0x09;                 // <9> ：=终端请求链路状态
const unsigned char M_LKR_NA_3 = 0x0b;                 // <11> ：=确认帧    链路状态
const unsigned char M_PS_NA_1 = 0x14;
const unsigned char M_ME_NA_1 = 0x09;
const unsigned char M_ME_NB_1 = 0x0b;
const unsigned char M_IT_NA_1 = 0x0f;
const unsigned char M_ME_ND_1 = 0x15;
const unsigned char M_SP_TB_1 = 0x1e;
const unsigned char M_DP_TB_1 = 0x1f;
const unsigned char M_EI_NA_1 = 0x46;

//针对101规约的传送原因定义
const unsigned char trans_spont = 0x03;
const unsigned char trans_req = 0x05;
const unsigned char trans_act = 0x06;
const unsigned char trans_actcon = 0x07;
const unsigned char trans_deact = 0x08;
const unsigned char trans_deactcon = 0x09;
const unsigned char trans_actterm = 0x0a;
const unsigned char trans_all = 0x14;

//针对101规约的报文类型标识定义
const unsigned char M_SP_NA_1 = 0x01;
const unsigned char M_SP_TA_1 = 0x02;
const unsigned char M_DP_NA_1 = 0x03;
const unsigned char M_SC_NA_1 = 0x2d;
const unsigned char M_DC_NA_1 = 0x2e;
const unsigned char M_IC_NA_1 = 0x64;
const unsigned char M_CI_NA_1 = 0x65;
const unsigned char M_CS_NA_1 = 0x67;
const unsigned char M_TE_NA_1 = 0x68;//测试过程

const unsigned char EnableISQ = 0x00;
const unsigned char DisableISQ = 0x80;
const unsigned char QOI = 0x14;

//类成员函数
CH101_B::CH101_B(boost::asio::io_service & io_service)
:CProtocol(io_service)
{
	bActiveRepeatFrame_ = true;
	bHeartTimeOutResetLink_ = true;
	bClearDataBaseQuality_ = true;
	SynCharNum_ = 5;

	InitObjectIndex();
	InitDefaultStartAddr();
	InitDefaultFrameElem();
	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);

	EnablePriKey();

}

CH101_B::~CH101_B(void)
{
    H101_BObjectCounter_--;
}

void CH101_B::InitObjectIndex()
{
	ProtocolObjectIndex_ = H101_BObjectCounter_++;
}

void CH101_B::InitDefaultStartAddr()
{
	SYX_START_ADDR_ = DEFAULT_SYX_START_ADDR;                              //单点yx起始地址
	DYX_START_ADDR_ = DEFAULT_DYX_START_ADDR;                              //双点yx起始地址
	YC_START_ADDR_ =  DEFAULT_YC_START_ADDR;                               //yc起始地址
	SYK_START_ADDR_ = DEFAULT_SYK_START_ADDR;                              //单点yk起始地址
	DYK_START_ADDR_ = DEFAULT_DYK_START_ADDR;                              //双点yk起始地址
	YM_START_ADDR_ =  DEFAULT_YM_START_ADDR;                               //ym起始地址
	HIS_START_ADDR_ = DEFAULT_HIS_START_ADDR;                              //历史数据起始地址
}

void CH101_B::InitDefaultFrameElem()
{
	FrameTypeLength_ =   DEFAULT_FrameTypeLength;                           //报文类型标识的字节长度
	InfoNumLength_ =     DEFAULT_InfoNumLength;                             //信息体数目标识的字节长度
	TransReasonLength_ = DEFAULT_TransReasonLength;                         //传送原因标识的字节长度
	AsduAddrLength_ =    DEFAULT_AsduAddrLength;                            //装置地址标识的字节长度
	InfoAddrLength_ =    DEFAULT_InfoAddrLength;                            //信息体地址标识的字节长度

}

void CH101_B::InitFrameLocation(size_t frameHead)
{
	FrameTypeLocation_ = frameHead;
	InfoNumLocation_ = FrameTypeLocation_ + FrameTypeLength_;
	TransReasonLocation_ = InfoNumLocation_ + InfoNumLength_;
	AsduAddrLocation_ = TransReasonLocation_ + TransReasonLength_;
	InfoAddrLocation_ = AsduAddrLocation_ + AsduAddrLength_;
	DataLocation_ = InfoAddrLocation_ + InfoAddrLength_;
}

void CH101_B::InitDefaultTimeOut()
{
	bUseTimeOutQueryUnActivePoint_ = false;
	timeOutQueryUnActivePoint_ = DEFAULT_timeOutQueryUnActivePoint;
	timeOutRequireLink_ = DEFAULT_timeOutRequireLink;
	timeOutCallAllData_ = DEFAULT_timeOutCallAllData;
	timeOutSynTime_ = DEFAULT_timeOutSynTime;
	timeOutHeartFrame_ = DEFAULT_timeOutHeartFrame;
	timeOutTestAct_ = DEFAULT_timeOutTestAct;
	timeOutCallAllDD_ = DEFAULT_timeOutCallAllDD;
	timeOutYkSel_ = DEFAULT_timeOutYkSel;
	timeOutYkExe_ = DEFAULT_timeOutYkExe;
	timeOutYkCancel_ = DEFAULT_timeOutYkCancel;
}

void CH101_B::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	timerRequireLink_.reset(new deadline_timer(io_service,seconds(timeOutRequireLink_)));
	AddTimer(timerRequireLink_);

	timerCallAllData_.reset(new deadline_timer(io_service,seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllData,timeOutCallAllData_))));
	AddTimer(timerCallAllData_);

	timerSynTime_.reset(new deadline_timer(io_service,seconds(getMeanvalueOfPointsSum(MIN_timeOutSynTime,timeOutSynTime_))));
	AddTimer(timerSynTime_);

	timerHeartFrame_.reset(new deadline_timer(io_service,seconds(timeOutHeartFrame_)));
	AddTimer(timerHeartFrame_);

	timerTestAct_.reset(new deadline_timer(io_service,seconds(timeOutTestAct_)));
	AddTimer(timerTestAct_);

	timerCallAllDD_.reset(new deadline_timer(io_service,seconds(getMeanvalueOfPointsSum(MIN_timeOutCallAllDD,timeOutCallAllDD_))));
	AddTimer(timerCallAllDD_);

	timerYkSel_.reset(new deadline_timer(io_service,seconds(timeOutYkSel_)));
	AddTimer(timerYkSel_);

	timerYkExe_.reset(new deadline_timer(io_service,seconds(timeOutYkExe_)));
	AddTimer(timerYkExe_);

	timerYkCancel_.reset(new deadline_timer(io_service,seconds(timeOutYkCancel_)));
	AddTimer(timerYkCancel_);
}

int CH101_B::InitProtocol()
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

void CH101_B::UninitProtocol()
{
	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("H101_B规约的通道关闭成功。\n");
}

int CH101_B::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
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

int CH101_B::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
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

int CH101_B::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
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

int CH101_B::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
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

	case LINK_STATUS:
		buf[count++] = 0x10;
		break;

	case CONFIRM_ACK:
		buf[count++] = 0x10;
		break;
		
	case TEST_ACT:
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

int CH101_B::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
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
		ostr<<"H101_B规约不能从发送命令中获得terminal ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (cmd.getCmdType())
	{
	case REQUIRE_LINK:
		bytesAssemble = AssembleRequireLink(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerRequireLink(terminalPtr,true);
			ResetTimerTestAct(terminalPtr,false);
		}
		break;

	case RESET_LINK:
		bytesAssemble = AssembleResetLink(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;
	case LINK_STATUS:
		bytesAssemble = AssembleLinkStatus(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			//setWaitingForAnswer(cmd.getCommPoint()); 
		}
		break;
	case CONFIRM_ACK:
		bytesAssemble = AssembleConfirmAck(bufIndex,buf,terminalPtr);
		if (bytesAssemble > 0)
		{
			//setWaitingForAnswer(cmd.getCommPoint());
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
   case SYN_TIME_ACT:
	   bytesAssemble = AssembleSynTime(bufIndex,buf,terminalPtr,boost::posix_time::microsec_clock::local_time());
	   if (bytesAssemble > 0)
	   {
		   setWaitingForAnswer(cmd.getCommPoint());
		   ResetTimerSynTime(terminalPtr,true);
	   }
	   break;

  /* case TEST_ACT:
	   bytesAssemble = AssembleTestAck(bufIndex,buf,terminalPtr);
	   if (bytesAssemble > 0)
	   {
		   setWaitingForAnswer(cmd.getCommPoint());
		   ResetTimerHeartFrame(terminalPtr,true);
	   }
	   break;*/

   case CALL_ALL_DD_ACT:
	   bytesAssemble = AssembleCallAllDD(bufIndex,buf,terminalPtr);
	   if (bytesAssemble > 0)
	   {
		   setWaitingForAnswer(cmd.getCommPoint());
		   ResetTimerCallAllDD(terminalPtr,true);
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

		   if (yk_type == DataBase::YkClose)
		   {
			   if (terminalPtr->getbHYkDouble(yk_no))
			   {
				   bytesAssemble = AssembleDoubleYKSel(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_CLOSE);
			   }
			   else
			   {
				   bytesAssemble = AssembleSingleYKSel(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_CLOSE);
			   }

		   }
		   else if (yk_type == DataBase::YkOpen)
		   {
			   if (terminalPtr->getbHYkDouble(yk_no))
			   {
				   bytesAssemble = AssembleDoubleYKSel(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_OPEN);
			   }
			   else
			   {
				   bytesAssemble = AssembleSingleYKSel(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_OPEN);
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

		   if (yk_type == DataBase::YkClose)
		   {
			   if (terminalPtr->getbHYkDouble(yk_no))
			   {
				   bytesAssemble = AssembleDoubleYKExe(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_CLOSE);
			   }
			   else
			   {
				   bytesAssemble = AssembleSingleYKExe(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_CLOSE);
			   }

		   }
		   else if (yk_type == DataBase::YkOpen)
		   {
			   if (terminalPtr->getbHYkDouble(yk_no))
			   {
				   bytesAssemble = AssembleDoubleYKExe(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_OPEN);
			   }
			   else
			   {
				   bytesAssemble = AssembleSingleYKExe(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_OPEN);
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

		   if (yk_type == DataBase::YkClose)
		   {
			   if (terminalPtr->getbHYkDouble(yk_no))
			   {
				   bytesAssemble = AssembleDoubleYKCancel(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_CLOSE);
			   }
			   else
			   {
				   bytesAssemble = AssembleSingleYKCancel(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_CLOSE);
			   }

		   }
		   else if (yk_type == DataBase::YkOpen)
		   {
			   if (terminalPtr->getbHYkDouble(yk_no))
			   {
				   bytesAssemble = AssembleDoubleYKCancel(bufIndex,buf,terminalPtr,yk_no,DYK_TYPE_OPEN);
			   }
			   else
			   {
				   bytesAssemble = AssembleSingleYKCancel(bufIndex,buf,terminalPtr,yk_no,SYK_TYPE_OPEN);
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

	default:
		break;
	}

	return bytesAssemble;
}

int CH101_B::AssembleFrameTail( size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
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

	case LINK_STATUS:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;

	case CONFIRM_ACK:
		buf[count++] = CalcCheckSumByte(&buf[bufBegin + 1],length - 1);
		buf[count++] = 0x16;
		break;

	case TEST_ACT:
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

			buf[count++] = CalcCheckSumByte(&buf[bufBegin + 4],framelength);
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

int CH101_B::QueryUnAliveCommPoint(share_commpoint_ptr point)
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

int CH101_B::LoadXmlCfg(std::string filename)
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
	if (xml.FindElem(strHeartTimeOutResetLink))
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

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}


void CH101_B::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();

	CProtocol::SaveXmlCfg(xml);

	if (bHeartTimeOutResetLink_)
	{
		xml.AddElem(strHeartTimeOutResetLink,strboolTrue);
	}

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

	xml.OutOfElem();

	xml.Save(filename);
}


//send frame assemble
int CH101_B::AssembleRequireLink(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_PLK_NA_3 |  DIR_PTOT | PRM_ACK | NACK_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH101_B::AssembleResetLink(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_RCU_NA_3 | DIR_PTOT | PRM_ACK | NACK_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH101_B::AssembleLinkStatus(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_LKR_NA_3 | DIR_PTOT | PRM_CON | NACK_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH101_B::AssembleConfirmAck(size_t bufIndex,unsigned char * buf,share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_RCU_NA_3 | DIR_PTOT | PRM_CON | NACK_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	return count - bufIndex;
}

int CH101_B::AssembleCallAllData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_IC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI;

	return count - bufIndex;
}

int CH101_B::AssembleSynTime(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr, boost::posix_time::ptime time)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
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

int CH101_B::AssembleTestAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_TEST_NA_3 |  DIR_PTOT | PRM_ACK | NACK_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);

	//count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
	//count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	//count += ValToBuf(&buf[count],M_TE_NA_1,FrameTypeLength_);
	//count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	//count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	//count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	//count += ValToBuf(&buf[count],0,InfoAddrLength_);

	return count - bufIndex;
}

int CH101_B::AssembleCallAllDD(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_CI_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],0,InfoAddrLength_);
	buf[count++] = QOI;

	return count - bufIndex;
}

int CH101_B::AssembleDoubleYKSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH101_B::AssembleDoubleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CH101_B::AssembleDoubleYKCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no,unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_DC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_deact,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],DYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH101_B::AssembleSingleYKSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}

int CH101_B::AssembleSingleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_act,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x00 | yk_code;

	return count - bufIndex;
}

int CH101_B::AssembleSingleYKCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no,unsigned char yk_code)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_REQ_NA_3 | getFCB(terminalPtr)| DIR_PTOT | PRM_ACK | ACT_FCV,1);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],M_SC_NA_1,FrameTypeLength_);
	count += ValToBuf(&buf[count],0x01,InfoNumLength_);
	count += ValToBuf(&buf[count],trans_deact,TransReasonLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),AsduAddrLength_);
	count += ValToBuf(&buf[count],SYK_START_ADDR_ + yk_no,InfoAddrLength_);
	buf[count++] = 0x80 | yk_code;

	return count - bufIndex;
}



//recv frame parse
int CH101_B::ParseShortFrame(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	unsigned char ContrCode = buf[1] & 0x0f;

	unsigned char DIR = buf[1] & 0x80;
	unsigned char PRM = buf[1] & 0x40;

	if(PRM == PRM_CON)//报文来自终端启动站
	{
		switch (ContrCode)
		{
		case M_LKR_NA_3: //链路状态
			{
				if (getLastSendCmd() == REQUIRE_LINK)
				{
					AddSendCmdVal(RESET_LINK,REQUIRE_LINK_PRIORITY,terminalPtr);
				}
			}
			break;
		case M_CON_NA_3: //肯定认可
			{
               switch (getLastSendCmd())
			   {
			   case RESET_LINK:
				   {
					//   AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,terminalPtr);
					//   AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,terminalPtr);
					//   ResetTimerHeartFrame(terminalPtr,true);
					//   ResetTimerTestAct(terminalPtr,true);
					   terminalPtr->setInitCommPointFlag(true);
				   }
				   break;
			   case TEST_ACT:
				   ResetTimerTestAct(terminalPtr,true);
				   break;
			   default:
				   AddStatusLogWithSynT("收到FC=0的确认报文\n");
				   break;
			   }
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

		default:
			{
				std::ostringstream ostr;
				ostr<<"接收报文错误，未定义的报文类型 Control_Code ="<<ContrCode<<std::endl;
				AddStatusLogWithSynT(ostr.str());
			}
			break;
		}
	}
	else
	{
		switch (ContrCode)
		{
		case M_PLK_NA_3:
			{
				AddSendCmdVal(LINK_STATUS,LINK_STATUS_PRIORITY,terminalPtr);
			}
			break;
		case M_CON_NA_3://肯定确认
			{
				if (getLastSendCmd() == LINK_STATUS)
				{
                    AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY+2,terminalPtr);

					//AddSendCmdVal(CALL_ALL_DATA_ACT,CALL_ALL_DATA_ACT_PRIORITY,terminalPtr);
					//AddSendCmdVal(SYN_TIME_ACT,SYN_TIME_ACT_PRIORITY,terminalPtr);
					ResetTimerSynTime(terminalPtr,true,15);
					ResetTimerCallAllData(terminalPtr,true,1);

					//ResetTimerHeartFrame(terminalPtr,true);
					//ResetTimerTestAct(terminalPtr,true);
				}
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
	}

	return 0;
}

int CH101_B::ParseLongFrame(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t FrameType = BufToVal(&buf[FrameTypeLocation_],FrameTypeLength_);        //类型标识
	size_t TransReason = BufToVal(&buf[TransReasonLocation_],TransReasonLength_);  //传送原因
	unsigned char Data_Code = buf[DataLocation_] & 0x80;

	switch (FrameType)
	{
	case M_IC_NA_1:// call all data
		switch (TransReason)
		{
		case trans_actcon:
			ParseAllDataCallCon(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
			break;

		case trans_actterm:
			ParseAllDataCallOver(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
		}
		else if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseSingleCOS(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"单点YX返校报文错误，未定义的传送原因 TRANS_REASON ="<<TransReason<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;

	case M_DP_NA_1: //double yx
		if (TransReason == trans_all || TransReason == trans_actcon)
		{
			ParseAllDoubleYX(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
		}
		else if(TransReason == trans_spont || TransReason == trans_req)
		{
			ParseDoubleCOS(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
		} 
		else if(TransReason == trans_spont || TransReason == trans_req)
		{
			ParseYCCH(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
		}
		else if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseYCCHWithValid(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
		}
		else if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseYCCHWithValid(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
		AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
		break;

	case M_SP_TA_1: //short synt single soe
		if (TransReason == trans_spont || TransReason == trans_req)
		{
			ParseSingleSOE(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
			break;

		case trans_actterm:
			ParseAllYMCallOver(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
		AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
		break;

	case M_SC_NA_1:
		switch (TransReason)
		{
		case trans_actcon: //single yk
			if (Data_Code == 0x80)
			{
				ParseSingleYKSelCon(buf,terminalPtr);
				AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
			}
			else if (!Data_Code)
			{
				ParseSingleYKExeCon(buf,terminalPtr);
				AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
			break;

		case trans_actterm:
			ParseSingleYKOverCon(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
				AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
			}
			else if (!Data_Code)
			{
				ParseDoubleYKExeCon(buf,terminalPtr);
				AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
			break;

		case trans_actterm:
			ParseDoubleYKOverCon(buf,terminalPtr);
			AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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

	case M_TE_NA_1:
		ParseTestFramCon(buf,terminalPtr);
		AddSendCmdVal(CONFIRM_ACK,CONFIRM_ACK_PRIORITY,terminalPtr);
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

int CH101_B::ParseAllDataCallCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{

	ResetTimerCallAllData(terminalPtr,true);

	return 0;
}

int CH101_B::ParseAllDataCallOver(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	terminalPtr->setInitCommPointFlag(true);

	ResetTimerCallAllData(terminalPtr,true);

	return 0;
}

int CH101_B::ParseAllSingleYX(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseSingleCOS(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseAllDoubleYX(unsigned char * buf, share_terminal_ptr terminalPtr)
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
				int ret = terminalPtr->SaveOriYxVal(info_addr + i,yx_val,true/*terminalPtr->getInitCommPointFlag()*/);
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
				int ret = terminalPtr->SaveOriYxVal(info_addr,yx_val,true/*terminalPtr->getInitCommPointFlag()*/);
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

int CH101_B::ParseDoubleCOS(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseAllYXByte(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseAllYCData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int info_num = BufToVal(&buf[InfoNumLocation_],InfoNumLength_) & (~(0x80<<(InfoNumLength_ - 1) * 8));
	int info_ISQ =(BufToVal(&buf[InfoNumLocation_],InfoNumLength_) >> ((InfoNumLength_ - 1) * 8)) & 0x80;


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
				terminalPtr->SaveYcQuality(info_addr + i,true);
				int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,true/*terminalPtr->getInitCommPointFlag()*/);
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
				terminalPtr->SaveYcQuality(info_addr,true);
				int ret = terminalPtr->SaveOriYcVal(info_addr,YcVal,true/*terminalPtr->getInitCommPointFlag()*/);
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

int CH101_B::ParseYCCH(unsigned char * buf, share_terminal_ptr terminalPtr)
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
				terminalPtr->SaveYcQuality(info_addr + i,true);
				int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,true/*terminalPtr->getInitCommPointFlag()*/);
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
				terminalPtr->SaveYcQuality(info_addr,true);
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

int CH101_B::ParseAllYCDataWithValid(unsigned char * buf, share_terminal_ptr terminalPtr)
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

				terminalPtr->SaveYcQuality(info_addr + i,true);
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

				terminalPtr->SaveYcQuality(info_addr,true);
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


int CH101_B::ParseYCCHWithValid(unsigned char * buf, share_terminal_ptr terminalPtr)
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

				terminalPtr->SaveYcQuality(info_addr + i,true);
				int ret = terminalPtr->SaveOriYcVal(info_addr + i,YcVal,true/*terminalPtr->getInitCommPointFlag()*/);
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

				terminalPtr->SaveYcQuality(info_addr,true);
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


int CH101_B::ParseSynTimeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	terminalPtr->setSynTCommPointFlag(true);

	ResetTimerSynTime(terminalPtr,true);

	return 0;
}

int CH101_B::ParseSingleSOE(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseDoubleSOE(unsigned char * buf, share_terminal_ptr terminalPtr)
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



int CH101_B::ParseAllYMCallCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101_B::ParseAllYMCallOver(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}

int CH101_B::ParseAllYMData(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseEndInit(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	return 0;
}


int CH101_B::ParseDoubleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseDoubleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseDoubleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseDoubleYKOverCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseSingleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseSingleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseSingleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseSingleYKOverCon(unsigned char * buf, share_terminal_ptr terminalPtr)
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

int CH101_B::ParseTestFramCon(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	ResetTimerTestAct(terminalPtr,true);
	return 0;
}


//timer api
void CH101_B::handle_timerRequireLink(const boost::system::error_code& error,share_commpoint_ptr point)
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

void CH101_B::handle_timerCallAllData(const boost::system::error_code& error,share_commpoint_ptr point)
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

void CH101_B::handle_timerSynTime(const boost::system::error_code& error,share_commpoint_ptr point)
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

void CH101_B::handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point)
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

void CH101_B::handle_timerTestAct(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,true,point);
		if (val)
		{
			AddStatusLogWithSynT("警告：H101_B心跳包超时未收到回复，但是不影响正常通讯！\n");
			if (bHeartTimeOutResetLink_)
			{
				//ResetTimerHeartFrame(point,false);
				AddSendCmdVal(REQUIRE_LINK,REQUIRE_LINK_PRIORITY,point);
			}
		}
	}
}

void CH101_B::handle_timerCallAllDD(const boost::system::error_code& error,share_commpoint_ptr point)
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

void CH101_B::handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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

void CH101_B::handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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

void CH101_B::handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no)
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


void CH101_B::ResetTimerRequireLink(share_commpoint_ptr point,bool bContinue , unsigned short val)
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

		timerRequireLink_->async_wait(boost::bind(&CH101_B::handle_timerRequireLink,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerRequireLink_->cancel();
	}
}


void CH101_B::ResetTimerCallAllData(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,true,point);
		if(nextPoint)
		{
			if(1)//(nextPoint->getInitCommPointFlag())
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

			timerCallAllData_->async_wait(boost::bind(&CH101_B::handle_timerCallAllData,this,boost::asio::placeholders::error,point));
		}
	}
	else
	{
		timerCallAllData_->cancel();
	}
}


void CH101_B::ResetTimerSynTime(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		share_commpoint_ptr nextPoint = getNextCommPoint(TERMINAL_NODE,true,point);
		if(nextPoint)
		{
			if(1)//(nextPoint->getSynTCommPointFlag())
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

			timerSynTime_->async_wait(boost::bind(&CH101_B::handle_timerSynTime,this,boost::asio::placeholders::error,point));
		}
	}
	else
	{
		timerSynTime_->cancel();
	}
}
void CH101_B::ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
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

		timerHeartFrame_->async_wait(boost::bind(&CH101_B::handle_timerHeartFrame,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerHeartFrame_->cancel();
	}
}

void CH101_B::ResetTimerTestAct(share_commpoint_ptr point,bool bContinue,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerTestAct_->expires_from_now(boost::posix_time::seconds(timeOutTestAct_));
		}
		else
		{
			timerTestAct_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerTestAct_->async_wait(boost::bind(&CH101_B::handle_timerTestAct,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerTestAct_->cancel();
	}
}


void CH101_B::ResetTimerCallAllDD(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
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

		timerCallAllDD_->async_wait(boost::bind(&CH101_B::handle_timerCallAllDD,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerCallAllDD_->cancel();
	}
}

void CH101_B::ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkSel_->async_wait(boost::bind(&CH101_B::handle_timerYkSel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkSel_->cancel();
	}
}

void CH101_B::ResetTimerYkExe(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkExe_->async_wait(boost::bind(&CH101_B::handle_timerYkExe,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkExe_->cancel();
	}
}

void CH101_B::ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue /*= false*/,unsigned short val/* = 0*/)
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

		timerYkCancel_->async_wait(boost::bind(&CH101_B::handle_timerYkCancel,this,boost::asio::placeholders::error,point,yk_no));
	}
	else
	{
		timerYkCancel_->cancel();
	}
}




//para api
int CH101_B::setSYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	SYX_START_ADDR_ = val;

	return 0;
}

int CH101_B::setDYX_START_ADDR(size_t val)
{
	if (val < 0 || val >= 0x4001)
	{
		return -1;
	}

	DYX_START_ADDR_ = val;

	return 0;
}

int CH101_B::setYC_START_ADDR(size_t val)
{
	if (val < 0x101 || val >= 0x6001)
	{
		return -1;
	}

	YC_START_ADDR_ = val;

	return 0;
}

int CH101_B::setSYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	SYK_START_ADDR_ = val;

	return 0;
}

int CH101_B::setDYK_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	DYK_START_ADDR_ = val;

	return 0;
}

int CH101_B::setYM_START_ADDR(size_t val)
{
	if (val < 0x701)
	{
		return -1;
	}

	YM_START_ADDR_ = val;

	return 0;
}

int CH101_B::setHIS_START_ADDR(size_t val)
{
	HIS_START_ADDR_ = val;

	return 0;
}

int CH101_B::setFrameTypeLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	FrameTypeLength_ = val;

	return 0;
}

int CH101_B::setInfoNumLength(unsigned short val)
{
	if (val <= 0 || val > 4)
	{
		return -1;
	}

	InfoNumLength_ = val;

	return 0;
}

int CH101_B::setTransReasonLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	TransReasonLength_ = val;

	return 0;
}

int CH101_B::setAsduAddrLength(unsigned short val)
{
	if (val <= 0 || val > 8)
	{
		return -1;
	}

	AsduAddrLength_ = val;

	return 0;
}

int CH101_B::setInfoAddrLength(unsigned short val)
{
	if (val <= 0 || val > 12)
	{
		return -1;
	}

	InfoAddrLength_ = val;

	return 0;
}

int CH101_B::setTimeOutQueryUnActivePoint(unsigned short val)
{
	if (val < MIN_timeOutQueryUnActivePoint)
	{
		return -1;
	}

	timeOutQueryUnActivePoint_ = val;

	return 0;
}

int CH101_B::setTimeOutRequrieLink(unsigned short val)
{
	if (val < MIN_timeOutRequireLink || val > 60)
	{
		return -1;
	}

	timeOutRequireLink_ = val;

	return 0;
}

int CH101_B::setTimeOutCallAllData(unsigned short val)
{
	if (val < 60 || val > 12000)
	{
		return -1;
	}

	timeOutCallAllData_ = val;

	return 0;
}

int CH101_B::setTimeOutCallAllDD(unsigned short val)
{
	if (val < 60 || val > 12000)
	{
		return -1;
	}

	timeOutCallAllDD_ = val;

	return 0;
}

int CH101_B::setTimeOutSynTime(unsigned short val)
{
	if (val < 60 || val > 12000)
	{
		return -1;
	}

	timeOutSynTime_ = val;

	return 0;
}

int CH101_B::setTimeOutHeartFrame(unsigned short val)
{
	if (val <= 0 || val > 60) 
	{
		return -1;
	}

	timeOutHeartFrame_ = val;

	return 0;
}

int CH101_B::setTimeOutYkSel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkSel_ = val;

	return 0;
}

int CH101_B::setTimeOutYkExe(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkExe_ = val;

	return 0;
}

int CH101_B::setTimeOutYkCancel(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutYkCancel_ = val;

	return 0;
}

int CH101_B::setTimeOutCallPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutCallPara_ = val;

	return 0;
}

int CH101_B::setTimeOutSetPara(unsigned short val)
{
	if (val <= 0 || val > 300)
	{
		return -1;
	}

	timeOutSetPara_ = val;

	return 0;
}


//protocol func api
int CH101_B::getAddrByRecvFrame(unsigned char * buf)
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

	if ((dirPrmByte & DIR_TTOP) == DIR_PTOT) //检查报文方向位标志
	{
		std::ostringstream ostr;
		ostr<<"控制域解析发现报文方向标志位错误,这帧报文将不会被解析。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		return -1;
	}

	return addr;
}

int CH101_B::getFCB(share_terminal_ptr terminalPtr)
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


};//namespace Protocol
