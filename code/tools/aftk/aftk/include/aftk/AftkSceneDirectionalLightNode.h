#ifndef AFTK_SCENE_DIRECTIONAL_LIGHT_NODE_H
#define AFTK_SCENE_DIRECTIONAL_LIGHT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;

	class C_SceneDirectionalLightNode : public C_SceneNonAmbientLightNode
	{
	public:
		///\brief Constructor.
		///
		///In order for a scene node to be valid you must supply a valid scene pointer.
		///
		///@param pScene C_Scene pointer.
		C_SceneDirectionalLightNode(C_Scene* pScene);

		C_SceneDirectionalLightNode(const C_SceneDirectionalLightNode &rhs);

		const C_SceneDirectionalLightNode& operator=(const C_SceneDirectionalLightNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Destructor.
		///
		virtual ~C_SceneDirectionalLightNode();

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

		bool operator== (const C_SceneDirectionalLightNode &rhs) const;

		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Clears the contents of this node to their initial state.
		///
		virtual void Clear();

		bool IsEqualToThis(const I_Object& rhs, U32 flags = E_Comparison_LocalTransform) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void ClearThis();

		bool IsEqualToAll(const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;
	#endif
	};
}
#endif
