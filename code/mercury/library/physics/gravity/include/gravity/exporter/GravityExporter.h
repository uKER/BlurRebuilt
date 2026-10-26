#ifndef GRAVITYEXPORTER_H
#define GRAVITYEXPORTER_H

//#include "gravity/sharedphysics/sortedarray.h"
//#include "gravity/exporter/ExportPrimitivesList.h"
#include "gravity/exporter/GravityConverter.h"
//#include "gravity/sharedphysics/GraphNode.h"
//#include <system/serialise/BinarySerialiseAutoEndian.h>


namespace Gravity
{
	enum GravityExporterFlag
	{
		GE_Active_Flag,
		GE_Sleeping_Flag,
		GE_Static_Flag,
		GE_Kinematic_Flag
	};

	enum GravityExporterType
	{
		GE_SHAPE_TYPE,
		GE_COMPOUND_TYPE,
		GE_INSTANCE_TYPE,
		GE_CONSTRAINT_TYPE,
		GE_MESH_TYPE,
		GE_SPLINE_TYPE,
		GE_CLOTH_TYPE,
		GE_FLUIDMESH_TYPE,
		GE_DEFORMABLE_TYPE
	};

	class C_Node
	{
	public:
		C_Node(): m_NodeFlag(SharedPhysics::C_GraphNode::E_Type_Static),m_Type(GE_SHAPE_TYPE), 
			m_PhysicsMaterialIndex(0), m_EffectMaterialIndex(0), m_CollisionFlag(0xffffffff),
			m_UserDataFlag(0),m_Mass(1.f), 
			m_UniqueID(-1)
		{
		}

		C_Node(GravityExporterType type): m_Type(type), m_NodeFlag(SharedPhysics::C_GraphNode::E_Type_Static),
			m_PhysicsMaterialIndex(0), m_EffectMaterialIndex(0), m_CollisionFlag(0), 
			m_UserDataFlag(0), m_Mass(1.f),
			m_UniqueID(-1)
		{
		}

		virtual ~C_Node()
		{

		}

		SharedPhysics::C_GraphNode::E_NodeFlags m_NodeFlag;
		GravityExporterType m_Type;
		U16 m_PhysicsMaterialIndex;
		U16 m_EffectMaterialIndex;
		U32 m_CollisionFlag;
		U32 m_UserDataFlag;
		F32 m_Mass;
		S32 m_UniqueID;
		//S32 m_CompoundID;
		Maths::HMat43 m_Transform;
	};

	class C_ShapeNode : public C_Node
	{
	public:
		enum E_ShapeType
		{
			E_ShapeType_Sphere = 0,
			E_ShapeType_ConvexHull,
			E_ShapeType_Box,
			E_ShapeType_Cylinder,
			E_ShapeType_Capsule,
			E_ShapeType_Cone,

			E_ShapeType_Unknown,
		};

		C_ShapeNode(): C_Node(GE_SHAPE_TYPE)
		{
		}

		virtual ~C_ShapeNode()
		{
		}

		/*
			The shape node include in a compound object, the rotation and position should be the local matrix rotation and position;
			If the shape node is individual shape node, the rotation and position should be the world matrix rotation and position
		*/
		/*Maths::HMat33 m_Rotation;
		Maths::HVec3 m_Position;*/
		char* m_NodeName;
		Maths::HVec3 m_Scale;
		E_ShapeType m_ShapeType;
		SharedPhysics::SortedArray<Maths::HVec3> m_VertexList; // for the convex hull
	
	};

	class C_ConverterTriangle
	{
	public:
		C_ConverterTriangle()
		{
		}

		~C_ConverterTriangle()
		{
		}

		U16 m_PhysicsMaterialIndex;
		U16 m_EffectMaterialIndex;
		U32 m_CollisionFlag;
		U32 m_UserDataFlag;
		U32 m_IndexA;
		U32 m_IndexB;
		U32 m_IndexC;
		Maths::HVec3 vertexA;
		Maths::HVec3 vertexB;
		Maths::HVec3 vertexC;
		Maths::RGBA m_ColourA, m_ColourB, m_ColourC;
	};


	class C_MeshNode : public C_Node
	{
	public:
		C_MeshNode(): C_Node(GE_MESH_TYPE)
		{
		}

		virtual ~C_MeshNode()
		{
			for(S32 i=0; i<m_Tris.m_numElements; ++i)
			{
				if(m_Tris[i])
					delete m_Tris[i];
				m_Tris[i]=NULL;
			}
		}
		U32 m_UniqueID;
		SharedPhysics::SortedArray<C_ConverterTriangle*> m_Tris;
	};

	class C_SplineMeshNode : public C_Node
	{
	public:
		C_SplineMeshNode(): C_Node(GE_SPLINE_TYPE)
		{
		}

		virtual ~C_SplineMeshNode()
		{
			for(S32 i=0; i<m_Tris.m_numElements; ++i)
			{
				if(m_Tris[i])
					delete m_Tris[i];
				m_Tris[i]=NULL;
			}
		}
		U32 m_UniqueID;
		SharedPhysics::SortedArray<C_ConverterTriangle*> m_Tris;
	};

	class C_ClothNode: public C_Node
	{
	public:
		C_ClothNode() : C_Node(GE_CLOTH_TYPE)
		{
		}

		virtual ~C_ClothNode()
		{
			/*for(S32 i=0; i<m_Edge.m_numElements; ++i)
			{
				if(m_Edge[i])
					delete m_Edge[i];
				m_Edge[i]=NULL;
			}*/

			for(S32 i=0; i<m_Tris.m_numElements; ++i)
			{
				if(m_Tris[i])
					delete m_Tris[i];
				m_Tris[i]=NULL;
			}
		}

		U32 m_UniqueID;
		//SharedPhysics::SortedArray<C_ConverterEdge*> m_Edge;
		SharedPhysics::SortedArray<C_ConverterTriangle*> m_Tris;
	};

	class C_DeformableNode: public C_Node
	{
	public:
		C_DeformableNode() : C_Node(GE_DEFORMABLE_TYPE)
		{
		}

		virtual ~C_DeformableNode()
		{

			for(S32 i=0; i<m_Tris.m_numElements; ++i)
			{
				if(m_Tris[i])
					delete m_Tris[i];
				m_Tris[i]=NULL;
			}
		}

		U32 m_UniqueID;
		SharedPhysics::SortedArray<C_ConverterTriangle*> m_Tris;
	};

	class C_FluidMeshNode: public C_Node
	{
	public:
		C_FluidMeshNode() : C_Node(GE_FLUIDMESH_TYPE), m_Depth(100.f)
		{
		}

		virtual ~C_FluidMeshNode()
		{

			/*for(S32 i=0; i<m_Tris.m_numElements; ++i)
			{
				if(m_Tris[i])
					delete m_Tris[i];
				m_Tris[i]=NULL;
			}*/
			if(m_pMesh)
				delete m_pMesh;
			m_pMesh = NULL;
		}

		U32 m_UniqueID;
		F32 m_Depth;

		C_MeshNode* m_pMesh;
		//SharedPhysics::SortedArray<C_ConverterTriangle*> m_Tris;
	};

	class C_CompoundNode : public C_Node
	{
	public:
		C_CompoundNode(): C_Node(GE_COMPOUND_TYPE)
		{
		}

		virtual ~C_CompoundNode()
		{
			for(S32 i=0; i<m_Nodes.m_numElements; ++i)
			{
				if(m_Nodes[i])
					delete m_Nodes[i];
				m_Nodes[i]=NULL;
			}

		}

		void AddNode(C_Node* node)
		{
			m_Nodes.AddToEnd(&node);
		}
		SharedPhysics::SortedArray<C_Node*> m_Nodes;
		//U32 m_ShapeBodyId;
		//Aftk::Matrix44 a3dMatrix;
		//Maths::HMat43 m_Transform;
	};

	class C_ConstraintNode : public C_Node
	{
	
	public:
		C_ConstraintNode() : C_Node(GE_CONSTRAINT_TYPE), m_pPhysicsBodyA(NULL), m_pPhysicsBodyB(NULL), m_UsedLimit(false), m_MaskCollisions(false)
		{
		}

		virtual ~C_ConstraintNode()
		{
		}

		
		Maths::HMat43 m_BodyAWTransform;
		Maths::HMat43 m_BodyBWTransform;
		Gravity::C_ShapeNode* m_pPhysicsBodyA;
		Gravity::C_ShapeNode* m_pPhysicsBodyB;
		U32 m_pParentPhysicsBodyAIndex;
		U32 m_pParentPhysicsBodyBIndex;
		bool m_UsedLimit;
		bool m_MaskCollisions; //mask collision between neighbour body
		bool m_MaskProxy; // mask collision between different bodies
		U32 m_ConstraintType;
		U32 m_PhysicsBodyAStatus;
		U32 m_PhysicsBodyBStatus;
		U32 m_Axis; // rotation axis
		F64 m_MinSlide;
		F64 m_MaxSlide;
		F64 m_MinRotateX;
		F64 m_MaxRotateX;
		F64 m_MinRotateY;
		F64 m_MaxRotateY;
		F64 m_MinRotateZ;
		F64 m_MaxRotateZ;

	};

	class C_InstanceNode : public C_Node
	{
	public:
	
		C_InstanceNode(): C_Node(GE_INSTANCE_TYPE)
		{
		}

		virtual ~C_InstanceNode()
		{
		}
		//Maths::HMat43 m_Transform;
		S32 m_BaseNodeID;
	};

	class C_InstanceBaseNode : public C_Node
	{
	public:

		C_InstanceBaseNode(): C_Node(GE_INSTANCE_TYPE), m_pMesh(NULL)
		{

		}

		virtual ~C_InstanceBaseNode()
		{
			if(m_pMesh)
				delete m_pMesh;
			m_pMesh=NULL;

			for(S32 i=0; i<m_InstanceNodes.m_numElements; ++i)
			{
				if(m_InstanceNodes[i])
					delete m_InstanceNodes[i];
				m_InstanceNodes[i]=NULL;
			}
		}

		//Aftk::C_SceneNode* m_BaseNode;
		SharedPhysics::SortedArray<C_InstanceNode*> m_InstanceNodes;
		C_MeshNode* m_pMesh;
		//U32 m_Id;
		U32 m_A3dId;
		//SharedPhyscis::SortedArray<C_ConverterTriangle* > m_Tris;
	};

	

	class C_ExportScene
	{
	public:
		
		C_ExportScene() :  m_pSplineMeshNode(NULL)
		{
		}

		~C_ExportScene();

		void AddShapeNode(C_ShapeNode* pShapeNode)
		{
			m_ShapeNodes.AddToEnd(&pShapeNode);
		}

		void AddCompoundNode(C_CompoundNode* pCompoundNode)
		{
			m_CompoundNodes.AddToEnd(&pCompoundNode);
		}

		void AddInstanceBaseNode(C_InstanceBaseNode* pInstanceBaseNode)
		{
			m_InstanceBaseNodes.AddToEnd(&pInstanceBaseNode);
		}
		
		void AddConstraintNode(C_ConstraintNode* pConstraintNode)
		{
			m_ConstraintNodes.AddToEnd(&pConstraintNode);
		}

		/*void AddMeshNode(C_MeshNode* pMeshNode)
		{
			m_pStaticMeshNode = pMeshNode;
		}*/

		/*void AddShootableMeshNode(C_MeshNode* pMeshNode)
		{
			m_pStaticShootableMeshNode = pMeshNode;
		}

		void AddCollidableMeshNode(C_MeshNode* pMeshNode)
		{
			m_pStaticCollidableMeshNode = pMeshNode;
		}*/

		void AddStaticMeshNode(C_MeshNode* pMeshNode)
		{
			m_StaticMeshNodes.AddToEnd(&pMeshNode);

		}

		void AddSplineMeshNode(C_SplineMeshNode* pMeshNode)
		{
			m_pSplineMeshNode = pMeshNode;
		}

		void AddClothNode(C_ClothNode* pClothNode)
		{
			m_ClothNodes.AddToEnd(&pClothNode);
		}

		void AddDeformableNode(C_DeformableNode* pDeformableNode)
		{
			m_DeformableNodes.AddToEnd(&pDeformableNode);
		}

		void AddStaticFluidMeshNode(C_FluidMeshNode* pFluidMeshNode)
		{
			m_StaticFluidMeshNodes.AddToEnd(&pFluidMeshNode);
		}

		Gravity::C_MeshNode* GetMeshNode()
		{
			return new C_MeshNode;
		}

		Gravity::C_SplineMeshNode* GetSplineMeshNode()
		{
			return new C_SplineMeshNode;
		}

		Gravity::C_FluidMeshNode* GetFluidMeshNode()
		{
			return new C_FluidMeshNode;
		}

		Gravity::C_ClothNode* GetClothNode()
		{
			return new C_ClothNode;
		}

		Gravity::C_ShapeNode* GetShapeNode()
		{
			return new C_ShapeNode;
		}

		Gravity::C_CompoundNode* GetCompoundNode()
		{
			return new C_CompoundNode;
		}

		Gravity::C_ConstraintNode* GetConstraintNode()
		{
			return new C_ConstraintNode;
		}

		Gravity::C_InstanceNode* GetInstanceNode()
		{
			return new C_InstanceNode;
		}

		Gravity::C_InstanceBaseNode* GetInstanceBaseNode()
		{
			return new C_InstanceBaseNode;
		}


		bool Process()
		{
			return true;
		}

		void SetModelTreeSplitRatio(F32 splitRatio)
		{
			buildPrim.SetSplitRatio(splitRatio);
		}

		void SetModelTriangleIncreaseThreshold(F32 triangleIncreaseThreshold)
		{
			buildPrim.SetTriangleIncreaseThreshold(triangleIncreaseThreshold);
		}

		void SetModelTreeMaxDepth(U32 maxDepth)
		{
			buildPrim.SetMaxDepth(maxDepth);
		}

		void SetModelAABBTreeParameters(F32 splitRatio, F32 triangleIncreaseThreshold, U32 maxDepth)
		{
			buildPrim.SetModelAABBTreeParameters(splitRatio, triangleIncreaseThreshold, maxDepth);
		}

	
		C_InstanceBaseNode* GetInstanceBaseNode(U32 id);
	
		bool Export(Sys::I_BinaryStream* pFile);

		void Clear();
	

		/*C_MeshNode* m_pStaticShootableMeshNode;
		C_MeshNode* m_pStaticCollidableMeshNode;*/
		C_SplineMeshNode* m_pSplineMeshNode;
		SharedPhysics::SortedArray<C_MeshNode*> m_StaticMeshNodes;
		SharedPhysics::SortedArray<C_ShapeNode*> m_ShapeNodes;
		SharedPhysics::SortedArray<C_CompoundNode*> m_CompoundNodes;
		SharedPhysics::SortedArray<C_InstanceBaseNode*> m_InstanceBaseNodes;
		SharedPhysics::SortedArray<C_ConstraintNode*> m_ConstraintNodes;
		SharedPhysics::SortedArray<C_ClothNode*> m_ClothNodes;
		SharedPhysics::SortedArray<C_DeformableNode*> m_DeformableNodes;
		SharedPhysics::SortedArray<C_FluidMeshNode*> m_StaticFluidMeshNodes;
		Gravity::C_PhysicsExporter buildPrim;
		Gravity::C_ExportConstraint buildCons;

	};
}


#endif
