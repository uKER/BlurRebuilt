#include "stdafx.h"
#include "gravity/exporter/GravityExporter.h"
#include "gravity/sharedphysics/sortedarray.h"
#include "gravity/exporter/ExportPrimitivesList.h"
//#include "gravity/exporter/ConvexHullDecomposition.h"
#include "gravity/exporter/GravityConverter.h"
#include "gravity/sharedphysics/GraphNode.h"
//#include "gravity/treebuilder/aabbtree.h"
#include <system/serialise/BinarySerialiseAutoEndian.h>


//#define WRITE_MESH_NODE_POINT
//#define DEBUG_PHYSICS
//#define WRITE_CH_DECOMPOSITION_MODEL


namespace Gravity
{

	C_ExportScene::~C_ExportScene()
	{
		for(S32 i=0; i<m_ShapeNodes.m_numElements; ++i)
		{
			if(m_ShapeNodes[i])
				delete m_ShapeNodes[i];
			m_ShapeNodes[i]=NULL;
		}

		for(S32 i=0; i<m_CompoundNodes.m_numElements; ++i)
		{
			if(m_CompoundNodes[i])
				delete m_CompoundNodes[i];
			m_CompoundNodes[i]=NULL;
		}


		for(S32 i=0; i<m_InstanceBaseNodes.m_numElements; ++i)
		{
			if(m_InstanceBaseNodes[i])
				delete m_InstanceBaseNodes[i];
			m_InstanceBaseNodes[i]=NULL;
		}
		for(S32 i=0; i<m_ConstraintNodes.m_numElements; ++i)
		{
			if(m_ConstraintNodes[i])
				delete m_ConstraintNodes[i];
			m_ConstraintNodes[i]=NULL;
		}

		for(S32 i=0; i<m_ClothNodes.m_numElements; ++i)
		{
			if(m_ClothNodes[i])
				delete m_ClothNodes[i];
			m_ClothNodes[i]=NULL;
		}

		for(S32 i=0; i<m_DeformableNodes.m_numElements; ++i)
		{
			if(m_DeformableNodes[i])
				delete m_DeformableNodes[i];
			m_DeformableNodes[i]=NULL;
		}


		for(S32 i=0; i<m_StaticFluidMeshNodes.m_numElements; ++i)
		{
			if(m_StaticFluidMeshNodes[i])
				delete m_StaticFluidMeshNodes[i];
			m_StaticFluidMeshNodes[i]=NULL;
		}

		for(S32 i=0; i<m_StaticMeshNodes.m_numElements; ++i)
		{
			if(m_StaticMeshNodes[i])
				delete m_StaticMeshNodes[i];
			m_StaticMeshNodes[i] = NULL;
		}

		/*if(m_pStaticShootableMeshNode)
				delete m_pStaticShootableMeshNode;
		if(m_pStaticCollidableMeshNode)
				delete m_pStaticCollidableMeshNode;*/
		if(m_pSplineMeshNode)
			delete m_pSplineMeshNode;
		/*m_pStaticShootableMeshNode = NULL;
		m_pStaticCollidableMeshNode = NULL;*/
		m_pSplineMeshNode = NULL;
	}

	C_InstanceBaseNode* C_ExportScene::GetInstanceBaseNode(U32 id)
	{
		for(S32 i=0; i<m_InstanceBaseNodes.m_numElements; ++i)
		{
			C_InstanceBaseNode* pBase=m_InstanceBaseNodes[i];
			if(pBase->m_A3dId == id)
			{
				return pBase;
			}
		}
		return NULL;
	}


	void C_ExportScene::Clear()
	{
		buildPrim.Clear();
		buildCons.Clear();
	}


	/*void DecompoisitionObjects(Gravity::C_TriangleMeshDesc* pMeshDesc)
	{
		C_ConvexHullDecomposition chDecomp;
		SharedPhysics::SortedArray<Maths::HVec3>& vertices = pMeshDesc->m_AABBTree->GetVertice();
		U32 size = vertices.m_numElements;
		chDecomp.m_Vertices.Resize(size);
		for(S32 a=0; a< size; ++a)
		{
			C_GraphVertex graphVertex;
			graphVertex.m_Position = vertices[a];
			chDecomp.m_Vertices.AddToEnd(&graphVertex);
		}

		const SharedPhysics::SortedArray<Baking::AABBBuildTreeTriangle*>& treeTriangles = pMeshDesc->m_AABBTree->GetTriangles();

		size = treeTriangles.m_numElements ;
		chDecomp.m_Edges.Resize(size * 3);
		for(S32 a = 0; a < size; ++a)
		{
			Baking::AABBBuildTreeTriangle* triangle = treeTriangles[a];
			Gravity::C_GraphEdge newEdge;
			newEdge.m_VertexAIndex = triangle->m_IndexA;
			newEdge.m_VertexBIndex = triangle->m_IndexB;
			chDecomp.m_Edges.AddToEnd(&newEdge);
			newEdge.m_VertexAIndex = triangle->m_IndexA;
			newEdge.m_VertexBIndex = triangle->m_IndexC;
			chDecomp.m_Edges.AddToEnd(&newEdge);
			newEdge.m_VertexAIndex = triangle->m_IndexB;
			newEdge.m_VertexBIndex = triangle->m_IndexC;
			chDecomp.m_Edges.AddToEnd(&newEdge);
		}

		for(S32 a=0; a < size; ++a)
		{
			Baking::AABBBuildTreeTriangle* triangle = treeTriangles[a];
			Gravity::C_GraphTriangle tri;
			tri.m_VertexA = triangle->m_IndexA;
			tri.m_VertexB = triangle->m_IndexB;
			tri.m_VertexC = triangle->m_IndexC;
			chDecomp.m_Triangles.AddToEnd(&tri);
		}


		SharedPhysics::C_S32SafeStack stack(16092);
		chDecomp.m_Edges.QuickSort(stack, 10);

		SharedPhysics::SortedArray<Gravity::C_GraphEdge> edges(size*3);
		

		for(S32 a = 0; a < chDecomp.m_Edges.m_numElements -1;)
		{
			Gravity::C_GraphEdge& edge = chDecomp.m_Edges[a++];
			edges.AddToEnd(&edge);
			while(a < chDecomp.m_Edges.m_numElements && edge == chDecomp.m_Edges[a] )
			{
				++a;
			}
		}

#ifdef WRITE_CH_DECOMPOSITION_MODEL
		{
				
			Btl::String filename;
			filename = Btl::String::Format("z:\\CHDecomposition.dat");
			Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(filename.Chars(), Sys::E_FileAccess_Write);

			if(pFile)
			{
				pFile->SetEndianSwapFlag( true );
				{
					Sys::BinarySerialiser<Ut::DoEndianSwap> ar(*pFile);

					S32 numVertices= chDecomp.m_Vertices.m_numElements;

					ar << numVertices;

					for(S32 a = 0; a < numVertices; ++a)
					{
						C_GraphVertex& vertex = chDecomp.m_Vertices[a];
						ar << vertex.m_Position;
					}

					S32 numEdges = edges.m_numElements;
					ar << numEdges;

					for(S32 a = 0; a<numEdges; ++a)
					{
						Gravity::C_GraphEdge& edge =edges[a];
						ar << edge.m_VertexAIndex << edge.m_VertexBIndex;
					}

					S32 numTriangles = chDecomp.m_Triangles.m_numElements;
					ar << numTriangles;
					for(S32 a = 0; a<numTriangles; ++a)
					{
						Gravity::C_GraphTriangle& tri =chDecomp.m_Triangles[a];
						ar << tri.m_VertexA << tri.m_VertexB << tri.m_VertexC;
					}
					

					SAFE_DELETE_PTR(pFile);
				}
			}		
		}
#endif

	}*/


#ifdef DEBUG_PHYSICS
#pragma optimize ("", off)
#endif
	bool C_ExportScene::Export(Sys::I_BinaryStream* pFile)
	{
		Sys::BinarySerialiserAutoEndian archive(*pFile);

		
		//Gravity::C_ExportConstraint buildCons; //Constraints
		Gravity::C_GravityConverter* convert=Gravity::C_GravityConverter::Instance();

		//Gravity::C_PhysicsExporter buildPrim; //Primitives

		
		/*
			Export all the static shapes
		*/

	
		for(S32 i=0; i<m_ShapeNodes.m_numElements; ++i)
		{
			C_ShapeNode* shapeNodes=m_ShapeNodes[i];
			convert->BuildShapeNode(buildPrim, shapeNodes);
		}

		/*
			Export all the compound nodes
		*/

		Maths::HMat43 mat;
		mat.SetIdentity();

		
		//static bool decompositionObject = true;
		for(S32 i=0;i<m_CompoundNodes.m_numElements; ++i)
		{
			C_CompoundNode* pCompound=m_CompoundNodes[i];
			SharedPhysics::SortedArray<Gravity::C_MeshNode* > meshList;

			Maths::HMat33 ineriaTensor;
			Gravity::C_CompositeDesc* pComposite=convert->BuildCompoundNode(pCompound, ineriaTensor, meshList);
			
			/*
				Compound object has mesh/instance mesh inside
			*/
			if(meshList.m_numElements > 0)
			{
				Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
				Gravity::C_TriangleMeshDesc* pMeshDesc=m_Exporter->CreateTriangleStaticMesh();
				pMeshDesc->m_OriginalExportID = meshList[0]->m_UniqueID;

				for(S32 j=0; j<meshList.m_numElements; ++j)
				{
					Gravity::C_MeshNode* pMeshNode=meshList[j];
					m_Exporter->AddTriangleMesh(*pMeshDesc->m_AABBTree, pCompound->m_Transform, pMeshNode->m_Tris);
				}
				pMeshDesc->m_AABBTree->setSplitRatio(0.2f);
				Maths::HF32 normThresh(0.7f);
				pMeshDesc->m_AABBTree->BuildAABBTree(normThresh);
				////Add to the decomposition object
				//if(decompositionObject)
				//{
				//	DecompoisitionObjects(pMeshDesc);
				//	decompositionObject = false;
				//}

			//	pMeshDesc->m_AABBTree->CompressAABBTreeData();
				
				U32 combinedCollision = pMeshDesc->GetCombinedCollisionFlags();
				pMeshDesc->m_CollisionFlag = combinedCollision;
			
				/*
					Mesh don't have ineriaTensor
				*/
				//const Maths::HMat33 meshIneriaTensor;
				pComposite->AddOwnShape(pMeshDesc, mat, Maths::HMat33(Maths::ZERO_MATRIX), Maths::Constants<Maths::HF32>::Zero());
			}
			
			buildPrim.AddCompositeObjects(pComposite, ineriaTensor, pCompound->m_Transform.Translation(), pCompound->m_Transform.Rotation(), pCompound->m_UserDataFlag);
		}

		/*
			Export all the static meshes
		*/
		/*buildPrim.CreateStaticMesh(mat.Translation() , mat.Rotation());
		for(S32 i=0; i<m_MeshNodes.m_numElements; ++i)
		{
			C_MeshNode* pMeshNode=m_MeshNodes[i];
			buildPrim.AddStaticMesh(pMeshNode->m_Tris);
		}
		buildPrim.BuildTree(0.2f, 0.7f);*/

		

		for(S32 i=0; i<m_StaticMeshNodes.m_numElements; ++i)
		{

			Gravity::C_MeshNode* pNode = m_StaticMeshNodes[i];
			if(pNode && pNode->m_Tris.m_numElements !=0)
			{
#ifdef WRITE_MESH_NODE_POINT
				{
					
					Btl::String filename;
					filename = Btl::String::Format("z:\\LevelMesh%i.dat, i");
					Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(filename.Chars(), Sys::E_FileAccess_Append );

					if(pFile)
					{
						pFile->SetEndianSwapFlag( true );
						{
							Sys::BinarySerialiser<Ut::DoEndianSwap> ar(*pFile);

							S32 numTriangles = m_pStaticShootableMeshNode->m_Tris.m_numElements;

							ar << numTriangles;

							for(U32 a = 0; a < numTriangles; ++a)
							{
								Gravity::C_ConverterTriangle* tri = m_pStaticShootableMeshNode->m_Tris[a];
								ar << tri->vertexA << tri->vertexB << tri->vertexC;
							}

							SAFE_DELETE_PTR(pFile);
						}
					}		
				}
#endif
				buildPrim.CreateStaticMesh(pNode->m_UniqueID, mat.Translation() , mat.Rotation(), pNode->m_PhysicsMaterialIndex, pNode->m_EffectMaterialIndex, pNode->m_CollisionFlag, pNode->m_NodeFlag, pNode->m_Tris);
			}
		}

		buildPrim.BuildTree();


		if( m_pSplineMeshNode && m_pSplineMeshNode->m_Tris.m_numElements != 0)
		{
			buildPrim.CreateStaticSplineMesh(m_pSplineMeshNode->m_UniqueID, mat.Translation() , mat.Rotation(), m_pSplineMeshNode->m_PhysicsMaterialIndex, m_pSplineMeshNode->m_EffectMaterialIndex, m_pSplineMeshNode->m_CollisionFlag, m_pSplineMeshNode->m_NodeFlag, m_pSplineMeshNode->m_Tris);
			buildPrim.BuildSplineTree();
		}

		/*
			Export all the instance meshes
		*/
		for(S32 i=0; i<m_InstanceBaseNodes.m_numElements; ++i)
		{
			C_InstanceBaseNode* pBaseNode=m_InstanceBaseNodes[i];
			BIZ_ASSERT(pBaseNode->m_pMesh->m_Tris.m_numElements != 0);
			//buildPrim.AddInstanceBaseNode(pBaseNode->m_UniqueID, 0.2f, 0.7f,  mat.Translation(), mat.Rotation(), pBaseNode->m_pMesh->m_Tris);
			const U32 collisionFlag = buildPrim.AddInstanceBaseNode(pBaseNode->m_UniqueID, 0.2f, 0.7f,  pBaseNode->m_Transform.Translation(), pBaseNode->m_Transform.Rotation(), pBaseNode->m_pMesh->m_Tris);
			

			for(S32 i=0; i<pBaseNode->m_InstanceNodes.m_numElements; ++i)
			{
				
				C_InstanceNode* pInstance=pBaseNode->m_InstanceNodes[i];
				BIZ_ASSERT(pInstance->m_UniqueID !=0);
				buildPrim.AddInstance(pInstance->m_UniqueID, pInstance->m_Transform.Translation(), pInstance->m_Transform.Rotation(), collisionFlag, pInstance->m_BaseNodeID);
			}
		}

		//Export the constraint;
		for(S32 i=0; i< m_ConstraintNodes.m_numElements; ++i)
		{
			C_ConstraintNode* pCons=m_ConstraintNodes[i];
			convert->BuildConstraintNode(buildCons, pCons);
		}

		//Export Cloths
		for(S32 i=0; i<m_ClothNodes.m_numElements; ++i)
		{
			C_ClothNode* pCloth=m_ClothNodes[i];
			buildPrim.AddCloth(pCloth->m_UniqueID, pCloth->m_Transform.Translation(), pCloth->m_Transform.Rotation(), pCloth->m_PhysicsMaterialIndex, pCloth->m_EffectMaterialIndex, pCloth->m_CollisionFlag, pCloth->m_NodeFlag, pCloth->m_Tris);
		}

		//Export Deformable object
		for(S32 i=0; i<m_DeformableNodes.m_numElements; ++i)
		{
			C_DeformableNode* pDeformable = m_DeformableNodes[i];
			buildPrim.AddDeformable(pDeformable->m_UniqueID, pDeformable->m_Transform.Translation(), pDeformable->m_Transform.Rotation(), pDeformable->m_PhysicsMaterialIndex, pDeformable->m_EffectMaterialIndex, pDeformable->m_CollisionFlag, pDeformable->m_NodeFlag, pDeformable->m_Tris);
		}

		//Export static fluid mesh
		for(S32 i=0; i<m_StaticFluidMeshNodes.m_numElements; ++i)
		{
			C_FluidMeshNode* pFluidMesh=m_StaticFluidMeshNodes[i];
			buildPrim.AddFluidMesh(pFluidMesh->m_UniqueID, pFluidMesh->m_Depth, 0.2f, 0.7, pFluidMesh->m_PhysicsMaterialIndex, pFluidMesh->m_EffectMaterialIndex, pFluidMesh->m_NodeFlag, pFluidMesh->m_Transform.Translation(), pFluidMesh->m_Transform.Rotation(), pFluidMesh->m_pMesh->m_Tris);
		}

	
		buildPrim.ExportHeader(archive);
		buildPrim.Export(archive);

		archive << 0x0fffffff;
		buildCons.ExportConstraint<Sys::BinarySerialiserAutoEndian>(archive);

		archive << 0xffffffff; //The end of primitives and mesh 

		Clear();

		return true;
	}
#ifdef DEBUG_PHYSICS
#pragma optimize ("", on)
#endif
}
