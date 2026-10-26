#ifndef RENDER_DYNAMICEFFECT_LIGHT_DECAL3D_H
#define RENDER_DYNAMICEFFECT_LIGHT_DECAL3D_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Display/Buffer/VertexDeclarations.h"
#include "Render/Billboard/Decal.h"
#include "Dynamic/Light/Shape/ShapePolicyInterface.h"
#include "Dynamic/Light/Instance.h"
#include "Dynamic/Light/Light.h"

namespace DynamicEffect
{
namespace Light
{

template<typename InstanceType>
class Decal3dPolicy : public I_ShapePolicy
{
public:

	typedef Display::Vertex_XYZ_C_UVW			ShapeVertex;
	typedef volatile ShapeVertex* __restrict	ShapeVertexPtr;
	
	void*	Build(const RenderNodePtrs& blocks, volatile void* __restrict pBuffer) const;

	void	Begin() const {};
	void	End() const {};

	U32 GetVertexSize() const
	{
		return sizeof(ShapeVertex);
	}

	Display::C_VertexDeclaration* GetVertexDeclaration() const
	{
		return ShapeVertex::m_pDeclaration;
	}

	U32	GetNumPrimitives() const
	{
		return m_nNumPrimitives;
	}
	
	U32 GetNumVertices() const
	{
		return m_nNumVertices;
	}
	
	Display::E_PrimitiveType GetPrimitiveType() const
	{
		return m_ePrimitiveType;
	}

private:

#ifndef PLATFORM_PC
	static const U32 m_nNumPrimitives = 1;
	static const U32 m_nNumVertices = 4;
	static const Display::E_PrimitiveType	m_ePrimitiveType = Display::E_PrimitiveType_QuadList;
#else
	static const U32 m_nNumPrimitives = 2;
	static const U32 m_nNumVertices = 3;
	static const Display::E_PrimitiveType	m_ePrimitiveType = Display::E_PrimitiveType_TriangleList;
#endif

};

template<typename InstanceType>
INLINE void* Decal3dPolicy<InstanceType>::Build(const RenderNodePtrs& blocks, volatile void* __restrict pBuffer) const
{
	ShapeVertexPtr pVertices = (ShapeVertexPtr)pBuffer;

	Maths::HF32 vZero = Maths::Constants<Maths::HF32>::Zero();
	Maths::HF32 vOne = Maths::Constants<Maths::HF32>::One();

	Render::Billboard::Decal decal;
	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		InstanceType* pBlock = (InstanceType *) blocks[i];
		Light& light = *pBlock->m_hLight;

		Maths::HVec3 vPosition = Maths::Load<Maths::HVec3>(&pBlock->m_vPosition);
		Maths::C_Quaternion vRotation = Maths::Load<Maths::C_Quaternion>(&pBlock->m_vRotation);
		if (pBlock->m_pNode)
		{
			const Maths::C_QuatTrans& transform = *pBlock->m_pNode;
			vPosition = vPosition * transform;
			vRotation = vRotation * transform.Quaternion();
		}
		Maths::HVec4 vSizeData	= Maths::Load<Maths::HVec4>((MathsStorage::Float4A16 *) &pBlock->m_vDimensions);
		Maths::HVec3 vSize		= Maths::HVec3(vSizeData);
		vSize.Z(vSize.Y());

		const Maths::HVec4 vData				= Maths::Load<Maths::HVec4>((MathsStorage::Float4A16 *) &light.m_fIntensity);
		const Maths::HF32 vLightIntensity		= m_pEffect->GetLightIntensity(vData.X() * vSizeData.Z());
		const Maths::HF32 vViewIntensity		= m_pEffect->GetViewIntensity(vData.Z());
		const Maths::HF32 vDistanceIntensity	= m_pEffect->GetDistanceIntensity(vData.W());
		const Maths::HF32 vIntensity			= vLightIntensity * vViewIntensity * vDistanceIntensity;

		const Render::Billboard::TextureCoords& textureCoords = m_pEffect->GetTextureCoords(pBlock->m_iTexture);

		Maths::HMat33 mRotate(vRotation);
		decal.Build(vPosition, mRotate, vSize);

		// 0
		Maths::Store(decal.m_vPosition0, (MathsStorage::Float3*)&pVertices->x);
		pVertices->c = pBlock->m_hLight->m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(0).u;
		pVertices->v = textureCoords.GetTextureCoord(0).v;
		pVertices->w = pBlock->m_hLight->m_fIntensity;
		++pVertices;
		// 1
		Maths::Store(decal.m_vPosition1, (MathsStorage::Float3*)&pVertices->x);
		pVertices->c = pBlock->m_hLight->m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(1).u;
		pVertices->v = textureCoords.GetTextureCoord(1).v;
		pVertices->w = pBlock->m_hLight->m_fIntensity;
		++pVertices;
		// 2
		Maths::Store(decal.m_vPosition2, (MathsStorage::Float3*)&pVertices->x);
		pVertices->c = pBlock->m_hLight->m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(2).u;
		pVertices->v = textureCoords.GetTextureCoord(2).v;
		pVertices->w = pBlock->m_hLight->m_fIntensity;
		++pVertices;
#ifndef PLATFORM_PC
		// 3
		Maths::Store(decal.m_vPosition3, (MathsStorage::Float3*)&pVertices->x);
		pVertices->c = pBlock->m_hLight->m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(3).u;
		pVertices->v = textureCoords.GetTextureCoord(3).v;
		pVertices->w = pBlock->m_hLight->m_fIntensity;
		++pVertices;
#else
		// 2
		Maths::Store(decal.m_vPosition2, (MathsStorage::Float3*)&pVertices->x);
		pVertices->c = pBlock->m_hLight->m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(2).u;
		pVertices->v = textureCoords.GetTextureCoord(2).v;
		pVertices->w = pBlock->m_hLight->m_fIntensity;
		++pVertices;
		// 3
		Maths::Store(decal.m_vPosition3, (MathsStorage::Float3*)&pVertices->x);
		pVertices->c = pBlock->m_hLight->m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(3).u;
		pVertices->v = textureCoords.GetTextureCoord(3).v;
		pVertices->w = pBlock->m_hLight->m_fIntensity;
		++pVertices;
		// 0
		Maths::Store(decal.m_vPosition0, (MathsStorage::Float3*)&pVertices->x);
		pVertices->c = pBlock->m_hLight->m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(0).u;
		pVertices->v = textureCoords.GetTextureCoord(0).v;
		pVertices->w = pBlock->m_hLight->m_fIntensity;
		++pVertices;
#endif
	}
	return (void*)pVertices;
}

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_DECAL3D_H
