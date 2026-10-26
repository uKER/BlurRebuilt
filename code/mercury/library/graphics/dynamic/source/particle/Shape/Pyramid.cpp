#include "stdafx.h"

#include "Dynamic/Particle/Shape/Pyramid.h"

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
void* Pyramid::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	Maths::HVec3 vVertex0(0.0f, 1.0f, 0.0f);
	Maths::HVec3 vVertex1(0.0f, -1.0f, 1.0f);
	Maths::HVec3 vVertex2(0.866f, -1.0f, -0.5f);
	Maths::HVec3 vVertex3(-0.866f, -1.0f, -0.5f);

	U32 indices[] = {	0, 1, 2,
						0, 2, 3,
						0, 3, 1,
						1, 3, 2 };
	
	Maths::HF32 vOne = 1.0f;
	
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

			DynamicEffect::TextureCoords<F32> uv0 = pConstants->pTexCoords->GetTextureCoordinates<F32>(pParticle->iVariant);
			const DynamicEffect::TextureCoordsMapping& map = pConstants->pTexCoords->GetTextureCoordinatesMapping(pParticle->iUV);
			
			Maths::C_Quaternion qRotation(Maths::Load<Maths::HVec4>(&pParticle->qRotation));
			Maths::HMat33 mRotate(qRotation);

			mRotate = mRotate * mLocal.Rotation();

			Maths::HVec3 vTransformedVertex0((vVertex0 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex1((vVertex1 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex2((vVertex2 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex3((vVertex3 * vSize) * mRotate + vPosition);
			
			// store the final positions into aligned stack memory
			MathsStorage::Float4A16 vFinalPosition[4];
			Maths::Store<Maths::HVec3>(vTransformedVertex0, &vFinalPosition[0]);
			Maths::Store<Maths::HVec3>(vTransformedVertex1, &vFinalPosition[1]);
			Maths::Store<Maths::HVec3>(vTransformedVertex2, &vFinalPosition[2]);
			Maths::Store<Maths::HVec3>(vTransformedVertex3, &vFinalPosition[3]);
			
			for(S32 i = 0; i < 12; i+=3)
			{
				// reload appropriate vectors
				Maths::HVec3 vFinalPosition0(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i]]));
				Maths::HVec3 vFinalPosition1(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i+1]]));
				Maths::HVec3 vFinalPosition2(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i+2]]));
				
				Maths::HVec3 v0 = vFinalPosition2 - vFinalPosition0;
				Maths::HVec3 v1 = vFinalPosition1 - vFinalPosition0;
				Maths::HVec3 vNormal = v0 ^ v1;
				vNormal.Normalise();
				
				// 0
				Maths::Store<Maths::HVec3>(vFinalPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[map.m0];
				pVertices->v = uv0.v[map.m0];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 1
				Maths::Store<Maths::HVec3>(vFinalPosition1, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[map.m1];
				pVertices->v = uv0.v[map.m1];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 2
				Maths::Store<Maths::HVec3>(vFinalPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[map.m2];
				pVertices->v = uv0.v[map.m2];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
			}
		}

		#ifdef PLATFORM_SPU
		// store the blocks worth of particles to system memory
		Spu::StoreAsync((void*)pVerticesEA, (void*)buffer.GetBuffer(), sizeof(ParticleVertex) * pBlock->m_Element.Size() * 12, buffer.GetIndex());
		pVerticesEA += pBlock->m_Element.Size() * 12;
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
void* Pyramid::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	XMVECTOR vVertices[8];
	vVertices[0] = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	vVertices[1] = XMVectorSet(0.0f, -1.0f, 1.0f, 1.0f);
	vVertices[2] = XMVectorSet(0.866f, -1.0f, -0.5f, 1.0f);
	vVertices[3] = XMVectorSet(-0.866f, -1.0f, -0.5f, 1.0f);

	U32 indices[] = {	0, 1, 2,
						0, 2, 3,
						0, 3, 1,
						1, 3, 2 };

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
	
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);
			XMVECTOR qRotation = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->qRotation);

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
			vSize = XMVectorMultiply(vSize, XMVectorSplatZ(vForceSizeLife));
			vColour = XMVectorMultiply(vColour, vLighting);
			}
			DynamicEffect::TextureCoords<F32> uv0 = pConstants->pTexCoords->GetTextureCoordinates<F32>(pParticle->iVariant);
			const DynamicEffect::TextureCoordsMapping& map = pConstants->pTexCoords->GetTextureCoordinatesMapping(pParticle->iUV);
			
			XMMATRIX mRotate = XMMatrixRotationQuaternion(qRotation);
			mRotate = XMMatrixMultiply(mRotate, mLocal);

			XMVECTOR vTransformedVertices[4];

			vTransformedVertices[0] = XMVector3TransformNormal(XMVectorMultiply(vVertices[0], vSize), mRotate);
			vTransformedVertices[1] = XMVector3TransformNormal(XMVectorMultiply(vVertices[1], vSize), mRotate);
			vTransformedVertices[2] = XMVector3TransformNormal(XMVectorMultiply(vVertices[2], vSize), mRotate);
			vTransformedVertices[3] = XMVector3TransformNormal(XMVectorMultiply(vVertices[3], vSize), mRotate);

			XMVECTOR vFinalPosition[8];
			vFinalPosition[0] = XMVectorAdd(vTransformedVertices[0], vPosition);
			vFinalPosition[1] = XMVectorAdd(vTransformedVertices[1], vPosition);
			vFinalPosition[2] = XMVectorAdd(vTransformedVertices[2], vPosition);
			vFinalPosition[3] = XMVectorAdd(vTransformedVertices[3], vPosition);
			
			for(S32 i = 0; i < 12; i+=3)
			{
				XMVECTOR v0 = XMVectorSubtract(vFinalPosition[indices[i+1]], vFinalPosition[indices[i]]);
				XMVECTOR v1 = XMVectorSubtract(vFinalPosition[indices[i+2]], vFinalPosition[indices[i]]);
				XMVECTOR vNormal = XMVector3Cross(v0, v1);
				vNormal = XMVector3NormalizeEst(vNormal);
				
				// 0
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition[indices[i]]);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				pVertices->u = uv0.u[map.m0];
				pVertices->v = uv0.v[map.m0];
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
				++pVertices;
				// 1
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition[indices[i+1]]);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				pVertices->u = uv0.u[map.m1];
				pVertices->v = uv0.v[map.m1];
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
				++pVertices;
				// 2
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition[indices[i+2]]);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				pVertices->u = uv0.u[map.m2];
				pVertices->v = uv0.v[map.m2];
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
				++pVertices;
			}
		}
	}
	return (void*)pVertices;
}
#endif

}
} // ShapePolicy
} // Particles

