#ifndef __MISC_SHARED_CACHEENTRY_H__
#define __MISC_SHARED_CACHEENTRY_H__

#include <system/time/time.h>

namespace DiskCache
{

class Cache;

/// contains a single data channel
class CacheData
{
public:
	Util::C_String	m_Name;
	CacheKey		m_Data;

	template<typename T>
	void Serialise(T &ar)
	{
		ar << m_Name << m_Data ;
	}

	template<typename T>
	void Deserialise(T &ar)
	{
		ar >> m_Name >> m_Data ;
	}
};

/// contains the key and data channels for a cache entry
class CacheEntry
{
	friend class Cache;
	friend class CacheWriteStream;
protected:
	/// protected constructor for writing
							CacheEntry(Cache *pOwner, bool bWriting, CacheKey key, Time::C_Time timeStamp = Time::GetCurrentTime());

public:
	/// destructor
							~CacheEntry();

	/// set data into a cache entry. multiple pieces of data can be set
	Sys::I_BinaryStream		*SetData(const char *pszType);

	/// get data from a cache entry
	Sys::I_BinaryStream		*GetData(const char *pszType);

	/// get the timestamp of the cache entry
	Time::C_Time			GetTime();

	/// set user data
	void					SetUserData(U32 i);

	/// get user data
	U32						GetUserData();

	template<typename T>
	void Serialise(T &ar)
	{
		ar << m_Key << m_TimeStamp << m_UserData << m_Data ;
	}

	template<typename T>
	void Deserialise(T &ar)
	{
		ar >> m_Key >> m_TimeStamp >> m_UserData >> m_Data ;
	}

protected:
	/// internal function for setting binary data into m_Data
	void					SetDataInternal(const char *pszType, U8 *pData, int c);

	/// find the index of this data type in the m_Data vector
	int						FindDataType(const char *psz);

	Cache					*m_pCache;
	bool					m_bWriting;	// true if the cache entry is to be written when the destructor is called

	CacheKey				m_Key;
	Ut::Vector<CacheData>	m_Data;
	Time::C_Time			m_TimeStamp;
	U32						m_UserData;
};

}

#endif
