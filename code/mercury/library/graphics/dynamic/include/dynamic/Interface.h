#ifndef RENDER_DYNAMICINTERFACE_H
#define RENDER_DYNAMICINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "DebugInfo.h"
#include "Dynamic/Enums.h"
#include "Dynamic/RenderNode.h"
#include <core\containers\bitvector.h>

namespace Display
{
	class C_Context;
}

namespace SceneGraph
{
	class C_View;
}

namespace DynamicEffect
{

class C_Renderer;
class C_Node;
class C_View;
class I_DynamicEffectManager;
struct C_CompositeRenderNode;
struct BlockArrays;


struct RenderStates
{
	F32								fDepthBias;
};

// [TODO SCENEGRAPH] derive off SceneGraph::I_Gather
class I_DynamicEffect
{
public:
	I_DynamicEffect(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex);
	virtual ~I_DynamicEffect();

	// display for non-managed effects
	void							Render() const;
	
	bool							Begin() const { return m_bBegin; }
	bool							AlwaysUpdate() const { return m_bAlwaysUpdate; }
	S32								GetNumBlocks() const { return m_nBlocks; }
	S32								GetNumBlocksLocal() const { return m_nBlocksLocal; }
	S32								GetNumBlocksWorld() const { return m_nBlocks - m_nBlocksLocal; }
	U32								GetIndex() const { return m_iIndex; }
	U32								GetTypeIndex() const { return m_iTypeIndex; }
	const C_Renderer*				GetRenderer() const { return m_pRenderer; }
	const I_DynamicEffectManager*	GetManagerInterface() const { return m_pManagerInterface; }
	const RenderStates&				GetRenderStates() const { return m_RenderStates; }
	friend class 					C_Renderer;
protected:
	// pure virtuals
	virtual void				Update() {};
	virtual void				Reset() = 0;
	
	// culling and gathering functions for managed effects
	// only accessible to dynanamic effect renderer

	void						Gather(const SceneGraph::C_View& view) const;
	void						Build(const C_View& view, C_CompositeRenderNode& batch) const;
	void						Render(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& batch, const U32 states) const;
	MathsStorage::Int2			DisplayDebug(MathsStorage::Int2 position) const;
	void						BindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pDisplay) const;
	
	virtual bool				Create() { return true; }
	virtual bool				CreateVolatile() { return true; }
	virtual bool				Initialise() { return true; }
	virtual void				Destroy() {}
	virtual void				DestroyVolatile() {}
	virtual void				Shutdown() {}

	virtual void				Clean(const C_Node* pNode) {};
	virtual void				Clean(const Display::C_Texture2D* pTexture) {};

	virtual void				BeginView() {};
	virtual void				BeginFrame() {};

	// culling and gathering functions for managed effects
	virtual void				OnGather(const SceneGraph::C_View& view) const = 0;
	virtual void				OnBuild(const C_View& view, C_CompositeRenderNode& batch) const = 0;
	virtual void				OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& batch, const U32 states) const = 0;
	virtual MathsStorage::Int2	OnRenderDebug(MathsStorage::Int2 position) const { return position; }
	virtual void				OnBindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pDisplay) const {}

	// version of display for non-managed effects
	virtual void				OnRender() const { return void(); }

	// utility functions
	void						SetWorldMatrix(const Display::C_Matrix* pMatrix) const;

	RenderStates				m_RenderStates;

	C_Renderer*					m_pRenderer;
	I_DynamicEffectManager*		m_pManagerInterface;		
	U32							m_iIndex;
	U32							m_iTypeIndex;	// index of type of class, eg Shape2DSimple, SpriteRotating, Shape3DSimpleSuper, etc 
	
	S32							m_nBlocks;		// number of active blocks
	S32							m_nBlocksLocal;	// number of blocks which are in local space (external transformations)
	bool						m_bBegin;		// whether begin view and begin frame need to be called on this object
	bool						m_bAlwaysUpdate;	// whether update should be called even if no blocks exist
};

INLINE void I_DynamicEffect::Render() const
{
	if(m_nBlocks > 0)
		OnRender();
}

INLINE void	I_DynamicEffect::Gather(const SceneGraph::C_View& view) const
{
	if(m_nBlocks > 0)
		OnGather(view);
}

INLINE void I_DynamicEffect::Build(const C_View& view, C_CompositeRenderNode& node) const
{
	if(m_nBlocks > 0)
		OnBuild(view, node);
}

INLINE void I_DynamicEffect::Render(const C_View& view, Display::C_Context* pContext, const C_CompositeRenderNode& batch, const U32 states) const
{
	if(m_nBlocks > 0)
		OnRender(view, pContext, batch, states);
}

INLINE MathsStorage::Int2 I_DynamicEffect::DisplayDebug(MathsStorage::Int2 position) const
{
	if(m_nBlocks > 0)
		return OnRenderDebug(position);
	else
		return position;
}

INLINE void I_DynamicEffect::BindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pDisplay) const
{
	OnBindPixelConstants(view, pDisplay);
}

}

#endif // RENDER_DYNAMICINTERFACE_H
