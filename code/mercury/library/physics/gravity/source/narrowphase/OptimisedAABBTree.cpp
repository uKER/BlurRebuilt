#include "stdafx.h"
#include "gravity/narrowphase/optimisedaabbtree.h"
#include "gravity/narrowphase/collisionfilter.inl"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/AxisBox.inl"


namespace Collision
{
                   
#ifndef PLATFORM_SPU
	Debug::Logger &g_Log = Debug::GetLogger("Gravity.OptimisedAABBTree");
#endif

#ifndef PLATFORM_SPU
void OptimisedAABBNode::DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s)
{
	//Get bounding box
	//s >> m_BoundingBox.m_min >> m_BoundingBox.m_max;
	m_BoundingBox.DeSerialise(s);
	s >> m_TriangleSizeIndices >> m_TriangleStartIndex;

	if(s.m_stream.GetVersionNumber() >= 10)
		s >> m_VerticeStartIndex >> m_NumVertices; 

	//assert(m_NumIndices < 256);
}

void AABBTreeTriangle::DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s)
{
	s >> m_IndexA >> m_IndexB >> m_IndexC;
	if(s.m_stream.GetVersionNumber() >= 1)
	{
		s >> m_SurfaceNormal.v;
	}
	else
	{
		U16 index;
		s >> index;
		m_SurfaceNormal.v = index;
	}
	
	if(s.m_stream.GetVersionNumber() >= 7)
	{
		s >> m_ActiveEdgeDataIndex;
	}
	else
	{
		U8 edgeFlags;
		s >> edgeFlags;
		m_ActiveEdgeDataIndex = 0;
	}
}

void OldAABBTreeTriangle::DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s)
{
	s >> m_IndexA >> m_IndexB >> m_IndexC;
	if(s.m_stream.GetVersionNumber() >= 1)
	{
		s >> m_SurfaceNormal.v;
	}
	else
	{
		U16 index;
		s >> index;
		m_SurfaceNormal.v = index;
	}
	
	if(s.m_stream.GetVersionNumber() >= 7)
	{
		s >> m_ActiveEdgeDataIndex;
	}
	else
	{
		U8 edgeFlags;
		s >> edgeFlags;
		m_ActiveEdgeDataIndex = 0;
	}
}

void C_TriangleHeader::DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s)
{
	s >> m_CollisionFlags >> m_PhysicsMaterialIndex;
	if(s.m_stream.GetVersionNumber() >= 2)
	{
		s >> m_EffectMaterialIndex;
	}
	else
	{
		m_EffectMaterialIndex = m_PhysicsMaterialIndex;
	}
	s >> m_UserDataFlags >> m_NumTriangles;
	//assert(m_MaterialIndex < 64);
	BIZ_ASSERT(m_NumTriangles != 0);
}

void C_TriangleStream::DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& serial, U32 size)
{
	serial.Read(m_pStream, size);
}

bool OptimisedAABBTree::CheckNormals()
{
	//U32 offset = 0;//sizeof(Collision::C_TriangleHeader);
	//

	//while(offset < m_NumTriangles)
	//{
	//	Collision::C_TriangleHeader* header = m_Triangles.GetHeader(offset);
	//	offset += sizeof(Collision::C_TriangleHeader);
	//	Collision::AABBTreeTriangle* tri = m_Triangles.GetTriangles(offset);
	//	offset += sizeof(Collision::AABBTreeTriangle) * header->m_NumTriangles;
	//	for(U32 a =0; a < header->m_NumTriangles; ++a)
	//	{
	//		const Maths::C_Normal normal(Maths::Load<Maths::HVec3>(&tri->m_SurfaceNormal));
	//	/*	const Maths::HVec3 v0(Maths::Load<Maths::HVec3>(&tempVertices[tri->m_IndexA]));
	//		const Maths::HVec3 v1(Maths::Load<Maths::HVec3>(&tempVertices[tri->m_IndexB]));
	//		const Maths::HVec3 v2(Maths::Load<Maths::HVec3>(&tempVertices[tri->m_IndexC]));*/

	//		
	//		const Maths::HVec3 v0(&m_pVertices[tri->m_IndexA]);
	//		const Maths::HVec3 v1(&m_pVertices[tri->m_IndexB]);
	//		const Maths::HVec3 v2(&m_pVertices[tri->m_IndexC]);

	//		const Maths::HVec3 ab(v1 - v0);
	//		const Maths::HVec3 ac(v2 - v0);

	//		const Maths::C_Normal norm(ab ^ ac);
	//		if((normal | norm) < 0.99f)
	//		{
	//			DebugBreak();
	//			return false;
	//		}

	//	}
	//}
	return true;
}

//Returns the combined flags!
U32 OptimisedAABBTree::DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s)
{

	if(s.m_stream.GetVersionNumber() < 10)
	{
		LegacyAABBTree legacyTree;
		legacyTree.DeSerialise(s);
		return legacyTree.LoadMesh(this);
	}

	U32 combinedFlags = 0;

	s >> m_NumVertices >> m_NumNormals >> m_NumTriangles >> m_NumNodes;

	
	s >> m_Low >> m_High;

	//m_Range = m_High - m_Low;

	if(s.m_stream.GetVersionNumber() < 2)
	{
		m_NumTriangles *= 4;	//Not enough space for final data structure, so double it - more than enough!
		m_NumTriangles /= 3;
	}

	delete [] m_pVertices;
	delete [] m_pNormals;

	//if(m_pTriangles)
	//	delete [] m_pTriangles;
	if(m_pNodes)
		//delete [] m_pNodes;
		BizAlignedFree(m_pNodes);

	/*if(m_NumVertices == 0 || m_NumNormals == 0 || m_NumNodes == 0)
		return;*/

	//m_pVertices = new MathsStorage::Float3[m_NumVertices];
	//m_pVertices = (m_NumVertices > 0) ? new MathsStorage::Fixed3_S16<14>[m_NumVertices] : NULL;
	m_pVertices = (m_NumVertices > 0) ? new MathsStorage::Fixed3_S16<14>[ m_NumVertices + 3 ] : NULL; // JASOND: Padded to prevent unaligned vector loading crash!

	//MathsStorage::Float3* tempVertices = new MathsStorage::Float3[m_NumVertices];
	MathsStorage::Float3* tempVertices = new MathsStorage::Float3[m_NumVertices+1]; // JASOND: Padded to prevent unaligned vector loading crash!
	
	m_pNormals = (m_NumNormals > 0) ? new MathsStorage::HenDN3[m_NumNormals] : NULL;
	//m_pTriangles = new AABBTreeTriangle[m_NumTriangles];
	m_Triangles.Resize(m_NumTriangles);
	//m_pNodes = new OptimisedAABBNode[m_NumNodes];
	if (m_NumNodes > 0)
		m_pNodes = (Collision::OptimisedAABBNode*)BizAlignedMalloc((sizeof(Collision::OptimisedAABBNode) * m_NumNodes + SharedPhysics::SmallCachePageSize + SharedPhysics::SmallCachePageSize - 1) & (~(SharedPhysics::SmallCachePageSize - 1)), SharedPhysics::SmallCachePageSize);
	else
		m_pNodes = NULL;
#ifndef PLATFORM_PC
	BIZ_ASSERT((((U32)m_pNodes) & (SharedPhysics::SmallCachePageSize - 1)) == 0);
#endif
	//const Maths::HF32 recipRange(m_Range.Reciprocal());
	
	for(U32 a = 0; a < m_NumVertices; ++a)
	{
		s >> tempVertices[a];
		/*MathsStorage::Float3 temp;
		s >> temp;
		const HVec3 localVertices((temp - m_low) * recipRange);
		m_pVertices[a] = (MathsStorage::Fixed3_S16<14>)localVertices;*/
		//s >> m_pVertices[a];
	}

	//Maths::HVec3 normal;
	for(U32 a = 0; a < m_NumNormals; ++a)
	{
		//s >> normal;
		//s >> m_pNormals[a].x >> m_pNormals[a].y >> m_pNormals[a].z;
		s >> m_pNormals[a].v;
		//Maths::Store<Maths::HVec3>(normal, &m_pNormals[a]);
	}

	/*for(U32 a = 0; a < m_NumTriangles; ++a)
	{
		m_pTriangles[a].DeSerialise(s);
	}*/

	for(U32 a = 0; a < m_NumNodes; ++a)
	{
		m_pNodes[a].DeSerialise(s);
	}
	if(m_NumNodes > 0)
	{

		SharedPhysics::C_SafeStack<true> stack(16000);
		U32 size = 1;
		stack.Push(m_pNodes, 0);
		U32 offset = 0;
		
		while(size--)
		{
			Collision::OptimisedAABBNode* node(stack.Pop<Collision::OptimisedAABBNode>(size));
			if(node->m_TriangleSizeIndices == 0)
			{
				stack.Push(node + node->m_RightChildOffset, size++);
				stack.Push(node + 1, size++);
				continue;
			}

			Collision::C_AxisBoxMinMax box;
			//Remap the temp vertices corresponding to quantized vertices
			node->m_BoundingBox.LoadBox(box, m_Low, m_High);
			
			const Maths::HVec3 recipRange = (box.m_max - box.m_min).Reciprocal();
		
			const U32 startIndex = node->m_VerticeStartIndex;
			const U32 numVertices = node->m_NumVertices;

			//BIZ_ASSERT(numVertices < 256);
#ifndef PLATFORM_SPU
			if(numVertices > 256)
				LOGWARN(g_Log, "The number of vertices in the cell is more than 256, which mean the mesh might has a hole or be weird");
#endif

		/*	if(numVertices > 256)
			{
				const Maths::HVec3 position = (box.m_max + box.m_min)*Maths::Constants<Maths::HF32>::Half();
				Maths::HMat33 identity;
				identity.SetIdentity();
				const Maths::HMat43 mat(identity, position);
				const Maths::HVec3 extents = (box.m_max - box.m_min)*Maths::Constants<Maths::HF32>::Half();
				DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dBox(-extents, extents, mat, Maths::RGBA(255, 0, 0));
			}*/

			for(U32 i=startIndex; i<startIndex + numVertices ; ++i)
			{
				BIZ_ASSERT(startIndex < m_NumVertices);
				const Maths::HVec3 temp=Maths::Load<Maths::HVec3, MathsStorage::Float3>(&tempVertices[i]);
				const Maths::HVec3 localVertices((temp - box.m_min) * recipRange);
				m_pVertices[i] = (MathsStorage::Fixed3_S16<14>)localVertices;
			}

			//We have a leaf node...read in the header!
			//assert(node->m_StartIndex == offset);
			//if(s.m_stream.GetVersionNumber() < 2)
			node->m_TriangleStartIndex = offset; //fix offset value!
			Collision::C_TriangleHeader* header(this->m_Triangles.GetHeader(offset));
			offset += sizeof(Collision::C_TriangleHeader);
			Collision::AABBTreeTriangle* tri(m_Triangles.GetTriangles(offset));
			header->DeSerialise(s);
			combinedFlags |= header->m_CollisionFlags;
			U32 headerIndex = 0;
			for(U32 a = 0; a < node->m_TriangleSizeIndices; ++a, ++headerIndex, ++tri)
			{
				if(headerIndex >= header->m_NumTriangles)
				{
					header = m_Triangles.GetHeader(offset);
					offset += sizeof(Collision::C_TriangleHeader);
					header->DeSerialise(s);
					tri = m_Triangles.GetTriangles(offset);
					headerIndex = 0;
					combinedFlags |= header->m_CollisionFlags;
				}
				tri->DeSerialise(s);
				if(s.m_stream.GetVersionNumber() < 8)
				{
					tri->m_SurfaceNormal = m_pNormals[tri->m_SurfaceNormal.v];
				}
				//Cache the triangle normal. TODO - Get rid of normal array!
				
				offset += sizeof(Collision::AABBTreeTriangle);
				BIZ_ASSERT(offset <= m_NumTriangles);
			}
			node->m_TriangleStreamLength = offset - node->m_TriangleStartIndex;
		}
		m_NumTriangles = offset;
		/*if(s.m_stream.GetVersionNumber() < 7)
		{
			assert(offset <= m_NumTriangles);
			m_NumTriangles = offset;
		}*/

	}

	delete[] tempVertices;

	if(s.m_stream.GetVersionNumber() >= 7)
	{
		s >> m_NumActiveEdges;
		this->m_pActiveEdgeNormals = (m_NumActiveEdges > 0) ? new C_ActiveEdgeNormals[m_NumActiveEdges] : NULL;
		for(U32 a = 0; a < m_NumActiveEdges; ++a)
		{
			m_pActiveEdgeNormals[a].DeSerialise(s);
		}
	}

	return combinedFlags;

	
	
	//assert(CheckNormals());
	//m_Triangles.DeSerialise(s, m_NumTriangles);
}

int FindVerticeIndex(SharedPhysics::SortedArray<MathsStorage::Float3>& remapVertices, const U32& startIndex, const MathsStorage::Float3& vertex)
{
	const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
	const Maths::HVec3 v(Maths::Load<Maths::HVec3>(&vertex));
	for(S32 i=startIndex; i<remapVertices.m_numElements; ++i)
	{
		//const Maths::HVec3 temp = remapVertices[i];
		const Maths::HVec3 remapVertex(Maths::Load<Maths::HVec3>(&remapVertices[i]));
		const Maths::HVec3 vD(v - remapVertex);
		//if(vertex.All_EQ(temp))
		if((vD|vD).LE(epsilon))
		{
			return i;
		}
	}
	return -1;
}

void LegacyAABBTree::RemapTriangleVertices(Collision::OldAABBTreeTriangle& tri, U32 startIndex, SharedPhysics::SortedArray<MathsStorage::Float3>& remappedVertices)
{
	//Get the vertices and remap them into the array...
	MathsStorage::Float3& v0(m_pUncompressedVertices[tri.m_IndexA]);
	MathsStorage::Float3& v1(m_pUncompressedVertices[tri.m_IndexB]);
	MathsStorage::Float3& v2(m_pUncompressedVertices[tri.m_IndexC]);
	
	S32 indexA = FindVerticeIndex(remappedVertices, startIndex, v0);
	if(indexA == -1)
	{
		//Not in there, so add it...
		tri.m_IndexA = remappedVertices.m_numElements;
		remappedVertices.AddToEnd(&v0);
	}
	else
	{
		tri.m_IndexA = indexA;
	}
	S32 indexB = FindVerticeIndex(remappedVertices, startIndex, v1);
	if(indexB == -1)
	{
		//Not in there, so add it...
		tri.m_IndexB = remappedVertices.m_numElements;
		remappedVertices.AddToEnd(&v1);
	}
	else
	{
		tri.m_IndexB = indexB;
	}

	S32 indexC = FindVerticeIndex(remappedVertices, startIndex, v2);
	if(indexC == -1)
	{
		//Not in there, so add it...
		tri.m_IndexC = remappedVertices.m_numElements;
		remappedVertices.AddToEnd(&v2);
	}
	else
	{
		tri.m_IndexC = indexC;
	}
}

U32 LegacyAABBTree::LoadMesh(Collision::OptimisedAABBTree* pTree)
{
	U32 combinedFlags = 0;
	pTree->m_NumVertices = m_Vertices.m_numElements;

	pTree->m_NumNormals = m_NumNormals;
	pTree->m_NumTriangles = m_NumTriangles;
	pTree->m_NumNodes = m_NumNodes;

	pTree->m_Low = m_Low;
	pTree->m_High = m_High;

	if(pTree->m_pVertices)
		delete [] pTree->m_pVertices;
	if(pTree->m_pNormals)
		delete [] pTree->m_pNormals;
	if(pTree->m_pNodes)
		delete [] pTree->m_pNodes;

	pTree->m_pVertices = new MathsStorage::Fixed3_S16<14>[m_NumVertices + 3];
	pTree->m_pNormals = (m_NumNormals > 0) ? new MathsStorage::HenDN3[m_NumNormals] : NULL;
	//m_pTriangles = new AABBTreeTriangle[m_NumTriangles];
	pTree->m_Triangles.Resize(m_NumTriangles);
	//pTree->m_pNodes = new OptimisedAABBNode[m_NumNodes];
	pTree->m_pNodes = (Collision::OptimisedAABBNode*)BizAlignedMalloc(sizeof(OptimisedAABBNode) * m_NumNodes + SharedPhysics::SmallCachePageSize, SharedPhysics::SmallCachePageSize);

	//const Maths::HF32 recipRange(m_Range.Reciprocal());
	
	for(U32 a = 0; a < m_NumVertices; ++a)
	{
		pTree->m_pVertices[a] = m_Vertices[a];
	}

	//Maths::HVec3 normal;
	for(U32 a = 0; a < m_NumNormals; ++a)
	{
		pTree->m_pNormals[a] = m_pNormals[a];
	}

	for(U32 a = 0; a < m_NumNodes; ++a)
	{
		pTree->m_pNodes[a] = m_pNodes[a];
	}
	if(m_NumNodes > 0)
	{
		SharedPhysics::C_SafeStack<true> stack(16000);
		SharedPhysics::C_SafeStack<true> stack2(16000);
		U32 size = 1;
		stack.Push(pTree->m_pNodes, 0);
		stack2.Push(m_pNodes, 0);
		U32 offset = 0;
		U32 legacyOffset = 0;
		
		while(size--)
		{
			Collision::OptimisedAABBNode* treeNode(stack.Pop<Collision::OptimisedAABBNode>(size));
			Collision::OptimisedAABBNode* legacyNode(stack2.Pop<Collision::OptimisedAABBNode>(size));
			if(treeNode->m_TriangleSizeIndices == 0)
			{
				stack.Push(treeNode + treeNode->m_RightChildOffset, size);
				stack2.Push(legacyNode + legacyNode->m_RightChildOffset, size++);
				stack.Push(treeNode + 1, size);
				stack2.Push(legacyNode + 1, size++);
				continue;
			}

			Collision::C_AxisBoxMinMax box;

			treeNode->m_TriangleStartIndex = offset;

			Collision::C_TriangleHeader* treeHeader(pTree->m_Triangles.GetHeader(offset));
			Collision::C_TriangleHeader* legacyHeader(m_Triangles.GetHeader(legacyOffset));

			offset += sizeof(Collision::C_TriangleHeader);
			legacyOffset += sizeof(Collision::C_TriangleHeader);

			Collision::AABBTreeTriangle* treeTri(pTree->m_Triangles.GetTriangles(offset));
			Collision::OldAABBTreeTriangle* legacyTri(m_Triangles.GetTriangles(legacyOffset));

			*treeHeader = *legacyHeader;

			U32 headerIndex = 0;

			for(U32 a = 0; a < treeNode->m_TriangleSizeIndices; ++a, ++headerIndex, ++treeTri, ++legacyTri)
			{
				if(headerIndex >= treeHeader->m_NumTriangles)
				{
					treeHeader = pTree->m_Triangles.GetHeader(offset);
					legacyHeader = m_Triangles.GetHeader(legacyOffset);
					offset += sizeof(Collision::C_TriangleHeader);
					legacyOffset += sizeof(Collision::C_TriangleHeader);
					*treeHeader = *legacyHeader;
					treeTri = pTree->m_Triangles.GetTriangles(offset);
					legacyTri = m_Triangles.GetTriangles(legacyOffset);
					headerIndex = 0;
					combinedFlags |= treeHeader->m_CollisionFlags;
				}
				
				treeTri->m_ActiveEdgeDataIndex = legacyTri->m_ActiveEdgeDataIndex;
				treeTri->m_IndexA = legacyTri->m_IndexA - treeNode->m_VerticeStartIndex;
				treeTri->m_IndexB = legacyTri->m_IndexB - treeNode->m_VerticeStartIndex;
				treeTri->m_IndexC = legacyTri->m_IndexC - treeNode->m_VerticeStartIndex;
				treeTri->m_SurfaceNormal = legacyTri->m_SurfaceNormal;

				offset += sizeof(Collision::AABBTreeTriangle);
				legacyOffset += sizeof(Collision::OldAABBTreeTriangle);
				BIZ_ASSERT(offset <= m_NumTriangles);
			}
			treeNode->m_TriangleStreamLength = offset - treeNode->m_TriangleStartIndex;
		}
		pTree->m_NumTriangles = offset;
	}

	pTree->m_NumActiveEdges = m_NumActiveEdges;;
	if(m_NumActiveEdges > 0)
	{
		pTree->m_pActiveEdgeNormals = new C_ActiveEdgeNormals[m_NumActiveEdges];
		for(U32 a = 0; a < m_NumActiveEdges; ++a)
		{
			pTree->m_pActiveEdgeNormals[a] = m_pActiveEdgeNormals[a];
		}
	}
	return combinedFlags;
}

void LegacyAABBTree::DeSerialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s)
{
	s >> m_NumVertices >> m_NumNormals >> m_NumTriangles >> m_NumNodes;

	
	s >> m_Low >> m_High;

	m_Range = m_High - m_Low;

	if(s.m_stream.GetVersionNumber() < 2)
	{
		m_NumTriangles *= 4;	//Not enough space for final data structure, so double it - more than enough!
		m_NumTriangles /= 3;
	}

	if(m_pUncompressedVertices)
		delete [] m_pUncompressedVertices;
	if(m_pNormals)
		delete [] m_pNormals;
	//if(m_pTriangles)
	//	delete [] m_pTriangles;
	if(m_pNodes)
		delete [] m_pNodes;

	/*if(m_NumVertices == 0 || m_NumNormals == 0 || m_NumNodes == 0)
		return;*/

	//m_pVertices = new MathsStorage::Float3[m_NumVertices];
	//m_Vertices = new MathsStorage::Fixed3_S16<14>[m_NumVertices];

	//m_pUncompressedVertices = new MathsStorage::Float3[m_NumVertices];
	m_pUncompressedVertices = new MathsStorage::Float3[m_NumVertices+1]; // JASOND: Padded to prevent unaligned vector loading crash!
	m_pNormals = new MathsStorage::HenDN3[m_NumNormals];
	//m_pTriangles = new AABBTreeTriangle[m_NumTriangles];
	m_Triangles.Resize(m_NumTriangles);
	m_pNodes = new OptimisedAABBNode[m_NumNodes];

	//const Maths::HF32 recipRange(m_Range.Reciprocal());
	
	for(U32 a = 0; a < m_NumVertices; ++a)
	{
		s >> m_pUncompressedVertices[a];
	}

	//Maths::HVec3 normal;
	for(U32 a = 0; a < m_NumNormals; ++a)
	{
		s >> m_pNormals[a].v;
	}

	for(U32 a = 0; a < m_NumNodes; ++a)
	{
		m_pNodes[a].DeSerialise(s);
	}
	if(m_NumNodes > 0)
	{

		SharedPhysics::C_SafeStack<true> stack(16000);
		U32 size = 1;
		stack.Push(m_pNodes, 0);
		U32 offset = 0;

		SharedPhysics::SortedArray<MathsStorage::Float3> remappedVertexArray;
		S32 startIndex = 0;
		
		while(size--)
		{
			Collision::OptimisedAABBNode* node(stack.Pop<Collision::OptimisedAABBNode>(size));
			if(node->m_TriangleSizeIndices == 0)
			{
				stack.Push(node + node->m_RightChildOffset, size++);
				stack.Push(node + 1, size++);
				continue;
			}

			Collision::C_AxisBoxMinMax box;
			//Remap the temp vertices corresponding to quantized vertices
			node->m_BoundingBox.LoadBox(box, m_Low, m_High);
			
			const Maths::HVec3 recipRange = (box.m_max - box.m_min).Reciprocal();

			//We have a leaf node...read in the header!
			//assert(node->m_StartIndex == offset);
			//if(s.m_stream.GetVersionNumber() < 2)
			node->m_TriangleStartIndex = offset; //fix offset value!
			Collision::C_TriangleHeader* header(this->m_Triangles.GetHeader(offset));
			offset += sizeof(Collision::C_TriangleHeader);
			Collision::OldAABBTreeTriangle* tri(m_Triangles.GetTriangles(offset));
			header->DeSerialise(s);
			U32 headerIndex = 0;
			for(U32 a = 0; a < node->m_TriangleSizeIndices; ++a, ++headerIndex, ++tri)
			{
				if(headerIndex >= header->m_NumTriangles)
				{
					header = m_Triangles.GetHeader(offset);
					offset += sizeof(Collision::C_TriangleHeader);
					header->DeSerialise(s);
					tri = m_Triangles.GetTriangles(offset);
					headerIndex = 0;
				}
				tri->DeSerialise(s);

				RemapTriangleVertices(*tri, startIndex, remappedVertexArray);

				if(s.m_stream.GetVersionNumber() < 8)
				{
					tri->m_SurfaceNormal = m_pNormals[tri->m_SurfaceNormal.v];
				}
				//Cache the triangle normal. TODO - Get rid of normal array!
				
				offset += sizeof(Collision::OldAABBTreeTriangle);
				BIZ_ASSERT(offset <= m_NumTriangles);
			}

			//OK. Compress the vertices now...
			for(S32 i=startIndex; i<remappedVertexArray.m_numElements; ++i)
			{
				const Maths::HVec3 temp=Maths::Load<Maths::HVec3, MathsStorage::Float3>(&remappedVertexArray[i]);
				const Maths::HVec3 localVertices((temp - box.m_min) * recipRange);
				MathsStorage::Fixed3_S16<14> localVert(localVertices);
				m_Vertices.AddToEnd(&localVert);
			}
			

			node->m_VerticeStartIndex = startIndex;
			node->m_NumVertices = remappedVertexArray.m_numElements - startIndex;
			startIndex = remappedVertexArray.m_numElements;

		}
		m_NumTriangles = offset;
		m_NumVertices = m_Vertices.m_numElements;
	}

	if(s.m_stream.GetVersionNumber() >= 7)
	{
		s >> m_NumActiveEdges;
		this->m_pActiveEdgeNormals = new C_ActiveEdgeNormals[m_NumActiveEdges];
		for(U32 a = 0; a < m_NumActiveEdges; ++a)
		{
			m_pActiveEdgeNormals[a].DeSerialise(s);
		}
	}
}


#endif

#if !defined(PLATFORM_SPU) || defined(SPU_CONTINUOUS_COLLISION)
bool OptimisedAABBTree::Collide(C_CollisionObject * obj, SharedPhysics::C_SafeStack<true>& stack, Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, const Maths::HVec3 PCREF r,
								Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, U32& physicsMaterial, U32& effectMaterial,
								bool (*TriCollisions)(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const  tris, const U32 numTris, const U32 triangleStartIndex, 
		const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, const Maths::HVec3& low, const Maths::HVec3& high),
		SharedPhysics::C_SmallSPUCache& spuCache)
{

	//This is continuous collision detection. Use the bounding sphere of the object, and then treat it like a swept raytrace!!!
	//const Maths::HF32 radius(obj->m_pSpuShape->m_Radius);
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());

	//const Collision::C_CRAxisBox box=obj->m_TransformedBox;
	const Maths::HVec3 start(obj->m_TransformedBox.centre);
	Maths::HVec3 end(start - (r * maxLambda));

	/*Collision::C_CRAxisBox objBox;
	objBox.centre = (start + end) * half;
	objBox.extents = obj->m_TransformedBox.extents + Maths::Abs((end - start) * half);*/
	Maths::HF32 tmin(Maths::UNDEFINED), tmin2(Maths::UNDEFINED);

	stack.Push( m_pNodes, 0 );
	OptimisedAABBNode* pStackTop(m_pNodes);
	S32 size = 1;
	bool collide = false;
	Collision::C_Shape* shape = obj->m_pSpuShape;
	C_AxisBoxMinMax box1, box2;

	while(size--)
	{
		Spu::MemBaseMarker marker;
		OptimisedAABBNode* node = pStackTop;
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size - 1, 0) );
		//OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node, sizeof(OptimisedAABBNode));
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)spuCache.FetchAllocData(node, sizeof(OptimisedAABBNode));
		
		fetchedNode->m_BoundingBox.LoadBox(box1, m_Low, m_High, obj->m_TransformedBox.extents);

		if(IntersectRayAABBMinMax(start, end, box1.m_min, box1.m_max, tmin))
		//if(box1.Intersects(objBox))
		{
			if(fetchedNode->m_TriangleSizeIndices == 0)
			{
				OptimisedAABBNode* node1(node + fetchedNode->m_RightChildOffset);
				OptimisedAABBNode* node2(node + 1);
				/*OptimisedAABBNode* _node1 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* _node2 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node2, sizeof(OptimisedAABBNode));*/
				OptimisedAABBNode* _node1 = (OptimisedAABBNode*)spuCache.FetchAllocData(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* _node2 = (OptimisedAABBNode*)spuCache.FetchAllocData(node2, sizeof(OptimisedAABBNode));

				/*stack.Push(node1, size++);
				stack.Push(node2, size++);
				pStackTop = node2;*/

				_node1->m_BoundingBox.LoadBox(box1, m_Low, m_High, obj->m_TransformedBox.extents);
				_node2->m_BoundingBox.LoadBox(box2, m_Low, m_High, obj->m_TransformedBox.extents);

				S32 b1(IntersectRayAABBMinMax(start, end, box1.m_min, box1.m_max, tmin));
				S32 b2(IntersectRayAABBMinMax(start, end, box2.m_min, box2.m_max, tmin2));

				bool node2First1(tmin2 < tmin);
				if(node2First1)
				{
					stack.Push(node1, size);
					size += b1;
					stack.Push(node2, size);
					size += b2;
					pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b2!=0, node2, 
					SharedPhysics::Fast_Select<OptimisedAABBNode>(b1!=0, node1, pStackTop));
				}
				else
				{
					stack.Push(node2, size);
					size += b2;
					stack.Push(node1, size);
					size += b1;
					pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b1!=0, node1, 
					SharedPhysics::Fast_Select<OptimisedAABBNode>(b2!=0, node2, pStackTop));
				}
			}
			else
			{
				//Loop through triangles/triangle headers...
				Spu::MemBaseMarker innerMarker;
				U32 offset = fetchedNode->m_TriangleStartIndex + sizeof(C_TriangleHeader);
				C_TriangleHeader* header = (C_TriangleHeader*)Spu::FetchAllocUnaligned(m_Triangles.GetHeader(fetchedNode->m_TriangleStartIndex), sizeof(C_TriangleHeader));
				U32 numTris(0);
				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnaligned(m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices);
				Collision::C_AxisBoxMinMax b;
				fetchedNode->m_BoundingBox.LoadBox(b, m_Low, m_High);
				
				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a+=numTris)
				{
					numTris = header->m_NumTriangles;
					if(shape->m_Filter.CanCollide(header->m_CollisionFlags))
					{
						AABBTreeTriangle* pTempTriangles = m_Triangles.GetTriangles(offset);
						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)Spu::FetchAllocUnaligned(pTempTriangles, sizeof(AABBTreeTriangle)*numTris);
						if(TriCollisions( obj->m_pShape, pVertices, m_pNormals, pTriangles, header->m_NumTriangles,
							offset, r, maxLambda, lambda, nor, closestA, closestB, b.m_min, b.m_max))
						{
							maxLambda = lambda;
							end = start - (r * lambda);
							physicsMaterial = header->m_PhysicsMaterialIndex | (shape->m_PhysicsMaterialIndex << 16);
							effectMaterial = header->m_EffectMaterialIndex | (shape->m_EffectMaterialIndex << 16);
							collide = true;
						}						
					}
					offset += header->m_NumTriangles * sizeof(AABBTreeTriangle);
					innerMarker.Restore();
					header = (C_TriangleHeader*)Spu::FetchAllocUnaligned(m_Triangles.GetHeader(offset), sizeof(C_TriangleHeader));
					offset += sizeof(C_TriangleHeader);
				}					
			}
		}
	}
	lambda = maxLambda;
	return collide;
}
#endif

#ifndef PLATFORM_SPU
bool OptimisedAABBTree::Collide(C_CollisionObject * obj, SharedPhysics::C_SafeStack<true>& stack, Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, const Maths::HVec3 PCREF r,
		Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, U32& physicsMaterial, U32& effectMaterial, Collision::EPA& epa,
		bool (*TriCollisions)(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const  tris, const U32 numTris, const U32 triangleStartIndex, 
		const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, Collision::EPA& epa, const Maths::HVec3& low, const Maths::HVec3& high),
		SharedPhysics::C_SmallSPUCache& spuCache)
{
	//This is continuous collision detection. Use the bounding sphere of the object, and then treat it like a swept raytrace!!!
	//const Maths::HF32 radius(obj->m_pSpuShape->m_Radius);

	const Maths::HVec3 start(obj->m_TransformedBox.centre);
	Maths::HVec3 end(start + (r * maxLambda));
	Maths::HF32 tmin(Maths::UNDEFINED), tmin2(Maths::UNDEFINED);

	//const Collision::C_CRAxisBox box=obj->m_TransformedBox;
	stack.Push( m_pNodes, 0 );
	OptimisedAABBNode* pStackTop(m_pNodes);
	S32 size = 1;
	bool collide = false;
	Collision::C_Shape* shape = obj->m_pSpuShape;
	C_AxisBoxMinMax box1, box2;

	while(size--)
	{
		Spu::MemBaseMarker marker;
		OptimisedAABBNode* node = pStackTop;
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size - 1, 0) );
		/*OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node, sizeof(OptimisedAABBNode));*/
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)spuCache.FetchAllocData(node, sizeof(OptimisedAABBNode));
		fetchedNode->m_BoundingBox.LoadBox(box1, m_Low, m_High, obj->m_TransformedBox.extents);

		if(IntersectRayAABBMinMax(start, end, box1.m_min, box1.m_max, tmin))
		{
			if(fetchedNode->m_TriangleSizeIndices == 0)
			{
				OptimisedAABBNode* node1(node + fetchedNode->m_RightChildOffset);
				OptimisedAABBNode* node2(node + 1);
				/*OptimisedAABBNode* _node1 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* _node2 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node2, sizeof(OptimisedAABBNode));*/
				OptimisedAABBNode* _node1 = (OptimisedAABBNode*)spuCache.FetchAllocData(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* _node2 = (OptimisedAABBNode*)spuCache.FetchAllocData(node2, sizeof(OptimisedAABBNode));

				_node1->m_BoundingBox.LoadBox(box1, m_Low, m_High, obj->m_TransformedBox.extents);
				_node2->m_BoundingBox.LoadBox(box2, m_Low, m_High, obj->m_TransformedBox.extents);

				S32 b1(IntersectRayAABBMinMax(start, end, box1.m_min, box1.m_max, tmin));
				S32 b2(IntersectRayAABBMinMax(start, end, box2.m_min, box2.m_max, tmin2));

				bool node2First1(tmin2 < tmin);
				if(node2First1)
				{
					stack.Push(node1, size);
					size += b1;
					stack.Push(node2, size);
					size += b2;
					pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b2!=0, node2, 
					SharedPhysics::Fast_Select<OptimisedAABBNode>(b1!=0, node1, pStackTop));
				}
				else
				{
					stack.Push(node2, size);
					size += b2;
					stack.Push(node1, size);
					size += b1;
					pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b1!=0, node1, 
					SharedPhysics::Fast_Select<OptimisedAABBNode>(b2!=0, node2, pStackTop));
				}
			}
			else
			{
				//Loop through triangles/triangle headers...
				Spu::MemBaseMarker innerMarker;
				U32 offset = fetchedNode->m_TriangleStartIndex + sizeof(C_TriangleHeader);
				C_TriangleHeader* header = (C_TriangleHeader*)Spu::FetchAllocUnaligned(m_Triangles.GetHeader(fetchedNode->m_TriangleStartIndex), sizeof(C_TriangleHeader));
				U32 numTris(0);

				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnaligned(m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices);
				Collision::C_AxisBoxMinMax b;
				fetchedNode->m_BoundingBox.LoadBox(b, m_Low, m_High);

				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a+=numTris)
				{
					numTris = header->m_NumTriangles;
					if(shape->m_Filter.CanCollide(header->m_CollisionFlags))
					{
						AABBTreeTriangle* pTempTriangles = m_Triangles.GetTriangles(offset);
						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)Spu::FetchAllocUnaligned(pTempTriangles, sizeof(AABBTreeTriangle)*numTris);
						if(TriCollisions( obj->m_pShape, pVertices, m_pNormals, pTriangles, header->m_NumTriangles, offset, r, maxLambda, lambda, nor, closestA, closestB, epa, b.m_min, b.m_max))
						{
							maxLambda = lambda;
							end = start + (r * maxLambda);
							physicsMaterial = header->m_PhysicsMaterialIndex | (shape->m_PhysicsMaterialIndex << 16);
							effectMaterial = header->m_EffectMaterialIndex | (shape->m_EffectMaterialIndex << 16);
							collide = true;
						}

						
					}
					offset += header->m_NumTriangles * sizeof(AABBTreeTriangle);
					innerMarker.Restore();
					header = (C_TriangleHeader*)Spu::FetchAllocUnaligned(m_Triangles.GetHeader(offset), sizeof(C_TriangleHeader));
					offset += sizeof(C_TriangleHeader);
				}					
			}
		}
	}
	lambda = maxLambda;
	return collide;

}
#endif


}

#if !defined(PLATFORM_SPU) || defined(SPU_FILL_CACHE)
#include "gravity/narrowphase/OptimisedAABBTree_FillCache.inl"
#endif
