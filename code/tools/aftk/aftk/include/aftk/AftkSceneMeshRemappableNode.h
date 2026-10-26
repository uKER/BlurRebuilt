#ifndef AFTK_I_SCENE_MESH_REMAPPABLE_NODE_H
#define AFTK_I_SCENE_MESH_REMAPPABLE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_Scene;
	class C_Polygon;
	class C_MaterialNode;
	class I_SceneMeshNode;
	class C_ShaderImageNode;

	typedef	Ut::OpenHashMap<const C_MaterialResource*, const C_MaterialResource*, Ut::HashKeyOpPtr > 
		MaterialRemappingHashTable;

	class C_SceneMeshRemappableNode : public I_SceneMeshNode
	{
	public:
		struct C_UvLink 
		{
			const C_ShaderImageNode* pImageNode;
			unsigned int uvIndex;
		};

		///\brief Constructor.
		///
		///@param pScene The scene to which this mesh is attached.
		///
		C_SceneMeshRemappableNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene, const MaterialRemappingHashTable& mappingTable) const = 0;

		///
		///\brief Copy constructor.
		///
		///@param rhs The mesh node to copy.
		C_SceneMeshRemappableNode(const C_SceneMeshRemappableNode& rhs);

		///
		///\brief Asignment operator.
		///
		///@param rhs The mesh node to copy.
		///@return A reference to this mesh node.
		const C_SceneMeshRemappableNode& operator=(const C_SceneMeshRemappableNode& rhs);

		const C_SceneMeshRemappableNode& Copy(const C_SceneMeshRemappableNode& rhs, const MaterialRemappingHashTable* pMappingTable);

		///\brief Destructor.
		///
		virtual	~C_SceneMeshRemappableNode();

		///\brief Set the value of a texture coordinate offset.
		///
		///@param set The texture coordinate set.
		///@param offset The offset.
		virtual void SetTextureCoordinateOffset(U32 set, const TextureCoordinate4& offset);

		///\brief Sets the resource that for a given entry in the blind data mapping table.
		///
		///@param index The mapping index to modify.
		///@param pBlindData The resource for this index.
		virtual void SetPolygonBlindDataMapping(U16 index, const C_BlindDataResource* pBlindData);

		///\brief Sets the resource that for a given entry in the material mapping table.
		///
		///@param index The mapping index to modify.
		///@param pMaterial The resource for this index.
		virtual void SetPolygonMaterialMapping(U16 index, const C_MaterialResource* pMaterial);

		///\brief Returns the texture coordinate offset for a given texture coordinate set.
		///
		///@param set The texture coordinate set.
		///@return The texture coordinate offset.
		const TextureCoordinate4& GetTextureCoordinateOffset(U32 set) const;

		///\brief Returns the blind data resource at a given position in the blind data mapping table.
		///
		///@param index The blind data mapping table index.
		///@return The blind data resource.
		const C_BlindDataResource* GetPolygonBlindDataMappingPtr(S16 index) const;

		///\brief Returns the material resource at a given position in the material mapping table.
		///
		///@param index The material mapping table index.
		///@return The material resource.
		const C_MaterialResource* GetPolygonMaterialMappingPtr(S16 index) const;

		void GetPolygonBlindDataMappingArray(
			Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<C_BlindDataNode>* > >& mapping) const;

		void GetPolygonMaterialMappingArray(
			Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<C_MaterialNode>* > >& mapping) const;

		///\brief Returns the reference count for the resource at a given blind data mapping table index.
		///
		///@param index The blind data mapping table index.
		///@return The reference count.
		virtual U32 GetPolygonBlindDataMappingReferenceCount(S16 index) const;

		///\brief Returns the reference count for the resource at a given material mapping table index.
		///
		///@param index The material mapping table index.
		///@return The reference count.
		virtual U32 GetPolygonMaterialMappingReferenceCount(S16 index) const;

		///\brief Returns the blind data mapping table index for a given blind data resource.
		///
		///@param pResource The blind data resource.
		///@return The blind data mapping table index.
		virtual S16 GetPolygonBlindDataMappingIndex(const C_BlindDataResource* pResource) const;

		///\brief Returns the material mapping table index for a given material resource.
		///
		///@param pResource The material resource.
		///@return The blind data mapping table index.
		virtual S16 GetPolygonMaterialMappingIndex(const C_MaterialResource* pResource) const;

		///\brief Returns the number of entries in the blind data mapping table.
		///
		///@return The number of blind data mappings.
		virtual U16 GetNumPolygonBlindDataMappings() const;

		///\brief Returns the number of entries in the material mapping table.
		///
		///@return The number of material mappings.
		virtual U16 GetNumPolygonMaterialMappings() const;

		///\brief Copies the blind data mapping table from the given node.
		///
		///@param rhs The node to copy the mapping table from.
		virtual void CopyPolygonBlindDataMappings(const C_SceneMeshRemappableNode* rhs);

		///\brief Copies the material mapping table from the given node.
		///
		///@param rhs The node to copy the mapping table from.
		virtual void CopyPolygonMaterialMappings(const C_SceneMeshRemappableNode* rhs, const MaterialRemappingHashTable* pMappingTable);

		///\brief Copies the texture coordinate offsets from the given node.
		///
		///@param rhs The node to copy the texture coordinate offsets from.
		virtual void CopyTextureCoordinateOffsets(const C_SceneMeshRemappableNode* rhs);

		///\brief Equivilance operator.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and rhs are equivilant. Otherwise 'false' is returned.
		bool operator== (const C_SceneMeshNode &rhs) const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(
			const I_Object &rhs,
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

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

		///\brief Clears the contents of this node to their initial state.
		///
		virtual void Clear();

		///\brief Returns the index of a given material in the local material palette.
		///
		///@param material The material to search for.
		///@return The index of the material in the palette. -1 is returned if the material is not found.
		S16 Exists(const C_MaterialNode& material);

		///\brief Returns the index of a given blind data node in the local blind data node palette.
		///
		///@param blindData The blind data node to search for.
		///@return The index of the blind data node in the palette. -1 is returned if the blind data node is not found.
		S16 Exists(const C_BlindDataNode& blindData);

		///\brief Removes entries from the material palette that have a reference count of 0.
		///
		///@return The number of materials entries removed.
		U16 DeleteUnusedPolygonMaterialMappings();

		///\brief Removes entries from the blind data node palette that have a reference count of 0.
		///
		///@return The number of blind data entries removed. 
		U16 DeleteUnusedPolygonBlindDataMappings();

		///\brief Get the uv set index of the given C_ShaderImageNode
		///
		S32 GetUVSetIndex(
			const C_MaterialResource* pRes,
			const C_ShaderImageNode* pShaderImageNode, 
			C_Status* pStatus = 0) const;

		U32 GetNumUVLinks(const C_MaterialResource* pRes);

		const C_UvLink& GetUVLink(const C_MaterialResource* pRes, U32 index);

		///\brief Add the given uv set index to the mesh's UVLinkageList
		///
		void AddUVSetIndex(
			const C_MaterialResource* pRes,
			const C_ShaderImageNode* pShaderImageNode, 
			S32 index);

		///\brief Remap all pointers within the Mesh's UVLinkageList
		///
		void RemapUVLinkageMaterialPtr(const C_MaterialResource* pOldMaterialRes, const C_MaterialResource* pNewMaterialRes);

		void DeleteUVLinks(const C_MaterialResource* pRes);

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object &rhs, U32 flags = E_Comparison_LocalTransform) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		U32 ProcessProblems(bool fix);

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToAll(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		virtual bool IsRemappedSource() const = 0;

		virtual S16 AddPolygonBlindDataMapping(const C_BlindDataResource* pResource, bool forceAddition = false);

		virtual S16 AddPolygonMaterialMapping(const C_MaterialResource* pResource, bool forceAddition = false);

		///
		///\brief ONLY to be used during reading from disk.
		///
		///@param pResource The shared resource to add.
		///@param refCountToSet The local reference count for this resource.
		S16 AddPolygonBlindDataMappingSetCount(const C_BlindDataResource* pResource, U32 refCountToSet);

		///
		///\brief ONLY to be used during reading from disk.
		///
		///@param pResource The shared resource to add.
		///@param refCountToSet The local reference count for this resource.
		S16 AddPolygonMaterialMappingSetCount(const C_MaterialResource* pResource, U32 refCountToSet);

		void IncrementPolygonMaterialMappingReferenceCount(U16 index);

		void IncrementPolygonBlindDataMappingReferenceCount(U16 index);

		void DecrementPolygonMaterialMappingReferenceCount(U16 index);

		void DecrementPolygonBlindDataMappingReferenceCount(U16 index);

		virtual void DeletePolygonBlindDataMapping(U16 mappingIndex);

		virtual void DeletePolygonMaterialMapping(U16 mappingIndex);

		void ClearMappingTables();

		template <class T>
		S16 GetPolygonResourceMappingIndex(
			const Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<T>* > >& polyMappingArray,
			const C_SharedResource<T>* pResource) const
		{
			for (U32 i = 0; i < (U32)polyMappingArray.Size(); i++)
			{
				if (polyMappingArray[i]->GetSharedResourcePtr() == pResource)
					//We've already got this resource.
					return polyMappingArray[i]->GetIndex();
			}
			//We've not already got this resource.
			return -1;
		}

		template <class T>
		void IncrementPolygonResourceMappingReferenceCount(
			Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<T>* > >& polyMappingArray,
			U16 index)
		{
			polyMappingArray[index]->IncrementReferenceCount();
		}

		template <class T>
		void DecrementPolygonResourceMappingReferenceCount(
			Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<T>* > >& polyMappingArray,
			U16 index)
		{
			polyMappingArray[index]->DecrementReferenceCount();
		}

		template <class T>
		S16 AddPolygonResourceMapping(
			Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<T>* > >& polyMappingArray,
			const C_SharedResource< T >* pResource,
			bool forceAddition)
		{
			if (pResource == 0)
			{
				//Nothing to add!
				return -1;
			}

			S16 it = -1;
			bool addNewResource = false;

			//If we're not a remapped instance source then the only way a mapping can be added
			//is by making an addition to our base instance, therefore we must always add mapping
			//entries here. E.g. when loading from disk to make sure we're in sync with our parent.
			if (!IsRemappedSource())
				addNewResource = true;
			else
			{
				if (forceAddition)
					addNewResource = true;
				else
				{
					it = GetPolygonResourceMappingIndex(polyMappingArray, pResource);
					if (it == -1)
						addNewResource = true;
				}
			}

			if (addNewResource)
			{
				//We've not encountered this resource on this mesh yet, add an entry for it.
				polyMappingArray.PushBack();
				U16 mappingIndex = polyMappingArray.Size()-1;

				C_SharedResourceMapping<T>* pMapping = new C_SharedResourceMapping<T>;

				pMapping->SetSharedResourcePtr(pResource);

				//Let setup this shared resource mapping.
				pMapping->SetIndex(mappingIndex);

				//Note that our initial reference count is zero!
				pMapping->SetReferenceCount(0);

				//Insert into our array...
				polyMappingArray[mappingIndex] = pMapping;
#ifdef BUILD_CONFIG_DEBUG
				U32 oldRefCount = pResource->GetReferenceCount();
#endif
				m_pScene->IncrementResourceReferenceCount(pResource);
#ifdef BUILD_CONFIG_DEBUG
				U32 newRefCount = pResource->GetReferenceCount();
				AFTK_ASSERT(newRefCount = oldRefCount + 1);
#endif
				return mappingIndex;
			}

			//Else We've already encountered this material before, increase its refernce count.
			C_SharedResourceMapping<T>* pMapping = polyMappingArray[it];

			return pMapping->GetIndex();
		}

		template <class T>
		void DeletePolygonResourceMapping(
			Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<T>* > >& polyMappingArray,
			U16 mappingIndex)
		{
			AFTK_ASSERT(m_pScene);
			AFTK_ASSERT((U16)mappingIndex < (U16)polyMappingArray.Size());

			C_SharedResourceMapping<T>* pMapping =
				polyMappingArray[mappingIndex];
			AFTK_ASSERT(pMapping);

			//Delete the mapping entry for this resource after first shuffling all of the indices
			//for the entries above it in the list.
			for (U16 i = mappingIndex+1; i < (U16)polyMappingArray.Size(); i++)
			{
				C_SharedResourceMapping<T>* pMappingToShuffle =
					polyMappingArray[i];

				AFTK_ASSERT(pMappingToShuffle->GetIndex() == i);
				AFTK_ASSERT(pMappingToShuffle->GetIndex() != 0);

				//Decrement resource mapping index.
				pMappingToShuffle->SetIndex(pMappingToShuffle->GetIndex() - 1);
			}

			//Erase from mapping array.
			polyMappingArray.EraseByIndex(mappingIndex);

			m_pScene->DecrementResourceReferenceCount(pMapping->GetSharedResourcePtr());

			//Free the memory allocated to our redundant mapping
			delete pMapping;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Member variables
		////////////////////////////////////////////////////////////////////////////////

		typedef Ut::UtVector<Ut::DynamicVector<C_UvLink> > UVLinkageList;

		Ut::UtMap<const Aftk::C_MaterialResource*, UVLinkageList>	m_uvLinkInfo;


		Ut::UtVector< Ut::DynamicVector<TextureCoordinate4> >	
			m_uvwxOffsets;

		//The array below may be overridden by mesh instances.
		Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<C_BlindDataNode>* > >
			m_polygonBlindDataMappingArray;

		//The array below may be overridden by mesh instances.
		Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<C_MaterialNode>* > >
			m_polygonMaterialMappingArray;

		friend class C_Scene;
		friend class C_Polygon;

		friend class C_NativeSceneMeshRemappableNode;
		friend class C_SceneMeshNode;
		friend class C_NativePolygon;

	#endif
	};
}
#endif
