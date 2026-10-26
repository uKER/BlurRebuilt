#ifndef AFTK_BLIND_DATA_NODE_H
#define AFTK_BLIND_DATA_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_GenericAttribute;

	class C_BlindDataNode : public C_Node
	{
	public:

		///\brief Default constructor.
		///
		C_BlindDataNode();

		C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor.
		///
		///@param rhs Blind data node to copy.
		C_BlindDataNode(const C_BlindDataNode &rhs);

		///\brief Destructor.
		virtual ~C_BlindDataNode();

		///\brief Assignment operator.
		const C_BlindDataNode& operator=(const C_BlindDataNode &rhs);

		///\brief Equality operator.
		///
		///@param rhs The node we are to be compared with.
		///@return true if both nodes are identical.

		bool operator==(const C_BlindDataNode &rhs) const;

		///\brief Inequality operator.
		///
		///@param rhs The node we are to be compared with.
		///@return false if both nodes are identical.
		bool operator!=(const C_BlindDataNode &rhs) const;

		///\brief Reserves space in the attribute array.
		///
		///@param capacity The ammount of elements to reserve space for.
		void SetAttributeInitialCapacity(U32 capacity);

		///\brief Adds an attribute to this blind data node.
		///
		///@param attribute The attribute to add.
		///@return the index of the new attribute in this node's array of attributes.
		U32	AddAttribute(const C_GenericAttribute &attribute);

		///\brief Deletes an attribute from this blind data node
		///
		///@param index The index of the new attribute that is to be deleted
		void DeleteAttribute(U32 index);

		///\brief Overwrites an attribute to this blind data node.
		///
		///@param instance The attribute instance to overwrite
		///@param attribute The attribute to copy.
		///@return the index of the new attribute in this node's array of attributes
		void SetAttribute(U32 instance, const C_GenericAttribute &attribute);

		///\brief Retrieves the attribute at the given index in this node's array of attributes.
		///
		///@param instance The attribute instance to retrieve.
		///@return The desired attribute.
		const C_GenericAttribute& GetAttribute(U32 instance) const;

		///\brief Retrieves the attribute at the given index in this node's array of attributes.
		///
		///@param instance The attribute instance to retrieve.
		///@return The desired attribute.
		C_GenericAttribute& GetAttribute(U32 instance);

		///\brief Returns the number of attributes contained by this class.
		///
		///@return The number of attributes contained by this class.
		U32	GetNumAttributes() const;

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera etc.
		///@return C_Node type enum.
		E_ObjType GetType() const;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		bool HasType(E_ObjType type) const;

		U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const;

		static void Output(const C_BlindDataNode& blindData);

		///\brief Clears the contents of this class to their initial state.
		///
		virtual void Clear();

		bool IsEqualToThis(const I_Object &_rhs, U32 flags = 0) const;

		//New
		void* operator new (size_t size);

		//Delete.
		void operator delete(void* p);

		//Placement new/delete
		void* operator new (size_t size, void* pMem) {return pMem;}

		void operator delete(void* pMem, void* p) {}

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		bool IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		U32 GetHashOfThis() const;

	#ifdef AFTK_USE_MEMORY_POOL
		Ut::UtVector< Ut::DynamicVector<C_GenericAttribute, C_ArrayAllocator<C_GenericAttribute> > >	m_attributes;
	#else
		Ut::UtVector< Ut::DynamicVector<C_GenericAttribute> >	m_attributes;
	#endif

	#endif
	};
}

#endif

