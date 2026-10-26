#include <stdafx.h>

#include "SectionPhysics.h"

#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include "gravity/narrowphase/OptimisedKDTree.h"
#include "gravity/narrowphase/OptimisedKDTree.inl"
#include "gravity/treebuilder/KDTree.inl"
#include "SegmentSetting.h"
#include "gravity/narrowphase/RayTriangle.h"
#include "gravity/narrowphase/RayTriangle.inl"

using namespace Baking;
using namespace Collision;

namespace Samples
{

Debug::Bool g_DrawKDTriangles("Test.DrawKDTriangles", true);

void C_SectionPhysics::TestBuildKDTree()
{
	/*using namespace Physics;
	C_World* world=this->m_Level.m_pPhysicsWorld;
	const C_ImportUnit* pImportUnit = world->PersistentImportUnit();
	const U32 numVertices = pImportUnit->m_CompressedVertexCount + pImportUnit->m_UncompressedVertexCount;
	Baking::KDTree tree; 
	//Loop through the prim mesh objects in the import unit and add them to the AABB tree

	const U32 numMeshes = pImportUnit->m_PrimMeshArray.Capacity();
	//const C_PrimMesh* pMeshes = pImportUnit->m_PrimMeshArray.Data();
	U32 totalVertexCount = 0;
	const C_StaticBodyKdTreeObject* __restrict pObjectArray = pImportUnit->StaticKdTreeObjectArray();
	for(U32 a = 0; a < pImportUnit->StaticKdTreeObjectCount(); ++a)
	{
		const C_StaticBodyKdTreeObject* __restrict pObject = &pObjectArray[a];
		C_PrimMesh const * const __restrict pPrimMesh = pObject->m_pPrimitive->PrimMesh();
		if ( pPrimMesh != NULL && pPrimMesh->TestFlag(C_Primitive::E_Flag_Collide) )
		{
			C_StaticBody const * const __restrict pStaticBody         = pObject->m_pStaticBody;
			Physics::C_Shape      const * const __restrict pShape              = pObject->m_pShape;
			const U32                             shapePrimitiveIndex = pObject->m_ShapePrimitiveIndex;
	        
			// NOTE: Internal/External doesn't matter here since this is done just after the previous synchronise where they're the same!
			const HMat43 transform( pShape->Transform( shapePrimitiveIndex ) * pStaticBody->Matrix( E_DB_Internal ) );
	        
			const U32 vertexCount   = pPrimMesh->Vertices();
			const U32 triangleCount = pPrimMesh->Triangles();

			Maths::C_Vertex* pVertices = (Maths::C_Vertex*)_alloca(sizeof(C_Vertex) * pPrimMesh->Vertices() + 255);
			pPrimMesh->DecompressVertexArray(pVertices, transform);
			tree.AddVertices((Maths::HVec3*)pVertices, pPrimMesh->Vertices());
			const U32 numTriangles = pPrimMesh->Triangles();
			const C_PrimMeshTriangle* pTriangle = pPrimMesh->TriangleArray();
			for(U32 b = 0; b < numTriangles; ++b, pTriangle++)
			{
				const U32 indexA(pTriangle->VertexIndex<0>());
				const U32 indexB(pTriangle->VertexIndex<1>());
				const U32 indexC(pTriangle->VertexIndex<2>());
				Baking::KDTreeTriangle tri;
				tri.m_IndexA = indexA + totalVertexCount;
				tri.m_IndexB = indexB + totalVertexCount;
				tri.m_IndexC = indexC + totalVertexCount;
				tree.AddIndex(tri);
			}
			totalVertexCount += pPrimMesh->Vertices();
		}
	}
	tree.setSplitRatio(0.f);
	tree.SetBonus(0.3f);

	tree.BuildKDTree();

	static U32 maxDepth=0;

	if(m_input.IsButtonDn(E_Button_A))
	{
		maxDepth++;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		if(maxDepth > 0)
			maxDepth--;
	}

	

	DrawKDTreeNode(tree.GetRoot(), tree, 0, maxDepth);

	{
		Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "kdtest.dat" , Sys::E_FileAccess_Write);
	
		Sys::BinarySerialiser<Ut::NativeToBigEndian> archive(*filePtr);
		//Sys::TextSerialiser archive(*filePtr);

		tree.Serialise(archive);

		delete filePtr;
	}

	Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "kdtest.dat" );
	Sys::BinaryDeserialiser<Ut::NativeToBigEndian> archive(*filePtr);
	Collision::OptimisedKDTree optTree;
	optTree.DeSerialise(archive);
	delete filePtr;*/
}

void C_SectionPhysics::BuildKDTree()
{
	/*using namespace Physics;
	C_World* world=this->m_Level.m_pPhysicsWorld;
	const C_ImportUnit* pImportUnit = world->PersistentImportUnit();
	const U32 numVertices = pImportUnit->m_CompressedVertexCount + pImportUnit->m_UncompressedVertexCount;
	//Loop through the prim mesh objects in the import unit and add them to the AABB tree

	const U32 numMeshes = pImportUnit->m_PrimMeshArray.Capacity();
	//const C_PrimMesh* pMeshes = pImportUnit->m_PrimMeshArray.Data();
	U32 totalVertexCount = 0;
	const C_StaticBodyKdTreeObject* __restrict pObjectArray = pImportUnit->StaticKdTreeObjectArray();
	for(U32 a = 0; a < pImportUnit->StaticKdTreeObjectCount(); ++a)
	{
		const C_StaticBodyKdTreeObject* __restrict pObject = &pObjectArray[a];
		C_PrimMesh const * const __restrict pPrimMesh = pObject->m_pPrimitive->PrimMesh();
		if ( pPrimMesh != NULL && pPrimMesh->TestFlag(C_Primitive::E_Flag_Collide) )
		{
			C_StaticBody const * const __restrict pStaticBody         = pObject->m_pStaticBody;
			Physics::C_Shape      const * const __restrict pShape              = pObject->m_pShape;
			const U32                             shapePrimitiveIndex = pObject->m_ShapePrimitiveIndex;
	        
			// NOTE: Internal/External doesn't matter here since this is done just after the previous synchronise where they're the same!
			const HMat43 transform( pShape->Transform( shapePrimitiveIndex ) * pStaticBody->Matrix( E_DB_Internal ) );
	        
			const U32 vertexCount   = pPrimMesh->Vertices();
			const U32 triangleCount = pPrimMesh->Triangles();

			Maths::C_Vertex* pVertices = (Maths::C_Vertex*)_alloca(sizeof(C_Vertex) * pPrimMesh->Vertices() + 255);
			pPrimMesh->DecompressVertexArray(pVertices, transform);
			m_kdtree.AddVertices((Maths::HVec3*)pVertices, pPrimMesh->Vertices());
			const U32 numTriangles = pPrimMesh->Triangles();
			const C_PrimMeshTriangle* pTriangle = pPrimMesh->TriangleArray();
			for(U32 b = 0; b < numTriangles; ++b, pTriangle++)
			{
				const U32 indexA(pTriangle->VertexIndex<0>());
				const U32 indexB(pTriangle->VertexIndex<1>());
				const U32 indexC(pTriangle->VertexIndex<2>());
				Baking::KDTreeTriangle tri;
				tri.m_IndexA = indexA + totalVertexCount;
				tri.m_IndexB = indexB + totalVertexCount;
				tri.m_IndexC = indexC + totalVertexCount;
				m_kdtree.AddIndex(tri);
			}
			totalVertexCount += pPrimMesh->Vertices();
		}
	}
	m_kdtree.setSplitRatio(0.f);
	m_kdtree.SetBonus(0.3f);

	m_kdtree.BuildKDTree();

	{
		Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "kdtest.dat" , Sys::E_FileAccess_Write);
	
		Sys::BinarySerialiser<Ut::NativeToBigEndian> archive(*filePtr);
		//Sys::TextSerialiser archive(*filePtr);

		m_kdtree.Serialise(archive);

		delete filePtr;
	}

	Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "kdtest.dat" );
	Sys::BinaryDeserialiser<Ut::NativeToBigEndian> archive(*filePtr);
	m_OptKdTree.DeSerialise(archive);
	delete filePtr;*/
}


void C_SectionPhysics::TestKDTreeRay()
{
		
	/*static bool initialised = false;
	if(!initialised)
	{
		BuildKDTree();
		initialised = true;
	}
	//DrawAABBTreeNode(m_tree.GetRoot(), m_tree, 0, 8);
	Ray ray;
	ray.m_start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	ray.m_end=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));
	ray.m_result.m_contact=ray.m_end;

	SharedPhysics::C_SafeStack<true> stack(1024);
	bool collide=m_OptKdTree.RayTracing<Collision::CollideRayTriangleMesh<Collision::KDTreeTriangle>>(ray, stack);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_result.m_contact, RGBA(255, 255, 0, 255));

	if(collide)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_result.m_contact, RGBA(0, 0, 255, 255));
	}*/
}

/*void C_SectionPhysics::DrawKDTreeNode(KDTreeNode* node, KDTree& tree, U32 depth, U32 maxDepth)
{
	using namespace Maths;

	if(depth > maxDepth)
		return;
	
	HMat33 m(UNDEFINED);
	m.SetIdentity();

	const HVec3 half(Maths::Constants<HVec3>::Half());
	const HMat43 mat(m, HVec3());
	//const U32 endIndex = node->m_startIndex + node->m_sizeIndice;
	//const Maths::RGBA triColour(255 - endIndex, endIndex, 0, 255);
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(node->min, node->max, mat, triColour);
	

	U32 a=node->m_axis;
	HF32 dist(node->m_dist);

	KDTreeNode* left=node->m_leftNode;
	KDTreeNode* right=node->m_rightNode;

	HVec3 leftMax(node->max);
	HVec3 rightMin(node->min);

	if(a == Baking::X)
	{
		//const HF32 p(dist-node->min.X());
		leftMax.X(dist);
		rightMin.X(dist);
		
	}
	else if(a == Baking::Y)
	{
		//const HF32 p(dist-node->min.Y());
		leftMax.Y(dist);
		rightMin.Y(dist);
	}
	else if(a == Baking::Z)
	{
		//const HF32 p(dist-node->min.Z());
		leftMax.Z(dist);
		rightMin.Z(dist);
	}

	//const U32 endIndex = left->m_startIndex + left->m_sizeIndice;
	//const Maths::RGBA triColour(255 - endIndex, endIndex, 0, 255);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(node->min, leftMax, mat, RGBA(255, 0, 0, 255));
		//const U32 endIndex = right->m_startIndex + right->m_sizeIndice;
		//const Maths::RGBA triColour(255 - endIndex, endIndex, 0, 255);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(rightMin, node->max, mat,RGBA(255, 255, 0, 255));
	
	if(node->m_leftNode)
	{
		
		//if(node->m_leftNode->m_leftNode)
			DrawKDTreeNode(node->m_leftNode, tree, depth+1, maxDepth);
		//if(node->m_rightNode->m_rightNode)
			DrawKDTreeNode(node->m_rightNode, tree, depth+1, maxDepth);
	}	
	else
	{
	if(g_DrawKDTriangles)
	{
		SharedPhysics::SortedArray<HVec3>& verts(tree.GetVertice());
		SharedPhysics::SortedArray<Baking::KDTreeTriangle>& inds(tree.GetTriangles());
		const U32 endIndex = node->m_startIndex + node->m_sizeIndice;

		for(U32 a =node->m_startIndex; a < endIndex; ++a)
		{
			Baking::KDTreeTriangle& tri = inds[a];
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(verts[tri.m_IndexA],
				verts[tri.m_IndexB], verts[tri.m_IndexC], RGBA(255, 0, 255, 255));
		}
	}
	}


}*/

};
