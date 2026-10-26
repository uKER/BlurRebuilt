#ifndef AFTK_POLYGON_H
#define AFTK_POLYGON_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_SceneMeshNode;
	class C_PolygonCluster;

	class C_Polygon
	{
	public:
		///\brief Constructor.
		///
		C_Polygon();

		///\brief Destructor.
		///
		~C_Polygon();

		///\brief Copy constructor.
		///
		///@param rhs C_Polygon to copy.
		C_Polygon(const C_Polygon& rhs);

		///\brief Asignment operator.
		///
		///@param rhs C_Polygon to copy.
		C_Polygon& operator=(const C_Polygon& rhs);

		///\brief Sets the initial capacity of this polygon's vertex array.
		///
		///@param capacity The desired vertex capacity.
		void SetPolygonVertexInitialCapacity(U32 capacity);

		///\brief Attaches a pointer to some user data.
		///
		///Note that this memory is deallocated by the AFTK, not the user.
		///@param attachData data to attach
		void SetAttachData(const C_AttachData& attachData);

		///\brief Adds an index into the meshes array of polygon vertices.
		///
		///@param index The new polygon vertex index.
		///@return the index of the polygon vertex index in this polygon's array of polygon vertex indices..
		U32 AddPolygonVertexIndex(U32 index);

		///\brief Set the value of a given polygon vertex instance.
		///
		///@param instance The index of the polygon vertex to set.
		///@param polyVert The new polygon vertex instance.
		void SetPolygonVertex(U32 instance, const C_PolygonVertex& polyVert);

		///\brief Overwrites an existing polygon vertex index.
		///
		///@param instance The index of the polygon vertex index to overwrite.
		///@param index The new index value.
		void SetPolygonVertexIndex(U32 instance, U32 index);

		///\brief Sets the mesh-relative material index for this polygon.
		///
		///@param index the material mapping index.
		void SetMaterialMappingIndex(S16 index);

		///\brief Sets the mesh-relative blind data index for this polygon.
		///
		///@param index the blind data mapping index.
		void SetBlindDataMappingIndex(S16 index);

		void SetTrianglePolygonVertexIndices(U32 instance, const U32* pTriangle);

		///\brief Removes the blind data associated with this polygon.
		///
		void DeleteBlindData();

		///\brief Removes the material associated with this polygon
		///
		void DeleteMaterial();

		///\brief Get the attached user data.
		///
		///@return user data
		const C_AttachData*	GetAttachDataPtr() const;

		///\brief Get the attached user data.
		///
		///@return user data
		C_AttachData*	GetAttachDataPtr();

		///\brief Returns the number of polygon vertices.
		///
		///@return The number of polygon vertices.
		U32 GetNumPolygonVertices() const;

		///\brief Returns the number of triangles in this polygon if it were to be triangulated.
		///
		///@return The number of triangles.
		U32 GetNumTriangles() const;

		///\brief Returns the polygon vertices comprising the nth triangle of this polygon.
		///
		///@param instance The triangle to retrieve.
		///@param ppTriangle The triangle's polygon vertices.
		C_Status GetTrianglePolygonVertices(U32 instance, const C_PolygonVertex** ppTriangle) const;
		
		///\brief Returns the polygon vertex indices comprising the nth triangle of this polygon.
		///
		///@param instance The triangle to retrieve.
		///@param pTriangle The triangle's polygon vertex indices.
		C_Status GetTrianglePolygonVertexIndices(U32 instance, U32* pTriangle) const;

		///\brief Returns a particular instance of a polygon vertex.
		///
		///@param instance C_Polygon vertex instance to fetch.
		///@return C_Polygon vertex index.
		const C_PolygonVertex& GetPolygonVertex(U32 instance) const;

		///\brief Returns a particular instance of a polygon vertex.
		///
		///@param instance C_Polygon vertex instance to fetch.
		///@return C_Polygon vertex index.
		C_PolygonVertex& GetPolygonVertex(U32 instance);

		///\brief Returns a polygon vertex index.
		///
		///@param instance The index of the polygon vertex index.
		///@return The polygon vertex index.
		U32 GetPolygonVertexIndex(U32 instance) const;

		///\brief Returns the adjacent polygon of an edge
		///
		///@param instance The first vertex of the edge that is shared with the adjacent polygon
		///@return C_Polygon index (-1 means there is no adjacent polygon)
		S32 GetAdjacentPolygonIndex(U32 instance) const;

		///\brief Returns the mesh relative materal mapping index for this polygon.
		///
		///@return The mesh relative mapping index.
		S16 GetMaterialMappingIndex() const;

		///\brief Returns the mesh relative blind data mapping index for this polygon.
		///
		///@return The mesh relative mapping index.
		S16 GetBlindDataMappingIndex() const;

		///\brief Removes a polygon vertex index.
		///
		///@param instance The polygon vertex index to remove
		void DeletePolygonVertexIndex(U32 instance);

		///\brief Removes the last polygon vertex in the array.
		///
		void DeletePolygonVertexIndex();

		///\brief Deletes the user attached data at the given index in the array of attached user data for this polygon. 
		///
		void DeleteAttachData();

		///\brief Returns the index of the polygon that this polygon was originally associated with.
		///
		///These base polygon indices are created during the triangulation process. The first triangle of
		///a polygon is termed the 'base polygon'. The subsequent triangles for that polygon then have indices
		///to that polygon (the first triangle). In this way it is possible to rebuild a polygon mesh after it has
		///been triangulated. An index of -1 indicates that this is a base polygon.
		///@return The base polygon index.
		S32 GetBasePolygonIndex() const;

		///\brief Returns the polygon cluster that this polygon is associated with.
		///
		///All polygons are associated with one polygon cluster at all times.
		///
		///@return The polygon cluster.
		C_PolygonCluster* GetClusterPtr();

		///\brief Returns the polygon cluster that this polygon is associated with.
		///
		///All polygons are associated with one polygon cluster at all times.
		///
		///@return The polygon cluster.
		const C_PolygonCluster* GetClusterPtr() const;

		void ReverseWindingOrder();

		///\brief Sets the base polygon index of this polygon.
		///
		///These base polygon indices are created during the triangulation process. The first triangle of
		///a polygon is termed the 'base polygon'. The subsequent triangles for that polygon then have indices
		///to that polygon (the first triangle). In this way it is possible to rebuild a polygon mesh after it has
		///been triangulated. An index of -1 indicates that this is a base polygon.
		void SetBasePolygonIndex(S32 index);

		///\brief Returns the mesh that this polygon belongs to.
		///
		///@return The mesh node.
		C_SceneMeshNode* GetMeshNodePtr();

		///\brief Returns the mesh that this polygon belongs to.
		///
		///@return The mesh node.
		const C_SceneMeshNode* GetMeshNodePtr() const;

		///\brief Returns the normal of this polygon.
		///
		///This is recalculated for each call of the function.
		///
		///@return The normal.
		Normal3 GetNormal(const Matrix44* pMatrix = 0, bool* pIsDegenerate = 0) const;

		///\brief Returns the area of this polygon.
		///
		///This is recalculated for each call of the function.
		///
		///@return The area.
		F32 GetArea(const Matrix44* pMatrix = 0) const;

		///\brief Equality operator
		///
		///Is this polygon the same as the other?
		///
		///@return true or false
		
		bool operator==(const C_Polygon& rhs) const;

		U32 Repair();

		///\brief Validate that the data contained by this polygon is correct.
		///
		///@return The number of errors found.
		U32 Validate() const;

		///\brief Clears the contents of this node to their initial state.
		///
		void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void OneTimeInit();

		void SetClusterPtr(C_PolygonCluster* pPolyCluster);

		void SetAdjacencyIndex(S32 index) const;

		//Links this polygon to a mesh and increases the reference counts of it's shared resources.
		void AttachToMesh(C_SceneMeshNode* pMeshNode);

		void ShuffleBlindDataMappingIndex();

		void ShuffleMaterialMappingIndex();

		bool TriangleIsAnEar(
			const Matrix44* pMatrix,
			const Vector3F32& n0,
			S32 pvi1,
			S32 pvi2, 
			S32 pvi3);

		void AddNewTriangle(
			Ut::UtVector<Ut::DynamicVector<C_Polygon> >& polygonArray,
			S32& firstEmittedTriangleIndex,
			S32 pvi1, S32 pvi2, S32 pvi3);

		U32 Triangulate(
			Ut::UtVector<Ut::DynamicVector<C_Polygon> >& polygonArray,
			Ut::UtVector<Ut::DynamicVector<bool> >& activeVertexArray,
			Ut::UtVector<Ut::DynamicVector<bool> >& vertexClassification,
			const Matrix44* pMatrix = 0);

		U32 ProcessProblems(bool fix);

		C_AttachData*	m_pAttachedData;

	#ifdef AFTK_USE_MEMORY_POOL
		Ut::UtVector< 
			Ut::DynamicVector<U32, C_ArrayAllocator<U32> > >
										m_polygonVertexIndices;
	#else
		Ut::UtVector< 
			Ut::DynamicVector<U32> > 
										m_polygonVertexIndices;
	#endif
		mutable S32						m_iAdjacencyIndex;
		S16								m_materialMappingIndex;
		S16								m_blindDataMappingIndex;

		C_SceneMeshNode*				m_pMeshNode;
		S32								m_basePolygonIndex;

		C_PolygonCluster*				m_pPolygonCluster;

		friend class C_SceneMeshNode;
		friend class C_BuildAdjacency;
		friend class C_PolygonCluster;
		friend class C_TriangleIterator;
		friend class C_NativeSceneMeshNode;
		friend class C_NativePolygon;
		friend class C_MeshTriangulate;
	#endif
	};

	typedef Ut::UtVector< Ut::DynamicVector<C_Polygon> >	PolygonArray;
	typedef Ut::UtVector< Ut::DynamicVector<C_Polygon*> >	PolygonPtrArray;
}

#endif
