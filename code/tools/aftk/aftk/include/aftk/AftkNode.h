#ifndef AFTK_NODE_H
#define AFTK_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
#ifndef AFTK_EXTRA_LEAN_AND_MEAN
	class C_AttachData;
#endif
	class I_Plug;
	class C_DynamicPlug;
	struct C_PlugLogicalIndexList;

	class C_Node : public I_Object
	{
	public:
		///\brief Default constructor.
		///
		C_Node(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor
		///
		///@param rhs The source node for the copy operation
		C_Node(const C_Node& rhs);

		///\brief Destructor.
		virtual ~C_Node();

		///\brief Asignment operator.
		///
		///@param rhs The source node for the copy operator.
		C_Node& operator= (const C_Node& rhs);

		///\brief Equivilance operator which operates on entire class heirachy.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool operator== (const C_Node &rhs) const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = 0) const;

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
		bool operator!= (const C_Node &aftkNode);

#ifndef AFTK_EXTRA_LEAN_AND_MEAN
		///\brief Attaches a pointer to some user data.
		///
		///Note that this memory is deallocated by the AFTK, not the user.
		///@param attachDataPtr data to attach
		void SetAttachData(const C_AttachData& attachDataPtr);
#endif

		///\brief Sets the name of this node.
		///
		///@param name New node name.
		void SetName(const std::string &name);

		///\brief Retruns the name of this node.
		///
		///@return C_Node name.
		virtual const std::string& GetName() const;

		///\brief Retruns the name of this node.
		///
		///@return C_Node name.
		std::string& GetName()
		{
			return const_cast<std::string&>(((const C_Node*)this)->GetName());
		}

#ifndef AFTK_EXTRA_LEAN_AND_MEAN
		///\brief Get the user data occupying the given position in the user data array.
		///
		///@return user data.
		const C_AttachData* GetAttachDataPtr() const;

		///\brief Get the user data occupying the given position in the user data array.
		///
		///@return user data.
		C_AttachData*	GetAttachDataPtr();
#endif

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

#ifndef AFTK_EXTRA_LEAN_AND_MEAN
		///\brief Deletes the user attached data at the given index in the array of attached user data for this node. 
		void DeleteAttachData();
#endif

		virtual std::string GetPath() const;

		///\brief Get a pointer to the scene this node belongs to.
		///
		///@return The scene pointer.
		const C_Scene* GetScenePtr() const;

		///\brief Get a pointer to the scene this node belongs to.
		///
		///@return The scene pointer.
		C_Scene* GetScenePtr();

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		 const I_Plug* GetPlugPtr(U32 instance)  const;
		
		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		I_Plug* GetPlugPtr(U32 instance);

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		const I_Plug* GetPlugPtr(const C_PlugLogicalIndexList& locationIndices) const;
		
		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		I_Plug* GetPlugPtr(const C_PlugLogicalIndexList& locationIndices);

		///\brief Returns the first occurance of a plug with the given name.
		///
		///The plug returned can be either static or dynamic.
		///
		///@param name The name of the plug to search for.
		///@return The plug.
		const I_Plug* GetPlugPtr(const std::string& name) const;

		///\brief Returns the first occurance of a plug with the given name.
		///
		///The plug returned can be either static or dynamic.
		///
		///@param name The name of the plug to search for.
		///@return The plug.
		I_Plug* GetPlugPtr(const std::string& name);
		
		///\brief Returns the number of static plugs on this node.
		///
		///@return The number of dynamic plugs.
		U32	GetNumPlugs() const;

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual I_Plug* GetStaticPlugPtr(U32 instance);
		
		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual const I_Plug* GetStaticPlugPtr(U32 instance) const;
		
		///\brief Returns the number of static plugs on this node.
		///
		///@return The number of dynamic plugs.
		virtual U32	GetNumStaticPlugs() const;

		///\brief Returns the dynamic plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual C_DynamicPlug* GetDynamicPlugPtr(U32 instance);

		///\brief Returns the dynamic plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual const C_DynamicPlug* GetDynamicPlugPtr(U32 instance) const;
		
		///\brief Returns the number of dynamic plugs on this node.
		///
		///Dynamic plugs are plugs that can be deleted and manipulated. Furthermore they are not
		///interpreted by the AFTK.
		///@return The number of dynamic plugs.
		virtual U32	GetNumDynamicPlugs() const;

		virtual bool DeletePlug(U32 plugIndex);

		virtual bool IsUsed() const;
		///\brief Clears the contents of this node.
		///
		virtual void Clear();

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object& rhs, U32 flags = 0) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit(C_Scene* pScene);

		void ClearThis();

		void SetScenePtr(C_Scene* pScene);

		bool IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		std::string		m_name;

#ifndef AFTK_EXTRA_LEAN_AND_MEAN
		C_AttachData*	m_pAttachedData;
#endif

		C_Scene*		m_pScene;

		friend class C_NativeMaterialNode;
		friend class C_NativeNode;

	#endif
	};
}

#endif
