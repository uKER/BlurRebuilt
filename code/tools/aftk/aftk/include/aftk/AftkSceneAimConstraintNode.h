#ifndef AFTK_AIMCONSTRAINT_H
#define AFTK_AIMCONSTRAINT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeSceneAimConstraintNode;

	class C_SceneAimConstraintNode : public I_SceneConstraintNode, public C_LimitMixin
	{
	public:
		///\brief Default constructor.
		///
		C_SceneAimConstraintNode(C_Scene* pScene);

		C_SceneAimConstraintNode(const C_SceneAimConstraintNode& rhs);

		const C_SceneAimConstraintNode& operator=(const C_SceneAimConstraintNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;

		virtual ~C_SceneAimConstraintNode();

		///\brief Generate hash of data in current class and hierachy
		///
		///@return 32b hash value
		virtual U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const;

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

		virtual bool IsUsed() const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(
			const I_Object &rhs,
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object &rhs, U32 flags = E_Comparison_LocalTransform) const;

		C_NullPlug& GetConstraintTranslatePlug();

		const C_NullPlug& GetConstraintTranslatePlug() const;

		C_NullPlug& GetTargetTranslatePlug();

		const C_NullPlug& GetTargetTranslatePlug() const;

		C_NullPlug& GetConstraintParentPlug();

		const C_NullPlug& GetConstraintParentPlug() const;

		C_NullPlug& GetTargetParentPlug();

		const C_NullPlug& GetTargetParentPlug() const;

		C_NullPlug& GetAimVectorPlug();

		const C_NullPlug& GetAimVectorPlug() const;

		C_NullPlug& GetUpVectorPlug();

		const C_NullPlug& GetUpVectorPlug() const;

		C_NullPlug& GetRotationalOffsetPlug();

		const C_NullPlug& GetRotationalOffsetPlug() const;

		C_StaticPlug<U32>& GetWorldUpVectorTypePlug();

		const C_StaticPlug<U32>& GetWorldUpVectorTypePlug() const;

		C_NullPlug& GetWorldUpVectorPlug();

		const C_NullPlug& GetWorldUpVectorPlug() const;

		C_StaticPlug<U32>& GetConstraintRotateOrderPlug();

		const C_StaticPlug<U32>& GetConstraintRotateOrderPlug() const;

		C_NullPlug& GetConstraintRotatePlug();

		const C_NullPlug& GetConstraintRotatePlug() const;

		virtual U32 GetNumStaticPlugs() const;

		virtual void Clear();

		Matrix44 GetAimMatrix() const;

		C_StaticPlug<Matrix44>& GetWorldUpMatrixPlug();

		const C_StaticPlug<Matrix44>& GetWorldUpMatrixPlug() const;

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		I_Plug* GetStaticPlugPtr(U32 instance)
		{
			return const_cast<I_Plug*>(((const C_Node*)this)->GetStaticPlugPtr(instance));
		}

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual const I_Plug* GetStaticPlugPtr(U32 instance) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		void ClearThis(bool evaluateDependants);

		static void OnInputPlugChanged(void* pThis);

		virtual void Evaluate();

		//Input
		C_NullPlug			m_constraintParentPlug;
		C_NullPlug			m_constraintTranslatePlug;
		C_NullPlug			m_targetParentPlug;
		C_NullPlug			m_targetTranslatePlug;
		C_NullPlug			m_aimVectorPlug;
		C_NullPlug			m_upVectorPlug;
		C_NullPlug			m_rotationalOffsetPlug;

		C_StaticPlug<U32>	m_worldUpVectorTypePlug;
		C_NullPlug			m_worldUpVectorPlug;

		//Input/output
		C_StaticPlug<U32>	m_constraintRotateOrderPlug;

		C_StaticPlug<Matrix44> m_worldUpMatrixPlug;

		//Output
		C_NullPlug			m_constraintRotatePlug;

		FRIEND_CLASS		C_NativeSceneAimConstraintNode;
	#endif
	};
}

#endif

/* XSI constraint types
	enum EConstraintType
	{
		SI_SCALING,			//!<Scaling constraint
		SI_ORIENTATION,		//!<Orientation constraint
		SI_DIRECTION,		//!<Direction constraint
		SI_POSITION,		//!<Position constraint
		SI_INTEREST,		//!<Interest constraint
		SI_UP_VECTOR,		//!<Up Vector constraint
		SI_PREFERED_AXIS,	//!<Prefered Axis constraint
	};

	Maya constraint types
	geometryConstraint,
	normalConstraint,
	orientConstraint, 
	parentConstraint,
	pointConstraint,
	poleVectorConstraint,
	tangentConstraint, 
	scaleConstraint,
	lookAt 
*/
