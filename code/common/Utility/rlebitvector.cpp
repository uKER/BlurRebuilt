#include "stdafx.h"
#include "rlebitvector.h"

namespace Ut
{

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

RleBitVector::RleBitVector()
	: m_numUncompressedElements( 0 )
{
}

void RleBitVector::Init( BitVector const & bitVector )
{
	if ( bitVector.Size() == 0 )
		return;

	int numZeroBytes = 0;

	U8 * data = bitVector.Data(); 

	int numBits = bitVector.Size();
	int numBytes = numBits / 8;

	for (int byte = 0; byte < numBytes; ++byte)
	{
		// If all bits in the byte are 0 then they are run length encoded

		int bits = data[byte];
		if ( bits == 0 )
		{
			++numZeroBytes;
		}
		else
		{
			// Finish up any previous run length encoding

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

			// Store the current byte.

			m_elements.PushBack( static_cast<U8>(bits) );
		}
	}

	// Deal with any bits that are left over at the end

	int numRemainingBits = numBits % 8;
 	if ( numRemainingBits )
	{
		int mask = 0xff >> (8 - numRemainingBits);

		int bits = data[numBytes] & mask;
		if ( bits == 0 )
		{
			++numZeroBytes;
		}
		else
		{
			// Finish up any previous run length encoding

			while ( numZeroBytes > 0 )
			{
				m_elements.PushBack( 0 );

				if ( numZeroBytes > 255 )
				{
					m_elements.PushBack( 255 );
					numZeroBytes -= 255;
				}
				else
				{
					m_elements.PushBack( numZeroBytes );
					numZeroBytes = 0;
				}
			}

			// Store the current byte.

			m_elements.PushBack( bits );
		}
	}

	// Finaly if the last few bits were all zero then we need to keep track of that as well.

	while ( numZeroBytes > 0 )
	{
		m_elements.PushBack( 0 );

		if ( numZeroBytes > 255 )
		{
			m_elements.PushBack( 255 );
			numZeroBytes -= 255;
		}
		else
		{
			m_elements.PushBack( numZeroBytes );
			numZeroBytes = 0;
		}
	}

	// Put a final terminating byte into the vector to simplify the iteration logic.
	m_elements.PushBack( 255 );

	m_numUncompressedElements = (numBits + 7) / 8;
}

void RleBitVector::Init( int numElements, int numUncompressedElements )
{
	m_elements.Resize( numElements );

	if ( numElements )
		ZeroMemory( &m_elements[0], sizeof(U8) * numElements );

	m_numUncompressedElements = numUncompressedElements;
}

int RleBitVector::CountSetBits() const
{
	int count = 0;

	ConstSkipIterator it = sbegin();
	while ( it != send() )
	{
		int bits = (*it).second;
		while ( bits )
		{
			count += bits & 1;
			bits >>= 1;
		}

		++it;
	}

	return count;
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

}; // namespace

#if UNIT_TESTS

using namespace Ut;

void RleBitVectorTest()
{
	RleBitVector empty;

	for ( RleBitVector::ConstIterator it = empty.begin(); it != empty.end(); ++it )
	{
		assert( 0 );
	}

	int bits = 0;
	int index = 0;

	BitVector initializer;

	initializer.PushBack( 0 );
	initializer.PushBack( 1 );
	initializer.PushBack( 0 );
	initializer.PushBack( 1 );

	RleBitVector initialized;
	initialized.Init( initializer );

	RleBitVector::ConstIterator it0 = initialized.begin();

	bits = *it0;
	++it0;

	assert( it0 == initialized.end() );
	assert( bits == 0x0a );

	initializer.PushBack( 1 );
	initializer.PushBack( 1 );
	initializer.PushBack( 1 );
	initializer.PushBack( 1 );

	RleBitVector initialized2;
	initialized2.Init( initializer );

	RleBitVector::ConstIterator it1 = initialized2.begin();

	bits = *it1;
	++it1;

	assert( it1 == initialized2.end() );
	assert( bits == 0xfa );

	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 1 );

	RleBitVector initialized3;
	initialized3.Init( initializer );

	RleBitVector::ConstIterator it2 = initialized3.begin();

	bits = *it2;
	++it2;

	assert( bits == 0xfa );

	bits = *it2;
	++it2;

	assert( bits == 0x08 );
	assert( it2 == initialized3.end() );

	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );

	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );

	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );
	initializer.PushBack( 0 );

	RleBitVector initialized4;
	initialized4.Init( initializer );

	RleBitVector::ConstIterator it3 = initialized4.begin();

	bits = *it3;
	++it3;

	assert( bits == 0xfa );

	bits = *it3;
	++it3;

	assert( bits == 0x08 );

	bits = *it3;
	++it3;

	assert( bits == 0x00 );

	bits = *it3;
	++it3;

	assert( bits == 0x00 );

	assert( it3 == initialized4.end() );

	RleBitVector::ConstSkipIterator it4 = initialized4.sbegin();

	bits = it4.Bits();
	index = it4.Index();
	++it4;

	assert( bits == 0xfa );
	assert( index == 0 );

	bits = it4.Bits();
	index = it4.Index();
	++it4;

	assert( bits == 0x08 );
	assert( index == 1 );

	assert( it4 == initialized4.send() );

	initializer.PushBack( 1 );

	RleBitVector initialized5;
	initialized5.Init( initializer );

	RleBitVector::ConstSkipIterator it5 = initialized5.sbegin();

	bits = it5.Bits();
	index = it5.Index();
	++it5;

	assert( bits == 0xfa );
	assert( index == 0 );

	bits = it5.Bits();
	index = it5.Index();
	++it5;

	assert( bits == 0x08 );
	assert( index == 1 );

	bits = it5.Bits();
	index = it5.Index();
	++it5;

	assert( bits == 0x01 );
	assert( index == 4 );

	assert( it5 == initialized5.send() );

	BitVector initializer2;

	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );

	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );

	initializer2.PushBack( 0 );

	RleBitVector initialized6;
	initialized6.Init( initializer2 );

	RleBitVector::ConstSkipIterator it6 = initialized6.sbegin();

	assert( it6 == initialized6.send() );

	initializer2.PushBack( 1 );

	RleBitVector initialized7;
	initialized7.Init( initializer2 );

	RleBitVector::ConstSkipIterator it7 = initialized7.sbegin();
	
	bits = it7.Bits();
	index = it7.Index();
	++it7;

	assert( bits == 0x02 );
	assert( index == 2 );

	assert( it7 == initialized7.send() );

	initializer2.PushBack( 1 );
	initializer2.PushBack( 1 );
	initializer2.PushBack( 1 );
	initializer2.PushBack( 1 );
	initializer2.PushBack( 1 );
	initializer2.PushBack( 1 );

	for ( int i = 0; i < 513; ++i )
	{
		initializer2.PushBack( 0 );
		initializer2.PushBack( 0 );
		initializer2.PushBack( 0 );
		initializer2.PushBack( 0 );
		initializer2.PushBack( 0 );
		initializer2.PushBack( 0 );
		initializer2.PushBack( 0 );
		initializer2.PushBack( 0 );
	}

	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );
	initializer2.PushBack( 0 );

	RleBitVector initialized8;
	initialized8.Init( initializer2 );

	RleBitVector::ConstSkipIterator it8 = initialized8.sbegin();
	
	bits = it8.Bits();
	index = it8.Index();
	++it8;

	assert( bits == 0xfe );
	assert( index == 2 );

	assert( it8 == initialized8.send() );

	assert( initialized8.CountSetBits() == 0x06 ); 
}

#endif
