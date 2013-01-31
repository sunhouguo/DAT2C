#pragma once
#include "SpaceRouteTab.h"
namespace FileSystem
{
	class CMarkup;
}

namespace PublicSupport {

class CEfficientRouteTab :
	public CSpaceRouteTab
{
public:
	CEfficientRouteTab(TransTerminalIndexFunCType val);
	virtual ~CEfficientRouteTab(void);

	size_t getSrcPointNum();
	
	int InintRouteTab(FileSystem::CMarkup & xml,size_t DatabaseSum,bool bCountSrc = false);
	void UninitRouteTab();

	int getDstIndexBySrcNO(size_t srcNO);
	int setSrcToDstIndex(size_t index, size_t val);

protected:
	std::vector<typeTabIndex> RouteTab_SrcToDst_;
};

}; //namespace PublicSupport 


