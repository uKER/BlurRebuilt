#ifndef SCENE_GRAPH_RENDERNODE_H
#define SCENE_GRAPH_RENDERNODE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
class C_DeviceContext;
}
namespace SceneGraph
{
class C_Context;

enum E_RenderNodeType
{
	E_RenderNodeType_Unknown = -1,
	
	// model instance nodes
	E_RenderNodeType_ModelInstance,
	E_RenderNodeType_ModelInstancePrimitive,

	// static renderlist nodes
	E_RenderNodeType_StaticPrimitive,
	E_RenderNodeType_StaticInstance,
	E_RenderNodeType_StaticInstanceBatch,

	// lights
	E_RenderNodeType_DynamicLight,

	E_RenderNodeType_Count
};

class C_VisibleNode;

// basic renderable node, collected into lists
class I_RenderNode
{
public:
	I_RenderNode();
	I_RenderNode(S32 type);
	virtual ~I_RenderNode();

	void Render(C_Context& context) const;
	virtual	void	DebugRender(const C_VisibleNode& node) const {};
	const S32 GetType() const { return m_Type; }
	
protected:

	// render this object for this render pass. don't care about the view
	virtual void OnRender(C_Context& context) const { assert(0); }

	S32		m_Type;
};

class C_PassCull
{
public:
	C_PassCull() : m_RenderPassFlags(0), m_RenderFlags(0xffffffff) {}
	~C_PassCull() {}

	void		EnableRenderPass(const U32 renderPassFlag);
	void		DisableRenderPass(const U32 renderPassFlag);

	void		EnableRenderFlag(const U32 renderFlag);
	void		DisableRenderFlag(const U32 renderFlag);

	const U32	RenderPassEnabled(const U32 renderPassFlag) const;
	const U32	RenderPasses() const;
	const U32	RenderFlagEnabled(const U32 renderFlag) const;
	const U32	RenderFlags() const;

private:

	// which passes this node has renderables in
	U32			m_RenderPassFlags;
	U32			m_RenderFlags;

	// functionality flags
	U32			m_Flags;
};

class C_GatherCull
{
public:
	C_GatherCull() : m_VisibilityFlags(0xffffffff) {}
	~C_GatherCull() {}


	void		SetVisibilityFlags(U32 visibilityFlags) { m_VisibilityFlags = visibilityFlags;}
	const U32	GetVisibilityFlags() const { return m_VisibilityFlags; }

private:
	// gather mask
	U32			m_VisibilityFlags;
};

} // namespace SceneGraph

#include "RenderNode.inl"

#endif // SCENE_GRAPH_NODE_H
