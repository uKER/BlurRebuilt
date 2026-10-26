#ifndef UT_RADIX_H
#define UT_RADIX_H

#pragma once

#include <core/containers/vector.h>

namespace Ut
{

/// Fast O(N) sorting algorithm.
/// Requires ~1k + size of vector as working space.
/// Uses function RadixKey(Vector::DataType), which produces an integral sorting key.
// RadixKey() is defined below for standard types (in theory this could be system-dependent).
template <typename Vector>
class RadixSort
{
public:
	RadixSort() {}

	void Sort(Vector& vector); ///< @a

private:
	template <typename T> static T* MakeT();
	enum {KEY_SIZE = sizeof(RadixKey(*MakeT<typename Vector::DataType>()))};

	UtVector<ArrayVector<UtVector<ArrayVector<int, 256> >, KEY_SIZE> > count;
	UtVector<ArrayVector<typename Vector::DataType*, 256> > pointers;
	Vector temp;

	template <typename T>
	int GetByte(T t, int shift) const {return (t >> shift) & 0xff;}
};

UT_FORCE_INLINE U32 RadixKey(U32 value) {return value;}
UT_FORCE_INLINE U32 RadixKey(S32 value) {return ((U32&) value) + ((U32)(1 << 31) - 1);}

UT_FORCE_INLINE U16 RadixKey(U16 value) {return value;}
UT_FORCE_INLINE U16 RadixKey(S16 value) {return ((U16&) value) + ((U16)(1 << 15) - 1);}

UT_FORCE_INLINE U8 RadixKey(U8 value) {return value;}
UT_FORCE_INLINE U8 RadixKey(S8 value) {return ((U8&) value) + ((U8)(1 << 7) - 1);}

UT_FORCE_INLINE U32 RadixKey(float value) 
{
	S32 s = ((S32&) value);
	if (s >> 31) s = (~s | (1 << 31)) + 1; // to get the right order for negative floats
	return RadixKey(s);
}

template <typename Vector>
void RadixSort<Vector>::Sort(Vector& vector)
{
	if (vector.Empty()) return;

	int size = vector.Size();

	memset(&count, 0, sizeof(count));

	// count number of elements for each byte of the key
	for (typename Vector::Iterator it = vector.Begin(); it != vector.End(); ++it)
		for (int n = 0; n < KEY_SIZE; n++)
			count[n][GetByte(RadixKey(*it), n * 8)]++;

	temp.Resize(size);

	for (int n = 0; n < KEY_SIZE; n++)
	{
		if (count[n][GetByte(RadixKey(vector[0]), n * 8)] < size) // don't need to sort if all key values are the same
		{
			// pointers
			pointers[0] = &temp[0];

			for (int i = 1; i < 256; i++)
				pointers[i] = pointers[i - 1] + count[n][i - 1];

			// sort
			for (typename Vector::Iterator it = vector.Begin(); it != vector.End(); ++it)
				*(pointers[GetByte(RadixKey(*it), n * 8)])++ = *it;

			Ut::Swap(vector, temp);
		}
	}
}

} // Ut

#endif

