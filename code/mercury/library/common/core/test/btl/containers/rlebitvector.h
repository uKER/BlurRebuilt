#include "test/btl/test.h"
#include <btl/containers/rlebitvector.h>
#include <cxxtest/TestSuite.h>

using namespace Btl;

class Btl_RleBitVector_Tests : public CxxTest::TestSuite
{
public:
	void testRleBitVector()
	{
		RleBitVector empty;

		for ( RleBitVector::ConstIterator it = empty.begin(); it != empty.end(); ++it )
		{
			assert( 0 );
		}

		U8 bytes[1025];
		memset( bytes, 0, sizeof(bytes) );

		int numBits = 1024 * 8 + 1;
		bytes[1024] = 0xff;

		RleBitVector rle;
		rle.Init( bytes, numBits );

		int numSetBits = rle.CountSetBits();
		assert( numSetBits == 1 );

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

		assert( initialized8.CountSetBits() == 0x07 ); 

		RleBitVector::ConstSkipIterator it8 = initialized8.sbegin();
		
		bits = it8.Bits();
		index = it8.Index();
		++it8;

		assert( bits == 0xfe );
		assert( index == 2 );

		assert( it8 == initialized8.send() );
	}

	void testCompression()
	{
		BitVector initializer3;

		RleBitVector initialized9;
		initialized9.Init( initializer3 );

		assert( initialized9.CountSetBits() == 0x00 );

		RleBitVector::ConstSkipIterator it9 = initialized9.sbegin();
		assert( it9 == initialized9.send() );

		for ( int i = 0; i < 1024 * 1; ++i )
		{
			initializer3.PushBack( 0 );

			RleBitVector initialized;
			initialized.Init( initializer3 );

			int numBits = initialized.CountSetBits();
			assert( numBits == 0 );
		}

		for ( int i = 0; i < 1024 * 1; ++i )
		{
			initializer3.PushBack( 1 );

			RleBitVector initialized;
			initialized.Init( initializer3 );
			
			int numBits = initialized.CountSetBits();
			assert( numBits == i+1 );
		}

		for ( int i = 0; i < 1024 * 1; ++i )
		{
			initializer3.PushBack( 0 );

			RleBitVector initialized;
			initialized.Init( initializer3 );

			int numBits = initialized.CountSetBits();
			assert( numBits == 1024 * 1 );
		}
	}

	void testRandom()
	{
		for ( int j = 0; j < 1; ++j )
		{
			BitVector initializer4;

			int bitsAdded = 0;

			for ( int i = 0; i < 1024 * 1; ++i )
			{
				if ( rand() > (RAND_MAX>>1) )
				{
					initializer4.PushBack( 1 );
					++bitsAdded;
				}
				else
				{
					initializer4.PushBack( 0 );
				}

				RleBitVector initialized;
				initialized.Init( initializer4 );

				int numBits = initialized.CountSetBits();
				assert( numBits == bitsAdded );
			}
		}
	}
};

