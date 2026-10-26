#ifndef BTL_STRING_H
#define BTL_STRING_H
#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////
// String
////////////////////////////////////////////////////////////////

#include <btl/allocators/Allocator.h>
#include <btl/containers/Vector.h>

namespace Btl
{
	//TODO: Unicode support.

	template<typename DataType>
	class StringFunctions
	{
	public:
		static S32 StrLen(const DataType* src);
		static const DataType* StrChr(const DataType* start, DataType c);
		static const DataType* StrRChr(const DataType* start, DataType c);
		static const DataType* StrStr(const DataType* start, const DataType* search);
		static S32 StrCmp(const DataType* lhs, const DataType* rhs);
		static void StrNCpy(DataType* dst, S32 max, const DataType* src, S32 count);
		static void MemCpy(DataType* dst, const DataType* src, S32 size);

		// This removes the string classes dependency on the maths library
		template<typename Type> 
		static Type Min(const Type lhs, const Type rhs);

		template<typename Type> 
		static Type Max(const Type lhs, const Type rhs);
	};

	/// Base class for StringBase
	template <typename T, typename Allocator, U32 BufferSize>
	class StringBuffer
	{
	public:
		typedef T DataType;

		StringBuffer();
		~StringBuffer();

		const DataType* Chars() const; ///< @a
		DataType* NonConstChars();///< @a

		/// String length
		S32 Length() const;

		// Buffer capacity.
		S32 Capacity() const;

		void Reserve(U32 length); ///< Reserve storage for 'length' characters.

	protected:
		const static U32 BUFFER_SIZE = BufferSize;

		void FreeStorage();
		void SetLength(U32 length);

	private:
		bool IsUsingLocalStorage() const;

		struct HeapData
		{
			DataType*	m_pHeapData;
			U32			m_stringLength;
			U32			m_heapCapacity;
		};

		union
		{
			//Last member of this buffer is used as a flag to indicate if we're using the heap.
			DataType m_localData[BufferSize];
			HeapData m_heapData;
		};
	};

	/// Base class for StringBase
	template <typename T, U32 BufferSize>
	class FixedStringBuffer
	{
	public:
		typedef T DataType;

		const DataType* Chars() const; ///< @a
		DataType* NonConstChars(); ///< @a

		/// String length
		S32 Length() const;

		// Buffer capacity.
		S32 Capacity() const;

		void Reserve(U32 length);

	protected:
		const static U32 BUFFER_SIZE = BufferSize;

		void FreeStorage() {}
		void SetLength(U32 length);

	private:
		DataType m_localData[BufferSize];
	};

	/// String template.
	/// See @ref String typedef.
	template <typename DataType, typename Buffer>
	class StringBase : public Buffer
	{
	public:
		/// Default constructor
		StringBase();
	   
		/// Constructor from a char array
		StringBase( const DataType* src );

		/// Constructor from a char array
		StringBase(const DataType* src, S32 count);

		 /// Constructor from a character
		StringBase( const DataType c, const S32 count = 1 );
	    
		/// Copy Constructor
		StringBase( const StringBase& src );

		template <typename BufferType>
		StringBase( const StringBase<DataType, BufferType>& src );
	     
		/// Assignment operator
		const StringBase& operator = ( const StringBase& src );

		template <typename BufferType>
		const StringBase& operator = ( const StringBase<DataType, BufferType>& src );

		/// Var-arg initialisation
		const StringBase& Printf( DataType* format, ... );
		
		/// Var-arg construction.
		static StringBase Format( DataType* format, ... );
	    
		/// Concatenation operator
		StringBase operator + ( const StringBase& src ) const;
		
		StringBase operator + (const DataType* src) const;

		/// Concatenation operator
		const StringBase& operator += ( const StringBase& src );

		/// Concatenation operator
		const StringBase& operator += ( const DataType* src );

		/// Sub-string extraction operator.
		/// NOTE: lo = inclusive, hi = exclusive, range ( 0, 0 ) denotes an empty string!
		StringBase operator () ( S32 lo, S32 hi ) const;

		/// Forward Search for char
		/// returns -1 if the substring was not found
		INLINE S32 Search( const DataType c, S32 offset = 0 ) const;
	   
		/// Reverse search for char
		/// returns -1 if the substring was not found
		INLINE S32 RevSearch( const DataType c, S32 offset = 0 ) const;
	    
		/// Forward search for sub-string
		/// returns -1 if the substring was not found
		S32 Search( const DataType* search, S32 offset = 0 ) const;

		S32 Search( const StringBase& search, S32 offset = 0 ) const;

		/// case insensitive Forward search for sub-string
		/// returns -1 if the substring was not found
		S32 ISearch( const StringBase& search, const S32 offset = 0 ) const;

		StringBase Replace( const StringBase& src, const StringBase& dst ) const;

		// Case Insensitive
		StringBase IReplace( const StringBase& src, const StringBase& dst ) const;

		// Replace characters in string with input from src.
		void Replace(S32 start, S32 count, const DataType* src);

		/// Copy fixed length buffer
		void CopyFrom( const DataType* src, S32 length );

		/// Copy char to fixed length buffer
		void CopyTo( DataType* dst, S32 length ) const;
	    
		/// Equality
		const bool operator == ( const StringBase& src ) const;

		/// less than
		const bool operator < ( const StringBase& src ) const;

		/// greater than
		const bool operator > ( const StringBase& src ) const;

		/// Char Array Equality
		const bool operator == ( const DataType* src ) const;

		/// Inequality
		const bool operator != ( const StringBase& src ) const;

		/// Char Array Inequality
		const bool operator != ( const DataType* src ) const;
	    
		DataType Lower(const DataType c) const;

		DataType Upper(const DataType c) const;

		void ConvertToUpper(); ///< @a 

		void ConvertToLower(); ///< @a 

		/// Convert to upper case
		StringBase Upper() const;

		/// Convert to lower case
		StringBase Lower() const;
	    
		const void Tokenise(Btl::Vector<StringBase>& strings, const char tok) const;

		const DataType& operator [] (int i) const {assert(i > -1 && i < Buffer::Length()); return Buffer::Chars()[i];} ///< @a 
		DataType& operator [] (int i) {assert(i > -1 && i < Buffer::Length()); return Buffer::NonConstChars()[i];} ///< @a 

		void Clear(); ///< Sets the size to zero.

		bool Empty() const; ///< @a

		bool Resize(U32 length, bool assertOnFailure = true); ///< Sets the size to 'length'.

	private:
		void SetLength(U32 length) {Buffer::SetLength(length);}

		/// Returns index of character ptr
		S32 PtrIndex( const DataType* ptr ) const;

		void Append(const DataType* src, S32 srcLength);
	};

//Implementation of class.
#include "String.inl"

#if !PLATFORM_SPU
	typedef ArrayAllocator<char> Allocator;
#else
	typedef LinearArrayAllocator<char> Allocator;
#endif

	// default narrow string
#if defined(WORD_SIZE) && WORD_SIZE == 64
	typedef StringBase<char, StringBuffer<char, Allocator, 32> > String; ///<
#else
	typedef StringBase<char, StringBuffer<char, Allocator, 16> > String; ///< Standard String Type
#endif

	typedef StringBase<char, FixedStringBuffer<char, 4> >      String4; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 6> >      String6; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 8> >      String8; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 12> >     String12; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 16> >     String16; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 32> >     String32; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 64> >     String64; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 80> >     String80; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 96> >     String96; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 128> >    String128; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 256> >    String256; ///< Fixed size string.
	typedef StringBase<char, FixedStringBuffer<char, 260> >    String260; ///< Fixed size string.

#include "StringHelpers.inl"

}

#endif

