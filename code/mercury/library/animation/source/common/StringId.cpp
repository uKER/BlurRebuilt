#include "stdafx.h"
#include <animation/common/StringId.h>
#include "common/StringTable.h"
#include <animation/common/StringUtil.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

namespace Animation
{

//
// C_StringId
//

#ifndef PLATFORM_SPU

C_StringTable gStringRegistry( 1024, 8192, true );

void DumpStringTable(void* context)
{
	C_StringTable* pStringTable = static_cast<C_StringTable*>(context);
	
	for (U32 i = 0, count = pStringTable->GetNumStrings(); i < count; ++i)
	{
		LOGINFO(Debug::GetLogger("mercury.animation"), "%5d '%s'", i, pStringTable->GetString(i));
	}
}

DECLARE_DEBUG_FUNCTION(gDumpStringTable, "Animation.DumpStringTable", &DumpStringTable, &gStringRegistry);


C_StringId::C_StringId( const CHAR8* string, bool insertIfNewHint )
{
	if (insertIfNewHint)
	{
		Set( string );
	}
	else
	{
		m_stringIndex = gStringRegistry.Find(string);
	}
}

void C_StringId::Set( const CHAR8* string )
{
	m_stringIndex = gStringRegistry.Insert( string );
}

const CHAR8* C_StringId::GetString() const
{
	return gStringRegistry.GetString( m_stringIndex );
}

bool C_StringId::Match(const CHAR8* pattern) const
{
	bool match = false;
	if (IsValid())
	{
		match = Animation::StringUtil::Match(GetString(), pattern);
	}
	return match;
}

#endif

//
// C_StringIdArray
//

#ifndef PLATFORM_SPU

C_StringIdArray::C_StringIdArray():
	m_numStrings( 0 ),
	m_stringIds( NULL )
{
}

C_StringIdArray::C_StringIdArray( U32 numStrings ):
	m_numStrings( numStrings ),
	m_stringIds( NULL )
{
	if ( numStrings > 0 )
	{
		m_stringIds = new C_StringId[ numStrings ];
	}

}
		
C_StringIdArray::~C_StringIdArray()
{
	delete[] m_stringIds;
}


void C_StringIdArray::Clear()
{
	delete[] m_stringIds;
	m_stringIds = 0;
	m_numStrings = 0;
}

void C_StringIdArray::Resize( U32 numStrings )
{
	C_StringId* newStringIds = NULL;
	
	if (numStrings > 0)
	{
		newStringIds = new C_StringId[ numStrings ];
	}
	
	if ( m_stringIds && m_numStrings > 0 )
	{
		memcpy( newStringIds, m_stringIds, m_numStrings * sizeof( C_StringId ) );
		delete[] m_stringIds;
	}
	m_stringIds = newStringIds;
	m_numStrings = numStrings;
}

#endif

U32 C_StringIdArray::Size() const
{
	return m_numStrings;
}

const C_StringId& C_StringIdArray::operator [] ( const U32 index ) const
{
	BIZ_ASSERT( index < m_numStrings );
	return m_stringIds[ index ];
}

C_StringId& C_StringIdArray::operator [] ( const U32 index )
{
	BIZ_ASSERT( index < m_numStrings );
	return m_stringIds[ index ];
}


} // namespace Animation
