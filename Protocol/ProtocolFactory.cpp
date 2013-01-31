#include <boost/algorithm/string/predicate.hpp>
#include "ProtocolFactory.h"
#include "H101.h"
#include "H104.h"
#include "S101.h"
#include "S104.h"
#include "H103.h"
#include "Modbus.h"
#include "H103_DigiproII.h"
#include "H103_MMP_LI.h"
#include "H103_MMP_CK.h"
#include "H103_NZ.h"
#include "H103_NZ_UDP.h"
#include "S101_B.h"
#include "TestCommInterface.h"
#include "GB_T18657_1.h"
#include "BF533.h"
#include "S104_518.h"
#include "S101_518.h"
#include "H101_B.h"
#include "HTestprocotol.h"
#include "S101_Transmit.h"
#include "S104_Transmit.h"
#include "H101_Transmit.h"
#include "H104_Transmit.h"
#include "HCdt_Transmit.h"
#include "SCdt_Transmit.h"

namespace Protocol {

//自定义规约类型标识
const short H101 = 1;
const short H104 = 2;
const short S101 = 3;
const short S104 = 4;
const short H103 = 5;
const short Modbus = 6;
const short H103_DigiproII = 7;
const short H103_MMP_LI = 8;
const short H103_NZ_UDP = 9;
const short H103_NZ = 10;
const short S101_B = 11;
const short H103_MMP_CK = 12;
const short TEST = 13;
const short GB_T18657_1 = 14;
const short BF533 = 15;
const short S101_518 = 16;
const short S104_518 = 17;
const short H101_B = 18;
const short HTest =19;
const short S101_Transmit = 20;
const short S104_Transmit = 21;
const short H101_Transmit = 22;
const short H104_Transmit = 23;
const short HCDT_Transmit = 24;
const short SCDT_Transmit = 25;

const std::string strH101Val = "H101";
const std::string strS101Val = "S101";
const std::string strH104Val = "H104";
const std::string strS104Val = "S104";
const std::string strH103Val = "H103";
const std::string strModbusVal = "Modbus";
const std::string strH103_DigiproII = "H103_DigiproII";
const std::string strH103_MMP_LI = "H103_MMP_LI";
const std::string strH103_MMP_CK = "H103_MMP_CK";
const std::string strH103_NZ_UDP = "H103_NZ_UDP";
const std::string strH103_NZ = "H103_NZ";
const std::string strS101_B = "S101_B";
const std::string strTest = "TEST";
const std::string strGB_T18657_1 = "T18657";
const std::string strBF533 = "BF533";
const std::string strS101_518 = "S101_518";
const std::string strS104_518 = "S104_518";
const std::string strH101_B = "H101_B";
const std::string strHTest = "HTest";
const std::string strS101_Transmit = "S101_Transmit";
const std::string strS104_Transmit = "S104_Transmit";
const std::string strH101_Transmit = "H101_Transmit";
const std::string strH104_Transmit = "H104_Transmit";
const std::string strSCDT_Transmit = "SCDT_Transmit";
const std::string strHCDT_Trnasmit = "HCDT_Transmit";
const std::string strUnspecificProtocolVal = "UnspecificProtocol";
 
CProtocolFactory::CProtocolFactory(void)
{
}

CProtocolFactory::~CProtocolFactory(void)
{
}

CProtocol * CProtocolFactory::CreateProtocol(std::string protocolType,boost::asio::io_service & io_service)
{
	int ret = TransProtocolTyeFromString(protocolType);
	if (ret < 0)
	{
		return NULL;
	}

	CProtocol * pl = NULL;

	switch (ret)
	{
	case H104:
		pl = new Protocol::CH104(io_service);
		break;

	case S101:
		pl = new Protocol::CS101(io_service);
		break;

	case S104:
		pl = new Protocol::CS104(io_service);
		break;

	case TEST:
		pl = new Protocol::CTestCommInterface(io_service);
		break;

	case BF533:
		pl = new Protocol::CBF533(io_service);
		break;

	case S104_518:
		pl = new Protocol::CS104_518(io_service);
		break;

	case S101_518:
		pl = new Protocol::CS101_518(io_service);
		break;

	case H101:
		pl = new Protocol::CH101(io_service);
		break;

	case GB_T18657_1:
		pl = new Protocol::CGB_T18657_1(io_service);
		break;

	case Modbus:
		pl = new Protocol::CModbus(io_service);
		break;

	case H103_DigiproII:
		pl = new Protocol::CH103_DigiproII(io_service);
		break;

	case H103_MMP_LI:
		pl = new Protocol::CH103_MMP_LI(io_service);
		break;

	case H103_MMP_CK:
		pl = new Protocol::CH103_MMP_CK(io_service);
		break;

	case H103_NZ_UDP:
		pl = new Protocol::CH103_NZ_UDP(io_service);
		break;

	case H103_NZ:
		pl = new Protocol::CH103_NZ(io_service);
		break;

	case H103:
		pl = new Protocol::CH103(io_service);
		break;	

	case S101_B:
		pl = new Protocol::CS101_B(io_service);
		break;

	case H101_B:
		pl = new Protocol::CH101_B(io_service);
		break;

	case HTest:
		pl = new Protocol::CTestHProcotol(io_service);
		break;

	case S101_Transmit:
		pl = new Protocol::CS101_Transmit(io_service);
		break;

	case S104_Transmit:
		pl = new Protocol::CS104_Transmit(io_service);
		break;

	case H101_Transmit:
		pl = new Protocol::CH101_Transmit(io_service);
		break;

	case H104_Transmit:
		pl = new Protocol::CH104_Transmit(io_service);
		break;

	case HCDT_Transmit:
		pl = new Protocol::CHCdt_Transmit(io_service);
		break;

	case SCDT_Transmit:
		pl = new Protocol::CSCdt_Transmit(io_service);
		break;

	default:
		pl = NULL;
		break;
	}

	return pl;

}

std::string CProtocolFactory::TransProtocolTypeToString(unsigned short val)
{
	std::string strTmp = strUnspecificProtocolVal;

	switch (val)
	{
	case H101:
		strTmp = strH101Val;
		break;

	case H104:
		strTmp = strH104Val;
		break;

	case S101:
		strTmp = strS101Val;
		break;

	case S104:
		strTmp = strS104Val;
		break;

	case H103:
		strTmp = strH103Val;
		break;

	case Modbus:
		strTmp = strModbusVal;
		break;

	case H103_DigiproII:
		strTmp = strH103_DigiproII;
		break;

	case H103_MMP_LI:
		strTmp = strH103_MMP_LI;
		break;

	case H103_MMP_CK:
		strTmp = strH103_MMP_CK;
		break;

	case H103_NZ_UDP:
		strTmp = strH103_NZ_UDP;
		break;

	case H103_NZ:
		strTmp = strH103_NZ;
		break;

	case S101_B:
		strTmp = strS101_B;
		break;

	case TEST:
		strTmp = strTest;
		break;

	case GB_T18657_1:
		strTmp = strGB_T18657_1;
		break;

	case BF533:
		strTmp = strBF533;
		break;

	case S101_518:
		strTmp = strS101_518;
		break;

	case S104_518:
		strTmp = strS104_518;
		break;

	case H101_B:
		strTmp = strH101_B;
		break;

	case HTest:
		strTmp = strHTest;
		break;

	case S101_Transmit:
		strTmp = strS101_Transmit;
		break;

	case S104_Transmit:
		strTmp = strS104_Transmit;
		break;

	case H101_Transmit:
		strTmp = strH101_Transmit;
		break;

	case H104_Transmit:
		strTmp = strH104_Transmit;
		break;

	case HCDT_Transmit:
		strTmp = strHCDT_Trnasmit;
		break;

	case SCDT_Transmit:
		strTmp = strSCDT_Transmit;
		break;

	default:
		break;
	}

	return strTmp;
}

short CProtocolFactory::TransProtocolTyeFromString( std::string val )
{
	short ret = -1;

	if (boost::iequals(strH101Val,val))
	{
		ret = H101;
	}
	else if (boost::iequals(strH104Val,val))
	{
		ret = H104;
	}
	else if (boost::iequals(strS101Val,val))
	{
		ret = S101;
	}
	else if (boost::iequals(strS104Val,val))
	{
		ret = S104;
	}
	else if (boost::iequals(strH103Val,val))
	{
		ret = H103;
	}
	else if (boost::iequals(strModbusVal,val))
	{
		ret = Modbus;
	}
	else if (boost::iequals(strH103_DigiproII,val))
	{
		ret = H103_DigiproII;
	}
	else if (boost::iequals(strH103_MMP_LI,val))
	{
		ret = H103_MMP_LI;
	}

	else if (boost::iequals(strH103_MMP_CK,val))
	{
		ret = H103_MMP_CK;
	}
	else if (boost::iequals(strH103_NZ_UDP,val))
	{
		ret = H103_NZ_UDP;
	}
	else if (boost::iequals(strH103_NZ,val))
	{
		ret = H103_NZ;
	}
	else if (boost::iequals(strS101_B,val))
	{
		ret = S101_B;
	}
	else if (boost::iequals(strTest,val))
	{
		ret = TEST;
	}
	else if (boost::iequals(strGB_T18657_1,val))
	{
		ret = GB_T18657_1;
	}
	else if (boost::iequals(strBF533,val))
	{
		ret = BF533;
	}
	else if (boost::iequals(strS101_518,val))
	{
		ret = S101_518;
	}
	else if (boost::iequals(strS104_518,val))
	{
		ret = S104_518;
	}
	else if (boost::iequals(strH101_B,val))
	{
		ret = H101_B;
	}
	else if (boost::iequals(strHTest,val))
	{
		ret = HTest;
	}
	else if (boost::iequals(strS101_Transmit,val))
	{
		ret = S101_Transmit;
	}
	else if (boost::iequals(strS104_Transmit,val))
	{
		ret = S104_Transmit;
	}
	else if (boost::iequals(strH101_Transmit,val))
	{
		ret = H101_Transmit;
	}
	else if (boost::iequals(strH104_Transmit,val))
	{
		ret = H104_Transmit;
	}
	else if (boost::iequals(strHCDT_Trnasmit,val))
	{
		ret = HCDT_Transmit;
	}
	else if (boost::iequals(strSCDT_Transmit,val))
	{
		ret = SCDT_Transmit;
	}

	return ret;
}

};//namespace Protocol

