#ifndef AFTK_SCENE_LOD_GROUP_NODE_H
#define AFTK_SCENE_LOD_GROUP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;

	class C_SceneLodGroupNode : public C_SceneNode
	{
	public:
		///\brief Constructor.
		///
		///In order for a scene node to be valid you must supply a valid scene pointer.
		///
		///@param pScene C_Scene pointer.
		C_SceneLodGroupNode(C_Scene* pScene);

		C_SceneLodGroupNode(const C_SceneLodGroupNode& rhs);

		const C_SceneLodGroupNode& operator=(const C_SceneLodGroupNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;
		
		///\brief Destructor.
		///
		virtual ~C_SceneLodGroupNode();

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		virtual E_ObjType GetType() const;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const;

		///\brief Adds a level of detail description.
		///
		///@param info The level of detail description.
		///@return The level of detail index for the new description.
		S32 AddLodLevelInfo(const C_GroupLodLevelInfo& info);

		///\brief Inserts a level of detail description.
		///
		///@param lodLevel The lod level for the new description.
		///@param info The level of detail description.
		void InsertLodLevelInfo(U32 lodLevel, const C_GroupLodLevelInfo& info);

		///\brief Returns the description information for a given lod level.
		///
		///@param lodLevel The level of the lod info to fetch.
		///@return The level of detail description.
		const C_GroupLodLevelInfo& GetLodLevelInfo(U32 lodLevel) const;

		///\brief Returns the description information for a given lod level.
		///
		///@param lodLevel The level of the lod info to fetch.
		///@return The level of detail description.
		C_GroupLodLevelInfo& GetLodLevelInfo(U32 lodLevel);

		///\brief Deletes the level of detail description at the given index.
		///
		///@param lodLevel The index of the LOD description to remove.
		void DeleteLodLevelInfo(U32 lodLevel);

		///\brief Deletes the level of detail description associated with the given scene node.
		///
		///@param pNode The scene node for the LOD level to be deleted.
		void DeleteLodLevelInfo(C_SceneNode* pNode);

		///\brief Returns the number of LOD levels in this group.
		///
		///@return The number of LOD levels.
		U32 GetNumLodLevels() const;

		S32 GetLodLevelIndex(const C_SceneNode* pNode) const;

		bool operator== (const C_SceneLodGroupNode &rhs) const;

		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Clears the contents of this class to their initial state.
		///
		virtual void Clear();

		bool IsEqualToThis(const I_Object& rhs, U32 flags = E_Comparison_LocalTransform) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		Ut::UtVector< Ut::DynamicVector< C_GroupLodLevelInfo > > m_infoList;

	#endif
	};
}
#endif
