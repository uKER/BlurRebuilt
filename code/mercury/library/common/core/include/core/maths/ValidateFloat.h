#ifndef MATHS_VALIDATE_FLOAT_H
#define MATHS_VALIDATE_FLOAT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace MathsUtil
{

FORCE_INLINE bool IsValidFloat(F32 f)
{
	int i = *(int*)&f;
	return ((i&0xff000000) != 0xff000000);
}

#ifdef __MVECTOR_H__
template <class T, int N>
FORCE_INLINE bool IsValidFloat(const UtMaths::BaseVector<T, N>& value)
{
	for (U32 i = 0; i < value.SIZE; i++)
		if (!IsValidFloat(value[i]))
			return false;
	return true;
}
#endif

#ifdef __CORE_MMATRIX_H__
template <class T, int N>
INLINE bool IsValidFloat(const UtMaths::MMatrix<UtMaths::BasicMatrix<T, N> >& value)
{
	for (U32 i = 0; i < value.SIZE; i++)
		if (!IsValidFloat(value[i]))
			return false;
	return true;
}
#endif

//TODO sanity-checks for other maths lib.

}

#endif