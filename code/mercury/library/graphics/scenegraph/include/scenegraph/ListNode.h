#ifndef SCENE_GRAPH_LISTNODE_H
#define SCENE_GRAPH_LISTNODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/GatherNode.h>
#include <Btl/allocators/Pool.h>

namespace SceneGraph
{
	class C_SceneGraph;
	class I_Visitor;
	class C_View;
	class C_ModelInstance;

	/// Common interface for list classes.
	class I_ListNode : public I_GatherNode
	{
	public:
		/// Different types of list node.
		enum ListNodeType
		{
			LISTNODETYPE_FLAT_LIST,			// Standard frustum culled flat-list.
			LISTNODETYPE_PORTAL_LIST,		// Portal culled model instance lists.

			LISTNODETYPE_COUNT
		};

		/// Default c'tor.
		explicit I_ListNode();
		/// Specified c'tor.
		explicit I_ListNode(S32 iType, const C_SceneGraph* pSceneGraph);
		/// D'tor.
		virtual ~I_ListNode();

		/// Accept a visitor node.
		virtual void						Accept(I_Visitor& rkVisitor);
		/// Gather for a view.
		virtual void						Gather(const C_View& rkView) const;
		/// Return the type of the list node.
		virtual NOALIAS const ListNodeType	GetType() const __restrict = 0;
		/// Insert a model instance node into the list.
		virtual void						Insert(C_ModelInstance* pModelInstance) = 0;
		/// Flag a model instance node has moved.
		virtual void						OnMove(C_ModelInstance* pModelInstance); 
		/// Remove a model instance node from the list.
		virtual void						Remove(C_ModelInstance* pModelInstance, const bool bFreeCullNode = true) = 0;
	
	private:
		/// Begin frame.
		virtual void						OnBeginFrame() const;
	};
}


#endif // SCENE_GRAPH_LISTNODE_H
