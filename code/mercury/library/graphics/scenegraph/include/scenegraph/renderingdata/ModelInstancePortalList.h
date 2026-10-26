#ifndef SCENE_GRAPH_MODELINSTANCEPORTALLIST_H
#define SCENE_GRAPH_MODELINSTANCEPORTALLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Allocators/pool.h>
#include <SceneGraph/ListNode.h>
#include <SceneGraph/RenderingData/PortalCullNodeClassify.h>
#include <SceneGraph/RenderingData/ModelInstanceFlatList.h>


namespace SceneGraph
{
	// Call these forward.
	class C_ModelInstanceNode;
	class C_ModelInstanceBasicCullNode;
	class C_PortalCullNode;

	/// Model Instance list culled using portals.
	class C_ModelInstancePortalList : public I_ListNode
	{
	public:
		/// C'tor.
		explicit C_ModelInstancePortalList(const C_SceneGraph* pSceneGraph);
		/// D'tor.
		~C_ModelInstancePortalList();

		/// Inherited from I_GatherNode: Accept a visitor.
		void						Accept(I_Visitor& rkVisitor) const;
		/// Inherited from I_GatherNode: Gather a view.
		void						Gather(const C_View& rkView) const;
		/// Return the type of the list node.
		NOALIAS const ListNodeType	GetType() const __restrict
		{ return I_ListNode::LISTNODETYPE_PORTAL_LIST; }
		/// Kick off all the portal-room classification jobs for the pending queue.
		void						KickClassifyJobs(const C_SceneGraph* pSceneGraph) const;
		/// Handle classification of portal cull nodes.
		void						HandleClassifiedCullNodes() const;
		/// Insert a model instance into the portal list.
		void						Insert(C_ModelInstance* pModelInstance);
		/// Handle movement of a constituent model instance node.
		void						OnMove(C_ModelInstance* pModelInstance);
		/// Remove a model instance from the portal list.
		void						Remove(C_ModelInstance* pModelInstance, const bool bFreeCullNode = true);
		/// Reserve model instance flat lists for each portal room.
		void						ReserveModelInstanceFlatLists(U32 iCount);

	private:
		Btl::Vector<C_PortalCullNode*>							m_vCullNodes;			// List of cull-nodes on the list.
		Btl::Vector<C_ModelInstanceFlatList*>					m_vRoomFlatLists;		// List of flat lists, one per portal room.
		mutable Btl::Vector<C_ModelInstance*>					m_vClassificationQueue;	// Queue of cull-nodes that need to be classified.
		mutable Btl::DynamicPool<PortalCullNodeClassifyData>	m_ClassificationPool;	// Pool for classification tasks.
		mutable Btl::Vector<PortalCullNodeClassifyData*, Btl::VectorParameters_Static<100> >
																m_vLiveSpuJobData;		// Pointers to live SPU job data -- free'd in classified cull node handling.
		Scheduler::GroupHandle									m_JobQueueGroup;		// Job group for portal classification jobs.
		mutable bool											m_bLastFrameJobsHandled;// Set to true when results from the jobs kicked in BeginFrame are handled.
		static Debug::Bool										m_bJobQueueActive;		// Controls the status of the job queue.
		static Debug::Bool										m_bDisablePortalList;	// When set flat lists are not gathered.
		static bool												m_bDebugJobsInFlight;	// Set to true when jobs are in flight!

		/// Begin frame.
		virtual void				OnBeginFrame() const;
		/// Gather the portal list.
		void						OnGather(const C_View& rkView) const;
		/// Deferred gather of portal list.
		void						OnGatherDeferred(const void* pData) const;

		/// Functor to delete a flat list in the RoomFlatList vector.
		class DeleteFlatList
		{
		public:
			/// Delete the ModelInstanceFlatList.
			void operator () (const C_ModelInstanceFlatList* pFlatList)
			{
				if(pFlatList)
					delete pFlatList;
				return;
			}
		};
		DeleteFlatList											m_DeleteFunctor;		// Functor to clean up flat lists.

		/// Return true if the job queue is active.
		static	const bool JobQueueActive();
	};

} // namespace SceneGraph

#endif // SCENE_GRAPH_MODELINSTANCEPORTALLIST_H
