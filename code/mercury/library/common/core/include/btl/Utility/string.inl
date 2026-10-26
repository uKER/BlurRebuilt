////////////////////////////////////////////////////////////////////////////
// StringFunctions

/// @cond

template <typename DataType>
S32 StringFunctions<DataType>::StrLen(const DataType* __restrict src)
{
	S32 len = (S32)strlen(src);
	assert(len != -1);
	return len;
}

template <typename DataType>
void StringFunctions<DataType>::MemCpy(DataType* __restrict dst, const DataType* __restrict src, S32 size) 
{
	//Test for overlap of pointers... not in speed though...
#ifndef CFG_SPEED
	assert(abs((int)(src-dst)) >= size); //pointer aliasing check, we need __restrict pointers
#endif

	memcpy(dst, src, sizeof(DataType)*size);
}

template <typename DataType>
template<typename Type> 
Type StringFunctions<DataType>::Min(const Type lhs, const Type rhs)
{
	return (lhs > rhs) ? rhs : lhs;
}

template <typename DataType>
template<typename Type> 
Type StringFunctions<DataType>::Max(const Type lhs, const Type rhs)
{
	return (lhs > rhs) ? lhs : rhs;
}

template <typename DataType>
FORCE_INLINE const DataType* StringFunctions<DataType>::StrChr(const DataType* start, DataType c)
{
	return strchr(start, c);
}

template <typename DataType>
FORCE_INLINE const DataType* StringFunctions<DataType>::StrRChr(const DataType* start, DataType c)
{
	return strrchr(start, c);
}

template <typename DataType>
FORCE_INLINE const DataType* StringFunctions<DataType>::StrStr(const DataType* start, const DataType* search)
{
	return strstr(start, search);
}

template <typename DataType>
FORCE_INLINE S32 StringFunctions<DataType>::StrCmp(const DataType* lhs, const DataType* rhs)
{
	return strcmp(lhs, rhs);
}

template <typename DataType>
FORCE_INLINE void StringFunctions<DataType>::StrNCpy(DataType* dst, S32 max, const DataType* src, S32 count)
{
	strncpy_s(dst, max, src, count);
}

////////////////////////////////////////////////////////////////////////////
// StringBuffer

template <typename T, typename Allocator, U32 BufferSize>
StringBuffer<T, Allocator, BufferSize>::StringBuffer()
{
	//Flag as using local storage.
	m_localData[BufferSize-1] = 0;
}

template <typename T, typename Allocator, U32 BufferSize>
StringBuffer<T, Allocator, BufferSize>::~StringBuffer()
{
	FreeStorage();
}

template <typename T, typename Allocator, U32 BufferSize>
S32 StringBuffer<T, Allocator, BufferSize>::Length() const
{
	if (IsUsingLocalStorage())
		return StringFunctions<T>::StrLen(&m_localData[0]);

	return m_heapData.m_stringLength;
}

template <typename T, typename Allocator, U32 BufferSize>
S32 StringBuffer<T, Allocator, BufferSize>::Capacity() const
{
	if (IsUsingLocalStorage())
		return BufferSize - 1;

	return m_heapData.m_heapCapacity;
}

template <typename T, typename Allocator, U32 BufferSize>
void StringBuffer<T, Allocator, BufferSize>::Reserve(U32 length) 
{
	U32 size = length + 1;
	assert(size > 0);

	if (size > BufferSize - 1) // fits in local storage
	{
		if (!IsUsingLocalStorage() && m_heapData.m_heapCapacity >= size) // already have enough space
			return;

		DataType* p = Allocator::Allocate(size);

		if (p)
		{
			U32 length = Length();

			StringFunctions<DataType>::MemCpy(p, Chars(), length + 1); // copy to new string including null terminator

			if (!IsUsingLocalStorage())
				Allocator::Free(m_heapData.m_pHeapData, m_heapData.m_heapCapacity);

			m_heapData.m_pHeapData = p;
			m_heapData.m_heapCapacity = size;
			m_heapData.m_stringLength = length;
			m_localData[BufferSize-1] = 1;
		}
	}
}

template <typename T, typename Allocator, U32 BufferSize>
const T* StringBuffer<T, Allocator, BufferSize>::Chars() const
{
	if (IsUsingLocalStorage())
		return &m_localData[0];

	return m_heapData.m_pHeapData;
}

template <typename T, typename Allocator, U32 BufferSize>
bool StringBuffer<T, Allocator, BufferSize>::IsUsingLocalStorage() const
{
	if (m_localData[BufferSize-1] == 0)
		return true;
	return false;
}

template <typename T, typename Allocator, U32 BufferSize>
void StringBuffer<T, Allocator, BufferSize>::FreeStorage()
{
	if (!IsUsingLocalStorage())
		Allocator::Free(m_heapData.m_pHeapData, m_heapData.m_heapCapacity);

	//Flag as using local storage.
	m_localData[BufferSize-1] = 0;
}

template <typename T, typename Allocator, U32 BufferSize>
T* StringBuffer<T, Allocator, BufferSize>::NonConstChars()
{
	if (IsUsingLocalStorage())
		return &m_localData[0];

	return m_heapData.m_pHeapData;
}

template <typename T, typename Allocator, U32 BufferSize>
void StringBuffer<T, Allocator, BufferSize>::SetLength(U32 length)
{
	if (!IsUsingLocalStorage())
	{
		m_heapData.m_stringLength = length;
		assert(length < m_heapData.m_heapCapacity);
	}
	else
	{
		assert(length < BufferSize);
	}

	NonConstChars()[length] = 0;
}

////////////////////////////////////////////////////////////////////////////
// FixedStringBuffer

template <typename T, U32 BufferSize>
S32 FixedStringBuffer<T, BufferSize>::Length() const
{
	return StringFunctions<T>::StrLen(&m_localData[0]);
}

template <typename T, U32 BufferSize>
void FixedStringBuffer<T, BufferSize>::Reserve(U32 length) 
{
	assert(length < BufferSize);
}

template <typename T, U32 BufferSize>
const T* FixedStringBuffer<T, BufferSize>::Chars() const
{
	return &m_localData[0];
}

template <typename T, U32 BufferSize>
T* FixedStringBuffer<T, BufferSize>::NonConstChars()
{
	return &m_localData[0];
}

template <typename T, U32 BufferSize>
void FixedStringBuffer<T, BufferSize>::SetLength(U32 length)
{
	assert(length < BufferSize);
	NonConstChars()[length] = 0;
}

template <typename T, U32 BufferSize>
S32 FixedStringBuffer<T, BufferSize>::Capacity() const
{
	return BufferSize - 1;
}

/////////////////////////////////////////////////////////////////////////////////////

template <typename DataType, typename Buffer>
StringBase<DataType, Buffer>::StringBase()
{
	Buffer::Reserve(0);
	SetLength(0);
}

template <typename DataType, typename Buffer>
StringBase<DataType, Buffer>::StringBase( const DataType* src )
{
	const S32 length = StringFunctions<DataType>::StrLen(src);
	Buffer::Reserve(length);
	StringFunctions<DataType>::MemCpy(Buffer::NonConstChars(), src, length);
	SetLength(length);
}

template <typename DataType, typename Buffer>
StringBase<DataType, Buffer>::StringBase(const DataType* src, S32 count)
{
	assert(src);
	assert(count >= 0);
	assert(StringFunctions<DataType>::StrLen(src) >= count);
	Buffer::Reserve(count);
	StringFunctions<DataType>::MemCpy(Buffer::NonConstChars(), src, count);
	SetLength(count);
}

template <typename DataType, typename Buffer>
StringBase<DataType, Buffer>::StringBase( const DataType c, const S32 count)
{
	Buffer::Reserve(count);
	DataType* pData = Buffer::NonConstChars();
	memset( pData, c, count );
	SetLength(count);
}

template <typename DataType, typename Buffer>
StringBase<DataType, Buffer>::StringBase( const StringBase& src )
{
	*this = src;
}

template <typename DataType, typename Buffer>
template <typename BufferType>
StringBase<DataType, Buffer>::StringBase( const StringBase<DataType, BufferType>& src )
{
	*this = src;
}

template <typename DataType, typename Buffer>
const StringBase<DataType, Buffer>& StringBase<DataType, Buffer>::operator = ( const StringBase& src )
{
	if ((void*)this == (void*)&src) return *this;
	const DataType* srcData = src.Chars();
	S32 length = src.Length();
	Buffer::Reserve(length);
	StringFunctions<DataType>::MemCpy(Buffer::NonConstChars(), srcData, length);
	SetLength(length);
	return *this;
}

template <typename DataType, typename Buffer>
template <typename BufferType>
const StringBase<DataType, Buffer>& StringBase<DataType, Buffer>::operator = ( const StringBase<DataType, BufferType>& src )
{
	if ((void*)this == (void*)&src) return *this;
	const DataType* srcData = src.Chars();
	S32 length = src.Length();
	Buffer::Reserve(length);
	StringFunctions<DataType>::MemCpy(Buffer::NonConstChars(), srcData, length);
	SetLength(length);
	return *this;
}

template <typename DataType, typename Buffer>
const StringBase<DataType, Buffer>& StringBase<DataType, Buffer>::Printf( DataType* format, ... )
{
#ifdef TOOL
	DataType buffer[ 16*1024 ];
#else
	DataType buffer[ 1024 ];
#endif
	va_list arglist;
	va_start( arglist, format );
	vsprintf_s( buffer, lengthof(buffer), format, arglist );
	va_end( arglist );

	const S32 length = StringFunctions<DataType>::StrLen(buffer);

	Buffer::Reserve(length);
	DataType* pDst = Buffer::NonConstChars();
	const DataType* pSrc = buffer;
	StringFunctions<DataType>::MemCpy(pDst, pSrc, length);
	SetLength(length);
    
	return *this;
}

template <typename DataType, typename Buffer>
StringBase<DataType, Buffer> StringBase<DataType, Buffer>::Format( DataType* format, ... )
{
	DataType buffer[ 1024 ];
    
	va_list arglist;
	va_start( arglist, format );
	vsprintf_s( buffer, 1024, format, arglist );
	va_end( arglist );
	
	return StringBase<DataType, Buffer>( buffer );
}

template <typename DataType, typename Buffer>
StringBase<DataType, Buffer> StringBase<DataType, Buffer>::operator + ( const StringBase& src ) const
{
	const S32 thisLength = Buffer::Length();
	const S32 totalLength = src.Length() + thisLength;

	StringBase ret;
	ret.Reserve(totalLength);

	DataType* pDst2;

	//copy from this string to ret
	DataType* pDst1 = ret.NonConstChars();

	const DataType* pSrc1 = Buffer::Chars();
	StringFunctions<DataType>::MemCpy(pDst1, pSrc1, thisLength);

	//copy from other string to ret
	pDst2 = &(ret.NonConstChars())[thisLength];

	const DataType* pSrc2 = &(src.Chars())[0];
	StringFunctions<DataType>::MemCpy(pDst2, pSrc2,(totalLength-thisLength));
	ret.SetLength(totalLength);
	
	return ret;
}

template <typename DataType, typename Buffer>
StringBase<DataType, Buffer> StringBase<DataType, Buffer>::operator + (const DataType* src) const
{
	const S32 thisLength = Buffer::Length();
	const S32 totalLength = StringFunctions<DataType>::StrLen(src) + thisLength;

	StringBase ret;
	ret.Reserve(totalLength);

	DataType* pDst2;

	//copy from this string to ret
	DataType* pDst1 = ret.NonConstChars();

	const DataType* pSrc1 = Buffer::Chars();
	StringFunctions<DataType>::MemCpy(pDst1, pSrc1, thisLength);

	//copy from other string to ret
	pDst2 = &(ret.NonConstChars())[thisLength];

	StringFunctions<DataType>::MemCpy(pDst2, src,(totalLength-thisLength));
	ret.SetLength(totalLength);
	
	return ret;
}

template <typename DataType, typename Buffer>
const StringBase<DataType, Buffer>& StringBase<DataType, Buffer>::operator += ( const StringBase& src )
{
	Append(src.Chars(), src.Length());
	return *this;
}

template <typename DataType, typename Buffer>
const StringBase<DataType, Buffer>& StringBase<DataType, Buffer>::operator += ( const DataType* src )
{
	Append(src, StringFunctions<DataType>::StrLen(src));
	return *this;
}

template <typename DataType, typename Buffer>
/// Sub-string extraction operator.
/// NOTE: lo = inclusive, hi = exclusive, range ( 0, 0 ) denotes an empty string!
StringBase<DataType, Buffer> StringBase<DataType, Buffer>::operator () ( S32 lo, S32 hi ) const
{
	assert(lo > -1 && lo <= Buffer::Length());
	assert(hi > -1 && hi <= Buffer::Length());
	assert(lo <= hi);

	const S32 length = hi - lo;

	StringBase ret;
	ret.Reserve(length);

	DataType* pDst = ret.NonConstChars();
	const DataType* pSrc = &(Buffer::Chars())[lo];
	StringFunctions<DataType>::MemCpy(pDst, pSrc, length);
    
	ret.SetLength(length);

	return ret;
}

/// Forward Search for char
/// returns -1 if the substring was not found
template <typename DataType, typename Buffer>
S32 StringBase<DataType, Buffer>::Search( const DataType c, S32 offset ) const
{
	offset = StringFunctions<DataType>::Min( offset, Buffer::Length() - 1 );
	const DataType* ptr = StringFunctions<DataType>::StrChr(Buffer::Chars()+offset, c );
	return PtrIndex( ptr );
}

/// Reverse search for char
/// returns -1 if the substring was not found
template <typename DataType, typename Buffer>
S32 StringBase<DataType, Buffer>::RevSearch( const DataType c, S32 offset ) const
{
	offset = StringFunctions<DataType>::Min( offset, Buffer::Length() - 1 );
	const DataType* ptr = StringFunctions<DataType>::StrRChr(Buffer::Chars()+offset, c );
	return PtrIndex( ptr );
}

/// Forward search for sub-string
/// returns -1 if the substring was not found
template <typename DataType, typename Buffer>
S32 StringBase<DataType, Buffer>::Search( const DataType* search, S32 offset) const
{
	S32 index = -1;
	const S32 length = Buffer::Length();
	if ( length > 0 )
	{
		offset = StringFunctions<DataType>::Min( offset, length - 1 );
		const DataType* start = Buffer::Chars() + offset;
		const DataType* ptr = StringFunctions<DataType>::StrStr( start, search );

		index = PtrIndex( ptr );
	}
	
	return index;
}

template <typename DataType, typename Buffer>
S32 StringBase<DataType, Buffer>::Search( const StringBase& search, S32 offset) const
{
	return Search( search.Chars(), offset );
}

/// case insensitive Forward search for sub-string
/// returns -1 if the substring was not found
template <typename DataType, typename Buffer>
S32 StringBase<DataType, Buffer>::ISearch( const StringBase& search, const S32 offset) const
{
	//Working version which isn't amazingly efficient, replace with Boyer-Moore 
	//or something - Ian
	const S32 length = Buffer::Length();
	const S32 searchLength = search.Length();

	S32 charMatches = 0;
	for (S32 i=offset; i<length; i++)
	{
		const DataType c1 = Lower(Buffer::Chars()[i]);

		const DataType c2 = Lower(search.Chars()[charMatches]);

		if (c1 != c2)
		{
			i = i-charMatches;
			charMatches = 0;
		}
		else
		{
			charMatches++;
			if (charMatches == searchLength)
			{
				return (i-searchLength)+1;
			}
		}
	}
	
	return -1;
}

template <typename DataType, typename Buffer>
StringBase<DataType, Buffer> StringBase<DataType, Buffer>::Replace( 
	const StringBase& src, const StringBase& dst ) const
{
	StringBase result;
	const S32 length = Buffer::Length();
	const S32 srcLength = src.Length();
	if (0 == srcLength)
	{
		return *this; 
	}
	S32 index = 0;
	while ( index < length )
	{
		S32 pos = Search( src, index );
		if ( pos >= 0 )
		{
			if ( pos > index )
			{
				result = result + (*this)( index, pos );
			}
			result += dst;
			index = pos + srcLength;
		}
		else
		{
			return result + (*this)( index, length );
		}
	}
	return result;
}

// Case Insensitive
template <typename DataType, typename Buffer>
StringBase<DataType, Buffer> StringBase<DataType, Buffer>::IReplace( 
	const StringBase& src, const StringBase& dst ) const
{
	StringBase result;
	const S32 length = Buffer::Length();
	const S32 srcLength = src.Length();
	S32 index = 0;
	while ( index < length )
	{
		S32 pos = ISearch( src, index );
		if ( pos >= 0 )
		{
			if ( pos > index )
			{
				result = result + (*this)( index, pos );
			}
			result += dst;
			index = pos + srcLength;
		}
		else
		{
			return result + (*this)( index, length );
		}
	}
	return result;
}

// Replace characters in string with input from src.
template <typename DataType, typename Buffer>
void StringBase<DataType, Buffer>::Replace(S32 start, S32 count, const DataType* src)
{
	assert(count >= 0);
	assert(start >= 0);
	assert(StringFunctions<DataType>::StrLen(src) >= count);
	assert(Buffer::Length() >= start + count);

	DataType* dst = &Buffer::NonConstChars()[start];
	StringFunctions<DataType>::MemCpy( dst, src, count );
}

/// Copy fixed length buffer
template <typename DataType, typename Buffer>
void StringBase<DataType, Buffer>::CopyFrom( const DataType* src, S32 length )
{
	assert(src);
	Buffer::Reserve(length);
	DataType* dst = Buffer::NonConstChars();
	StringFunctions<DataType>::MemCpy( dst, src, length );        	    
	SetLength(length);
}

/// Copy char to fixed length buffer
template <typename DataType, typename Buffer>
void StringBase<DataType, Buffer>::CopyTo( DataType* dst, S32 length ) const
{
	StringFunctions<DataType>::StrNCpy( dst, length + 1, Buffer::Chars(), _TRUNCATE);
}

/// Equality
template <typename DataType, typename Buffer>
const bool StringBase<DataType, Buffer>::operator == ( const StringBase& src ) const
{
	return StringFunctions<DataType>::StrCmp(Buffer::Chars(), src.Chars() ) == 0;
}

/// less than
template <typename DataType, typename Buffer>
const bool StringBase<DataType, Buffer>::operator < ( const StringBase& src ) const
{
	return StringFunctions<DataType>::StrCmp(Buffer::Chars(), src.Chars() ) < 0;
}

/// greater than
template <typename DataType, typename Buffer>
const bool StringBase<DataType, Buffer>::operator > ( const StringBase& src ) const
{
	return StringFunctions<DataType>::StrCmp(Buffer::Chars(), src.Chars() ) > 0;
}

/// Char Array Equality
template <typename DataType, typename Buffer>
const bool StringBase<DataType, Buffer>::operator == ( const DataType* src ) const
{
	return StringFunctions<DataType>::StrCmp(Buffer::Chars(), src ) == 0;
}

/// Inequality
template <typename DataType, typename Buffer>
const bool StringBase<DataType, Buffer>::operator != ( const StringBase& src ) const
{
	return StringFunctions<DataType>::StrCmp(Buffer::Chars(), src.Chars() ) != 0;
}
/// Char Array Inequality
template <typename DataType, typename Buffer>
const bool StringBase<DataType, Buffer>::operator != ( const DataType* src ) const
{
	return StringFunctions<DataType>::StrCmp(Buffer::Chars(), src ) != 0;
}

template <typename DataType, typename Buffer>
DataType StringBase<DataType, Buffer>::Lower(const DataType c) const
{
	DataType ret = c;

	if ((c <= 'Z') && (c >= 'A'))
	{
		ret += ('a'-'A');
	}

	return ret;
}

template <typename DataType, typename Buffer>
DataType StringBase<DataType, Buffer>::Upper(const DataType c) const
{
	DataType ret = c;

	if ((c <= 'z') && (c >= 'a'))
	{
		ret -= ('a'-'A');
	}

	return ret;
}

template <typename DataType, typename Buffer>
void StringBase<DataType, Buffer>::ConvertToUpper()
{
	//inplace version of upper...
	const S32 length = Buffer::Length();

	DataType* pData = Buffer::NonConstChars();
	for (S32 i=0; i<length; i++)
		pData[i] = Upper(pData[i]);
}

template <typename DataType, typename Buffer>
void StringBase<DataType, Buffer>::ConvertToLower()
{
	//inplace version of lower...
	const S32 length = Buffer::Length();

	DataType* pData = Buffer::NonConstChars();
	for (S32 i=0; i<length; i++)
		pData[i] = Lower(pData[i]);
}

/// Convert to upper case
template <typename DataType, typename Buffer>
StringBase<DataType, Buffer> StringBase<DataType, Buffer>::Upper() const
{
	StringBase ret;
	ret = *this;
	ret.ConvertToUpper();
	return ret;
}

/// Convert to lower case
template <typename DataType, typename Buffer>
StringBase<DataType, Buffer> StringBase<DataType, Buffer>::Lower() const
{
	StringBase ret(*this);
	ret.ConvertToLower();
	return ret;
}

template <typename DataType, typename Buffer>
const void StringBase<DataType, Buffer>::Tokenise(Vector<StringBase>& strings, const char tok) const
{
	const S32 length = Buffer::Length();

	Vector<DataType> tmp;

	const DataType* pSrcChars = Buffer::Chars();
	for (S32 i=0; i<length; i++)
	{
		const DataType c = pSrcChars[i];
		if (c == tok)
		{
			if (!tmp.Empty())
			{
				tmp.PushBack(0);
				strings.PushBack(StringBase(&tmp[0]));
				tmp.Clear();
			}
		}
		else
		{
			tmp.PushBack(c);
		}
	}
	if (!tmp.Empty())
	{
		tmp.PushBack(0);
		strings.PushBack(StringBase(&tmp[0]));
	}
}

template <typename DataType, typename Buffer>
void StringBase<DataType, Buffer>::Clear()
{
	Buffer::FreeStorage();
	SetLength(0);
}

template <typename DataType, typename Buffer>
bool StringBase<DataType, Buffer>::Empty() const
{
	return *Buffer::Chars() == 0; 
}

/// Returns index of character ptr
template <typename DataType, typename Buffer>
S32 StringBase<DataType, Buffer>::PtrIndex( const DataType* ptr ) const
{
	if ( ptr )
	{
		S32 index = (S32)( ptr - Buffer::Chars() );
		return index;
	}
	return -1;
}

template <typename DataType, typename Buffer>
void StringBase<DataType, Buffer>::Append(const DataType* src, S32 srcLength)
{
	U32 oldLength = Buffer::Length();
	Buffer::Reserve(oldLength + srcLength);
	StringFunctions<DataType>::MemCpy(Buffer::NonConstChars() + oldLength, src, srcLength);
	SetLength(oldLength + srcLength);
}

template <typename DataType, typename Buffer>
bool StringBase<DataType, Buffer>::Resize(U32 length, bool assertOnFailure)
{
	U32 oldLength = Buffer::Length();

	if (oldLength < length)
	{
		Buffer::Reserve(length);

		if (Buffer::Capacity() < (S32) length)
		{
			if (assertOnFailure)
				BIZ_ASSERT_FAIL("Btl::String::Resize failed.");

			return false;
		}

		memset(&Buffer::NonConstChars()[oldLength], ' ', length - oldLength);
	}

	SetLength(length);

	return true;
}

/// @endcond
