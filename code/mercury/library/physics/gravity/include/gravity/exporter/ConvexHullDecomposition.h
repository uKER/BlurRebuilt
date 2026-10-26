#ifndef CONVEXHULL_DECOMPOSITION_H
#define	CONVEXHULL_DECOMPOSITION_H


namespace DbUtil
{
	class I_DebugRender;
}

namespace Gravity
{
	class C_ConvexHullMesh;
	class C_Facet;
	class C_Point;
	class C_EdgeBuffer;
	class C_Edge;
	class C_Bridge;
	class C_Pocket;

	enum C_DecompositionState
	{
		E_Decomposition_Succeed		=	0,
		E_Decomposition_Fail		=	1,
		E_Decomposition_Media		
	};

	class C_ConvexHullMeshWrapper
	{
	public:
		C_ConvexHullMeshWrapper():m_ConvexHullMesh(NULL), state(E_Decomposition_Media)
		{
			//m_ConvexHullMesh = new C_ConvexHullMesh;
		}

		~C_ConvexHullMeshWrapper();
		C_ConvexHullMesh* m_ConvexHullMesh;
		C_DecompositionState state;
	};

	class C_GraphVertex;
	

	class C_GraphEdge
	{
	public:
		C_GraphEdge(): m_VertexAIndex(-1), m_VertexBIndex(-1)
		{
		}

		C_GraphEdge(S32 vertexAIndex, S32 vertexBIndex)
		{
			m_VertexAIndex = Maths::Min(vertexAIndex, vertexBIndex);
			m_VertexBIndex = Maths::Max(vertexAIndex, vertexBIndex);
		}

		bool operator < (const C_GraphEdge& otherEdge)
		{
			return (m_VertexAIndex < otherEdge.m_VertexAIndex) | 
				((m_VertexAIndex == otherEdge.m_VertexAIndex) & (m_VertexBIndex < otherEdge.m_VertexBIndex));
		}

		bool operator <= (const C_GraphEdge& otherEdge)
		{
			return (m_VertexAIndex < otherEdge.m_VertexAIndex) | 
				((m_VertexAIndex == otherEdge.m_VertexAIndex) & (m_VertexBIndex <= otherEdge.m_VertexBIndex));
		}

		bool operator > (const C_GraphEdge& otherEdge)
		{
			return (m_VertexAIndex > otherEdge.m_VertexAIndex) | 
				((m_VertexAIndex == otherEdge.m_VertexAIndex) & (m_VertexBIndex > otherEdge.m_VertexBIndex));
		}

		bool operator >= (const C_GraphEdge& otherEdge)
		{
			return (m_VertexAIndex > otherEdge.m_VertexAIndex) | 
				((m_VertexAIndex == otherEdge.m_VertexAIndex) & (m_VertexBIndex >= otherEdge.m_VertexBIndex));
		}

		bool operator == (const C_GraphEdge& otherEdge)
		{
			return (m_VertexAIndex == otherEdge.m_VertexAIndex) & (m_VertexBIndex == otherEdge.m_VertexBIndex);
		}

		bool operator != (const C_GraphEdge& otherEdge)
		{
			return (m_VertexAIndex != otherEdge.m_VertexAIndex) |(m_VertexBIndex != otherEdge.m_VertexBIndex);
		}

		//C_GraphVertex* m_pVertexA, *m_pVertexB;
		//C_GraphEdge* m_pNext;//the next edge contected in the shortest path finding
		S32 m_VertexAIndex;
		S32 m_VertexBIndex;
		F32 m_sqDist;
		
	};

	class C_GraphVertex
	{
	public:
		C_GraphVertex(): m_Visited(false),m_Dist(0.f), m_GraphEdges(10), m_pPreEdge(NULL)
		{
		}

		bool operator > (const C_GraphVertex& otherVertex)
		{
			return m_Dist < otherVertex.m_Dist;
		}

		bool operator >= (const C_GraphVertex& otherVertex)
		{
			return m_Dist <= otherVertex.m_Dist; 
		}

		bool operator < (const C_GraphVertex& otherVertex)
		{
			return m_Dist > otherVertex.m_Dist;
		}

		bool operator <= (const C_GraphVertex& otherVertex)
		{
			return m_Dist >= otherVertex.m_Dist;
		}

		bool operator == (const C_GraphVertex& otherVertex)
		{
			return m_Dist >= otherVertex.m_Dist;
		}

		bool operator != (const C_GraphVertex& otherVertex)
		{
			return m_Dist != otherVertex.m_Dist;
		}

		Maths::HVec3 m_Position;
		SharedPhysics::SortedArray<C_GraphEdge*> m_GraphEdges;
		C_GraphEdge* m_pPreEdge; //which recalled the path of the previous edges for one point to another point
		bool m_Visited;
		bool m_Discarded;
		F32 m_Dist;
		U32 m_Index;
	};

	class C_GraphTriangle
	{
	public:
		C_GraphTriangle() : m_VertexA(-1), m_VertexB(-1), m_VertexC(-1)
		{
		}

		S32 m_VertexA;
		S32 m_VertexB;
		S32 m_VertexC;
	};


	//class C_GraphEdgeBuffer
	//{
	//	//Physics::C_BlockAllocator<Edge, 32> m_Array;
	//	
	//public:
	//	FORCE_INLINE C_GraphEdgeBuffer() : m_Size(0)
	//	{
	//#ifdef PLATFORM_SPU
	//		m_pEdges = (C_GraphEdge*)Spu::Alloc(sizeof(C_GraphEdge) * MaxEdges);
	//#endif
	//	}

	//	FORCE_INLINE C_GraphEdge* Insert(const C_GraphEdge& edge)
	//	{
	//#ifndef PLATFORM_SPU
	//		C_GraphEdge* pEdge = m_Array.Create(m_Size++);
	//#else
	//		BIZ_ASSERT(m_Size < MaxEdges);
	//		C_GraphEdge* pEdge = &m_pEdges[m_Size++];
	//#endif
	//		*pEdge = edge;
	//		return pEdge;
	//	}

	//	FORCE_INLINE C_GraphEdge* Insert(C_Facet* facet, U32 index)
	//	{
	//		//const Edge edge(facet, index);
	//#ifndef PLATFORM_SPU
	//		C_GraphEdge* pEdge = m_Array.Create(m_Size++);
	//#else
	//		BIZ_ASSERT(m_Size < MaxEdges);
	//		C_GraphEdge* pEdge = &m_pEdges[m_Size++];
	//#endif
	//		pEdge->m_facet=facet;
	//		pEdge->m_index=index;
	//		return pEdge;
	//	}


	//	FORCE_INLINE C_GraphEdge* Get(U32 index)
	//	{
	//		BIZ_ASSERT(index < m_Size);
	//#ifndef PLATFORM_SPU
	//		return m_Array.Get(index);
	//#else
	//		return &m_pEdges[index];
	//#endif
	//	}

	//	FORCE_INLINE U32 Size()
	//	{
	//		return m_Size;
	//	}

	//	FORCE_INLINE bool IsEmpty()
	//	{
	//		return m_Size == 0;

	//	}

	//	FORCE_INLINE void MakeEmpty()
	//	{
	//		m_Size = 0;
	//	}
	//#ifndef PLATFORM_SPU
	//	SharedPhysics::MemoryAllocator<C_GraphEdge, 128> m_Array;
	//#else
	//	C_GraphEdge* m_pEdges;
	//#endif
	//	U32 m_Size;
	//};

	/*class C_Bridge
	{
	public:
		C_Bridge() : m_pPreFacets(&m_Facets1), m_pCurFacets(&m_Facets2), m_BridgeBoundary(10)
		{
			m_Facets1.m_numElements = 0;
			m_Facets2.m_numElements = 0;
		}

		C_Bridge& operator = (C_Bridge& other)
		{
			m_SupportPlaneNormal = other.m_SupportPlaneNormal;
			m_SupportPlaneDist = other.m_SupportPlaneDist;

			m_Facets1.m_numElements = other.m_Facets1.m_numElements;
			for(S32 i=0; i<other.m_Facets1.m_numElements; ++i)
			{
				m_Facets1[i] = other.m_Facets1[i];
			}

			m_Facets2.m_numElements = other.m_Facets2.m_numElements;
			for(S32 i=0; i<other.m_Facets2.m_numElements; ++i)
			{
				m_Facets2[i] = other.m_Facets2[i];
			}

			m_pPreFacets = (other.m_pPreFacets == &other.m_Facets1) ? &m_Facets1 : &m_Facets2;
			m_pCurFacets = (other.m_pCurFacets == &other.m_Facets1) ? &m_Facets1 : &m_Facets2;

			m_BridgeBoundary.m_numElements = other.m_BridgeBoundary.m_numElements;
			for(S32 i=0; i< other.m_BridgeBoundary.m_numElements; ++i)
			{
				m_BridgeBoundary[i]=other.m_BridgeBoundary[i];
			}
			m_Point = other.m_Point;
			return *this;
		}
		
		Maths::C_Normal m_SupportPlaneNormal;
		Maths::HF32 m_SupportPlaneDist;
		Maths::HVec3 m_Point;
		SharedPhysics::SortedArray<C_Facet*> m_Facets1;
		SharedPhysics::SortedArray<C_Facet*> m_Facets2;
		SharedPhysics::SortedArray<C_Facet*>* m_pPreFacets;
		SharedPhysics::SortedArray<C_Facet*>* m_pCurFacets;
		SharedPhysics::SortedArray<C_GraphEdge> m_BridgeBoundary;
	};*/

	/*class C_Pocket
	{
	public:
		C_Pocket(): m_Bridge(NULL), m_PocketBoundary(10)
		{}

		SharedPhysics::SortedArray<C_GraphEdge> m_PocketBoundary;
		C_Bridge* m_Bridge;
	};*/

	class C_ConvexHullDecomposition
	{
	public:
		C_ConvexHullDecomposition(C_ConvexHullMesh* pBaseMesh = NULL);

		~C_ConvexHullDecomposition();

		void SetBaseMesh(C_ConvexHullMesh* pBaseMesh);
		C_DecompositionState Decomposition(C_ConvexHullMesh* pBaseMesh, U32 count, U32 parentIndex, C_Point& startPoint, const Maths::C_Normal PCREF dir);
		void SetNumLayers(const U32 layer);
		void Reset();
		Maths::HVec3 AddStaddlePoint(C_ConvexHullMesh* pBaseMesh, C_Facet* pAdjFacet, U32 adjEdge, const Maths::C_Normal PCREF dir, const Maths::HVec3 PCREF point);

		
		void ClusterEstimation(C_ConvexHullMesh* pMesh, const Maths::HF32 PCREF eps);
		void ClusterRefinement(C_ConvexHullMesh* pMesh, const Maths::HF32 PCREF eps);
		void ConstructBridgesBoundary(C_ConvexHullMesh* pMesh, C_Bridge& bridge);
		void CalculatePockets(C_ConvexHullMesh* pMesh);
		void CalculateKnots(C_ConvexHullMesh* pMesh);
		//void CalculatePockets(U32 bridgeIndex, C_ConvexHullMesh* pMesh, C_Facet* pFacet,  U32 index, C_GraphEdgeBuffer& edgeBuffer, C_GraphEdge& stack);
		void C_ConvexHullDecomposition::GetShorestPath(const U32 vertexSourceIndex, const U32 vertexTargetIndex, SharedPhysics::SortedArray<C_GraphEdge>& shorestPath, const U32 bridgeBoundaryIndex);
		void DebugRender(DbUtil::I_DebugRender* pRender);
		void DrawShorestPath(DbUtil::I_DebugRender* pRender, SharedPhysics::SortedArray<Gravity::C_GraphEdge>& shorestPath);
		void DrawBridges(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender);
		void DrawBridges(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender, const U32 bridgeIndex);
		void DrawPockets(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender, const U32 bridgeIndex);
		void DrawBridgesBoundary(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender);
		void DrawBridgesBoundary(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender, const U32 bridgeIndex);

		bool CalculateBridges(C_ConvexHullMesh* pMesh, C_Facet* pCurFacet, const Maths::HF32 PCREF eps);
		void SanityCheck();
		void SanityCheckPreFacets();
		
		
		C_ConvexHullMesh* m_pBaseMesh;
		C_ConvexHullMeshWrapper* m_ConvexHullMeshes;
		U32 m_Layer;
		U32 m_MaxMeshWrapper;
		U32 m_ValidMeshCount;

		SharedPhysics::SortedArray<C_Bridge> m_Bridges;
		SharedPhysics::SortedArray<C_GraphVertex> m_Vertices;
		SharedPhysics::SortedArray<C_GraphEdge> m_Edges;
		SharedPhysics::SortedArray<C_GraphTriangle> m_Triangles;
		/*C_EdgeBuffer edgeBuffer;
		C_Edge stack[500];*/

	private:
		void SetFailFlags(U32 index);
		void ClusterEstimation(C_ConvexHullMesh* pMesh, C_Facet* pFacet, C_Bridge& bridge, const U32 bridgeIndex, const Maths::HF32 PCREF eps);
		bool ClusterRefinement(C_ConvexHullMesh* pMesh, C_Facet* pFacet, C_Bridge& bridge, const U32 bridgeIndex, const Maths::HF32 PCREF eps);
	};
}

#endif