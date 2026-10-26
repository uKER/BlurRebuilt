#ifndef AFTK_SCENE_XREF_NODE_H
#define AFTK_SCENE_XREF_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_Scene;

	class C_SceneXRefNode : public C_SceneNode
	{
	public:
		C_SceneXRefNode(C_Scene* pScene);

		C_SceneXRefNode(const C_SceneXRefNode& rhs);

		const C_SceneXRefNode& operator=(const C_SceneXRefNode& rhs);
		
		virtual ~C_SceneXRefNode();

		void SetSourceSceneFileName(const std::string& fileName);

		void SetSourceNodePath(const std::string& name);

		const C_SceneNode* GetSourceNodePtr() const;

		C_SceneNode* GetSourceNodePtr();

		C_Scene* GetSourceScenePtr();

		const C_Scene* GetSourceScenePtr() const;

		const std::string& GetSourceSceneFileName() const;

		const std::string& GetSourceNodePath() const;

		bool IsLoaded() const;

		///
		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		E_ObjType GetType() const;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const;

		virtual void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		C_Status SetSourceScenePtr(C_Scene* pSourceScene);

		std::string		m_sourceSceneFileName;
		std::string 	m_sourceSceneNodePath;

		//The pointers below will be null if the node is off-line.
		C_Scene*		m_pSourceScene;
		C_SceneNode*	m_pSourceSceneNode;

		friend class C_Scene;

	#endif
	};
}
#endif
