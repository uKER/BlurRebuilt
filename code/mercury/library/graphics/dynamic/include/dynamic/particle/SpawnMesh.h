#ifndef RENDER_DYNAMICEFFECT_PARTICLE_SPAWNMESH_H
#define RENDER_DYNAMICEFFECT_PARTICLE_SPAWNMESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Node.h"

#ifndef PLATFORM_SPU
namespace DbUtil
{
	class I_DebugRender;
}
#endif

namespace DynamicEffect
{
namespace Particle
{
class C_Random;

struct Vertex
{
	MathsStorage::Float3	vPosition;
	MathsStorage::Float4	qRotation;
};

struct Triangle
{
	S16	i0;
	S16	i1;
	S16	i2;
};

// this allows C_SpawnMesh to modify C_Node members directly
class C_SpawnMeshNode : public C_Node
{
public:
	friend class C_SpawnMesh;
};

// stores triangles which emitters can use to spawn particles from
class ALIGN16 C_SpawnMesh
{
public:
#ifndef PLATFORM_SPU
	C_SpawnMesh();
	~C_SpawnMesh();

	S32		AddVertex(const Maths::HVec3& vPosition, const Maths::HVec3& vDirection);
	S32		AddVertex(const Vertex& vertex);
	S32		AddTriangle(const Triangle& triangle);
	S32		AddTriangle(S16 i0, S16 i1, S16 i2);

	void	SetNumReservedVertices(const S32 nVertices);
	void	SetNumReservedTriangles(const S32 nTriangles);

	void	CalculateArea();
	
	void	Display(const Maths::HMat43& matrix = Maths::HMat43(), DbUtil::I_DebugRender* pDebugRender = NULL) const;
#endif

	void	SpuFetch() const;
	void	SpuFetchAsync(U32 tag) const;

	C_Node	GetRandomNode(C_Random& random) const;
	C_Node	GetRandomNode(C_Random& random, S32 iTriangleIndex) const;

	S32		GetNumVertices() const;
	S32		GetNumTriangles() const;

	F32		GetArea() const;

private:
#ifdef PLATFORM_SPU
	mutable Btl::Vector<Vertex>		m_Vertices;
	mutable Btl::Vector<Triangle>	m_Triangles;
#else
	Btl::Vector<Vertex>		m_Vertices;
	Btl::Vector<Triangle>	m_Triangles;
#endif

	F32						m_fArea;
};

INLINE S32 C_SpawnMesh::GetNumVertices() const
{
	return m_Vertices.Size();
}

INLINE S32 C_SpawnMesh::GetNumTriangles() const
{
	return m_Triangles.Size();
}

INLINE void	C_SpawnMesh::SpuFetch() const
{
	#ifdef PLATFORM_SPU
	Btl::SpuFetch(m_Vertices);
	Btl::SpuFetch(m_Triangles);
	#endif
}

INLINE void	C_SpawnMesh::SpuFetchAsync(U32 tag) const
{
#ifdef PLATFORM_SPU
	Btl::SpuFetchAsync(m_Vertices, tag);
	Btl::SpuFetchAsync(m_Triangles, tag);
#endif
}

#ifndef PLATFORM_SPU
INLINE void C_SpawnMesh::SetNumReservedVertices(const S32 nVertices)
{
	if(nVertices > m_Vertices.Size())
		m_Vertices.Reserve(nVertices);
}

INLINE void C_SpawnMesh::SetNumReservedTriangles(const S32 nTriangles)
{
	if(nTriangles > m_Triangles.Size())
		m_Triangles.Reserve(nTriangles);
}
#endif

INLINE F32 C_SpawnMesh::GetArea() const
{
	return m_fArea;
}

} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_SPAWNMESH_H
