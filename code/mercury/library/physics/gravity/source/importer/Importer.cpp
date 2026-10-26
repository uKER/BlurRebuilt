#include "Stdafx.h"
#include "gravity/broadphase/DistributionTree.h"
#include "system/serialise/I_FileSystem.h"
#include "system/serialise/I_BinaryStream.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/importer/importer.h"
#include "gravity/sharedphysics/PhysicsVersion.h"
#include <system/DebugLog.h>
#include "gravity/sdk/Scene.h"
#include "gravity/sdk/PhysicsSDK.h"

#include "gravity/sdk/Body.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sdk/ConstraintDescriptors.h"
#include "gravity/sdk/CompositeDescriptor.h"
#include "gravity/narrowphase/Shapes.h"
#include <system/memory/memorytracker.h>

//#include <system/Memory.h>
//


namespace Gravity
{


	bool C_ImportBodyWrapper::operator < (const C_ImportBodyWrapper& other) const
	{
		//return m_pBody->GetOriginalExportBodyId()< other.m_pBody->GetOriginalExportBodyId();
		return m_pBody->GetConstraintBodyId()< other.m_pBody->GetConstraintBodyId();
	}
	bool C_ImportBodyWrapper::operator <= (const C_ImportBodyWrapper& other) const
	{
		//return m_pBody->GetOriginalExportBodyId() <= other.m_pBody->GetOriginalExportBodyId();
		return m_pBody->GetConstraintBodyId()<= other.m_pBody->GetConstraintBodyId();
	}
	bool C_ImportBodyWrapper::operator > (const C_ImportBodyWrapper& other) const
	{
		//return m_pBody->GetOriginalExportBodyId() > other.m_pBody->GetOriginalExportBodyId();
		return m_pBody->GetConstraintBodyId() > other.m_pBody->GetConstraintBodyId();
	}
	bool C_ImportBodyWrapper::operator >= (const C_ImportBodyWrapper& other) const
	{
		//return m_pBody->GetOriginalExportBodyId() >= other.m_pBody->GetOriginalExportBodyId();
		return m_pBody->GetConstraintBodyId() >= other.m_pBody->GetConstraintBodyId();
	}
	bool C_ImportBodyWrapper::operator == (const C_ImportBodyWrapper& other) const
	{
		//return m_pBody->GetOriginalExportBodyId() == other.m_pBody->GetOriginalExportBodyId();
		return m_pBody->GetConstraintBodyId() == other.m_pBody->GetConstraintBodyId();
	}
	bool C_ImportBodyWrapper::operator != (const C_ImportBodyWrapper& other) const
	{
		//return m_pBody->GetOriginalExportBodyId() != other.m_pBody->GetOriginalExportBodyId();
		return m_pBody->GetConstraintBodyId() != other.m_pBody->GetConstraintBodyId();
	}

	// create logger
	Debug::Logger &g_Log = Debug::GetLogger("gravity.import");

	C_ImportSceneStaticInstance::C_ImportSceneStaticInstance( C_Scene* pScene, C_ImportScene* pImportScene, 
		const Maths::HVec3 PCREF desiredSize, const Maths::HVec3 PCREF maxCellSize, Maths::HF32 PCREF fluidDepth) : 
	m_DesiredSize(desiredSize), m_MaxCellSize(maxCellSize), m_FluidDepth(fluidDepth)
	{
		MEMORY_TRACKER_PUSH("gravity.m_pPhysicsStaticInstance");
		m_FluidDepth = fluidDepth;
		m_pImportScene=pImportScene;
		pImportScene->m_pStaticScene = this;
		m_pScene=pScene;
		m_ExternalConstants=new Gravity::C_ExternalConstants;
		m_ExternalConstants->m_Gravity = Maths::HVec3(0, -9.8f, 0);
		m_DesiredSize = desiredSize;
		m_MaxCellSize = maxCellSize;
		Import();
		MEMORY_TRACKER_POP("gravity.m_pPhysicsStaticInstance");
	}

	C_ImportSceneStaticInstance::C_ImportSceneStaticInstance( C_Scene* pScene, C_ImportScene* pImportScene, const Maths::HVec3 PCREF desiredSize,
		const Maths::HVec3 PCREF maxCellSize, const Collision::C_CollisionFilter* pFilter, const Collision::C_CollisionFilter* pFluidFilter, Maths::HF32 PCREF fluidDepth) : 
	m_DesiredSize(desiredSize), m_MaxCellSize(maxCellSize), m_FluidDepth(fluidDepth)
	{
		MEMORY_TRACKER_PUSH("gravity.m_pPhysicsStaticInstance");
		m_FluidDepth = fluidDepth;
		m_pImportScene=pImportScene;
		pImportScene->m_pStaticScene = this;
		m_pScene=pScene;
		m_ExternalConstants=new Gravity::C_ExternalConstants;
		m_Filter = *pFilter;
		m_FluidFilter = *pFluidFilter;
		m_ExternalConstants->m_Gravity = Maths::HVec3(0, -9.8f, 0);
		m_DesiredSize = desiredSize;
		m_MaxCellSize = maxCellSize;
		Import();
		MEMORY_TRACKER_POP("gravity.m_pPhysicsStaticInstance");
	}


	// When this object is destroyed, the static parts that were added to the C_Scene in the constructor will be removed from the C_Scene.
	C_ImportSceneStaticInstance::~C_ImportSceneStaticInstance()
	{
		if( (m_pImportScene != NULL) && (m_pScene != NULL) )
		{
			//m_pImportScene->m_pStaticScene = NULL;
			m_pImportScene->RemoveStaticShapes();
		}
		delete m_ExternalConstants;
		//delete m_Filter;		
	}

	C_ImportScene::C_ImportScene()
	{
		m_ShareData = C_PhysicsSDK::PhysicsSDK()->GetShareVertexData();
	}


	C_ImportScene::~C_ImportScene()
	{
		for(S32 i=0; i<m_DynamicShapes.m_numElements; ++i)
		{
			if(m_DynamicShapes[i])
				delete m_DynamicShapes[i];
			m_DynamicShapes[i]=NULL;
		}

		for(S32 i=0; i<m_ClothShapes.m_numElements; ++i)
		{
			if(m_ClothShapes[i])
				delete m_ClothShapes[i];
			m_ClothShapes[i]=NULL;
		}

		for(S32 i=0; i<m_DeformableShapes.m_numElements; ++i)
		{
			if(m_DeformableShapes[i])
				delete m_DeformableShapes[i];
			m_DeformableShapes[i]=NULL;
		}

		for(S32 i=0; i<m_FluidMeshShapes.m_numElements; ++i)
		{
			if(m_FluidMeshShapes[i])
				delete m_FluidMeshShapes[i];
			m_FluidMeshShapes[i]=NULL;
		}
		
		for(S32 i=0; i<m_StaticShapesDesc.m_numElements; ++i)
		{
			if(m_StaticShapesDesc[i])
				delete m_StaticShapesDesc[i];
			m_StaticShapesDesc[i]=NULL;
		}

		for(S32 i=0; i<m_ConstraintsDesc.m_numElements; ++i)
		{
			if(m_ConstraintsDesc[i])
				delete m_ConstraintsDesc[i];
			m_ConstraintsDesc[i]=NULL;
		}

		for(S32 i=0; i<m_StaticInstance.m_numElements; ++i)
		{
			if(m_StaticInstance[i])
				delete m_StaticInstance[i];
			m_StaticInstance[i]=NULL;
		}
	}


	void C_ImportScene::RemoveStaticShapes()
	{
		C_Scene* pScene = m_pStaticScene->m_pScene;

		for(S32 i=0; i<m_pStaticScene->m_pBodies.m_numElements; ++i)
		{
			if(m_pStaticScene->m_pBodies[i])
			{
				Gravity::C_ShapeDesc* pDesc=m_StaticShapesDesc[i];
				if(pDesc->m_Type == Collision::E_TriangleMesh_Shape)
				{
					pScene->RemoveMeshBody(m_pStaticScene->m_pBodies[i]);
				}
				else
				{
					pScene->RemoveStaticBody(m_pStaticScene->m_pBodies[i]);
				}
			}
		}

		m_pStaticScene->m_pBodies.m_numElements = 0;
		
		m_pStaticScene=NULL;
	}
	

	Gravity::C_ShapeDesc* C_ImportScene::GetShapeDesc(U32 m_Type)
	{
		switch(m_Type)
		{
			case Collision::E_ImportSphere_Shape:
				{
					return new C_SphereDesc;
				}
			case Collision::E_ImportBox_Shape:
				{
					return new C_BoxDesc;
				}
			case Collision::E_ImportConvexHull_Shape:
				{
					return new C_ConvexHullDesc(m_ShareData);
				}
			case Collision::E_ImportCapsule_Shape:
				{
					return new C_CapsuleDesc;
				}
			case Collision::E_ImportComposite_Shape:
				{
					return new C_CompositeDesc;
				}
			case Collision::E_ImportCylinder_Shape:
				{
					return new C_CylinderDesc;
				}
			case Collision::E_ImportTriangleMesh_Shape:
				{
					return new C_TriangleMeshDesc;
				}
			case Collision::E_ImportTriangleMeshBase_Shape:
				{
					return new C_TriangleMeshBaseDesc;
				}
			case Collision::E_ImportInstancedTriangleMesh_Shape:
				{
					return new C_InstancedTriangleMeshDesc;
				}
			case Collision::E_ImportCloth_Shape:
				{
					return new C_ClothDesc;
				}
			case Collision::E_ImportCone_Shape:
				{
					return new C_ConeDesc;
				}
			case Collision::E_ImportFluidMesh_Shape:
				{
					return new C_FluidMeshDesc;
				}
			case Collision::E_ImportDeformable_Shape:
				{
					return new C_DeformableDesc;
				}
			default:
				return NULL;
		}
	}
	
	bool CmpString(const char* a, const char* b, const U32 size)
	{
		bool bMatch = true;
		for(U32 i = 0; i < size; ++i)
		{
			bMatch = bMatch && a[i] == b[i];
		}
		return bMatch;
	}

#ifndef CFG_SPEED
#pragma optimize("", off)
#endif
	// This loads the scene file into memory but does not create any shapes or bodies or touch the C_Scene in any way.
    //(except it may store internally a shape for the static mesh).
	bool C_ImportScene::Import( Sys::I_BinaryStream* filePtr )
	{

//		MEMORY_TRACKER_PUSH("Gravity.Importer.ImportScene");
		BIZ_ASSERT(filePtr);
		Deserialiser archive(*filePtr);
		Collision::C_CollisionFilter m_Filter;

		
		U32 header;
		archive >> header;

		char* head = (char*)&header;
		U32 m_Type;
		//if(_stricmp(head, "Hed") == 0) //Gravity current header
		S32 version = 0;
		if(CmpString(head, "Head", 4))
		{
			archive >> version;
			if(version != GRAVITY_CURRENT_VERSION)
				LOGWARN(g_Log, "Gravity data version mismatch while Loading %s (Data Version = %d, Code Version = %d)", filePtr->Name().Chars(), version, GRAVITY_CURRENT_VERSION);
			archive >> m_Type;
		}
		else
		{
			LOGWARN(g_Log, "Error - unversioned file loaded!");
			m_Type = header;
		}


		U32 oldVersionNo(filePtr->GetVersionNumber());
		filePtr->SetVersionNumber(version);

		//Btl::String name = filePtr->Name();

	/*	U32 m_Type;
		archive >> m_Type;*/
		
		const Maths::HF32 base(0.5f);
		const Maths::HVec3 p(13, 2, -5);
		   
		U32 count=0;
		while(m_Type!=0x0fffffff) // the end of primitives and mesh
		{
			
			Gravity::C_ShapeDesc* pShapeDesc=GetShapeDesc(m_Type);
			if(pShapeDesc == NULL)
				continue;
			switch(pShapeDesc->m_Type)
			{
			case Collision::E_TriangleMeshBase_Shape:
				{
					Gravity::C_TriangleMeshBaseDesc* triDesc = (Gravity::C_TriangleMeshBaseDesc*) pShapeDesc;
					S32 m_Id;
					if(version < 4)
					{
						archive >> m_Id;
					}
					else
					{
						triDesc->ImportBasic(archive);
						m_Id = triDesc->m_OriginalExportID;
						triDesc->m_RemapExportID = triDesc->m_OriginalExportID;
					}
					
					Collision::C_TriangleMeshShape* tri=(Collision::C_TriangleMeshShape*)triDesc->CreateShape(archive, &m_Filter);

					Maths::HVec3 position;
					Maths::HMat33 rotation;
					Maths::HMat33 ineriaTensor;
					archive >> position >> rotation >> ineriaTensor;

				//	assert(tri->m_OptimisedTree.m_NumTriangles > 0);
					
					m_StaticInstanceManager.AddBase(tri, m_Id);

					delete triDesc;
					
					break;
				}
			case Collision::E_Composite_Shape:
				{
					//DebugBreak();
					//MEMORY_TRACKER_PUSH("gravity.m_CompositeShape");
					Gravity::C_CompositeDesc* pDesc =(Gravity::C_CompositeDesc*) pShapeDesc;
					pDesc->m_ShareData = m_ShareData;
					pDesc->SetMeshManager(&m_StaticInstanceManager);
	
					bool succeeded = pDesc->Import(archive);

					BIZ_ASSERT(pDesc->GetDepth() < 2);
				
					pDesc->m_RemapExportID = pDesc->m_OriginalExportID;
					
					//C_ShapeInstance won't delete the shape descriptor's pointer, therefore, the shape descriptor's point
					//will be delete in the dynamic shapes
					
					Gravity::C_ShapeInstance pInst;
					pInst.m_pDesc=pDesc;
					pInst.Import(archive);

					if(!succeeded || pDesc->shapes.m_numElements == 0)
					{
			 			delete pDesc;
					//	Util::C_String  name = filePtr->Name();
						LOGWARN(g_Log, "Physics: Loading file %s (Data Version = %d, Code Version = %d)", filePtr->Name().Chars(), version, GRAVITY_CURRENT_VERSION);
						//LOGDEBUG(g_Log, "Physics: Loading file %s\n", name);
						break;
					} 

				/*	const Maths::HF32 check(1000.f);
					BIZ_ASSERT(pDesc->m_RenderOffset.X().LT(check) & pDesc->m_RenderOffset.Y().LT(check) & pDesc->m_RenderOffset.Z().LT(check));
					
					const char* name =filePtr->Name().Chars();*/
					
					/*
						If the compound object just has one shape inside, turn the compound object into a normal shape
					*/
					if(pDesc->shapes.m_numElements ==1)
					{
						Gravity::C_ShapeDescWrapper* pWrapper=&pDesc->shapes[0];
						Gravity::C_ShapeDesc* pSubDesc=pWrapper->m_pShapeDesc;
						if(pSubDesc->ValidShapeDescriptor())
						{	
							pSubDesc->m_RenderOffset = pDesc->m_RenderOffset;// + pWrapper->m_RelativeTransformation.Translation();
							//pSubDesc->m_RenderOffset = pInst.m_Position + pWrapper->m_RelativeTransformation.Translation();
							//pSubDesc->m_OriginalID = pSubDesc->m_ID;
							pSubDesc->m_RemapExportID = pDesc->m_OriginalExportID;
							pSubDesc->m_ConstraintBodyID = pSubDesc->m_OriginalExportID;
							pDesc->m_OwnedShapes.m_numElements = 0;
							
							C_ImportShape* pImportShape=new C_ImportShape;

							pImportShape->SetShapeDesc(pSubDesc);
					
							const Maths::HMat33 relRotation=Maths::Load<Maths::HMat33>(&pInst.m_Rotation);
							const Maths::HMat33 rotation = pWrapper->m_RelativeTransformation.Rotation() * relRotation; 
							//const Maths::HVec3 position = pInst.m_Position + pWrapper->m_RelativeTransformation.Translation();
							const Maths::HVec3 position = pInst.m_Position + pWrapper->m_RelativeTransformation.Translation();
							pImportShape->SetImportPosition(position);
							pImportShape->SetImportRotation(rotation);
							pImportShape->SetImportIneria(pWrapper->m_IneriaTensor);
							m_DynamicShapes.AddToEnd(&pImportShape);
						}

						delete pDesc;
					}
					else
					{
					/*	if(pDesc->m_ConstraintBodyID != -1)
						{
							for(U32 i=0; i<pDesc->shapes.m_numElements; ++i)
							{
								if(pDesc->shapes[i].m_pShapeDesc->m_ConstraintBodyID == pDesc->m_ConstraintBodyID)
								{
									pImportShape->SetConstraintDesc(pDesc->shapes[i].m_pShapeDesc);
								}
							}
						}*/
						if(!pDesc->ValidShapeDescriptor())
						{
							delete pDesc;
							break;
						}
						
						C_ImportShape* pImportShape=new C_ImportShape;
						
						////Override the composite shape descriptor's physics material and effect material index
						//U32 size = pDesc->shapes.m_numElements;
						//for(U32 a = 0; a<size; ++a)
						//{
						//	Gravity::C_ShapeDesc* pSubDesc = pDesc->shapes[a].m_pShapeDesc;
						//	if(pSubDesc->m_Type != Collision::E_InstancedTriangleMesh_Shape && pSubDesc->m_PhysicsMaterialIndex != 0)
						//	{
						//		pDesc->m_PhysicsMaterialIndex = pSubDesc->m_PhysicsMaterialIndex;
						//		pDesc->m_EffectMaterialIndex = pSubDesc->m_EffectMaterialIndex;
						//		break;
						//	}
						//} 
						
						pImportShape->SetShapeDesc(pDesc);
						pImportShape->SetImportPosition(pInst.m_Position);
		
						Maths::HMat33 rotation=Maths::Load<Maths::HMat33>(&pInst.m_Rotation);
						Maths::HMat33 ineria=Maths::Load<Maths::HMat33>(&pInst.m_IneriaTensor);
						pImportShape->SetImportRotation(rotation);
						pImportShape->SetImportIneria(ineria);
						m_DynamicShapes.AddToEnd(&pImportShape);
					}
					
					//MEMORY_TRACKER_POP("gravity.m_CompositeShape");
					break;
				}
			case Collision::E_Cloth_Shape:
				{
					pShapeDesc->Import(archive);
					pShapeDesc->m_RemapExportID = pShapeDesc->m_OriginalExportID;
					Gravity::C_ShapeInstance pInst;
					pInst.m_pDesc=pShapeDesc;
					pInst.Import(archive);

					C_ImportShape* pImportShape=new C_ImportShape;
					pImportShape->SetShapeDesc(pShapeDesc);
					pImportShape->SetImportPosition(pInst.m_Position);
	
					Maths::HMat33 rotation=Maths::Load<Maths::HMat33>(&pInst.m_Rotation);
					Maths::HMat33 ineria=Maths::Load<Maths::HMat33>(&pInst.m_IneriaTensor);
					pImportShape->SetImportRotation(rotation);
					pImportShape->SetImportIneria(ineria);
					m_ClothShapes.AddToEnd(&pImportShape);
					//m_DynamicShapes.AddToEnd(&pImportShape);
					break;

				}
			case Collision::E_Deformable_Shape:
				{
					pShapeDesc->Import(archive);
					pShapeDesc->m_RemapExportID = pShapeDesc->m_OriginalExportID;
					Gravity::C_ShapeInstance pInst;
					pInst.m_pDesc=pShapeDesc;
					pInst.Import(archive);

					C_ImportShape* pImportShape=new C_ImportShape;
					pImportShape->SetShapeDesc(pShapeDesc);
					pImportShape->SetImportPosition(pInst.m_Position);
	
					Maths::HMat33 rotation=Maths::Load<Maths::HMat33>(&pInst.m_Rotation);
					Maths::HMat33 ineria=Maths::Load<Maths::HMat33>(&pInst.m_IneriaTensor);
					pImportShape->SetImportRotation(rotation);
					pImportShape->SetImportIneria(ineria);
					m_DeformableShapes.AddToEnd(&pImportShape);
					//m_DynamicShapes.AddToEnd(&pImportShape);
					break;
				}
			/*case Collision::E_FluidMesh_Shape:
				{
					Gravity::C_FluidMeshDesc* pFluidDesc = (Gravity::C_FluidMeshDesc*) pShapeDesc;
					pFluidDesc->Import(archive);
					pFluidDesc->m_RemapExportID = pFluidDesc->m_OriginalExportID;
					
					Collision::C_FluidMeshShape* pFluidShape=(Collision::C_FluidMeshShape*)pFluidDesc->CreateShape(archive, &m_Filter);

					Gravity::C_ShapeInstance pInst;
					pInst.m_pDesc=pShapeDesc;
					pInst.Import(archive);

					C_ImportShape* pImportShape=new C_ImportShape;
					pImportShape->SetShapeDesc(pFluidDesc);
					pImportShape->SetImportPosition(pInst.m_Position);
	
					Maths::HMat33 rotation=Maths::Load<Maths::HMat33>(&pInst.m_Rotation);
					Maths::HMat33 ineria=Maths::Load<Maths::HMat33>(&pInst.m_IneriaTensor);
					pImportShape->SetImportRotation(rotation);
					pImportShape->SetImportIneria(ineria);
					m_StaticFluidMeshShapes.AddToEnd(&pImportShape);
					
					break;
					
				}*/
			default: // other shape and static mesh
				{
					//MEMORY_TRACKER_PUSH("gravity.m_StaticMesh");
					pShapeDesc->Import(archive);
					pShapeDesc->m_RemapExportID = pShapeDesc->m_OriginalExportID;
					
					BIZ_ASSERT(pShapeDesc->m_NodeFlag == SharedPhysics::C_GraphNode::E_Type_Static);

					m_StaticShapesDesc.AddToEnd(&pShapeDesc);

					Gravity::C_ShapeInstance* pInst=new Gravity::C_ShapeInstance;
					pInst->m_pDesc=pShapeDesc;
				
					pInst->Import(archive);

					m_StaticInstance.AddToEnd(&pInst);
					//MEMORY_TRACKER_POP("gravity.m_StaticMesh");
				}
			}

			archive >> m_Type;
		
		}

//#ifndef PLATFORM_PS3


		archive >> m_Type;

		while(m_Type!=0xffffffff) // the end of file
		{
			//archive >> bodyAID >> bodyBID >> bodyAState >> bodyBState;
			switch(m_Type)
			{
			case SharedPhysics::C_Constraint::E_Type_Spherical:
				{
					//archive >> anchorA >> anchorB;
					Gravity::SphericalJointDescriptor* pConsDesc=new Gravity::SphericalJointDescriptor;
					pConsDesc->Import(archive);
				
					m_ConstraintsDesc.AddToEnd((Gravity::C_ConstraintDescriptors** )(&pConsDesc));
					break;
				}
			case SharedPhysics::C_Constraint::E_Type_Revolute:
				{

					//archive >> anchor01 >> anchor02 >> jointAxisA >> anchor11  >> anchor12 >> jointAxisB;
					Gravity::RevoluteJointDescriptor* pConsDesc=new Gravity::RevoluteJointDescriptor;
					pConsDesc->Import(archive);
					

					m_ConstraintsDesc.AddToEnd((Gravity::C_ConstraintDescriptors** )(&pConsDesc));

					break;
				}
			case SharedPhysics::C_Constraint::E_Type_Prismatic:
				{
					//archive >> jointAxisA >> jointAxisB;
					Gravity::PrismaticJointDescriptor* pConsDesc=new Gravity::PrismaticJointDescriptor;
					pConsDesc->Import(archive);

					m_ConstraintsDesc.AddToEnd((Gravity::C_ConstraintDescriptors** )(&pConsDesc));
					break;
				}
			case SharedPhysics::C_Constraint::E_Type_SphericalLimit:
				{
					Gravity::SphericalJointLimitDescriptor* pConsDesc = new Gravity::SphericalJointLimitDescriptor;
					pConsDesc->Import(archive);
					m_ConstraintsDesc.AddToEnd((Gravity::C_ConstraintDescriptors**)(&pConsDesc));
					break;
				}
			case SharedPhysics::C_Constraint::E_Type_RevoluteLimit:
				{
					Gravity::RevoluteJointLimitDescriptor* pConsDesc = new Gravity::RevoluteJointLimitDescriptor;
					pConsDesc->Import(archive);
					m_ConstraintsDesc.AddToEnd((Gravity::C_ConstraintDescriptors**)(&pConsDesc));
					break;
				}
			case SharedPhysics::C_Constraint::E_Type_PrismaticLimit:
				{
					Gravity::PrismaticJointLimitDescriptor* pConsDesc = new Gravity::PrismaticJointLimitDescriptor;
					pConsDesc->Import(archive);
					m_ConstraintsDesc.AddToEnd((Gravity::C_ConstraintDescriptors**)(&pConsDesc));
					break;
				}

			default:
				break;
			}

			archive >> m_Type;
		}
//#endif

		filePtr->SetVersionNumber(oldVersionNo);

//		MEMORY_TRACKER_POP("Gravity.Importer.ImportScene");

		return true;
	}

	// Returns the number of C_ImportShapes in the C_ImportScene. There should be one C_ImportShape per export call to C_Converter::AddShape().
	U32 C_ImportScene::GetImportShapeCount() const
	{
		return  m_DynamicShapes.m_numElements;
	}

	U32 C_ImportScene::GetNumConstraintCount() const
	{
		return m_ConstraintsDesc.m_numElements;
	}


	// Returns a C_ImportShape given an index. The index will be the U32 returned from the exporter call to C_Converter::AddShape().
	C_ImportShape* C_ImportScene::GetImportShape( const U32 index ) const
	{
		C_ImportShape* pShape;
		for(S32 i=0; i< m_DynamicShapes.m_numElements; ++i)
		{
			pShape= m_DynamicShapes[i];

			/*
				0x110....index
			*/
			U32 num=((6 << 29)|index);
			if(pShape->GetShapeDesc()->m_RemapExportID == num)
				return pShape;
		}
		return NULL;
	}

	
	C_ConstraintDescriptors* C_ImportScene::GetConstraint(const U32 id) const
	{
		C_ConstraintDescriptors* pDesc;

		for(S32 i=0; i< m_ConstraintsDesc.m_numElements; ++i)
		{
			pDesc = m_ConstraintsDesc[i];

			//0x010....index
			U32 num = ((2 << 29) | id);
			if(pDesc->m_Id == num)
				return pDesc;
		}
		return NULL;
	}


	/*Gravity::C_ShapeDesc* C_ImportScene::GetSubDynamicShape( Gravity::C_CompositeDesc& comp, const U32 subIndex ) const
	{
		for(S32 i=0; i<comp.m_OwnedShapes.m_numElements; ++i)
		{
			C_ShapeDesc* pDesc=comp.m_OwnedShapes[i];
			U32 num = (subIndex&~(1<<30)) | (1<<31);

			if(pDesc->m_ID == num)
				return pDesc;
		}
		return NULL;
	}*/


	void C_ImportSceneStaticInstance::Import()
	{
		if( (m_pImportScene == NULL) || (m_pScene == NULL) )
			return;
		
//		MEMORY_TRACKER_PUSH("Gravity.Importer.ImportUnitStaticInstance");
		bool bHit = false;
		Collision::C_AxisBoxMinMax worldBox = m_pScene->GetWorldBounds();
		worldBox.m_min.Y(Maths::Min(worldBox.m_min.Y(), -200.f));
		worldBox.m_max.Y(Maths::Max(worldBox.m_max.Y(), 500.f));
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		//const Maths::HVec3 desiredSize(20.f, 20.f, 20.f);
		const Maths::HVec3 desiredSize(m_DesiredSize);
		const Maths::HVec3 maxCellSize(m_MaxCellSize);
		S32 maxDepth = 12; 



		const Maths::HF32 zero=Maths::Constants<Maths::HF32>::Zero();

		/*
			Import all the static shape, include instances and mesh
		*/
		
		for(S32 i=0; i< m_pImportScene->m_StaticInstance.m_numElements; ++i)
		{

			Gravity::C_ShapeInstance* pInst = m_pImportScene->m_StaticInstance[i];
			Gravity::C_ShapeDesc* pDesc=pInst->m_pDesc;
			//if(!pDesc->ValidShapeDescriptor())
			//	continue;

			Gravity::C_Body* pBody(m_pScene->CreateBody());
			Gravity::C_BodyHelper helper(pDesc);
			//Store the body pointer for removing
			m_pBodies.AddToEnd(&pBody);

			if(pDesc->m_Type == Collision::E_TriangleMesh_Shape)
			{
				helper.CreateBody(pBody, &m_Filter);
				pBody->SetMass(0.f);
				
				//Collision::C_TriangleMeshShape* pShape = (Collision::C_TriangleMeshShape*)pBody->GetShape();

				//pBody->MakeStatic();
				bHit = true;
				pBody->MakeStatic();
				pBody->MakeStaticMesh();

				pBody->SetUserData(m_pScene->GetStaticUserData());

				m_pScene->AddMeshBody(pBody);
				
				pBody->Slop(Maths::Constants<F32>::Max());
				pBody->Baumgarte(Maths::Constants<F32>::Max());

				worldBox.m_max = Maths::Max(worldBox.m_max, pBody->GetShape()->m_LocalBoundingBox.centre + pBody->GetShape()->m_LocalBoundingBox.extents);
				worldBox.m_min = Maths::Min(worldBox.m_min, pBody->GetShape()->m_LocalBoundingBox.centre - pBody->GetShape()->m_LocalBoundingBox.extents);

  
				//N.B. Uncomment this to make all static meshes into instances which are rolled 45 degrees around the x-axis
				//pBody->SetMass(0.f);
				//
				//Gravity::C_TriangleMeshDesc* pMeshDesc = (Gravity::C_TriangleMeshDesc*)pDesc;
				//Collision::C_InstancedTriangleMeshShape* pInstancedShape = new Collision::C_InstancedTriangleMeshShape(&m_Filter);

				//Collision::C_AxisBoxMinMax tempBox;
				//pMeshDesc->m_OptimisedTree->m_pNodes[0].m_BoundingBox.LoadBox(tempBox, pMeshDesc->m_OptimisedTree->m_Low, pMeshDesc->m_OptimisedTree->m_High);
				//Collision::C_CRAxisBox localBox;
				//localBox.centre = (tempBox.m_max + tempBox.m_min) * 0.5f;
				//localBox.extents = (tempBox.m_max - tempBox.m_min) * 0.5f;
				//pInstancedShape->m_AABBTree.m_pTree = pMeshDesc->m_OptimisedTree;
				//pInstancedShape->m_LocalBoundingBox = localBox;	

				//Maths::RotX rotY(3.14f/8.f);
				//Maths::HMat33 rot(rotY);

				//Maths::HVec3 position(5.f, 10.f, 5.f);

				//helper.CreateBody(0.f, position, rot, pBody, pInstancedShape, 0);			

				////pBody->MakeStatic();
				//bHit = true;
				//pBody->MakeStatic();

				//pBody->SetUserData(m_pScene->GetStaticUserData());

				//m_pScene->AddStaticBody(pBody);
				//
				//pBody->Slop(Maths::Constants<F32>::Max());
				//pBody->Baumgarte(Maths::Constants<F32>::Max());

				//worldBox.m_max = Maths::Max(worldBox.m_max, pBody->GetShape()->m_LocalBoundingBox.centre + pBody->GetShape()->m_LocalBoundingBox.extents);
				//worldBox.m_min = Maths::Min(worldBox.m_min, pBody->GetShape()->m_LocalBoundingBox.centre - pBody->GetShape()->m_LocalBoundingBox.extents);


			}
			else if(pDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
			{
				
				
				Gravity::C_InstancedTriangleMeshDesc* pTriDesc=(Gravity::C_InstancedTriangleMeshDesc*)pDesc; 
				/*if(!pDesc->ValidShapeDescriptor())
					continue;  */

				BIZ_ASSERT(pTriDesc->m_OriginalExportID != 0);
				Collision::C_TriangleMeshShape* tri=m_pImportScene->m_StaticInstanceManager.FindBase(pTriDesc->m_BaseNodeId);
				BIZ_ASSERT(tri!=NULL);

				if(tri->m_AABBTree.m_NumTriangles == 0)

					continue;

				bHit = true;

				const Maths::HMat33 rotation=Maths::Load<Maths::HMat33>(&pInst->m_Rotation);
				const Maths::HMat43 transform(rotation, pInst->m_Position);

				pTriDesc->m_pShape = tri;
				pTriDesc->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Static;
				
				helper.CreateBody(zero, pInst->m_Position, rotation, pBody, &m_Filter, tri->m_PhysicsMaterialIndex, tri->m_EffectMaterialIndex);
				

				Collision::C_CRAxisBox box;
				pBody->GetShape()->m_LocalBoundingBox.Transform(transform, box);
				worldBox.m_max = Maths::Max(worldBox.m_max, box.centre + box.extents);
				worldBox.m_min = Maths::Min(worldBox.m_min, box.centre - box.extents);

				//m_pScene->ResizeScene(worldBox, desiredSize, maxDepth);
				pBody->MakeStatic();
				pBody->SetMass(0.f);
				pBody->Baumgarte(Maths::Constants<F32>::Max());
				pBody->MakeStatic();
				pBody->SetUserData(m_pScene->GetStaticUserData());
			
				m_pScene->AddStaticBody(pBody);
			}
			else if(pDesc->m_Type == Collision::E_FluidMesh_Shape)
			{
				Gravity::C_FluidMeshDesc* pFluidDesc = (Gravity::C_FluidMeshDesc*)pDesc;
				pFluidDesc->m_Depth = m_FluidDepth;
				helper.CreateBody(pBody, &m_FluidFilter);
				pBody->SetMass(0.f);
				
				//Collision::C_TriangleMeshShape* pShape = (Collision::C_TriangleMeshShape*)pBody->GetShape();

				//pBody->MakeStatic();
				bHit = true;
				//pBody->MakeStaticMesh();
				pBody->MakeStatic();

				pBody->SetUserData(m_pScene->GetStaticUserData());
				m_pScene->AddFluidMeshBody(pBody);
				
				pBody->Slop(Maths::Constants<F32>::Max());
				pBody->Baumgarte(Maths::Constants<F32>::Max());

				Collision::C_FluidMeshShape* pFluidShape = (Collision::C_FluidMeshShape*)pBody->GetShape();
				//pFluidShape->m_FluidDepth = m_FluidDepth;
				//pFluidShape->m_LocalBoundingBox.extents.Y(pFluidShape->m_LocalBoundingBox.extents.Y() + m_FluidDepth * half);
				//pFluidShape->m_LocalBoundingBox.centre.Y(pFluidShape->m_LocalBoundingBox.centre.Y() - m_FluidDepth * half);
				worldBox.m_max = Maths::Max(worldBox.m_max, pFluidShape->m_LocalBoundingBox.centre + pBody->GetShape()->m_LocalBoundingBox.extents);
				worldBox.m_min = Maths::Min(worldBox.m_min, pBody->GetShape()->m_LocalBoundingBox.centre - pBody->GetShape()->m_LocalBoundingBox.extents);

			}
			else
			{

				//Compound object is dynamic object, should not be here at all
				BIZ_ASSERT(pDesc->m_Type != Collision::E_Composite_Shape);

				Maths::HMat33 rotation=Maths::Load<Maths::HMat33>(&pInst->m_Rotation);
				helper.CreateBody(pInst->m_Position, rotation, pBody, &m_Filter);

				pBody->MakeStatic();
				pBody->SetMass(0.f);
				pBody->Baumgarte(Maths::Constants<F32>::Max());
				pBody->SetUserData(m_pScene->GetStaticUserData());
			
				m_pScene->AddStaticBody(pBody);
			}
		}

		//m_pScene->ProcessCommands();

		/*
			Import the constraints
		*/
		//for(S32 i=0; i<m_pImportScene->m_ConstraintsDesc.m_numElements; ++i)
		//{
		//	
		//	Gravity::ConstraintDescriptors* pDesc=m_pImportScene->m_ConstraintsDesc[i];

		//	U32 numBodies= m_pScene->NumBodies();
		//	C_Body* physicsBodyA = NULL;
		//	C_Body* physicsBodyB = NULL;

		//	S32 bodyID;

		//	for(U32 i=0; i< numBodies; ++i)
		//	{
		//		C_Body* pBody=m_pScene->GetBody(i);
		//		bodyID=pBody->GetBodyId();
		//		if(bodyID == pDesc->m_PhysicsBodyAID)
		//		{
		//			physicsBodyA = pBody;
		//			if(physicsBodyB != NULL)
		//				break;
		//		}
		//		else if(bodyID == pDesc->m_PhysicsBodyBID)
		//		{
		//			physicsBodyB = pBody;
		//			if(physicsBodyA != NULL)
		//				break;
		//		}
		//	}

		//	if(pDesc->m_PhysicsBodyAState == 4)//static object
		//		physicsBodyA->MakeStatic();
		//	else
		//		physicsBodyA->Activate();

		//	if(pDesc->m_PhysicsBodyBState == 4)//static object
		//		physicsBodyB->MakeStatic();
		//	else
		//		physicsBodyB->Activate();

		//	SharedPhysics::C_Constraint* pConstraint=pDesc->CreateConstraint(*m_pScene->m_pGraphManager, physicsBodyA, physicsBodyB);
		//	//m_pScene->m_Constraints.AddToEnd(&pConstraint);
		//	m_pScene->AddConstraint(pConstraint);
		//	
		//}
		if(bHit)
		{
			m_pScene->ResizeScene(worldBox, desiredSize, maxCellSize, maxDepth);
		}
//		MEMORY_TRACKER_POP("Gravity.Importer.ImportUnitStaticInstance");
	}

#ifndef CFG_SPEED
#pragma optimize("", on)
#endif


	/*C_BaseInstanceMeshNode::C_BaseInstanceMeshNode(Collision::C_TriangleMeshShape* mesh, const U32 id): m_Mesh(mesh), m_Id(id)
	{
	}

	C_BaseInstanceMeshNode::C_BaseInstanceMeshNode(): m_Mesh(NULL), m_Id(0)
	{
	}

	C_BaseInstanceMeshNode::~C_BaseInstanceMeshNode()
	{
		if(m_Mesh)
			delete m_Mesh;
		m_Mesh=NULL;
	}*/

	/*Collision::C_TriangleMeshShape* C_InstanceMeshResourceManager::FindBase(const U32 id)
	{
		C_BaseInstanceMeshNode* pBaseNode;
		for(S32 i=0; i<m_BaseShapes.m_numElements; ++i)
		{
			pBaseNode=m_BaseShapes[i];
			if(pBaseNode->m_Id == id)
				return pBaseNode->m_Mesh;
		}
		return NULL;
	}
	void C_InstanceMeshResourceManager::AddBase(const Collision::C_TriangleMeshShape* mesh, const U32 id)
	{
		C_BaseInstanceMeshNode* node=new C_BaseInstanceMeshNode(const_cast<Collision::C_TriangleMeshShape*>(mesh), id);
		m_BaseShapes.AddToEnd(&node);
	}

	void C_InstanceMeshResourceManager::Reset()
	{
		for(S32 a = 0; a < m_BaseShapes.m_numElements; ++a)
		{
			if(m_BaseShapes[a])
				delete m_BaseShapes[a];
			m_BaseShapes[a] = NULL;
		}
		m_BaseShapes.m_numElements = 0;
	}*/
}
