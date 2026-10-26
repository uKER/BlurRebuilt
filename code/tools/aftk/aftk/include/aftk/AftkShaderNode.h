#ifndef AFTK_SHADER_NODE_H
#define AFTK_SHADER_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_MaterialNode;

	class C_ShaderNode : public C_Node
	{
	public:
		///\brief Constructor.
		///
		C_ShaderNode();

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor.
		///
		///@param rhs The node to copy.
		C_ShaderNode(const C_ShaderNode &rhs);

		///\brief Destructor.
		///
		virtual ~C_ShaderNode();

		///\brief Asignment operator.
		///
		///@param rhs The node to copy.
		///@return A reference to this node.
		virtual const C_ShaderNode& operator=(const C_ShaderNode &rhs);
		
		///\brief Equality test.
		///
		///Tests the contents of this node with rhs, returning true if they are equal.
		///@param rhs The node to test against.
		///@return True if equal.
		bool operator==(const C_ShaderNode &rhs) const;

		///\brief Inequality test.
		///
		///Tests the contents of this node with rhs, returning true if they are not equal.
		///@param rhs The node to test against.
		///@return True if not equal.
		virtual bool operator!=(const C_ShaderNode &rhs) const;

		///\brief Returns a pointer to the material that this node is associated with.
		///
		///@return The material node.
		C_MaterialNode* GetMaterialNodePtr();

		///\brief Returns a pointer to the material that this node is associated with.
		///
		///@return The material node.
		const C_MaterialNode*	GetMaterialNodePtr() const;

		///\brief Returns The index of this node in the materials array of shader nodes.
		///
		///An index of -1 indicates that this node is not associated with a material.
		///
		///@return Shader node index.
		S32	GetNodeIndex() const;

		///\brief Adds a new dynamic plug to this node.
		///
		///@param newPlug The plug to add.
		///@return The plug index.
		virtual S32	AddPlugPtr(C_DynamicPlug* pNewPlug);

		///\brief Set the name of this node, as it's known in the original 3D package.
		///
		///This allows us to reconstruct the original material network when importing back in to
		///the 3D package. For example, a node might be of type "Phong", or "Lambert".
		///@param name The name of this node type.
		void SetOriginalNodeTypeName(const std::string& name);

		///\brief Returns the name of this node, as it's known in the original 3D package.
		///
		///@return The name of this node type.
		const std::string&	GetOriginalNodeTypeName() const;

		///\brief Returns the number of dynamic plugs on this node.
		///
		///Dynamic plugs are plugs that can be deleted and manipulated. Furthermore they are not
		///interpreted by the AFTK.
		///@return The number of dynamic plugs.
		U32	GetNumDynamicPlugs() const;

		///\brief Returns the dynamic plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		C_DynamicPlug* GetDynamicPlugPtr(U32 instance);
		
		///\brief Returns the dynamic plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		const C_DynamicPlug* GetDynamicPlugPtr(U32 instance) const;

		///\brief Returns the first occurance of a dynamic plug with the given name.
		///
		///@param name The name of the plug to search for.
		///@return The plug.
		C_DynamicPlug* GetDynamicPlugPtr(const std::string& name);
		
		///\brief Returns the first occurance of a dynamic plug with the given name.
		///
		///@param name The name of the plug to search for.
		///@return The plug.
		const C_DynamicPlug* GetDynamicPlugPtr(const std::string& name) const;

		virtual bool DeletePlug(U32 plugIndex);

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

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

		///\brief Returns a pointer to a copy of this node.
		///
		///@return A copy of this node.
		virtual C_ShaderNode*	Clone() const;

		std::string GetPath() const;

		///\brief Clears the contents of this node to their original state.
		///
		virtual void Clear();

		bool IsManagedResource() const;

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object& rhs, U32 flags = 0) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void SetMaterialNodePtr(C_MaterialNode* pMaterial);

		void SetStaticPlugInitialCapacity();

		void SetParentPtr(C_MaterialNode* pParent);

		void SetNodeInstance(S32 instance);

		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		C_MaterialNode*	m_pParentMaterial;
		// The index of this node in the materials array of nodes
		S32				m_nodeInstance;

		//The array of plugs containing the attribute information for this shader node.
	#ifdef AFTK_USE_MEMORY_POOL
		Ut::UtVector< Ut::DynamicVector<C_DynamicPlug*, C_ArrayAllocator<C_DynamicPlug*> > >	
			m_dynamicPlugs;
	#else
		Ut::UtVector< Ut::DynamicVector<C_DynamicPlug*> >	m_dynamicPlugs;
	#endif

		std::string	m_originalNodeTypeName;

		friend class C_MaterialNode;
		friend class C_NativeShaderNode;

	#endif
	};
}

#endif
