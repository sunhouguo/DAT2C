#include <iostream>
#include <boost/bind.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include "OutLetSwitch.h"
#include "../DataBase/DAOperate.h"
#include "../FileSystem/Markup.h"
#include "../Protocol/Protocol.h"
#include "../DataBase/YxPoint.h"

namespace CentralizedFA {

const std::string strSwitchID = "SwitchID";
const std::string strSwitchName = "SwitchName";
const std::string strPositionIndex = "PositionIndex";
const std::string strProtectIndex = "ProtectIndex";
const std::string strYkIndex = "YkIndex";
const std::string strAllowYk = "AllowYk";
const std::string strAllowYx = "AllowYx";
const std::string strInputYx = "InputYx";
const std::string strOutputYx = "OutputYx";
const std::string strPolar = "Polar";

//const unsigned char InYx_FA_ComInterrupt = 0;    //开关通讯中断
//const unsigned char InYx_FA_YkDisable = 1;       //开关遥控远方就地闭锁
//const unsigned char InYx_FA_PowerOff = 2;        //开关储能
//const unsigned char InYx_FA_TerminalFault = 3;   //终端故障
//const unsigned char InYx_FA_SwitchFault = 4;     //负荷开关故障
//const unsigned char InYx_FA_BatteryFault = 6;    //终端电池故障

const unsigned char OutYx_Switch_Act = 0;          //终端遥控动作启动
const unsigned char OutYx_Switch_Over = 1;         //终端遥控动作结束
const unsigned char OutYx_Switch_Error = 2;        //终端遥控返校失败
const unsigned char OutYx_Switch_Refuse = 3;       //终端遥控拒动
const unsigned char OutYx_FA_OverLoad = 10;        //联络开关预判过负荷

COutLetSwitch::COutLetSwitch(DataBase::CDAOperate & da_op)
							:da_op_(da_op)
{
	SwitchID_ = -1;
	bAllowYk_ = true;
	bAllowYx_ = false;

	//SwitchLoadValue_=0;
}

COutLetSwitch::~COutLetSwitch(void)
{
	
}

std::string COutLetSwitch::getSwitchName()
{
	return name_;
}

int COutLetSwitch::setSwitchName(std::string val)
{
	name_ = val;

	return 0;
}

typeID COutLetSwitch::getSwitchID()
{
	return SwitchID_;
}

int COutLetSwitch::setSwitchID(typeID val)
{
	SwitchID_ = val;

	return 0;
}

bool COutLetSwitch::getSwitchPosition()
{
	if(da_op_.getYxVal(PositionYxIndex_))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool COutLetSwitch::getSwitchProtected()
{
	if(da_op_.getYxVal(ProtectedYxIndex_))
	{
		return true;
	}
	else
	{
		return false;
	}
}

int COutLetSwitch::setAllowYk(bool val)
{
	bAllowYk_ = val;

	return 0;
}

bool COutLetSwitch::AllowYk()
{
	return bAllowYk_;
}

bool COutLetSwitch::AllowYx()
{
	return bAllowYx_;
}

int COutLetSwitch::getPositionIndex()
{
	return PositionYxIndex_;
}

int COutLetSwitch::setPositionIndex(int val)
{
	if (val < 0 || val >= (int)da_op_.getYxSum())
	{
		PositionYxIndex_ = val;

		return 0;
	}

	return -1;

}
int COutLetSwitch::getProtectIndex()
{
	return ProtectedYxIndex_;
}

int COutLetSwitch::setProtectIndex(int val)
{
	if (val < 0 || val >= (int)da_op_.getYxSum())
	{
		ProtectedYxIndex_ = val;

		return 0;
	}

	return -1;
}

int COutLetSwitch::getOutYkIndex()
{
	return OutYkIndex_;
}

int COutLetSwitch::setOutYkIndex(int val)
{
	if (val < 0 || val >= (int)da_op_.getYkSum())
	{
		OutYkIndex_ = val;

		return 0;
	}

	return -1;
}

int COutLetSwitch::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	using namespace std;

	xml.ResetMainPos();
	if (xml.FindElem(strSwitchID))
	{
		typeID id;
		try
		{
			string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			id = boost::lexical_cast<typeID>(strTmp);
			setSwitchID(id);
		}
		catch(boost::bad_lexical_cast& e)
		{
			ostringstream ostr;
			ostr<<"非法的开关ID参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("未找到开关ID配置项");
	}

	xml.ResetMainPos();
	if (xml.FindElem(strSwitchName))
	{
		string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setSwitchName(strTmp);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strPositionIndex))
	{
		size_t index;
		try
		{
			string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			index = boost::lexical_cast<size_t>(strTmp);
			setPositionIndex(index);
		}
		catch(boost::bad_lexical_cast& e)
		{
			ostringstream ostr;
			ostr<<"非法的开关位置遥信参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("未找到开关位置遥信配置项");
	}

	xml.ResetMainPos();
	if (xml.FindElem(strProtectIndex))
	{
		size_t index;
		try
		{
			string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			index = boost::lexical_cast<size_t>(strTmp);
			setProtectIndex(index);
		}
		catch(boost::bad_lexical_cast& e)
		{
			ostringstream ostr;
			ostr<<"非法的开关保护遥信参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("未找到开关位保护遥信配置项");
	}

	xml.ResetMainPos();
	if (xml.FindElem(strAllowYk))
	{
		string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strTmp,strboolTrue))
		{
			bAllowYk_ = true;
		}
		else
		{
			bAllowYk_ = false;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strAllowYx))
	{
		string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strTmp,strboolTrue))
		{
			bAllowYx_ = true;
		}
		else
		{
			bAllowYx_ = false;
		}
	}

	if(AllowYk())
	{
		xml.ResetMainPos();
		if (xml.FindElem(strYkIndex))
		{
			size_t index;
			try
			{
				string strTmp = xml.GetData();
				boost::algorithm::trim(strTmp);
				index = boost::lexical_cast<size_t>(strTmp);
				setOutYkIndex(index);
			}
			catch(boost::bad_lexical_cast& e)
			{
				ostringstream ostr;
				ostr<<"非法的开关遥控参数:"<<e.what();

				throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
			}
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("未找到开关位遥控配置项");
		}
	}

	xml.ResetMainPos();
	while(xml.FindElem(strInputYx))
	{
		try
		{
			string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			size_t index = boost::lexical_cast<size_t>(strTmp);

			strTmp = xml.GetAttrib(strPolar);
			bool polar = true;
			if (boost::iequals(strTmp,strboolFalse))
			{
				polar = false;
			}

			strTmp = xml.GetAttrib(strOutputYx);
			int out = boost::lexical_cast<int>(strTmp);

			stIn val(index,out,polar);
			input_set_.push_back(val);
		}
		catch(boost::bad_lexical_cast & e)
		{
			ostringstream ostr;
			ostr<<"非法的输入遥信参数:"<<e.what();
			cerr<<ostr.str();
		}
	}

	xml.ResetMainPos();
	while(xml.FindElem(strOutputYx))
	{
		try
		{
			string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			size_t index = boost::lexical_cast<size_t>(strTmp);
			strTmp = xml.GetAttrib(strPolar);
			bool polar = true;
			if (boost::iequals(strTmp,strboolFalse))
			{
				polar = false;
			}
			stOut val(index,polar);
			output_set_.push_back(val);
		}
		catch(boost::bad_lexical_cast & e)
		{
			ostringstream ostr;
			ostr<<"非法的输出遥信参数:"<<e.what();
			cerr<<ostr.str();
		}
	}

	//xml.ResetMainPos();
	//if (xml.FindElem(strOverLoadChannel))
	//{
	//	try
	//	{
	//		string strTmp = xml.GetData();
	//		boost::algorithm::trim(strTmp);
	//		unsigned short value = boost::lexical_cast<size_t>(strTmp);
	//		SwitchLoad_=value;
	//	}
	//	catch(boost::bad_lexical_cast& e)
	//	{
	//		ostringstream ostr;
	//		ostr<<"非法的遥测点号参数:"<<e.what();

	//		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
	//	}
	//}

	return 0;
}

void COutLetSwitch::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	xml.AddElem(strSwitchID,getSwitchID());
	xml.AddElem(strSwitchName,getSwitchName());
	xml.AddElem(strPositionIndex,getPositionIndex());
	xml.AddElem(strProtectIndex,getProtectIndex());
	
	if (AllowYk())
	{
		xml.AddElem(strAllowYk,strboolTrue);
		xml.AddElem(strYkIndex,getOutYkIndex());
	}
	else
	{
		xml.AddElem(strAllowYk,strboolFalse);
	}

	if (AllowYx())
	{
		xml.AddElem(strAllowYx,strboolTrue);
	}
	else
	{
		xml.AddElem(strAllowYx,strboolFalse);
	}

	for(std::vector<stIn>::iterator it = input_set_.begin();it != input_set_.end();it++)
	{
		std::string polar;
		if((*it).polar_)
		{
			polar = strboolTrue;
		}
		else
		{
			polar = strboolFalse;
		}

		xml.AddElem(strInputYx,(*it).index_);
		xml.AddAttrib(strPolar,polar);
		xml.AddAttrib(strOutputYx,(*it).out_);
	}

	for(std::vector<stOut>::iterator it = output_set_.begin();it != output_set_.end();it++)
	{
		std::string polar;
		if ((*it).polar_)
		{
			polar = strboolTrue;
		}
		else
		{
			polar = strboolFalse;
		}

		xml.AddElem(strOutputYx,(*it).index_);
		xml.AddAttrib(strPolar,polar);
	}
}

int COutLetSwitch::ConnectYkSig()
{
	SubYkSigConnection_ = da_op_.ConnectYkSig(boost::bind(&COutLetSwitch::ProcessSubAlgorithmSig,this,_1,_2,_3,_4));

	return 0;
}

int COutLetSwitch::DisconnectSubYkSig()
{
	SubYkSigConnection_.disconnect();

	return 0;
}

int COutLetSwitch::OpearteSwitchSel(bool bCloseOrOpen)
{
	boost::unique_lock<boost::mutex> uLock(ykMutex_,boost::try_to_lock);
	if(uLock.owns_lock())
	{
		if(!da_op_.AddYkSelCmd(OutYkIndex_,bCloseOrOpen))
		{
			ConnectYkSig();

			if(ykCond_.timed_wait(uLock,boost::get_system_time() + boost::posix_time::seconds(wait_ykcon_timeout)))
			{
				DisconnectSubYkSig();

				return 0;
			}
		}
	}

	DisconnectSubYkSig();

	return -1;
}

int COutLetSwitch::OpearteSwitchExe(bool bCloseOrOpen)
{
	boost::unique_lock<boost::mutex> uLock(ykMutex_,boost::try_to_lock);
	if(uLock.owns_lock())
	{
		if(!da_op_.AddYkExeCmd(OutYkIndex_,bCloseOrOpen))
		{
			ConnectYkSig();

			if(ykCond_.timed_wait(uLock,boost::get_system_time() + boost::posix_time::seconds(wait_ykcon_timeout)))
			{
				DisconnectSubYkSig();

				return 0;
			}
		}
	}

	DisconnectSubYkSig();

	return -1;
}

void COutLetSwitch::SwitchActStart()
{
	WriteOutputYx(OutYx_Switch_Act,1,true);
}

void COutLetSwitch::SwitchActOver()
{
	WriteOutputYx(OutYx_Switch_Over,1,true);
}

void COutLetSwitch::SwitchActError()
{
	WriteOutputYx(OutYx_Switch_Error,1,true);
}

void COutLetSwitch::SwitchRefuseAct()
{
	WriteOutputYx(OutYx_Switch_Refuse,1,true);
}

int COutLetSwitch::OpenSwitch()
{
	if (AllowYk())
	{
		SwitchActStart();

		int ret = OpearteSwitchSel(false);
		if (ret)
		{
			SwitchRefuseAct();//返校失败，拒动

			return ret;
		}

		return OpearteSwitchExe(false);
	}

	return 0;
}

int COutLetSwitch::CloseSwitch()
{
	if(AllowYk())
	{
		SwitchActStart();

		int ret = OpearteSwitchSel(true);
		if (ret)
		{
			SwitchRefuseAct();//返校失败，拒动

			return ret;
		}

		return OpearteSwitchExe(true);
	}

	return 0;
}

void COutLetSwitch::ProcessSubAlgorithmSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	switch(cmdType)
	{
	case Protocol::YK_SEL_CON:
		{
			try
			{
				int index =boost::any_cast<int>(val);
				if (index == getOutYkIndex() && ReturnCode == RETURN_CODE_ACTIVE)
				{
					boost::unique_lock<boost::mutex> uLock(ykMutex_,boost::try_to_lock);
					if (uLock.owns_lock())
					{
						ykCond_.notify_one();
					}
				}
			}
			catch(boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控选择确认消息的参数错误:"<<e.what()<<std::endl;
				std::cerr<<ostr.str();
			}
		}
		break;

	case Protocol::YK_EXE_CON:
		{
			try
			{
				int index =boost::any_cast<int>(val);
				if (index == getOutYkIndex() && ReturnCode == RETURN_CODE_ACTIVE)
				{
					boost::unique_lock<boost::mutex> uLock(ykMutex_,boost::try_to_lock);
					if (uLock.owns_lock())
					{
						ykCond_.notify_one();
					}
				}
			}
			catch(boost::bad_any_cast & e)
			{
				std::ostringstream ostr;
				ostr<<"遥控执行确认消息的参数错误:"<<e.what()<<std::endl;
				std::cerr<<ostr.str();
			}
		}
		break;

	default:
		break;
	}
}

bool COutLetSwitch::CheckInputYxSet()
{
	for (std::vector<stIn>::iterator it = input_set_.begin(); it != input_set_.end(); it++)
	{
		size_t yxindex = (*it).index_;
		bool bPolar = (*it).polar_;

		bool bSingleYx = (da_op_.getYxType(yxindex) == DataBase::single_yx_point);
		typeYxval yxval = da_op_.getYxVal(yxindex);


		bool bRet = false;

		if (bSingleYx)
		{
			if (bPolar)
			{
				bRet = (yxval == 0x00);
			}
			else
			{
				bRet = (yxval == 0x01);
			}
		}
		else
		{
			if (bPolar)
			{
				bRet = (yxval == 0x01);
			}
			else
			{
				bRet = (yxval == 0x10);
			}
		}

		if (!bRet && (*it).out_ > 0)
		{
			(*it,1,true);

			return bRet;
		}
	}

	return true;
}

int COutLetSwitch::WriteInputYx(stIn st,typeYxval val ,bool bSingleType)
{
	if (st.out_ > 0)
	{
		if (!st.polar_)
		{
			if (bSingleType)
			{
				val = ((~val) & 0x01);
			}
			else
			{
				val = ((~val) & 0x03);
			}
		}

		da_op_.TrigCosEvent(st.out_,val,bSingleType);

		return 0;
	}

	return -1;
}

int COutLetSwitch::WriteOutputYx(int outputindex,typeYxval val,bool bSingleType /* = true*/)
{
	if (outputindex < 0 || outputindex >= (int)output_set_.size())
	{
		return -1;
	}

	if (!output_set_[outputindex].polar_)
	{
		if (bSingleType)
		{
			val = ((~val) & 0x01);
		}
		else
		{
			val = ((~val) & 0x03);
		}
	}

	da_op_.TrigCosEvent(output_set_[outputindex].index_,val,bSingleType);

	return 0;
}

//bool COutLetSwitch::ResetOutputYx(typeYxval val,bool bSingleType)
//{
//	for (std::vector<stInOut>::iterator it = output_set_.begin(); it != output_set_.end(); it++)
//	{
//		if (!(*it).polar_)
//		{
//			if (bSingleType)
//			{
//				val = ((~val) & 0x01);
//			}
//			else
//			{
//				val = ((~val) & 0x03);
//			}
//		}
//
//		da_op_.SaveOriYxVal((*it).index_,val,true);
//	}
//
//	return true;
//}

//void COutLetSwitch::GetLoad()
//{
//	typeYcval yc_val = da_op_.getYcVal(SwitchLoad_);
//
//	if((yc_val<= MaxOverLoadYc) && (yc_val >= MinOverLoadYc))
//	{
//		SwitchLoadValue_ = yc_val;
//	}
//}
//
//int COutLetSwitch::GetSwitchLoad()
//{
//	return SwitchLoadValue_;
//}
//
//int COutLetSwitch::GetYkIndex()
//{
//	return OutYkIndex_;
//}

}; //namespace FeederAutomation
