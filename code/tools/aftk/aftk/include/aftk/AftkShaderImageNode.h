#ifndef AFTK_IMAGE_NODE_H
#define AFTK_IMAGE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_ShaderImageNode : public C_ShaderNode
	{
	public:
		
		///\brief Constructor.
		///
		C_ShaderImageNode(Aftk::C_Status* pStatus = 0);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor.
		///
		///@param rhs The node to copy.
		C_ShaderImageNode(const C_ShaderImageNode &rhs);

		///\brief Destructor.
		///
		~C_ShaderImageNode();

		///\brief Asignment operator.
		///
		///@param rhs The node to copy.
		///@return A reference to this node.
		const C_ShaderImageNode& operator=(const C_ShaderImageNode &rhs);
		
		///\brief Equality test.
		///
		///Tests the contents of this node with rhs, returning true if they are equal.
		///@param rhs The node to test against.
		///@return True if equal.
		bool operator==(const C_ShaderImageNode &rhs) const;

		///\brief Inequality test.
		///
		///Tests the contents of this node with rhs, returning true if they are not equal.
		///@param rhs The node to test against.
		///@return True if not equal.
		bool operator!=(const C_ShaderImageNode &rhs) const;

		///\brief Less than test.
		///
		///Tests the contents of this node with rhs, returning true if they evaluate as less than the rhs.
		///Saying one node is less than another is only of value for sorting.
		///@param rhs The node to test against.
		///@return True if less than.
		bool operator<(const C_ShaderImageNode &rhs) const;

		///brief Returns the image file name.
		///
		///@return The image file name.
		C_StaticPlug<std::string>& GetFilePlug();

		///\brief Returns the image type string.
		///
		///@return The image usage cue string.
		C_StaticPlug<std::string>& GetImageTypePlug();

		///brief Returns the image file name.
		///
		///@return The image file name.
		const C_StaticPlug<std::string>& GetFilePlug() const;

		///\brief Returns the image type string.
		///
		///@return The image usage cue string.
		const C_StaticPlug<std::string>& GetImageTypePlug() const;

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual I_Plug*	GetStaticPlugPtr(U32 instance);

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual const I_Plug* GetStaticPlugPtr(U32 instance) const;

		///\brief Returns the number of static plugs on this node.
		///
		///@return The number of static plugs.
		virtual U32	GetNumStaticPlugs() const;

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		E_ObjType GetType() const;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		///\brief Returns a pointer to a copy of this node.
		///
		///@return A copy of this node.
		C_ShaderNode*	Clone() const;

		///\brief Clears the contents of this node to their initial state.
		///
		virtual void Clear();

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object &rhs, U32 flags = 0) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		C_Status OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		friend class C_NativeShaderImageNode;

		C_StaticPlug<std::string> m_filePlug;
		C_StaticPlug<std::string> m_imageTypePlug;

	#endif
	};
}

#endif
