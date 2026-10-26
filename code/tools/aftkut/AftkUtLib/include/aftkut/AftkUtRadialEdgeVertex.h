#pragma once 


namespace AftkUt
{
	namespace RadialEdgeMesh
	{
		struct TextureCoordinates
		{
			TextureCoordinates() : m_uvwx(0.f,0.f,0.f,0.f), m_tangents(0.f,0.f,0.f), m_binormals(0.f,0.f,0.f)
			{
				
			}
			
			inline bool operator== (const TextureCoordinates& rhs) const 
			{
				return (Aftk::EqualityTestFVector(m_uvwx, rhs.m_uvwx, g_Tolerances.UVEpsilon()) && 
						Aftk::EqualityTestFVector(m_binormals, rhs.m_binormals, g_Tolerances.UVEpsilon()) && 
						Aftk::EqualityTestFVector(m_tangents, rhs.m_tangents, g_Tolerances.UVEpsilon()));
			}
			
			inline bool HasSameTangentSpaceAs(const TextureCoordinates& rhs, float tolerance) const 
			{
				return (Aftk::EqualityTestFVector(m_binormals, rhs.m_binormals, tolerance) && 
						Aftk::EqualityTestFVector(m_tangents, rhs.m_tangents, tolerance));
			}
			
			
			inline TextureCoordinates& operator= (const TextureCoordinates& rhs)
			{
				m_uvwx = rhs.m_uvwx;
				m_binormals = rhs.m_binormals;
				m_tangents = rhs.m_tangents;
				return *this;
			}
			

			Aftk::TextureCoordinate4 m_uvwx;
			Aftk::Vector3F32 m_tangents;
			Aftk::Vector3F32 m_binormals;

		};

		//PolygonVertex class: Each polyvert has a pointer to a shared vertex and
		//any UV, color etc info.

		class C_PolygonVertex
		{
			friend class C_Mesh;
			friend class C_Vertex;
			friend class C_Edge;

			C_Polygon* m_pl;

			Ut::UtVector< Ut::DynamicVector< TextureCoordinates> >	
				m_textureCoordinates;

			Ut::UtVector< Ut::DynamicVector<U8> > m_dataByteArray;

			U32 m_colour;

			Aftk::Normal3 m_normal;
			
			S32 m_aftkIndex;


		public:

			C_PolygonVertex():m_colour(0){}
			~C_PolygonVertex(){}

			bool operator==(const C_PolygonVertex& other);
			
			bool HasSameTangentSpaceAs(const C_PolygonVertex& other, float tolerance) const;

			bool HasSameColourAs(const C_PolygonVertex& other) const;

			inline void Set(const C_PolygonVertex& other)
			{
				m_textureCoordinates = other.m_textureCoordinates;
				m_normal = other.m_normal;
				m_aftkIndex = other.m_aftkIndex;
				m_dataByteArray = other.m_dataByteArray;
				m_colour = other.m_colour;
			}

			void CopyDataFromAftkToLinked(
				const Aftk::C_PolygonVertex* apv, 
				C_Mesh* mesh);
			
			void CopyDataFromLinkedToAftk(
				Aftk::C_PolygonVertex* apv, 
				C_Mesh* mesh);

			inline void SetNormal(const Aftk::Normal3 norm)
			{
				m_normal = norm;
			}

			inline void SetVertexColour(const U32 colour)
			{
				m_colour = colour;
			}

			inline const Aftk::Normal3& GetNormal() const
			{
				return m_normal;
			}

			inline const U32 NumTextureCoordinatesSets() const
			{
				return (unsigned int)m_textureCoordinates.Size();
			}

			inline void SetTextureCoordinates(const U32 set, const TextureCoordinates uvs)
			{
				if ((unsigned int)m_textureCoordinates.Size() <= set)
				{
					m_textureCoordinates.Resize(set+1);
				}

				m_textureCoordinates[set] = uvs;
			}

			inline const TextureCoordinates& GetTextureCoordinates(const U32 set) const
			{
				assert(set < (unsigned int)m_textureCoordinates.Size());
				return m_textureCoordinates[set];
			}

			void Lerp(
				const C_PolygonVertex& pv1, 
				const C_PolygonVertex& pv2, 
				const float f);

			S32 GetAftkIndex() const
			{
				return m_aftkIndex;
			}

			inline C_Polygon* GetPolygon() const
			{
				return m_pl;
			}

			U32 m_index;

		};

		//Vertex class: shared vertex nodes, used to link structure together. The base template 
		//does not involve any information on positions, normals etc. It is purely a node in a graph.

		class C_Vertex
		{
			//friend classes needed to preserve access levels without
			//completely "over interfacing" the classes.
			//i.e. we want to be able to show the public part of the class to the outside world, 
			//but its innards only to the other LinkedMesh mesh classes

			friend class C_Mesh;
			friend class C_Edge;

			//Each vertex has a list of the edges connected to it. 

			List<C_Edge> m_pEdges;
			List<C_PolygonVertex>	m_pPolygonVerts;
			U32 m_planeTestBits;
			U32 m_planeIntersectBits;

		public:
			Aftk::Position3 m_pos;
			Aftk::Normal3 m_norm;

			C_Vertex() : m_planeTestBits(0), m_planeIntersectBits(0){}
			~C_Vertex()
			{
				//we actually store the polyvert data locally, so this must be deleted

				for (ListPolyVert* pvItem = m_pPolygonVerts.Begin(); pvItem; pvItem = pvItem->GetNext())
				{
					delete &pvItem->m_data;
				}
			}

			void ResetPlaneTests(){m_planeTestBits=0;m_planeIntersectBits=0;}
			void TestPlaneList(const Ut::Vector<C_Plane>* pPlanes);
			bool TestCachedPlane(U32 index);
			bool IntersectCachedPlane(U32 index);

			inline ListEdge* GetFirstEdge() const
			{
				return m_pEdges.Begin();
			}

			inline ListPolyVert* GetFirstPolygonVertex() const
			{
				return m_pPolygonVerts.Begin();
			}

			inline U32 NumPolygonVertices() const
			{
				U32 count = 0;
				ListPolyVert* pPolyVert = m_pPolygonVerts.Begin();
				while (pPolyVert)
				{
					count++;
					pPolyVert = pPolyVert->GetNext();
				}
				return count;
			}

			inline ListPolyVert* GetPolygonVertex(const C_Polygon* pl) const 
			{
				for (ListPolyVert* pvItem = m_pPolygonVerts.Begin(); pvItem; pvItem = pvItem->GetNext())
				{
					if (pvItem->m_data.m_pl == pl)
					{
						return pvItem;
					}
				}
				return 0;
			}
						
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
			//index is NOT maintained, its just useful to be able to identify components.

			inline bool IsAtPoint(const float x, const float y, const float z) const 
			{
				Aftk::Position3 pos(x,y,z);
				return Aftk::EqualityTestFVector(pos, m_pos, 0.0001f);
			}

			bool IsBoundaryVertex() const;
			bool PolysShareAttributes() const;

			bool PolygonVerticesShareAttributes() const;
			bool PolygonVerticesShareTangentSpace() const;
			bool PolygonVerticesShareColour() const;
			bool EdgesPolygonVerticesShareTangentSpace() const;
			bool EdgesPolygonVerticesShareColour() const;
		
			bool IntersectsPlane(const Aftk::Position3& planePos, const Aftk::Normal3& planeNorm) const;

			C_Edge* GetNextEdgeAround(
				const C_Edge* edge, 
				const C_Polygon* poly) const;
			C_Edge* GetUVBoundaryEdge(
				const C_Edge* edge,
				const C_Polygon* poly, 
				const C_Edge* startEdge) const;
			C_Edge* GetMaterialBoundaryEdge(
				const C_Edge* edge,
				const C_Polygon* poly, 
				const C_Edge* startEdge) const;
			C_Edge* GetNormalBoundaryEdge(
				const C_Edge* edge, 
				const C_Polygon* poly, 
				const C_Edge* startEdge) const;
			bool GetBoundaryEdge(
				C_Edge** pEdge,
				C_Polygon** pPoly, 
				const C_Edge* edge, 
				const C_Polygon* poly, 
				const C_Edge* startEdge,
				bool first);

			void GetOrderedEdgesAndPolys(
				ListEdge* startEdge,
				Ut::UtVector<Ut::DynamicVector<ListEdge*> >& edges,
				Ut::UtVector<Ut::DynamicVector<ListPoly*> >& polys,
				Aftk::C_Status& stat);
			bool ArePolygonVerticesOnSameSide(
				const C_Edge* edge1, 
				const C_Edge* edge2, 
				const C_PolygonVertex* pv1, 
				const C_PolygonVertex* pv2) const;
			bool HasIntegerUVVariation() const;	
			void SetUVsToHighestSharedValue();			
			U32 m_index;
		};
	}
}
