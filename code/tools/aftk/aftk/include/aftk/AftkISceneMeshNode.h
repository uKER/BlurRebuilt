#ifndef AFTK_I_SCENE_MESH_NODE_H
#define AFTK_I_SCENE_MESH_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
	class C_Polygon;
	class C_MaterialNode;
	class C_InfluenceClusterNode;
	class C_VertexWeight;
	struct C_MatrixPaletteEntry;

	class I_SceneMeshNode : public C_SceneGeometryNode
	{
	public:

		///\brief Constructor.
		///
		///@param pScene The scene to which this mesh is attached.
		///
		I_SceneMeshNode(C_Scene* pScene) 
		: 
		C_SceneGeometryNode(pScene)
		{
		}

		///\brief Copy constructor.
		///
		///@param rhs The mesh node to copy.
		I_SceneMeshNode(const I_SceneMeshNode& rhs)
		: 
		C_SceneGeometryNode(0)
		{
			*this = rhs;
		}

		///\brief Destructor.
		///
		virtual	~I_SceneMeshNode()
		{
		}

		///\brief Asignment operator.
		///
		///@param rhs The mesh node to copy.
		///@return A reference to this mesh node.
		const I_SceneMeshNode& operator=(const I_SceneMeshNode& rhs)
		{
			C_SceneGeometryNode::operator =(rhs);

			return *this;
		}

		///\brief Equivilance operator.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool operator== (const I_SceneMeshNode &rhs) const
		{
			return C_SceneGeometryNode::operator==(rhs);
		}

		///\brief Returns the index of the first texture coordinate set with the given name.
		///
		///@param name The texture coordinate set name.
		///@return The texture coordinate set index. -1 if not found.
		virtual S32	GetTextureCoordinateSetIndex(const std::string& name) const = 0;

		///\brief Fetches a texture coordinate set name
		///
		///@param set The set from which to retrieve the the name.
		///@return The name of the given texture coordinate set.
		virtual const std::string& GetTextureCoordinateSetName(U32 set) const = 0;

		///\brief Fetches vertex channel information.
		///@param channel The channel from which to retrieve the the info.
		///@return The vertex channel information
		virtual const C_VertexDataChannelInfo& GetVertexDataChannelInfo(U32 channel) const = 0;
		
		///\brief Fetches a polygon.
		///
		///@param lodLevel The polygon set to query.
		///@param instance The polygon to retrieve.
		///@return The polygon.	
		virtual const C_Polygon& GetPolygon(U32 lodLevel, U32 instance) const = 0;

		///\brief Fetches a polygon.
		///
		///@param instance The polygon to retrieve.
		///@return The polygon.	
		virtual const C_Polygon& GetPolygon(U32 instance) const = 0;

		///\brief Returns a polygon vertex.
		///
		///@param instance The index of the polygon vertex to return.
		///@return The polygon vertex.
		virtual const C_PolygonVertex& GetPolygonVertex(U32 instance) const = 0;

		///\brief Fetches a texture coordinate.
		///@param set The texture coordinate set from which to retrieve the texture coordinate.
		///@param instance The texture coordinate to fetch.
		///@return The texture coordinate.
		virtual const TextureCoordinate4& GetTextureCoordinate(U32 set, U32 instance) const = 0;

		///\brief Fetches a vertex tangent.
		///
		///@param set The texture coordinate set from which to retrieve the vertex tangent.
		///@param instance The vertex tangent to fetch.
		///@return The vertex tangent.
		virtual const Vector3F32& GetVertexTangent(U32 set, U32 instance) const = 0;

		///\brief Fetches a vertex binormal.
		///
		///@param set The texture coordinate set from which to retrieve the vertex binormal.
		///@param instance The vertex binormal to fetch.
		///@return The vertex binormal.
		virtual const Vector3F32& GetVertexBinormal(U32 set, U32 instance) const = 0;

		///\brief Returns a polygon cluster.
		///
		///C_Polygon clusters are groups of polygons. All polygons are always a member of one polygon 
		///cluster.
		///@param instance The index of the polygon cluster.
		virtual const C_PolygonCluster* GetPolygonClusterPtr(U32 instance) const = 0;	
		
		///\brief Get the number of vertex positions in this mesh.
		///
		///@return Number of vertex positions.
		virtual U32	GetNumVertexPositions() const = 0;
		
		///\brief Get the number of vertex normals in this mesh.
		///
		///@return Number of vertex normals.
		virtual U32	GetNumVertexNormals() const = 0;

		///\brief Get the number of vertex tangents in this mesh.
		///@param set Texture coordinate set.
		///
		///@return Number of vertex tangents.
		virtual U32	GetNumVertexTangents(U32 set) const = 0;

		///\brief Get the number of vertex tangents in this mesh.
		///@param set Texture coordinate set.
		///
		///@return Number of vertex tangents.
		virtual U32	GetNumVertexBinormals(U32 set) const = 0;
		
		///\brief Get the number of vertex data entries in this mesh.
		///
		///@return Ammount of vertex data.
		virtual U32	GetNumVertexData() const = 0;
		
		///\brief Get the number of texture coordinates for a given mesh UV set.
		///
		///@param set UV set from which to get the number of texture coordinates.
		///@return Number of texture coordinates.
		virtual U32	GetNumTextureCoordinates(U32 set = 0) const = 0;
		
		///\brief Get the number of texture coordinates sets in this mesh.
		///
		///@return Number of texture coordinates sets.
		virtual U32	GetNumTextureCoordinateSets() const = 0;

		///\brief Get the number of vertex data channels in this mesh.
		///@return Number of vertex data channels.
		virtual U32	GetNumVertexDataChannels() const = 0;

		///\brief Fetches lod level information.
		///
		///@param lodLevel The lod level from which to retrieve the the info.
		///@return The lod level information.
		virtual const C_MeshLodLevelInfo& GetLodLevelInfo(U32 lodLevel) const = 0;

		///\brief Fetches lod level information.
		///
		///@param lodLevel The lod level from which to retrieve the the info.
		///@return The lod level information.
		virtual C_MeshLodLevelInfo& GetLodLevelInfo(U32 lodLevel) = 0;
		
		///\brief Get the number of polygon vertices in this mesh.
		///
		///@return Number of polygon vertices.
		virtual U32	GetNumPolygonVertices() const = 0;

		///\brief Returns the number of LOD levels in this mesh.
		///
		///@return The number of LOD levels.
		virtual U32 GetNumLodLevels() const = 0;
		
		///\brief Get the number of polygons in this mesh.
		///
		///@param lodLevel The polygon set to query.
		///@return Number of polygons.
		virtual U32	GetNumPolygons(U32 lodLevel) const = 0;

		///\brief Get the number of triangles in this mesh if it were to be triangulated.
		///
		///@param lodLevel The polygon set to query.
		///@return Number of triangles.
		virtual U32	GetNumTriangles(U32 lodLevel) const = 0;
		
		///\brief Returns the number of UNIQUE material nodes used by this mesh.
		///
		///@param lodLevel The polygon set to query.
		///@return Number of unique material nodes.
		virtual U32	GetNumMaterialNodes(U32 lodLevel) const = 0;
		
		///\brief Returns the number of polygon clusters used by this mesh.
		///
		///@return Number of polygon clusters.
		virtual U32 GetNumPolygonClusters() const = 0;

		virtual E_ObjType GetType() const = 0;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const 
		{
			if (C_SceneGeometryNode::HasType(type))
				return true;
			if (type == E_ObjType_ISceneMesh) 
				return true;

			return false;
		}

		virtual U32 Repair() = 0;
		///\brief Checks to see whether the contents of this mesh are valid.
		///
		///If a debugger has been attached to the AFTK then a report of any encountered errors will be given.
		///
		///@return The number of errors found.
		virtual U32	Validate() const = 0;

		///\brief Clears the contents of this mesh to their initial state.
		///
		virtual void Clear() 
		{
			C_SceneGeometryNode::Clear();
		}

		///\brief Builds a list of influences (bones), weights and creates a matrix palette for this mesh.
		///
		///@return C_Status class, indicating the result of the operation.
		virtual C_Status BuildAnimationHelpers() const = 0;

		virtual void ClearAnimationHelpers() const = 0;

		///\brief Returns a vertex weight.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@param instance The index of the vertex weight.
		///@return The vertex weight.
		virtual const C_VertexWeight& GetVertexWeight(U32 instance) const = 0;

		///\brief Returns the number of vertex weights for this mesh.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@return The number of vertex weights.
		virtual U32 GetNumVertexWeights() const = 0;

		///\brief Evaluates the matrix palette for this mesh, with the animation for this mesh
		///being a function of 'x'.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@param x The variable that the animation of this mesh is a function of.
		///@return C_Status class, indicating the result of the operation.
		virtual C_Status Evaluate() const = 0;
		
		///\brief Build adjacency information for this mesh.
		///
		///This is an O(n^2) algorithm.
		///@return True if adjacency information was successfully built.
		virtual bool IsAdjacencyBuilt() const = 0;

		///\brief Returns a matrix palette entry.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@param instance The matrix palette index.
		///@return The matrix.
		virtual const C_MatrixPaletteEntry& GetPaletteEntry(U32 instance) = 0;

		///\brief Returns the number of matrix palette entries for this mesh.
		///
		///This function is only meaningfull after first calling 'BuildAnimationHelpers'.
		///@return The number of matrix palette entries.
		virtual U32 GetNumPaletteEntries() = 0;

		///\brief Get a list of adjacent polygons given a polygon index.
		///
		///@param iThisPoly C_Polygon index.
		///@return Array of adjacent polygon indices.
		virtual const U32* GetAdjacencyList(U32 iThisPoly) const = 0;

		///\brief Returns true if this mesh is skinned.
		///
		///This performs a lookup into the influence clusters for this mesh. If any of them have
		///their usage set to AFTK_INFLUENCE_CLUSTER_USAGE_SKIN, then this is a skinned mesh.
		virtual bool IsSkinned() const = 0;

		///\brief Returns true if this meshes skin is animated.
		///
		///This performs a lookup into the influence clusters for this mesh. If any of them are
		///driven by a clip mixer output then true is returned. False is returned if the mesh
		///is either not animated or is not skinned.
		virtual bool IsSkinAnimated() const = 0;

		///\brief Returns a vertex position at the time the mesh was last evaluated.
		///
		///@param polyVertIndex The polygon vertex for which to retrieve the position.
		///@return The vertex position.
		virtual Position3 GetPolygonVertexPosition(U32 polyVertIndex) const = 0;

		///\brief Returns a vertex normal at the time the mesh was last evaluated.
		///
		///@param polyVertIndex The polygon vertex for which to retrieve the normal.
		///@return The vertex normal.
		virtual Normal3 GetPolygonVertexNormal(U32 polyVertIndex) const = 0;

		///\brief Returns a vertex tangent at the time the mesh was last evaluated.
		///
		///@param uvSet The texture coordinate set that the tangent is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the vertex tangent.
		///@return The vertex tangent.
		virtual Vector3F32 GetPolygonVertexTangent(U32 uvSet, U32 polyVertIndex) const = 0;

		///\brief Returns a vertex binormal at the time the mesh was last evaluated.
		///
		///@param uvSet The texture coordinate set that the binormal is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the vertex binormal.
		///@return The vertex binormal.
		virtual Vector3F32 GetPolygonVertexBinormal(U32 uvSet, U32 polyVertIndex) const = 0;

		///\brief Returns a vertex position at the time the mesh was last evaluated.
		///
		///@param polyVertIndex The polygon vertex for which to retrieve the position.
		///@return The vertex position.
		virtual Position3 GetEvaluatedPolygonVertexPosition(U32 polyVertIndex) const = 0;

		///\brief Returns a texture coordinate from the given polygon vertex and set.
		///
		///@param uvSet The texture coordinate set that the uv information is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the uv data.
		///@return The texture coordinate.
		virtual TextureCoordinate4 GetPolygonVertexTextureCoordinate(U32 uvSet, U32 polyVertIndex) const = 0;

		///\brief Returns a vertex normal at the time the mesh was last evaluated.
		///
		///@param polyVertIndex The polygon vertex for which to retrieve the normal.
		///@return The vertex normal.
		virtual Normal3 GetEvaluatedPolygonVertexNormal(U32 polyVertIndex) const = 0;

		///\brief Returns a vertex tangent at the time the mesh was last evaluated.
		///
		///@param uvSet The texture coordinate set that the tangent is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the vertex tangent.
		///@return The vertex tangent.
		virtual Vector3F32 GetEvaluatedPolygonVertexTangent(U32 uvSet, U32 polyVertIndex) const = 0;

		///\brief Returns a vertex binormal at the time the mesh was last evaluated.
		///
		///@param uvSet The texture coordinate set that the binormal is associated with.
		///@param polyVertIndex The polygon vertex for which to retrieve the vertex binormal.
		///@return The vertex binormal.
		virtual Vector3F32 GetEvaluatedPolygonVertexBinormal(U32 uvSet, U32 polyVertIndex) const = 0;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const
		{
			return IsEqualToAll(rhs, classToAbortTest, flags);
		}

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object &rhs, U32 flags = E_Comparison_LocalTransform) const
		{
			return true;
		}

		void SetEvaluateNormalInverseTranspose(bool flag)
		{
			m_normalInverseTranspose = flag;
		}

		bool GetEvaluateNormalInverseTranspose() const
		{
			return m_normalInverseTranspose;
		}

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const
		{
			return C_SceneGeometryNode::IsEqualToAll(rhs, classToAbortTest, flags);
		}

		friend class C_Scene;
		friend class C_InfluenceClusterNode;
	#endif
	};
}

#endif
