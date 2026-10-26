#include "stdafx.h"
#include "Dynamic/Block.h"

#include <system/scheduler/jobqueue.h>

namespace DynamicEffect
{

void C_RenderNode::SetBounds(const Maths::C_AxisBox& axisBox, const Maths::HF32& cullDistance)
{
	Maths::HVec3 vCentre(axisBox.Centre());
	Maths::HVec3 vSize = axisBox.Max() - vCentre;

	Maths::HVec4 vCentreOne(vCentre);
	vCentreOne.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vSizeCull(vSize);
	vSizeCull.W(cullDistance);

	Maths::Store(vCentreOne, &m_vCentre);
	Maths::Store(vSizeCull, &m_vSize);
}

void C_RenderNode::SetBounds(const Maths::C_Sphere& sphere, const Maths::HF32& cullDistance)
{
	Maths::HVec3 vCentre(sphere.Centre());
	Maths::HVec3 vSize(sphere.Radius(), sphere.Radius(), sphere.Radius());

	Maths::HVec4 vCentreOne(vCentre);
	vCentreOne.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vSizeCull(vSize);
	vSizeCull.W(cullDistance);

	Maths::Store(vCentreOne, &m_vCentre);
	Maths::Store(vSizeCull, &m_vSize);
}

void C_RenderNode::SetCullDistance(const F32 cullDistance)
{
	m_vSize.w = cullDistance;
}

U8* ElementVector::Fetch()
{
	U8* pDataEA = m_pData;

#ifdef PLATFORM_SPU
	S32 size = (m_ElementSize * m_Size + 15) & ~15;
	m_pData = (U8*)Spu::FetchAlloc(m_pData, size);
#endif

	return pDataEA;
}

U8* ElementVector::FetchAsync(U32 tag)
{
	U8* pDataEA = m_pData;

#ifdef PLATFORM_SPU
	S32 size = (m_ElementSize * m_Size + 15) & ~15;
	m_pData = (U8*)Spu::FetchAllocAsync(m_pData, size, tag);
#endif

	return pDataEA;
}

void ElementVector::Store(U8* pEA)
{
#ifdef PLATFORM_SPU
	S32 size = (m_ElementSize * m_Size + 15) & ~15;
	Spu::Store(pEA, m_pData, size);
	m_pData = pEA;
#endif
}

void ElementVector::StoreAsync(U8* pEA, U32 tag)
{
#ifdef PLATFORM_SPU
	S32 size = (m_ElementSize * m_Size + 15) & ~15;
	Spu::StoreAsync(pEA, m_pData, size, tag);
	m_pData = pEA;
#endif
}

} // namespace DynamicEffect
