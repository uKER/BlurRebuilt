#pragma once 

namespace AftkUt
{
	namespace RadialEdgeMesh
	{
		//Edge class: Each edge has pointers to 2 vertices and N polygons


		class C_Edge 
		{
			friend class C_Mesh;
			friend class C_Vertex;
			friend class C_Polygon;
			//Radial edge structure needs N pointers per component type
			//need lists...

			ListItem<C_Vertex>*		m_pVertices[2];
			List<C_Polygon>			m_pPolygons; 
			ListItem<C_Edge>*		m_pMeshListItem; //back pointer to main list
		public:

			C_Edge()
			{
				m_pVertices[0] = 0;
				m_pVertices[1] = 0;
				m_pMeshListItem = 0;
			}

			ListEdge* AddToPoly(ListPoly* plItemNew, ListPoly* plItemOld);

			inline C_Vertex* GetOtherVertex(const C_Vertex* vt) const
			{
				assert(vt);
				if (vt == &m_pVertices[0]->m_data)
				{
					return &m_pVertices[1]->m_data;
				}
				else
				{
					return &m_pVertices[0]->m_data;
				}
			}

			inline bool HasVertex(const C_Vertex* vt) const
			{
				if (vt)
				{
					if ((vt == &m_pVertices[0]->m_data) || 
						(vt == &m_pVertices[1]->m_data))
					{
						return true;
					}
				}
				return false;
			}

			inline ListVert* GetVertex(const U32 vtIndex) const
			{
				assert(vtIndex < 2);
				return m_pVertices[vtIndex];
			}

			inline ListPoly* GetFirstPoly() const
			{
				return m_pPolygons.Begin();
			}

			void GetPolygons(Ut::UtVector<Ut::DynamicVector< ListPoly*> >& polys) const;

			U32 GetNumPolygons() const
			{
				return m_pPolygons.Size();
			}

			bool HasPoly(const C_Polygon* pl) const;

			inline bool IsAtPos(float x1, float y1, float z1, 
								float x2, float y2, float z2)
			{	
				if ((m_pVertices[0]->m_data.IsAtPoint(x1, y1, z1) &&
					 m_pVertices[1]->m_data.IsAtPoint(x2, y2, z2)) ||
					(m_pVertices[1]->m_data.IsAtPoint(x1, y1, z1) &&
					 m_pVertices[0]->m_data.IsAtPoint(x2, y2, z2)))
				{
					return true;
				}
				return false;
			}

			bool IntersectsPlane(
				const Aftk::Position3& planePos, 
				const Aftk::Normal3& planeNorm, 
				float* intersection = 0) const;

			float GetLength() const;

			bool PolysShareAttributes() const;
			bool HasBoundaryVertex() const;
			bool VerticesShareAttributes(const int deleteIndex) const;
			bool WillCollapseAlterPolyNormals(const int deleteIndex =1, const float angle = 60.0f) const;
			bool WillCollapseAlterBoundary(const int deleteIndex = 1) const;
			bool WillCollapseAffectUVMapping(const int deleteIndex = 1) const;
			bool WillCollapseAffectMaterials(const int deleteIndex = 1) const;
			bool WillCollapseGenerateEdgeLongerThan(const F32 maxLength, const int deleteIndex = 1) const;
			bool CanCollapseEdge(const int deleteIndex = 1, C_Edge** pBoundaryEdge = 0) const;
			bool CanFlipEdge() const;
			bool PolygonVerticesShareTangentSpace(const C_Polygon* pl) const;
			bool PolygonVerticesShareColour(const C_Polygon* pl) const;
			bool IsBoundaryEdge() const;
			bool IsMaterialBoundaryEdge() const;
			bool IsMappingBoundaryEdge() const;
			bool IsEdgeCoLinear(const C_Edge* edge) const;
			U32 m_index;

		};

	}

}
