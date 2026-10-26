#pragma once 

namespace AftkUt
{
	namespace RadialEdgeMesh
	{
		//Polygon class: Each polygon has a list of edges and polygon verts

		class C_Polygon 
		{
			friend class C_Mesh;
			friend class C_Edge;
			friend class C_Vertex;
			
			List<C_Edge>		m_pEdges;
			ListItem<C_Polygon>* m_pMeshListItem;
			S32 m_aftkIndex;

			Aftk::Vector3F32 m_centroid;
			//mutable Ut::UtVector<Ut::DynamicVector<ListVert*> > m_verts;

		public:
		
			C_Polygon() : m_pMeshListItem(0), m_aftkIndex(-1), m_basePolygonIndex(-1)
			{
			}
			
			U32 m_index;
			S16 m_aftkMaterialIndex; //this lets us get at all the Aftk::C_Polygon properties
			S16 m_aftkBlindDataIndex; //this lets us get at all the Aftk::C_Polygon properties
			S32	m_basePolygonIndex;

			inline ListEdge* GetFirstEdge() const
			{
				return m_pEdges.Begin();
			}

			void GetOrderedEdgeVerts(Ut::UtVector<Ut::DynamicVector<ListVert*> >& verts) const;
			void GetOrderedEdgeVerts(ListVert** pVerts, U32 bufferLength) const;
			
			inline ListEdge* HasEdge(const ListEdge* edge) const
			{
				if (edge)
				{
					for (ListEdge* edItem = m_pEdges.Begin(); edItem; edItem = edItem->GetNext())
					{
						if (&edge->m_data == &edItem->m_data)
						{
							return edItem;
						}
					}
				}
				return 0;
			}

			bool IsInternal() const
			{
				for (ListEdge* edItem = m_pEdges.Begin(); edItem; edItem = edItem->GetNext())
				{
					if (edItem->m_data.m_pPolygons.Size() < 2)
					{
						return false;
					}
				}
				return true;
			}

			bool IsVertexInPlane(const C_Vertex* vtPoly, const C_Vertex* vt) const;
			
			E_PlaneTest TestPlane(const U32 index) const;
			bool IsInFrontOfPlane(const C_Plane& plane) const;
			bool IsCentroidInFrontOfPlane(const C_Plane& plane) const;

			bool IntersectsPlane(const Aftk::Position3& planePos, const Aftk::Normal3& planeNorm) const;
			void GetPolyNormal(Maths::HVec3& normal, const C_Vertex* pFromVertex) const;

			inline S32 GetAftkIndex() const
			{
				return m_aftkIndex;
			}		
			
			void CalculateCentroid();
			
			const Aftk::Vector3F32& GetCentroid() const 
			{
				return m_centroid;
			}

		};

		struct C_PolySortItem
		{
			C_PolySortItem(ListItem<C_Polygon>* pPolyItem) : m_pPolyItem(pPolyItem){}
			bool operator < (const C_PolySortItem& rhs) const
			{
				return (m_pPolyItem->m_data.m_basePolygonIndex < rhs.m_pPolyItem->m_data.m_basePolygonIndex);
			}

			ListItem<C_Polygon>* m_pPolyItem;
		};

		bool PolygonIntersectsConvexHull(const Ut::Vector<Aftk::Position3>& positions, const Ut::Vector<C_Plane>& planes);



	}

}
