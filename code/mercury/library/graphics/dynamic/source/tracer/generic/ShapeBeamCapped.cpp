#include "stdafx.h"
#include "Dynamic/Tracer/Generic/ShapeBeamCapped.h"
#include "Dynamic/Node.h"



namespace DynamicEffect
{
namespace Tracer
{

void ShapeBeamCapped::Spawn(const C_Node *pNode, const Maths::C_RGBA_F32 &vColour)
{
	if (pNode)
	{
		Maths::HVec3 vPosition = Maths::C_QuatTrans(*pNode).Translation();

		Tracer* pTracer = NewTracer();
		Maths::Store(vPosition, &pTracer->m_vPrevious);
		Maths::Store(vColour, &pTracer->m_vColour);
		pTracer->m_pNode = pNode;
		pTracer->m_pEffect = this;
		pTracer->m_iFlags = 0;
		pTracer->m_iVertexBuffer = 0xFFFF;
	}
}

void ShapeBeamCapped::Update()
{
	for (S32 tracerNo = 0; tracerNo < m_Tracers.Size(); ++tracerNo)
	{
		Tracer& tracer = m_Tracers[tracerNo];
		if (tracer.m_pNode)
		{
			Maths::HVec3 vPosition = Maths::C_QuatTrans(*tracer.m_pNode).Translation();
			Maths::Store(vPosition, &tracer.m_vPrevious);
		}
	}
}

}; // namespace Tracer
}; // namespace DynamicEffect
