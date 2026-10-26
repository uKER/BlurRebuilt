#include "stdafx.h"
// header

#include "Dynamic/Strip/Generic/Beam.h"
#include "Dynamic/Strip/Emitter.h"
#include "Dynamic/TypeIndex.h"

#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Context/Context.h"

#include <System/Misc/Random.h>

namespace DynamicEffect
{
namespace Strip
{

Util::C_Random g_Random;

C_Beam::C_Beam(C_Renderer* pRenderer, C_Manager* pManager, const U32 iIndex) : I_StripEffect<BeamParticle, BeamStrip, 100>(pRenderer, pManager, iIndex, E_EffectTypeIndex_Strip_Beam, sizeof(BeamParticle))
{
	m_fCycle = 0.0f;
}

C_Beam::~C_Beam()
{
}

bool C_Beam::Create()
{
	m_pTexture = m_pRenderer->LoadTexture("beam");

	return m_pTexture != NULL;
}

void C_Beam::Destroy()
{
	if(m_pTexture)
		m_pTexture->Release();
}

C_RenderNode* C_Beam::Spawn(const C_Emitter* pEmitter)
{
	Strip* pStrip = NULL;

	if(pEmitter)
	{
		U32 begin;
		pStrip = (Strip*)pEmitter->GetBlock();

		if(!pStrip)
		{
			// if this is the start of the strip we need to emit from position 0 (0.0f/1.0f interpolant)
			begin = 0;
			if((pStrip = NewStrip()))
			{
				Maths::HVec4 vColour1(Maths::Load<Maths::HVec4>(&pEmitter->GetDefinition()->vColour1));
				Maths::HVec4 vColour2(Maths::Load<Maths::HVec4>(&pEmitter->GetDefinition()->vColour2));
				Maths::HVec4 vRandom(g_Random.FRand(1.0f), g_Random.FRand(1.0f), g_Random.FRand(1.0f), g_Random.FRand(1.0f));
				Maths::HVec4 vColourDiff(vColour2 - vColour1);
				Maths::HVec4 vColour = vColour1 + vColourDiff * vRandom;

				Maths::Store(vColour, &pStrip->vColour);
				pStrip->fLife =  pEmitter->GetDefinition()->fStripLife + g_Random.FRand(-pEmitter->GetDefinition()->fStripLifeVariation, pEmitter->GetDefinition()->fStripLifeVariation);;
				pStrip->bUpdateStrip = pEmitter->GetDefinition()->bUpdateStrip;
				pStrip->bUpdateElements = pEmitter->GetDefinition()->bUpdateElements; 
				m_Strips.PushBack(*pStrip);
			}
		}
		else
		{
			// if we already have a strip then position 0 has already been created by the previous spawning (1.0f/1.0f interpolant)
			begin = 1;
			// increment interpolant inside emitter
			pEmitter->GetSpawnPosition();
		}

		if(pStrip)
		{
			F32 fLifeRcp = 1.0f / pEmitter->GetDefinition()->fElementLife;

			for(U32 i = begin; i <= pEmitter->GetSpawnSegments(); ++i)
			{
				if(ElementType* pParticle = (ElementType*)pStrip->PushBack())
				{
					Maths::HVec3 vPosition = pEmitter->GetSpawnPosition();
					Maths::HVec3 vRandom(g_Random.FRand(-pEmitter->GetDefinition()->vSpawnOffset.x, pEmitter->GetDefinition()->vSpawnOffset.x), g_Random.FRand(-pEmitter->GetDefinition()->vSpawnOffset.y, pEmitter->GetDefinition()->vSpawnOffset.y), g_Random.FRand(-pEmitter->GetDefinition()->vSpawnOffset.z, pEmitter->GetDefinition()->vSpawnOffset.z));
					vPosition += vRandom;

					Maths::Store(vPosition, &pParticle->vPosition);
					pParticle->fWidth = pEmitter->GetDefinition()->fWidth + g_Random.FRand(-pEmitter->GetDefinition()->fWidthVariation, pEmitter->GetDefinition()->fWidthVariation);
					pParticle->fLife = 1.0f;
					pParticle->fLifeScale = 1.0f / (pEmitter->GetDefinition()->fElementLife + g_Random.FRand(-pEmitter->GetDefinition()->fElementLifeVariation, pEmitter->GetDefinition()->fElementLifeVariation));
					pParticle->fTexCoord = pEmitter->GetSpawnTexCoord();
				}
			}
			pStrip->CalculateExtents();
		}
	}

	return pStrip;
}

void C_Beam::Update()
{
	const F32 frameTime = m_pRenderer->GetFrameTime();
	
	m_fCycle += m_pRenderer->GetFrameTime();

	for(StripList::Iterator pIterator = m_Strips.Begin(); pIterator != m_Strips.End(); )
	{
		bool recalcBounds = false;
		StripList::Iterator pStripIterator = pIterator;
		Strip* pStrip = &(*pIterator);
		++pIterator;

		if(pStrip->bUpdateStrip)
		{
			pStrip->fLife -= m_pRenderer->GetFrameTime();

			if(pStrip->fLife < 0.0f)
			{
				FreeStrip(pStripIterator);
				continue;
			}
		}

		if(pStrip->bUpdateElements)
		{
			for(Strip::BlockList::ConstIterator pIterator = pStrip->m_Blocks.Begin(); pIterator != pStrip->m_Blocks.End(); )
			{
				Strip::BlockList::ConstIterator pNext = pIterator;
				++pNext;
				
				const Strip::ElementBlockType* pBlock = &(*pIterator);

				U32 begin;

				if(pIterator == pStrip->m_Blocks.Begin())
					begin = pStrip->m_iOffset;
				else
					begin = 0;

				for(S32 i = begin; i < pBlock->m_Element.Size() && !pStrip->m_Blocks.Empty(); ++i)
				{
					ElementType* pElement = (ElementType*)&pBlock->m_Element[i];
					pElement->fLife -= pElement->fLifeScale * frameTime;

					if(pElement->fLife < 0.0f)
					{
						if(pIterator == pStrip->m_Blocks.Begin() && i == pStrip->m_iOffset)
						{
							recalcBounds = true;
							pStrip->PopFront();
						}
						else
							pElement->fLife = 0.0f;
					}
				}

				pIterator = pNext;
			}

			if(pStrip->m_Blocks.Size() == 0)
				FreeStrip(pStripIterator);
		}

		if(recalcBounds)
			pStrip->CalculateExtents();
	}

	
}

void C_Beam::OnRender(const C_View& view, Display::C_Context* pContext, const C_CompositeRenderNode& batch, const U32 states) const
{
	U32 vertexCount = 0;
	for(S32 i = 0; i < batch.blocks.Size(); ++i)
	{
		const Strip* pStrip = (Strip*)batch.blocks[i];
		U32 stripCount = pStrip->GetElementCount();
		if(stripCount > 0)
			vertexCount += stripCount * 2 + 2;
	}

	if(vertexCount > 0)
		vertexCount -= 2;
	else
		return;

	if(states & E_States_BindEffect)
	{
		pContext->SetTexture(0, m_pTexture);
		
		m_pRenderer->SetBlendMode(pContext, DynamicEffect::E_BlendMode_Alpha);
		pContext->SetVertexShader(&m_pManager->GetVertexShader());
		pContext->SetVertexDeclaration(Vertex::m_pDeclaration);
		pContext->SetPixelShader(&m_pManager->GetPixelShader());
	}

	Vertex* pVertices = (Vertex*)pContext->BeginVertices(Display::E_PrimitiveType_TriangleStrip, vertexCount, sizeof(Vertex), Vertex::m_pDeclaration);

	if(pVertices)
	{
		m_Build.Build(batch.blocks, pVertices, this);
		Display::Device()->EndVertices();
	}
}

} // Particle
} // DynamicEffect
















