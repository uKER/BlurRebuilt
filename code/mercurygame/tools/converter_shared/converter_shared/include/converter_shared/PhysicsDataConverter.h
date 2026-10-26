#ifndef PHYSICSDATACONVERTER_H
#define PHYSICSDATACONVERTER_H

#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sdk/ConstraintDescriptors.h"
#include "gravity/exporter/GravityExporter.h"
#include "gravity/sharedphysics/SortedArray.h"
#include <gravity\treebuilder\AABBTree.h>
#include <gravity\narrowphase\Shape.h>

#include "AftkUt/AftkUt.h"
#include "btl/containers/vector.h"
#include "gravity/exporter/Exporter.h"



namespace Gravity
{

	class C_ExportCounter
	{
	private:
		C_ExportCounter(): m_StaticCount(0), m_DynamicCount(0), m_ConstraintCount(0), 
			m_InstanceCount(0), m_BaseInstanceCount(0), m_ClothCount(0), m_SubDynamicCount(0),
			m_FluidMeshCount(0)
		{
		}

		~C_ExportCounter()
		{
		}


		U32 m_StaticCount; //the total number of static objects
		U32 m_DynamicCount;//the total number of dynamic objects
		U32 m_ConstraintCount;//the total number of constraints
		U32 m_InstanceCount;//the total number of instance meshes
		U32 m_BaseInstanceCount;//the total number of base instance meshes
		U32 m_ClothCount;//the total number of cloth objects 
		U32 m_SubDynamicCount;//the total number of dynamic object's components
		U32 m_FluidMeshCount; //the total number of static fluid meshes

	public:

		
		//0x110....index, 6 dec = 110 hex and left shift 110 29 bits
		U32 GetDynamicObjectId()
		{
			return ((6 << 29)|(m_DynamicCount++));
		}

		////0x111....index
		//U32 GetSubDynamicObjectId(U32 index)
		//{
		//	return ((7 <<29)|index);
		//}
		
		//0x111....index
		U32 GetSubDynamicObjectId()
		{
			return ((7 << 29 )|(m_SubDynamicCount++));
		}

		//0x000....index
		U32 GetStaticObjectId()
		{
			return ((~(7<<29))& (m_StaticCount++));
		}

		//0x010....index
		U32 GetConstraintId()
		{
			return ((2<<29)|(m_ConstraintCount++));
		}

		//0x011....index
		U32 GetClothId()
		{
			return ((3<<29)|(m_ClothCount++));
		}

		//0x101.....index
		U32 GetInstanceId()
		{
			return ((5 << 29)|(m_InstanceCount++));
		}

		//0x100......index
		U32 GetBaseInstanceId()
		{
			return ((4<<29)|(m_BaseInstanceCount++));
		}

		//0x00001......index
		U32 GetFluidMeshId()
		{
			return ((8<<24)|(m_FluidMeshCount++));
		}

		static C_ExportCounter* Instance()
		{
			static C_ExportCounter m_Counter;
			return &m_Counter;
		}
		
	};

	class C_PolyInform
	{
	public:
		virtual U32 GetUserData(const Aftk::C_Polygon& poly) = 0;
		virtual U32 GetPhysicsMaterial(const Aftk::C_Polygon& poly) = 0;
		virtual U32 GetEffectMaterial(const Aftk::C_Polygon& poly) = 0;
		virtual U32 GetCollisionFlags(const Aftk::C_Polygon& poly) = 0;
	};

	class C_NodeInform
	{
	public:		
		virtual bool IsWater(const Aftk::C_SceneNode *pSceneNode) const = 0;
		virtual U32 GetEffectMaterial(const Aftk::C_SceneNode *pSceneNode) const = 0;
	};

	class C_DefaultPolyInform : public C_PolyInform
	{
		//virtual U32 GetCollisionFlags(const Aftk::C_Polygon& poly);
		//virtual U32 GetMaterial(const Aftk::C_Polygon& poly);
		virtual U32 GetUserData(const Aftk::C_Polygon& poly)
		{
			return 1;
		}

		virtual U32 GetPhysicsMaterial(const Aftk::C_Polygon& poly)
		{
			const Aftk::C_SceneMeshNode* pSceneMeshNode = poly.GetMeshNodePtr();
			const Aftk::C_Scene* pAftk = pSceneMeshNode->GetScenePtr();
			S16 blindDataMappingIndex = poly.GetBlindDataMappingIndex();

			const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);
			U32 materialMask = 0;
			if(pBlindDataResource != NULL)
			{
				const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();

				U32 numAttributes = aftkBlindDataNode.GetNumAttributes();
				for (U32 i = 0; i < numAttributes; i++) 
				{
					const Aftk::C_GenericAttribute&   aftkAttribute = aftkBlindDataNode.GetAttribute( i ); 

					const char* attrName = aftkAttribute.GetName().c_str();

					//Boolean value
					if (!_stricmp(attrName, "polySetDropDown_materialType"))
					{
						//Get the material index
						AFTK_ASSERT(aftkAttribute.GetDataType() == Aftk::E_GenericAttributeType_U32);
						aftkAttribute.GetValue(materialMask);
						return materialMask;
					}
				}
			}
			return materialMask;
		}

		virtual U32 GetEffectMaterial(const Aftk::C_Polygon& poly)
		{
			const Aftk::C_SceneMeshNode* pSceneMeshNode = poly.GetMeshNodePtr();
			const Aftk::C_Scene* pAftk = pSceneMeshNode->GetScenePtr();
			S16 blindDataMappingIndex = poly.GetBlindDataMappingIndex();

			const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);
			U32 materialMask = 0;
			if(pBlindDataResource != NULL)
			{
				const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();

				U32 numAttributes = aftkBlindDataNode.GetNumAttributes();
				for (U32 i = 0; i < numAttributes; i++) 
				{
					const Aftk::C_GenericAttribute&   aftkAttribute = aftkBlindDataNode.GetAttribute( i ); 

					const char* attrName = aftkAttribute.GetName().c_str();

					//Boolean value
					if (!_stricmp(attrName, "polySetDropDown_materialType"))
					{
						//Get the material index
						AFTK_ASSERT(aftkAttribute.GetDataType() == Aftk::E_GenericAttributeType_U32);
						aftkAttribute.GetValue(materialMask);
						return materialMask;
					}
				}
			}
			return materialMask;
		}

		virtual U32 GetCollisionFlags(const Aftk::C_Polygon& poly)
		{
			const Aftk::C_SceneMeshNode* pSceneMeshNode = poly.GetMeshNodePtr();
			const Aftk::C_Scene* pAftk = pSceneMeshNode->GetScenePtr();
			S16 blindDataMappingIndex = poly.GetBlindDataMappingIndex();

			const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);
			U32 collisionFlag = 0;
			if(pBlindDataResource != NULL)
			{
				const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();

				U32 numAttributes = aftkBlindDataNode.GetNumAttributes();
				for (U32 i = 0; i < numAttributes; i++) 
				{
					const Aftk::C_GenericAttribute&   aftkAttribute = aftkBlindDataNode.GetAttribute( i ); 

					const char* attrName = aftkAttribute.GetName().c_str();

					//Boolean value
					if (!_stricmp(attrName, "polySet_CharacterSolid"))
					{
						collisionFlag |= 1;
					}
					else if (!_stricmp(attrName, "polySet_Shootable"))
					{
						collisionFlag |= 2;
					}
					else if (!_stricmp(attrName, "polySet_CameraSolid"))
					{
						collisionFlag |= 4;
					}
					else if (!_stricmp(attrName, "polySet_AILookSolid"))
					{
						collisionFlag |= 8;
					}
					else if (!_stricmp(attrName, "polySet_DynObjBlocker"))
					{
						collisionFlag |= 16;
					}
					else if (!_stricmp(attrName, "polySet_CapsuleSolid"))
					{
						collisionFlag |= 32;
					}
					else if (!_stricmp(attrName, "polySet_RagdollSolid"))
					{
						collisionFlag |= 64;
					}
					else if (!_stricmp(attrName, "polySet_AllSolid"))
					{
						collisionFlag |= 127;
					}
				}
			}
			return collisionFlag;
		}
	};
	
	class C_DefaultNodeInform : public C_NodeInform
	{
		bool IsWater(const Aftk::C_SceneNode *pSceneNode) const
		{
			return false;
		}

		U32 GetEffectMaterial(const Aftk::C_SceneNode *pSceneNode) const
		{
			const U16 EffectMaterial_Default = 0;

			return EffectMaterial_Default;		
		}

	};

	typedef Btl::Vector< const Aftk::C_SceneNode* > C_AftkSceneNodeList;


	class C_ShapeIndex
	{
	public:
		C_ShapeIndex(): m_pSceneNode(NULL), m_ShapeNode(NULL)
		{
		}

		~C_ShapeIndex(){}

		Aftk::C_SceneNode* m_pSceneNode;
		C_ShapeNode* m_ShapeNode;
	};

	class C_Converter
	{
	private:


		Maths::HMat43 GetRelativeTransformationMatrix(Aftk::C_SceneNode* pParent, Aftk::C_SceneNode* pChild);

		//Maths::HMat43 FlipZComponent(const Maths::HMat43 PCREF transform);
		
		void ConvertA3DMatrixToTransformationMatrix(const Aftk::Matrix44& a3dMatrix, Maths::HVec3& scale, Maths::HVec3& position, Maths::HMat33& rotation);
		void ConvertA3DMatrixToTransformationMatrix(const Aftk::Matrix44& a3dMatrix, Maths::HMat43& transform);
		void CreateConvexHull(const Aftk::C_SceneMeshNode* pMesh, Maths::HVec3 PCREF Scale, SharedPhysics::SortedArray<Maths::HVec3>& vertexList);
		

		/*
			Used to construct a static mesh
		*/
		Gravity::C_MeshNode* ConstructMeshNode(const Aftk::C_SceneMeshNode *pMesh);//construct a static mesh
		Gravity::C_MeshNode* ConstructFluidMeshNode(const Aftk::C_SceneMeshNode *pMesh);//construct a fluid mesh

		/*
			Used to construct a mesh which is part of a compound object
		*/
		Gravity::C_MeshNode* ConstructMeshNode(const Aftk::C_SceneMeshNode* pMesh, const Aftk::Matrix44 PCREF trans);
		Gravity::C_MeshNode* ConstructFluidMeshNode(const Aftk::C_SceneMeshNode* pMesh, const Aftk::Matrix44 PCREF trans);
		Gravity::C_ShapeNode* ConstructShapeNode(const Aftk::C_SceneNode* pSceneNode);
		Gravity::C_ConstraintNode* ConstructConstraintNode(const Aftk::C_SceneNode* pSceneNode);
		Gravity::C_CompoundNode* ConstructCompoundNode();
		Gravity::C_InstanceBaseNode* ConstructInstanceBaseNode();


		/*
			Used to create constraints
		*/
		U32 GetConstraintBodyStatus(const Aftk::C_DynamicPlug* pPlug);
		U32 GetConstraintType(const Aftk::C_DynamicPlug* pPlug);
		C_ShapeNode* GetShapeNode(const Aftk::C_SceneNode* pSceneNode);

		
		
		/*
			Just create one static mesh node for all the meshes
		*/
		S32 CreateStaticMeshNode();
		S32 CreateStaticSplineMeshNode();
		void ConstructStaticMeshNode(const Aftk::C_SceneMeshNode* pMesh);
		void ConstructStaticSplineMeshNode(const Aftk::C_SceneSplineNode* pSplineNode);
		void AddToStaticMesh(const Aftk::C_SceneNode* pSceneNode);
		void AddToStaticSplineMesh(const Aftk::C_SceneNode* pSceneNode);
		S32 AddToStaticWater(const Aftk::C_SceneNode* pSceneNode);


		
		C_ShapeNode* C_Converter::AddToConstraintShape(const Aftk::C_SceneNode* pSceneNode);
		S32 AddToShape(const Aftk::C_SceneNode* pSceneNode);
		S32 AddToMesh(const Aftk::C_SceneNode* pSceneNode);
		S32 AddShapes(const Aftk::C_SceneNode* pSceneNode);
		S32 AddToInstance(const Aftk::C_SceneNode* pSceneNode);
		Gravity::C_CompoundNode* BuildCompoundNode(const C_AftkSceneNodeList& aftkSceneNodeList, const Maths::HMat43 PCREF worldMatrix);		
		
		SharedPhysics::SortedArray<S32> m_Instances;
		SharedPhysics::SortedArray<C_ShapeIndex* > m_ShapeIndex;
		Gravity::C_ExportScene m_Exporter;
		C_PolyInform* m_pPolyInform;
		C_NodeInform* m_pNodeInform;
		/*
			Deleted by the C_ExportScene: C_ExportScene.m_StaticMeshNode == m_pMeshNode;
		*/
		Gravity::C_MeshNode* m_pMeshNode;
		Gravity::C_SplineMeshNode* m_pSplineMeshNode;
		U32 m_FluidCollisionFlag;
		bool m_Processed;
		bool m_RotateY180;

	public:

		// TODO: Remove this hack, and set m_Processed to true when it's been processed
		C_Converter(C_PolyInform* pPolyInform,  C_NodeInform* pNodeInform, F32 splitRatio = 0.2f, F32 triangleIncreaseThreshold = 0.7f, U32 maxDepth = MAX_DEPTH) : m_Processed(true), m_pPolyInform(pPolyInform), m_pNodeInform( pNodeInform ),
			m_FluidCollisionFlag(0), m_RotateY180(false)
		{
			m_Exporter.SetModelAABBTreeParameters(splitRatio, triangleIncreaseThreshold, maxDepth);
			CreateStaticMeshNode();
			CreateStaticSplineMeshNode();
		}


		~C_Converter()
		{
			for(S32 i=0; i<m_ShapeIndex.m_numElements; ++i)
			{
				if(m_ShapeIndex[i])
					delete m_ShapeIndex[i];
				m_ShapeIndex[i]=NULL;
			}
		}

		void SetMaxConvexHullTriangles(const U32 maxTriangles)
		{
			Gravity::C_Exporter::Instance()->SetMaxConvexHullTriangle(maxTriangles);
		}

		void SetFluidCollisionFlag(const U32 fluidCollisionFlag)
		{
			m_FluidCollisionFlag = fluidCollisionFlag;
		}

		int ConvertA3DToGravity(const char* fileName, const char* resultFile);
		/* 
			The nodes in this list may be mesh or physics shapes and may be aftk instance nodes in which case they should be instanced. 
			This function would generally be only be called once per C_Converter with the full list of static nodes to process.
			For the route functionality, we will have several C_Converters.
	   */
		void AddStaticNodes( const C_AftkSceneNodeList& aftkSceneNodeList );

		/*
			This will export a shape by gluing together all the aftk scene nodes provided which may be mesh or physics shapes. 
			Unless it is possible to export as a single shape.
			This will not actually create a physics body in the game, just the shape.
			If this list contains instance nodes, the instances can be expanded out.
			The matrix provided is a world space matrix which will be orthonormal. 
			This will be the world space body position when the object is created in game so the shape components will need to be transformed by the inverse of this.
		*/

       U32 AddDynamicShape( const C_AftkSceneNodeList& aftkSceneNodeList, const Maths::HMat43& originInWorldSpace );

	   // This must be called once before Export
       bool Process();

       // There needs to be a version that takes a binary stream rather than a filename
	   // Process must be called once before export
       bool Export( Sys::I_BinaryStream* pFile );


	   S32 AddToConstraint(const Aftk::C_SceneNode* pSceneNode);

	   void SetRotateY180( bool rotateY180 ) { m_RotateY180 = rotateY180; }
	};
}
#endif
