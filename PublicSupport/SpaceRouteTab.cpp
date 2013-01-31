#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include "SpaceRouteTab.h"
#include "../FileSystem/Markup.h"
#include "../PublicSupport/Dat2cTypeDef.h"

namespace PublicSupport {

#define strDstNode "DstNode"
#define strDstIndex "DstNO."
#define strTabDstNum "TabDstNum"
#define strTerminalIndex "TerminalIndex"

CSpaceRouteTab::CSpaceRouteTab()
{
	ClearMaxSrcNum();
}

CSpaceRouteTab::CSpaceRouteTab(TransTerminalIndexFunCType val)
	:TransIndex(val)
{
	ClearMaxSrcNum();
}

CSpaceRouteTab::~CSpaceRouteTab(void)
{
	UninitRouteTab();
}

size_t CSpaceRouteTab::getDstPointNum()
{
	return RouteTab_DstToSrc_.size();
}

size_t CSpaceRouteTab::getSrcPointNum()
{
	return MaxSrcNum_;
}

int CSpaceRouteTab::InitMaxSrcNum()
{
	int no = -1;

	for (int i=0;i<(int)getDstPointNum();i++)
	{
		int srcIndex = getSrcIndexByDstNO(i);
		
		if (srcIndex < 0)
		{
			std::ostringstream ostr;
			ostr<<"转发表原始点号非法:"<<srcIndex<<std::endl;

			dat2def_exception err;
			err<<boost::errinfo_type_info_name(ostr.str());
			err<<boost::errinfo_errno(i);
			throw err;
		}

		if(srcIndex > no)
		{
			no = getSrcIndexByDstNO(i);
		}
	}

	MaxSrcNum_ = no + 1;

	return 0;
}

void CSpaceRouteTab::ClearMaxSrcNum()
{
	MaxSrcNum_ = 0;
}

int CSpaceRouteTab::InintRouteTab( FileSystem::CMarkup & xml,size_t DatabaseSum,bool bCountSrc /*= false*/ )
{
	LoadXmlCfg(xml,DatabaseSum,bCountSrc);
	InitMaxSrcNum();

	return 0;
}

void CSpaceRouteTab::UninitRouteTab()
{
	RouteTab_DstToSrc_.clear();
	ClearMaxSrcNum();
}

int CSpaceRouteTab::getDstIndexBySrcNO(size_t srcNO)
{
	if (srcNO < 0 )
	{
		return -1;
	}

	for (size_t i=0;i<getDstPointNum();i++)
	{
		if (RouteTab_DstToSrc_[i] == srcNO)
		{
			return i;
		}
	}

	return -1;
}

int CSpaceRouteTab::getSrcIndexByDstNO(size_t dstNO)
{
	if (dstNO < 0 || dstNO >= getDstPointNum())
	{
		return -1;
	}

	return RouteTab_DstToSrc_[dstNO];
}

int CSpaceRouteTab::setDstToSrcIndex(size_t index, size_t val)
{
	if (index < 0 || index >= getDstPointNum())
	{
		return -1;
	}

	RouteTab_DstToSrc_[index] = val;

	return 0;
}

void CSpaceRouteTab::addDstToSrcIndex(size_t val)
{
	RouteTab_DstToSrc_.push_back(val);
}

int CSpaceRouteTab::checkDstToSrcIndex( int dstIndex,int srcIndex,size_t srcSum,bool bCountSrc /*= false*/ )
{
	if (dstIndex < 0 || dstIndex >= (int)srcSum)
	{
		return -1;
	}
	
	if (bCountSrc)
	{
		if (srcIndex < 0  || srcIndex >= (int)srcSum)
		{
			return -1;
		}
	}
	
	return 0;
}

/*
bool CSpaceRouteTab::getbEnableTab()
{
	return bEnableTab_;
}

int CSpaceRouteTab::setbEnableTab(bool val)
{
	bEnableTab_ = val;

	return 0;
}
*/

//xml api
void CSpaceRouteTab::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	xml.AddAttrib(strTabDstNum,getDstPointNum());
	//xml.AddAttrib(strIndexOrderEnable,strboolFalse);

	xml.IntoElem();
	for (size_t i=0;i<getDstPointNum();i++)
	{
		xml.AddElem(strDstNode,getDstIndexBySrcNO(i));
		xml.AddAttrib(strDstIndex,i);
	}
	xml.OutOfElem();
}

int CSpaceRouteTab::LoadXmlCfg( FileSystem::CMarkup & xml,size_t srcSum,bool bCountSrc /*= false*/ )
{
	int dstSum = 0;
	std::string strTmp = xml.GetAttrib(strTabDstNum);
	boost::algorithm::trim(strTmp);
	try
	{
		dstSum = boost::lexical_cast<typeTabIndex>(strTmp);
	}
	catch(boost::bad_lexical_cast& e)
	{
		//do nothing, just ignore strTabDstNum attrib
		std::ostringstream ostr;
		ostr<<e.what();

		dstSum = 0;
	}

	bool bIndexOrderEnable = false;
	strTmp = xml.GetAttrib(strIndexOrderEnable);
	boost::algorithm::trim(strTmp);
	if (boost::algorithm::iequals(strboolTrue,strTmp))
	{
		bIndexOrderEnable = true;
	}

	if (dstSum > (int)srcSum)
	{
		std::ostringstream ostr;
		ostr<<strTabDstNum<<"属性值（"<<dstSum<<"）超过数据库的点号数量值（"<<srcSum<<"）"<<std::endl;
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
	}

	xml.IntoElem();

	if (dstSum > 0)
	{
		if(bIndexOrderEnable)
		{
			RouteTab_DstToSrc_.resize(dstSum);
			for (std::vector<typeTabIndex>::iterator it=RouteTab_DstToSrc_.begin();it != RouteTab_DstToSrc_.end();it++)
			{
				(*it) = -1;
			}

			int count = 0;
			while (xml.FindElem(strDstNode))
			{
				int dstIndex = -1;
				std::string strTmp = xml.GetAttrib(strDstIndex);
				boost::algorithm::trim(strTmp);
				try
				{
					dstIndex = boost::lexical_cast<typeTabIndex>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					xml.OutOfElem();

					std::ostringstream ostr;
					ostr<<strDstIndex<<":"<<strTmp<<" 非法数字:"<<e.what()<<std::endl;

					dat2def_exception err;
					err<<boost::errinfo_type_info_name(ostr.str());
					err<<boost::errinfo_errno(count);
					throw err;
				}

				int srcIndex = getSrcIndexByXml(xml,dstIndex);
				
				if (!checkDstToSrcIndex(dstIndex,srcIndex,srcSum,bCountSrc))
				{
					//addDstToSrcIndex(srcIndex);
					setDstToSrcIndex(dstIndex,srcIndex);
				}
				else
				{
					xml.OutOfElem();

					dat2def_exception err;
					err<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
					err<<boost::errinfo_errno(count);
					throw err;
				}

				count++;
			}
		}
		else
		{
			int count = 0;
			while (xml.FindElem(strDstNode) && count < dstSum)
			{
				typeTabIndex srcIndex = getSrcIndexByXml(xml,count);

				if (!checkDstToSrcIndex(count,srcIndex,srcSum,bCountSrc))
				{
					addDstToSrcIndex(srcIndex);
				}
				else
				{
					xml.OutOfElem();

					std::ostringstream ostr;
					ostr<<strDstNode<<":"<<strTmp<<":"<<IndexOutOfMemoryErr<<std::endl;

					dat2def_exception err;
					err<<boost::errinfo_type_info_name(ostr.str());
					err<<boost::errinfo_errno(count);
					throw err;
				}

				count++;
			}
		}
	}
	else
	{
		int count = 0;
		while (xml.FindElem(strDstNode) && count < (int)srcSum)
		{
			int srcIndex = getSrcIndexByXml(xml,count);

			if (!checkDstToSrcIndex(count,srcIndex,srcSum,bCountSrc))
			{
				addDstToSrcIndex(srcIndex);
			}
			else
			{
				xml.OutOfElem();

				std::ostringstream ostr;
				ostr<<strDstNode<<":"<<strTmp<<":"<<IndexOutOfMemoryErr<<std::endl;

				dat2def_exception err;
				err<<boost::errinfo_type_info_name(ostr.str());
				err<<boost::errinfo_errno(count);
				throw err;
			}

			count++;
		}
	}

	xml.OutOfElem();

	return 0;
}

int CSpaceRouteTab::getSrcIndexByXml(FileSystem::CMarkup & xml,int dstIndex)
{
	std::string strTmp = xml.GetData();

	int srcIndex = -1;
	try
	{
		boost::algorithm::trim(strTmp);
		srcIndex = boost::lexical_cast<typeTabIndex>(strTmp);

		if(!TransIndex.empty())
		{
			std::string strTI = xml.GetAttrib(strTerminalIndex);
			boost::algorithm::trim(strTI);
			if(!strTI.empty())
			{
				try
				{
					int terminalIndex = boost::lexical_cast<int>(strTI);
					int retVal = TransIndex(terminalIndex,srcIndex);
					if (retVal >= 0)
					{
						srcIndex = retVal;
					}
					else
					{
						xml.OutOfElem();

						std::ostringstream ostr;
						ostr<<strTerminalIndex<<":"<<strTI<<":"<<IndexOutOfMemoryErr<<std::endl;

						dat2def_exception err;
						err<<boost::errinfo_type_info_name(ostr.str());
						err<<boost::errinfo_errno(dstIndex);
						throw err;
					}
				}
				catch(boost::bad_lexical_cast& e)
				{
					xml.OutOfElem();

					std::ostringstream ostr;
					ostr<<strTerminalIndex<<":"<<strTI<<" 非法数字:"<<e.what()<<std::endl;

					dat2def_exception err;
					err<<boost::errinfo_type_info_name(ostr.str());
					err<<boost::errinfo_errno(dstIndex);
					throw err;
				}
			}
		}
	}
	catch(boost::bad_lexical_cast& e)
	{
		xml.OutOfElem();

		std::ostringstream ostr;
		ostr<<strDstNode<<":"<<strTmp<<" 非法数字:"<<e.what()<<std::endl;

		dat2def_exception err;
		err<<boost::errinfo_type_info_name(ostr.str());
		err<<boost::errinfo_errno(dstIndex);
		throw err;
	}

	return srcIndex;
}

}; //namespace PublicSupport


