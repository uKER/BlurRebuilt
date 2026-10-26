#ifndef	EXPORTPRIMITIVESLIST_H
#define EXPORTPRIMITIVESLIST_H

#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/sdk/BodyHelper.h"
#include "gravity/sdk/ConstraintDescriptors.h"
#include "gravity/exporter/Exporter.h"
#include "gravity/sharedphysics/PhysicsVersion.h"
#include "gravity/treebuilder/buildclothtree.h"
#include "gravity/treebuilder/aabbtree.h"
#include "gravity/sdk/ShapeDescriptors.inl"

namespace Gravity
{

	class C_ConstraintNode;
		
	//template <class Deserialiser>
	class C_CompositeObjectWrapper
	{
	public:

		C_CompositeObjectWrapper() : m_relativeTransform(Maths::UNDEFINED), m_Translate(Maths::UNDEFINED), m_Shape(NULL)
		{
		}
		~C_CompositeObjectWrapper()
		{
			/*if(m_Shape)
				delete m_Shape;
			m_Shape = NULL;*/
		}
		
		Maths::HMat33 m_relativeTransform;
		Maths::HVec3 m_Translate;
		Gravity::C_ShapeDesc* m_Shape;

	};

	//template <class Deserialiser>
	class C_CompositeObject
	{
	public:
		C_CompositeObject(): m_Iter(&m_pDesc)
		{
		}

		~C_CompositeObject()
		{
		}

		void AddSphere(const Maths::HF32 PCREF radius, const Maths::HVec3 PCREF translate, const Maths::HMat33 PCREF relativeTransform)
		{
			C_CompositeObjectWrapper* wrapper=m_Iter.Create();
			Gravity::C_SphereDesc* sphere= new Gravity::C_SphereDesc;
			sphere->m_Radius=radius;
			wrapper->m_Shape=sphere;
			wrapper->m_Translate=translate;
			wrapper->m_relativeTransform=relativeTransform;
		}

		void AddOBB(const Maths::HVec3 PCREF halfExtents,  const Maths::HVec3 PCREF translate, const Maths::HMat33 PCREF relativeTransform)
		{
			C_CompositeObjectWrapper* wrapper=m_Iter.Create();
			Gravity::C_BoxDesc* box= new Gravity::C_BoxDesc;
			box->m_HalfExtents=halfExtents;
			wrapper->m_Shape=box;
			wrapper->m_Translate=translate;
			wrapper->m_relativeTransform=relativeTransform;

		}

		void AddCapsule(const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const Maths::HVec3 PCREF translate, const Maths::HMat33 PCREF relativeTransform)
		{
			C_CompositeObjectWrapper* wrapper=m_Iter.Create();
			Gravity::C_CapsuleDesc* capsule=new Gravity::C_CapsuleDesc;
			capsule->m_Radius=radius;
			capsule->m_Height=height;
			wrapper->m_Shape=capsule;
			wrapper->m_Translate=translate;
			wrapper->m_relativeTransform=relativeTransform;
		}

		void AddCylinder(const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const Maths::HVec3 PCREF translate, const Maths::HMat33 PCREF relativeTransform)
		{
			C_CompositeObjectWrapper* wrapper=m_Iter.Create();
			Gravity::C_CylinderDesc* cylinder=new Gravity::C_CylinderDesc;
			cylinder->m_Radius=radius;
			cylinder->m_Height=height;
			wrapper->m_Shape=cylinder;
			wrapper->m_Translate=translate;
			wrapper->m_relativeTransform=relativeTransform;
		}

		void AddConvexHull(const SharedPhysics::SortedArray<Maths::HVec3>& points, const Maths::HVec3 PCREF translate, const Maths::HMat33 PCREF relativeTransform)
		{
			C_CompositeObjectWrapper* wrapper=m_Iter.Create();
			Gravity::C_ConvexHullDesc* convex=new Gravity::C_ConvexHullDesc;
			for(S32 i=0; i<points.m_numElements; ++i)
			{
				convex->m_Points.AddToEnd(&points[i]);
			}
			wrapper->m_Shape=convex;
			wrapper->m_Translate=translate;
			wrapper->m_relativeTransform=relativeTransform;
		}

		void AddCompositeObject(const C_CompositeObject& obj, const Maths::HVec3 PCREF translate, const Maths::HMat33 PCREF relativeTransform)
		{
			C_CompositeObjectWrapper* wrapper=m_Iter.Create();
			Gravity::C_CompositeDesc* composite=new Gravity::C_CompositeDesc;
			wrapper->m_Shape = composite;
			wrapper->m_Translate=translate;
			wrapper->m_relativeTransform=relativeTransform;
		}

		SharedPhysics::MemoryAllocator<C_CompositeObjectWrapper, 32, true> m_pDesc;
		SharedPhysics::C_MemoryIterator<C_CompositeObjectWrapper, 32, true> m_Iter;
	};



	class C_ExportConstraint
	{
	public:
		C_ExportConstraint()
		{
		}

		~C_ExportConstraint()
		{
			Clear();
		}

		template <class Serialiser>
		void ExportConstraint(Serialiser& serial)
		{
			for(S32 i=0; i< m_DescList.m_numElements; ++i)
			{
				Gravity::C_ConstraintDescriptors* pDesc=m_DescList.m_pElems[i];
				SharedPhysics::C_Constraint::E_ConstraintType type=pDesc->m_Type;
				pDesc->Export<Serialiser>(serial);
				if(type==SharedPhysics::C_Constraint::E_Type_Spherical)
				{
					Gravity::SphericalJointDescriptor* pSphericalJ=(Gravity::SphericalJointDescriptor*)pDesc;
					pSphericalJ->Export<Serialiser>(serial);
				}
				else if(type == SharedPhysics::C_Constraint::E_Type_Revolute)
				{
					Gravity::RevoluteJointDescriptor* pRevoluteJ=(Gravity::RevoluteJointDescriptor*)pDesc;
					pRevoluteJ->Export<Serialiser>(serial);
				}
				else if(type == SharedPhysics::C_Constraint::E_Type_Prismatic)
				{
					Gravity::PrismaticJointDescriptor* pPrismaticJ=(Gravity::PrismaticJointDescriptor*)pDesc;
					pPrismaticJ->Export<Serialiser>(serial);
				}
				else if(type == SharedPhysics::C_Constraint::E_Type_SphericalLimit)
				{
					Gravity::SphericalJointLimitDescriptor* pSpericalJL=(Gravity::SphericalJointLimitDescriptor*) pDesc;
					pSpericalJL->Export<Serialiser>(serial);
				}
				else if(type == SharedPhysics::C_Constraint::E_Type_RevoluteLimit)
				{
					Gravity::RevoluteJointLimitDescriptor* pRevoluteJL = (Gravity::RevoluteJointLimitDescriptor*) pDesc;
					pRevoluteJL->Export<Serialiser>(serial);
				}
				else if(type == SharedPhysics::C_Constraint::E_Type_PrismaticLimit)
				{
					Gravity::PrismaticJointLimitDescriptor* pPrismaticJL = (Gravity::PrismaticJointLimitDescriptor*) pDesc;
					pPrismaticJL->Export<Serialiser>(serial);
				}
			
			}
		}


		void AddSphericalJointConstraint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF anchorB, const U32 bodyAState, const U32 bodyBState);
		void AddRevoluteJointConstraint (const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF anchorB, const Maths::HVec3 PCREF jointAxisB, const U32 bodyAState, const U32 bodyBState);
		void AddRevoluteJointLimitConstraint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF anchorB, const Maths::HVec3 PCREF jointAxisB, const Maths::HF32 PCREF lowLimit, const Maths::HF32 PCREF highLimit, const U32 bodyAState, const U32 bodyBState);
		void AddPrismaticJointConstraint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF jointAxisB,const U32 bodyAState, const U32 bodyBState);
		void AddPrismaticJointLimitConstraint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF jointAxisB,const U32 bodyAState, const U32 bodyBState);
		void AddCarWheelConstraint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF anchorB, const Maths::HVec3 PCREF jointAxisB,const U32 bodyAState, const U32 bodyBState);
		
		
		void AddSphericalJointConstraint(const Gravity::C_ConstraintNode* pConstraint);
		void AddSphericalJointLimitConstraint(const Gravity::C_ConstraintNode* pConstraint);
		void AddRevoluteJointConstraint (const Gravity::C_ConstraintNode* pConstraint);
		void AddRevoluteJointLimitConstraint(const Gravity::C_ConstraintNode* pConstraint);
		void AddPrismaticJointConstraint(const Gravity::C_ConstraintNode* pConstraint);
		void AddPrismaticJointLimitConstraint(const Gravity::C_ConstraintNode* pConstraint);
		void AddCarWheelConstraint(const Gravity::C_ConstraintNode* pConstraint);
		
		void Clear();

		SharedPhysics::SortedArray<Gravity::C_ConstraintDescriptors*> m_DescList;

	};

	//class C_ExportVersionCounter
	//{
	//private:
	//	C_ExportVersionCounter(): m_VersionCount(0)
	//	{
	//	}


	//	U32 m_VersionCount;

	//public:

	//	
	//	//0x110....index
	//	U32 GetVersionId()
	//	{
	//		return m_VersionCount++;
	//	}

	//	static C_ExportVersionCounter* Instance()
	//	{
	//		static C_ExportVersionCounter m_Counter;
	//		return &m_Counter;
	//	}
	//	
	//};
	
	class C_PhysicsExporter
	{
	public:
		C_PhysicsExporter(F32 splitRatio = 0.1f, F32 triangleIncreaseThreshold = 0.7f, U32 maxDepth = MAX_DEPTH) : 
			m_Iter(&m_List), m_AABBTreeSplitRatio(splitRatio), 
			m_TriangleIncreaseThreshold(triangleIncreaseThreshold),
			m_AABBTreeMaxDepth(maxDepth)
		{
		}

		~C_PhysicsExporter()
		{
			Clear();
			/*for(S32 i=0; i< m_DescList.m_numElements; ++i)
			{
				if(m_DescList[i])
					delete m_DescList[i];
				m_DescList[i]=NULL;
			}*/

		/*	if(m_pMeshDesc)
				delete m_pMeshDesc;
			m_pMeshDesc=NULL;*/
		}

		void SetSplitRatio(F32 splitRatio)
		{
			m_AABBTreeSplitRatio = splitRatio;
		}

		void SetTriangleIncreaseThreshold(F32 triangleIncreaseThreshold)
		{
			m_TriangleIncreaseThreshold = triangleIncreaseThreshold;
		}

		void SetMaxDepth(U32 maxDepth)
		{
			m_AABBTreeMaxDepth = maxDepth;
		}

		void SetModelAABBTreeParameters(F32 splitRatio, F32 triangleIncreaseThreshold, U32 maxDepth)
		{
			m_AABBTreeSplitRatio = splitRatio;
			m_TriangleIncreaseThreshold = triangleIncreaseThreshold;
			m_AABBTreeMaxDepth = maxDepth;
		}

		void ExportHeader(Sys::BinarySerialiserAutoEndian& serial);
		void Export(Sys::BinarySerialiserAutoEndian& serial);

		template<class Serialiser>
		void ExportHeader(Serialiser& serial)
		{
			
			char* header = "Head";
			for(U32 a = 0; a < 4; ++a)
			{
				serial << header[a];
			}
			
			serial << GRAVITY_CURRENT_VERSION;
		}

		template<class Serialiser>
		void Export(Serialiser& serial)
		{
			U32 size = m_Iter.NumElements();
			m_Iter.Reset();
			while(size--)
			{
				C_ShapeInstance* pInst=m_Iter.Get();
				Gravity::C_ShapeDesc* pDesc=pInst->m_pDesc;
				//pDesc->RemapShape();
				switch(pDesc->m_Type)
				{
				case Collision::E_TriangleMesh_Shape:
					{
						Gravity::C_TriangleMeshDesc* triDesc=(Gravity::C_TriangleMeshDesc*)pDesc;
						
						//pInst->m_IneriaTensor = Maths::HMat33(Maths::ZERO_MATRIX);
						Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(Maths::HMat33(Maths::ZERO_MATRIX), &pInst->m_IneriaTensor);
						
						
						triDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_Box_Shape:
					{
						Gravity::C_BoxDesc* boxDesc=(Gravity::C_BoxDesc*)pDesc;
						boxDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_ConvexHull_Shape:
					{
						Gravity::C_ConvexHullDesc* cHullDesc=(Gravity::C_ConvexHullDesc*)pDesc;
						cHullDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_Sphere_Shape:
					{
						
						Gravity::C_SphereDesc* sphereDesc=(Gravity::C_SphereDesc*)pDesc;
						sphereDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_Capsule_Shape:
					{
						Gravity::C_CapsuleDesc* capsuleDesc=(Gravity::C_CapsuleDesc*)pDesc;
						capsuleDesc->Export<Serialiser>(serial);
						break;
					}
				
				case Collision::E_Cylinder_Shape:
					{
						Gravity::C_CylinderDesc* cylDesc=(Gravity::C_CylinderDesc*)pDesc;
						cylDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_Composite_Shape:
					{
						Gravity::C_CompositeDesc* compDesc=(Gravity::C_CompositeDesc*)pDesc;
						compDesc->Export<Serialiser>(serial);
						break;
					}
				
				case Collision::E_InstancedTriangleMesh_Shape:
					{
						Gravity::C_InstancedTriangleMeshDesc* insDesc=(Gravity::C_InstancedTriangleMeshDesc*)pDesc;
						//pInst->m_IneriaTensor = Maths::HMat33(Maths::ZERO_MATRIX);
						Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(Maths::HMat33(Maths::ZERO_MATRIX), &pInst->m_IneriaTensor);
						insDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_TriangleMeshBase_Shape:
					{
						Gravity::C_TriangleMeshBaseDesc* triDesc=(Gravity::C_TriangleMeshBaseDesc*)pDesc;
						//pInst->m_IneriaTensor = Maths::HMat33(Maths::ZERO_MATRIX);
						Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(Maths::HMat33(Maths::ZERO_MATRIX), &pInst->m_IneriaTensor);
						triDesc->Export<Serialiser>(serial);
						break;
					}
				
				case Collision::E_Cloth_Shape:
					{
						Gravity::C_ClothDesc* clothDesc=(Gravity::C_ClothDesc*) pDesc;
						Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(Maths::HMat33(Maths::ZERO_MATRIX), &pInst->m_IneriaTensor);
						clothDesc->Export<Serialiser>(serial);
						break;

					}
				case Collision::E_Cone_Shape:
					{
						Gravity::C_ConeDesc* coneDesc = (Gravity::C_ConeDesc*)pDesc;
						coneDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_FluidMesh_Shape:
					{
						Gravity::C_FluidMeshDesc* fluidMeshDesc = (Gravity::C_FluidMeshDesc*)pDesc;
						fluidMeshDesc->Export<Serialiser>(serial);
						break;
					}
				default:
					{
						BIZ_ASSERT(0);
						return;
					}
				}

				pInst->Export<Serialiser>(serial);
			}

			//serial << 0xffffffff; //The end of the file
			
		}


		template<class Serialiser>
		void ExportDesc(Serialiser& serial)
		{
			for(U32 i=0; i<m_DescList.m_numElements; ++i)
			{
				Gravity::C_ShapeDesc* pDesc=m_DescList[i];
				//pDesc->RemapShape();
				switch(pDesc->m_ImportType)
				{
				case Collision::E_ImportSphere_Shape:
					{
						Gravity::C_SphereDesc* sphereDesc=(Gravity::C_SphereDesc*)pDesc;
						sphereDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_ImportBox_Shape:
					{
						Gravity::C_BoxDesc* boxDesc=(Gravity::C_BoxDesc*)pDesc;
						boxDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_ImportCapsule_Shape:
					{
						Gravity::C_CapsuleDesc* capsuleDesc=(Gravity::C_CapsuleDesc*)pDesc;
						capsuleDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_ImportConvexHull_Shape:
					{
						Gravity::C_ConvexHullDesc* cHullDesc=(Gravity::C_ConvexHullDesc*)pDesc;
						cHullDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_ImportCylinder_Shape:
					{
						Gravity::C_CylinderDesc* cylDesc=(Gravity::C_CylinderDesc*)pDesc;
						cylDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_ImportComposite_Shape:
					{
						Gravity::C_CompositeDesc* compDesc=(Gravity::C_CompositeDesc*)pDesc;
						compDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_ImportTriangleMesh_Shape:
					{
						Gravity::C_TriangleMeshDesc* triDesc=(Gravity::C_TriangleMeshDesc*)pDesc;
						triDesc->Export<Serialiser>(serial);
						break;
					}
				case Collision::E_ImportCone_Shape:
					{
						Gravity::C_ConeDesc* coneDesc = (Gravity::C_ConeDesc*)pDesc;
						coneDesc->Export<Serialiser>(serial);
						break;
					}
				default:
					{
						BIZ_ASSERT(0);
						return;
					}
				}
			}

			//serial << 0xffffffff; //The end of the file
			
		}

		//User can define the position and orientation of the importing objects
		void AddSphere(const S32 id, const Maths::HF32 PCREF radius, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddOBB(const S32 id, const Maths::HVec3 PCREF halfExtents, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddCapsule(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddCylinder(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF halfHeight, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddCone(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF halfHeight, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddConvexHull(const S32 id, const SharedPhysics::SortedArray<Maths::HVec3> PCREF points, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddCompositeObjects(Gravity::C_CompositeDesc* pDesc, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const F32 nodeFlag, const U32 userDataFlag);
		
		//if the objects are defined as static, using the position and orientation from the a3d files
		void AddSphere(const S32 id, const Maths::HF32 PCREF radius, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF trans, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddOBB(const S32 id, const Maths::HVec3 PCREF halfExtents, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF trans, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddCapsule(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const Maths::HVec3 PCREF position,  const Maths::HMat33 PCREF trans, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddCylinder(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF halfHeight, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF trans, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddCone(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF halfHeight, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF trans, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddConvexHull(const S32 id, const SharedPhysics::SortedArray<Maths::HVec3> PCREF points, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF trans, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		void AddCompositeObjects( Gravity::C_CompositeDesc* composite, const Maths::HMat33& ineriaTensor, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF trans, const U32 userDataFlag);
	
		void AddCloth(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris);
		void AddDeformable(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris);
		void AddFluidMesh(const S32 id, const F32 depth, const F32 splitRatio, const F32 normThresh, const U16 physicsMaterial, const U16 effectMaterial, const U32 nodeFlag, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*> & tris);
		
		/*void CreateStaticShootableMesh(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag);
		void CreateStaticCollidableMesh(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag);*/
		
		void CreateStaticMesh(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris);
		void CreateStaticSplineMesh(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris);
		
	/*	void AddStaticShootableMesh(const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*> & tris );
		void AddStaticCollidableMesh(const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*> & tris );*/
		//void AddStaticMesh(const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris);
		void BuildTree();
		//void AddStaticSplineMesh(const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*> & tris);
		/*void BuildShootableTree();
		void BuildCollidableTree();*/
		void BuildSplineTree();


		U32 AddInstanceBaseNode(const S32 id, const F32 splitRatio, const F32 normThresh, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*> & tris );
		void AddInstance(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U32 baseCollisionFlag, const S32 baseNodeId);
	

		void ResetIndices(const S32 startIndex, const Maths::HVec3 PCREF vertex, const U32 vertexIndex, SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris );
		void RemoveDuplicateVertices(SharedPhysics::SortedArray<Maths::HVec3>& vertices, Maths::HVec3 PCREF p, U32& index);
		void RemoveDuplicateEdge(SharedPhysics::SortedArray<Collision::C_SoftBodyEdge>& edges, Collision::C_SoftBodyEdge& other, U32& index);

		void Clear();

		SharedPhysics::MemoryAllocator<C_ShapeInstance, 32> m_List;
		SharedPhysics::C_MemoryIterator<C_ShapeInstance, 32> m_Iter;

		SharedPhysics::SortedArray<C_ShapeDesc*> m_DescList;

	/*	Gravity::C_TriangleMeshDesc* m_pShootableMeshDesc;
		Gravity::C_TriangleMeshDesc* m_pCollidableMeshDesc;*/
		SharedPhysics::SortedArray<C_TriangleMeshDesc*> m_MeshDesc; 
		Gravity::C_TriangleMeshDesc* m_pSplineMeshDesc;

		U32 m_Version;
		F32 m_AABBTreeSplitRatio;
		F32 m_TriangleIncreaseThreshold;
		U32 m_AABBTreeMaxDepth;


		//SharedPhysics::MemoryAllocator<C_ShapeDesc<Deserialiser>, 32> m_DescList;
		//SharedPhysics::C_MemoryIterator<C_ShapeDesc<Deserialiser>, 32> m_DescIter;	
	};
	
};

#endif
