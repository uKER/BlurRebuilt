#pragma once

namespace Ut
{

////////////////////////////////////////////////////

// Bitfield using vector as a buffer. 
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

	/// Grow or shrink the capacity. Must have size  not less than the number of elements.
	void Reserve(int nBits)
	{
		int nElements = (nBits + BitsPerElement() - 1) / BitsPerElement();
		m_buffer.Resize(nElements);
	}

protected:
	BufferType m_buffer;

	int BitsPerElement() const {return sizeof(DataType) * 8;}

private:
	int m_nBits;
};

///////////////////////////////////////////////

// Bitfield using static vector as a buffer.
// Main reason for this is to avoid storing a separate size variable.
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

protected:
	BufferType m_buffer;

	int BitsPerElement() const {return sizeof(DataType) * 8;}
};

///////////////////////////////////////////////

template <typename Base>
class BitVectorTemplate : public Base
{
public:
	typedef typename Base::DataType DataType;

	// default copy constructor and = operator.

	//////////////////////////////////////////////////
	// vector functions

	// use these to access bits as bools
	class ConstBitVectorAccessor
	{
	public:
		ConstBitVectorAccessor(const BitVectorTemplate& v, int bit) : vector(const_cast<BitVectorTemplate&>(v)), m_bit(bit) {}

		__forceinline operator bool () {return vector.GetBitValue(m_bit);}

	protected:
		BitVectorTemplate& vector;
		int m_bit;
	};

	class BitVectorAccessor : public ConstBitVectorAccessor
	{
	public:
		BitVectorAccessor(BitVectorTemplate& v, int bit) : ConstBitVectorAccessor(v, bit) {}

		__forceinline BitVectorAccessor& operator = (bool value) {vector.SetBitValue(m_bit, value); return *this;}

		//to do: other operators
	};

	typedef ConstBitVectorAccessor ConstReferenceType;
	typedef BitVectorAccessor ReferenceType;

	// Array-style access.
	ConstReferenceType operator [] (int i) const {return ConstBitVectorAccessor(*this, i);}
	ReferenceType operator [] (int i) {return BitVectorAccessor(*this, i);}

	/// Access first element.
	ReferenceType Front() {return (*this)[0];}

	/// Access last element.
	ReferenceType Back() {return (*this)[Size() - 1];}

	/// Access first element.
	ConstReferenceType Front() const {return (*this)[0];}

	/// Access last element.
	ConstReferenceType Back() const {return (*this)[Size() - 1];}

	void PushBack()
	{
		Resize(Size() + 1);
	}

	void PushBack(bool value)
	{
		Resize(Size() + 1);
		Back() = value;
	}

	// to do: iterators

	////////////////////////////////////////////
	// Bitfield functions

	// raw data
	int SizeInBytes() const {return m_buffer.Size() * sizeof(DataType);}
	U8* Data() const {return (U8*) &m_buffer[0];}

	bool AnyBitsSet() const
	{
		int n = Size() / BitsPerElement();

		// check the main vector
		for (int i = 0; i < n; i++)
			if (m_buffer[i])
				return true;

		// check the last few bits
		for (i = n * BitsPerElement(); i < Size(); i++)
			if ((*this)[i]) return true;

		return false;
	}

	// individual bits
	__forceinline bool GetBitValue(int i) const
	{
		assert(i > -1 && i < Size());

		int index = i / BitsPerElement();
		int bit = i % BitsPerElement();
		return (m_buffer[index] & (1 << bit)) != 0;
	}

	__forceinline int GetBitValueAsInt(int i) const
	{
		assert(i > -1 && i < Size());

		int index = i / BitsPerElement();
		int bit = i % BitsPerElement();
		return (m_buffer[index] & (1 << bit));
	}

	__forceinline void SetBitValue(int i, bool value)
	{
		assert(i > -1 && i < Size());

		int index = i / BitsPerElement();
		int bit = i % BitsPerElement();
		if (value)
			m_buffer[index] |= (1 << bit);
		else
			m_buffer[index] &= ~(1 << bit);	
	}

	__forceinline void SetBitValueAsInt(int i, int value)
	{
		assert(i > -1 && i < Size());

		int index = i / BitsPerElement();
		int bit = i % BitsPerElement();
		
		if (value)
			m_buffer[index] |= (1 << bit);
		else
			m_buffer[index] &= ~(1 << bit);	
	}

	__forceinline void SetBit(int i)
	{
		assert(i > -1 && i < Size());

		int index = i / BitsPerElement();
		int bit = i % BitsPerElement();
		m_buffer[index] |= (1 << bit);
	}

	__forceinline void ClearBit(int i)
	{
		assert(i > -1 && i < Size());

		int index = i / BitsPerElement();
		int bit = i % BitsPerElement();
		m_buffer[index] &= ~(1 << bit);	
	}

	// functions to affect the whole vector
	void ClearBits() 
	{
		for (int i = 0; i < m_buffer.Size(); i++)
			m_buffer[i] = 0;
	}

	void SetBits() 
	{
		for (int i = 0; i < m_buffer.Size(); i++)
			m_buffer[i] = ~DataType(0);
	}

	void operator |= (const BitVectorTemplate& other)
	{
		assert(Size() == other.Size());

		for (int i = 0; i < m_buffer.Size(); i++)
			m_buffer[i] |= other.m_buffer[i];
	}

	void operator &= (const BitVectorTemplate& other)
	{
		assert(Size() == other.Size());

		for (int i = 0; i < m_buffer.Size(); i++)
			m_buffer[i] &= other.m_buffer[i];
	}
};

// some typedefs to make this easier to use
typedef BitVectorTemplate<BitVectorBase<Ut::UtVector<Ut::DynamicVector<U8> > > > BitVector;
typedef BitVectorTemplate<BitArrayBase<Ut::UtVector<Ut::ArrayVector<U32, 1> > > > BitArray32;
typedef BitVectorTemplate<BitArrayBase<Ut::UtVector<Ut::ArrayVector<U16, 1> > > > BitArray16;

template <int ID>
struct TrackedBitVector
{
	typedef BitVectorTemplate<BitVectorBase<Ut::UtVector<typename Ut::TrackedDynamicVector<U8, ID>::Result > > > Result;
};

}