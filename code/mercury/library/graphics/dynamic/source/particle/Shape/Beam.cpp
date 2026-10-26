#include "stdafx.h"

#include "Dynamic/Particle/Shape/Beam.h"

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

// build from a sorted list
void* Beam::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

#if defined(RENDER_DYNAMICEFFECT_USE_HVEC)
	Maths::HVec3 vViewDirectionUp(Maths::Load<Maths::HVec3>(&pConstants->vUp));
	Maths::HVec3 vViewDirectionRight(Maths::Load<Maths::HVec3>(&pConstants->vRight));
	Maths::HVec3 vViewPosition(Maths::Load<Maths::HVec3>(&pConstants->vPosition));
	
	Maths::HF32 vOne = Maths::Constants<Maths::HF32>::One();
	Maths::HF32 vTime(pConstants->fFrameTime);
	
#ifdef PLATFORM_SPU
	S32 batchCount = 64;
	// allocate memory for temporary storage of particles, and store pointer to main memory
	ParticleVertexPtr pVerticesEA = pVertices;
	C_SpuDoubleBuffer<ParticleVertex> buffer;
	buffer.Alloc(4 * batchCount * sizeof(ParticleVertex));
	S32 batch = 0;
#endif

	for(S32 a = 0; a < sortedList.Size(); ++a)
	{
		// set a memory marker for the per particle data
		Spu::MemBaseMarker marker;

#ifdef PLATFORM_SPU
		// reset the vertex pointer back into to the start of temp vertex storage, making sure DMA are done
		if(batch == 0)
			pVertices = buffer.GetBuffer();
#endif

		const C_Particle3D* pParticle = (const C_Particle3D*)sortedList[a].pParticle;
						
		// calculate colour
		Maths::HVec4 vPositionLife(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pParticle->vPosition));	
		Maths::HVec3 vPosition(vPositionLife);

		Maths::HF32 vLife = vPositionLife.W();
		F32 fLife = pParticle->fLife;

		Maths::HVec4 vColourCurve = LookupValue(pSettings->colourTable, vLife, fLife);
		Maths::HVec4 vForceSizeLife(Maths::Load<Maths::HVec4>(&pParticle->vForceSizeLife));
		Maths::HVec4 vLighting(Maths::Load<Maths::HVec4>(&pParticle->vColour));
		
		Maths::HVec4 vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
		Maths::HVec4 vColour = vColourCurve * vLighting;
		
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

		Maths::HVec3 vVelocity(Maths::Load<Maths::HVec3>(&pParticle->vVelocity));
			
		Maths::HVec3 vToView = vViewPosition - vPosition;
		Maths::HVec3 vCross = vToView ^ vVelocity;
		vCross.Normalise();

		vCross = vCross * vSize.X() * vForceSizeLife.Z();
		Maths::HVec3 vCrossNeg = -vCross;

		Maths::HF32 vVelocityModifier = vSize.Y() * vLife;
		vVelocity = vVelocity * vVelocityModifier;
		vVelocity = vVelocity * vTime;

		Maths::HVec3 vFront = vPosition + vVelocity;
		Maths::HVec3 vBack = vPosition - vVelocity;

		Maths::HVec3 vPosition0 = vBack + vCross;
		Maths::HVec3 vPosition1 = vBack + vCrossNeg;
		Maths::HVec3 vPosition2 = vFront + vCrossNeg;
		Maths::HVec3 vPosition3 = vFront + vCross;

#ifdef PLATFORM_SPU
		if(batch == 0)
			buffer.Wait();
#endif

		// 0
		Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = uv0.u[map.m0];
		pVertices->v = uv0.v[map.m0];
		++pVertices;
		// 1
		Maths::Store<Maths::HVec3>(vPosition1, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = uv0.u[map.m1];
		pVertices->v = uv0.v[map.m1];
		++pVertices;
		// 2
		Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = uv0.u[map.m2];
		pVertices->v = uv0.v[map.m2];
		++pVertices;
	#ifndef PLATFORM_PC
		// 3
		Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = uv0.u[map.m3];
		pVertices->v = uv0.v[map.m3];
		++pVertices;
	#else
		// 2
		Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = uv0.u[map.m2];
		pVertices->v = uv0.u[map.m2];
		++pVertices;
		// 3
		Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = uv0.u[map.m3];
		pVertices->v = uv0.u[map.m3];
		++pVertices;
		// 0
		Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u = uv0.u[map.m0];
		pVertices->v = uv0.u[map.m0];
		++pVertices;
	#endif

#ifdef PLATFORM_SPU
		if(++batch == batchCount)
		{
			Spu::StoreAsync((void*)pVerticesEA, (void*)buffer.GetBuffer(), sizeof(ParticleVertex) * 4 * batch, buffer.GetIndex());
			pVerticesEA += 4 * batch;
			buffer.Swap();
			batch = 0;
		}
#endif
	}

#ifdef PLATFORM_SPU
	if(batch > 0)
	{
		Spu::StoreAsync((void*)pVerticesEA, (void*)buffer.GetBuffer(), sizeof(ParticleVertex) * 4 * batch, buffer.GetIndex());
		pVerticesEA += 4 * batch;
		buffer.Swap();
		batch = 0;
	}
	buffer.Wait();
#endif

#endif

#if defined(RENDER_DYNAMICEFFECT_USE_XMVECTOR)
	XMVECTOR vViewPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vPosition);
	XMVECTOR vViewDirectionUp = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vUp);
	XMVECTOR vViewDirectionRight = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vRight);
	XMVECTOR vOne = XMVectorSplatOne();
	XMVECTOR vTime = XMVectorReplicate(pConstants->fFrameTime);
	

	for(S32 i = 0; i < sortedList.Size(); ++i)
	{
		const C_Particle3D* pParticle = (const C_Particle3D*)sortedList[i].pParticle;
		
		// calculate colour
		XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);	
		XMVECTOR vLife = XMVectorSplatW(vPosition);
		F32 fLife = pParticle->fLife;
		// colour and size
		XMVECTOR vColour, vSize, vForceSizeLife;
		{
			vColour = LookupValue(pSettings->colourTable, vLife, fLife);
			vForceSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vForceSizeLife);
			XMVECTOR vLighting = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vColour);
			vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
			vColour = XMVectorMultiply(vColour, vLighting);
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
		XMVECTOR vVelocity = XMLoadFloat3A16((XMFLOAT3A16*)&pParticle->vVelocity);
		
			
		XMVECTOR vVelocityModifier = XMVectorMultiply(XMVectorSplatY(vSize), vLife);
		vVelocity = XMVectorMultiply(vVelocity, vVelocityModifier);
		vVelocity = XMVectorMultiply(vVelocity, vTime);
		
		XMVECTOR vToView = vViewPosition - vPosition;
		XMVECTOR vCross = XMVector3NormalizeEst(XMVector3Cross(vToView, vVelocity));

		vCross = XMVectorMultiply(vCross, XMVectorMultiply(XMVectorSplatX(vSize), XMVectorSplatZ(vForceSizeLife)));
		XMVECTOR vCrossNeg = XMVectorNegate(vCross);

		XMVECTOR vFront = XMVectorAdd(vPosition, vVelocity);
		XMVECTOR vBack = XMVectorSubtract(vPosition, vVelocity);

		XMVECTOR vPosition0 = XMVectorAdd(vBack, vCross);
		XMVECTOR vPosition1 = XMVectorAdd(vBack, vCrossNeg);
		XMVECTOR vPosition2 = XMVectorAdd(vFront, vCrossNeg);
		XMVECTOR vPosition3 = XMVectorAdd(vFront, vCross);

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
#endif

	return (void*)pVertices;
}

// uses cross products
void* Beam::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

#if defined(RENDER_DYNAMICEFFECT_USE_HVEC)
	Maths::HVec3 vViewDirectionUp(Maths::Load<Maths::HVec3>(&pConstants->vUp));
	Maths::HVec3 vViewDirectionRight(Maths::Load<Maths::HVec3>(&pConstants->vRight));
	Maths::HVec3 vViewPosition(Maths::Load<Maths::HVec3>(&pConstants->vPosition));
	
	Maths::HF32 vOne = Maths::Constants<Maths::HF32>::One();
	Maths::HF32 vTime(pConstants->fFrameTime);
	
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

			Maths::HVec3 vVelocity(Maths::Load<Maths::HVec3>(&pParticle->vVelocity));
			vVelocity = vVelocity * mLocal.Rotation();
				
			Maths::HVec3 vToView = vViewPosition - vPosition;
			Maths::HVec3 vCross = vToView ^ vVelocity;
			vCross.Normalise();

			vCross = vCross * vSize.X() * vForceSizeLife.Z();
			Maths::HVec3 vCrossNeg = -vCross;

			Maths::HF32 vVelocityModifier = vSize.Y() * vLife;
			vVelocity = vVelocity * vVelocityModifier;
			vVelocity = vVelocity * vTime;

			Maths::HVec3 vFront = vPosition + vVelocity;
			Maths::HVec3 vBack = vPosition - vVelocity;

			Maths::HVec3 vPosition0 = vBack + vCross;
			Maths::HVec3 vPosition1 = vBack + vCrossNeg;
			Maths::HVec3 vPosition2 = vFront + vCrossNeg;
			Maths::HVec3 vPosition3 = vFront + vCross;

			// 0
			Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m0];
			pVertices->v = uv0.v[map.m0];
			++pVertices;
			// 1
			Maths::Store<Maths::HVec3>(vPosition1, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m1];
			pVertices->v = uv0.v[map.m1];
			++pVertices;
			// 2
			Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m2];
			pVertices->v = uv0.v[map.m2];
			++pVertices;
		#ifndef PLATFORM_PC
			// 3
			Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m3];
			pVertices->v = uv0.v[map.m3];
			++pVertices;
		#else
			// 2
			Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m2];
			pVertices->v = uv0.v[map.m2];
			++pVertices;
			// 3
			Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u = uv0.u[map.m3];
			pVertices->v = uv0.v[map.m3];
			++pVertices;
			// 0
			Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
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
#endif

#if defined(RENDER_DYNAMICEFFECT_USE_XMVECTOR)
	XMVECTOR vViewPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vPosition);
	XMVECTOR vViewDirectionUp = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vUp);
	XMVECTOR vViewDirectionRight = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vRight);
	XMVECTOR vOne = XMVectorSplatOne();
	XMVECTOR vTime = XMVectorReplicate(pConstants->fFrameTime);
	C_TextureCoords& texCoords = C_TextureCoords::Instance();

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
			const C_Particle3D* pParticle = (const C_Particle3D*)&pBlock->m_Element[a];
			
			// calculate colour
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);	
			XMVECTOR vLife = XMVectorSplatW(vPosition);
			F32 fLife = pParticle->fLife;

			vPosition = XMVector3TransformCoord(vPosition, mLocal);
			
			// colour and size
			XMVECTOR vColour, vSize, vForceSizeLife;
			{
				vColour = LookupValue(pSettings->colourTable, vLife, fLife);
				vForceSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vForceSizeLife);
				XMVECTOR vLighting = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vColour);
				vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
				vColour = XMVectorMultiply(vColour, vLighting);
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

			XMVECTOR vVelocity = XMLoadFloat3A16((XMFLOAT3A16*)&pParticle->vVelocity);
				
			XMVECTOR vVelocityModifier = XMVectorMultiply(XMVectorSplatY(vSize), vLife);
			vVelocity = XMVectorMultiply(vVelocity, vVelocityModifier);
			vVelocity = XMVectorMultiply(vVelocity, vTime);
			
			XMVECTOR vToView = vViewPosition - vPosition;
			XMVECTOR vCross = XMVector3NormalizeEst(XMVector3Cross(vToView, vVelocity));

			vCross = XMVectorMultiply(vCross, XMVectorMultiply(XMVectorSplatX(vSize), XMVectorSplatZ(vForceSizeLife)));
			XMVECTOR vCrossNeg = XMVectorNegate(vCross);

			XMVECTOR vFront = XMVectorAdd(vPosition, vVelocity);
			XMVECTOR vBack = XMVectorSubtract(vPosition, vVelocity);

			XMVECTOR vPosition0 = XMVectorAdd(vBack, vCross);
			XMVECTOR vPosition1 = XMVectorAdd(vBack, vCrossNeg);
			XMVECTOR vPosition2 = XMVectorAdd(vFront, vCrossNeg);
			XMVECTOR vPosition3 = XMVectorAdd(vFront, vCross);

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
#endif

	#ifdef PLATFORM_SPU
	pVertices = pVerticesEA;
	buffer.Wait();
	#endif

	return (void*)pVertices;
}

}
} // ShapePolicy
} // Particles

