#ifndef OPTIMISEDAABBTREE_H
#define OPTIMISEDAABBTREE_H

#include "gravity/narrowphase/AxisBox.h"
#include "gravity/narrowphase/collisionresult.h"
#include <system/serialise.h>
#include "gravity/narrowphase/LineCollisionCache.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/sharedphysics/SPU_Memory.h"
//#include "gravity/sharedphysics/safestack.h"
//#include "gravity/narrowphase/Primitive.h"
//#include "gravity/narrowphase/CollisionManager.h"
//#include "Shapes.h"


namespace SharedPhysics
{
	template <bool bFixedSize> class C_SafeStack;
}

namespace Gravity
{
	extern Debug::Logger& g_Log;
}

namespace Collision
{

class C_Shape;
class C_Ray;
template <typename ShapeType>
class C_CollisionShape
{
public:
	C_CRAxisBox m_bbox;
	ShapeType* m_pShape;
};

class OldAABBTreeTriangle
{
public:
	U32 m_IndexA, m_IndexB, m_IndexC;
	//U32 m_NormalIndex;
	MathsStorage::HenDN3 m_SurfaceNormal;
	U32 m_ActiveEdgeDataIndex;
	//U8 m_ActiveEdgeFlags;
	

	template <typename Serial>
	void Serialise(Serial& s);

	//template <typename Serial>
	void DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s);

	NOALIAS FORCE_INLINE bool ActiveEdge(const U32 edge) const
	{
		return (m_ActiveEdgeDataIndex & (1 << edge)) != 0;
	}

	NOALIAS FORCE_INLINE U32 ActiveEdgeDataIndex() const
	{
		return m_ActiveEdgeDataIndex >> 3;
	}
};

struct AABBTreeTriangle
{
	MathsStorage::HenDN3 m_SurfaceNormal;
	U32 m_ActiveEdgeDataIndex;
	U8 m_IndexA, m_IndexB, m_IndexC;
	

	template <typename Serial>
	void Serialise(Serial& s);

	//template <typename Serial>
	void DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s);

	NOALIAS FORCE_INLINE bool ActiveEdge(const U32 edge) const
	{
		return (m_ActiveEdgeDataIndex & (1 << edge)) != 0;
	}

	NOALIAS FORCE_INLINE U32 ActiveEdgeDataIndex() const
	{
		return m_ActiveEdgeDataIndex >> 3;
	}
};

class C_ActiveEdgeNormals
{
public:
	MathsStorage::HenDN3 m_EdgeNormA, m_EdgeNormB, m_EdgeNormC;

	template <typename Serial>
	void Serialise(Serial& s)
	{
		s << m_EdgeNormA.v << m_EdgeNormB.v << m_EdgeNormC.v;
	}

	void DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s)
	{
		s >> m_EdgeNormA.v >> m_EdgeNormB.v >> m_EdgeNormC.v;
	}
};

struct C_TriangleHeader
{
	U32 m_CollisionFlags;
	U32 m_UserDataFlags;
	//U16 m_MaterialIndex;
	U32 m_EffectMaterialIndex;
	U16 m_PhysicsMaterialIndex;
	U8 m_NumTriangles;

	template <typename Serial>
	void Serialise(Serial& s);

	void DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s);
};

class C_TriangleStream
{
public:
	U8* m_pStream;
	bool m_ExternallyOwned;

	C_TriangleStream() : m_pStream(NULL), m_ExternallyOwned(false)
	{
	}

	C_TriangleStream(U32 size)
	{
		m_pStream = (size > 0) ? new U8[size] : NULL;
	}

	C_TriangleStream(const C_TriangleStream& stream)
	{
		m_pStream = stream.m_pStream;
		m_ExternallyOwned = true;
	}
	
	C_TriangleStream& operator = (const C_TriangleStream& stream)
	{
		BIZ_ASSERT(m_ExternallyOwned || (m_pStream == NULL));
		m_pStream = stream.m_pStream;
		m_ExternallyOwned = true;
		return *this;
	}

	~C_TriangleStream()
	{
		//LOGINFO(Gravity::g_Log, "Destructing triangle stream");
		if (!m_ExternallyOwned)
		{
			//LOGINFO(Gravity::g_Log, "Actually deleting triangle stream");
			delete [] m_pStream;
		}
	}

	void Resize(U32 size)
	{
		BIZ_ASSERT(!m_ExternallyOwned);
		delete [] m_pStream;
		m_pStream = (size > 0) ? new U8[size] : NULL;
	}

	NOALIAS FORCE_INLINE C_TriangleHeader* GetHeader(U32 offset) const
	{
		return (C_TriangleHeader*)(m_pStream + offset);
	}

	NOALIAS FORCE_INLINE AABBTreeTriangle* GetTriangles(U32 offset) const
	{
		return (AABBTreeTriangle*)(m_pStream + offset);
	}

	/*NOALIAS FORCE_INLINE C_TriangleHeader* GetHeader(U32 offset)
	{
		return (C_TriangleHeader*)(m_pStream + offset);
	}

	NOALIAS FORCE_INLINE AABBTreeTriangle* GetTriangles(U32 offset)
	{
		return (AABBTreeTriangle*)(m_pStream + offset);
	}*/

	template <typename Serial>
	void Serialise(Serial& serial, U32 size);

	void DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s, U32 size);

};

/*class C_TriangleHeaderIterator
{
	C_TriangleStream& m_Stream;
	U32 m_StreamSize;
	U32 m_Offset;
public:
	C_TriangleHeaderIterator(C_TriangleStream& stream, const U32 streamSize) : m_Stream(stream), m_StreamSize(streamSize), m_Offset(0)
	{
	}

	bool HasNextHeader()
	{
		return m_Offset < m_StreamSize;
	}

	C_TriangleHeader* NextHeader()
	{
		C_TriangleHeader* header = m_Stream.GetHeader(m_Offset);
		m_Offset += sizeof(C_TriangleHeader) + header->m_NumTriangles * sizeof(AABBTreeTriangle);
		return header;
	}
};*/

class C_TriangleStreamIterator
{
	C_TriangleStream& m_Stream;
	C_TriangleHeader* m_pHeader;
	U32 m_StreamSize;
	U32 m_Offset;
	U32 m_Triangle;
public:
	C_TriangleStreamIterator(C_TriangleStream& stream, const U32 streamSize) : m_Stream(stream), m_StreamSize(streamSize), m_Offset(0), m_Triangle(0)
	{
		m_pHeader = m_Stream.GetHeader(m_Offset);
		m_Offset += sizeof(C_TriangleHeader);
	}

	bool HasNext()
	{
		return m_Offset < m_StreamSize;
	}

	C_TriangleHeader* CurrentHeader()
	{
		return m_pHeader;
	}

	C_TriangleHeader* NextHeader()
	{
		//Jumps forward to next header
		m_Offset = ((U8*)m_pHeader) - m_Stream.m_pStream;
		m_Offset += sizeof(C_TriangleHeader) + m_pHeader->m_NumTriangles * sizeof(AABBTreeTriangle);
		m_Triangle = 0;
		if(m_Offset < m_StreamSize)
		{
			m_pHeader = m_Stream.GetHeader(m_Offset);
			m_Offset += sizeof(C_TriangleHeader);// + header->m_NumTriangles * sizeof(AABBTreeTriangle);
			return m_pHeader;
		}
		m_pHeader = NULL;
		return NULL;
	}

	AABBTreeTriangle* NextTriangle()
	{
		if(!m_pHeader)
			return NULL;
		if(m_Triangle == m_pHeader->m_NumTriangles)
		{
			//Go to next header...
			m_pHeader=  m_Stream.GetHeader(m_Offset);
			m_Offset += sizeof(C_TriangleHeader);
		}
		m_Triangle++;
		AABBTreeTriangle* tri = m_Stream.GetTriangles(m_Offset);
		m_Offset += sizeof(AABBTreeTriangle);
		return tri;
	}
};


class ALIGN16 OptimisedAABBNode
{
public:
	//C_AxisBoxMinMax m_BoundingBox;
	C_QuantizedAxisBoxMinMax m_BoundingBox;

	union
	{
		U32 m_TriangleStartIndex;
		U32 m_RightChildOffset;
	};
	U32 m_VerticeStartIndex;
	U32 m_TriangleSizeIndices;
	U32 m_TriangleStreamLength;
	U32 m_NumVertices;

	void DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s);
};

class C_LeafData
{
	Maths::HVec3 m_Low, m_High;
	AABBTreeTriangle* m_pTriangles;
	MathsStorage::Fixed3_S16<14>* m_pVertices;
	C_ActiveEdgeNormals* m_pActiveEdgeNormals;
	U32 m_StartTriangleIndex;
	U32 m_StartVertexIndex;
	U32 m_StartEdgeIndex;
};

class OptimisedAABBTree
{
public:
	Maths::HVec3 m_Low, m_High;
	//Maths::HVec3 m_Range;
	MathsStorage::Fixed3_S16<14>* m_pVertices;
	MathsStorage::HenDN3 * m_pNormals;
	C_TriangleStream m_Triangles;
	OptimisedAABBNode* m_pNodes;
	C_ActiveEdgeNormals* m_pActiveEdgeNormals;
	U32 m_NumVertices, m_NumTriangles, m_NumNodes, m_NumNormals, m_NumActiveEdges;
	bool m_ExternallyOwned;


	OptimisedAABBTree() : m_Low(Maths::UNDEFINED), m_High(Maths::UNDEFINED), m_pVertices(NULL), 
		m_pNormals(NULL), m_pNodes(NULL), m_pActiveEdgeNormals(NULL), m_NumVertices(0),
		m_NumTriangles(0), m_NumNodes(0), m_NumNormals(0), m_NumActiveEdges(0), m_ExternallyOwned(false)
	{
	}
	
	OptimisedAABBTree(const OptimisedAABBTree& tree) : m_Low(tree.m_Low), m_High(tree.m_High)
	{
		m_pVertices = tree.m_pVertices;
		m_pNormals = tree.m_pNormals;
		m_Triangles = tree.m_Triangles;
		m_pNodes = tree.m_pNodes;
		m_NumVertices = tree.m_NumVertices;
		m_NumTriangles = tree.m_NumTriangles;
		m_NumNodes = tree.m_NumNodes;
		m_NumNormals = tree.m_NumNormals;
		m_NumActiveEdges = tree.m_NumActiveEdges;
		m_pActiveEdgeNormals = tree.m_pActiveEdgeNormals;
		m_ExternallyOwned = true;
	}
	
	OptimisedAABBTree& operator = (const OptimisedAABBTree& tree)
	{
		m_Low = tree.m_Low;
		m_High = tree.m_High;
		m_pVertices = tree.m_pVertices;
		m_pNormals = tree.m_pNormals;
		m_Triangles = tree.m_Triangles;
		m_pNodes = tree.m_pNodes;
		m_NumVertices = tree.m_NumVertices;
		m_NumTriangles = tree.m_NumTriangles;
		m_NumNodes = tree.m_NumNodes;
		m_NumNormals = tree.m_NumNormals;
		m_NumActiveEdges = tree.m_NumActiveEdges;
		m_pActiveEdgeNormals = tree.m_pActiveEdgeNormals;
		m_ExternallyOwned = true;

		return *this;
	}

	~OptimisedAABBTree()
	{
		if(!m_ExternallyOwned)
		{
			if(m_pVertices)
				delete [] m_pVertices;
			if(m_pNormals)
				delete [] m_pNormals;
			if(m_pNodes)
				//delete [] m_pNodes;
				BizAlignedFree(m_pNodes);
			if(m_pActiveEdgeNormals)
				delete [] m_pActiveEdgeNormals;
		}

		m_pVertices = NULL;
		m_pNormals = NULL;
		m_pNodes = NULL;
		m_pActiveEdgeNormals = NULL;
	}

	const Maths::HVec3 GetVertex(const U32& index, const Maths::HVec3 PCREF low, const Maths::HVec3 PCREF high) const
	{
		Spu::MemBaseMarker marker;
		const Maths::HVec3 range(high - low);
		MathsStorage::Fixed3_S16<14>* pF = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnaligned(m_pVertices + index, sizeof(MathsStorage::Fixed3_S16<14>));
		const Maths::HVec3 localV(pF);
		return (localV * range + low);
	}

	/*NOALIAS FORCE_INLINE MathsStorage::Fixed3_S16<14>* GetVertices(const U32 startIndex, const U32 numVertices)
	{
		MathsStorage::Fixed3_S16<14>* pVertices=(MathsStorage::Fixed3_S16<14>)Spu::FetchAllocUnaligned(m_pVertices + startIndex, sizeof(MathsStorage::Fixed3_S16<14>)*numVertices);
		return pVertices;
	}*/


	bool CheckNormals();

	U32 DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s);
	//void DeSerialise(Deserialiser& s);

#ifndef PLATFORM_SPU
	template <int MaxPatches, int MaxContacts, bool (*TriCollisions)(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const  tris, const U32 numTris, 
		const U32 triangleStartIndex, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)>
	bool Collide(Collision::C_CollisionObject * obj, 
				 Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, 
				 SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);
#else
	template <int MaxPatches, int MaxContacts>
	bool Collide(C_CollisionObject * obj, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_SafeStack<true>& stack,
		bool (*TriCollisions)(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const  tris, const U32 numTris, 
		const U32 triangleStartIndex, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high),
		SharedPhysics::C_SmallSPUCache& spuCache);


#endif

	bool Collide(C_CollisionObject * obj, SharedPhysics::C_SafeStack<true>& stack, Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, const Maths::HVec3 PCREF r,
		Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, U32& physicsMaterial, U32& effectMaterial,
		bool (*TriCollisions)(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const  tris, const U32 numTris, const U32 triangleStartIndex, 
		const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, const Maths::HVec3& low, const Maths::HVec3& high),
		SharedPhysics::C_SmallSPUCache& spuCache);

	bool Collide(C_CollisionObject * obj, SharedPhysics::C_SafeStack<true>& stack, Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, const Maths::HVec3 PCREF r,
		Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, U32& physicsMaterial, U32& effectMaterial, Collision::EPA& epa,
		bool (*TriCollisions)(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const  tris, const U32 numTris, const U32 triangleStartIndex, 
		const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, Collision::EPA& epa, const Maths::HVec3& low, const Maths::HVec3& high),
		SharedPhysics::C_SmallSPUCache& spuCache);

	//template <int MaxPatches, int MaxContacts, bool (*TriCollisions)(const C_Shape* const __restrict pShape, const MathsStorage::Float3* const __restrict verts, const MathsStorage::Float3 * const __restrict normals, const Collision::AABBTreeTriangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 materialIndex)>
	//bool Collide(const Collision::C_ShapeWrapper *__restrict const obj, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_SafeStack<true>& stack);

	
	template<bool(*TriCollisions)(const MathsStorage::Fixed3_S16<14>* const verts,const MathsStorage::HenDN3* const __restrict normals, const Collision::AABBTreeTriangle* const __restrict tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, Collision::C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high)>
		bool RayTracing(Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);

	template<bool (*TriCollisions)(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3* const normals, const Collision::AABBTreeTriangle* const tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, Collision::C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, const Maths::HVec3& low, const Maths::HVec3& high)>
		bool RayTracing(Collision::C_TransparentRay &ray, Collision::C_CollisionFilter* pRayFilter, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);

	void FillLineCollisionCache(Collision::C_CRAxisBox& box, Collision::C_LineCollisionCache& cache, 
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter, 
		Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF maxCosAngle, SharedPhysics::C_SmallSPUCache& spuCache);

};


class C_LegacyTriangleStream
{
public:
	U8* m_pStream;

	C_LegacyTriangleStream() : m_pStream(NULL)
	{
	}

	C_LegacyTriangleStream(U32 size)
	{
		m_pStream = new U8[size];
	}

	~C_LegacyTriangleStream()
	{
		//LOGINFO(Gravity::g_Log, "Destructing triangle stream");
		if(m_pStream)
		{
			//LOGINFO(Gravity::g_Log, "Actually deleting triangle stream");
			delete [] m_pStream;
		}
		m_pStream = NULL;
	}

	NOALIAS FORCE_INLINE C_TriangleHeader* GetHeader(U32 offset) const
	{
		return (C_TriangleHeader*)(m_pStream + offset);
	}

	NOALIAS FORCE_INLINE OldAABBTreeTriangle* GetTriangles(U32 offset) const
	{
		return (OldAABBTreeTriangle*)(m_pStream + offset);
	}

	void Resize(U32 size)
	{
		if(m_pStream)
			delete [] m_pStream;
		m_pStream = new U8[size];
	}

	//NOALIAS FORCE_INLINE C_TriangleHeader* GetHeader(U32 offset)
	//{
	//	return (C_TriangleHeader*)(m_pStream + offset);
	//}

	//NOALIAS FORCE_INLINE AABBTreeTriangle* GetTriangles(U32 offset)
	//{
	//	return (OldAABBTreeTriangle*)(m_pStream + offset);
	//}
};

#ifndef PLATFORM_SPU
//Class used to load in old data formats!!!
//Will be slow, but hopefully work!!!!
class LegacyAABBTree
{
	Maths::HVec3 m_Low, m_High;
	Maths::HVec3 m_Range;
	SharedPhysics::SortedArray<MathsStorage::Fixed3_S16<14> > m_Vertices;
	MathsStorage::Float3* m_pUncompressedVertices;
	MathsStorage::HenDN3 * m_pNormals;
	C_LegacyTriangleStream m_Triangles;
	OptimisedAABBNode* m_pNodes;
	C_ActiveEdgeNormals* m_pActiveEdgeNormals;
	U32 m_NumVertices, m_NumTriangles, m_NumNodes, m_NumNormals, m_NumActiveEdges;

public:


	LegacyAABBTree() : m_Low(Maths::UNDEFINED), m_High(Maths::UNDEFINED), m_Range(Maths::UNDEFINED), m_pUncompressedVertices(NULL), 
		m_pNormals(NULL), m_pNodes(NULL), m_pActiveEdgeNormals(NULL), m_NumVertices(0),
		m_NumTriangles(0), m_NumNodes(0), m_NumNormals(0), m_NumActiveEdges(0)
	{
	}

	~LegacyAABBTree()
	{
		delete [] m_pUncompressedVertices;
		delete [] m_pNormals;
		delete [] m_pNodes;
		delete [] m_pActiveEdgeNormals;
		m_pUncompressedVertices = NULL;
		m_pNormals = NULL;
		m_pNodes = NULL;
		m_pActiveEdgeNormals = NULL;
	}

	void DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s);

	U32 LoadMesh(Collision::OptimisedAABBTree* pTree);

	void RemapTriangleVertices(Collision::OldAABBTreeTriangle& tri, U32 startIndex, SharedPhysics::SortedArray<MathsStorage::Float3>& remappedVertices);


};
#endif


//NB - instanced mesh objects are done by maintaining a pointer to the aabb tree we are interested in.
//We do collision detection between these in the local space of the instanced aabb tree and transform 
//results back into world space. NB - this means it is necessary to make a local space proxy for the object

class C_InstancedAABBTree
{
public:
	Maths::HMat43 m_PosRot;
	OptimisedAABBTree* m_pTree;

	C_InstancedAABBTree(OptimisedAABBTree* pTree) : m_pTree(pTree)
	{
	}

	~C_InstancedAABBTree()
	{
		//Do nothing - m_pTree is allocated/freed elsewhere
	}

	#if 0 //defined( PLATFORM_SPU )
    C_InstancedAABBTree( const Spu::C_ConstructVirtualTable& cvt )
        : m_PosRot( Maths::UNDEFINED )
    {
    }
    #endif
    
	template <typename Serial>
	void DeSerialise(Serial& s);

#ifndef PLATFORM_SPU
	template <int MaxPatches, int MaxContacts, typename CollisionProxy, bool (*TriCollisions)(CollisionProxy& proxy, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const tris, const U32 numTris, 
		const U32 triangleStartIndex, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)>
	bool Collide(C_CollisionObject *obj, CollisionProxy& proxy, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_SafeStack<true>& stack);
#else
	template <int MaxPatches, int MaxContacts, typename CollisionProxy>
	bool Collide(C_CollisionObject * obj, CollisionProxy& proxy, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_SafeStack<true>& stack,
		bool (*TriCollisions)(CollisionProxy& proxy, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const  tris, const U32 numTris, 
		const U32 triangleStartIndex, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high));
#endif

	template<bool(*TriCollisions)(const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3* const normals, const Collision::AABBTreeTriangle* const  tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, Collision::C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high)>
		bool RayTracing(Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);

	template<bool(*TriCollisions)(const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3* const normals, const Collision::AABBTreeTriangle* const  tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, Collision::C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, const Maths::HVec3& low, const Maths::HVec3& high,
		const Maths::HMat43& matrix)>
		bool RayTracing(Collision::C_TransparentRay& ray, Collision::C_CollisionFilter* pRayFilter, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);

	void FillLineCollisionCache(Collision::C_CRAxisBox& box, Collision::C_LineCollisionCache& cache, 
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter, 
		Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, const Maths::C_Normal PCREF negativeNormal,
					const Maths::HF32 PCREF maxCosAngle, SharedPhysics::C_SmallSPUCache& spuCache);
};

template <typename Serial>
void AABBTreeTriangle::Serialise(Serial& s)
{
	//assert(m_NormalIndex < 65536);
	//U32 normIndex = m_NormalIndex;
	s << m_IndexA << m_IndexB << m_IndexC << m_SurfaceNormal.v << m_ActiveEdgeDataIndex;
	//	<< m_EdgeNormA.v << m_EdgeNormB.v << m_EdgeNormC.v;
}

template <typename Serial>
void C_TriangleHeader::Serialise(Serial& s)
{
	assert(m_NumTriangles != 0);
	s << m_CollisionFlags << m_PhysicsMaterialIndex << m_EffectMaterialIndex << m_UserDataFlags << m_NumTriangles;
}

template <typename Serial>
void C_TriangleStream::Serialise(Serial& serial, U32 size)
{
	serial.Write(m_pStream, size);
}

};

//#include "gravity/narrowphase/OptimisedAABBTree.inl"

#endif
