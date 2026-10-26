#ifndef SCENE_GRAPH_MODELINSTANCEFLATLIST_H
#define SCENE_GRAPH_MODELINSTANCEFLATLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SceneGraph/GatherNode.h"
#include "SceneGraph/RenderingData/ModelInstanceFlatListGather.h"
#include "SceneGraph/ListNode.h"

namespace SceneGraph
{
	class C_ModelInstance;
	class C_ModelInstanceBasicCullNode;

	class C_ModelInstanceFlatList :	public I_ListNode
	{
	public:
		C_ModelInstanceFlatList(const C_SceneGraph* pSceneGraph);
		~C_ModelInstanceFlatList();

		// I_GatherNode methods
		void	Gather(const C_View& view) const;
		void	Gather(const C_MultiView& multiView) const;
		void	Accept(I_Visitor& visitor) const;
		
		// I_ListNode methods.
		NOALIAS const ListNodeType	GetType() const __restrict
		{ return I_ListNode::LISTNODETYPE_FLAT_LIST; }

		void	Insert(C_ModelInstance* pModelInstance);
		void	Remove(C_ModelInstance* pModelInstance, const bool bFreeCullNode = true);

		static void BeginFrame();
	
	private:

		void	OnGather(const C_View& view) const { Gather(view); }
		void	OnGather(const C_MultiView& multiView) const { Gather(multiView); }
		void	OnGatherDeferred(const void* pData) const;

		Btl::Vector<C_ModelInstanceBasicCullNode*>										m_CullNodes;
	};

} // namespace SceneGraph

#endif // SCENE_GRAPH_MODELINSTANCEFLATLIST_H
