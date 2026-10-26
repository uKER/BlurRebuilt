#include "stdafx.h"

#include "Dynamic/Particle/SpawnMesh.h"
#include "Dynamic/Particle/Random.h"
#include "Dynamic/Common.h"

#ifndef PLATFORM_SPU
#include <System/Debug/I_DebugRender.h>
#endif

namespace DynamicEffect
{
namespace Particle
{

#ifndef PLATFORM_SPU
C_SpawnMesh::C_SpawnMesh() : m_fArea(0.f)
{
	m_Vertices.Reserve(16);
	m_Triangles.Reserve(24);
};

C_SpawnMesh::~C_SpawnMesh()
{
};

// this function will calculate a quaternion from the direction parameter
S32 C_SpawnMesh::AddVertex(const Maths::HVec3& vPosition, const Maths::HVec3& vDirection)
{
	S32 index = m_Vertices.Size();

	Maths::HVec3 vNormal = Maths::Normalise(vDirection);
	
	m_Vertices.PushBack();
	Maths::Store(vPosition, &m_Vertices.Back().vPosition);
	//MathsStorage::Float4A16 rotation = GetQuaternionTo<MathsStorage::Float4A16>(vNormal);	
	MathsStorage::Float4A16 rotation = GetQuaternionFromTo<MathsStorage::Float4A16>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ(), vNormal);
	m_Vertices.Back().qRotation = (MathsStorage::Float4&)rotation;

	return index;
}

// this function accepts pre-calculated vertices
S32 C_SpawnMesh::AddVertex(const Vertex& vertex)
{
	S32 index = m_Vertices.Size();
	m_Vertices.PushBack(vertex);
	return index;
}

S32 C_SpawnMesh::AddTriangle(S16 i0, S16 i1, S16 i2)
{
	Triangle triangle;
	triangle.i0 = i0;
	triangle.i1 = i1;
	triangle.i2 = i2;

	return AddTriangle(triangle);
}

S32	C_SpawnMesh::AddTriangle(const Triangle& triangle)
{
	bool bSuccess = true;

	if(triangle.i0 < 0 || triangle.i0 >= m_Vertices.Size())
	{
		bSuccess = false;
	}
	if(triangle.i1 < 0 || triangle.i1 >= m_Vertices.Size())
	{
		bSuccess = false;
	}
	if(triangle.i2 < 0 || triangle.i2 >= m_Vertices.Size())
	{
		bSuccess = false;
	}

	if(triangle.i0 == triangle.i1 || triangle.i1 == triangle.i2 || triangle.i0 == triangle.i2)
	{
		bSuccess = false;
	}
	
	if(bSuccess)
	{
		S32 index = m_Triangles.Size();
		m_Triangles.PushBack(triangle);
		return index;
	}
	else
		return -1;
}

void C_SpawnMesh::CalculateArea()
{
	Maths::HF32 fTotalArea(Maths::Constants<Maths::HF32>::Zero());

	for(S32 i = 0; i < m_Triangles.Size(); ++i)
	{
		const Vertex& vert0 = m_Vertices[m_Triangles[i].i0];
		const Vertex& vert1 = m_Vertices[m_Triangles[i].i1];
		const Vertex& vert2 = m_Vertices[m_Triangles[i].i2];

		Maths::HVec3 vPosition0(Maths::Load<Maths::HVec3>(&vert0.vPosition));
		Maths::HVec3 vPosition1(Maths::Load<Maths::HVec3>(&vert1.vPosition));
		Maths::HVec3 vPosition2(Maths::Load<Maths::HVec3>(&vert2.vPosition));

		Maths::HVec3 vEdge0 = vPosition1 - vPosition0;
		Maths::HVec3 vEdge1 = vPosition2 - vPosition0;

		Maths::HVec3 vCross = vEdge0 ^ vEdge1;

		fTotalArea += Maths::Constants<Maths::HF32>::Half() * Maths::Length(vCross);
	}

	m_fArea = fTotalArea;
}
#endif

C_Node C_SpawnMesh::GetRandomNode(C_Random& random) const
{
	if(m_Triangles.Size() > 0)
		return GetRandomNode(random, random.Rand(m_Triangles.Size()));
	else
		return DynamicEffect::C_Node();
}

C_Node	C_SpawnMesh::GetRandomNode(C_Random& random, S32 iTriangleIndex) const
{
	const Triangle& triangle = m_Triangles[iTriangleIndex];
	const Vertex& vert0 = m_Vertices[triangle.i0];
	const Vertex& vert1 = m_Vertices[triangle.i1];
	const Vertex& vert2 = m_Vertices[triangle.i2];

	Maths::HVec3 vPosition0(Maths::Load<Maths::HVec3>(&vert0.vPosition));
	Maths::HVec3 vPosition1(Maths::Load<Maths::HVec3>(&vert1.vPosition));
	Maths::HVec3 vPosition2(Maths::Load<Maths::HVec3>(&vert2.vPosition));

	Maths::C_Quaternion qRotation0(Maths::Load<Maths::C_Quaternion>(&vert0.qRotation));
	Maths::C_Quaternion qRotation1(Maths::Load<Maths::C_Quaternion>(&vert1.qRotation));
	Maths::C_Quaternion qRotation2(Maths::Load<Maths::C_Quaternion>(&vert2.qRotation));

	Maths::HVec3 vBaryCentric(random.FRand(1.f), random.FRand(1.f), random.FRand(1.f));
	vBaryCentric.Normalise();
	vBaryCentric = vBaryCentric * vBaryCentric;
	
	Maths::HVec3 vPosition = vPosition0 * vBaryCentric.X() + vPosition1 * vBaryCentric.Y() + vPosition2 * vBaryCentric.Z();
	Maths::C_Quaternion qRotation = qRotation0 * vBaryCentric.X() + qRotation1 * vBaryCentric.Y() + qRotation2 * vBaryCentric.Z();
	qRotation.Normalise();

	// quaternion is in XMVECTOR format, so store it to a node directly
	C_SpawnMeshNode node;
	node.SetPosition(vPosition);
	Maths::Store(qRotation, &node.m_vRotation);

	return node;
}

#ifndef PLATFORM_SPU
void C_SpawnMesh::Display(const Maths::HMat43& matrix, DbUtil::I_DebugRender* pDebugRender) const
{
	if( pDebugRender == NULL )
	{
		pDebugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
	}

	// draw triangles
	for(S32 i = 0; i < m_Triangles.Size(); ++i)
	{
		const Vertex& vert0 = m_Vertices[m_Triangles[i].i0];
		const Vertex& vert1 = m_Vertices[m_Triangles[i].i1];
		const Vertex& vert2 = m_Vertices[m_Triangles[i].i2];

		Maths::HVec3 vPosition0(Maths::Load<Maths::HVec3>(&vert0.vPosition));
		Maths::HVec3 vPosition1(Maths::Load<Maths::HVec3>(&vert1.vPosition));
		Maths::HVec3 vPosition2(Maths::Load<Maths::HVec3>(&vert2.vPosition));

		vPosition0 = vPosition0 * matrix;
		vPosition1 = vPosition1 * matrix;
		vPosition2 = vPosition2 * matrix;
	
		pDebugRender->Draw3dTriangleOutline(vPosition0, vPosition1, vPosition2, Maths::RGBA(1.f, 1.f, 1.f, 1.f));
	}

	// draw normals
	for(S32 i = 0; i < m_Vertices.Size(); ++i)
	{
		Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&m_Vertices[i].vPosition));
		Maths::C_Quaternion qRotation(Maths::Load<Maths::C_Quaternion>(&m_Vertices[i].qRotation));
		
		Maths::HVec3 vNormal(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ());
		// quaternion is in XMVECTOR format so conjugate it here
#ifdef RENDER_DYNAMICEFFECT_USE_HVEC
		vNormal = vNormal * qRotation;
#else
		vNormal = vNormal * !qRotation;
#endif
		Maths::HVec3 vEnd = vPosition + vNormal * 0.5f;

		vPosition = vPosition * matrix;
		vEnd = vEnd * matrix;

		pDebugRender->Draw3dLine(vPosition, vEnd, Maths::RGBA(1.f, 0.f, 0.f, 1.f));
	}
}
#endif

}
}



