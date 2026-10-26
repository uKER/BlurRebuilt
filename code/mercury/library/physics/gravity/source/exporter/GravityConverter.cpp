#include "Stdafx.h"
#include "gravity/exporter/GravityExporter.h"
#include "gravity/sdk/InertiaTensorGenerator.h"

namespace Gravity
{

	void C_GravityConverter::BuildConstraintNode(Gravity::C_ExportConstraint& buildCons, Gravity::C_ConstraintNode* pConstraint)
	{
		switch(pConstraint->m_ConstraintType)
		{
		case 0: //"BallAndSocket" 
				{
					if(pConstraint->m_UsedLimit)
						buildCons.AddSphericalJointLimitConstraint(pConstraint);
					else
						buildCons.AddSphericalJointConstraint(pConstraint);
					break;
				}
		case 1: //"Hinge"
				{
					if(pConstraint->m_UsedLimit)
						buildCons.AddRevoluteJointLimitConstraint(pConstraint);
					else
						buildCons.AddRevoluteJointConstraint(pConstraint);
					break;
				
				}
		case 2: //"Sliding"
				{
					if(pConstraint->m_UsedLimit)
						buildCons.AddPrismaticJointLimitConstraint(pConstraint);
					else
						buildCons.AddPrismaticJointConstraint(pConstraint);
					break;
				}
		case 3: //"Nail"
				{
					buildCons.AddSphericalJointConstraint(pConstraint);
					break;
				}
		default:
			BIZ_ASSERT("Not Support constraint");
		}
			
			
		
	}


	void C_GravityConverter::BuildShapeNode(Gravity::C_PhysicsExporter& buildPrim, Gravity::C_ShapeNode* pShapeNode)
	{
		using namespace Maths;
		const HF32 half(Constants<HF32>::Half());
		const HF32 two(Constants<HF32>::Two());
		switch(pShapeNode->m_ShapeType)
		{
			case C_ShapeNode::E_ShapeType_Sphere:
				buildPrim.AddSphere(pShapeNode->m_UniqueID, pShapeNode->m_Scale.X(), pShapeNode->m_Transform.Translation(), pShapeNode->m_Transform.Rotation(), pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag, pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
				break;
			case C_ShapeNode::E_ShapeType_ConvexHull:
				buildPrim.AddConvexHull(pShapeNode->m_UniqueID, pShapeNode->m_VertexList, pShapeNode->m_Transform.Translation(), pShapeNode->m_Transform.Rotation(), pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag, pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
				break;
			case C_ShapeNode::E_ShapeType_Box:
				buildPrim.AddOBB(pShapeNode->m_UniqueID, pShapeNode->m_Scale, pShapeNode->m_Transform.Translation(), pShapeNode->m_Transform.Rotation(), pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag, pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
				break;
			case C_ShapeNode::E_ShapeType_Cylinder:
				{
					const HF32 radius(pShapeNode->m_Scale.X());
					const HF32 height(pShapeNode->m_Scale.Y()*two);

					buildPrim.AddCylinder(pShapeNode->m_UniqueID, radius, height,pShapeNode->m_Transform.Translation(), pShapeNode->m_Transform.Rotation(), pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag, pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
					break;
				}
			case C_ShapeNode::E_ShapeType_Capsule:
				{
					const HF32 radius(pShapeNode->m_Scale.X());
					const HF32 height(pShapeNode->m_Scale.Y()*two);

					buildPrim.AddCapsule(pShapeNode->m_UniqueID, radius, height,pShapeNode->m_Transform.Translation(), pShapeNode->m_Transform.Rotation(), pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag, pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
					break;
				}
			case C_ShapeNode::E_ShapeType_Cone:
				{
					const HF32 radius(pShapeNode->m_Scale.X());
					const HF32 height(pShapeNode->m_Scale.Y() * two);
					const HMat33 rotate = pShapeNode->m_Transform.Rotation();
					const HVec3 midPoint = pShapeNode->m_Transform.Translation();
					const HVec3 offset(0.f, height*Constants<HF32>::Quarter(), 0.f);
					const HVec3 centerOfMass = midPoint - offset*rotate;
					buildPrim.AddCone(pShapeNode->m_UniqueID, radius, height, centerOfMass, rotate, pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag, pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
					break;
				}
			default:
				return;

		}
	}

	
	Gravity::C_CompositeDesc* C_GravityConverter::BuildCompoundNode(Gravity::C_CompoundNode* pCompoundNode, Maths::HMat33& inertiaTensor, SharedPhysics::SortedArray<Gravity::C_MeshNode*>& meshList)
	{
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		Gravity::C_CompositeDesc* cObj=new Gravity::C_CompositeDesc;
		cObj->m_PhysicsMaterialIndex=pCompoundNode->m_PhysicsMaterialIndex;
		cObj->m_EffectMaterialIndex =pCompoundNode->m_EffectMaterialIndex;
		cObj->m_CollisionFlag=pCompoundNode->m_CollisionFlag;
		cObj->m_OriginalExportID=pCompoundNode->m_UniqueID;
		cObj->m_Mass=pCompoundNode->m_Mass;
		cObj->m_NodeFlag = pCompoundNode->m_NodeFlag;
		Maths::HF32 mass;

		for(S32 i=0; i< pCompoundNode->m_Nodes.m_numElements; ++i)
		{
			Gravity::C_Node* pNode=pCompoundNode->m_Nodes[i];
			
			//if(pNode->m_Type == Gravity::GE_COMPOUND_TYPE)
			//{
			//	Gravity::C_CompoundNode* pComp= (Gravity::C_CompoundNode*)pNode;

			//	Maths::HMat33 subIneria;
			//	Gravity::C_CompositeDesc* subCObj=BuildCompoundNode(pComp, subIneria, meshList);
			//	/*Aftk::Matrix44 mLocalMatrix=pComp->a3dMatrix;
			//	Maths::HMat43 transform(Maths::UNDEFINED);
			//	ConvertA3DMatrixToTransformationMatrix(mLocalMatrix, transform);*/

			//	cObj->AddOwnShape(subCObj, pComp->m_Transform, subIneria, pComp->m_Mass);
			//	ineriaTensor = ineriaTensor + subIneria;
			//	
			//}
			if(pNode->m_Type == Gravity::GE_SHAPE_TYPE)
			{

				Gravity::C_ShapeNode* pShapeNode=(Gravity::C_ShapeNode*)pNode;

				BuildCompoundObject(pCompoundNode->m_Transform, pShapeNode, inertiaTensor, *cObj);
				mass += one;
			}
			else if(pNode->m_Type == Gravity::GE_MESH_TYPE)
			{
				Gravity::C_MeshNode* pMeshNode=(Gravity::C_MeshNode*)pNode;
				meshList.AddToEnd(&pMeshNode);

			}
			else if(pNode->m_Type == Gravity::GE_FLUIDMESH_TYPE)
			{
				Gravity::C_FluidMeshNode* pFluidNode = (Gravity::C_FluidMeshNode*) pNode;

				Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
				Gravity::C_FluidMeshDesc* pDesc=m_Exporter->AddFluidMesh(pFluidNode->m_UniqueID, pFluidNode->m_Depth, 0.2f, 0.7f, pFluidNode->m_PhysicsMaterialIndex, pFluidNode->m_EffectMaterialIndex, pFluidNode->m_NodeFlag, pFluidNode->m_pMesh->m_Tris);
				pDesc->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Active;
				cObj->AddOwnShape(pDesc, pFluidNode->m_Transform, inertiaTensor, pFluidNode->m_Mass);
			}
		}

		cObj->m_Mass = mass;
		return cObj;
	}

	
	void C_GravityConverter::BuildCompoundObject(Maths::HMat43& parentWorldMatrix, Gravity::C_ShapeNode* pShapeNode, Maths::HMat33& ineriaTensor, Gravity::C_CompositeDesc& cObj)
	{
	
		using namespace Maths;
		Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		cObj.m_ConstraintBodyID = pShapeNode->m_UniqueID;

		switch(pShapeNode->m_ShapeType)
		{
			case C_ShapeNode::E_ShapeType_Sphere:
				{
					Gravity::C_SphereDesc* sphere=m_Exporter->AddSphere(pShapeNode->m_UniqueID, pShapeNode->m_Scale.X(), pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag,  pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
					sphere->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Active;
					//const Maths::HMat33 _ineriaTensor = Gravity::C_InertiaTensorGenerator::SphereInertiaTensor(pShapeNode->m_Mass,  pShapeNode->m_Scale.X());
					//ineriaTensor =ineriaTensor + _ineriaTensor;
					cObj.AddOwnShape(sphere, pShapeNode->m_Transform, ineriaTensor, pShapeNode->m_Mass);
					break;
				}
			case C_ShapeNode::E_ShapeType_ConvexHull:
				{
					Gravity::C_ConvexHullDesc* convex=m_Exporter->AddConvexHull(pShapeNode->m_UniqueID, pShapeNode->m_VertexList,pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag,  pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
					if(convex == NULL)
					{
						LOGWARN(g_Log, "Name of the invalid model %s", pShapeNode->m_NodeName);
					}
					else
					{
						convex->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Active;
						//const Maths::HMat33 _ineriaTensor=Gravity::C_InertiaTensorGenerator::ConvexHullInertiaTensor(pShapeNode->m_Mass, pShapeNode->m_VertexList);
						//ineriaTensor =ineriaTensor + _ineriaTensor;
						cObj.AddOwnShape(convex,pShapeNode->m_Transform, ineriaTensor, pShapeNode->m_Mass);
					}

					break;
				}
				
			case C_ShapeNode::E_ShapeType_Box:
				{
					
					Gravity::C_BoxDesc* box=m_Exporter->AddOBB(pShapeNode->m_UniqueID, pShapeNode->m_Scale, pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag,  pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
					box->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Active;
					//const Maths::HMat33 _ineriaTensor=Gravity::C_InertiaTensorGenerator::BoxInertiaTensor(pShapeNode->m_Mass, pShapeNode->m_Scale);
					//ineriaTensor =ineriaTensor + _ineriaTensor;
					cObj.AddOwnShape(box, pShapeNode->m_Transform, ineriaTensor, pShapeNode->m_Mass);
					break;
				}
			case C_ShapeNode::E_ShapeType_Cylinder:
				{
					const Maths::HF32 radius(pShapeNode->m_Scale.X());
					const Maths::HF32 height(pShapeNode->m_Scale.Y()*Maths::Constants<Maths::HF32>::Two());
					Gravity::C_CylinderDesc* cylinder=m_Exporter->AddCylinder(pShapeNode->m_UniqueID, radius, height, pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag,  pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
					cylinder->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Active;
					//const Maths::HMat33 _ineriaTensor=Gravity::C_InertiaTensorGenerator::CylinderInertiaTensor(pShapeNode->m_Mass, radius, height);
					//ineriaTensor =ineriaTensor + _ineriaTensor;
					cObj.AddOwnShape(cylinder, pShapeNode->m_Transform, ineriaTensor, pShapeNode->m_Mass);
					break;
				}
			case C_ShapeNode::E_ShapeType_Capsule:
				{
					const Maths::HF32 radius(pShapeNode->m_Scale.X());
					const Maths::HF32 height(pShapeNode->m_Scale.Y()*Maths::Constants<Maths::HF32>::Two());
					Gravity::C_CapsuleDesc* capsule=m_Exporter->AddCapsule(pShapeNode->m_UniqueID, radius, height, pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag, pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
					capsule->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Active;
					//const Maths::HMat33 _ineriaTensor=Gravity::C_InertiaTensorGenerator::CapsuleInertiaTensor(pShapeNode->m_Mass, height, pShapeNode->m_Mass, height);
					//ineriaTensor =ineriaTensor + _ineriaTensor;
					cObj.AddOwnShape(capsule, pShapeNode->m_Transform, ineriaTensor, pShapeNode->m_Mass);
					break;
				}
			case C_ShapeNode::E_ShapeType_Cone:
				{
					const Maths::HF32 radius(pShapeNode->m_Scale.X());
					const Maths::HF32 height(pShapeNode->m_Scale.Y()*Maths::Constants<Maths::HF32>::Two());

					const HMat33 rotate = pShapeNode->m_Transform.Rotation();
					const HVec3 midPoint = pShapeNode->m_Transform.Translation();
					const HVec3 offset(0.f, height*Constants<HF32>::Quarter(), 0.f);
					const HVec3 centerOfMass = midPoint - offset*rotate;

					const HMat43 transform( rotate, centerOfMass);
					
					Gravity::C_ConeDesc* cone=m_Exporter->AddCone(pShapeNode->m_UniqueID, radius, height, pShapeNode->m_PhysicsMaterialIndex, pShapeNode->m_EffectMaterialIndex, pShapeNode->m_CollisionFlag, pShapeNode->m_NodeFlag, pShapeNode->m_Mass, pShapeNode->m_UserDataFlag);
					cone->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Active;
					//const Maths::HMat33 _ineriaTensor=Gravity::C_InertiaTensorGenerator::CapsuleInertiaTensor(pShapeNode->m_Mass, height, pShapeNode->m_Mass, height);
					//ineriaTensor =ineriaTensor + _ineriaTensor;
					cObj.AddOwnShape(cone, transform, ineriaTensor, pShapeNode->m_Mass);
					break;

				}
			default:
				return;

		}
	}
}
