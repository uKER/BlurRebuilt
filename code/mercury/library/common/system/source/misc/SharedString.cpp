#include "SystemPch.h"
#include "btl/Utility/string.h"
#include "btl/Containers/chainedhashmap.h"
#include "btl/Allocators/stackallocator.h"
#include "btl/Allocators/pool.h"

#include "system/misc/SharedString.h"

namespace SharedString
{

class StringReferenceCounter
{
public:

	StringReferenceCounter() : m_stringCount(0), m_memoryUsed(0), m_originalSize(0), m_collisions(0) {}

	const String Insert(const char* string, U32 length)
	{
		U32 hash = Btl::Hash(string);
		StringMap::InsertionReturnType r = m_strings.Insert(hash);

		m_originalSize += length + 1;
		m_stringCount++;

		if (r.inserted)
		{
			char* pString = m_Allocator.Allocate<char>(length+1);
			assert(pString);

			m_memoryUsed += length+1;
			strcpy(pString, string);	
		
			r.iterator->GetValue() = pString;
		}

		char* pSharedString = r.iterator->GetValue();

		if (strcmp(string, pSharedString) != 0)
		{
			m_collisions++;
			pSharedString = m_Allocator.Allocate<char>(length+1);
		}
		
		return String(pSharedString);
	}

private:
	typedef Btl::ChainedHashMap<U32, char*, Btl::HashKeyOpDefault<U32>, Btl::DynamicPool<Btl::NullType> > StringMap;

	StringMap m_strings;
	Btl::DynamicStackAllocator m_Allocator;

	S32 m_stringCount;
	S32 m_memoryUsed;
	S32 m_originalSize;
	S32 m_collisions;
};

static StringReferenceCounter g_StringRefCount;

String::String(const char *psz)
: m_psz(psz)
{
}

const String GetString(const char *psz)
{
	return g_StringRefCount.Insert(psz, strlen(psz));
}

String::String(const String &other)
{
	m_psz = other.m_psz;
}

}
