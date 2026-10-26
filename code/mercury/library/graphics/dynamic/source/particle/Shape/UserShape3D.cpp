#include "stdafx.h"

#include "Dynamic/Particle/Shape/UserShape3D.h"

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

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
void* UserShape3D::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	//This check should be removed in the final version of the game and replaced with an assertion.
	assert(pSettings->iParticleLibraryIndex != -1);

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

			//Find the particular variant we wish to use.
			const C_UserShape3DData::VertexArray& shapeDataVertexArray = 
				pSettings->pUserShape3DVariants->GetShape3DArray()[pParticle->iVariant].GetVertexArray();
	
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);
			XMVECTOR qRotation = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->qRotation);
			XMVECTOR vLife = XMVectorSplatW(vPosition);
			F32 fLife = pParticle->fLife;

			vPosition = XMVector3TransformCoord(vPosition, mLocal);
			// colour and size
			XMVECTOR vColour, vSize;
			{
				vColour = LookupValue(pSettings->colourTable, vLife, fLife);
				XMVECTOR vSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vForceSizeLife);
				XMVECTOR vLighting = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vColour);
				vSize = LookupValue(pSettings->sizeTable, vLife, fLife);
				vSize = XMVectorMultiply(vSize, XMVectorSplatZ(vSizeLife));
				vColour = XMVectorMultiply(vColour, vLighting);
			}

			XMMATRIX mRotate = XMMatrixRotationQuaternion(qRotation);
			mRotate = XMMatrixMultiply(mRotate, mLocal);

			const C_UserShape3DData::C_Vertex* pUserVertex = &shapeDataVertexArray[0];

			XMVECTOR vFinalPosition0;
			XMVECTOR vFinalPosition1;
			XMVECTOR vFinalPosition2;
			XMVECTOR vFinalNormal0;
			XMVECTOR vFinalNormal1;
			XMVECTOR vFinalNormal2;
			XMVECTOR uvFinal0;
			XMVECTOR uvFinal1;
			XMVECTOR uvFinal2;
			U32 nNumPrimitives = pSettings->pUserShape3DVariants->GetNumPrimitives();
			for(U32 prim = 0; prim < nNumPrimitives; prim++)
			{
				{
					//Convert untransformed vertex from storage type.
					XMVECTOR vtxPos;
					vtxPos = XMLoadFloat3((XMFLOAT3*)&pUserVertex->position);
					XMVECTOR vtxNorm;
					vtxNorm = XMLoadFloat3((XMFLOAT3*)&pUserVertex->normal);
					//Transform vertex.
					XMVECTOR vtxPosTransformed = XMVectorMultiply(vtxPos, vSize);
					vtxPosTransformed = XMVector3TransformNormal(vtxPosTransformed, mRotate);
					XMVECTOR vtxNormTransformed = XMVector3TransformNormal(vtxNorm, mRotate);

					vFinalPosition0 = XMVectorAdd(vtxPosTransformed, vPosition);
					vFinalNormal0 = vtxNormTransformed;
					uvFinal0 = XMLoadFloat2((XMFLOAT2*)&pUserVertex->uv0);
					++pUserVertex;
				}
				{
					//Convert untransformed vertex from storage type.
					XMVECTOR vtxPos;
					vtxPos = XMLoadFloat3((XMFLOAT3*)&pUserVertex->position);
					XMVECTOR vtxNorm;
					vtxNorm = XMLoadFloat3((XMFLOAT3*)&pUserVertex->normal);
					//Transform vertex.
					XMVECTOR vtxPosTransformed = XMVectorMultiply(vtxPos, vSize);
					vtxPosTransformed = XMVector3TransformNormal(vtxPosTransformed, mRotate);
					XMVECTOR vtxNormTransformed = XMVector3TransformNormal(vtxNorm, mRotate);

					vFinalPosition1 = XMVectorAdd(vtxPosTransformed, vPosition);
					vFinalNormal1 = vtxNormTransformed;
					uvFinal1 = XMLoadFloat2((XMFLOAT2*)&pUserVertex->uv0);
					++pUserVertex;
				}
				{
					//Convert untransformed vertex from storage type.
					XMVECTOR vtxPos;
					vtxPos = XMLoadFloat3((XMFLOAT3*)&pUserVertex->position);
					XMVECTOR vtxNorm;
					vtxNorm = XMLoadFloat3((XMFLOAT3*)&pUserVertex->normal);
					//Transform vertex.
					XMVECTOR vtxPosTransformed = XMVectorMultiply(vtxPos, vSize);
					vtxPosTransformed = XMVector3TransformNormal(vtxPosTransformed, mRotate);
					XMVECTOR vtxNormTransformed = XMVector3TransformNormal(vtxNorm, mRotate);

					vFinalPosition2 = XMVectorAdd(vtxPosTransformed, vPosition);
					vFinalNormal2 = vtxNormTransformed;
					uvFinal2 = XMLoadFloat2((XMFLOAT2*)&pUserVertex->uv0);
					++pUserVertex;
				}

				// 0
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition0);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				XMStoreFloat2((XMFLOAT2*)&pVertices->u, uvFinal0);
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vFinalNormal0);
				++pVertices;
				// 1
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition1);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				XMStoreFloat2((XMFLOAT2*)&pVertices->u, uvFinal1);
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vFinalNormal1);
				++pVertices;
				// 2
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vFinalPosition2);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
				XMStoreFloat2((XMFLOAT2*)&pVertices->u, uvFinal2);
				XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vFinalNormal2);
				++pVertices;
			}
		}
	}
	return (void*)pVertices;
}
#endif

#ifdef RENDER_DYNAMICEFFECT_USE_HVEC
void* UserShape3D::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	// get the user shape data
	Maths::HF32 vOne = 1.0f;
	const C_UserShape3DVariants* pUserShape3DVariants = (C_UserShape3DVariants*)Spu::FetchAlloc(pSettings->pUserShape3DVariants, sizeof(C_UserShape3DVariants));

#ifdef PLATFORM_SPU
	// allocate enough memory pointers for shape data and vertices
	U32 ptrAllocSize = (pUserShape3DVariants->GetShape3DArray().Size() * 4 + 15) & ~15;
	C_UserShape3DData** ppUserShape3DData = (C_UserShape3DData**)Spu::Alloc(ptrAllocSize);
	C_UserShape3DData::C_Vertex** ppShapeVertices = (C_UserShape3DData::C_Vertex**)Spu::Alloc(ptrAllocSize);

	// set pointer to array of user shape 3D data
	const C_UserShape3DData* pUserShape3DDataEA = &pUserShape3DVariants->GetShape3DArray()[0];

	// fetch all the user shape 3D data to memory
	for(S32 i = 0; i < pUserShape3DVariants->GetShape3DArray().Size(); ++i)
		ppUserShape3DData[i] = (C_UserShape3DData*)Spu::FetchAlloc(&pUserShape3DDataEA[i], sizeof(C_UserShape3DData));
	
	// fetch all the corresponding vertices
	U32 vertexAllocSize = sizeof(C_UserShape3DData::C_Vertex) * pUserShape3DVariants->GetNumVertices();
	for(S32 i = 0; i < pUserShape3DVariants->GetShape3DArray().Size(); ++i)
		ppShapeVertices[i] = (C_UserShape3DData::C_Vertex*)Spu::FetchAlloc(&ppUserShape3DData[i]->GetVertexArray()[0], vertexAllocSize);

	U32 vertexBufferSize = pUserShape3DVariants->GetNumVertices() * sizeof(ParticleVertex);
	// allocate memory for double buffered temporary storage of particles, and store pointer to main memory
	ParticleVertexPtr pVerticesEA = pVertices;
	C_SpuDoubleBuffer<ParticleVertex> buffer;
	buffer.Alloc(vertexBufferSize);
#endif

	for(S32 i = 0; i < blocks.Size(); ++i)
	{
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
			#ifdef PLATFORM_SPU
			pVertices = buffer.GetBuffer();
			buffer.Wait();
			#endif

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
			
			Maths::C_Quaternion qRotation(Maths::Load<Maths::HVec4>(&pParticle->qRotation));
			Maths::HMat33 mRotate(qRotation);

			mRotate = mRotate * mLocal.Rotation();

#ifdef PLATFORM_SPU
			const C_UserShape3DData::C_Vertex* pUserVertex = ppShapeVertices[pParticle->iVariant];
#else
			const C_UserShape3DData::C_Vertex* pUserVertex = &pUserShape3DVariants->GetShape3DArray()[pParticle->iVariant].GetVertexArray()[0];
#endif

			Maths::HVec3 vFinalPosition0;
			Maths::HVec3 vFinalPosition1;
			Maths::HVec3 vFinalPosition2;
			Maths::HVec3 vFinalNormal0;
			Maths::HVec3 vFinalNormal1;
			Maths::HVec3 vFinalNormal2;
			MathsStorage::Float2 uvFinal0;
			MathsStorage::Float2 uvFinal1;
			MathsStorage::Float2 uvFinal2;
			U32 nNumPrimitives = pUserShape3DVariants->GetNumPrimitives();

			for(U32 prim = 0; prim < nNumPrimitives; prim++)
			{
				{
					//Convert untransformed vertex from storage type.
					Maths::HVec3 vtxPos(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pUserVertex->position));
					Maths::HVec3 vtxNorm(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pUserVertex->normal));
					//Transform vertex.
					Maths::HVec3 vtxPosTransformed((vtxPos * vSize) * mRotate + vPosition);
					Maths::HVec3 vtxNormTransformed(vtxNorm * mRotate);
					// store the final positions into aligned stack memory
					vFinalPosition0 = vtxPosTransformed;
					vFinalNormal0 = vtxNormTransformed;
					uvFinal0 = (MathsStorage::Float2&)pUserVertex->uv0;
					++pUserVertex;
				}
				{
					//Convert untransformed vertex from storage type.
					Maths::HVec3 vtxPos(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pUserVertex->position));
					Maths::HVec3 vtxNorm(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pUserVertex->normal));
					//Transform vertex.
					Maths::HVec3 vtxPosTransformed((vtxPos * vSize) * mRotate + vPosition);
					Maths::HVec3 vtxNormTransformed(vtxNorm * mRotate);
					// store the final positions into aligned stack memory
					vFinalPosition1 = vtxPosTransformed;
					vFinalNormal1 = vtxNormTransformed;
					uvFinal1 = (MathsStorage::Float2&)pUserVertex->uv0;
					++pUserVertex;
				}
				{
					//Convert untransformed vertex from storage type.
					Maths::HVec3 vtxPos(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pUserVertex->position));
					Maths::HVec3 vtxNorm(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pUserVertex->normal));
					//Transform vertex.
					Maths::HVec3 vtxPosTransformed((vtxPos * vSize) * mRotate + vPosition);
					Maths::HVec3 vtxNormTransformed(vtxNorm * mRotate);
					// store the final positions into aligned stack memory
					vFinalPosition2 = vtxPosTransformed;
					vFinalNormal2 = vtxNormTransformed;
					uvFinal2 = (MathsStorage::Float2&)pUserVertex->uv0;
					++pUserVertex;
				}

				// 0
				Maths::Store<Maths::HVec3>(vFinalPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uvFinal0.x;
				pVertices->v = uvFinal0.y;
				Maths::Store<Maths::HVec3>(vFinalNormal0, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 1
				Maths::Store<Maths::HVec3>(vFinalPosition1, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uvFinal1.x;
				pVertices->v = uvFinal1.y;
				Maths::Store<Maths::HVec3>(vFinalNormal1, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 2
				Maths::Store<Maths::HVec3>(vFinalPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = uvFinal2.x;
				pVertices->v = uvFinal2.y;
				Maths::Store<Maths::HVec3>(vFinalNormal2, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
			}

			#ifdef PLATFORM_SPU
			// store the blocks worth of particles to system memory
			Spu::StoreAsync((void*)pVerticesEA, (void*)buffer.GetBuffer(), sizeof(ParticleVertex) * pUserShape3DVariants->GetNumVertices(), buffer.GetIndex());
			pVerticesEA += pUserShape3DVariants->GetNumVertices();
			buffer.Swap();
			#endif
		}
	}

	#ifdef PLATFORM_SPU
	buffer.Wait();
	#endif

	return (void*)pVertices;
}
#endif

}
} // ShapePolicy
} // Particles

