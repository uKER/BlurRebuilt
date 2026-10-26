#ifndef DISPLAY_UTILS_VECTOR_H
#define DISPLAY_UTILS_VECTOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

typedef MathsStorage::Float2 C_Float2;
typedef MathsStorage::Float3 C_Float3;
typedef MathsStorage::Float4 C_Float4;

void LoadFloat4(MathsStorage::Float4* src, Maths::HVec4& dst);
void StoreFloat4(const Maths::HVec4& src, MathsStorage::Float4* dst);

void VectorSet(MathsStorage::Float4* pOut);
void VectorSet(MathsStorage::Float4* pOut, const F32 x, const F32 y, const F32 z, const F32 w);

}; // namespace Display

#include "Display/Utils/Vector.inl"

#endif // DISPLAY_UTILS_VECTOR_H
