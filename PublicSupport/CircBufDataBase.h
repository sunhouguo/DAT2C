#pragma once

#ifndef CircBufDataBase_H
#define CircBufDataBase_H

#include <boost/thread/mutex.hpp>

namespace PublicSupport {

template <class Elemtype> class CCircBufDataBase
{
public:
	CCircBufDataBase(void);
	~CCircBufDataBase(void);

	void InitLoadPtr(size_t & loadPtr);
	int loadPtrPlus(size_t & loadPtr,size_t loadnum);
	int getPointDataNum(size_t loadPtr);
	void putPointData(Elemtype val);
	Elemtype getPointData(size_t loadPtr);
	int savePointDatas(const Elemtype * pointDataBuf,size_t savenum);
	int loadPointDatas(size_t & loadPtr,Elemtype* pointDataBuf,size_t loadnum);
	int copyPointDatas(size_t loadPtr,Elemtype* pointDataBuf,size_t loadnum);

private:
	void savePtrPlus();
	int loadPtrPlus(size_t & loadPtr);

private:
	enum
	{
		max_length = 2048,
	};

	size_t savePtr_;
	Elemtype pointDatas_[max_length];

	boost::mutex mutex_;
};

template <class Elemtype> CCircBufDataBase<Elemtype>::CCircBufDataBase(void)
{
	savePtr_ = 0;
}

template <class Elemtype> CCircBufDataBase<Elemtype>::~CCircBufDataBase(void)
{
}

template <class Elemtype> void CCircBufDataBase<Elemtype>::InitLoadPtr(size_t & loadPtr)
{
	loadPtr = savePtr_;
}

template <class Elemtype> int CCircBufDataBase<Elemtype>::loadPtrPlus(size_t & loadPtr)
{
	int sum = getPointDataNum(loadPtr);
	if (sum <= 0)
	{
		return -1;
	}

	/*
	if (loadPtr == max_length - 1)
	{
		loadPtr = 0;
	}
	else
	{
		loadPtr++;
	}
	*/

	loadPtr = (loadPtr + 1) % max_length;

	return 0;
}

template <class Elemtype> int CCircBufDataBase<Elemtype>::loadPtrPlus(size_t & loadPtr,size_t loadnum)
{
	int sum = getPointDataNum(loadPtr);
	if (sum < 0)
	{
		return sum;
	}

	if (loadnum > (size_t)sum)
	{
		loadnum = sum;
	}

	loadPtr = (loadPtr + loadnum) % max_length;

	return loadnum;
}

template <class Elemtype> void CCircBufDataBase<Elemtype>::savePtrPlus()
{
	if (savePtr_ >= max_length - 1)
	{
		boost::unique_lock<boost::mutex> uLock(mutex_);

		savePtr_ = 0;
	}
	else
	{
		boost::unique_lock<boost::mutex> uLock(mutex_);

		savePtr_++;
	}
}

template <class Elemtype> int CCircBufDataBase<Elemtype>::getPointDataNum(size_t loadPtr)
{
	if (loadPtr >= max_length)
	{
		return -1;
	}

	//mutex_.lock();

	int var = savePtr_ - loadPtr;

	//mutex_.unlock();

	if ( var >= 0 )
	{
		return var;
	}
	else
	{
		return max_length + var;
	}
}

template <class Elemtype> void CCircBufDataBase<Elemtype>::putPointData(Elemtype val)
{
	pointDatas_[savePtr_] = val;

	savePtrPlus();
}

template <class Elemtype> Elemtype CCircBufDataBase<Elemtype>::getPointData(size_t loadPtr)
{
	if (loadPtr >= max_length)
	{
		loadPtr = loadPtr % max_length;
	}

	return pointDatas_[loadPtr];
}

template <class Elemtype> int CCircBufDataBase<Elemtype>::savePointDatas(const Elemtype * pointDataBuf,size_t savenum)
{
	if (savenum > max_length)
	{
		savenum = max_length;
	}

	size_t i;

	for (i=0; i<savenum;i++)
	{
		pointDatas_[savePtr_] = pointDataBuf[i];
		savePtrPlus();
	}

	return i;
}

template <class Elemtype> int CCircBufDataBase<Elemtype>::loadPointDatas(size_t & loadPtr, Elemtype *pointDataBuf, size_t loadnum)
{
	int sum = getPointDataNum(loadPtr);
	if (sum <= 0 )
	{
		return sum;
	}

	if (loadnum > (size_t)sum)
	{
		loadnum = sum;
	}

	size_t i;

	for (i=0; i<loadnum; i++)
	{
		pointDataBuf[i] = pointDatas_[loadPtr];
		loadPtrPlus(loadPtr);
	}

	return i;
}

template <class Elemtype> int CCircBufDataBase<Elemtype>::copyPointDatas(size_t loadPtr, Elemtype *pointDataBuf, size_t loadnum)
{
	int sum = getPointDataNum(loadPtr);
	if (sum <= 0 )
	{
		return sum;
	}

	if (loadnum > (size_t)sum)
	{
		loadnum = sum;
	}

	size_t i;

	for (i=0; i<loadnum; i++)
	{
		pointDataBuf[i] = pointDatas_[loadPtr];
		loadPtrPlus(loadPtr);
	}

	return i;
}

} //namespace PublicSupport

#endif
