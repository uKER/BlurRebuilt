#include "MiscSharedPch.h"

#include <system/debuglog.h>
#include <system/time/time.h>
#include "misc_shared\cache\cache.h"
#include "misc_shared\cache\cachestream.h"

namespace DiskCache
{

/// protected constructor for creating a cache entry for writing into the cache
CacheEntry::CacheEntry(Cache *pOwner, bool bWriting, CacheKey key, Time::C_Time time) :
	m_pCache( pOwner ),
	m_Key( key ),
	m_TimeStamp( time )
{
	m_bWriting = bWriting;
}

/// destructor - writes the cache entry to the cache if m_bWriting is true
CacheEntry::~CacheEntry()
{
	if (m_bWriting)
		m_pCache->Write(this);
}

/// set data into a cache entry. multiple pieces of data can be set
Sys::I_BinaryStream	*CacheEntry::SetData(const char *pszType)
{
	return new CacheWriteStream(this, pszType);
}

/// internal function for setting binary data into m_Data
void CacheEntry::SetDataInternal(const char *pszType, U8 *pData, int c)
{
	// find data type
	int i = FindDataType(pszType);
	if (i == -1)
	{
		// create new data type
		i = m_Data.Size();
		m_Data.PushBack();
		m_Data[i].m_Name = pszType;
	}

	// set data
	m_Data[i].m_Data = CacheKey(new CacheKeyObj(pData, c));
}

/// find the index of this data type in the m_Data vector
int	CacheEntry::FindDataType(const char *psz)
{
	// does this type of data already exist?
	for (int i=0; i<m_Data.Size(); i++)
	{
		if (!strcmp(psz, m_Data[i].m_Name))
			return i;
	}
	return -1;
}

/// get data from a cache entry
Sys::I_BinaryStream	*CacheEntry::GetData(const char *pszType)
{
	int idx = FindDataType(pszType);
	if (idx == -1)
		return NULL;

	CacheKey &data = m_Data[idx].m_Data;
	return new Sys::C_MemoryStream(data->pData, data->iSize);
}

/// get the timestamp of the cache entry
Time::C_Time CacheEntry::GetTime()
{
	return m_TimeStamp;
}

/// set user data
void CacheEntry::SetUserData(U32 i)
{
	m_UserData = i;
}

/// get user data
U32 CacheEntry::GetUserData()
{
	return m_UserData;
}

}
