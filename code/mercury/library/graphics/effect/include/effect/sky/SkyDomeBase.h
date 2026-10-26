#ifndef EFFECT_SKY_SKYDOMEBASE_H
#define EFFECT_SKY_SKYDOMEBASE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <SceneGraph/RenderNode.h>
#include <SceneGraph/GatherNode.h>

namespace Display
{
class C_DeviceContext;
class C_Context;
}

namespace SceneGraph
{
class C_View;
}

namespace Effect
{
namespace Sky
{

class C_SkyDomeBase : public Display::I_Renderer, public SceneGraph::I_RenderNode, public SceneGraph::I_GatherNode
{
public:

	C_SkyDomeBase();
	virtual ~C_SkyDomeBase();

	virtual bool OnCreate();
	virtual bool OnCreateVolatile();
	virtual void OnDestroy();
	virtual void OnDestroyVolatile();

	void Render(Display::C_Context* pContext) const;
	void Render() const;

	void OnRender(SceneGraph::C_Context& context) const;
	void OnGather(const SceneGraph::C_View& view) const;
	void Accept(SceneGraph::I_Visitor& visitor) const;
	void SetRenderPass(const U32 renderPass);

	virtual void Bind(Display::C_Context* pContext) const;
	virtual void Unbind(Display::C_Context* pContext) const;

public:

	void SetVertexShader(const Display::C_VertexShader* pVertexShader) const	{ m_pVertexShader = pVertexShader; }
	void SetPixelShader(const Display::C_PixelShader* pPixelShader) const		{ m_pPixelShader = pPixelShader; }
	void SetVertexShader() const												{ m_pVertexShader = &m_VertexShader; }
	void SetPixelShader() const													{ m_pPixelShader = &m_PixelShader; }

private:

	bool BuildSphere();
	bool BuildHemisphere();

protected:

	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShader;

private:

	mutable const Display::C_VertexShader*	m_pVertexShader;
	mutable const Display::C_PixelShader*	m_pPixelShader;

	Display::C_VertexBuffer			m_VertexBuffer;
	Display::C_IndexBuffer			m_IndexBuffer;
	U32								m_VertexCount;
	U32								m_IndexCount;
	U32								m_RenderPass;

};

}; // namespace Sky
}; // namespace Effect

#endif
