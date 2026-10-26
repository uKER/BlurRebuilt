#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <core/core.h>
#include <core/maths.h>
#include "Aftk/Aftk.h"
#include <tools_debug/DebugManager.h>

#include "system/scheduler/jobqueue.h"

//#include "gravity/narrowphase/shape.h"

#include "gravity/treebuilder/aabbtree.h"
#include "gravity/treebuilder/aabbtree.inl"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sdk/shapedescriptors.inl"



#include "gravity/exporter/ExportPrimitivesList.h"
//#include "gravity/exporter/ExportPrimitivesList.inl"

#include "system/serialise/C_FileSystem.h"
#include "system/serialise/I_BinaryStream.h"

#include "gravity/exporter/GravityExporter.h"
#include "gravity/exporter/GravityConverter.h"
//#include "gravity/exporter/GravityConverter.inl"
#include "gravity/narrowphase/OptimisedAABBTree.inl"
#include "converter_shared/PhysicsDataConverter.h"
#include <core/maths/hvec/new/IncludeGeneral.h>

#include <converter_shared/helpers/AftkHelpers.h>

namespace Gravity
{
	Debug::Logger &g_Log = Debug::GetLogger("gravity.export");
	//Maths::HMat43 C_Converter::GetRelativeTransformationMatrix(Aftk::C_SceneNode* pParent, Aftk::C_SceneNode* pChild)
	//{
	//	Aftk::Matrix44 worldMatrix = pParent->GetWorldMatrix();
	//	//CreateTree(pMesh, worldMatrix, tree);

	//	Maths::HMat43 trans1;
	//	Aftk::TemplateToNightmareMatrix43(trans1, worldMatrix);

	//	Aftk::Matrix44 mWorldMatrix=pChild->GetWorldMatrix();

	//	Maths::HMat43 trans2;

	//	Aftk::TemplateToNightmareMatrix43(trans2, mWorldMatrix);

	//	return Maths::HMat43(trans2 * trans1.InverseRotTrans());

	//}

	/*Maths::HMat43 C_Converter::FlipZComponent(const Maths::HMat43 PCREF transform)
	{
		Maths::HMat33 rotation(transform.Rotation());
		Maths::HVec3 translation(transform.Translation());

		const Maths::HVec3 ax(rotation.AX());
		const Maths::HVec3 ay(rotation.AY());
		const Maths::HVec3 az(rotation.AZ());

		const Maths::HVec3 az2(ax^ay);
		
		if((az|az2).LE(Maths::Constants<Maths::HF32>::Zero()))
			rotation.AZ(-rotation.AZ());

		return Maths::HMat43(rotation, translation);
		
	}*/

	/*void C_Converter::GetMinMaxDifferenceVector(const Aftk::C_SceneNode* pSceneNode, Maths::HVec3& vDif)
	{
		Aftk::C_Bounds& bounds=pSceneNode->GetWorldBounds();
		Aftk::Position3 p0=bounds.GetMax();
		Aftk::Position3 p1=bounds.GetMin();
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		const Maths::HVec3 vmax(p0.X(), p0.Y(), p0.Z());
		const Maths::HVec3 vmin(p1.X(), p1.Y(), p1.Z());
		vDif=Maths::HVec3((vmax-vmin)*half);
	}*/



	void C_Converter::ConvertA3DMatrixToTransformationMatrix(const Aftk::Matrix44& a3dMatrix, Maths::HVec3& scale, Maths::HVec3& position, Maths::HMat33& rotation)
	{

		Maths::HMat43 matrix(Maths::UNDEFINED);
		Aftk::TemplateToNightmareMatrix43(matrix, a3dMatrix);

		Maths::ScaleMat3 scale1;
		Maths::ShearMat3 shear1;
		Maths::Mat33 rotation1;
		Maths::DecomposeScaleShearRotate( matrix.Rotation(), &scale1, &shear1, &rotation1);

		scale = Maths::HVec3(scale1.m_Scale);
		position = matrix.Translation();
		rotation = rotation1;
		scale = Abs(scale);


		/*Maths::HMat33 scaleMatrix;
		scaleMatrix.SetIdentity();
		scaleMatrix.AX(scaleMatrix.AX() * scale.X());
		scaleMatrix.AY(scaleMatrix.AY() * scale.Y());
		scaleMatrix.AZ(scaleMatrix.AZ() * scale.Z());

		Maths::HMat33 final(scaleMatrix*rotation);*/

	}

	void C_Converter::ConvertA3DMatrixToTransformationMatrix(const Aftk::Matrix44& a3dMatrix, Maths::HMat43& transform)
	{
		Maths::HMat43 matrix(Maths::UNDEFINED);
		Aftk::TemplateToNightmareMatrix43(matrix, a3dMatrix);

		Maths::ScaleMat3 scale1;
		Maths::ShearMat3 shear1;
		Maths::Mat33 rotation1;
		Maths::DecomposeScaleShearRotate( matrix.Rotation(), &scale1, &shear1, &rotation1);

	/*	Maths::HVec3 scale = Maths::HVec3(scale1.m_Scale);
		Maths::HMat33 rotation = rotation1;

		scale = Abs(scale);

		Maths::HMat33 scaleMatrix;
		scaleMatrix.SetIdentity();
		scaleMatrix.AX(scaleMatrix.AX() * scale.X());
		scaleMatrix.AY(scaleMatrix.AY() * scale.Y());
		scaleMatrix.AZ(scaleMatrix.AZ() * scale.Z());

		Maths::HMat33 final(scaleMatrix*rotation);*/

		transform = Maths::HMat43(rotation1, matrix.Translation());
                   
	}

	
	Gravity::C_MeshNode* C_Converter::ConstructMeshNode(const Aftk::C_SceneMeshNode* pMesh)
	{

		const U32 numTriangles(pMesh->GetNumTriangles(0));

		if(numTriangles == 0)
			return NULL;
		Gravity::C_MeshNode* pMeshNode=new Gravity::C_MeshNode;

		//const U32 numVertices(pMesh->GetNumVertexPositions());
		

		
		
		if(numTriangles == 0)
			LOGWARN(g_Log, "Mesh is Empty: A3D Scene Id %i " , pMesh->GetUniqueId());

		Aftk::Matrix44 mWorldMatrix=pMesh->GetWorldMatrix();

		for(U32 a = 0; a < numTriangles; ++a)
		{
			const Aftk::C_Polygon& poly(pMesh->GetPolygon(a));
			const Aftk::C_PolygonVertex& pPA = poly.GetPolygonVertex(0);
			const Aftk::C_PolygonVertex& pPB = poly.GetPolygonVertex(1);
			const Aftk::C_PolygonVertex& pPC = poly.GetPolygonVertex(2);

			Aftk::Position3 vPosA = pMesh->GetVertexPosition(pPA.GetPositionIndex());
			Aftk::Position3 vPosB = pMesh->GetVertexPosition(pPB.GetPositionIndex());
			Aftk::Position3 vPosC = pMesh->GetVertexPosition(pPC.GetPositionIndex());
			vPosA = Aftk::ApplyMatrix(mWorldMatrix, vPosA);
			vPosB = Aftk::ApplyMatrix(mWorldMatrix, vPosB);
			vPosC = Aftk::ApplyMatrix(mWorldMatrix, vPosC);

			Maths::HVec3 posA(vPosA.X(), vPosA.Y(), vPosA.Z());
			Maths::HVec3 posB(vPosB.X(), vPosB.Y(), vPosB.Z());
			Maths::HVec3 posC(vPosC.X(), vPosC.Y(), vPosC.Z());
			
			Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			tri->m_CollisionFlag = m_pPolyInform->GetCollisionFlags(poly);
			if(tri->m_CollisionFlag != 0)
			{
				tri->m_MaterialIndex = m_pPolyInform->GetMaterial(poly);
				tri->m_UserDataFlag= m_pPolyInform->GetUserData(poly);
				tri->vertexA=posA;
				tri->vertexB=posB;
				tri->vertexC=posC;
				pMeshNode->m_Tris.AddToEnd(&tri);
			}
			else
			{
				//LOGWARN(g_Log, "Triangle CollisionFlag Not Set: %s %i, %i", pMesh->GetName().c_str(), pMesh->GetUniqueId(), a);
				delete tri;
			}
		}
		
		return pMeshNode;
	}

	

	Gravity::C_MeshNode* C_Converter::ConstructMeshNode(const Aftk::C_SceneMeshNode* pMesh, const Aftk::Matrix44 PCREF trans)
	{
		const U32 numTriangles(pMesh->GetNumTriangles(0));

		if(numTriangles == 0)
			return NULL;
		Gravity::C_MeshNode* pMeshNode=new Gravity::C_MeshNode;

		//const U32 numVertices(pMesh->GetNumVertexPositions());
		
		

		for(U32 a = 0; a < numTriangles; ++a)
		{
			const Aftk::C_Polygon& poly(pMesh->GetPolygon(a));
			const Aftk::C_PolygonVertex& pPA = poly.GetPolygonVertex(0);
			const Aftk::C_PolygonVertex& pPB = poly.GetPolygonVertex(1);
			const Aftk::C_PolygonVertex& pPC = poly.GetPolygonVertex(2);

			Aftk::Position3 vPosA = pMesh->GetVertexPosition(pPA.GetPositionIndex());
			Aftk::Position3 vPosB = pMesh->GetVertexPosition(pPB.GetPositionIndex());
			Aftk::Position3 vPosC = pMesh->GetVertexPosition(pPC.GetPositionIndex());
			vPosA = Aftk::ApplyMatrix(trans, vPosA);
			vPosB = Aftk::ApplyMatrix(trans, vPosB);
			vPosC = Aftk::ApplyMatrix(trans, vPosC);

			Maths::HVec3 posA(vPosA.X(), vPosA.Y(), vPosA.Z());
			Maths::HVec3 posB(vPosB.X(), vPosB.Y(), vPosB.Z());
			Maths::HVec3 posC(vPosC.X(), vPosC.Y(), vPosC.Z());
			
			Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			tri->m_CollisionFlag = m_pPolyInform->GetCollisionFlags(poly);
			if(tri->m_CollisionFlag != 0)
			{
				tri->m_MaterialIndex = m_pPolyInform->GetMaterial(poly);
				
				tri->m_UserDataFlag= m_pPolyInform->GetUserData(poly);
				tri->vertexA=posA;
				tri->vertexB=posB;
				tri->vertexC=posC;

				pMeshNode->m_Tris.AddToEnd(&tri);
			}
			else
			{
				//LOGWARN(g_Log, "Triangle CollisionFlag Not Set: %s %i, %i", pMesh->GetName().c_str(), pMesh->GetUniqueId(), a);
				delete tri;
			}
		}
		
		return pMeshNode;
	}

	void C_Converter::ConstructStaticMeshNode(const Aftk::C_SceneMeshNode* pMesh)
	{
		
		const U32 numVertices(pMesh->GetNumVertexPositions());
		const U32 numTriangles(pMesh->GetNumTriangles(0));
		Aftk::Matrix44 mWorldMatrix=pMesh->GetWorldMatrix();

		for(U32 a = 0; a < numTriangles; ++a)
		{
			const Aftk::C_Polygon& poly(pMesh->GetPolygon(a));
			const Aftk::C_PolygonVertex& pPA = poly.GetPolygonVertex(0);
			const Aftk::C_PolygonVertex& pPB = poly.GetPolygonVertex(1);
			const Aftk::C_PolygonVertex& pPC = poly.GetPolygonVertex(2);

			Aftk::Position3 vPosA = pMesh->GetVertexPosition(pPA.GetPositionIndex());
			Aftk::Position3 vPosB = pMesh->GetVertexPosition(pPB.GetPositionIndex());
			Aftk::Position3 vPosC = pMesh->GetVertexPosition(pPC.GetPositionIndex());
			vPosA = Aftk::ApplyMatrix(mWorldMatrix, vPosA);
			vPosB = Aftk::ApplyMatrix(mWorldMatrix, vPosB);
			vPosC = Aftk::ApplyMatrix(mWorldMatrix, vPosC);

			Maths::HVec3 posA(vPosA.X(), vPosA.Y(), vPosA.Z());
			Maths::HVec3 posB(vPosB.X(), vPosB.Y(), vPosB.Z());
			Maths::HVec3 posC(vPosC.X(), vPosC.Y(), vPosC.Z());
			
			Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			tri->m_CollisionFlag = m_pPolyInform->GetCollisionFlags(poly);
			if(tri->m_CollisionFlag != 0 )
			{
				tri->m_MaterialIndex = m_pPolyInform->GetMaterial(poly);
				
				tri->m_UserDataFlag= m_pPolyInform->GetUserData(poly);
				tri->vertexA=posA;
				tri->vertexB=posB;
				tri->vertexC=posC;

				m_pMeshNode->m_Tris.AddToEnd(&tri);
			}
			else
			{
				//LOGWARN(g_Log, "Triangle CollisionFlag Not Set: %s %i, %i", pMesh->GetName().c_str(), pMesh->GetUniqueId(), a);
				delete tri;
			}
		}
		
	}

	C_ShapeNode::E_ShapeType GetShapeNodeType( const AftkHelpers::E_CollisionType collisionType )
	{
		switch( collisionType )
		{
			case AftkHelpers::E_CollisionType_Sphere:		return C_ShapeNode::E_ShapeType_Sphere; break;
			case AftkHelpers::E_CollisionType_ConvexHull:	return C_ShapeNode::E_ShapeType_ConvexHull; break;
			case AftkHelpers::E_CollisionType_Box:			return C_ShapeNode::E_ShapeType_Box; break;
			case AftkHelpers::E_CollisionType_Cylinder:		return C_ShapeNode::E_ShapeType_Cylinder; break;
			case AftkHelpers::E_CollisionType_Capsule:		return C_ShapeNode::E_ShapeType_Capsule; break;
			default:
				LOGWARN( g_Log, "Shape not supported" );
				return C_ShapeNode::E_ShapeType_Unknown;
			break;
		}
	}

	Gravity::C_ShapeNode* C_Converter::ConstructShapeNode(const Aftk::C_SceneNode* pSceneNode)
	{
		Gravity::C_ShapeNode* pShapeNode=new Gravity::C_ShapeNode;
		
		Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pSceneNode;
	
		const Aftk::C_Polygon* pPolygon = &pMesh->GetPolygon(0);
		pShapeNode->m_MaterialIndex=m_pPolyInform->GetMaterial(*pPolygon);
		pShapeNode->m_CollisionFlag=m_pPolyInform->GetCollisionFlags(*pPolygon);
		if(pShapeNode->m_CollisionFlag == 0)
			LOGWARN(g_Log, "Shape Node Collision Flag Not Set: A3D Scene Id : "+ pSceneNode->GetUniqueId());
		pShapeNode->m_UserDataFlag=m_pPolyInform->GetUserData(*pPolygon);
	
		pShapeNode->m_NodeFlag=SharedPhysics::C_GraphNode::E_Type_Static;


		Maths::HVec3 scale;
		Maths::HVec3 position;
		Maths::HMat33 rotation;
	    ConvertA3DMatrixToTransformationMatrix(pMesh->GetWorldMatrix(), scale, position, rotation);
		pShapeNode->m_Scale = scale;
		pShapeNode->m_Transform=Maths::HMat43(rotation, position);

		const AftkHelpers::E_CollisionType collisionType = AftkHelpers::GetCollisionType(pSceneNode);

		const U32 lodLevel=0;
		U32 noPolys = pMesh->GetNumPolygons(lodLevel);
		if(noPolys > 0 && collisionType != AftkHelpers::E_CollisionType_None)
		{
			pShapeNode->m_ShapeType = GetShapeNodeType( collisionType );

			if( collisionType == AftkHelpers::E_CollisionType_ConvexHull )
			{
				CreateConvexHull(pMesh, scale, pShapeNode->m_VertexList);
				if(pShapeNode->m_VertexList.m_numElements == 0)
				{
					delete pShapeNode;
					return NULL;
				}
			}		
		}

		return pShapeNode;
	}

	void C_Converter::CreateConvexHull(const Aftk::C_SceneMeshNode* pMesh, Maths::HVec3 PCREF scale, SharedPhysics::SortedArray<Maths::HVec3>& vertexList)
	{
		const Aftk::C_Polygon* pPolygon = &pMesh->GetPolygon(0);
		/*const U32 materialIndex = GetMaterial(*pPolygon);
		const U32 collisionFlag = GetCollisionFlags(*pPolygon);*/

		// NOTE : this vertex list contains the vertices for all lods of this mesh
		const U32 noVertices = pMesh->GetNumVertexPositions();
		//SharedPhysics::SortedArray<Maths::HVec3> vertexList(noVertices);
		vertexList.Resize(noVertices);

		for(U32 posIndex = 0; posIndex<noVertices; posIndex++)
		{
			Aftk::Position3 aftkPos = pMesh->GetVertexPosition(posIndex);
			Maths::HVec3 hPos(aftkPos.X(), aftkPos.Y(), -aftkPos.Z());
			hPos *= scale;
			//Now I need to scale it!!!!

			if(hPos.Finite())
			{
				/*Aftk::Position3 aftkPos = pMesh->GetVertexPosition(posIndex);
				Maths::HVec3 hPos(aftkPos.X(), aftkPos.Y(), -aftkPos.Z());

				if(hPos.Finite())*/
				{
					vertexList.AddToEnd(&hPos);
				}
			}
		}
	}

	Gravity::C_ConstraintNode* C_Converter::ConstructConstraintNode(const Aftk::C_SceneNode* pSceneNode)
	{
		Gravity::C_ConstraintNode* pConstraintNode=new Gravity::C_ConstraintNode;
		
		ConvertA3DMatrixToTransformationMatrix(pSceneNode->GetWorldMatrix(), pConstraintNode->m_Transform);
		


		const Aftk::C_Scene* pScene=pSceneNode->GetScenePtr();

		for(U32 j=0; j<pSceneNode->GetNumDynamicPlugs() ; ++j)
		{
			const Aftk::I_Plug& dynPlug = *pSceneNode->GetDynamicPlugPtr(j);
			const std::string plugName=dynPlug.GetName();

			if(plugName == "constraintType")
			{
				//do something with this.
				U32 enumValue;
				dynPlug.GetValue(enumValue);
				pConstraintNode->m_ConstraintType = enumValue;
			}
			else if(plugName == "physicsBodyA")
			{
				std::string bodyPath;
				dynPlug.GetValue(bodyPath);
				
				Aftk::C_SceneNode* pPhysicsBody = const_cast<Aftk::C_SceneNode*>(pScene->GetSceneNodePtrByPartialPath(bodyPath));
				pConstraintNode->m_pPhysicsBodyA=GetShapeNode(pPhysicsBody);
			}
			else if(plugName == "physicsBodyB")
			{
				std::string bodyPath;
				dynPlug.GetValue(bodyPath);

				Aftk::C_SceneNode* pPhysicsBody = const_cast<Aftk::C_SceneNode*>(pScene->GetSceneNodePtrByPartialPath(bodyPath));
				pConstraintNode->m_pPhysicsBodyB=GetShapeNode(pPhysicsBody);
			}
			else if(plugName == "statePhysicsBodyA")
			{
				dynPlug.GetValue(pConstraintNode->m_PhysicsBodyAStatus);
			}
			else if(plugName =="statePhysicsBodyB")
			{
				dynPlug.GetValue(pConstraintNode->m_PhysicsBodyBStatus);
			}
			else if(plugName == "axis")
			{
				dynPlug.GetValue(pConstraintNode->m_Axis);
			}
			else if(plugName == "minSlide")
			{
				dynPlug.GetValue(pConstraintNode->m_MinSlide);
			}
			else if(plugName == "maxSlide")
			{
				dynPlug.GetValue(pConstraintNode->m_MaxSlide);
			}
			else if(plugName == "minRotateX")
			{
				dynPlug.GetValue(pConstraintNode->m_MinRotateX);
			}
			else if(plugName == "maxRotateX")
			{
				dynPlug.GetValue(pConstraintNode->m_MaxRotateX);
			}
			else if(plugName == "minRotateY")
			{
				dynPlug.GetValue(pConstraintNode->m_MinRotateY);
			}
			else if(plugName == "maxRotateY")
			{
				dynPlug.GetValue(pConstraintNode->m_MaxRotateY);
			}
			else if(plugName == "minRotateZ")
			{
				dynPlug.GetValue(pConstraintNode->m_MinRotateZ);
			}
			else if(plugName == "maxRotateZ")
			{
				dynPlug.GetValue(pConstraintNode->m_MaxRotateZ);
			}
		}

		return pConstraintNode;

	}

	S32 C_Converter::AddToConstraint(const Aftk::C_SceneNode* pSceneNode)
	{
		Gravity::C_ConstraintNode* pConstraintNode=ConstructConstraintNode(pSceneNode);
		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
		pConstraintNode->m_UniqueID=counter->GetConstraintId();
		m_Exporter.AddConstraintNode(pConstraintNode);
		return pConstraintNode->m_UniqueID;

	}

	C_ShapeNode* C_Converter::AddToConstraintShape(const Aftk::C_SceneNode* pSceneNode)
	{
		
		Gravity::C_ShapeNode* pShapeNode=ConstructShapeNode(pSceneNode);
		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
		pShapeNode->m_UniqueID=counter->GetConstraintId();
		m_Exporter.AddShapeNode(pShapeNode);

		Gravity::C_ShapeIndex* pShapeIndex=new Gravity::C_ShapeIndex;
		pShapeIndex->m_ShapeNode=pShapeNode;
		pShapeIndex->m_pSceneNode = const_cast<Aftk::C_SceneNode*>(pSceneNode);
		m_ShapeIndex.AddToEnd(&pShapeIndex);
		return pShapeNode;
		//m_ShapeNodes.AddToEnd(&pSceneNode);
	}

	S32 C_Converter::AddToShape(const Aftk::C_SceneNode* pSceneNode)
	{
		
		Gravity::C_ShapeNode* pShapeNode=ConstructShapeNode(pSceneNode);
		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
		pShapeNode->m_UniqueID=counter->GetStaticObjectId();
		m_Exporter.AddShapeNode(pShapeNode);

		Gravity::C_ShapeIndex* pShapeIndex=new Gravity::C_ShapeIndex;
		pShapeIndex->m_ShapeNode=pShapeNode;
		pShapeIndex->m_pSceneNode = const_cast<Aftk::C_SceneNode*>(pSceneNode);
		m_ShapeIndex.AddToEnd(&pShapeIndex);
		return pShapeNode->m_UniqueID;
		//m_ShapeNodes.AddToEnd(&pSceneNode);
	}

	S32 C_Converter::AddToMesh(const Aftk::C_SceneNode* pSceneNode)
	{
		Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pSceneNode;
		Gravity::C_MeshNode* pMeshNode=ConstructMeshNode(pMesh);
		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
		pMeshNode->m_UniqueID=counter->GetStaticObjectId();
		m_Exporter.AddMeshNode(pMeshNode);
		return pMeshNode->m_UniqueID;
		
		//m_MeshNodes.AddToEnd(&pSceneNode);
	}

	void C_Converter::AddToStaticMesh(const Aftk::C_SceneNode* pSceneNode)
	{
		Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pSceneNode;
		ConstructStaticMeshNode(pMesh);
	}

	S32 C_Converter::CreateStaticMeshNode()
	{
		Gravity::C_MeshNode* pMeshNode=new Gravity::C_MeshNode;
		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
		pMeshNode->m_UniqueID=counter->GetStaticObjectId();
		m_pMeshNode = pMeshNode;
		m_Exporter.AddMeshNode(pMeshNode);
		return pMeshNode->m_UniqueID;
	}


	S32 C_Converter::AddShapes(const Aftk::C_SceneNode* pSceneNode)
	{
		if(pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
		{
			if( AftkHelpers::GetCollisionType( pSceneNode ) != AftkHelpers::E_CollisionType_None )
			{
				return AddToShape(pSceneNode);
			}
			else
			{
				if(AftkUt::Query::IsBaseInstance(pSceneNode))
				{
					return -1;
				}
				return AddToMesh(pSceneNode);
			}
		}
		return -1;
	}

	S32 C_Converter::AddToInstance(const Aftk::C_SceneNode* pSceneNode)
	{
		/*Aftk::C_SceneInstanceNode *pInstance = (Aftk::C_SceneInstanceNode*)pSceneNode;
		Aftk::C_SceneNode *pBaseInstance = pInstance->GetSourceNodePtr();*/
		const Aftk::C_SceneNode *pBaseInstance = AftkHelpers::InstanceGetSourceNode(pSceneNode);

		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();

		
		U32 id=counter->GetInstanceId();

		if(pBaseInstance->HasType(Aftk::E_ObjType_SceneMesh))
		{

			Gravity::C_InstanceNode* pConverterInstance=new Gravity::C_InstanceNode;

			S32 a3dId=pBaseInstance->GetUniqueId();

			if(!m_Instances.Find(&a3dId))
			{
				m_Instances.AddToEnd(&a3dId);
				
				Gravity::C_InstanceBaseNode* pConverterBase=new Gravity::C_InstanceBaseNode;
				
				/*
					Here might need to use another matrix
				*/
				//pConverterInstance->m_Transform=GetRelativeTransformationMatrix(pBaseInstance, pInstance);
				/*Maths::HMat43 trans;
				Aftk::TemplateToNightmareMatrix43(trans, pSceneNode->GetWorldMatrix());
				Maths::HMat33 rotation=trans.Rotation();
				if(rotation.Determinant().LE(Maths::Constants<Maths::HF32>::Zero()))
				{
					rotation.AZ(-rotation.AZ());
				}
				pConverterInstance->m_Transform=Maths::HMat43(rotation, trans.Translation());*/
				ConvertA3DMatrixToTransformationMatrix(pSceneNode->GetWorldMatrix(), pConverterInstance->m_Transform);
				pConverterInstance->m_UniqueID=id;
				
			
				Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pBaseInstance;

				//Gravity::C_MeshNode* pMeshNode=new Gravity::C_MeshNode;
				Gravity::C_MeshNode* pMeshNode=ConstructMeshNode(pMesh);
				if(pMeshNode == NULL)
					return -1;
				pConverterBase->m_pMesh=pMeshNode;
				pConverterBase->m_UniqueID=counter->GetBaseInstanceId();
				pConverterBase->m_A3dId=a3dId;

				pConverterInstance->m_BaseNodeID=pConverterBase->m_UniqueID;

				pConverterBase->m_InstanceNodes.AddToEnd(&pConverterInstance);

				
				m_Exporter.AddInstanceBaseNode(pConverterBase);
				
			}
			else
			{
				
				Gravity::C_InstanceBaseNode* pConverterBase=m_Exporter.GetInstanceBaseNode(a3dId);
				pConverterInstance->m_UniqueID=id;
				pConverterInstance->m_BaseNodeID=pConverterBase->m_UniqueID;

				/*Maths::HMat43 trans;
				Aftk::TemplateToNightmareMatrix43(trans, pSceneNode->GetWorldMatrix());
				Maths::HMat33 rotation=trans.Rotation();
				if(rotation.Determinant().LE(Maths::Constants<Maths::HF32>::Zero()))
				{
					rotation.AZ(-rotation.AZ());
				}
				pConverterInstance->m_Transform=Maths::HMat43(rotation, trans.Translation());*/
				
				ConvertA3DMatrixToTransformationMatrix(pSceneNode->GetWorldMatrix(), pConverterInstance->m_Transform);
				//pConverterInstance->m_Transform=GetRelativeTransformationMatrix(pBaseInstance, pInstance);
				pConverterBase->m_InstanceNodes.AddToEnd(&pConverterInstance);

			}
			
		}
		else //normal shape
		{

			//Aftk::C_SceneNode* pSceneNode=m_ShapeNodes[i];
			Gravity::C_ShapeNode* pShapeNode=new Gravity::C_ShapeNode;
		//	pShapeNode->m_Node=pSceneNode;
			if(pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
			{
				Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pSceneNode;
				
				if(pMesh->GetNumPolygons(0) > 0)
				{
					Aftk::C_Polygon polygon=pMesh->GetPolygon(0);
					pShapeNode->m_MaterialIndex=m_pPolyInform->GetMaterial(polygon);
					pShapeNode->m_CollisionFlag=m_pPolyInform->GetCollisionFlags(polygon);
					pShapeNode->m_UserDataFlag=m_pPolyInform->GetUserData(polygon);
				}
			}
//			pShapeNode->m_Type=Gravity::GE_Shape_Type;
			pShapeNode->m_NodeFlag=SharedPhysics::C_GraphNode::E_Type_Static;
			pShapeNode->m_UniqueID=id;
			m_Exporter.AddShapeNode(pShapeNode);
		}

		return id;
	}



	
	void C_Converter::AddStaticNodes( const C_AftkSceneNodeList& aftkSceneNodeList )
	{
		
		U32 size = aftkSceneNodeList.Size();
		for(U32 i=0; i< size; ++i)
		{
			const Aftk::C_SceneNode* pSceneNode = aftkSceneNodeList[i];
			//Instance node
			if(AftkUt::Query::IsInstance(pSceneNode))
			{
				U32 id=AddToInstance(pSceneNode);
	
				continue;
			}
		
			if(pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
			{
				if( AftkHelpers::GetCollisionType( pSceneNode ) != AftkHelpers::E_CollisionType_None )
				{
					U32 id=AddToShape(pSceneNode);
				}
				else
				{
					if(AftkUt::Query::IsBaseInstance(pSceneNode))
					{
						continue;
					}
					AddToStaticMesh(pSceneNode);
				}
			}
		}
	}


	/*
			This will export a compound shape by gluing together all the aftk scene nodes provided which may be mesh or physics shapes. 
			This will not actually create a physics body in the game, just the shape.
			If this list contains instance nodes, the instances can be expanded out.
			The matrix provided is a world space matrix which will be orthonormal. 
			This will be the world space body position when the object is created in game so the shape components will need to be transformed by the inverse of this.
	*/

	U32 C_Converter::AddDynamicShape( const C_AftkSceneNodeList& aftkSceneNodeList, const Maths::HMat43& originInWorldSpace )
	{
		U32 size = aftkSceneNodeList.Size();

		if(size == 0)
		{
			LOGWARN(g_Log, "Aftk Scene Node List is empty!!!!!");
			return -1;
		}

		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();

		Gravity::C_CompoundNode* pCompoundNode=BuildCompoundNode(aftkSceneNodeList, originInWorldSpace);
		if(pCompoundNode == NULL)
			return -1;
		pCompoundNode->m_NodeFlag=SharedPhysics::C_GraphNode::E_Type_Active;
		
		pCompoundNode->m_UniqueID=counter->GetDynamicObjectId();
		m_Exporter.AddCompoundNode(pCompoundNode);

		return pCompoundNode->m_UniqueID;
		
	}

	Gravity::C_CompoundNode* C_Converter::BuildCompoundNode(const C_AftkSceneNodeList& aftkSceneNodeList, const Maths::HMat43 PCREF worldMatrix)
	{
		
		Gravity::C_CompoundNode* pCompoundNode=new Gravity::C_CompoundNode;

		pCompoundNode->m_Transform = worldMatrix;

		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();

		U32 subDynamicIndex = 0;

		const U32 size = aftkSceneNodeList.Size();

		for(S32 i=0; i<aftkSceneNodeList.Size(); ++i)
		{
			const Aftk::C_SceneNode* pChild = aftkSceneNodeList[i];

			if(AftkUt::Query::IsInstance(pChild))
			{
				Aftk::C_SceneInstanceNode *pInstance = (Aftk::C_SceneInstanceNode*)pChild;
				Aftk::C_SceneNode *pBaseInstance = pInstance->GetSourceNodePtr();
				Aftk::C_SceneMeshNode* pMesh=(Aftk::C_SceneMeshNode* )pBaseInstance;
				Gravity::C_MeshNode* pMeshNode=ConstructMeshNode(pMesh, pInstance->GetWorldMatrix());
				if(pMeshNode == NULL)
					continue;
				pMeshNode->m_UniqueID = counter->GetSubDynamicObjectId(subDynamicIndex++);
				pCompoundNode->AddNode(pMeshNode);
				
				continue;
			}

			if(pChild->HasType(Aftk::E_ObjType_SceneMesh))
			{
				if( AftkHelpers::GetCollisionType( pChild ) != AftkHelpers::E_CollisionType_None )
				{
					Gravity::C_ShapeNode* pShapeNode=ConstructShapeNode(pChild);
					if(pShapeNode == NULL)
						continue;
					pShapeNode->m_UniqueID=counter->GetSubDynamicObjectId(subDynamicIndex++);

					Maths::HMat43 transform(Maths::UNDEFINED);

					ConvertA3DMatrixToTransformationMatrix(pChild->GetWorldMatrix(), transform);

					Maths::HMat33 relativeRotation= transform.Rotation() * worldMatrix.Rotation().InverseRotation();

					Maths::HVec3 translation = transform.Translation() - worldMatrix.Translation();

					pShapeNode->m_Transform = Maths::HMat43(relativeRotation, translation);
					
					pCompoundNode->AddNode(pShapeNode);
				}
				else
				{
	
					Aftk::C_SceneMeshNode* pMesh=(Aftk::C_SceneMeshNode*) pChild;
					Aftk::Matrix44 childWorld=pChild->GetWorldMatrix();
				
					Gravity::C_MeshNode* pMeshNode=ConstructMeshNode(pMesh, childWorld);
					if(pMeshNode == NULL)
						continue;
					pMeshNode->m_UniqueID=counter->GetSubDynamicObjectId(subDynamicIndex++);
					pCompoundNode->AddNode(pMeshNode);

				}
			}
			/*else
			{
				C_AftkSceneNodeList aftkSceneNodeList;

				aftkSceneNodeList.PushBack<Aftk::C_SceneNode*>(const_cast<Aftk::C_SceneNode*>(pChild));
	
				U32 size=pChild->GetNumChildren();
				for(U32 i=0; i< size; ++i)
				{
					Aftk::C_SceneNode* pSubChild = (Aftk::C_SceneNode*)pChild->GetChildPtr(i);
					aftkSceneNodeList.PushBack<Aftk::C_SceneNode*>(pSubChild);	
				}

				Maths::HMat43 transform(Maths::UNDEFINED);
				ConvertA3DMatrixToTransformationMatrix(pChild->GetWorldMatrix(), transform);

				Gravity::C_CompoundNode* pSubCompoundNode=BuildCompoundNode(aftkSceneNodeList, transform);
				
				pSubCompoundNode->m_Flag=SharedPhysics::C_GraphNode::E_Type_Static;
				pSubCompoundNode->m_UniqueID= counter->GetSubDynamicObjectId(subDynamicIndex++);

				Maths::HMat33 relativeRotation= transform.Rotation() * worldMatrix.Rotation().InverseRotation();

				Maths::HVec3 translation = transform.Translation() - worldMatrix.Translation();

				pSubCompoundNode->m_Transform = Maths::HMat43(relativeRotation, translation);
				
				//ConvertA3DMatrixToTransformationMatrix(pChild->GetLocalMatrix(), pSubCompoundNode->m_Transform);
				//pSubCompoundNode->m_Transform.m_Translate.Z(-pSubCompoundNode->m_Transform.m_Translate.Z());
				pCompoundNode->AddNode(pSubCompoundNode);
			}*/
		}
		
		if(pCompoundNode->m_Nodes.m_numElements == 0)
		{
			delete pCompoundNode;
			return NULL;
		}
		return pCompoundNode;
	}

	bool C_Converter::Process()
	{
		m_Processed = true;
		return m_Exporter.Process();
	}
	

	 bool C_Converter::Export( Sys::I_BinaryStream* pFile )
	 {
		assert(m_Processed);
		return m_Exporter.Export(pFile);
	 }
	 
}