#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef SCENEGRAPH_ALLOW_INCLUDE
#error "Illegal include of SceneGraph library internals.h"
#endif

#include <system/scheduler/jobqueue.h>
#if !RENDERING_SPU_HACK
#include "btl/containers/set.h"
#include "btl/containers/openhashmap.h"
#endif
#include <SceneGraph/Context.h>
#include <SceneGraph/SceneGraphMaxContexts.h>
#include <SceneGraph/RenderPass.h>
#include <SceneGraph/ListNode.h>
#include <SceneGraph/RenderFlags.h>
#include <SceneGraph/Enums.h>

namespace RenderingData
{
class MaterialList;
namespace Target
{
	class Scene;
	class RenderListNode;
	class Context;
	class MaterialList;
	class MaterialListDmaData;
}
}
namespace Portal
{
	class C_System;
};

namespace Display
{
	class C_View;
};

namespace Animation
{
	class C_PoseFormat;
}

namespace SceneGraph
{

class C_View;
class C_MultiView;
class I_Renderer;
class I_GatherNode;
class C_LevelInstance;
class C_ModelInstance;
class C_ModelInstanceFlatList;
class C_ModelInstancePortalList;
class C_LightManager;
class C_RenderListNode;
class I_Visitor;
class C_SceneGraph;
struct C_VisibleNodeList;
class C_Context;
class I_VisibleNodeListSorter;
class I_Renderer;
class C_VisibleDetect;
class C_RenderGroupSettings;
struct RenderStates;

typedef Btl::Vector<C_View*>			ViewList;

#if !RENDERING_SPU_HACK
// renderers
typedef Btl::Set<I_Renderer*>			Renderers;
#endif

#ifndef PLATFORM_SPU

// C_CachedStaticNodeMaterialParameter

struct ALIGN16 C_CachedStaticNodeMaterialParameter
{
	C_CachedStaticNodeMaterialParameter():
		index(-1),
		offset(-1),
		size(0)
	{}
	
	S16 index;
	S16 offset;
	U16	size;
};

// C_CachedStaticNodeMaterialData

struct C_CachedStaticNodeMaterialData
{
	Btl::Vector<U8*, Btl::VectorParameters_Static<24> >	m_Params;
};

#endif // PLATFORM_SPU

// C_CachedStaticNodeInternalMaterialData

class ALIGN16 C_CachedStaticNodeInternalMaterialData
{
public:
	C_CachedStaticNodeInternalMaterialData();
	~C_CachedStaticNodeInternalMaterialData();

	void	ClearPaletteMap();

	RenderingData::Target::MaterialList*		m_pMaterialList;
	const void*									m_pMaterialListDmaData;

	U32											m_MaterialListSize;
	const U8*									m_pPaletteMap;

#ifndef PLATFORM_SPU
	Btl::Vector<C_CachedStaticNodeMaterialData>	m_MaterialDataList;

	Btl::Vector<S32, Btl::VectorParameters_Static<24> >
												m_AnimatedParamIndices;
	Btl::Vector<U16, Btl::VectorParameters_Static<24> >
												m_PoseFieldIndexes;
#endif
};

//! C_SceneGraph

class C_SceneGraph
{
public:

	C_SceneGraph();
	virtual ~C_SceneGraph();

	// view configuration
	C_View*						AddView();
	void						RemoveView(C_View* pView);

	// render passes - this needs some more thought
	void						SetNumRenderPasses(const S32 renderPasses);
	void						RegisterRenderPass(const S32 pass, I_VisibleNodeListSorter* pSorter = NULL, U32 subTechniqueHandle = 0, U32 subTechniqueMask = 0);
	
	S32							GetNumRenderPasses() const { return m_RenderPasses.Size(); }
	
	// set which of the rendering data passes correspond to which scenegraph render group for the static scene
	void						SetRenderingDataRenderPassMapping(const U32 renderPass, const S32 passType);
	S32							GetRenderingDataRenderPassMapping(const U32 renderPass) const;

	const C_RenderPass*			GetRenderPass(const S32 passType) const;
	C_RenderPass*				GetRenderPass(const S32 passType);

	// insert rendering data scenes
	C_LevelInstance*			Insert(C_LevelInstance *pScene);
	void						Remove(C_LevelInstance* pScene);

	// model instance nodes
	void						Insert(C_ModelInstance* pModelInstance);
	void						Remove(C_ModelInstance* pModelInstance);

	// generic nodes
	void						Insert(I_GatherNode* pGatherNode);
	void						Remove(I_GatherNode* pGatherNode);

#if !RENDERING_SPU_HACK
	// node rendering
	const I_Renderer*			GetRenderer(const S32 nodeType) const;
	void						ReserveModelInstanceFlatLists(U32 iCount);
	void						Accept(I_Visitor& visitor) const;
#endif

	// rendering
	void						BeginFrame() const;
	void						EndFrame() const;

	// render flags
	RenderFlags						GetRenderFlags() const { return m_RenderFlags; }
	void							SetRenderFlags(const RenderFlags renderFlags) { m_RenderFlags = renderFlags; }
	const C_RenderGroupSettings*	GetRenderGroupSettings() const { return m_pDefaultRenderGroupSettings; }

	// lights
	C_LightManager*				GetLightManager() { return m_pLightManager; }
	const C_LightManager*		GetLightManager() const { return m_pLightManager; }

	C_Context*					GetContext() const;

	C_Context*					GetContextList() const { return &m_Contexts[0]; }

	// get a pointer to the newly visible list
	const Btl::Vector<C_VisibleDetect*> &
								GetNewlyVisibleList() const { return m_NewlyVisibleList; }
	U32							GetFrame() const { return m_Frame; }
	
	// scheduler related stuff
	void						SetJobQueuePolicyGather(Scheduler::PolicyHandle policyHandle);
	void						SetJobQueuePolicyRender(Scheduler::PolicyHandle lowHandle, Scheduler::PolicyHandle medHandle, Scheduler::PolicyHandle highHandle);
	void						SetJobQueuePolicyRender(Scheduler::PolicyHandle handle);
	void						SetJobQueueInitialised();

	/// Set the list type to use for gathering.
	void						SetGatherListType(I_ListNode::ListNodeType type)
	{
		m_ListType = type;
		return;
	}
	/// Add a portal system to the scene graph.
	void						AddPortalSystem(const Portal::C_System* pSystem);
	/// Return the number of portal system objects in the scene graph.
	NOALIAS const U32			GetPortalSystemCount() const __restrict;
	/// Return a pointer to the list of portal system objects.
	const Btl::Vector<const Portal::C_System*>&	
								GetPortalSystemList() const;

	// TODO: [rthomas]	Crowd stuff, tidy this up
	/// Used by animated static render list nodes
	void						SetPoseFormat(const Animation::C_PoseFormat* pPoseFormat);
	const Animation::C_PoseFormat*
								GetPoseFormat() const;

	void						SetNumMatrixPalettes(const U32 numMatrixPalettes);
	void						ClearMatrixPalettes(const U32 offset);
	void						SetMatrixPalette(const S32 index, const Maths::Mat43* pMatrixPalette);

	const U32					GetNumMatrixPalettes() const;
	const Maths::Mat43*			GetMatrixPalette(const S32 index) const;

	void						SetPaletteMap(const U8* pPaletteMap) const;

	// TODO: [rthomas]	Get rid of this
	void						GetDynamicMaterialList(const RenderingData::Target::MaterialList** ppOutMaterialList,
									const RenderingData::Target::MaterialList** ppOutMaterialListDmaData, const S16 id, S32 &offset, S32 &count) const;

	const C_CachedStaticNodeInternalMaterialData*
								GetStaticNodeInternalMaterialData() const { return m_pCachedStaticNodeInternalMaterialData; }
	void						SetStaticNodeInternalMaterialData(C_CachedStaticNodeInternalMaterialData* pInternalData) const;

	/// Event that indicates a model instance has moved.
	void						OnMove(C_ModelInstance* pModelInstance) const;

	bool						JobQueueActive() const;
	//Scheduler::GroupHandle		GetJobQueueGroup(U32 index) const { return m_JobQueueGroup[index]; }
	Scheduler::PolicyHandle		GetJobQueuePolicyGather() const { return m_JobQueuePolicyGather; }
	Scheduler::PolicyHandle		GetJobQueuePolicyRender(E_RenderPriority renderPriority = E_RenderPriority_Medium) const { return m_JobQueuePolicyRender[renderPriority]; }

	void						SetSharedParameters(const Display::C_View& view) const;
	void						SetSharedParameters(const C_View& view) const;

	void						Gather(const C_View& view) const;
	void						Gather(const C_MultiView& multiView) const;

	friend class I_Renderer;
	friend class C_View;
	friend class C_MultiView;	
	friend class C_ModelInstance;
	friend class C_Light;

private:

	// add an entry into the newly visible list
	void						AddNewlyVisible(C_VisibleDetect *p) const { m_NewlyVisibleList.PushBack(p); }


	void						BeginRender(const Display::C_View& view) const;
	
	void						AddRendererViews(C_View* pView);
	void						RemoveRendererViews(C_View* pView);

	// TODO: [rthomas]	Get rid of this
#ifndef PLATFORM_SPU
	void						GetParameters(const char* pName, Btl::Vector<C_CachedStaticNodeMaterialParameter> & materialParameters,
									const RenderingData::Target::MaterialList* pMaterialList, const U32 numPalettes) const;
	void						BindAnimatedParams(const Animation::C_PoseFormat* pPoseFormat, const U32 numMatrixPalettes) const;
	void						SetJointTransformParameters(const Btl::Vector<U16, Btl::VectorParameters_Static<24> > &poseFieldIndices, const U32 poseFieldIndexCount,
									U8** __restrict ppParams, const Maths::Mat43* __restrict pMatrixPalette) const;
#endif
	
	// render passes (keep this at the top - it needs to be 16 byte aligned)
	Btl::Vector<C_RenderPass> ALIGN16			m_RenderPasses;

	// this also needs to be aligned, so putting under render passes for time being
	mutable C_CachedStaticNodeInternalMaterialData*
												m_pCachedStaticNodeInternalMaterialData;

	Btl::Vector<S32, Btl::VectorParameters_Array<6> >	m_RenderingDataRenderPassMapping;

	// static scenes - special implementation of RenderingData::Scene types for static level
	Btl::Vector<const C_LevelInstance*>			m_Scenes;

	// models
	C_ModelInstanceFlatList*					m_pModelInstanceFlatList;
	C_ModelInstancePortalList*					m_pModelInstancePortalList;

	// lights
	C_LightManager*								m_pLightManager;					

	// generic hierarchy, used for other stuff, skydomes, global effects, anything which isn't scene or dynamic object
	Btl::Vector<const I_GatherNode*>			m_GenericNodes;

	// views
	ViewList									m_Views;

	// list of the custom data of all the objects that have become visible this frame
	mutable Btl::Vector<C_VisibleDetect*>		m_NewlyVisibleList;

	// The type of list to use for gathering model instances.
	I_ListNode::ListNodeType					m_ListType;

	// rendering contexts. one for each worker thread
	mutable Btl::Vector<C_Context, Btl::VectorParameters_Array<SCENEGRAPH_MAX_CONTEXTS> >
												m_Contexts;
	// list of portal systems that are in use throughout the worldview scenes
	mutable Btl::Vector<const Portal::C_System*>
												m_vPortalSystems;

	// render flags
	RenderFlags									m_RenderFlags;

#if !RENDERING_SPU_HACK
	// renderers
	void										AddRenderer(I_Renderer* pRenderer);
	void										RemoveRenderer(I_Renderer* pRenderer);
	Renderers									m_Renderers;
	Btl::Vector<const I_Renderer*>				m_NodeTypeRenderer;
#endif

#if !RENDERING_SPU_HACK
	// pose format used by animated static render list nodes
	const Animation::C_PoseFormat*				m_pPoseFormat;

	// matrix palettes used by static render list nodes
	Btl::Vector<const Maths::Mat43*>			m_MatrixPalettes;
	U32											m_NumMatrixPalettes;
#endif

	// scheduler stuff

	Scheduler::PolicyHandle						m_JobQueuePolicyGather;
	Scheduler::PolicyHandle						m_JobQueuePolicyRender[E_RenderPriority_Count];
	bool										m_JobQueueInitialised;

	RenderStates*								m_pDefaultRenderStates;
	C_RenderGroupSettings*						m_pDefaultRenderGroupSettings;

	U32											m_WorldParam;
	U32											m_ViewParam;
	U32											m_ProjParam;
	U32											m_ViewProjParam;
	U32											m_ViewPositionParam;
	U32											m_LodDistanceScale;
	mutable U32									m_Frame;
};

} // namespace SceneGraph

#include "SceneGraph/SceneGraphImpl.inl"

#endif // SCENE_GRAPH_H
