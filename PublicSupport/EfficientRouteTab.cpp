#include "EfficientRouteTab.h"
#include "../FileSystem/Markup.h"

namespace PublicSupport {

CEfficientRouteTab::CEfficientRouteTab(TransTerminalIndexFunCType val)
	:CSpaceRouteTab(val)
{
	//bEnableTab_ = false;
}

CEfficientRouteTab::~CEfficientRouteTab(void)
{
	UninitRouteTab();
}

size_t CEfficientRouteTab::getSrcPointNum()
{
	return RouteTab_SrcToDst_.size();
}

int CEfficientRouteTab::InintRouteTab( FileSystem::CMarkup & xml,size_t DatabaseSum,bool bCountSrc /*= false*/ )
{
	CSpaceRouteTab::InintRouteTab(xml,DatabaseSum,bCountSrc);

	if (bCountSrc)
	{
		if (DatabaseSum >= getDstPointNum())
		{
			RouteTab_SrcToDst_.resize(DatabaseSum);
			for(size_t i=0;i<getSrcPointNum();i++)
			{
				RouteTab_SrcToDst_[i] = -1;
			}

			for (size_t i=0;i<getDstPointNum();i++)
			{
				int srcIndex = getSrcIndexByDstNO(i);
				setSrcToDstIndex(srcIndex,i);
			}
		}
	}

	return 0;
}

void CEfficientRouteTab::UninitRouteTab()
{
	CSpaceRouteTab::UninitRouteTab();

	RouteTab_SrcToDst_.clear();
}

int CEfficientRouteTab::getDstIndexBySrcNO(size_t srcNO)
{
	if (srcNO < 0 || srcNO >= getSrcPointNum())
	{
		return -1;
	}

	return RouteTab_SrcToDst_[srcNO];
}

int CEfficientRouteTab::setSrcToDstIndex(size_t index, size_t val)
{
	if (index < 0 || index >= getSrcPointNum())
	{
		return -1;
	}

	if (val < 0 || val >= getDstPointNum())
	{
		return -1;
	}

	RouteTab_SrcToDst_[index] = val;

	return 0;
}

}; //namespace PublicSupport 


