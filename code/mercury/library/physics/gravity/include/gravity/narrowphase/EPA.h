#ifndef EPA_H
#define EPA_H

#ifdef PLATFORM_SPU
#include "gravity/narrowphase/SPUManager.h"
#endif 

#include "gravity/narrowphase/Primitive.h"
#include "gravity/sharedphysics/BinaryHeap.h"
#include "gravity/sharedphysics/MemoryAllocator.h"

namespace Collision
{
#define MaxFacets 64
#define MaxSupportPoints 64
#define MaxEdges 128

enum GJKResultType
{
	E_NO_INTERSECTION,
	E_MARGIN_INTERSECTION,
	E_DEEP_INTERSECTION
};

const U32 lookUp[3] = {1, 2, 0};
__forceinline U32 incMod3(U32 i) { return lookUp[i]; } 

class Facet2;

class Edge {
public:
    FORCE_INLINE Edge() {}
	FORCE_INLINE Edge(Facet2 *facet, const U32 index) : m_facet(facet), m_index(index) {}

	FORCE_INLINE Edge(const Edge& other) : m_facet(other.m_facet), m_index(other.m_index)
	{
	}

	FORCE_INLINE Edge& operator = (const Edge& other)
	{
		m_facet = other.m_facet;
		m_index = other.m_index;
		return *this;
	}

    FORCE_INLINE Facet2 *getFacet() const { return m_facet; }
    FORCE_INLINE U32   getIndex() const { return m_index; }

	FORCE_INLINE U32 getSource() const;
    FORCE_INLINE U32 getTarget() const;
  
    Facet2 *m_facet;
    U32   m_index;
};



class EdgeBuffer
{
	//Physics::C_BlockAllocator<Edge, 32> m_Array;
	
public:
	FORCE_INLINE EdgeBuffer() : m_Size(0)
	{
#ifdef PLATFORM_SPU
		m_pEdges = (Edge*)Spu::Alloc(sizeof(Edge) * MaxEdges);
#endif
	}

	FORCE_INLINE Edge* Insert(const Edge& edge)
	{
		BIZ_ASSERT(m_Size < MaxEdges);
		Edge* pEdge = &m_pEdges[m_Size++];
		*pEdge = edge;
		return pEdge;
	}

	FORCE_INLINE Edge* Insert(Facet2* facet, U32 index)
	{
		//const Edge edge(facet, index);
		BIZ_ASSERT(m_Size < MaxEdges);
		Edge* pEdge = &m_pEdges[m_Size++];
		pEdge->m_facet=facet;
		pEdge->m_index=index;
		return pEdge;
	}

	FORCE_INLINE Edge* Get(U32 index)
	{
		BIZ_ASSERT(index < m_Size);
		return &m_pEdges[index];
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
	Edge m_pEdges[MaxEdges];
#else
	Edge* m_pEdges;
#endif
	U32 m_Size;
};

class Facet2
{
public:

	NOALIAS inline Facet2(): m_closest( Maths::UNDEFINED ), m_dir(Maths::UNDEFINED), m_dist(Maths::UNDEFINED), m_obsolete(false)
	{
		m_adjFacets[0] = m_adjFacets[1] = m_adjFacets[2] = NULL;
		m_adjEdges[0] = m_adjEdges[1] = m_adjEdges[2] = -1;
	}

	NOALIAS inline Facet2(const U32 _i0, const U32 _i1, const U32 _i2)
		: m_closest( Maths::UNDEFINED ), m_dir(Maths::UNDEFINED), m_dist(Maths::UNDEFINED), m_obsolete(false)
	{
		m_indices[0]=_i0;
		m_indices[1]=_i1;
		m_indices[2]=_i2;


		m_adjFacets[0] = m_adjFacets[1] = m_adjFacets[2] = NULL;
		m_adjEdges[0] = m_adjEdges[1] = m_adjEdges[2] = -1;
	}

	NOALIAS inline bool Valid()
	{
		return m_adjFacets[0] != NULL && m_adjFacets[1] != NULL && 
			m_adjFacets[2] != NULL;
	}

	inline U32 operator[](const U32 i) const 
	{ 
		return m_indices[i]; 
	} 
	
	inline bool link(U32 edge0, Facet2 *facet, U32 edge1);

	
    inline bool isObsolete() const { return m_obsolete; }
    

	inline bool computeClosest(const Maths::HVec3 *verts);

	inline bool computeClosest(const Maths::HVec3*  a_verts, const Maths::HVec3* b_verts);
	inline bool computeClosest(const U32 _i0, const U32 _i1, const U32 _i2, const Maths::HVec3*  a_verts, const Maths::HVec3*  b_verts);
    
	inline const Maths::HVec3& getClosest() const 
	{ 
		return m_closest; 
	}

	inline void setClosest(const Maths::HVec3 PCREF closest)
	{ 
		m_closest = closest;
	}

	inline void setDir(const Maths::HVec3 PCREF dir)
	{ 
		m_dir = dir;
	} 

	inline const Maths::HVec3& getDir() const 
	{ 
		return m_dir; 
	}
    

	inline void isValid(const U32 i0, const U32 i1, const U32 i2, const Maths::HVec3*  a_verts, const Maths::HVec3*   b_verts,
		S32& b1, Maths::Scalar_Bool& b2);

	inline Maths::HF32 getDist() const 
	{ 
		//return Maths::Load<Maths::HF32>( &m_dist ); 
		return m_dist;
	}
	

	inline void getClosestPoint(const Maths::HVec3 *aBuf, const Maths::HVec3 *bBuf, Maths::HVec3& closestA, Maths::HVec3& closestB) const 
	{
		/*using namespace Maths;

		const HVec3& pa = aBuf[m_indices[0]];
		const HVec3& pb = bBuf[m_indices[0]];
		const HF32 lambda1(Load<Maths::HF32>( &m_lambda1 ));
		const HF32 lambda2(Load<Maths::HF32>( &m_lambda2 ));
		const HF32 det(Load<Maths::HF32>( &m_det ));

		closestA=pa + (lambda1 * (aBuf[m_indices[1]] - pa) + lambda2  * (aBuf[m_indices[2]] - pa)) / det ;
		closestB=pb + (lambda1 * (bBuf[m_indices[1]] - pb) + lambda2  * (bBuf[m_indices[2]] - pb)) / det ;*/


		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());

		const HVec3 pa0(aBuf[m_indices[0]]);
		const HVec3 pa1(aBuf[m_indices[1]]);
		const HVec3 pa2(aBuf[m_indices[2]]);

		const HVec3 pb0(bBuf[m_indices[0]]);
		const HVec3 pb1(bBuf[m_indices[1]]);
		const HVec3 pb2(bBuf[m_indices[2]]);

		const HVec3 p0(pa0-pb0);
		const HVec3 v1((pa1-pb1) - p0);
		const HVec3 v2((pa2-pb2) - p0);

		const HF32 v1dv1 = v1 | v1;
		const HF32 v1dv2 = v1 | v2;
		const HF32 v2dv2 = v2 | v2;
		const HF32 p0dv1 = p0 | v1; 
		const HF32 p0dv2 = p0 | v2;
	    
		const HF32 det( v1dv1 * v2dv2 - v1dv2 * v1dv2 ); // non-negative
		BIZ_ASSERT(det.GE(zero));
		const HF32 lambda1(p0dv2 * v1dv2 - p0dv1 * v2dv2);
		const HF32 lambda2(p0dv1 * v1dv2 - p0dv2 * v1dv1);

	
		closestA=pa0 + (lambda1 * (pa1 - pa0) + lambda2  * (pa2 - pa0)) / det ;
		closestB=pb0 + (lambda1 * (pb1 - pb0) + lambda2  * (pb2 - pb0)) / det ;
    }

	inline void silhouette(const Maths::HVec3 PCREF w, EdgeBuffer& edgeBuffer, Edge* const stack);

	inline bool operator <(const Facet2& b) const
	{
		//return m_dist.LT( b.getDist());
		return m_dist < b.m_dist;
	}

	inline bool operator > (const Facet2& b) const
	{
		return m_dist > b.m_dist;
	}

	inline bool operator <=(const Facet2& b) const
	{
		return m_dist <= b.m_dist;
	}


//private:
	inline void silhouette(const U32 index, const Maths::HVec3 PCREF w, EdgeBuffer& edgeBuffer, Edge* const stack);
	
	Maths::HVec3 m_closest;		//closest point to the origin
	Maths::HVec3 m_dir; // search direction
	//F32 m_lambda1;			//barycentric coordinate
	//F32 m_lambda2;
	//F32  m_det;			//determinate
	//F32  m_dist;		//the distance of v to the origin
	Maths::HF32 m_dist;
	Facet2* m_adjFacets[3];		//the triangle adjacent to edge i in this triangle
	S32   m_adjEdges[3];		//the edge connected with the corresponding triangle
	S32   m_indices[3];			//the index of vertices of the triangle
	bool  m_obsolete;			//a flag to denote whether the triangle is visible from the new support point
	
};


class FacetSimplified
{
	public:

	FacetSimplified(): m_closest(Maths::UNDEFINED), m_dist(Maths::UNDEFINED), m_dir(Maths::UNDEFINED)//, m_closest(Maths::UNDEFINED)
	{
	}

		FacetSimplified(const U32& _i0, const U32& _i1, const U32& _i2): m_closest(Maths::UNDEFINED), m_dist(Maths::UNDEFINED), m_dir(Maths::UNDEFINED)//, m_closest(Maths::UNDEFINED)
	{
		m_indices[0]=_i0;
		m_indices[1]=_i1;
		m_indices[2]=_i2;

	}

	FacetSimplified& operator = (const FacetSimplified& other)
	{
		//m_closest = other.m_closest;
		m_dir = other.m_dir;
		m_dist = other.m_dist;		//the distance of v to the origin
		m_indices[0] = other.m_indices[0];			//the index of vertices of the triangle
		m_indices[1] = other.m_indices[1];			//the index of vertices of the triangle
		m_indices[2] = other.m_indices[2];			//the index of vertices of the triangle

		return *this;
	}

	__forceinline U32 operator[](const U32& i) const 
	{ 
		return m_indices[i]; 
	} 

	void Initialise(const U32& i0, const U32& i1, const U32& i2, const Maths::HVec3* __restrict a_verts, const Maths::HVec3* __restrict  b_verts);

	__forceinline bool isValid(const U32& i0, const U32& i1, const U32& i2, const Maths::HVec3* __restrict a_verts, const Maths::HVec3* __restrict  b_verts);

	__forceinline Maths::HF32 getDist() const 
	{ 
		//return Maths::Load<Maths::HF32>( &m_dist ); 
		return m_dist;
	}
	
	__forceinline void getClosestPoint(const Maths::HVec3 *aBuf, const Maths::HVec3 *bBuf, Maths::HVec3& closestA, Maths::HVec3& closestB) const 
	{
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());

		const HVec3 pa0(aBuf[m_indices[0]]);
		const HVec3 pa1(aBuf[m_indices[1]]);
		const HVec3 pa2(aBuf[m_indices[2]]);

		const HVec3 pb0(bBuf[m_indices[0]]);
		const HVec3 pb1(bBuf[m_indices[1]]);
		const HVec3 pb2(bBuf[m_indices[2]]);

	/*	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(pa0, pa1, pa2, RGBA(255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(pb0, pb1, pb2, RGBA(0, 255, 0));*/


		const HVec3 p0(pa0-pb0);
		const HVec3 v1((pa1-pb1) - p0);
		const HVec3 v2((pa2-pb2) - p0);

		const HF32 v1dv1 = v1 | v1;
		const HF32 v1dv2 = v1 | v2;
		const HF32 v2dv2 = v2 | v2;
		const HF32 p0dv1 = p0 | v1; 
		const HF32 p0dv2 = p0 | v2;
	    
		const HF32 det( v1dv1 * v2dv2 - v1dv2 * v1dv2 ); // non-negative
		BIZ_ASSERT(det.GE(zero));
		const HF32 lambda1(p0dv2 * v1dv2 - p0dv1 * v2dv2);
		const HF32 lambda2(p0dv1 * v1dv2 - p0dv2 * v1dv1);

	
		closestA=pa0 + (lambda1 * (pa1 - pa0) + lambda2  * (pa2 - pa0)) / det ;
		closestB=pb0 + (lambda1 * (pb1 - pb0) + lambda2  * (pb2 - pb0)) / det ;
    }

	inline bool operator <(const FacetSimplified& b) const
	{
		return m_dist.LT( b.m_dist);
	}

	inline bool operator > (const FacetSimplified& b) const
	{
		return m_dist.GT( b.m_dist);
	}

	inline bool operator <=(const FacetSimplified& b) const
	{
		return m_dist.LE(b.m_dist);
	}

	inline Maths::HVec3& getSupportDir()
	{
		return m_dir;
	}

	inline void setSupportDir(const Maths::HVec3 PCREF dir)
	{
		m_dir = dir; 
	}

	inline Maths::HVec3& getClosest()
	{
		return m_closest;
	}

	inline void setClosest(const Maths::HVec3 PCREF closest)
	{
		m_closest = closest; 
	}


	//Maths::HVec3 m_closest;
private:
	
	Maths::HVec3 m_closest; 
	Maths::HVec3 m_dir;			// search direction
	Maths::HF32 m_dist;			//the distance of v to the origin
	S32 m_indices[3];			//the index of vertices of the triangle

	friend class EPASimplified;
};



//template <class ConvexA, class ConvexB>
class EPASilhouette
{
public:

	EPASilhouette(): num_verts(0), num_facets(0), freeFacet(0), heap(MaxFacets), aBuf((Maths::HVec3*)aBufData),
			bBuf((Maths::HVec3*)bBufData), facetBuf((Facet2*)facetBufData)
	{
	}

	EPASilhouette(void* data, const U32 size): num_verts(0), num_facets(0), freeFacet(0), heap((SharedPhysics::BHStorage<Facet2>*)data, MaxFacets),
		aBuf((Maths::HVec3*)aBufData), bBuf((Maths::HVec3*)bBufData), facetBuf((Facet2*)facetBufData)
	{
		BIZ_ASSERT(size <= (MaxFacets * sizeof(Facet2)));
	}


#ifdef PLATFORM_SPU
	bool PenetrationDepth(const C_Convex& a, const C_Convex& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8& size,  Maths::HVec3& pa, Maths::HVec3& pb);
	bool PenetrationDepthMargin(const C_Convex& a, const C_Convex& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8& size,  Maths::HVec3& pa, Maths::HVec3& pb, const Maths::HF32 PCREF marginA, const Maths::HF32 PCREF marginB);
	SPUManager m_EpaManager;
#else
	template <class ConvexA, class ConvexB>
	bool PenetrationDepth(const ConvexA& a, const ConvexB& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8 size,  Maths::HVec3& pa, Maths::HVec3& pb);

	template <class ConvexA, class ConvexB>
	bool PenetrationDepthMargin(const ConvexA& a, const ConvexB& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8 size,  Maths::HVec3& pa, Maths::HVec3& pb, const Maths::HF32 PCREF marginA, const Maths::HF32 PCREF marginB);
#endif

	__forceinline Facet2* addFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2);
	__forceinline Facet2* addInitialFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2);
	//__forceinline Facet2* addFirstFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2);
	__forceinline bool originInTetrahedron(const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF p2, const Maths::HVec3 PCREF p3, const Maths::HVec3 PCREF p4);
	
	//void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF d);
	S32 num_verts;
	S32	num_facets;
	S32 freeFacet;

	Maths::HVec3* aBuf;
	Maths::HVec3* bBuf;
	Facet2* facetBuf;

	ALIGN16 char* aBufData[sizeof(Maths::HVec3) * MaxSupportPoints];
	ALIGN16 char* bBufData[sizeof(Maths::HVec3) * MaxSupportPoints];
	ALIGN16 char* facetBufData[sizeof(Facet2) * MaxFacets];

	SharedPhysics::BinaryHeapStorage<Facet2> heap;
	Edge stack[MaxFacets * 2];
	EdgeBuffer edgeBuffer;

};


class EPASimplified
{
	public:

		EPASimplified(): num_verts(0), num_facets(0), freeFacet(0), heap(MaxFacets), aBuf((Maths::HVec3*)aBufData),
			bBuf((Maths::HVec3*)bBufData), facetBuf((FacetSimplified*)facetBufData)
	{
	}

	EPASimplified(void* data, const U32 size): num_verts(0), num_facets(0), freeFacet(0), heap((FacetSimplified*)data, MaxFacets),
		aBuf((Maths::HVec3*)aBufData), bBuf((Maths::HVec3*)bBufData), facetBuf((FacetSimplified*)facetBufData)
	{
		BIZ_ASSERT(size >= (MaxFacets * sizeof(FacetSimplified)));
	}

	void Reset()
	{
		num_verts = 0;
		num_facets = 0;
		freeFacet = 0;
		heap.makeEmpty();
	}

#ifdef PLATFORM_SPU
	bool PenetrationDepth(const C_Convex& a, const C_Convex& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8& size,  Maths::HVec3& pa, Maths::HVec3& pb);
	bool PenetrationDepthMargin(const C_Convex& a, const C_Convex& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8& size,  Maths::HVec3& pa, Maths::HVec3& pb, const Maths::HF32 PCREF marginA, const Maths::HF32 PCREF marginB);
	SPUManager m_EpaManager;
#else
	template <class ConvexA, class ConvexB>
	bool PenetrationDepth(const ConvexA& a, const ConvexB& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8& size,  Maths::HVec3& pa, Maths::HVec3& pb);

	template <class ConvexA, class ConvexB>
	bool PenetrationDepthMargin(const ConvexA& a, const ConvexB& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8 PCREF size,  Maths::HVec3& pa, Maths::HVec3& pb, const Maths::HF32 PCREF marginA, const Maths::HF32 PCREF marginB);
#endif

	
	FacetSimplified* addFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2);

	FacetSimplified* addInitialFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2);

	bool originInTetrahedron(const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF p2, const Maths::HVec3 PCREF p3, const Maths::HVec3 PCREF p4);
	int num_verts;
	int	num_facets;
	int freeFacet;

	//Shuffled all this around a bit so that we can avoid having to construct the HVec3 arrays repeatedly
	Maths::HVec3* aBuf;
	Maths::HVec3* bBuf;
	FacetSimplified* facetBuf;

	ALIGN16 char* aBufData[sizeof(Maths::HVec3) * MaxSupportPoints];
	ALIGN16 char* bBufData[sizeof(Maths::HVec3) * MaxSupportPoints];
	ALIGN16 char* facetBufData[sizeof(FacetSimplified) * MaxFacets];


	/*Maths::HVec3  aBuf[MaxSupportPoints];
	Maths::HVec3  bBuf[MaxSupportPoints];

	FacetSimplified facetBuf[MaxFacets];*/
	SharedPhysics::BinaryHeap<FacetSimplified> heap;

};

//typedef EPASimplified EPA;
//typedef FacetSimplified EPAFacet;
//typedef EPASilhouette EPA;

class EPA : public EPASilhouette
{
public:
	EPA()
	{
	}

	EPA(void* data, const U32 size): EPASilhouette(data, size)
	{ 
	}
};
typedef Facet2 EPAFacet;


//class EPA : public EPASimplified
//{
//public:
//	EPA()
//	{
//	}
//
//	EPA(void* data, const U32 size): EPASimplified(data, size)
//	{
//	}
//};
//typedef FacetSimplified EPAFacet;
//
}

#endif

#include "gravity/narrowphase/epasilhouette.inl"
#include "gravity/narrowphase/epa.inl"
