#ifndef AFTK_LOOKATCONSTRAINT_H
#define AFTK_LOOKATCONSTRAINT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeSceneLookAtConstraintNode;

	class C_SceneLookAtConstraintNode : public C_SceneAimConstraintNode
	{
	public:
		///\brief Default constructor.
		///
		C_SceneLookAtConstraintNode(C_Scene* pScene);

		C_SceneLookAtConstraintNode(const C_SceneLookAtConstraintNode& rhs);

		const C_SceneLookAtConstraintNode& operator=(const C_SceneLookAtConstraintNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;

		virtual ~C_SceneLookAtConstraintNode();

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

		C_StaticPlug<F32>& GetRollPlug();

		const C_StaticPlug<F32>& GetRollPlug() const;

		C_StaticPlug<F32>& GetDistanceBetweenPlug();

		const C_StaticPlug<F32>& GetDistanceBetweenPlug() const;

		virtual U32 GetNumStaticPlugs() const;

		virtual void Clear();

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

		static void OnInputPlugChanged(void* pThis);

		virtual void Evaluate();

		void ClearThis(bool evaluateDependants);

		//Input
		C_StaticPlug<F32>	m_rollPlug;

		//Output
		C_StaticPlug<F32>	m_distanceBetweenPlug;

		FRIEND_CLASS C_NativeSceneLookAtConstraintNode;
	#endif
	};
}

#endif

