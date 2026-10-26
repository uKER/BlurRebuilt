#ifndef AFTK_SCENE_CONSTRAINT_NODE_H
#define AFTK_SCENE_CONSTRAINT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeSceneConstraintNode;

	class C_SceneConstraintNode : public I_SceneConstraintNode
	{
	public:
		///\brief Default constructor.
		///
		C_SceneConstraintNode(C_Scene* pScene);

		C_SceneConstraintNode(const C_SceneConstraintNode& rhs);

		const C_SceneConstraintNode& operator=(const C_SceneConstraintNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;

		virtual ~C_SceneConstraintNode();

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

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		FRIEND_CLASS		C_NativeSceneConstraintNode;
	#endif
	};
}

#endif
