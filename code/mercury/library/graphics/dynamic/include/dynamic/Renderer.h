#ifndef RENDER_DYNAMICEFFECT_RENDERER_H
#define RENDER_DYNAMICEFFECT_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

// core containers
#include <core\containers\radix.h>
#include <core\containers\bitvector.h>

// display device
#include "Display/Device/Device.h"
#include "Display/Context/SamplerUsageFlags.h"

// debug rendering
#include "Render/Debug/DebugRender.h"

// scene graph integration
#include "SceneGraph/GatherNode.h"
#include "SceneGraph/RendererInterface.h"
//#include "SceneGraph/View.h"

// dynamic effect stuff
#include "Dynamic/Defines.h"
#include "Dynamic/Light.h"
#include "Dynamic/Enums.h"
#include "Dynamic/DynamicVertexBuffer.h"

#include "Dynamic/View.h"
#include "Render/Utils/ListPool.h"

// jobqueue
#include <system/scheduler/jobqueue.h>

#include <Display/context/SamplerState.h>

namespace Render
{
	class C_RenderState;
}

namespace SceneGraph
{
	class C_View;
}

namespace Display
{
	class C_Context;
}

namespace DynamicEffect
{

class I_DynamicEffect;
class I_DynamicEffectManager;
class C_Node;

typedef Display::C_RefTexture	C_Texture;

enum E_States
{
	// bind states needed for this effect which are constant across all rendered blocks
	E_States_BindEffect				= 0x00000001,
	E_States_UnbindEffect			= 0x00000002,

	// bind states which are specific to certain blocks (eg lights, shadows, fade in/out, etc)
	E_States_BindEffectVolatile		= 0x00000004,
	E_States_UnbindEffectVolatile	= 0x00000008,
	
	E_States_BindManager			= 0x00000010,
	E_States_UnbindManager			= 0x00000020,
	
	E_States_Default				= 0xFFFFFFFF
};
	
struct EffectSortInfo
{
	EffectSortInfo() {}
	EffectSortInfo(I_DynamicEffect* effect, U32 sort) : sortKey(sort), pEffect(effect) {}

	U32 sortKey;
	I_DynamicEffect* pEffect;
};

inline U32 RadixKey(const EffectSortInfo& info)
{
	return info.sortKey;
}

typedef Ut::UtVector<Ut::DynamicVector<EffectSortInfo> > EffectArray;

// use sets and maps to track objects
class Textures : public Ut::UtMap<Util::C_String, C_Texture*>
{
};
typedef Ut::UtMap<U32, I_DynamicEffect*>		Effects;
typedef Ut::UtSet<I_DynamicEffectManager*>		Managers;
typedef Ut::Vector<C_Light>						Lights;

// renderer class manages sorting, batching and rendering for all effects etc
class C_Renderer : public Display::I_Renderer, public SceneGraph::I_GatherNode, public SceneGraph::T_Renderer<C_View>
{
public:
	C_Renderer(SceneGraph::C_SceneGraph* pSceneGraph);
	~C_Renderer();

	bool							OnCreate();
	bool							OnCreateVolatile();
	bool							OnInitialise();
	void							OnDestroy();
	void							OnDestroyVolatile();
	void							OnShutdown();

	void							ReserveVertexBuffers(const U32 bufferSize, const U32 bufferCount, const U32 blocksCount = 128);
	void							SetExternalTextures(const Textures* pExternalTextures) { m_pExternalTextures = pExternalTextures; }

	// scenegraph I_Renderer integration
	void							SetRenderPassMapping(E_RenderPassIndex eRenderPassIndex, const S32 passType);
	S32								GetRenderPassMapping(E_RenderPassIndex eRenderPassIndex) const;
	S32								GetInverseRenderPassMapping(S32 renderPass) const;
	void							BuildRenderPasses();
	
	void							BeginFrame() const;			// called once at start of frame
	void							Build(const SceneGraph::C_View& view, const S32 renderPass, SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const;
	void							EndBuild(const SceneGraph::C_View& view, const S32 renderPass) const;
	void							Render(const SceneGraph::C_View& view, SceneGraph::C_Context& context, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const;
	
	// update
	void							Update(F32 fGameTime, F32 fFrameTime, bool bPaused);
	
	// reset all active dynamic effects
	void							OnReset();

	void							SetBlendMode(Display::C_Context* pContext, const E_BlendMode blendMode) const;

	void							SetSRGB(bool bSRGB) { m_bSRGB = bSRGB; }
	bool							GetSRGB() const { return m_bSRGB; }

	F32								GetTime() const { return m_fTime; }
	F32								GetFrameTime() const { return m_fFrameTime; }
	F32								GetFrameRate() const { return m_fFrameRate; }
	F32								GetFrameTimeRender() const { return m_fFrameTimeRender; }
	F32								GetFrameTimeAdjustment() const { return m_fFrameTimeAdjustment; }
	const MathsStorage::Float4A16&	GetTiming() const { return m_vTiming; }
	bool							Paused() const { return m_bPaused; }

    // debug stuff
	U32								m_nVisibleBlocks;
	U32								m_nDisplayBatches;
	U32								m_nBuildBatches;
	U32								m_nElements;
	U32								m_nBlocks;
	U32								m_nBlocksLocal;

	// light management - to be moved to scenegraph
	void							AddLight(const C_Light& light) { m_Lights.PushBack(light); }
	void							SetLightConstants1(Display::C_Context* pDisplay, U32 light) const;
	void							SetLightConstants4(Display::C_Context* pDisplay, U32 lights) const;
	const Lights&					GetLights() const { return m_Lights; }

	U32								GetLightMask(const Maths::C_Sphere& blockSphere) const;
	const C_Texture&				GetWhiteTexture() const { return m_WhiteTexture; }
	const C_Texture&				GetGreyTexture() const { return m_GreyTexture; }

	friend class I_DynamicEffect;
	friend class I_DynamicEffectManager;
	friend class C_SceneNode;
	
	C_Texture*						LoadTexture(const Util::C_String& name);
	U32								RemoveTextures();
	
	I_DynamicEffect*				FindEffect(const U32 id);
	void							CleanNode(const C_Node* pNode);
	void							CleanEffect(const I_DynamicEffect* pEffect);

	C_DynamicVertexBuffer*			GetDynamicVertexBuffer() const { return &m_DynamicVertexBuffer; }
	
	bool							JobQueueActive() const;
	Scheduler::PolicyHandle			GetJobQueuePolicyHandle() const { return m_JobQueuePolicyHandle; }
	void							SetJobQueuePolicyHandle(Scheduler::PolicyHandle jobQueuePolicyHandle) { m_JobQueuePolicyHandle = jobQueuePolicyHandle; }

	// manage dynamic effects
	void							AddEffect(I_DynamicEffect* pEffect);
	void							RemoveEffect(I_DynamicEffect* pEffect);

	void							AddBindPixelConstantsEffect(I_DynamicEffect* pEffect);
	void							RemoveBindPixelConstantsEffect(I_DynamicEffect* pEffect);

	void							BindVertexConstants(const SceneGraph::C_View& view, Display::C_Context* pDisplay) const;
	void							BindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pDisplay) const;
protected:
	

	void							AddManager(I_DynamicEffectManager* pEffect);
	void							RemoveManager(I_DynamicEffectManager* pEffect);

	// scene graph I_GatherNode integration
	void							Accept(SceneGraph::I_Visitor& visitor) const;
	void							OnGather(const SceneGraph::C_View& view) const;
	void							OnInsert(SceneGraph::C_SceneGraph& sceneGraph);
	void							OnRemove(SceneGraph::C_SceneGraph& sceneGraph);

	// general states
	void							Bind(Display::C_Context* pDisplay) const;
	void							Unbind(Display::C_Context* pDisplay) const;

	// pass specific states
	void							BindOpaque(Display::C_Context* pDisplay) const;
	void							BindDecal(Display::C_Context* pDisplay) const;
	void							BindTransparent(Display::C_Context* pDisplay) const;
	void							BindTwoDimensional(Display::C_Context* pDisplay) const;
	void							UnbindOpaque(Display::C_Context* pDisplay) const;
	void							UnbindDecal(Display::C_Context* pDisplay) const;
	void							UnbindTransparent(Display::C_Context* pDisplay) const;
	void							UnbindTwoDimensional(Display::C_Context* pDisplay) const;

	void							Render(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode* pBatch, const U32 states) const;
	void							FlushPass(const SceneGraph::C_View& view, S32 renderPass);

	const Textures*					m_pExternalTextures;
	Textures						m_Textures;
	Effects							m_Effects;
	EffectArray						m_EffectsSorted;
	Managers						m_Managers;
	Btl::Vector<I_DynamicEffect*>	m_BindPixelConstantsEffects;

	// maps to and from dynamic effect render passes to scene graph passes
	Btl::Vector<S32>														m_SgToDePassMapping;
	Btl::Vector<S32, Btl::VectorParameters_Array<E_RenderPassIndex_Count> >	m_DeToSgPassMapping;
	SceneGraph::C_PassCull			m_PassCull;

	// [TODO SCENEGRAPH]: move these into scenegraph
	mutable Lights					m_Lights;
	
	Ut::RadixSort<EffectArray>		m_EffectSorter;

	// mutable stuff which changes during rendering
	mutable C_DynamicVertexBuffer	m_DynamicVertexBuffer;
	U32								m_BufferSize;
	U32								m_BufferCount;
	U32								m_BlocksCount;
	
	// scheduler stuff
	Scheduler::PolicyHandle			m_JobQueuePolicyHandle;

	C_Texture						m_WhiteTexture;
	C_Texture						m_GreyTexture;

	F32								m_fCycle;
	F32								m_fTime;
	F32								m_fFrameTime;
	F32								m_fFrameRate;
	F32								m_fFrameTimeRender;
	F32								m_fFrameTimeAdjustment;

	MathsStorage::Float4A16			m_vTiming;
	bool							m_bPaused;
	bool							m_bSRGB;
	Display::C_SamplerUsageFlags	m_SamplerUsage;

	const Render::C_RenderState*	m_pRenderState;

	Display::C_SamplerState			m_DiffuseSamplerState;
	Display::C_SamplerState			m_NormalSamplerState;
	Display::C_SamplerState			m_DepthSamplerState;


	mutable	bool					m_bIdentity;
};

void GetLodFade(MathsStorage::Float2& lodFade, const F32 drawDistance, const F32 fadeDistance = 0.0f);

}

#endif // RENDER_DYNAMICEFFECT_RENDERER_H
