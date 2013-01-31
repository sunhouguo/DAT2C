#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "YkPoint.h"
#include "YkReadyState.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

#define strSYkDouble "SDoubleYk"

CYkPoint::CYkPoint()
{
	ykType_ = DefaultYkType;
	//ykStatus_ = DefaultYkStatus;
	ykHDouble_ = DefaultYkDouble;
	ykSDouble_ = DefaultYkDouble;
	ykAllow_ = true;

	ChangeState(new CYkReadyState());
}

CYkPoint::~CYkPoint(void)
{
}

CYkPoint::CYkPoint(CYkPoint const & rhs)
{
	ykHDouble_ = rhs.ykHDouble_;
	ykSDouble_ = rhs.ykSDouble_;
	ykAllow_ = rhs.ykAllow_;
	ykType_ = ykType_;

	ChangeState(new CYkReadyState());
}

CYkPoint & CYkPoint::operator=(CYkPoint const & rhs)
{
	if (this == &rhs)
	{
		return * this;
	}

	ykHDouble_ = rhs.ykHDouble_;
	ykSDouble_ = rhs.ykSDouble_;
	ykAllow_ = rhs.ykAllow_;
	ykType_ = ykType_;

	ChangeState(new CYkReadyState());

	return * this;
}

//typeYkstatus CYkPoint::getYkStatus()
//{
//	return ykStatus_;
//}
//
//int CYkPoint::setYkStatus(typeYkstatus val)
//{
//	ykStatus_ = val;
//
//	return 0;
//}

typeYktype CYkPoint::getYkType()
{
	return ykType_;
}

int CYkPoint::setYkType(typeYktype val)
{
	ykType_ = val;

	return 0;
}

bool CYkPoint::getbHYkDouble()
{
	return ykHDouble_;
}

int CYkPoint::setbHYkDouble(bool val)
{
	ykHDouble_ = val;

	return 0;
}

bool CYkPoint::getbSYkDouble()
{
	return ykSDouble_;
}

int CYkPoint::setbSYkDouble(bool val)
{
	ykSDouble_ = val;

	return 0;
}

bool CYkPoint::getbYkAllow()
{
	return ykAllow_;
}

int CYkPoint::setbYkAllow(bool val)
{
	ykAllow_ = val;

	return 0;
}

//xml api
void CYkPoint::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	xml.AddElem(strYkDouble,TransYkDoubleToString(getbHYkDouble()));
	xml.AddElem(strYkDouble,TransYkDoubleToString(getbSYkDouble()));
}

int CYkPoint::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	xml.ResetMainPos();
	if (xml.FindElem(strYkDouble))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		bool val = TransYkDoulbeFromString(strTmp);
		setbHYkDouble(val);
	}

	return 0;
}

std::string CYkPoint::TransYkDoubleToString(bool val)
{
	std::string strTmp = strboolTrue;

	if (val)
	{
		strTmp = strboolTrue;
	}
	else
	{
		strTmp = strboolFalse;
	}

	return strTmp;
}

bool CYkPoint::TransYkDoulbeFromString(std::string val)
{
	bool ret = true;

	if (boost::iequals(strboolTrue,val))
	{
		ret = true;
	}
	else if (boost::iequals(strboolFalse,val))
	{
		ret = false;
	}

	return ret;


}

int CYkPoint::RecvSelEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->RecvSelEvent(*this);
	}

	return -1;
}

int CYkPoint::SendSelEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->SendSelEvent(*this);
	}

	return -1;
}

int CYkPoint::BackSelEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->BackSelEvent(*this);
	}

	return -1;
}

int CYkPoint::SelResponEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->SelResponEvent(*this);
	}

	return -1;
}

int CYkPoint::RecvExeEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->RecvExeEvent(*this);
	}

	return -1;
}

int CYkPoint::SendExeEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->SendExeEvent(*this);
	}

	return -1;
}

int CYkPoint::BackExeEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->BackExeEvent(*this);
	}

	return -1;
}

int CYkPoint::ExeResponEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->ExeResponEvent(*this);
	}

	return -1;
}

int CYkPoint::RecvCancelEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->RecvCancelEvent(*this);
	}

	return -1;
}

int CYkPoint::SendCancelEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->SendCancelEvent(*this);
	}

	return -1;
}

int CYkPoint::BackCancelEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->BackCancelEvent(*this);
	}

	return -1;
}

int CYkPoint::CancelResponEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->CancelResponEvent(*this);
	}

	return -1;
}

int CYkPoint::OverYkEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->OverYkEvent(*this);
	}

	return -1;
}

int CYkPoint::TimeOutEvent()
{
	if (getbYkAllow() && ykState_)
	{
		return ykState_->TimeOutEvent(*this);
	}

	return -1;
}

int CYkPoint::ClearYkState()
{
	ChangeState(new CYkReadyState());

	return 0;
}

void CYkPoint::ChangeState(CYkState * state)
{
	ykState_.reset(state);
}


}; //namespace DataBase 


