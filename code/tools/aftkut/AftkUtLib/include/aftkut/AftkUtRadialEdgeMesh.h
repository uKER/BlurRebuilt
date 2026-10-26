#ifndef AFTKUT_LINKED_MESH_NODE_H
#define AFTKUT_LINKED_MESH_NODE_H

namespace AftkUt
{
	namespace RadialEdgeMesh
	{
		//Linked Mesh node and component classes

		class C_Polygon;
		class C_PolygonVertex;

		class C_Vertex;
		class C_Edge;
		class C_PolygonVertex;
		class C_Mesh;
		class C_Plane;

		//defines for templated list items, reducing clutter

		typedef ListItem<C_Vertex> ListVert; 
		typedef ListItem<C_Edge> ListEdge;
		typedef ListItem<C_Polygon> ListPoly;
		typedef ListItem<C_PolygonVertex> ListPolyVert;

		//MeshNode class: The AftkMesh in LinkedMesh mesh form. 

		class C_Mesh
		{
			friend class C_Polygon;
			friend class C_PolygonVertex;

			friend class C_Vertex;
			friend class C_Edge;
			//friend class C_PolygonVertex;
			
			C_Mesh(){}//private: makes no sense to construct without aftkmesh

		public:
			//Takes a pointer to an Aftk mesh and creates a linked edge mesh from it
			//The aftkmesh pointer is stored, so the aftkmesh can be rebuilt.
			C_Mesh(Aftk::C_SceneMeshNode* pAftkMesh, bool bUseInputAsOutput=true);
			~C_Mesh(void);

			//Uses the pointer to an Aftk mesh and creates a linked edge mesh from it
			void Build(Aftk::C_Status& stat, bool bWorldSpace = false);

			//Creates an Aftk mesh from the edge mesh, at the pointer stored in the 
			//creation of the edge mesh.
			Aftk::C_SceneMeshNode* BuildAftk(Aftk::C_Status& stat, 
				Ut::Vector<AftkUt::RadialEdgeMesh::ListPoly*>* polyList=0);

			//Output debug information on the edge mesh links
			bool ValidateMesh();

			//Accessors

			inline ListVert* GetFirstVertex() const
			{
				return m_vertices.Begin();
			}

			inline ListEdge* GetFirstEdge() const
			{
				return m_edges.Begin();
			}

			inline ListPoly* GetFirstPoly() const
			{
				return m_polygons.Begin();
			}

			//Cut all polys in mesh by plane
			void CutByPlane(
				const Aftk::Position3& planePos,
				const Aftk::Normal3& planeNormal);
			//all polys in array
			void CutByPlane(
				const Aftk::Position3& planePos, 
				const Aftk::Normal3& planeNormal, 
				Ut::UtVector<Ut::DynamicVector<ListPoly*> >& polys);
			//single poly
			void CutByPlane(
				const Aftk::Position3& planePos, 
				const Aftk::Normal3& planeNormal, ListPoly* poly,
				ListPoly** front=0, ListPoly** back=0);

			int RemoveCoPlanarPolys(bool bProcessBoundaryEdges, Aftk::C_Status& stat);
			void CreateUVIntegerOffsets(Aftk::C_Status& stat);
			
			void ClipByPlaneList(const Ut::Vector<C_Plane>* planes, Ut::Vector<ListPoly*>* polys=0);

			void GetPolysInsidePlaneList(	const Ut::Vector<C_Plane>* planes, 
												Ut::Vector<ListPoly*>* polys, bool bAllowIntersect=true);
			int GetNumPolys(){return m_polygons.Size();}

		private:


			//Initial members for creating the structure

			ListItem<C_Vertex>*	CreateVertex(C_Vertex* vt, Aftk::C_Status& stat);

			ListItem<C_Polygon>*	CreatePolygon(	
				Ut::UtVector<Ut::DynamicVector<ListItem<C_Vertex>*> >& verts, 
				Aftk::C_Status& stat);
			
			ListItem<C_Polygon>*	CreateTriangle(	
				ListItem<C_Vertex>* v1, 
				ListItem<C_Vertex>* v2, 
				ListItem<C_Vertex>* v3, 
				Aftk::C_Status& stat);
			
			ListItem<C_Polygon>*	CreateQuad(
				ListItem<C_Vertex>* v1, 
				ListItem<C_Vertex>* v2, 
				ListItem<C_Vertex>* v3, 
				ListItem<C_Vertex>* v4, 
				Aftk::C_Status& stat);

			ListItem<C_Edge>* CreateEdge(
				ListItem<C_Vertex>* v1,  
				ListItem<C_Vertex>* v2,
				ListItem<C_Polygon>* pl, 
				Aftk::C_Status& stat);

			S32 AddAftkPolygonVertexFromLinked(
				const ListPoly& plItem,
				const C_Vertex& vt);


			//Members for editing the structure...

			//deleting removes component without affecting others

			void DeleteVertex(
				ListItem<C_Vertex>* v1, 
				Aftk::C_Status& stat);

			void DeleteEdge(
				ListItem<C_Edge>* ed,
				Aftk::C_Status& stat,
				bool bDeleteVertices = true);

			void DeletePoly(
				C_Polygon* pl, 
				Aftk::C_Status& stat,
				bool bDeleteEdges = true);

			//collapsing component requires other components to move

			void CollapseVertex(
				ListItem<C_Vertex>* vt,
				Aftk::C_Status& stat);

			void CollapseEdge(
				ListItem<C_Edge>* ed, 
				Aftk::C_Status& stat,
				int deleteIndex = 1, 
				C_Edge* boundaryEdge = 0);

			void CollapsePoly(
				ListItem<C_Polygon>* pl,
				Aftk::C_Status& stat);

			//Splitting existing components adds new geometry

			ListItem<C_Vertex>* SplitEdge(
				ListItem<C_Edge>* ed, 
				const float splitFactor,
				Aftk::C_Status& stat);

			void  SplitPoly(
				ListItem<C_Polygon>* plItem, 
				ListItem<C_Edge>* ed1, 
				ListItem<C_Edge>* ed2, 
				const float splitFactor1,
				const float splitFactor2,
				Aftk::C_Status& stat, ListPoly** pl0=0,
				ListPoly** pl1=0);

			void  SplitPoly(
				ListItem<C_Polygon>* plItem, 
				ListItem<C_Vertex>* newVtItem1, 
				ListItem<C_Vertex>* newVtItem2, 
				Aftk::C_Status& stat, ListPoly** pl0=0,
				ListPoly** pl1=0);

			ListItem<C_Edge>* FlipEdge(
				ListItem<C_Edge>* edge);

			inline Ut::UtVector<Ut::DynamicVector<Aftk::C_VertexDataChannelInfo> >& GetAftkVertexDataChannelInfo()
			{
				return m_aftkVertexDataChannelInfo;
			}
			
			void RemoveDegeneratePolys(Aftk::C_Status& stat);
			void RemoveDegeneratePolys(C_Polygon* deletedEdgePoly, Aftk::C_Status& stat);

			//Components stored as lists

			List<C_Vertex>		m_vertices;
			List<C_Edge>		m_edges;
			List<C_Polygon>		m_polygons;

			//Pointer to original aftkMesh
			Aftk::C_SceneMeshNode* m_pAftkMeshIn;
			Aftk::C_SceneMeshNode* m_pAftkMeshOut;

			//storage for vertex colours etc. 
			Ut::UtVector<Ut::DynamicVector<Aftk::C_VertexDataChannelInfo> > m_aftkVertexDataChannelInfo;
			
		};

		class C_Tolerances
		{
		public:

			C_Tolerances()
			{
				Reset();
			}

			void Reset()
			{
				m_epsilon = 0.000001f;
				m_normalEpsilon = 0.01f;
				m_sliverousEpsilon = 0.000001f;
				m_UVEpsilon = 0.00001f;
				m_posEpsilon = 0.001f;
				m_maxEdgeLength = 32.0f;
			}

			void Set(	float epsilon,		float normalEpsilon,	float sliverousEpsilon,
						float uvEpsilon,	float posEpsilon,		float maxEdgeLength)
			{
				m_epsilon = epsilon;
				m_normalEpsilon = normalEpsilon;
				m_sliverousEpsilon = sliverousEpsilon;
				m_UVEpsilon = uvEpsilon;
				m_posEpsilon = posEpsilon;
				m_maxEdgeLength = maxEdgeLength;
			}

			float Epsilon(){return m_epsilon;}
			float NormalEpsilon(){return m_normalEpsilon;}
			float SliverousEpsilon(){return m_sliverousEpsilon;}
			float UVEpsilon(){return m_UVEpsilon;}
			float PosEpsilon(){return m_posEpsilon;}
			float MaxEdgeLength(){return m_maxEdgeLength;}

		private:

			float m_epsilon;
			float m_normalEpsilon;
			float m_sliverousEpsilon;
			float m_UVEpsilon;
			float m_posEpsilon;
			float m_maxEdgeLength;
		};

		extern Debug::Logger& g_Log;
		extern C_Tolerances g_Tolerances;

		class C_Plane
		{
		public:

			C_Plane(){}
			C_Plane(Aftk::Position3 pos, Aftk::Normal3 norm) : m_pos(pos), m_norm(norm){}
			C_Plane(const C_Plane& other)
			{
				m_pos = other.m_pos;
				m_norm = other.m_norm;
			}

			Aftk::Position3 m_pos;
			Aftk::Normal3 m_norm;
		};

		enum E_PlaneTest
		{
			E_PlaneTest_InFront, E_PlaneTest_Behind, E_PlaneTest_Intersect
		};
	}
}


//comment out the line below to toggle error checking
#define AFTKUT_RADIAL_EDGE_MESH_CHECK_ERRORS

//Error checking defines, some set Aftk::C_Status references.

#ifdef  AFTKUT_RADIAL_EDGE_MESH_CHECK_ERRORS
#define VALIDATE_ALLOC_STAT(PTR)\
	if (PTR == 0)\
	{\
		stat = Aftk::C_Status::E_Status_OutOfMemory;\
		return 0;\
	}
#else 
	#define VALIDATE_ALLOC_STAT(PTR)
#endif

#ifdef  AFTKUT_RADIAL_EDGE_MESH_CHECK_ERRORS
#define VALIDATE_ALLOC_STAT_VOID(PTR)\
	if (PTR == 0)\
	{\
		stat = Aftk::C_Status::E_Status_OutOfMemory;\
		return;\
	}
#else 
	#define VALIDATE_ALLOC_STAT(PTR)
#endif

#ifdef  AFTKUT_RADIAL_EDGE_MESH_CHECK_ERRORS
#define VALIDATE_PTR(PTR)\
	if (PTR == 0)\
	{\
		return 0;\
	}
#else 
	#define VALIDATE_PTR(PTR)
#endif

#ifdef  AFTKUT_RADIAL_EDGE_MESH_CHECK_ERRORS
#define VALIDATE_PTR_VOID(PTR)\
	if (PTR == 0)\
	{\
		return;\
	}
#else 
	#define VALIDATE_PTR_VOID(PTR)
#endif

#ifdef  AFTKUT_RADIAL_EDGE_MESH_CHECK_ERRORS
#define CHECK_STATUS(STAT)\
	if (STAT != Aftk::C_Status::E_Status_Success)\
	{\
		return 0;\
	}
#else 
	#define CHECK_STATUS(STAT)
#endif

#ifdef  AFTKUT_RADIAL_EDGE_MESH_CHECK_ERRORS
#define CHECK_STATUS_VOID(STAT)\
	if (STAT != Aftk::C_Status::E_Status_Success)\
	{\
		return;\
	}
#else 
	#define CHECK_STATUS(STAT)
#endif


#endif

