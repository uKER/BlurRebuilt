#ifndef RADIX_SORT_H
#define RADIX_SORT_H

namespace SharedPhysics
{

inline U32 FloatFlip(U32 f)
{
	U32 mask = -int(f >> 31) | 0x80000000;
	return f ^ mask;
}

inline void FloatFlipX(U32 &f)
{
	U32 mask = -int(f >> 31) | 0x80000000;
	f ^= mask;
}

// ================================================================================================
// flip a float back (invert FloatFlip)
//  signed was flipped from above, so:
//  if sign is 1 (negative), it flips the sign bit back
//  if sign is 0 (positive), it flips all bits back
// ================================================================================================
inline void IFloatFlip(U32& f)
{
	U32 mask = ((f >> 31) - 1) | 0x80000000;
	f ^= mask;
}

// ---- utils for accessing 11-bit quantities
#define _0(x)	(x & 0x7FF)
#define _1(x)	(x >> 11 & 0x7FF)
#define _2(x)	(x >> 22 & 0x7FF)
#define _3(x)	(x >> 33 & 0x7FF)
#define _4(x)	(x >> 44 & 0x7FF)
#define _5(x)	(x >> 55 & 0x7FF)

template <class Sortable>
class C_RadixSorter
{

	//U32 b0[6144]; //2048 * 3
	U32 b[6144+96];
	Sortable* m_pTempData;
	//Sortable* m_pTempData2;
	U32 m_DataSize;
public:
	C_RadixSorter() : m_pTempData(NULL), m_DataSize(0)//, m_pTempData2(NULL)
	{
	}

	~C_RadixSorter()
	{
		if(m_pTempData)
		{
			delete [] m_pTempData;
		}
		m_pTempData = NULL;

		/*if(m_pTempData)
		{
			delete[] m_pTempData2;
		}
		m_pTempData2 = NULL;*/
	}

	//Sorts the data. The results are written back into the data param. N is the number of elements in the data
	void SortUnsignedInt(Sortable* data, int N);
	void SortFloat(Sortable* data, int N);
};

template <class Sortable>
struct Radix64
{
	Sortable m_Item;
	U64 m_Radix64;
	U32 m_Radix32;

	Radix64()
	{
	}

	Radix64(const Radix64<Sortable>& sort) : m_Item(sort.m_Item), m_Radix64(sort.m_Radix64), 
		m_Radix32(sort.m_Radix32)
	{
	}

	Radix64<Sortable>& operator = (const Radix64<Sortable>& sort)
	{
		m_Item = sort.m_Item;
		m_Radix64 = sort.m_Radix64;
		m_Radix32 = sort.m_Radix32;
		return *this;
	}
};

template <class Sortable>
struct Radix32
{
	Sortable m_Item;
	U32 m_Radix;

	
	Radix32()
	{
	}

	Radix32(const Radix32<Sortable>& sort) : m_Item(sort.m_Item), m_Radix(sort.m_Radix)
	{
	}

	Radix32<Sortable>& operator = (const Radix32<Sortable>& sort)
	{
		m_Item = sort.m_Item;
		m_Radix = sort.m_Radix;
		return *this;
	}



};


template <class Sortable>
class C_RadixSorter64
{

	//U32 b0[6144]; //2048 * 3
	U32 b[(2048 * 6) + 192]; ////2048 * 6
	

	/*U32 b[256];
	U32 index[256];*/

	Radix64<Sortable>* m_pTempData;
	Radix64<Sortable>* m_pInputData;
	//Sortable* m_pTempData2;
	U32 m_DataSize;
public:
	C_RadixSorter64() : m_pTempData(NULL), m_pInputData(NULL), m_DataSize(0)//, m_pTempData2(NULL)
	{
	}

	~C_RadixSorter64()
	{
		if(m_pTempData)
		{
			delete [] m_pTempData;
		}
		m_pTempData = NULL;
		if(m_pInputData)
		{
			delete [] m_pInputData;
		}
		m_pInputData = NULL;

		/*if(m_pTempData)
		{
			delete[] m_pTempData2;
		}
		m_pTempData2 = NULL;*/
	}

	//Sorts the data. The results are written back into the data param. N is the number of elements in the data
	void SortUnsigned64(Sortable* data, U32 N, U32 simpleSortThreshold);
	void SortUnsigned32(Sortable* data, U32 N, U32 simpleSortThreshold);
	//bool Radix(Radix64<Sortable>* input, Radix64<Sortable>* output, U32 N, U32 radix);
};



};

#include "gravity/sharedphysics/RadixSort.inl"

#endif
