#pragma once

#include "vector.h"
#include "bitvector.h"

namespace Ut
{

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

class RleBitVector {

public:
	RleBitVector();

	void Init( Ut::BitVector const & data );
	void Init( int sizeInBytes, int uncompressedSizeInBytes );

	U8 const * Data() const { return reinterpret_cast<U8 const *>(&m_elements[0]); }
	
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

	Ut::UtVector< DynamicVector< U8 > > m_elements;
	int                                 m_numUncompressedElements;
};

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

// Iterator over the run length encoded data, will return the bits for one byte
// at a time.

class RleBitVector::ConstIterator
{

public:
	inline ConstIterator();
	inline ConstIterator( const RleBitVector * );
	inline ConstIterator( const ConstIterator& );
	inline ConstIterator( int );

	__forceinline bool operator == ( const ConstIterator& it );
	__forceinline bool operator != ( const ConstIterator& it );
	__forceinline bool operator < ( const ConstIterator& it );
	__forceinline int operator * ();
	__forceinline void operator ++ ();

private:

	const RleBitVector * m_bitVector;

	int m_elementIndex;
	int m_numZeros;
	int m_currentBits;

	int m_remainingBytes;
};

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

RleBitVector::ConstIterator::ConstIterator()
	: m_bitVector( 0 )
	, m_remainingBytes( -1 )
{
}

RleBitVector::ConstIterator::ConstIterator( const RleBitVector * bitVector )
	: m_bitVector( bitVector )
	, m_elementIndex( 0 )
	, m_numZeros( 0 )
{
	m_remainingBytes = m_bitVector->UncompressedSizeInBytes();

	if ( m_remainingBytes )
	{
		assert( m_bitVector->Data()[ m_bitVector->SizeInBytes() - 1 ] == 255 );

		m_currentBits = m_bitVector->Data()[ m_elementIndex++ ];
		if ( m_currentBits == 0 )
			m_numZeros = m_bitVector->Data()[ m_elementIndex++ ] - 1;
	}

	--m_remainingBytes;
}

RleBitVector::ConstIterator::ConstIterator( const ConstIterator& it )
	: m_bitVector( it.m_bitVector )
	, m_elementIndex( it.m_elementIndex )
	, m_numZeros( it.m_numZeros )
	, m_currentBits( it.m_currentBits )
	, m_remainingBytes( it.m_remainingBytes )
{
}

bool RleBitVector::ConstIterator::operator == ( const ConstIterator& it )
{
	return ( m_remainingBytes == it.m_remainingBytes );
}

bool RleBitVector::ConstIterator::operator != ( const ConstIterator& it )
{
	return ( m_remainingBytes != it.m_remainingBytes );
}


bool RleBitVector::ConstIterator::operator < ( const ConstIterator& it )
{
	return m_remainingBytes > it.m_remainingBytes;
}

int RleBitVector::ConstIterator::operator * ()
{
	assert( m_remainingBytes != -1 );
	return m_currentBits;
}

void RleBitVector::ConstIterator::operator ++ ()
{
	assert( m_remainingBytes != -1 );

	if ( m_numZeros )
	{
		--m_numZeros;
	}
	else
	{
		assert( m_elementIndex < m_bitVector->SizeInBytes() );
		m_currentBits = m_bitVector->Data()[ m_elementIndex++ ];
		if ( m_currentBits == 0 )
		{
			assert( m_elementIndex < m_bitVector->SizeInBytes() );
			m_numZeros = m_bitVector->Data()[ m_elementIndex++ ] - 1;
		}
	}

	--m_remainingBytes;

	// The iterator can go one past the end of the actual compressed data.  For that purpose
	//   an extra byte was added to the end of the compressed data.  The value for that byte
	//   is 255.

	assert( m_remainingBytes != -1 || m_currentBits == 255 ); 
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

// Iterator over the run length encoded data, returns those bytes that have bits in
// them and the index of those bytes in the uncompressed data stream.

class RleBitVector::ConstSkipIterator
{

public:
	inline ConstSkipIterator();
	inline ConstSkipIterator( const RleBitVector * );
	inline ConstSkipIterator( const ConstSkipIterator& );
	inline ConstSkipIterator( int );

	__forceinline bool operator == ( const ConstSkipIterator& it );
	__forceinline bool operator != ( const ConstSkipIterator& it );
	__forceinline bool operator < ( const ConstSkipIterator& it );
	__forceinline void operator ++ ();

	__forceinline std::pair< int, int > operator * () const;

	__forceinline int Bits() const;
	__forceinline int Index() const;

private:

	const RleBitVector * m_bitVector;

	int m_elementIndex;
	int m_numZeros;
	int m_currentBits;

	int m_numBytes;
	int m_remainingBytes;
};

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

RleBitVector::ConstSkipIterator::ConstSkipIterator()
	: m_bitVector( 0 )
	, m_remainingBytes( -1 )
{
}

RleBitVector::ConstSkipIterator::ConstSkipIterator( const RleBitVector * bitVector )
	: m_bitVector( bitVector )
	, m_elementIndex( 0 )
	, m_numZeros( 0 )
{
	m_numBytes = m_remainingBytes = m_bitVector->UncompressedSizeInBytes();

	if ( m_remainingBytes )
	{
		assert( m_bitVector->Data()[ m_bitVector->SizeInBytes() - 1 ] == 255 );

		while ( true )
		{
			m_currentBits = m_bitVector->Data()[ m_elementIndex++ ];
			if ( m_currentBits != 0 )
				break;

			m_remainingBytes -= m_bitVector->Data()[ m_elementIndex++ ];
		}
	}

	--m_remainingBytes;

	assert( m_remainingBytes >= -1 );
}

RleBitVector::ConstSkipIterator::ConstSkipIterator( const ConstSkipIterator& it )
	: m_bitVector( it.m_bitVector )
	, m_elementIndex( it.m_elementIndex )
	, m_numZeros( it.m_numZeros )
	, m_currentBits( it.m_currentBits )
	, m_remainingBytes( it.m_remainingBytes )
	, m_numBytes( it.m_numBytes )
{
}

bool RleBitVector::ConstSkipIterator::operator == ( const ConstSkipIterator& it )
{
	return ( m_remainingBytes == it.m_remainingBytes );
}

bool RleBitVector::ConstSkipIterator::operator != ( const ConstSkipIterator& it )
{
	return ( m_remainingBytes != it.m_remainingBytes );
}

bool RleBitVector::ConstSkipIterator::operator < ( const ConstSkipIterator& it )
{
	return m_remainingBytes > it.m_remainingBytes;
}

std::pair< int, int > RleBitVector::ConstSkipIterator::operator * () const
{
	return std::pair< int, int >( m_numBytes - m_remainingBytes - 1, m_currentBits );
}

int RleBitVector::ConstSkipIterator::Bits() const
{
	return m_currentBits;
}

int RleBitVector::ConstSkipIterator::Index() const
{
	return m_numBytes - m_remainingBytes - 1;
}

void RleBitVector::ConstSkipIterator::operator ++ ()
{
	assert( m_remainingBytes != -1 );

	do
	{
		m_currentBits = m_bitVector->Data()[ m_elementIndex++ ];
		if ( m_currentBits != 0 )
		{
			--m_remainingBytes;
			break;
		}

		int numZeros = m_bitVector->Data()[ m_elementIndex++ ];
		m_remainingBytes -= numZeros;
	}
	while ( true );

	// The iterator can go one past the end of the actual compressed data.  For that purpose
	//   an extra byte was added to the end of the compressed data.  The value for that byte
	//   is 255.

	assert( (m_remainingBytes != -1) || (m_currentBits == 255) ); 
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

RleBitVector::ConstIterator RleBitVector::begin() const
{
	return ConstIterator( this );
}

RleBitVector::ConstIterator RleBitVector::end() const
{
	return ConstIterator();
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------


RleBitVector::ConstSkipIterator RleBitVector::sbegin() const
{
	return ConstSkipIterator( this );
}

RleBitVector::ConstSkipIterator RleBitVector::send() const
{
	return ConstSkipIterator();
}

}; // namespace
