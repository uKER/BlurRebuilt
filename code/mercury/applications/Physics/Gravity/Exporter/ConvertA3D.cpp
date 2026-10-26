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

#include "gravity/sdk/ConstraintDescriptors.h"
//#include "gravity/sdk/ConstraintDescriptors.inl"



#include "gravity/exporter/ExportPrimitivesList.h"
//#include "gravity/exporter/ExportPrimitivesList.inl"

#include "system/serialise/C_FileSystem.h"
#include "system/serialise/I_BinaryStream.h"

#include "gravity/exporter/GravityExporter.h"
#include "gravity/exporter/GravityConverter.h"
//#include "gravity/exporter/GravityConverter.inl"
#include "gravity/narrowphase/OptimisedAABBTree.inl"
#include "ConvertA3D.h"
#include <core/maths/hvec/new/IncludeGeneral.h>


#define WRITE_MESH_NODE_POINT

namespace Gravity
{
	Debug::Logger &g_Log = Debug::GetLogger("gravity.export");

	Maths::HMat43 FlipAftkMatrix(const Maths::HMat43 PCREF m)
	{
		Maths::HMat43 rotMatrix = Maths::HMat43( Maths::HMat33( Maths::RotY( Maths::Pi ) ) );
		Maths::HMat43 nodeHMat43 = rotMatrix * m;

		return nodeHMat43;
	}


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
				tri->m_PhysicsMaterialIndex = m_pPolyInform->GetPhysicsMaterial(poly);
				tri->m_EffectMaterialIndex = m_pPolyInform->GetEffectMaterial(poly);
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
		
	
		if(pMeshNode->m_Tris.m_numElements != 0)
		{
			return pMeshNode;
		}
		else
		{
			delete pMeshNode;
			return NULL;
		}
		
	}

	

	Gravity::C_MeshNode* C_Converter::ConstructMeshNode(const Aftk::C_SceneMeshNode* pMesh, const Aftk::Matrix44 PCREF trans)
	{
	

		const U32 numTriangles(pMesh->GetNumTriangles(0));

		if(numTriangles == 0)
			return NULL; // no mesh

		Gravity::C_MeshNode* pMeshNode=new Gravity::C_MeshNode;

		//const U32 numVertices(pMesh->GetNumVertexPositions());
		
		//Aftk::Matrix44 mWorldMatrix=pMesh->GetWorldMatrix();

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

		/*	posA = mat * posA;
			posB = mat * posB;
			posC = mat * posC;*/
			
			Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			
			tri->m_PhysicsMaterialIndex = m_pPolyInform->GetPhysicsMaterial(poly);
			tri->m_EffectMaterialIndex = m_pPolyInform->GetEffectMaterial(poly);
			tri->m_CollisionFlag = m_pPolyInform->GetCollisionFlags(poly);
			if(tri->m_CollisionFlag != 0)
			{
			
				tri->m_UserDataFlag= m_pPolyInform->GetUserData(poly);
				tri->vertexA=posA;
				tri->vertexB=posB;
				tri->vertexC=posC;

				pMeshNode->m_Tris.AddToEnd(&tri);
			}
			else
			{
				LOGWARN(g_Log, "Triangle CollisionFlog Not Set: %s %i, %i", pMesh->GetName().c_str(), pMesh->GetUniqueId(), a);
				delete tri;
			}
		}
		
		return pMeshNode;
	}

	Gravity::C_MeshNode* C_Converter::ConstructFluidMeshNode(const Aftk::C_SceneMeshNode* pMesh)
	{

		const U32 numTriangles(pMesh->GetNumTriangles(0));

		if(numTriangles == 0)
			return NULL;
		Gravity::C_MeshNode* pMeshNode=new Gravity::C_MeshNode;

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
			tri->m_CollisionFlag = m_FluidCollisionFlag;
		
			tri->m_PhysicsMaterialIndex = m_pPolyInform->GetPhysicsMaterial(poly);
			tri->m_EffectMaterialIndex = m_pPolyInform->GetEffectMaterial(poly);
			tri->m_UserDataFlag= m_pPolyInform->GetUserData(poly);
			tri->vertexA=posA;
			tri->vertexB=posB;
			tri->vertexC=posC;
			pMeshNode->m_Tris.AddToEnd(&tri);
		}
		
		return pMeshNode;
	}


	Gravity::C_MeshNode* C_Converter::ConstructFluidMeshNode(const Aftk::C_SceneMeshNode* pMesh, const Aftk::Matrix44 PCREF trans)
	{
		const U32 numTriangles(pMesh->GetNumTriangles(0));

		if(numTriangles == 0)
			return NULL;
		Gravity::C_MeshNode* pMeshNode=new Gravity::C_MeshNode;

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
			tri->m_CollisionFlag = m_FluidCollisionFlag;
			
			tri->m_PhysicsMaterialIndex = m_pPolyInform->GetPhysicsMaterial(poly);
			tri->m_EffectMaterialIndex = m_pPolyInform->GetEffectMaterial(poly);
			tri->m_UserDataFlag= m_pPolyInform->GetUserData(poly);
			tri->vertexA=posA;
			tri->vertexB=posB;
			tri->vertexC=posC;

			pMeshNode->m_Tris.AddToEnd(&tri);
		
		}
		
		return pMeshNode;
	}


	void C_Converter::ConstructStaticSplineMeshNode(const Aftk::C_SceneSplineNode* pSplineNode)
	{
		//Generate triangles from spline!

		const U32 numCVs = pSplineNode->GetNumCVs();

		if(numCVs < 2)
			return;

		
		const F32 height(50.f);
		//For the 3D spline
		F32 positiveHeight = height;
		F32 negativeHeight = height;
		
	
		AftkUt::C_NodeFlagParser parser(*pSplineNode);

		if(parser.GetFlagGroupName() == "Collide_3D")
		{
			std::string posHeight("positive_height");
			std::string negHeight("negative_height");
			if (!parser.GetParameterValue(posHeight, positiveHeight) ||
				!parser.GetParameterValue(negHeight, negativeHeight))
			{
				LOGDEBUG(g_Log, "Collide_3D spline doesn't have height parameters!!!");
				return;
			}

		}
		else if(parser.GetFlagGroupName() != "Collide_2D")
		{
			return;
		}

		negativeHeight = -negativeHeight;
		F32 positiveHalfHeight = positiveHeight/2.f;
		F32 negativeHalfHeight = (negativeHeight/2.f);


		//TODO - implement collision flags/material indices!
		const U32 collisionFlags(0xffffffff);
		const U32 materialIndex = 0;
		const U32 userData = 0;

		const Aftk::Matrix44 matrix = pSplineNode->GetWorldMatrix();
		
		Aftk::Position3 pointA = Aftk::ApplyMatrix(matrix, pSplineNode->GetCV(0));
		for(U32 a = 1; a < numCVs; ++a)
		{
			const Aftk::Position3 pointB = Aftk::ApplyMatrix(matrix, pSplineNode->GetCV(a));

			

			//OK - extrude this up/down by 100m!

			const Maths::HF32 ax(pointA.X());
			const Maths::HF32 ay(pointA.Y());
			const Maths::HF32 az(pointA.Z());

			const Maths::HF32 bx(pointB.X());
			const Maths::HF32 by(pointB.Y());
			const Maths::HF32 bz(pointB.Z());

			const Maths::HVec3 pA(ax, ay, az);
			const Maths::HVec3 pB(bx, by, bz);

		
			const Maths::HVec3 baseA(ax, ay + negativeHeight, az);
			const Maths::HVec3 topA(ax,  ay + positiveHeight, az);
			const Maths::HVec3 midBaseA(ax, ay + negativeHalfHeight, az);
			const Maths::HVec3 midTopA(ax, ay + positiveHalfHeight, az);
			
			const Maths::HVec3 baseB(bx, by + negativeHeight, bz);
			const Maths::HVec3 topB(bx,  by + positiveHeight, bz);
			const Maths::HVec3 midBaseB(bx, by + negativeHalfHeight, bz);
			const Maths::HVec3 midTopB(bx, by + positiveHalfHeight, bz);



			//Add in triangles now

			{
				Gravity::C_ConverterTriangle* tri =new Gravity::C_ConverterTriangle;
				tri->m_PhysicsMaterialIndex = materialIndex;
				tri->m_EffectMaterialIndex = materialIndex;
				tri->m_CollisionFlag = collisionFlags;
				tri->m_UserDataFlag = userData;
				tri->vertexA = baseA;
				tri->vertexB = baseB;
				tri->vertexC = midBaseB;
				m_pSplineMeshNode->m_Tris.AddToEnd(&tri);
			}
			{
				Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
				
				tri->m_PhysicsMaterialIndex = materialIndex;
				tri->m_EffectMaterialIndex = materialIndex;
				tri->m_CollisionFlag = collisionFlags;
				tri->m_UserDataFlag= userData;
				tri->vertexA=baseA;
				tri->vertexB=midBaseB;
				tri->vertexC=midBaseA;
				m_pSplineMeshNode->m_Tris.AddToEnd(&tri);
			}
			{
				Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			
				tri->m_PhysicsMaterialIndex = materialIndex;
				tri->m_EffectMaterialIndex = materialIndex;
				tri->m_CollisionFlag = collisionFlags;
				tri->m_UserDataFlag= userData;
				tri->vertexA=midBaseA;
				tri->vertexB=midBaseB;
				tri->vertexC=pB;
				m_pSplineMeshNode->m_Tris.AddToEnd(&tri);
			}

			{
				Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			
				tri->m_PhysicsMaterialIndex = materialIndex;
				tri->m_EffectMaterialIndex = materialIndex;
				tri->m_CollisionFlag = collisionFlags;
				tri->m_UserDataFlag= userData;
				tri->vertexA=midBaseA;
				tri->vertexB=pB;
				tri->vertexC=pA;
				m_pSplineMeshNode->m_Tris.AddToEnd(&tri);
			}

			{
				Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			
				tri->m_PhysicsMaterialIndex = materialIndex;
				tri->m_EffectMaterialIndex = materialIndex;
				tri->m_CollisionFlag = collisionFlags;
				tri->m_UserDataFlag= userData;
				tri->vertexA=pA;
				tri->vertexB=pB;
				tri->vertexC=midTopB;
				m_pSplineMeshNode->m_Tris.AddToEnd(&tri);
			}

			{
				Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			
				tri->m_PhysicsMaterialIndex = materialIndex;
				tri->m_EffectMaterialIndex = materialIndex;
				tri->m_CollisionFlag = collisionFlags;
				tri->m_UserDataFlag= userData;
				tri->vertexA=pA;
				tri->vertexB=midTopB;
				tri->vertexC=midTopA;
				m_pSplineMeshNode->m_Tris.AddToEnd(&tri);
			}

			{
				Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			
				tri->m_PhysicsMaterialIndex = materialIndex;
				tri->m_EffectMaterialIndex = materialIndex;
				tri->m_CollisionFlag = collisionFlags;
				tri->m_UserDataFlag= userData;
				tri->vertexA=midTopA;
				tri->vertexB=midTopB;
				tri->vertexC=topB;
				m_pSplineMeshNode->m_Tris.AddToEnd(&tri);
			}

			{
				Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
			
				tri->m_PhysicsMaterialIndex = materialIndex;
				tri->m_EffectMaterialIndex = materialIndex;
				tri->m_CollisionFlag = collisionFlags;
				tri->m_UserDataFlag= userData;
				tri->vertexA=midTopA;
				tri->vertexB=topB;
				tri->vertexC=topA;
				m_pSplineMeshNode->m_Tris.AddToEnd(&tri);
			}

			//Back up original point!
			pointA = pointB;
		}
	}


	void C_Converter::ConstructStaticMeshNode(const Aftk::C_SceneMeshNode* pMesh)
	{
		
		const U32 numVertices(pMesh->GetNumVertexPositions());
		const U32 numTriangles(pMesh->GetNumTriangles(0));
		Aftk::Matrix44 mWorldMatrix=pMesh->GetWorldMatrix();

#ifdef WRITE_MESH_NODE_POINT
		{
			static U32 count =0;
			Btl::String filename;
			filename = Btl::String::Format("z:\\LevelMesh%i.dat", count++);
			Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(filename.Chars(), Sys::E_FileAccess_Write );

			if(pFile)
			{
				pFile->SetEndianSwapFlag( true );
				{
					Sys::BinarySerialiser<Ut::DoEndianSwap> ar(*pFile);

					ar << numTriangles;

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

						ar << posA << posB << posC;
					}

					SAFE_DELETE_PTR(pFile);
				}
			}		
		}
#endif


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
			
			tri->m_PhysicsMaterialIndex = m_pPolyInform->GetPhysicsMaterial(poly);
			tri->m_EffectMaterialIndex = m_pPolyInform->GetEffectMaterial(poly);
			
			tri->m_CollisionFlag = m_pPolyInform->GetCollisionFlags(poly);
			if(tri->m_CollisionFlag != 0)
			{
					
				//TEMP - test this for collision flags in a dodgy level!
				//assert(tri->m_MaterialIndex < 64);
				tri->m_UserDataFlag= m_pPolyInform->GetUserData(poly);
				tri->vertexA=posA;
				tri->vertexB=posB;
				tri->vertexC=posC;

				m_pMeshNode->m_Tris.AddToEnd(&tri);
			}
			else
			{
				LOGWARN(g_Log, "Triangle CollisionFlog Not Set: %s %i, %i", pMesh->GetName().c_str(), pMesh->GetUniqueId(), a);			
				delete tri;
			}
		}
		
	}

	Gravity::C_ClothNode* C_Converter::ConstructClothNode(const Aftk::C_SceneNode* pSceneNode)
	{
		Gravity::C_ClothNode* pClothNode = new Gravity::C_ClothNode;

		const Aftk::C_SceneMeshNode* pMesh=(Aftk::C_SceneMeshNode* )pSceneNode;
		
		const U32 numVertices(pMesh->GetNumVertexPositions());
		const U32 numTriangles(pMesh->GetNumTriangles(0));
		
		Aftk::Matrix44 mWorldMatrix=pMesh->GetWorldMatrix();

		/*
			Construct triangle mesh
		*/
		//U32 index =0;

		S32 vertexColourChannel = -1;
		for (U32 i = 0; i < pMesh->GetNumVertexDataChannels(); i++)
		{
			const Aftk::C_VertexDataChannelInfo& info = pMesh->GetVertexDataChannelInfo(i);
			if (info.GetName() == AFTK_DATA_CHANNEL_USAGE_VERTEX_COLOUR &&
				info.GetDataType() == Aftk::E_GenericAttributeType_U32)
			{
				vertexColourChannel = i;
				break;
			}
		}

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
			if (vertexColourChannel != -1)
			{
				U32 colour;
				pPA.GetData(colour, vertexColourChannel);
				tri->m_ColourA = Maths::RGBA(colour);
			}

			if (vertexColourChannel != -1)
			{
				U32 colour;
				pPB.GetData(colour, vertexColourChannel);
				tri->m_ColourB = Maths::RGBA(colour);
			}

			if (vertexColourChannel != -1)
			{
				U32 colour;
				pPC.GetData(colour, vertexColourChannel);
				tri->m_ColourC = Maths::RGBA(colour);
			}
			
			tri->m_PhysicsMaterialIndex = m_pPolyInform->GetPhysicsMaterial(poly);
			tri->m_EffectMaterialIndex = m_pPolyInform->GetEffectMaterial(poly);
			
			tri->m_CollisionFlag = m_pPolyInform->GetCollisionFlags(poly);
			
					
			//TEMP - test this for collision flags in a dodgy level!
			//assert(tri->m_MaterialIndex < 64);
			tri->m_UserDataFlag= m_pPolyInform->GetUserData(poly);
			tri->vertexA=posA;
			tri->vertexB=posB;
			tri->vertexC=posC;

			//tri->m_IndexA = index++;
			//tri->m_IndexB = index++;
			//tri->m_IndexC = index++;

			pClothNode->m_Tris.AddToEnd(&tri);
		}

		/*
			Construct Edge
		*/

		return pClothNode;

	}


	//Gravity::C_FluidMeshNode* C_Converter::ConstructFluidMeshNode(const Aftk::C_SceneNode* pSceneNode)
	//{
	//	Gravity::C_FluidMeshNode* pFluidMeshNode = new Gravity::C_FluidMeshNode;

	//	const Aftk::C_SceneMeshNode* pMesh=(Aftk::C_SceneMeshNode* )pSceneNode;
	//	
	//	const U32 numVertices(pMesh->GetNumVertexPositions());
	//	const U32 numTriangles(pMesh->GetNumTriangles(0));
	//	
	//	Aftk::Matrix44 mWorldMatrix=pMesh->GetWorldMatrix();

	//	/*
	//		Construct triangle mesh
	//	*/
	//

	//	for(U32 a = 0; a < numTriangles; ++a)
	//	{
	//		const Aftk::C_Polygon& poly(pMesh->GetPolygon(a));
	//		const Aftk::C_PolygonVertex& pPA = poly.GetPolygonVertex(0);
	//		const Aftk::C_PolygonVertex& pPB = poly.GetPolygonVertex(1);
	//		const Aftk::C_PolygonVertex& pPC = poly.GetPolygonVertex(2);

	//		Aftk::Position3 vPosA = pMesh->GetVertexPosition(pPA.GetPositionIndex());
	//		Aftk::Position3 vPosB = pMesh->GetVertexPosition(pPB.GetPositionIndex());
	//		Aftk::Position3 vPosC = pMesh->GetVertexPosition(pPC.GetPositionIndex());
	//		
	//		vPosA = Aftk::ApplyMatrix(mWorldMatrix, vPosA);
	//		vPosB = Aftk::ApplyMatrix(mWorldMatrix, vPosB);
	//		vPosC = Aftk::ApplyMatrix(mWorldMatrix, vPosC);

	//		Maths::HVec3 posA(vPosA.X(), vPosA.Y(), vPosA.Z());
	//		Maths::HVec3 posB(vPosB.X(), vPosB.Y(), vPosB.Z());
	//		Maths::HVec3 posC(vPosC.X(), vPosC.Y(), vPosC.Z());
	//		
	//		Gravity::C_ConverterTriangle* tri = new Gravity::C_ConverterTriangle;
	//		
	//		tri->m_PhysicsMaterialIndex = m_pPolyInform->GetPhysicsMaterial(poly);
	//		tri->m_EffectMaterialIndex = m_pPolyInform->GetEffectMaterial(poly);
	//		
	//		tri->m_CollisionFlag = m_pPolyInform->GetCollisionFlags(poly);
	//		tri->m_UserDataFlag= m_pPolyInform->GetUserData(poly);
	//		tri->vertexA=posA;
	//		tri->vertexB=posB;
	//		tri->vertexC=posC;

	//		pFluidMeshNode->m_Tris.AddToEnd(&tri);
	//	}

	//	return pFluidMeshNode;

	//}

	const E_CollisionType C_Converter::GetCollisionType( const Aftk::C_SceneNode* pAftkSceneNode )
	{	
		for (U32 i = 0; i < pAftkSceneNode->GetNumDynamicPlugs(); i++)
		{
			const Aftk::I_Plug& dynPlug = *pAftkSceneNode->GetDynamicPlugPtr(i);
			if (dynPlug.GetName() == "collisionPrimitive")
			{
				bool hasIntValue = false;
				S32 value = 0;
				if (dynPlug.GetDataType() == Aftk::E_GenericAttributeType_U32)
				{
					dynPlug.GetValue((U32&)value);
					hasIntValue = true;
				}
				else 
				if (dynPlug.GetDataType() == Aftk::E_GenericAttributeType_S32)
				{
					dynPlug.GetValue(value);
					hasIntValue = true;
				}

				if( hasIntValue )
				{
					switch(value)
					{
						case 0:
							return E_CollisionType_Sphere;
						break;

						case 1:
							return E_CollisionType_ConvexHull;
						break;

						case 2:
							return E_CollisionType_Box;
						break;

						case 3:
							return E_CollisionType_Cylinder;
						break;

						case 4:
							return E_CollisionType_Capsule;
						break;

						case 5:
							return E_CollisionType_ConvexHull;
						break;

						default:
						break;
					}
				}
				else
				{
					if (dynPlug.GetDataType() == Aftk::E_GenericAttributeType_String)
					{
						std::string value;
						dynPlug.GetValue(value);

						if(_stricmp( value.c_str(), "sphere" ) == 0 )
						{
							return E_CollisionType_Sphere;
						}

						if(_stricmp( value.c_str(), "convex hull" ) == 0 )
						{
							return E_CollisionType_ConvexHull;
						}

						if(_stricmp( value.c_str(), "box" ) == 0 )
						{
							return E_CollisionType_Box;
						}

						if(_stricmp( value.c_str(), "cylinder" ) == 0 )
						{
							return E_CollisionType_Cylinder;
						}

						if(_stricmp( value.c_str(), "capsule" ) == 0 )
						{
							return E_CollisionType_Capsule;
						}

						if(_stricmp( value.c_str(), "cone" ) == 0 )
						{
							return E_CollisionType_ConvexHull;						
							//return E_CollisionType_Cone;
						}
					}
				}
			}
		}

		return E_CollisionType_None;
	}


	C_ShapeNode::E_ShapeType C_Converter::GetShapeNodeType( const E_CollisionType collisionType )
	{
		switch( collisionType )
		{
			case E_CollisionType_Sphere:		return C_ShapeNode::E_ShapeType_Sphere; break;
			case E_CollisionType_ConvexHull:	return C_ShapeNode::E_ShapeType_ConvexHull; break;
			case E_CollisionType_Box:			return C_ShapeNode::E_ShapeType_Box; break;
			case E_CollisionType_Cylinder:		return C_ShapeNode::E_ShapeType_Cylinder; break;
			case E_CollisionType_Capsule:		return C_ShapeNode::E_ShapeType_Capsule; break;
			case E_CollisionType_Cone:			return C_ShapeNode::E_ShapeType_Cone; break;
			default:
				LOGWARN( g_Log, "Shape not supported" );
				return C_ShapeNode::E_ShapeType_Unknown;
			break;
		}
	}

	Gravity::C_ShapeNode* C_Converter::ConstructShapeNode(const Aftk::C_SceneNode* pSceneNode)
	{

		Gravity::C_ShapeNode* pShapeNode=new Gravity::C_ShapeNode;

		const Aftk::C_SceneNode *pBaseInst = pSceneNode;
		if(AftkUt::Query::IsInstance(pSceneNode))
			pBaseInst = AftkUt::Query::GetTopLevelSourceNodePtr(pSceneNode);
		if (!pBaseInst || !pBaseInst->HasType(Aftk::E_ObjType_SceneMesh))
			return NULL;

		Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pBaseInst;

		Maths::HVec3 scale;
		Maths::HVec3 position;
		Maths::HMat33 rotation;
	    ConvertA3DMatrixToTransformationMatrix(pSceneNode->GetWorldMatrix(), scale, position, rotation);
		pShapeNode->m_Scale = scale;
		pShapeNode->m_Transform=Maths::HMat43(rotation, position);

		const Aftk::C_Polygon* pPolygon = &pMesh->GetPolygon(0);
		pShapeNode->m_PhysicsMaterialIndex=m_pPolyInform->GetPhysicsMaterial(*pPolygon);
		pShapeNode->m_EffectMaterialIndex =m_pPolyInform->GetEffectMaterial(*pPolygon);
		pShapeNode->m_CollisionFlag=m_pPolyInform->GetCollisionFlags(*pPolygon);
		if(pShapeNode->m_CollisionFlag == 0)
			LOGWARN(g_Log, "Shape Node Collision Flag Not Set: A3D Scene Id : %s", pSceneNode->GetPath().c_str());
		pShapeNode->m_UserDataFlag=m_pPolyInform->GetUserData(*pPolygon);
	
		pShapeNode->m_NodeFlag=SharedPhysics::C_GraphNode::E_Type_Static;


		const E_CollisionType collisionType = GetCollisionType(pSceneNode);

		const U32 lodLevel=0;
		U32 noPolys = pMesh->GetNumPolygons(lodLevel);
		if(noPolys > 0 && collisionType != E_CollisionType_None)
		{
			pShapeNode->m_ShapeType = GetShapeNodeType( collisionType );

			if( collisionType == E_CollisionType_ConvexHull )
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

		// NOTE : this vertex list contains the vertices for all lods of this mesh
		const U32 noVertices = pMesh->GetNumVertexPositions();
		
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

	C_ShapeNode* C_Converter::GetShapeNode(const Aftk::C_SceneNode* pSceneNode)
	{
		C_ShapeIndex* pShapeIndex;
		
		for(S32 i=0; i<m_ShapeIndex.m_numElements; ++i)
		{
			pShapeIndex=m_ShapeIndex[i];
			if(pShapeIndex->m_pSceneNode == pSceneNode )
			{
				return pShapeIndex->m_ShapeNode;
			}
		}

		return AddToConstraintShape(pSceneNode);
	}

	U32 C_Converter::GetConstraintType(const Aftk::C_DynamicPlug* pPlug)
	{
		std::string plugValue;
		pPlug->GetValue(plugValue);
		if(plugValue == "BallAndSocket")
		{
			return 0;
		}
		else if(plugValue == "Hinge")
		{
			return 1;
		}
		else if(plugValue == "Sliding")
		{
			return 2;
		}
		else if(plugValue == "Nail")
		{
			return 3;
		}
		else
		{
			assert("Constraint type isn't supported!");
			return -1;
		}
		/*case "Hinge":			return 1;
		case "Sliding":			return 2;
		case "Nail":			return 3;
		default:				assert("Constraint type isn't supported!");
		};*/
	}

	U32 C_Converter::GetConstraintBodyStatus(const Aftk::C_DynamicPlug* pPlug)
	{
		std::string plugValue;
		pPlug->GetValue(plugValue);

		if(plugValue == "Active")
			return 0;
		else if(plugValue == "Kinematic")
			return 1;
		else if(plugValue == "Sleep")
			return 2;
		else if(plugValue == "Passive")
			return 3;
		else
			assert("Constraint body status isn't supported!");
			return -1;

		/*switch(plugValue)
		{
		case "Active":			return 0;
		case "Kinematic":		return 1;
		case "Sleep":			return 2;
		case "Passive":			return 3;
		default:				assert("Constraint body status isn't supported!");
		};*/
	}

	Gravity::C_ConstraintNode* C_Converter::ConstructConstraintNode(const Aftk::C_SceneNode* pSceneNode)
	{
		Aftk::C_SceneConstraintNode* pConstraint = (Aftk::C_SceneConstraintNode*)pSceneNode;

		Aftk::C_SceneNode* pBodyA = 0;
		{
			Aftk::I_Plug* pPlug = pConstraint->GetPlugPtr("inWorldMatrix1");
			Aftk::I_Object* pSrcObj = pPlug->GetSourceConnectionPtr();
			if (pSrcObj)
			{
				BIZ_ASSERT(pSrcObj->HasType(Aftk::E_ObjType_IPlug));
				Aftk::I_Object* pBodyObj = ((Aftk::I_Plug*)(pSrcObj))->GetTopLevelParentPtr();
				BIZ_ASSERT(pBodyObj->HasType(Aftk::E_ObjType_SceneNode));
				pBodyA = (Aftk::C_SceneNode*)pBodyObj;
			}
		}
		Aftk::C_SceneNode* pBodyB = 0;
		{
			Aftk::I_Plug* pPlug = pConstraint->GetPlugPtr("inWorldMatrix2");
			Aftk::I_Object* pSrcObj = pPlug->GetSourceConnectionPtr();
			if (pSrcObj)
			{
				BIZ_ASSERT(pSrcObj->HasType(Aftk::E_ObjType_IPlug));
				Aftk::I_Object* pBodyObj = ((Aftk::I_Plug*)(pSrcObj))->GetTopLevelParentPtr();
				BIZ_ASSERT(pBodyObj->HasType(Aftk::E_ObjType_SceneNode));
				pBodyB = (Aftk::C_SceneNode*)pBodyObj;
			}
		}

		if (pBodyA && pBodyB)
		{
			//WIN

			Gravity::C_ConstraintNode* pConstraintNode=new Gravity::C_ConstraintNode;
			ConvertA3DMatrixToTransformationMatrix(pSceneNode->GetWorldMatrix(), pConstraintNode->m_Transform);
			if(m_RotateY180)
			{
				pConstraintNode->m_Transform = FlipAftkMatrix(pConstraintNode->m_Transform);
			}

			pConstraintNode->m_pPhysicsBodyA=GetShapeNode(pBodyA);
			pConstraintNode->m_pPhysicsBodyB=GetShapeNode(pBodyB);


			Aftk::C_DynamicPlug* pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("constraintType");
			pConstraintNode->m_ConstraintType = GetConstraintType(pPlug);

			pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("maskProxy");
			bool maskProxy = false;
			pPlug->GetValue(maskProxy);

				

			if(maskProxy)
			{
				pConstraintNode->m_MaskProxy = true;
			}
			else
			{

				pConstraintNode->m_MaskProxy = false;
			

				pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("statePhysicsBodyA");
				pConstraintNode->m_PhysicsBodyAStatus = GetConstraintBodyStatus(pPlug);

				pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("statePhysicsBodyB");
				pConstraintNode->m_PhysicsBodyBStatus = GetConstraintBodyStatus(pPlug);

				pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("colMask");
				bool disableCollisions = true;
				pPlug->GetValue(disableCollisions);
				pConstraintNode->m_MaskCollisions = disableCollisions;

				pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("useLimits");
				bool usedLimits = false;
				pPlug->GetValue(usedLimits);

				pPlug = (Aftk::C_DynamicPlug*) pConstraint->GetPlugPtr("axis");
				std::string axis;
				pPlug->GetValue(axis);
				if(axis == "x")
					pConstraintNode->m_Axis = 0;
				else if(axis == "y")
					pConstraintNode->m_Axis = 1;
				else
					pConstraintNode->m_Axis = 2;

				
				if(usedLimits )
				{
					F64 minSlide, maxSlide;
					F64 minRotateX, maxRotateX;
					F64 minRotateY, maxRotateY;
					F64 minRotateZ, maxRotateZ;

					pConstraintNode->m_UsedLimit = true;

					pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("minSlide");
					pPlug->GetValue(minSlide);

					pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("maxSlide");
					pPlug->GetValue(maxSlide);

					pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("minRotateX");
					pPlug->GetValue(minRotateX);

					pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("maxRotateX");
					pPlug->GetValue(maxRotateX);

					pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("minRotateY");
					pPlug->GetValue(minRotateY);

					pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("maxRotateY");
					pPlug->GetValue(maxRotateY);

					pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("minRotateZ");
					pPlug->GetValue(minRotateZ);

					pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("maxRotateZ");
					pPlug->GetValue(maxRotateZ);

					if(m_RotateY180)
					{
						pConstraintNode->m_MinSlide = -maxSlide;
						pConstraintNode->m_MaxSlide = -minSlide;
						pConstraintNode->m_MinRotateX = -maxRotateX;
						pConstraintNode->m_MaxRotateX = -minRotateX;
						pConstraintNode->m_MinRotateY = -maxRotateY;
						pConstraintNode->m_MaxRotateY = -minRotateY;
						pConstraintNode->m_MinRotateZ = -maxRotateZ;
						pConstraintNode->m_MaxRotateZ = -minRotateZ;
					}
					else
					{
						pConstraintNode->m_MinSlide = minSlide;
						pConstraintNode->m_MaxSlide = maxSlide;
						pConstraintNode->m_MinRotateX = minRotateX;
						pConstraintNode->m_MaxRotateX = maxRotateX;
						pConstraintNode->m_MinRotateY = minRotateY;
						pConstraintNode->m_MaxRotateY = maxRotateY;
						pConstraintNode->m_MinRotateZ = minRotateZ;
						pConstraintNode->m_MaxRotateZ = maxRotateZ;

					}
				}

				ConvertA3DMatrixToTransformationMatrix(pBodyA->GetWorldMatrix(), pConstraintNode->m_BodyAWTransform);
				ConvertA3DMatrixToTransformationMatrix(pBodyB->GetWorldMatrix(), pConstraintNode->m_BodyBWTransform);
				

				//pConstraintNode->m_BodyAWTransform = AftkHelpers::GetHMat43( pBodyA );
				//pConstraintNode->m_BodyBWTransform = AftkHelpers::GetHMat43( pBodyB );
				
				
				/*Aftk::Matrix44 mat;
				pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("inWorldMatrix1");
				pPlug->GetValue(mat);
				ConvertA3DMatrixToTransformationMatrix(mat, pConstraintNode->m_BodyAWTransform);*/
				if(m_RotateY180)
					pConstraintNode->m_BodyAWTransform = FlipAftkMatrix(pConstraintNode->m_BodyAWTransform);

				/*pPlug = (Aftk::C_DynamicPlug*)pConstraint->GetPlugPtr("inWorldMatrix2");
				pPlug->GetValue(mat);
				ConvertA3DMatrixToTransformationMatrix(mat, pConstraintNode->m_BodyBWTransform);		*/
				if(m_RotateY180)
					pConstraintNode->m_BodyBWTransform = FlipAftkMatrix(pConstraintNode->m_BodyBWTransform);
				
			}

			return pConstraintNode;
		}
		return NULL;		

	}


	S32 C_Converter::AddToConstraint(const Aftk::C_SceneNode* pSceneNode)
	{
		Gravity::C_ConstraintNode* pConstraintNode=ConstructConstraintNode(pSceneNode);
		if(pConstraintNode)
		{
			Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
			pConstraintNode->m_UniqueID=counter->GetConstraintId();
			m_Exporter.AddConstraintNode(pConstraintNode);

			return pConstraintNode->m_UniqueID;
		}

		return -1;
	}


	S32 C_Converter::AddToCloth(const Aftk::C_SceneNode* pSceneNode)
	{
		Gravity::C_ClothNode* pClothNode=ConstructClothNode(pSceneNode);
		pClothNode->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Active;
		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
		pClothNode->m_UniqueID = counter->GetClothId();
		m_Exporter.AddClothNode(pClothNode);
		return pClothNode->m_UniqueID;

	}

	//S32 C_Converter::AddToFluidMesh(const Aftk::C_SceneNode* pSceneNode)
	//{
	//	Gravity::C_FluidMeshNode* pFluidMeshNode=ConstructFluidMeshNode(pSceneNode);
	//	pFluidMeshNode->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Active;
	//	Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
	//	pFluidMeshNode->m_UniqueID = counter->GetFluidMeshId();
	//	m_Exporter.AddFluidMeshNode(pFluidMeshNode);
	//	return pFluidMeshNode->m_UniqueID;

	//}

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
		if(pShapeNode == NULL)//pSceneNode is a convex object and the vertices inside is 0
			return -1;
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

	
	S32 C_Converter::AddToStaticWater(const Aftk::C_SceneNode *pSceneNode)
	{
		Aftk::C_SceneMeshNode *pWater = (Aftk::C_SceneMeshNode*)pSceneNode;
		Gravity::C_FluidMeshNode* pFluidMeshNode = new Gravity::C_FluidMeshNode;
		pFluidMeshNode->m_pMesh=ConstructFluidMeshNode(pWater);
		if(pFluidMeshNode->m_pMesh == NULL)
			return -1;
		pFluidMeshNode->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Static;
		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
		pFluidMeshNode->m_UniqueID = counter->GetFluidMeshId();
		m_Exporter.AddStaticFluidMeshNode(pFluidMeshNode);
		return pFluidMeshNode->m_UniqueID;
	}

	void C_Converter::AddToStaticSplineMesh(const Aftk::C_SceneNode* pSceneNode)
	{
		Aftk::C_SceneSplineNode *pMesh = (Aftk::C_SceneSplineNode*)pSceneNode;
		ConstructStaticSplineMeshNode(pMesh);
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

	S32 C_Converter::CreateStaticSplineMeshNode()
	{
		Gravity::C_SplineMeshNode* pMeshNode=new Gravity::C_SplineMeshNode;
		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();
		pMeshNode->m_UniqueID=counter->GetStaticObjectId();
		m_pSplineMeshNode = pMeshNode;
		m_Exporter.AddSplineMeshNode(pMeshNode);
		return pMeshNode->m_UniqueID;
	}


	S32 C_Converter::AddShapes(const Aftk::C_SceneNode* pSceneNode)
	{
		if(pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
		{
			if( GetCollisionType( pSceneNode ) != E_CollisionType_None )
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
		const Aftk::C_SceneNode *pBaseInstance = AftkUt::Query::GetTopLevelSourceNodePtr(pSceneNode);


		Gravity::C_ExportCounter* counter=Gravity::C_ExportCounter::Instance();

		if(pBaseInstance->HasType(Aftk::E_ObjType_SceneMesh))
		{

			S32 a3dId=pBaseInstance->GetUniqueId();

			if(!m_Instances.Find(&a3dId))
			{

				Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pBaseInstance;
				Gravity::C_MeshNode* pMeshNode=ConstructMeshNode(pMesh);
				if(pMeshNode == NULL)
				{
					//no triangles or no collision flags found in all triangles in the base instance
					return -1;
				}

				assert(pMeshNode->m_Tris.m_numElements!=0);

				U32 id=counter->GetInstanceId();
				Gravity::C_InstanceBaseNode* pConverterBase=new Gravity::C_InstanceBaseNode; 
				Gravity::C_InstanceNode* pConverterInstance=new Gravity::C_InstanceNode;

				Maths::HMat43 baseInstanceMat;
				ConvertA3DMatrixToTransformationMatrix(pBaseInstance->GetWorldMatrix(), baseInstanceMat);
				
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
				

				pConverterBase->m_pMesh=pMeshNode;
				pConverterBase->m_UniqueID=counter->GetBaseInstanceId();
				pConverterBase->m_A3dId=a3dId;
				pConverterBase->m_Transform = baseInstanceMat;
				pConverterBase->m_InstanceNodes.AddToEnd(&pConverterInstance);

				m_Exporter.AddInstanceBaseNode(pConverterBase);

				Maths::HMat43 instanceMat;
				ConvertA3DMatrixToTransformationMatrix(pSceneNode->GetWorldMatrix(), instanceMat);
				const Maths::Mat43 relativeMat = baseInstanceMat.InverseRotTrans() * instanceMat;
				pConverterInstance->m_Transform = relativeMat;

				pConverterInstance->m_UniqueID=id;
				pConverterInstance->m_BaseNodeID=pConverterBase->m_UniqueID;

				m_Instances.AddToEnd(&a3dId);
				return id;
				
			}
			else
			{
				U32 id=counter->GetInstanceId();
				Gravity::C_InstanceBaseNode* pConverterBase=m_Exporter.GetInstanceBaseNode(a3dId);
				Gravity::C_InstanceNode* pConverterInstance=new Gravity::C_InstanceNode;
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
				
				Maths::HMat43 instanceMat;
				ConvertA3DMatrixToTransformationMatrix(pSceneNode->GetWorldMatrix(), instanceMat);
				const Maths::Mat43 relativeMat = pConverterBase->m_Transform.InverseRotTrans() * instanceMat;
				pConverterInstance->m_Transform = relativeMat;
				//pConverterInstance->m_Transform=GetRelativeTransformationMatrix(pBaseInstance, pInstance);
				pConverterBase->m_InstanceNodes.AddToEnd(&pConverterInstance);
				return id;

			}
			
		}
		return -1;
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

				const Aftk::C_SceneNode* pBaseInst = AftkUt::Query::GetTopLevelSourceNodePtr(pSceneNode);
				if (!pBaseInst)
					continue;

			
				if( GetCollisionType(pBaseInst) != E_CollisionType_None )
				{
					U32 id=AddToShape(pSceneNode);
					continue;
				}

				U32 id=AddToInstance(pSceneNode);
				continue;
			}


			if(m_pNodeInform->IsWater(pSceneNode))
			{
				AddToStaticWater(pSceneNode);
				continue;
			}

			if(pSceneNode->HasType(Aftk::E_ObjType_SceneSpline))
			{
				AddToStaticSplineMesh(pSceneNode);
				continue;
			}
		
			if(pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
			{
				if(GetCollisionType( pSceneNode ) != E_CollisionType_None )
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

		//U32 subDynamicIndex = 0;

		const U32 size = aftkSceneNodeList.Size();

		for(U32 i=0; i<size; ++i)
		{
			const Aftk::C_SceneNode* pChild = aftkSceneNodeList[i];

			if(pChild->GetType() == Aftk::E_ObjType_SceneInstance)
			{
				Aftk::C_SceneInstanceNode *pInstance = (Aftk::C_SceneInstanceNode*)pChild;
				Aftk::C_SceneNode *pBaseInstance = pInstance->GetSourceNodePtr();
				Aftk::C_SceneMeshNode* pMesh=(Aftk::C_SceneMeshNode* )pBaseInstance;
				Gravity::C_MeshNode* pMeshNode=ConstructMeshNode(pMesh, pInstance->GetWorldMatrix());
				if(pMeshNode == NULL)// the base mesh is empty
					break;
				//pMeshNode->m_CompoundID = counter->GetSubDynamicObjectId(subDynamicIndex++);
	
				pCompoundNode->AddNode(pMeshNode);
				
				continue;
			}

			if(pChild->HasType(Aftk::E_ObjType_SceneMesh))
			{
				bool bMesh = true;

				for (U32 i = 0; i < pChild->GetNumDynamicPlugs(); i++)
				{
					const Aftk::I_Plug& dynPlug = *pChild->GetDynamicPlugPtr(i);
					if (dynPlug.GetName() == "collisionPrimitive")
					{
						bMesh=false;
						Gravity::C_ShapeNode* pShapeNode=ConstructShapeNode(pChild);
						if(pShapeNode == NULL)//if the pChild is a convex hull and the number of vertices in the convex hull is zero
							break;

						
						//pShapeNode->m_CompoundID=counter->GetSubDynamicObjectId(subDynamicIndex++);
						pShapeNode->m_UniqueID = counter->GetSubDynamicObjectId();

						Gravity::C_ShapeIndex* pShapeIndex=new Gravity::C_ShapeIndex;
						pShapeIndex->m_ShapeNode=pShapeNode;
						pShapeIndex->m_pSceneNode = const_cast<Aftk::C_SceneNode*>(pChild);
						m_ShapeIndex.AddToEnd(&pShapeIndex);

						Maths::HMat43 transform(Maths::UNDEFINED);

						ConvertA3DMatrixToTransformationMatrix(pChild->GetWorldMatrix(), transform);

						Maths::HMat33 relativeRotation= transform.Rotation() * worldMatrix.Rotation().InverseRotation();

						Maths::HVec3 translation = transform.Translation() - worldMatrix.Translation();

						pShapeNode->m_Transform = Maths::HMat43(relativeRotation, translation);
						
						pCompoundNode->AddNode(pShapeNode);
					}
				}
				if(bMesh)
				{
	
					Aftk::C_SceneMeshNode* pMesh=(Aftk::C_SceneMeshNode*) pChild;
					Aftk::Matrix44 childWorld=pChild->GetWorldMatrix();
				
					Gravity::C_MeshNode* pMeshNode=ConstructMeshNode(pMesh, childWorld);
					if(pMeshNode == NULL)// the mesh is empty
						break;
					
					//pMeshNode->m_CompoundID = counter->GetSubDynamicObjectId(subDynamicIndex++);
			
					pCompoundNode->AddNode(pMeshNode);

				}
			}
			
		}
		
		if(pCompoundNode->m_Nodes.m_numElements == 0)
		{
			delete pCompoundNode;
			return NULL;
		}
		return pCompoundNode;
		
	}
	

	 bool C_Converter::Export( Sys::I_BinaryStream* pFile )
	 {
		return m_Exporter.Export(pFile);
	 }
	 
}