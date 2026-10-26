#ifndef RENDER_DYNAMICEFFECT_LIGHT_BEAM2D_H
#define RENDER_DYNAMICEFFECT_LIGHT_BEAM2D_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Display/Buffer/VertexDeclarations.h"
#include "Dynamic/Light/Shape/ShapePolicyInterface.h"
#include "Dynamic/Light/Instance.h"
#include "Dynamic/Light/Light.h"

namespace DynamicEffect
{
namespace Light
{

template<typename InstanceType>
class Beam2dPolicy : public I_ShapePolicy
{
public:

	typedef Display::Vertex_XY_C_UVW			ShapeVertex;
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
INLINE void* Beam2dPolicy<InstanceType>::Build(const RenderNodePtrs& blocks, volatile void* __restrict pBuffer) const
{
	const Maths::HF32 vZero = Maths::Constants<Maths::HF32>::Zero();
	const Maths::HF32 vOne = Maths::Constants<Maths::HF32>::One();

	const Maths::HMat44 mProj = Display::Device()->GetViewSettings3D().GetCameraToDevice();
	const Maths::HF32 vProj11 = mProj.AX().X();
	const Maths::HF32 vProj22 = mProj.AY().Y();

	const Maths::HVec2 vSizeScale = Maths::HVec2(vProj11, vProj22);

	const Maths::HVec2 vGlobalOffset = m_pEffect->GetGlobalOffset();

	ShapeVertexPtr pVertices = (ShapeVertexPtr)pBuffer;

	for(S32 i = 0; i < blocks.Size(); ++i)
	{
		InstanceType* pBlock = (InstanceType *) blocks[i];
		const Light& light = *pBlock->m_hLight;

		Maths::HVec3 vXTransform		= Maths::Load<Maths::HVec3>(&pBlock->m_vXTransform2d);
		Maths::HVec3 vYTransform		= Maths::Load<Maths::HVec3>(&pBlock->m_vYTransform2d);
		Maths::HVec4 vScreenPosition	= Maths::Load<Maths::HVec4>(&light.m_vScreenPosition);
		Maths::HVec3 vScreen2D			= Maths::HVec3(vScreenPosition.X() + vGlobalOffset.X(), vScreenPosition.Y() + vGlobalOffset.X(), vOne);
		Maths::HVec2 vBegin2D			= Maths::HVec2(vScreen2D) - vGlobalOffset;
		Maths::HVec2 vCentre2D			= Maths::HVec2(vXTransform | vScreen2D, vYTransform | vScreen2D) - vGlobalOffset;
		Maths::HVec2 vDirection			= Maths::HVec2(vCentre2D - vBegin2D).Normalised();
		Maths::HVec4 vSizeData			= Maths::Load<Maths::HVec4>((MathsStorage::Float4A16 *) &pBlock->m_vDimensions);
		Maths::HVec2 vOffset			= Maths::HVec2(vDirection.Y(), -vDirection.X()) * vSizeScale * vSizeData.X();
		Maths::HVec2 vEnd2D				= vBegin2D + vDirection * vSizeData.Y();
		Maths::HVec2 vPosition0			= vBegin2D - vOffset;
		Maths::HVec2 vPosition1			= vBegin2D + vOffset;
		Maths::HVec2 vPosition2			= vEnd2D + vOffset;
		Maths::HVec2 vPosition3			= vEnd2D - vOffset;

		const Maths::HVec4 vData				= Maths::Load<Maths::HVec4>((MathsStorage::Float4A16 *) &light.m_fIntensity);
		const Maths::HF32 vLightIntensity		= m_pEffect->GetLightIntensity(vData.X() * vSizeData.Z());
		const Maths::HF32 vOcclusionIntensity	= vData.Y();
		const Maths::HF32 vViewIntensity		= m_pEffect->GetViewIntensity(vData.Z());
		const Maths::HF32 vDistanceIntensity	= m_pEffect->GetDistanceIntensity(vData.W());
		const Maths::HF32 vPositionIntensity	= m_pEffect->GetPositionIntensity(Maths::HVec2(vScreenPosition), vCentre2D);
		const Maths::HF32 vIntensity			= vLightIntensity * vOcclusionIntensity * vPositionIntensity * vViewIntensity * vDistanceIntensity;

		const Render::Billboard::TextureCoords& textureCoords = m_pEffect->GetTextureCoords(pBlock->m_iTexture);

		// 0
		Maths::Store(vPosition0, (MathsStorage::Float2*)&pVertices->x);
		pVertices->c = light.m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(0).u;
		pVertices->v = textureCoords.GetTextureCoord(0).v;
		pVertices->w = vIntensity;
		++pVertices;
		// 1
		Maths::Store(vPosition1, (MathsStorage::Float2*)&pVertices->x);
		pVertices->c = light.m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(1).u;
		pVertices->v = textureCoords.GetTextureCoord(1).v;
		pVertices->w = vIntensity;
		++pVertices;
		// 2
		Maths::Store(vPosition2, (MathsStorage::Float2*)&pVertices->x);
		pVertices->c = light.m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(2).u;
		pVertices->v = textureCoords.GetTextureCoord(2).v;
		pVertices->w = vIntensity;
		++pVertices;
#ifndef PLATFORM_PC
		// 3
		Maths::Store(vPosition3, (MathsStorage::Float2*)&pVertices->x);
		pVertices->c = light.m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(3).u;
		pVertices->v = textureCoords.GetTextureCoord(3).v;
		pVertices->w = vIntensity;
		++pVertices;
#else
		// 2
		Maths::Store(vPosition2, (MathsStorage::Float2*)&pVertices->x);
		pVertices->c = light.m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(2).u;
		pVertices->v = textureCoords.GetTextureCoord(2).v;
		pVertices->w = vIntensity;
		++pVertices;
		// 3
		Maths::Store(vPosition3, (MathsStorage::Float2*)&pVertices->x);
		pVertices->c = light.m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(3).u;
		pVertices->v = textureCoords.GetTextureCoord(3).v;
		pVertices->w = vIntensity;
		++pVertices;
		// 0
		Maths::Store(vPosition0, (MathsStorage::Float2*)&pVertices->x);
		pVertices->c = light.m_iColour;
		pVertices->u = textureCoords.GetTextureCoord(0).u;
		pVertices->v = textureCoords.GetTextureCoord(0).v;
		pVertices->w = vIntensity;
		++pVertices;
#endif
	}
	return (void*)pVertices;
}

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_BEAM2D_H
