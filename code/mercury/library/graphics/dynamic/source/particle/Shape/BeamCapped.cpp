#include "stdafx.h"

#include "Dynamic/Particle/Shape/BeamCapped.h"

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
void* BeamCapped::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	Maths::HVec3 vViewDirectionUp(Maths::Load<Maths::HVec3>(&pConstants->vUp));
	Maths::HVec3 vViewDirectionRight(Maths::Load<Maths::HVec3>(&pConstants->vRight));
	Maths::HVec3 vViewPosition(Maths::Load<Maths::HVec3>(&pConstants->vPosition));
	
	Maths::HF32 vOne = Maths::Constants<Maths::HF32>::One();
	Maths::HF32 vTime(pConstants->fFrameTime);
	Maths::HF32 vLifeScale;

	// normally we scale size by life
	if(pSettings->eParticleShape == E_ParticleShape_BeamCapped)
		vLifeScale = -Maths::Constants<Maths::HF32>::One();
	else
		vLifeScale = Maths::Constants<Maths::HF32>::Zero();

	#ifdef PLATFORM_SPU
	// allocate memory for double buffered temporary storage of particles, and store pointer to main memory
	ParticleVertexPtr pVerticesEA = pVertices;
	C_SpuDoubleBuffer<ParticleVertex> buffer;
	buffer.Alloc(64 * 12 * sizeof(ParticleVertex));
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
			const C_Particle3D* pParticle = (const C_Particle3D*)&pBlock->m_Element[a];
			
			// calculate colour
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
			
			DynamicEffect::TextureCoords<S16> uv = pConstants->pTexCoords->GetTextureCoordinates<S16>(pParticle->iVariant);
			const DynamicEffect::TextureCoordsMapping& map = pConstants->pTexCoords->GetTextureCoordinatesMapping(pParticle->iUV);

			Maths::HVec3 vVelocity(Maths::Load<Maths::HVec3>(&pParticle->vVelocity));
			vVelocity = vVelocity * mLocal.Rotation();
				
			Maths::HVec3 vToView = vViewPosition - vPosition;
			Maths::HVec3 vCross = vToView ^ vVelocity;
			vCross.Normalise();

			vCross = vCross * vSize.X() * vForceSizeLife.Z();
			Maths::HVec3 vCrossNeg = -vCross;

			Maths::HF32 vVelocityLifeScale = vLifeScale * vLife + vOne;
			Maths::HF32 vVelocityModifier = vSize.Y() * vVelocityLifeScale;
			vVelocity = vVelocity * vVelocityModifier;
			vVelocity = vVelocity * vTime;

			Maths::HVec3 vFront = vPosition + vVelocity;
			Maths::HVec3 vBack = vPosition - vVelocity;

			Maths::HVec3 vPosition0 = vBack + vCross;
			Maths::HVec3 vPosition1 = vBack + vCrossNeg;
			Maths::HVec3 vPosition2 = vFront + vCrossNeg;
			Maths::HVec3 vPosition3 = vFront + vCross;

			Maths::HVec3 vPosition4 = (vPosition0 + vPosition1) * 0.5f;
			Maths::HVec3 vPosition5 = (vPosition2 + vPosition3) * 0.5f;
			Maths::HVec3 vVectorBack = vPosition4 - vPosition0;
			Maths::HVec3 vVectorFront = vPosition5 - vPosition2;

			Maths::HF32 vFrontDotRight = vViewDirectionRight | vVectorFront;
			Maths::HF32 vFrontDotUp = vViewDirectionUp | vVectorFront;
			Maths::HVec3 vPositionFront = vPosition5;
			vPositionFront = vViewDirectionRight * -vFrontDotUp + vPositionFront;
			vPositionFront = vViewDirectionUp * vFrontDotRight + vPositionFront;

			Maths::HF32 vBackDotRight = vViewDirectionRight | vVectorBack;
			Maths::HF32 vBackDotUp = vViewDirectionUp | vVectorBack;
			Maths::HVec3 vPositionBack = vPosition4;
			vPositionBack = vViewDirectionRight * -vBackDotUp + vPositionBack;
			vPositionBack = vViewDirectionUp * vBackDotRight + vPositionBack;

			// Tri 1
				Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m3]; // make sure buffer writes are 4 bytes minimum
				++pVertices;
				Maths::Store<Maths::HVec3>(vPositionBack, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m0];
				++pVertices;
				Maths::Store<Maths::HVec3>(vPosition1, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;

			// Tri 2
				Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m3]; // make sure buffer writes are 4 bytes minimum
				++pVertices;
				Maths::Store<Maths::HVec3>(vPosition1, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;
				Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;

			// Tri 3
				Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;
				Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m3];
				++pVertices;
				Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m3];
				++pVertices;

			// Tri 4
				Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;
				Maths::Store<Maths::HVec3>(vPositionFront, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m2];
				++pVertices;
				Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				(U32&)pVertices->u = (U32&)uv.u[map.m3];
				++pVertices;
		}

		
		#ifdef PLATFORM_SPU
		//LOGFATAL(0,"Store Dest %x, Src %x", (void*)pVerticesEA, (void*)buffer.GetBuffer());
		// store the blocks worth of particles to system memory
		Spu::StoreAsync((void*)pVerticesEA, (void*)buffer.GetBuffer(), sizeof(ParticleVertex) * pBlock->m_Element.Size() * 12, buffer.GetIndex());
		pVerticesEA += pBlock->m_Element.Size() * 12;
		buffer.Swap();
		#endif
	}

	#ifdef PLATFORM_SPU
	pVertices = pVerticesEA;
	buffer.Wait();
	#endif

	return (void*)pVertices;
}
#endif

//
//	XMVECTOR version
//

#if defined(RENDER_DYNAMICEFFECT_USE_XMVECTOR)
void* BeamCapped::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	XMVECTOR vViewPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vPosition);
	XMVECTOR vViewDirectionUp = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vUp);
	XMVECTOR vViewDirectionRight = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vRight);
	XMVECTOR vOne = XMVectorSplatOne();
	XMVECTOR vTime = XMVectorReplicate(pConstants->fFrameTime);
	
	XMVECTOR vLifeScale;
	if(pSettings->eParticleShape == E_ParticleShape_BeamCapped)
		vLifeScale = XMVectorNegate(XMVectorSplatOne());
	else
		vLifeScale = XMVectorZero();


	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		ParticleBlock* pBlock = (ParticleBlock*)blocks[i];
		DCBT(0, &pBlock->m_Element[0]);

		XMMATRIX mLocal;
		if(pBlock->m_pNode)
		{
			XMVECTOR vRotation = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_pNode->GetRotation());
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_pNode->GetPosition());
			mLocal = XMMatrixAffineTransformation(XMVectorSplatOne(), XMVectorZero(), vRotation, vPosition);
		}
		else
			mLocal = XMMatrixIdentity();

		for(S32 a = 0; a < pBlock->m_Element.Size(); a++)
		{
			const C_Particle3D* pParticle = (C_Particle3D*)&pBlock->m_Element[a];
			DCBT(128, pParticle);
			
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);	
			// calculate colour
			XMVECTOR vLife = XMVectorSplatW(vPosition);
			F32 fLife = pParticle->fLife;

			DynamicEffect::TextureCoords<S16> uv = pConstants->pTexCoords->GetTextureCoordinates<S16>(pParticle->iVariant);
			const DynamicEffect::TextureCoordsMapping& map = pConstants->pTexCoords->GetTextureCoordinatesMapping(pParticle->iUV);

			vPosition = XMVector3TransformCoord(vPosition, mLocal);
			XMVECTOR vVelocity = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vVelocity);
			vVelocity = XMVector3TransformNormal(vVelocity, mLocal);
			XMVECTOR vToView = vViewPosition - vPosition;
			XMVECTOR vCross = XMVector3NormalizeEst(XMVector3Cross(vToView, vVelocity));

			// colour and size
			XMVECTOR vColour, vSize, vForceSizeLife;
			{
				vForceSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vForceSizeLife);
				XMVECTOR vLighting = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vColour);

				vColour = LookupValue(pSettings->colourTable, vLife, fLife);
				vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
				vColour = XMVectorMultiply(vColour, vLighting);
			}

			vCross = XMVectorMultiply(vCross, XMVectorMultiply(XMVectorSplatX(vSize), XMVectorSplatZ(vForceSizeLife)));
			XMVECTOR vCrossNeg = XMVectorNegate(vCross);

			XMVECTOR vVelocityLifeScale = XMVectorMultiplyAdd(vLifeScale, vLife, vOne); 
			XMVECTOR vVelocityModifier = XMVectorMultiply(XMVectorSplatY(vSize), vVelocityLifeScale);
			vVelocity = XMVectorMultiply(vVelocity, vVelocityModifier);
			vVelocity = XMVectorMultiply(vVelocity, vTime);

			XMVECTOR vFront = XMVectorAdd(vPosition, vVelocity);
			XMVECTOR vBack = XMVectorSubtract(vPosition, vVelocity);

			XMVECTOR vPosition0 = XMVectorAdd(vBack, vCross);
			XMVECTOR vPosition1 = XMVectorAdd(vBack, vCrossNeg);
			XMVECTOR vPosition2 = XMVectorAdd(vFront, vCrossNeg);
			XMVECTOR vPosition3 = XMVectorAdd(vFront, vCross);

			static CONST XMVECTOR vHalf = {0.5f, 0.5f, 0.5f, 0.5f};

			XMVECTOR vPosition4 = XMVectorMultiply(XMVectorAdd(vPosition0, vPosition1), vHalf);
			XMVECTOR vPosition5 = XMVectorMultiply(XMVectorAdd(vPosition2, vPosition3), vHalf);
			XMVECTOR vVectorBack = XMVectorSubtract(vPosition4, vPosition0);
			XMVECTOR vVectorFront = XMVectorSubtract(vPosition5, vPosition2);

			XMVECTOR vFrontDotRight = XMVector3Dot(vViewDirectionRight, vVectorFront);
			XMVECTOR vFrontDotUp = XMVector3Dot(vViewDirectionUp, vVectorFront);
			XMVECTOR vPositionFront = vPosition5;
			vPositionFront = XMVectorMultiplyAdd(vViewDirectionRight, XMVectorNegate(vFrontDotUp), vPositionFront);
			vPositionFront = XMVectorMultiplyAdd(vViewDirectionUp, vFrontDotRight, vPositionFront);

			XMVECTOR vBackDotRight = XMVector3Dot(vViewDirectionRight, vVectorBack);
			XMVECTOR vBackDotUp = XMVector3Dot(vViewDirectionUp, vVectorBack);
			XMVECTOR vPositionBack = vPosition4;
			vPositionBack = XMVectorMultiplyAdd(vViewDirectionRight, XMVectorNegate(vBackDotUp), vPositionBack);
			vPositionBack = XMVectorMultiplyAdd(vViewDirectionUp, vBackDotRight, vPositionBack);

			// Tri 1
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m3]; // make sure buffer writes are 4 bytes minimum
				++pVertices;
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPositionBack);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m0];
				++pVertices;
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition1);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;

			// Tri 2
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m3];
				++pVertices;
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition1);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;

			// Tri 3
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m3];
				++pVertices;
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m3];
				++pVertices;

			// Tri 4
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m1];
				++pVertices;
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPositionFront);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m2];
				++pVertices;
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				(U32&)pVertices->u = (U32&)uv.u[map.m3];
				++pVertices;
		}
	}
	return (void*)pVertices;
}
#endif

}
} // ShapePolicy
} // Particles

