/// @cond 

template <typename Base>
BitVectorTemplate<Base>::ConstIterator::ConstIterator(const ConstIterator& other)
{
	m_cachedBits = other.m_cachedBits;
	m_pVector = other.m_pVector;
	m_index = other.m_index;
}

template <typename Base>
BitVectorTemplate<Base>::ConstIterator::ConstIterator(const BitVectorTemplate* p, int index, DataType cachedBits)
{
	m_pVector = const_cast<BitVectorTemplate*>(p);
	m_index = index;
	m_cachedBits = cachedBits;
}

template <typename Base>
bool BitVectorTemplate<Base>::ConstIterator::operator != (const ConstIterator& c) const
{
	return m_index != c.m_index || m_pVector != c.m_pVector;
}

template <typename Base>
bool BitVectorTemplate<Base>::ConstIterator::operator == (const ConstIterator& c) const
{
	return m_index == c.m_index || m_pVector == c.m_pVector;
}	

template <typename Base>
typename BitVectorTemplate<Base>::ConstIterator& BitVectorTemplate<Base>::ConstIterator::operator ++()
{
	m_index++;
	int bitsPerElement = m_pVector->BitsPerElement();

	if (m_index % bitsPerElement != 0)
		m_cachedBits >>= 1;
	else if (m_index < m_pVector->Size())
		m_cachedBits = m_pVector->Data()[m_index / bitsPerElement];

	return *this;
}

template <typename Base>
typename BitVectorTemplate<Base>::ConstIterator BitVectorTemplate<Base>::ConstIterator::operator ++(int)
{
	ConstIterator i(this);
	++(*this);
	return i;
}

template <typename Base>
bool BitVectorTemplate<Base>::ConstIterator::operator < (const ConstIterator& other) const
{
	return m_index < other.m_index;
}

template <typename Base>
bool BitVectorTemplate<Base>::ConstIterator::operator > (const ConstIterator& other) const
{
	return m_index > other.m_index;
}

template <typename Base>
bool BitVectorTemplate<Base>::ConstIterator::operator <= (const ConstIterator& other) const
{
	return m_index <= other.m_index;
}

template <typename Base>
bool BitVectorTemplate<Base>::ConstIterator::operator >= (const ConstIterator& other) const
{
	return m_index >= other.m_index;
}

////////////////////////////////////////////////////////////////////////////

template <typename Base>
typename BitVectorTemplate<Base>::ConstReferenceType BitVectorTemplate<Base>::operator [] (int i) const 
{
	BTL_ASSERT(i >= 0 && i < Size());
	int bitsPerElement = BitsPerElement();
	DataType data = Data()[i / bitsPerElement];
	data >>= (i % bitsPerElement);
	return ConstBitVectorAccessor(*this, i, data);
}

template <typename Base>
typename BitVectorTemplate<Base>::ReferenceType BitVectorTemplate<Base>::operator [] (int i) 
{
	BTL_ASSERT(i >= 0 && i < Size());
	int bitsPerElement = BitsPerElement();
	DataType data = Data()[i / bitsPerElement];
	data >>= (i % bitsPerElement);
	return BitVectorAccessor(*this, i, data);
}

template <typename Base>
bool BitVectorTemplate<Base>::AnyBitsSet() const
{
	int n = Size() / BitsPerElement();

	// check the main vector
	for (int i = 0; i < n; i++)
	{
		if (m_buffer[i])
			return true;
	}

	// check the last few bits
	for (int i = n * BitsPerElement(); i < Size(); i++)
	{
		if ((*this)[i]) return true;
	}

	return false;
}

inline U32 BitCount(U32 v)
{    
	v = v - ((v >> 1) & 0x55555555);
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
	return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

template <typename Base>
S32 BitVectorTemplate<Base>::CountBitsSet() const
{
	int n = Size() / BitsPerElement();
	S32 count = 0;

	// count the main vector
	for (int i = 0; i < n; i++)
		count += BitCount(m_buffer[i]);

	// count the last few bits
	for (int i = n * BitsPerElement(); i < Size(); i++)
		if ((*this)[i]) count++;

	return count;
}

template <typename Base>
FORCE_INLINE bool BitVectorTemplate<Base>::GetBitValue(int i) const
{
	BTL_ASSERT(i > -1 && i < Size());

	int index = i / BitsPerElement();
	int bit = i % BitsPerElement();
	return (m_buffer[index] & (1 << bit)) != 0;
}

template <typename Base>
FORCE_INLINE int BitVectorTemplate<Base>::GetBitValueAsInt(int i) const
{
	BTL_ASSERT(i > -1 && i < Size());

	int index = i / BitsPerElement();
	int bit = i % BitsPerElement();
	return (m_buffer[index] & (1 << bit));
}

template <typename Base>
FORCE_INLINE void BitVectorTemplate<Base>::SetBitValue(int i, bool value)
{
	BTL_ASSERT(i > -1 && i < Size());

	int index = i / BitsPerElement();
	int bit = i % BitsPerElement();
	m_buffer[index] |= (int(value) << bit);
	m_buffer[index] &= ~(int(!value) << bit);	
}

template <typename Base>
FORCE_INLINE void BitVectorTemplate<Base>::SetBitValueAsInt(int i, int value)
{
	BTL_ASSERT(i > -1 && i < Size());

	int index = i / BitsPerElement();
	int bit = i % BitsPerElement();
	
	m_buffer[index] |= (int(value) << bit);
	m_buffer[index] &= ~(int(!value) << bit);
}

template <typename Base>
FORCE_INLINE void BitVectorTemplate<Base>::SetBit(int i)
{
	BTL_ASSERT(i > -1 && i < Size());

	int index = i / BitsPerElement();
	int bit = i % BitsPerElement();
	m_buffer[index] |= (1 << bit);
}

template <typename Base>
FORCE_INLINE void BitVectorTemplate<Base>::ClearBit(int i)
{
	BTL_ASSERT(i > -1 && i < Size());

	int index = i / BitsPerElement();
	int bit = i % BitsPerElement();
	m_buffer[index] &= ~(1 << bit);	
}

template <typename Base>
void BitVectorTemplate<Base>::ClearBits() 
{
	for (int i = 0; i < m_buffer.Size(); i++)
		m_buffer[i] = 0;
}

template <typename Base>
void BitVectorTemplate<Base>::SetBits() 
{
	for (int i = 0; i < m_buffer.Size(); i++)
		m_buffer[i] = ~DataType(0);
}

template <typename Base>
void BitVectorTemplate<Base>::Invert()
{
	for (int i = 0; i < m_buffer.Size(); i++)
		m_buffer[i] = ~m_buffer[i];
}

template <typename Base>
template <typename Other>
BitVectorTemplate<Base>& BitVectorTemplate<Base>::operator = (const BitVectorTemplate<Other>& other)
{
	BIZ_STATIC_ASSERT((!Btl::SameType<Base, Other>::result));

	Resize(other.Size());

	for (int i = 0; i < other.Size(); i++)
		(*this)[i] = other[i];

	return *this;
}

template <typename Base>
template <typename Other>
void BitVectorTemplate<Base>::operator |= (const BitVectorTemplate<Other>& other)
{
	BIZ_STATIC_ASSERT(sizeof(typename Base::DataType) == sizeof(typename Other::DataType));

	BTL_ASSERT(Size() == other.Size());

	for (int i = 0; i < m_buffer.Size(); i++)
		m_buffer[i] |= other.m_buffer[i];
}

template <typename Base>
template <typename Other>
void BitVectorTemplate<Base>::operator &= (const BitVectorTemplate<Other>& other)
{
	BIZ_STATIC_ASSERT(sizeof(typename Base::DataType) == sizeof(typename Other::DataType));

	BTL_ASSERT(Size() == other.Size());

	for (int i = 0; i < m_buffer.Size(); i++)
		m_buffer[i] &= other.m_buffer[i];
}

template <typename Base>
template <typename Other>
void BitVectorTemplate<Base>::operator ^= (const BitVectorTemplate<Other>& other)
{
	BIZ_STATIC_ASSERT(sizeof(typename Base::DataType) == sizeof(typename Other::DataType));

	BTL_ASSERT(Size() == other.Size());

	for (int i = 0; i < m_buffer.Size(); i++)
		m_buffer[i] ^= other.m_buffer[i];
}

template <typename Base>
template <typename Other>
const bool BitVectorTemplate<Base>::operator < (const BitVectorTemplate<Other>& other) const
{
	BIZ_STATIC_ASSERT(sizeof(typename Base::DataType) == sizeof(typename Other::DataType));

	if(Size() != other.Size())
		return Size() < other.Size();

	int n = Size() / BitsPerElement();

	S32 cmp = memcmp(Data(), other.Data(), n * sizeof(DataType));

	if (cmp == -1) return true;
	else if (cmp == 1) return false;

	// compare the last few bits
	for (int i = n * BitsPerElement(); i < Size(); i++)
		if ((*this)[i] < other[i])
			return true;
		else if ((*this)[i] > other[i])
			return false;

	return false;
}

template <typename Base>
template <typename Other>
const bool BitVectorTemplate<Base>::operator == (const BitVectorTemplate<Other>& other) const
{
	BIZ_STATIC_ASSERT(sizeof(typename Base::DataType) == sizeof(typename Other::DataType));

	if(Size() != other.Size())
		return false;
	
	int n = Size() / BitsPerElement();

	// count the main vector
	for (int i = 0; i < n; i++)
		if(m_buffer[i] != other.m_buffer[i])
			return false;

	// count the last few bits
	for (int i = n * BitsPerElement(); i < Size(); i++)
		if ((*this)[i] != other[i])
			return false;
	
	return true;
}

template <typename Base>
template <typename Other>
const bool BitVectorTemplate<Base>::operator != (const BitVectorTemplate<Other>& other) const
{
	BIZ_STATIC_ASSERT(sizeof(typename Base::DataType) == sizeof(typename Other::DataType));

	return !(*this == other);
}

/// @endcond 
