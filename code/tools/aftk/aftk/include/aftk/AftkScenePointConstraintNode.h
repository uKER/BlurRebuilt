#ifndef AFTK_POINT_CONSTRAINT_H
#define AFTK_POINT_CONSTRAINT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeScenePointConstraintNode;

	class C_ScenePointConstraintNode : public I_SceneConstraintNode, public C_LimitMixin
	{
	public:
		///\brief Default constructor.
		///
		C_ScenePointConstraintNode(C_Scene* pScene);

		C_ScenePointConstraintNode(const C_ScenePointConstraintNode& rhs);

		const C_ScenePointConstraintNode& operator=(const C_ScenePointConstraintNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;

		virtual ~C_ScenePointConstraintNode();

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

		C_NullPlug& GetTranslateOffsetPlug();

		const C_NullPlug& GetTranslateOffsetPlug() const;

		C_StaticPlug<F32>& GetOffsetPolarityPlug();

		const C_StaticPlug<F32>& GetOffsetPolarityPlug() const;

		C_NullPlug& GetRestTranslatePlug();

		const C_NullPlug& GetRestTranslatePlug() const;

		C_NullPlug& GetConstraintTranslatePlug();

		const C_NullPlug& GetConstraintTranslatePlug() const;

		bool RestTranslateEnabled() const;

		void EnableRestTranslate(bool enable);

		C_NullPlug& GetTargetTranslatePlug(U32 target);

		const C_NullPlug& GetTargetTranslatePlug(U32 target) const;

		C_StaticPlug<F32>& GetTargetWeightPlug(U32 target);

		const C_StaticPlug<F32>& GetTargetWeightPlug(U32 target) const;

		C_NullPlug& GetTargetParentPlug(U32 target);

		const C_NullPlug& GetTargetParentPlug(U32 target) const;

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
			C_Target(C_ScenePointConstraintNode* pParent);

			//Input
			C_NullPlug			targetParentPlug;
			C_NullPlug			targetTranslatePlug;
			C_StaticPlug<F32>	targetWeightPlug;
		};

		Ut::UtVector<Ut::DynamicVector<C_Target*> >	m_inputs;

		C_NullPlug			m_translateOffsetPlug;
		C_StaticPlug<F32>	m_offsetPolarityPlug;
		C_NullPlug			m_restTranslatePlug;
		bool				m_enableRestTranslate;

		//Output
		C_NullPlug			m_constraintTranslatePlug;

		FRIEND_CLASS		C_NativeScenePointConstraintNode;
	#endif
	};
}

#endif


