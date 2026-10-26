#ifndef CONVEXHULLGENERATION_H
#define CONVEXHULLGENERATION_H

#include "gravity/sharedphysics/BinaryHeap.h"
#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/sharedphysics/sortedarray.h"

namespace Collision
{
	class C_AABBMinMax;
}

namespace Gravity
{
	class C_Facet;
	class C_EdgeBuffer;

	const U32 lookUp[3] = {1, 2, 0};
	inline U32 incMod3(U32 i) { return lookUp[i]; } 

	enum GenerationState
	{
		E_Generation_Succeed = 0,
		E_Generation_Fail,
		E_Generating
	};

	/*class C_PointFacetStructure
	{
	public:
		C_PointFacetStructure(C_Facet* pFacet = NULL, const S32 index = -1): m_pFacet(pFacet), m_Index(index)
		{
		}
		C_Facet* m_pFacet;
		S32 m_Index;
	};*/

	class C_Point
	{
	public:
		C_Point(Maths::HVec3 p = Maths::Constants<Maths::HVec3>::Zero()): allocatedOutsidePoint(false), deleted(false)
			//m_Facets(4)
		{
			m_Point = p;
		}

		C_Point(C_Point& other)
		{
			m_Point = other.m_Point;
			allocatedOutsidePoint = other.allocatedOutsidePoint;
			deleted = other.deleted;
			//m_Facets = other.m_Facets;
		}

		C_Point& operator =(C_Point& other)
		{
			m_Point = other.m_Point;
			allocatedOutsidePoint = other.allocatedOutsidePoint;
			deleted = other.deleted;
			m_IndexInOriginalModel = other.m_IndexInOriginalModel;
			//m_Facets = other.m_Facets;
			return *this;
		}

	/*	void SetFacetsVisited();
		void ClearFacetsVisited();*/
	

		Maths::HVec3 m_Point;
		U32 m_IndexInOriginalModel;
		bool allocatedOutsidePoint;
		bool deleted;
		//SharedPhysics::SortedArray<C_PointFacetStructure> m_Facets;
	};

	class C_OutsidePoint
	{
	public:
		C_OutsidePoint()
		{
		}

		C_OutsidePoint& operator = (C_OutsidePoint& other)
		{
			m_OutsidePoint = other.m_OutsidePoint;
			m_Index = other.m_Index;
			return *this;
		}

		C_Point m_OutsidePoint;
		U32 m_Index;
	};

	class C_Edge
	{
	public:

		C_Edge() {}
		C_Edge(C_Facet *facet, const U32 index) : m_facet(facet), m_index(index) {}

		C_Edge(const C_Edge& other) : m_facet(other.m_facet), m_index(other.m_index)
		{
		}

		C_Edge& operator = (const C_Edge& other)
		{
			m_facet = other.m_facet;
			m_index = other.m_index;
			return *this;
		}

		C_Facet *getFacet() const { return m_facet; }
		U32   getIndex() const { return m_index; }

		inline U32 getSource() const ;

		inline U32 getTarget() const ;

		
		C_Facet *m_facet;
		U32   m_index;

	};

	class C_Facet
	{
	public:
		C_Facet(U32 indexA=0, U32 indexB=0 , U32 indexC=0) :m_Discarded(false), m_Visited(false)//, visible(false)
		{
			m_indice[0]=indexA;
			m_indice[1]=indexB;
			m_indice[2]=indexC;
			m_FurthestPointDist = -Maths::Constants<Maths::HF32>::Max();
		}

		inline U32 operator[](const U32 i) const 
		{ 
			return m_indice[i]; 
		} 

		inline void Initialise(SharedPhysics::SortedArray<U32>& points, SharedPhysics::SortedArray<C_Point>& supportPoints)
		{
			using namespace Maths;
			C_Point& point0 = supportPoints[points[m_indice[0]]];
			C_Point& point1 = supportPoints[points[m_indice[1]]];
			C_Point& point2 = supportPoints[points[m_indice[2]]];
			/*point0.m_Facets.AddToEnd(&C_PointFacetStructure(this, 0));
			point1.m_Facets.AddToEnd(&C_PointFacetStructure(this, 1));
			point2.m_Facets.AddToEnd(&C_PointFacetStructure(this, 2));*/
			const HVec3& a(point0.m_Point);
			const HVec3& b(point1.m_Point);
			const HVec3& c(point2.m_Point);
			const HVec3 ab(b-a);
			const HVec3 ac(c-a);
			m_Normal = C_Normal(ac^ab);
			//DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow((a+b+c)/3.f, m_Normal, 2.f, 0.25f, RGBA(0, 0, 255));
			m_Dist = m_Normal|a;
		}


		inline void AddOutsideSet(C_Point* p,  U32 index, const Maths::HF32 signDist)
		{
			using namespace Maths;
			C_OutsidePoint outPoint;
			outPoint.m_OutsidePoint = *p; 
			outPoint.m_Index = index;
			outPoint.m_OutsidePoint.allocatedOutsidePoint = true;

			//const HF32 dist =(m_Normal|p->m_Points) - m_Dist;
			if(signDist.GT(m_FurthestPointDist))
			{
				m_FurthestPoint = outPoint;
				m_FurthestPointDist = signDist;
			}
			
			m_OutsideSet.AddToEnd(&outPoint);
		}

		inline bool IsAbove(Maths::HVec3& p, Maths::HF32& signDist)
		{
			using namespace Maths;
			//const HF32 zero(-Constants<HF32>::Epsilon());
			//const HF32 zero(Constants<HF32>::Zero());
			//const HF32 zero(Constants<HF32>::Epsilon());
			const HF32 epsilon(0.0001f);
			signDist =(m_Normal|p) - m_Dist;
			return signDist.GT(epsilon);
		}

		inline bool IsAbove(const Maths::HVec3& p)
		{
			using namespace Maths;
			//const HF32 zero(Constants<HF32>::Zero());
			const HF32 epsilon(0.0001f);
			const HF32 signDist((m_Normal|p) - m_Dist);
			return signDist.GT(epsilon);
		}

		bool IsDistarded()
		{
			return m_Discarded;
		}

		inline S32 NumOutsideSet()
		{
			return m_OutsideSet.m_numElements;
		}

		//inline bool link(U32 edge0, C_Facet *facet, U32 edge1); 
		inline void link(U32 edge0, C_Facet *facet, U32 edge1); 

		void silhouette(const U32 _index, const Maths::HVec3 PCREF w, C_EdgeBuffer& edgeBuffer, C_Edge* const __restrict stack); 


		void silhouette(const Maths::HVec3 PCREF w, C_EdgeBuffer& edgeBuffer, C_Edge* const __restrict stack);


		Maths::C_Normal m_Normal;
		Maths::HF32 m_Dist;
		SharedPhysics::SortedArray<C_OutsidePoint> m_OutsideSet;
		C_OutsidePoint m_FurthestPoint;
		Maths::HF32 m_FurthestPointDist;
		bool m_Discarded;
		C_Facet* m_adjFacets[3];	//the triangle adjacent to edge i in this triangle
		U32 m_adjEdges[3];
		U32 m_indice[3];
		U32 m_BridgeIndex;
		U32 m_Index;
		bool m_Visited;
	};

	class C_EdgeBuffer
	{
		//Physics::C_BlockAllocator<Edge, 32> m_Array;
		
	public:
		FORCE_INLINE C_EdgeBuffer() : m_Size(0)
		{
	#ifdef PLATFORM_SPU
			m_pEdges = (C_Edge*)Spu::Alloc(sizeof(C_Edge) * MaxEdges);
	#endif
		}

		FORCE_INLINE C_Edge* Insert(const C_Edge& edge)
		{
	#ifndef PLATFORM_SPU
			C_Edge* pEdge = m_Array.Create(m_Size++);
	#else
			BIZ_ASSERT(m_Size < MaxEdges);
			C_Edge* pEdge = &m_pEdges[m_Size++];
	#endif
			*pEdge = edge;
			return pEdge;
		}

		FORCE_INLINE C_Edge* Insert(C_Facet* facet, U32 index)
		{
			//const Edge edge(facet, index);
	#ifndef PLATFORM_SPU
			C_Edge* pEdge = m_Array.Create(m_Size++);
	#else
			BIZ_ASSERT(m_Size < MaxEdges);
			C_Edge* pEdge = &m_pEdges[m_Size++];
	#endif
			pEdge->m_facet=facet;
			pEdge->m_index=index;
			return pEdge;
		}

		FORCE_INLINE C_Edge* Get(U32 index)
		{
			BIZ_ASSERT(index < m_Size);
	#ifndef PLATFORM_SPU
			return m_Array.Get(index);
	#else
			return &m_pEdges[index];
	#endif
		}

		FORCE_INLINE U32 Size()
		{
			return m_Size;
		}

		FORCE_INLINE bool IsEmpty()
		{
			return m_Size == 0;

		}

		FORCE_INLINE void MakeEmpty()
		{
			m_Size = 0;
		}
	#ifndef PLATFORM_SPU
		SharedPhysics::MemoryAllocator<C_Edge, 128> m_Array;
	#else
		C_Edge* m_pEdges;
	#endif
		U32 m_Size;
	};


	class C_ConvexHullMesh
	{
	public:

		C_ConvexHullMesh() : m_Iterator(&m_Allocator), m_Points(64),
			m_SupportPoints(64), m_Facets(128)
		{
		}

		~C_ConvexHullMesh()
		{
		}

		void Clear();
		
		GenerationState TetrahedronGeneration(/*const SharedPhysics::SortedArray<Maths::HVec3>& points*/);
		GenerationState Build(const SharedPhysics::SortedArray<Maths::HVec3>& points);
		GenerationState Initialise(const SharedPhysics::SortedArray<Maths::HVec3>& point);
		GenerationState StepBuild();

		void SanityCheck();
		void SanityCheckSupport();
		void DebugRender(DbUtil::I_DebugRender* pRender, Maths::RGBA PCREF color, bool wireframe);
		void RemapFacets();
		void ClearFacetsVisitedFlags();
		Collision::C_AABBMinMax ConstructC_AABBMinMax();
		//void RemoveRedundantTriangle();
		U32 NumValidFacets();

		inline Maths::HVec3 GetPointFromFacetLocalIndex(const U32 localIndex)
		{
			return m_SupportPoints[m_Points[localIndex]].m_Point;
		}

		inline Maths::HVec3 GetPointFromGlobalIndex(const U32 globalIndex)
		{
			return m_SupportPoints[globalIndex].m_Point;
		}


		//SharedPhysics::SortedArray<C_ConvexVertex> m_Points;
		SharedPhysics::MemoryAllocator<C_Facet, 32, true> m_Allocator;
		SharedPhysics::C_MemoryIterator<C_Facet, 32, true> m_Iterator;
		SharedPhysics::SortedArray<U32> m_Points;
		//SharedPhysics::SortedArray<Maths::HVec3> m_ResultPoints;
		SharedPhysics::SortedArray<C_Point> m_SupportPoints; 
		SharedPhysics::SortedArray<C_Facet*> m_Facets;
		SharedPhysics::BinaryHeapStorage<C_Facet> heap;
		C_EdgeBuffer edgeBuffer;
		C_Edge stack[500];

	private:
		C_ConvexHullMesh& operator = (const C_ConvexHullMesh& other);
	};


}

#endif