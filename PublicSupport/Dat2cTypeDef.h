#pragma once
#ifndef Dat2cTypeDef_H
#define Dat2cTypeDef_H

#include <boost/signals2.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>

namespace DataBase
{
	class CCommPoint;
	class CPriStation;
	class CTerminal;
};

//database ptr
typedef boost::weak_ptr<DataBase::CPriStation> weak_pristation_ptr;
typedef boost::weak_ptr<DataBase::CTerminal> weak_terminal_ptr;
typedef boost::weak_ptr<DataBase::CCommPoint> weak_commpoint_ptr;
typedef boost::shared_ptr<DataBase::CPriStation> share_pristation_ptr;
typedef boost::shared_ptr<DataBase::CTerminal> share_terminal_ptr;
typedef boost::shared_ptr<DataBase::CCommPoint> share_commpoint_ptr;

//cmd type
typedef unsigned short typeCmd;
typedef boost::signals2::signal<void(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)> CmdRecallSignalType;
typedef CmdRecallSignalType::slot_type CmdRecallSlotType;
typedef boost::signals2::connection SigConnection;

//yc data type
typedef short typeYcval;
typedef short typeYcplus;
typedef float typeYcmul;
typedef unsigned short typeYcdead;
typedef unsigned char  typeYcquality;
typedef unsigned int typeFinalYcval;

const typeYcval MaxYcVal = 32767;
const typeYcval MinYcVal = -32768;

//yk data type
typedef unsigned char typeYkstatus;
typedef unsigned char typeYktype;

//ym data type
typedef unsigned int typeYmval;
typedef unsigned char  typeYmquality;

//yx data type
typedef unsigned char typeYxval;
typedef unsigned char typeYxtype;
typedef unsigned char typeYxQuality;

//addr type
typedef unsigned int typeAddr;

//bool to string
const std::string strboolTrue = "TRUE";
const std::string strboolFalse = "FALSE";
const std::string strIndexOrderEnable = "IndexOrderEnable";

//log
const std::string strFileType = "FileType";
const std::string strLimit = "Limit";
//const std::string strXmlLog   = "XmlLog";
//const std::string strTextLog  = "TextLog";

//record
const std::string strYkRecord = "YkRecord";
const std::string strSoeRecord = "SoeRecord";
const std::string strCosRecord = "CosRecord";
const std::string strFaultRecord = "FaultRecord";
const std::string strYcHisRecord = "YcHisRecord";
const std::string strYcStatRecord = "YcStatRecord";
const std::string strYcCycleTime  = "YcCycleTime";

//exception
typedef boost::error_info<struct errinfo_middle_name,std::string> errinfo_middletype_name;

namespace PublicSupport
{
	struct dat2def_exception:
		virtual boost::exception,
		virtual std::exception
	{};
}

const std::string strXmlHead ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
const std::string strSubXsl ("<?xml-stylesheet type=\"text/xsl\" href=\"../web/sub.xsl\"?>\r\n");
const std::string strProtocolXsl ("<?xml-stylesheet type=\"text/xsl\" href=\"../web/protocol.xsl\"?>\r\n");
const std::string strSerialXsl ("<?xml-stylesheet type=\"text/xsl\" href=\"../web/serial.xsl\"?>\r\n");
const std::string IndexOutOfMemoryErr = "Index out of range";
const int MinutesRemainderMillisecs = 60000; 

const unsigned char BYTE_CHECK_TRUE[8] ={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
const unsigned char BYTE_CHECK_FALSE[8]={0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F};

const unsigned char RETURN_CODE_CMDSEND = 1;
const unsigned char RETURN_CODE_ACTIVE = 2;
const unsigned char RETURN_CODE_NEGATIVE = 3;
const unsigned char RETURN_CODE_TIMEOUT = 4;
const unsigned char RETURN_CODE_KEYERROR = 5;  //√‹‘ø¥ÌŒÛ
const unsigned char RETURN_CODE_CMDRECV = 6;

const unsigned char CpuVersion = 1;

const unsigned short MAX_IP_MTU = 1480;
const unsigned short MAX_TCP_MTU = 1460;

#endif //#ifndef Dat2cTypeDef_H



