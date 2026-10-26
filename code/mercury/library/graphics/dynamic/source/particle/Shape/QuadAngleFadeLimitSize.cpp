#include "stdafx.h"

#include "Dynamic/Particle/Shape/QuadAngleFadeLimitSize.h"

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

#ifdef RENDER_DYNAMICEFFECT_USE_HVEC
void* QuadAngleFadeLimitSize::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	Maths::HVec3 vVertex0(-1.0f, 1.0f,  0.0f);
	Maths::HVec3 vVertex1(1.0f,  1.0f,  0.0f);
	Maths::HVec3 vVertex2(1.0f,  -1.0f, 0.0f);
	Maths::HVec3 vVertex3(-1.0f, -1.0f, 0.0f);
	Maths::HVec3 vNormal(0.0f, 0.0f, 1.0f);
	Maths::HVec3 vViewPosition(Maths::Load<Maths::HVec3>(&pConstants->vPosition));
	Maths::HVec3 vPivot(Maths::Load<Maths::HVec3>(&pSettings->pivot));
	vVertex0 += vPivot;
	vVertex1 += vPivot;
	vVertex2 += vPivot;
	vVertex3 += vPivot;

	Maths::HVec3 vView(Maths::Load<Maths::HVec3>(&pConstants->vPosition));
	Maths::HVec3 vForward(Maths::Load<Maths::HVec3>(&pConstants->vForward));
	Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&pConstants->mProj));
	Maths::HVec4 vLimitSize(Maths::Load<Maths::HVec4>(&pSettings->limitSize));
	Maths::HF32 vLimitSizeBegin(vLimitSize.X());
	Maths::HF32 vLimitSizeMax(vLimitSize.Y());
	Maths::HF32 vLimitSizeDiff(vLimitSize.Z());
	Maths::HF32 vLimitSizeDiffRcp(vLimitSize.W());

	Maths::HF32 vOne = 1.0f;
	
	#ifdef PLATFORM_SPU
	// allocate memory for double buffered temporary storage of particles, and store pointer to main memory
	ParticleVertexPtr pVerticesEA = pVertices;
	C_SpuDoubleBuffer<ParticleVertex> buffer;
	buffer.Alloc(64 * 4 * sizeof(ParticleVertex));
	#endif

	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		#ifdef PLATFORM_SPU
		pVertices = buffer.GetBuffer();
		#endif

		const ParticleBlock* pBlock = (const ParticleBlock*)blocks[i];
				
		Maths::HMat43 mLocal(Maths::UNDEFINED);
		if(pBlock->m_pNode)
		{
			mLocal = Maths::C_QuatTrans(*pBlock->m_pNode).Matrix();
		}
		else
			mLocal.SetIdentity();

		#ifdef PLATFORM_SPU
		// make sure any previous DMA has finished
		buffer.Wait();
		#endif

		for(S32 a = 0; a < pBlock->m_Element.Size(); a++)
		{
			const C_Particle3D_R3D* pParticle = (const C_Particle3D_R3D*)&pBlock->m_Element[a];
	
			Maths::HVec4 vPositionLife(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pParticle->vPosition));	
			Maths::HVec3 vPosition(vPositionLife);

			vPosition = vPosition * mLocal;

			Maths::HF32 vLife = vPositionLife.W();
			F32 fLife = pParticle->fLife;

			
			Maths::HVec4 vColourCurve(LookupValue(pSettings->colourTable, vLife, fLife));
			Maths::HVec4 vForceSizeLife(Maths::Load<Maths::HVec4>(&pParticle->vForceSizeLife));
			Maths::HVec4 vLighting(Maths::Load<Maths::HVec4>(&pParticle->vColour));
			Maths::HVec3 vSize(LookupValue(pSettings->sizeTable, vLife, fLife));
			vSize *= vForceSizeLife.Z();
			Maths::HVec4 vColour = vColourCurve * vLighting;

			// limit size on screen
			{
				Maths::HVec3 vViewToParticle = vPosition - vView;
				Maths::HF32 vDot = vViewToParticle | vForward;
				Maths::HVec4 vCoord(vSize.X(), Maths::Constants<Maths::HF32>::Zero(), vDot, Maths::Constants<Maths::HF32>::One());
				vCoord = vCoord * mProjection;
				Maths::HF32 vScreenSize = Maths::Abs(vCoord.X() / vCoord.W());

				if(pSettings->eLimitSize == E_ParticleLimitSize_Clamp && vScreenSize > vLimitSizeMax)
					vSize *= vLimitSizeMax / vScreenSize;
				
				if(pSettings->eLimitSize == E_ParticleLimitSize_AlphaFade)
				{
					Maths::HF32 fFadeAmount = (vScreenSize - vLimitSizeBegin) * vLimitSizeDiffRcp;
					Maths::HF32 fAlphaFade = Maths::Clamped(fFadeAmount, Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::One());
					fAlphaFade = Maths::Constants<Maths::HF32>::One() - fAlphaFade;
					vColour.W(vColour.W() * fAlphaFade);

					if(fAlphaFade == Maths::Constants<Maths::HF32>::Zero())
						vSize = Maths::Constants<Maths::HVec3>::Zero();
				}
			}

			DynamicEffect::TextureCoords<F32> uv0;
			const DynamicEffect::TextureCoordsMapping& map = pConstants->pTexCoords->GetTextureCoordinatesMapping(pParticle->iUV);
			if(pSettings->eAtlasType == E_AtlasType_Random)
				uv0 = pConstants->pTexCoords->GetTextureCoordinates<F32>(pParticle->iVariant);
			else
			{
				F32 fAtlasAnimCurrentPeriod = (pSettings->fAtlasAnimPeriod != 0.0f) 
											?	GetAnimationPosition(pParticle->fLife, pParticle->vForceSizeLife.w, pSettings->fAtlasAnimPeriod)  
											:	pParticle->fLife;

				uv0 = pConstants->pTexCoords->GetTextureCoordinates<F32>(GetAnimatedTextureCoordinate(fAtlasAnimCurrentPeriod, pSettings->iAtlasSize, pSettings->iAtlasOffset));
			}
			
			Maths::C_Quaternion qRotation(Maths::Load<Maths::HVec4>(&pParticle->qRotation));
			Maths::HMat33 mRotate(qRotation);

			mRotate = mRotate * mLocal.Rotation();

			Maths::HVec3 vFinalPosition0((vVertex0 * vSize) * mRotate + vPosition);
			Maths::HVec3 vFinalPosition1((vVertex1 * vSize) * mRotate + vPosition);
			Maths::HVec3 vFinalPosition2((vVertex2 * vSize) * mRotate + vPosition);
			Maths::HVec3 vFinalPosition3((vVertex3 * vSize) * mRotate + vPosition);

			Maths::HVec3 vToView = Maths::Normalise(vViewPosition - vPosition);
			Maths::HF32 vAlphaScale = Maths::Abs((vNormal * mRotate) | vToView);
			vColour.W(vColour.W() * vAlphaScale);

			// 0
			Maths::Store<Maths::HVec3>(vFinalPosition0, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m0];
			pVertices->v = uv0.v[map.m0];
			++pVertices;
			// 1
			Maths::Store<Maths::HVec3>(vFinalPosition1, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m1];
			pVertices->v = uv0.v[map.m1];
			++pVertices;
			// 2
			Maths::Store<Maths::HVec3>(vFinalPosition2, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m2];
			pVertices->v = uv0.v[map.m2];
			++pVertices;
#ifndef PLATFORM_PC
			// 3
			Maths::Store<Maths::HVec3>(vFinalPosition3, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m3];
			pVertices->v = uv0.v[map.m3];
			++pVertices;
#else
			// 2
			Maths::Store<Maths::HVec3>(vFinalPosition2, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m2];
			pVertices->v = uv0.v[map.m2];
			++pVertices;
			// 3
			Maths::Store<Maths::HVec3>(vFinalPosition3, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m3];
			pVertices->v = uv0.v[map.m3];
			++pVertices;
			// 0
			Maths::Store<Maths::HVec3>(vFinalPosition0, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m0];
			pVertices->v = uv0.v[map.m0];
			++pVertices;
#endif
		}

		#ifdef PLATFORM_SPU
		// store the blocks worth of particles to system memory
		Spu::StoreAsync((void*)pVerticesEA, (void*)buffer.GetBuffer(), sizeof(ParticleVertex) * pBlock->m_Element.Size() * 4, buffer.GetIndex());
		pVerticesEA += pBlock->m_Element.Size() * 4;
		buffer.Swap();
		#endif
	}

	#ifdef PLATFORM_SPU
	buffer.Wait();
	#endif

	return (void*)pVertices;
}
#endif

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
void* QuadAngleFadeLimitSize::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	XMVECTOR vVertex0 = XMVectorSet(-1.0f, 1.0f,  0.0f, 1.0f);
	XMVECTOR vVertex1 = XMVectorSet( 1.0f, 1.0f,  0.0f, 1.0f);
	XMVECTOR vVertex2 = XMVectorSet( 1.0f, -1.0f, 0.0f,  1.0f);
	XMVECTOR vVertex3 = XMVectorSet(-1.0f, -1.0f, 0.0f,  1.0f);
	XMVECTOR vNormal = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR vViewPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vPosition);
	XMVECTOR vPivot = XMLoadFloat4A16((XMFLOAT4A16*)&pSettings->pivot);
	vVertex0 = XMVectorAdd(vVertex0, vPivot);
	vVertex1 = XMVectorAdd(vVertex1, vPivot);
	vVertex2 = XMVectorAdd(vVertex2, vPivot);
	vVertex3 = XMVectorAdd(vVertex3, vPivot);

	XMVECTOR vView = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vPosition);
	XMVECTOR vForward = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vForward);
	XMMATRIX mProj = XMLoadFloat4x4A16((XMFLOAT4X4A16*)&pConstants->mProj);
	XMVECTOR vLimitSize = XMLoadFloat4A16((XMFLOAT4A16*)&pSettings->limitSize);
	XMVECTOR vLimitSizeBegin = XMVectorSplatX(vLimitSize);
	XMVECTOR vLimitSizeMax = XMVectorSplatY(vLimitSize);
	XMVECTOR vLimitSizeDiff = XMVectorSplatZ(vLimitSize);
	XMVECTOR vLimitSizeDiffRcp = XMVectorSplatW(vLimitSize);
	
	XMVECTOR vOne = XMVectorSplatOne();

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

		for(S32 a = 0; a < pBlock->m_Element.Size(); a++)
		{
			const C_Particle3D_R3D* pParticle = (C_Particle3D_R3D*)&pBlock->m_Element[a];

			// prefetch the next batch of particles, and attempt to prefetch Size Table.
			DCBT(1024, pParticle);
			DCBT(0,&pSettings->sizeTable);
			DCBT(0,&pSettings->colourTable);
	
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);
			XMVECTOR qRotation = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->qRotation);
			XMVECTOR vLife = XMVectorSplatW(vPosition);
			F32 fLife = pParticle->fLife;

			vPosition = XMVector3TransformCoord(vPosition, mLocal);
			
			XMMATRIX mRotate = XMMatrixRotationQuaternion(qRotation);
			mRotate = XMMatrixMultiply(mRotate, mLocal);

			// colour and size
			XMVECTOR vColour, vSize;
			{
				vColour = LookupValue(pSettings->colourTable, vLife, fLife);
				XMVECTOR vForceSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vForceSizeLife);
				XMVECTOR vLighting = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vColour);
				// TODO - Quiz charlie on wether it IS possible to make a vector-only calculation sort this out.
				// suprised that a LUT is as fast as indexing :(
				// other solutions - turn life-factor into a FixedPoint value ? (then shifts/masks can index)
				vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
				vSize = XMVectorMultiply(vSize, XMVectorSplatZ(vForceSizeLife));
				vColour = XMVectorMultiply(vColour, vLighting);
			}

			// limit size on screen
			{
				XMVECTOR vViewToParticle = XMVectorSubtract(vPosition, vView);
				XMVECTOR vDot = XMVector3Dot(vViewToParticle, vForward);
				XMVECTOR vCoord0 = XMVectorMergeXY(vSize, vDot);
				XMVECTOR vCoord1 = XMVectorMergeXY(XMVectorZero(), vOne);
				XMVECTOR vCoord = XMVectorMergeXY(vCoord0, vCoord1);
				XMVECTOR vX = XMVector4Dot(vCoord, mProj.r[0]);
				XMVECTOR vW = XMVector4Dot(vCoord, mProj.r[3]);
				XMVECTOR vScreenSize = XMVectorAbs(XMVectorMultiply(vX, XMVectorReciprocalEst(vW)));

				if(pSettings->eLimitSize == E_ParticleLimitSize_Clamp && XMVector4Greater(vScreenSize, vLimitSizeMax))
					vSize = XMVectorMultiply(vSize, XMVectorMultiply(vLimitSizeMax, XMVectorReciprocalEst(vScreenSize)));
				
				if(pSettings->eLimitSize == E_ParticleLimitSize_AlphaFade)
				{
					XMVECTOR fFadeAmount = XMVectorMultiply(XMVectorSubtract(vScreenSize, vLimitSizeBegin), vLimitSizeDiffRcp);
					XMVECTOR fAlphaFade = XMVectorClamp(fFadeAmount, XMVectorZero(), XMVectorSplatOne());
					fAlphaFade = XMVectorSubtract(vOne, fAlphaFade);

					vColour = XMVectorSelect(vColour, XMVectorMultiply(vColour, fAlphaFade), XMVectorSelectControl(0, 0, 0, 1));
					
					if(XMVector4LessOrEqual(fAlphaFade, XMVectorZero()))
						vSize = XMVectorZero();
				}
			}

			DynamicEffect::TextureCoords<F32> uv0;
			const DynamicEffect::TextureCoordsMapping& map = pConstants->pTexCoords->GetTextureCoordinatesMapping(pParticle->iUV);
			if(pSettings->eAtlasType == E_AtlasType_Random)
				uv0 = pConstants->pTexCoords->GetTextureCoordinates<F32>(pParticle->iVariant);
			else
			{
				F32 fAtlasAnimCurrentPeriod = (pSettings->fAtlasAnimPeriod != 0.0f) 
											?	GetAnimationPosition(pParticle->fLife, pParticle->vForceSizeLife.w, pSettings->fAtlasAnimPeriod)  
											:	pParticle->fLife;

				uv0 = pConstants->pTexCoords->GetTextureCoordinates<F32>(GetAnimatedTextureCoordinate(fAtlasAnimCurrentPeriod, pSettings->iAtlasSize, pSettings->iAtlasOffset));
			}

			XMVECTOR vTransformedVertex0 = XMVectorMultiply(vVertex0, vSize);
			XMVECTOR vTransformedVertex1 = XMVectorMultiply(vVertex1, vSize);
			XMVECTOR vTransformedVertex2 = XMVectorMultiply(vVertex2, vSize);
			XMVECTOR vTransformedVertex3 = XMVectorMultiply(vVertex3, vSize);
			
			vTransformedVertex0 = XMVector3TransformNormal(vTransformedVertex0, mRotate);
			vTransformedVertex1 = XMVector3TransformNormal(vTransformedVertex1, mRotate);
			vTransformedVertex2 = XMVector3TransformNormal(vTransformedVertex2, mRotate);
			vTransformedVertex3 = XMVector3TransformNormal(vTransformedVertex3, mRotate);

			// fade out based on angle
			XMVECTOR vToView = XMVector3NormalizeEst(XMVectorSubtract(vViewPosition, vPosition));
			XMVECTOR vTransformedNormal = XMVector3TransformNormal(vNormal, mRotate);
			XMVECTOR vAlphaScale = XMVectorAbs(XMVector3Dot(vTransformedNormal, vToView));
			vColour = XMVectorSelect(vColour, XMVectorMultiply(vAlphaScale, vColour), XMVectorSelectControl(0, 0, 0, 1));

			XMVECTOR vPosition0 = XMVectorAdd(vTransformedVertex0, vPosition);
			XMVECTOR vPosition1 = XMVectorAdd(vTransformedVertex1, vPosition);
			XMVECTOR vPosition2 = XMVectorAdd(vTransformedVertex2, vPosition);
			XMVECTOR vPosition3 = XMVectorAdd(vTransformedVertex3, vPosition);
						
			// 0
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[map.m0];
			pVertices->v = uv0.v[map.m0];
			++pVertices;
			// 1
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition1);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[map.m1];
			pVertices->v = uv0.v[map.m1];
			++pVertices;
			// 2
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[map.m2];
			pVertices->v = uv0.v[map.m2];
			++pVertices;
#ifndef PLATFORM_PC
			// 3
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[map.m3];
			pVertices->v = uv0.v[map.m3];
			++pVertices;	
#else
			// 2
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[map.m2];
			pVertices->v = uv0.v[map.m2];
			++pVertices;
			// 3
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[map.m3];
			pVertices->v = uv0.v[map.m3];
			++pVertices;
			// 0
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[map.m0];
			pVertices->v = uv0.v[map.m0];
			++pVertices;
#endif
		}
	}
	return (void*)pVertices;
}
#endif

}
} // ShapePolicy
} // Particles

