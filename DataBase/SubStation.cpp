#if !defined(_WIN32)
#include <sys/time.h>
#endif

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <iostream>
#include "SubStation.h"
#include "Terminal.h"
#include "BF533Terminal.h"
#include "TerminalFactroy.h"
#include "PriStation.h"
#include "CalcYxPoint.h"
#include "YCPoint.h"
#include "YkPoint.h"
#include "YmPoint.h"
#include "../FileSystem/Log.h"
#include "../FileSystem/LogFactory.h"
#include "../LocalDrive/WatchDog.h"
#include "../LocalDrive/WatchDogFactory.h"
#include "../LocalDrive/DIO.h"
#include "../LocalDrive/DioFactory.h"
#include "../LocalDrive/Bluetooth_BF518.h"
#include "../LocalDrive/BatteryActive.h"
#include "../LocalDrive/LightDriver_TPE3000.h"
#include "../LocalDrive/IEEE1588_TPE3000.h"
#include "../PublicSupport/EfficientRouteTab.h"
#include "../PublicSupport/SpaceRouteTab.h"
#include "../CommInterface/CommFactory.h"
#include "../CentralizedFA/Algorithm.h"
#include "../Protocol/Protocol.h"
#include "DAOperateSub.h"
#include "YkRecord.h"
#include "CosRecord.h"
#include "SoeRecord.h"

namespace DataBase{

//xml结点字符定义
const std::string strSubStation = "SubStationNode";
const std::string strSubAddr = "Address";
const std::string strYxDB = "YxDB";
const std::string strCalcYxDB = "CalcYxDB";
const std::string strYcDB = "YcDB";
const std::string strYmDB = "YmDB";
const std::string strYkDB = "YkDB";
const std::string strPriStations = "PriStationSet";
const std::string strPriStationNum = "PriStationNum";
const std::string strPriStationNode = "PriStationNode";
const std::string strPriStationIndex = "PriStationNO.";
const std::string strTerminals = "TerminalSet";
const std::string strTerminalNum = "TerminalNum";
const std::string strTerminalNode = "TerminalNode";
const std::string strTerminalIndex = "TerminalNO.";
const std::string strTerminalType = "TerminalType";
const std::string strLoadSubCfgLogName = "LoadSubCfgLogName";
const std::string strInitCommLogName = "InitCommLogName";

const std::string strEnableWatchdog = "EnableWatchdog";
const std::string strWatchdogLog = "WatchdogLog";
const std::string strEnableDIO = "EnableDIO";
const std::string strEnableDA = "EnableDA";
const std::string strEnableBattery = "EnableBattery";
const std::string strEnableLight = "EnableLight";
const std::string strEnableIEEE1588 = "EnableIEEE1588";

const std::string strSoeRecord = "SoeRecord";
const std::string strCosRecord = "CosRecord";
const std::string strYkRecord =  "YkRecord";

const std::string strYxEventNO = "YxEventNO";
const std::string strYkEventNO = "YkEventNO";
const std::string strSelfEventNO = "SelfEventNO";

const std::string DefaultSoeRecordPath = "./Record/SoeRecord.xml";
const std::string DefaultCosRecordPath = "./Record/CosRecord.xml";
const std::string DefaultYkRecordPath = "./Record/YkRecord.xml";

CSubStation::CSubStation(void)
	:LocalYkConTimer_(io_service_),
	ResetDevTimer_(io_service_),
	WriteTime2Dev_(io_service_)
{
	addr_ = DefaulAddr;
	bWatchdogLog_ = false;

	bSynTimeImmediately_ = false;
	bCallDataImmediately_ = false;
	bAddGeneralCmd_ = true;
	bAddSpeCmd_ = true;

	YxReisterNO_ = -1;
	YkRegisterNO_ = -1;
	SelfRegisterNO_ = -1;

	timeOutWriteCmosClock_ = 0;

	work_.reset(new boost::asio::io_service::work(io_service_));

}

CSubStation::~CSubStation(void)
{
}

int CSubStation::InitSubStation()
{
	InitLocalServices();

	InitCommPoints();

	return 0;
}

void CSubStation::UninitSubStation()
{
	UninitCommPoints();

	UninitLocalServices();
}

bool CSubStation::getbWatchdogLog()
{
	return bWatchdogLog_;
}

int CSubStation::setbWatchdogLog(bool val)
{
	bWatchdogLog_ = val;

	return 0;
}

std::string CSubStation::getWatchdogStr()
{
	return strWatchdog_;
}

int CSubStation::setWatchdogStr( std::string val )
{
	strWatchdog_ = val;

	return 0;
}

std::string CSubStation::getDioStr()
{
	return strDIO_;
}

int CSubStation::setDioStr( std::string val )
{
	strDIO_ = val;

	return 0;
}

typeAddr CSubStation::getAddr()
{
	return addr_;
}

int CSubStation::setAddr( typeAddr val )
{
	addr_ = val;

	return 0;
}

//cos api
void CSubStation::InitCosLoadPtr(size_t &loadPtr)
{
	return cosDB_.InitLoadPtr(loadPtr);
}

int CSubStation::getCosPointNum(size_t loadPtr)
{
	return cosDB_.getPointDataNum(loadPtr);
}

int CSubStation::putCosPoint(CCosPoint val)
{
	if (val.getYxIndex() < 0 || val.getYxIndex() >= getYxSum())
	{
		return -1;
	}

	AddCosRecord(val);
	cosDB_.putPointData(val);
	setOriYxVal(val.getYxIndex(),val.getYxVal());

	CheckEffectCalcCos(val.getYxIndex());

	return 0;
}

int CSubStation::putCosPoint(size_t yxIndex,typeYxval yxVal,typeYxtype yxType,typeYxQuality yxQuality /* = 0 */)
{
	CCosPoint cosVal(yxIndex,yxVal,yxType,yxQuality);

	return putCosPoint(cosVal);
}

int CSubStation::putCosPoint(size_t terminalIndex, size_t terminalyxIndex, typeYxval yxVal,typeYxtype yxType,typeYxQuality yxQuality /* = 0 */)
{
	int sub_yxIndex = TerminalyxToSubyx(terminalIndex,terminalyxIndex);

	if (sub_yxIndex < 0)
	{
		return sub_yxIndex;
	}

	return putCosPoint(sub_yxIndex,yxVal,yxType,yxQuality);
}

CCosPoint CSubStation::getCosPoint(size_t loadPtr)
{
	return cosDB_.getPointData(loadPtr);
}

int CSubStation::saveCosPoints( const CCosPoint * cosBuf,size_t count )
{
	return cosDB_.savePointDatas(cosBuf,count);
}

int CSubStation::loadCosPoints(size_t & loadPtr,CCosPoint * cosBuf,size_t count)
{
	return cosDB_.loadPointDatas(loadPtr,cosBuf,count);
}

int CSubStation::copyCosPoints(size_t loadPtr,CCosPoint * cosBuf,size_t count)
{
	return cosDB_.copyPointDatas(loadPtr,cosBuf,count);
}

int CSubStation::moveCosLoadPtr(size_t & loadPtr,size_t count)
{
	return cosDB_.loadPtrPlus(loadPtr,count);
}

int CSubStation::CheckEffectCalcCos(size_t index)
{
	for (int i=0;i<getEffectCalcYxSum(index);i++)
	{
		int effectIndex = getEffectCalcYxIndex(index,i);
		if (effectIndex >= (int)(getTerminalYxSum() + getSubYxSum()) && effectIndex < (int)getYxSum())
		{
			typeYxval oldVal = (calc_yxDB_.getPointDataPtr(effectIndex - getTerminalYxSum() - getSubYxSum()))->getLastYxVal();
			typeYxval newVal = (calc_yxDB_.getPointDataPtr(effectIndex - getTerminalYxSum() - getSubYxSum()))->getOriYxVal();
			if (newVal != oldVal)
			{
				putCosPoint(effectIndex,newVal,getYxType(effectIndex));
			}
		}
	}

	return 0;
}

//soe api
void CSubStation::InitSoeLoadPtr(size_t &loadPtr)
{
	return soeDB_.InitLoadPtr(loadPtr);
}

int CSubStation::getSoePointNum(size_t loadPtr)
{
	return soeDB_.getPointDataNum(loadPtr);
}

int CSubStation::putSoePoint(CSoePoint val)
{
	if (val.getYxIndex() < 0 || val.getYxIndex() >= getYxSum())
	{
		return -1;
	}

	AddSoeRecord(val);
	soeDB_.putPointData(val);
	setOriYxVal(val.getYxIndex(),val.getYxVal());

	CheckEffectCalcSoe(val.getYxIndex(),val.getYxTime());

	return 0;
}

int CSubStation::putSoePoint(size_t yxIndex,typeYxval yxVal,typeYxtype yxType,ptime time,typeYxQuality yxQuality /* = 0 */)
{
	CSoePoint soeVal(yxIndex,yxVal,yxType,yxQuality,time);

	return putSoePoint(soeVal);
}

int CSubStation::putSoePoint(size_t terminalIndex,size_t terminalyxIndex,typeYxval yxVal,typeYxtype yxType,ptime time,typeYxQuality yxQuality /* = 0 */)
{
	int sub_yxIndex = TerminalyxToSubyx(terminalIndex,terminalyxIndex);

	if (sub_yxIndex < 0)
	{
		return sub_yxIndex;
	}

	return putSoePoint(sub_yxIndex,yxVal,yxType,time,yxQuality);
}

CSoePoint CSubStation::getSoePoint(size_t loadPtr)
{
	return soeDB_.getPointData(loadPtr);
}

int CSubStation::saveSoePoints( const CSoePoint * soeBuf,size_t count )
{
	return soeDB_.savePointDatas(soeBuf,count);
}

int CSubStation::loadSoePoints(size_t & loadPtr,CSoePoint * soeBuf,size_t count)
{
	return soeDB_.loadPointDatas(loadPtr,soeBuf,count);
}

int CSubStation::copySoePoints(size_t loadPtr,CSoePoint * soeBuf,size_t count)
{
	return soeDB_.copyPointDatas(loadPtr,soeBuf,count);
}

int CSubStation::moveSoeLoadPtr(size_t & loadPtr, size_t count)
{
	return soeDB_.loadPtrPlus(loadPtr,count);
}

int CSubStation::CheckEffectCalcSoe(size_t index,ptime time)
{
	for (int i=0;i<getEffectCalcYxSum(index);i++)
	{
		int effectIndex = getEffectCalcYxIndex(index,i);
		if (effectIndex >= (int)(getTerminalYxSum() + getSubYxSum()) && effectIndex < (int)getYxSum())
		{
			typeYxval oldVal = (calc_yxDB_.getPointDataPtr(effectIndex - getTerminalYxSum() - getSubYxSum()))->getLastYxVal();
			typeYxval newVal = (calc_yxDB_.getPointDataPtr(effectIndex - getTerminalYxSum() - getSubYxSum()))->getOriYxVal();
			if (newVal != oldVal)
			{
				putSoePoint(effectIndex,newVal,getYxType(effectIndex),time);
			}
		}
	}

	return 0;
}

//ycvar api
void CSubStation::InitYcvarLoadPtr(size_t & loadPtr)
{
	return ycvarDB_.InitLoadPtr(loadPtr);
}

int CSubStation::getYcvarPointNum(size_t loadPtr)
{
	return ycvarDB_.getPointDataNum(loadPtr);
}

int CSubStation::putYcvarPoint(CYcVarPoint val)
{
	if (val.getYcIndex() < 0 || val.getYcIndex() >= getYcSum()) 
	{
		return -1;
	}

	ycvarDB_.putPointData(val);

	return 0;
}

int CSubStation::putYcvarPoint(size_t ycIndex,typeYcval ycVal,typeYcquality ycQuality /* = 0 */,ptime time /* = ptime() */)
{
	CYcVarPoint ycvarVal(ycIndex,ycVal,ycQuality,time);

	return putYcvarPoint(ycvarVal);
}

int CSubStation::putYcvarPoint(size_t terminalIndex,size_t terminalycIndex,typeYcval ycVal,typeYcquality ycQuality /* = 0 */,ptime time /* = ptime() */)
{
	int sub_ycIndex = TerminalycToSubyc(terminalIndex,terminalycIndex);

	if (sub_ycIndex < 0)
	{
		return sub_ycIndex;
	}

	return putYcvarPoint(sub_ycIndex,ycVal,ycQuality,time);
}

CYcVarPoint CSubStation::getYcvarPoint(size_t loadPtr)
{
	return ycvarDB_.getPointData(loadPtr);
}

int CSubStation::saveYcvarPoints( const CYcVarPoint * ycvarBuf,size_t count )
{
	return ycvarDB_.savePointDatas(ycvarBuf,count);
}

int CSubStation::loadYcvarPoints(size_t & loadPtr,CYcVarPoint * ycvarBuf,size_t count)
{
	return ycvarDB_.loadPointDatas(loadPtr,ycvarBuf,count);
}

//terminal api
size_t CSubStation::getTerminalNum()
{
	return terminals_.size();
}

int CSubStation::getTerminalIndexByPtr(share_terminal_ptr val)
{
	for (size_t i=0;i<getTerminalNum();i++)
	{
		if (terminals_[i] == val)
		{
			return i;
		}
	}

	return -1;
}

int CSubStation::setBf533Terminal(share_bf533_ptr val)
{
	bf533_.reset();
	bf533_ = val;

	return 0;
}

share_bf533_ptr CSubStation::getBf533Terminal()
{
	return bf533_;
}

//pristation api
size_t CSubStation::getPristationNum()
{
	return pristations_.size();
}

int CSubStation::getPristationIndexByPtr(share_pristation_ptr val)
{
	for (size_t i=0;i<getPristationNum();i++)
	{
		if (pristations_[i] == val)
		{
			return i;
		}
	}

	return -1;
}

//index transform
int CSubStation::TerminalyxToSubyx(size_t terminalIndex,size_t yxIndex)
{
	if (terminalIndex < 0 || terminalIndex >= getTerminalNum())
	{
		return -1;
	}

	if (yxIndex < 0 || yxIndex >= terminals_[terminalIndex]->getYxSum())
	{
		return -1;
	}

	size_t sum = 0;
	for (size_t i=0; i<terminalIndex;i++)
	{
		sum += terminals_[i]->getYxSum();
	}
	sum += yxIndex;

	return sum;
}

int CSubStation::SubyxToTerminalyx(size_t yxIndex,size_t & terminalIndex,size_t & terminalyxIndex)
{
	if (yxIndex < 0 || yxIndex >= getTerminalYxSum())
	{
		return -1;
	}
    
	size_t count = 0;
	for (size_t i=0;i<getTerminalNum();i++)
	{
		if (count <= yxIndex && yxIndex < count + terminals_[i]->getYxSum())
		{
			terminalIndex = i;
			terminalyxIndex = yxIndex - count;
			return 0;
		}

		count += terminals_[i]->getYxSum();
	}

	return -1;
}

int CSubStation::TerminalycToSubyc(size_t terminalIndex,size_t ycIndex)
{
	if (terminalIndex < 0 || terminalIndex >= getTerminalNum())
	{
		return -1;
	}

	if (ycIndex < 0 || ycIndex >= terminals_[terminalIndex]->getYcSum())
	{
		return -1;
	}

	size_t sum = 0;
	for (size_t i=0; i<terminalIndex;i++)
	{
		sum += terminals_[i]->getYcSum();
	}
	sum += ycIndex;

	return sum;
}

int CSubStation::SubycToTerminalyc(size_t ycIndex,size_t & terminalIndex,size_t & terminalycIndex)
{
	if (ycIndex < 0 || ycIndex >= getTerminalYcSum())
	{
		return -1;
	}

	size_t count = 0;
	for (size_t i=0;i<getTerminalNum();i++)
	{
		if (count <= ycIndex && ycIndex < count + terminals_[i]->getYcSum())
		{
			terminalIndex = i;
			terminalycIndex = ycIndex - count;
			return 0;
		}

		count += terminals_[i]->getYcSum();
	}

	return -1;
}

int CSubStation::TerminalykToSubyk(size_t terminalIndex,size_t ykIndex)
{
	if (terminalIndex < 0 || terminalIndex >= getTerminalNum())
	{
		return -1;
	}

	if (ykIndex < 0 || ykIndex >= terminals_[terminalIndex]->getYkSum())
	{
		return -1;
	}

	size_t sum = 0;
	for (size_t i=0; i<terminalIndex;i++)
	{
		sum += terminals_[i]->getYkSum();
	}
	sum += ykIndex;

	return sum;
}

int CSubStation::SubykToTerminalyk(size_t ykIndex,size_t & terminalIndex,size_t & terminalykIndex)
{
	if (ykIndex < 0 || ykIndex >= getTerminalYkSum())
	{
		return -1;
	}

	size_t count = 0;
	for (size_t i=0;i<getTerminalNum();i++)
	{
		if (count <= ykIndex && ykIndex < count + terminals_[i]->getYkSum())
		{
			terminalIndex = i;
			terminalykIndex = ykIndex - count;
			return 0;
		}

		count += terminals_[i]->getYkSum();
	}

	return -1;
}

int CSubStation::TerminalymToSubym(size_t terminalIndex,size_t ymIndex)
{
	if (terminalIndex < 0 || terminalIndex >= getTerminalNum())
	{
		return -1;
	}

	if (ymIndex < 0 || ymIndex >= terminals_[terminalIndex]->getYmSum())
	{
		return -1;
	}

	size_t sum = 0;
	for (size_t i=0; i<terminalIndex;i++)
	{
		sum += terminals_[i]->getYmSum();
	}
	sum += ymIndex;

	return sum;
}

int CSubStation::SubymToTerminalym(size_t ymIndex,size_t & terminalIndex,size_t & terminalymIndex)
{
	if (ymIndex < 0 || ymIndex >= getTerminalYmSum())
	{
		return -1;
	}

	size_t count = 0;
	for (size_t i=0;i<getTerminalNum();i++)
	{
		if (count <= ymIndex && ymIndex < count + terminals_[i]->getYmSum())
		{
			terminalIndex = i;
			terminalymIndex = ymIndex - count;
			return 0;
		}

		count += terminals_[i]->getYmSum();
	}

	return -1;
}

int CSubStation::TerminallineToSubline(size_t terminalIndex,size_t lineIndex)
{
	if (terminalIndex < 0 || terminalIndex >= getTerminalNum())
	{
		return -1;
	}

	if (lineIndex < 0 || lineIndex >= terminals_[terminalIndex]->getYmSum())
	{
		return -1;
	}

	size_t sum = 0;
	for (size_t i=0; i<terminalIndex;i++)
	{
		sum += terminals_[i]->getLineSum();
	}
	sum += lineIndex;

	return sum;
}

int CSubStation::SublineToTerminalline(size_t lineIndex,size_t & terminalIndex,size_t & terminallineIndex)
{
	if (lineIndex < 0 || lineIndex >= getTerminalYmSum())
	{
		return -1;
	}

	size_t count = 0;
	for (size_t i=0;i<getTerminalNum();i++)
	{
		if (count <= lineIndex && lineIndex < count + terminals_[i]->getLineSum())
		{
			terminalIndex = i;
			terminallineIndex = lineIndex - count;
			return 0;
		}

		count += terminals_[i]->getLineSum();
	}

	return -1;
}

//yx api
size_t CSubStation::getYxSum()
{
	return getSubYxSum() + getTerminalYxSum() + getCalcYxSum();
}

size_t CSubStation::getCalcYxSum()
{
	return calc_yxDB_.getDataBaseSum();
}

size_t CSubStation::getSubYxSum()
{
	return sub_yxDB_.getDataBaseSum();
}

size_t CSubStation::getTerminalYxSum()
{
	size_t sum = 0;
	for (size_t i=0; i<getTerminalNum();i++)
	{
		sum += terminals_[i]->getYxSum();
	}

	return sum;
}

int CSubStation::setCalcYxSum(size_t val)
{
	return calc_yxDB_.setDataBaseSum(val);
}

int CSubStation::setSubYxSum(size_t val)
{
	return sub_yxDB_.setDataBaseSum(val);
}

typeYxval CSubStation::getOriYxVal(size_t index)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->getOriYxVal(terminalyxIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		if (dio_)
		{
			int ret = dio_->read_di(index - getTerminalYxSum());
			if (ret >= 0)
			{
				return ret;
			}
		}

		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->getOriYxVal();
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->getOriYxVal();
	}
}

int CSubStation::setOriYxVal(size_t index, typeYxval val)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->setOriYxVal(terminalyxIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->setOriYxVal(val);
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->setOriYxVal(val);
	}
}
typeYxtype CSubStation::getYxType(size_t index)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->getYxType(terminalyxIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->getYxType();
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->getYxType();
	}
}

int CSubStation::setYxType(size_t index,typeYxtype val)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->setYxType(terminalyxIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->setYxType(val);
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->setYxType(val);
	}		
}

bool CSubStation::getYxPolar(size_t index)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->getYxPolar(terminalyxIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->getYxPolar();
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->getYxPolar();
	}
}

int CSubStation::setYxPolar(size_t index,bool val)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->setYxPolar(terminalyxIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->setYxPolar(val);
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->setYxPolar(val);
	}
}

typeYxQuality CSubStation::getYxQuality(size_t index)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->getYxQuality(terminalyxIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		if (dio_)
		{
			if (dio_->check_di(index - getTerminalYxSum()) >= 0)
			{
				return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->getYxQuality();
			}
		}

		return CYxPoint::QualityNegative;

	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->getYxQuality();
	}
}

int CSubStation::setYxQuality(size_t index,typeYxQuality val)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->setYxQuality(terminalyxIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->setYxQuality(val);
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->setYxQuality(val);
	}
}

typeYxval CSubStation::getFinalYxVal(size_t index)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->getFinalYxVal(terminalyxIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->getFinalYxVal();
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->getFinalYxVal();
	}
}

int CSubStation::AddEffectCalcYxIndex(size_t index,size_t val)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->AddEffectYxIndex(terminalyxIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->AddEffectYxIndex(val);
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->AddEffectYxIndex(val);
	}
}

int CSubStation::getEffectCalcYxSum(size_t index)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->getEffectYxSum(terminalyxIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->getEffectYxSum();
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->getEffectYxSum();
	}
}

int CSubStation::getEffectCalcYxIndex(size_t index,int val)
{
	if (index < 0 || index >= getYxSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYxSum())
	{
		size_t terminalIndex;
		size_t terminalyxIndex;
		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
		{
			return terminals_[terminalIndex]->getEffectYxIndex(terminalyxIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else if(index < getTerminalYxSum() + getSubYxSum())
	{
		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->getEffectYxIndex(val);
	}
	else
	{
		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->getEffectYxIndex(val);
	}
}

//yc api
size_t CSubStation::getYcSum()
{
	return getSubYcSum() + getTerminalYcSum();
}

size_t CSubStation::getSubYcSum()
{
	return sub_ycDB_.getDataBaseSum();
}

size_t CSubStation::getTerminalYcSum()
{
	size_t sum = 0;
	for (size_t i=0; i<getTerminalNum();i++)
	{
		sum += terminals_[i]->getYcSum();
	}

	return sum;
}

int CSubStation::setSubYcSum(size_t val)
{
	return sub_ycDB_.setDataBaseSum(val);
}

typeYcval CSubStation::getOriYcVal(size_t index)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->getOriYcVal(terminalycIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->getOriYcVal();
	}
}

int CSubStation::setOriYcVal(size_t index,typeYcval val)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->setOriYcVal(terminalycIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->setOriYcVal(val);
	}
}

typeYcplus CSubStation::getYcPlus(size_t index)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->getYcPlus(terminalycIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->getYcPlus();
	}
}

int CSubStation::setYcPlus(size_t index,typeYcplus val)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->setYcPlus(terminalycIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->setYcPlus(val);
	}
}

typeYcmul CSubStation::getYcMul(size_t index)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->getYcMul(terminalycIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->getYcMul();
	}
}

int CSubStation::setYcMul(size_t index,typeYcmul val)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->setYcMul(terminalycIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->setYcMul(val);
	}
}

typeYcquality CSubStation::getYcQuality(size_t index)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->getYcQuality(terminalycIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->getYcQuality();
	}
}

int CSubStation::setYcQuality(size_t index,typeYcquality val)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->setYcQuality(terminalycIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->setYcQuality(val);
	}
}

typeYcdead CSubStation::getYcDeadLimit(size_t index)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->getYcDeadLimit(terminalycIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->getYcDeadLimit();
	}
}

int CSubStation::setYcDeadLimit(size_t index,typeYcdead val)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->setYcDeadLimit(terminalycIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->setYcDeadLimit(val);
	}
}

typeFinalYcval CSubStation::getFinalYcVal(size_t index,bool bUseMul/* = false*/)
{
	if (index < 0 || index >= getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->getFinalYcVal(terminalycIndex,bUseMul);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ycDB_.getPointDataPtr(index - getTerminalYcSum()))->getFinalYcVal(bUseMul);
	}
}

CYcPoint * CSubStation::getYcPointPtr(int index)
{
	if (index < 0 || index >= (int)getYcSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < (int)getTerminalYcSum())
	{
		size_t terminalIndex;
		size_t terminalycIndex;
		if (!SubycToTerminalyc(index,terminalIndex,terminalycIndex))
		{
			return terminals_[terminalIndex]->getYcPointPtr(terminalycIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return sub_ycDB_.getPointDataPtr(index - getTerminalYcSum());
	}
}

//yk api
size_t CSubStation::getYkSum()
{
	return getSubYkSum() + getTerminalYkSum();
}

size_t CSubStation::getSubYkSum()
{
	return sub_ykDB_.getDataBaseSum();
}

size_t CSubStation::getTerminalYkSum()
{
	size_t sum = 0;
	for (size_t i=0; i<getTerminalNum();i++)
	{
		sum += terminals_[i]->getYkSum();
	}

	return sum;
}

int CSubStation::setSubYkSum(size_t val)
{
	return sub_ykDB_.setDataBaseSum(val);
}

//typeYkstatus CSubStation::getYkStatus(size_t index)
//{
//	if (index < 0 || index >= getYkSum())
//	{
//		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
//	}
//
//	if (index < getTerminalYkSum())
//	{
//		size_t terminalIndex;
//		size_t terminalykIndex;
//		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
//		{
//			return terminals_[terminalIndex]->getYkStatus(terminalykIndex);
//		}
//		else
//		{
//			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
//		}
//	}
//	else
//	{
//		return (sub_ykDB_.getPointDataPtr(index - getTerminalYkSum()))->getYkStatus();
//	}
//}
//
//int CSubStation::setYkStatus(size_t index,typeYkstatus val)
//{
//	if (index < 0 || index >= getYkSum())
//	{
//		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
//	}
//
//	if (index < getTerminalYkSum())
//	{
//		size_t terminalIndex;
//		size_t terminalykIndex;
//		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
//		{
//			return terminals_[terminalIndex]->setYkStatus(terminalykIndex,val);
//		}
//		else
//		{
//			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
//		}
//	}
//	else
//	{
//		return (sub_ykDB_.getPointDataPtr(index - getTerminalYkSum()))->setYkStatus(val);
//	}
//}

CYkPoint * CSubStation::getYkPointPtr(int index)
{
	if (index < 0 || index >= (int)getYkSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < (int)getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			return terminals_[terminalIndex]->getYkPointPtr(terminalykIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return sub_ykDB_.getPointDataPtr(index - getTerminalYkSum());
	}
}

typeYktype CSubStation::getYkType(size_t index)
{
	if (index < 0 || index >= getYkSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			return terminals_[terminalIndex]->getYkType(terminalykIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ykDB_.getPointDataPtr(index - getTerminalYkSum()))->getYkType();
	}
}

int CSubStation::setYkType(size_t index,typeYkstatus val)
{
	if (index < 0 || index >= getYkSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			return terminals_[terminalIndex]->setYkType(terminalykIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ykDB_.getPointDataPtr(index - getTerminalYkSum()))->setYkType(val);
	}
}

bool CSubStation::getbHYkDouble(size_t index)
{
	if (index < 0 || index >= getYkSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			return terminals_[terminalIndex]->getbHYkDouble(terminalykIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ykDB_.getPointDataPtr(index - getTerminalYkSum()))->getbHYkDouble();
	}
}
int CSubStation::setbHYkDouble(size_t index,bool val)
{
	if (index < 0 || index >= getYkSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			return terminals_[terminalIndex]->setbHYkDouble(terminalykIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ykDB_.getPointDataPtr(index - getTerminalYkSum()))->setbHYkDouble(val);
	}
}

bool CSubStation::getbSYkDouble(size_t index)
{
	if (index < 0 || index >= getYkSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			return terminals_[terminalIndex]->getbSYkDouble(terminalykIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ykDB_.getPointDataPtr(index - getTerminalYkSum()))->getbSYkDouble();
	}
}
int CSubStation::setbSYkDouble(size_t index,bool val)
{
	if (index < 0 || index >= getYkSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			return terminals_[terminalIndex]->setbSYkDouble(terminalykIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ykDB_.getPointDataPtr(index - getTerminalYkSum()))->setbSYkDouble(val);
	}
}

void CSubStation::handle_timerDelayLocalYkCon(const boost::system::error_code& error,typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	if (!error)
	{
		SubTemporarySig_(cmdType,ReturnCode,point,val);
	}
}

void CSubStation::DelayLocalYkCon(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	LocalYkConTimer_.expires_from_now(boost::posix_time::seconds(DefaultYkConDelay));
	LocalYkConTimer_.async_wait(boost::bind(&CSubStation::handle_timerDelayLocalYkCon,this,boost::asio::placeholders::error,cmdType,ReturnCode,point,val));
}

void CSubStation::handle_timerResetDev(const boost::system::error_code& error,bool val)
{
#if defined(_BF518_)
	system("reboot");//重启装置
#endif
}

void CSubStation::DelayResetDev(bool val)
{
	ResetDevTimer_.expires_from_now(boost::posix_time::seconds(DefaultYkConDelay + DefaultResetDevDelay));
	ResetDevTimer_.async_wait(boost::bind(&CSubStation::handle_timerResetDev,this,boost::asio::placeholders::error,val));
}

void CSubStation::handle_timerWriteTime2Dev(const boost::system::error_code& error,bool bContinue)
{
#if !defined(_WIN32)
	system("hwclock -w");//设置硬件时钟

	ResetTimerWriteTime2Dev(bContinue);
#endif
}

void CSubStation::ResetTimerWriteTime2Dev(bool bContinue)
{
	if (bContinue)
	{
		WriteTime2Dev_.expires_from_now(boost::posix_time::minutes(DefaultWriteTime2DevTime));
		WriteTime2Dev_.async_wait(boost::bind(&CSubStation::handle_timerWriteTime2Dev,this,boost::asio::placeholders::error,bContinue));
	}
	else
	{
		WriteTime2Dev_.cancel();
	}
}

int CSubStation::AddYkSelCmd(size_t index,bool bCloseOrOpen)
{
	if (index < 0 || index >= getYkSum())
	{
		AddYkRecord(yk_sel_fail,index,bCloseOrOpen);
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			AddYkRecord(yk_sel,index,bCloseOrOpen);
			return terminals_[terminalIndex]->AddYkSelCmd(terminalykIndex,bCloseOrOpen);
		}
		else
		{
			AddYkRecord(yk_sel_fail,index,bCloseOrOpen);
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		if (dio_)
		{
			int ret = dio_->check_do(index - getTerminalYkSum());
			if(ret >= 0)
			{
				if (bCloseOrOpen)
				{
					setYkType(index,YkClose);
				}
				else
				{
					setYkType(index,YkOpen);
				}

				AddYkRecord(yk_sel,index,bCloseOrOpen);
				DelayLocalYkCon(Protocol::YK_SEL_CON,RETURN_CODE_ACTIVE,share_commpoint_ptr(),(int)index);
				return 0;
			}
		}
	}

	AddYkRecord(yk_sel_fail,index,bCloseOrOpen);
	return -1;
}

int CSubStation::AddYkExeCmd(size_t index,bool bCloseOrOpen)
{
	if (index < 0 || index >= getYkSum())
	{
		AddYkRecord(yk_exe_fail,index,bCloseOrOpen);
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			AddYkRecord(yk_exe,index,bCloseOrOpen);
			return terminals_[terminalIndex]->AddYkExeCmd(terminalykIndex,bCloseOrOpen);
		}
		else
		{
			AddYkRecord(yk_exe_fail,index,bCloseOrOpen);
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		if (dio_)
		{
			int ret = dio_->write_do((index - getTerminalYkSum()),bCloseOrOpen);
			if(ret >= 0)
			{
				//if (bCloseOrOpen)
				//{
				//	setYkType(index,YkClose);
				//}
				//else
				//{
				//	setYkType(index,YkOpen);
				//}

				AddYkRecord(yk_exe,index,bCloseOrOpen);
				DelayLocalYkCon(Protocol::YK_EXE_CON,RETURN_CODE_ACTIVE,share_commpoint_ptr(),(int)index);
				return 0;
			}
		}
	}

	AddYkRecord(yk_exe_fail,index,bCloseOrOpen);
	return -1;
}

int CSubStation::AddYkCancelCmd(size_t index,bool bCloseOrOpen /*= true*/)
{
	if (index < 0 || index >= getYkSum())
	{
		AddYkRecord(yk_cancel_fail,index,bCloseOrOpen);
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYkSum())
	{
		size_t terminalIndex;
		size_t terminalykIndex;
		if (!SubykToTerminalyk(index,terminalIndex,terminalykIndex))
		{
			AddYkRecord(yk_cancel,index,bCloseOrOpen);
			return terminals_[terminalIndex]->AddYkCancelCmd(terminalykIndex,bCloseOrOpen);
		}
		else
		{
			AddYkRecord(yk_cancel_fail,index,bCloseOrOpen);
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		if (dio_)
		{
			int ret = dio_->check_do(index - getTerminalYkSum());
			if(ret >= 0)
			{
				//if (bCloseOrOpen)
				//{
				//	setYkType(index,YkClose);
				//}
				//else
				//{
				//	setYkType(index,YkOpen);
				//}

				AddYkRecord(yk_cancel,index,bCloseOrOpen);
				DelayLocalYkCon(Protocol::YK_CANCEL_CON,RETURN_CODE_ACTIVE,share_commpoint_ptr(),(int)index);
				return 0;
			}
		}
	}

	AddYkRecord(yk_cancel_fail,index,bCloseOrOpen);
	return -1;
}

int CSubStation::getTerminalYcINum(int tIndex)
{
	share_bf533_ptr bf533 = getBf533Terminal();

	if (bf533)
	{
		return bf533->getTerminalYcINum(tIndex);
	}
	else
	{
		return -1;
	}
}

int CSubStation::AddBF533Cmd(int tIndex,Protocol::CCmd cmdVal)
{
	int ret = -1;

	share_bf533_ptr bf533 = getBf533Terminal();

	if (bf533)
	{
		if (tIndex > 0)
		{
			ret = bf533->Reconnect();
		}
		else
		{
			ret = bf533->AddGeneralCmd(cmdVal);
		}
	}

	return ret;
}

//ym api
size_t CSubStation::getYmSum()
{
	return getSubYmSum() + getTerminalYmSum();
}

size_t CSubStation::getSubYmSum()
{
	return sub_ymDB_.getDataBaseSum();
}

size_t CSubStation::getTerminalYmSum()
{
	size_t sum = 0;
	for (size_t i=0; i<getTerminalNum();i++)
	{
		sum += terminals_[i]->getYmSum();
	}

	return sum;
}

int CSubStation::setSubYmSum(size_t val)
{
	return sub_ymDB_.setDataBaseSum(val);
}

typeYmval CSubStation::getOriYmVal(size_t index)
{
	if (index < 0 || index >= getYmSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYmSum())
	{
		size_t terminalIndex;
		size_t terminalymIndex;
		if (!SubymToTerminalym(index,terminalIndex,terminalymIndex))
		{
			return terminals_[terminalIndex]->getOriYmVal(terminalymIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ymDB_.getPointDataPtr(index - getTerminalYmSum()))->getOriYmVal();
	}
}

int CSubStation::setOriYmVal(size_t index,typeYmval val)
{
	if (index < 0 || index >= getYmSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYmSum())
	{
		size_t terminalIndex;
		size_t terminalymIndex;
		if (!SubymToTerminalym(index,terminalIndex,terminalymIndex))
		{
			return terminals_[terminalIndex]->setOriYmVal(terminalymIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ymDB_.getPointDataPtr(index - getTerminalYmSum()))->setOriYmVal(val);
	}
}

typeYmquality CSubStation::getYmQuality(size_t index)
{
	if (index < 0 || index >= getYmSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYmSum())
	{
		size_t terminalIndex;
		size_t terminalymIndex;
		if (!SubymToTerminalym(index,terminalIndex,terminalymIndex))
		{
			return terminals_[terminalIndex]->getYmQuality(terminalymIndex);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ymDB_.getPointDataPtr(index - getTerminalYmSum()))->getYmQuality();
	}
}

int CSubStation::setYmQuality(size_t index,typeYmquality val)
{
	if (index < 0 || index >= getYmSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalYmSum())
	{
		size_t terminalIndex;
		size_t terminalymIndex;
		if (!SubymToTerminalym(index,terminalIndex,terminalymIndex))
		{
			return terminals_[terminalIndex]->setYmQuality(terminalymIndex,val);
		}
		else
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
		}
	}
	else
	{
		return (sub_ymDB_.getPointDataPtr(index - getTerminalYmSum()))->setYmQuality(val);
	}
}

//line api
size_t CSubStation::getLineSum()
{
	size_t sum = 0;
	for (size_t i=0; i<getTerminalNum();i++)
	{
		sum += terminals_[i]->getLineSum();
	}

	return sum;
}

CLine * CSubStation::getLinePtr(int index)
{
	if (index < 0 || index >= (int)getLineSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	size_t terminalIndex;
	size_t terminallineIndex;
	if (!SublineToTerminalline(index,terminalIndex,terminallineIndex))
	{
		return terminals_[terminalIndex]->getLinePtr(terminallineIndex);
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}
}

//comm api
share_commpoint_ptr CSubStation::getCommPoint(size_t index)
{
	if (index < 0 || index >= getTerminalNum() + getPristationNum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (index < getTerminalNum())
	{
		return terminals_[index];
	}
	else
	{
		return pristations_[index - getTerminalNum()];
	}
}

int CSubStation::InitCommPoints()
{
	for (size_t i=0;i<getCommPointNum();i++)
	{
		try
		{
			size_t j=0;
			for (;j<i;j++)
			{
				bool bSeverChannel;
				if(CommInterface::CCommFactory::CompareCommPara(getCommPoint(i)->getCommPara(),getCommPoint(j)->getCommPara(),bSeverChannel))
				{
					if (bSeverChannel)
					{
						getCommPoint(i)->EnableCommunication(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4),getCommPoint(j)->getServerPtr(),false);
					}
					else
					{
						getCommPoint(i)->EnableCommunication(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4),getCommPoint(j)->getCommPtr(),false);
					}

					break;
				}
			}

			if (j >= i)
			{
				getCommPoint(i)->EnableCommunication(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4));
			}

		}

		catch(PublicSupport::dat2def_exception & e)
		{
			std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
			if (strPtr)
			{
				std::ostringstream ostr;
				ostr<<"激活第"<<i<<"个通讯结点失败，";
				int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
				if (indexPtr)
				{
					ostr<<"data no:"<<(*indexPtr)<<"，";
				}
				ostr<<"error info:"<<(*strPtr)<<std::endl;
				AddInitCommLogWithSynT(ostr.str());
			}
		}
	}

	AddInitCommLogWithSynT("激活各个通道的通讯完毕！\n");

	io_service_.run(); //服务池线程开始工作

	return 0;
}

//int CSubStation::InitCommPoints2()
//{
//	//FileSystem::CLog EnableCommLog("EnableComm.log","a");
//
//	for (size_t i=0;i<getTerminalNum() + getPristationNum();i++)
//	{
//		try
//		{
//			int ret = -1;
//			bool bMatched = false;
//
//			for (size_t j=0;j<i;j++)
//			{
//				if ((getCommPoint(i)->getCommChannelType() == getCommPoint(j)->getCommChannelType()) 
//					&& (boost::algorithm::iequals(getCommPoint(i)->getPort(),getCommPoint(j)->getPort())))
//				{
//					switch (getCommPoint(i)->getCommChannelType())
//					{
//					case COMMTYPE_TCP_CLIENT:
//						if (boost::algorithm::iequals(getCommPoint(i)->getNetIP(),getCommPoint(j)->getNetIP()))
//						{
//							ret = getCommPoint(i)->EnableCommunication(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4),getCommPoint(j)->getCommPtr(),false);
//							bMatched = true;
//						}
//						break;
//
//					case COMMTYPE_UDP_CLIENT:
//						ret = getCommPoint(i)->EnableCommunication(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4),getCommPoint(j)->getCommPtr(),false);
//						bMatched = true;
//						break;
//
//					case COMMTYPE_SERIAL_PORT:
//						ret = getCommPoint(i)->EnableCommunication(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4),getCommPoint(j)->getCommPtr(),false);
//						bMatched = true;
//						break;
//
//					case COMMTYPE_TCP_SERVER:
//						ret = getCommPoint(i)->EnableCommunication(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4),getCommPoint(j)->getServerPtr(),false);
//						bMatched = true;
//						break;
//
//					case COMMTYPE_UDP_SERVER:
//						ret = getCommPoint(i)->EnableCommunication(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4),getCommPoint(j)->getServerPtr(),false);
//						bMatched = true;
//						break;
//
//					default:
//						break;
//					}
//
//
//					if (bMatched)
//					{
//						break;
//					}
//				}
//			
//			}
//			if (!bMatched)
//			{
//				getCommPoint(i)->EnableCommunication(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4));
//
//				/*
//				if (!ret)
//				{
//					if (getCommPoint(i)->getCommPointType() == TERMINAL_NODE)
//					{
//						getCommPoint(i)->ConnectCmdRecallSig(boost::bind(&CSubStation::ProcessRecallCmd,this,_1,_2,_3,_4));
//					}
//				}
//				*/
//			}
//
//		}
//
//		catch(PublicSupport::dat2def_exception & e)
//		{
//			std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
//			if (strPtr)
//			{
//				std::ostringstream ostr;
//				ostr<<"激活第"<<i<<"个通讯结点失败，";
//				int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
//				if (indexPtr)
//				{
//					ostr<<"data no:"<<(*indexPtr)<<"，";
//				}
//				ostr<<"error info:"<<(*strPtr)<<std::endl;
//				AddInitCommLogWithSynT(ostr.str());
//			}
//		}
//	}
//
//	AddInitCommLogWithSynT("激活各个通道的通讯完毕！\n");
//	
//	io_service_.run(); //服务池线程开始工作
//
//	return 0;
//}

void CSubStation::UninitCommPoints()
{

	for (size_t i=0;i<getTerminalNum() + getPristationNum();i++)
	{
		getCommPoint(i)->DisableCommunication();
	}

	io_service_.reset();
}

size_t CSubStation::getCommPointNum()
{
	return getTerminalNum() + getPristationNum();
}

//Local api
std::string CSubStation::getBaStr()
{
	return strBA_;
}

int CSubStation::setBaStr(std::string val)
{
	strBA_ = val;

	return 0;
}

std::string CSubStation::getDaStr()
{
	return strda_;
}

int CSubStation::setDaStr(std::string val)
{
	strda_ = val;

	return 0;
}

std::string CSubStation::getLightStr()
{
	return strLight_;
}

int CSubStation::setLightStr(std::string val)
{
	strLight_ = val;

	return 0;
}

int CSubStation::EnableWatchDog()
{
	watchdog_.reset(LocalDrive::CWatchDogFactory::CreateWatchDog(getWatchdogStr(),io_service_,getbWatchdogLog()));

	if (watchdog_)
	{
		if(watchdog_->run())
		{
			watchdog_->stop();
			watchdog_.reset();

			return -1;
		}
	}

	return 0;
}

int CSubStation::EnableBlueTooth()
{
	bluetooth_.reset(new LocalDrive::CBluetooth_BF518(false,getAddr()));

	if (bluetooth_)
	{
		if(bluetooth_->run())
		{
			bluetooth_->stop();
			bluetooth_.reset();

			return -1;
		}
	}

	return 0;
}

int CSubStation::EnableDIO()
{
	if ((!getDioStr().empty()) && (getSubYxSum() > 0) || (getSubYkSum() > 0))
	{
		dio_.reset(LocalDrive::CDIOFactory::CreateDIO(getDioStr(),*this));

		if(dio_)
		{
			if(dio_->open())
			{
				dio_->close();
				dio_.reset();

				return -1;
			}
		}
	}

	return 0;
}

int CSubStation::EnableDA()
{
	if(!getDaStr().empty())
	{
		da_op_.reset(new DataBase::CDAOperateSub(*this));
		da_.reset(new CentralizedFA::CCentralizedDA(getDaStr(),*da_op_));
		int ret = da_->InitAlgorithm();
		if (ret)
		{
			std::ostringstream ostr;
			ostr<<"初始化DA参数失败，DA将不启动"<<std::endl;
			AddLoadSubCfgWithSynT(ostr.str());

			da_.reset();

			return -1;
		}

		if (da_)
		{
			return da_->start(SubAliveSig_);
		}
	}

	return 0;
}

int CSubStation::EnableBA()
{
	if(!getBaStr().empty())
	{
		ba_.reset(new LocalDrive::CBatteryActive(io_service_,getBaStr(),*this));
		if (ba_)
		{
			if (ba_->run())
			{
				ba_->stop();
				ba_.reset();

				return -1;
			}
		}
	}

	return 0;
}

int CSubStation::EnableLight()
{
	if(!getLightStr().empty())
	{
		light_.reset(new LocalDrive::CLightDriver_TPE3000(io_service_));
		if (light_)
		{
			if (light_->run())
			{
				light_->stop();
				light_.reset();

				return -1;
			}

			for (std::vector<share_pristation_ptr>::iterator it = pristations_.begin();it != pristations_.end();it++)
			{
				(*it)->setbAcceptSynSubTime(false); //打开了1588对时，则其他的通讯报文对时端口会被关闭
			}
		}
	}

	return 0;
}

int CSubStation::EnableIEEE1588()
{
	if (timeOutWriteCmosClock_ > 0)
	{
		ieee1588_.reset(new LocalDrive::CIEEE1588_TPE3000(io_service_,timeOutWriteCmosClock_));
		if (ieee1588_)
		{
			if (ieee1588_->run())
			{
				ieee1588_->stop();
				ieee1588_.reset();

				return -1;
			}
		}
	}

	return 0;
}

bool CSubStation::CheckEventProcessReady()
{
	return light_;
}

int CSubStation::InitLocalServices()
{
	EnableBlueTooth();

	EnableWatchDog();

	EnableDIO();

	EnableLight();

	EnableDA();

	EnableBA();

	EnableIEEE1588();

	bool bEventReady = CheckEventProcessReady();

	if (bEventReady)
	{
		ConnectEventStatusSig(boost::bind(&CSubStation::ProcessEventCmd,this,_1,_2,_3,_4));
	}

	for (size_t i=0;i<getCommPointNum();i++)
	{
		getCommPoint(i)->InitLocalServices(boost::bind(&CSubStation::ProcessEventCmd,this,_1,_2,_3,_4),bEventReady);
	}

	ResetTimerWriteTime2Dev(true);

	return 0;
}

void CSubStation::DisableWatchDog()
{
	if(watchdog_)
	{
		watchdog_->stop();
	}
}

void CSubStation::DisableBluetooth()
{
	if (bluetooth_)
	{
		bluetooth_->stop();
	}
}

void CSubStation::DisableDIO()
{
	if (dio_)
	{
		dio_->close();
	}
}

void CSubStation::DisableDA()
{
	if (da_)
	{
		da_->stop();
	}
}

void CSubStation::DisableBA()
{
	if (ba_)
	{
		ba_->stop();
	}
}

void CSubStation::DisableLight()
{
	if (light_)
	{
		light_->stop();
	}
}

void CSubStation::DisableIEEE1588()
{
	if (ieee1588_)
	{
		ieee1588_->stop();
	}
}

void CSubStation::UninitLocalServices()
{
	DisableBluetooth();

	DisableWatchDog();

	DisableDIO();

	DisableLight();

	DisableDA();

	DisableBA();

	DisableIEEE1588();

	DisconnectEventStatusSig();

	for (size_t i=0;i<getCommPointNum();i++)
	{
		getCommPoint(i)->UnInitLocalServices();
	}
}

void CSubStation::ReFeedWatchDog()
{
	if (watchdog_)
	{
		watchdog_->reset();
	}
}

int CSubStation::WriteSysTime(unsigned short year,unsigned char month,unsigned char day,unsigned char hour,unsigned char minnutes,unsigned char seconds,unsigned short milliseconds,bool bSynTerminal)
{
	//std::cout<<"before write:"<<boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time())<<std::endl;

	CSubStation::WriteSystemTime(year,month,day,hour,minnutes,seconds,milliseconds);

	//std::cout<<"after write:"<<boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time())<<std::endl;

	ReFeedWatchDog();

#ifdef _BF518_
	if(bSynTerminal)
	{
		if (getBf533Terminal())
		{
			getBf533Terminal()->SynTime();
		}
	}
#endif

	return 0;
}

//xml cfg api
int CSubStation::LoadXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;

	if (!xml.Load(filename))
	{
		std::cerr<<"导入xml配置文件"<<filename<<"失败！"<<std::endl;
		return -1;
	}

	terminals_.clear();
	pristations_.clear();

	xml.ResetMainPos();
	xml.FindElem(); //root strSubStation
	xml.IntoElem();  //enter strSubStation

	xml.ResetMainPos();
	if (xml.FindElem(strLoadSubCfgLogName))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (!strTmp.empty())
		{
			std::string strType = xml.GetAttrib(strFileType);
			boost::algorithm::trim(strType);
			std::string strFileLimit = xml.GetAttrib(strLimit);
			boost::algorithm::trim(strFileLimit);

			EnableLoadSubCfgLog(strTmp,strType,strFileLimit);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strInitCommLogName))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (!strTmp.empty())
		{
			std::string strType = xml.GetAttrib(strFileType);
			boost::algorithm::trim(strType);
			std::string strFileLimit = xml.GetAttrib(strLimit);
			boost::algorithm::trim(strFileLimit);

			EnableInitCommLog(strTmp,strType,strFileLimit);		}
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

	xml.ResetMainPos();
	if (xml.FindElem(strEnableIEEE1588))
	{
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			timeOutWriteCmosClock_ = boost::lexical_cast<unsigned short>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			timeOutWriteCmosClock_ = 0;

			std::ostringstream ostr;
			ostr<<"非法的IEEE1588参数："<<e.what()<<"，将使用默认地址值\n";
			AddLoadSubCfgWithSynT(ostr.str());
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strEnableLight))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setLightStr(strTmp);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strEnableBattery))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setBaStr(strTmp);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strEnableDA))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setDaStr(strTmp);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strEnableWatchdog))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setWatchdogStr(strTmp);

		strTmp = xml.GetAttrib(strWatchdogLog);
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strTmp,strboolTrue))
		{
			setbWatchdogLog(true);
		}
		else
		{
			setbWatchdogLog(false);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strEnableDIO))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setDioStr(strTmp);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strSubAddr))
	{
		unsigned short iaddr = DefaulAddr;
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			iaddr = boost::lexical_cast<unsigned short>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的子站地址参数："<<e.what()<<"，将使用默认地址值\n";
			AddLoadSubCfgWithSynT(ostr.str());

			//LoadCfgLog.AddStringWithSynT("非法的子站地址参数：");
			//LoadCfgLog.AddString(e.what());
			//LoadCfgLog.AddString("，将使用默认地址值\n");
		}
		setAddr(iaddr);
	}
	else
	{
		setAddr(DefaulAddr);
		AddLoadSubCfgWithSynT("未能找到子站地址配置项，将使用默认地址值\n");
		//LoadCfgLog.AddStringWithSynT("未能找到子站地址配置项，将使用默认地址值\n");
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYxDB))
	{
		try
		{
			sub_yxDB_.InitDataBase(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
			if (strPtr)
			{
				std::ostringstream ostr;
				ostr<<"初始化"<<strYxDB<<"失败，";
				int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
				if (indexPtr)
				{
					ostr<<"data no:"<<(*indexPtr)<<"，";
				}
				ostr<<"error info:"<<(*strPtr)<<std::endl;
				AddLoadSubCfgWithSynT(ostr.str());
				//LoadCfgLog.AddStringWithSynT(ostr.str());
			}

		}

	}

	xml.ResetMainPos();
	if (xml.FindElem(strYcDB))
	{
		try
		{
			sub_ycDB_.InitDataBase(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
			if (strPtr)
			{
				std::ostringstream ostr;
				ostr<<"初始化"<<strYcDB<<"失败，";
				int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
				if (indexPtr)
				{
					ostr<<"data no:"<<(*indexPtr)<<"，";
				}
				ostr<<"error info:"<<(*strPtr)<<std::endl;
				AddLoadSubCfgWithSynT(ostr.str());
				//LoadCfgLog.AddStringWithSynT(ostr.str());
			}

		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYmDB))
	{
		try
		{
			sub_ymDB_.InitDataBase(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
			if (strPtr)
			{
				std::ostringstream ostr;
				ostr<<"初始化"<<strYmDB<<"失败，";
				int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
				if (indexPtr)
				{
					ostr<<"data no:"<<(*indexPtr)<<"，";
				}
				ostr<<"error info:"<<(*strPtr)<<std::endl;
				AddLoadSubCfgWithSynT(ostr.str());
				//LoadCfgLog.AddStringWithSynT(ostr.str());
			}

		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYkDB))
	{
		try
		{
			sub_ykDB_.InitDataBase(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
			if (strPtr)
			{
				std::ostringstream ostr;
				ostr<<"初始化"<<strYkDB<<"失败，";
				int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
				if (indexPtr)
				{
					ostr<<"data no:"<<(*indexPtr)<<"，";
				}
				ostr<<"error info:"<<(*strPtr)<<std::endl;
				AddLoadSubCfgWithSynT(ostr.str());
				//LoadCfgLog.AddStringWithSynT(ostr.str());
			}

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
				std::cerr<<"CSubStation::LoadXmlCfg "<<e.what()<<std::endl;
				limit = -1;
			}
		}

		std::string strID = xml.GetData();
		boost::algorithm::trim(strID);

		if (limit > 0)
		{
			YkRecordPtr_.reset(new CYkRecord(strID,limit));
		}
		else if(limit == 0)
		{
			YkRecordPtr_.reset();
		}
		else
		{
			YkRecordPtr_.reset(new CYkRecord(strID,DefaultYkRecordLimit));
		}
	}
	else
	{
		YkRecordPtr_.reset(new CYkRecord(DefaultYkRecordPath,DefaultYkRecordLimit));
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
				std::cerr<<"CSubStation::LoadXmlCfg "<<e.what()<<std::endl;
				limit = -1;
			}
		}

		std::string strID = xml.GetData();
		boost::algorithm::trim(strID);

		if (limit > 0)
		{
			SoeRecordPtr_.reset(new CSoeRecord(strID,limit));
		}
		else if(limit == 0)
		{
			SoeRecordPtr_.reset();
		}
		else
		{
			SoeRecordPtr_.reset(new CSoeRecord(strID,DefaultSoeRecordLimit));
		}
	}
	else
	{
		SoeRecordPtr_.reset(new CSoeRecord(DefaultSoeRecordPath,DefaultSoeRecordLimit));
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
				std::cerr<<"CSubStation::LoadXmlCfg "<<e.what()<<std::endl;
				limit = -1;
			}
		}

		std::string strID = xml.GetData();
		boost::algorithm::trim(strID);

		if (limit > 0)
		{
			CosRecordPtr_.reset(new CCosRecord(strID,limit));
		}
		else if(limit == 0)
		{
			CosRecordPtr_.reset();
		}
		else
		{
			CosRecordPtr_.reset(new CCosRecord(strID,DefaultCosRecordLimit));
		}
	}
	else
	{
		CosRecordPtr_.reset(new CCosRecord(DefaultCosRecordPath,DefaultCosRecordLimit));
	}

	xml.ResetMainPos();
	if (xml.FindElem(strTerminals))
	{

		int sum = 0;
		try
		{
			std::string strTmp = xml.GetAttrib(strTerminalNum);
			boost::algorithm::trim(strTmp);
			sum = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<< "不能将"<<strTerminalNum<<"属性转换为数字。exception: " << e.what()<<". ";
			ostr<< "这个属性将被忽略."<<std::endl;
			AddLoadSubCfgWithSynT(ostr.str());
			//LoadCfgLog.AddStringWithSynT(ostr.str());
		}

		bool bIndexOrderEnable = false;
		std::string strTmp = xml.GetAttrib(strIndexOrderEnable);
		boost::algorithm::trim(strTmp);
		if (boost::algorithm::iequals(strboolTrue,strTmp))
		{
			bIndexOrderEnable = true;
		}

		xml.IntoElem(); //enter strTerminals

		if (sum > 0)
		{
			if (bIndexOrderEnable)
			{
				for (int i=0; i<sum; i++)
				{
					share_terminal_ptr ptrTmp;
					terminals_.push_back(ptrTmp);
				}

				int count = 0;
				while (xml.FindElem(strTerminalNode) && count < sum)
				{
					int index = -1;
					try
					{
						std::string strTmp = xml.GetAttrib(strTerminalIndex);
						boost::algorithm::trim(strTmp);
						index = boost::lexical_cast<int>(strTmp);
					}
					catch(boost::bad_lexical_cast& e)
					{
						std::ostringstream ostr;
						ostr<<"不能将第"<<count<<strTerminalNode<<"结点的"<<strTerminalIndex<<"属性转换为数字。 exception: " << e.what()<<"。";
						ostr<< "这个结点的配置将被忽略。"<<std::endl;
						AddLoadSubCfgWithSynT(ostr.str());
						//LoadCfgLog.AddStringWithSynT(ostr.str());
					}

					if (index >=0 && index < sum)
					{
						std::string strType = xml.GetAttrib(strTerminalType);
						//terminals_[index].reset(CTerminalFactroy::CreateTerminal(strType,io_service_,*this));
						terminals_[index] = CTerminalFactroy::CreateTerminal(strType,io_service_,*this);

						xml.IntoElem();
						try
						{
							terminals_[index]->InitTerminal(xml);
						}
						catch(PublicSupport::dat2def_exception & e)
						{
							terminals_[index].reset();

							std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
							if (strPtr)
							{
								std::ostringstream ostr;
								ostr<<"初始化第"<<count<<"个"<<strTerminalNode<<"失败，";

								std::string const * strPtrMiddle = boost::get_error_info<errinfo_middletype_name>(e);
								if (strPtrMiddle)
								{
									ostr<<" item info:"<<(*strPtrMiddle);
								}

								int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
								if (indexPtr)
								{
									ostr<<" data no:"<<(*indexPtr)<<"，";
								}

								ostr<<"error info:"<<(*strPtr)<<"。";
								ostr<< "这个结点的配置将被忽略。"<<std::endl;
								AddLoadSubCfgWithSynT(ostr.str());
								//LoadCfgLog.AddStringWithSynT(ostr.str());
							}
						}

						xml.OutOfElem();
					}
					else
					{
						std::ostringstream ostr;
						ostr<<"第"<<count<<strTerminalNode<<"的"<<strTerminalIndex<<"属性值越限，";
						ostr<< "这个结点的配置将被忽略。"<<std::endl;
						AddLoadSubCfgWithSynT(ostr.str());
						//LoadCfgLog.AddStringWithSynT(ostr.str());
					}

					count++;
				}
			}
			else
			{
				int count = 0;
				while (xml.FindElem(strTerminalNode) && count < sum)
				{
					std::string strType = xml.GetAttrib(strTerminalType);
					share_terminal_ptr ptrTmp(CTerminalFactroy::CreateTerminal(strType,io_service_,*this));

					xml.IntoElem();
					try
					{
						ptrTmp->InitTerminal(xml);
					}
					catch(PublicSupport::dat2def_exception & e)
					{
						std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
						if (strPtr)
						{
							std::ostringstream ostr;
							ostr<<"初始化第"<<count<<"个"<<strTerminalNode<<"失败，";

							std::string const * strPtrMiddle = boost::get_error_info<errinfo_middletype_name>(e);
							if (strPtrMiddle)
							{
								ostr<<" item info:"<<(*strPtrMiddle);
							}

							int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
							if (indexPtr)
							{
								ostr<<" data no:"<<(*indexPtr)<<"，";
							}

							ostr<<"error info:"<<(*strPtr)<<"。";
							ostr<< "这个结点的配置将被忽略。"<<std::endl;
							AddLoadSubCfgWithSynT(ostr.str());
							//LoadCfgLog.AddStringWithSynT(ostr.str());
						}
					}
					terminals_.push_back(ptrTmp);
					xml.OutOfElem();

					count++;
				}
			}
		}
		else
		{
			int count = 0;
			while (xml.FindElem(strTerminalNode))
			{
				std::string strType = xml.GetAttrib(strTerminalType);
				share_terminal_ptr ptrTmp(CTerminalFactroy::CreateTerminal(strType,io_service_,*this));

				xml.IntoElem();
				try
				{
					ptrTmp->InitTerminal(xml);
				}
				catch(PublicSupport::dat2def_exception & e)
				{
					std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
					if (strPtr)
					{
						std::ostringstream ostr;
						ostr<<"初始化第"<<count<<"个"<<strTerminalNode<<"失败，";

						std::string const * strPtrMiddle = boost::get_error_info<errinfo_middletype_name>(e);
						if (strPtrMiddle)
						{
							ostr<<" item info:"<<(*strPtrMiddle);
						}

						int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
						if (indexPtr)
						{
							ostr<<" data no:"<<(*indexPtr)<<"，";
						}

						ostr<<"error info:"<<(*strPtr)<<"。";
						ostr<< "这个结点的配置将被忽略。"<<std::endl;
						AddLoadSubCfgWithSynT(ostr.str());
						//LoadCfgLog.AddStringWithSynT(ostr.str());
					}
				}
				terminals_.push_back(ptrTmp);
				xml.OutOfElem();

				count++;
			}
		}

		xml.OutOfElem(); //out strTerminals
	}

	xml.ResetMainPos();
	if (xml.FindElem(strCalcYxDB))
	{
		try
		{
			calc_yxDB_.InitDataBase(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
			if (strPtr)
			{
				std::ostringstream ostr;
				ostr<<"初始化"<<strCalcYxDB<<"失败，";
				int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
				if (indexPtr)
				{
					ostr<<"data no:"<<(*indexPtr)<<"，";
				}
				ostr<<"error info:"<<(*strPtr)<<std::endl;
				AddLoadSubCfgWithSynT(ostr.str());
				//LoadCfgLog.AddStringWithSynT(ostr.str());
			}

		}

		for (size_t i=0;i<calc_yxDB_.getDataBaseSum();i++)
		{
			(calc_yxDB_.getPointDataPtr(i))->setFuctionObj(boost::bind(&CSubStation::getOriYxVal,this,_1));

			if((calc_yxDB_.getPointDataPtr(i))->getEnableEffectEvent())
			{
				size_t leftIndex = (calc_yxDB_.getPointDataPtr(i))->getLeftIndex();
				size_t rightIndex = (calc_yxDB_.getPointDataPtr(i))->getRightIndex();
				if (leftIndex >= 0 && leftIndex < (getTerminalYxSum() + getSubYxSum()))
				{
					AddEffectCalcYxIndex(leftIndex,(getTerminalYxSum() + getSubYxSum() + i));
				}

				if (rightIndex >= 0 && rightIndex < (getTerminalYxSum() + getSubYxSum()))
				{
					AddEffectCalcYxIndex(rightIndex,(getTerminalYxSum() + getSubYxSum() + i));
				}
			}
		}

	}

	xml.ResetMainPos();
	if (xml.FindElem(strPriStations))
	{
		int sum = 0;
		try
		{
			std::string strTmp = xml.GetAttrib(strPriStationNum);
			boost::algorithm::trim(strTmp);
			sum = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<< "不能将"<<strPriStationNum<<"属性转换为数字。exception: " << e.what()<<"。";
			ostr<< "这个属性将被忽略。"<<std::endl;
			AddLoadSubCfgWithSynT(ostr.str());
			//LoadCfgLog.AddStringWithSynT(ostr.str());
		}

		bool bIndexOrderEnable = false;
		std::string strTmp = xml.GetAttrib(strIndexOrderEnable);
		boost::algorithm::trim(strTmp);
		if (boost::algorithm::iequals(strboolTrue,strTmp))
		{
			bIndexOrderEnable = true;
		}

		xml.IntoElem(); //enter strPriStations

		if (sum > 0)
		{
			if (bIndexOrderEnable)
			{
				for (int i=0; i<sum; i++)
				{
					share_pristation_ptr ptrTmp;
					pristations_.push_back(ptrTmp);
				}

				int count = 0;
				while (xml.FindElem(strPriStationNode) && count < sum)
				{
					int index = -1;
					try
					{
						std::string strTmp = xml.GetAttrib(strPriStationIndex);
						boost::algorithm::trim(strTmp);
						index = boost::lexical_cast<int>(strTmp);
					}
					catch(boost::bad_lexical_cast& e)
					{
						std::ostringstream ostr;
						ostr<<"不能将第"<<count<<strPriStationNode<<"的"<<strPriStationIndex<<"属性转换为数字。exception: " << e.what()<<". ";
						ostr<< "这个结点的配置将被忽略。"<<std::endl;
						AddLoadSubCfgWithSynT(ostr.str());
						//LoadCfgLog.AddStringWithSynT(ostr.str());
					}

					if (index >=0 && index < sum)
					{
						pristations_[index].reset(new CPriStation(io_service_,*this));

						xml.IntoElem();
						try
						{
							pristations_[index]->InitPriStation(xml);
						}
						catch(PublicSupport::dat2def_exception & e)
						{
							pristations_[index].reset();

							std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
							if (strPtr)
							{
								std::ostringstream ostr;
								ostr<<"初始化第"<<count<<"个"<<strPriStationNode<<"失败，";

								std::string const * strPtrMiddle = boost::get_error_info<errinfo_middletype_name>(e);
								if (strPtrMiddle)
								{
									ostr<<" item info:"<<(*strPtrMiddle);
								}

								int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
								if (indexPtr)
								{
									ostr<<" data no:"<<(*indexPtr)<<"，";
								}

								ostr<<"error info:"<<(*strPtr)<<"。";
								ostr<< "这个结点的配置将被忽略。"<<std::endl;
								AddLoadSubCfgWithSynT(ostr.str());
								//LoadCfgLog.AddStringWithSynT(ostr.str());
							}
						}

						xml.OutOfElem();
					}
					else
					{
						std::ostringstream ostr;
						ostr<<"第"<<count<<strPriStationNode<<"的"<<strPriStationIndex<<"属性值越限，";
						ostr<< "这个结点的配置将被忽略。"<<std::endl;
						AddLoadSubCfgWithSynT(ostr.str());
						//LoadCfgLog.AddStringWithSynT(ostr.str());
					}

					count++;
				}
			}
			else
			{
				int count = 0;
				while (xml.FindElem(strPriStationNode) && count < sum)
				{
					share_pristation_ptr ptrTmp(new CPriStation(io_service_,*this));

					xml.IntoElem();
					try
					{
						ptrTmp->InitPriStation(xml);
					}
					catch(PublicSupport::dat2def_exception & e)
					{
						std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
						if (strPtr)
						{
							std::ostringstream ostr;
							ostr<<"初始化第"<<count<<"个"<<strPriStationNode<<"失败，";

							std::string const * strPtrMiddle = boost::get_error_info<errinfo_middletype_name>(e);
							if (strPtrMiddle)
							{
								ostr<<" item info:"<<(*strPtrMiddle);
							}

							int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
							if (indexPtr)
							{
								ostr<<" data no:"<<(*indexPtr)<<"，";
							}

							ostr<<"error info:"<<(*strPtr)<<"。";
							ostr<< "这个结点的配置将被忽略。"<<std::endl;
							AddLoadSubCfgWithSynT(ostr.str());
							//LoadCfgLog.AddStringWithSynT(ostr.str());
						}
					}
					pristations_.push_back(ptrTmp);
					xml.OutOfElem();

					count++;
				}
			}
		}
		else
		{
			int count = 0;
			while (xml.FindElem(strPriStationNode))
			{
				share_pristation_ptr ptrTmp(new CPriStation(io_service_,*this));

				xml.IntoElem();
				try
				{
					ptrTmp->InitPriStation(xml);
				}
				catch(PublicSupport::dat2def_exception & e)
				{
					std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
					if (strPtr)
					{
						std::ostringstream ostr;
						ostr<<"初始化第"<<count<<"个"<<strPriStationNode<<"失败，";

						std::string const * strPtrMiddle = boost::get_error_info<errinfo_middletype_name>(e);
						if (strPtrMiddle)
						{
							ostr<<" item info:"<<(*strPtrMiddle);
						}

						int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
						if (indexPtr)
						{
							ostr<<" data no:"<<(*indexPtr)<<"，";
						}

						ostr<<"error info:"<<(*strPtr)<<"。";
						ostr<< "这个结点的配置将被忽略。"<<std::endl;
						AddLoadSubCfgWithSynT(ostr.str());
						//LoadCfgLog.AddStringWithSynT(ostr.str());
					}
				}
				pristations_.push_back(ptrTmp);
				xml.OutOfElem();

				count++;
			}
		}

		xml.OutOfElem(); //out strPriStations
	}

	xml.OutOfElem(); //out strSubStation

	AddLoadSubCfgWithSynT("初始化子站配置文件成功结束！\n");

	return 0;
}

void CSubStation::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strSubXsl);

	xml.AddElem(strSubStation);
	xml.IntoElem();//enter strSubStation

	if (LoadSubCfgLog_)
	{
		xml.AddElem(strLoadSubCfgLogName,LoadSubCfgLog_->getLogPath());
		if (!LoadSubCfgLog_->getFileType().empty())
		{
			xml.AddAttrib(strFileType,LoadSubCfgLog_->getFileType());
		}
	}

	if (InitCommLog_)
	{
		xml.AddElem(strInitCommLogName,InitCommLog_->getLogPath());
		if (!InitCommLog_->getFileType().empty())
		{
			xml.AddAttrib(strFileType,InitCommLog_->getFileType());
		}
	}

	if (bSynTimeImmediately_)
	{
		xml.AddElem(strSynImm,strboolTrue);
	}

	if (bCallDataImmediately_)
	{
		xml.AddElem(strCallImm,strboolTrue);
	}

	if (ieee1588_)
	{
		xml.AddElem(strEnableIEEE1588,timeOutWriteCmosClock_);
	}

	if (light_)
	{
		xml.AddElem(strEnableLight,getLightStr());
	}

	if (ba_)
	{
		xml.AddElem(strEnableBattery,getBaStr());
	}

	if (da_)
	{
		xml.AddElem(strEnableDA,getDaStr());
	}

	xml.AddElem(strEnableWatchdog,getWatchdogStr());
	if (getbWatchdogLog())
	{
		xml.AddAttrib(strWatchdogLog,strboolTrue);
	}

	if (dio_)
	{
		xml.AddElem(strEnableDIO,getDioStr());
	}

	xml.AddElem(strSubAddr,getAddr());

	if (sub_yxDB_.getDataBaseSum() > 0)
	{
		xml.AddElem(strYxDB);
		sub_yxDB_.SaveXmlCfg(xml);
	}

	if (sub_ycDB_.getDataBaseSum() > 0)
	{
		xml.AddElem(strYcDB);
		sub_ycDB_.SaveXmlCfg(xml);
	}

	if (sub_ymDB_.getDataBaseSum() > 0)
	{
		xml.AddElem(strYmDB);
		sub_ymDB_.SaveXmlCfg(xml);
	}

	if (sub_ykDB_.getDataBaseSum() > 0)
	{
		xml.AddElem(strYkDB);
		sub_ykDB_.SaveXmlCfg(xml);
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

	if(getTerminalNum() > 0)
	{
		xml.AddElem(strTerminals);
		xml.AddAttrib(strTerminalNum,getTerminalNum());
		xml.AddAttrib(strIndexOrderEnable,strboolFalse);
		xml.IntoElem();//enter strTerminals
		for (size_t i=0;i<getTerminalNum();i++)
		{
			if (terminals_[i])
			{
				xml.AddElem(strTerminalNode);
				xml.AddAttrib(strTerminalType,CTerminalFactroy::TransTerminalTypeToString(terminals_[i]->getTerminalType()));
				xml.AddAttrib(strTerminalIndex,i);
				xml.IntoElem();//enter strTerminalNode + i
				terminals_[i]->SaveXmlCfg(xml);
				xml.OutOfElem();//out strTerminalNode + i
			}
		}
		xml.OutOfElem();//out strTerminals
	}

	if (calc_yxDB_.getDataBaseSum() > 0)
	{
		xml.AddElem(strCalcYxDB);
		calc_yxDB_.SaveXmlCfg(xml);
	}

	if (getPristationNum() > 0)
	{
		xml.AddElem(strPriStations);
		xml.AddAttrib(strPriStationNum,getPristationNum());
		xml.AddAttrib(strIndexOrderEnable,strboolFalse);
		xml.IntoElem();//enter strPriStations
		for (size_t i=0;i<getPristationNum();i++)
		{
			if (pristations_[i])
			{
				xml.AddElem(strPriStationNode);
				xml.AddAttrib(strPriStationIndex,i);
				xml.IntoElem();//enter strPriStationNode + i
				pristations_[i]->SaveXmlCfg(xml);
				xml.OutOfElem();//out strPriStationNode + i
			}
		}
		xml.OutOfElem();//out strPriStations
	}

	xml.OutOfElem();//out strSubStation

	xml.Save(filename);//save xml file

}

//cmd recall api
void CSubStation::ProcessRecallCmd(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	if (!point)
	{
		return;
	}

	if (point->getCommPointType() != TERMINAL_NODE)
	{
		return;
	}

	share_terminal_ptr terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(point);

	if (!terminalPtr)
	{
		return;
	}

	switch (cmdType)
	{
	case Protocol::YK_SEL_CON:
		{
			try
			{
				int iVal = boost::any_cast<int>(val);
				int terminalIndex = getTerminalIndexByPtr(terminalPtr);
				if (terminalIndex >= 0)
				{
					int SubIndex = TerminalykToSubyk(terminalIndex,iVal);
					if (SubIndex >= 0 && SubIndex < (int)getYkSum())
					{
						if (ReturnCode == RETURN_CODE_ACTIVE)
						{
							AddYkRecord(yk_sel_con,SubIndex,(getYkType(SubIndex) == YkClose));
						}
						else
						{
							AddYkRecord(yk_sel_fail,SubIndex,(getYkType(SubIndex) == YkClose));
						}
						
						SubTemporarySig_(Protocol::YK_SEL_CON,ReturnCode,point,SubIndex);
					}
				}
			}
			catch(boost::bad_any_cast & e)
			{
				std::cerr<<"CSubStation::ProcessRecallCmd "<<e.what()<<std::endl;
			}
		}
		break;

	case Protocol::YK_EXE_CON:
		{
			try
			{
				int iVal = boost::any_cast<int>(val);
				int terminalIndex = getTerminalIndexByPtr(terminalPtr);
				if (terminalIndex >= 0)
				{
					int SubIndex = TerminalykToSubyk(terminalIndex,iVal);
					if (SubIndex >= 0 && SubIndex < (int)getYkSum())
					{
						if (ReturnCode == RETURN_CODE_ACTIVE)
						{
							AddYkRecord(yk_exe_con,SubIndex,(getYkType(SubIndex) == YkClose));
						}
						else
						{
							AddYkRecord(yk_exe_fail,SubIndex,(getYkType(SubIndex) == YkClose));
						}
						SubTemporarySig_(Protocol::YK_EXE_CON,ReturnCode,point,SubIndex);
					}
				}
			}
			catch(boost::bad_any_cast & e)
			{
				std::cerr<<"CSubStation::ProcessRecallCmd "<<e.what()<<std::endl;
			}
			catch(...)
			{
				std::cerr<<"CSubStation::ProcessRecallCmd undefine ex"<<std::endl;
			}
		}
		break;

	case Protocol::YK_CANCEL_CON:
		{
			try
			{
				int iVal = boost::any_cast<int>(val);
				int terminalIndex = getTerminalIndexByPtr(terminalPtr);
				if (terminalIndex >= 0)
				{
					int SubIndex = TerminalykToSubyk(terminalIndex,iVal);
					if (SubIndex >= 0 && SubIndex < (int)getYkSum())
					{
						if (ReturnCode == RETURN_CODE_ACTIVE)
						{
							AddYkRecord(yk_cancel_con,SubIndex,(getYkType(SubIndex) == YkClose));
						}
						else
						{
							AddYkRecord(yk_cancel_fail,SubIndex,(getYkType(SubIndex) == YkClose));
						}
						SubTemporarySig_(Protocol::YK_CANCEL_CON,ReturnCode,point,SubIndex);
					}
				}
			}
			catch(boost::bad_any_cast & e)
			{
				std::cerr<<"CSubStation::ProcessRecallCmd "<<e.what()<<std::endl;
			}
		}
		break;

	case Protocol::CALL_EQU_PARA_CON:
		{

			SubTemporarySig_(Protocol::CALL_EQU_PARA_CON,ReturnCode,point,val);

		}
		break;

	case Protocol::CALL_PROVAL_CON:
		{

			SubTemporarySig_(Protocol::CALL_PROVAL_CON,ReturnCode,point,val);

		}
		break;

	case Protocol::CALL_CHTYPE_CON:
		{

			SubTemporarySig_(Protocol::CALL_CHTYPE_CON,ReturnCode,point,val);

		}
		break;

	case Protocol::CALL_LINEPARA_CON:
		{

			SubTemporarySig_(Protocol::CALL_LINEPARA_CON,ReturnCode,point,val);

		}
		break;

	case Protocol::CALL_INTERFACE_PARA_CON:
		{

			SubTemporarySig_(Protocol::CALL_INTERFACE_PARA_CON,ReturnCode,point,val);

		}
		break;

	case Protocol::LINE_VAL_VER_QYC:
		{
			SubAliveSig_(Protocol::LINE_VAL_VER_QYC,ReturnCode,point,val);
		}
		break;

	case Protocol::LINE_DCVAL_VER_SUCESS:
		{
			SubAliveSig_(Protocol::LINE_DCVAL_VER_SUCESS,ReturnCode,point,val);
		}
		break;

	case Protocol::HARMONIC_CON:
		{
			SubAliveSig_(Protocol::HARMONIC_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::CALL_VALCOEF_CON:
		{
			SubAliveSig_(Protocol::CALL_VALCOEF_CON,ReturnCode,point,val);
		}
		break;


		//case Protocol::CALL_EQU_PARA_CON:
		//	{
		//		SubAliveSig_(Protocol::CALL_EQU_PARA_CON,ReturnCode,point,val);
		//	}
		//	break;

		//case Protocol::CALL_PROVAL_CON:
		//	{
		//		SubAliveSig_(Protocol::CALL_PROVAL_CON,ReturnCode,point,val);
		//	}
		//	break;
		//case Protocol::CALL_CHTYPE_CON:
		//	{
		//		SubAliveSig_(Protocol::CALL_CHTYPE_CON,ReturnCode,point,val);
		//	}
		//	break;
		//case Protocol::CALL_LINEPARA_CON:
		//	{
		//		SubAliveSig_(Protocol::CALL_LINEPARA_CON,ReturnCode,point,val);
		//	}
		//	break;

	case Protocol::SIGNAL_RESET_CON:
		{
			SubAliveSig_(Protocol::SIGNAL_RESET_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::BOARD_REQ_CON:
		{
			SubAliveSig_(Protocol::BOARD_REQ_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::DOWNLOAD_PARA_CON:
		{
			//			std::cout<<"sub开始与消息进行连接... .."<<std::endl;
			SubAliveSig_(Protocol::DOWNLOAD_PARA_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::BATTERY_ACTIVE_CON:
		{
			SubAliveSig_(Protocol::BATTERY_ACTIVE_CON,ReturnCode,point,val);
		}
		break;
	case Protocol::BATTERY_ACTIVE_OVER_CON:
		{
			SubAliveSig_(Protocol::BATTERY_ACTIVE_OVER_CON,ReturnCode,point,val);
		}
		break;
	case Protocol::LINE_BVAL_VER_QYC:
		{
			//std::cout<<"sub收到LINE_BVAL_VER_QYC"<<std::endl;
			SubAliveSig_(Protocol::LINE_BVAL_VER_QYC,ReturnCode,point,val);
		}
		break;
	case Protocol::LINE_BVAL_VER_SUCESS:
		{
			SubAliveSig_(Protocol::LINE_BVAL_VER_SUCESS,ReturnCode,point,val);
		}
		break;
	case Protocol::CALL_PM_ANG_CON:
		{
			SubAliveSig_(Protocol::CALL_PM_ANG_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::DSP_VERSION_INQ_CON:
		{
			SubAliveSig_(Protocol::DSP_VERSION_INQ_CON,ReturnCode,point,val);
		}
		break;

		//case Protocol::CALL_INTERFACE_PARA_CON:
		//	{
		//		SubAliveSig_(Protocol::CALL_INTERFACE_PARA_CON,ReturnCode,point,val);
		//	}
		//	break;

	case Protocol::EVENT_MESSAGE:
		{
			SubAliveSig_(Protocol::EVENT_MESSAGE,ReturnCode,point,val);
		}
		break;

	case Protocol::CALL_JB_PARA_CON:
		{
			SubAliveSig_(Protocol::CALL_JB_PARA_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::SEND_JB_PARA_CON:
		{
			SubAliveSig_(Protocol::SEND_JB_PARA_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::ACT_JB_PARA_CON:
		{
			SubAliveSig_(Protocol::ACT_JB_PARA_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::DEACT_JB_PARA_CON:
		{
			SubAliveSig_(Protocol::DEACT_JB_PARA_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::JB_SIGNAL_RESET_CON:
		{
			SubAliveSig_(Protocol::JB_SIGNAL_RESET_CON,ReturnCode,point,val);
		}
		break;

	case Protocol::COS_DATA_UP:
		SubAliveSig_(Protocol::COS_DATA_UP,ReturnCode,point,val);
		//AlgorithmSig_(Protocol::COS_DATA_UP,ReturnCode,point,val);
		break;

	case Protocol::SOE_DATA_UP:
		SubAliveSig_(Protocol::SOE_DATA_UP,ReturnCode,point,val);
		//AlgorithmSig_(Protocol::SOE_DATA_UP,ReturnCode,point,val);
		break;

	case Protocol::YCVAR_DATA_UP:
		SubAliveSig_(Protocol::YCVAR_DATA_UP,ReturnCode,point,val);
		break;

	case Protocol::TRANSMIT_FRAME:
		SubAliveSig_(Protocol::TRANSMIT_FRAME,ReturnCode,point,val);
		break;

	default:
		break;
	}

}

SigConnection CSubStation::ConnectSubAliveSig(CmdRecallSlotType slotVal)
{
	return SubAliveSig_.connect(slotVal);
}

SigConnection CSubStation::ConnectSubTempSig(CmdRecallSlotType slotVal)
{
	return SubTemporarySig_.connect(slotVal);
}

//boost::signals::connection CSubStation::ConnectAlgorithmSig(CmdRecallSlotType slotVal)
//{
//	return AlgorithmSig_.connect(slotVal);
//}

int CSubStation::EnableLoadSubCfgLog( std::string fileName,std::string filetype,std::string limit )
{	
	FileSystem::CLogFactory::CreateLog(fileName,filetype,limit);

	return 0;
}

int CSubStation::EnableInitCommLog( std::string fileName,std::string filetype,std::string limit )
{
	FileSystem::CLogFactory::CreateLog(fileName,filetype,limit);

	return 0;
}

int CSubStation::AddLoadSubCfgLog(std::string strVal)
{
	if (LoadSubCfgLog_)
	{
		return LoadSubCfgLog_->AddRecord(strVal);
	}

	return -1;
}

int CSubStation::AddLoadSubCfgWithSynT(std::string strVal)
{
	if (LoadSubCfgLog_)
	{
		return LoadSubCfgLog_->AddRecordWithSynT(strVal);
	}

	return -1;
}

int CSubStation::AddInitCommLog(std::string strVal)
{
	if (InitCommLog_)
	{
		return InitCommLog_->AddRecord(strVal);
	}

	return -1;
}

int CSubStation::AddInitCommLogWithSynT(std::string strVal)
{
	if (InitCommLog_)
	{
		return InitCommLog_->AddRecordWithSynT(strVal);
	}

	return -1;
}

boost::shared_ptr<PublicSupport::CSpaceRouteTab> CSubStation::getTerminalYxTabPtr(size_t index)
{
	if (index >= 0 && index < getTerminalNum())
	{
		return terminals_[index]->getYxTabPtr();
	}

	return boost::shared_ptr<PublicSupport::CSpaceRouteTab>();
}

boost::shared_ptr<PublicSupport::CSpaceRouteTab> CSubStation::getTerminalYcTabPtr(size_t index)
{
	if (index >= 0 && index < getTerminalNum())
	{
		return terminals_[index]->getYcTabPtr();
	}

	return boost::shared_ptr<PublicSupport::CSpaceRouteTab>();
}

boost::shared_ptr<PublicSupport::CSpaceRouteTab> CSubStation::getTerminalYkTabPtr(size_t index)
{
	if (index >= 0 && index < getTerminalNum())
	{
		return terminals_[index]->getYkTabPtr();
	}

	return boost::shared_ptr<PublicSupport::CSpaceRouteTab>();
}

boost::shared_ptr<PublicSupport::CSpaceRouteTab> CSubStation::getTerminalYmTabPtr(size_t index)
{
	if (index >= 0 && index < getTerminalNum())
	{
		return terminals_[index]->getYmTabPtr();
	}

	return boost::shared_ptr<PublicSupport::CSpaceRouteTab>();
}

boost::shared_ptr<PublicSupport::CEfficientRouteTab> CSubStation::getPriStationYxTabPtr(size_t index)
{
	if (index >= 0 && index < getPristationNum())
	{
		return pristations_[index]->getYxTabPtr();
	}

	return boost::shared_ptr<PublicSupport::CEfficientRouteTab>();
}

boost::shared_ptr<PublicSupport::CEfficientRouteTab> CSubStation::getPriStationYcTabPtr(size_t index)
{
	if (index >= 0 && index < getPristationNum())
	{
		return pristations_[index]->getYcTabPtr();
	}

	return boost::shared_ptr<PublicSupport::CEfficientRouteTab>();
}

boost::shared_ptr<PublicSupport::CEfficientRouteTab> CSubStation::getPriStationYkTabPtr(size_t index)
{
	if (index >= 0 && index < getPristationNum())
	{
		return pristations_[index]->getYkTabPtr();
	}

	return boost::shared_ptr<PublicSupport::CEfficientRouteTab>();
}

boost::shared_ptr<PublicSupport::CEfficientRouteTab> CSubStation::getPriStationYmTabPtr(size_t index)
{
	if (index >= 0 && index < getPristationNum())
	{
		return pristations_[index]->getYmTabPtr();
	}

	return boost::shared_ptr<PublicSupport::CEfficientRouteTab>();
}

int CSubStation::WriteSystemTime(unsigned short year,unsigned char month,unsigned char day,unsigned char hour,unsigned char minnutes,unsigned char seconds,unsigned short milliseconds)
{
	if (year > 2050 || year < 1900)
	{
		return -1;
	}

	//std::cout<<(short)year<<"-"<<(short)month<<"-"<<(short)day<<" "<<(short)hour<<":"<<(short)minnutes<<":"<<(short)seconds<<std::endl;

#if defined(_WIN32)

	SYSTEMTIME SysTtime;
	SysTtime.wYear = year;
	SysTtime.wMonth = month;
	SysTtime.wDay = day;
	SysTtime.wHour = hour;
	SysTtime.wMinute = minnutes;
	SysTtime.wSecond = seconds;
	SysTtime.wMilliseconds = milliseconds;

	if(SetLocalTime(&SysTtime))
	{
		return 0;
	}

#else

	tm TMtime;
	TMtime.tm_year = year - 1900;
	TMtime.tm_mon = month -1;
	TMtime.tm_mday = day;
	TMtime.tm_hour = hour;
	TMtime.tm_min = minnutes;
	TMtime.tm_sec = seconds;

	timeval TVtime;
	TVtime.tv_sec = mktime(&TMtime);
	TVtime.tv_usec = milliseconds * 1000;

	if (!settimeofday(&TVtime,NULL))
	{
		//std::cout<<TMtime.tm_year<<"-"<<TMtime.tm_mon<<"-"<<TMtime.tm_mday<<" "<<TMtime.tm_hour<<":"<<TMtime.tm_min<<":"<<TMtime.tm_sec<<std::endl;
		//system("hwclock -w");//设置硬件时钟，ZHANGZHIHUA

		return 0;
	}

#endif
	std::cerr << "Set System Time err!"<< std::endl;
	return -1;
}

void CSubStation::ResetDev(bool val)
{
	DelayResetDev(val);
}

void CSubStation::TrigCosEvent(size_t index,typeYxval val,bool bSingleType /* = true*/)
{
	int ret = -1;

	if (bSingleType)
	{
		ret = putCosPoint(index,val,single_yx_point);
	}
	else
	{
		ret = putCosPoint(index,val,double_yx_point);
	}

	if (!ret)
	{
		SubAliveSig_(Protocol::COS_DATA_UP,RETURN_CODE_ACTIVE,share_commpoint_ptr(),1);
	}
}

void CSubStation::TrigSoeEvent(size_t index,typeYxval val,boost::posix_time::ptime time,bool bSingleType /*= true*/)
{
	int ret = -1;

	if (bSingleType)
	{
		ret = putSoePoint(index,val,single_yx_point,time);
	}
	else
	{
		ret = putSoePoint(index,val,double_yx_point,time);
	}

	if (!ret)
	{
		SubAliveSig_(Protocol::SOE_DATA_UP,RETURN_CODE_ACTIVE,share_commpoint_ptr(),1);
	}
}

void CSubStation::TrigCosEvent(int num)
{
	SubAliveSig_(Protocol::COS_DATA_UP,RETURN_CODE_ACTIVE,share_commpoint_ptr(),num);
}

void CSubStation::TrigSoeEvent(int num)
{
	SubAliveSig_(Protocol::SOE_DATA_UP,RETURN_CODE_ACTIVE,share_commpoint_ptr(),num);
}

void CSubStation::TrigYcSendByTimeEvent(int num)
{
	SubAliveSig_(Protocol::YCI_SEND_BYTIME,RETURN_CODE_ACTIVE,share_commpoint_ptr(),num);
}

int CSubStation::AddYkRecord(unsigned char recordType,int yk_no,bool bCloseOrOpen)
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

	case yk_sel_con:
		NotifyEventStatus(YK_EVENT,RETURN_CODE_CMDSEND);
		break;

	case yk_exe_con:
		NotifyEventStatus(YK_EVENT,RETURN_CODE_CMDSEND);
		break;

	case yk_cancel_con:
		NotifyEventStatus(YK_EVENT,RETURN_CODE_CMDSEND);
		break;

	case yk_sel_fail:
		NotifyEventStatus(YK_EVENT,RETURN_CODE_NEGATIVE);
		break;

	case yk_exe_fail:
		NotifyEventStatus(YK_EVENT,RETURN_CODE_NEGATIVE);
		break;

	case yk_cancel_fail:
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

int CSubStation::AddCosRecord(const CCosPoint & cos)
{
	NotifyEventStatus(YX_EVENT,RETURN_CODE_ACTIVE);

	if (CosRecordPtr_)
	{
		return CosRecordPtr_->AddDataRecord(cos);
	}

	return -1;
}

int CSubStation::AddSoeRecord(const CSoePoint & soe)
{
	NotifyEventStatus(YX_EVENT,RETURN_CODE_CMDSEND);

	if (SoeRecordPtr_)
	{
		return SoeRecordPtr_->AddDataRecord(soe);
	}

	return -1;
}


void CSubStation::InitDefaultRecord()
{
	YkRecordPtr_.reset(new CYkRecord(DefaultYkRecordPath,DefaultYkRecordLimit));
	SoeRecordPtr_.reset(new CSoeRecord(DefaultSoeRecordPath,DefaultSoeRecordLimit));
	CosRecordPtr_.reset(new CCosRecord(DefaultYkRecordPath,DefaultCosRecordLimit));
}

//int CSubStation::setOriYxValDateUp(size_t index, typeYxval val)
//{
//	if (index < 0 || index >= getYxSum())
//	{
//		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
//	}
//
//	if (index < getTerminalYxSum())
//	{
//		size_t terminalIndex;
//		size_t terminalyxIndex;
//		if (!SubyxToTerminalyx(index,terminalIndex,terminalyxIndex))
//		{
//			int ret=terminals_[terminalIndex]->setOriYxVal(terminalyxIndex,val,true);//
//			SubAliveSig_(Protocol::COS_DATA_UP,RETURN_CODE_ACTIVE,share_commpoint_ptr(),1);
//			return ret;
//		}
//		else
//		{
//			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
//		}
//	}
//	else if(index < getTerminalYxSum() + getSubYxSum())
//	{
//		return (sub_yxDB_.getPointDataPtr(index - getTerminalYxSum()))->setOriYxVal(val);
//	}
//	else
//	{
//		return (calc_yxDB_.getPointDataPtr(index - getTerminalYxSum() - getSubYxSum()))->setOriYxVal(val);
//	}
//}

int CSubStation::ConnectEventStatusSig(CmdRecallSlotType slotVal)
{
	if (!EventStatusConnection_.connected())
	{
		EventStatusConnection_ = EventStatusSig_.connect(slotVal);
	}

	return 0;
}

void CSubStation::DisconnectEventStatusSig()
{
	EventStatusConnection_.disconnect();
}

int CSubStation::NotifyEventStatus(typeCmd EventType,unsigned char ReturnCode)
{
	if(EventStatusConnection_.connected())
	{
		int EventNO = -1;

		switch(EventType)
		{
		case YX_EVENT:
			EventNO = YxReisterNO_;
			break;

		case YK_EVENT:
			EventNO = YkRegisterNO_;
			break;

		case SELF_EVENT:
			EventNO = SelfRegisterNO_;
			break;

		default:
			break;
		}

		if(EventNO > 0)
		{
			EventStatusSig_(EventType,ReturnCode,share_commpoint_ptr(),EventNO);

			return 0;
		}
	}

	return -1;
}

void CSubStation::ProcessEventCmd(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	if (!CheckEventProcessReady())
	{
		return;
	}

	int index = -1;
	try
	{
		int index = boost::any_cast<int>(val);
	}
	catch(boost::bad_any_cast & e)
	{
		std::cerr<<"CSubStation::ProcessEventCmd "<<e.what()<<std::endl;

		return;
	}

	if (index < 0 || index >= light_->getLedSum())
	{
		return;
	}

	switch (cmdType)
	{
	case YK_EVENT:
		switch (ReturnCode)
		{
		case RETURN_CODE_CMDRECV: //遥控选择
			light_->WriteLed(index,true);
			break;

		case RETURN_CODE_ACTIVE: //遥控执行
			light_->BlingLed(index,1);
			light_->WriteLed(index,false);
			break;

		case RETURN_CODE_NEGATIVE: //遥控撤消或者遥控选择、执行、撤消失败
			light_->WriteLed(index,false);
			break;

		case RETURN_CODE_CMDSEND: //遥控选择、执行、撤消回应
			light_->BlingLed(index,1);
			break;

		default:
			break;
		}
		break;

	case YX_EVENT:
		switch (ReturnCode)
		{
		case RETURN_CODE_ACTIVE: //cos
			light_->BlingLed(index,1);
			break;

		case RETURN_CODE_CMDSEND: //soe
			light_->BlingLed(index,1);
			break;

		default:
			break;
		}
		break;

	case COMM_EVENT:
		switch (ReturnCode)
		{
		case RETURN_CODE_ACTIVE: //通讯建立
			light_->WriteLed(index,true);
			break;

		case RETURN_CODE_NEGATIVE: //通讯关闭
			light_->WriteLed(index,false);
			break;

		case RETURN_CODE_CMDSEND: //发送报文
			light_->BlingLed(index,1);
			break;

		case RETURN_CODE_CMDRECV: //接收报文
			light_->BlingLed(index,1);
			break;

		default:
			break;
		}
		break;

	case SELF_EVENT:
		light_->BlingLed(index,1);
		break;

	default:
		break;
	}
}

int CSubStation::CallData(int terminalIndex)
{
	if (terminalIndex < 0 || terminalIndex > (int)getTerminalNum())
	{
		return -1;
	}

	return terminals_[terminalIndex]->CallData();
}

int CSubStation::SynTime(int terminalIndex)
{
	if (terminalIndex < 0 || terminalIndex > (int)getTerminalNum())
	{
		return -1;
	}

	return terminals_[terminalIndex]->SynTime();
}

//frame api
int CSubStation::CallAllData()
{
	if(bCallDataImmediately_)
	{
		for (int i=0;i<getTerminalNum();i++)
		{
			terminals_[i]->CallData();
		}

		return 0;
	}

	return -1;
}

int CSubStation::SynAllTime()
{
	if(bSynTimeImmediately_)
	{
		for (int i=0;i<getTerminalNum();i++)
		{
			terminals_[i]->SynTime();
		}

		return 0;
	}

	return -1;
}

int CSubStation::AddGeneralCmd(int tIndex,Protocol::CCmd cmdVal)
{
	int ret = -1;

	share_bf533_ptr bf533 = getBf533Terminal();

	for (int i=0;i<getTerminalNum();i++)
	{
		if(bAddGeneralCmd_)
		{
			terminals_[i]->AddGeneralCmd(cmdVal);
		}
	}

	return ret;
}

int CSubStation::AddSpeCmd(int tIndex,Protocol::CCmd cmdVal)
{
	int ret = -1;

	share_bf533_ptr bf533 = getBf533Terminal();

	for (int i=0;i<getTerminalNum();i++)
	{
		if(bAddSpeCmd_)
		{
			terminals_[i]->AddGeneralCmd(cmdVal);
		}
	}

	return ret;
}

};//namespace DataBase



