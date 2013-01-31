#include "FileHandle.h"

#include <fstream>
#include <stdio.h> 
#include <iostream>
#include <cstring>
#include<sstream>

using namespace std;

namespace FileSystem
{

CFileHandle::CFileHandle(std::string filename)
{
	FileLoadPtr_ = 0;
	FileRemain_  = 0;
	totallen_ = 0;

	memset(FileDataBuf,0,FILE_LENGTH);

	FileName = filename;
}

CFileHandle::~CFileHandle(void)
{
	
}

int CFileHandle::HandleRead(void)
{
	return GetFromFile();
}

int CFileHandle::HandleReadFor533Pro(void)
{
	return GetFromFileFor533Pro();
}

int CFileHandle::HandleWrite(void)
{	
	return WriteToFile();
}

int CFileHandle::HandleWriteByByte(void)
{	
	return WriteToFileByByte();
}

int CFileHandle::OutFile(unsigned char * filedata,int length)
{
	for (int i = 0;i < length;i ++)
	{
		FileDataBuf[FileLoadPtr_ ++] = filedata[i];
	}
	return FileLoadPtr_;
}

int CFileHandle::GetFile(unsigned char * filedata,int length)
{
	int Readlen = 0;
	if (FileRemain_ > length)
	{
		Readlen = length;
		FileRemain_ -= length;
	} 
	else
	{
		Readlen = FileRemain_;
		FileRemain_ = 0;
	}

	for (int i = 0;i < Readlen; i ++)
	{
		filedata[i] = FileDataBuf[FileLoadPtr_ ++];
	}

	return Readlen;
}

unsigned long CFileHandle::GetRemainLength(void)
{
	return FileRemain_;
}

unsigned long CFileHandle::GetTotalLength(void)
{
	return totallen_;
}

unsigned int CFileHandle::SetTotalLength(unsigned long Totallen)
{
	totallen_ = Totallen;
	return 0;
}

std::string CFileHandle::GetFileName(void)
{
	return FileName;
}

int CFileHandle::WriteToFile()
{
	std::ofstream fout;
	fout.open(FileName.c_str(),std::ios::out);

	if(fout.is_open())
	{
		fout.seekp(0,std::ios::beg);
		fout<<FileDataBuf<<"\n";
		fout<<std::flush;
	}

	fout.close();
	fout.clear();

	return FileLoadPtr_;
}

int CFileHandle::WriteToFileByByte()
{
	std::ofstream fout;
	fout.open(FileName.c_str(),std::ios::out);
	
	if(fout.is_open())
	{
		fout.seekp(0,std::ios::beg);
		for (int i = 0;i < FileLoadPtr_;i ++)
		{
			fout.put(FileDataBuf[i]);
		}
		fout<<std::flush;
	}

	fout.close();
	fout.clear();

	return FileLoadPtr_;
}

int CFileHandle::GetFromFile()
{
	char data;
	FileLoadPtr_ = 0;
	std::ifstream fin(FileName.c_str(),std::ios::in);

	if(fin.is_open())
	{
		while(!fin.eof())
		{
			fin.get(data);
			FileDataBuf[FileLoadPtr_++] = data;
		}
		fin.close();
		totallen_ = FileLoadPtr_;
		FileRemain_ = FileLoadPtr_;
		FileLoadPtr_= 0;

		return FileRemain_;
	}

	return -1;
    
}

int CFileHandle::GetFromFileFor533Pro()
{
    ifstream fin;

	std::string strHelper;

	FileLoadPtr_ = 0;

	fin.open(FileName.c_str());

	if(fin.is_open())
	{
		for(std::string str;getline(fin,str);)
		{
			istringstream sin(str);
			while(sin>>strHelper)
			{
				int len_ = strHelper.size();
				if ((len_ > 12)&&(strHelper[8] == 48))//只读取第9位为“0”的数据
				{
					for (int i = 9;i < len_ - 2; i += 2)//将读出来的数据丢掉前9个和最后2个,并将所得数据转换
					{
						FileDataBuf[FileLoadPtr_++] = ((TransToChar(strHelper[i]))<<4) + (TransToChar(strHelper[i + 1]));
					}
				} 
			}
		}
		fin.close();

		totallen_ = FileLoadPtr_;//8096为正确
		FileRemain_ = FileLoadPtr_;
		FileLoadPtr_= 0;

		return FileRemain_;
	}

	return -1;
}

char CFileHandle::TransToChar(char data)
{
	if ((data >= 65)&&(data <= 70))
	{
		return (data -= 55);
	} 
	else
	{
		return (data -= 48);
	}
}

}

