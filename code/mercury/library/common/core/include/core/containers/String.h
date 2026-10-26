#ifndef STRING_H
#define STRING_H

#ifdef _MSC_VER
#pragma once
#endif

#define USE_OLD_UTIL_CSTRING
#define ALLOW_UTIL_CSTRING_IMPLICIT_CASTS

#ifndef USE_OLD_UTIL_CSTRING
#include <btl/utility/string.h>
#endif

#include <core/allocators/Allocator.h>
#include <core/containers/FixedString.h>
#include <core/containers/Vector.h>
#include <core/utility/Utility.h>

namespace Util
{
	//TODO: Unicode support.

	template <typename DataType, typename Allocator, U32 BufferSize>
	class C_StringBase
	{
	public:
		/// Default constructor
		C_StringBase();
	   
		template< int i >    
		C_StringBase( const C_FixedString<i>& fixedString );

		/// Constructor from a char array
		C_StringBase( const DataType* src );

		/// Constructor from a char array
		C_StringBase(const DataType* src, S32 count);

		 /// Constructor from a character
		C_StringBase( const DataType c, const S32 count = 1 );

		/// String length
		FORCE_INLINE S32 Length() const;
	    
		/// Copy Constructor
		C_StringBase( const C_StringBase& src );
	     
		/// Assignment operator
		const C_StringBase& operator = ( const C_StringBase& src );
		const C_StringBase& operator = ( const DataType* src );

		/// Var-arg initialisation
		const C_StringBase& Printf( const DataType* format, ... );

		/// Var-arg construction
		static C_StringBase Format( const DataType* format, ... );
	    
		/// _Concatenation operator
		C_StringBase operator + ( const C_StringBase& src ) const;
		
		C_StringBase operator + (const DataType* src) const;

		/// _Concatenation operator
		const C_StringBase& operator += ( const C_StringBase& src );

		/// _Concatenation operator
		const C_StringBase& operator += ( const DataType* src );

		/// Sub-string extraction operator.
		/// NOTE: lo = inclusive, hi = exclusive, range ( 0, 0 ) denotes an empty string!
		C_StringBase operator () ( S32 lo, S32 hi ) const;

		template <typename Archive>
		void Deserialise(Archive &ar);

		template <typename Archive>
		void Serialise(Archive &ar);

		/// Forward Search for char
		/// returns -1 if the substring was not found
		INLINE S32 Search( const DataType c, S32 offset = 0 ) const;
	   
		/// Reverse search for char
		/// returns -1 if the substring was not found
		INLINE S32 RevSearch( const DataType c, S32 offset = 0 ) const;
	    
		/// Forward search for sub-string
		/// returns -1 if the substring was not found
		S32 Search( const DataType* search, S32 offset = 0 ) const;

		S32 Search( const C_StringBase& search, S32 offset = 0 ) const;

		/// case insensitive Forward search for sub-string
		/// returns -1 if the substring was not found
		S32 ISearch( const C_StringBase& search, const S32 offset = 0 ) const;

		C_StringBase Replace( const C_StringBase& src, const C_StringBase& dst ) const;

		// Case Insensitive
		C_StringBase IReplace( const C_StringBase& src, const C_StringBase& dst ) const;

		// Replace characters in string with input from src.
		void Replace(S32 start, S32 count, const DataType* src);

		/// Copy fixed length buffer
		void CopyFrom( const DataType* src, S32 length );

		/// Copy char to fixed length buffer
		INLINE void Copy( DataType* buffer, S32 length ) const;
	    
		/// Equality
		INLINE const bool operator == ( const C_StringBase& src ) const;

		/// less than
		INLINE const bool operator < ( const C_StringBase& src ) const;

		/// Char Array Equality
		INLINE const bool operator == ( const DataType* src ) const;

		/// Inequality
		INLINE const bool operator != ( const C_StringBase& src ) const;

		/// Char Array Inequality
		INLINE const bool operator != ( const DataType* src ) const;
	    
		DataType Lower(const DataType c) const;

		DataType Upper(const DataType c) const;

		void ConvertToUpper();

		void ConvertToLower();

		/// Convert to upper case
		C_StringBase Upper() const;

		/// Convert to lower case
		C_StringBase Lower() const;
	    
		const void Tokenise(Ut::Vector<C_StringBase>& strings, const char tok) const;

		#ifdef ALLOW_UTIL_CSTRING_IMPLICIT_CASTS
		operator const DataType* () const;

		operator DataType* ();
		#else

		const DataType& operator [] (int i) const {assert(i > -1 && i < Length()); return Chars()[i];} ///< @a 
		DataType& operator [] (int i) {assert(i > -1 && i < Length()); return NonConstChars()[i];} ///< @a 

		#endif

		void Clear();
	    
		DEPRECATED void Print() const;
	    
		~C_StringBase();
	  
		FORCE_INLINE const DataType* Chars() const;
		
		FORCE_INLINE DataType* NonConstChars();

	protected:
		/// Returns index of character ptr
		FORCE_INLINE S32 _PtrIndex( const DataType* ptr ) const;

		FORCE_INLINE S32 _StrLen(const DataType* __restrict src) const;

		FORCE_INLINE S32 _StrSize(const DataType* __restrict src) const;

		FORCE_INLINE void _MemCpy(DataType* __restrict dst, const DataType* __restrict src, S32 size);

		FORCE_INLINE void _MemCpy(const DataType* __restrict src, S32 size);

		void _Concat(const DataType* src, S32 srcLength);

		//Note that this is SIZE not length. Size = length+1
		void _Resize(U32 size);

		FORCE_INLINE bool _IsUsingLocalStorage() const;

		FORCE_INLINE void _InitialiseStorage(U32 size);

		FORCE_INLINE void _FreeStorage();

		// This removes the string classes dependency on the maths library
		template<typename Type> 
		FORCE_INLINE Type _Min(const Type lhs, const Type rhs) const;

		template<typename Type> 
		FORCE_INLINE Type _Max(const Type lhs, const Type rhs) const;
		union
		{
			//Last member of this buffer is used as a flag to indicate if we're using the heap.
			DataType m_localData[BufferSize];
			struct
			{
				DataType*	m_pHeapData;
				U32			m_heapSize;
				U32			m_heapCapacity;
			};
		};
	};

	//Implementation of class.
	#include "String.inl"

	// default narrow string
#ifdef USE_OLD_UTIL_CSTRING

#if defined(WORD_SIZE) && WORD_SIZE == 64
	typedef C_StringBase<char, Ut::ArrayAllocator<char>, 32> C_String;
#else
	typedef C_StringBase<char, Ut::ArrayAllocator<char>, 16> C_String;
#endif

#else

	typedef Btl::String C_String;

#endif

} // namespace Util

#include "StringHelpers.h"

#endif
