#ifndef BTL_RADIX_H
#define BTL_RADIX_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Radix sort
/////////////////////////////////////////////////////

#include <btl/containers/vector.h>

namespace Btl
{

/// Class which can be used to sort a vector.
/// Uses function RadixKey(Vector::DataType), which should return an integral sorting key. @n
/// See @ref RadixSort.
/// @remarks
///	O(N) sorting algorithm which is particularly fast if the sort key is small. @n
/// Requires ~4k + size of vector as working space. This is held by the RadixSorter object
/// and can be reused. @n
/// RadixKey() is defined for standard types (in theory this could be system-dependent).
/// User defined types can also be used.
template <typename VectorType>
class RadixSorter
{
public:
	RadixSorter() {}

	void Sort(VectorType& vector); ///< @a 

private:
	template <typename T> static T* MakeT();
	enum {KEY_SIZE = sizeof(RadixKey(*MakeT<typename VectorType::DataType>()))};

	Vector<typename VectorType::DataType*, VectorParameters_Array<256> > pointers;
	int dummy; // Stops the vectors being exactly 4k apart and causing load-hit-stores.
	typedef Vector<int, Btl::VectorParameters_Array<256> > Array256;
	Btl::Vector<Array256, VectorParameters_Array<KEY_SIZE> > count;
	VectorType temp;

	template <typename T>
	int GetByte(T t, int shift) const {return (t >> shift) & 0xff;}
};

/// Sort a vector using a RadixSorter.
template <typename VectorType>
void RadixSort(VectorType& vector)
{
	RadixSorter<VectorType> sorter;
	sorter.Sort(vector);
}

FORCE_INLINE U32 RadixKey(U32 value) {return value;}
FORCE_INLINE U32 RadixKey(S32 value) {return ((U32&) value) + ((U32)(1 << 31) - 1);}

FORCE_INLINE U16 RadixKey(U16 value) {return value;}
FORCE_INLINE U16 RadixKey(S16 value) {return ((U16&) value) + ((U16)(1 << 15) - 1);}

FORCE_INLINE U8 RadixKey(U8 value) {return value;}
FORCE_INLINE U8 RadixKey(S8 value) {return ((U8&) value) + ((U8)(1 << 7) - 1);}

FORCE_INLINE U32 RadixKey(float value) 
{
	S32 s = ((S32&) value);
	if (s >> 31) s = (~s | (1 << 31)) + 1; // to get the right order for negative floats
	return RadixKey(s);
}

template <typename VectorType>
void RadixSorter<VectorType>::Sort(VectorType& vector)
{
	if (vector.Empty()) return;

	int size = vector.Size();

	memset(&count, 0, sizeof(count));

	// count number of elements for each byte of the key
	for (typename VectorType::Iterator it = vector.Begin(); it != vector.End(); ++it)
		for (int n = 0; n < KEY_SIZE; n++)
			count[n][GetByte(RadixKey(*it), n * 8)]++;

	temp.Resize(size);

	for (int n = 0; n < KEY_SIZE; n++)
	{
		if (count[n][GetByte(RadixKey(vector[0]), n * 8)] < size) // don't need to sort if all key values are the same
		{
			// pointers
			pointers[0] = &temp[0];
			typename VectorType::DataType* previous = &temp[0];

			for (int i = 1; i < 256; i++)
			{
				typename VectorType::DataType* next = previous + count[n][i - 1];
				pointers[i] = next;
				previous = next;
			}

			// sort
			for (typename VectorType::Iterator it = vector.Begin(); it != vector.End(); ++it)
				*(pointers[GetByte(RadixKey(*it), n * 8)])++ = *it;

			Swap(vector, temp);
		}
	}
}

}

#endif

