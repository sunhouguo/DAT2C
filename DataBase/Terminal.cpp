#include <iostream>
#include <boost/bind.hpp>
#include "Terminal.h"
#include "SubStation.h"
//#include "CommInterface.h"
#include "CalcYxPoint.h"
#include "YxPoint.h"
#include "YCPoint.h"
#include "YkPoint.h"
#include "YmPoint.h"
#include "Line.h"
#include "FaultPoint.h"
#include "../Protocol/CmdQueue.h"
#include "../PublicSupport/SpaceRouteTab.h"
#include "SoeRecord.h"
#include "CosRecord.h"
#include "FaultRecord.h"
#include "YcHisRecord.h"
#include "YcStatRecord.h"
#include "YkRecord.h"
#include "FaultRecord.h"

namespace DataBase {

#define strTerminalAddr "Address"
#define strActiveCommYx "ActiveCommYx"
#define strActiveCommQuality  "ActiveCommQuality"
#define strEncryptOutSide "Encrypt"
#define strYxDB "YxDB"
#define strYcDB "YcDB"
#define strYmDB "YmDB"
#define strYkDB "YkDB"
#define strYxTab "YxRouteTab"
#define strYcTab "YcRouteTab"
#define strYmTab "YmRouteTab"
#define strYkTab "YkRouteTab"
#define strUseTabIndex "UseTabIndex"

/*
CTerminal::CTerminal()
{
	addr_ = 0xffff;
}
*/

CTerminal::CTerminal(boost::asio::io_service & io_service,
					 CSubStation & sub)
					:CCommPoint(io_service),
					sub_(sub)
{
	pointType_ = TERMINAL_NODE;
	terminalType_ = NormalTerminal;
	YcHisCycleTime_ = DefaultYcCycleTime;

	addr_ = 0xffff;
	bActiveCommYX_ = true;
	bActiveCommQuality_ = true;
	bActiveInitQuality_ = false;
	bCommActiveBackup_ = bCommActive_;

	bSynTimeImmediately_ = true;
	bCallDataImmediately_ = true;
	bAddGeneralCmd_ = true;
	bAddSpeCmd_ =false;

	UseYxTabIndex_ = -1;
	UseYcTabIndex_ = -1;
	UseYmTabIndex_ = -1;
	UseYkTabIndex_ = -1;
}

CTerminal::~CTerminal(void)
{
}

int CTerminal::InitTerminal(FileSystem::CMarkup & xml)
{
	LoadXmlCfg(xml);

	return 0;
}

void CTerminal::UnInitTerminal()
{
}

//yx api
size_t CTerminal::getYxSum()
{
	int YxSpace = 0;
	if (getActiveCommYx())
	{
		YxSpace = 1;
	}

	return getSrcYxSum() + YxSpace; 
}

size_t CTerminal::getSrcYxSum()
{
	return yxDB_.getDataBaseSum();
}

int CTerminal::setYxSum(size_t val)
{
	return yxDB_.setDataBaseSum(val);
}

size_t CTerminal::getRecvYxSum()
{
	if (yxTab_)
	{
		return yxTab_->getSrcPointNum();
	}
	else
	{
		return getSrcYxSum();
	}
}

typeYxval CTerminal::getOriYxVal(size_t index)
{
	if (getActiveCommYx() && (index == getSrcYxSum()))
	{
		typeYxval yx_val = 0;

		if(getCommActive())
		{
			yx_val = 1;
		}
		else
		{
			yx_val = 0;
		}

		return yx_val;
	}
	
	return (yxDB_.getPointDataPtr(index))->getOriYxVal();
}

int CTerminal::setOriYxVal(size_t index, typeYxval val,bool bCheckYxVar/* = false*/)
{
	if (getActiveCommYx() &&(index == getSrcYxSum()))
	{
		return 0;
	}

	if (bCheckYxVar)
	{
		typeYxval oldVal = yxDB_.getPointDataPtr(index)->getOriYxVal();
		int ret = (yxDB_.getPointDataPtr(index))->setOriYxVal(val);
		if(!ret)
		{
			if (oldVal != val)
			{
				putCosPoint(index,val,yxDB_.getPointDataPtr(index)->getYxType(),yxDB_.getPointDataPtr(index)->getYxQuality());
				//putSoePoint(index,val,yxDB_.getPointData(index)->getYxType(),boost::posix_time::microsec_clock::local_time(),yxDB_.getPointData(index)->getYxQuality());
				return CauseActiveData;
			}
		}

		return ret;
	}
	else
	{
		return (yxDB_.getPointDataPtr(index))->setOriYxVal(val);
	}
}

int CTerminal::SaveOriYxVal(size_t index,typeYxval val,bool bCheckYxVar/* = false*/)
{
	int dstIndex = SrcyxToDstyx(index);
	if (dstIndex < 0)
	{
		return dstIndex;
	}

	return setOriYxVal(dstIndex,val,bCheckYxVar);
}

typeYxtype CTerminal::getYxType(size_t index)
{
	if (getActiveCommYx() && (index == getSrcYxSum()))
	{
		return single_yx_point;
	}

	return (yxDB_.getPointDataPtr(index))->getYxType();
}

int CTerminal::setYxType(size_t index,typeYxtype val)
{
	if (getActiveCommYx() && (index == getSrcYxSum()))
	{
		return 0;
	}

	return (yxDB_.getPointDataPtr(index))->setYxType(val);
}

int CTerminal::SaveYxType(size_t index,typeYxtype val)
{
	int dstIndex = SrcyxToDstyx(index);
	if (dstIndex < 0)
	{
		return dstIndex;
	}

	return setYxType(dstIndex,val);
}

bool CTerminal::getYxPolar(size_t index)
{
	if (getActiveCommYx() && (index == getSrcYxSum()))
	{
		return true;
	}

	return (yxDB_.getPointDataPtr(index))->getYxPolar();
}

int CTerminal::setYxPolar(size_t index,bool val)
{
	if (getActiveCommYx() && (index == getSrcYxSum()))
	{
		return 0;
	}

	return (yxDB_.getPointDataPtr(index))->setYxPolar(val);
}

typeYxQuality CTerminal::getYxQuality(size_t index)
{
	if (getActiveCommYx() && (index == getSrcYxSum()))
	{
		return CYxPoint::QualityActive;
	}

	if (getActiveCommQuality() && (!getCommActive()))
	{
		return CYxPoint::QualityNegative;
	}

	if (getActiveInitQuality() && (!getInitCommPointFlag()))
	{
		return CYxPoint::QualityNegative;
	}
	
	return (yxDB_.getPointDataPtr(index))->getYxQuality();
	
}

int CTerminal::setYxQuality(size_t index,typeYxQuality val)
{
	if (getActiveCommYx() && (index == getSrcYxSum()))
	{
		return 0;
	}

	return (yxDB_.getPointDataPtr(index))->setYxQuality(val);
}

int CTerminal::SaveYxQuality(size_t index,typeYxQuality val)
{
	int dstIndex = SrcyxToDstyx(index);
	if (dstIndex < 0)
	{
		return dstIndex;
	}

	return setYxQuality(dstIndex,val);
}

typeFinalYcval CTerminal::getFinalYxVal(size_t index)
{
	if (getActiveCommYx() && (index == getSrcYxSum()))
	{
		typeYxval yx_val = 0;

		if(getCommActive())
		{
			yx_val = 1;
		}
		else
		{
			yx_val = 0;
		}

		return yx_val;
	}

	return (yxDB_.getPointDataPtr(index))->getFinalYxVal();
}

bool CTerminal::getActiveCommYx()
{
	return bActiveCommYX_;
}

void CTerminal::setActiveCommYx(bool val)
{
	bActiveCommYX_ = val;
}

bool CTerminal::getActiveCommQuality()
{
	return bActiveCommQuality_;
}

int CTerminal::setActiveCommQuality(bool val)
{
	bActiveCommQuality_ = val;

	return 0;
}

bool CTerminal::getActiveInitQuality()
{
	return bActiveInitQuality_;
}

int CTerminal::setActiveInitQuality(bool val)
{
	bActiveInitQuality_ = val;

	return 0;
}

int CTerminal::AddEffectYxIndex(size_t index,size_t val)
{
	if (getActiveCommYx())
	{
		if (index == getSrcYxSum())
		{
			return 0;
		}
	}

	return (yxDB_.getPointDataPtr(index))->AddEffectYxIndex(val);
}

int CTerminal::getEffectYxSum(size_t index)
{
	if (getActiveCommYx())
	{
		if (index == getSrcYxSum())
		{
			return 0;
		}
	}

	return (yxDB_.getPointDataPtr(index))->getEffectYxSum();
}

int CTerminal::getEffectYxIndex(size_t index,int val)
{
	if (getActiveCommYx())
	{
		if (index == getSrcYxSum())
		{
			return -1;
		}
	}

	return (yxDB_.getPointDataPtr(index))->getEffectYxIndex(val);
}

//yc api
size_t CTerminal::getYcSum()
{
	return ycDB_.getDataBaseSum();
}

/*
size_t CTerminal::getSrcYcSum()
{
	if (ycTab_)
	{
		return ycTab_->getSrcPointNum();
	}
	else
	{
		return getYcSum();
	}
}
*/

size_t CTerminal::getRecvYcSum()
{
	if (ycTab_)
	{
		return ycTab_->getSrcPointNum();
	}
	else
	{
		return getYcSum();
	}
}

int CTerminal::setYcSum(size_t val)
{
	return ycDB_.setDataBaseSum(val);
}

typeYcval CTerminal::getOriYcVal(size_t index)
{
	return (ycDB_.getPointDataPtr(index))->getOriYcVal();
}

int CTerminal::setOriYcVal(size_t index,typeYcval val,bool bCheckYcVar /* = false */)
{
	if (bCheckYcVar)
	{
		//typeYcval oldVal = ycDB_.getPointDataPtr(index)->getOriYcVal();
		typeYcval oldVal = ycDB_.getPointDataPtr(index)->getBackupYcVal();
		int ret = (ycDB_.getPointDataPtr(index))->setOriYcVal(val);
		if(!ret)
		{
			int limit = (ycDB_.getPointDataPtr(index))->getYcDeadLimit();
			if ((abs(oldVal - val) >= (ycDB_.getPointDataPtr(index))->getYcDeadLimit()) || (val == 0 && oldVal != 0))
			{
				putYcvarPoint(index,getOriYcVal(index),getYcQuality(index),boost::posix_time::microsec_clock::local_time());
				return CauseActiveData;
			}
		}

		return ret;
	}
	else
	{
		return (ycDB_.getPointDataPtr(index))->setOriYcVal(val);
	}
}

int CTerminal::SaveOriYcVal(size_t index,typeYcval val,bool bCheckYcVar /* = false */)
{
	int dstIndex = SrcycToDstyc(index);
	if (dstIndex < 0)
	{
		return dstIndex;
	}

	return setOriYcVal(dstIndex,val,bCheckYcVar);
}

typeYcplus CTerminal::getYcPlus(size_t index)
{
	return (ycDB_.getPointDataPtr(index))->getYcPlus();
}

int CTerminal::setYcPlus(size_t index,typeYcplus val)
{
	return (ycDB_.getPointDataPtr(index))->setYcPlus(val);
}

typeYcmul CTerminal::getYcMul(size_t index)
{
	return (ycDB_.getPointDataPtr(index))->getYcMul();
}

int CTerminal::setYcMul(size_t index,typeYcmul val)
{
	return (ycDB_.getPointDataPtr(index))->setYcMul(val);
}

typeYcquality CTerminal::getYcQuality(size_t index)
{
	if (getActiveCommQuality() && (!getCommActive()))
	{
		return CYcPoint::QualityNegative;
	}

	if (getActiveInitQuality() && (!getInitCommPointFlag()))
	{
		return CYxPoint::QualityNegative;
	}
	
	return (ycDB_.getPointDataPtr(index))->getYcQuality();	
}

int CTerminal::setYcQuality(size_t index,typeYcquality val)
{
	return (ycDB_.getPointDataPtr(index))->setYcQuality(val);
}

int CTerminal::SaveYcQuality(size_t index,typeYcquality val)
{
	int dstIndex = SrcycToDstyc(index);
	if (dstIndex < 0)
	{
		return dstIndex;
	}

	return setYcQuality(dstIndex,val);
}

typeYcdead CTerminal::getYcDeadLimit(size_t index)
{
	return (ycDB_.getPointDataPtr(index))->getYcDeadLimit();
}

int CTerminal::setYcDeadLimit(size_t index,typeYcdead val)
{
	return (ycDB_.getPointDataPtr(index))->setYcDeadLimit(val);
}

typeFinalYcval CTerminal::getFinalYcVal(size_t index,bool bUseMul /* = false */)
{
	return (ycDB_.getPointDataPtr(index))->getFinalYcVal(bUseMul);
}

CYcPoint * CTerminal::getYcPointPtr(int index)
{
	return ycDB_.getPointDataPtr(index);
}

CYcPoint * CTerminal::loadYcPointPtr(int index)
{
	int dstIndex = SrcycToDstyc(index);
	if (dstIndex < 0)
	{
		return NULL;
	}

	return getYcPointPtr(dstIndex);
}

//yk api
size_t CTerminal::getYkSum()
{
	return ykDB_.getDataBaseSum();
}

/*
size_t CTerminal::getSrcYkSum()
{
	if (ykTab_)
	{
		return ykTab_->getSrcPointNum();
	}
	else
	{
		return getYkSum();
	}
}
*/

size_t CTerminal::getRecvYkSum()
{
	if (ykTab_)
	{
		return ykTab_->getSrcPointNum();
	}
	else
	{
		return getYkSum();
	}
}

int CTerminal::setYkSum(size_t val)
{
	return ykDB_.setDataBaseSum(val);
}

//typeYkstatus CTerminal::getYkStatus(size_t index)
//{
//	return (ykDB_.getPointDataPtr(index))->getYkStatus();
//}
//
//int CTerminal::setYkStatus(size_t index,typeYkstatus val)
//{
//	return (ykDB_.getPointDataPtr(index))->setYkStatus(val);
//}
//
//int CTerminal::saveYkStatus(size_t index,typeYkstatus val)
//{
//	int dstIndex = SrcykToDstyk(index);
//	if (dstIndex <  0)
//	{
//		return dstIndex;
//	}
//
//	return setYkStatus(dstIndex,val);
//}

CYkPoint * CTerminal::getYkPointPtr(int index)
{
	return ykDB_.getPointDataPtr(index);
}

CYkPoint * CTerminal::loadYkPointPtr(int index)
{
	int dstIndex = SrcykToDstyk(index);
	if (dstIndex < 0)
	{
		return NULL;
	}

	return getYkPointPtr(dstIndex);
}

typeYktype CTerminal::getYkType(size_t index)
{
	return (ykDB_.getPointDataPtr(index))->getYkType();
}

int CTerminal::setYkType( size_t index,typeYktype val )
{
	return (ykDB_.getPointDataPtr(index))->setYkType(val);
}

int CTerminal::saveYkType(size_t index,typeYktype val)
{
	int dstIndex = SrcykToDstyk(index);
	if (dstIndex < 0)
	{
		return dstIndex;
	}

	return setYkType(dstIndex,val);
}

bool CTerminal::getbHYkDouble(size_t index)
{
	return (ykDB_.getPointDataPtr(index))->getbHYkDouble();
}

int CTerminal::setbHYkDouble(size_t index, bool val)
{
	return (ykDB_.getPointDataPtr(index))->setbHYkDouble(val);
}
bool CTerminal::getbSYkDouble(size_t index)
{
	return (ykDB_.getPointDataPtr(index))->getbSYkDouble();
}

int CTerminal::setbSYkDouble(size_t index, bool val)
{
	return (ykDB_.getPointDataPtr(index))->setbSYkDouble(val);
}

int CTerminal::AddYkSelCmd( int index,bool bCloseOrOpen )
{
	if (index < 0 || index >= (int)getYkSum())
	{
		AddYkRecord(yk_sel_fail,index,bCloseOrOpen);
		return 1;
	}

	if (bCloseOrOpen)
	{
		setYkType(index,YkClose);
	}
	else
	{
		setYkType(index,YkOpen);
	}

	if (!getCommActive())
	{
		AddYkRecord(yk_sel_fail,index,bCloseOrOpen);
		return -1;
	}

	//if (!CYkPoint::CheckYkStatusDevelopWithSel(getYkStatus(index),YkSelSend))
	//{
	//	return -2;
	//}

	Protocol::CCmd cmdVal(Protocol::YK_SEL_ACT,Protocol::YK_SEL_ACT_PRIORITY,getSelfPtr(),index);

	AddYkRecord(yk_sel,index,bCloseOrOpen);
	return AddCmdVal(cmdVal);
}

int CTerminal::AddYkExeCmd( int index,bool bCloseOrOpen )
{
	if (index < 0 || index >= (int)getYkSum())
	{
		AddYkRecord(yk_exe_fail,index,bCloseOrOpen);
		return 1;
	}

	if (!getCommActive())
	{
		AddYkRecord(yk_exe_fail,index,bCloseOrOpen);
		return -1;
	}

	//if (!CYkPoint::CheckYkStatusDevelopNoSel(getYkStatus(index),YkExeSend))
	//{
	//	return -2;
	//}

	//if (bCloseOrOpen)
	//{
	//	setYkType(index,YkClose);
	//}
	//else
	//{
	//	setYkType(index,YkOpen);
	//}

	Protocol::CCmd cmdVal(Protocol::YK_EXE_ACT,Protocol::YK_EXE_ACT_PRIORITY,getSelfPtr(),index);

	AddYkRecord(yk_exe,index,bCloseOrOpen);
	return AddCmdVal(cmdVal);
}

int CTerminal::AddYkCancelCmd( int index,bool bCloseOrOpen /*= true*/ )
{
	if (index < 0 || index >= (int)getYkSum())
	{
		AddYkRecord(yk_cancel_fail,index,bCloseOrOpen);
		return 1;
	}

	//if (bCloseOrOpen)
	//{
	//	setYkType(index,YkClose);
	//}
	//else
	//{
	//	setYkType(index,YkOpen);
	//}

	if (!getCommActive())
	{
		AddYkRecord(yk_cancel_fail,index,bCloseOrOpen);
		return -1;
	}
	
	//if (!CYkPoint::CheckYkStatusDevelopNoSel(getYkStatus(index),YkCancelSend))
	//{
	//	return -2;
	//}

	Protocol::CCmd cmdVal(Protocol::YK_CANCEL_ACT,Protocol::YK_CANCEL_ACT_PRIORITY,getSelfPtr(),index);

	AddYkRecord(yk_cancel,index,bCloseOrOpen);
	return AddCmdVal(cmdVal);
}

//ym api
size_t CTerminal::getYmSum()
{
	return ymDB_.getDataBaseSum();
}

/*
size_t CTerminal::getSrcYmSum()
{
	if (ymTab_)
	{
		return ymTab_->getSrcPointNum();
	}
	else
	{
		return getYmSum();
	}
}
*/

size_t CTerminal::getRecvYmSum()
{
	if (ymTab_)
	{
		return ymTab_->getSrcPointNum();
	}
	else
	{
		return getYmSum();
	}
}

int CTerminal::setYmSum(size_t val)
{
	return ymDB_.setDataBaseSum(val);
}

typeYmval CTerminal::getOriYmVal(size_t index)
{
	return (ymDB_.getPointDataPtr(index))->getOriYmVal();
}

int CTerminal::setOriYmVal(size_t index,typeYmval val)
{
	return (ymDB_.getPointDataPtr(index))->setOriYmVal(val);
}

int CTerminal::SaveOriYmVal(size_t index,typeYmval val)
{
	int dstIndex = SrcymToDstym(index);
	if (dstIndex < 0)
	{
		return dstIndex;
	}

	return setOriYmVal(dstIndex,val);
}

typeYmquality CTerminal::getYmQuality(size_t index)
{
	return (ymDB_.getPointDataPtr(index))->getYmQuality();
}

int CTerminal::setYmQuality(size_t index,typeYmquality val)
{
	return (ymDB_.getPointDataPtr(index))->setYmQuality(val);
}

int CTerminal::SaveYmQuality(size_t index,typeYmquality val)
{
	int dstIndex = SrcymToDstym(index);
	if (dstIndex < 0)
	{
		return dstIndex;
	}

	return setYmQuality(dstIndex,val);
}


//cos api
int CTerminal::putCosPoint(size_t yxIndex,typeYxval val,typeYxtype yxType,typeYxQuality yxQuality /* = 0 */)
{
	int index = sub_.getTerminalIndexByPtr(shared_from_this());
	if (index < 0)
	{
		return index;
	}

	int ret = sub_.putCosPoint(index,yxIndex,val,yxType,yxQuality);

	CCosPoint cos(yxIndex,val,yxType,yxQuality);
	AddCosRecord(cos);

	return ret;
}

int CTerminal::SaveCosPoint(size_t yxIndex, typeYxval val,typeYxtype yxType,typeYxQuality yxQuality /* = 0 */)
{
	int dstyxIndex = SrcyxToDstyx(yxIndex);
	if (dstyxIndex < 0)
	{
		return dstyxIndex;
	}

	return putCosPoint(dstyxIndex,val,yxType,yxQuality);
}

//soe api
int CTerminal::putSoePoint(size_t yxIndex,typeYxval val,typeYxtype yxType,ptime time,typeYxQuality yxQuality /* = 0 */)
{
	int index = sub_.getTerminalIndexByPtr(shared_from_this());
	if (index < 0)
	{
		return index;
	}

	
	int ret = sub_.putSoePoint(index,yxIndex,val,yxType,time,yxQuality);

	CSoePoint soe(yxIndex,val,yxType,yxQuality,time);
	AddSoeRecord(soe);

	return ret;
}

int CTerminal::SaveSoePoint(size_t yxIndex, typeYxval val,typeYxtype yxType,ptime time,typeYxQuality yxQuality /* = 0 */)
{
	int dstyxIndex = SrcyxToDstyx(yxIndex);
	if (dstyxIndex < 0)
	{
		return dstyxIndex;
	}

	return putSoePoint(dstyxIndex,val,yxType,time,yxQuality);
}

//ycvar api
int CTerminal::putYcvarPoint(size_t ycIndex,typeYcval val,typeYcquality ycQuality /* = 0 */,ptime time /* = ptime() */)
{
	int index = sub_.getTerminalIndexByPtr(shared_from_this());
	if (index < 0)
	{
		return index;
	}

	return sub_.putYcvarPoint(index,ycIndex,val,ycQuality,time);
}

int CTerminal::SaveYcvarPoint(size_t ycIndex, typeYcval val,typeYcquality ycQuality /* = 0 */, ptime time /* = ptime() */)
{
	int dstycIndex = SrcycToDstyc(ycIndex);
	if (dstycIndex < 0)
	{
		return dstycIndex;
	}

	return putYcvarPoint(dstycIndex,val,ycQuality,time);
}

//fault api
int CTerminal::putFaultPoint(size_t faultIndex,typeYcval val,ptime time)
{
	CFaultPoint fault;

	fault.setFaultNO(faultIndex);
	fault.setFaultVal(val);
	fault.setFaultTime(time);

	AddFaultRecord(fault);

	return 0;
}

int CTerminal::SaveFaultPoint(size_t faultIndex,typeYcval val,ptime time)
{
	return putFaultPoint(faultIndex,val,time);
}

//index transform
int CTerminal::DstyxToSrcyx(size_t val)
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

int CTerminal::SrcyxToDstyx(size_t val)
{
	if (val < 0)
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

int CTerminal::DstycToSrcyc(size_t val)
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

int CTerminal::SrcycToDstyc(size_t val)
{
	if (val < 0 )
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

int CTerminal::DstykToSrcyk(size_t val)
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

int CTerminal::SrcykToDstyk(size_t val)
{
	if (val < 0)
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

int CTerminal::DstymToSrcym(size_t val)
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

int CTerminal::SrcymToDstym(size_t val)
{
	if (val < 0)
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

//虚函数接口
share_commpoint_ptr CTerminal::getSelfPtr()
{
	return shared_from_this();
}

/*
void CTerminal::AddSelfPointToProtocol()
{
	if (commPtr_)
	{
		commPtr_->AddCommPoint(getSelfPtr());
	}

	//ConnectCmdRecallSig();
}

void CTerminal::AddSelfPointToTcpServerPtr()
{
	if (serverPtr_)
	{
		serverPtr_->AddCommPoint(getSelfPtr());
	}
}

int CTerminal::ResetTimerRecv(size_t LostRecvTimeOut)
{
	timerRecv_.expires_from_now(boost::posix_time::seconds(LostRecvTimeOut));
	timerRecv_.async_wait(boost::bind(&CCommPoint::handle_timerRecv,getSelfPtr(),boost::asio::placeholders::error));

	return 0;
}

void CTerminal::handle_timerRecv(const boost::system::error_code& error)
{
	if (!error)
	{
		setCommActive(false);
	}
}

int CTerminal::ConnectCmdRecallSig()
{
	if (commPtr_)
	{
		return commPtr_->ConnectCmdRecallSig(boost::bind(&CCommPoint::ProcessRelayCmd,getSelfPtr(),_1,_2,_3,_4));
	}

	return -1;
}
*/

SigConnection CTerminal::ConnectSubTempSig(CmdRecallSlotType slotVal)
{
	return sub_.ConnectSubTempSig(slotVal);
}

SigConnection CTerminal::ConnectSubAliveSig(CmdRecallSlotType slotVal)
{
	return sub_.ConnectSubAliveSig(slotVal);
}

void CTerminal::ClearDataBaseQuality(bool active)
{
	typeYxQuality yxQuality;
	typeYcquality ycQuality;

	if (active)
	{
		yxQuality = CYxPoint::QualityActive;
		ycQuality = CYcPoint::QualityActive;
	}
	else
	{
		yxQuality = CYxPoint::QualityNegative;
		ycQuality = CYcPoint::QualityNegative;
	}

	for (int i=0;i<(int)yxDB_.getDataBaseSum();i++)
	{
		yxDB_.getPointDataPtr(i)->setYxQuality(yxQuality);
	}

	for (int i=0;i<(int)ycDB_.getDataBaseSum();i++)
	{
		ycDB_.getPointDataPtr(i)->setYcQuality(ycQuality);
	}
}

int CTerminal::InitLocalServices( CmdRecallSlotType slotVal,bool SlotReady )
{
	CCommPoint::InitLocalServices(slotVal,SlotReady);

	if (YcHisRecordPtr_)
	{
		ResetTimerYcHisCycle(true);
	}

	if (YcStatRecordPtr_)
	{
		ResetTimerYcStatCycle(true);
	}

	return 0;
}

void CTerminal::UnInitLocalServices()
{
	CCommPoint::UnInitLocalServices();

	ResetTimerYcHisCycle(false);
	ResetTimerYcStatCycle(false);
}

//xml cfg api
int CTerminal::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	try
	{
		CCommPoint::LoadXmlCfg(xml);
	}
	catch(PublicSupport::dat2def_exception & e)
	{
		e<<errinfo_middletype_name("通讯参数出错");
		throw e;
	}

	xml.ResetMainPos();
	if (xml.FindElem(strTerminalAddr))
	{
		unsigned short iaddr;
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			iaddr = boost::lexical_cast<unsigned short>(strTmp);
			setAddr(iaddr);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的终端地址参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("未找到终端地址配置项");
	}

	xml.ResetMainPos();
	if (xml.FindElem(strActiveCommYx))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setActiveCommYx(TransActiveCommFromString(strTmp));
	}

	xml.ResetMainPos();
	if (xml.FindElem(strActiveCommQuality))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if(boost::iequals(strboolTrue,strTmp))
		{
			setActiveCommQuality(true);
		}
		else
		{
			setActiveCommQuality(false);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strEncryptOutSide))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if(boost::iequals(strboolTrue,strTmp))
		{
			setEncryptOutside(true);
		}
		else
		{
			setEncryptOutside(false);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYxDB))
	{
		try
		{
			yxDB_.InitDataBase(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			e<<errinfo_middletype_name(strYxDB);
			throw e;
		}

	}

	xml.ResetMainPos();
	if (xml.FindElem(strYcDB))
	{
		try
		{
			ycDB_.InitDataBase(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			e<<errinfo_middletype_name(strYcDB);
			throw e;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYmDB))
	{
		try
		{
			ymDB_.InitDataBase(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			e<<errinfo_middletype_name(strYmDB);
			throw e;
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYkDB))
	{
		try
		{
			ykDB_.InitDataBase(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			e<<errinfo_middletype_name(strYkDB);
			throw e;
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
					std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
					UseYxTabIndex_ = -1;
				}

				if (UseYxTabIndex_ >= 0 && UseYxTabIndex_ < (int)sub_.getTerminalNum())
				{
					yxTab_.reset();
					yxTab_ = sub_.getTerminalYxTabPtr(UseYxTabIndex_);
				}
			}

			if (UseYxTabIndex_ < 0)
			{
				yxTab_.reset(new PublicSupport::CSpaceRouteTab());
				yxTab_->InintRouteTab(xml,getYxSum());
			}

		}
		catch (PublicSupport::dat2def_exception & e)
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
					std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
					UseYcTabIndex_ = -1;
				}

				if (UseYcTabIndex_ >= 0 && UseYcTabIndex_ < (int)sub_.getTerminalNum())
				{
					ycTab_.reset();
					ycTab_ = sub_.getTerminalYcTabPtr(UseYcTabIndex_);
				}
			}

			if (UseYcTabIndex_ < 0)
			{
				ycTab_.reset(new PublicSupport::CSpaceRouteTab());
				ycTab_->InintRouteTab(xml,getYcSum());
			}
		}
		catch (PublicSupport::dat2def_exception & e)
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
					std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
					UseYmTabIndex_ = -1;
				}

				if (UseYmTabIndex_ >= 0 && UseYmTabIndex_ < (int)sub_.getTerminalNum())
				{
					ymTab_.reset();
					ymTab_ = sub_.getTerminalYmTabPtr(UseYmTabIndex_);
				}
			}

			if (UseYmTabIndex_ < 0)
			{
				ymTab_.reset(new PublicSupport::CSpaceRouteTab());
				ymTab_->InintRouteTab(xml,getYmSum());
			}
		}
		catch (PublicSupport::dat2def_exception & e)
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
					std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
					UseYkTabIndex_ = -1;
				}

				if (UseYkTabIndex_ >= 0 && UseYkTabIndex_ < (int)sub_.getTerminalNum())
				{
					ykTab_.reset();
					ykTab_ = sub_.getTerminalYkTabPtr(UseYkTabIndex_);
				}
			}

			if (UseYkTabIndex_ < 0)
			{
				ykTab_.reset(new PublicSupport::CSpaceRouteTab());
				ykTab_->InintRouteTab(xml,getYkSum());
			}
		}
		catch (PublicSupport::dat2def_exception & e)
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
				std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
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
	if (xml.FindElem(strYcHisRecord))
	{
		std::string strTmp = xml.GetAttrib(strLimit);
		boost::algorithm::trim(strTmp);
		int limit = 0;
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

		strTmp = xml.GetAttrib(strYcCycleTime);
		boost::algorithm::trim(strTmp);
		if (!strTmp.empty())
		{
			try
			{
				YcHisCycleTime_ = boost::lexical_cast<int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
				YcHisCycleTime_ = DefaultYcCycleTime;
			}
		}

		std::string strID = xml.GetData();
		boost::algorithm::trim(strID);

		if (limit > 0)
		{
			YcHisRecordPtr_.reset(new CYcHisRecord(strID,limit));
		}
		else
		{
			YcHisRecordPtr_.reset(new CYcHisRecord(strID));
		}

		YcHisTimerPtr_.reset(new boost::asio::deadline_timer(io_service_));
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYcStatRecord))
	{
		std::string strTmp = xml.GetAttrib(strLimit);
		boost::algorithm::trim(strTmp);
		int limit = 0;
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

		strTmp = xml.GetAttrib(strYcCycleTime);
		boost::algorithm::trim(strTmp);
		if (!strTmp.empty())
		{
			try
			{
				YcStatCycleTime_ = boost::lexical_cast<int>(strTmp);
			}
			catch(boost::bad_lexical_cast& e)
			{
				std::cerr<<"CTerminal::LoadXmlCfg "<<e.what()<<std::endl;
				YcStatCycleTime_ = DefaultYcCycleTime;
			}
		}

		std::string strID = xml.GetData();
		boost::algorithm::trim(strID);

		if (limit > 0)
		{
			YcStatRecordPtr_.reset(new CYcStatRecord(strID,limit));
		}
		else
		{
			YcStatRecordPtr_.reset(new CYcStatRecord(strID));
		}

		YcStatTimerPtr_.reset(new boost::asio::deadline_timer(io_service_));
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

void CTerminal::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	CCommPoint::SaveXmlCfg(xml);

	xml.AddElem(strActiveCommYx,TransActiveCommYxToString(getActiveCommYx()));
	
	if (getActiveCommQuality())
	{
		xml.AddElem(strActiveCommQuality,strboolTrue);
	}

	if (getEncryptOutside())
	{
		xml.AddElem(strEncryptOutSide,strboolTrue);
	}

	if (yxDB_.getDataBaseSum() > 0)
	{
		xml.AddElem(strYxDB);
		yxDB_.SaveXmlCfg(xml);
	}

	if (ycDB_.getDataBaseSum() > 0)
	{
		xml.AddElem(strYcDB);
		ycDB_.SaveXmlCfg(xml);
	}

	if (ymDB_.getDataBaseSum() > 0)
	{
		xml.AddElem(strYmDB);
		ymDB_.SaveXmlCfg(xml);
	}

	if (ykDB_.getDataBaseSum() > 0)
	{
		xml.AddElem(strYkDB);
		ykDB_.SaveXmlCfg(xml);
	}
	if (yxTab_)
	{
		xml.AddElem(strYxTab);
		yxTab_->SaveXmlCfg(xml);
	}

	if (ycTab_)
	{
		xml.AddElem(strYcTab);
		ycTab_->SaveXmlCfg(xml);
	}

	if (ymTab_)
	{
		xml.AddElem(strYmTab);
		ymTab_->SaveXmlCfg(xml);
	}

	if (ykTab_)
	{
		xml.AddElem(strYkTab);
		ykTab_->SaveXmlCfg(xml);
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
		xml.AddAttrib(strLimit,FaultRecordPtr_->getLimit());
	}

	if (YcHisRecordPtr_)
	{
		xml.AddElem(strYcHisRecord,YcHisRecordPtr_->getID());
		xml.AddAttrib(strLimit,YcHisRecordPtr_->getLimit());
		xml.AddAttrib(strYcCycleTime,YcHisCycleTime_);
	}

	if (YcStatRecordPtr_)
	{
		xml.AddElem(strYcStatRecord,YcStatRecordPtr_->getID());
		xml.AddAttrib(strLimit,YcStatRecordPtr_->getLimit());
		xml.AddAttrib(strYcCycleTime,YcStatCycleTime_);
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

std::string CTerminal::TransActiveCommYxToString(bool val)
{
	if (val)
	{
		return strboolTrue;
	}
	else
	{
		return strboolFalse;
	}
}

bool CTerminal::TransActiveCommFromString(std::string val)
{
	if (boost::iequals(strboolTrue,val))
	{
		return true;
	}
	else 
	{
		return false;
	}

}

//protocol api
bool CTerminal::getCurFcbFlag()
{
	bFcbFlag_ = !bFcbFlag_;

	return bFcbFlag_;
}

boost::shared_ptr<PublicSupport::CSpaceRouteTab> CTerminal::getYxTabPtr()
{
	return yxTab_;
}

boost::shared_ptr<PublicSupport::CSpaceRouteTab> CTerminal::getYcTabPtr()
{
	return ycTab_;
}

boost::shared_ptr<PublicSupport::CSpaceRouteTab> CTerminal::getYkTabPtr()
{
	return ykTab_;
}

boost::shared_ptr<PublicSupport::CSpaceRouteTab> CTerminal::getYmTabPtr()
{
	return ymTab_;
}

unsigned char CTerminal::getTerminalType()
{
	return terminalType_;
}

int CTerminal::AddYkRecord(unsigned char recordType,int yk_no,bool bCloseOrOpen)
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

int CTerminal::AddCosRecord(const CCosPoint & cos)
{
	NotifyEventStatus(YX_EVENT,RETURN_CODE_ACTIVE);

	if (CosRecordPtr_)
	{
		return CosRecordPtr_->AddDataRecord(cos);
	}

	return -1;
}

int CTerminal::AddSoeRecord(const CSoePoint & soe)
{
	NotifyEventStatus(YX_EVENT,RETURN_CODE_CMDSEND);

	if (SoeRecordPtr_)
	{
		return SoeRecordPtr_->AddDataRecord(soe);
	}

	return -1;
}

int CTerminal::AddFaultRecord(const CFaultPoint & fault)
{
	if (FaultRecordPtr_)
	{
		return FaultRecordPtr_->AddDataRecord(fault);
	}

	return -1;
}

int CTerminal::AddHisYcRecord()
{
	if (YcHisRecordPtr_)
	{
		return YcHisRecordPtr_->AddDataRecord(ycDB_);
	}

	return -1;
}

int CTerminal::AddStatYcRecord()
{
	if (YcStatRecordPtr_)
	{
		return YcStatRecordPtr_->AddDataRecord(ycDB_);
	}

	return -1;
}

void CTerminal::handle_timerYcHisCycle(const boost::system::error_code& error)
{
	if (!YcHisTimerPtr_)
	{
		return;
	}

	if (!error)
	{
		AddHisYcRecord();
		ResetTimerYcHisCycle(true);
	}
}

void CTerminal::ResetTimerYcHisCycle(bool bContinue)
{
	if (!YcHisTimerPtr_)
	{
		return;
	}

	if (bContinue)
	{
		YcHisTimerPtr_->expires_from_now(boost::posix_time::minutes(YcHisCycleTime_));
		YcHisTimerPtr_->async_wait(boost::bind(&CTerminal::handle_timerYcHisCycle,this,boost::asio::placeholders::error));
	}
	else
	{
		YcHisTimerPtr_->cancel();
	}
}

void CTerminal::handle_timerYcStatCycle(const boost::system::error_code& error)
{
	if (!YcStatTimerPtr_)
	{
		return;
	}

	if (!error)
	{
		AddStatYcRecord();
		ResetTimerYcStatCycle(true);
	}
}

void CTerminal::ResetTimerYcStatCycle(bool bContinue)
{
	if (!YcStatTimerPtr_)
	{
		return;
	}

	if (bContinue)
	{
		YcStatTimerPtr_->expires_from_now(boost::posix_time::minutes(YcStatCycleTime_));
		YcStatTimerPtr_->async_wait(boost::bind(&CTerminal::handle_timerYcStatCycle,this,boost::asio::placeholders::error));
	}
	else
	{
		YcStatTimerPtr_->cancel();
	}
}

//comm active api
bool CTerminal::getCommActive()
{
	return CCommPoint::getCommActive();
}

void CTerminal::setCommActive(bool val)
{
	CCommPoint::setCommActive(val);

	setCommActiveBackup(getCommActive());
}

bool CTerminal::getCommActieBackup()
{
	return bCommActiveBackup_;
}

void CTerminal::setCommActiveBackup(bool val)
{
	if (getCommActieBackup() == val)
	{
		return;
	}

	if ((!getCommActieBackup()) && val)
	{
		NotifyEventStatus(COMM_EVENT,RETURN_CODE_ACTIVE);
	}
	else
	{
		NotifyEventStatus(COMM_EVENT,RETURN_CODE_NEGATIVE);
	}

	bCommActiveBackup_ = val;

	if (getActiveCommYx())
	{
		typeYxval yx_val = 0;
		if (val)
		{
			yx_val = 1;
		}
		else
		{
			yx_val = 0;
		}

		TrigCosEvent(getSrcYxSum(),yx_val,true);
		TrigSoeEvent(getSrcYxSum(),yx_val,boost::posix_time::microsec_clock::local_time(),true);
	}
}

//event api
void CTerminal::TrigCosEvent(size_t index,typeYxval val,bool bSingleType/* = true*/)
{
	typeYxtype yx_type = single_yx_point;
	if (bSingleType)
	{
		yx_type = single_yx_point;
	}
	else
	{
		yx_type = double_yx_point;
	}

	putCosPoint(index,val,yx_type,0);

	sub_.TrigCosEvent(1);
}

void CTerminal::TrigSoeEvent(size_t index,typeYxval val,boost::posix_time::ptime time,bool bSingleType/* = true*/)
{
	typeYxtype yx_type = single_yx_point;
	if (bSingleType)
	{
		yx_type = single_yx_point;
	}
	else
	{
		yx_type = double_yx_point;
	}

	putSoePoint(index,val,yx_type,time,0);

	sub_.TrigSoeEvent(1);
}

void CTerminal::TrigYcSendByTimeEvent(int num)
{
	sub_.TrigYcSendByTimeEvent(num);
}

//line api
size_t CTerminal::getLineSum()
{
	return lineDB_.getDataBaseSum();
}

CLine * CTerminal::getLinePtr(int index)
{
	return lineDB_.getPointDataPtr(index);
}

//frame api
int CTerminal::CallData()
{
	if (bCallDataImmediately_ && getCommActive())
	{
		return AddCmdVal(Protocol::CALL_ALL_DATA_ACT,Protocol::CALL_ALL_DATA_ACT_PRIORITY,getSelfPtr());
	}

	return -1;
}

int CTerminal::SynTime()
{
	if (bSynTimeImmediately_ && getCommActive())
	{
		return AddCmdVal(Protocol::SYN_TIME_ACT,Protocol::SYN_TIME_ACT_PRIORITY,getSelfPtr());
	}

	return -1;
}

int CTerminal::AddGeneralCmd(Protocol::CCmd cmdVal)
{
	if(bAddSpeCmd_ && getCommActive())
	{
		cmdVal.setCommPoint(getSelfPtr());

		return AddCmdVal(cmdVal);
	}

	return -1;
}

};//namespace DataBase
