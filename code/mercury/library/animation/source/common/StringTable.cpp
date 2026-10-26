#include "stdafx.h"
#include "StringTable.h"
//#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/memory/MemoryTracker.h>
#include <animation/common/StringUtil.h>

namespace Animation
{

#define kStringTableMemoryTag	"mercury.animation.stringtable"

namespace
{
	bool StringCompareCaseSensitive( const char* str1, const char* str2 )
	{
		BIZ_ASSERT( str1 );
		BIZ_ASSERT( str2 );
		
		const CHAR8* strA = str1;
		const CHAR8* strB = str2;

		while ( *strA && *strB )
		{
			if ( *strA != *strB )
			{
				return false;
			}
			++strA;
			++strB;
		}
		
		return !*strA && !*strB;
	}

	bool StringCompareCaseInsensitive( const char* str1, const char* str2 )
	{
		BIZ_ASSERT( str1 );
		BIZ_ASSERT( str2 );
		
		const CHAR8* strA = str1;
		const CHAR8* strB = str2;

		while ( *strA && *strB )
		{
			if ( StringUtil::ToLower( *strA ) != StringUtil::ToLower( *strB ) )
			{
				return false;
			}
			
			++strA;
			++strB;
		}
		
		return !*strA && !*strB;
	}

	static const CHAR8* kNullString = "<null>";
	
	// Jenkins one-at-a-time hash
	// http://en.wikipedia.org/wiki/Hash_table
	U32 HashString(const CHAR8* str, U32* outStringLength = NULL)
	{
		if (str == NULL) return 0;
		
		U32 hash = 0;
		const CHAR8* pChar = str;
		while (*pChar != 0)
		{
			hash += *pChar;
			hash += (hash << 10);
			hash ^= (hash >> 6);
			++pChar;
		}
		if (outStringLength != NULL)
		{
			*outStringLength = pChar - str;
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		
		return hash;
	}
	
	class C_AutoLock
	{
	public:
		C_AutoLock(MT::Lock& lock): m_lock(lock)
		{
			m_lock.Enter();
		}
		
		~C_AutoLock()
		{
			m_lock.Exit();
		}
		
	private:
		MT::Lock&	m_lock;
	};
}

C_StringTable::C_StringTable( U32 maxStrings, U32 maxStringDataSize, bool isCaseSensitive ):
	m_numStrings( 0 ),
	m_maxStrings( maxStrings ),
	m_stringInfo( NULL ),
	m_stringDataSize( 0 ),
	m_maxStringDataSize( maxStringDataSize ),
	m_stringData( NULL ),
	m_hashTable( NULL ),
	m_hashTableSize( 0 )
{
	MEMORY_TRACKER_PUSH( kStringTableMemoryTag );
	
	m_stringInfo = new StringInfo[ m_maxStrings ];
	m_stringData = new CHAR8[ m_maxStringDataSize ];
	
	Rehash();
	
	SetCaseSensitive( isCaseSensitive );
	
	MEMORY_TRACKER_POP( kStringTableMemoryTag );
}


C_StringTable::~C_StringTable()
{
	delete[] m_stringInfo;
	delete[] m_stringData;
	delete[] m_hashTable;
}
		
void C_StringTable::Clear()
{
	C_AutoLock lock(m_lock);
	
	m_numStrings = 0;
	m_stringDataSize = 0;
	delete[] m_stringInfo;
	m_stringInfo = 0;
	delete[] m_stringData;
	m_stringData = 0;
	delete[] m_hashTable;
	m_hashTable = 0;
}

U32 C_StringTable::Insert( const CHAR8* string )
{
	if ( !string )
	{
		return 0;
	}
	
	C_AutoLock lock(m_lock);	
	
	U32 hash;
	U32 strLength;
	U32 stringId = FindInHashTable( string, hash, strLength );
	
	if (stringId != 0)
	{
		return stringId;
	}
	
	S32 length = strLength + 1;
	
	EnsureCapacity( length );
	
	U32 index = m_numStrings;
	++m_numStrings;
	
	m_stringInfo[index].offset = m_stringDataSize;
	m_stringInfo[index].hash = hash;
	m_stringDataSize += length;
	
	CHAR8* dest = &m_stringData[m_stringInfo[index].offset];
	const CHAR8* src = string;
	
	for (S32 i = 0; i < length; ++i)
	{
		dest[i] = src[i];
	}
	
	//memcpy( &m_stringData[m_stringInfo[index].offset], string, length );
	
	InsertIntoHashTable(index + 1, hash);
	
	return index + 1;
}

U32 C_StringTable::Find( const CHAR8* string ) const
{
	C_AutoLock lock(m_lock);
	U32 hash;
	U32 length;
	return FindInHashTable( string, hash, length );
}

const CHAR8* C_StringTable::GetString( U32 index ) const
{
	if ( 0 == index )
	{
		return kNullString;
	}
	C_AutoLock lock(m_lock);	
	if ( index - 1 >= m_numStrings )
	{
		BIZ_ASSERT_FAIL("Invalid string index");
		return kNullString;
	}
	BIZ_ASSERT( index - 1 < m_numStrings );
	return &m_stringData[m_stringInfo[index - 1].offset];
}

void C_StringTable::SetCaseSensitive( bool isCaseSensitive )
{
	C_AutoLock lock(m_lock);
	m_isCaseSensitive = isCaseSensitive;
	m_stringCompareFunction = isCaseSensitive ? &StringCompareCaseSensitive : &StringCompareCaseInsensitive;
}

U32 C_StringTable::GetNumStrings() const
{
	C_AutoLock lock(m_lock);
	return m_numStrings;
}

U32 C_StringTable::GetMaxStrings() const
{
	C_AutoLock lock(m_lock);
	return m_maxStrings;
}

U32 C_StringTable::GetStringDataSize() const
{
	C_AutoLock lock(m_lock);
	return m_stringDataSize;	
}

U32 C_StringTable::GetStringDataCapacity() const
{
	C_AutoLock lock(m_lock);
	return m_maxStringDataSize;
}

void C_StringTable::Serialise( Sys::I_BinaryStream& stream ) const
{
	C_AutoLock lock(m_lock);	
	Sys::BinarySerialiserAutoEndian out( stream );
	
	out << U32( m_numStrings );
	for ( U32 i = 0; i < m_numStrings; ++i )
	{
		out << m_stringInfo[i].offset;
		out << m_stringInfo[i].hash;
	}

	out << U32( m_stringDataSize );
	for ( U32 i = 0; i < m_stringDataSize; ++i )
	{
		out << m_stringData[ i ];
	}
}

void C_StringTable::Deserialise( Sys::I_BinaryStream& stream )
{
	C_AutoLock lock(m_lock);	
	MEMORY_TRACKER_PUSH( kStringTableMemoryTag );
	
	delete[] m_stringInfo;
	m_stringInfo = 0;
	m_numStrings = 0;
	m_maxStrings = 0;
	
	delete[] m_stringData;
	m_stringData = 0;
	m_stringDataSize = 0;
	m_maxStringDataSize = 0;
	
	Sys::BinaryDeserialiserAutoEndian in( stream );

	in >> m_numStrings;
	m_stringInfo = new StringInfo[ m_numStrings ];
	for ( U32 i = 0; i < m_numStrings; ++i )
	{
		in >> m_stringInfo[ i ].offset;
		in >> m_stringInfo[ i ].hash;
	}

	in >> m_stringDataSize;
	m_stringData = new CHAR8[ m_stringDataSize ];
	for ( U32 i = 0; i < m_stringDataSize; ++i )
	{
		in >> m_stringData[ i ];
	}
	
	Rehash();
	
	MEMORY_TRACKER_POP( kStringTableMemoryTag );
}

void C_StringTable::EnsureCapacity( U32 newStringLength )
{
	MEMORY_TRACKER_PUSH( kStringTableMemoryTag );
	
	bool expanded = false;

	const U32 requiredStringDataSize = m_stringDataSize + newStringLength;
	if ( requiredStringDataSize > m_maxStringDataSize )
	{
		// Expand string data buffer by 50%.
		const U32 newMaxStringDataSize = Maths::Max( ( m_maxStringDataSize * 3 ) / 2, requiredStringDataSize );
			
		CHAR8* newStringData = new CHAR8[ newMaxStringDataSize ];
		
		if ( m_stringData )
		{
			memcpy( newStringData, m_stringData, sizeof( CHAR8 ) * m_stringDataSize );
			delete[] m_stringData;
		}
		m_stringData = newStringData;
		m_maxStringDataSize = newMaxStringDataSize;
		
		expanded = true;
	}
	
	BIZ_ASSERT( m_maxStringDataSize >= requiredStringDataSize );
	
	const U32 requiredNumStrings = m_numStrings + 1;
	if ( requiredNumStrings > m_maxStrings )
	{
		// Expand string offset table by 50%.
		const U32 newMaxStrings = Maths::Max( ( m_maxStrings * 3 ) / 2, requiredNumStrings );
		
		StringInfo* newStringInfo = new StringInfo[ newMaxStrings ];
		
		if ( m_stringInfo )
		{
			memcpy( newStringInfo, m_stringInfo, sizeof(StringInfo) * m_numStrings );
			delete[] m_stringInfo;
		}
		m_stringInfo = newStringInfo;
		m_maxStrings = newMaxStrings;
		expanded = true;
		
		// Re-insert all strings into hash table (ouch).
		Rehash();
		
		LOGDEBUG(Debug::GetLogger("mercury.animation"), "Animation string table rehash: %d entries, %.1fK bytes", m_hashTableSize, m_hashTableSize * sizeof(U16) / 1024.f);
	}

	if (expanded)
	{
		LOGDEBUG(Debug::GetLogger("mercury.animation"), "Animation string table expanded: %d/%d strings, %.1fK/%1.fK bytes", requiredNumStrings, m_maxStrings, requiredStringDataSize / 1024.f, m_maxStringDataSize / 1024.f);
	}	
	
	MEMORY_TRACKER_POP( kStringTableMemoryTag );
}

U32 C_StringTable::FindInHashTable( const CHAR8* string, U32& outHash, U32& outLength ) const
{
	if (string == NULL)
	{
		outLength = 0;
		outHash = 0;
		return 0;
	}
	
	U32 hash = HashString( string, &outLength );
	outHash = hash;
		
	const U32 hashTableSize = m_hashTableSize;		
	U32 hashIndex = hash % hashTableSize;
		
	U32 probe = 1;
	while (m_hashTable[hashIndex] != 0)		
	{
		// Lookup string.
		U32 stringId = m_hashTable[hashIndex];
		if ((*m_stringCompareFunction)( &m_stringData[m_stringInfo[stringId - 1].offset], string ))
		{
			return stringId;
		}
		
		// Quadratic probe.
		U32 newHashIndex = (hashIndex + 1 + probe) % hashTableSize;
		BIZ_ASSERT(newHashIndex != hashIndex);
		hashIndex = newHashIndex;
		probe <<= 1;
	}

	return 0;
}

void C_StringTable::InsertIntoHashTable( U16 stringId, U32 hash )
{
	BIZ_ASSERT(stringId != 0);	// 0 is reserved for null string and unoccupied hash table slots.
	
	const U32 hashTableSize = m_hashTableSize;
	U32 hashIndex = hash % hashTableSize;
		
	U32 probe = 1;
	while (m_hashTable[hashIndex] != 0)
	{
		if (m_hashTable[hashIndex] == stringId)
		{
			// duplicate!
			BIZ_ASSERT_FAIL("Duplicate string ID in hash table");
			return;
		}
		
		// Quadratic probe.
		U32 newHashIndex = (hashIndex + 1 + probe) % hashTableSize;
		BIZ_ASSERT(newHashIndex != hashIndex);
		hashIndex = newHashIndex;
		probe <<= 1;
	}
	
	BIZ_ASSERT(m_hashTable[hashIndex] == 0);
	m_hashTable[hashIndex] = stringId;
}

void C_StringTable::Rehash()
{
	// Keep load factor under 80%.
	m_hashTableSize = (m_maxStrings * 5) / 4;

	delete[] m_hashTable;
	m_hashTable = new U16[m_hashTableSize];
	memset(m_hashTable, 0, sizeof(U16) * m_hashTableSize);
		
	for (U32 i = 0, count = m_numStrings; i < count; ++i)
	{
		InsertIntoHashTable(i + 1, m_stringInfo[i].hash);
	}
}

} // namespace Animation
