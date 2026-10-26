#ifndef _GROUP_SUBSYSTEM_H_
#define _GROUP_SUBSYSTEM_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/list.h>

namespace Audio
{

	enum NodeContents
	{
		eEntityInstance = 0,
		ePointer
	};

	class GroupNode;

	struct NodeListRelationship
	{
		GroupNode*	pNode;
		U32			u32GroupId;
	};

	// each entity that can be part of a group will contain a pointer to a "group node"
	// each such entity will have a unique group code, though group nodes can exist without entities (eg, "virtual" objects, like in the old crowd system)
	// a group node is a node in one or more lists, and will contain a dynamic list of all the groups it is a member of,
	// plus it's position in each group
	class GroupNode
	{

	public:
		GroupNode() { m_u32NumLists = 0; m_bHasResource = false; m_eNodeContents = ePointer; }
		inline void							SetContents(NodeContents eContents) { m_eNodeContents = eContents; }
		inline U32							GetNumLists() { return m_u32NumLists; }

	private:
		Btl::List<NodeListRelationship*>*	m_apListRelationships;
		U32									m_u32Resource;
		U32									m_u32NumLists;
		NodeContents						m_eNodeContents;
		bool								m_bHasResource;
	};

	struct GroupSetupData
	{
		U32 u32NumGroups;
		U32 u32InitialGroupSize;
		
	};

	// NB, Group contains an EntityType, but a Group can also legitimately contain EntityInstances spawned from different types
	// the EntityType is there for ease of use when we only need one type of entity per group
	struct Group
	{
		Btl::List<NodeListRelationship>		aNodes;
		EntityTypeHandle					hType;
	};

	class GroupSubsystem : public Subsystem
	{

	public:
		GroupSubsystem ();
		~GroupSubsystem ();

		bool								InitGroups(GroupSetupData& setupData);
		bool								AssociateEntityTypeWithGroup(EntityTypeHandle hType, U32 u32GroupIndex = 0);
		bool								PopulateGroupWithEntityInstances(U32 u32NumInstances, U32 u32GroupIndex = 0);
		bool								MoveNodeToEndOfList(U32 u32NodeIndex, U32 u32GroupIndex = 0);
		bool								MoveNodeToStartOfList(U32 u32NodeIndex, U32 u32GroupIndex = 0);
		bool								AddEntityToStartOfList(EntityInstanceHandle hInstance, U32 u32GroupIndex = 0);
		bool								AddEntityToEndOfList(EntityInstanceHandle hInstance, U32 u32GroupIndex = 0);
		bool								AddMiscDataToStartOfList(void* pData, U32 u32GroupIndex = 0);
		bool								AddMiscDataToEndOfList(void* pData, U32 u32GroupIndex = 0);
		bool								RemoveEntryFromList(U32 u32NodeIndex, U32 u32GroupIndex = 0);
		bool								RemoveEntityFromList(EntityInstanceHandle hInstance, U32 u32GroupIndex = 0);
		U32									GetGroupSize(U32 u32GroupIndex = 0);
		bool								FlushGroup(U32 u32GroupIndex = 0);
		EntityInstanceHandle				GetEntity(U32 u32NodeIndex, U32 u32GroupIndex = 0);

	private:
		inline Group*						GetGroup(U32 u32Index) { if (u32Index >= m_u32NumGroups) return 0; return &(m_aGroup[u32Index]); }
		Group*								m_aGroup;
		U32									m_u32NumGroups;
	};

}

#endif	//	_GROUP_SUBSYSTEM_H_
