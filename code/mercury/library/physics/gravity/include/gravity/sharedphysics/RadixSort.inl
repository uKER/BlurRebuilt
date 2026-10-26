//#include "gravity/sharedphysics/RadixSort.h"

#ifndef RADIXSORT_INL
#define RADIXSORT_INL

namespace SharedPhysics
{
template <class Sortable>
void C_RadixSorter<Sortable>::SortFloat(Sortable* data, int N)
{
	if(m_DataSize < N)
	{
		if(m_pTempData)
		{
			delete [] m_pTempData;
		}
		m_pTempData = new Sortable[N];
		m_DataSize = N;
	}

	//Begin the sort routine...

	S32 i;

	const U32 kHist = 2048;
	U32 *b0 = b;
	U32 *b1 = b + kHist + 32;
	U32 *b2 = b + kHist*2 + 64;

	for (i = kHist * 3 - 1; i >=0; --i) 
	{
		b0[i] = 0;
	}

	U32 fi = FloatFlip(data[0].GetRadix());
	b0[_0(fi)] ++;
	b1[_1(fi)] ++;
	b2[_2(fi)] ++;
	bool sorted = true;
	for (i = 1; i < N; ++i) 
	{
		sorted = sorted && ((data[i-1]) <= (data[i]));
		fi = FloatFlip(data[i].GetRadix32());
		b0[_0(fi)] ++;
		b1[_1(fi)] ++;
		b2[_2(fi)] ++;
	}
	if(sorted)
		return;
	
	// 2.  Sum the histograms -- each histogram entry records the number of values preceding itself.
	{
		U32 sum0 = 0, sum1 = 0, sum2 = 0;
		U32 tsum;
		for (i = 0; i < (S32)kHist; ++i) 
		{
			tsum = b0[i] + sum0;
			b0[i] = sum0 - 1;
			sum0 = tsum;

			tsum = b1[i] + sum1;
			b1[i] = sum1 - 1;
			sum1 = tsum;

			tsum = b2[i] + sum2;
			b2[i] = sum2 - 1;
			sum2 = tsum;
		}
	}

	//Could insert some pre-fetches here but will leave this for now

	U32 pos;
	// byte 0: floatflip entire value, read/write histogram, write out flipped
	for (i = 0; i < N; ++i) 
	{
		FloatFlipX(data[i].GetRadix32());
		pos = _0(data[i].GetRadix32());
		m_pTempData[++b0[pos]] = data[i];
	}

	for (i = 0; i < N; ++i) 
	{
		pos = _1(m_pTempData[i].GetRadix32());
		data[++b1[pos]] = m_pTempData[i];
	}

	for (i = 0; i < N; ++i) 
	{
		pos = _2(data[i].GetRadix32());
		IFloatFlip(data[i].GetRadix32());
		m_pTempData[++b2[pos]] = data[i];
	}

	//Finally, stick the answer back into the data vector, so we have the result in its original position
	for(i = 0; i < N; ++i)
	{
		data[i] = m_pTempData[i];
	}
}

template <class Sortable>
void C_RadixSorter<Sortable>::SortUnsignedInt(Sortable* data, int N)
{
	if(m_DataSize < N)
	{
		if(m_pTempData)
		{
			delete [] m_pTempData;
		}
		m_pTempData = new Sortable[N];
		m_DataSize = N;

	/*	if(m_pTempData2)
		{
			delete[] m_pTempData2;
		}
		m_pTempData2 = new Sortable[N];*/
	}

	//Begin the sort routine...

	S32 i;

	const U32 kHist = 2048;
	U32 * __restrict b0 = b;
	U32 * __restrict b1 = b + kHist + 32;
	U32 * __restrict b2 = b + kHist*2 + 64;

	for (i = kHist * 3 - 1; i >=0; --i) 
	{
		b0[i] = 0;
	}


	U32 fi = data[0].GetRadix32();
	b0[_0(fi)] ++;
	b1[_1(fi)] ++;
	b2[_2(fi)] ++;
	bool sorted = true;
	for (i = 1; i < N; ++i) 
	{
		sorted = sorted && ((data[i-1]) <= (data[i]));
		fi = data[i].GetRadix32();
		b0[_0(fi)] ++;
		b1[_1(fi)] ++;
		b2[_2(fi)] ++;
	}
	if(sorted)
		return;
	
	// 2.  Sum the histograms -- each histogram entry records the number of values preceding itself.
	{
		U32 sum0 = 0, sum1 = 0, sum2 = 0;
		U32 tsum0, tsum1, tsum2;
		for (i = 0; i < (S32)kHist; ++i) 
		{
			
			tsum0 = b0[i] + sum0;
			tsum1 = b1[i] + sum1;
			tsum2 = b2[i] + sum2;

			ReadWriteBarrier();

			b0[i] = sum0 - 1;
			sum0 = tsum0;

			
			b1[i] = sum1 - 1;
			sum1 = tsum1;

			
			b2[i] = sum2 - 1;
			sum2 = tsum2;
		}
	}

	//Could insert some pre-fetches here but will leave this for now

	U32 pos;
	// byte 0: floatflip entire value, read/write histogram, write out flipped
	for (i = 0; i < N; ++i) 
	{
		pos = _0(data[i].GetRadix32());
		m_pTempData[++b0[pos]] = data[i];
	}


	for (i = 0; i < N; ++i) 
	{
		pos = _1(m_pTempData[i].GetRadix32());
		data[++b1[pos]] = m_pTempData[i];
	}

	for (i = 0; i < N; ++i) 
	{
		pos = _2(data[i].GetRadix32());
		m_pTempData[++b2[pos]] = data[i];
	}

	//Finally, stick the answer back into the data vector, so we have the result in its original position
	for(i = 0; i < N; ++i)
	{
		data[i] = m_pTempData[i];
	}

}

//template <class Sortable>
//void C_RadixSorter64<Sortable>::SortUnsigned64(Sortable* data, U32 N, const U32 simpleSortThreshold)
//{
//	if(N < simpleSortThreshold)
//	{
//		bool bChanged = true;
//		while(bChanged)
//		{
//			bChanged = false;
//			//const S32 count = ((S32)N)-1;
//			for(U32 a = 1; a < N; ++a)
//			{
//				if(data[a-1] > data[a])
//				{
//					Sortable temp = data[a];
//					data[a] = data[a-1];
//					data[a-1] = temp;
//					bChanged = true;
//				}
//			}
//		}
//		return;
//	}
//	if(m_DataSize < N)
//	{
//		if(m_pTempData)
//		{
//			delete [] m_pTempData;
//		}
//		if(m_pInputData)
//		{
//			delete [] m_pInputData;
//		}
//		m_pTempData = new Radix64<Sortable>[N];
//		m_pInputData = new Radix64<Sortable>[N];
//		m_DataSize = N;
//	}
//
//	for(U32 a = 0; a < N; ++a)
//	{
//		m_pInputData[a].m_Item = data[a];
//		m_pInputData[a].m_Radix = data[a].GetRadix();
//	}
//
//	Radix64<Sortable>* answer = m_pInputData;
//	
//	if(!Radix(m_pInputData, m_pTempData, N, 0))
//	{
//		goto Result;
//	}
//	
//	if(!Radix(m_pTempData, m_pInputData, N, 1))
//	{
//		answer = m_pTempData;
//		goto Result;
//	}
//
//	if(!Radix(m_pInputData, m_pTempData, N, 2))
//	{
//		goto Result;
//	}
//
//	if(!Radix(m_pTempData, m_pInputData, N, 3))
//	{
//		answer = m_pTempData;
//		goto Result;
//	}
//
//	if(!Radix(m_pInputData, m_pTempData, N, 4))
//	{
//		goto Result;
//	}
//	if(!Radix(m_pTempData, m_pInputData, N, 5))
//	{
//		answer = m_pTempData;
//		goto Result;
//	}
//
//	if(!Radix(m_pInputData, m_pTempData, N, 6))
//	{
//		goto Result;
//	}
//	if(!Radix(m_pTempData, m_pInputData, N, 7))
//	{
//		answer = m_pTempData;
//	}
//
//Result:
//	for(U32 a = 0; a < N; ++a)
//	{
//		data[a] = answer[a].m_Item;
//	}
//}

template <class Sortable>
bool ValidList(Radix64<Sortable>* list, U32 N)
{
	for(U32 a = 0; a < N; ++a)
	{
		if(list[a].m_Item.m_pCollider == NULL)
		{
			BIZ_ASSERT(0);
			return false;
		}
	}
	return true;
}


template <class Sortable>
void C_RadixSorter64<Sortable>::SortUnsigned64(Sortable* data, U32 N, const U32 simpleSortThreshold)
{
	if(N == 0)
		return;
	if(m_DataSize < N)
	{
		if(m_pTempData)
		{
			delete [] m_pTempData;
		}
		if(m_pInputData)
		{
			delete [] m_pInputData;
		}
		m_pTempData = new Radix64<Sortable>[N];
		m_pInputData = new Radix64<Sortable>[N];
		m_DataSize = N;
	}

	bool sorted = true;
	m_pInputData[0].m_Item = data[0];
	m_pInputData[0].m_Radix64 = data[0].GetRadix64();
	for(U32 a = 1; a < N; ++a)
	{
		sorted = sorted && ((data[a-1]) <= (data[a]));
		m_pInputData[a].m_Item = data[a];
		m_pInputData[a].m_Radix64 = data[a].GetRadix64();
	}

	if(sorted)
		return;

	U32 i;

	const U32 kHist = 2048 + 32;
	U32 * __restrict b0 = b;
	U32 * __restrict b1 = b + kHist;
	U32 * __restrict b2 = b + kHist*2;
	U32 * __restrict b3 = b + kHist*3;
	U32 * __restrict b4 = b + kHist*4;
	U32 * __restrict b5 = b + kHist*5;

	const U32 histLen = kHist * 6;
	for (S32 a = 0; a < histLen; a+=6) 
	{
		DCBT(128, &b0[a]);
		//const S32 end(a + cacheSize);
		b0[a] = 0;
		b0[a+1] = 0;
		b0[a+2] = 0;
		b0[a+3] = 0;
		b0[a+4] = 0;
		b0[a+5] = 0;
	}


	U64 fi = m_pInputData[0].m_Radix64;
	b0[_0(fi)] ++;
	b1[_1(fi)] ++;
	b2[_2(fi)] ++;
	b3[_3(fi)] ++;
	b4[_4(fi)] ++;
	b5[_5(fi)] ++;
	
	for (i = 1; i < N; ++i) 
	{
		fi = m_pInputData[i].m_Radix64;
		b0[_0(fi)] ++;
		b1[_1(fi)] ++;
		b2[_2(fi)] ++;
		b3[_3(fi)] ++;
		b4[_4(fi)] ++;
		b5[_5(fi)] ++;
	}
	
	// 2.  Sum the histograms -- each histogram entry records the number of values preceding itself.
	{
		U32 sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;
		//U32 tsum;
		for (i = 0; i < kHist; ++i) 
		{
			
			const U32 tsum = b0[i] + sum0;
			const U32 tsum1 = b1[i] + sum1;
			const U32 tsum2 = b2[i] + sum2;
			const U32 tsum3 = b3[i] + sum3;
			const U32 tsum4 = b4[i] + sum4;
			const U32 tsum5 = b5[i] + sum5;


			ReadWriteBarrier();

			b0[i] = sum0 - 1;
			sum0 = tsum;

			
			b1[i] = sum1 - 1;
			sum1 = tsum1;

			
			b2[i] = sum2 - 1;
			sum2 = tsum2;

			
			b3[i] = sum3 - 1;
			sum3 = tsum3;

			
			b4[i] = sum4 - 1;
			sum4 = tsum4;

			
			b5[i] = sum5 - 1;
			sum5 = tsum5;
		}
	}

	//Could insert some pre-fetches here but will leave this for now

	U64 pos;
	// byte 0: floatflip entire value, read/write histogram, write out flipped
	for (i = 0; i < N; ++i) 
	{
		pos = _0(m_pInputData[i].m_Radix64);
		m_pTempData[++b0[pos]] = m_pInputData[i];
	}
	//assert(ValidList(m_pTempData, N));

	for (i = 0; i < N; ++i) 
	{
		pos = _1(m_pTempData[i].m_Radix64);
		m_pInputData[++b1[pos]] = m_pTempData[i];
	}

//	assert(ValidList(m_pInputData, N));

	for (i = 0; i < N; ++i) 
	{
		pos = _2(m_pInputData[i].m_Radix64);
		m_pTempData[++b2[pos]] = m_pInputData[i];
	}

	//assert(ValidList(m_pTempData, N));

	for (i = 0; i < N; ++i) 
	{
		pos = _3(m_pTempData[i].m_Radix64);
		m_pInputData[++b3[pos]] = m_pTempData[i];
	}

//	assert(ValidList(m_pInputData, N));

	for (i = 0; i < N; ++i) 
	{
		pos = _4(m_pInputData[i].m_Radix64);
		m_pTempData[++b4[pos]] = m_pInputData[i];
	}

	//assert(ValidList(m_pTempData, N));

	for (i = 0; i < N; ++i) 
	{
		pos = _5(m_pTempData[i].m_Radix64);
		m_pInputData[++b5[pos]] = m_pTempData[i];
	}

//	assert(ValidList(m_pInputData, N));

	//Finally, stick the answer back into the data vector, so we have the result in its original position
	for(i = 0; i < N; ++i)
	{
		data[i] = m_pInputData[i].m_Item;
	}
}

template <class Sortable>
void C_RadixSorter64<Sortable>::SortUnsigned32(Sortable* data, U32 N, const U32 simpleSortThreshold)
{
	if(N == 0)
		return;
	if(m_DataSize < N)
	{
		if(m_pTempData)
		{
			delete [] m_pTempData;
		}
		if(m_pInputData)
		{
			delete [] m_pInputData;
		}
		m_pTempData = new Radix64<Sortable>[N];
		m_pInputData = new Radix64<Sortable>[N];
		m_DataSize = N;
	}

	bool sorted = true;
	m_pInputData[0].m_Item = data[0];
	m_pInputData[0].m_Radix32 = data[0].GetRadix32();
	for(U32 a = 1; a < N; ++a)
	{
		sorted = sorted && ((data[a-1]) <= (data[a]));
		m_pInputData[a].m_Item = data[a];
		m_pInputData[a].m_Radix32 = data[a].GetRadix32();
	}

	if(sorted)
		return;

	U32 i;

	const U32 kHist = 2048 + 32;
	U32 * __restrict b0 = b;
	U32 * __restrict b1 = b + kHist;
	U32 * __restrict b2 = b + kHist*2;

	const U32 histLen = kHist * 3;
	for (S32 a = 0; a < histLen; a+=6) 
	{
		//DCBT(128, &b0[a]);
		//const S32 end(a + cacheSize);
		b0[a] = 0;
		b0[a+1] = 0;
		b0[a+2] = 0;
		b0[a+3] = 0;
		b0[a+4] = 0;
		b0[a+5] = 0;
	}


	U64 fi = m_pInputData[0].m_Radix32;
	b0[_0(fi)] ++;
	b1[_1(fi)] ++;
	b2[_2(fi)] ++;
	
	for (i = 1; i < N; ++i) 
	{
		fi = m_pInputData[i].m_Radix32;
		b0[_0(fi)] ++;
		b1[_1(fi)] ++;
		b2[_2(fi)] ++;
	}
	
	// 2.  Sum the histograms -- each histogram entry records the number of values preceding itself.
	{
		U32 sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;
		//U32 tsum;
		for (i = 0; i < kHist; ++i) 
		{
			
			const U32 tsum = b0[i] + sum0;
			const U32 tsum1 = b1[i] + sum1;
			const U32 tsum2 = b2[i] + sum2;


			ReadWriteBarrier();

			b0[i] = sum0 - 1;
			sum0 = tsum;

			
			b1[i] = sum1 - 1;
			sum1 = tsum1;

			
			b2[i] = sum2 - 1;
			sum2 = tsum2;
		}
	}

	//Could insert some pre-fetches here but will leave this for now

	U32 pos;
	// byte 0: floatflip entire value, read/write histogram, write out flipped
	for (i = 0; i < N; ++i) 
	{
		pos = _0(m_pInputData[i].m_Radix32);
		m_pTempData[++b0[pos]] = m_pInputData[i];
	}

	for (i = 0; i < N; ++i) 
	{
		pos = _1(m_pTempData[i].m_Radix32);
		m_pInputData[++b1[pos]] = m_pTempData[i];
	}

	for (i = 0; i < N; ++i) 
	{
		pos = _2(m_pInputData[i].m_Radix32);
		m_pTempData[++b2[pos]] = m_pInputData[i];
	}

	//Finally, stick the answer back into the data vector, so we have the result in its original position
	for(i = 0; i < N; ++i)
	{
		data[i] = m_pTempData[i].m_Item;
	}
}


inline U32 RadixFunc(U64 value, U32 radix)
{
	return (U32)((value >> radix) & 0xff);
}

//template <class Sortable>
//bool C_RadixSorter64<Sortable>::Radix(Radix64<Sortable>* input, Radix64<Sortable>* output, U32 N, U32 radix)
//{
//	const U32 shift = radix * 8;
//	//Implement normal, 8-bit radix sort...
//	for(U32 a = 0; a < 256; ++a)
//	{
//		b[a] = 0;
//	}
//
//	bool bAlreadySorted = true;
//	U32 ind = RadixFunc(input[0].m_Radix, shift);
//	b[ind]++;
//	for(U32 a = 1; a < N; ++a)
//	{
//		bAlreadySorted = bAlreadySorted && (input[a-1].m_Item <= input[a].m_Item);
//		ind = RadixFunc(input[a].m_Radix, shift);
//		b[ind]++;
//	}
//	if(bAlreadySorted)
//		return false;
//
//	//U32 sum = 0;
//	index[0] = 0;
//	for(U32 a = 1; a < 256; ++a)
//	{
//		index[a] = index[a-1] + b[a-1];
//	}
//
//	for(U32 a = 0; a < N; ++a)
//	{
//		ind = index[RadixFunc(input[a].m_Radix, shift)]++;
//		output[ind] = input[a];
//	}
//
//	return true;
//}


};

#endif

