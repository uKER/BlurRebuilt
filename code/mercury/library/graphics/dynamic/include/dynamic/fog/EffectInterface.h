#ifndef RENDER_DYNAMICEFFECT_FOG_EFFECTINTERFACE_H
#define RENDER_DYNAMICEFFECT_FOG_EFFECTINTERFACE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Core/Containers/Algorithm.h"
#include "Display/Buffer/VertexDeclarations.h"
#include "Dynamic/Interface.h"
#include "Dynamic/Node.h"
#include "Dynamic/Fog/Enums.h"
#include "Dynamic/Fog/Manager.h"
#include "Dynamic/Fog/Volume.h"
#include "Dynamic/Shaders/Constants.h"
#include "Render/Utils/Globals.h"

namespace DynamicEffect
{

class C_Renderer;
class I_DynamicEffectManager;
	
namespace Fog
{

extern Debug::Float g_debugCameraInsideEpsilon;

class I_VolumeEffect : public I_DynamicEffect
{
public:

	I_VolumeEffect(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex);
	virtual ~I_VolumeEffect();

	// interface methods
	virtual void	OnBuild(const C_View& view, C_CompositeRenderNode& node) const;

	// virtual methods
	virtual Maths::HVec4	EvaluateFogAt(const Maths::HVec3& vPoint) = 0;

protected:

	// interface methods
	virtual void	Update();
	virtual void	Reset();
	virtual void	OnGather(const SceneGraph::C_View& view) const;
	virtual void	Clean(const C_Node* pNode);

protected:

	Volumes		m_Volumes;

	E_Draw		m_eDraw;
	E_Detail	m_eDetail;

public:

	FORCE_INLINE Volume* NewVolume()
	{
		++m_nBlocks;

		m_Volumes.PushBack(new Volume);
		return m_Volumes.Back();
	}

	FORCE_INLINE Volume* NewLocalVolume()
	{
		++m_nBlocksLocal;
		return NewVolume();
	}

	FORCE_INLINE void FreeVolume(Volume* pVolume)
	{
		Volumes::Iterator it = Ut::Find(m_Volumes, pVolume);
		if (it != m_Volumes.End())
		{
			--m_nBlocks;
			if (pVolume->m_pNode)
				--m_nBlocksLocal;

			m_Volumes.Erase(it);
			delete pVolume;
		}
	}

	FORCE_INLINE void FreeVolume(const S32 volumeNo)
	{
		--m_nBlocks;
		if (m_Volumes[volumeNo]->m_pNode)
			--m_nBlocksLocal;

		Volume* pVolume = m_Volumes[volumeNo];
		delete pVolume;

		m_Volumes[volumeNo] = m_Volumes.Back();
		m_Volumes.PopBack();
	}

	FORCE_INLINE void ClearVolumes()
	{
		for (Volumes::Iterator it = m_Volumes.Begin(); it != m_Volumes.End(); ++it)
			delete *it;
		m_Volumes.Clear();
	}

};

template<typename Shape>
class C_VolumeEffect : public I_VolumeEffect
{
public:
	
	C_VolumeEffect(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : I_VolumeEffect(pRenderer, pManager, iIndex, iTypeIndex) {}
	virtual ~C_VolumeEffect() {};

	// interface methods
	virtual void OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const;

	// virtual methods
	virtual Maths::HVec4 EvaluateFogAt(const Maths::HVec3& vPoint);

protected:

	// interface methods
	virtual bool	Create();
	virtual void	Destroy();

protected:

	Shape m_Shape;

};

template<class Shape>
FORCE_INLINE void C_VolumeEffect<Shape>::OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const
{
	C_Manager* pManager = (C_Manager*) m_pManagerInterface;

	// constants
	const F32 fFrameTime = GetRenderer()->GetFrameTime();
	const Maths::HVec3 vViewPosition = Maths::HVec3(Maths::Load<Maths::HVec4>(&Display::Device()->GetViewSettings3D().GetViewPosition()));

	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	const E_Shader eVertexShader = pManager->GetVertexShader(m_Shape.m_eShape, m_eDraw, m_eDetail);
	const E_Shader ePixelShader = pManager->GetPixelShader(m_Shape.m_eShape, m_eDraw, m_eDetail);

	pManager->SetShaderConstants(ePixelShader);
	pManager->SetVertexShader(eVertexShader);
	pManager->SetPixelShader(ePixelShader);
	Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ::m_pDeclaration);
	Display::Device()->SetVertexBuffer(0, &m_Shape.m_VertexBuffer, 0, sizeof(Display::Vertex_XYZ));
	Display::Device()->SetIndexBuffer(&m_Shape.m_IndexBuffer);
	
	Display::Device()->SetTexture(0, Render::GetGlobalDepthStencilBuffer());
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);

	if (m_eDraw == E_Draw_Front)
	{
		Display::Device()->SetCullMode(Display::E_CullMode_Ccw);
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
	}
	else if (m_eDraw == E_Draw_Back)
	{
		Display::Device()->SetCullMode(Display::E_CullMode_Cw);
		Display::Device()->SetDepthEnable(Display::E_Bool_False);
	}

	for (S32 blockNo = 0; blockNo < node.blocks.Size(); ++blockNo)
	{
		Volume* pVolume = (Volume*)node.blocks[blockNo];

		// local matrix
		Maths::HVec4 vTranslation(Maths::Load<Maths::HVec4>(&pVolume->m_vCentre));
		Maths::C_Quaternion qRotation(Maths::Load<Maths::C_Quaternion>(&pVolume->m_vRotation));
		Maths::HVec3 vScale(Maths::Load<Maths::HVec3>((MathsStorage::Float3A16*)&pVolume->m_vSize));
		Maths::HMat43 mWorld = Maths::HMat43(Maths::HMat33(qRotation), Maths::HVec3(vTranslation));
		// transform into world space
		if (pVolume->m_pNode)
			mWorld = mWorld * Maths::C_QuatTrans(*pVolume->m_pNode).Matrix();
		// scale
		Maths::HMat33 mScale = Maths::HMat33(Maths::C_MatrixScale(vScale));
		mWorld = Maths::HMat43(mScale) * mWorld;
		// object matrix
		Maths::HMat43 mObject = Maths::HMat43(Maths::HMat44(mWorld).Inverse());

		// switch states depending on whether or not the view position is inside the shape
		Display::C_Float4 vInside;
		const bool bInside = m_Shape.IsInside(vViewPosition, mObject, Maths::HF32(g_debugCameraInsideEpsilon));
		const F32 fInside = bInside ? 1.0f : -1.0f;

		// vertex shader constants
		Display::C_Matrix matWorld, matObject;
		Maths::Store(Maths::HMat44(mWorld), &matWorld);
		Maths::Store(Maths::HMat44(mObject), &matObject);
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&matWorld, 4);
		Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)&matObject, 4);
		Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_4, Display::C_PSConstant(pVolume->m_vSettings.x, pVolume->m_vSettings.y, fInside), 1);
		Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_5, (F32*)&pVolume->m_vColour, 1);

		// draw mesh
		Display::Device()->DrawIndexedPrimitive(m_Shape.m_PrimitiveType, 0, 0, m_Shape.m_VertexCount, 0, m_Shape.m_IndexCount);
	}

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
}

template<class Shape>
FORCE_INLINE bool C_VolumeEffect<Shape>::Create()
{
	if (!m_Shape.Create())
		return false;

	return true;
}

template<class Shape>
FORCE_INLINE void C_VolumeEffect<Shape>::Destroy()
{
	m_Shape.Destroy();
}

template<class Shape>
FORCE_INLINE Maths::HVec4 C_VolumeEffect<Shape>::EvaluateFogAt(const Maths::HVec3& vPoint)
{
	const Maths::HF32 vZero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 vOne(Maths::Constants<Maths::HF32>::One());

	for (S32 volumeNo = 0; volumeNo < m_Volumes.Size(); ++volumeNo)
	{
		Volume* pVolume = m_Volumes[volumeNo];

		// local matrix
		Maths::HVec4 vTranslation(Maths::Load<Maths::HVec4>(&pVolume->m_vCentre));
		Maths::C_Quaternion qRotation(Maths::Load<Maths::C_Quaternion>(&pVolume->m_vRotation));
		Maths::HVec3 vScale(Maths::Load<Maths::HVec3>((MathsStorage::Float3A16*)&pVolume->m_vSize));
		Maths::HMat43 mWorld = Maths::HMat43(Maths::HMat33(qRotation), Maths::HVec3(vTranslation));
		// transform into world space
		if (pVolume->m_pNode)
			mWorld = mWorld * Maths::C_QuatTrans(*pVolume->m_pNode).Matrix();
		// scale
		Maths::HMat33 mScale = Maths::HMat33(Maths::C_MatrixScale(vScale));
		mWorld = Maths::HMat43(mScale) * mWorld;
		// object matrix
		Maths::HMat43 mObject = Maths::HMat43(Maths::HMat44(mWorld).Inverse());

		if (m_Shape.IsInside(vPoint, mObject))
		{
			Maths::HVec2 vSettings = Maths::Load<Maths::HVec2>(&pVolume->m_vSettings);
			Maths::HVec4 vColour = Maths::Load<Maths::HVec4>(&pVolume->m_vColour);
			Maths::HF32 vDensity = m_Shape.EvaluateDensityAt(vPoint, mObject, vSettings);
			return Maths::HVec4(vColour.X(), vColour.Y(), vColour.Z(), vOne - vDensity);
		}
	}
	return Maths::HVec4(vZero, vZero, vZero, vZero);
}

}; // namespace Fog
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_FOG_EFFECTINTERFACE_H
