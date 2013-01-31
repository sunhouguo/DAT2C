#include <iostream>
#include "./DataBase/SubStation.h"

#ifdef _PCM82X_
#define strDevType "PCM82X"
#endif

#ifdef _TPE3000_
#define strDevType "TPE3000"
#endif

#ifdef _BF518_
#define strDevType "BF518"
#endif


#if !defined(_WIN32)

#ifndef strDevType
#define strDevType "Unknown"
#endif

int main(int argc, char* argv[])
{
	for (;;)
	{
		DataBase::CSubStation sub;
		try
		{
			if(!sub.LoadXmlCfg(MainCfgFile))
			{
				std::cout << "Program Type: "<<strDevType << std::endl;
				std::cout << "Version: " << ver << std::endl;
				
				sub.InitSubStation();
			}
			else
			{
				std::cerr << "can't load main cfg file : " << MainCfgFile << std::endl;
			}
		}

		catch (std::exception& e)
		{
			std::cerr << "main catch exception: " << e.what() << std::endl;
			
			sub.UninitSubStation();
        }
	}
}
#endif // !defined(_WIN32)

