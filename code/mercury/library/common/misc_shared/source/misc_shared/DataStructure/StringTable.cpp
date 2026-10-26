
#include "MiscSharedPch.h"

#include <string.h>
#include <core/core.h>
#include <core/containers.h>

#include <misc_shared/DataStructure/StringTable.h>

namespace Util
{

// NOTE:
// C_StringTableBuilder implementation now inlined into header
// due to templatisation of C_StringTableBuilder class.
///  -- STOVE, 081208

// C_StringTable

C_StringTable::C_StringTable()
{
}


C_StringTable::~C_StringTable()
{
}

// returns number of strings in string table
const S32 C_StringTable::GetStringCount() const
{
	return m_StringOffsetList.Size();
}


// returns the string with the specified index
const char* C_StringTable::GetString( const C_StringId index ) const
{
	return &m_StringData[ m_StringOffsetList[index] ];
}

// returns the string offset with the specified index
U32 C_StringTable::GetOffsetByIndex( const C_StringId index ) const
{
	return m_StringOffsetList[index];
}

// returns the string with the specified index
const char* C_StringTable::operator []( const C_StringId index ) const
{
	return GetString(index);
}



} // namespace Util

