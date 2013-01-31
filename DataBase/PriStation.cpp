#include <iostream>
#include <boost/scoped_array.hpp>
#include <boost/bind.hpp>
#include "PriStation.h"
#include "SubStation.h"
//#include "CommInterface.h"
//#include "TcpServer.h"
#include "../FileSystem/Markup.h"
#include "../Protocol/CmdQueue.h"
#include "../PublicSupport/EfficientRouteTab.h"
#include "YkPoint.h"
#include "YkRecord.h"
#include "CosRecord.h"
#include "SoeRecord.h"
#include "FaultRecord.h"

namespace DataBase {

//xml结点字符定义
#define strPriAddr "Address"
#define strBroadCastAddr "BroadCastAddr"
#define strAcpSynT "AcceptSynTime"
#define strAuthor "Author"
#define strYxTab "YxRouteTab"
#define strYcTab "YcRouteTab"
#define strYmTab "YmRouteTab"
#define strYkTab "YkRouteTab"
#define strUseTabIndex "UseTabIndex"
#define strbYcMul "bYcMul"

const std::string strUnAuthorisedPort = "/dev/ttyBF0";

CPriStation::CPriStation(boost::asio::io_service & io_service,
							 CSubStation & sub)
							:CCommPoint(io_service),
							sub_(sub)
{
	pointType_ = PRISTATION_NODE;

	bAcceptSynSubTime_ = true;
	bAuthorised_ = true;

	bSynTimeImmediately_ = true;
	bCallDataImmediately_ = true;
	bAddGeneralCmd_ = true;
	bAddSpeCmd_ =true;

	cosLoadPtr_ = 0;
	soeLoadPtr_ = 0;
	ycvarLoadPtr_ = 0;

	addr_ = 0xffff;
	BroadCastAddr_ = 0xffff;

	UseYxTabIndex_ = -1;
	UseYcTabIndex_ = -1;
	UseYkTabIndex_ = -1;
	UseYmTabIndex_ = -1;

	bCommActiveBackup_ = bCommActive_;

	bycmul_ = false;
}

CPriStation::~CPriStation(void)
{
}

int CPriStation::InitPriStation( FileSystem::CMarkup & xml )
{
	LoadXmlCfg(xml);

	InitCosLoadPtr();
	InitSoeLoadPtr();
	InitYcvarLoadPtr();

	return 0;
}

void CPriStation::UninitPriStation()
{
	if (yxTab_)
	{
		yxTab_->UninitRouteTab();
	}
	
	if (ycTab_)
	{
		ycTab_->UninitRouteTab();
	}
	
	if (ykTab_)
	{
		ykTab_->UninitRouteTab();
	}
	
	if (ymTab_)
	{
		ymTab_->UninitRouteTab();
	}
}

//index transform
int CPriStation::PriyxToSubyx(size_t val)
{
	if (val < 0 || val >= getYxSum())
	{
		return -1;
	}
	
	if (yxTab_)
	{
		return yxTab_->getSrcIndexByDstNO(val);
	}
	else
	{
		return val;
	}
	
}

int CPriStation::SubyxToPriyx(size_t val)
{
	if (val < 0 || val >= getSubYxSum())
	{
		return -1;
	}

	if (yxTab_)
	{
		return yxTab_->getDstIndexBySrcNO(val);
	}
	else
	{
		return val;
	}
	
}

int CPriStation::PriycToSubyc(size_t val)
{
	if (val < 0 || val >= getYcSum())
	{
		return -1;
	}

	if (ycTab_)
	{
		return ycTab_->getSrcIndexByDstNO(val);
	}
	else
	{
		return val;
	}
	
}

int CPriStation::SubycToPriyc(size_t val)
{
	if (val < 0 || val >= getSubYcSum())
	{
		return -1;
	}

	if (ycTab_)
	{
		return ycTab_->getDstIndexBySrcNO(val);
	}
	else
	{
		return val;
	}
	
}

int CPriStation::PriykToSubyk(size_t val)
{
	if (val < 0 || val >= getYkSum())
	{
		return -1;
	}

	if (ykTab_)
	{
		return ykTab_->getSrcIndexByDstNO(val);
	}
	else
	{
		return val;
	}
	
}

int CPriStation::SubykToPriyk(size_t val)
{
	if (val < 0 || val >= getSubYkSum())
	{
		return -1;
	}

	if (ykTab_)
	{
		return ykTab_->getDstIndexBySrcNO(val);
	}
	else
	{
		return val;
	}
	
}

int CPriStation::PriymToSubym(size_t val)
{
	if (val < 0 || val >= getYmSum())
	{
		return -1;
	}

	if (ymTab_)
	{
		return ymTab_->getSrcIndexByDstNO(val);
	}
	else
	{
		return val;
	}
	
}

int CPriStation::SubymToPriym(size_t val)
{
	if (val < 0 || val >= getSubYmSum())
	{
		return -1;
	}

	if (ymTab_)
	{
		return ymTab_->getDstIndexBySrcNO(val);
	}
	else
	{
		return val;
	}
	
}

//yx api
size_t CPriStation::getYxSum()
{
	if (yxTab_)
	{
		return yxTab_->getDstPointNum();
	}
	else
	{
		return sub_.getYxSum();
	}
	
}

size_t CPriStation::getSubYxSum()
{
	if (yxTab_)
	{
		return yxTab_->getSrcPointNum();
	}
	else
	{
		return sub_.getYxSum();
	}
}

typeYxval CPriStation::getOriYxVal(size_t index)
{
	int subIndex = PriyxToSubyx(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getOriYxVal(subIndex);
}

int CPriStation::setOriYxVal(size_t index, typeYxval val)
{
	int subIndex = PriyxToSubyx(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setOriYxVal(subIndex,val);
}

typeYxtype CPriStation::getYxType(size_t index)
{
	int subIndex = PriyxToSubyx(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYxType(subIndex);
}

int CPriStation::setYxType(size_t index,typeYxtype val)
{
	int subIndex = PriyxToSubyx(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setYxType(subIndex,val);
}

bool CPriStation::getYxPolar(size_t index)
{
	int subIndex = PriyxToSubyx(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYxPolar(subIndex);
}

int CPriStation::setYxPolar(size_t index,bool val)
{
	int subIndex = PriyxToSubyx(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setYxPolar(subIndex,val);
}

typeYxQuality CPriStation::getYxQuality(size_t index)
{
	int subIndex = PriyxToSubyx(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYxQuality(subIndex);
}

int CPriStation::setYxQuality(size_t index,typeYxQuality val)
{
	int subIndex = PriyxToSubyx(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setYxQuality(subIndex,val);
}

typeFinalYcval CPriStation::getFinalYxVal(size_t index)
{
	int subIndex = PriyxToSubyx(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getFinalYxVal(subIndex);
}

//yc api
size_t CPriStation::getYcSum()
{
	if (ycTab_)
	{
		return ycTab_->getDstPointNum();
	}
	else
	{
		return sub_.getYcSum();
	}
	
}

size_t CPriStation::getSubYcSum()
{
	if (ycTab_)
	{
		return ycTab_->getSrcPointNum();
	}
	else
	{
		return sub_.getYcSum();
	}
	
}

typeYcval CPriStation::getOriYcVal(size_t index)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getOriYcVal(subIndex);
}

int CPriStation::setOriYcVal(size_t index,typeYcval val)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setOriYcVal(subIndex,val);
}

typeYcplus CPriStation::getYcPlus(size_t index)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYcPlus(subIndex);
}

int CPriStation::setYcPlus(size_t index,typeYcplus val)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setYcPlus(subIndex,val);
}

typeYcmul CPriStation::getYcMul(size_t index)
{
	if(!bycmul_)
	{
       return 1;
	}
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYcMul(subIndex);
}

int CPriStation::setYcMul(size_t index,typeYcmul val)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setYcMul(subIndex,val);
}

typeYcquality CPriStation::getYcQuality(size_t index)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYcQuality(subIndex);
}

int CPriStation::setYcQuality(size_t index,typeYcquality val)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setYcQuality(subIndex,val);
}

typeYcdead CPriStation::getYcDeadLimit(size_t index)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYcDeadLimit(subIndex);
}

int CPriStation::setYcDeadLimit(size_t index,typeYcdead val)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setYcDeadLimit(subIndex,val);
}

typeFinalYcval CPriStation::getFinalYcVal(size_t index,bool bUseMul/* = false*/)
{
	int subIndex = PriycToSubyc(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getFinalYcVal(subIndex,bycmul_/*bUseMul*/);
}

//yk api
size_t CPriStation::getYkSum()
{
	if (ykTab_)
	{
		return ykTab_->getDstPointNum();
	}
	else
	{
		return sub_.getYkSum();
	}
	
}

size_t CPriStation::getSubYkSum()
{
	if (ykTab_)
	{
		return ykTab_->getSrcPointNum();
	}
	else
	{
		return sub_.getYkSum();
	}	
}

//typeYkstatus CPriStation::getYkStatus(size_t index)
//{
//	int subIndex = PriykToSubyk(index);
//	if (subIndex < 0)
//	{
//		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
//	}
//
//	return sub_.getYkStatus(subIndex);
//}
//
//int CPriStation::setYkStatus(size_t index,typeYkstatus val)
//{
//	int subIndex = PriykToSubyk(index);
//	if (subIndex < 0)
//	{
//		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
//	}
//
//	return sub_.setYkStatus(subIndex,val);
//}

CYkPoint * CPriStation::getYkPointPtr(int index)
{
	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYkPointPtr(subIndex);
}

typeYktype CPriStation::getYkType(size_t index)
{
	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYkType(subIndex);
}

int CPriStation::setYkType(size_t index,typeYkstatus val)
{
	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setYkType(subIndex,val);
}

bool CPriStation::getbHYkDouble(size_t index)
{
	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getbHYkDouble(subIndex);
}

int CPriStation::setbHYkDouble(size_t index,bool val)
{
	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setbHYkDouble(subIndex,val);
}

bool CPriStation::getbSYkDouble(size_t index)
{
	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getbSYkDouble(subIndex);
}

int CPriStation::setbSYkDouble(size_t index,bool val)
{
	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setbSYkDouble(subIndex,val);
}


int CPriStation::AddYkSelCmd(size_t index,bool bCloseOrOpen)
{
	if (!getbAuthor())
	{
		AddYkRecord(yk_sel_fail,index,bCloseOrOpen);
		return -1;
	}

	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		AddYkRecord(yk_sel_fail,index,bCloseOrOpen);
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	AddYkRecord(yk_sel,index,bCloseOrOpen);
	return sub_.AddYkSelCmd(subIndex,bCloseOrOpen);
}

int CPriStation::AddYkExeCmd(size_t index,bool bCloseOrOpen)
{
	if (!getbAuthor())
	{
		AddYkRecord(yk_exe_fail,index,bCloseOrOpen);
		return -1;
	}

	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		AddYkRecord(yk_exe_fail,index,bCloseOrOpen);
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	AddYkRecord(yk_exe,index,bCloseOrOpen);
	return sub_.AddYkExeCmd(subIndex,bCloseOrOpen);
}

int CPriStation::AddYkCancelCmd(size_t index,bool bCloseOrOpen/* = true*/)
{
	if (!getbAuthor())
	{
		AddYkRecord(yk_cancel_fail,index,bCloseOrOpen);
		return -1;
	}

	int subIndex = PriykToSubyk(index);
	if (subIndex < 0)
	{
		AddYkRecord(yk_cancel_fail,index,bCloseOrOpen);
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	AddYkRecord(yk_cancel,index,bCloseOrOpen);
	return sub_.AddYkCancelCmd(subIndex,bCloseOrOpen);
}

int CPriStation::AddBF533Cmd(int tIndex,Protocol::CCmd cmdVal) //用于增加常规指令的传递
{
	if (!getbAuthor())
	{
		return -1;
	}

	return sub_.AddBF533Cmd(tIndex,cmdVal);
}

int CPriStation::getTerminalYcINum(int tIndex) //用于增加常规指令的传递
{
	return sub_.getTerminalYcINum(tIndex);
}

//ym api
size_t CPriStation::getYmSum()
{
	if (ymTab_)
	{
		return ymTab_->getDstPointNum();
	}
	else
	{
		return sub_.getYmSum();
	}
	
}

size_t CPriStation::getSubYmSum()
{
	if (ymTab_)
	{
		return ymTab_->getSrcPointNum();
	}
	else
	{
		return sub_.getYmSum();
	}
}

typeYmval CPriStation::getOriYmVal(size_t index)
{
	int subIndex = PriymToSubym(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getOriYmVal(subIndex);
}

int CPriStation::setOriYmVal(size_t index,typeYmval val)
{
	int subIndex = PriymToSubym(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setOriYmVal(subIndex,val);
}

typeYcquality CPriStation::getYmQuality(size_t index)
{
	int subIndex = PriymToSubym(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.getYmQuality(subIndex);
}

int CPriStation::setYmQuality(size_t index,typeYmquality val)
{
	int subIndex = PriymToSubym(index);
	if (subIndex < 0)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return sub_.setYmQuality(subIndex,val);
}


//cos api
void CPriStation::InitCosLoadPtr()
{
	return sub_.InitCosLoadPtr(cosLoadPtr_);
}

int CPriStation::getCosPointNum()
{
	return sub_.getCosPointNum(cosLoadPtr_);
}

int CPriStation::getFirstCosPoint(CCosPoint & outVal)
{
	int leftPointNum = getCosPointNum();
	for (int i=0;i<leftPointNum;i++)
	{
		outVal = sub_.getCosPoint(cosLoadPtr_ + i);
		int priyxIndex = SubyxToPriyx(outVal.getYxIndex());
		if (priyxIndex >= 0)
		{
			outVal.setYxIndex(priyxIndex);
			return i;
		}
	}
	
	return -1;
}

int CPriStation::loadCosPoints(CCosPoint * cosBuf,size_t count,typeYxtype & yxType)
{
	boost::scoped_array<CCosPoint> tempBuf(new CCosPoint[count + 1]);
	int sum = sub_.copyCosPoints(cosLoadPtr_,tempBuf.get(),count);
	if (sum <= 0)
	{
		cosBuf = NULL;
		return sum;
	}

	int outCount = 0;
	int readCount = 0;

	while(readCount < sum)
	{
		int priyxIndex = SubyxToPriyx(tempBuf[readCount].getYxIndex());
		if (priyxIndex >= 0)
		{
			if (tempBuf[readCount].getYxType() == yxType)
			{
				tempBuf[readCount].setYxIndex(priyxIndex);
				cosBuf[outCount] = tempBuf[readCount];
				AddCosRecord(tempBuf[readCount]);

				outCount++;
			}
			else
			{
				yxType = tempBuf[readCount].getYxType();
				break;
			}
		}

		readCount++;
		
	}

	sub_.moveCosLoadPtr(cosLoadPtr_,readCount);

	return outCount;
}

//soe api
void CPriStation::InitSoeLoadPtr()
{
	return sub_.InitSoeLoadPtr(soeLoadPtr_);
}

int CPriStation::getSoePointNum()
{
	return sub_.getSoePointNum(soeLoadPtr_);
}

int CPriStation::getFirstSoePoint(CSoePoint & outVal)
{
	int leftPointNum = getSoePointNum();
	for (int i=0;i<leftPointNum;i++)
	{
		outVal = sub_.getSoePoint(soeLoadPtr_ + i);
		int priyxIndex = SubyxToPriyx(outVal.getYxIndex());
		if (priyxIndex >= 0)
		{
			outVal.setYxIndex(priyxIndex);
			return i;
		}
	}

	return -1;
}

int CPriStation::loadSoePoints(CSoePoint * soeBuf,size_t count,typeYxtype & yxType)
{
	boost::scoped_array<CSoePoint> tempBuf(new CSoePoint[count + 1]);
	int sum = sub_.copySoePoints(soeLoadPtr_,tempBuf.get(),count);
	if (sum <= 0)
	{
		soeBuf = NULL;
		return 0;
	}

	int outCount = 0;
	int readCount = 0;

	while(readCount < sum)
	{
		int priyxIndex = SubyxToPriyx(tempBuf[readCount].getYxIndex());
		if (priyxIndex >= 0)
		{
			if (tempBuf[readCount].getYxType() == yxType)
			{
				tempBuf[readCount].setYxIndex(priyxIndex);
				soeBuf[outCount] = tempBuf[readCount];
				AddSoeRecord(tempBuf[outCount]);

				outCount++;
			}
			else
			{
				yxType = tempBuf[readCount].getYxType();
				break;
			}
			
		}

		readCount++;

	}

	sub_.moveSoeLoadPtr(soeLoadPtr_,readCount);

	return outCount;
}

//ycvar api
void CPriStation::InitYcvarLoadPtr()
{
	return sub_.InitYcvarLoadPtr(ycvarLoadPtr_);
}

int CPriStation::getYcvarPointNum()
{
	return sub_.getYcvarPointNum(ycvarLoadPtr_);
}

int CPriStation::getFirstYcvarPoint(CYcVarPoint & outVal)
{
	int leftPointNum = getYcvarPointNum();
	for (int i=0;i<leftPointNum;i++)
	{
		outVal = sub_.getYcvarPoint(ycvarLoadPtr_ + i);
		int priycIndex = SubycToPriyc(outVal.getYcIndex());
		if (priycIndex >= 0)
		{
			outVal.setYcIndex(priycIndex);
			return i;
		}
	}

	return -1;
}

int CPriStation::loadYcvarPoints(CYcVarPoint * ycvarBuf,size_t count)
{
	boost::scoped_array<CYcVarPoint> tempBuf(new CYcVarPoint[count + 1]);
	int sum = sub_.loadYcvarPoints(ycvarLoadPtr_,tempBuf.get(),count);
	if (sum <= 0)
	{
		ycvarBuf = NULL;
		return sum;
	}

	size_t outCount = 0;

	for (int i=0;i<sum;i++)
	{
		int priycIndex = SubycToPriyc(tempBuf[i].getYcIndex());
		if (priycIndex >= 0)
		{
			tempBuf[i].setYcIndex(priycIndex);
			ycvarBuf[outCount] = tempBuf[i];
			outCount++;
		}

	}

	return outCount;
}

//************************************
// Method:    getFinalYcVarVal
// FullName:  DataBase::CPriStation::getFinalYcVarVal
// Access:    public 
// Returns:   typeFinalYcval
// Qualifier: 这个接口是给对主站的通讯规约用的，它假设使用输入参数var.getYcIndex()获得是主站视图的点号（经过转发表转换），如果在CSubStation或者其他位置调用这个接口请自行留意点号的转换。
// Parameter: CYcVarPoint & var
//************************************
typeFinalYcval CPriStation::getFinalYcVarVal(CYcVarPoint & var)
{
	if (bycmul_)
	{
		return (var.getYcVal() + getYcPlus(var.getYcIndex())) * getYcMul(var.getYcIndex());
	}
	else
	{
		return var.getYcVal() + getYcPlus(var.getYcIndex());
	}
}

//虚函数接口
share_commpoint_ptr CPriStation::getSelfPtr()
{
	return shared_from_this();
}

/*
void CPriStation::AddSelfPointToProtocol()
{
	if (commPtr_)
	{
		commPtr_->AddCommPoint(getSelfPtr());
	}

	//ConnectCmdRecallSig();
}

void CPriStation::AddSelfPointToTcpServerPtr()
{
	if (serverPtr_)
	{
		serverPtr_->AddCommPoint(getSelfPtr());
	}
}

int CPriStation::ResetTimerRecv(size_t LostRecvTimeOut)
{
	timerRecv_.expires_from_now(boost::posix_time::seconds(LostRecvTimeOut));
	timerRecv_.async_wait(boost::bind(&CCommPoint::handle_timerRecv,getSelfPtr(),boost::asio::placeholders::error));

	return 0;
}

void CPriStation::handle_timerRecv(const boost::system::error_code& error)
{
	if (!error)
	{
		setCommActive(false);
	}
}

int CPriStation::ConnectCmdRecallSig()
{
	if (commPtr_)
	{
		return commPtr_->ConnectCmdRecallSig(boost::bind(&CCommPoint::ProcessRelayCmd,getSelfPtr(),_1,_2,_3,_4));
	}

	return -1;
}
*/

SigConnection CPriStation::ConnectSubTempSig(CmdRecallSlotType slotVal)
{
	return sub_.ConnectSubTempSig(slotVal);
}

SigConnection CPriStation::ConnectSubAliveSig(CmdRecallSlotType slotVal)
{
	return sub_.ConnectSubAliveSig(slotVal);
}

void CPriStation::ResetAuthorisedFlagByLocalPort(std::string strPort)
{

#if defined(_BF518_)
	if (boost::iequals(getLocalPort(),strPort))
	{
		setbAuthor(false);
	}
#endif

}

//xml cfg api
int CPriStation::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	CCommPoint::LoadXmlCfg(xml);

	ResetAuthorisedFlagByLocalPort(strUnAuthorisedPort);

	xml.ResetMainPos();
	if (xml.FindElem(strPriAddr))
	{
		unsigned short iaddr = sub_.getAddr();
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			iaddr = boost::lexical_cast<unsigned short>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
			//do nothing,use sub addr val
		}
		setAddr(iaddr);
	}
	else
	{
		setAddr(sub_.getAddr());
	}

	xml.ResetMainPos();
	if (xml.FindElem(strBroadCastAddr))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			unsigned short iaddr = boost::lexical_cast<unsigned short>(strTmp);
			setbAcceptBroadCast(true);
			setBroadCastAddr(iaddr);
		}
		catch(boost::bad_lexical_cast & e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
			//do nothing,use sub addr val
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strAcpSynT))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if(boost::iequals(strboolTrue,strTmp))
		{
			setbAcceptSynSubTime(true);
		}
		else
		{
			setbAcceptSynSubTime(false);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strAuthor))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if(boost::iequals(strboolFalse,strTmp))
		{
			setbAuthor(false);
		}
		else
		{
			setbAuthor(true);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYxTab))
	{
		try
		{
			std::string strTmp = xml.GetAttrib(strUseTabIndex);
			boost::algorithm::trim(strTmp);
			if (!strTmp.empty())
			{
				try
				{
					UseYxTabIndex_ = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::cerr<<"CPriStation::LoadXmlCfg "<<e.what()<<std::endl;
					UseYxTabIndex_ = -1;
				}

				if (UseYxTabIndex_ >= 0 && UseYxTabIndex_ < (int)sub_.getPristationNum())
				{
					yxTab_.reset();
					yxTab_ = sub_.getPriStationYxTabPtr(UseYxTabIndex_);
				}
			}
			
			if (UseYxTabIndex_ < 0)
			{
				yxTab_.reset(new PublicSupport::CEfficientRouteTab(boost::bind(&CSubStation::TerminalyxToSubyx,&sub_,_1,_2)));
				yxTab_->InintRouteTab(xml,sub_.getYxSum(),true);
			}
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			yxTab_.reset();
			e<<errinfo_middletype_name(strYxTab);
			throw e;
		}
		
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYcTab))
	{
		try
		{
			std::string strTmp = xml.GetAttrib(strUseTabIndex);
			boost::algorithm::trim(strTmp);
			if (!strTmp.empty())
			{
				try
				{
					UseYcTabIndex_ = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::cerr<<"CPriStation::LoadXmlCfg "<<e.what()<<std::endl;
					UseYcTabIndex_ = -1;
				}

				if (UseYcTabIndex_ >= 0 && UseYcTabIndex_ < (int)sub_.getPristationNum())
				{
					ycTab_.reset();
					ycTab_ = sub_.getPriStationYcTabPtr(UseYcTabIndex_);
				}
			}

			if (UseYcTabIndex_ < 0)
			{
				ycTab_.reset(new PublicSupport::CEfficientRouteTab(boost::bind(&CSubStation::TerminalycToSubyc,&sub_,_1,_2)));
				ycTab_->InintRouteTab(xml,sub_.getYcSum(),true);
			}

		}
		catch(PublicSupport::dat2def_exception & e)
		{
			ycTab_.reset();
			e<<errinfo_middletype_name(strYcTab);
			throw e;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYmTab))
	{
		try
		{
			std::string strTmp = xml.GetAttrib(strUseTabIndex);
			boost::algorithm::trim(strTmp);
			if (!strTmp.empty())
			{
				try
				{
					UseYmTabIndex_ = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::cerr<<"CPriStation::LoadXmlCfg "<<e.what()<<std::endl;
					UseYmTabIndex_ = -1;
				}

				if (UseYmTabIndex_ >= 0 && UseYmTabIndex_ < (int)sub_.getPristationNum())
				{
					ymTab_.reset();
					ymTab_ = sub_.getPriStationYmTabPtr(UseYmTabIndex_);
				}
			}

			if (UseYmTabIndex_ < 0)
			{
				ymTab_.reset(new PublicSupport::CEfficientRouteTab(boost::bind(&CSubStation::TerminalymToSubym,&sub_,_1,_2)));
				ymTab_->InintRouteTab(xml,sub_.getYmSum(),true);
			}
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			ymTab_.reset();
			e<<errinfo_middletype_name(strYmTab);
			throw e;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYkTab))
	{
		try
		{
			std::string strTmp = xml.GetAttrib(strUseTabIndex);
			boost::algorithm::trim(strTmp);
			if (!strTmp.empty())
			{
				try
				{
					UseYkTabIndex_ = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					std::cerr<<"CPriStation::LoadXmlCfg "<<e.what()<<std::endl;
					UseYkTabIndex_ = -1;
				}

				if (UseYkTabIndex_ >= 0 && UseYkTabIndex_ < (int)sub_.getPristationNum())
				{
					ykTab_.reset();
					ykTab_ = sub_.getPriStationYkTabPtr(UseYkTabIndex_);
				}
			}

			if (UseYkTabIndex_ < 0)
			{
				ykTab_.reset(new PublicSupport::CEfficientRouteTab(boost::bind(&CSubStation::TerminalykToSubyk,&sub_,_1,_2)));
				ykTab_->InintRouteTab(xml,sub_.getYkSum(),true);
			}

		}
		catch(PublicSupport::dat2def_exception & e)
		{
			ykTab_.reset();
			e<<errinfo_middletype_name(strYkTab);
			throw e;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYkRecord))
	{
		std::string strTmp = xml.GetAttrib(strLimit);
		boost::algorithm::trim(strTmp);
		int limit = -1;

		if (!strTmp.empty())
		{
			try
			{
				limit = boost::lexical_cast<int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::cerr<<"CPriStation::LoadXmlCfg "<<e.what()<<std::endl;
				limit = -1;
			}
		}

		std::string strID = xml.GetData();
		boost::algorithm::trim(strID);

		if (limit > 0)
		{
			YkRecordPtr_.reset(new CYkRecord(strID,limit));
		}
		else
		{
			YkRecordPtr_.reset(new CYkRecord(strID));
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strSoeRecord))
	{
		std::string strTmp = xml.GetAttrib(strLimit);
		boost::algorithm::trim(strTmp);
		int limit = -1;

		if (!strTmp.empty())
		{
			try
			{
				limit = boost::lexical_cast<int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
				limit = -1;
			}
		}

		std::string strID = xml.GetData();
		boost::algorithm::trim(strID);

		if (limit > 0)
		{
			SoeRecordPtr_.reset(new CSoeRecord(strID,limit));
		}
		else
		{
			SoeRecordPtr_.reset(new CSoeRecord(strID));
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strCosRecord))
	{
		std::string strTmp = xml.GetAttrib(strLimit);
		boost::algorithm::trim(strTmp);
		int limit = -1;

		if (!strTmp.empty())
		{
			try
			{
				limit = boost::lexical_cast<int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
				limit = -1;
			}
		}

		std::string strID = xml.GetData();
		boost::algorithm::trim(strID);

		if (limit > 0)
		{
			CosRecordPtr_.reset(new CCosRecord(strID,limit));
		}
		else
		{
			CosRecordPtr_.reset(new CCosRecord(strID));
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strFaultRecord))
	{
		std::string strTmp = xml.GetAttrib(strLimit);
		boost::algorithm::trim(strTmp);
		int limit = -1;

		if (!strTmp.empty())
		{
			try
			{
				limit = boost::lexical_cast<int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
				limit = -1;
			}
		}

		std::string strID = xml.GetData();
		boost::algorithm::trim(strID);

		if (limit > 0)
		{
			FaultRecordPtr_.reset(new CFaultRecord(strID,limit));
		}
		else
		{
			FaultRecordPtr_.reset(new CFaultRecord(strID));
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strbYcMul))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			bycmul_ = true;
		}
		else
		{
			bycmul_ = false;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strSynImm))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			bSynTimeImmediately_ = true;
		}
		else
		{
			bSynTimeImmediately_ = false;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strCallImm))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			bCallDataImmediately_ = true;
		}
		else
		{
			bCallDataImmediately_ = false;
		}
	}
	xml.ResetMainPos();
	if (xml.FindElem(strGeneralCmd))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			bAddGeneralCmd_ = true;
		}
		else
		{
			bAddGeneralCmd_ = false;
		}
	}
	xml.ResetMainPos();
	if (xml.FindElem(strSpeCmd))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			bAddSpeCmd_ = true;
		}
		else
		{
			bAddSpeCmd_ = false;
		}
	}

	return 0;
}

void CPriStation::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	CCommPoint::SaveXmlCfg(xml);

	if (getAddr() != sub_.getAddr())
	{
		xml.AddElem(strPriAddr,getAddr());
	}

	if (getbAcceptBroadCast())
	{
		xml.AddElem(strBroadCastAddr,getBroadCastAddr());
	}

	if (!getbAcceptSynSubTime())
	{
		xml.AddElem(strAcpSynT,strboolFalse);
	}

	if (!getbAuthor())
	{
		xml.AddElem(strAuthor,strboolFalse);
	}

	if (yxTab_)
	{
		xml.AddElem(strYxTab);
		if (UseYxTabIndex_ >= 0 && UseYxTabIndex_ < (int)sub_.getPristationNum())
		{
			xml.AddAttrib(strUseTabIndex,UseYxTabIndex_);
		}
		else
		{
			yxTab_->SaveXmlCfg(xml);
		}
	}

	if (ycTab_)
	{
		xml.AddElem(strYcTab);
		if (UseYcTabIndex_ >= 0 && UseYcTabIndex_ < (int)sub_.getPristationNum())
		{
			xml.AddAttrib(strUseTabIndex,UseYcTabIndex_);
		}
		else
		{
			ycTab_->SaveXmlCfg(xml);
		}
	}

	if (ymTab_)
	{
		xml.AddElem(strYmTab);
		if (UseYmTabIndex_ >= 0 && UseYmTabIndex_ < (int)sub_.getPristationNum())
		{
			xml.AddAttrib(strUseTabIndex,UseYmTabIndex_);
		}
		else
		{
			ymTab_->SaveXmlCfg(xml);
		}
	}

	if (ykTab_)
	{
		xml.AddElem(strYkTab);
		if (UseYkTabIndex_ >= 0 && UseYkTabIndex_ < (int)sub_.getPristationNum())
		{
			xml.AddAttrib(strUseTabIndex,UseYkTabIndex_);
		}
		else
		{
			ykTab_->SaveXmlCfg(xml);
		}
	}

	if (YkRecordPtr_)
	{
		xml.AddElem(strYkRecord,YkRecordPtr_->getID());
		xml.AddAttrib(strLimit,YkRecordPtr_->getLimit());
	}

	if (SoeRecordPtr_)
	{
		xml.AddElem(strSoeRecord,SoeRecordPtr_->getID());
		xml.AddAttrib(strLimit,SoeRecordPtr_->getLimit());
	}

	if (CosRecordPtr_)
	{
		xml.AddElem(strCosRecord,CosRecordPtr_->getID());
		xml.AddAttrib(strLimit,CosRecordPtr_->getLimit());
	}

	if (FaultRecordPtr_)
	{
		xml.AddElem(strFaultRecord,FaultRecordPtr_->getID());
		xml.AddChildAttrib(strLimit,FaultRecordPtr_->getID());
	}

	if (bycmul_)
	{
		xml.AddElem(strbYcMul,strboolTrue);
	}

	if (bSynTimeImmediately_)
	{
		xml.AddElem(strSynImm,strboolTrue);
	}

	if (bCallDataImmediately_)
	{
		xml.AddElem(strCallImm,strboolTrue);
	}
}

//active data api
/*
void CPriStation::checkActiveCos()
{
	if (getDataActiveUp())
	{
		if (getCosPointNum(cosLoadPtr_) > 0)
		{
			using namespace Protocol;
			//timerCos_->expires_from_now(boost::posix_time::milliseconds(WaitActiveDataTimeOut));
			//timerCos_->async_wait(boost::bind(&CPriStation::handle_timerCos,getSelfPtr(),boost::asio::placeholders::error));
			CCmd cmdVal(COS_DATA_UP,COS_DATA_UP_PRIORITY,getSelfPtr());
			AddCmdVal(cmdVal);
		}
	}
}

void CPriStation::checkActiveSoe()
{
	if (getDataActiveUp())
	{
		if(getSoePointNum(soeLoadPtr_) > 0)
		{
			using namespace Protocol;
			//timerSoe_->expires_from_now(boost::posix_time::milliseconds(WaitActiveDataTimeOut));
			//timerSoe_->async_wait(boost::bind(&CPriStation::handle_timerSoe,getSelfPtr(),boost::asio::placeholders::error));
			CCmd cmdVal(SOE_DATA_UP,SOE_DATA_UP_PRIORITY,getSelfPtr());
			AddCmdVal(cmdVal);
		}
	}
}

void CPriStation::checkActiveYcvar()
{
	if (getDataActiveUp())
	{
		if (getYcvarPointNum(ycvarLoadPtr_) > 0)
		{
			using namespace Protocol;
			//timerYcvar_->expires_from_now(boost::posix_time::milliseconds(WaitActiveDataTimeOut));
			//timerYcvar_->async_wait(boost::bind(&CPriStation::handle_timerYcvar,getSelfPtr(),boost::asio::placeholders::error));
			CCmd cmdVal(YCVAR_DATA_UP,YCVAR_DATA_UP_PRIORITY,getSelfPtr());
			AddCmdVal(cmdVal);
		}
	}
}

void CPriStation::handle_timerCos(const boost::system::error_code& error)
{
	using namespace Protocol;
	
	if (!error)
	{
		CCmd cmdVal(COS_DATA_UP,COS_DATA_UP_PRIORITY,getSelfPtr());
		AddCmdVal(cmdVal);
	}
}

void CPriStation::handle_timerSoe(const boost::system::error_code& error)
{
	using namespace Protocol;

	if (!error)
	{
		CCmd cmdVal(SOE_DATA_UP,SOE_DATA_UP_PRIORITY,getSelfPtr());
		AddCmdVal(cmdVal);
	}
}

void CPriStation::handle_timerYcvar(const boost::system::error_code& error)
{
	using namespace Protocol;

	if (!error)
	{
		CCmd cmdVal(YCVAR_DATA_UP,YCVAR_DATA_UP_PRIORITY,getSelfPtr());
		AddCmdVal(cmdVal);
	}
}
*/

bool CPriStation::getbAcceptSynSubTime()
{
	return bAcceptSynSubTime_;
}

int CPriStation::setbAcceptSynSubTime(bool val)
{
	bAcceptSynSubTime_ = val;

	return 0;
}

bool CPriStation::getbAuthor()
{
	return bAuthorised_;
}

int CPriStation::setbAuthor(bool val)
{
	bAuthorised_ = val;

	return 0;
}

bool CPriStation::getLastFcbFlag()
{
	return bLastFcbFlag_;
}

int CPriStation::setLastFcbFlag(bool val)
{
	bLastFcbFlag_ = val;

	return 0;
}

bool CPriStation::getSendFcbFlag()
{
	bSendFcbFlag_ = !bSendFcbFlag_;

	return bSendFcbFlag_;
}

int CPriStation::setSendFcbFlag(bool val)
{
	bSendFcbFlag_ = val;

	return 0;
}

int CPriStation::WriteSysTime(unsigned short year,unsigned char month,unsigned char day,unsigned char hour,unsigned char minnutes,unsigned char seconds,unsigned short milliseconds,bool bSynTerminal/* = true*/)
{
	int ret = 0;

	if (getbAcceptSynSubTime())
	{
		ret = sub_.WriteSysTime(year,month,day,hour,minnutes,seconds,milliseconds,bSynTerminal);
	}

	return ret;
	
}

boost::shared_ptr<PublicSupport::CEfficientRouteTab> CPriStation::getYxTabPtr()
{
	return yxTab_;
}

boost::shared_ptr<PublicSupport::CEfficientRouteTab> CPriStation::getYcTabPtr()
{
	return ycTab_;
}

boost::shared_ptr<PublicSupport::CEfficientRouteTab> CPriStation::getYkTabPtr()
{
	return ykTab_;
}

boost::shared_ptr<PublicSupport::CEfficientRouteTab> CPriStation::getYmTabPtr()
{
	return ymTab_;
}

int CPriStation::AddYkRecord(unsigned char recordType,int yk_no,bool bCloseOrOpen)
{
	switch (recordType)
	{
	case yk_sel:
		NotifyEventStatus(YK_EVENT,RETURN_CODE_CMDRECV);
		break;

	case yk_exe:
		NotifyEventStatus(YK_EVENT,RETURN_CODE_ACTIVE);
		break;

	case yk_cancel:
		NotifyEventStatus(YK_EVENT,RETURN_CODE_NEGATIVE);
		break;

	default:
		break;
	}

	if (YkRecordPtr_)
	{
		return YkRecordPtr_->AddDataRecord(recordType,yk_no,bCloseOrOpen);
	}

	return -1;
}

int CPriStation::AddCosRecord(const CCosPoint & cos)
{
	NotifyEventStatus(YX_EVENT,RETURN_CODE_ACTIVE);

	if (CosRecordPtr_)
	{
		return CosRecordPtr_->AddDataRecord(cos);
	}

	return -1;
}

int CPriStation::AddSoeRecord(const CSoePoint & soe)
{
	NotifyEventStatus(YX_EVENT,RETURN_CODE_CMDSEND);

	if (SoeRecordPtr_)
	{
		return SoeRecordPtr_->AddDataRecord(soe);
	}

	return -1;
}

int CPriStation::AddFaultRecord(const CFaultPoint & fault)
{
	if (FaultRecordPtr_)
	{
		return FaultRecordPtr_->AddDataRecord(fault);
	}

	return -1;
}

//comm active api
bool CPriStation::getCommActive()
{
	return CCommPoint::getCommActive();
}

void CPriStation::setCommActive(bool val)
{
	CCommPoint::setCommActive(val);

	setCommActiveBackup(getCommActive());
}

bool CPriStation::getCommActieBackup()
{
	return bCommActiveBackup_;
}

void CPriStation::setCommActiveBackup(bool val)
{
	if (getCommActieBackup() == val)
	{
		return;
	}

	if ((!getCommActieBackup()) && val)
	{
		CCosPoint firstCos;
		int ret = getFirstCosPoint(firstCos);
		if (ret >= 0)
		{
			AddCmdVal(Protocol::COS_DATA_UP,Protocol::COS_DATA_UP_PRIORITY,getSelfPtr());
		}

		CSoePoint firstSoe;
		ret = getFirstSoePoint(firstSoe);
		if (ret >= 0)
		{
			AddCmdVal(Protocol::SOE_DATA_UP,Protocol::SOE_DATA_UP_PRIORITY,getSelfPtr());
		}

		NotifyEventStatus(COMM_EVENT,RETURN_CODE_ACTIVE);
	}
	else
	{
		NotifyEventStatus(COMM_EVENT,RETURN_CODE_NEGATIVE);
	}

	bCommActiveBackup_ = val;
}

//line api
size_t CPriStation::getLineSum()
{
	return sub_.getLineSum();
}

CLine * CPriStation::getLinePtr(int index)
{
	return sub_.getLinePtr(index);
}

//frame api
int CPriStation::CallAllData()
{
	if (bCallDataImmediately_)
	{
		return sub_.CallAllData();
	}

	return -1;
}

int CPriStation::SynAllTime()
{
	if (bSynTimeImmediately_)
	{
		return sub_.SynAllTime();
	}
	
	return -1;
}
int CPriStation::AddGeneralCmd(int tIndex,Protocol::CCmd cmdVal) //用于增加常规指令的传递
{
	if (!bAddGeneralCmd_)
	{
		return -1;
	}

	return sub_.AddGeneralCmd(tIndex,cmdVal);
}

int CPriStation::AddSpeCmd(int tIndex,Protocol::CCmd cmdVal) //用于增加常规指令的传递
{
	if (!bAddSpeCmd_)
	{
		return -1;
	}

	return sub_.AddSpeCmd(tIndex,cmdVal);
}

};//namespace DataBase

