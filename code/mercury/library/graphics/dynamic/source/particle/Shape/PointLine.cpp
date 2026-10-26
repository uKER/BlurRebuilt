#include "stdafx.h"

#include "Dynamic/Particle/Shape/PointLine.h"

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

//
//	HVec version
//

#if defined(RENDER_DYNAMICEFFECT_USE_HVEC)
void* PointLine::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr	pVertices = (ParticleVertexPtr)pBuffer;
	Maths::HF32 vOne = 1.0f;
	Maths::HF32 vTime(pConstants->fFrameTime * 0.5f);
	Maths::HF32 vThird(0.3333333f);

	#ifdef PLATFORM_SPU
	// allocate memory for temporary storage of particles, and store pointer to main memory
	ParticleVertexPtr pVerticesEA = pVertices;
	pVertices = (ParticleVertexPtr)Spu::Alloc(64 * sizeof(ParticleVertex) * 4);
	ParticleVertexPtr pVerticesLS = pVertices;
	#endif

	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		#ifdef PLATFORM_SPU
		pVertices = pVerticesLS;
		#endif

		const ParticleBlock* pBlock = (const ParticleBlock*)blocks[i];
				
		Maths::HMat43 mLocal(Maths::UNDEFINED);
		if(pBlock->m_pNode)
		{
			mLocal = Maths::C_QuatTrans(*pBlock->m_pNode).Matrix();
		}
		else
			mLocal.SetIdentity();

		for(S32 a = 0; a < pBlock->m_Element.Size(); a++)
		{
			const C_Particle3D* pParticle = (const C_Particle3D*)&pBlock->m_Element[a];

			Maths::HVec4 vPositionLife(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pParticle->vPosition));	
			Maths::HVec3 vPosition(vPositionLife);
			Maths::HVec3 vVelocity(Maths::Load<Maths::HVec3>((MathsStorage::Float4A16*)&pParticle->vVelocity));	

			vPosition = vPosition * mLocal;
			vVelocity = vVelocity * mLocal.Rotation();
			
			Maths::HF32 vLife = vPositionLife.W();
			F32 fLife = pParticle->fLife;
			
			vVelocity = vVelocity * vTime;
			Maths::HVec3 vVelocityThird = vVelocity * vThird;

			Maths::HVec3 vPosition1 = vPosition + vVelocity;
			Maths::HVec3 vPosition2 = vPosition + vVelocityThird;
			Maths::HVec3 vPosition3 = vPosition - vVelocity;
			Maths::HVec3 vPosition4 = vPosition - vVelocityThird;
			
			Maths::HVec4 vColourCurve = LookupValue(pSettings->colourTable, vLife, fLife);
			Maths::HVec4 vForceSizeLife(Maths::Load<Maths::HVec4>(&pParticle->vForceSizeLife));
			Maths::HVec4 vLighting(Maths::Load<Maths::HVec4>(&pParticle->vColour));
			
			Maths::HVec4 vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
			Maths::HVec4 vColour = vColourCurve * vLighting;
			vSize *= vForceSizeLife.Z();

			Maths::Store<Maths::HVec3>(vPosition1, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = vSize.X();
			++pVertices;
			Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = vSize.X();
			++pVertices;
			Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = vSize.X();
			++pVertices;
			Maths::Store<Maths::HVec3>(vPosition4, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = vSize.X();
			++pVertices;
		}

		#ifdef PLATFORM_SPU
		Spu::Wait(0);
		// store the blocks worth of particles to system memory
		Spu::StoreAsync((void*)pVerticesEA, (void*)pVerticesLS, sizeof(ParticleVertex) * pBlock->m_Element.Size() * 4 , 0);
		pVerticesEA += pBlock->m_Element.Size() * 4;
		#endif
	}
	return (void*)pVertices;
}

void* PointLine::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	return (void*)pBuffer;
}

#endif

//
//	XMVECTOR version
//


// build from a sorted list
#if defined(RENDER_DYNAMICEFFECT_USE_XMVECTOR)
void* PointLine::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	return (void*)pBuffer;
}

void* PointLine::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr	pVertices = (ParticleVertexPtr)pBuffer;
	XMVECTOR vOne = XMVectorSplatOne();
	XMVECTOR vTime = XMVectorReplicate(pConstants->fFrameTime * 0.5f);
	XMVECTOR vThird = XMVectorReplicate(0.3333333f);

	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		ParticleBlock* pBlock = (ParticleBlock*)blocks[i];
		DCBT(0, &pBlock->m_Element[0]);
		DCBT(128, &pBlock->m_Element[0]);
		DCBT(256, &pBlock->m_Element[0]);

		XMMATRIX mLocal = XMMatrixIdentity();
		if(pBlock->m_pNode)
		{
			XMVECTOR vRotation = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_pNode->GetRotation());
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_pNode->GetPosition());
			mLocal = XMMatrixAffineTransformation(XMVectorSplatOne(), XMVectorZero(), vRotation, vPosition);
		}

		for(S32 a = 0; a < pBlock->m_Element.Size(); a++)
		{
			const C_Particle3D* pParticle = (C_Particle3D*)&pBlock->m_Element[a];

			DCBT(384, pParticle);
			DCBT(256, pParticle);

			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);
			XMVECTOR vVelocity = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vVelocity);
			XMVECTOR vLife = XMVectorSplatW(vPosition);
			F32 fLife = pParticle->fLife;

			vPosition = XMVector3TransformCoord(vPosition, mLocal);
			vVelocity = XMVector3TransformNormal(vVelocity, mLocal);
			vVelocity = XMVectorMultiply(vVelocity, vTime);
			XMVECTOR vVelocityThird = XMVectorMultiply(vVelocity, vThird);

			XMVECTOR vPosition1 = XMVectorAdd(vPosition, vVelocity);
			XMVECTOR vPosition2 = XMVectorAdd(vPosition, vVelocityThird);
			XMVECTOR vPosition3 = XMVectorSubtract(vPosition, vVelocity);
			XMVECTOR vPosition4 = XMVectorSubtract(vPosition, vVelocityThird);

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

			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition1); ReadWriteBarrier();
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour); ReadWriteBarrier();
			XMStoreFloat2((XMFLOAT2*)&pVertices->u, vSize); ReadWriteBarrier();
			++pVertices;
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2); ReadWriteBarrier();
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour); ReadWriteBarrier();
			XMStoreFloat2((XMFLOAT2*)&pVertices->u, vSize); ReadWriteBarrier();
			++pVertices;
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3); ReadWriteBarrier();
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour); ReadWriteBarrier();
			XMStoreFloat2((XMFLOAT2*)&pVertices->u, vSize); ReadWriteBarrier();
			++pVertices;
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition4); ReadWriteBarrier();
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour); ReadWriteBarrier();
			XMStoreFloat2((XMFLOAT2*)&pVertices->u, vSize); ReadWriteBarrier();
			++pVertices;
		}
	}
	return (void*)pVertices;
}
#endif

}
} // ShapePolicy
} // Particles

