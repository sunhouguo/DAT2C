#pragma once

#ifndef DynamicArrayDataBase_H
#define DynamicArrayDataBase_H

//#include <boost/serialization/vector.hpp>
#include <vector>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include "../PublicSupport/Dat2cTypeDef.h"
#include "../FileSystem/Markup.h"

//namespace FileSystem
//{
//	class FileSystem::CMarkup;
//}

namespace PublicSupport {

#define strDataBaseNum "DataBaseNum"
#define strDataBaseIndex "DataNO."
#define strDataBaseNode "DataNode"

template <class Elemtype> class CDynamicArrayDataBase :
	public std::vector<Elemtype>
{
public:
	CDynamicArrayDataBase();
	CDynamicArrayDataBase(int test);
	~CDynamicArrayDataBase();

	int InitDataBase(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);
	size_t getDataBaseSum();
	int setDataBaseSum(size_t val);

	Elemtype * getPointDataPtr(size_t index);
	Elemtype getPointData(size_t index);
	void AddPointData(Elemtype val);

private:
	int LoadXmlCfg(FileSystem::CMarkup & xml);
	void UninitDataBase();
	virtual int InitDataNodes();
};

template <class Elemtype> CDynamicArrayDataBase<Elemtype>::CDynamicArrayDataBase(void)
{
}

template <class Elemtype> CDynamicArrayDataBase<Elemtype>::~CDynamicArrayDataBase(void)
{
}

template <class Elemtype> int CDynamicArrayDataBase<Elemtype>::InitDataNodes()
{
	return 0;
}

template <class Elemtype> int CDynamicArrayDataBase<Elemtype>::InitDataBase(FileSystem::CMarkup & xml)
{
	LoadXmlCfg(xml);

	return 0;
}

template <class Elemtype> void CDynamicArrayDataBase<Elemtype>::UninitDataBase()
{
	std::vector<Elemtype>::clear();
}

template <class Elemtype> size_t CDynamicArrayDataBase<Elemtype>::getDataBaseSum()
{
	return std::vector<Elemtype>::size();
}

template <class Elemtype> int CDynamicArrayDataBase<Elemtype>::setDataBaseSum(size_t val)
{
	std::vector<Elemtype>::resize(val);

	InitDataNodes();

	return 0;
}

template <class Elemtype> Elemtype * CDynamicArrayDataBase<Elemtype>::getPointDataPtr(size_t index) 
{
	if (index < 0 || index >= getDataBaseSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return &(std::vector<Elemtype>::at(index));
}

template <class Elemtype> Elemtype CDynamicArrayDataBase<Elemtype>::getPointData(size_t index)
{
	if (index < 0 || index >= getDataBaseSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return (std::vector<Elemtype>::at(index));
}

template <class Elemtype> void CDynamicArrayDataBase<Elemtype>::AddPointData(Elemtype val)
{
	std::vector<Elemtype>::push_back(val);
}

//xml api
template <class Elemtype> void CDynamicArrayDataBase<Elemtype>::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	xml.AddAttrib(strDataBaseNum,getDataBaseSum());
	xml.AddAttrib(strIndexOrderEnable,strboolFalse);

	xml.IntoElem();
	for (size_t i=0;i<getDataBaseSum();i++)
	{
		xml.AddElem(strDataBaseNode);
		xml.AddAttrib(strDataBaseIndex,i);

		xml.IntoElem();
		std::vector<Elemtype>::at(i).SaveXmlCfg(xml);
		xml.OutOfElem();
	}
	xml.OutOfElem();
}

template <class Elemtype> int CDynamicArrayDataBase<Elemtype>::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	int sum = 0;
	std::string strTmp = xml.GetAttrib(strDataBaseNum);
	boost::algorithm::trim(strTmp);
	try
	{
		sum = boost::lexical_cast<int>(strTmp);
	}
	catch(boost::bad_lexical_cast & e)
	{
		//do nothing,just Ignore sum attrib.
		e.what();
		sum = 0;
	}

	bool bIndexOrderEnable = false;
	strTmp = xml.GetAttrib(strIndexOrderEnable);
	boost::algorithm::trim(strTmp);
	if (boost::algorithm::iequals(strboolTrue,strTmp))
	{
		bIndexOrderEnable = true;
	}

	xml.IntoElem();
	if (sum > 0)
	{
		setDataBaseSum(sum);

		if (bIndexOrderEnable)
		{
			int count = 0;
			while (xml.FindElem(strDataBaseNode) && count<sum)
			{
				int index = -1;
				std::string strTmp = xml.GetAttrib(strDataBaseIndex);
				boost::algorithm::trim(strTmp);
				try
				{
					index = boost::lexical_cast<int>(strTmp);
				}
				catch(boost::bad_lexical_cast& e)
				{
					xml.OutOfElem();

					UninitDataBase();

					std::ostringstream ostr;
					ostr<<"不能将"<<strDataBaseIndex<<"属性转化为数字："<<e.what();

					dat2def_exception err;
					err<<boost::errinfo_errno(count);
					err<<boost::errinfo_type_info_name(ostr.str());

					throw err;
				}

				if (index >= 0 && index < sum)
				{
					xml.IntoElem();
					try
					{
						std::vector<Elemtype>::at(index).LoadXmlCfg(xml);
						//InitDataNode(std::vector<Elemtype>::at(index),xml);
					}
					catch(dat2def_exception& e)
					{
						xml.OutOfElem();
						xml.OutOfElem();

						UninitDataBase();

						e<<boost::errinfo_errno(count);
						throw e;
					}
					xml.OutOfElem();
				}
				else
				{
					xml.OutOfElem();

					UninitDataBase();

					dat2def_exception e;
					e<<boost::errinfo_errno(count);
					e<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
					
					throw e;
				}

				count++;
			}
		}
		else
		{
			int count = 0;
			while (xml.FindElem(strDataBaseNode) && count<sum)
			{
				xml.IntoElem();
				try
				{
					std::vector<Elemtype>::at(count).LoadXmlCfg(xml);
					//InitDataNode(std::vector<Elemtype>::at(count),xml);
				}
				catch(PublicSupport::dat2def_exception & e)
				{
					xml.OutOfElem();
					xml.OutOfElem();

					UninitDataBase();

					e<<boost::errinfo_errno(count);
					throw e;
				}
				xml.OutOfElem();
				count++;
			}
		}
	}
	else
	{
		int count = 0;
		while (xml.FindElem(strDataBaseNode))
		{
			Elemtype datapointTmp;

			xml.IntoElem();
			try
			{
				datapointTmp.LoadXmlCfg(xml);
				//InitDataNode(datapointTmp,xml);
			}
			catch(PublicSupport::dat2def_exception & e)
			{
				xml.OutOfElem();
				xml.OutOfElem();

				UninitDataBase();

				e<<boost::errinfo_errno(count);
				throw e;
			}
			xml.OutOfElem();

			std::vector<Elemtype>::push_back(datapointTmp);

			count++;
		}
	}
	xml.OutOfElem();

	return 0;
}

}; //namespace PublicSupport

#endif
