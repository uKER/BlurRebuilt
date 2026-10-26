#ifndef AFTK_SCENE_MESH_REMAPPED_INSTANCE_NODE_H
#define AFTK_SCENE_MESH_REMAPPED_INSTANCE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
	class C_Polygon;
	class C_MaterialNode;

	class C_SceneMeshRemappedInstanceNode : public C_SceneMeshRemappableNode
	{
	public:

		///\brief Constructor.
		///
		///@param pScene The scene to which this mesh is attached.
		///
		C_SceneMeshRemappedInstanceNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		virtual C_Node* Clone(C_Scene* pScene, const MaterialRemappingHashTable& mappingTable) const;

		const C_SceneMeshRemappedInstanceNode& Copy(const C_SceneMeshRemappedInstanceNode& rhs, const MaterialRemappingHashTable* pMappingTable);

		///\brief Destructor.
		///
		virtual ~C_SceneMeshRemappedInstanceNode();

		///\brief Returns the index of the first texture coordinate set with the given name.
		///
		///@param name The texture coordinate set name.
		///@return The texture coordinate set index. -1 if not found.
		S32	GetTextureCoordinateSetIndex(const std::string& name) const;

		///\brief Fetches a texture coordinate set name
		///
		///@param set The set from which to retrieve the the name.
		///@return The name of the given texture coordinate set.
		const std::string& GetTextureCoordinateSetName(U32 set) const;

		///\brief Fetches vertex channel information.
		///
		///@param channel The channel from which to retrieve the the info.
		///@return The vertex channel information
		const C_VertexDataChannelInfo& GetVertexDataChannelInfo(U32 channel) const;
		
		///\brief Fetches a polygon.
		///
		///@param lodLevel The polygon set to query.
		///@param instance The polygon to retrieve.
		///@return The polygon.	
		const C_Polygon& GetPolygon(U32 lodLevel, U32 instance) const;

		///\brief Fetches a polygon.
		///
		///@param instance The polygon to retrieve.
		///@return The polygon.	
		const C_Polygon& GetPolygon(U32 instance) const;

		///\brief Fetches a texture coordinate.
		///@param set The texture coordinate set from which to retrieve the texture coordinate.
		///@param instance The texture coordinate to fetch.
		///@return The texture coordinate.
		const TextureCoordinate4& GetTextureCoordinate(U32 set, U32 instance) const;

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

		///\brief Returns a polygon vertex.
		///
		///@param instance The index of the polygon vertex to return.
		///@return The polygon vertex.
		const C_PolygonVertex& GetPolygonVertex(U32 instance) const;

		///\brief Returns a polygon cluster.
		///
		///C_Polygon clusters are groups of polygons. All polygons are always a member of one polygon 
		///cluster.
		///@param instance The index of the polygon cluster.
		const C_PolygonCluster* GetPolygonClusterPtr(U32 instance) const;
		
		///\brief Get the number of vertex positions in this mesh.
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

		///\brief Get the number of triangles in this mesh if it were to be triangulated.
		///
		///@param lodLevel The polygon set to query.
		///@return Number of triangles.
		U32	GetNumTriangles(U32 lodLevel) const;
		
		///\brief Returns the number of UNIQUE material nodes used by this mesh.
		///
		///@param lodLevel The polygon set to query.
		///@return Number of unique material nodes.
		U32	GetNumMaterialNodes(U32 lodLevel) const;

		///\brief Returns the number of polygon clusters used by this mesh.
		///
		///@return Number of polygon clusters.
		U32 GetNumPolygonClusters() const;

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

		U32 Repair();

		///\brief Checks to see whether the contents of this mesh are valid.
		///
		///If a debugger has been attached to the AFTK then a report of any encountered errors will be given.
		///
		///@return The number of errors found.
		U32	Validate() const;

		///\brief Applies the given matrix to the contents of this node.
		///
		///@param matrix The matrix to apply.
		void ApplyMatrix(const Matrix44& matrix);

		bool operator== (const C_SceneMeshRemappedInstanceNode &rhs) const;

		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Clears the contents of this mesh to their initial state.
		///
		virtual void Clear();

		///\brief Copy constructor.
		///
		///@param rhs The mesh node to copy.
		C_SceneMeshRemappedInstanceNode(const C_SceneMeshRemappedInstanceNode& rhs);

		///\brief Builds a list of influences (bones), weights and creates a matrix palette for this mesh.
		///
		///@return C_Status class, indicating the result of the operation.
		C_Status BuildAnimationHelpers() const;

		///\brief Removes the animation helper cache.
		///
		void ClearAnimationHelpers() const;

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

		///brief Sets the node that we are instancing.
		///
		///Setting the source node automatically copies the blind data and material mapping tables from the
		///source, and also copies the texture coordinate offsets.
		///@param pSource The source node.
		void SetSourceNodePtr(C_SceneMeshNode* pSource, bool copyMappings = true);

		///\brief Asignment operator.
		///
		///@param rhs The mesh node to copy.
		///@return A reference to this mesh node.
		const C_SceneMeshRemappedInstanceNode& operator=(const C_SceneMeshRemappedInstanceNode& rhs);

		///\brief Evaluates the matrix palette for this mesh, with the animation for this mesh
		///being a function of 'x'.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@param x The variable that the animation of this mesh is a function of.
		///@return C_Status class, indicating the result of the operation.
		C_Status Evaluate() const {return C_Status::E_Status_Success;} 

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

		///\brief Returns the source node that we're instancing.
		///
		///@return The source node.
		C_SceneMeshNode* GetSourceNodePtr();

		///\brief Returns the source node that we're instancing.
		///
		///@return The source node.
		const C_SceneMeshNode* GetSourceNodePtr() const;

		///\brief Returns the reference count for the resource at a given blind data mapping table index.
		///
		///The reference count is taken from the source node.
		///@param index The blind data mapping table index.
		///@return The reference count.
		U32 GetPolygonBlindDataMappingReferenceCount(S16 index) const;

		///\brief Returns the reference count for the resource at a given material mapping table index.
		///
		///The reference count is taken from the source node.
		///@param index The material mapping table index.
		///@return The reference count.
		U32 GetPolygonMaterialMappingReferenceCount(S16 index) const;

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

		bool IsEqualToThis(const I_Object& rhs, U32 flags = E_Comparison_LocalTransform) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		virtual bool IsRemappedSource() const;

		///\brief Ensure that the mapping tables on each instance are equal in number to those on the mesh
		///being instanced. This is only executed when reading files from disk, as old files may potentially
		///be 'broken' in this way.
		void RepairMappingTables();

		virtual void CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const;

		void SetOnlySourceNodePtr(C_SceneMeshNode* pSource);

		U32 ProcessProblems(bool fix);

		virtual void DeleteInstanceSource(C_SceneNode* pSource);

		C_SceneMeshNode* m_pSourceNode;
		
		friend class C_NativeFtk;
		friend class C_NativeSceneMeshRemappedInstanceNode;
	    
	#endif
	};
}
#endif
