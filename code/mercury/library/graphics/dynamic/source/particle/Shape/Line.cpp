#include "stdafx.h"

#include "Dynamic/Particle/Shape/Line.h"

// dynamic effect level includes
#include "Dynamic/Node.h"
#include "Dynamic/RenderNode.h"
#include "Dynamic/VertexBuildConstants.h"

// particle level includes
#include "Dynamic/Particle/Particle.h"
#include "Dynamic/Particle/Block.h"
#include "Dynamic/Particle/VertexBuildSettings.h"

#ifndef PLATFORM_SPU
#include "Dynamic/Particle/EffectInterface.h"
#endif

namespace DynamicEffect
{
namespace Particle
{
namespace ShapePolicy
{

/*
// build from a sorted list
void* Line::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;
#if defined(RENDER_DYNAMICEFFECT_USE_XMVECTOR)

	XMVECTOR vOne = XMVectorSplatOne();
	XMVECTOR vTime = XMVectorReplicate(pConstants->fFrameTime);

	for(S32 i = 0; i < sortedList.Size(); ++i)
	{
		const C_Particle3D* pParticle = (C_Particle3D*)sortedList[i].pParticle;
		
		// calculate colour
		XMVECTOR vLife = XMVectorReplicate(pParticle->fLife);
		F32 fLife = pParticle->fLife;
		
		// colour and size
		XMVECTOR vColour, vSize;
		{
			vColour = LookupValue(pSettings->colourTable, vLife, fLife);
			XMVECTOR vForceSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vForceSizeLife);
			XMVECTOR vLighting = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vColour);
			vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
			vSize = XMVectorMultiply(XMVectorSplatX(vSize), XMVectorSplatZ(vForceSizeLife));
			vColour = XMVectorMultiply(vColour, vLighting);
		}

		XMVECTOR vVelocity = XMLoadFloat3A16((XMFLOAT3A16*)&pParticle->vVelocity);
		XMVECTOR vPosition = XMLoadFloat3A16((XMFLOAT3A16*)&pParticle->vPosition);	
			
		XMVECTOR vVelocityModifier = XMVectorMultiply(XMVectorSplatY(vSize), vLife);
		vVelocity = XMVectorMultiply(vVelocity, vVelocityModifier);
		vVelocity = XMVectorMultiply(vVelocity, vTime);
		
		XMVECTOR vFront = XMVectorAdd(vPosition, vVelocity);
		XMVECTOR vBack = XMVectorSubtract(vPosition, vVelocity);

		// front
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFront);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		++pVertices;

		// back
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vBack);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		++pVertices;
	}
#endif
	return (void*)pVertices;
}

void* Line::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;
#if defined(RENDER_DYNAMICEFFECT_USE_XMVECTOR)

	XMVECTOR vOne = XMVectorSplatOne();
	XMVECTOR vTime = XMVectorReplicate(pConstants->fFrameTime);

	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		ParticleBlock* pBlock = (ParticleBlock*)blocks[i];

		XMMATRIX mLocal = XMMatrixIdentity();
		if(pBlock->m_pNode)
		{
			XMVECTOR vRotation = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_pNode->vRotation);
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_pNode->vPosition);
			mLocal = XMMatrixAffineTransformation(XMVectorSplatOne(), XMVectorZero(), vRotation, vPosition);
		}

		for(S32 a = 0; a < pBlock->Size(); a++)
		{
			const C_Particle3D* pParticle = (C_Particle3D*)&pBlock->m_Element[a];
			
			// calculate colour
			XMVECTOR vLife = XMVectorReplicate(pParticle->fLife);
			F32 fLife = pParticle->fLife;
			
			// colour and size
			XMVECTOR vColour, vSize;
			{
				vColour = LookupValue(pSettings->colourTable, vLife, fLife);
				XMVECTOR vForceSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vForceSizeLife);
				XMVECTOR vLighting = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vColour);
				vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
				vSize = XMVectorMultiply(XMVectorSplatX(vSize), XMVectorSplatZ(vForceSizeLife));
				vColour = XMVectorMultiply(vColour, vLighting);
			}

			XMVECTOR vVelocity = XMLoadFloat3A16((XMFLOAT3A16*)&pParticle->vVelocity);
			XMVECTOR vPosition = XMLoadFloat3A16((XMFLOAT3A16*)&pParticle->vPosition);	
				
			XMVECTOR vVelocityModifier = XMVectorMultiply(XMVectorSplatY(vSize), vLife);
			vVelocity = XMVectorMultiply(vVelocity, vVelocityModifier);
			vVelocity = XMVectorMultiply(vVelocity, vTime);
			
			XMVECTOR vFront = XMVectorAdd(vPosition, vVelocity);
			XMVECTOR vBack = XMVectorSubtract(vPosition, vVelocity);

			// front
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFront);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			++pVertices;

			// back
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vBack);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			++pVertices;
		}
	}
#endif
	return (void*)pVertices;
}
*/

}
} // ShapePolicy
} // Particles

