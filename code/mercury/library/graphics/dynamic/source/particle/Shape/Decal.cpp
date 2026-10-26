#include "stdafx.h"

#include "Dynamic/Particle/Shape/Decal.h"

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
void* Decal::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

// 	Maths::HVec3 vVertex0(-1.0f, 0.01f, 1.0f);
// 	Maths::HVec3 vVertex1(-1.0f, 0.01f, 1.0f);
// 	Maths::HVec3 vVertex2(1.0f, 0.01f, -1.0f);
// 	Maths::HVec3 vVertex3(1.0f, 0.01f, -1.0f);
	
	Maths::HVec3 vVertex0(-1.0f, 1.0f, 0.01f);
	Maths::HVec3 vVertex1(-1.0f, 1.0f, 0.01f);
	Maths::HVec3 vVertex2(1.0f, -1.0f, 0.01f);
	Maths::HVec3 vVertex3(1.0f, -1.0f, 0.01f);

	Maths::HF32 vOne = 1.0f;
	

	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		ParticleBlock* pBlock = (ParticleBlock*)blocks[i];

		Maths::HMat43 mLocal(Maths::UNDEFINED);
		if(pBlock->m_pNode)
			mLocal = Maths::C_QuatTrans(*pBlock->m_pNode).Matrix();
		else
			mLocal.SetIdentity();

		for(S32 a = 0; a < pBlock->m_Element.Size(); a++)
		{
			const C_Particle3D_R3D* pParticle = (C_Particle3D_R3D*)&pBlock->m_Element[a];
	
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

			Maths::HVec3 vNormal = Maths::Constants<Maths::HVec3>::UnitAxisPositiveY() * mRotate;

			Maths::HVec3 vFinalPosition0((vVertex0 * vSize) * mRotate + vPosition);
			Maths::HVec3 vFinalPosition1((vVertex1 * vSize) * mRotate + vPosition);
			Maths::HVec3 vFinalPosition2((vVertex2 * vSize) * mRotate + vPosition);
			Maths::HVec3 vFinalPosition3((vVertex3 * vSize) * mRotate + vPosition);

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
	return (void*)pVertices;
}
#endif

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
void* Decal::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;

	XMVECTOR vVertex0 = XMVectorSet(-1.0f, 1.0f, 0.01f, 1.0f);
	XMVECTOR vVertex1 = XMVectorSet(-1.0f, 1.0f, 0.01f, 1.0f);
	XMVECTOR vVertex2 = XMVectorSet(1.0f, -1.0f, 0.01f,  1.0f);
	XMVECTOR vVertex3 = XMVectorSet(1.0f, -1.0f, 0.01f,  1.0f);
	XMVECTOR vUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	
	
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
			DCBT(1024, pParticle);
			DCBT(0,&pSettings->sizeTable);
	
			XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);
			XMVECTOR qRotation = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->qRotation);
			XMVECTOR vLife = XMVectorSplatW(vPosition);
			F32 fLife = pParticle->fLife;

			vPosition = XMVector3TransformCoord(vPosition, mLocal);
						
			XMMATRIX mRotate = XMMatrixRotationQuaternion(qRotation);
			mRotate = XMMatrixMultiply(mRotate, mLocal);

			XMVECTOR vNormal = XMVector3TransformNormal(vUp, mRotate);

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
			
			XMVECTOR vTransformedVertex0 = XMVectorMultiply(vVertex0, vSize);
			XMVECTOR vTransformedVertex1 = XMVectorMultiply(vVertex1, vSize);
			XMVECTOR vTransformedVertex2 = XMVectorMultiply(vVertex2, vSize);
			XMVECTOR vTransformedVertex3 = XMVectorMultiply(vVertex3, vSize);

			vTransformedVertex0 = XMVector3TransformNormal(vTransformedVertex0, mRotate);
			vTransformedVertex1 = XMVector3TransformNormal(vTransformedVertex1, mRotate);
			vTransformedVertex2 = XMVector3TransformNormal(vTransformedVertex2, mRotate);
			vTransformedVertex3 = XMVector3TransformNormal(vTransformedVertex3, mRotate);

			XMVECTOR vPosition0 = XMVectorAdd(vTransformedVertex0, vPosition);
			XMVECTOR vPosition1 = XMVectorAdd(vTransformedVertex1, vPosition);
			XMVECTOR vPosition2 = XMVectorAdd(vTransformedVertex2, vPosition);
			XMVECTOR vPosition3 = XMVectorAdd(vTransformedVertex3, vPosition);
						
			// 0
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[0];
			pVertices->v = uv0.v[0];
			XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
			++pVertices;
			// 1
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition1);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[1];
			pVertices->v = uv0.v[1];
			XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
			++pVertices;
			// 2
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[2];
			pVertices->v = uv0.v[2];
			XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
			++pVertices;
#ifndef PLATFORM_PC
			// 3
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[3];
			pVertices->v = uv0.v[3];
			XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
			++pVertices;	
#else
			// 2
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition2);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[2];
			pVertices->v = uv0.v[2];
			XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
			++pVertices;
			// 3
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition3);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[3];
			pVertices->v = uv0.v[3];
			XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
			++pVertices;
			// 0
			XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition0);
			XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour);
			pVertices->u = uv0.u[0];
			pVertices->v = uv0.v[0];
			XMStoreFloat3((XMFLOAT3*)&pVertices->nx, vNormal);
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

