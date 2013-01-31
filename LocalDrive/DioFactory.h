#pragma once

namespace DataBase
{
	class CSubStation;
}

namespace LocalDrive {

class CDIO;

class CDIOFactory
{
public:
	~CDIOFactory(void);
	static std::string TransDIOTypeToString(unsigned char val);
	static CDIO * CreateDIO(std::string dioType,DataBase::CSubStation & sub);

private:
	CDIOFactory(void);
	static unsigned char TransDIOTypeFromString(std::string val);
};

}; //namespace LocalDrive

