template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize>::C_StringBase()
{
	//Set null string.
	m_localData[0] = 0;
	//Flag as using local storage.
	m_localData[BufferSize-1] = 0; 
}

template <typename DataType, typename Allocator, U32 BufferSize>
template< int i >    
C_StringBase<DataType, Allocator, BufferSize>::C_StringBase( const C_FixedString<i>& fixedString )
{
	BIZ_STATIC_ASSERT(i);
	const S32 size = _StrSize(&fixedString[0]);
	assert(size);
	_InitialiseStorage(size);
	_MemCpy(&fixedString[0], size);
}

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize>::C_StringBase( const DataType* src )
{
	const S32 size = _StrSize(src);
	_InitialiseStorage(size);
	_MemCpy(src, size);
}

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize>::C_StringBase(const DataType* src, S32 count)
{
	assert(src);
	assert(count >= 0);
	assert((S32)_StrLen(src) >= count);
	_InitialiseStorage(count+1);
	_MemCpy(src, count+1);
}

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize>::C_StringBase( const DataType c, const S32 count)
{
	_InitialiseStorage(count+1);
	DataType* pData = NonConstChars();
	memset( pData, c, count );
	//Unleash the null terminator.
	pData[ count ] = 0;
}

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize>::C_StringBase( const C_StringBase& src )

{
	//Set null string.
	m_localData[0] = 0;
	//Flag as using local storage.
	m_localData[BufferSize-1] = 0; 
	*this = src;
}
 
template <typename DataType, typename Allocator, U32 BufferSize>
const C_StringBase<DataType, Allocator, BufferSize>& C_StringBase<DataType, Allocator, BufferSize>::operator = ( const C_StringBase& src )
{
	if (this == &src) 
		return *this;
	const DataType* srcData = src.Chars();
	S32 size = _StrSize(srcData);
	_Resize(size);
	_MemCpy(NonConstChars(), srcData, size);
	return *this;
}

template <typename DataType, typename Allocator, U32 BufferSize>
const C_StringBase<DataType, Allocator, BufferSize>& C_StringBase<DataType, Allocator, BufferSize>::operator = ( const DataType* src )
{
	if (Chars() == src)
		return *this;

	S32 size = _StrSize(src);
	_Resize(size);
	_MemCpy(NonConstChars(), src, size);
	return *this;
}

template <typename DataType, typename Allocator, U32 BufferSize>
template<typename Type> 
Type C_StringBase<DataType, Allocator, BufferSize>::_Min(const Type lhs, const Type rhs) const
{
	return (lhs > rhs) ? rhs : lhs;
}

template <typename DataType, typename Allocator, U32 BufferSize>
template<typename Type> 
Type C_StringBase<DataType, Allocator, BufferSize>::_Max(const Type lhs, const Type rhs) const
{
	return (lhs > rhs) ? lhs : rhs;
}


template <typename DataType, typename Allocator, U32 BufferSize>
S32 C_StringBase<DataType, Allocator, BufferSize>::Length() const
{
	if (_IsUsingLocalStorage())
		return _StrLen(&m_localData[0]);

	return m_heapSize-1;
}

template <typename DataType, typename Allocator, U32 BufferSize>
const C_StringBase<DataType, Allocator, BufferSize>& C_StringBase<DataType, Allocator, BufferSize>::Printf( const DataType* format, ... )
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

	const S32 length = (S32)_StrLen(buffer)+1;

	_Resize(length);
	DataType* pDst = NonConstChars();
	const DataType* pSrc = buffer;
	_MemCpy(pDst, pSrc, length);
    
	return *this;
}

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize> C_StringBase<DataType, Allocator, BufferSize>::Format( const DataType* format, ... )
{
	DataType buffer[ 1024 ];
    
	va_list arglist;
	va_start( arglist, format );
	vsprintf_s( buffer, 1024, format, arglist );
	va_end( arglist );
	
	return C_StringBase<DataType, Allocator, BufferSize>( buffer );
}

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize> C_StringBase<DataType, Allocator, BufferSize>::operator + ( const C_StringBase& src ) const
{
	const S32 thisLength = Length();
	const S32 totalLength = src.Length() + thisLength;

	C_StringBase ret;
	ret._Resize(totalLength+1);

	DataType* pDst2;

	//copy from this string to ret
	DataType* pDst1 = ret.NonConstChars();

	const DataType* pSrc1 = Chars();
	(const_cast<C_StringBase*>(this))->_MemCpy(pDst1, pSrc1, thisLength);

	//copy from other string to ret
	pDst2 = &(ret.NonConstChars())[thisLength];

	const DataType* pSrc2 = &(src.Chars())[0];
	(const_cast<C_StringBase*>(this))->_MemCpy(pDst2, pSrc2,(totalLength-thisLength)+1);
	
	return ret;
}

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize> C_StringBase<DataType, Allocator, BufferSize>::operator + (const DataType* src) const
{
	const S32 thisLength = Length();
	const S32 totalLength = (S32)strlen(src) + thisLength;

	C_StringBase ret;
	ret._Resize(totalLength+1);

	DataType* pDst2;

	//copy from this string to ret
	DataType* pDst1 = ret.NonConstChars();

	const DataType* pSrc1 = Chars();
	(const_cast<C_StringBase*>(this))->_MemCpy(pDst1, pSrc1, thisLength);

	//copy from other string to ret
	pDst2 = &(ret.NonConstChars())[thisLength];

	(const_cast<C_StringBase*>(this))->_MemCpy(pDst2, src,(totalLength-thisLength)+1);
	
	return ret;
}

template <typename DataType, typename Allocator, U32 BufferSize>
const C_StringBase<DataType, Allocator, BufferSize>& C_StringBase<DataType, Allocator, BufferSize>::operator += ( const C_StringBase& src )
{
	_Concat(src.Chars(), src.Length());
	return *this;
}

template <typename DataType, typename Allocator, U32 BufferSize>
const C_StringBase<DataType, Allocator, BufferSize>& C_StringBase<DataType, Allocator, BufferSize>::operator += ( const DataType* src )
{
	_Concat(src, _StrLen(src));
	return *this;
}

template <typename DataType, typename Allocator, U32 BufferSize>
/// Sub-string extraction operator.
/// NOTE: lo = inclusive, hi = exclusive, range ( 0, 0 ) denotes an empty string!
C_StringBase<DataType, Allocator, BufferSize> C_StringBase<DataType, Allocator, BufferSize>::operator () ( S32 lo, S32 hi ) const
{
	const S32 length = Length();
	hi = _Min( hi, length );
	lo = _Min( lo, hi );
	const S32 size = hi - lo;

	C_StringBase ret;
	ret._Resize(size+1);

	DataType* pDst = ret.NonConstChars();
	const DataType* pSrc = &(Chars())[lo];
	(const_cast<C_StringBase*>(this))->_MemCpy(pDst, pSrc, size);
    
	//Null terminate.
	pDst[size] = 0;

	return ret;
}

template <typename DataType, typename Allocator, U32 BufferSize>
template <typename Archive>
void C_StringBase<DataType, Allocator, BufferSize>::Deserialise(Archive &ar)
{
	U32 n;
	ar >> n;
	_Resize(n+1);
	DataType* pData = NonConstChars();
	for (U32 i=0; i<n; i++)
		ar >> pData[i];
	pData[n] = 0;
}

template <typename DataType, typename Allocator, U32 BufferSize>
template <typename Archive>
void C_StringBase<DataType, Allocator, BufferSize>::Serialise(Archive &ar)
{
	U32 n = Length();
	const DataType *pChars = Chars();
	ar << n;
	for (U32 i=0; i<n; i++)
	{
		ar << pChars[i];
	}
}

/// Forward Search for char
/// returns -1 if the substring was not found
template <typename DataType, typename Allocator, U32 BufferSize>
S32 C_StringBase<DataType, Allocator, BufferSize>::Search( const DataType c, S32 offset ) const
{
	offset = _Min( offset, Length() - 1 );
	const DataType* ptr = strchr( Chars() + offset, c );
	return _PtrIndex( ptr );
}

/// Reverse search for char
/// returns -1 if the substring was not found
template <typename DataType, typename Allocator, U32 BufferSize>
S32 C_StringBase<DataType, Allocator, BufferSize>::RevSearch( const DataType c, S32 offset ) const
{
	offset = _Min( offset, Length() - 1 );
	const DataType* ptr = strrchr( Chars() + offset, c );
	return _PtrIndex( ptr );
}

/// Forward search for sub-string
/// returns -1 if the substring was not found
template <typename DataType, typename Allocator, U32 BufferSize>
S32 C_StringBase<DataType, Allocator, BufferSize>::Search( const DataType* search, S32 offset) const
{
	S32 index = -1;
	const S32 length = Length();
	if ( length > 0 )
	{
		offset = _Min( offset, length - 1 );
		const DataType* start = Chars() + offset;
		const DataType* ptr = strstr( start, search );

		index = _PtrIndex( ptr );
	}
	
	return index;
}

template <typename DataType, typename Allocator, U32 BufferSize>
S32 C_StringBase<DataType, Allocator, BufferSize>::Search( const C_StringBase& search, S32 offset) const
{
	return Search( search.Chars(), offset );
}

/// case insensitive Forward search for sub-string
/// returns -1 if the substring was not found
template <typename DataType, typename Allocator, U32 BufferSize>
S32 C_StringBase<DataType, Allocator, BufferSize>::ISearch( const C_StringBase& search, const S32 offset) const
{
	//Working version which isn't amazingly efficient, replace with Boyer-Moore 
	//or something - Ian
	const S32 length = Length();
	const S32 searchLength = search.Length();

	S32 charMatches = 0;
	for (S32 i=offset; i<length; i++)
	{
		const DataType c1 = Lower(Chars()[i]);

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

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize> C_StringBase<DataType, Allocator, BufferSize>::Replace( 
	const C_StringBase& src, const C_StringBase& dst ) const
{
	C_StringBase result;
	const S32 length = Length();
	const S32 srcLength = src.Length();
	if (0 == srcLength)
	{
		return src;
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
template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize> C_StringBase<DataType, Allocator, BufferSize>::IReplace( 
	const C_StringBase& src, const C_StringBase& dst ) const
{
	C_StringBase result;
	const S32 length = Length();
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
template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::Replace(S32 start, S32 count, const DataType* src)
{
	assert(count >= 0);
	assert(start >= 0);
	assert((S32)strlen(src) >= count);
	assert(Length() >= start + count);

	DataType* dst = &NonConstChars()[start];
	_MemCpy( dst, src, count );
}

/// Copy fixed length buffer
template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::CopyFrom( const DataType* src, S32 length )
{
	if (src)
	{
		_Resize(length + 1);
		DataType* dst = NonConstChars();
		_MemCpy( dst, src, length );        	    
		dst[ length ] = 0;
	}
}

/// Copy char to fixed length buffer
template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::Copy( DataType* buffer, S32 length ) const
{
	strncpy_s( buffer, length, Chars(), length - 1 );
	buffer[ length - 1 ] = 0;
}

/// Equality
template <typename DataType, typename Allocator, U32 BufferSize>
const bool C_StringBase<DataType, Allocator, BufferSize>::operator == ( const C_StringBase& src ) const
{
	return strcmp( Chars(), src.Chars() ) == 0;
}

/// less than
template <typename DataType, typename Allocator, U32 BufferSize>
const bool C_StringBase<DataType, Allocator, BufferSize>::operator < ( const C_StringBase& src ) const
{
	return strcmp( Chars(), src.Chars() ) < 0;
}

/// Char Array Equality
template <typename DataType, typename Allocator, U32 BufferSize>
const bool C_StringBase<DataType, Allocator, BufferSize>::operator == ( const DataType* src ) const
{
	return strcmp( Chars(), src ) == 0;
}

/// Inequality
template <typename DataType, typename Allocator, U32 BufferSize>
const bool C_StringBase<DataType, Allocator, BufferSize>::operator != ( const C_StringBase& src ) const
{
	return strcmp( Chars(), src.Chars() ) != 0;
}
/// Char Array Inequality
template <typename DataType, typename Allocator, U32 BufferSize>
const bool C_StringBase<DataType, Allocator, BufferSize>::operator != ( const DataType* src ) const
{
	return strcmp( Chars(), src ) != 0;
}

template <typename DataType, typename Allocator, U32 BufferSize>
DataType C_StringBase<DataType, Allocator, BufferSize>::Lower(const DataType c) const
{
	DataType ret = c;

#ifdef PLATFORM_PS3
	if (__builtin_expect((c <= 'Z') & (c >= 'A'),0))
#else
	if ((c <= 'Z') && (c >= 'A'))
#endif
	{
		ret += ('a'-'A');
	}

	return ret;
}

template <typename DataType, typename Allocator, U32 BufferSize>
DataType C_StringBase<DataType, Allocator, BufferSize>::Upper(const DataType c) const
{
	DataType ret = c;

	if ((c <= 'z') && (c >= 'a'))
	{
		ret -= ('a'-'A');
	}

	return ret;
}

template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::ConvertToUpper()
{
	//inplace version of upper...
	const S32 length = Length();

	DataType* pData = NonConstChars();
	for (S32 i=0; i<length; i++)
		pData[i] = Upper(pData[i]);
}

template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::ConvertToLower()
{
	//inplace version of lower...
	const S32 length = Length();

	DataType* pData = NonConstChars();
	for (S32 i=0; i<length; i++)
		pData[i] = Lower(pData[i]);
}

/// Convert to upper case
template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize> C_StringBase<DataType, Allocator, BufferSize>::Upper() const
{
	C_StringBase ret;
	ret = *this;
	ret.ConvertToUpper();
	return ret;
}

/// Convert to lower case
template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize> C_StringBase<DataType, Allocator, BufferSize>::Lower() const
{
	C_StringBase ret(*this);
	ret.ConvertToLower();
	return ret;
}

template <typename DataType, typename Allocator, U32 BufferSize>
const void C_StringBase<DataType, Allocator, BufferSize>::Tokenise(Ut::Vector<C_StringBase>& strings, const char tok) const
{
	const S32 length = Length();

	Ut::UtVector< 
		Ut::DynamicVector<DataType, Ut::ArrayAllocator<DataType> > > tmp;

	const DataType* pSrcChars = Chars();
	for (S32 i=0; i<length; i++)
	{
		const DataType c = pSrcChars[i];
		if (c == tok)
		{
			if (!tmp.Empty())
			{
				tmp.PushBack(0);
				strings.PushBack(C_StringBase(&tmp[0]));
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
		strings.PushBack(C_StringBase(&tmp[0]));
	}
}

#ifdef ALLOW_UTIL_CSTRING_IMPLICIT_CASTS

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize>::operator const DataType* () const
{
	return Chars();
}

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize>::operator DataType* ()
{
	return NonConstChars();
}

#endif

template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::Clear()
{
	if (!_IsUsingLocalStorage())
	{
		_FreeStorage();
	}
	else
	{
		//Null terminate.
		m_localData[0] = 0;
	}
}

template <typename DataType, typename Allocator, U32 BufferSize>
DEPRECATED void C_StringBase<DataType, Allocator, BufferSize>::Print() const 
{
	// JAN: Deprecated. Don't want core\String.h to depend on Windows.h
//		OutputDebugString( Chars() );
//		OutputDebugString( "\n" );
}

template <typename DataType, typename Allocator, U32 BufferSize>
C_StringBase<DataType, Allocator, BufferSize>::~C_StringBase()
{
	if (!_IsUsingLocalStorage())
	{
		_FreeStorage();
	}
}

template <typename DataType, typename Allocator, U32 BufferSize>
const DataType* C_StringBase<DataType, Allocator, BufferSize>::Chars() const
{
	if (_IsUsingLocalStorage())
		return &m_localData[0];

	return m_pHeapData;
}


/// Returns index of character ptr
template <typename DataType, typename Allocator, U32 BufferSize>
S32 C_StringBase<DataType, Allocator, BufferSize>::_PtrIndex( const DataType* ptr ) const
{
	if ( ptr )
	{
		S32 index = (S32)( ptr - Chars() );
		return index;

	}
	return -1;
}

template <typename DataType, typename Allocator, U32 BufferSize>
S32 C_StringBase<DataType, Allocator, BufferSize>::_StrLen(const DataType* __restrict src) const
{
	S32 len = (S32)strlen(src);
	assert(len != -1);
	return len;
}

template <typename DataType, typename Allocator, U32 BufferSize>
S32 C_StringBase<DataType, Allocator, BufferSize>::_StrSize(const DataType* __restrict src) const
{
	return _StrLen(src)+1;
}

template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::_MemCpy(DataType* __restrict dst, const DataType* __restrict src, S32 size) 
{
	//Test for overlap of pointers... not in speed though...
#ifndef CFG_SPEED
	assert(abs((int)(src-dst)) >= size); //pointer aliasing check, we need __restrict pointers
#endif

	memcpy(dst, src, sizeof(DataType)*size);
	
}

template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::_MemCpy(const DataType* __restrict src, S32 size) 
{
	_MemCpy(NonConstChars(), src, size);
}

template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::_Concat(const DataType* src, S32 srcLength)
{
	const U32 growthFactor = 2;
	const U32 thisLength = (U32)Length();
	const U32 totalLength = srcLength + thisLength;
	const U32 totalSize = totalLength + 1;

	//If we don't need to use the heap.
	if (totalSize < BufferSize -1)
	{
		assert(_IsUsingLocalStorage());

		DataType tmp[BufferSize-1];
		//Copy first fragment.
		_MemCpy(tmp, m_localData, thisLength);
		//Copy second fragment.
		_MemCpy(tmp+thisLength, src, (totalLength-thisLength)+1);

		_MemCpy(m_localData, tmp, totalSize);
		return;
	}

	//We need to start using the heap.
	if (_IsUsingLocalStorage() && totalSize >= BufferSize-1)
	{
		U32 newCapacity = _Max(totalSize, (BufferSize-1)*growthFactor);
		DataType* pTmp = Allocator::Allocate(newCapacity);
		assert(pTmp);
		//Copy first fragment.
		_MemCpy(pTmp, m_localData, thisLength);
		//Copy second fragment.
		_MemCpy(pTmp+thisLength, src, (totalLength-thisLength)+1);

		m_pHeapData = pTmp;
		m_heapSize = totalSize;
		m_heapCapacity = newCapacity;

		//Flag as using the heap
		m_localData[BufferSize-1] = 1;
		return;
	}
	
	//We're already using the heap.
	assert(!_IsUsingLocalStorage());
	{
		if (totalSize > m_heapCapacity)
		{
			U32 newCapacity = _Max(totalSize, m_heapCapacity*growthFactor);
			DataType* pTmp = Allocator::Allocate(newCapacity);
			assert(pTmp);
			//Copy first fragment.
			_MemCpy(pTmp, m_pHeapData, thisLength);
			//Copy second fragment.
			_MemCpy(pTmp+thisLength, src, (totalLength-thisLength)+1);

			Allocator::Free(m_pHeapData, m_heapCapacity);
			m_heapCapacity = newCapacity;
			m_pHeapData = pTmp;
		}
		else
		{
			//Copy second fragment.
			_MemCpy(m_pHeapData+thisLength, src, (totalLength-thisLength)+1);
		}
		
		m_heapSize = totalSize;
	}
}

//Note that this is SIZE not length. Size = length+1
template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::_Resize(U32 size) 
{
	assert(size > 0);

	bool usingLocalStorage = _IsUsingLocalStorage();

	//Use local storage.
	if (size <= BufferSize-1)
	{
		//Already using local storage.
		if (usingLocalStorage)
		{
			//Null terminate.
			m_localData[size-1] = 0;
		}
		//Previously using heap.
		else
		{
			S32 heapSize = m_heapSize;
			DataType* pOldData = m_pHeapData;

			//Truncate.
			m_localData[size-1] = 0;
			
			//Free old heap data.
			Allocator::Free(pOldData, heapSize);

			//Flag as using local storage.
			m_localData[BufferSize-1] = 0;
		}

		return;
	}

	//Local -> Heap Copy.
	if (usingLocalStorage)
	{
		//Alloc storage.
		m_pHeapData = Allocator::Allocate(size);
		assert(m_pHeapData);
		m_heapSize = size;
		m_heapCapacity = size;

		//Flag as using the heap.
		m_localData[BufferSize-1] = 1;
	}

	//Heap -> Heap copy.
	else
	{
		if (m_heapSize == size)
			return;

		//Free old heap data.
		Allocator::Free(m_pHeapData, m_heapSize);

		//Alloc storage.
		m_pHeapData = Allocator::Allocate(size);
		assert(m_pHeapData);
		m_heapSize = size;
		m_heapCapacity = m_heapSize;
	}
}

template <typename DataType, typename Allocator, U32 BufferSize>
bool C_StringBase<DataType, Allocator, BufferSize>::_IsUsingLocalStorage() const
{
	if (m_localData[BufferSize-1] == 0)
		return true;
	return false;
}

template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::_InitialiseStorage(U32 size) 
{
	//We need to ensure we're not going to trample on our local vs. heap flag.
	if (size < BufferSize-1)
	{
		m_localData[0] = 0;
		//Use local storage.
		m_localData[BufferSize-1] = 0;
	}
	else
	{
		//Alloc storage.
		m_pHeapData = Allocator::Allocate(size);
		assert(m_pHeapData);
		m_heapSize = size;
		m_heapCapacity = size;

		//Flag as using the heap
		m_localData[BufferSize-1] = 1;
	}
}

template <typename DataType, typename Allocator, U32 BufferSize>
void C_StringBase<DataType, Allocator, BufferSize>::_FreeStorage()
{
	Allocator::Free(m_pHeapData, m_heapSize);
	//Flag as using local storage.
	m_localData[BufferSize-1] = 0;
	//Null terminate.
	m_localData[0] = 0;
}

template <typename DataType, typename Allocator, U32 BufferSize>
DataType* C_StringBase<DataType, Allocator, BufferSize>::NonConstChars()
{
	if (_IsUsingLocalStorage())
		return &m_localData[0];

	return m_pHeapData;
}
