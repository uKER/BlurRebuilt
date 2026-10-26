#ifndef AFTK_LAYER_H
#define AFTK_LAYER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_SceneNode;

	class C_LayerNode : public C_Node
	{
	public:
		///\brief Constructor.
		///
		///Note that you must supply the scene that this layer belongs to.
		///
		///@param pScene AFTK scene pointer.
		C_LayerNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Asignment operator.
		///
		///@param rhs Class to copy.
		const C_LayerNode& operator=(const C_LayerNode& rhs);

		///\brief Returns true if the given node is a member of this layer.
		///
		///@param pSceneNode The node to test.
		///@return true if a member, otherwise false.
		bool IsMember(const C_SceneNode* pSceneNode) const;
		
		///\brief Adds a scene node to this layer.
		///
		///@param pSceneNode A pointer to the scene node to be added.
		///@return The instance of the scene node in the array of layer members.
		U32 AddMember(C_SceneNode* pSceneNode);

		///\brief Overwrites the scene node pointer at the given index.
		///
		///@param instance C_Scene node index to overwrite.
		///@param pSceneNode The new scene node pointer.
		void SetMember(U32 instance, C_SceneNode* pSceneNode);

		///\brief Get the number of members of this layer.
		///
		///@return The number of members of this layer.
		U32	GetNumMembers() const;

		///\brief Get a member from this layer by index.
		///
		///@param instance The scene node index.
		///@return A pointer to the scene node.
		C_SceneNode* GetMemberPtr(U32 instance);

		///\brief Get a member from this layer by index.
		///
		///@param instance The scene node index.
		///@return A pointer to the scene node.
		const C_SceneNode* GetMemberPtr(U32 instance) const;

		///\brief Get the index of a given scene node in this layer.
		///
		///Returns -1 if the given node is not a member of this layer.
		///
		///@param pSceneNode C_Scene node pointer.
		///@return The index of the scene node in this layer.
		S32 GetMemberIndex(C_SceneNode* pSceneNode);

		///\brief Deletes a scene node from this layer.
		///
		///This does NOT delete the scene node from the scene hierachy itself.
		///@param pSceneNode A pointer to the scene node to be deleted.
		void DeleteMember(C_SceneNode* pSceneNode);

		virtual bool IsUsed() const;

		virtual E_ObjType GetType() const;

		virtual bool HasType(E_ObjType type) const;
		
		///\brief Clears the contents of this class.
		///
		void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void OneTimeInit();

		///\brief Destructor.
		///
		~C_LayerNode();

		///\brief Copy constructor.
		///
		///@param rhs Class to copy.
		C_LayerNode(const C_LayerNode& rhs);

		void ClearThis();

		//Note that we use two containers to hold the members of the layer. We use a 'map' in order to 
		//make it possible to quickly determine whether a given node already exists in this layer. We also
		//use a vector in order to gain array-style access to members of the layer.

		///\brief A map containing the scene nodes which are members of this layer.
		Ut::UtMap<C_SceneNode*, S32>	m_memberMap;

		///\brief A list of the scene nodes which are members of this layer.
		Ut::UtVector< Ut::DynamicVector<C_SceneNode*> >		m_memberVector;

		friend class C_Scene;
		friend class C_SceneNode;
		friend class C_NativeLayerNode;
	#endif
	};
}

#endif
