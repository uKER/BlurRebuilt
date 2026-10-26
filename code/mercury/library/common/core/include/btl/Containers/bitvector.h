#ifndef BTL_BIT_VECTOR_H
#define BTL_BIT_VECTOR_H
#ifdef _MSC_VER
#pragma once
#endif

/////////////////////////////////////////////////////
// Bit Vector
/////////////////////////////////////////////////////

#include "btl/containers/vector.h"

namespace Btl
{

////////////////////////////////////////////////////

/// @addtogroup Containers
/// @{

/// Base type for BitVectorTemplate using vector as a buffer. 
template <typename BufferType>
class BitVectorBase
{
public:
	typedef typename BufferType::DataType DataType;

	BitVectorBase() : m_nBits(0) {}

	/// Number of bits.
	int Size() const {return m_nBits;}

	/// Number of elements reserved.
	int Capacity() const {return m_buffer.Capacity() * BitsPerElement();}

	/// Return true if the list is empty.
	bool Empty() const {return m_nBits == 0;}

	/// Change the number of elements (but don't free memory).
	void Resize(int nBits)
	{
		int nElements = (nBits + BitsPerElement() - 1) / BitsPerElement();
		if (nElements != m_buffer.Size())
			m_buffer.Resize(nElements);

		m_nBits = nBits;
	}

	/// Grow or shrink the capacity. Must have size not less than the number of elements.
	void Reserve(int nBits)
	{
		int nElements = (nBits + BitsPerElement() - 1) / BitsPerElement();
		m_buffer.Resize(nElements);
	}

	// Set the number of bits in use without changing the size of the vector.
	void SetNumBits(int nBits)
	{
		int nElements = (nBits + BitsPerElement() - 1) / BitsPerElement();
		assert(nElements <= m_buffer.Size());
		m_nBits = nBits;
	}

	const BufferType& GetBuffer() const {return m_buffer;}
	BufferType& GetBuffer() {return m_buffer;}

protected:
	BufferType m_buffer;

	int BitsPerElement() const {return sizeof(DataType) * 8;}

private:
	int m_nBits;
};

///////////////////////////////////////////////

/// Base type for BitVectorTemplate using a static vector as a buffer.
/// Main reason for this is to avoid storing a separate size variable.
template <typename BufferType>
class BitArrayBase
{
public:
	typedef typename BufferType::DataType DataType;

	/// Number of bits.
	int Size() const {return Capacity();}

	/// Number of elements reserved.
	int Capacity() const {return m_buffer.Capacity() * BitsPerElement();}

	/// An array is never empty.
	bool Empty() const {return false;}

	const BufferType& GetBuffer() const {return m_buffer;}
	BufferType& GetBuffer() {return m_buffer;}

	/// Resize does nothing but assert that the size is the same. 
	void Resize(int nBits) {BTL_ASSERT(nBits == Size());}
	void SetNumBits(int nBits) {BTL_ASSERT(nBits == Size());}

protected:
	BufferType m_buffer;

	int BitsPerElement() const {return sizeof(DataType) * 8;}
};

///////////////////////////////////////////////

/// BitVector template.
/// See @ref BitVector typedef.
template <typename Base>
class BitVectorTemplate : public Base
{
public:
	typedef typename Base::DataType DataType;

	// default copy constructor and = operator.

	//////////////////////////////////////////////////
	// iterators

	class ConstBitVectorAccessor;
	class BitVectorAccessor;

	/// @a 
	class ConstIterator
	{
	public:
		ConstIterator() {}
		ConstIterator(const ConstIterator& other);

		ConstIterator(const BitVectorTemplate* p, int index, DataType cachedBits);

		// comparison
		bool operator == (const ConstIterator& c) const;
		bool operator != (const ConstIterator& c) const;
		bool operator < (const ConstIterator& other) const;
		bool operator > (const ConstIterator& other) const;
		bool operator <= (const ConstIterator& other) const;
		bool operator >= (const ConstIterator& other) const;

		// Uses cached value for speed.
		// Any change to the vector invalidates ConstIterators.
		// Due to caching the iterator can't go backwards.
		ConstBitVectorAccessor& operator *() const {return (ConstBitVectorAccessor&)(*this);}
		ConstIterator& operator ++();
		ConstIterator operator ++(int);

		friend class ConstBitVectorAccessor;
		friend class BitVectorAccessor;

	private:
		DataType m_cachedBits; // to avoid doing a computed shift
		BitVectorTemplate* m_pVector;
		int m_index;
	};

	/// @a 
	class Iterator : public ConstIterator
	{
	public:
		Iterator() {}
		Iterator(const Iterator& other) : ConstIterator(other) {}
		Iterator(BitVectorTemplate* p, int index, DataType cachedBits) : ConstIterator(p, index, cachedBits) {}

		BitVectorAccessor& operator *() const {return (BitVectorAccessor&)(*this);}

		Iterator& operator ++() {++static_cast<ConstIterator&>(*this); return *this;}
		Iterator operator ++(int) {return static_cast<ConstIterator&>(*this)++;}
	};

	/////////////////////////////////////////////////

	// use these to access bits as bools
	class ConstBitVectorAccessor
	{
	public:
		FORCE_INLINE operator bool () const {return m_iterator.m_cachedBits & 1;}

	protected:
		ConstBitVectorAccessor(const BitVectorTemplate& v, int bit, DataType cachedBits) : m_iterator(&v, bit, cachedBits) {}
		
		FRIEND_CLASS BitVectorTemplate;
		FRIEND_CLASS ConstIterator;

		ConstIterator m_iterator;

	private:
		ConstBitVectorAccessor(const ConstBitVectorAccessor& other); // no copy
		ConstBitVectorAccessor& operator = (const ConstBitVectorAccessor& other);
	};

	class BitVectorAccessor : public ConstBitVectorAccessor
	{
	public:
		FORCE_INLINE BitVectorAccessor& operator = (bool value) 
		{
			ConstBitVectorAccessor::m_iterator.m_cachedBits |= (int(value));
			ConstBitVectorAccessor::m_iterator.m_cachedBits &= ~(int(!value));
			ConstBitVectorAccessor::m_iterator.m_pVector->SetBitValue(ConstBitVectorAccessor::m_iterator.m_index, value); return *this;
		}

	private:
		FRIEND_CLASS BitVectorTemplate;
		FRIEND_CLASS Iterator;

		BitVectorAccessor& operator = (const BitVectorAccessor& other);
		BitVectorAccessor(const BitVectorAccessor& other); // no copy
		BitVectorAccessor(BitVectorTemplate& v, int bit, DataType cachedBits) : ConstBitVectorAccessor(v, bit, cachedBits) {}
	};

	////////////////////////////////////////////////

	typedef ConstBitVectorAccessor ConstReferenceType;
	typedef BitVectorAccessor ReferenceType;
	using Base::Size;
	using Base::m_buffer;
	using Base::BitsPerElement;

	// iterators
	ConstIterator Begin() const {return ConstIterator(this, 0, Data()[0]);} ///< @a 
	ConstIterator End() const {return ConstIterator(this, Size(), 0);} ///< @a 
	Iterator Begin() {return Iterator(this, 0, Data()[0]);} ///< @a 
	Iterator End() {return Iterator(this, Size(), 0);} ///< @a 

	// Array-style access.
	ConstReferenceType operator [] (int i) const; ///< @a 
	ReferenceType operator [] (int i); ///< @a 

	/// Access first element.
	ReferenceType Front() {return (*this)[0];}

	/// Access last element.
	ReferenceType Back() {return (*this)[Size() - 1];}

	/// Access first element.
	ConstReferenceType Front() const {return (*this)[0];}

	/// Access last element.
	ConstReferenceType Back() const {return (*this)[Size() - 1];}

	/// Add one element to the end of the vector.
	void PushBack() {Resize(Size() + 1);}

	/// Add one element to the end of the vector and set its value.
	void PushBack(bool value) {Resize(Size() + 1); Back() = value;}

	/// Set the size to zero.
	void Clear() {Base::Resize(0);}

	/// Set the size to zero and release the memory used.
	void Destroy() {Base::Resize(0); m_buffer.Destroy();}

	////////////////////////////////////////////
	// Bitfield functions

	// raw data
	int SizeInBytes() const {return m_buffer.Size() * sizeof(DataType);} ///< @a 
	int BufferSize() const {return m_buffer.Size();} ///< @a 
	const DataType* Data() const {return &m_buffer[0];} ///< @a 
	DataType* Data() {return &m_buffer[0];} ///< @a 

	// individual bits
	bool GetBitValue(int i) const; ///< @a 
	int GetBitValueAsInt(int i) const; ///< @a 
	void SetBitValue(int i, bool value); ///< @a 
	void SetBitValueAsInt(int i, int value); ///< @a 
	void SetBit(int i); ///< @a 
	void ClearBit(int i); ///< @a 

	// functions to affect the whole vector
	bool AnyBitsSet() const; ///< @a
	S32 CountBitsSet() const;
	void ClearBits(); ///< @a 
	void SetBits(); ///< @a
	void Invert();

	template <typename Other> BitVectorTemplate& operator = (const BitVectorTemplate<Other>& other);

	template <typename Other> void operator |= (const BitVectorTemplate<Other>& other); ///< @a 
	template <typename Other> void operator &= (const BitVectorTemplate<Other>& other); ///< @a 
	template <typename Other> void operator ^= (const BitVectorTemplate<Other>& other); ///< @a 
	template <typename Other> const bool operator < (const BitVectorTemplate<Other>& other) const; ///< @a
	template <typename Other> const bool operator == (const BitVectorTemplate<Other>& other) const; ///< @a
	template <typename Other> const bool operator != (const BitVectorTemplate<Other>& other) const; ///< @a
};

// some typedefs to make this easier to use
typedef BitVectorTemplate<BitVectorBase<Vector<U8, VectorParameters_Dynamic<> > > > BitVector; ///< @a 
typedef BitVectorTemplate<BitVectorBase<Vector<U32, VectorParameters_Dynamic<> > > > BitVector32; ///< @a 
typedef BitVectorTemplate<BitArrayBase<Vector<U32, VectorParameters_Array<1> > > > BitArray32; ///< @a 
typedef BitVectorTemplate<BitArrayBase<Vector<U16, VectorParameters_Array<1> > > > BitArray16; ///< @a 
typedef BitVectorTemplate<BitArrayBase<Vector<U8, VectorParameters_Array<16> > > > BitArray128; ///< @a 

#include "bitvector.inl"

/// @}

}

#endif
