#include "MiscSharedPch.h"

#include <misc_shared/NewCurve/reparameterise.h>

namespace Curve
{

F32 C_Reparameterise::GetArcLength() const
{
	S32 size = m_arcLengthByUInterval.Size();
	if (size == 0)
		return 0;

	return m_arcLengthByUInterval[size-1];
}

F32 C_Reparameterise::GetArcLength(F32 u) const
{
	U32 interval = GetUInterval(u);

	//Edge-case, at end of table.
	U32 size = (U32)m_arcLengthByUInterval.Size();
	if (interval == size-1)
		return m_arcLengthByUInterval[size-1];
	
	F32 arcLength0 = m_arcLengthByUInterval[interval];
	F32 arcLength1 = m_arcLengthByUInterval[interval+1];
	F32 u0 = GetUByIndex(interval);
	F32 u1 = GetUByIndex(interval+1);

	//Parameter distance between points.
	F32 period = u1 - u0;
	if (period < Maths::Constants<F32>::Epsilon())
		return arcLength1;

	F32 lSlope = (arcLength1 - arcLength0)/period;
	return arcLength0 + lSlope*(u - u0);
}

U32 C_Reparameterise::GetUInterval(F32 u) const
{
	F32 clampedU = Maths::Clamp(u, m_uMin, m_uMax);
	//Don't round up...
	return (U32)((m_nSubDivisions-1.f)*clampedU);
}

U32 C_Reparameterise::GetLengthInterval(F32 arcLength) const
{
	F32 clampedL = Maths::Clamp(arcLength / m_arcLength, 0.f, 1.f);
	//Don't round up...
	return (U32)((m_nSubDivisions-1.f)*clampedL);
}

//The method below is what you can also see on Page 12 of David Eberly's paper "Moving along a Curve with Specified Speed".
//It's the rubbish approximate version and needs improving.
F32 C_Reparameterise::GetU(F32 arcLength) const
{
	S32 size = m_arcLengthByUInterval.Size();
	if (size == 0)
		return m_uMin;

	//Edge-case, at end of table.
	if (arcLength >= m_arcLengthByUInterval[size-1])
		return m_uMax;
	
	U32 index = GetLengthInterval(arcLength);
	U32 next = Maths::Min(index + 1, (U32) (m_uByLengthInterval.Size() - 1));
	S32 interval = FindUInterval(arcLength, m_uByLengthInterval[index], m_uByLengthInterval[next] + 1);
	//S32 interval = FindUInterval(arcLength, 0, m_arcLengthByUInterval.Size())

	assert(FindUInterval(arcLength, 0, m_arcLengthByUInterval.Size()) == interval);

	F32 arcLength0 = m_arcLengthByUInterval[interval];
	F32 arcLength1 = m_arcLengthByUInterval[interval+1];
	F32 u0 = m_uScale * (interval);
	F32 u1 = u0 + m_uScale;

	F32 period = arcLength1 - arcLength0;
	F32 uSlope = (u1 - u0) / period;

	using namespace Maths;

	return (float)__fsel(period - Maths::Constants<F32>::Epsilon(), u0 + uSlope*(arcLength - arcLength0), u1);
}

S32 C_Reparameterise::FindUInterval(F32 arcLength, S32 low, S32 high) const
{
	while(high > low + 1)
	{
		S32 mid = (low + high) >> 1;

		if((arcLength < m_arcLengthByUInterval[mid]))
			high = mid;
		else
			low = mid;
	}
	
	return low;
}

}