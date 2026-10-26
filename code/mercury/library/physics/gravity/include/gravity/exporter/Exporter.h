#ifndef EXPORTER_H
#define EXPORTER_H

#include "gravity/sdk/ShapeDescriptors.h"
#include "gravity/sdk/ConstraintDescriptors.h"

namespace Gravity
{
	class C_ConverterTriangle;


	class C_Exporter
	{

	private:
		C_Exporter() : m_MaxConvexHullTriangles(64)
		{
		}

	public:
		void SetMaxConvexHullTriangle(const U32 maxTriangles)
		{
			m_MaxConvexHullTriangles = maxTriangles;
		}
		Gravity::SphericalJointDescriptor* addSpherialJoint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF anchorB, const U32 bodyAState, const U32 bodyBState);
		Gravity::RevoluteJointDescriptor* addRevoluteJoint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF anchorB, const Maths::HVec3 PCREF jointAxisB, const U32 bodyAState, const U32 bodyBState);
		Gravity::PrismaticJointDescriptor* addPrismaticJoint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF jointAxisB, const U32 bodyAState, const U32 bodyBState);

	//	Gravity::CarWheelDescriptor* addCarWheelDescriptor();

		Gravity::C_SphereDesc* AddSphere(const S32 id, const Maths::HF32 PCREF radius, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		Gravity::C_BoxDesc* AddOBB(const S32 id, const Maths::HVec3 PCREF halfExtents, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		Gravity::C_CapsuleDesc* AddCapsule(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		Gravity::C_CylinderDesc* AddCylinder(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		Gravity::C_ConvexHullDesc* AddConvexHull(const S32 id, const SharedPhysics::SortedArray<Maths::HVec3>& points, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		Gravity::C_ConeDesc* AddCone(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag);
		Gravity::C_ClothDesc* CreateCloth();
		Gravity::C_DeformableDesc* CreateDeformable();

		
		/*
			All the static collidable mesh node should just create one aabb tree, therefore we just call createTriangleMesh one time, the pass the tree inside
			the triangle mesh descriptor into the AddTriangleMesh method
		*/
		Gravity::C_TriangleMeshDesc* CreateTriangleStaticMesh();

		void AddTriangleStaticMesh(Baking::AABBTree& tree, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris);


		/*
			This method for the mesh inside the compound objects. We need to transform all the vertex inside the mesh node into
			the local space of the mesh descriptor
		*/
		void AddTriangleMesh(Baking::AABBTree& tree, const Maths::HMat43 PCREF transform, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle* >& tris);

		Gravity::C_FluidMeshDesc* AddFluidMesh(const S32 id, const F32 depth, const F32 splitRatio, const F32 normThresh, const U16 physicsMaterial, const U16 effectMaterial, const U32 nodeFlag, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris);
		
		
		Gravity::C_TriangleMeshBaseDesc* AddInstanceBase(const S32 id, const F32 splitRatio, const F32 normThresh, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris);
		Gravity::C_InstancedTriangleMeshDesc* AddInstance(const S32 id, const S32 baseNodeID);
		
		Gravity::C_CompositeDesc* AddCompositeObjects(const S32 id, Gravity::C_CompositeDesc* composite, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass);
		Gravity::C_CompositeDesc* AddCompositeObjects();
		

		static C_Exporter* Instance()
		{
			static C_Exporter m_sExporter;
			return &m_sExporter;
		}

		U32 m_MaxConvexHullTriangles;

		
	};
}



#endif



