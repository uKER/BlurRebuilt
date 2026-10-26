#ifndef AFTK_POLYGON_CLUSTER_H
#define AFTK_POLYGON_CLUSTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_PolygonCluster : public C_ComponentCluster
	{

	public:
		///\brief Constructor.
		///
		C_PolygonCluster(C_SceneMeshNode* pMesh);

		///\brief Copy constructor.
		///
		///@param rhs The cluster to copy.
	//	C_PolygonCluster(const C_PolygonCluster& rhs);

		///\brief Destructor.
		///
		virtual ~C_PolygonCluster();

		///\brief Asignment operator.
		///
		///@param rhs The cluster to copy.
		///@return A reference to this cluster.
		const C_PolygonCluster& operator=(const C_PolygonCluster& rhs);

		///\brief Add a new polygon index to our cluster.
		///
		///A return value of -1 indicates a failure to add the index.
		///@param lodLevel The new polygon-set relative index value.
		///@param index The new polygon-set relative index value.
		///@return The index of the polygon index in the clusters array of indices. 
		S32 AddPolygonIndex(U32 lodLevel, U32 index);

		///\brief Sets a particular polygon index.
		///
		///@param instance The index of the polygon index to overwrite.
		///@param lodLevel The polygon set for the polygon.
		///@param index The new polygon-set relative index value.
	//	void SetPolygonIndex(U32 instance, U32 lodLevel, U32 index);

		///\brief Sets the polygon index array's initial capacity.
		///
		///@param capacity The array capacity.
		void SetPolygonIndexInitialCapacity(U32 lodLevel, U32 capacity);

		///\brief Returns a polygon index.
		///
		///@param instance The index of the polygon index to return.
		///@param lodLevel The polygons lod level.
		///@return The polygon index.
		S32 GetPolygonIndex(U32 lodLevel, U32 instance) const;

		///\brief Returns the number of polygon indices in this cluster.
		///
		///@return The number of polygon indices.
		U32 GetNumPolygonIndices(U32 lodLevel) const;

		///\brief Returns the number of polygon indices in this cluster.
		///
		///@return The number of polygon indices.
		U32 GetNumLodLevels() const;

		///\brief Returns the cluster type. In this case AFTK_COMPONENT_CLUSTER_TYPE_POLYGON.
		///
		///@return AFTK_COMPONENT_CLUSTER_TYPE_POLYGON.
		virtual E_ComponentClusterType GetType() const;

		void DeletePolygonIndex(U32 lodLevel, U32 index);

		///\brief Removes the polygons from this cluster.
		///
		virtual void Clear();

	protected:
		void OneTimeInit();
		
		void ClearThis();

		void ClearPolygonIndices();

		void ClearPolygonIndices(U32 lodLevel);

		void DeletePolygonIndex(U32 lodLevel, U32 index, bool forceDelete);

		U32 Repair();

		///\brief Add a new polygon index to our cluster.
		///
		///A return value of -1 indicates a failure to add the index.
		///@param lodLevel The new polygon-set relative index value.
		///@param index The new polygon-set relative index value.
		///@return The index of the polygon index in the clusters array of indices. 
		S32 AddPolygonIndexDoNotModifyPolygon(U32 lodLevel, U32 index);

		///For use only in cluster repair function.
		void DeletePolygonIndexDoNotModifyPolygon(U32 lodLevel, U32 index);

		void DeletePolygonIndicesDoNotModifyPolygons(U32 lodLevel);

		bool operator== (const C_PolygonCluster& rhs) const;

		void ShufflePolygonIndices(U32 lod, U32 polyIndex);

		Ut::UtVector< Ut::DynamicVector<
			Ut::UtVector<Ut::DynamicVector<U32> > > >	m_polygonIndices;

		friend class C_SceneMeshNode;
		friend class C_Polygon;
		friend class C_NativePolygonCluster;
		friend class C_MeshWeld;
		friend class C_MeshRemoveDegenerates;
		friend class C_MeshTriangulate;
	};
}

#endif
