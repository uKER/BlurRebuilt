#include "stdafx.h"

#include "Dynamic/Particle/Shape/SpriteLimitSize.h"

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
void* SpriteLimitSize::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;
	XMVECTOR vUp = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vUp);
	XMVECTOR vRight = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vRight);

	XMVECTOR vView = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vPosition);
	XMVECTOR vForward = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vForward);
	XMMATRIX mProj = XMLoadFloat4x4A16((XMFLOAT4X4A16*)&pConstants->mProj);
	XMVECTOR vLimitSize = XMLoadFloat4A16((XMFLOAT4A16*)&pSettings->limitSize);
	XMVECTOR vLimitSizeBegin = XMVectorSplatX(vLimitSize);
	XMVECTOR vLimitSizeMax = XMVectorSplatY(vLimitSize);
	XMVECTOR vLimitSizeDiff = XMVectorSplatZ(vLimitSize);
	XMVECTOR vLimitSizeDiffRcp = XMVectorSplatW(vLimitSize);

	XMVECTOR vDown = -vUp;
	XMVECTOR vLeft = -vRight;
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
			XMVECTOR vSizeX = XMVectorSplatX(vSize);
			XMVECTOR vSizeY = XMVectorSplatY(vSize);

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
			
			// top left
			XMVECTOR vPosition0 = XMVectorMultiplyAdd(vUp, vSizeY, XMVectorMultiplyAdd(vLeft, vSizeX, vPosition));
			// top right
			XMVECTOR vPosition1 = XMVectorMultiplyAdd(vUp, vSizeY, XMVectorMultiplyAdd(vRight, vSizeX, vPosition));
			// bottom right
			XMVECTOR vPosition2 = XMVectorMultiplyAdd(vDown, vSizeY, XMVectorMultiplyAdd(vRight, vSizeX, vPosition));
			// bottom left
			XMVECTOR vPosition3 = XMVectorMultiplyAdd(vDown, vSizeY, XMVectorMultiplyAdd(vLeft, vSizeX, vPosition));
						
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

void* SpriteLimitSize::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;
	XMVECTOR vUp = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vUp);
	XMVECTOR vRight = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vRight);

	XMVECTOR vView = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vPosition);
	XMVECTOR vForward = XMLoadFloat4A16((XMFLOAT4A16*)&pConstants->vForward);
	XMMATRIX mProj = XMLoadFloat4x4A16((XMFLOAT4X4A16*)&pConstants->mProj);
	XMVECTOR vLimitSize = XMLoadFloat4A16((XMFLOAT4A16*)&pSettings->limitSize);
	XMVECTOR vLimitSizeBegin = XMVectorSplatX(vLimitSize);
	XMVECTOR vLimitSizeMax = XMVectorSplatY(vLimitSize);
	XMVECTOR vLimitSizeDiff = XMVectorSplatZ(vLimitSize);
	XMVECTOR vLimitSizeDiffRcp = XMVectorSplatW(vLimitSize);

	XMVECTOR vDown = -vUp;
	XMVECTOR vLeft = -vRight;
	XMVECTOR vOne = XMVectorSplatOne();

	for(S32 a = 0; a < sortedList.Size(); ++a)
	{
		const C_Particle3D* pParticle = (C_Particle3D*)sortedList[a].pParticle;
	
		XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle->vPosition);	
		
		XMVECTOR vLife = XMVectorSplatW(vPosition);
		F32 fLife = pParticle->fLife;
		//vPosition = XMVector3TransformCoord(vPosition, mLocal);

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
		XMVECTOR vSizeX = XMVectorSplatX(vSize);
		XMVECTOR vSizeY = XMVectorSplatY(vSize);

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

		// top left
		XMVECTOR vPosition0 = XMVectorMultiplyAdd(vUp, vSizeY, XMVectorMultiplyAdd(vLeft, vSizeX, vPosition));
		// top right
		XMVECTOR vPosition1 = XMVectorMultiplyAdd(vUp, vSizeY, XMVectorMultiplyAdd(vRight, vSizeX, vPosition));
		// bottom right
		XMVECTOR vPosition2 = XMVectorMultiplyAdd(vDown, vSizeY, XMVectorMultiplyAdd(vRight, vSizeX, vPosition));
		// bottom left
		XMVECTOR vPosition3 = XMVectorMultiplyAdd(vDown, vSizeY, XMVectorMultiplyAdd(vLeft, vSizeX, vPosition));
					
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
	return (void*)pVertices;
}

#endif

#ifdef RENDER_DYNAMICEFFECT_USE_HVEC
void* SpriteLimitSize::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;
	
	Maths::HVec3 vUp(Maths::Load<Maths::HVec3>(&pConstants->vUp));
	Maths::HVec3 vRight(Maths::Load<Maths::HVec3>(&pConstants->vRight));

	Maths::HVec3 vView(Maths::Load<Maths::HVec3>(&pConstants->vPosition));
	Maths::HVec3 vForward(Maths::Load<Maths::HVec3>(&pConstants->vForward));
	Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&pConstants->mProj));
	Maths::HVec4 vLimitSize(Maths::Load<Maths::HVec4>(&pSettings->limitSize));
	Maths::HF32 vLimitSizeBegin(vLimitSize.X());
	Maths::HF32 vLimitSizeMax(vLimitSize.Y());
	Maths::HF32 vLimitSizeDiff(vLimitSize.Z());
	Maths::HF32 vLimitSizeDiffRcp(vLimitSize.W());
	
	Maths::HVec3 vDown = -vUp;
	Maths::HVec3 vLeft = -vRight;
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
			const C_Particle3D* pParticle = (const C_Particle3D*)&pBlock->m_Element[a];
	
			Maths::HVec4 vPositionLife(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pParticle->vPosition));	
			Maths::HVec3 vPosition(vPositionLife);
			vPosition = vPosition * mLocal;

			Maths::HF32 vLife = vPositionLife.W();
			F32 fLife = pParticle->fLife;

			Maths::HVec4 vColourCurve = LookupValue(pSettings->colourTable, vLife, fLife);
			Maths::HVec4 vForceSizeLife(Maths::Load<Maths::HVec4>(&pParticle->vForceSizeLife));
			Maths::HVec4 vLighting(Maths::Load<Maths::HVec4>(&pParticle->vColour));
			
			Maths::HVec4 vSizeCurve(LookupValue(pSettings->sizeTable, vLife, fLife));
			Maths::HVec4 vColour = vColourCurve * vLighting;
			Maths::HF32 vAnim = vSizeCurve.W();
			vSizeCurve = vSizeCurve * vForceSizeLife.Z();

			// clamp particle to fixed size on screen
			{
				Maths::HVec3 vViewToParticle = vPosition - vView;
				Maths::HF32 vDot = vViewToParticle | vForward;
				Maths::HVec4 vCoord(vSizeCurve.X(), Maths::Constants<Maths::HF32>::Zero(), vDot, Maths::Constants<Maths::HF32>::One());
				vCoord = vCoord * mProjection;
				Maths::HF32 vScreenSize = Maths::Abs(vCoord.X() / vCoord.W());

				if(pSettings->eLimitSize == E_ParticleLimitSize_Clamp && vScreenSize > vLimitSizeMax)
					vSizeCurve *= vLimitSizeMax / vScreenSize;
				
				if(pSettings->eLimitSize == E_ParticleLimitSize_AlphaFade)
				{
					Maths::HF32 fFadeAmount = (vScreenSize - vLimitSizeBegin) * vLimitSizeDiffRcp;
					Maths::HF32 fAlphaFade = Maths::Clamped(fFadeAmount, Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::One());
					fAlphaFade = Maths::Constants<Maths::HF32>::One() - fAlphaFade;
					vColour.W(vColour.W() * fAlphaFade);

					if(fAlphaFade == Maths::Constants<Maths::HF32>::Zero())
						vSizeCurve = Maths::Constants<Maths::HVec3>::Zero();
				}
			}
			Maths::HF32 vSizeX = vSizeCurve.X();
			Maths::HF32 vSizeY = vSizeCurve.Y();

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

	#ifdef PLATFORM_SPU
	pVertices = pVerticesEA;
	buffer.Wait();
	#endif

	return (void*)pVertices;
}

void* SpriteLimitSize::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	ParticleVertexPtr pVertices = (ParticleVertexPtr)pBuffer;
	
	Maths::HVec3 vUp(Maths::Load<Maths::HVec3>(&pConstants->vUp));
	Maths::HVec3 vRight(Maths::Load<Maths::HVec3>(&pConstants->vRight));

	Maths::HVec3 vView(Maths::Load<Maths::HVec3>(&pConstants->vPosition));
	Maths::HVec3 vForward(Maths::Load<Maths::HVec3>(&pConstants->vForward));
	Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&pConstants->mProj));
	Maths::HVec4 vLimitSize(Maths::Load<Maths::HVec4>(&pSettings->limitSize));
	Maths::HF32 vLimitSizeBegin(vLimitSize.X());
	Maths::HF32 vLimitSizeMax(vLimitSize.Y());
	Maths::HF32 vLimitSizeDiff(vLimitSize.Z());
	Maths::HF32 vLimitSizeDiffRcp(vLimitSize.W());
	
	Maths::HVec3 vDown = -vUp;
	Maths::HVec3 vLeft = -vRight;
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
		
		const C_Particle3D* pParticle = (const C_Particle3D*)sortedList[a].pParticle;
	
		Maths::HVec4 vPositionLife(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pParticle->vPosition));	
		Maths::HVec3 vPosition(vPositionLife);
	
		Maths::HF32 vLife = vPositionLife.W();
		F32 fLife = pParticle->fLife;

		Maths::HVec4 vColourCurve = LookupValue(pSettings->colourTable, vLife, fLife);
		Maths::HVec4 vForceSizeLife(Maths::Load<Maths::HVec4>(&pParticle->vForceSizeLife));
		Maths::HVec4 vLighting(Maths::Load<Maths::HVec4>(&pParticle->vColour));
		
		Maths::HVec4 vSizeCurve(LookupValue(pSettings->sizeTable, vLife, fLife));
		Maths::HVec4 vColour = vColourCurve * vLighting;
		Maths::HF32 vAnim = vSizeCurve.W();
		vSizeCurve = vSizeCurve * vForceSizeLife.Z();

		// clamp particle to fixed size on screen
		{
			Maths::HVec3 vViewToParticle = vPosition - vView;
			Maths::HF32 vDot = vViewToParticle | vForward;
			Maths::HVec4 vCoord(vSizeCurve.X(), 0.0f, vDot, 1.0f);
			vCoord = vCoord * mProjection;
			Maths::HF32 vScreenSize = Maths::Abs(vCoord.X() / vCoord.W());

			if(pSettings->eLimitSize == E_ParticleLimitSize_Clamp && vScreenSize > vLimitSizeMax)
				vSizeCurve *= vLimitSizeMax / vScreenSize;
			
			if(pSettings->eLimitSize == E_ParticleLimitSize_AlphaFade)
			{
				Maths::HF32 fFadeAmount = (vScreenSize - vLimitSizeBegin) * vLimitSizeDiffRcp;
				Maths::HF32 fAlphaFade = Maths::Clamped(fFadeAmount, Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::One());
				fAlphaFade = Maths::Constants<Maths::HF32>::One() - fAlphaFade;
				vColour.W(vColour.W() * fAlphaFade);

				if(fAlphaFade == Maths::Constants<Maths::HF32>::Zero())
					vSizeCurve = Maths::Constants<Maths::HVec3>::Zero();
			}
		}
		Maths::HF32 vSizeX = vSizeCurve.X();
		Maths::HF32 vSizeY = vSizeCurve.Y();

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

		// top left
		Maths::HVec3 vPosition0 = vUp * vSizeY + vLeft * vSizeX + vPosition;
		// top right
		Maths::HVec3 vPosition1 = vUp * vSizeY + vRight * vSizeX + vPosition;
		// bottom right
		Maths::HVec3 vPosition2 = vDown * vSizeY + vRight * vSizeX + vPosition;
		// bottom left
		Maths::HVec3 vPosition3 = vDown * vSizeY + vLeft * vSizeX + vPosition;

		#ifdef PLATFORM_SPU
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

}
} // ShapePolicy
} // Particles

