#pragma once
#include <vector>
#include <string>
#include <boost/function.hpp>

namespace FileSystem
{
	class CMarkup;
}

namespace PublicSupport {

typedef boost::function<int(size_t,size_t)> TransTerminalIndexFunCType;
typedef int typeTabIndex;

class CSpaceRouteTab
{
public:
	CSpaceRouteTab();
	CSpaceRouteTab(TransTerminalIndexFunCType val);
	virtual ~CSpaceRouteTab(void);

	size_t getDstPointNum();
	size_t getSrcPointNum();

	int InintRouteTab(FileSystem::CMarkup & xml,size_t DatabaseSum,bool bCountSrc = false);
	void UninitRouteTab();

	int getDstIndexBySrcNO(size_t srcNO);
	int getSrcIndexByDstNO(size_t dstNO);
	
	//bool getbEnableTab();
	//int setbEnableTab(bool val);

	//xml api
	void SaveXmlCfg(FileSystem::CMarkup & xml);
	int LoadXmlCfg(FileSystem::CMarkup & xml,size_t DatabaseSum,bool bCountSrc = false);

private:
	int getSrcIndexByXml(FileSystem::CMarkup & xml,int dstIndex);
	int setDstToSrcIndex(size_t index, size_t val);
	void addDstToSrcIndex(size_t val);

	int InitMaxSrcNum();
	void ClearMaxSrcNum();

	int checkDstToSrcIndex(int index,int val,size_t DatabaseSum,bool bCountSrc = false);

	 TransTerminalIndexFunCType TransIndex;

protected:
	//bool bEnableTab_;
	std::vector<typeTabIndex> RouteTab_DstToSrc_;

private:
	size_t MaxSrcNum_;
};

}; //namespace PublicSupport
