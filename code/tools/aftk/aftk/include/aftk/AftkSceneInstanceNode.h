#ifndef AFTK_SCENE_INSTANCE_NODE_H
#define AFTK_SCENE_INSTANCE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;

	class C_SceneInstanceNode : public C_SceneNode
	{
	public:

		///\brief Constructor.
		///
		///@param pScene The scene that this node belongs to.
		C_SceneInstanceNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;
		
		///\brief Copy constructor.
		///
		///@param rhs The node to copy.
		C_SceneInstanceNode(const C_SceneInstanceNode& rhs);

		///\brief Destructor.
		///
		virtual ~C_SceneInstanceNode();

		///\brief Asignment operator.
		///
		///@param rhs The node to copy.
		const C_SceneInstanceNode& operator=(const C_SceneInstanceNode& rhs);

		///\brief Sets the node that we are instancing.
		///
		///Internally this lets the instanced node know what is instancing it.
		///@param pSceneNode The node to instance.
		void SetSourceNodePtr(C_SceneNode* pSceneNode);

		///\brief Returns the node which we are instancing.
		///
		///@return The instanced node.
		C_SceneNode* GetSourceNodePtr();

		///\brief Returns the node which we are instancing.
		///
		///@return The instanced node.
		const C_SceneNode* GetSourceNodePtr() const;

		C_SceneNode* GetTopLevelSourceNodePtr();

		const C_SceneNode* GetTopLevelSourceNodePtr() const;

		U32 Repair();

		///\brief Checks that the contents of this node are valid.
		///
		///@return The number of errors found.
		U32	Validate() const;

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

		///\brief Equivilance operator.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool operator== (const C_SceneInstanceNode &rhs) const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
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
		C_SceneInstanceNode();

		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		virtual void CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const;

		U32 ProcessProblems(bool fix);

		virtual void DeleteInstanceSource(C_SceneNode* pSource);

		C_SceneNode*	m_pSourceNode;

		friend class C_NativeFtk;
		friend class C_NativeSceneInstanceNode;
	#endif
	};
}
#endif
