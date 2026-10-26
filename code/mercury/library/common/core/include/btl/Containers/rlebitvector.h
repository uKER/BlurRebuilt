#ifndef BTL_RLEBIT_VECTOR_H
#define BTL_RLEBIT_VECTOR_H
#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////
// Run Length Encoded Bit Vector
///////////////////////////////////////////////////////

#include <btl/containers/vector.h>
#include <btl/containers/bitvector.h>
#include <btl/utility/pair.h>

namespace Btl
{

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

template<typename StorageType>
class RleBitVector_t
{
public:
	RleBitVector_t();

	void Init( BitVector const & data );
	void Init( const U8 * data, int sizeInBits );
	void Init( int sizeInBytes, int uncompressedSizeInBytes );

	U8 const * Data() const
	{
		return reinterpret_cast<U8 const *>(&m_elements[0]);
	}
	
	int SizeInBytes() const { return m_elements.Size(); }
	int UncompressedSizeInBytes() const { return m_numUncompressedElements; }

	int CountSetBits() const;

	class ConstIterator;

	__forceinline ConstIterator begin() const;       // iterator begin
	__forceinline ConstIterator end() const;         // iterator end

	class ConstSkipIterator;

	__forceinline ConstSkipIterator sbegin() const;  // skip iterator begin
	__forceinline ConstSkipIterator send() const;    // skip iterator end

protected:
	void EncodeZeros( int numZeroBytes );

	StorageType m_elements;
	int         m_numUncompressedElements;
};

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

// Iterator over the run length encoded data, will return the bits for one byte
// at a time.

template<typename StorageType>
class RleBitVector_t<StorageType>::ConstIterator
{

public:
	inline ConstIterator();
	inline ConstIterator( const RleBitVector_t<StorageType> * );
	inline ConstIterator( const ConstIterator& );
	inline ConstIterator( int );

	__forceinline bool operator == ( const ConstIterator& it );
	__forceinline bool operator != ( const ConstIterator& it );
	__forceinline bool operator < ( const ConstIterator& it );
	__forceinline int operator * ();
	__forceinline void operator ++ ();

private:

	const RleBitVector_t<StorageType> * m_bitVector;

	int m_elementIndex;
	int m_numZeros;
	int m_currentBits;

	int m_remainingBytes;
};

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

template<typename StorageType>
RleBitVector_t<StorageType>::ConstIterator::ConstIterator()
	: m_bitVector( 0 )
	, m_remainingBytes( -1 )
{
}

template<typename StorageType>
RleBitVector_t<StorageType>::ConstIterator::ConstIterator( const RleBitVector_t<StorageType> * bitVector )
	: m_bitVector( bitVector )
	, m_elementIndex( 0 )
	, m_numZeros( 0 )
{
	m_remainingBytes = m_bitVector->UncompressedSizeInBytes();

	if ( m_remainingBytes )
	{
		BTL_ASSERT( m_bitVector->Data()[ m_bitVector->SizeInBytes() - 1 ] == 255 );

		m_currentBits = m_bitVector->Data()[ m_elementIndex++ ];
		if ( m_currentBits == 0 )
			m_numZeros = m_bitVector->Data()[ m_elementIndex++ ] - 1;
	}

	--m_remainingBytes;
}

template<typename StorageType>
RleBitVector_t<StorageType>::ConstIterator::ConstIterator( const ConstIterator& it )
	: m_bitVector( it.m_bitVector )
	, m_elementIndex( it.m_elementIndex )
	, m_numZeros( it.m_numZeros )
	, m_currentBits( it.m_currentBits )
	, m_remainingBytes( it.m_remainingBytes )
{
}

template<typename StorageType>
bool RleBitVector_t<StorageType>::ConstIterator::operator == ( const ConstIterator& it )
{
	return ( m_remainingBytes == it.m_remainingBytes );
}

template<typename StorageType>
bool RleBitVector_t<StorageType>::ConstIterator::operator != ( const ConstIterator& it )
{
	return ( m_remainingBytes != it.m_remainingBytes );
}


template<typename StorageType>
bool RleBitVector_t<StorageType>::ConstIterator::operator < ( const ConstIterator& it )
{
	return m_remainingBytes > it.m_remainingBytes;
}

template<typename StorageType>
int RleBitVector_t<StorageType>::ConstIterator::operator * ()
{
	BTL_ASSERT( m_remainingBytes != -1 );
	return m_currentBits;
}

template<typename StorageType>
void RleBitVector_t<StorageType>::ConstIterator::operator ++ ()
{
	BTL_ASSERT( m_remainingBytes != -1 );

	if ( m_numZeros )
	{
		--m_numZeros;
	}
	else
	{
		BTL_ASSERT( m_elementIndex < m_bitVector->SizeInBytes() );
		m_currentBits = m_bitVector->Data()[ m_elementIndex++ ];
		if ( m_currentBits == 0 )
		{
			BTL_ASSERT( m_elementIndex < m_bitVector->SizeInBytes() );
			m_numZeros = m_bitVector->Data()[ m_elementIndex++ ] - 1;
		}
	}

	--m_remainingBytes;

	// The iterator can go one past the end of the actual compressed data.  For that purpose
	//   an extra byte was added to the end of the compressed data.  The value for that byte
	//   is 255.

	BTL_ASSERT( m_remainingBytes != -1 || m_currentBits == 255 ); 
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

// Iterator over the run length encoded data, returns those bytes that have bits in
// them and the index of those bytes in the uncompressed data stream.

template<typename StorageType>
class RleBitVector_t<StorageType>::ConstSkipIterator
{

public:
	inline ConstSkipIterator();
	inline ConstSkipIterator( const RleBitVector_t<StorageType> * );
	inline ConstSkipIterator( const ConstSkipIterator& );
	inline ConstSkipIterator( int );

	__forceinline bool operator == ( const ConstSkipIterator& it );
	__forceinline bool operator != ( const ConstSkipIterator& it );
	__forceinline bool operator < ( const ConstSkipIterator& it );
	__forceinline void operator ++ ();

	__forceinline Pair< int, int > operator * () const;

	__forceinline int Bits() const;
	__forceinline int Index() const;

private:

	const RleBitVector_t<StorageType> * m_bitVector;

	int m_elementIndex;
	int m_numZeros;
	int m_currentBits;

	int m_numBytes;
	int m_remainingBytes;
};

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

template<typename StorageType>
RleBitVector_t<StorageType>::ConstSkipIterator::ConstSkipIterator()
	: m_bitVector( 0 )
	, m_remainingBytes( -1 )
{
}

template<typename StorageType>
RleBitVector_t<StorageType>::ConstSkipIterator::ConstSkipIterator( const RleBitVector_t<StorageType> * bitVector )
	: m_bitVector( bitVector )
	, m_elementIndex( 0 )
	, m_numZeros( 0 )
{
	m_numBytes = m_remainingBytes = m_bitVector->UncompressedSizeInBytes();

	if ( m_remainingBytes )
	{
		BTL_ASSERT( m_bitVector->Data()[ m_bitVector->SizeInBytes() - 1 ] == 255 );

		for (;;)
		{
			m_currentBits = m_bitVector->Data()[ m_elementIndex++ ];
			if ( m_currentBits != 0 )
				break;

			m_remainingBytes -= m_bitVector->Data()[ m_elementIndex++ ];
		}
	}

	--m_remainingBytes;

	BTL_ASSERT( m_remainingBytes >= -1 );
}

template<typename StorageType>
RleBitVector_t<StorageType>::ConstSkipIterator::ConstSkipIterator( const ConstSkipIterator& it )
	: m_bitVector( it.m_bitVector )
	, m_elementIndex( it.m_elementIndex )
	, m_numZeros( it.m_numZeros )
	, m_currentBits( it.m_currentBits )
	, m_numBytes( it.m_numBytes )
	, m_remainingBytes( it.m_remainingBytes )
{
}

template<typename StorageType>
bool RleBitVector_t<StorageType>::ConstSkipIterator::operator == ( const ConstSkipIterator& it )
{
	return ( m_remainingBytes == it.m_remainingBytes );
}

template<typename StorageType>
bool RleBitVector_t<StorageType>::ConstSkipIterator::operator != ( const ConstSkipIterator& it )
{
	return ( m_remainingBytes != it.m_remainingBytes );
}

template<typename StorageType>
bool RleBitVector_t<StorageType>::ConstSkipIterator::operator < ( const ConstSkipIterator& it )
{
	return m_remainingBytes > it.m_remainingBytes;
}

template<typename StorageType>
Pair< int, int > RleBitVector_t<StorageType>::ConstSkipIterator::operator * () const
{
	return Pair< int, int >( m_numBytes - m_remainingBytes - 1, m_currentBits );
}

template<typename StorageType>
int RleBitVector_t<StorageType>::ConstSkipIterator::Bits() const
{
	return m_currentBits;
}

template<typename StorageType>
int RleBitVector_t<StorageType>::ConstSkipIterator::Index() const
{
	return m_numBytes - m_remainingBytes - 1;
}

template<typename StorageType>
void RleBitVector_t<StorageType>::ConstSkipIterator::operator ++ ()
{
	BTL_ASSERT( m_remainingBytes != -1 );

	for (;;)
	{
		m_currentBits = m_bitVector->Data()[ m_elementIndex ];
		++m_elementIndex;

		if ( m_currentBits != 0 )
		{
			--m_remainingBytes;
			break;
		}

		int numZeros = m_bitVector->Data()[ m_elementIndex ];
		++m_elementIndex;

		m_remainingBytes -= numZeros;
	}

	// The iterator can go one past the end of the actual compressed data.  For that purpose
	//   an extra byte was added to the end of the compressed data.  The value for that byte
	//   is 255.

	BTL_ASSERT( (m_remainingBytes != -1) || (m_currentBits == 255) ); 
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

template<typename StorageType>
typename RleBitVector_t<StorageType>::ConstIterator RleBitVector_t<StorageType>::begin() const
{
	return ConstIterator( this );
}

template<typename StorageType>
typename RleBitVector_t<StorageType>::ConstIterator RleBitVector_t<StorageType>::end() const
{
	return ConstIterator();
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------


template<typename StorageType>
typename RleBitVector_t<StorageType>::ConstSkipIterator RleBitVector_t<StorageType>::sbegin() const
{
	return ConstSkipIterator( this );
}

template<typename StorageType>
typename RleBitVector_t<StorageType>::ConstSkipIterator RleBitVector_t<StorageType>::send() const
{
	return ConstSkipIterator();
}

typedef RleBitVector_t<Vector< U8 > > RleBitVector;


//////////////////////////////////////////////////////////////////////////////////

template<typename StorageType>
RleBitVector_t<StorageType>::RleBitVector_t()
	: m_numUncompressedElements( 0 )
{
}

template<typename StorageType>
void RleBitVector_t<StorageType>::EncodeZeros( int numZeroBytes )
{
	while ( numZeroBytes )
	{
		m_elements.PushBack( 0 );

		if ( numZeroBytes > 255 )
		{
			m_elements.PushBack( 255 );
			numZeroBytes -= 255;
		}
		else
		{
			m_elements.PushBack( static_cast<U8>(numZeroBytes) );
			numZeroBytes = 0;
		}
	}
}

template<typename StorageType>
void RleBitVector_t<StorageType>::Init( BitVector const & bitVector )
{
	int numBits = bitVector.Size();
	if ( numBits == 0 )
		return;

	const U8 * data = bitVector.Data(); 
	Init( data, numBits );
}

template<typename StorageType>
void RleBitVector_t<StorageType>::Init( const U8* data, int numBits )
{
	if ( numBits == 0 )
		return;

	int numZeroBytes = 0;
	int numBytes = numBits / 8;

	for (int byte = 0; byte < numBytes; ++byte)
	{
		// If all bits in the byte are 0 then they are run length encoded

		U8 bits = data[byte];
		if ( bits == 0 )
		{
			++numZeroBytes;
		}
		else
		{
			// Finish up any previous run length encoding
			EncodeZeros( numZeroBytes );
			numZeroBytes = 0;

			// Store the current byte.
			m_elements.PushBack( bits );
		}
	}

	// Deal with any bits that are left over at the end

	int numRemainingBits = numBits % 8;
 	if ( numRemainingBits )
	{
		int mask = 0xff >> (8 - numRemainingBits);

		U8 bits = data[numBytes] & mask;
		if ( bits == 0 )
		{
			++numZeroBytes;
		}
		else
		{
			// Finish up any previous run length encoding
			EncodeZeros( numZeroBytes );
			numZeroBytes = 0;

			// Store the current byte.
			m_elements.PushBack( bits );
		}
	}

	// Finaly if the last few bits were all zero then we need to keep track of that as well.
	EncodeZeros( numZeroBytes );
	numZeroBytes = 0;

	// Put a final terminating byte into the vector to simplify the iteration logic.
	m_elements.PushBack( 255 );

	m_numUncompressedElements = (numBits + 7) / 8;
}

template<typename StorageType>
void RleBitVector_t<StorageType>::Init( int numElements, int numUncompressedElements )
{
	m_elements.Resize( numElements );

	if ( numElements )
	memset( &m_elements[0], 0, sizeof(U8) * numElements );

	m_numUncompressedElements = numUncompressedElements;
}

template<typename StorageType>
int RleBitVector_t<StorageType>::CountSetBits() const
{
	int count = 0;

	ConstSkipIterator it = sbegin();
	while ( it != send() )
	{
		int bits = (*it).second;
		while ( bits )
		{
			count += (bits & 1);
			bits >>= 1;
		}

		++it;
	}

	return count;
}

}; // namespace

#endif


