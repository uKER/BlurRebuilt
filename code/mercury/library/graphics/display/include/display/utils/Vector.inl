#ifndef DISPLAY_UTILS_VECTOR_INL
#define DISPLAY_UTILS_VECTOR_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE void VectorSet(MathsStorage::Float4* pOut)
{
	assert(pOut);
	pOut->x = pOut->y = pOut->z = pOut->w = 0.f;
}

INLINE void VectorSet(MathsStorage::Float4* pOut, const F32 x, const F32 y, const F32 z, const F32 w)
{
	assert(pOut);
	pOut->x = x;
	pOut->y = y;
	pOut->z = z;
	pOut->w = w;
}

}; // namespace Display

#endif // DISPLAY_UTILS_VECTOR_INL
