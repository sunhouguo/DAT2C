#include "TestCommInterface.h"
#include "../FileSystem/Markup.h"
//#include "../PublicSupport/Dat2cTypeDef.h"

namespace Protocol {

CTestCommInterface::CTestCommInterface(boost::asio::io_service & io_service)
:CProtocol(io_service)
{
	SynCharNum_ = 1;
}

CTestCommInterface::~CTestCommInterface(void)
{
}

int CTestCommInterface::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	exceptedBytes = getFrameBufLeft();

	return 0;
}

int CTestCommInterface::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

int CTestCommInterface::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	myframe frame;
	
	for (size_t i=0;i < exceptedBytes;i++)
	{
		frame.push_back(buf[i]);
	}

	CCmd TestCmd(TEST_CMD,TEST_CMD_PRIORITY,getFirstCommPoint(),frame);
	
	AddSendCmdVal(TestCmd);
	
	return 0;
}

int CTestCommInterface::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	//buf[count++] = 0xff;
	//buf[count++] = 0x00;
	
	return count - bufIndex;
}

int CTestCommInterface::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;
	
	myframe frame;
	
	try
	{
		frame = boost::any_cast<myframe>(cmd.getVal());
	}
	catch(const boost::bad_any_cast & e)
	{
		std::cerr<<"CTestCommInterface::AssembleFrameBody "<<e.what()<<std::endl;
		return -1;
	}
	
	for(size_t i=0;i<frame.size();i++)
	{
		buf[count++] = frame[frame.size() - i -1];
	}
	
	return count - bufIndex;
}

int CTestCommInterface::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	//buf[count++] = 0x00;
	//buf[count++] = 0x0f;
	
	return count - bufIndex;
}
	
int CTestCommInterface::LoadXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;

	if (!xml.Load(filename))
	{
		return -1;
	}

	xml.ResetMainPos();
	xml.FindElem();  //root strProtocolRoot
	xml.IntoElem();  //enter strProtocolRoot

	CProtocol::LoadXmlCfg(xml);

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CTestCommInterface::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();

	CProtocol::SaveXmlCfg(xml);

	xml.OutOfElem();

	xml.Save(filename);
}

}; //namespace Protocol 
