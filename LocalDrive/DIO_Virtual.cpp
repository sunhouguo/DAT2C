#include <boost/shared_ptr.hpp>
#include <iostream>
#include "DIO_Virtual.h"
#include "../PublicSupport/Dat2cTypeDef.h"
#include "../DataBase/BF533Terminal.h"
#include "../DataBase/SubStation.h"
#include "../DataBase/YkPoint.h"
#include "../FileSystem/Markup.h"

namespace LocalDrive {

const std::string DioCfgName = "VirtualDoCfg.xml";

const std::string strRootDIO = "DIO";
const std::string strAllowYk = "AllowYk";
const std::string strResetYkIndex = "ResetYkIndex";
const std::string strResetDevIndex = "ResetDevIndex";
const std::string strActiveBatteryIndex = "ActiveBatteryIndex";
const std::string strYkAllowNotifyIndex = "YkAllowNotifyIndex";

CDIO_Virtual::CDIO_Virtual(DataBase::CSubStation & sub)
						:sub_(sub),
						MaxDoNum_(sub_.getSubYkSum()),
						MaxDiNum_(sub_.getSubYxSum())
{
	bAllowYk_ = true;

	ActiveBatteryIndex_ = DefaultActiveBatteryIndex;
	ResetDevIndex_ = DefaultResetDevIndex;
	ResetYkAllowIndex_ = DefaultResetYkAllowIndex;
	AllowYkNotifyIndex_ = DefaultAllowYkNotifyIndex;
}


CDIO_Virtual::~CDIO_Virtual(void)
{
}

int CDIO_Virtual::CheckAllIndex()
{
	if ((getActiveBatteryIndex() >= 0 && getActiveBatteryIndex() < MaxDoNum_)
		&&(getResetDevIndex() >= 0 && getResetDevIndex() < MaxDoNum_)
		&&(getResetYkAllowIndex() >= 0 && getResetYkAllowIndex() < MaxDoNum_))
	{
		return 0;
	}

	return -1;
}

int CDIO_Virtual::open()
{
	LoadXmlCfg(DioCfgName);

	
	if (CheckAllIndex() == 0)
	{
		return ResetSubYkAllow();
	}

	std::cerr<<"Open DIO_Virtual Err"<<std::endl;
	
	return -1;
}

void CDIO_Virtual::close()
{
	SaveXmlCfg(DioCfgName);
}

int CDIO_Virtual::check_di(int index)
{
	if (index < 0 || index >= MaxDiNum_)
	{
		return -1;
	}

	return 0;
}

int CDIO_Virtual::read_di(int index)
{
	if (index < 0 || index >= MaxDiNum_)
	{
		return -1;
	}

	return sub_.getFinalYxVal(sub_.getTerminalYxSum() + index);
}

int CDIO_Virtual::check_do(int index)
{
	if (index < 0 || index >= MaxDoNum_)
	{
		return -1;
	}

	return 0;
}

unsigned char CDIO_Virtual::read_do(int index)
{
	if (index < 0 || index >= MaxDoNum_)
	{
		return -1;
	}

	return 0;
}

int CDIO_Virtual::write_do(int index,bool bCloseOrOpen)
{
	if (index < 0 || index>= MaxDoNum_)
	{
		return -1;
	}

	if (index == getActiveBatteryIndex())
	{
		ActiveBattery(bCloseOrOpen);
	}
	else if (index == getResetDevIndex())
	{
		ResetDev(bCloseOrOpen);
	}
	else if (index == getResetYkAllowIndex())
	{
		ResetYkAllow(bCloseOrOpen);
	}

	return 0;
}

int CDIO_Virtual::ActiveBattery(bool val)
{
	DataBase::share_bf533_ptr bf533 = sub_.getBf533Terminal();

	if (bf533)
	{
		return bf533->ActiveBattery(val);
	}

	return -1;
}

int CDIO_Virtual::ResetDev(bool val)
{
	sub_.ResetDev(val);

	return 0;
}

int CDIO_Virtual::ResetSubYkAllow()
{
	for (size_t i = 0; i < sub_.getYkSum(); i++)
	{
		sub_.getYkPointPtr(i)->setbYkAllow(getbAllowYk());
	}

	sub_.getYkPointPtr(sub_.getTerminalYkSum() + getResetYkAllowIndex())->setbYkAllow(true);

	typeYxval yxval;
	if (getbAllowYk())
	{
		yxval = 1;
	}
	else
	{
		yxval = 0;
	}

	int NotifyYxIndex = getAllowYkNotifyIndex();
	if(NotifyYxIndex >=0 && NotifyYxIndex < MaxDiNum_)
	{
		sub_.TrigCosEvent(sub_.getTerminalYxSum() + NotifyYxIndex,yxval,true);
		sub_.TrigSoeEvent(sub_.getTerminalYxSum() + NotifyYxIndex,yxval,boost::posix_time::microsec_clock::local_time(),true);
	}

	return 0;
}

int CDIO_Virtual::ResetYkAllow(bool val)
{
	setbAllowYk(val);

	ResetSubYkAllow();

	SaveXmlCfg(DioCfgName);

	return 0;
}

int CDIO_Virtual::LoadXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;

	if (!xml.Load(filename))
	{
		//std::cout<<"导入xml配置文件"<<filename<<"失败！"<<std::endl;

		return -1;
	}

	xml.ResetMainPos();
	xml.FindElem(); //root strRootDIO
	xml.IntoElem();  //enter strRootDIO

	xml.ResetMainPos();
	if (xml.FindElem(strAllowYk))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strTmp,strboolFalse))
		{
			setbAllowYk(false);
		}
		else
		{
			setbAllowYk(true);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strActiveBatteryIndex))
	{
		int index;
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			index = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的电池活化序号参数："<<e.what()<<"，将使用默认值\n";
			std::cerr<<ostr.str();

			index = DefaultActiveBatteryIndex;
		}

		setActiveBattryIndex(index);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strResetDevIndex))
	{
		int index;
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			index = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的重启装置序号参数："<<e.what()<<"，将使用默认值\n";
			std::cerr<<ostr.str();

			index = DefaultResetDevIndex;
		}

		setResetDevIndex(index);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strResetYkIndex))
	{
		int index;
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			index = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的重置遥控软压板序号参数："<<e.what()<<"，将使用默认值\n";
			std::cerr<<ostr.str();

			index = DefaultResetYkAllowIndex;
		}

		setResetYkAllowIndex(index);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYkAllowNotifyIndex))
	{
		int index;
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			index = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的遥控软压板遥信通知序号参数："<<e.what()<<"，将使用默认值\n";
			std::cerr<<ostr.str();

			index = DefaultAllowYkNotifyIndex;
		}

		setAllowYkNotifyIndex(index);
	}

	xml.OutOfElem(); //out strRootDIO

	return 0;
}

void CDIO_Virtual::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strSubXsl);

	xml.AddElem(strRootDIO);
	xml.IntoElem();//enter strRootDIO

	if (getbAllowYk())
	{
		xml.AddElem(strAllowYk,strboolTrue);
	}
	else
	{
		xml.AddElem(strAllowYk,strboolFalse);
	}

	if (getActiveBatteryIndex() != DefaultActiveBatteryIndex)
	{
		xml.AddElem(strActiveBatteryIndex,getActiveBatteryIndex());
	}

	if (getResetDevIndex() != DefaultResetDevIndex)
	{
		xml.AddElem(strResetDevIndex,getResetDevIndex());
	}

	if (getResetYkAllowIndex() != DefaultResetYkAllowIndex)
	{
		xml.AddElem(strResetYkIndex,getResetYkAllowIndex());
	}

	if (getAllowYkNotifyIndex() != DefaultAllowYkNotifyIndex)
	{
		xml.AddElem(strYkAllowNotifyIndex,getAllowYkNotifyIndex());
	}

	xml.OutOfElem(); //out strRootDIO

	xml.Save(filename);//save xml file
}

bool CDIO_Virtual::getbAllowYk()
{
	return bAllowYk_;
}

int CDIO_Virtual::setbAllowYk(bool val)
{
	bAllowYk_ = val;

	return 0;
}

int CDIO_Virtual::getActiveBatteryIndex()
{
	return ActiveBatteryIndex_;
}

int CDIO_Virtual::setActiveBattryIndex(int val)
{
	ActiveBatteryIndex_ = val;

	return 0;
}

int CDIO_Virtual::getResetDevIndex()
{
	return ResetDevIndex_;
}

int CDIO_Virtual::setResetDevIndex(int val)
{
	ResetDevIndex_ = val;

	return 0;
}

int CDIO_Virtual::getResetYkAllowIndex()
{
	return ResetYkAllowIndex_;
}

int CDIO_Virtual::setResetYkAllowIndex(int val)
{
	ResetYkAllowIndex_ = val;

	return 0;
}

int CDIO_Virtual::getAllowYkNotifyIndex()
{
	return AllowYkNotifyIndex_;
}

int CDIO_Virtual::setAllowYkNotifyIndex(int val)
{
	AllowYkNotifyIndex_ = val;

	return 0;
}

};//namespace LocalDrive
