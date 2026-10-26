#ifndef RENDER_DYNAMICEFFECT_MINIMAL_H
#define RENDER_DYNAMICEFFECT_MINIMAL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Interface.h"
#include "Dynamic/Block.h"

namespace Render
{
	class C_RenderState;
}

namespace SceneGraph
{
	class C_View;
}

namespace DynamicEffect
{

class C_MinimalObject : public C_RenderNode
{
public:
	C_MinimalObject() {};
	~C_MinimalObject() {};

	Maths::C_RGBA_F32		m_Colour;
	Display::C_Matrix		m_Transform;
	bool					m_Transparent;
};

class C_Minimal : public I_DynamicEffect
{
public:
	C_Minimal(C_Renderer* pRenderer, const U32 iIndex, const U32 iTypeIndex) : I_DynamicEffect(pRenderer, NULL, iIndex, iTypeIndex) {}
	virtual ~C_Minimal() {}

	bool						Create();
	void						Destroy();

	void						OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const;
	void						AddObject(const Maths::C_RGBA_F32& colour, const Maths::HMat43& transform, bool transparent);
	void						ClearObjects();
	const C_MinimalObject*		GetObject(U32 index) const;
	U32							GetObjectCount() const;
		
protected:

	void						Update() {};
	void						Reset() {};

	void						OnGather(const SceneGraph::C_View& view) const;
	void						OnBuild(const C_View& view, C_CompositeRenderNode& node) const;

	Ut::Vector<C_MinimalObject>	m_Objects;
	
	Display::C_PixelShader				m_PixelShader;
	Display::C_PixelShaderParameter		m_LuminanceParam;
	Display::C_VertexShader				m_VertexShader;
	const Render::C_RenderState*		m_pRenderState;
};

}

#endif // RENDER_DYNAMICEFFECT_MINIMAL_H
