#ifndef SCENE_GRAPH_VIEW_H
#define SCENE_GRAPH_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

//#ifndef SCENEGRAPH_ALLOW_INCLUDE
//#error "Illegal include of SceneGraph library internals.h"
//#endif

#include <SceneGraph/Visible/Nodes.h>
#include <SceneGraph/Visible/VolatileNodeInfo.h>
#include <SceneGraph/Enums.h>
#include <SceneGraph/ViewInterface.h>
#include <SceneGraph/RendererView.h>
#include <SceneGraph/ViewData.h>
#include <SceneGraph/Light/Light.h>

#include <SceneGraph/RenderTask.h>
#include <Btl/Containers/bitvector.h>

namespace Display
{
	struct C_TileSetup;
}

namespace SceneGraph
{

class C_SceneGraph;
class C_Context;
class I_Renderer;
class I_GatherNode;
class I_VisibleNodeListSorter;
struct RenderStates;

// flag converts from an index to a flag at compile time
// usage : Flag<E_View_Main>(), Flag<E_RenderGroup_Solid>()

// compile time
template<int I>
inline U32 Flag()
{
	BIZ_STATIC_ASSERT(I < 32);
	return 1 << I;
}

// runtime
inline U32 Flag(int i)
{
	return 1 << i;
}

class C_View;
class C_Context;

//! C_View
// view encompasses any renderable image of the scene, eg main view, cubemap faces, shadowmaps, reflections, mirrors, etc

class C_View : public I_View
{
public:
	void		SetViewFrustum(const Display::C_ViewFrustum& viewFrustum, const Display::C_TileSetup* pTileSetup = NULL) const;
	void		SetViewData(const Display::C_ViewSettings3D& viewSettings3D) const;
	void		SetFarClip(const F32 farClip) const;
	
	void		SetDrawDistanceScale(F32 drawDistanceScale) const;
	void		SetInstanceDrawDistanceScale(F32 instanceDrawDistanceScale) const;
	void		SetModelInstanceDrawDistanceScale(F32 modelInstanceDrawDistanceScale) const;
	void		SetType(const U32 type) const { m_ViewData.type = type; m_ViewData.typeFlag = 1 << type; }
	void		SetTypeFlag(const U32 type, const U32 typeflag) const { m_ViewData.type = type, m_ViewData.typeFlag = typeflag; }

	const ViewData&						GetViewData() const { return m_ViewData; }
	const Display::C_ViewFrustum&		GetViewFrustum() const { return m_ViewData.viewFrustum; }
	const Display::C_ViewSettings3D&	GetViewSettings3D() const { return m_ViewData.viewSettings3D; }
	const U32							GetType() const { return m_ViewData.type; }
	const U32							GetTypeFlag() const { return m_ViewData.typeFlag; }

	void		SetSubtechniqueHandle(const U32 handle);
	U32			GetSubtechniqueHandle() const { return m_ConstantSubtechnique; }

	void		SetDistanceSubtechniqueHandle(const bool enable, const F32 distance = 1.0f, const U32 handle = 0);
	U32			GetDistanceSubtechniqueHandle() const { return m_DistanceSubtechnique; }

	void		SetRenderPassSorter(const U32 renderPass, const I_VisibleNodeListSorter* pVisibleNodeListSorter);

	// debug only showing certain lists
	const bool	RenderListEnabled() const;

	void		AddVisibleNode(const I_RenderNode* pRenderNode, const U32 renderPassType, const U32 predication = 0, const U32 volatileInfo = 0, const U32 screenSize = 0) const;
	void		AddVisibleNode(const C_StaticPrimitive* pStaticPrimitive, const U32 renderPassType, const U32 predication = 0, const U32 volatileInfo = 0, const U32 screenSize = 0) const;
	void		AddVisibleNode(const C_StaticInstance* pStaticInstance, const U32 renderPassType, const U32 predication = 0, const U32 volatileInfo = 0, const U32 screenSize = 0) const;
	void		AddVisibleNode(const C_ModelInstance* pModelInstance, const U32 renderPassType, const U32 predication = 0, const U32 volatileInfo = 0, const U32 screenSize = 0) const;
	
	C_VisibleNodes*	GetVisiblePrimitives(const U32 renderPassType) const;
	C_VisibleNodes*	GetVisibleInstances(const U32 renderPassType) const;
	C_VisibleNodes*	GetVisibleModelInstances(const U32 renderPassType) const;

	void		AddVisiblePointLight(const C_VisiblePointLight& visibleLight) const;
	void		AddVisibleSpotLight(const C_VisibleSpotLight& visibleLight) const;
	void		AddVisibleDropShadow(const C_VisibleDropShadow& visibleLight) const;

	void		AddLightSphere(const MathsStorage::Float4& sphere) const;
	void		AddLightSphere(const MathsStorage::Float4A16& sphere) const;

	void		BeginGather() const;
	void		EndGather() const;

	void		Initialise();

	/// set the scheduler group to add tasks to
	C_RenderGroup*	BeginSchedule(const S32 groupIndex, const Display::C_View& view, const C_RenderGroupSettings* pSettings = NULL);
	void			SchedulePass(S32 iPass, const RenderStates* pStates = NULL);
	void			EndSchedule();

	/// render into the main command buffer
	void		RenderGroup(const S32 group = 0);
	void		ReserveGroups(const S32 groups);
	
	void		BeginRender() const;
	void		BeginRender(const Display::C_View& view) const;
	void		RenderPass(const S32 renderPass, const RenderStates* pRenderStates = NULL, U32 subtechnique = 0, U32 distanceSubtechnique = 0) const;
	void		EndRender() const;

	// debug rendering
	void		DebugRender() const;
	void		DebugRenderPass(U32 iPass) const;

#if !PLATFORM_SPU
	// todo: support a different cell index for multiple levels!
	void		SetVisiblePortalRoomSet(const Btl::BitVector& rkBitField) const { m_PortalBitField = rkBitField; }
	void		SetVisiblePortalRoomList(const Btl::Vector<U16>& rkVector) const { m_PortalRoomList = rkVector; } 

	C_VisibleNodeList*	GetVisibleNodeList(S32 renderPass) const
	{
		S32 pass = m_PassMapping[renderPass];
		return (pass == -1) ? NULL : &m_VisibleNodeLists[pass];
	}

	const C_VisiblePointLightList&	GetVisiblePointLightList() const { return m_VisiblePointLightList; }
	const C_VisibleSpotLightList&	GetVisibleSpotLightList() const { return m_VisibleSpotLightList; }
	const C_VisibleDropShadowList&	GetVisibleDropShadowsList() const { return m_VisibleDropShadowList; }

	const S32					GetIndex() const { return m_Index; }
	Btl::BitVector&				GetVisiblePortalRoomSet() const { return m_PortalBitField; }
	Btl::Vector<U16>&			GetVisibleRoomList() const { return m_PortalRoomList; }
	const Display::C_View*		GetDisplayView() const { return m_pDisplayView; }
	
	C_RenderGroup*				GetGroup() { return &m_Groups[m_GroupIndex]; }
	const C_RenderGroup*		GetGroup() const { return &m_Groups[m_GroupIndex]; }
	C_RenderGroup*				GetGroup(const U32 group) { return &m_Groups[group]; }
	const C_RenderGroup*		GetGroup(const U32 group) const { return &m_Groups[group]; }

	C_Context&					GetContext(const U32 group = 0) { return m_Groups[group].GetContext(); }
	const C_Context&			GetContext(const U32 group = 0) const { return m_Groups[group].GetContext(); }

	const C_RenderGroupSettings*	GetRenderGroupSettings() const { return m_pRenderGroupSettings; }

	void						EnableRenderPass(const U32 passFlag);
	void						DisableRenderPass(const U32 passFlag);

	void						EnableCullMethod(const U32 cullMethodFlag);
	void						DisableCullMethod(const U32 cullMethodFlag);
	const U32					CullMethodEnabled(const U32 cullMethodFlag) const;
	void						SetCullMethod(const U32 cullMethodFlags);

	const Maths::HMat43&		GetFutureCamera() const;
	void						SetFutureCamera(const Maths::HMat43& m);
	bool						UseFutureCamera() const {return m_useFutureCamera;}
#endif

	friend class C_SceneGraph;
	friend class C_RenderTask;
	friend class C_MultiView;

private:
	C_View(C_SceneGraph* pSceneGraph);
	~C_View();

	void		SetupLists();
	void		BeginFrame();
	void		EndFrame();

	// clears visible nodes, lights, light spheres, etc
	void		ClearNodes() const;
	void		RendererViewsBeginGather() const;
	// processing of lists after gathering
	void		GatherDeferredNodes() const;
	void		ProcessVisibleNodes(C_VisibleNodeList& visibleNodeList) const;
	void		SortVisibleNodes(C_VisibleNodeList& visibleNodeList) const;
	void		BuildNodeBatchLists(C_VisibleNodeList& visibleNodeList) const;
	void		BuildNodeBatches(C_VisibleNodeList& visibleNodeList) const;
	void		RemoveNullNodes(C_VisibleNodeList& visibleNodeList) const;

	mutable ViewData							m_ViewData;
	mutable Btl::Vector<LightSphere>			m_LightSpheres;

	
	// constant subtechnique always enabled for this view
	U32											m_ConstantSubtechnique;

	// distance subtechnique enabled based on node distance
	U32											m_DistanceSubtechnique;
	
#if !PLATFORM_SPU
	void				SetIndex(S32 index) { m_Index = index; }

	Maths::HMat43								m_futureCamera;

	// view information
	bool										m_Initialised;
	mutable S32									m_RenderListIndex;
	mutable bool								m_Rendering;
	
	// view type indentifier - should correspond to E_ViewPass
	S32											m_Index;

	// split rendering into various groups each with a different subtechique
	mutable const Display::C_View*				m_pDisplayView;
	
	bool										m_Scheduling;
	S32											m_GroupIndex;
	const C_RenderGroupSettings*				m_pRenderGroupSettings;
	Btl::Vector<C_RenderGroup>					m_Groups;
	
	// maps from render passes to elements of m_VisibleNodeLists
	Btl::Vector<S32>							m_PassMapping;

	// gathered stuff for this view
	mutable Btl::Vector<C_VisibleNodeList>		m_VisibleNodeLists;
	
	mutable C_VisiblePointLightList				m_VisiblePointLightList;
	mutable C_VisibleSpotLightList				m_VisibleSpotLightList;
	mutable C_VisibleDropShadowList				m_VisibleDropShadowList;

	// renderer views
	Btl::Vector<I_RendererView*>				m_RendererViews;
	mutable Btl::BitVector						m_PortalBitField;
	mutable Btl::Vector<U16>					m_PortalRoomList;

	bool m_useFutureCamera;
#endif
};

#if !PLATFORM_SPU
INLINE void C_View::EnableCullMethod(const U32 cullMethodFlag)
{
	m_ViewData.cullFlags |= cullMethodFlag;
}

INLINE void C_View::DisableCullMethod(const U32 cullMethodFlag)
{
	m_ViewData.cullFlags &= ~cullMethodFlag;
}

INLINE const U32 C_View::CullMethodEnabled(const U32 cullMethodFlag) const
{
	return m_ViewData.cullFlags & cullMethodFlag;
}

INLINE void C_View::AddVisibleNode(const C_StaticPrimitive* pStaticPrimitive, const U32 renderPassType, const U32 predication, const U32 volatileInfo, const U32 screenSize) const
{
	// if render node exists in any passes
	S32 i = m_PassMapping[renderPassType];
	
	if(i != -1)
	{
		C_VisibleNodeList& visibleNodeList = m_VisibleNodeLists[i];
		visibleNodeList.m_ScreenSize += screenSize;
		visibleNodeList.m_StaticPrimitives.PushBack();
		C_VisibleNode& node = visibleNodeList.m_StaticPrimitives.Back();
		node.m_pNode = (I_RenderNode*)pStaticPrimitive;
		node.m_VolatileNodeInfo.data = volatileInfo;
	}
}

INLINE void C_View::AddVisibleNode(const C_StaticInstance* pStaticInstance, const U32 renderPassType, const U32 predication, const U32 volatileInfo, const U32 screenSize) const
{
	// if render node exists in any passes
	S32 i = m_PassMapping[renderPassType];
	
	if(i != -1)
	{
		C_VisibleNodeList& visibleNodeList = m_VisibleNodeLists[i];
		visibleNodeList.m_ScreenSize += screenSize;
		visibleNodeList.m_StaticInstances.PushBack();
		C_VisibleNode& node = visibleNodeList.m_StaticInstances.Back();
		node.m_pNode = (I_RenderNode*)pStaticInstance;
		node.m_VolatileNodeInfo.data = volatileInfo;
	}
}

INLINE void C_View::AddVisibleNode(const C_ModelInstance* pModelInstance, const U32 renderPassType, const U32 predication, const U32 volatileInfo, const U32 screenSize) const
{
	// if render node exists in any passes
	S32 i = m_PassMapping[renderPassType];
	
	if(i != -1)
	{
		C_VisibleNodeList& visibleNodeList = m_VisibleNodeLists[i];
		visibleNodeList.m_ScreenSize += screenSize;
		visibleNodeList.m_ModelInstances.PushBack();
		C_VisibleNode& node = visibleNodeList.m_ModelInstances.Back();
		node.m_pNode = (I_RenderNode*)pModelInstance;
		node.m_VolatileNodeInfo.data = volatileInfo;
	}
}

INLINE C_VisibleNodes* C_View::GetVisiblePrimitives(const U32 renderPassType) const
{
	S32 i = m_PassMapping[renderPassType];
	
	if(i != -1)
		return &m_VisibleNodeLists[i].m_StaticPrimitives;
	else
		return NULL;
}

INLINE C_VisibleNodes* C_View::GetVisibleInstances(const U32 renderPassType) const
{
	S32 i = m_PassMapping[renderPassType];
	
	if(i != -1)
		return &m_VisibleNodeLists[i].m_StaticInstances;
	else
		return NULL;
}

INLINE C_VisibleNodes* C_View::GetVisibleModelInstances(const U32 renderPassType) const
{
	S32 i = m_PassMapping[renderPassType];
	
	if(i != -1)
		return &m_VisibleNodeLists[i].m_ModelInstances;
	else
		return NULL;
}

#endif

} // namespace SceneGraph

#endif // SCENE_GRAPH_VIEW_H
