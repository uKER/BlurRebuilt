#ifndef RENDER_DYNAMICEFFECT_STRIP_SINGLESTRIP_H
#define RENDER_DYNAMICEFFECT_STRIP_SINGLESTRIP_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display\Buffer\VertexDeclarations.h"

namespace DynamicEffect
{
namespace Strip
{

template<typename T_Strip>
class SingleStrip
{
public:
	typedef T_Strip							StripType;
	typedef Display::Vertex_XYZ_HDR_UV		Vertex;

	void* Build(const RenderNodePtrs& blocks, volatile void* __restrict pBuffer, const I_StripEffectBase* pEffect) const;
};

template<typename T_Strip>
void* SingleStrip<T_Strip>::Build(const RenderNodePtrs& blocks, volatile void* __restrict pBuffer, const I_StripEffectBase* pEffect) const
{
	assert(pBuffer);
	Vertex* pVertices = (Vertex*)pBuffer;

	const typename StripType::ElementType* pDegenPrevElement = NULL;
	Maths::HVec3 vDegenVertexPosition;
	Maths::HVec3 vViewPosition(Maths::Load<Maths::HVec3>(&pEffect->GetRenderer()->GetViewPosition()));
	Maths::HVec3 vViewDirection(Maths::Load<Maths::HVec3>(&pEffect->GetRenderer()->GetViewDirection()));

	Maths::HVec3 vSide(0.5f, 0.0f, 0.0f);

	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		const StripType* pStrip = (StripType*)blocks[i];

		if(pStrip->m_Blocks.Size() > 0)
		{
			// add degenerate triangles to link individual tracks
			if(pDegenPrevElement)
			{
				// get the first and second elements of the strip cause we need them to work out vertex position
				assert(pStrip->GetElementCount() > 1);
				
				const typename StripType::ElementType* pElement = pStrip->GetElement(0);
								
				Maths::HVec4 vPositionWidth(Maths::Load<Maths::HVec4>((MathsStorage::Float4*)&pElement->vPosition));
				Maths::HVec3 vPosition(vPositionWidth);
				Maths::HF32 fWidth(vPositionWidth.W());
				Maths::C_Quaternion qRotation(Maths::Load<Maths::C_Quaternion>(&pElement->qRotation));
				Maths::HMat33 rotate(qRotation);
				Maths::HVec3 vOffset = vSide * rotate;

				Maths::HVec3 vNextVertexPosition = vPosition + vOffset;

				Maths::Store(vDegenVertexPosition, (MathsStorage::Float3*)&pVertices->x);
				pVertices->r = 0.0f;
				pVertices->g = 0.0f;
				pVertices->b = 0.0f;
				pVertices->a = 0.0f;
				pVertices->u = 0.0f;
				pVertices->v = 0.0f;
				++pVertices;

				Maths::Store(vNextVertexPosition, (MathsStorage::Float3*)&pVertices->x);
				pVertices->r = 0.0f;
				pVertices->g = 0.0f;
				pVertices->b = 0.0f;
				pVertices->a = 0.0f;
				pVertices->u = 1.0f;
				pVertices->v = 0.0f;
				++pVertices;
			}

			const typename StripType::ElementType* pPrevElement = NULL;

			for(typename StripType::BlockList::ConstIterator pIterator = pStrip->m_Blocks.Begin(); pIterator != pStrip->m_Blocks.End(); ++pIterator)
			{
				const typename StripType::ElementBlockType* pBlock = &(*pIterator);

				const U32 count = pBlock->Size();
				const U32 countLessOne = pBlock->Size() - 1;

				U32 begin = 0;

				// offset the start of the first block by the front offset
				if(pIterator == pStrip->m_Blocks.Begin())
					begin = pStrip->m_iOffset;

				for(U32 j = begin; j < count; ++j)
				{
					const typename StripType::ElementType* pElement = (typename StripType::ElementType*)&pBlock->m_Element[j];
					
					Maths::HVec4 vPositionWidth(Maths::Load<Maths::HVec4>((MathsStorage::Float4*)&pElement->vPosition));
					Maths::HVec3 vPosition(vPositionWidth);
					Maths::HF32 fWidth(vPositionWidth.W());
					
					Maths::C_Quaternion qRotation(Maths::Load<Maths::C_Quaternion>(&pElement->qRotation));
					Maths::HMat33 rotate(qRotation);
					Maths::HVec3 vOffset = vSide * rotate;
					
					Maths::HVec3 vRight = vPosition + vOffset;
					Maths::HVec3 vLeft = vPosition - vOffset;

					// right
					Maths::Store(vRight, (MathsStorage::Float3*)&pVertices->x);
					pVertices->r = pStrip->vColour.x;
					pVertices->g = pStrip->vColour.y;
					pVertices->b = pStrip->vColour.z;
					pVertices->a = pStrip->vColour.w * pElement->fLife;
					pVertices->u = 1.0f;
					pVertices->v = pElement->fTexCoord;
					++pVertices;

					// left
					Maths::Store(vLeft, (MathsStorage::Float3*)&pVertices->x);
					pVertices->r = pStrip->vColour.x;
					pVertices->g = pStrip->vColour.y;
					pVertices->b = pStrip->vColour.z;
					pVertices->a = pStrip->vColour.w * pElement->fLife;
					pVertices->u = 0.0f;
					pVertices->v = pElement->fTexCoord;
					++pVertices;

					pPrevElement = pElement;

					vDegenVertexPosition = vLeft;
					pDegenPrevElement = pElement;
				}

			}
		}
	}
		
	return (void*)pVertices;
}

} // Strip
} // DynamicEffect

#endif  // RENDER_DYNAMICEFFECT_STRIP_SINGLESTRIP_H


















