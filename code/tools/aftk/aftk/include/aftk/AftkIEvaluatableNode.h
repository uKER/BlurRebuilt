#ifndef AFTK_EVALUATABLE_NODE_H
#define AFTK_EVALUATABLE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
	class C_AttachData;

	class I_EvaluatableNode : public C_Node
	{
	public:
		virtual C_Node* Clone(C_Scene* pScene) const = 0;

		virtual ~I_EvaluatableNode() {}

		///\brief Generate hash of data in current class and hierachy
		///
		///@return 32b hash value
		virtual U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const
		{
			return C_Node::GetHash(classToAbortTest);
		}

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		virtual E_ObjType GetType() const = 0;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const
		{
			if (C_Node::HasType(type))
				return true;
			if (type == E_ObjType_IEvaluatable) 
				return true;

			return false;
		}

		virtual bool IsUsed() const 
		{
			return C_Node::IsUsed();
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(U8& value) const 
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(U16& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(U32& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(U64& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(S8& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(S16& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(S32& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(S64& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(F32& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(F64& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(bool& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(std::string& value) const
		{ 
			return false; 
		}

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool Evaluate(Matrix44& value) const
		{ 
			return false; 
		}

		///\brief Find the values either side of the current evaluation point.
		virtual bool GetAdjacentKeys(C_BaseKey& key1, C_BaseKey& key2) const
		{
			return false;
		}

		///\brief Find the values either side of the current evaluation point.
		virtual bool GetAdjacentKeys(C_LinearKey& key1, C_LinearKey& key2) const
		{
			return false;
		}

		virtual bool GetAdjacentKeys(C_HermiteKey& key1, C_HermiteKey& key2) const
		{
			return false;
		}

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const
		{
			return IsEqualToAll(rhs, classToAbortTest, flags);
		}

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object &rhs, U32 flags = 0) const
		{
			return true;
		}

		virtual void Clear()
		{
			C_Node::Clear();
		}

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		bool IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const
		{
			return C_Node::IsEqualToAll(rhs, classToAbortTest, flags);
		}

		///\brief Default constructor.
		///
		I_EvaluatableNode(C_Scene* pScene) : C_Node(pScene) {}
	#endif
	};
}

#endif
