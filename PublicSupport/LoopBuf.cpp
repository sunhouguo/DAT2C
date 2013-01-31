#include <cstring>
#include "LoopBuf.h"

namespace PublicSupport {

CLoopBuf::CLoopBuf(size_t max_buff)
		:buffer_length_(max_buff)
{
	savePtr_ = 0;
	loadPtr_ = 0;

	buf_.reset(new unsigned char[buffer_length_]);
}

CLoopBuf::~CLoopBuf(void)
{
}

int CLoopBuf::getBuf(unsigned char *dst, int count)
{
	using namespace std;

	int max_copy_count;
	int copy_count = 0;

	if (savePtr_ - loadPtr_ >= 0)
	{
		max_copy_count = savePtr_ - loadPtr_;
		if (count < max_copy_count)
		{
			memcpy(dst,buf_.get() + loadPtr_,count);
			copy_count = count;
			loadPtr_ += count;
		}
		else
		{
			memcpy(dst,buf_.get() + loadPtr_,max_copy_count);
			copy_count = max_copy_count;
			loadPtr_ += max_copy_count;
		}
	}
	else
	{
		max_copy_count = savePtr_ + buffer_length_ - loadPtr_;
		if (count <= (buffer_length_ - loadPtr_))
		{
			memcpy(dst,buf_.get() + loadPtr_,count);
			copy_count = count;
			loadPtr_ += count;
		}
		else if (count < max_copy_count)
		{
			int firstcopyNum = buffer_length_ - loadPtr_;
			memcpy(dst,buf_.get() + loadPtr_,firstcopyNum);
			int secondcopyNum = count - firstcopyNum;
			memcpy(dst + firstcopyNum,buf_.get(),secondcopyNum);
			copy_count = count;
			loadPtr_ = secondcopyNum;
		}
		else
		{
			int firstcopyNum = buffer_length_ - loadPtr_;
			memcpy(dst,buf_.get() + loadPtr_,firstcopyNum);
			int secondcopyNum = savePtr_;
			memcpy(dst + firstcopyNum,buf_.get(),secondcopyNum);
			copy_count = max_copy_count;
			loadPtr_ = savePtr_;
		}

	}

	return copy_count;
}

int CLoopBuf::putBuf(unsigned char *src, int count)
{
	if (count > buffer_length_)
	{
		return - 1;
	}

	if ((savePtr_ + count) >= buffer_length_)
	{
		int firstcopyNum = buffer_length_ - savePtr_;
		memcpy(buf_.get() + savePtr_,src,firstcopyNum);
		savePtr_ = 0;
		int secondcopyNum = count - firstcopyNum;
		putBuf(&src[firstcopyNum],secondcopyNum);
	}
	else
	{
		memcpy(buf_.get() + savePtr_,src,count);
		savePtr_ += count;
	}

	return count;
}

int CLoopBuf::copyBuf( unsigned char * dst,int count )
{
	int max_copy_count;
	int copy_count = 0;

	if (savePtr_ - loadPtr_ >= 0)
	{
		max_copy_count = savePtr_ - loadPtr_;
		if (count < max_copy_count)
		{
			memcpy(dst,buf_.get() + loadPtr_,count);
			copy_count = count;
		}
		else
		{
			memcpy(dst,buf_.get() + loadPtr_,max_copy_count);
			copy_count = max_copy_count;
		}
	}
	else
	{
		max_copy_count = savePtr_ + buffer_length_ - loadPtr_;
		if (count <= (buffer_length_ - loadPtr_))
		{
			memcpy(dst,buf_.get() + loadPtr_,count);
			copy_count = count;
		}
		else if (count < max_copy_count)
		{
			int firstcopyNum = buffer_length_ - loadPtr_;
			memcpy(dst,buf_.get() + loadPtr_,firstcopyNum);
			int secondcopyNum = count - firstcopyNum;
			memcpy(dst + firstcopyNum,buf_.get(),secondcopyNum);
			copy_count = count;
		}
		else
		{
			int firstcopyNum = buffer_length_ - loadPtr_;
			memcpy(dst,buf_.get() + loadPtr_,firstcopyNum);
			int secondcopyNum = savePtr_;
			memcpy(dst + firstcopyNum,buf_.get(),secondcopyNum);
			copy_count = max_copy_count;
		}

	}

	return copy_count;
}

int CLoopBuf::popChar(unsigned char & popedChar)
{
	if (savePtr_ == loadPtr_)
	{
		return -1;
	}
	else
	{
		popedChar = buf_[loadPtr_];
		loadPtr_ = (loadPtr_ + 1)%buffer_length_;
	}

	return 0;
}

int CLoopBuf::charNum()
{
	int count = 0;

	if ((savePtr_ - loadPtr_) >= 0)
	{
		count = savePtr_ - loadPtr_;
	}
	else
	{
		count = savePtr_ + buffer_length_ - loadPtr_;
	}

	return count;
}

int CLoopBuf::getLoadPtr()
{
	return loadPtr_;
}

int CLoopBuf::getSavePtr()
{
	return savePtr_;
}

}; //namespace PublicSupport


