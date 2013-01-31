#include "BF533_CfgFile.h"
#include "../FileSystem/Markup.h"

namespace Protocol
{
	CBF533_CfgFile::CBF533_CfgFile(boost::asio::io_service & io_service)
	{
	}

	CBF533_CfgFile::~CBF533_CfgFile(void)
	{
	}

//以下开始为文件处理接口定义

void CBF533_CfgFile::Creat_BasePara(void)
{
	FileSystem::CMarkup xml;
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "PDZ_BasePara_Set" );    
	xml.AddChildElem( "PDZ_Name", "NARI" );
	xml.AddChildElem( "PDZ_Addr", "1" );
	xml.AddChildElem( "Version");
	xml.IntoElem();
	xml.AddChildElem( "BF518", "131" );
	xml.AddChildElem( "DSP", "131" );
	xml.OutOfElem();
	xml.AddChildElem( "PDZ_Type", "821" );
	xml.AddChildElem( "PDZ_Storgtime", "10" );  
	xml.AddChildElem( "BoardType", "FFE0" );  
	xml.OutOfElem();  
	xml.Save(BFBasePara);
}

void CBF533_CfgFile::Creat_DevSanyaoPara(void)
{
	FileSystem::CMarkup xml;
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "PDZ_SanyaoPara_Set" );  
	xml.AddChildElem( "YkSum","16" );
	xml.AddChildElem( "YxSum","100" );
	xml.AddChildElem( "ChannelSum","123" );
	xml.AddChildElem( "LineSum","10" );
	xml.AddChildElem( "YKCloseTime","60000" );
	xml.AddChildElem( "YKOpenTime","600" );
	xml.AddChildElem( "YXLvBoTime","10" );
	xml.AddChildElem( "AutoRstYxTime","10000" );
	xml.AddChildElem( "AutoRstYxFlag","0" );
	xml.AddChildElem( "HuoHua_AutoFlag","0" );
	xml.AddChildElem( "HuoHua_Day","20" );
	xml.AddChildElem( "AutoRstProtectTime","12000" );
	xml.AddChildElem( "AutoRstProtectFlag","0" );
	xml.AddChildElem( "I_Rated","0" );
    xml.AddChildElem( "U_SwitchFlag","0" );
	xml.AddChildElem( "PT_L_Arlam","0" );//Modfiy by Zhangzhihua 20111220
	xml.Save(BFDevSanyaoPara);
}

void CBF533_CfgFile::Creat_ChanneltypePara(void)
{
	FileSystem::CMarkup xml;
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "PDZ_Channel_Type" ); 
	xml.AddChildElem( "ChannelSum" );
	xml.SetChildAttrib( "Sum",40);
	xml.IntoElem();
	xml.AddChildElem( "CH000", "0" );
	xml.AddChildElem( "CH001", "0" );
	xml.AddChildElem( "CH002", "0" );
	xml.AddChildElem( "CH003", "1" );
	xml.AddChildElem( "CH004", "1" );
	xml.AddChildElem( "CH005", "1" );
	xml.AddChildElem( "CH006", "2" );
	xml.AddChildElem( "CH007", "2" );
	xml.AddChildElem( "CH008", "1" );
	xml.AddChildElem( "CH009", "1" );
	xml.AddChildElem( "CH010", "1" );
	xml.AddChildElem( "CH011", "1" );
	xml.AddChildElem( "CH012", "1" );
	xml.AddChildElem( "CH013", "1" );
	xml.AddChildElem( "CH014", "1" );
	xml.AddChildElem( "CH015", "1" );
	xml.AddChildElem( "CH016", "1" );
	xml.AddChildElem( "CH017", "1" );
	xml.AddChildElem( "CH018", "1" );
	xml.AddChildElem( "CH019", "1" );
	xml.AddChildElem( "CH020", "1" );
	xml.AddChildElem( "CH021", "1" );
	xml.AddChildElem( "CH022", "1" );
	xml.AddChildElem( "CH023", "1" );
	xml.AddChildElem( "CH024", "1" );
	xml.AddChildElem( "CH025", "1" );
	xml.AddChildElem( "CH026", "1" );
	xml.AddChildElem( "CH027", "1" );
	xml.AddChildElem( "CH028", "1" );
	xml.AddChildElem( "CH029", "1" );
	xml.AddChildElem( "CH030", "1" );
	xml.AddChildElem( "CH031", "1" );
	xml.AddChildElem( "CH032", "1" );
	xml.AddChildElem( "CH033", "1" );
	xml.AddChildElem( "CH034", "1" );
	xml.AddChildElem( "CH035", "1" );
	xml.AddChildElem( "CH036", "1" );
	xml.AddChildElem( "CH037", "1" );
	xml.AddChildElem( "CH038", "1" );
	xml.AddChildElem( "CH039", "1" );
	xml.Save(BFChanneltypePara);   
}

void CBF533_CfgFile::Creat_InterfacePara(void)
{
#if defined(__linux__)

	FileSystem::CMarkup xml;
	char IP[24];
	char MASK[24];
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "PDZ_InterfacePara_Set" );    
	xml.AddChildElem( "PDZ_Lan_Set" ); 
	xml.SetChildAttrib( "LanNo", "1" );
	getip("eth1",IP,MASK) ;
	xml.IntoElem();
	xml.AddChildElem( "IP", IP );
	xml.AddChildElem( "MASK", MASK );
	xml.AddChildElem( "Gatway", "192.168.10.254" );
	xml.OutOfElem();

	xml.AddChildElem( "PDZ_Lan_Set" ); 
	xml.SetChildAttrib( "LanNo", "2" );
	getip("eth2",IP,MASK) ;
	xml.IntoElem();
	xml.AddChildElem( "IP", IP );
	xml.AddChildElem( "MASK", MASK );
	xml.AddChildElem( "Gatway", "192.168.11.254" );
	xml.OutOfElem(); 

	xml.AddChildElem( "PDZ_Serial_Set" ); 
	xml.SetChildAttrib( "SerialNo", "1" );
	xml.IntoElem();
	xml.AddChildElem( "BaudRate", "9600" );
	xml.AddChildElem( "Parity", "0" );
	xml.AddChildElem( "DATA", "8" );
	xml.AddChildElem( "STOP", "1" );
	xml.OutOfElem();  

	xml.AddChildElem( "PDZ_Serial_Set" ); 
	xml.SetChildAttrib( "SerialNo", "2" );
	xml.IntoElem();
	xml.AddChildElem( "BaudRate", "9600" );
	xml.AddChildElem( "Parity", "0" );
	xml.AddChildElem( "DATA", "8" );
	xml.AddChildElem( "STOP", "1" );
	xml.OutOfElem(); 

	xml.AddChildElem( "PDZ_Master_Num", "2" );      
	xml.AddChildElem( "PDZ_Master_Set" );  
	xml.SetChildAttrib( "MasterNo", "1" );
	xml.IntoElem();
	xml.AddChildElem( "CommType", "2" );
	xml.AddChildElem( "Protocol", "1" );
	xml.AddChildElem( "IP", "192.168.1.1" );
	xml.AddChildElem( "PortNo", "2404" );
	xml.AddChildElem( "SerialPort");
	xml.OutOfElem(); 
	xml.AddChildElem( "PDZ_Master_Set" );  
	xml.SetChildAttrib( "MasterNo", "2" );
	xml.IntoElem();
	xml.AddChildElem( "CommType", "0" );
	xml.AddChildElem( "Protocol", "0" );
	xml.AddChildElem( "IP");
	xml.AddChildElem( "PortNo");
	xml.AddChildElem( "SerialPort", "0" );       

	xml.Save(BFInterfacePara);   
#endif
}

void CBF533_CfgFile::Creat_LinePara(void)
{
	FileSystem::CMarkup xml;
	int i;
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "PDZ_LinePara_Set" ); 
	xml.AddChildElem( "LineSum" );
	xml.SetChildAttrib( "Sum",10);
	xml.IntoElem();
	for(i=0;i<10;i++)
	{
		xml.AddChildElem( "Line_Set" );
		xml.SetChildAttrib( "LineNo",i);
		xml.IntoElem();
		xml.AddChildElem( "CTRate", "5" );
		xml.AddChildElem( "PTRate", "100" );
		xml.AddChildElem( "I0Rate", "5" );
		xml.AddChildElem( "U0Rate", "100" );
		xml.AddChildElem( "CTRatio", "120" );
		xml.AddChildElem( "PTRatio", "1100" );
		xml.AddChildElem( "I0Ratio", "50" );
		xml.AddChildElem( "U0Ratio", "100" );
		xml.AddChildElem( "YXStart", (80 + i*64 ));
		xml.AddChildElem( "IO_Flage", "0" );//Modfiy by Zhangzhihua 20111220

		xml.AddChildElem( "PTChannelNo" ); 
		xml.IntoElem();
		xml.AddChildElem( "U1", "0" );
		xml.AddChildElem( "U2", "1" );
		xml.AddChildElem( "U3", "2" );
		xml.AddChildElem( "U0", "255" );
		xml.OutOfElem();

		xml.AddChildElem( "CTChannelNo" ); 
		xml.IntoElem();
		xml.AddChildElem( "CI1", "3" );
		xml.AddChildElem( "CI2", "4" );
		xml.AddChildElem( "CI3", "5" );
		xml.AddChildElem( "BI1", "3" );
		xml.AddChildElem( "BI2", "4" );
		xml.AddChildElem( "BI3", "5" );
		xml.AddChildElem( "I0", "255" );
		xml.OutOfElem();   
		xml.OutOfElem(); 
	}
	xml.Save(BFLinePara);   
}

void CBF533_CfgFile::Creat_Master1YcZFTab(void)
{
	FileSystem::CMarkup xml;
	int i;
	char zftab[6];
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "PDZ_Master1YCZFTab_Set" );
	xml.AddChildElem( "YCZFTab"); 
	xml.SetChildAttrib( "YCZFNum", "120" );
	xml.IntoElem();
	for(i=0;i<123;i++)
	{
		sprintf(zftab,"YC%03d",i);
		xml.AddChildElem( zftab, i );
	}
	xml.OutOfElem();
	xml.Save(BFMaster1YcZFTab);   
}

void CBF533_CfgFile::Creat_Master1YxZFTab(void)
{
	FileSystem::CMarkup xml;
	int i;
	char zftab[6];
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "PDZ_Master1YXZFTab_Set" );    
	xml.AddChildElem( "YXZFTab");
	xml.SetChildAttrib( "YXZFNum", "152" );
	xml.IntoElem();
	for(i=0;i<121;i++)
	{
		sprintf(zftab,"YX%03d",i);
		xml.AddChildElem( zftab, i );
	}
	xml.OutOfElem();
	xml.Save(BFMaster1YxZFTab); 
}

void CBF533_CfgFile::Creat_Master2YcZFTab(void)
{
	FileSystem::CMarkup xml;
	int i;
	char zftab[6];
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "PDZ_Master2YCZFTab_Set" );
	xml.AddChildElem( "YCZFTab"); 
	xml.SetChildAttrib( "YCZFNum", "120" );
	xml.IntoElem();
	for(i=0;i<123;i++)
	{
		sprintf(zftab,"YC%03d",i);
		xml.AddChildElem( zftab, i );
	}
	xml.OutOfElem();
	xml.Save(BFMaster2YcZFTab);
}

void CBF533_CfgFile::Creat_Master2YxZFTab(void)
{
	FileSystem::CMarkup xml;
	int i;
	char zftab[6];
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "PDZ_Master2YXZFTab_Set" );    
	xml.AddChildElem( "YXZFTab");
	xml.SetChildAttrib( "YXZFNum", "152" );
	xml.IntoElem();
	for(i=0;i<121;i++)
	{
		sprintf(zftab,"YX%03d",i);
		xml.AddChildElem( zftab, i );
	}
	xml.OutOfElem();
	xml.Save(BFMaster2YxZFTab);   
}

void CBF533_CfgFile::Creat_ProtectValPara(void)
{
	FileSystem::CMarkup xml;
	int i;
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "ProVal_Set" ); 
	xml.AddChildElem( "LineSum" );
	xml.SetChildAttrib( "Sum",10);
	xml.IntoElem();
	for(i=0;i<10;i++)
	{
		xml.AddChildElem( "Pro_Set" );
		xml.SetChildAttrib( "Line",i);
		xml.IntoElem();
		xml.AddChildElem( "LowVol_Val", "22000" );
		xml.AddChildElem( "OL_P_Val", "6000" );
		xml.AddChildElem( "OL_Ck_T", "0" );
		xml.AddChildElem( "OL_TZ_Delay", "6000" );
		xml.AddChildElem( "OF_I_P_Val", "6000" );
		xml.AddChildElem( "OF_I_Ck_T", "0" );
		xml.AddChildElem( "OF_I_TZ_Delay", "0" );
		xml.AddChildElem( "OF_II_P_Val","10000");
		xml.AddChildElem( "OF_II_Ck_T","0");
		xml.AddChildElem( "OF_II_TZ_Delay","0");
		xml.AddChildElem( "I0_I_P_val", "100" );
		xml.AddChildElem( "I0_I_Ck_T", "0" );
		xml.AddChildElem( "I0_I_TZ_Delay", "0");
		xml.AddChildElem( "I0_II_P_val", "100" );
		xml.AddChildElem( "I0_II_Ck_T", "0" );
		xml.AddChildElem( "I0_II_TZ_Delay", "0");
		xml.AddChildElem( "ReClose_T", "0" );
		//xml.AddChildElem( "U0_P_val", "1000" );
		//xml.AddChildElem( "U0_P_T", "0" );
		xml.AddChildElem( "Accel_T", "0" );
		xml.AddChildElem( "OverU_P_Val", "12000" );
		xml.AddChildElem( "OverU_Ck_T", "0" );   
		xml.AddChildElem( "OverU_TZ_T","0");
		xml.AddChildElem( "NetFA_PS_Addr", "0" );
		xml.AddChildElem( "NetFA_LS_Addr", "0" );
		xml.AddChildElem( "FA_Lose_Sum", "0" );
		xml.AddChildElem( "FA_Rst_T", "0" );
		xml.AddChildElem( "LowVol_F", "0" );
		xml.AddChildElem( "ReClose_F", "0" );
		xml.AddChildElem( "OL_TZ_F", "0" );
		xml.AddChildElem( "OL_Alarm_F", "0" );
		xml.AddChildElem( "OF_I_TZ_F", "0" );
		xml.AddChildElem( "OF_I_Alarm_F", "0" ); 
		xml.AddChildElem( "OF_II_TZ_F","0");
		xml.AddChildElem( "OF_II_Alarm_F","0");
		xml.AddChildElem( "I0_I_TZ_F", "0" );
		xml.AddChildElem( "I0_I_Alarm_F", "0" );
		xml.AddChildElem( "I0_II_TZ_F", "0" );
		xml.AddChildElem( "I0_II_Alarm_F", "0" );
		//xml.AddChildElem( "U0_P_F", "0" );
		xml.AddChildElem( "NetFA_F", "0" );
		xml.AddChildElem( "Accel_F", "0" );
		xml.AddChildElem( "OverU_Alarm_F", "0" );
		xml.AddChildElem( "OverU_TZ_F","0");
		xml.AddChildElem( "FA_F", "0" );

		xml.OutOfElem();

	}
	xml.Save(BFProtectValPara);
}

void CBF533_CfgFile::Creat_Extremum(int linenum,boost::posix_time::ptime time)
{
	std::stringstream ss2;
	FileSystem::CMarkup xml;
	std::string str2;
//	char Time[50];
	int year,month,day,hour,min;

	boost::posix_time::time_duration td = time.time_of_day();
	boost::gregorian::date::ymd_type ymd = time.date().year_month_day();
	int Sec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) / (1000);
	int Msec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) % (1000);

	year = ymd.year;
	month = ymd.month;
	day = ymd.day;
	hour = td.hours();
	min = td.minutes();

	ss2 <<year<<"/"<<month<<"/"<<day<<"-"<<hour<<":"<<min<<":"<<Sec<<":"<<Msec;
	ss2 >> str2;


	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "HistoricalData" ); 
	xml.SetAttrib( "LineSum",linenum);
	xml.AddChildElem( "ExtremumRecordNode" ); 
	xml.IntoElem();
	for(int i = 0;i < linenum;i ++)
	{
		xml.AddChildElem( "ExtremumVal" );
		xml.SetChildAttrib( "LineNo",i);
		xml.IntoElem();
		xml.AddChildElem( "MaxUa","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinUa","5770");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxUb","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinUb","5770");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxUc","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinUc","5770");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxIa","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinIa","5000");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxIb","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinIb","5000");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxIc","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinIc","5000");
		xml.SetChildAttrib( "Time",str2);
		xml.OutOfElem();
	}
	xml.OutOfElem();

	xml.Save(BFExtremum);
}

void CBF533_CfgFile::Creat_HistoricalData(int linenum,boost::posix_time::ptime time)
{
	std::stringstream ss1,ss2;
	FileSystem::CMarkup xml;
	std::string str1;
	std::string str2;
//	char Time[50];
//	char Name[20];
	int year,month,day,hour,min;

	boost::posix_time::time_duration td = time.time_of_day();
	boost::gregorian::date::ymd_type ymd = time.date().year_month_day();
	int Sec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) / (1000);
	int Msec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) % (1000);

	year = ymd.year;
	month = ymd.month;
	day = ymd.day;
	hour = td.hours();
	min = td.minutes();

	ss1 <<"/mnt/"<<year<<month<<day<<".xml";
	ss1 >> str1;
	std::cout<<"文件名为："<< str1 <<std::endl; 
	ss2 <<year<<"/"<<month<<"/"<<day<<"-"<<hour<<":"<<min<<":"<<Sec<<":"<<Msec;
	ss2 >> str2;
	//	std::cout<<"现在时间是："<< str2 <<std::endl; 

	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "HistoricalData" ); 
	xml.SetAttrib( "LineSum",linenum);
	xml.AddChildElem( "ExtremumRecordNode" ); 
	xml.IntoElem();
	for(int i = 0;i < linenum;i ++)
	{
		xml.AddChildElem( "ExtremumVal" );
		xml.SetChildAttrib( "LineNo",i);
		xml.IntoElem();
		xml.AddChildElem( "MaxUa","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinUa","5770");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxUb","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinUb","5770");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxUc","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinUc","5770");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxIa","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinIa","5000");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxIb","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinIb","5000");
		xml.SetChildAttrib( "Time",str2);

		xml.AddChildElem( "MaxIc","0");
		xml.SetChildAttrib( "Time",str2);
		xml.AddChildElem( "MinIc","5000");
		xml.SetChildAttrib( "Time",str2);
		xml.OutOfElem();
	}
	xml.OutOfElem();

	xml.Save(str1);

}

void CBF533_CfgFile::Creat_BFYMData(int LineNum)
{
	FileSystem::CMarkup xml;

	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");

	xml.AddElem( "YMData" ); 
	xml.SetAttrib( "Num",0);
	xml.SetAttrib( "CurPtr",0);

	xml.Save(BFYMData);
}

void CBF533_CfgFile::Creat_BFFaultRecord(void)
{
	FileSystem::CMarkup xml;

	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");

	xml.AddElem( "FaultmRecordNode" ); 
	xml.SetAttrib( "sum",0);
	xml.SetAttrib( "CurPtr",0);

	xml.Save(BFFaultRecord);
}


void CBF533_CfgFile::Creat_BFSOERecord(void)
{
	FileSystem::CMarkup xml;

	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");

	xml.AddElem( "SOERecordNode"); 
	xml.SetAttrib( "Sum",0);
	xml.SetAttrib( "Index",0);

	xml.Save(BFSOERecord);
}

void CBF533_CfgFile::Creat_RecordFileCfg(void)
{
	FileSystem::CMarkup xml;

	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "RecordFileCfg");
	xml.IntoElem();
	xml.ResetMainPos(); 

	xml.AddElem("FaultRecord");
	xml.AddChildElem("SaveFlag","false");
	xml.AddChildElem("FaultRecordMax",20);
	xml.AddElem("SOEStroe");
	xml.AddChildElem("SaveFlag","false");
	xml.AddChildElem("SOEStroeMax",20);
	xml.AddElem("HisData");
	xml.AddChildElem("SaveFlag","false");
	xml.AddChildElem("HisDataMax",5);
	xml.AddElem("YkRecord");
	xml.AddChildElem("SaveFlag","false");
	xml.AddChildElem("YkRecord",20);

	xml.OutOfElem();

	xml.Save(RecordFileCfg);
}

void CBF533_CfgFile::Creat_NoCurRecord(unsigned int LineNum)
{
	std::stringstream ss2;

	std::string str2;

	int year,month,day,hour,min;

	using namespace boost::posix_time;
	boost::posix_time::time_duration td = (boost::posix_time::microsec_clock::local_time()).time_of_day();
	boost::gregorian::date::ymd_type ymd = (boost::posix_time::microsec_clock::local_time()).date().year_month_day();

	int Sec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) / (1000);
	int Msec = ((td.total_milliseconds() % (1000*60*60))% (1000*60)) % (1000);

	year = ymd.year;
	month = ymd.month;
	day = ymd.day;
	hour = td.hours();
	min = td.minutes();


	ss2 <<year<<"/"<<month<<"/"<<day<<"-"<<hour<<":"<<min<<":"<<Sec<<":"<<Msec;
	ss2 >> str2;

	FileSystem::CMarkup xml;
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
	xml.AddElem( "NoCurRecord");
	xml.AddAttrib("Count",LineNum);
	xml.IntoElem();
	for (int i = 0; i < LineNum ;i ++)
	{
		xml.AddElem("RecordNode");
		xml.AddAttrib("LineNo",i);
		xml.AddAttrib("Time",str2);
		xml.AddChildElem("Ua");
		xml.SetChildData(0);
		xml.AddChildElem("Ub");
		xml.SetChildData(0);
		xml.AddChildElem("Uc");
		xml.SetChildData(0);
		xml.AddChildElem("Ia");
		xml.SetChildData(0);
		xml.AddChildElem("Ib");
		xml.SetChildData(0);
		xml.AddChildElem("Ic");
		xml.SetChildData(0);
	}
	xml.OutOfElem();
	xml.Save(NoCurRecord);
}

int  CBF533_CfgFile::getip(char *name,char *ip_buf,char *mask) //char *
{
#if defined(__linux__)
	struct ifreq temp;
	struct sockaddr_in *myaddr;
	int fd = 0;
	int ret = -1;
	strcpy(temp.ifr_name, name);
	if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		//       return -1;
	}

	ret = ioctl(fd, SIOCGIFADDR, &temp);
	//    close(fd);
	if(ret < 0)
		return NULL;
	myaddr = (struct sockaddr_in *)&(temp.ifr_addr);
	//    printf("ip %s\n",inet_ntoa(myaddr->sin_addr));
	strcpy(ip_buf, inet_ntoa(myaddr->sin_addr));

	ret = ioctl(fd, SIOCGIFNETMASK, &temp);
	//    close(fd);
	if(ret < 0)
		return NULL;
	myaddr = (struct sockaddr_in *)&(temp.ifr_addr);
	//    printf("mask %s\n",inet_ntoa(myaddr->sin_addr));
	strcpy(mask, inet_ntoa(myaddr->sin_addr));

	ret = ioctl(fd, SIOCGIFBRDADDR, &temp);
	close(fd);
	if(ret < 0)
		return NULL;
	myaddr = (struct sockaddr_in *)&(temp.ifr_addr);
	//    printf("SIOCGIFBRDADDR %s\n",inet_ntoa(myaddr->sin_addr));
	//    strcpy(ip_buf, inet_ntoa(myaddr->sin_addr));


	//    return ip_buf;
#endif
	return 0;
}

int CBF533_CfgFile::setip(char *name,char *ip,char *mask)
{
#if defined(__linux__)
	struct ifreq temp;
	struct sockaddr_in *addr;
	int fd = 0;
	int ret = -1;
	strcpy(temp.ifr_name, name);
	if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		return -1;
	}

	addr = (struct sockaddr_in *)&(temp.ifr_addr);
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr(ip);
	ret = ioctl(fd, SIOCSIFADDR, &temp);

	addr = (struct sockaddr_in *)&(temp.ifr_netmask);
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr(mask);
	ret = ioctl(fd, SIOCSIFNETMASK, &temp);

	addr = (struct sockaddr_in *)&(temp.ifr_addr);
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr("192.168.152.254");
	ret = ioctl(fd, SIOCADDRT, &temp);

	close(fd);
	if(ret < 0)
		return -1;
#endif
	return 0;
}

};

