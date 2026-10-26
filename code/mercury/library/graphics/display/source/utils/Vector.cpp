#include "stdafx.h"
#include "Display/Utils/Vector.h"

namespace Display
{

void LoadFloat4(MathsStorage::Float4* src, Maths::HVec4& dst)
{
	dst = Maths::HVec4(src->x, src->y, src->z, src->w);
}

void StoreFloat4(const Maths::HVec4& src, MathsStorage::Float4* dst)
{
	dst->x = src.X();
	dst->y = src.Y();
	dst->z = src.Z();
	dst->w = src.W();
}

}; // namespace Display
