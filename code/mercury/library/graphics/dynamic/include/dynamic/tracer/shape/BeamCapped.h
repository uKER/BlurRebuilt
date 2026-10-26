#ifndef RENDER_DYNAMICEFFECT_TRACER_BEAMCAPPED3D_H
#define RENDER_DYNAMICEFFECT_TRACER_BEAMCAPPED3D_H

#if _MSC_VER > 1000
#pragma once
#endif



#include "Display/Buffer/VertexDeclarations.h"
#include "Render/Billboard/BeamCapped.h"

#include "Dynamic/Tracer/Shape/ShapePolicyInterface.h"



namespace DynamicEffect
{
namespace Tracer
{



template<typename TracerType>
class BeamCappedPolicy : public I_ShapePolicy
{
public:

	typedef Display::Vertex_XYZ_HDR_SUV			ShapeVertex;
	typedef volatile ShapeVertex* __restrict	ShapeVertexPtr;
	
	void*	Build(const RenderNodePtrs& blocks, volatile void* __restrict pBuffer) const;

	void	Begin() const {};
	void	End() const {};

	U32 GetVertexSize() const
	{
		return sizeof(ShapeVertex);
	}

	Display::C_VertexDeclaration* GetVertexDeclaration() const
	{
		return ShapeVertex::m_pDeclaration;
	}

	U32	GetNumPrimitives() const
	{
		return m_nNumPrimitives;
	}
	
	U32 GetNumVertices() const
	{
		return m_nNumVertices;
	}
	
	Display::E_PrimitiveType GetPrimitiveType() const
	{
		return m_ePrimitiveType;
	}

private:

	static const U32 m_nNumPrimitives = 4;
	static const U32 m_nNumVertices = 3;
	static const Display::E_PrimitiveType m_ePrimitiveType = Display::E_PrimitiveType_TriangleList;

};



template<typename TracerType>
INLINE void* BeamCappedPolicy<TracerType>::Build(const RenderNodePtrs& blocks, volatile void* __restrict pBuffer) const
{
	const Maths::HF32 vZero = Maths::Constants<Maths::HF32>::Zero();
	const Maths::HF32 vOne = Maths::Constants<Maths::HF32>::One();
	const Maths::HVec2 vSize = Maths::Load<Maths::HVec2>(&m_pRenderSettings->vSize);
	const Maths::HF32 vFade = Maths::Load<Maths::HF32>(&m_pRenderSettings->fFade);

	ShapeVertexPtr pVertices = (ShapeVertexPtr)pBuffer;

	Render::Billboard::BeamCapped beam;
	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		TracerType* pTracer = (TracerType *) blocks[i];
		
		Maths::HVec3 vPosition1 = Maths::Load<Maths::HVec3>(&pTracer->m_vPrevious);
		Maths::HVec3 vPosition0 = vPosition1;
		if (pTracer->m_pNode)
		{
			vPosition0 = Maths::C_QuatTrans(*pTracer->m_pNode).Translation();
		}
		Maths::HVec4 vColourFront = Maths::Load<Maths::HVec4>(&pTracer->m_vColour);
		Maths::HVec4 vColourBack = vColourFront * vFade;;
		vPosition1 = vPosition0 + (vPosition1 - vPosition0) * vSize.Y();

		beam.Build(vPosition1, vPosition0, vSize.X());

		// triangle 0
		Maths::Store(beam.m_vPosition0, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourBack, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 0;
		pVertices->v = 1;
		++pVertices;
		Maths::Store(beam.m_vPositionBack, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourBack, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 0;
		pVertices->v = 0;
		++pVertices;
		Maths::Store(beam.m_vPosition1, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourBack, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 1;
		pVertices->v = 0;
		++pVertices;
		// triangle 1
		Maths::Store(beam.m_vPosition0, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourBack, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 0;
		pVertices->v = 1;
		++pVertices;
		Maths::Store(beam.m_vPosition1, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourBack, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 1;
		pVertices->v = 0;
		++pVertices;
		Maths::Store(beam.m_vPosition2, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourFront, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 1;
		pVertices->v = 0;
		++pVertices;
		// triangle 2
		Maths::Store(beam.m_vPosition2, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourFront, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 1;
		pVertices->v = 0;
		++pVertices;
		Maths::Store(beam.m_vPosition3, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourFront, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 0;
		pVertices->v = 1;
		++pVertices;
		Maths::Store(beam.m_vPosition0, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourBack, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 0;
		pVertices->v = 1;
		++pVertices;
		// triangle 3
		Maths::Store(beam.m_vPosition2, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourFront, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 1;
		pVertices->v = 0;
		++pVertices;
		Maths::Store(beam.m_vPositionFront, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourFront, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 1;
		pVertices->v = 1;
		++pVertices;
		Maths::Store(beam.m_vPosition3, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store(vColourFront, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = 0;
		pVertices->v = 1;
		++pVertices;
	}
	return (void *) pVertices;
}



}; // namespace Tracer
}; // namespace DynamicEffect



#endif // RENDER_DYNAMICEFFECT_TRACER_BEAMCAPPED_H
