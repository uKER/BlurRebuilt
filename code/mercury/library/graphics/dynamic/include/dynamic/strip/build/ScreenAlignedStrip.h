#ifndef RENDER_DYNAMICEFFECT_STRIP_SCREENALIGNEDSTRIP_H
#define RENDER_DYNAMICEFFECT_STRIP_SCREENALIGNEDSTRIP_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display\Buffer\VertexDeclarations.h"

namespace DynamicEffect
{
namespace Strip
{

template<typename T_Strip>
class ScreenAlignedStrip
{
public:
	typedef T_Strip							StripType;
	typedef Display::Vertex_XYZ_HDR_UV		Vertex;

	void* Build(const RenderNodePtrs& blocks, volatile void* __restrict pBuffer, const I_StripEffectBase* pEffect) const;
};

template<typename T_Strip>
void* ScreenAlignedStrip<T_Strip>::Build(const RenderNodePtrs& blocks, volatile void* __restrict pBuffer, const I_StripEffectBase* pEffect) const
{
	assert(pBuffer);
	Vertex* pVertices = (Vertex*)pBuffer;

	const typename StripType::ElementType* pDegenPrevElement = NULL;
	Maths::HVec3 vDegenVertexPosition;
	Maths::HVec3 vViewPosition;//(Maths::Load<Maths::HVec3>(&pEffect->GetRenderer()->GetViewPosition()));
	Maths::HVec3 vViewDirection;//(Maths::Load<Maths::HVec3>(&pEffect->GetRenderer()->GetViewDirection()));

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
				const typename StripType::ElementType* pNextElement = pStrip->GetElement(1);
				
				Maths::HVec3 vCross;
				Maths::HVec4 vPositionWidth(Maths::Load<Maths::HVec4>((MathsStorage::Float4*)&pElement->vPosition));
				Maths::HVec3 vPosition(vPositionWidth);
				Maths::HF32 fWidth(vPositionWidth.W());
				Maths::HVec3 vPositionNext(Maths::Load<Maths::HVec3>(&pNextElement->vPosition));
				Maths::HVec3 vPositionDiff = vPositionNext - vPosition;
				Maths::HVec3 vToView = vViewPosition - vPosition;
				vCross = vToView ^ vPositionDiff;
				vCross.Normalise();
				vCross = vCross * fWidth;
				
				Maths::HVec3 vNextVertexPosition = vPosition + vCross;

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

				const U32 count = pBlock->m_Element.Size();
				const U32 countLessOne = pBlock->m_Element.Size() - 1;

				U32 begin = 0;

				// offset the start of the first block by the front offset
				if(pIterator == pStrip->m_Blocks.Begin())
					begin = pStrip->m_iOffset;

				for(U32 j = begin; j < count; ++j)
				{
					const typename StripType::ElementType* pElement = (typename StripType::ElementType*)&pBlock->m_Element[j];
					const typename StripType::ElementType* pNextElement = NULL;

					// see if current element is last in block
					if(j == countLessOne)
					{
						// get next element from next block
						typename StripType::BlockList::ConstIterator pNextIterator = pIterator + 1;
						if(pNextIterator != pStrip->m_Blocks.End())
						{
							pNextElement = (typename StripType::ElementType*)&pNextIterator->m_Element[0];
						}
					}
					else
					{
						// get next element from current block
						pNextElement = (typename StripType::ElementType*)&pBlock->m_Element[j+1];
					}

					Maths::HVec3 vCross;
					Maths::HVec4 vPositionWidth(Maths::Load<Maths::HVec4>((MathsStorage::Float4*)&pElement->vPosition));
					Maths::HVec3 vPosition(vPositionWidth);
					Maths::HF32 fWidth(vPositionWidth.W());
					Maths::HVec3 vPositionDiff;

					if(pNextElement && pPrevElement)
					{
						// middle element, use from previous to next element
						Maths::HVec3 vPositionPrev(Maths::Load<Maths::HVec3>(&pPrevElement->vPosition));
						Maths::HVec3 vPositionNext(Maths::Load<Maths::HVec3>(&pNextElement->vPosition));
						vPositionDiff = vPositionNext - vPositionPrev;
					}
					else if(pNextElement)
					{
						// first element, use from current to next position
						Maths::HVec3 vPositionNext(Maths::Load<Maths::HVec3>(&pNextElement->vPosition));
						vPositionDiff = vPositionNext - vPosition;
						
					}
					else if(pPrevElement)
					{
						// last element, use from previous to current element
						Maths::HVec3 vPositionPrev(Maths::Load<Maths::HVec3>(&pPrevElement->vPosition));
						vPositionDiff = vPosition - vPositionPrev;
					}

					Maths::HVec3 vToView = vViewPosition - vPosition;
					vCross = vToView ^ vPositionDiff;
					vCross.Normalise();
					vCross = vCross * fWidth;

					// alpha fade based on viewing angle of beam, fade out when parallel
					Maths::HVec3 vPositionDiffNorm = Maths::Normalise(vPositionDiff);
					Maths::HVec3 vToViewNorm = Maths::Normalise(vToView);
					Maths::HF32 dot = Maths::Abs(vPositionDiffNorm | vToViewNorm);
					dot = dot * dot;
					Maths::HF32 alpha = Maths::Constants<Maths::HF32>::One() - dot;
					alpha *= Maths::Constants<Maths::HF32>::Two();
					alpha = Maths::Clamp(alpha, Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::One());
					
					Maths::HVec3 vRight = vPosition + vCross;
					Maths::HVec3 vLeft = vPosition - vCross;

					// right
					Maths::Store(vRight, (MathsStorage::Float3*)&pVertices->x);
					pVertices->r = pStrip->vColour.x;
					pVertices->g = pStrip->vColour.y;
					pVertices->b = pStrip->vColour.z;
					pVertices->a = pStrip->vColour.w * pElement->fLife * alpha;
					pVertices->u = 1.0f;
					pVertices->v = pElement->fTexCoord;
					++pVertices;

					// left
					Maths::Store(vLeft, (MathsStorage::Float3*)&pVertices->x);
					pVertices->r = pStrip->vColour.x;
					pVertices->g = pStrip->vColour.y;
					pVertices->b = pStrip->vColour.z;
					pVertices->a = pStrip->vColour.w * pElement->fLife * alpha;
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

#endif  // RENDER_DYNAMICEFFECT_STRIP_SCREENALIGNEDSTRIP_H


















