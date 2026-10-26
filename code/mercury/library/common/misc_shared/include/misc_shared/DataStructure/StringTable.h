#ifndef STRING_TABLE_H
#define STRING_TABLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Core.h>
#include <core/containers/containerserialisation.h>

// allows many arbitrary length char strings to be concatenated
// into a single memory block for more efficient memory allocation.
// strings in the string table are accessed by index

// use the string table builder in tools to construct a string table

namespace Util
{

typedef S32 C_StringId;

class C_StringTable;

// less efficient class for constructing string table in tools
template <typename _T = Util::C_String>	// Default is old-style strings.
class T_StringTableBuilder
{
friend class C_StringTable;

public:
	T_StringTableBuilder()
	{
	}
	~T_StringTableBuilder()
	{
	}

	// returns number of strings in string table
	const S32 GetStringCount() const
	{
		return m_StringList.Size();
	}

	// simple, slow search to find if string is in string table builder
	// returns index if found -1 if not found
	const C_StringId GetIndex( const _T& str )
	{
		const S32 stringCount = GetStringCount();
		for(S32 i=0; i<stringCount; i++)
		{
			if( str == GetString(i) )
				return i;
		}

		return -1;
	}

	// adds the string to the string table builder and returns the index of the string
	// if the string already exists in the string table builder the existing index will be returned
	const C_StringId AddString( const _T& str )
	{
		S32 index = GetIndex( str );
		if(index != -1)
			return index;

		index = m_StringList.Size();
		m_StringList.PushBack( str );

		return index;
	}

	// returns the string with the specified index
	const char* GetString( const C_StringId index ) const
	{
		return m_StringList[index].Chars();
	}

	// returns the string with the specified index
	const char* operator []( const C_StringId index ) const
	{
		return GetString(index);
	}

private:
	Ut::Vector<_T>		m_StringList;
};

typedef T_StringTableBuilder<> C_StringTableBuilder;

// C_StringTable

class C_StringTable
{
public:
	C_StringTable();
	
	template <typename _T>
	C_StringTable( T_StringTableBuilder<_T>& builder )
	{
		Build(builder);
	}

	~C_StringTable();

	// build a string table from a string table builder
	template <typename _T>
	void Build( T_StringTableBuilder<_T>& builder )
	{
		U32 dataSize = 0;
		const S32 stringCount = builder.GetStringCount();
		for(S32 stringNo=0; stringNo<stringCount; stringNo++)
		{
			_T str(builder[stringNo]);
			dataSize = dataSize + str.Length() + 1;
		}
		
		m_StringData.Resize( dataSize );
		m_StringOffsetList.Resize( stringCount );
		if (stringCount == 0)
			return;

		char* dst = &m_StringData[0];

		U32 byteOffset = 0;
		for(S32 stringNo=0; stringNo<stringCount; stringNo++)
		{
			m_StringOffsetList[stringNo] = byteOffset;
			
			_T str(builder[stringNo]);
			U32 strSize = str.Length() + 1;

			memcpy( dst, str.Chars(), strSize ); 

			dst += strSize;
			byteOffset += strSize;
		}
	}


	// returns number of strings in string table
	const S32 GetStringCount() const; 

	//Slow search to find if string is in string table builder
	template <typename _T>
	const C_StringId GetIndex( const _T& str )
	{
		const S32 stringCount = GetStringCount();
		for(S32 i=0; i<stringCount; i++)
		{
			const char* curStr = GetString(i);
			if(!strcmp(str.Chars(), curStr))
				return i;
		}

		return -1;
	}

	//Slow
	template <typename _T>
	const C_StringId GetOffset( const _T& str )
	{
		S32 index = GetIndex(str);
		if (index == -1)
			return -1;

		return m_StringOffsetList[index];
	}

	// returns the string with the specified index
	const char* GetString( const C_StringId index ) const;

	// returns the string offset with the specified index
	U32 GetOffsetByIndex( const C_StringId index ) const;

	// returns the string with the specified index
	const char* operator []( const C_StringId index ) const;

	const char* GetData() const { return m_StringData.Size() > 0 ? &m_StringData[0] : 0; }

	char* GetData() { return m_StringData.Size() > 0 ? &m_StringData[0] : 0; }

	U32 GetSizeBytes() const { return m_StringData.Size(); }

	template <typename Archive>
	void Serialise( Archive& ar )
	{
		ar << m_StringOffsetList;
		ar << m_StringData;
	}
 
	template <typename Archive>
	void Deserialise( Archive& ar )
	{
		ar >> m_StringOffsetList;
		ar >> m_StringData;
	}

private:

	Ut::Vector<U32>		m_StringOffsetList;
	Ut::Vector<char>	m_StringData;
};



} // namespace Util

#endif

