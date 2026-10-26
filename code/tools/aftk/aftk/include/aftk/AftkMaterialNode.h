#ifndef AFTK_MATERIAL_NODE_H
#define AFTK_MATERIAL_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_GenericAttribute;

	class C_MaterialNode : public C_Node
	{
	public:
		
		///\brief Constructor.
		///
		C_MaterialNode();

		C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor.
		///
		///@param rhs The node to copy.
		C_MaterialNode(const C_MaterialNode &rhs);

		///\brief Destructor.
		///
		virtual ~C_MaterialNode();

		///\brief Asignment operator.
		///
		///@param rhs The node to copy.
		C_MaterialNode& operator=(const C_MaterialNode &rhs);
		
		///\brief Equality test.
		///
		///Tests the contents of this node with rhs, returning true if they are equal.
		///@param rhs The node to test against.
		///@return True if equal.
		bool operator==(const C_MaterialNode &rhs) const;

		///\brief Inequality test.
		///
		///Tests the contents of this node with rhs, returning true if they are not equal.
		///@param rhs The node to test against.
		///@return True if not equal.
		bool operator!=(const C_MaterialNode &rhs) const;

		///\brief Sets the initial capacity of this material's plug array.
		///
		///@param capacity The plug array capacity.
		void SetPlugInitialCapacity(U32 capacity);

		///\brief Sets the initial capacity of this material's shader node array.
		///
		///@param capacity The shader node capacity.
		void SetShaderNodeInitialCapacity(U32 capacity);

		///\brief Adds a new shader node to this material.
		///
		///Note that the material takes ownership of any pointers provided. It is therefore
		///responsible for their deletion.
		///
		///@param pShader A pointer to the shader node to add.
		///@return The index of the shader node.
		U32	AddShaderNodePtr(C_ShaderNode* pShader);

		///\brief Replaces a shader node in this material.
		///
		///Note that the material takes ownership of any pointers provided. It is therefore
		///responsible for their deletion.
		///
		///@param instance The index of the shader node to replace.
		///@param pShader A pointer to the shader node to add.
		void SetShaderNodePtr(C_Scene* pScene, const C_SharedResource<C_MaterialNode>* pRes, U32 instance, C_ShaderNode* pShader);

		///\brief Sets the origin of the material.
		///
		///The origin is an enum describing where this material originally came from.
		///This currently has the following possible values: 
		///AFTK_MATERIAL_ORIGIN_EDITOR, AFTK_MATERIAL_ORIGIN_MAYA, AFTK_MATERIAL_ORIGIN_MAYA_CGFX,
		///AFTK_MATERIAL_ORIGIN_XSI, AFTK_MATERIAL_ORIGIN_SI.
		///@param origin The material origin.
		void SetOrigin(E_MaterialOrigin origin);

		///\brief Sets the type id for this material.
		///
		///The material type id is a GUID provided by the original 3D package that describes the type
		///of material structure represented by this node.
		///@param id The material id.
		void SetMaterialTypeId(U32 id);

		///\brief Returns the type id for this material.
		///
		///The material type id is a GUID provided by the original 3D package that describes the type
		///of material structure represented by this node.
		///@return The material id.
		U32 GetMaterialTypeId() const;

		///\brief Returns the shader node at the given index.
		///
		///@param instance The shader node index.
		///@return The shader node.
		const C_ShaderNode* GetShaderNodePtr(U32 instance) const;

		///\brief Returns the shader node at the given index.
		///
		///@param instance The shader node index.
		///@return The shader node.
		C_ShaderNode*	GetShaderNodePtr(U32 instance);

		///\brief Returns the first shader node found with the given name.
		///
		///@param name The shader node name.
		///@return Returns the shader node if found, otherwise returns 0.
		const C_ShaderNode* GetShaderNodePtr(const std::string& name) const;

		///\brief Returns the first shader node found with the given name.
		///
		///@param name The shader node name.
		///@return Returns the shader node if found, otherwise returns 0.
		C_ShaderNode*	GetShaderNodePtr(const std::string& name);

		///\brief Returns the origin of this material node.
		///
		///The origin is an enum describing where this material originally came from.
		///This currently has the following possible values: 
		///AFTK_MATERIAL_ORIGIN_EDITOR, AFTK_MATERIAL_ORIGIN_MAYA, AFTK_MATERIAL_ORIGIN_MAYA_CGFX,
		///AFTK_MATERIAL_ORIGIN_XSI, AFTK_MATERIAL_ORIGIN_SI.
		///@return The material origin.	
		E_MaterialOrigin GetOrigin() const;

		///\brief Adds a new plug to this material.
		///
		///@param aftkPlug The plug to add.
		///@return The plug index.
		S32	AddPlugPtr(C_DynamicPlug* pAftkPlug);
		
		///\brief Returns the plug at the given index.
		///
		///@param instance The plug index.
		const C_DynamicPlug*	GetPlugPtr(U32 instance) const;

		///\brief Returns the plug at the given index.
		///
		///@param instance The plug index.
		C_DynamicPlug* GetPlugPtr(U32 instance);

		///\brief Returns the number of plugs on this material.
		///
		///@return The number of plugs.
		U32	GetNumPlugs() const;

		///\brief Returns the number of shader nodes on this material.
		///
		///@return The number of shader nodes.
		U32	GetNumShaderNodes() const;

		///\brief Specifically returns the number image shader nodes on this material.
		///
		///@return The number of image shader nodes.
		U32	GetNumShaderImageNodes() const;

		///\brief Returns the nth shader image node.
		///
		///@param instance The shader image node instance to return.
		///@return The shader image node.
		const C_ShaderImageNode* GetShaderImageNodePtr(U32 instance) const;

		///\brief Returns the nth shader image node.
		///
		///@param instance The shader image node instance to return.
		///@return The shader image node.
		C_ShaderImageNode* GetShaderImageNodePtr(U32 instance);

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
		bool HasType(E_ObjType type) const;

		///\brief Displays given material resource's contents
		///
		///Contents are displayed via the output manager. A callback to this can be registered in by a
		///call to 'SetOutputNotificationCallback'.
		///@param pMaterial The material to write.
		static void Output(const C_MaterialNode& material);

		///\brief Clears the contents of this node to their initial state.
		///
		virtual void Clear();

		bool IsEqualToThis(const I_Object &_rhs) const;

		//New
		void* operator new (size_t size);

		//Delete.
		void operator delete(void* p);

		//Placement new.
		void* operator new (size_t size, void* pMem) {return pMem;}

		//Placemnt delete.
		void operator delete(void* pMem, void* p) {}

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown) const;

		bool IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown) const;

		void AddPlugToPointerMapping(
			Ut::UtMap<const I_Object*, I_Object*>& pointerMapping, 
			const I_Plug* oldPlug,
			I_Plug* newPlug);

		void ResolvePlugPointerMapping(
			const Ut::UtMap<const I_Object*, I_Object*>& pointerMapping, 
			const I_Plug* oldPlug,
			I_Plug* newPlug);

		bool IsManagedResource() const;

	#ifdef AFTK_USE_MEMORY_POOL
		//The array of plugs containing the attribute information for this shader node.
		Ut::UtVector< Ut::DynamicVector<C_DynamicPlug*, C_ArrayAllocator<C_DynamicPlug*> > >					m_plugs;

		Ut::UtVector< Ut::DynamicVector<C_ShaderNode*, C_ArrayAllocator<C_ShaderNode*> > >	m_shaders;
	#else
		//The array of plugs containing the attribute information for this shader node.
		Ut::UtVector< Ut::DynamicVector<C_DynamicPlug*> >	m_plugs;

		Ut::UtVector< Ut::DynamicVector<C_ShaderNode*> >	m_shaders;
	#endif

		E_MaterialOrigin	m_origin;
		U32					m_materialTypeId;
		U32					m_numImages;

		friend class C_NativeMaterialNode;
		friend class C_Scene;
		template<typename> friend class C_NamedResourceManager;

	#endif
	};
}

#endif
