#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include "SwitchNode.h"
#include "../FileSystem/Markup.h"
#include "../DataBase/DAOperate.h"

namespace CentralizedFA {

const std::string strLinkSwitchID = "LinkID";
const std::string strSwitchType = "SwitchType";
const std::string strOutLetNode = "OutLetNode";
const std::string strPowerSwitch = "PowerSwitch";
const std::string strNormalSwitch = "NormalSwitch";

const std::string strUIndex = "YcIndex";
const std::string strYcOverLoad = "YcOverLoad";


const unsigned char PowerSwitch = 1;
const unsigned char NormalSwitch = 2;

CSwitchNode::CSwitchNode(DataBase::CDAOperate & da_op)
						:COutLetSwitch(da_op)
{
	SwitchType_ = NormalSwitch;

	bYcOverLoad_ = false;

	outlet_.reset();
}

CSwitchNode::~CSwitchNode(void)
{
}

bool CSwitchNode::getbPowerNode()
{
	return SwitchType_ == PowerSwitch;
}

unsigned char CSwitchNode::getSwitchType()
{
	return SwitchType_;
}

int CSwitchNode::setSwitchType(unsigned char val)
{
	SwitchType_ = val;

	return 0;
}

bool CSwitchNode::CheckNodebLinked(typeID OtherNodeID)
{
	for (std::vector<typeID>::iterator it = LinkedNodeIDSet_.begin();it != LinkedNodeIDSet_.end();it++)
	{
		if(*it == OtherNodeID)
		{
			return true;
		}
	}

	return false;
}

share_outletswitch_ptr CSwitchNode::getOutLetSwitch()
{
	return outlet_;
}

unsigned char CSwitchNode::TransSwitchTypeFromString(std::string val)
{
	unsigned char ret = NormalSwitch;

	if (boost::iequals(strNormalSwitch,val))
	{
		ret = NormalSwitch;
	}
	else if (boost::iequals(strPowerSwitch,val))
	{
		ret = PowerSwitch;
	}

	return ret;
}

std::string CSwitchNode::TransSwitchTypeToString(unsigned char val)
{
	std::string ret;

	switch (val)
	{
	case NormalSwitch:
		ret = strNormalSwitch;
		break;

	case PowerSwitch:
		ret = strPowerSwitch;
		break;

	default:
		break;
	}

	return ret;
}

int CSwitchNode::setYcIndex(size_t val)
{
	UIndex_ = val;

	return 0;
}

size_t CSwitchNode::getYcIndex()
{
	return UIndex_;
}

bool CSwitchNode::getSwitchProtected()
{
	if (bYcOverLoad_)
	{
		if(da_op_.getYxVal(getProtectIndex()) && (da_op_.getYcVal(UIndex_) < yc_limit_val))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if(da_op_.getYxVal(getProtectIndex()))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

int CSwitchNode::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	using namespace std;

	COutLetSwitch::LoadXmlCfg(xml);

	xml.ResetMainPos();
	if (xml.FindElem(strSwitchType))
	{
		string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setSwitchType(TransSwitchTypeFromString(strTmp));
	}


	xml.ResetMainPos();
	while(xml.FindElem(strLinkSwitchID))
	{
		typeID id;
		try
		{
			string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			id = boost::lexical_cast<typeID>(strTmp);
			LinkedNodeIDSet_.push_back(id);
		}
		catch(boost::bad_lexical_cast& e)
		{
			ostringstream ostr;
			ostr<<"非法的邻接开关ID参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strOutLetNode))
	{
		outlet_.reset(new COutLetSwitch(da_op_));
		xml.IntoElem();

		outlet_->LoadXmlCfg(xml);

		xml.OutOfElem();
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYcOverLoad))
	{
		string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strTmp,strboolTrue))
		{
			bYcOverLoad_ = true;
		}
		else
		{
			bYcOverLoad_ = false;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strUIndex))
	{
		try
		{
			string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			setYcIndex(boost::lexical_cast<size_t>(strTmp));
		}
		catch(boost::bad_lexical_cast& e)
		{
			ostringstream ostr;
			ostr<<"非法的遥测点号参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}

	return 0;
}

void CSwitchNode::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	COutLetSwitch::SaveXmlCfg(xml);

	xml.AddElem(strSwitchType,TransSwitchTypeToString(getSwitchType()));
	for (std::vector<typeID>::iterator it = LinkedNodeIDSet_.begin();it != LinkedNodeIDSet_.end();it++)
	{
		xml.AddElem(strLinkSwitchID,*it);
	}
	if (outlet_)
	{
		xml.AddElem(strOutLetNode);
		xml.IntoElem();
		outlet_->SaveXmlCfg(xml);
		xml.OutOfElem();
	}
	if (bYcOverLoad_)
	{
		xml.AddElem(strYcOverLoad,strboolTrue);
	}
	else
	{
		xml.AddElem(strYcOverLoad,strboolFalse);
	}
	xml.AddElem(strUIndex,getYcIndex());
}

}; //namespace FeederAutomation 

