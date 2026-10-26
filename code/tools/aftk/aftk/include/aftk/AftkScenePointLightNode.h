#ifndef AFTK_SCENE_POINT_LIGHT_NODE_H
#define AFTK_SCENE_POINT_LIGHT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;

	class C_ScenePointLightNode : public C_SceneNonAmbientLightNode
	{
	public:
		///\brief Constructor.
		///
		///In order for a scene node to be valid you must supply a valid scene pointer.
		///
		///@param pScene C_Scene pointer.
		C_ScenePointLightNode(C_Scene* pScene);

		C_ScenePointLightNode(const C_ScenePointLightNode& rhs);

		const C_ScenePointLightNode& operator=(const C_ScenePointLightNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Destructor.
		///
		virtual ~C_ScenePointLightNode();

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

		bool operator== (const C_ScenePointLightNode &rhs) const;

		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Clears the contents of this node to their initial state.
		///
		virtual void Clear();

		///\brief Sets the cut-off radius of this light.
		///
		///This is a visual prop used by the artists to define the intensity of the light within a 3D package.
		///It has no basis in reality.
		///@param cutOffRadius
		void SetCutOffRadius(F32 cutOffRadius);

		///\brief Returns the cut-off radius of this light.
		///
		///This is a visual prop used by the artists to define the intensity of the light within a 3D package.
		///It has no basis in reality.
		///@return The cut-off radius.
		F32 GetCutOffRadius() const;

		bool IsEqualToThis(const I_Object& rhs, U32 flags = E_Comparison_LocalTransform) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		F32 m_cutOffRadius;
	#endif
	};
}
#endif
