#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_SCENE_MESH_NODE_H
#define AFTK_SCENE_MESH_NODE_H

namespace Aftk
{
	class C_Scene;
	class C_Polygon;
	class C_MaterialNode;

	class C_SceneMeshNode : public C_SceneMeshRemappableNode
	{
	public:
		///\brief Constructor.
		///
		///@param pScene The scene to which this mesh is attached.
		///
		C_SceneMeshNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		virtual C_Node* Clone(C_Scene* pScene, const MaterialRemappingHashTable& mappingTable) const;

		const C_SceneMeshNode& Copy(const C_SceneMeshNode& rhs, const MaterialRemappingHashTable* pMappingTable);

		///\brief Copy constructor.
		///
		///@param rhs The mesh node to copy.
		C_SceneMeshNode(const C_SceneMeshNode& rhs);

		///\brief Builds a new mesh from the given boned instance.
		///
		///@param rhs The boned instance node.
		C_SceneMeshNode(const C_SceneMeshBonedInstanceNode& rhs);

		///\brief Destructor.
		///
		virtual ~C_SceneMeshNode();

		///\brief Asignment operator.
		///
		///@param rhs The mesh node to copy.
		///@return A reference to this mesh node.
		const C_SceneMeshNode& operator=(const C_SceneMeshNode& rhs);

		///\brief Asignment operator. Builds a new mesh from the given boned instance.
		///
		///@param rhs The boned instance node.
		///@return A reference to this mesh node.
		const C_SceneMeshNode& operator=(const C_SceneMeshBonedInstanceNode& rhs);

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

		///\brief Generate hash of data in current class
		///
		///@return 32b hash value
		U32 GetHashOfThis() const;

		///\brief Generate hash of data in current class and hierachy
		///
		///@return 32b hash value
		virtual U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const;

		///\brief Sets the initial capacity of this mesh's array of vertex positions.
		///
		///@param capacity The initial capacity.
		void SetVertexPositionInitialCapacity(U32 capacity);

		///\brief Sets the initial capacity of this mesh's array of vertex normals.
		///
		///@param capacity The initial capacity.
		void SetVertexNormalInitialCapacity(U32 capacity);

		///\brief Increment the size of this mesh's array of texture coordinates.
		///
		S32 AddTextureCoordinateSet();

		///\brief Sets the size of this mesh's array of texture coordinates.
		///
		///@param size The number of texture coordinate sets desired.
		void SetTextureCoordinateSetSize(U32 size);

		///\brief Sets the initial capacity of a given UV set's array of texture coordinates
		///
		///@param set The UV set to have its capacity set.
		///@param capacity The initial capacity.
		void SetTextureCoordinateInitialCapacity(U32 set, U32 capacity);

		///\brief Sets the initial capacity of a given UV set's array of tangents
		///
		///@param set The UV set to have its capacity set.
		///@param capacity The initial capacity.
		void SetVertexTangentInitialCapacity(U32 set, U32 capacity);

		///\brief Sets the initial capacity of a given UV set's array of tangents
		///
		///@param set The UV set to have its capacity set.
		///@param capacity The initial capacity.
		void SetVertexBinormalInitialCapacity(U32 set, U32 capacity);

		///\brief Sets the initial capacity of this mesh's array of polygons.
		///
		///@param lodLevel The polygon set to modify.
		///@param capacity The initial capacity.
		void SetPolygonInitialCapacity(U32 lodLevel, U32 capacity);

		///\brief Sets the initial capacity of this mesh's array of polygons.
		///
		///@param capacity The initial capacity.
		void SetPolygonInitialCapacity(U32 capacity);

		///\brief Sets the initial capacity of this mesh's array of polygon vertices.
		///
		///@param capacity The initial capaciy.
		void SetPolygonVertexInitialCapacity(U32 capacity);

		///\brief Sets the initial capacity of this mesh's array of polygon cluster.
		///
		///@param capacity The initial capacity.
		void SetPolygonClusterInitialCapacity(U32 capacity);

		///\brief Adds a new vertex position.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///@param position The new vertex position to add.
		///@return The index of the new vertex in the mesh's vertex array.
		S32	AddVertexPosition(const Position3& position = Position3());

		///\brief Adds a new vertex normal.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///@param normal The new vertex position to add.
		///@return The index of the new normal in the mesh's normal array.
		S32	AddVertexNormal(const Normal3& normal = Normal3());

		///\brief Adds a new texture coordinate.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///@param set The UV set into which the new texture coordinate will be added.
		///@param uvwx The new texture coordinate to add.
		///@return The index of the new texture coordinate in the relevant UV set.
		S32	AddTextureCoordinate(U32 set, const TextureCoordinate4& uvwx = TextureCoordinate4());

		///\brief Adds a new tangent vector to a texture coordinate set.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///@param set The UV set into which the new tangent will be added.
		///@param tangent The new tangent to add.
		///@return The index of the new tangent in the relevant UV set.
		S32	AddVertexTangent(U32 set, const Vector3F32& tangent = Vector3F32());

		///\brief Adds a new binormal vector to a texture coordinate set.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///@param set The UV set into which the new binormal will be added.
		///@param binormal The new binormal to add.
		///@return The index of the new binormal in the relevant UV set.
		S32	AddVertexBinormal(U32 set, const Vector3F32& binormal = Vector3F32());

		///\brief Adds a new polygon vertex.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///@param polyVert The new polygon vertex.
		///@return The index of the new polygon vertex.
		S32 AddPolygonVertex(const C_PolygonVertex& polyVert = C_PolygonVertex());

		///\brief Adds a new polygon cluster.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		S32 AddPolygonCluster();
		
		///\brief Set the name of a texture coordinate set.
		///
		///@param set The UV set to name.
		///@param name Texture coordinate set name.
		void SetTextureCoordinateSetName(U32 set, const std::string& name);

		///\brief Adds a new channel of vertex data, and uses the description provided.
		///
		///@param info Vertex data channel information.
		///@return The index of the new channel.
		S32 AddVertexDataChannelInfo(const C_VertexDataChannelInfo& info);

		///\brief Set vertex channel information.
		///
		///@param channel The vertex data channel to name.
		///@param info Vertex data channel information.
		S32 SetVertexDataChannelInfo(U32 channel, const C_VertexDataChannelInfo& info);

		///\brief Deletes a vertex data channel.
		///
		///@param channel The vertex data channel to delete.
		void DeleteVertexDataChannelInfo(U32 channel);

		///\brief Merges the given mesh with the current mesh.
		///
		///@param pMesh The mesh from which to copy.
		C_Status AddMesh(
			const C_SceneMeshNode* pMesh,
			bool mergeVertexChannelsByName = false);
		
		///\brief Sets the Lod info for the given lod level.
		///
		///@param lodLevel The lod level to modify.
		///@param info Lod level information.
		void SetLodLevelInfo(U32 lodLevel, const C_MeshLodLevelInfo& info);

		///\brief Inserts a new lod level before the given index.
		///
		///@param lodLevel The lod level before which to insert.
		///@param info Lod level information.
		void InsertLodLevelInfo(U32 lodLevel, const C_MeshLodLevelInfo& info);

		///\brief Copies the first Lod level from the given mesh into a specified lod level of this mesh.
		///
		///@param lodLevel The lod level to modify.
		///@param pMeshLodLevel The mesh from which to copy.
		C_Status SetLodLevelMesh(
			U32 lodLevel,
			const C_SceneMeshNode* pMeshLodLevel, 
			bool mergeWeightsByEnvIndex,
			bool mergeVertexChannelsByName = false);

		///\brief Adds a new polygon.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@param aftkPolygon The polygon to be added.
		///@return The index of the new polygon in the mesh's polygon array.
		S32	AddPolygon(const C_Polygon& aftkPolygon = C_Polygon());

		void DeletePolygon(U32 lodLevel, U32 index);

		///\brief Adds a new polygon.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@param lodLevel The polygon set in to which the new polygon will be added.
		///@param cluster The polygon cluster into which the polygon will be added.
		///@param aftkPolygon The polygon to be added.
		///@return The index of the new polygon in the mesh's polygon array.
		S32	AddPolygon(U32 lodLevel, U32 cluster, const C_Polygon& aftkPolygon);

		///\brief Overwrites an existing vertex position.
		///
		///@param instance The vertex position to overwrite.
		///@param position The new vertex position.
		void SetVertexPosition(U32 instance, const Position3& position);
		
		///\brief Overwrites an existing vertex normal.
		///
		///@param instance The vertex normal to overwrite.
		///@param normal The new vertex normal.
		void SetVertexNormal(U32 instance, const Normal3& normal);
		
		///\brief Overwrites an existing texture coordinate.
		///
		///@param set The UV set in which the target coordinate resides.
		///@param instance The index of the texture coordinate to overwrite within the given set.
		///@param uvwx The new texture coordinate.
		void SetTextureCoordinate(U32 set, U32 instance, const TextureCoordinate4& uvwx);

		///\brief Overwrites an existing vertex tangent.
		///
		///@param set The UV set in which the target tangent resides.
		///@param instance The index of the tangent to overwrite within the given set.
		///@param tangent The new tangent.
		void SetVertexTangent(U32 set, U32 instance, const Vector3F32& tangent);

		///\brief Overwrites an existing vertex binormal.
		///
		///@param set The UV set in which the target binormal resides.
		///@param instance The index of the binormal to overwrite within the given set.
		///@param binormal The new binormal.
		void SetVertexBinormal(U32 set, U32 instance, const Vector3F32& binormal);

		///\brief Overwrites an existing polygon vertex.
		///
		///@param instance The index of the polygon vertex to overwrite.
		///@param polyVert The new polygon vertex.
		void SetPolygonVertex(U32 instance, const C_PolygonVertex& polyVert);
		
		///\brief Overwrites an existing polygon.
		///
		///@param lodLevel The polygon set to modify.
		///@param instance The polygon to overwrite.
		///@param aftkPolygon The new polygon.
		void SetPolygon(U32 lodLevel, U32 instance, const C_Polygon& aftkPolygon);

		///\brief Overwrites an existing polygon.
		///
		///@param lodLevel The polygon set to modify.
		///@param instance The polygon to overwrite.
		///@param aftkPolygon The new polygon.
		void SetPolygon(U32 instance, const C_Polygon& aftkPolygon);

		///\brief Fetches a vertex position.
		///
		///@param instance The vertex position to retrieve.
		///@return The vertex position.	
		const Position3& GetVertexPosition(U32 instance) const;

		const Position3Array& GetVertexPositionArray() const;
		
		///\brief Fetches a vertex position.
		///
		///@param instance The vertex position to retrieve.
		///@return The vertex position.	
		Position3& GetVertexPosition(U32 instance);

		Position3Array& GetVertexPositionArray();

		///\brief Fetches a vertex normal
		///
		///@param instance The vertex normal to retrieve.
		///@return The vertex normal.	
		const Normal3& GetVertexNormal(U32 instance) const;

		const Normal3Array& GetVertexNormalArray() const;

		///\brief Fetches a vertex normal.
		///
		///@param instance The vertex normal to retrieve.
		///@return The vertex normal.	
		Normal3& GetVertexNormal(U32 instance);

		Normal3Array& GetVertexNormalArray();

		///\brief Fetches a texture coordinate.
		///@param set The texture coordinate set from which to retrieve the texture coordinate.
		///@param instance The texture coordinate to fetch.
		///@return The texture coordinate.
		const TextureCoordinate4& GetTextureCoordinate(U32 set, U32 instance) const;

		const TextureCoordinate4Array& GetTextureCoordinateArray(U32 set) const;

		///\brief Fetches a vertex tangent.
		///
		///@param set The texture coordinate set from which to retrieve the vertex tangent.
		///@param instance The vertex tangent to fetch.
		///@return The vertex tangent.
		const Vector3F32& GetVertexTangent(U32 set, U32 instance) const;

		///\brief Fetches a vertex binormal.
		///
		///@param set The texture coordinate set from which to retrieve the vertex binormal.
		///@param instance The vertex binormal to fetch.
		///@return The vertex binormal.
		const Vector3F32& GetVertexBinormal(U32 set, U32 instance) const;

		///\brief Fetches a texture coordinate.
		///@param set The set from which to retrieve the texture coordinate.
		///@param instance The texture coordinate set from which to retrieve the texture coordinate.
		///@return The texture coordinate.
		TextureCoordinate4&	GetTextureCoordinate(U32 set, U32 instance);

		TextureCoordinate4Array& GetTextureCoordinateArray(U32 set);

		///\brief Returns a vertex tangent.
		///
		///@param set The texture coordinate set from which to retrieve the vertex tangent.
		///@param instance The index of the vertex tangent to fetch.
		///@return The vertex tangent.
		Vector3F32& GetVertexTangent(U32 set, U32 instance);

		///\brief Returns a vertex binormal.
		///
		///@param set The texture coordinate set from which to retrieve the vertex binormal.
		///@param instance The index of the vertex binormal to fetch.
		///@return The vertex binormal.
		Vector3F32& GetVertexBinormal(U32 set, U32 instance);

		///\brief Returns the index of the first texture coordinate set with the given name.
		///
		///@param name The texture coordinate set name.
		///@return The texture coordinate set index. -1 if not found.
		S32	GetTextureCoordinateSetIndex(const std::string& name) const;

		C_TextureCoordinateSet& GetTextureCoordinateSet(U32 set);

		const C_TextureCoordinateSet& GetTextureCoordinateSet(U32 set) const;
		
		///\brief Fetches a texture coordinate set name.
		///
		///@param set The set from which to retrieve the the name.
		///@return The name of the given texture coordinate set.
		const std::string& GetTextureCoordinateSetName(U32 set) const;

		///\brief Fetches vertex channel information.
		///
		///@param channel The channel from which to retrieve the the info.
		///@return The vertex channel information.
		const C_VertexDataChannelInfo& GetVertexDataChannelInfo(U32 channel) const;
		
		///\brief Fetches vertex channel information.
		///
		///@param channel The channel from which to retrieve the the info.
		///@return The vertex channel information.
		C_VertexDataChannelInfo& GetVertexDataChannelInfo(U32 channel);

		///\brief Fetches lod level information.
		///
		///@param lodLevel The lod level from which to retrieve the the info.
		///@return The lod level information.
		const C_MeshLodLevelInfo& GetLodLevelInfo(U32 lodLevel) const;

		///\brief Fetches lod level information.
		///
		///@param lodLevel The lod level from which to retrieve the the info.
		///@return The lod level information.
		C_MeshLodLevelInfo& GetLodLevelInfo(U32 lodLevel);

		///\brief Fetches a polygon.
		///
		///@param lodLevel The polygon set to query.
		///@param instance The polygon to retrieve.
		///@return The polygon.	
		const C_Polygon& GetPolygon(U32 lodLevel, U32 instance) const;

		///\brief Fetches a polygon.
		///
		///@param lodLevel The polygon set to query.
		///@param instance The polygon to retrieve.
		///@return The polygon.	
		C_Polygon& GetPolygon(U32 lodLevel, U32 instance);

		///\brief Fetches a polygon.
		///
		///@param instance The polygon to retrieve.
		///@return The polygon.	
		const C_Polygon& GetPolygon(U32 instance) const;

		///\brief Fetches a polygon.
		///
		///@param instance The polygon to retrieve.
		///@return The polygon.	
		C_Polygon& GetPolygon(U32 instance);

		PolygonArray& GetPolygonArray(U32 lodLevel);

		const PolygonArray& GetPolygonArray(U32 lodLevel) const;

		///\brief Returns a polygon vertex.
		///
		///@param instance The index of the polygon vertex to return.
		///@return The polygon vertex.
		C_PolygonVertex& GetPolygonVertex(U32 instance);

		PolygonVertexArray& GetPolygonVertexArray();

		///\brief Returns a polygon vertex.
		///
		///@param instance The index of the polygon vertex to return.
		///@return The polygon vertex.
		const C_PolygonVertex& GetPolygonVertex(U32 instance) const;

		const PolygonVertexArray& GetPolygonVertexArray() const;

		///\brief Returns the index of a polygon cluster.
		///
		///C_Polygon clusters are groups of polygons. All polygons are always a member of one polygon 
		///cluster.
		///@param A pointer to the polygon cluster to query.
		///@return The index of the polygon cluster.
		S32 GetPolygonClusterIndex(const C_PolygonCluster* pPolygonCluster) const;

		///\brief Returns a polygon cluster.
		///
		///C_Polygon clusters are groups of polygons. All polygons are always a member of one polygon 
		///cluster.
		///@param instance The index of the polygon cluster.
		C_PolygonCluster* GetPolygonClusterPtr(U32 instance);

		///\brief Returns a polygon cluster.
		///
		///C_Polygon clusters are groups of polygons. All polygons are always a member of one polygon 
		///cluster.
		///@param instance The index of the polygon cluster.
		const C_PolygonCluster* GetPolygonClusterPtr(U32 instance) const;
		
		///Get the number of vertex positions in this mesh.
		///
		///@return Number of vertex positions.
		U32 GetNumVertexPositions() const;
		
		///\brief Get the number of vertex normals in this mesh.
		///
		///@return Number of vertex normals.
		U32	GetNumVertexNormals() const;

		///\brief Get the number of vertex tangents in this mesh.
		///@param set Texture coordinate set.
		///
		///@return Number of vertex tangents.
		U32	GetNumVertexTangents(U32 set) const;

		///\brief Get the number of vertex tangents in this mesh.
		///@param set Texture coordinate set.
		///
		///@return Number of vertex tangents.
		U32	GetNumVertexBinormals(U32 set) const;

		///\brief Get the number of vertex data entries in this mesh.
		///
		///@return Ammount of vertex data.
		U32	GetNumVertexData() const;
		
		///\brief Get the number of texture coordinates for a given mesh UV set.
		///
		///@param set UV set from which to get the number of texture coordinates.
		///@return Number of texture coordinates.
		U32	GetNumTextureCoordinates(U32 set = 0) const;
		
		///\brief Get the number of texture coordinates sets in this mesh.
		///
		///@return Number of texture coordinates sets.
		U32	GetNumTextureCoordinateSets() const;
		
		///\brief Get the number of vertex data channels in this mesh.
		///
		///@return Number of vertex data channels.
		U32	GetNumVertexDataChannels() const;

		///\brief Get the number of polygon vertices in this mesh.
		///
		///@return Number of polygon vertices.
		U32	GetNumPolygonVertices() const;

		///\brief Returns the number of LOD levels in this mesh.
		///
		///@return The number of LOD levels.
		U32 GetNumLodLevels() const;
		
		///\brief Get the number of polygons in this mesh.
		///
		///@param lodLevel The polygon set to query.
		///@return Number of polygons.
		U32	GetNumPolygons(U32 lodLevel) const;
		
		///\brief Returns the number of UNIQUE material nodes used by this mesh.
		///
		///@param lodLevel The polygon set to query.
		///@return Number of unique material nodes.
		U32	GetNumMaterialNodes(U32 lodLevel) const;

		///\brief Get the number of triangles in this mesh if it were to be triangulated.
		///
		///@return Number of triangles.
		U32 GetNumTriangles() const;

		///\brief Get the number of triangles in this mesh if it were to be triangulated.
		///
		///@param lodLevel The polygon set to query.
		///@return Number of triangles.
		U32 GetNumTriangles(U32 lodLevel) const;

		///\brief Returns the number of polygon clusters used by this mesh.
		///
		///@return Number of polygon clusters.
		U32 GetNumPolygonClusters() const;

		///\brief Deletes a polygon cluster.
		///
		///Deleting a polygon cluster will cause the clusters polygon to be associated with the default
		///polygon cluster. This default cluster cannot be removed.
		///@param instance The index of the polygon cluster to delete.
		///@return C_Status class, indicating the result of the operation.
		C_Status DeletePolygonCluster(U32 instance);

		///\brief Deletes a polygon cluster.
		///
		///Deleting a polygon cluster will cause the clusters polygon to be associated with the default
		///polygon cluster. This default cluster cannot be removed.
		///@param pPolygonCluster The polygon cluster to delete.
		///@return C_Status class, indicating the result of the operation.
		C_Status DeletePolygonCluster(C_PolygonCluster* pPolygonCluster);

		///\brief build the adjacency information for this mesh
		///
		///@return boolean indicating success or failure.
		bool BuildAdjacency() const;

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

		///\brief Clears the contents of this mesh to their initial state.
		///
		virtual void Clear();

		///\brief Builds a list of influences (bones), weights and creates a matrix palette for this mesh.
		///
		///@return C_Status class, indicating the result of the operation.
		C_Status BuildAnimationHelpers() const;

		///\brief Removes the animation helper cache.
		///
		void ClearAnimationHelpers() const;

		///\brief Return a flag indicating whether we have previously built the animation helpers for this node.
		///
		///@return True if the helpers have been built.
		E_AnimHelperState GetAnimationHelpersBuilt() const;

		///\brief Returns a vertex weight.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@param instance The index of the vertex weight.
		///@return The vertex weight.
		const C_VertexWeight& GetVertexWeight(U32 instance) const;

		///\brief Returns the number of vertex weights for this mesh.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@return The number of vertex weights.
		U32 GetNumVertexWeights() const;

		///\brief Evaluates the matrix palette for this mesh, with the animation for this mesh
		///being a function of 'x'.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@param x The variable that the animation of this mesh is a function of.
		///@return C_Status class, indicating the result of the operation.
		virtual C_Status Evaluate() const;

		///\brief Returns a matrix palette entry.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@param instance The matrix palette index.
		///@return The matrix.
		const C_MatrixPaletteEntry& GetPaletteEntry(U32 instance);

		///\brief Returns the number of matrix palette entries for this mesh.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@return The number of matrix palette entries.
		U32 GetNumPaletteEntries();

		///\brief Build adjacency information for this mesh.
		///
		///This is an O(n^2) algorithm.
		///@return True if adjacency information was successfully built.
		bool IsAdjacencyBuilt() const;

		///\brief Get a list of adjacent polygons given a polygon index.
		///
		///@param iThisPoly C_Polygon index.
		///@return Array of adjacent polygon indices.
		const U32* GetAdjacencyList(U32 iThisPoly) const;

		///\brief Repairs mesh problems.
		///
		U32 Repair();

		///\brief Removes unsused and invalid vertex positions.
		///
		///@return The number of unused vertex positions removed.
		U32 RepairVertexPositions();

		///\brief Removes unsused and invalid vertex normals.
		///
		///@return The number of unused vertex normals removed.
		U32 RepairVertexNormals();

		///\brief Removes unsused and invalid texture coordinates.
		///
		///@return The number of unused and invalid texture coordinates repaired.
		U32 RepairTextureCoordinates(U32 set);

		///\brief Removes unsused and invalid texture coordinates.
		///
		///@return The number of unused and invalid texture coordinates repaired.
		U32 RepairTextureCoordinates();

		///\brief Removes unsused and invalid vertex tangents for the given uv sets.
		///
		///@return The number of vertex binormals removed.
		U32 RepairVertexTangents(U32 set);

		///\brief Removes unsused and invalid vertex binormals for the given uv sets.
		///
		///@return The number of vertex binormals removed.
		U32 RepairVertexBinormals(U32 set);

		///\brief Removes unsused and invalid vertex tangents for all uv sets.
		///
		///@return The number of vertex tangents removed.
		 U32 RepairVertexTangents();

		///\brief Removes unsused and invalid vertex binormals for all uv sets.
		///
		///@return The number of vertex binormals removed.
		U32 RepairVertexBinormals();

		///\brief Removes unsused and invalid polygon vertices for all lod levels.
		///
		///@return The number of polygon vertices removed.
		U32 RepairPolygonVertices();

		///\brief Checks to see whether the contents of this mesh are valid.
		///
		///If a debugger has been attached to the AFTK then a report of any encountered errors will be given.
		///@return The number of errors found.
		U32 Validate() const;

		///\brief Applies the given matrix to the contents of this node.
		///
		///@param matrix The matrix to apply.
		void ApplyMatrix(const Matrix44& matrix);

		///\brief Adds a new mesh relative material mapping.
		///
		///This function takes a shared resource pointer and adds it to a mapping table.
		///This mapping table maps from an index -> a resource pointer. A resource pointer can
		///only occur once in a table. If an attempt is made to add the same resource twice, then a 
		///reference count for that resource is incremented, and the existing index of that resource
		///in the mapping table is returned.
		///A return value of -1 from this function indicates failure.
		///@param pResource The rsource to add.
		///@return The index of the resource in this meshes mapping table.
		S16 AddPolygonMaterialMapping(const C_MaterialResource* pResource, bool forceAddition = false);

		///\brief Adds a new mesh relative blind data mapping.
		///
		///This function takes a shared resource pointer and adds it to a mapping table.
		///This mapping table maps from an index -> a resource pointer. A resource pointer can
		///only occur once in a table. If an attempt is made to add the same resource twice, then a 
		///reference count for that resource is incremented, and the existing index of that resource
		///in the mapping table is returned.
		///A return value of -1 from this function indicates failure.
		///@param pResource The rsource to add.
		///@return The index of the resource in this meshes mapping table.
		S16 AddPolygonBlindDataMapping(const C_BlindDataResource* pResource, bool forceAddition = false);

		///\brief Attempts to remove a blind data mapping.
		///
		///A return value of true indicates that a resource was deleted.
		///A return value of false indicates that the resources reference count was decremented.
		///@return deletion status.
		virtual void DeletePolygonBlindDataMapping(U16 index);

		///\brief Attempts to remove a material mapping.
		///
		///A return value of true indicates that a resource was deleted.
		///A return value of false indicates that the resources reference count was decremented.
		///@return deletion status.
		virtual void DeletePolygonMaterialMapping(U16 index);

		///\brief Returns true if this mesh is skinned.
		///
		///This performs a lookup into the influence clusters for this mesh. If any of them have
		///their usage set to AFTK_INFLUENCE_CLUSTER_USAGE_SKIN, then this is a skinned mesh.
		bool IsSkinned() const;

		///\brief Returns true if this meshes skin is animated.
		///
		///This performs a lookup into the influence clusters for this mesh. If any of them are
		///driven by a clip mixer output then true is returned. False is returned if the mesh
		///is either not animated or is not skinned.
		bool IsSkinAnimated() const;

		///\brief Returns a vertex position at the time the mesh was last evaluated.
		///
		///@param polyVertIndex The polygon vertex for which to retrieve the position.
		///@return The vertex position.
		Position3 GetPolygonVertexPosition(U32 polyVertIndex) const;

		///\brief Returns a vertex normal at the time the mesh was last evaluated.
		///
		///@param polyVertIndex The polygon vertex for which to retrieve the normal.
		///@return The vertex normal.
		Normal3 GetPolygonVertexNormal(U32 polyVertIndex) const;

		///\brief Returns a vertex tangent at the time the mesh was last evaluated.
		///
		///@param uvSet The texture coordinate set that the tangent is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the vertex tangent.
		///@return The vertex tangent.
		Vector3F32 GetPolygonVertexTangent(U32 uvSet, U32 polyVertIndex) const;

		///\brief Returns a vertex binormal at the time the mesh was last evaluated.
		///
		///@param uvSet The texture coordinate set that the binormal is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the vertex binormal.
		///@return The vertex binormal.
		Vector3F32 GetPolygonVertexBinormal(U32 uvSet, U32 polyVertIndex) const;

		virtual C_Bounds GetEvaluatedLocalBounds() const;

		///\brief Returns the bounding box of this node in world space.
		///
		///@return The bounds information.
		virtual C_Bounds GetEvaluatedWorldBounds(bool evaluateTransform = true) const;

		///\brief Returns a vertex position at the time the mesh was last evaluated.
		///
		///@param polyVertIndex The polygon vertex for which to retrieve the position.
		///@return The vertex position.
		Position3 GetEvaluatedPolygonVertexPosition(U32 polyVertIndex) const;

		///\brief Returns a texture coordinate from the given polygon vertex and set.
		///
		///@param uvSet The texture coordinate set that the uv information is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the uv data.
		///@return The texture coordinate.
		TextureCoordinate4 GetPolygonVertexTextureCoordinate(U32 uvSet, U32 polyVertIndex) const;

		///\brief Returns a vertex normal at the time the mesh was last evaluated.
		///
		///@param polyVertIndex The polygon vertex for which to retrieve the normal.
		///@return The vertex normal.
		Normal3 GetEvaluatedPolygonVertexNormal(U32 polyVertIndex) const;

		///\brief Returns a vertex tangent at the time the mesh was last evaluated.
		///
		///@param uvSet The texture coordinate set that the tangent is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the vertex tangent.
		///@return The vertex tangent.
		Vector3F32 GetEvaluatedPolygonVertexTangent(U32 uvSet, U32 polyVertIndex) const;

		///\brief Returns a vertex binormal at the time the mesh was last evaluated.
		///
		///@param uvSet The texture coordinate set that the binormal is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the vertex binormal.
		///@return The vertex binormal.
		Vector3F32 GetEvaluatedPolygonVertexBinormal(U32 uvSet, U32 polyVertIndex) const;

		///\brief Removes all of the geometry data from this class.
		///
		void ClearGeometry();

		void DeleteVertexDataChannels();

		void DeleteTextureCoordinateSets();

		void DeleteMaterials();

		void DeleteBlindData();

		bool IsEqualToThis(const I_Object& rhs, U32 flags = E_Comparison_LocalTransform) const;

		virtual void DirtyGeometryDependantCaches() const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToAll(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		U32 RepairPolyClusters();

		virtual bool IsRemappedSource() const;

		virtual void CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const;

		C_Status BuildVertexWeightList() const;

		C_Status BuildInfluenceNodeList() const;

		S32 GetInfluenceNodeIndexFromSceneNode(C_SceneNode* pSceneNode) const;

		C_LodLevel& GetLodLevel(U32 lodLevel);

		const C_LodLevel& GetLodLevel(U32 lodLevel) const;

		///\brief Evaluates a polygon vertex position and returns the result in object space.
		///
		///The evaluation is done using the current state of the matrix palette. It is therefore only
		///valid if 'BuildAnimationHelpers' has been called.
		///@param instance The polygon vertex index to evaluate.
		///@param position The evaluated position.
		void EvaluatePolygonVertexPosition(
			U32 instance,
			Position3& position) const;

		///\brief Evaluates a polygon vertex position and transforms it into the given space.
		///
		///The evaluation is done using the current state of the matrix palette. It is therefore only
		///valid if 'BuildAnimationHelpers' has been called.
		///@param instance The polygon vertex index to evaluate.
		///@param worldMatrix The transform to apply after evaluation.
		///@param position The evaluated position.
		void EvaluatePolygonVertexPosition(
			U32 instance,
			const Matrix44& worldMatrix,
			Position3& position) const;

		///\brief Evaluates a polygon vertex position and normal. Returns the results in object space.
		///
		///The evaluation is done using the current state of the matrix palette. It is therefore only
		///valid if 'BuildAnimationHelpers' has been called.
		///@param instance The polygon vertex index to evaluate.
		///@param position The evaluated position.
		///@param normal The evaluated normal.
		void EvaluatePolygonVertexPositionAndNormal(
			U32 instance,
			Position3& position,
			Normal3& normal) const;

		///\brief Evaluates a polygon vertex position and normal. Returns the results in the given space.
		///
		///The evaluation is done using the current state of the matrix palette. It is therefore only
		///valid if 'BuildAnimationHelpers' has been called.
		///@param instance The polygon vertex index to evaluate.
		///@param worldMatrix The transform to apply after evaluation.
		///@param position The evaluated position.
		///@param normal The evaluated normal.
		void EvaluatePolygonVertexPositionAndNormal(
			U32 instance,
			const Matrix44& worldMatrix,
			const Matrix44& worldMatrixTransposedInverse,
			Position3& position,
			Normal3& normal) const;

		///\brief Evaluates a polygon vertex position, tangent and binormal. Returns the result in object space.
		///
		///The evaluation is done using the current state of the matrix palette. It is therefore only
		///valid if 'BuildAnimationHelpers' has been called.
		///@param instance The polygon vertex index to evaluate.
		///@param set The texture coordinate set that the tangent space is associated with.
		///@param position The evaluated position.
		///@param tangent The evaluated tangent.
		///@param binormal The evaluated binormal.
		void EvaluatePolygonVertexPositionTangentAndBinormal(
			U32 instance,
			U32 set,
			Position3& position,
			Vector3F32& tangent,
			Vector3F32& binormal) const;

		///\brief Evaluates a polygon vertex position, tangent and binormal. Returns the result in the given space.
		///
		///The evaluation is done using the current state of the matrix palette. It is therefore only
		///valid if 'BuildAnimationHelpers' has been called.
		///@param instance The polygon vertex index to evaluate.
		///@param worldMatrix The transform to apply after evaluation.
		///@param set The texture coordinate set that the tangent space is associated with.
		///@param position The evaluated position.
		///@param tangent The evaluated tangent.
		///@param binormal The evaluated binormal.
		void EvaluatePolygonVertexPositionTangentAndBinormal(
			U32 instance,
			const Matrix44& worldMatrix,
			const Matrix44& worldMatrixTransposedInverse,
			U32 set,
			Position3& position,
			Vector3F32& tangent,
			Vector3F32& binormal) const;

		///\brief Evaluates a polygon vertex tangent and binormal. Returns the result in object space.
		///
		///The evaluation is done using the current state of the matrix palette. It is therefore only
		///valid if 'BuildAnimationHelpers' has been called.
		///@param instance The polygon vertex index to evaluate.
		///@param set The texture coordinate set that the tangent space is associated with.
		///@param tangent The evaluated tangent.
		///@param binormal The evaluated binormal.
		void EvaluatePolygonVertexTangentAndBinormal(
			U32 instance,
			U32 set,
			Vector3F32& tangent,
			Vector3F32& binormal) const;

		///\brief Evaluates a polygon vertex tangent and binormal. Returns the result in the given space.
		///
		///The evaluation is done using the current state of the matrix palette. It is therefore only
		///valid if 'BuildAnimationHelpers' has been called.
		///@param instance The polygon vertex index to evaluate.
		///@param worldMatrix The transform to apply after evaluation.
		///@param set The texture coordinate set that the tangent space is associated with.
		///@param tangent The evaluated tangent.
		///@param binormal The evaluated binormal.
		void EvaluatePolygonVertexTangentAndBinormal(
			U32 instance,
			const Matrix44& worldMatrixTransposedInverse,
			U32 set,
			Vector3F32& tangent,
			Vector3F32& binormal) const;

		void PadPolygonVertexTextureCoordinateSetIndices(C_PolygonVertex* pPolyVert);

		void PadPolygonVertexDataChannels(C_PolygonVertex* pPolyVert);

		C_Status BuildVertexChannelMergeTable(
			Ut::Vector<S32>& vertexChannelMergeSources,
			const C_SceneMeshNode* pMesh,
			bool mergeVertexChannelsByName);

		C_Status AddMeshVertexData(
			const C_SceneMeshNode* pMesh,
			bool mergeWeightsByEnvIndex,
			const Ut::Vector<S32>& vertexChannelMergeSources);

		C_Status AddMeshPolygonData(
			const C_SceneMeshNode* pMesh,
			U32 targetLodLevel);

		void ResizeUVSetsIfNeeded(U32 set);

		///\brief Adds a new polygon.
		///
		///The index returned from this function is signed. A value of -1 indicates that the operation
		///failed.
		///
		///@param lodLevel The polygon set in to which the new polygon will be added.
		///@param aftkPolygon The polygon to be added.
		///@param cluster The polygon cluster into which the polygon will be added.
		///@return The index of the new polygon in the mesh's polygon array.
		S32	AddPolygonNoCluster(U32 lodLevel, const C_Polygon& aftkPolygon = C_Polygon());

		U32 ProcessProblems(bool fix);

		//The following two arrays are animation specific helpers that are created as needed.
		mutable MatrixPaletteArray	m_matrixPalette;
		mutable VertexWeightArray	m_vertexWeightsList;
		mutable SceneNodePtrArray	m_influenceNodeList;

		//Cache last evaluation of postions, normals, tangents and binormals.
		mutable Position3*		m_pPolygonVertexPositionCache;
		mutable Normal3*		m_pPolygonVertexNormalCache;
		//Cache tangents and binormals per UV set.
		mutable Vector3F32**	m_ppPolygonVertexTangentCache;
		mutable Vector3F32**	m_ppPolygonVertexBinormalCache;

		mutable E_AnimHelperState m_animationHelpersBuilt;

		Ut::UtVector< Ut::DynamicVector<Position3> >		m_vertexPositions;
		Ut::UtVector< Ut::DynamicVector<Normal3> >			m_vertexNormals;
		
		Ut::UtVector< Ut::DynamicVector<C_LodLevel> >		m_lodLevels;

		Ut::UtVector< Ut::DynamicVector<C_TextureCoordinateSet> >	m_textureCoordinateSets;

		Ut::UtVector< 
			Ut::DynamicVector<C_VertexDataChannelInfo> >		m_vertexDataChannelInfo;

		Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >		m_polygonVertices;

		Ut::UtVector< Ut::DynamicVector<C_PolygonCluster*> >	m_polygonClusters;


		//List of adjacency information, into which the polygons have an index
		mutable Ut::UtVector< Ut::DynamicVector<U32> >			m_adjacencyList;

		friend class C_SceneMeshBonedInstanceNode;
		friend class C_SceneMeshRemappedInstanceNode;
		friend class C_BuildAdjacency;
		friend class C_NativeSceneMeshNode;
		friend class C_NativePolygon;
		friend class C_NativeFtk;
	
	#endif
	};
}
#endif
