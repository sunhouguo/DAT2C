#include <boost/bind.hpp>
#include "GB_T18657_1.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../FileSystem/Markup.h"
#include "../DataBase/Terminal.h"
#include "../DataBase/YxPoint.h"

namespace Protocol {

const std::string strDefaultCfg = "GB_T18657_1Cfg.xml";
size_t CGB_T18657_1::GB_T18657_1ObjectCounter_ = 1;

//控制域标识位
const unsigned char DIR_PRM_N = 0x00;
const unsigned char DIR_PRM_A = 0x40;
const unsigned char ACT_FCV = 0x10;
const unsigned char NACK_FCV = 0;
const unsigned char ACT_ACD = 0x20;
const unsigned char NACK_ACD = 0;
const unsigned char ACT_FCB = 0x20;
const unsigned char NACK_FCB = 0;

//针对101规约的功能码定义-监视方向
const unsigned char M_CON_NA_3 = 0x00;                 // <0> ：=确认帧     确认
//const unsigned char M_BY_NA_3 = 0x01;                  // <1> ：=确认帧     链路忙，未收到报文
const unsigned char M_AV_NA_3 = 0x08;                  // <8> ：=响应帧     以数据响应请求帧
const unsigned char M_NV_NA_3 = 0x09;                  // <0> ：=响应帧     无所召唤的数据
const unsigned char M_LKR_NA_3 = 0x0b;                 // <11> ：=确认帧    链路状态

//针对101规约的功能码定义-控制方向
//const unsigned char C_RCU_NA_3 = 0x00;                 // <0> ：=发送/确认帧   复位通信单元（CU）
const unsigned char C_RET_NA_3 = 0x01;                   //复位
//const unsigned char C_REQ_NA_3 = 0x03;                 // <3> ：=发送/确认帧   传送数据
const unsigned char C_NEQ_NA_3 = 0x04;                 // <4> ：=发送/无回答帧 传送数据 
//const unsigned char C_RFB_NA_3 = 0x07;                 // <7> ：=发送/确认帧   复位帧计数位（FCB）
const unsigned char C_PLK_NA_3 = 0x09;                 // <9> ：=链路测试
const unsigned char C_PL1_NA_3 = 0x0a;                 // <10> ：=请求/响应     召唤1级用户数据
const unsigned char C_PL2_NA_3 = 0x0b;                 // <11> ：=请求/响应     召唤2级用户数据

//DevType 标识
const unsigned char DevTypeTTU = 0x01;

//AreaCode 标识
const unsigned char AreaCodeTJ = 0x90;

//AFN 标识
const unsigned char AFN_act_nack_con = 0;     //确认，否认
const unsigned char AFN_reset_cmd = 0x01;     //复位
const unsigned char AFN_test_link = 0x02;     //链路接口检测
const unsigned char AFN_PrimaryData = 0x0c;   //一类数据
const unsigned char AFN_SecondaryData = 0x0d; //二级数据

//FUN 标识
const unsigned char FUN_BasicYx = 0x04;
const unsigned char FUN_BasicYc = 0x05;

const unsigned char FUN_Hardware_Init = 0;
const unsigned char FUN_Data_Init = 0x01;
const unsigned char FUN_DATA_PARA_Init = 0x02;

const unsigned char FUN_Login = 0x00;

CGB_T18657_1::CGB_T18657_1(boost::asio::io_service & io_service)
							:CProtocol(io_service)
{
	bActiveRepeatFrame_ = true;
	SynCharNum_ = 8;
	Pristation_MSA_ = default_pritstation_addr;

	max_frame_length_ = 0xfffc;

	InitObjectIndex();
	InitDefaultFrameElem();
	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);
}

CGB_T18657_1::~CGB_T18657_1(void)
{
}

int CGB_T18657_1::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	if ((buf[0] == 0x68) && (buf[5] == 0x68))
	{
		int length1 = BufToVal(&buf[1],2);
		length1 = (length1 >> 2) & 0x3fff;

		int length2 = BufToVal(&buf[3],2);
		length2 = (length2 >> 2) & 0x3fff;

		if(length1 == length2)
		{
			exceptedBytes = length1 + 8;
			return 0;
		}
	}

	return -1;
}

int CGB_T18657_1::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	size_t sum = 0;

	sum = CalcCheckSumByte(&buf[6],exceptedBytes - 8);

	if ((buf[exceptedBytes -1] == 0x16) && (buf[exceptedBytes - 2] == sum))
	{
		return 0;
	}

	return -1;
}

int CGB_T18657_1::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
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
		ostr<<"GB_T18657_1规约不能根据接收报文中的地址匹配terminal ptr,这帧报文将不会被解析。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	int AFN = BufToVal(&buf[AFN_Location_],AFN_Length_);
	int SEQ = BufToVal(&buf[SEQ_Location_],SEQ_Length_);
	int FUN = BufToVal(&buf[FunCodeLocation_],FunCodeLength_);

	switch(AFN)
	{
	case AFN_PrimaryData:
		{
			switch(FUN)
			{
			case FUN_BasicYx:
				ParseBasicYxData(buf,terminalPtr);
				break;

			case FUN_BasicYc:
				ParseBasicYcData(buf,terminalPtr);
				break;

			default:
				{
					std::ostringstream ostr;
					ostr<<"接收一类数据报文错误，未定义的功能码类型 FUN ="<<FUN<<std::endl;
					AddStatusLogWithSynT(ostr.str());
				}
				break;
			}
		}
		break;

	case AFN_SecondaryData:
		break;

	default:
		{
			std::ostringstream ostr;
			ostr<<"接收报文错误，未定义的报文类型 AFN ="<<AFN<<std::endl;
			AddStatusLogWithSynT(ostr.str());
		}
		break;
	}

	return terminalIndex;
}

int CGB_T18657_1::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	buf[count++] = 0x68;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0x68;
	
	return count - bufIndex;
}

int CGB_T18657_1::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
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
		ostr<<"GB_T18657_1规约不能从发送命令中获得terminal ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}

	switch (cmd.getCmdType())
	{
	case CALL_PRIMARY_DATA:
		{
			try
			{
				int funCode = boost::any_cast<int>(cmd.getVal());
				bytesAssemble = AssembleCallPrimaryData(bufIndex,buf,terminalPtr,funCode);
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"请求一类数据的功能码参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}

			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());
				ResetTimerHeartFrame(terminalPtr,true);
			}
		}
		break;

	case CALL_SECONDARY_DATA:
		{
			try
			{
				int funCode = boost::any_cast<int>(cmd.getVal());
				bytesAssemble = AssembleCallSecondaryData(bufIndex,buf,terminalPtr,funCode);
			}
			catch(const boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"请求二类数据的功能码参数非法："<<e.what()<<std::endl;
				AddStatusLogWithSynT(ostr.str());
				return -1;
			}
			
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case RESET_LINK:
		{
			bytesAssemble = AssembleResetCMD(bufIndex,buf,terminalPtr,FUN_DATA_PARA_Init);
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	case LINK_STATUS:
		{
			bytesAssemble = AssembleLogin(bufIndex,buf,terminalPtr);
			if (bytesAssemble > 0)
			{
				setWaitingForAnswer(cmd.getCommPoint());
			}
		}
		break;

	default:
		break;
	}

	return bytesAssemble;
}

int CGB_T18657_1::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	int length = bufIndex - bufBegin;
	if (length <= 1)
	{
		return -1;
	}

	size_t count = bufIndex;

	int framelength = length - 6;
	if (framelength <= 0 || framelength > max_frame_length_)
	{
		return -1;
	}

	buf[count++] = CalcCheckSumByte(&buf[bufBegin + 6],framelength);
	buf[count++] = 0x16;
	
	framelength = ((framelength << 2) & 0xfffc) | 0x02;
	ValToBuf(&buf[bufBegin + 1],framelength,2);
	ValToBuf(&buf[bufBegin + 3],framelength,2);

	return count - bufIndex;
}

int CGB_T18657_1::LoadXmlCfg(std::string filename)
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

void CGB_T18657_1::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();

	CProtocol::SaveXmlCfg(xml);

	xml.OutOfElem();

	xml.Save(filename);
}

void CGB_T18657_1::InitObjectIndex()
{
	ProtocolObjectIndex_ = GB_T18657_1ObjectCounter_++;
}

void CGB_T18657_1::InitDefaultFrameElem()
{
	DevTypeLength_ = defautl_DevTypeLength;
	AreaCodeLength_ = default_AreaCodeLength;
	TerminalAddrLength_ = default_TerminalAddrLength;
	PristationAddrLength_ = default_PristationAddrLength;
	AFN_Length_ = default_AFN_Length;
	SEQ_Length_ = default_SEQ_Length;
	FunCodeLength_ = default_FunCodeLength;
}

void CGB_T18657_1::InitFrameLocation(size_t FrameHead)
{
	DevTypeLocation_ = FrameHead;
	AreaCodeLocation_ = DevTypeLocation_ + DevTypeLength_;
	TerminalAddrLocation_ = AreaCodeLocation_ + AreaCodeLength_;
	PristationAddrLocation_ = TerminalAddrLocation_ + TerminalAddrLength_;
	AFN_Location_ = PristationAddrLocation_ + PristationAddrLength_;
	SEQ_Location_ = AFN_Location_ + AFN_Length_;
	FunCodeLocation_ = SEQ_Location_ + SEQ_Length_;
	FunDataLocation_ = FunCodeLocation_ + FunCodeLength_;
}

int CGB_T18657_1::AssembleCallPrimaryData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char funCode)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_PL1_NA_3 | getFCB(terminalPtr) | DIR_PRM_A | ACT_FCV,1);
	count += ValToBuf(&buf[count],DevTypeTTU,DevTypeLength_);
	count += ValToBuf(&buf[count],AreaCodeTJ,AreaCodeLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),TerminalAddrLength_);
	count += ValToBuf(&buf[count],((Pristation_MSA_ << 3) & 0xf8),PristationAddrLength_);
	count += ValToBuf(&buf[count],AFN_PrimaryData,AFN_Length_);
	count += ValToBuf(&buf[count],0,SEQ_Length_);
	count += ValToBuf(&buf[count],funCode,FunCodeLength_);

	return count - bufIndex;
}

int CGB_T18657_1::AssembleCallSecondaryData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char funCode)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_PL2_NA_3 | getFCB(terminalPtr) | DIR_PRM_A | ACT_FCV,1);
	count += ValToBuf(&buf[count],DevTypeTTU,DevTypeLength_);
	count += ValToBuf(&buf[count],AreaCodeTJ,AreaCodeLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),TerminalAddrLength_);
	count += ValToBuf(&buf[count],((Pristation_MSA_ << 3) & 0xf8),PristationAddrLength_);
	count += ValToBuf(&buf[count],AFN_SecondaryData,AFN_Length_);
	count += ValToBuf(&buf[count],0,SEQ_Length_);
	count += ValToBuf(&buf[count],funCode,FunCodeLength_);

	return count - bufIndex;
}

int CGB_T18657_1::AssembleResetCMD(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char funCode)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_PL1_NA_3 | getFCB(terminalPtr) | DIR_PRM_A | ACT_FCV,1);
	count += ValToBuf(&buf[count],DevTypeTTU,DevTypeLength_);
	count += ValToBuf(&buf[count],AreaCodeTJ,AreaCodeLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),TerminalAddrLength_);
	count += ValToBuf(&buf[count],((Pristation_MSA_ << 3) & 0xf8),PristationAddrLength_);
	count += ValToBuf(&buf[count],AFN_reset_cmd,AFN_Length_);
	count += ValToBuf(&buf[count],0,SEQ_Length_);
	count += ValToBuf(&buf[count],funCode,FunCodeLength_);

	return count - bufIndex;
}

int CGB_T18657_1::AssembleLogin(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;

	count += ValToBuf(&buf[count],C_PL1_NA_3 | getFCB(terminalPtr) | DIR_PRM_A | ACT_FCV,1);
	count += ValToBuf(&buf[count],DevTypeTTU,DevTypeLength_);
	count += ValToBuf(&buf[count],AreaCodeTJ,AreaCodeLength_);
	count += ValToBuf(&buf[count],terminalPtr->getAddr(),TerminalAddrLength_);
	count += ValToBuf(&buf[count],((Pristation_MSA_ << 3) & 0xf8),PristationAddrLength_);
	count += ValToBuf(&buf[count],AFN_test_link,AFN_Length_);
	count += ValToBuf(&buf[count],0,SEQ_Length_);
	count += ValToBuf(&buf[count],FUN_Login,FunCodeLength_);

	return count - bufIndex;
}

int CGB_T18657_1::getNextPrimaryDataFunCode(int comm_point_index)
{
	if (PrimaryDataFunCodeSet_.size() == 0)
	{
		return -1;
	}

	int diff = getCommPointSum() - (int)PrimaryDataFunCodeIndex_.size();
	if (diff > 0)
	{
		for (int i=0;i<diff;i++)
		{
			PrimaryDataFunCodeIndex_.push_back(0);
		}
	}

	if (comm_point_index < 0 || comm_point_index >= (int)PrimaryDataFunCodeIndex_.size())
	{
		return -1;
	}

	int ret = PrimaryDataFunCodeSet_[PrimaryDataFunCodeIndex_[comm_point_index]];

	PrimaryDataFunCodeIndex_[comm_point_index] = (++PrimaryDataFunCodeIndex_[comm_point_index]) % PrimaryDataFunCodeSet_.size();

	return ret;
}

int CGB_T18657_1::getNextSecondaryDataFunCode(int comm_point_index)
{
	if (SecondaryDataFunCodeSet_.size() <= 0)
	{
		return -1;
	}

	int diff = getCommPointSum() - (int)SecondaryDataFunCodeIndex_.size();
	if (diff > 0)
	{
		for (int i=0;i<diff;i++)
		{
			SecondaryDataFunCodeIndex_.push_back(0);
		}
	}

	if (comm_point_index < 0 || comm_point_index >= (int)SecondaryDataFunCodeIndex_.size())
	{
		return -1;
	}

	int ret = SecondaryDataFunCodeSet_[SecondaryDataFunCodeIndex_[comm_point_index]];

	SecondaryDataFunCodeIndex_[comm_point_index] = (++SecondaryDataFunCodeIndex_[comm_point_index]) % SecondaryDataFunCodeSet_.size();

	return ret;
}

int CGB_T18657_1::InitPrimaryDataFunCodeSet()
{
	PrimaryDataFunCodeSet_.push_back(FUN_BasicYx);
	PrimaryDataFunCodeSet_.push_back(FUN_BasicYc);

	for (int i=0;i<getCommPointSum();i++)
	{
		PrimaryDataFunCodeIndex_.push_back(0);
	}

	return 0;
}

int CGB_T18657_1::InitSecondaryDataFunCodeSet()
{
	for (int i=0;i<getCommPointSum();i++)
	{
		SecondaryDataFunCodeIndex_.push_back(0);
	}

	return 0;
}

void CGB_T18657_1::InitDefaultTimeOut()
{
	timeOutHeartFrame_ = DEFAULT_timeOutHeartFrame;
}

void CGB_T18657_1::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	timerHeartFrame_.reset(new deadline_timer(io_service,seconds(timeOutHeartFrame_)));
	AddTimer(timerHeartFrame_);
}

void CGB_T18657_1::ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
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

		timerHeartFrame_->async_wait(boost::bind(&CGB_T18657_1::handle_timerHeartFrame,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerHeartFrame_->cancel();
	}
}

void CGB_T18657_1::handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,boost::logic::indeterminate,point);
		if (val)
		{
			int funCode = getNextPrimaryDataFunCode(getCommPointIndexByPtr(val));
			if (funCode >= 0)
			{
				AddOnlySendCmdWithoutVal(CALL_PRIMARY_DATA,CALL_PRIMARY_DATA_PRIORITY,val,funCode);
			}
			else
			{
				ResetTimerHeartFrame(val,true);
			}
		}
	}
}

int CGB_T18657_1::InitProtocol()
{

	CProtocol::InitProtocol();

	InitFrameLocation(7);

	InitPrimaryDataFunCodeSet();
	InitSecondaryDataFunCodeSet();

	if(getCommPointSum() > 0)
	{
		share_commpoint_ptr nextPoint = getFirstCommPoint();
		if (nextPoint)
		{
			int funCode = getNextPrimaryDataFunCode(getCommPointIndexByPtr(nextPoint));
			if (funCode >= 0)
			{
				//AddSendCmdVal(RESET_LINK,CALL_PRIMARY_DATA_PRIORITY + 1,nextPoint);
				//AddSendCmdVal(LINK_STATUS,CALL_PRIMARY_DATA_PRIORITY + 1,nextPoint);
				AddOnlySendCmdWithoutVal(CALL_PRIMARY_DATA,CALL_PRIMARY_DATA_PRIORITY,nextPoint,funCode);
			}
		}
	}

	AddStatusLogWithSynT("GB_T18657_1规约的通道打开成功。\n");

	return 0;
}

void CGB_T18657_1::UninitProtocol()
{
	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("GB_T18657_1规约的通道关闭成功。\n");
}

int CGB_T18657_1::getAddrByRecvFrame(unsigned char * buf)
{
	return BufToVal(&buf[TerminalAddrLocation_],TerminalAddrLength_);
}

int CGB_T18657_1::getFCB(share_terminal_ptr terminalPtr)
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

int CGB_T18657_1::ParseBasicYxData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int DataLength = 3;

	int count = 0;

	for (int i=0;i<DataLength;i++)
	{
		for (int j=0;j<8;j++)
		{
			int info_addr = i*8+j;

			if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYxSum())
			{
				terminalPtr->SaveYxQuality(info_addr,DataBase::CYxPoint::QualityActive);
				terminalPtr->SaveYxType(info_addr,DataBase::single_yx_point);

				int ret = 0;

				if((buf[FunDataLocation_ + i] & BYTE_CHECK_TRUE[j]) > 0)
				{
					ret = terminalPtr->SaveOriYxVal(info_addr,1);
				}
				else
				{
					ret = terminalPtr->SaveOriYxVal(info_addr,0);
				}

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

int CGB_T18657_1::ParseBasicYcData(unsigned char * buf, share_terminal_ptr terminalPtr)
{
	int YcVarCount = 0;
	int byteCount = 0;
	int info_addr = 0;

	for (int i=0;i<3;i++) //3U
	{
		if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
		{
			unsigned short u_val = BcdToVal(&buf[FunDataLocation_ + byteCount],2);
			byteCount += 2;

			int ret = terminalPtr->SaveOriYcVal(info_addr,u_val);
			info_addr++;

			if (ret == DataBase::CauseActiveData)
			{
				YcVarCount++;
			}
		}
	}

	for (int i=0;i<4;i++) //4I
	{
		if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
		{
			bool bSigned = false;
			short i_val = BcdToValWithHighBitSign(&buf[FunDataLocation_ + byteCount],2,bSigned);
			byteCount += 2;

			if (bSigned)
			{
				i_val = i_val * (-1);
			}
			int ret = terminalPtr->SaveOriYcVal(info_addr,i_val);
			info_addr++;

			if (ret == DataBase::CauseActiveData)
			{
				YcVarCount++;
			}
		}
	}

	if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum()) //temperature
	{
		unsigned char temperature = BufToVal(&buf[FunDataLocation_ + byteCount],1);
		byteCount += 1;

		int ret = terminalPtr->SaveOriYcVal(info_addr,temperature);
		info_addr++;

		if (ret == DataBase::CauseActiveData)
		{
			YcVarCount++;
		}
	}

	for (int i=0;i<8;i++) //P,Q
	{
		if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
		{
			bool bSigned = false;
			short P_Q = BcdToValWithHighBitSign(&buf[FunDataLocation_ + byteCount + 1],2,bSigned);
			byteCount += 3;

			if (bSigned)
			{
				P_Q = P_Q * (-1);
			}
			int ret = terminalPtr->SaveOriYcVal(info_addr,P_Q);
			info_addr++;

			if (ret == DataBase::CauseActiveData)
			{
				YcVarCount++;
			}
		}
	}

	for (int i=0;i<4;i++) //cos&
	{
		if (info_addr >= 0 && info_addr < (int)terminalPtr->getRecvYcSum())
		{
			bool bSigned = false;
			short cos = BcdToValWithHighBitSign(&buf[FunDataLocation_ + byteCount],2,bSigned);
			byteCount += 2;

			if (bSigned)
			{
				cos = cos * (-1);
			}
			int ret = terminalPtr->SaveOriYcVal(info_addr,cos);
			info_addr++;

			if (ret == DataBase::CauseActiveData)
			{
				YcVarCount++;
			}
		}
	}

	if (YcVarCount > 0)
	{
		std::ostringstream ostr;
		ostr<<"全YC报文产生了ycvar，数目："<<YcVarCount<<std::endl;
		AddStatusLogWithSynT(ostr.str());
		CmdConSig_(YCVAR_DATA_UP,RETURN_CODE_ACTIVE,terminalPtr,YcVarCount);
	}

	return 0;
}

};//namespace Protocol 

