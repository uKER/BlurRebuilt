#ifndef __MISC_SHARED_CACHEKEY_H__
#define __MISC_SHARED_CACHEKEY_H__

#include <core/utility/smartptr.h>

namespace DiskCache
{

// cache keys are arbitrary sized arrays of binary data
struct CacheKeyObj : public Sys::RefObj
{
	int iSize;
	U8 *pData;

	/// default constructor
	CacheKeyObj() : pData(NULL), iSize(0) {};
	/// construct key from binary data
	CacheKeyObj(U8 *_pData, int _iSize)
	{
		iSize = _iSize;
		pData = new U8[iSize];
		memcpy(pData, _pData, iSize);
	}
	// destructor
	~CacheKeyObj();

	int Compare(CacheKeyObj &t);

	template<typename T>
	void Serialise(T &ar)
	{
		ar << iSize;
		ar.Write(pData, iSize);
	}

	template<typename T>
	void Deserialise(T &ar)
	{
		assert(!pData && !iSize);
		ar >> iSize;
		pData = new U8[iSize];
		ar.Read(pData, iSize);
	}
};

/// as CacheKey constructor above, but automatically generates the key by serialising an object
template<typename T>
Sys::RefPtr<CacheKeyObj> GenerateCacheKey(T &obj)
{
	Sys::C_MemoryStream mem(1024);
	Sys::BinarySerialiser<Ut::NoEndianSwap> ar(mem);
	ar << obj;
	return CacheKey( new CacheKeyObj(mem.GetBufferPtr(), (int)mem.GetBufferSize()) );
}

typedef Sys::RefPtr<CacheKeyObj> CacheKey; 

inline bool operator<(const CacheKey &a, const CacheKey &b)
{
	return a->Compare(b)<0;
}

template<typename Archive>
void Serialise(Archive &ar, CacheKey &key)
{
	ar << *key.RawPtr();
}

template<typename T>
void Deserialise(T &ar, CacheKey &key)
{
	key = new CacheKeyObj;
	ar >> *key.RawPtr();
}

}

#endif
