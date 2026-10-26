#ifndef AFTK_SCENE_GEOMETRY_NODE_H
#define AFTK_SCENE_GEOMETRY_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_SceneGeometryNode: public C_SceneNode
	{
	public:

		///\brief Constructor.
		///
		///@param pScene The scene that this node belongs to.
		C_SceneGeometryNode(C_Scene* pScene);

		///\brief Copy constructor.
		///
		///@param rhs C_Node to copy.
		C_SceneGeometryNode(const C_SceneGeometryNode& rhs);

		///\brief Destructor.
		///
		virtual	~C_SceneGeometryNode();

		///\brief Asignment operator.
		///
		///@param rhs C_Node to copy.
		///@return reference to this node.
		const C_SceneGeometryNode& operator=(const C_SceneGeometryNode& rhs);

		///\brief Equivilance operator.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool operator== (const C_SceneGeometryNode &rhs) const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Generate hash of data in current class
		///
		///@return 32b hash value
		U32 GetHashOfThis() const;

		///\brief Generate hash of data in current class and hierachy
		///
		///@return 32b hash value
		virtual U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const;

		///\brief Applies the given matrix to the contents of this node.
		///
		///@param matrix The matrix to apply.
		virtual void ApplyMatrix(const Matrix44& matrix) = 0;

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		virtual E_ObjType GetType() const
		{
			return E_ObjType_SceneGeometry;
		}

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const 
		{
			if (C_SceneNode::HasType(type))
				return true;
			if (type == C_SceneGeometryNode::GetType()) 
				return true;

			return false;
		}

		///\brief Clear the contents of this node to their initial state.
		///
		virtual void Clear();

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object &rhs, U32 flags = E_Comparison_LocalTransform) const;

		virtual void DirtyGeometryDependantCaches() const
		{
			m_localBoundsCacheDirtyFlag = true;
			m_worldBoundsCacheDirtyFlag = true;
		}

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(
			const I_Object& rhs, E_ObjType 
			classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

	#endif
	};
}

#endif
