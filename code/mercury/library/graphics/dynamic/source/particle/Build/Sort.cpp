#include "stdafx.h"

// dynamic level includes
#include "Dynamic\VertexBuildConstants.h"
#include "Dynamic\RenderNode.h"

// particle level includes
#include "Dynamic\Particle\Particle.h"
#include "Dynamic\Particle\Block.h"
#include "Dynamic\Particle\VertexBuildSettings.h"

// build level includes
#include "Dynamic\Particle\Build\Sort.h"

// jobqueue header
#include <system/scheduler/jobqueue.h>

// sorting
#include <core\containers\radix.h>

namespace DynamicEffect
{
namespace Particle
{

#ifndef PLATFORM_SPU
ParticleArray	g_ParticleSortList;
#endif

//#ifndef CFG_GOLD
//extern	Debug::Bool g_debugParticleSortParticles;
//#endif

void SortParticles(const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, ParticleArray& sortList)
{
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
	XMVECTOR vViewDirection = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vForward);
	XMVECTOR vViewDirectionW = XMVectorSplatW(vViewDirection);
	XMVECTOR vMaxInt = XMVectorReplicate((F32)0xFFFFFFFF);
#else
	Maths::HVec4 vViewDirection4(Maths::Load<Maths::HVec4>(&pConstants->vForward));
	Maths::HVec3 vViewDirection(vViewDirection4);
	
	Maths::HF32 viewDirectionW(vViewDirection4.W());
	Maths::HF32 maxInt((F32)0xFFFFFFFF);
#endif

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
	// use the min and max z position of the blocks and size to get a depth range we want can scale our depths into
	// this gives us maximal usage of the 32bit int for sorting
	XMVECTOR vMinZ = XMVectorReplicate(10000.0f);
	XMVECTOR vMaxZ = XMVectorReplicate(-10000.0f);
	XMVECTOR vMaxSize = XMVectorZero();

	{
		Spu::MemBaseMarker marker;
		for(S32 i = 0; i < blocks.Size(); ++i)
		{
			const C_RenderNode* pBlock = (const C_RenderNode*)Spu::FetchAlloc(blocks[i], sizeof(C_RenderNode));

			XMVECTOR vCentre = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_vCentre);
			XMVECTOR vSize = XMVector3LengthEst(XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_vSize));

			vMaxSize = XMVectorMax(vSize, vMaxSize);

			XMVECTOR z = XMVector3Dot(vViewDirection, vCentre);
			z = XMVectorAdd(vViewDirectionW, z);

			vMinZ = XMVectorMin(vMinZ, z);
			vMaxZ = XMVectorMax(vMaxZ, z);
		}
	}
	vMinZ = XMVectorSubtract(vMinZ, vMaxSize);
	vMaxZ = XMVectorAdd(vMaxZ, vMaxSize);
	XMVECTOR vDiffZ = XMVectorSubtract(vMaxZ, vMinZ);
	XMVECTOR vDepthScale = XMVectorMultiply(XMVectorReplicate(4294967296.0f), XMVectorReciprocalEst(vDiffZ));
#else
	Maths::HF32 minZ(10000.0f);
	Maths::HF32 maxZ(-10000.0f);
	Maths::HF32 maxSize(0.0f);

	{
		for(S32 i = 0; i < blocks.Size(); ++i)
		{
			const C_RenderNode* pBlock = (const C_RenderNode*)blocks[i];

			Maths::HVec3 vCentre(Maths::Load<Maths::HVec3>(&pBlock->m_vCentre));
			Maths::HVec3 vSize(Maths::Load<Maths::HVec3>(&pBlock->m_vSize));
			
			Maths::HF32 size = Maths::Length(vSize);

			maxSize = Maths::Max(maxSize, size);

			Maths::HF32 z = vViewDirection | vCentre;
			z += viewDirectionW;

			minZ = Maths::Min(minZ, z);
			maxZ = Maths::Max(maxZ, z);
		}
	}
	minZ -= maxSize;
	maxZ += maxSize;
	Maths::HF32 diffZ = maxZ - minZ;
	Maths::HF32 depthScale = 4294967296.0f * diffZ.Rcp();
#endif

	{
		#ifndef PLATFORM_SPU
		PERF_EVENT("Particle.Radix::Build");
		#endif
		Spu::MemBaseMarker marker;

		for(S32 i = 0; i < blocks.Size(); ++i)
		{
			const ParticleBlock* pBlock = (const ParticleBlock*)blocks[i];
			U16 nBatch = pBlock->m_Element.Size() - (pBlock->m_Element.Size()%4);
			// work out the depths of 4 particles at a time
			for(U16 a = 0; a < nBatch; a+=4)
			{
				// prefetch the next batch of particles
				DCBT(1024, &pBlock->m_Element[a]);

				const Particle3D* pParticle0 = (const Particle3D*)&pBlock->m_Element[a];
				const Particle3D* pParticle1 = (const Particle3D*)&pBlock->m_Element[a + 1];
				const Particle3D* pParticle2 = (const Particle3D*)&pBlock->m_Element[a + 2];
				const Particle3D* pParticle3 = (const Particle3D*)&pBlock->m_Element[a + 3];

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR vPosition[4];
				vPosition[0] = XMLoadFloat3A16((XMFLOAT3*)&pParticle0->vPosition);
				vPosition[1] = XMLoadFloat3A16((XMFLOAT3*)&pParticle1->vPosition);
				vPosition[2] = XMLoadFloat3A16((XMFLOAT3*)&pParticle2->vPosition);
				vPosition[3] = XMLoadFloat3A16((XMFLOAT3*)&pParticle3->vPosition);
				
				XMVECTOR vDepth[4];
				vDepth[0] = XMVector3Dot(vViewDirection, vPosition[0]);
				vDepth[1] = XMVector3Dot(vViewDirection, vPosition[1]);
				vDepth[2] = XMVector3Dot(vViewDirection, vPosition[2]);
				vDepth[3] = XMVector3Dot(vViewDirection, vPosition[3]);

				vDepth[0] = XMVectorAdd(vDepth[0], vViewDirectionW);
				vDepth[1] = XMVectorAdd(vDepth[1], vViewDirectionW);
				vDepth[2] = XMVectorAdd(vDepth[2], vViewDirectionW);
				vDepth[3] = XMVectorAdd(vDepth[3], vViewDirectionW);

				vDepth[0] = XMVectorSubtract(vDepth[0], vMinZ);
				vDepth[1] = XMVectorSubtract(vDepth[1], vMinZ);
				vDepth[2] = XMVectorSubtract(vDepth[2], vMinZ);
				vDepth[3] = XMVectorSubtract(vDepth[3], vMinZ);

				vDepth[0] = XMVectorMultiply(vDepth[0], vDepthScale);
				vDepth[1] = XMVectorMultiply(vDepth[1], vDepthScale);
				vDepth[2] = XMVectorMultiply(vDepth[2], vDepthScale);
				vDepth[3] = XMVectorMultiply(vDepth[3], vDepthScale);

				vDepth[0] = XMVectorSubtract(vMaxInt, vDepth[0]);
				vDepth[1] = XMVectorSubtract(vMaxInt, vDepth[1]);
				vDepth[2] = XMVectorSubtract(vMaxInt, vDepth[2]);
				vDepth[3] = XMVectorSubtract(vMaxInt, vDepth[3]);

				U32 nDepth0 = (U32)vDepth[0].x;
				U32 nDepth1 = (U32)vDepth[1].x;
				U32 nDepth2 = (U32)vDepth[2].x;
				U32 nDepth3 = (U32)vDepth[3].x;
#else
				Maths::HVec3 vPosition0(Maths::Load<Maths::HVec3>(&pParticle0->vPosition));
				Maths::HVec3 vPosition1(Maths::Load<Maths::HVec3>(&pParticle1->vPosition));
				Maths::HVec3 vPosition2(Maths::Load<Maths::HVec3>(&pParticle2->vPosition));
				Maths::HVec3 vPosition3(Maths::Load<Maths::HVec3>(&pParticle3->vPosition));
				
				Maths::HF32 depth0 = vViewDirection | vPosition0;
				Maths::HF32 depth1 = vViewDirection | vPosition1;
				Maths::HF32 depth2 = vViewDirection | vPosition2;
				Maths::HF32 depth3 = vViewDirection | vPosition3;

				depth0 += viewDirectionW;
				depth1 += viewDirectionW;
				depth2 += viewDirectionW;
				depth3 += viewDirectionW;

				depth0 -= minZ;
				depth1 -= minZ;
				depth2 -= minZ;
				depth3 -= minZ;

				depth0 *= depthScale;
				depth1 *= depthScale;
				depth2 *= depthScale;
				depth3 *= depthScale;

				depth0 = maxInt - depth0;
				depth1 = maxInt - depth1;
				depth2 = maxInt - depth2;
				depth3 = maxInt - depth3;

				U32 nDepth0 = (U32)depth0;
				U32 nDepth1 = (U32)depth1;
				U32 nDepth2 = (U32)depth2;
				U32 nDepth3 = (U32)depth3;
#endif

				sortList.PushBack( ParticleSortInfo(pParticle0, nDepth0) );
				sortList.PushBack( ParticleSortInfo(pParticle1, nDepth1) );
				sortList.PushBack( ParticleSortInfo(pParticle2, nDepth2) );
				sortList.PushBack( ParticleSortInfo(pParticle3, nDepth3) );
			}
			for(U16 a = nBatch; a < pBlock->m_Element.Size(); a++)
			{
				const Particle3D* pParticle = (const Particle3D*)&pBlock->m_Element[a];

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR vPosition = XMLoadFloat3A16((XMFLOAT3*)&pParticle->vPosition);
				XMVECTOR vDepth = XMVector3Dot(vViewDirection, vPosition);
				vDepth = XMVectorAdd(vDepth, vViewDirectionW);

				vDepth = XMVectorSubtract(vDepth, vMinZ);
				vDepth = XMVectorMultiply(vDepth, vDepthScale);
				vDepth = XMVectorSubtract(vMaxInt, vDepth);
				U32 nDepth = (U32)vDepth.x;
#else
				Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&pParticle->vPosition));
				Maths::HF32 depth = vViewDirection | vPosition;
				depth += viewDirectionW;

				depth -= minZ;
				depth *= depthScale;
				depth = maxInt - depth;
				U32 nDepth = (U32)depth;	
#endif
				sortList.PushBack( ParticleSortInfo(pParticle, nDepth) );
			}
		}
	}

	//#ifndef CFG_GOLD
	///#ifndef PLATFORM_SPU
	//if(g_debugParticleSortParticles)
	//#endif
	//#endif
	{
		#ifndef PLATFORM_SPU
		PERF_EVENT("Particle.Radix::Sort");
		#endif
		Ut::RadixSort<ParticleArray> radixSorter;
		radixSorter.Sort(sortList);
	}
}

} // Particle
} // DynamicEffect
