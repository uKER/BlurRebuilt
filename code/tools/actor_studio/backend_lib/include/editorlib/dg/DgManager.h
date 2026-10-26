#ifndef EDITOR_LIB_DG_MANAGER_H
#define EDITOR_LIB_DG_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <btl/utility/string.h>
#include <btl/containers/openhashmap.h>
#include <btl/containers/openhashset.h>
#include <EditorLib/Misc/NodeId.h>

#include "DgNodeIterator.h"

namespace EditorLib
{
	class I_DgNode;
	class C_DgNode;
	class I_Level;

	namespace XtNodeHelpers
	{
		void XtInitialiseName(
			Btl::String&, 
			I_DgNode*, 
			LtStruct* pType,
			Xt::DomBlock*);

		bool XtDeserialiseName(
			Btl::String&, 
			I_DgNode*, 
			Xt::DomElement*);

		void AssumeXtId(
			C_NodeId& nodeId, 
			I_DgNode* pNode, 
			XtId xtId, 
			U32 childId);

		void ReleaseXtId(
			C_NodeId& nodeId, 
			I_DgNode* pNode);
	}

	struct C_UpdateItem
	{
		C_UpdateItem() {}
		C_UpdateItem(const C_NodeId& nodeId_, bool typeChange_) :
		nodeId(nodeId_), typeChange(typeChange_) {}
		C_UpdateItem(const C_UpdateItem& rhs) 
		{
			*this = rhs;
		}
		const C_UpdateItem& operator=(const C_UpdateItem& rhs)
		{
			nodeId = rhs.nodeId;
			typeChange = rhs.typeChange;
			return *this;
		}

		C_NodeId					nodeId;
		bool						typeChange;
	};

	class C_DgManager
	{
	public:
		C_DgManager();
		~C_DgManager();

		I_DgNode*		CreateNode(I_Level* pLevel, const Btl::String& typeName);
		I_DgNode*		CreateNode(I_Level* pLevel, E_ObjType type);
		bool			DestroyNode(I_DgNode* pNode, bool deleteOverride);
		void			Clear();

		I_DgNode*		FindNodeById(C_NodeId id);
		const I_DgNode*	FindNodeById(C_NodeId id) const;

		I_Level*		LevelOf(const C_NodeId& id);

		C_DgNodeIterator		GetDgIterator();
		C_DgNodeConstIterator	GetDgIterator() const;
		U32				GetNumDgNodes() const;

		void			CreateName(I_DgNode* pNode, Btl::String& name);
		void			DestroyName(I_DgNode* pNode, const Btl::String& name);

		void			AddForDeferredUpdate(const C_NodeId& nodeId, bool typeChange = false);

		void			RemoveForDeferredUpdate(const C_NodeId& nodeId);

		bool			IsDeferredUpdatePending() const;

		void			FlushDeferredUpdates();

		void			ClearDeferredUpdates();

		//TODO: Hide these
		bool			RegisterId(I_DgNode* pNode, C_NodeId& id);
		void			UnregisterId(C_NodeId id);

		void			AttachLevel(I_Level* pLevel);
		void			DetachLevel(I_Level* pLevel);
		S32				FindLevelIdx(const I_Level* pLevel) const;

	private:
		enum E_State
		{
			E_State_Unknown,
			E_State_Updating
		};


		typedef Btl::OpenHashSet<C_NodeId, HashKeyOpNodeId> NodeIdSet;
		typedef Btl::OpenHashMap<C_NodeId, C_UpdateItem*, HashKeyOpNodeId> UpdateItemMap;

		struct C_PendingDeferredUpdates
		{
			void Clear()
			{
				for (S32 i = 0; i < deferredUpdates.Size(); ++i)
					SAFE_DELETE_PTR(deferredUpdates[i]);
				deferredUpdates.Clear();
				deferredUpdateMap.Clear();
			}

			Btl::Vector<C_UpdateItem*>	deferredUpdates;
			UpdateItemMap				deferredUpdateMap;
		};

		void FlushDeferredUpdates(C_PendingDeferredUpdates& pendinfDeferredUpdates);

		C_PendingDeferredUpdates		deferredUpdatesPrimary;
		C_PendingDeferredUpdates		deferredUpdatesSecondary;

		Btl::Vector< Btl::Vector<C_UpdateItem*> > m_sortedUpdates;

		DgIdPtrMap			m_dgById;
		
		Btl::Vector<I_Level*> m_levels;
		
		FRIEND_CLASS		I_DgNode;

		friend				void XtNodeHelpers::XtInitialiseName(
			Btl::String&, 
			I_DgNode*,
			LtStruct* pType,
			Xt::DomBlock*);

		friend				bool XtNodeHelpers::XtDeserialiseName(
			Btl::String&, 
			I_DgNode*, 
			Xt::DomElement*);

		friend				void XtNodeHelpers::AssumeXtId(
			C_NodeId& nodeId, 
			I_DgNode* pNode, 
			XtId xtId, 
			U32 childId);

		friend				void XtNodeHelpers::ReleaseXtId(
			C_NodeId& nodeId, 
			I_DgNode* pNode);

		E_State m_state;
	};
}

#endif
