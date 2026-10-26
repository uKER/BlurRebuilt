#ifndef RENDER_DYNAMICEFFECT_MANAGER_INTERFACE_H
#define RENDER_DYNAMICEFFECT_MANAGER_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

// scene graph integration
#include "SceneGraph/GatherNode.h"

namespace Display
{
	class C_Context;
}

namespace SceneGraph
{
	class C_View;
	class C_MultiView;
	class I_Visitor;
}

namespace DynamicEffect
{

class C_Renderer;
class C_Node;
class I_DynamicEffect;

// dynamic effect managers can have objects which exist in the scene graph
class I_DynamicEffectManager : public SceneGraph::I_GatherNode
{
public:
	I_DynamicEffectManager(C_Renderer* pRenderer, const U32 iTypeIndex);
	virtual ~I_DynamicEffectManager();

	friend class 				C_Renderer;
	U32							GetTypeIndex() const { return m_iTypeIndex; }

protected:
	
	// pure virtuals
	virtual void				CleanNode(const C_Node* pNode) = 0;
	virtual void				CleanEffect(const I_DynamicEffect* pEffect) = 0;

	// normal virtuals
	virtual bool				Create() { return true; }
	virtual bool				CreateVolatile() { return true; }
	virtual bool				Initialise() { return true; }
	virtual void				Destroy() {}
	virtual void				DestroyVolatile() {}
	virtual void				Shutdown() {}

	virtual void				Reset() {}

	virtual void				BindVertexConstants(const SceneGraph::C_View& view, Display::C_Context* pContext) const {};
	virtual void				BindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pContext) const {};

	virtual void				BeginFrame() const {};

	// scene graph gather node
	virtual void				OnGather(const SceneGraph::C_View& view) const {}
	virtual void				OnGather(const SceneGraph::C_MultiView& view) const {}
	virtual void				Accept(SceneGraph::I_Visitor& visitor) const {}

	// 3D debug graphics
	virtual void				DisplayDebugGraphics() const {};
	// 2D overlays etc
	virtual MathsStorage::Int2	DisplayDebugInfo(MathsStorage::Int2 position) const { return position; }

	virtual void				Update(const F32 fFrameTime, const F32 fFrameRate) {};

	C_Renderer*					m_pRenderer;
	U32							m_iTypeIndex;
};


}

#endif // RENDER_DYNAMICEFFECT_MANAGER_INTERFACE_H
