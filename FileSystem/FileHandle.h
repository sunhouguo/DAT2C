#pragma once
#include <string>

#define  FILE_LENGTH   1024*100

namespace FileSystem
{
	class CFileHandle
	{
	public:
		CFileHandle(std::string filename);
		~CFileHandle(void);

	public:

		unsigned long GetRemainLength(void);
		unsigned long GetTotalLength(void);
		unsigned int SetTotalLength(unsigned long Totallen);
		std::string GetFileName(void);

		int HandleWrite(void);
		int HandleWriteByByte(void);
		int HandleRead(void);
		int HandleReadFor533Pro(void);

		int OutFile(unsigned char * filedata,int length);
		int GetFile(unsigned char * filedata,int length);

	private:

		int WriteToFile();
		int WriteToFileByByte();
		int GetFromFile();
		int GetFromFileFor533Pro();
		char TransToChar(char data);

	protected:
		enum
		{

		};

	private:
		unsigned char FileDataBuf[FILE_LENGTH];

		std::string FileName;
		unsigned long FileLoadPtr_;
		unsigned long FileRemain_;
		unsigned long totallen_;
	};
}


