#include "stdafx.h"

#include "Dynamic/Particle/Shape/Cube.h"

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

#ifdef RENDER_DYNAMICEFFECT_USE_HVEC
void* Cube::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	Maths::HVec3 vVertex0(1.0f, 1.0f, 1.0f);
	Maths::HVec3 vVertex1(-1.0f, 1.0f, 1.0f);
	Maths::HVec3 vVertex2(-1.0f, -1.0f, 1.0f);
	Maths::HVec3 vVertex3(1.0f, -1.0f, 1.0f);
	Maths::HVec3 vVertex4(1.0f, 1.0f, -1.0f);
	Maths::HVec3 vVertex5(-1.0f, 1.0f, -1.0f);
	Maths::HVec3 vVertex6(-1.0f, -1.0f, -1.0f);
	Maths::HVec3 vVertex7(1.0f, -1.0f, -1.0f);
	
	Maths::HF32 vOne = 1.0f;
	

	U32 indices[] = {	0, 1, 2, 3,
						4, 5, 1, 0,
						7, 6, 5, 4,
						3, 2, 6, 7,
						3, 7, 4, 0,
						5, 6, 2, 1 };
	
	#ifdef PLATFORM_SPU
	// allocate memory for double buffered temporary storage of particles, and store pointer to main memory
	ParticleVertexPtr pVerticesEA = pVertices;
	C_SpuDoubleBuffer<ParticleVertex> buffer;
	buffer.Alloc(64 * 24 * sizeof(ParticleVertex));
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

			Maths::C_Quaternion qRotation(Maths::Load<Maths::HVec4>(&pParticle->qRotation));
			Maths::HMat33 mRotate(qRotation);

			mRotate = mRotate * mLocal.Rotation();

			Maths::HVec3 vTransformedVertex0((vVertex0 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex1((vVertex1 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex2((vVertex2 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex3((vVertex3 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex4((vVertex4 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex5((vVertex5 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex6((vVertex6 * vSize) * mRotate + vPosition);
			Maths::HVec3 vTransformedVertex7((vVertex7 * vSize) * mRotate + vPosition);

			// store the final positions into aligned stack memory
			MathsStorage::Float4A16 vFinalPosition[8];
			Maths::Store<Maths::HVec3>(vTransformedVertex0, &vFinalPosition[0]);
			Maths::Store<Maths::HVec3>(vTransformedVertex1, &vFinalPosition[1]);
			Maths::Store<Maths::HVec3>(vTransformedVertex2, &vFinalPosition[2]);
			Maths::Store<Maths::HVec3>(vTransformedVertex3, &vFinalPosition[3]);
			Maths::Store<Maths::HVec3>(vTransformedVertex4, &vFinalPosition[4]);
			Maths::Store<Maths::HVec3>(vTransformedVertex5, &vFinalPosition[5]);
			Maths::Store<Maths::HVec3>(vTransformedVertex6, &vFinalPosition[6]);
			Maths::Store<Maths::HVec3>(vTransformedVertex7, &vFinalPosition[7]);
			
			for(S32 i = 0; i < 24; i+=4)
			{
				// reload appropriate vectors
				Maths::HVec3 vFinalPosition0(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i]]));
				Maths::HVec3 vFinalPosition1(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i+1]]));
				Maths::HVec3 vFinalPosition2(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i+2]]));
				Maths::HVec3 vFinalPosition3(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i+3]]));

				Maths::HVec3 v0 = vFinalPosition3 - vFinalPosition0;
				Maths::HVec3 v1 = vFinalPosition1 - vFinalPosition0;
				Maths::HVec3 vNormal = v0 ^ v1;
				vNormal.Normalise();
				
				// 0
				Maths::Store<Maths::HVec3>(vFinalPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[0];
				pVertices->v = uv0.v[0];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 1
				Maths::Store<Maths::HVec3>(vFinalPosition1, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[1];
				pVertices->v = uv0.v[1];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 2
				Maths::Store<Maths::HVec3>(vFinalPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[2];
				pVertices->v = uv0.v[2];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
	#ifndef PLATFORM_PC
				// 3
				Maths::Store<Maths::HVec3>(vFinalPosition3, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[3];
				pVertices->v = uv0.v[3];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;	
	#else
				// 2
				Maths::Store<Maths::HVec3>(vFinalPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[2];
				pVertices->v = uv0.v[2];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 3
				Maths::Store<Maths::HVec3>(vFinalPosition3, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[3];
				pVertices->v = uv0.v[3];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 0
				Maths::Store<Maths::HVec3>(vFinalPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uv0.u[0];
				pVertices->v = uv0.v[0];
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
	#endif
			}
		}

		#ifdef PLATFORM_SPU
		// store the blocks worth of particles to system memory
		Spu::StoreAsync((void*)pVerticesEA, (void*)buffer.GetBuffer(), sizeof(ParticleVertex) * pBlock->m_Element.Size() * 24, buffer.GetIndex());
		pVerticesEA += pBlock->m_Element.Size() * 24;
		buffer.Swap();
		#endif
	}

	#ifdef PLATFORM_SPU
	buffer.Wait();
	#endif

	return (void*)pVertices;
}
#endif

//
//	XMVECTOR version
//

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
void* Cube::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	XMVECTOR vVertices[8];
	vVertices[0] = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	vVertices[1] = XMVectorSet(-1.0f, 1.0f, 1.0f, 1.0f);
	vVertices[2] = XMVectorSet(-1.0f, -1.0f, 1.0f, 1.0f);
	vVertices[3] = XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f);

	vVertices[4] = XMVectorSet(1.0f, 1.0f, -1.0f, 1.0f);
	vVertices[5] = XMVectorSet(-1.0f, 1.0f, -1.0f, 1.0f);
	vVertices[6] = XMVectorSet(-1.0f, -1.0f, -1.0f, 1.0f);
	vVertices[7] = XMVectorSet(1.0f, -1.0f, -1.0f, 1.0f);

	U32 indices[] = {	0, 1, 2, 3,
						4, 5, 1, 0,
						7, 6, 5, 4,
						3, 2, 6, 7,
						3, 7, 4, 0,
						5, 6, 2, 1 };

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

			const DynamicEffect::TextureCoordsMapping& map = pConstants->pTexCoords->GetTextureCoordinatesMapping(pParticle->iUV);
			DynamicEffect::TextureCoords<F32> uv0 = pConstants->pTexCoords->GetTextureCoordinates<F32>(pParticle->iVariant);
			
			XMMATRIX mRotate = XMMatrixRotationQuaternion(qRotation);
			mRotate = XMMatrixMultiply(mRotate, mLocal);

			XMVECTOR vTransformedVertices[8];

			vTransformedVertices[0] = XMVectorMultiply(vVertices[0], vSize);
			vTransformedVertices[1] = XMVectorMultiply(vVertices[1], vSize);
			vTransformedVertices[2] = XMVectorMultiply(vVertices[2], vSize);
			vTransformedVertices[3] = XMVectorMultiply(vVertices[3], vSize);
			vTransformedVertices[4] = XMVectorMultiply(vVertices[4], vSize);
			vTransformedVertices[5] = XMVectorMultiply(vVertices[5], vSize);
			vTransformedVertices[6] = XMVectorMultiply(vVertices[6], vSize);
			vTransformedVertices[7] = XMVectorMultiply(vVertices[7], vSize);

			vTransformedVertices[0] = XMVector3TransformNormal(vTransformedVertices[0], mRotate);
			vTransformedVertices[1] = XMVector3TransformNormal(vTransformedVertices[1], mRotate);
			vTransformedVertices[2] = XMVector3TransformNormal(vTransformedVertices[2], mRotate);
			vTransformedVertices[3] = XMVector3TransformNormal(vTransformedVertices[3], mRotate);
			vTransformedVertices[4] = XMVector3TransformNormal(vTransformedVertices[4], mRotate);
			vTransformedVertices[5] = XMVector3TransformNormal(vTransformedVertices[5], mRotate);
			vTransformedVertices[6] = XMVector3TransformNormal(vTransformedVertices[6], mRotate);
			vTransformedVertices[7] = XMVector3TransformNormal(vTransformedVertices[7], mRotate);

			XMVECTOR vFinalPosition[8];
			vFinalPosition[0] = XMVectorAdd(vTransformedVertices[0], vPosition);
			vFinalPosition[1] = XMVectorAdd(vTransformedVertices[1], vPosition);
			vFinalPosition[2] = XMVectorAdd(vTransformedVertices[2], vPosition);
			vFinalPosition[3] = XMVectorAdd(vTransformedVertices[3], vPosition);
			vFinalPosition[4] = XMVectorAdd(vTransformedVertices[4], vPosition);
			vFinalPosition[5] = XMVectorAdd(vTransformedVertices[5], vPosition);
			vFinalPosition[6] = XMVectorAdd(vTransformedVertices[6], vPosition);
			vFinalPosition[7] = XMVectorAdd(vTransformedVertices[7], vPosition);
			
			for(S32 i = 0; i < 24; i+=4)
			{
				XMVECTOR v0 = XMVectorSubtract(vFinalPosition[indices[i+3]], vFinalPosition[indices[i]]);
				XMVECTOR v1 = XMVectorSubtract(vFinalPosition[indices[i+1]], vFinalPosition[indices[i]]);
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
	#ifndef PLATFORM_PC
				// 3
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition[indices[i+3]]);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				pVertices->u = uv0.u[map.m3];
				pVertices->v = uv0.v[map.m3];
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
				++pVertices;	
	#else
				// 2
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition[indices[i+2]]);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				pVertices->u = uv0.u[map.m2];
				pVertices->v = uv0.v[map.m2];
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
				++pVertices;
				// 3
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition[indices[i+3]]);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				pVertices->u = uv0.u[map.m3];
				pVertices->v = uv0.v[map.m3];
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
				++pVertices;
				// 0
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition[indices[i]]);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				pVertices->u = uv0.u[map.m0];
				pVertices->v = uv0.v[map.m0];
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
				++pVertices;
	#endif
			}
		}
	}
	return (void*)pVertices;
}
#endif

}
} // ShapePolicy
} // Particles

