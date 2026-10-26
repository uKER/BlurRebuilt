#ifndef AFTK_ORIENT_CONSTRAINT_NODE_H
#define AFTK_ORIENT_CONSTRAINT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeSceneOrientConstraintNode;

	class C_SceneOrientConstraintNode : public I_SceneConstraintNode, public C_LimitMixin
	{
	public:
		///\brief Default constructor.
		///
		C_SceneOrientConstraintNode(C_Scene* pScene);

		C_SceneOrientConstraintNode(const C_SceneOrientConstraintNode& rhs);

		const C_SceneOrientConstraintNode& operator=(const C_SceneOrientConstraintNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;

		virtual ~C_SceneOrientConstraintNode();

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

		C_NullPlug& GetRotateOffsetPlug();

		const C_NullPlug& GetRotateOffsetPlug() const;

		C_NullPlug& GetRestRotatePlug();

		const C_NullPlug& GetRestRotatePlug() const;

		bool RestRotateEnabled() const;

		void EnableRestRotate(bool enable);

		C_NullPlug& GetTargetRotatePlug(U32 target);

		const C_NullPlug& GetTargetRotatePlug(U32 target) const;

		C_StaticPlug<F32>& GetTargetWeightPlug(U32 target);

		const C_StaticPlug<F32>& GetTargetWeightPlug(U32 target) const;

		C_NullPlug& GetTargetParentPlug(U32 target);

		const C_NullPlug& GetTargetParentPlug(U32 target) const;

		C_NullPlug& GetConstraintRotatePlug();

		const C_NullPlug& GetConstraintRotatePlug() const;

		virtual U32 GetNumStaticPlugs() const;

		virtual void Clear();

		void SetNumInputs(U32 nInputs);

		U32 GetNumInputs() const;

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

		struct C_Target
		{
			C_Target(C_SceneOrientConstraintNode* pThis);

			//Input
			C_NullPlug			targetParentPlug;
			C_NullPlug			targetRotatePlug;
			C_StaticPlug<F32>	targetWeightPlug;
		};

		Ut::UtVector<Ut::DynamicVector<C_Target*> >	m_inputs;

		C_NullPlug			m_rotateOffsetPlug;
		C_NullPlug			m_restRotatePlug;
		bool				m_enableRestRotate;

		//Output
		C_NullPlug			m_constraintRotatePlug;

		FRIEND_CLASS		C_NativeSceneOrientConstraintNode;
	#endif
	};
}

#endif


