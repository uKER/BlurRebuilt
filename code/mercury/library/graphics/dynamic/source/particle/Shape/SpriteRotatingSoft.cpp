#include "stdafx.h"

#include "Dynamic/Particle/Shape/SpriteRotatingSoft.h"

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

#ifndef PLATFORM_SPU
E_VertexShader SpriteRotatingSoft::GetVertexShader(const I_ParticleEffectBase* pEffect)
{

	#ifndef CFG_SPEED
	if(pEffect->GetManager()->UseSoft())
		return E_VertexShader_Soft;
	else
		return E_VertexShader_UVW;
	#else
		return E_VertexShader_Soft;
	#endif
}
#endif

#ifdef RENDER_DYNAMICEFFECT_USE_HVEC
void* SpriteRotatingSoft::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;
	
	Maths::HVec3 vUp0(Maths::Load<Maths::HVec3>(&pConstants->vUp));
	Maths::HVec3 vRight0(Maths::Load<Maths::HVec3>(&pConstants->vRight));
	Maths::HVec3 vForward(Maths::Load<Maths::HVec3>(&pConstants->vForward));
	Maths::HVec3 vPivot(Maths::Load<Maths::HVec3>(&pSettings->pivot));
	Maths::HVec3 vDown0 = -vUp0;
	Maths::HVec3 vLeft0 = -vRight0;
	vPivot = vPivot.X() * vRight0 + vPivot.Y() * vUp0;
	vUp0 += vPivot;
	vRight0 += vPivot;
	vDown0 += vPivot;
	vLeft0 += vPivot;
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
			const C_Particle3D_R1D* pParticle = (const C_Particle3D_R1D*)&pBlock->m_Element[a];
			
			Maths::HVec4 vPositionLife(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pParticle->vPosition));	
			Maths::HVec3 vPosition(vPositionLife);
			vPosition = vPosition * mLocal;

			Maths::HF32 vLife = vPositionLife.W();
			F32 fLife = pParticle->fLife;

			Maths::HVec4 vColourCurve = LookupValue(pSettings->colourTable, vLife, fLife);
			Maths::HVec4 vForceSizeLife(Maths::Load<Maths::HVec4>(&pParticle->vForceSizeLife));
			Maths::HVec4 vLighting(Maths::Load<Maths::HVec4>(&pParticle->vColour));
			
			Maths::HVec4 vSizeCurve = LookupValue(pSettings->sizeTable, vLife, fLife);
			Maths::HF32 vAnim = vSizeCurve.W();
			vSizeCurve = vSizeCurve * vForceSizeLife.Z();
			Maths::HF32 vSizeX = vSizeCurve.X();
			Maths::HF32 vSizeY = vSizeCurve.Y();

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

			Maths::C_Quaternion qRotate(vForward, pParticle->vRotation.x);
			Maths::HMat33 mRotate(qRotate);
			
			Maths::HVec3 vUp = vUp0 * mRotate;
			Maths::HVec3 vDown = vDown0 * mRotate;
			Maths::HVec3 vLeft = vLeft0 * mRotate;
			Maths::HVec3 vRight = vRight0 * mRotate;
			
			// top left
			Maths::HVec3 vPosition0 = vUp * vSizeY + vLeft * vSizeX + vPosition;
			// top right
			Maths::HVec3 vPosition1 = vUp * vSizeY + vRight * vSizeX + vPosition;
			// bottom right
			Maths::HVec3 vPosition2 = vDown * vSizeY + vRight * vSizeX + vPosition;
			// bottom left
			Maths::HVec3 vPosition3 = vDown * vSizeY + vLeft * vSizeX + vPosition;
						
			// 0
			Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u0 = uv0.u[map.m0];
			pVertices->v0 = uv0.v[map.m0];
			pVertices->u1 = vAnim;
			pVertices->v1 = vSizeX;
			++pVertices;
			// 1
			Maths::Store<Maths::HVec3>(vPosition1, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u0 = uv0.u[map.m1];
			pVertices->v0 = uv0.v[map.m1];
			pVertices->u1 = vAnim;
			pVertices->v1 = vSizeX;
			++pVertices;
			// 2
			Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u0 = uv0.u[map.m2];
			pVertices->v0 = uv0.v[map.m2];
			pVertices->u1 = vAnim;
			pVertices->v1 = vSizeX;
			++pVertices;
#ifndef PLATFORM_PC
			// 3
			Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u0 = uv0.u[map.m3];
			pVertices->v0 = uv0.v[map.m3];
			pVertices->u1 = vAnim;
			pVertices->v1 = vSizeX;
			++pVertices;	
#else
			// 2
			Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u0 = uv0.u[map.m2];
			pVertices->v0 = uv0.v[map.m2];
			pVertices->u1 = vAnim;
			pVertices->v1 = vSizeX;
			++pVertices;
			// 3
			Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u0 = uv0.u[map.m3];
			pVertices->v0 = uv0.v[map.m3];
			pVertices->u1 = vAnim;
			pVertices->v1 = vSizeX;
			++pVertices;
			// 0
			Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
			Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
			pVertices->u0 = uv0.u[map.m0];
			pVertices->v0 = uv0.v[map.m0];
			pVertices->u1 = vAnim;
			pVertices->v1 = vSizeX;
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

void* SpriteRotatingSoft::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	Maths::HVec3 vUp0(Maths::Load<Maths::HVec3>(&pConstants->vUp));
	Maths::HVec3 vRight0(Maths::Load<Maths::HVec3>(&pConstants->vRight));
	Maths::HVec3 vForward(Maths::Load<Maths::HVec3>(&pConstants->vForward));
	Maths::HVec3 vPivot(Maths::Load<Maths::HVec3>(&pSettings->pivot));
	Maths::HVec3 vDown0 = -vUp0;
	Maths::HVec3 vLeft0 = -vRight0;
	vPivot = vPivot.X() * vRight0 + vPivot.Y() * vUp0;
	vUp0 += vPivot;
	vRight0 += vPivot;
	vDown0 += vPivot;
	vLeft0 += vPivot;
	Maths::HF32 vOne = 1.0f;
	
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
		
		const C_Particle3D_R1D* pParticle = (const C_Particle3D_R1D*)sortedList[a].pParticle;
			
		Maths::HVec4 vPositionLife(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pParticle->vPosition));	
		Maths::HVec3 vPosition(vPositionLife);
		//vPosition = vPosition * mLocal;

		Maths::HF32 vLife = vPositionLife.W();
		F32 fLife = pParticle->fLife;

		Maths::HVec4 vColourCurve = LookupValue(pSettings->colourTable, vLife, fLife);
		Maths::HVec4 vForceSizeLife(Maths::Load<Maths::HVec4>(&pParticle->vForceSizeLife));
		Maths::HVec4 vLighting(Maths::Load<Maths::HVec4>(&pParticle->vColour));
		
		Maths::HVec4 vSizeCurve = LookupValue(pSettings->sizeTable, vLife, fLife);
		Maths::HF32 vAnim = vSizeCurve.W();
		vSizeCurve = vSizeCurve * vForceSizeLife.Z();
		Maths::HF32 vSizeX = vSizeCurve.X();
		Maths::HF32 vSizeY = vSizeCurve.Y();

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
		Maths::C_Quaternion qRotate(vForward, pParticle->vRotation.x);
		Maths::HMat33 mRotate(qRotate);
		
		Maths::HVec3 vUp = vUp0 * mRotate;
		Maths::HVec3 vDown = vDown0 * mRotate;
		Maths::HVec3 vLeft = vLeft0 * mRotate;
		Maths::HVec3 vRight = vRight0 * mRotate;
		
		// top left
		Maths::HVec3 vPosition0 = vUp * vSizeY + vLeft * vSizeX + vPosition;
		// top right
		Maths::HVec3 vPosition1 = vUp * vSizeY + vRight * vSizeX + vPosition;
		// bottom right
		Maths::HVec3 vPosition2 = vDown * vSizeY + vRight * vSizeX + vPosition;
		// bottom left
		Maths::HVec3 vPosition3 = vDown * vSizeY + vLeft * vSizeX + vPosition;

		#ifdef PLATFORM_SPU
		if(batch == 0)
			buffer.Wait();
		#endif
					
		// 0
		Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u0 = uv0.u[map.m0];
		pVertices->v0 = uv0.v[map.m0];
		pVertices->u1 = vAnim;
		pVertices->v1 = vSizeX;
		++pVertices;
		// 1
		Maths::Store<Maths::HVec3>(vPosition1, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u0 = uv0.u[map.m1];
		pVertices->v0 = uv0.v[map.m1];
		pVertices->u1 = vAnim;
		pVertices->v1 = vSizeX;
		++pVertices;
		// 2
		Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u0 = uv0.u[map.m2];
		pVertices->v0 = uv0.v[map.m2];
		pVertices->u1 = vAnim;
		pVertices->v1 = vSizeX;
		++pVertices;
#ifndef PLATFORM_PC
		// 3
		Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u0 = uv0.u[map.m3];
		pVertices->v0 = uv0.v[map.m3];
		pVertices->u1 = vAnim;
		pVertices->v1 = vSizeX;
		++pVertices;	
#else
		// 2
		Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u0 = uv0.u[map.m2];
		pVertices->v0 = uv0.v[map.m2];
		pVertices->u1 = vAnim;
		pVertices->v1 = vSizeX;
		++pVertices;
		// 3
		Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u0 = uv0.u[map.m3];
		pVertices->v0 = uv0.v[map.m3];
		pVertices->u1 = vAnim;
		pVertices->v1 = vSizeX;
		++pVertices;
		// 0
		Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
		Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
		pVertices->u0 = uv0.u[map.m0];
		pVertices->v0 = uv0.v[map.m0];
		pVertices->u1 = vAnim;
		pVertices->v1 = vSizeX;
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

	return (void*)pVertices;
}
#endif

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
void* SpriteRotatingSoft::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	XMVECTOR vUp0 = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vUp);
	XMVECTOR vRight0 = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vRight);
	XMVECTOR vForward = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vForward);
	XMVECTOR vPivot = XMLoadFloat4A16((XMFLOAT4A16*)&pSettings->pivot);
	XMVECTOR vDown0 = XMVectorNegate(vUp0);
	XMVECTOR vLeft0 = XMVectorNegate(vRight0);
	vPivot = XMVectorMultiplyAdd(XMVectorSplatX(vPivot), vRight0, XMVectorMultiply(XMVectorSplatY(vPivot), vUp0));
	vUp0 = XMVectorAdd(vUp0, vPivot);
	vDown0 = XMVectorAdd(vDown0, vPivot);
	vRight0 = XMVectorAdd(vRight0, vPivot);
	vLeft0 = XMVectorAdd(vLeft0, vPivot);
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
			const C_Particle3D_R1D* pParticle = (C_Particle3D_R1D*)&pBlock->m_Element[a];
	
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);
			XMVECTOR vLife = XMVectorSplatW(vPosition);
			F32 fLife = pParticle->fLife;

			vPosition = XMVector3TransformCoord(vPosition, mLocal);
		
			// colour and size
			XMVECTOR vColour, vSizeX, vSizeY, vAnim;
			{
				vColour = LookupValue(pSettings->colourTable, vLife, fLife);
				XMVECTOR vForceSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vForceSizeLife);
				XMVECTOR vLighting = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vColour);
				XMVECTOR vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
				vAnim = XMVectorSplatW(vSize);
				vSize = XMVectorMultiply(vSize, XMVectorSplatZ(vForceSizeLife));
				vSizeX = XMVectorSplatX(vSize);
				vSizeY = XMVectorSplatY(vSize);
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

			XMVECTOR vUp, vDown, vLeft, vRight;
			XMMATRIX mRotate = XMMatrixRotationNormal(vForward, -pParticle->vRotation.x);

			vUp = XMVector3TransformNormal(vUp0, mRotate);
			vDown = XMVector3TransformNormal(vDown0, mRotate);
			vLeft = XMVector3TransformNormal(vLeft0, mRotate);
			vRight = XMVector3TransformNormal(vRight0, mRotate);

			// top left
			XMVECTOR vPosition0 = XMVectorMultiplyAdd(vUp, vSizeY, XMVectorMultiplyAdd(vLeft, vSizeX, vPosition));
			// top right
			XMVECTOR vPosition1 = XMVectorMultiplyAdd(vUp, vSizeY, XMVectorMultiplyAdd(vRight, vSizeX, vPosition));
			// bottom right
			XMVECTOR vPosition2 = XMVectorMultiplyAdd(vDown, vSizeY, XMVectorMultiplyAdd(vRight, vSizeX, vPosition));
			// bottom left
			XMVECTOR vPosition3 = XMVectorMultiplyAdd(vDown, vSizeY, XMVectorMultiplyAdd(vLeft, vSizeX, vPosition));
						
			// bottom left
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u0 = uv0.u[map.m0];
			pVertices->v0 = uv0.v[map.m0];
			XMStoreScalar((void*)&pVertices->u1, vAnim);
			XMStoreScalar((void*)&pVertices->v1, vSizeX);
			++pVertices;
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition1);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u0 = uv0.u[map.m1];
			pVertices->v0 = uv0.v[map.m1];
			XMStoreScalar((void*)&pVertices->u1, vAnim);
			XMStoreScalar((void*)&pVertices->v1, vSizeX);
			++pVertices;
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u0 = uv0.u[map.m2];
			pVertices->v0 = uv0.v[map.m2];
			XMStoreScalar((void*)&pVertices->u1, vAnim);
			XMStoreScalar((void*)&pVertices->v1, vSizeX);
			++pVertices;
#ifndef PLATFORM_PC
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u0 = uv0.u[map.m3];
			pVertices->v0 = uv0.v[map.m3];
			XMStoreScalar((void*)&pVertices->u1, vAnim);
			XMStoreScalar((void*)&pVertices->v1, vSizeX);
			++pVertices;
#else
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u0 = uv0.u[map.m2];
			pVertices->v0 = uv0.v[map.m2];
			XMStoreScalar((void*)&pVertices->u1, vAnim);
			XMStoreScalar((void*)&pVertices->v1, vSizeX);
			++pVertices;
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u0 = uv0.u[map.m3];
			pVertices->v0 = uv0.v[map.m3];
			XMStoreScalar((void*)&pVertices->u1, vAnim);
			XMStoreScalar((void*)&pVertices->v1, vSizeX);
			++pVertices;
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u0 = uv0.u[map.m0];
			pVertices->v0 = uv0.v[map.m0];
			XMStoreScalar((void*)&pVertices->u1, vAnim);
			XMStoreScalar((void*)&pVertices->v1, vSizeX);
			++pVertices;
#endif
		}
	}
	return (void*)pVertices;
}

void* SpriteRotatingSoft::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	XMVECTOR vUp0 = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vUp);
	XMVECTOR vRight0 = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vRight);
	XMVECTOR vForward = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vForward);
	XMVECTOR vPivot = XMLoadFloat4A16((XMFLOAT4A16*)&pSettings->pivot);
	XMVECTOR vDown0 = XMVectorNegate(vUp0);
	XMVECTOR vLeft0 = XMVectorNegate(vRight0);
	vPivot = XMVectorMultiplyAdd(XMVectorSplatX(vPivot), vRight0, XMVectorMultiply(XMVectorSplatY(vPivot), vUp0));
	vUp0 = XMVectorAdd(vUp0, vPivot);
	vDown0 = XMVectorAdd(vDown0, vPivot);
	vRight0 = XMVectorAdd(vRight0, vPivot);
	vLeft0 = XMVectorAdd(vLeft0, vPivot);
	XMVECTOR vOne = XMVectorSplatOne();
	

	for(S32 a = 0; a < sortedList.Size(); ++a)
	{
		const C_Particle3D_R1D* pParticle = (C_Particle3D_R1D*)sortedList[a].pParticle;

		XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);
		XMVECTOR vLife = XMVectorSplatW(vPosition);
		F32 fLife = pParticle->fLife;

		//vPosition = XMVector3TransformCoord(vPosition, mLocal);
	
		// colour and size
		XMVECTOR vColour, vSizeX, vSizeY, vAnim;
		{
			vColour = LookupValue(pSettings->colourTable, vLife, fLife);
			XMVECTOR vForceSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vForceSizeLife);
			XMVECTOR vLighting = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vColour);
			XMVECTOR vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
			vAnim = XMVectorSplatW(vSize);
			vSize = XMVectorMultiply(vSize, XMVectorSplatZ(vForceSizeLife));
			vSizeX = XMVectorSplatX(vSize);
			vSizeY = XMVectorSplatY(vSize);
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

		XMVECTOR vUp, vDown, vLeft, vRight;
		XMMATRIX mRotate = XMMatrixRotationNormal(vForward, -pParticle->vRotation.x);

		vUp = XMVector3TransformNormal(vUp0, mRotate);
		vDown = XMVector3TransformNormal(vDown0, mRotate);
		vLeft = XMVector3TransformNormal(vLeft0, mRotate);
		vRight = XMVector3TransformNormal(vRight0, mRotate);

		// top left
		XMVECTOR vPosition0 = XMVectorMultiplyAdd(vUp, vSizeY, XMVectorMultiplyAdd(vLeft, vSizeX, vPosition));
		// top right
		XMVECTOR vPosition1 = XMVectorMultiplyAdd(vUp, vSizeY, XMVectorMultiplyAdd(vRight, vSizeX, vPosition));
		// bottom right
		XMVECTOR vPosition2 = XMVectorMultiplyAdd(vDown, vSizeY, XMVectorMultiplyAdd(vRight, vSizeX, vPosition));
		// bottom left
		XMVECTOR vPosition3 = XMVectorMultiplyAdd(vDown, vSizeY, XMVectorMultiplyAdd(vLeft, vSizeX, vPosition));
					
		// bottom left
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		pVertices->u0 = uv0.u[map.m0];
		pVertices->v0 = uv0.v[map.m0];
		XMStoreScalar((void*)&pVertices->u1, vAnim);
		XMStoreScalar((void*)&pVertices->v1, vSizeX);
		++pVertices;
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition1);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		pVertices->u0 = uv0.u[map.m1];
		pVertices->v0 = uv0.v[map.m1];
		XMStoreScalar((void*)&pVertices->u1, vAnim);
		XMStoreScalar((void*)&pVertices->v1, vSizeX);
		++pVertices;
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		pVertices->u0 = uv0.u[map.m2];
		pVertices->v0 = uv0.v[map.m2];
		XMStoreScalar((void*)&pVertices->u1, vAnim);
		XMStoreScalar((void*)&pVertices->v1, vSizeX);
		++pVertices;
#ifndef PLATFORM_PC
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		pVertices->u0 = uv0.u[map.m3];
		pVertices->v0 = uv0.v[map.m3];
		XMStoreScalar((void*)&pVertices->u1, vAnim);
		XMStoreScalar((void*)&pVertices->v1, vSizeX);
		++pVertices;
#else
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		pVertices->u0 = uv0.u[map.m2];
		pVertices->v0 = uv0.v[map.m2];
		XMStoreScalar((void*)&pVertices->u1, vAnim);
		XMStoreScalar((void*)&pVertices->v1, vSizeX);
		++pVertices;
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		pVertices->u0 = uv0.u[map.m3];
		pVertices->v0 = uv0.v[map.m3];
		XMStoreScalar((void*)&pVertices->u1, vAnim);
		XMStoreScalar((void*)&pVertices->v1, vSizeX);
		++pVertices;
		XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
		XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
		pVertices->u0 = uv0.u[map.m0];
		pVertices->v0 = uv0.v[map.m0];
		XMStoreScalar((void*)&pVertices->u1, vAnim);
		XMStoreScalar((void*)&pVertices->v1, vSizeX);
		++pVertices;
#endif
	}
	return (void*)pVertices;
}
#endif

}
} // ShapePolicy
} // Particles

