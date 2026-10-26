#include "stdafx.h"

#include "Dynamic/Particle/Shape/Point.h"

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
void* Point::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr	pVertices = (ParticleVertexPtr)pBuffer;
	Maths::HF32 vOne = 1.0f;

	#ifdef PLATFORM_SPU
	// allocate memory for temporary storage of particles, and store pointer to main memory
	ParticleVertexPtr pVerticesEA = pVertices;
	pVertices = (ParticleVertexPtr)Spu::Alloc(64 * sizeof(ParticleVertex));
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

			vPosition = vPosition * mLocal;

			Maths::HF32 vLife = vPositionLife.W();
			F32 fLife = pParticle->fLife;
			
			Maths::HVec4 vColourCurve = LookupValue(pSettings->colourTable, vLife, fLife);
			Maths::HVec4 vForceSizeLife(Maths::Load<Maths::HVec4>(&pParticle->vForceSizeLife));
			Maths::HVec4 vLighting(Maths::Load<Maths::HVec4>(&pParticle->vColour));
			
			Maths::HVec4 vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
			Maths::HVec4 vColour = vColourCurve * vLighting;
			vSize *= vForceSizeLife.Z();

			// front
			Maths::Store<Maths::HVec3>(vPosition, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = vSize.X();
			++pVertices;
		}

		#ifdef PLATFORM_SPU
		Spu::Wait(0);
		// store the blocks worth of particles to system memory
		Spu::StoreAsync((void*)pVerticesEA, (void*)pVerticesLS, sizeof(ParticleVertex) * pBlock->m_Element.Size(), 0);
		pVerticesEA += pBlock->m_Element.Size();
		#endif
	}
	return (void*)pVertices;
}

void* Point::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	return (void*)pBuffer;
}

#endif

//
//	XMVECTOR version
//


// build from a sorted list
#if defined(RENDER_DYNAMICEFFECT_USE_XMVECTOR)
void* Point::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr	pVertices = (ParticleVertexPtr)pBuffer;

	XMVECTOR vOne = XMVectorSplatOne();

	for(S32 i = 0; i < sortedList.Size(); ++i)
	{
		const C_Particle3D* pParticle = (C_Particle3D*)sortedList[i].pParticle;
		
		XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);	
		XMVECTOR vLife = XMVectorSplatW(vPosition);
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
		// front
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		XMStoreScalar((F32*)&pVertices->u, vSize);
		++pVertices;
	}
	return (void*)pVertices;
}

void* Point::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr	pVertices = (ParticleVertexPtr)pBuffer;
	XMVECTOR vOne = XMVectorSplatOne();

	ParticleBlock* pBlock = (ParticleBlock*)blocks[0];
	DCBT(0, &pBlock->m_Element[0]);
	DCBT(128, &pBlock->m_Element[0]);
	DCBT(256, &pBlock->m_Element[0]);

	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		ParticleBlock* pBlock = (ParticleBlock*)blocks[i];

		XMMATRIX mLocal = XMMatrixIdentity();
		if(pBlock->m_pNode)
		{
			XMVECTOR vRotation = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_pNode->GetRotation());
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_pNode->GetPosition());
			mLocal = XMMatrixAffineTransformation(XMVectorSplatOne(), XMVectorZero(), vRotation, vPosition);
		}

		U32 nBatch = pBlock->m_Element.Size() - (pBlock->m_Element.Size()%4);

		for(U32 a = 0; a < nBatch; a+=4)
		{
			const C_Particle3D* pParticle0 = (C_Particle3D*)&pBlock->m_Element[a];
			const C_Particle3D* pParticle1 = (C_Particle3D*)&pBlock->m_Element[a+1];
			const C_Particle3D* pParticle2 = (C_Particle3D*)&pBlock->m_Element[a+2];
			const C_Particle3D* pParticle3 = (C_Particle3D*)&pBlock->m_Element[a+3];

			DCBT(384, pParticle0);
			DCBT(512, pParticle0);
			
			XMVECTOR vPosition0 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle0->vPosition);	
			XMVECTOR vPosition1 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle1->vPosition);	
			XMVECTOR vPosition2 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle2->vPosition);	
			XMVECTOR vPosition3 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle3->vPosition);	
			XMVECTOR vLife0 = XMVectorSplatW(vPosition0);
			XMVECTOR vLife1 = XMVectorSplatW(vPosition1);
			XMVECTOR vLife2 = XMVectorSplatW(vPosition2);
			XMVECTOR vLife3 = XMVectorSplatW(vPosition3);
			
			vPosition0 = XMVector3TransformCoord(vPosition0, mLocal);
			vPosition1 = XMVector3TransformCoord(vPosition1, mLocal);
			vPosition2 = XMVector3TransformCoord(vPosition2, mLocal);
			vPosition3 = XMVector3TransformCoord(vPosition3, mLocal);

			// colour and size
			XMVECTOR vColour0, vColour1, vColour2, vColour3, vSize0, vSize1, vSize2, vSize3;
			{
				vColour0 = LookupValue(pSettings->colourTable, vLife0);
				vColour1 = LookupValue(pSettings->colourTable, vLife1);
				vColour2 = LookupValue(pSettings->colourTable, vLife2);
				vColour3 = LookupValue(pSettings->colourTable, vLife3);
				XMVECTOR vForceSizeLife0 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle0->vForceSizeLife);
				XMVECTOR vForceSizeLife1 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle1->vForceSizeLife);
				XMVECTOR vForceSizeLife2 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle2->vForceSizeLife);
				XMVECTOR vForceSizeLife3 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle3->vForceSizeLife);
				XMVECTOR vLighting0 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle0->vColour);
				XMVECTOR vLighting1 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle1->vColour);
				XMVECTOR vLighting2 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle2->vColour);
				XMVECTOR vLighting3 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle3->vColour);
				vSize0 = LookupValue(pSettings->sizeTable, vLife0);
				vSize1 = LookupValue(pSettings->sizeTable, vLife1);
				vSize2 = LookupValue(pSettings->sizeTable, vLife2);
				vSize3 = LookupValue(pSettings->sizeTable, vLife3);
				vSize0 = XMVectorMultiply(XMVectorSplatX(vSize0), XMVectorSplatZ(vForceSizeLife0));
				vSize1 = XMVectorMultiply(XMVectorSplatX(vSize1), XMVectorSplatZ(vForceSizeLife1));
				vSize2 = XMVectorMultiply(XMVectorSplatX(vSize2), XMVectorSplatZ(vForceSizeLife2));
				vSize3 = XMVectorMultiply(XMVectorSplatX(vSize3), XMVectorSplatZ(vForceSizeLife3));
				vColour0 = XMVectorMultiply(vColour0, vLighting0);
				vColour1 = XMVectorMultiply(vColour1, vLighting1);
				vColour2 = XMVectorMultiply(vColour2, vLighting2);
				vColour3 = XMVectorMultiply(vColour3, vLighting3);
			}

			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0); ReadWriteBarrier();
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour0); ReadWriteBarrier();
			XMStoreFloat2((XMFLOAT2*)&pVertices->u, vSize0); ReadWriteBarrier();
			++pVertices;
			
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition1); ReadWriteBarrier();
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour1); ReadWriteBarrier();
			XMStoreFloat2((XMFLOAT2*)&pVertices->u, vSize1); ReadWriteBarrier();
			++pVertices;
			
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2); ReadWriteBarrier();
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour2); ReadWriteBarrier();
			XMStoreFloat2((XMFLOAT2*)&pVertices->u, vSize2); ReadWriteBarrier();
			++pVertices;
			
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3); ReadWriteBarrier();
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour3); ReadWriteBarrier();
			XMStoreFloat2((XMFLOAT2*)&pVertices->u, vSize3); ReadWriteBarrier();
			++pVertices;
		}
		
		if(i + 1 < blocks.Size())
		{
			ParticleBlock* pNextBlock = (ParticleBlock*)blocks[i + 1];
			DCBT(0, &pNextBlock->m_Element[0]);
			DCBT(128, &pNextBlock->m_Element[0]);
			DCBT(256, &pNextBlock->m_Element[0]);
		}

		for(int a = nBatch; a < pBlock->m_Element.Size(); a++)
		{
			const C_Particle3D* pParticle = (C_Particle3D*)&pBlock->m_Element[a];

			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);	
			XMVECTOR vLife = XMVectorSplatW(vPosition);
			F32 fLife = pParticle->fLife;

			vPosition = XMVector3TransformCoord(vPosition, mLocal);

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

			// front
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition); ReadWriteBarrier();
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

