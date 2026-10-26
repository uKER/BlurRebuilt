#include "stdafx.h"

#include <EditorApp/Editor/Primitives.h>

namespace Editor
{
namespace Primitive
{

struct Vertex
{
	F32 x, y, z;
};

inline void SetVertex(Vertex *pVertex,F32 x, F32 y, F32 z)
{
	pVertex->x = x;
	pVertex->y = y;
	pVertex->z = z;
}

PrimitiveResourcePtr Primitive::Axis(F32 sizeX, F32 sizeY, F32 sizeZ)
{
	const U32 nVertices = 6;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_LineList);

	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	
	sizeX *=0.5;
	sizeY *=0.5;
	sizeZ *=0.5;

	const Vertex v[6] = 
	{
		{ -sizeX,  0.f,  0.f },
		{  sizeX,  0.f,  0.f },
		{  0.f, -sizeY,  0.f },
		{  0.f,  sizeY,  0.f },
		{  0.f,  0.f, -sizeZ },
		{  0.f,  0.f,  sizeZ }
	};

	(*pVertices++) = v[0]; (*pVertices++) = v[1]; 
	(*pVertices++) = v[2]; (*pVertices++) = v[3];
	(*pVertices++) = v[4]; (*pVertices++) = v[5];

	return pMeshCluster;

}

PrimitiveResourcePtr Primitive::AxisPositive(F32 sizeX, F32 sizeY, F32 sizeZ)
{
	const U32 nVertices = 6;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_LineList);

	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	
	sizeX *=0.5;
	sizeY *=0.5;
	sizeZ *=0.5;

	const Vertex v[4] = 
	{
		{  0.f,   0.f,   0.f },
		{  sizeX,  0.f,  0.f },
		{  0.f,  sizeY,  0.f },
		{  0.f,  0.f,  sizeZ }
	};

	(*pVertices++) = v[0]; (*pVertices++) = v[1]; 
	(*pVertices++) = v[0]; (*pVertices++) = v[2];
	(*pVertices++) = v[0]; (*pVertices++) = v[3];

	return pMeshCluster;

}

PrimitiveResourcePtr Primitive::Locator(F32 sizeX, F32 sizeY, F32 sizeZ)
{
	const U32 nVertices = 18;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_LineList);

	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	
	sizeX *=0.5;
	sizeY *=0.5;
	sizeZ *=0.5;

	const Vertex v[11] = 
	{
		{ -0.5f*sizeX,  0.f,  0.f },
		{  sizeX,  0.f,  0.f },
		{  0.f, -0.5f*sizeY,  0.f },
		{  0.f,  0.8f*sizeY,  0.f },
		{  0.f,  0.f, -0.5f*sizeZ },
		{  0.f,  0.f,  1.2f*sizeZ },
		
		{  -0.1f*sizeX,  0.f,  sizeZ },
		{   0.1f*sizeX,  0.f,  sizeZ },
		
		{  -0.1f*sizeX,  0.9f*sizeY,  0.f },
		{   0.1f*sizeX,  0.9f*sizeY,  0.f },
		{  0.f,  1.f*sizeY,  0.f },
	};

	(*pVertices++) = v[0]; (*pVertices++) = v[1]; 
	(*pVertices++) = v[2]; (*pVertices++) = v[3];
	(*pVertices++) = v[4]; (*pVertices++) = v[5];
	(*pVertices++) = v[5]; (*pVertices++) = v[6];
	(*pVertices++) = v[5]; (*pVertices++) = v[7];
	(*pVertices++) = v[3]; (*pVertices++) = v[8];
	(*pVertices++) = v[3]; (*pVertices++) = v[9];
	(*pVertices++) = v[8]; (*pVertices++) = v[10];
	(*pVertices++) = v[9]; (*pVertices++) = v[10];

	return pMeshCluster;

}

PrimitiveResourcePtr Primitive::Box(F32 sizeX, F32 sizeY, F32 sizeZ)
{
	const U32 nVertices = 36;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_TriList);

	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	
	sizeX *=0.5;
	sizeY *=0.5;
	sizeZ *=0.5;

	const Vertex v[8] = 
	{
		{  sizeX,  sizeY,  sizeZ },
		{ -sizeX,  sizeY,  sizeZ },
		{ -sizeX, -sizeY,  sizeZ },
		{  sizeX, -sizeY,  sizeZ },
		{ -sizeX,  sizeY, -sizeZ },
		{  sizeX,  sizeY, -sizeZ },
		{  sizeX, -sizeY, -sizeZ },
		{ -sizeX, -sizeY, -sizeZ }
	};

	(*pVertices++) = v[0]; (*pVertices++) = v[1]; (*pVertices++) = v[2];
	(*pVertices++) = v[0]; (*pVertices++) = v[2]; (*pVertices++) = v[3];
	(*pVertices++) = v[1]; (*pVertices++) = v[4]; (*pVertices++) = v[7];
	(*pVertices++) = v[1]; (*pVertices++) = v[7]; (*pVertices++) = v[2];
	(*pVertices++) = v[4]; (*pVertices++) = v[5]; (*pVertices++) = v[6];
	(*pVertices++) = v[4]; (*pVertices++) = v[6]; (*pVertices++) = v[7];
	(*pVertices++) = v[5]; (*pVertices++) = v[0]; (*pVertices++) = v[3];
	(*pVertices++) = v[5]; (*pVertices++) = v[3]; (*pVertices++) = v[6];
	(*pVertices++) = v[5]; (*pVertices++) = v[4]; (*pVertices++) = v[1];
	(*pVertices++) = v[5]; (*pVertices++) = v[1]; (*pVertices++) = v[0];
	(*pVertices++) = v[3]; (*pVertices++) = v[2]; (*pVertices++) = v[7];
	(*pVertices++) = v[3]; (*pVertices++) = v[7]; (*pVertices++) = v[6];

	return pMeshCluster;
}

PrimitiveResourcePtr Primitive::BoxOutline(F32 sizeX, F32 sizeY, F32 sizeZ)
{
	const U32 nVertices = 24;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_LineList);

	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	
	sizeX *=0.5;
	sizeY *=0.5;
	sizeZ *=0.5;

	const Vertex v[8] = 
	{
		{  sizeX,  sizeY,  sizeZ },
		{ -sizeX,  sizeY,  sizeZ },
		{ -sizeX, -sizeY,  sizeZ },
		{  sizeX, -sizeY,  sizeZ },
		{ -sizeX,  sizeY, -sizeZ },
		{  sizeX,  sizeY, -sizeZ },
		{  sizeX, -sizeY, -sizeZ },
		{ -sizeX, -sizeY, -sizeZ }
	};

	(*pVertices++) = v[0]; (*pVertices++) = v[1]; (*pVertices++) = v[1]; (*pVertices++) = v[2];
	(*pVertices++) = v[2]; (*pVertices++) = v[3]; (*pVertices++) = v[3]; (*pVertices++) = v[0];
	(*pVertices++) = v[4]; (*pVertices++) = v[5]; (*pVertices++) = v[5]; (*pVertices++) = v[6];
	(*pVertices++) = v[6]; (*pVertices++) = v[7]; (*pVertices++) = v[7]; (*pVertices++) = v[4];
	(*pVertices++) = v[0]; (*pVertices++) = v[5]; (*pVertices++) = v[1]; (*pVertices++) = v[4];
	(*pVertices++) = v[2]; (*pVertices++) = v[7]; (*pVertices++) = v[3]; (*pVertices++) = v[6];

	return pMeshCluster;
}

PrimitiveResourcePtr Primitive::Rectangle(F32 width, F32 height)
{
	
	const U32 nVertices = 8;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[8];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_LineList);

	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);

	const Vertex v[4] = 
	{
		{ 0.f, 0.f, 0.f },
		{ 0.f, height, 0.f },
		{ width, height, 0.f },
		{ width, 0.f, 0.f}
	};

	(*pVertices++) = v[0]; (*pVertices++) = v[1]; (*pVertices++) = v[1]; (*pVertices++) = v[2];
	(*pVertices++) = v[2]; (*pVertices++) = v[3]; (*pVertices++) = v[3]; (*pVertices++) = v[0];
	
	return pMeshCluster;  
}



PrimitiveResourcePtr Primitive::Sphere(F32 radius, U32 tessU, U32 tessV)
{
	const U32 nVertices = 2*(tessU)*(tessV)+2;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_TriStrip);

	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	
	F32 deltaU = 2.f*Maths::Constants<F32>::Pi()/tessU;
	F32 deltaV = Maths::Constants<F32>::Pi()/tessV;

	Ut::UtVector< Ut::DynamicVector<Vertex> > vertexCache;
	vertexCache.Resize(tessU+1);
	memset(&vertexCache[0], 0.f, sizeof(Vertex)*(tessU+1));

	Vertex v;
	F32 currentV = -0.5*Maths::Constants<F32>::Pi();
	for (U32 j=0; j<tessV+1; ++j)
	{
		F32 currentU = 0.f;
		const F32 cV = Maths::Cos(currentV);
		const F32 sV = Maths::Sin(currentV);

		F32 z = radius*sV;

		for (U32 i=0; i<tessU; ++i)
		{
			F32 cU = Maths::Cos(currentU)*cV;
			F32 sU = Maths::Sin(currentU)*cV;

			//initial degenerate
			if (j>0)
				(*pVertices++) = vertexCache[i];
			if (j==1 && !i)
				(*pVertices++) = vertexCache[i];

			v.x = radius*cU;
			v.y = radius*sU;
			v.z = z;

			if (j)
				(*pVertices++) = v;

			vertexCache[i] = v;
			currentU += deltaU;
		}
		currentV += deltaV;
	}

	//final degenarate
	(*pVertices++) = v;

	U32 nV = pVertices - (Vertex*)pMeshCluster->GetVertexDataElement(AftkRender::E_ElementType_Position);
	assert(nV == nVertices);
	return pMeshCluster;
}

PrimitiveResourcePtr SphereWireframe(F32 radius, U32 tessU, U32 tessV)
{
	const U32 nVertices = 1+(tessU+1)*(tessV-1) + (tessU-1)*(1+tessV);
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_LineStrip);

	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	
	F32 deltaU = 2.f*Maths::Constants<F32>::Pi()/tessU;
	F32 deltaV = Maths::Constants<F32>::Pi()/tessV;

	{
		const Vertex v = {0.f, radius, 0.f};
		(*pVertices++) = v;
	}

	for (U32 j=1; j<tessV; ++j)
	{
		const F32 cV = Maths::Cos(j*deltaV);
		const F32 sV = Maths::Sin(j*deltaV);

		const F32 y = radius*cV;

		for (U32 i=0; i<tessU+1; ++i)
		{
			const F32 cU = Maths::Cos(i*deltaU)*sV;
			const F32 sU = Maths::Sin(i*deltaU)*sV;

			const Vertex v = {radius*cU, y, radius*sU};
			(*pVertices++) = v;

		}
	}

	for (U32 i=1; i<tessU; ++i)
	{
		{
			const Vertex v = {0.f, (i%2) ? -radius : radius, 0.f};
			(*pVertices++) = v;
		}

		for (U32 j=1; j<tessV+1; ++j)
		{
			const F32 cV = Maths::Cos(j*deltaV + i*Maths::Constants<F32>::Pi());
			const F32 sV = Maths::Sin(j*deltaV + i*Maths::Constants<F32>::Pi());

			const F32 cU = Maths::Cos(i*deltaU)*sV;
			const F32 sU = Maths::Sin(i*deltaU)*sV;

			const Vertex v = {radius*cU, radius*cV, radius*sU};
			(*pVertices++) = v;
		}
	}

	U32 nV = pVertices - (Vertex*)pMeshCluster->GetVertexDataElement(AftkRender::E_ElementType_Position);
	assert(nV == nVertices);
	return pMeshCluster;
}

PrimitiveResourcePtr Primitive::SphereOutline(F32 radius)
{
	const U32 tesselation = 20;
	const U32 nVertices = 6*tesselation;

	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_LineList);
	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	

	F32 cAPrev=1.f, sAPrev=0.f;

	for (U32 i=0; i<tesselation; ++i)
	{
		F32 theta=(i+1)*Maths::Constants<F32>::Pi()*2.f/tesselation;

		F32 cA = cosf(-theta);
		F32 sA = sinf(-theta);

		const Vertex v[6] =
		{
			{ 0.f, radius*sAPrev, radius*cAPrev },
			{ 0.f, radius*sA, radius*cA },

			{ radius*cAPrev, 0.f, radius*sAPrev },
			{ radius*cA, 0.f, radius*sA },

			{ radius*sAPrev, radius*cAPrev, 0.f },
			{ radius*sA, radius*cA, 0.f }
		};

		(*pVertices++) = v[0]; (*pVertices++) = v[1];
		(*pVertices++) = v[2]; (*pVertices++) = v[3];
		(*pVertices++) = v[4]; (*pVertices++) = v[5];

		cAPrev = cA;
		sAPrev = sA;
	}

	return pMeshCluster;
}

PrimitiveResourcePtr Primitive::Cone(F32 baseRadius, F32 height, U32 tess, bool base)
{
	const U32 nVertices = base ? 6*tess : 3*tess;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_TriList);
	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	

	F32 cAPrev=1.f, sAPrev=0.f;
	const Vertex o = { 0.f, 0.f, 0.f };
	const Vertex p = { 0.f, height, 0.f };

	for (U32 i=0; i<tess; ++i)
	{
		F32 theta=(i+1)*Maths::Constants<F32>::Pi()*2.f/tess;

		F32 cA = cosf(-theta);
		F32 sA = sinf(-theta);

		const Vertex v[2] =
		{
			{ baseRadius*cAPrev, 0.f, baseRadius*sAPrev },
			{ baseRadius*cA, 0.f, baseRadius*sA }
		};

		(*pVertices++) = p; (*pVertices++) = v[0]; (*pVertices++) = v[1];
		
		if (base)
		{
			(*pVertices++) = o; (*pVertices++) = v[1]; (*pVertices++) = v[0];
		}

		cAPrev = cA;
		sAPrev = sA;
	}

	return pMeshCluster;
}

PrimitiveResourcePtr Primitive::ConeOutline(F32 baseRadius, F32 height)
{
	const U32 tesselation = 20;
	const U32 nVertices = 2*tesselation + 8;

	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_LineList);
	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	

	F32 cAPrev=1.f, sAPrev=0.f;
	const Vertex p[7] =
	{
		{ 0.f, height, 0.f },
		{ -baseRadius, 0.f, 0.f },
		{  baseRadius, 0.f, 0.f },
		{ 0.f, 0.f, -baseRadius },
		{ 0.f, 0.f,  baseRadius }
	};

	for (U32 i=0; i<tesselation; ++i)
	{
		F32 theta=(i+1)*Maths::Constants<F32>::Pi()*2.f/tesselation;

		F32 cA = cosf(-theta);
		F32 sA = sinf(-theta);

		const Vertex v[2] =
		{
			{ baseRadius*cAPrev, 0.f, baseRadius*sAPrev },
			{ baseRadius*cA, 0.f, baseRadius*sA }
		};

		(*pVertices++) = v[0]; (*pVertices++) = v[1];

		cAPrev = cA;
		sAPrev = sA;
	}

	(*pVertices++) = p[0]; (*pVertices++) = p[1]; (*pVertices++) = p[0]; (*pVertices++) = p[2];
	(*pVertices++) = p[0]; (*pVertices++) = p[3]; (*pVertices++) = p[0]; (*pVertices++) = p[4];

	return pMeshCluster;
}

PrimitiveResourcePtr Primitive::Tetrahedron(F32 sizeX, F32 sizeY, F32 sizeZ)
{
	const U32 nVertices = 12;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_TriList);
	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	
	const F32 sqrt3 = Maths::Sqrt(3.f);

	sizeX *= sqrt3*0.5;
	sizeY *= sqrt3*0.5;
	sizeZ *= sqrt3*0.5;

	const Vertex v[4] = 
	{
		{ -sizeX, -sizeY, -sizeZ },
		{  sizeX,  sizeY, -sizeZ },
		{  sizeX, -sizeY,  sizeZ },
		{ -sizeX,  sizeY,  sizeZ }
	};

	(*pVertices++) = v[0]; (*pVertices++) = v[1]; (*pVertices++) = v[2];
	(*pVertices++) = v[0]; (*pVertices++) = v[3]; (*pVertices++) = v[1];
	(*pVertices++) = v[2]; (*pVertices++) = v[1]; (*pVertices++) = v[3];
	(*pVertices++) = v[3]; (*pVertices++) = v[0]; (*pVertices++) = v[2];

	return pMeshCluster;

}

PrimitiveResourcePtr Primitive::Octahedron(F32 sizeX, F32 sizeY, F32 sizeZ)
{
	const U32 nVertices = 24;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_TriList);
	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);
	
	sizeX *=0.5;
	sizeY *=0.5;
	sizeZ *=0.5;

	const Vertex v[6] = 
	{
		{ -sizeX, 0.f, 0.f },
		{  sizeX, 0.f, 0.f },
		{ 0.f, -sizeY, 0.f },
		{ 0.f,  sizeY, 0.f },
		{ 0.f, 0.f, -sizeZ },
		{ 0.f, 0.f,  sizeZ }
	};

	(*pVertices++) = v[0]; (*pVertices++) = v[4]; (*pVertices++) = v[2];
	(*pVertices++) = v[2]; (*pVertices++) = v[4]; (*pVertices++) = v[1];
	(*pVertices++) = v[1]; (*pVertices++) = v[4]; (*pVertices++) = v[3];
	(*pVertices++) = v[3]; (*pVertices++) = v[4]; (*pVertices++) = v[0];
	(*pVertices++) = v[0]; (*pVertices++) = v[2]; (*pVertices++) = v[5];
	(*pVertices++) = v[2]; (*pVertices++) = v[1]; (*pVertices++) = v[5];
	(*pVertices++) = v[1]; (*pVertices++) = v[3]; (*pVertices++) = v[5];
	(*pVertices++) = v[3]; (*pVertices++) = v[0]; (*pVertices++) = v[5];

	return pMeshCluster;
}


PrimitiveResourcePtr Primitive::Icosahedron(F32 sizeX, F32 sizeY, F32 sizeZ)
{
	const U32 nVertices = 60;
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	Vertex *pVertices = new Vertex[nVertices];

	if(!pMeshCluster || !pVertices)
	{
		SAFE_DELETE_PTR(pMeshCluster);
		SAFE_DELETE_PTR(pVertices);
		return 0;
	}

	pMeshCluster->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_TriList);
	pMeshCluster->SetNumVertices(nVertices);
	pMeshCluster->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);

	const F32 t = (1.f + Maths::Sqrt(5.f)) * 0.5f;
	const F32 s = 1.f/Maths::Sqrt(1.f + t*t);
	const F32 u = s*t;

	sizeX *= 0.5f;
	sizeY *= 0.5f;
	sizeZ *= 0.5f;

	const Vertex v[12] = 
	{
		{  sizeX*u,  sizeY*s, 0.f },
		{ -sizeX*u,  sizeY*s, 0.f },
		{ -sizeX*u, -sizeY*s, 0.f },
		{  sizeX*u, -sizeY*s, 0.f },
		{  sizeX*s, 0.f,  sizeZ*u },
		{  sizeX*s, 0.f, -sizeZ*u },
		{ -sizeX*s, 0.f, -sizeZ*u },
		{ -sizeX*s, 0.f,  sizeZ*u },
		{ 0.f,  sizeY*u,  sizeZ*s },
		{ 0.f, -sizeY*u,  sizeZ*s },
		{ 0.f, -sizeY*u, -sizeZ*s },
		{ 0.f,  sizeY*u, -sizeZ*s }
	};

    (*pVertices++) = v[4];  (*pVertices++) = v[8];  (*pVertices++) = v[7];
    (*pVertices++) = v[4];  (*pVertices++) = v[7];  (*pVertices++) = v[9];
    (*pVertices++) = v[5];  (*pVertices++) = v[6];  (*pVertices++) = v[11];
    (*pVertices++) = v[5];  (*pVertices++) = v[10]; (*pVertices++) = v[6];
    (*pVertices++) = v[0];  (*pVertices++) = v[4];  (*pVertices++) = v[3];
    (*pVertices++) = v[0];  (*pVertices++) = v[3];  (*pVertices++) = v[5];
    (*pVertices++) = v[2];  (*pVertices++) = v[7];  (*pVertices++) = v[1];
    (*pVertices++) = v[2];  (*pVertices++) = v[1];  (*pVertices++) = v[6];
    (*pVertices++) = v[8];  (*pVertices++) = v[0];  (*pVertices++) = v[11];
    (*pVertices++) = v[8];  (*pVertices++) = v[11]; (*pVertices++) = v[1];
    (*pVertices++) = v[9];  (*pVertices++) = v[10]; (*pVertices++) = v[3];
    (*pVertices++) = v[9];  (*pVertices++) = v[2];  (*pVertices++) = v[10];
    (*pVertices++) = v[8];  (*pVertices++) = v[4];  (*pVertices++) = v[0];
    (*pVertices++) = v[11]; (*pVertices++) = v[0];  (*pVertices++) = v[5];
    (*pVertices++) = v[4];  (*pVertices++) = v[9];  (*pVertices++) = v[3];
    (*pVertices++) = v[5];  (*pVertices++) = v[3];  (*pVertices++) = v[10];
    (*pVertices++) = v[7];  (*pVertices++) = v[8];  (*pVertices++) = v[1];
    (*pVertices++) = v[6];  (*pVertices++) = v[1];  (*pVertices++) = v[11];
    (*pVertices++) = v[7];  (*pVertices++) = v[2];  (*pVertices++) = v[9];
    (*pVertices++) = v[6];  (*pVertices++) = v[10]; (*pVertices++) = v[2];
   
	return pMeshCluster;
}

PrimitiveResourcePtr Primitive::GeodesicSphere(F32 radius, Primitive::E_SolidType baseSolid, U32 subdivisions)
{
	//TODO: recursive subdivision of triangles in base solid before projecting onto sphere surface

	return 0;
}

}//Primitive
}//Editor