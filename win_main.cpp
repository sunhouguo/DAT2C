#include <iostream>
#include "./DataBase/SubStation.h"

#if defined(_WIN32)

#define strDevType "WIN32"

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

	return 0;
}

#endif // defined(_WIN32)

