#ifndef Dat2cPublicAPI_H
#define Dat2cPublicAPI_H

#include <string>
#include <math.h>
#include <fstream>
#include <iomanip>
#include <cstdlib>

//#define DebugPrint_
#undef DebugPrint_

//************************************
// Method:    PrintDebug
// FullName:  PrintDebug
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: std::string strVal
//************************************
static int PrintDebug(std::string strVal)
{
#if defined(DebugPrint_)

	std::cout<<strVal<<std::endl;
	return 0;

#else

	return -1;

#endif
}

/* 双字节CRC检验工程算法 高位字节值表 */
const unsigned char CRC16_H[] = 
{
	(unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0,

	(unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41,

	(unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0,

	(unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40,

	(unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1,

	(unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41,

	(unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1,

	(unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41,

	(unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0,

	(unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40,

	(unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1,

	(unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40,

	(unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0,

	(unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40,

	(unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0,

	(unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40,

	(unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0,

	(unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41,

	(unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0,

	(unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41,

	(unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0,

	(unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40,

	(unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1,

	(unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41,

	(unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40, (unsigned char)0x01, (unsigned char)0xC0, (unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x01, (unsigned char)0xC0,

	(unsigned char)0x80, (unsigned char)0x41, (unsigned char)0x00, (unsigned char)0xC1, (unsigned char)0x81, (unsigned char)0x40
} ;

/* 双字节CRC检验工程算法 低位字节值表*/
const unsigned char CRC16_L[] = 
{
	(unsigned char)0x00, (unsigned char)0xC0, (unsigned char)0xC1, (unsigned char)0x01, (unsigned char)0xC3, (unsigned char)0x03, (unsigned char)0x02, (unsigned char)0xC2, (unsigned char)0xC6, (unsigned char)0x06,

	(unsigned char)0x07, (unsigned char)0xC7, (unsigned char)0x05, (unsigned char)0xC5, (unsigned char)0xC4, (unsigned char)0x04, (unsigned char)0xCC, (unsigned char)0x0C, (unsigned char)0x0D, (unsigned char)0xCD,

	(unsigned char)0x0F, (unsigned char)0xCF, (unsigned char)0xCE, (unsigned char)0x0E, (unsigned char)0x0A, (unsigned char)0xCA, (unsigned char)0xCB, (unsigned char)0x0B, (unsigned char)0xC9, (unsigned char)0x09,

	(unsigned char)0x08, (unsigned char)0xC8, (unsigned char)0xD8, (unsigned char)0x18, (unsigned char)0x19, (unsigned char)0xD9, (unsigned char)0x1B, (unsigned char)0xDB, (unsigned char)0xDA, (unsigned char)0x1A,

	(unsigned char)0x1E, (unsigned char)0xDE, (unsigned char)0xDF, (unsigned char)0x1F, (unsigned char)0xDD, (unsigned char)0x1D, (unsigned char)0x1C, (unsigned char)0xDC, (unsigned char)0x14, (unsigned char)0xD4,

	(unsigned char)0xD5, (unsigned char)0x15, (unsigned char)0xD7, (unsigned char)0x17, (unsigned char)0x16, (unsigned char)0xD6, (unsigned char)0xD2, (unsigned char)0x12, (unsigned char)0x13, (unsigned char)0xD3,

	(unsigned char)0x11, (unsigned char)0xD1, (unsigned char)0xD0, (unsigned char)0x10, (unsigned char)0xF0, (unsigned char)0x30, (unsigned char)0x31, (unsigned char)0xF1, (unsigned char)0x33, (unsigned char)0xF3,

	(unsigned char)0xF2, (unsigned char)0x32, (unsigned char)0x36, (unsigned char)0xF6, (unsigned char)0xF7, (unsigned char)0x37, (unsigned char)0xF5, (unsigned char)0x35, (unsigned char)0x34, (unsigned char)0xF4,

	(unsigned char)0x3C, (unsigned char)0xFC, (unsigned char)0xFD, (unsigned char)0x3D, (unsigned char)0xFF, (unsigned char)0x3F, (unsigned char)0x3E, (unsigned char)0xFE, (unsigned char)0xFA, (unsigned char)0x3A,

	(unsigned char)0x3B, (unsigned char)0xFB, (unsigned char)0x39, (unsigned char)0xF9, (unsigned char)0xF8, (unsigned char)0x38, (unsigned char)0x28, (unsigned char)0xE8, (unsigned char)0xE9, (unsigned char)0x29,

	(unsigned char)0xEB, (unsigned char)0x2B, (unsigned char)0x2A, (unsigned char)0xEA, (unsigned char)0xEE, (unsigned char)0x2E, (unsigned char)0x2F, (unsigned char)0xEF, (unsigned char)0x2D, (unsigned char)0xED,

	(unsigned char)0xEC, (unsigned char)0x2C, (unsigned char)0xE4, (unsigned char)0x24, (unsigned char)0x25, (unsigned char)0xE5, (unsigned char)0x27, (unsigned char)0xE7, (unsigned char)0xE6, (unsigned char)0x26,

	(unsigned char)0x22, (unsigned char)0xE2, (unsigned char)0xE3, (unsigned char)0x23, (unsigned char)0xE1, (unsigned char)0x21, (unsigned char)0x20, (unsigned char)0xE0, (unsigned char)0xA0, (unsigned char)0x60,

	(unsigned char)0x61, (unsigned char)0xA1, (unsigned char)0x63, (unsigned char)0xA3, (unsigned char)0xA2, (unsigned char)0x62, (unsigned char)0x66, (unsigned char)0xA6, (unsigned char)0xA7, (unsigned char)0x67,

	(unsigned char)0xA5, (unsigned char)0x65, (unsigned char)0x64, (unsigned char)0xA4, (unsigned char)0x6C, (unsigned char)0xAC, (unsigned char)0xAD, (unsigned char)0x6D, (unsigned char)0xAF, (unsigned char)0x6F,

	(unsigned char)0x6E, (unsigned char)0xAE, (unsigned char)0xAA, (unsigned char)0x6A, (unsigned char)0x6B, (unsigned char)0xAB, (unsigned char)0x69, (unsigned char)0xA9, (unsigned char)0xA8, (unsigned char)0x68,

	(unsigned char)0x78, (unsigned char)0xB8, (unsigned char)0xB9, (unsigned char)0x79, (unsigned char)0xBB, (unsigned char)0x7B, (unsigned char)0x7A, (unsigned char)0xBA, (unsigned char)0xBE, (unsigned char)0x7E,

	(unsigned char)0x7F, (unsigned char)0xBF, (unsigned char)0x7D, (unsigned char)0xBD, (unsigned char)0xBC, (unsigned char)0x7C, (unsigned char)0xB4, (unsigned char)0x74, (unsigned char)0x75, (unsigned char)0xB5,

	(unsigned char)0x77, (unsigned char)0xB7, (unsigned char)0xB6, (unsigned char)0x76, (unsigned char)0x72, (unsigned char)0xB2, (unsigned char)0xB3, (unsigned char)0x73, (unsigned char)0xB1, (unsigned char)0x71,

	(unsigned char)0x70, (unsigned char)0xB0, (unsigned char)0x50, (unsigned char)0x90, (unsigned char)0x91, (unsigned char)0x51, (unsigned char)0x93, (unsigned char)0x53, (unsigned char)0x52, (unsigned char)0x92,

	(unsigned char)0x96, (unsigned char)0x56, (unsigned char)0x57, (unsigned char)0x97, (unsigned char)0x55, (unsigned char)0x95, (unsigned char)0x94, (unsigned char)0x54, (unsigned char)0x9C, (unsigned char)0x5C,

	(unsigned char)0x5D, (unsigned char)0x9D, (unsigned char)0x5F, (unsigned char)0x9F, (unsigned char)0x9E, (unsigned char)0x5E, (unsigned char)0x5A, (unsigned char)0x9A, (unsigned char)0x9B, (unsigned char)0x5B,

	(unsigned char)0x99, (unsigned char)0x59, (unsigned char)0x58, (unsigned char)0x98, (unsigned char)0x88, (unsigned char)0x48, (unsigned char)0x49, (unsigned char)0x89, (unsigned char)0x4B, (unsigned char)0x8B,

	(unsigned char)0x8A, (unsigned char)0x4A, (unsigned char)0x4E, (unsigned char)0x8E, (unsigned char)0x8F, (unsigned char)0x4F, (unsigned char)0x8D, (unsigned char)0x4D, (unsigned char)0x4C, (unsigned char)0x8C,

	(unsigned char)0x44, (unsigned char)0x84, (unsigned char)0x85, (unsigned char)0x45, (unsigned char)0x87, (unsigned char)0x47, (unsigned char)0x46, (unsigned char)0x86, (unsigned char)0x82, (unsigned char)0x42,

	(unsigned char)0x43, (unsigned char)0x83, (unsigned char)0x41, (unsigned char)0x81, (unsigned char)0x80, (unsigned char)0x40
};

//************************************
// Method:    CRC16
// FullName:  CRC16
// Access:    public static 
// Returns:   unsigned short
// Qualifier: 双字节CRC校验
// Parameter: unsigned char * puchMsg
// Parameter: unsigned short usDataLen
//************************************
static unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen)
{

	unsigned char uchCRCHi = 0xff ; // 高CRC字节初始化 

	unsigned char uchCRCLo = 0xff ; // 低CRC 字节初始化

	unsigned uIndex ; // CRC循环中的索引

	while (usDataLen--) // 传输消息缓冲区
	{

		uIndex = uchCRCHi ^ *puchMsg++ ; // 计算CRC 

		uchCRCHi = uchCRCLo ^ CRC16_H[uIndex];

		uchCRCLo = CRC16_L[uIndex] ;

	}

	return (uchCRCHi << 8 | uchCRCLo) ;
}

/* 单字节CRC检验工程算法字节值表 */
const unsigned char CRC8Table[]={
	0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
	157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
	35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
	190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
	70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
	219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
	101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
	248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
	140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
	17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
	175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
	50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
	202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
	87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
	233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
	116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

static unsigned char CRC8(unsigned char *p, unsigned char counter)
{
	unsigned char crc8 = 0;

	for( ; counter > 0; counter--){
		crc8 = CRC8Table[crc8^*p];
		p++;
	}

	return crc8;

}

/* 单字节CRC检验工程算法字节值表 */
const unsigned char Bch_Table[] = {
	0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,0x38,0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D,
	0x70,0x77,0x7E,0x79,0x6C,0x6B,0x62,0x65,0x48,0x4F,0x46,0x41,0x54,0x53,0x5A,0x5D,
	0xE0,0xE7,0xEE,0xE9,0xFC,0xFB,0xF2,0xF5,0xD8,0xDF,0xD6,0xD1,0xC4,0xC3,0xCA,0xCD,
	0x90,0x97,0x9E,0x99,0x8C,0x8B,0x82,0x85,0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,0xBD,
	0xC7,0xC0,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA,
	0xB7,0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,0x8F,0x88,0x81,0x86,0x93,0x94,0x9D,0x9A,
	0x27,0x20,0x29,0x2E,0x3B,0x3C,0x35,0x32,0x1F,0x18,0x11,0x16,0x03,0x04,0x0D,0x0A,
	0x57,0x50,0x59,0x5E,0x4B,0x4C,0x45,0x42,0x6F,0x68,0x61,0x66,0x73,0x74,0x7D,0x7A,
	0x89,0x8E,0x87,0x80,0x95,0x92,0x9B,0x9C,0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4,
	0xF9,0xFE,0xF7,0xF0,0xE5,0xE2,0xEB,0xEC,0xC1,0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4,
	0x69,0x6E,0x67,0x60,0x75,0x72,0x7B,0x7C,0x51,0x56,0x5F,0x58,0x4D,0x4A,0x43,0x44,
	0x19,0x1E,0x17,0x10,0x05,0x02,0x0B,0x0C,0x21,0x26,0x2F,0x28,0x3D,0x3A,0x33,0x34,
	0x4E,0x49,0x40,0x47,0x52,0x55,0x5C,0x5B,0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,0x63,
	0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x13,
	0xAE,0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,0x96,0x91,0x98,0x9F,0x8A,0x8D,0x84,0x83,
	0xDE,0xD9,0xD0,0xD7,0xC2,0xC5,0xCC,0xCB,0xE6,0xE1,0xE8,0xEF,0xFA,0xFD,0xF4,0xF3
};

static unsigned char CRC8_CDT(unsigned char *buffer,unsigned char count)
{
	unsigned char temp = 0;

	for( int i= 0; i<count; i++ )
	{
		temp = temp ^ buffer[i];
		temp = Bch_Table[temp];
	}

	temp = temp^0xff;

	return temp;
}

//************************************
// Method:    CalcCheckSumWord
// FullName:  CalcCheckSumWord
// Access:    public static 
// Returns:   int
// Qualifier: 按word计算的校验和
// Parameter: unsigned char * Ptr
// Parameter: size_t count
//************************************
static int CalcCheckSumWord(unsigned char * Ptr,size_t count)
{
	if (count < 0)
	{
		return -1;
	}

	unsigned short sum = 0;

	for (size_t i=0;i<count;i += 2)
	{
		sum += Ptr[i]+Ptr[i + 1] * 0x100;
	}

	return sum;
}

//************************************
// Method:    CalcCheckSumByte
// FullName:  CalcCheckSumByte
// Access:    public static 
// Returns:   int
// Qualifier: 按byte计算的校验和
// Parameter: unsigned char * Ptr
// Parameter: size_t count
//************************************
static int CalcCheckSumByte(unsigned char * Ptr,size_t count)
{
	if (count < 0)
	{
		return -1;
	}

	unsigned char sum = 0;

	for (size_t i=0;i<count;i++)
	{
		sum += Ptr[i];
		//std::cout<<"sum = "<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)sum;
		//std::cout<<" curBuf = "<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)Ptr[i]<<std::endl;
	}

	return sum;
}

//************************************
// Method:    ValToBuf
// FullName:  ValToBuf
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: unsigned char * buf
// Parameter: size_t val
// Parameter: size_t lenth
// Parameter: bool bLowByteFirst
//************************************
static int ValToBuf(unsigned char * buf, size_t val, size_t lenth,bool bLowByteFirst = true)
{
	if (lenth <= 0)
	{
		return 0;
	}

	for (size_t i=0;i<lenth;i++)
	{
		if (bLowByteFirst)
		{
			buf[i] = (val >> (i * 8)) & 0xff;
		}
		else
		{
			buf[lenth - (i + 1)] = (val >> (i * 8)) & 0xff;
		}
		
	}

	return lenth;
}

//************************************
// Method:    BufToVal
// FullName:  BufToVal
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: unsigned char * buf
// Parameter: size_t lenth
// Parameter: bool bLowByteFirst
//************************************
static size_t BufToVal(unsigned char * buf, size_t lenth,bool bLowByteFirst = true)
{
	unsigned long sum = 0;

	if (lenth <= 0)
	{
		return 0;
	}

	for (size_t i=0;i<lenth;i++)
	{
		if (bLowByteFirst)
		{
			sum += buf[i] << (i * 8);
		}
		else
		{
			sum += buf[lenth - (i + 1)] << (i * 8);
		}
		
	}

	return sum;
}

//************************************
// Method:    ValToBcd
// FullName:  ValToBcd
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: unsigned char * buf
// Parameter: unsigned long long val
// Parameter: size_t lenth
// Parameter: bool bLowByteFirst
//************************************
static int ValToBcd(unsigned char * buf, unsigned long long val, size_t lenth,bool bLowByteFirst = true)
{
	if (lenth <= 0)
	{
		return 0;
	}

	for (size_t i=0;i<lenth;i++)
	{
		unsigned char low = (val / (unsigned long)pow((double)10,(double)(i * 2))) % 10;
		unsigned char high = (val / (unsigned long)pow((double)10,(double)(i * 2 + 1))) % 10;

		if (bLowByteFirst)
		{
			buf[i] = low | (high << 4);
		}
		else
		{
			buf[lenth - (i + 1)] = low | (high << 4);
		}

	}

	return lenth;
}

//************************************
// Method:    BcdToVal
// FullName:  BcdToVal
// Access:    public static 
// Returns:   unsigned long long
// Qualifier:
// Parameter: const unsigned char * buf
// Parameter: size_t lenth
// Parameter: bool bLowByteFirst
//************************************
static unsigned long long BcdToVal(const unsigned char * buf, size_t lenth,bool bLowByteFirst = true)
{
	unsigned long long sum = 0;

	if (lenth <= 0)
	{
		return 0;
	}

	for (size_t i=0;i<lenth;i++)
	{
		if (bLowByteFirst)
		{
			sum += (((buf[i] & 0x0f) % 10)+ ((((buf[i] >> 4) & 0x0f) % 10) * 10)) * (unsigned long)pow((double)10,(double)2*i);
		}
		else
		{
			sum += (((buf[lenth - (i + 1)] & 0x0f) % 10)+ ((((buf[lenth - (i + 1)] >> 4) & 0x0f) % 10) * 10)) * (unsigned long)pow((double)10,(double)2*i);
		}

	}

	return sum;
}

//************************************
// Method:    ValToBcdWithHighBitSign
// FullName:  ValToBcdWithHighBitSign
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: unsigned char * buf
// Parameter: long long val
// Parameter: size_t lenth
// Parameter: bool bSigned
// Parameter: bool bLowByteFirst
//************************************
static int ValToBcdWithHighBitSign(unsigned char * buf, long long val, size_t lenth,bool bSigned,bool bLowByteFirst = true)
{
	if (lenth <= 0)
	{
		return 0;
	}

	for (size_t i=0;i<lenth;i++)
	{
		unsigned char low = (val / (unsigned long)pow((double)10,(double)(i * 2))) % 10;
		unsigned char high = (val / (unsigned long)pow((double)10,(double)(i * 2 + 1))) % 10;

		if (i == lenth - 1)
		{
			if (bLowByteFirst)
			{
				if(bSigned)
				{
					buf[i] = low | (high << 4) | 0x80;
				}
				else
				{
					buf[i] = (low | (high << 4)) & 0x7f;
				}
			}
			else
			{
				if(bSigned)
				{
					buf[lenth - (i + 1)] = low | (high << 4) | 0x80;
				}
				else
				{
					buf[lenth - (i + 1)] = (low | (high << 4)) & 0x7f;
				}
			}
		}
		else
		{
			if (bLowByteFirst)
			{
				buf[i] = low | (high << 4);
			}
			else
			{
				buf[lenth - (i + 1)] = low | (high << 4);
			}
		}
	}

	return lenth;
}

//************************************
// Method:    BcdToValWithHighBitSign
// FullName:  BcdToValWithHighBitSign
// Access:    public static 
// Returns:   long long
// Qualifier:
// Parameter: const unsigned char * buf
// Parameter: size_t lenth
// Parameter: bool & bSigned
// Parameter: bool bLowByteFirst
//************************************
static long long BcdToValWithHighBitSign(const unsigned char * buf, size_t lenth,bool & bSigned, bool bLowByteFirst = true)
{
	unsigned long long sum = 0;

	if (lenth <= 0)
	{
		return 0;
	}

	for (size_t i=0;i<lenth;i++)
	{
		if(i == lenth - 1)
		{
			if (bLowByteFirst)
			{
				if ((buf[i] & 0x80) > 0)
				{
					bSigned = true;
				}
				else
				{
					bSigned = false;
				}
				unsigned char sBuf = buf[i] & 0x7f;

				sum += (((sBuf & 0x0f) % 10)+ ((((sBuf >> 4) & 0x0f) % 10) * 10)) * (unsigned long)pow((double)10,(double)2*i);
			}
			else
			{
				if ((buf[lenth - (i + 1)] & 0x80) > 0)
				{
					bSigned = true;
				}
				else
				{
					bSigned = false;
				}
				unsigned char sBuf = buf[lenth - (i + 1)] & 0x7f;

				sum += (((sBuf & 0x0f) % 10)+ ((((sBuf >> 4) & 0x0f) % 10) * 10)) * (unsigned long)pow((double)10,(double)2*i);
			}
		}
		else
		{
			if (bLowByteFirst)
			{
				sum += (((buf[i] & 0x0f) % 10)+ ((((buf[i] >> 4) & 0x0f) % 10) * 10)) * (unsigned long)pow((double)10,(double)2*i);
			}
			else
			{
				sum += (((buf[lenth - (i + 1)] & 0x0f) % 10)+ ((((buf[lenth - (i + 1)] >> 4) & 0x0f) % 10) * 10)) * (unsigned long)pow((double)10,(double)2*i);
			}
		}
	}

	return sum;
}

//************************************
// Method:    getFileStr
// FullName:  getFileStr
// Access:    public static 
// Returns:   std::string
// Qualifier: 
// Parameter: std::string filePath
// Parameter: int lineNum
//************************************
static std::string getFileStr(std::string filePath,int lineNum)
{
	std::ifstream infs(filePath.c_str(),std::ios::in);

	if (infs.is_open())
	{
		std::string strTmp = "";
		size_t count = 0;
		while(infs >> strTmp)
		{
			if(++count == lineNum)
			{
				infs.close();
				infs.clear();

				return strTmp;
			}
		}
	}

	infs.close();
	infs.clear();

	return "";
}

//************************************
// Method:    TransStrToHex
// FullName:  TransStrToHex
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: std::string strval
// Parameter: unsigned char * hex_buf
// Parameter: int buf_length
//************************************
static int TransStrToHex( std::string strval,unsigned char * hex_buf, int buf_length )
{
	int hex_num = (strval.length() + 1) / 2;
	std::string::iterator it = strval.begin();

	int count=0;
	for(;count<(hex_num < buf_length ? hex_num : buf_length);count++)
	{
		char strTmp[3];
		strTmp[0] = *it++;
		strTmp[1] = *it++;
		strTmp[2] = '\0';
		//std::cout<<strTmp;

		std::stringstream ss;
		ss <<std::hex << strTmp;
		//std::cout<<ss.str();

		short s_tmp;
		ss >> s_tmp;
		hex_buf[count] = static_cast<unsigned char>(s_tmp);
		//std::cout<<((short)hex_buf[i])<<" ";
	}

	return count;
}

//************************************
// Method:    TransHexToStr
// FullName:  TransHexToStr
// Access:    public static 
// Returns:   std::string
// Qualifier:
// Parameter: unsigned char * hex_buf
// Parameter: int buf_length
//************************************
static std::string TransHexToStr(unsigned char * hex_buf,int buf_length)
{
	std::stringstream ostr;

	for (int i=0;i<buf_length;i++)
	{
		short s = hex_buf[i];
		ostr<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<s;
	}

	return ostr.str();
}

//************************************
// Method:    ByteYearToWord
// FullName:  ByteYearToWord
// Access:    public static 
// Returns:   unsigned short
// Qualifier:
// Parameter: unsigned char year
//************************************
static unsigned short ByteYearToWord(unsigned char year)
{
	unsigned short ret = year;

	if (year < 70)
	{
		ret = 2000 + year;
	}
	else
	{
		ret = 1900 + year;
	}

	return ret;
}

//************************************
// Method:    RandSeed
// FullName:  RandSeed
// Access:    public static 
// Returns:   int
// Qualifier: 产生0到传入值val之间的随机数
// Parameter: int val
//************************************
static int RandSeed(int val)
{
	time_t seed;

	srand((unsigned)time(&seed));

	if (val == 0)
	{
		return rand();
	} 
	else
	{
		return rand() % val;
	}
}

#endif //#ifndef Dat2cPublicAPI_H

