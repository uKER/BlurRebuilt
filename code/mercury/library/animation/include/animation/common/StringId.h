#ifndef ANIMATION_STRINGID_H
#define ANIMATION_STRINGID_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class I_BinaryStream;
}


namespace Animation
{

	///
	/// A compact string identifier implemented as an index into an internal global string table.
	///
	/// Converting a string to a C_StringId is expensive and should be avoided outside of loading
	/// and initialisation code. 
	///
	class C_StringId
	{
	public:
	
		/// Default constructor, initialises string to null.
		C_StringId();
		
#ifndef PLATFORM_SPU
		/// Constructs a string identifier. If @a insertIfNewHint is false, then the resulting identifier
		/// may be null if the string has not been converted to a C_StringId sometime before. Depending
		/// on the internal implementation, this can help save memory being taken up by strings that never
		/// end up being used by the animation system.
		C_StringId( const CHAR8* string, bool insertIfNewHint = true );
		
		/// Set the string identifier.
		void Set( const CHAR8* string );

		/// Get the string represented by identifier. 
		const CHAR8* GetString() const;
		
#endif

		/// Clear string to null.
		void Clear();
		
		/// Returns true if string is null.
		bool IsNull() const;
		
		/// Returns true if string is valid (non-null).
		bool IsValid() const;
		
		/// Returns true if string is non-null.
		operator bool () const;
		
		/// Returns true if string is null.
		bool operator !() const;
		
		/// Returns an ID value for the string.
		U32 GetId() const;

		//
		// Comparison functions.
		//
		
		/// Test for equality.
		bool operator == ( const C_StringId& other ) const;
		
		/// Test for inequality.
		bool operator != ( const C_StringId& other ) const;
		
		/// Less than comparison, useful for sorting.
		bool operator < ( const C_StringId& other ) const;
		
		//
		// Pattern matching using simple wildcards (*)
		//
		
		/// Returns true if string matches specified pattern. Null/invalid strings always return false.
		bool Match(const CHAR8* pattern) const;
		
	
	private:
		U16 m_stringIndex;
	};
	
	///
	/// An array of StringIds.
	///
	class C_StringIdArray
	{
	public:
		C_StringIdArray();
		
		explicit C_StringIdArray( U32 numStrings );
		
		~C_StringIdArray();
		
		void Clear();
		
		void Resize( U32 numStrings );
				
		U32 Size() const;
		
		const C_StringId& operator [] ( const U32 index ) const;
		
		C_StringId& operator [] ( const U32 index );
		
		
		template < typename T >
		T& Serialise( T& out ) const;
		
		template < typename T >
		T& Deserialise( T& in );
	
	private:
		U32 m_numStrings;
		C_StringId* m_stringIds;
	};


}

#include "StringId.inl"


#endif
