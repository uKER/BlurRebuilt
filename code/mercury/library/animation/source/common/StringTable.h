#ifndef ANIMATION_STRINGTABLE_H
#define ANIMATION_STRINGTABLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/threads/Mutex.h>

namespace Sys
{
	class I_BinaryStream;
}

namespace Animation
{

	///
	/// String table.
	///
	/// Thread-safe.
	///
	/// Motivated by a need to reduce memory used by duplicate strings (e.g. joint names)
	/// and avoid the small, but possible risk of collisions when using string hashes.
	///
	/// The string table will expand to ensure it has enough capacity for all inserted strings.
	///
	class C_StringTable
	{
	public:
		/// Construct a string table with a maximum number of strings and maximum string data size.
		C_StringTable( U32 maxStrings, U32 maxStringDataSize, bool isCaseSensitive=true );
		
		/// Destructor.
		~C_StringTable();
				
		/// Clear the string table.
		void Clear();
		
		/// Insert a string into the string table. Slow, so don't use this too often.
		U32 Insert( const CHAR8* string );
		
		/// Finds a string in the string table, returns 0 if it doesn't exist.
		U32 Find( const CHAR8* string ) const;
				
		/// Lookup a string by its index.
		const CHAR8* GetString( U32 index ) const;
		
		/// Set case sensitivity.
		void SetCaseSensitive( bool isCaseSensitive = true );
		
		/// Get total number of strings stored in the string table.
		U32 GetNumStrings() const;
		
		/// Get maximum number of strings that can be stored.
		U32 GetMaxStrings() const;
		
		/// Get number of bytes in use by the string buffer, which will be less
		/// than or equal to GetStringDataCapacity().
		U32 GetStringDataSize() const;
		
		/// Get maximum number of bytes available for string data.
		U32 GetStringDataCapacity() const;
		
		/// Save string table into a binary stream.
		void Serialise( Sys::I_BinaryStream& stream ) const;
		
		/// Load string table from binary stream.
		void Deserialise( Sys::I_BinaryStream& stream );
		
	private:
		U32 FindInHashTable( const CHAR8* string, U32& outHash, U32& outLength ) const;
		void InsertIntoHashTable( U16 stringId, U32 hash );
		void Rehash();
		
		void EnsureCapacity( U32 newStringLength );
		//void Reserve( U32 maxStrings, U32 maxStringDataSize );
		
		struct StringInfo
		{
			U32		offset;
			U32		hash;
		};
		
		mutable MT::Lock m_lock;
		
		U32			m_numStrings;
		U32			m_maxStrings;
		StringInfo*	m_stringInfo;
		
		U32			m_stringDataSize;
		U32			m_maxStringDataSize;
		CHAR8*		m_stringData;
		
		U16*		m_hashTable;
		U32			m_hashTableSize;	// Usually m_maxStrings + 20%
		
		bool m_isCaseSensitive;
		
		bool (*m_stringCompareFunction)( const CHAR8* str1, const CHAR8* str2 );
	};

}

#include "StringTable.inl"

#endif


