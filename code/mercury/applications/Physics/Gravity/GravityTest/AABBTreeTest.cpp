#include <stdafx.h>
#include <system/scheduler/jobqueue.h>
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/sharedphysics/SortedArray.inl"
#include "SectionPhysics.h"

#include "gravity/narrowphase/OptimisedAABBTree.h"
#include "gravity/narrowphase/OptimisedAABBTree.inl"
#include "gravity/treebuilder/AABBTree.inl"


#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
//#include <Render/Shaders/World/psWorld.h>
//#include <Render/Shaders/World/vsWorld.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include "gravity/sharedphysics/AllocationManager.h"
#include "gravity/sharedphysics/AllocationManager.inl"
#include "SegmentSetting.h"
#include "SphereSetting.h"
#include "gravity/narrowphase/RayTriangle.h"
#include "gravity/narrowphase/RayTriangle.inl"
#include "gravity/narrowphase/SphereTriangle.h"
#include "gravity/narrowphase/SphereTriangle.inl"
#include "gravity/narrowphase/OBBTriangle.h"
#include "gravity/narrowphase/OBBTriangle.inl"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/Shapes.h"
#include "OBBSetting.h"
#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/sharedphysics/SafeStack.h"
#include "core/maths/hvec/VectorInt.h"
#include "core/maths/hvec/VectorInt.inl"
#include "gravity/sharedphysics/ContactStreamIterator.inl"
#include "Samples/System/View.h"
#include "Samples/System/Application.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/CollisionFilter.inl"




using namespace Baking;
using namespace Collision;

namespace Samples
{

Debug::Bool g_DrawTrianglesAABB("Test.DrawAABBTriangles", true);

void C_SectionPhysics::TestBuildAABBTree()
{
	/*C_World* world=this->m_Level.m_pPhysicsWorld;
	const C_ImportUnit* pImportUnit = world->PersistentImportUnit();
	const U32 numVertices = pImportUnit->m_CompressedVertexCount + pImportUnit->m_UncompressedVertexCount;
	Baking::AABBTree tree; 
	//Loop through the prim mesh objects in the import unit and add them to the AABB tree

	const U32 numMeshes = pImportUnit->m_PrimMeshArray.Capacity();
	//const C_PrimMesh* pMeshes = pImportUnit->m_PrimMeshArray.Data();
	U32 totalVertexCount = 0;
	const C_StaticBodyKdTreeObject* __restrict pObjectArray = pImportUnit->StaticKdTreeObjectArray();
	const U32 maxMeshes = 1;
	const U32 count = (maxMeshes > pImportUnit->StaticKdTreeObjectCount()) ? pImportUnit->StaticKdTreeObjectCount() : maxMeshes;
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
				Collision::AABBTreeTriangle tri;
				tri.m_IndexA = indexA + totalVertexCount;
				tri.m_IndexB = indexB + totalVertexCount;
				tri.m_IndexC = indexC + totalVertexCount;
				tri.m_MaterialIndex = 0; //TODO - make this actually mean something!!!
				tree.AddIndex(tri);
			}
			totalVertexCount += pPrimMesh->Vertices();
		}
	}
	tree.setSplitRatio(0.05f);
	Maths::HF32 normThresh(0.7f);
	tree.BuildAABBTree(normThresh);
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


	C_Application* g_App=GetApp();
	const C_View* view = g_App->GetView(0);
	const C_Camera& camera=view->m_Camera;
	Maths::HVec3 position(camera.m_Eye);
	Collision::C_CRAxisBox box;
	box.centre = position;
	box.extents = Maths::HVec3(10.f, 10.f, 10.f);

	DrawAABBTreeNode(tree.GetRoot(), tree, 0, maxDepth, box);

	{
		Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "test.dat" , Sys::E_FileAccess_Write);
		
		Sys::BinarySerialiser<Ut::NativeToBigEndian> archive(*filePtr);
		//Sys::TextSerialiser archive(*filePtr);

		tree.Serialise(archive);

		delete filePtr;
	}

	Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "test.dat" );
	Sys::BinaryDeserialiser<Ut::BigEndianToNative> archive(*filePtr);
	OptimisedAABBTree optTree;
	optTree.DeSerialise(archive);
	delete filePtr;*/
}

void C_SectionPhysics::TestAABBTreeRay()
{
		
	static bool initialised = false;
	if(!initialised)
	{
		BuildAABBTree();
		initialised = true;
	}
	//DrawAABBTreeNode(m_tree.GetRoot(), m_tree, 0, 8);
	C_Ray ray;
	ray.m_start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	ray.m_end=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));
	ray.m_result.m_contact=ray.m_end;

	SharedPhysics::C_SmallSPUCache spuCache;

	SharedPhysics::C_SafeStack<true> stack(1024);
	bool collide=m_optTree.RayTracing<Collision::CollideRayTriangleMesh<Collision::AABBTreeTriangle> >(ray, stack, spuCache);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_result.m_contact, RGBA(255, 255, 0, 255));

	if(collide)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_result.m_contact, RGBA(0, 0, 255, 255));
	}
}

void C_SectionPhysics::TestAABBTreeSphere()
{
		
	/*static bool initialised = false;
	if(!initialised)
	{
		BuildAABBTree();
		initialised = true;
	}

	C_Application* g_App=GetApp();
	const C_View* view = g_App->GetView(0);
	const C_Camera& camera=view->m_Camera;
	Maths::HVec3 position(camera.m_Eye);
	Collision::C_CRAxisBox box;
	box.centre = position;
	box.extents = Maths::HVec3(10.f, 10.f, 10.f);

	DrawAABBTreeNode(m_tree.GetRoot(), m_tree, 0, MAX_DEPTH, box);
	Collision::C_CollisionObject obj;
	C_DefaultCollisionFilter filter;
	C_SphereShape shape(&filter);
	shape.m_Sphere.Centre(HVec3(HF32(sX), HF32(sY), HF32(sZ)));
	shape.m_Sphere.Radius(Maths::HF32(r));
	obj.m_pShape=&shape;
	obj.m_TransformedBox.centre = shape.m_Sphere.Centre();
	obj.m_TransformedBox.extents = Maths::HVec3(Maths::HF32(r));

	SharedPhysics::C_SafeStack<true> stack(1024);
	Collision::C_CollisionResult<4, 16>res;
	bool collide=m_optTree.Collide<4, 16, CollideSphereTriangleMesh<4, 16, Collision::AABBTreeTriangle> >(&obj, res, stack);
	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(shape.m_Sphere.Centre(), shape.m_Sphere.Radius(), RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(shape.m_Sphere.Centre(), 0.5f, RGBA(255, 0, 0, 255));

	if(collide)
	{
		for(U32 i=0; i<res.m_NumContactPatches; ++i)
		{
			Collision::C_ContactPatch<16>& patch=res.m_ContactPatches[i];
			for(U32 j=0; j<patch.m_NumContactPoints; ++j)
			{
				Collision::C_ContactPoint& point=patch.m_ContactPoints[j];
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.5f, RGBA(0, 0, 255, 255));
			}
		}
	}*/
}

void C_SectionPhysics::TestAABBTreeOBB()
{
	/*static bool initialised = false;
	if(!initialised)
	{
		BuildAABBTree();
		initialised = true;
	}

	C_Application* g_App=GetApp();
	const C_View* view = g_App->GetView(0);
	const C_Camera& camera=view->m_Camera;
	Maths::HVec3 position(camera.m_Eye);
	Collision::C_CRAxisBox box;
	box.centre = position;
	box.extents = Maths::HVec3(10.f, 10.f, 10.f);
	DrawAABBTreeNode(m_tree.GetRoot(), m_tree, 0, MAX_DEPTH, box);
	Collision::C_CollisionObject obj;
	C_DefaultCollisionFilter filter;
	C_BoxShape shape(&filter);

	shape.m_Box.c=HVec3(HF32(obb_cX), HF32(obb_cY), HF32(obb_cZ));
	shape.m_Box.e=HVec3(HF32(obb_eX), HF32(obb_eY), HF32(obb_eZ));
	shape.m_Box.u.SetIdentity();

	const F32 smll(0.02f);
	const HF32 small (Load<HF32>(&smll));
	static HF32 rot=Maths::Constants<HF32>::Zero();
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=small;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=small;
	}

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	shape.m_Box.u=matra;

	Collision::C_CRAxisBox origBox;
	origBox.centre = Maths::HVec3(Maths::Constants<Maths::HVec3>::Zero());
	origBox.extents = shape.m_Box.e;

	SharedPhysics::C_GraphNode nodeA;
	SharedPhysics::C_GraphNode nodeB;

	Maths::HMat43 u(shape.m_Box.u, shape.m_Box.c);
	origBox.Transform(u, obj.m_TransformedBox);
	obj.m_pShape=&shape;
	obj.m_pNodePtr = &nodeA;

	C_CollisionObject objB;
	objB.m_pNodePtr = &nodeB;	

	const U32 numPatches = 4;
	const U32 numContacts = 64;
	Collision::C_CollisionResult<numPatches, numContacts>res;

	res.m_pObjectA = &obj;
	res.m_pObjectB = &objB;

	SharedPhysics::C_SafeStack<true> stack(1024);
	SharedPhysics::C_ContactStream stream(6400000);
	//for(U32 a = 0; a < 1000; ++a)
	{
		res.m_NumContactPatches = 0;
		for(U32 b = 0; b < 4; ++b)
			res.m_ContactPatches[b].m_NumContactPoints = 0;

		bool collide=m_optTree.Collide<numPatches, numContacts, CollideOBBTriangleMesh<numPatches, numContacts, Collision::AABBTreeTriangle> >(&obj, res, stack);
		Collision::ReduceContactsAndPackToStream<numPatches, numContacts, 3, 4>(res, stream);
	}
	
	const Maths::HVec3 min(-shape.m_Box.e);
	const Maths::HVec3 max(shape.m_Box.e);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(min, max, u, RGBA(255, 0, 0, 255));

	SharedPhysics::C_ContactStreamIterator iter(&stream);
	while(iter.hasNextContact())
	{
		SharedPhysics::C_ContactStreamResult* result(iter.nextContact());
		while(iter.hasNextPatch())
		{
			SharedPhysics::C_ContactStreamPatch* patch(iter.nextPatch());
			const Maths::HVec3 norm(Maths::Load<Maths::HVec3>(&patch->m_Normal));
			while(iter.hasNextPoint())
			{
				SharedPhysics::C_ContactStreamPoint* point(iter.nextPoint());
				const Maths::HF32 penDepth(Maths::Load<Maths::HF32>(&point->m_PenetrationDepth));
				const Maths::HVec3 contact(Maths::Load<Maths::HVec3>(&point->m_ContactPoint));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.5f, RGBA(0, 0, 255, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact + penDepth * norm, RGBA(0, 0, 255, 255));
			}
		}
	}

	//if(collide)
	//{
	//	for(U32 i=0; i<res.m_NumContactPatches; ++i)
	//	{
	//		Collision::C_ContactPatch<numContacts>& patch=res.m_ContactPatches[i];
	//		for(U32 j=0; j<patch.m_NumContactPoints; ++j)
	//		{
	//			Collision::C_ContactPoint& point=patch.m_ContactPoints[j];
	//			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.5f, RGBA(0, 0, 255, 255));
	//		}
	//	}
	//}
	*/

}

void C_SectionPhysics::BuildAABBTree()
{
	/*using namespace Physics;
	C_World* world=this->m_Level.m_pPhysicsWorld;
	const C_ImportUnit* pImportUnit = world->PersistentImportUnit();
	const U32 numVertices = pImportUnit->m_CompressedVertexCount + pImportUnit->m_UncompressedVertexCount;
	//Baking::AABBTree tree; 
	//Loop through the prim mesh objects in the import unit and add them to the AABB tree

	const U32 numMeshes = pImportUnit->m_PrimMeshArray.Capacity();
	//const C_PrimMesh* pMeshes = pImportUnit->m_PrimMeshArray.Data();
	U32 totalVertexCount = 0;
	const C_StaticBodyKdTreeObject* __restrict pObjectArray = pImportUnit->StaticKdTreeObjectArray();
	const U32 maxMeshes = 1;
	//const U32 count = (maxMeshes > pImportUnit->StaticKdTreeObjectCount()) ? pImportUnit->StaticKdTreeObjectCount() : maxMeshes;
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

			//Maths::C_Vertex* pVertices = (Maths::C_Vertex*)_alloca(sizeof(C_Vertex) * pPrimMesh->Vertices() + 255);
			Maths::C_Vertex* pVertices = new C_Vertex[pPrimMesh->Vertices() + 255];
			pPrimMesh->DecompressVertexArray(pVertices, transform);
			m_tree.AddVertices((Maths::HVec3*)pVertices, pPrimMesh->Vertices());
			const U32 numTriangles = pPrimMesh->Triangles();
			const C_PrimMeshTriangle* pTriangle = pPrimMesh->TriangleArray();
			for(U32 b = 0; b < numTriangles; ++b, pTriangle++)
			{
				const U32 indexA(pTriangle->VertexIndex<0>());
				const U32 indexB(pTriangle->VertexIndex<1>());
				const U32 indexC(pTriangle->VertexIndex<2>());
				Collision::AABBTreeTriangle tri;
				tri.m_IndexA = indexA + totalVertexCount;
				tri.m_IndexB = indexB + totalVertexCount;
				tri.m_IndexC = indexC + totalVertexCount;
				tri.m_MaterialIndex = 0;
				m_tree.AddIndex(tri);
			}
			totalVertexCount += pPrimMesh->Vertices();
			delete [] pVertices;
		}
	}
	m_tree.setSplitRatio(0.2f);

	Maths::HF32 normThresh(0.7f);
	m_tree.BuildAABBTree(normThresh);


	//DrawAABBTreeNode(tree.GetRoot(), tree, 0, 8);

	//
	{
		Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "test.dat" , Sys::E_FileAccess_Write);
		Sys::BinarySerialiser<Ut::NativeToBigEndian> archive(*filePtr);
		m_tree.Serialise(archive);
		delete filePtr;
	}


	//
	Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "test.dat" );
	Sys::BinaryDeserialiser<Ut::NativeToBigEndian> archive(*filePtr);
	m_optTree.DeSerialise(archive);
	delete filePtr;

	//Test & validate the tree - is it the same?

	SharedPhysics::C_SafeStack<true> stack1(1024);
	SharedPhysics::C_SafeStack<true> stack2(1024);

	S32 size = 1;
	stack1.Push(m_tree.GetRoot(), 0);
	stack2.Push(m_optTree.m_pNodes, 0);

	//Loop through and check the triangles and vertices...
	SharedPhysics::SortedArray<Collision::AABBTreeTriangle>& tris(m_tree.GetIndice());
	assert(tris.m_numElements == m_optTree.m_NumTriangles);
	for(S32 a = 0; a < tris.m_numElements; ++a)
	{
		Collision::AABBTreeTriangle& tri1(tris[a]);
		Collision::AABBTreeTriangle& tri2(m_optTree.m_pTriangles[a]);
		assert(tri1.m_IndexA == tri2.m_IndexA);
		assert(tri1.m_IndexB == tri2.m_IndexB);
		assert(tri1.m_IndexC == tri2.m_IndexC);
		assert(tri1.m_MaterialIndex == tri2.m_MaterialIndex);
		assert(tri1.m_NormalIndex == tri2.m_NormalIndex);
	}

	SharedPhysics::SortedArray<Maths::HVec3>& verts(m_tree.GetVertice());
	for(S32 a = 0; a < verts.m_numElements; ++a)
	{
		Maths::HVec3 optVert(Maths::Load<Maths::HVec3>(&m_optTree.m_pVertices[a]));
		assert(optVert.All_EQ(verts[a]));
	}

	while(size--)
	{
		Baking::AABBTreeNode* node(stack1.Pop<Baking::AABBTreeNode>(size));
		Collision::OptimisedAABBNode* optNode(stack2.Pop<Collision::OptimisedAABBNode>(size));

		//Assert that these are the same nodes...
		assert(node->aabb.m_min.All_EQ(optNode->m_BoundingBox.m_min));
		assert(node->aabb.m_max.All_EQ(optNode->m_BoundingBox.m_max));
		if(node->m_leftNode == NULL)
		{
			assert(node->m_startIndex == optNode->m_StartIndex);
			assert(node->m_sizeIndice == optNode->m_NumIndices);
		}
		else
		{
			assert(optNode->m_NumIndices == 0);
			//Push the left and right nodes...
			stack1.Push(node->m_rightNode, size);
			stack2.Push(optNode + optNode->m_RightChildOffset, size++);
			stack1.Push(node->m_leftNode, size);
			stack2.Push(optNode + 1, size++);
		}
	}*/
}

void C_SectionPhysics::DrawAABBTreeNode(AABBTreeNode* node, AABBTree& tree, U32 depth, U32 maxDepth,
										const Collision::C_CRAxisBox& box)
{	
	/*Maths::HMat33 m(Maths::UNDEFINED);
	m.SetIdentity();

	C_AxisBoxMinMax& aabb=node->aabb;
	const Maths::HVec3 half(Maths::Constants<HVec3>::Half());
	const HMat43 mat(m, HVec3());

	const Maths::RGBA red(255, 0, 0);

	if(!node->aabb.Intersects(box))
		return;
	
	
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(aabb.m_min, aabb.m_max, mat, triColour);
	if(node->m_leftNode)
	{
		if(maxDepth > 0)
		{
			DrawAABBTreeNode(node->m_leftNode, tree, depth+1, maxDepth-1, box);
			DrawAABBTreeNode(node->m_rightNode, tree, depth+1, maxDepth-1, box);
		}
	}
	else
	{	
		const Maths::HF32 third(Maths::Constants<Maths::HF32>::Three().Reciprocal());
		const U32 endIndex = node->m_startIndex + node->m_sizeIndice;
		const Maths::RGBA triColour((node->m_startIndex << 16) | endIndex | 255);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(aabb.m_min, aabb.m_max, mat, red);
		if(g_DrawTrianglesAABB)
		{
			SharedPhysics::SortedArray<HVec3>& verts(tree.GetVertice());
			SharedPhysics::SortedArray<HVec3>& normals(tree.GetNormals());
			SharedPhysics::SortedArray<Baking::AABBBuildTreeTriangle>& inds(tree.GetTriangles());
			
			for(U32 a =node->m_startIndex; a < endIndex; ++a)
			{
				Baking::AABBBuildTreeTriangle& tri = inds[a];
				const Maths::HVec3 vertA(verts[tri.m_IndexA]);
				const Maths::HVec3 vertB(verts[tri.m_IndexB]);
				const Maths::HVec3 vertC(verts[tri.m_IndexC]);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(vertA,
					vertB, vertC, triColour);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow((vertA + vertB + vertC) * third, normals[tri.m_NormalIndex],
					0.5f, 0.1f, triColour);

			}
		}
	}*/


}

};
