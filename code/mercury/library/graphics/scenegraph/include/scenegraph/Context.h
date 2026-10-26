#ifndef SCENE_GRAPH_CONTEXT_H
#define SCENE_GRAPH_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Display2.h>

#if RENDERING_SPU_HACK
// TEMP HACK TO GET THE SPU RENDERING CODE COMPILING
#define FRIEND_SERIALISE(x)
#define VIRTUAL_SERIALISATION_KEY_DECL

#if 1
#include <Display.h>
#else
namespace Display
{
	struct C_IndexBuffer {};
	struct C_VertexBuffer {};
	struct C_Texture2D 
	{
		const void*& GetGPUTexture() const;
	};
	struct C_VertexDeclaration {};
	struct C_CommandBuffer {};
	struct C_PixelShaderParameter {};
	struct C_VertexShader {};
	struct C_PixelShader {};
	struct C_StencilState
	{
		U32 d1, d2, d3, d4, d5, d6, d7, d8;
	};
	struct C_Device 
	{
		C_View* GetView();
	};

	typedef int E_ColourWriteEnable;

	C_Device* Device();
}
#include <display/device/DeviceTypesEnums.h>
#endif

#else
#include <Display/Buffer/CommandBuffer.h>
#endif
#include <Display/Device/DeviceContext.h>
#include <SceneGraph/RenderFlags.h>
#include <Display.h>
#include <display/context/Context.h>

#include "btl/utility/pair.h"

namespace FX
{
class Context;
}

namespace RenderingData
{
namespace Target
{
	class Scene;
	class RenderListNode;
	class Context;
}
}

namespace Display
{
class C_Context;
}

namespace SceneGraph
{

class C_View;
class C_SceneGraph;
class C_RenderTask;
class C_StaticPrimitive;
class C_StaticInstance;
class C_ModelInstance;
class C_RenderPass;
struct C_VisibleNodeList;
struct RenderSubtask;
class C_StaticRenderListNode;

//! C_Context
class ALIGN16 C_Context : public Display::C_DeviceContext
{
public:
	C_Context();
	~C_Context();

	/// clone state of another context into this context
	void				CloneState(const C_Context *pContext);

	/// invalidate state
	void				InvalidateState();

	/// begin rendering to the given command buffer using this context
	void				Begin(Display::C_CommandBuffer *);

	/// finish rendering to the command buffer
	void				End();								

	/// this is the entry function for rendering a single RenderTask
	void				RenderTask(C_RenderTask *pTask);

	/// this is the function for rendering a single subtask
	void				RenderTask(const RenderSubtask *pTask, const U32 subtechnique, const U32 distanceSubtechnique, const bool multiThreaded);

	/// render a C_StaticPrimitive
	void				Render(const C_StaticPrimitive *pPrim, const U32 subtechnique);

	/// render a C_StaticInstance
	void				Render(const C_StaticInstance *pPrim, const U32 subtechnique);

	/// render a C_ModelInstance
	void				Render(const C_ModelInstance *pModel, const U32 subtechnique, const U32 typeFlag = 0xFFFFFFFF);

	/// called by C_Renderer
	U32					GetRenderPassIndex() const { return m_RenderPassIndex; }

	/// called by BlobShadows.cpp
	void				SetRenderPassIndex(const U32 renderPass, const C_RenderPass *pRenderPass);
#if !PLATFORM_SPU
	void				SetRenderPassIndex(U32 renderPass);
#endif
	void				SetView(const C_View* pView) { m_pView = pView; }
	const C_View*		GetView() const				 { return m_pView; }
	const C_SceneGraph*	GetSceneGraph() const		 { return m_pSceneGraph; }

	friend class C_SceneGraph;
	friend class C_View;

private:
	RenderFlags			GetRenderFlags() const { return m_RenderFlags; }
	U32					GetRenderPassFlag() const { return m_RenderPassFlag; }

	void				SetAbstract(bool a);
	void				RenderAbstracts(const C_VisibleNodeList* pVisibleNodeList, const S32 start, const S32 end, U32 subtechniqueHandle, U32 distanceSubtechniqueHandle, bool predication);
	void				RenderPrimitives(const C_VisibleNodeList* pVisibleNodeList, const S32 start, const S32 end, U32 subtechniqueHandle, U32 distanceSubtechniqueHandle, bool predication);
	void				RenderInstances(const C_VisibleNodeList* pVisibleNodeList, const S32 start, const S32 end, U32 subtechniqueHandle, U32 distanceSubtechniqueHandle, bool predication);
	void				RenderModelInstances(const C_VisibleNodeList* pVisibleNodeList, const S32 start, const S32 end, U32 subtechniqueHandle, U32 distanceSubtechniqueHandle, bool predication);

	void				BindModelInstance(const C_ModelInstance *pModel, U32 renderPass);
	void				BindRenderListNode(const C_StaticRenderListNode *pNode);

	// fixed state
	const C_SceneGraph*								m_pSceneGraph;
	Display::C_Context*								m_pDisplay;
	FX::Context*									m_pFX;
	RenderingData::Target::Context*					m_pRD;

	// variable state
	const C_StaticRenderListNode*					m_pRenderListNode;
	S32												m_RenderPassIndex;
	S32												m_RenderPassFlag;
	const C_View*									m_pView;
	const C_RenderPass*								m_pRenderPass;
	RenderFlags										m_RenderFlags;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_H
