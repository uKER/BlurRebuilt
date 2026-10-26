#ifndef AFTK_UNIT_CONVERSION_NODE_H
#define AFTK_UNIT_CONVERSION_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
	class C_AttachData;

	class C_UnitConversionNode : public I_EvaluatableNode
	{
	public:
		///\brief Default constructor.
		///
		C_UnitConversionNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor
		///
		///@param rhs The source node for the copy operation
		C_UnitConversionNode(const C_UnitConversionNode& rhs);

		///\brief Destructor.
		virtual ~C_UnitConversionNode();

		///\brief Asignment operator.
		///
		///@param rhs The source node for the copy operator.
		C_UnitConversionNode& operator= (const C_UnitConversionNode& rhs);

		///\brief Equivilance operator which operates on entire class heirachy.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool operator== (const C_UnitConversionNode &rhs) const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		///\brief Generate hash of data in current class
		///
		///@return 32b hash value
		U32 GetHashOfThis() const;

		///\brief Generate hash of data in current class and hierachy
		///
		///@return 32b hash value
		virtual U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const;

		///\brief Negated equivilance operator.
		///
		///@param aftkNode The node for comparison.
		///@return returns 'false' if this node and aftkNode are equivilant. Otherwise 'true' is returned.
		bool operator!= (const C_UnitConversionNode &aftkNode);

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

		///\brief Clears the contents of this node.
		///
		virtual void Clear();

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object& rhs, U32 flags = 0) const;

		C_NullPlug& GetInputPlug();

		const C_NullPlug& GetInputPlug() const;

		void SetFactor(F32 factor);

		F32 GetFactor() const;

		virtual bool Evaluate(F32& value) const;

		virtual bool GetAdjacentKeys(C_BaseKey& key1, C_BaseKey& key2) const;

		virtual bool GetAdjacentKeys(C_LinearKey& key1, C_LinearKey& key2) const;

		virtual bool GetAdjacentKeys(C_HermiteKey& key1, C_HermiteKey& key2) const;

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

		///\brief Returns the number of static plugs on this node.
		///
		///@return The number of dynamic plugs.
		virtual U32	GetNumStaticPlugs() const;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		template<class T>
		bool GetAdjacentKeysT(T& key1, T& key2) const;

		friend class C_NativeUnitConversionNode;
		friend class C_NativeNode;

		C_NullPlug*	m_pInputPlug;
		F32			m_factor;
	#endif
	};

	#include "AftkUnitConversionNode.inl"
}

#endif
