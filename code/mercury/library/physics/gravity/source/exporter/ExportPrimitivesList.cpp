
#include "Stdafx.h"
#include "gravity/exporter/ExportPrimitivesList.h"
#include <System/Debug/I_DebugRender.h>
#include "misc_shared/Geometry/ConvexHull.h"
#include "gravity/sdk/InertiaTensorGenerator.h"
#include "gravity/exporter/GravityExporter.h"


namespace Gravity
{

	void C_ExportConstraint::AddSphericalJointConstraint(const S32 constraintId,const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF anchorB, const U32 bodyAState, const U32 bodyBState)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::SphericalJointDescriptor* pDesc=m_Exporter->addSpherialJoint(constraintId, bodyA_id, bodyB_id, anchorA, anchorB, bodyAState, bodyBState);
		m_DescList.AddToEnd((Gravity::C_ConstraintDescriptors**)&pDesc);
	}

	
	void C_ExportConstraint::AddRevoluteJointConstraint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF anchorB, const Maths::HVec3 PCREF jointAxisB, const U32 bodyAState, const U32 bodyBState)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::RevoluteJointDescriptor* pDesc=m_Exporter->addRevoluteJoint(constraintId, bodyA_id, bodyB_id, anchorA, jointAxisA, anchorB, jointAxisB, bodyAState, bodyBState);
		m_DescList.AddToEnd((Gravity::C_ConstraintDescriptors**)&pDesc);
	}

	
	void C_ExportConstraint::AddPrismaticJointConstraint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF jointAxisB, const U32 bodyAState, const U32 bodyBState)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::PrismaticJointDescriptor* pDesc=m_Exporter->addPrismaticJoint(constraintId, bodyA_id, bodyB_id, jointAxisA, jointAxisB, bodyAState, bodyBState);
		m_DescList.AddToEnd((Gravity::C_ConstraintDescriptors**)&pDesc);
	}


	void C_ExportConstraint::AddSphericalJointConstraint(const Gravity::C_ConstraintNode* pConstraint)
	{
		using namespace Maths;

	
		const HVec3 position(pConstraint->m_Transform.Translation());

		const HMat43 inverTransA = pConstraint->m_BodyAWTransform.InverseRotTrans();
		const HMat43 inverTransB = pConstraint->m_BodyBWTransform.InverseRotTrans();
		
		Gravity::SphericalJointDescriptor* pDesc = new Gravity::SphericalJointDescriptor;
		pDesc->m_Id = pConstraint->m_UniqueID;
		pDesc->m_PhysicsBodyAID = pConstraint->m_pPhysicsBodyA->m_UniqueID;
		pDesc->m_PhysicsBodyBID = pConstraint->m_pPhysicsBodyB->m_UniqueID;

		if(pConstraint->m_MaskProxy)
		{
			pDesc->m_MaskProxy = true;
		}
		else
		{
			pDesc->m_MaskProxy = false;
			
			pDesc->m_AnchorPoint1 = position * inverTransA;
			pDesc->m_AnchorPoint2 = position * inverTransB;
		
			pDesc->m_MaskCollisions = pConstraint->m_MaskCollisions;
			
			pDesc->m_PhysicsBodyAState = pConstraint->m_PhysicsBodyAStatus;
			pDesc->m_PhysicsBodyBState = pConstraint->m_PhysicsBodyBStatus;
		}

		m_DescList.AddToEnd((Gravity::C_ConstraintDescriptors**)&pDesc);
	}


	void C_ExportConstraint::AddSphericalJointLimitConstraint(const Gravity::C_ConstraintNode* pConstraint)
	{
		using namespace Maths;
		const HVec3 position(pConstraint->m_Transform.Translation());

		const HMat43 inverTransA = pConstraint->m_BodyAWTransform.InverseRotTrans();
		const HMat43 inverTransB = pConstraint->m_BodyBWTransform.InverseRotTrans();

		Gravity::SphericalJointLimitDescriptor* pDesc=new Gravity::SphericalJointLimitDescriptor;
		pDesc->m_Id = pConstraint->m_UniqueID;
		pDesc->m_PhysicsBodyAID = pConstraint->m_pPhysicsBodyA->m_UniqueID;
		pDesc->m_PhysicsBodyBID = pConstraint->m_pPhysicsBodyB->m_UniqueID;

		if(pConstraint->m_MaskProxy)
		{
			pDesc->m_MaskProxy = true;
		}
		else
		{
			pDesc->m_MaskProxy = false;
			pDesc->m_AnchorPoint1 = position * inverTransA;
			pDesc->m_AnchorPoint2 = position * inverTransB;
			pDesc->m_PhysicsBodyAState = pConstraint->m_PhysicsBodyAStatus;
			pDesc->m_PhysicsBodyBState = pConstraint->m_PhysicsBodyBStatus;
			pDesc->m_MaskCollisions = pConstraint->m_MaskCollisions;
			pDesc->m_LowJointLimit = HVec3(pConstraint->m_MinRotateX, pConstraint->m_MinRotateY, pConstraint->m_MinRotateZ);
			pDesc->m_HighJointLimit = HVec3(pConstraint->m_MaxRotateX, pConstraint->m_MaxRotateY, pConstraint->m_MaxRotateZ);
		}
		m_DescList.AddToEnd((Gravity::C_ConstraintDescriptors**)&pDesc);
	}


	void C_ExportConstraint::AddRevoluteJointConstraint(const Gravity::C_ConstraintNode* pConstraint)
	{
		using namespace Maths;
		const HVec3 position(pConstraint->m_Transform.Translation());

		const HMat43 inverTransA = pConstraint->m_BodyAWTransform.InverseRotTrans();
		const HMat43 inverTransB = pConstraint->m_BodyBWTransform.InverseRotTrans();

		const HMat33 invRotA = inverTransA.Rotation();
		const HMat33 rotB = pConstraint->m_BodyBWTransform.Rotation();

		Gravity::RevoluteJointDescriptor* pDesc=new Gravity::RevoluteJointDescriptor;
		pDesc->m_Id = pConstraint->m_UniqueID;
		pDesc->m_PhysicsBodyAID = pConstraint->m_pPhysicsBodyA->m_UniqueID;
		pDesc->m_PhysicsBodyBID = pConstraint->m_pPhysicsBodyB->m_UniqueID;

		if(pConstraint->m_MaskProxy)
		{
			pDesc->m_MaskProxy = true;
		}
		else
		{
			pDesc->m_MaskProxy = false;
			pDesc->m_AnchorPoint1 = position * inverTransA;
			pDesc->m_AnchorPoint2 = position * inverTransB;
			
			pDesc->m_PhysicsBodyAState = pConstraint->m_PhysicsBodyAStatus;
			pDesc->m_PhysicsBodyBState = pConstraint->m_PhysicsBodyBStatus;
			pDesc->m_MaskCollisions = pConstraint->m_MaskCollisions;

			const Maths::HMat33 aRot = pConstraint->m_pPhysicsBodyA->m_Transform.Rotation();
			const Maths::HMat33 bRot = pConstraint->m_pPhysicsBodyB->m_Transform.Rotation();

			switch(pConstraint->m_Axis)
			{
			case 0: // x
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeX() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeX();
				break;
			case 1:
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeY() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeY();
				break;
			case 2:
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeZ() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeZ();
				break;
			default:
				BIZ_ASSERT(0);
			}
			pDesc->m_JointAxis1 = (pDesc->m_JointAxis2 * aRot) * bRot.InverseRotation();
		}

		m_DescList.AddToEnd((Gravity::C_ConstraintDescriptors**)&pDesc);
	}


	void C_ExportConstraint::AddRevoluteJointLimitConstraint(const Gravity::C_ConstraintNode* pConstraint)
	{
		using namespace Maths;
		const HVec3 position(pConstraint->m_Transform.Translation());

		

		const HMat43 inverTransA = pConstraint->m_BodyAWTransform.InverseRotTrans();
		const HMat43 inverTransB = pConstraint->m_BodyBWTransform.InverseRotTrans();

		const HMat33 invRotA = inverTransA.Rotation();
		const HMat33 rotB = pConstraint->m_BodyBWTransform.Rotation();


		Gravity::RevoluteJointLimitDescriptor* pDesc=new Gravity::RevoluteJointLimitDescriptor;
		pDesc->m_Id = pConstraint->m_UniqueID;
		pDesc->m_PhysicsBodyAID = pConstraint->m_pPhysicsBodyA->m_UniqueID;
		pDesc->m_PhysicsBodyBID = pConstraint->m_pPhysicsBodyB->m_UniqueID;

		if(pConstraint->m_MaskProxy)
		{
			pDesc->m_MaskProxy = true;
		}
		else
		{
			pDesc->m_MaskProxy = false;

			pDesc->m_AnchorPoint1 = position * inverTransA;
			pDesc->m_AnchorPoint2 = position * inverTransB;
			pDesc->m_PhysicsBodyAState = pConstraint->m_PhysicsBodyAStatus;
			pDesc->m_PhysicsBodyBState = pConstraint->m_PhysicsBodyBStatus;
			pDesc->m_MaskCollisions = pConstraint->m_MaskCollisions;

			const Maths::HMat33 aRot = pConstraint->m_pPhysicsBodyA->m_Transform.Rotation();
			const Maths::HMat33 bRot = pConstraint->m_pPhysicsBodyB->m_Transform.Rotation();

			switch(pConstraint->m_Axis)
			{
			case 0: // x
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeX() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeX();
				pDesc->m_LowJointLimit = HF32(pConstraint->m_MinRotateX);
				pDesc->m_HighJointLimit = HF32(pConstraint->m_MaxRotateX);
				break;
			case 1:
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeY() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeY();
				pDesc->m_LowJointLimit = HF32(pConstraint->m_MinRotateY);
				pDesc->m_HighJointLimit = HF32(pConstraint->m_MaxRotateY);
				break;
			case 2:
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeZ() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeZ();
				pDesc->m_LowJointLimit = HF32(pConstraint->m_MinRotateZ);
				pDesc->m_HighJointLimit = HF32(pConstraint->m_MaxRotateZ);
				break;
			default:
				BIZ_ASSERT(0);
			}
			pDesc->m_JointAxis1 = (pDesc->m_JointAxis2 * aRot) * bRot.InverseRotation();
		}

		m_DescList.AddToEnd((Gravity::C_ConstraintDescriptors**)&pDesc);
	}

	
	void C_ExportConstraint::AddPrismaticJointConstraint(const Gravity::C_ConstraintNode* pConstraint)
	{
		using namespace Maths;

		Gravity::PrismaticJointDescriptor* pDesc=new Gravity::PrismaticJointDescriptor;
		pDesc->m_Id = pConstraint->m_UniqueID;
		pDesc->m_PhysicsBodyAID = pConstraint->m_pPhysicsBodyA->m_UniqueID;
		pDesc->m_PhysicsBodyBID = pConstraint->m_pPhysicsBodyB->m_UniqueID;

		const HMat33 invRotA = pConstraint->m_BodyAWTransform.Rotation().InverseRotation();
		const HMat33 rotB = pConstraint->m_BodyBWTransform.Rotation();

		if(pConstraint->m_MaskProxy)
		{
			pDesc->m_MaskProxy = true;
		}
		else
		{
			pDesc->m_MaskProxy = false;
			pDesc->m_PhysicsBodyAState = pConstraint->m_PhysicsBodyAStatus;
			pDesc->m_PhysicsBodyBState = pConstraint->m_PhysicsBodyBStatus;
			pDesc->m_MaskCollisions = pConstraint->m_MaskCollisions;

			const Maths::HMat33 aRot = pConstraint->m_pPhysicsBodyA->m_Transform.Rotation();
			const Maths::HMat33 bRot = pConstraint->m_pPhysicsBodyB->m_Transform.Rotation();

			switch(pConstraint->m_Axis)
			{
			case 0: // x
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeX() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeX();
				break;
			case 1:
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeY() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeY();
				break;
			case 2:
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeZ() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeZ();
				break;
			default:
				BIZ_ASSERT(0);
			}
			pDesc->m_JointAxis1 = (pDesc->m_JointAxis2 * aRot) * bRot.InverseRotation();
		}

		m_DescList.AddToEnd((Gravity::C_ConstraintDescriptors**)&pDesc);
	}

	void C_ExportConstraint::AddPrismaticJointLimitConstraint(const Gravity::C_ConstraintNode* pConstraint)
	{
		using namespace Maths;
	
		Gravity::PrismaticJointLimitDescriptor* pDesc=new Gravity::PrismaticJointLimitDescriptor;
		pDesc->m_Id = pConstraint->m_UniqueID;
		pDesc->m_PhysicsBodyAID = pConstraint->m_pPhysicsBodyA->m_UniqueID;
		pDesc->m_PhysicsBodyBID = pConstraint->m_pPhysicsBodyB->m_UniqueID;
		const HMat33 invRotA = pConstraint->m_BodyAWTransform.Rotation().InverseRotation();
		const HMat33 rotB = pConstraint->m_BodyBWTransform.Rotation();
		if(pConstraint->m_MaskProxy)
		{
			pDesc->m_MaskProxy = true;
		}
		else
		{
			pDesc->m_MaskProxy = false;
			pDesc->m_PhysicsBodyAState = pConstraint->m_PhysicsBodyAStatus;
			pDesc->m_PhysicsBodyBState = pConstraint->m_PhysicsBodyBStatus;
			pDesc->m_LowJointLimit = HF32(pConstraint->m_MinSlide);
			pDesc->m_HighJointLimit = HF32(pConstraint->m_MaxSlide);
			pDesc->m_MaskCollisions = pConstraint->m_MaskCollisions;

			const Maths::HMat33 aRot = pConstraint->m_pPhysicsBodyA->m_Transform.Rotation();
			const Maths::HMat33 bRot = pConstraint->m_pPhysicsBodyB->m_Transform.Rotation();

			switch(pConstraint->m_Axis)
			{
			case 0: // x
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeX() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeX();
				break;
			case 1:
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeY() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeY();
				break;
			case 2:
				pDesc->m_JointAxis1 = (Constants<HVec3>::UnitAxisNegativeZ() * rotB)*invRotA;
				pDesc->m_JointAxis2 = Constants<HVec3>::UnitAxisNegativeZ();
				break;
			default:
				BIZ_ASSERT(0);
			}
			pDesc->m_JointAxis1 = (pDesc->m_JointAxis2 * aRot) * bRot.InverseRotation();
		}

		m_DescList.AddToEnd((Gravity::C_ConstraintDescriptors**)&pDesc);
	}



	void C_ExportConstraint::Clear()
	{
		for(S32 i=0; i<m_DescList.m_numElements; ++i)
		{
			if(m_DescList[i])
				delete m_DescList[i];
			m_DescList[i]=NULL;
		}
		m_DescList.m_numElements = 0;
	}

	
	void C_PhysicsExporter::AddSphere(const S32 id, const Maths::HF32 PCREF radius, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_SphereDesc* pDesc = m_Exporter->AddSphere(id, radius, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);
	}

	
	void C_PhysicsExporter::AddOBB(const S32 id, const Maths::HVec3 PCREF halfExtents, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{

		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_BoxDesc* pDesc=m_Exporter->AddOBB(id, halfExtents, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);
	}

	
	void C_PhysicsExporter::AddCapsule(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_CapsuleDesc* pDesc=m_Exporter->AddCapsule(id, radius, height, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);
	}

	
	void C_PhysicsExporter::AddCylinder(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{

		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_CylinderDesc* pDesc=m_Exporter->AddCylinder(id, radius, height, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);
		pDesc->m_Radius=radius;
		pDesc->m_Height=height;
		
		const Maths::HF32 margin(Maths::Min(radius, height) * 0.2f);
		const Maths::HF32 epsilon(Maths::Max(radius, height) * 0.001f);
		pDesc->m_Margin = margin;
		pDesc->m_Epsilon = epsilon;
		pDesc->m_PhysicsMaterialIndex=physicsMaterial;
		pDesc->m_EffectMaterialIndex =effectMaterial;
		pDesc->m_CollisionFlag=collisionFlag;

	
	}

	
	void C_PhysicsExporter::AddConvexHull(const S32 id, const SharedPhysics::SortedArray<Maths::HVec3> PCREF points, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{
		if(points.m_numElements < 4 )
			return;
		ConvexHull::C_Mesh mesh;
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_ConvexHullDesc* pDesc=m_Exporter->AddConvexHull(id, points, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);
	}

	
	void C_PhysicsExporter::AddCompositeObjects(C_CompositeDesc* composite, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const F32 nodeFlag, const U32 userDataFlag)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_CompositeDesc* pDesc=composite;
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);
		pDesc->m_PhysicsMaterialIndex=physicsMaterial;
		pDesc->m_EffectMaterialIndex =effectMaterial;
		pDesc->m_CollisionFlag=collisionFlag;
		pDesc->m_UserDataFlag = userDataFlag;
		pDesc->m_NodeFlag=(U32)nodeFlag;
	}
	
	/*
		export the position and orientation from a3d file as well
	*/
	
	void C_PhysicsExporter::AddSphere(const S32 id, const Maths::HF32 PCREF radius, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_SphereDesc * pDesc =m_Exporter->AddSphere(id, radius, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);
		
		C_ShapeInstance* pShape=m_Iter.Create();
		Maths::HMat33 ineriaTensor=Gravity::C_InertiaTensorGenerator::SphereInertiaTensor(mass, radius); 
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(ineriaTensor, &pShape->m_IneriaTensor);
		//pShape->m_IneriaTensor = Gravity::C_InertiaTensorGenerator::SphereInertiaTensor(mass, radius); 
		pShape->m_pDesc=pDesc;
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);
	
	}

	
	void C_PhysicsExporter::AddOBB(const S32 id, const Maths::HVec3 PCREF halfExtents, const Maths::HVec3 PCREF position,  const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{

		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_BoxDesc* pDesc=m_Exporter->AddOBB(id, halfExtents, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		//pShape->m_IneriaTensor=Gravity::C_InertiaTensorGenerator::BoxInertiaTensor(mass, halfExtents);

		Maths::HMat33 ineriaTensor=Gravity::C_InertiaTensorGenerator::BoxInertiaTensor(mass,halfExtents); 
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(ineriaTensor, &pShape->m_IneriaTensor);

		pShape->m_pDesc=pDesc;
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);
	}

	
	void C_PhysicsExporter::AddCapsule(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_CapsuleDesc* pDesc= m_Exporter->AddCapsule(id, radius, height, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);
	
		pDesc->m_Epsilon = Maths::Max(height, radius) * 0.005f;
		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pDesc;
		const Maths::HMat33 ineriaTensor =Gravity::C_InertiaTensorGenerator::CapsuleInertiaTensor(mass, height, mass, radius);
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(ineriaTensor, &pShape->m_IneriaTensor);

		//pShape->m_IneriaTensor=Gravity::C_InertiaTensorGenerator::CapsuleInertiaTensor(mass, height, mass, radius);
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);

	}

	
	void C_PhysicsExporter::AddCylinder(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass,
		const U32 userDataFlag)
	{

		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_CylinderDesc* pDesc= m_Exporter->AddCylinder(id, radius, height, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pDesc;

		const Maths::HMat33 ineriaTensor = Gravity::C_InertiaTensorGenerator::CylinderInertiaTensor(mass, radius, height);
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(ineriaTensor, &pShape->m_IneriaTensor);

		//pShape->m_IneriaTensor= Gravity::C_InertiaTensorGenerator::CylinderInertiaTensor(mass, radius, height);
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);
	
	}

	void C_PhysicsExporter::AddCone(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{

		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_ConeDesc* pDesc= m_Exporter->AddCone(id, radius, height, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pDesc;

		const Maths::HMat33 ineriaTensor = Gravity::C_InertiaTensorGenerator::ConeInertiaTensor(mass, radius, height);
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(ineriaTensor, &pShape->m_IneriaTensor);

		//pShape->m_IneriaTensor= Gravity::C_InertiaTensorGenerator::CylinderInertiaTensor(mass, radius, height);
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);
	
	}

	
	void C_PhysicsExporter::AddConvexHull(const S32 id, const SharedPhysics::SortedArray<Maths::HVec3> PCREF points, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{
		if(points.m_numElements < 4 )
			return;
		ConvexHull::C_Mesh mesh;
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_ConvexHullDesc* pDesc=m_Exporter->AddConvexHull(id, points, physicsMaterial, effectMaterial, collisionFlag, nodeFlag, mass, userDataFlag);
		if(!pDesc)
			return;
		//pDesc->RemapShape();
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pDesc;
		const Maths::HMat33 ineriaTensor = Gravity::C_InertiaTensorGenerator::ConvexHullInertiaTensor(mass, points);
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(ineriaTensor, &pShape->m_IneriaTensor);

		//pShape->m_IneriaTensor=Gravity::C_InertiaTensorGenerator::ConvexHullInertiaTensor(mass, points);
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);
	}

	
	void C_PhysicsExporter::AddCompositeObjects( Gravity::C_CompositeDesc* composite, const Maths::HMat33& ineriaTensor, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U32 userDataFlag)
	{
		Gravity::C_CompositeDesc* pDesc=composite;
		pDesc->CalculateNormalisedMasses(Maths::HF32(1.f));
		pDesc->RemapShape();
		pDesc->SetRemapped();
		pDesc->m_UserDataFlag = userDataFlag;
		const Maths::HMat33 inertiaTensor = pDesc->InertiaTensor(pDesc->m_Mass);
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDesc);
		

		C_ShapeInstance* pShape=m_Iter.Create();
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(inertiaTensor, &pShape->m_IneriaTensor);
	
		pShape->m_pDesc=pDesc;
		pShape->m_Position=position;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);
		
	}

	void C_PhysicsExporter::ResetIndices(const S32 startIndex, const Maths::HVec3 PCREF vertex, const U32 vertexIndex, SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris )
	{
		for(S32 j=startIndex; j<tris.m_numElements; ++j)
		{
			Gravity::C_ConverterTriangle* tri=tris[j];
			const Maths::HVec3& v_1=tri->vertexA;
			const Maths::HVec3& v_2=tri->vertexB;
			const Maths::HVec3& v_3=tri->vertexC;

			if(vertex.All_EQ(v_1))
			{
				tri->m_IndexA = vertexIndex;
			}
			else if(vertex.All_EQ(v_2))
			{
				tri->m_IndexB = vertexIndex;
			}
			else if(vertex.All_EQ(v_3))
			{
				tri->m_IndexC = vertexIndex;
			}
		}

	}

	void C_PhysicsExporter::RemoveDuplicateVertices(SharedPhysics::SortedArray<Maths::HVec3>& vertices, Maths::HVec3 PCREF p, U32& index)
	{
		const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		for(S32 j=0; j<vertices.m_numElements; ++j)
		{
			const Maths::HVec3& vertex=vertices[j];
			//if(p.All_EQ(vertex))
			const Maths::HVec3 dif(p - vertex);
			const Maths::HF32 dd(dif | dif);
			if(dd < epsilon)
			{
				index = j;
				return;
			}
		}

		index = vertices.m_numElements;
		vertices.AddToEnd(&p);
		

	}

	void C_PhysicsExporter::RemoveDuplicateEdge(SharedPhysics::SortedArray<Collision::C_SoftBodyEdge>& edges, Collision::C_SoftBodyEdge& other, U32& index)
	{
		const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		for(S32 j=0; j<edges.m_numElements; ++j)
		{
			Collision::C_SoftBodyEdge& edge = edges[j];
			if(edge ==  other)
			{
				index = j;
				return;
			}
		}

		index = edges.m_numElements;
		edges.AddToEnd(&other);

	}

	void C_PhysicsExporter::AddCloth(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag , SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_ClothDesc* pClothDesc=m_Exporter->CreateCloth();
		pClothDesc->m_OriginalExportID= id;
		pClothDesc->m_PhysicsMaterialIndex = physicsMaterial;
		pClothDesc->m_EffectMaterialIndex = effectMaterial;
		pClothDesc->m_CollisionFlag = collisionFlag;
		pClothDesc->m_Margin = 0.f;
		pClothDesc->m_NodeFlag = nodeFlag;
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pClothDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pClothDesc;
		pShape->m_Position=position;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);


		/*
			Remove all the duplicate vertices and reset the index to the corresponding vertices
		*/

		for(S32 i = 0; i < tris.m_numElements; ++i)
		{
			Gravity::C_ConverterTriangle* tri=tris[i];
			Maths::HF32 massA(tri->m_ColourA.Colour() > 0 ? 0.f : 1.f);
			Maths::HF32 massB(tri->m_ColourB.Colour() > 0 ? 0.f : 1.f);
			Maths::HF32 massC(tri->m_ColourC.Colour() > 0 ? 0.f : 1.f);
			pClothDesc->m_ExportClothTree->AddTriangle(tri->vertexA, tri->vertexB, tri->vertexC,
				massA, massB, massC);
		}

		pClothDesc->m_ExportClothTree->BuildTree();


		/*for(S32 i=0; i< tris.m_numElements; ++i)
		{
			Gravity::C_ConverterTriangle* tri=tris[i];
			RemoveDuplicateVertices(pClothDesc->m_Vertices, tri->vertexA, tri->m_IndexA);
			RemoveDuplicateVertices(pClothDesc->m_Vertices, tri->vertexB, tri->m_IndexB);
			RemoveDuplicateVertices(pClothDesc->m_Vertices, tri->vertexC, tri->m_IndexC);
		}

		for(S32 i=0; i<tris.m_numElements; ++i)
		{
			Gravity::C_ConverterTriangle* tri=tris[i];

			Collision::C_ClothEdge edgeA, edgeB, edgeC;
			edgeA.particleA = tri->m_IndexA;
			edgeA.particleB = tri->m_IndexB;
			Maths::HVec3 lengthVec = tri->vertexB - tri->vertexA;
			edgeA.restLength=Maths::Sqrt(lengthVec|lengthVec);

			edgeB.particleA = tri->m_IndexA;
			edgeB.particleB = tri->m_IndexC;
			lengthVec = tri->vertexC - tri->vertexA;
			edgeB.restLength=Maths::Sqrt(lengthVec|lengthVec);

			edgeC.particleA = tri->m_IndexB;
			edgeC.particleB = tri->m_IndexC;
			
			lengthVec = tri->vertexC - tri->vertexB;
			edgeC.restLength = Maths::Sqrt(lengthVec | lengthVec);

			Collision::C_ClothTriangle clothTri;
			RemoveDuplicateEdge(pClothDesc->m_Edges, edgeA, clothTri.m_EdgeIndexA);
			RemoveDuplicateEdge(pClothDesc->m_Edges, edgeB, clothTri.m_EdgeIndexB);
			RemoveDuplicateEdge(pClothDesc->m_Edges, edgeC, clothTri.m_EdgeIndexC);

			pClothDesc->m_Triangles.AddToEnd(&clothTri);
		}*/
	}

	void C_PhysicsExporter::AddDeformable(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag , SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_DeformableDesc* pDeformableDesc=m_Exporter->CreateDeformable();
		pDeformableDesc->m_OriginalExportID= id;
		pDeformableDesc->m_PhysicsMaterialIndex = physicsMaterial;
		pDeformableDesc->m_EffectMaterialIndex = effectMaterial;
		pDeformableDesc->m_CollisionFlag = collisionFlag;
		pDeformableDesc->m_Margin = 0.f;
		pDeformableDesc->m_NodeFlag = nodeFlag;
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pDeformableDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pDeformableDesc;
		pShape->m_Position=position;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);


		/*
			Remove all the duplicate vertices and reset the index to the corresponding vertices
		*/

		for(S32 i = 0; i < tris.m_numElements; ++i)
		{
			Gravity::C_ConverterTriangle* tri=tris[i];
			Maths::HF32 massA(tri->m_ColourA.Colour() > 0 ? 0.f : 1.f);
			Maths::HF32 massB(tri->m_ColourB.Colour() > 0 ? 0.f : 1.f);
			Maths::HF32 massC(tri->m_ColourC.Colour() > 0 ? 0.f : 1.f);
			pDeformableDesc->m_ExportDeformableTree->AddTriangle(tri->vertexA, tri->vertexB, tri->vertexC,
				massA, massB, massC);
		}
		pDeformableDesc->m_ExportDeformableTree->BuildTree();
	}


	void C_PhysicsExporter::AddFluidMesh(const S32 id, const F32 depth, const F32 splitRatio, const F32 normThresh, const U16 physicsMaterial, const U16 effectMaterial, const U32 nodeFlag, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*> & tris)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_FluidMeshDesc* pMeshDesc=m_Exporter->AddFluidMesh(id, depth, splitRatio, normThresh, physicsMaterial, effectMaterial, nodeFlag, tris);

		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pMeshDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pMeshDesc;
		pShape->m_Position=position;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);
	}

	void C_PhysicsExporter::CreateStaticMesh(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_TriangleMeshDesc* pMeshDesc=m_Exporter->CreateTriangleStaticMesh();
		pMeshDesc->m_OriginalExportID= id;
		pMeshDesc->m_PhysicsMaterialIndex= physicsMaterial;
		pMeshDesc->m_EffectMaterialIndex = effectMaterial;
		pMeshDesc->m_CollisionFlag = collisionFlag;
		pMeshDesc->m_Margin = 0.f;
		pMeshDesc->m_NodeFlag = nodeFlag;
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pMeshDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pMeshDesc;
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);

		m_MeshDesc.AddToEnd(&pMeshDesc);

		m_Exporter->AddTriangleStaticMesh(*pMeshDesc->m_AABBTree, tris);
		
	}


	void C_PhysicsExporter::CreateStaticSplineMesh(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_TriangleMeshDesc* pMeshDesc=m_Exporter->CreateTriangleStaticMesh();
		pMeshDesc->m_OriginalExportID= id;
		pMeshDesc->m_PhysicsMaterialIndex= physicsMaterial;
		pMeshDesc->m_EffectMaterialIndex = effectMaterial;
		pMeshDesc->m_CollisionFlag = collisionFlag;
		pMeshDesc->m_Margin = 0.f;
		pMeshDesc->m_NodeFlag = nodeFlag;
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pMeshDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pMeshDesc;
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);

		m_pSplineMeshDesc = pMeshDesc;

		m_Exporter->AddTriangleStaticMesh(*pMeshDesc->m_AABBTree, tris);
	}



	void C_PhysicsExporter::BuildTree()
	{
		for(S32 i=0; i<m_MeshDesc.m_numElements; ++i)
		{
			C_TriangleMeshDesc* pMeshDesc=m_MeshDesc[i];
			pMeshDesc->m_AABBTree->setMaxDepth(m_AABBTreeMaxDepth);
			pMeshDesc->m_AABBTree->setSplitRatio(m_AABBTreeSplitRatio);
			Maths::HF32 normThreshV(m_TriangleIncreaseThreshold);
			pMeshDesc->m_AABBTree->BuildAABBTree(normThreshV);
			pMeshDesc->m_CollisionFlag = pMeshDesc->GetCombinedCollisionFlags();
		}
	}


	void C_PhysicsExporter::BuildSplineTree()
	{
		m_pSplineMeshDesc->m_AABBTree->setMaxDepth(m_AABBTreeMaxDepth);
		m_pSplineMeshDesc->m_AABBTree->setSplitRatio(m_AABBTreeSplitRatio);
		m_pSplineMeshDesc->m_AABBTree->SplitY(false);
		Maths::HF32 normThreshV(0.f);
		m_pSplineMeshDesc->m_AABBTree->BuildAABBTree(normThreshV);
		m_pSplineMeshDesc->m_CollisionFlag = m_pSplineMeshDesc->GetCombinedCollisionFlags();
	}

	
	U32 C_PhysicsExporter::AddInstanceBaseNode(const S32 id, const F32 splitRatio, const F32 normThresh, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*> & tris)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_TriangleMeshBaseDesc* pMeshDesc=m_Exporter->AddInstanceBase(id, splitRatio, normThresh, tris);
		pMeshDesc->m_CollisionFlag = pMeshDesc->GetCombinedCollisionFlags();
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pMeshDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pMeshDesc;
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);

		return pMeshDesc->m_CollisionFlag;
	}


	void C_PhysicsExporter::AddInstance(const S32 id, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF rotation, const U32 baseCollisionFlag, const S32 baseNodeId)
	{
		Gravity::C_Exporter* m_Exporter=Gravity::C_Exporter::Instance();
		Gravity::C_InstancedTriangleMeshDesc* pMeshDesc=m_Exporter->AddInstance(id, baseNodeId);
		pMeshDesc->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Static;
		pMeshDesc->m_CollisionFlag = baseCollisionFlag;
		m_DescList.AddToEnd((Gravity::C_ShapeDesc**)&pMeshDesc);

		C_ShapeInstance* pShape=m_Iter.Create();
		pShape->m_pDesc=pMeshDesc;
		pShape->m_Position=position;
		//pShape->m_Rotation=rotation;
		Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(rotation, &pShape->m_Rotation);
	}

	void C_PhysicsExporter::Clear()
	{

		m_Iter.Reset();

		//The m_DescDesc is included in the m_DescList, so it is no needed to delete it again
		for(S32 i=0; i< m_DescList.m_numElements; ++i)
		{
			if(m_DescList[i])
				delete m_DescList[i];
			m_DescList[i]=NULL;
		}
		m_DescList.m_numElements = 0;

		/*for(S32 i=0; i< m_MeshDesc.m_numElements; ++i)
		{
			if(m_MeshDesc[i])
				delete m_MeshDesc[i];
			m_MeshDesc[i]=NULL;
		}*/

		m_MeshDesc.m_numElements = 0;

	}

	
	void C_PhysicsExporter::ExportHeader(Sys::BinarySerialiserAutoEndian& serial)
	{
		
		char* header = "Head";
		for(U32 a = 0; a < 4; ++a)
		{
			serial << header[a];
		}
		
		serial << GRAVITY_CURRENT_VERSION;
	}


	void C_PhysicsExporter::Export(Sys::BinarySerialiserAutoEndian& serial)
	{
		U32 size = m_Iter.NumElements();
		m_Iter.Reset();
		while(size--)
		{
			C_ShapeInstance* pInst=m_Iter.Get();
			Gravity::C_ShapeDesc* pDesc=pInst->m_pDesc;
			//pDesc->RemapShape();
			switch(pDesc->m_ImportType)
			{
			case Collision::E_ImportTriangleMesh_Shape:
				{
					Gravity::C_TriangleMeshDesc* triDesc=(Gravity::C_TriangleMeshDesc*)pDesc;
					
					//pInst->m_IneriaTensor = Maths::HMat33(Maths::ZERO_MATRIX);
					Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(Maths::HMat33(Maths::ZERO_MATRIX), &pInst->m_IneriaTensor);
					
					
					triDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			case Collision::E_ImportBox_Shape:
				{
					Gravity::C_BoxDesc* boxDesc=(Gravity::C_BoxDesc*)pDesc;
					boxDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			case Collision::E_ImportConvexHull_Shape:
				{
					Gravity::C_ConvexHullDesc* cHullDesc=(Gravity::C_ConvexHullDesc*)pDesc;
					cHullDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			case Collision::E_ImportSphere_Shape:
				{
					
					Gravity::C_SphereDesc* sphereDesc=(Gravity::C_SphereDesc*)pDesc;
					sphereDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			case Collision::E_ImportCapsule_Shape:
				{
					Gravity::C_CapsuleDesc* capsuleDesc=(Gravity::C_CapsuleDesc*)pDesc;
					capsuleDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			
			case Collision::E_ImportCylinder_Shape:
				{
					Gravity::C_CylinderDesc* cylDesc=(Gravity::C_CylinderDesc*)pDesc;
					cylDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			case Collision::E_ImportComposite_Shape:
				{
					Gravity::C_CompositeDesc* compDesc=(Gravity::C_CompositeDesc*)pDesc;
					compDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			
			case Collision::E_ImportInstancedTriangleMesh_Shape:
				{
					Gravity::C_InstancedTriangleMeshDesc* insDesc=(Gravity::C_InstancedTriangleMeshDesc*)pDesc;
					//pInst->m_IneriaTensor = Maths::HMat33(Maths::ZERO_MATRIX);
					Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(Maths::HMat33(Maths::ZERO_MATRIX), &pInst->m_IneriaTensor);
					insDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			case Collision::E_ImportTriangleMeshBase_Shape:
				{
					Gravity::C_TriangleMeshBaseDesc* triDesc=(Gravity::C_TriangleMeshBaseDesc*)pDesc;
					//pInst->m_IneriaTensor = Maths::HMat33(Maths::ZERO_MATRIX);
					Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(Maths::HMat33(Maths::ZERO_MATRIX), &pInst->m_IneriaTensor);
					triDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			
			case Collision::E_ImportCloth_Shape:
				{
					Gravity::C_ClothDesc* clothDesc=(Gravity::C_ClothDesc*) pDesc;
					Maths::Store<Maths::HMat33, MathsStorage::Matrix33>(Maths::HMat33(Maths::ZERO_MATRIX), &pInst->m_IneriaTensor);
					clothDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;

				}
			case Collision::E_ImportCone_Shape:
				{
					Gravity::C_ConeDesc* coneDesc = (Gravity::C_ConeDesc*)pDesc;
					coneDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			case Collision::E_ImportFluidMesh_Shape:
				{
					Gravity::C_FluidMeshDesc* fluidMeshDesc = (Gravity::C_FluidMeshDesc*)pDesc;
					fluidMeshDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			case Collision::E_ImportDeformable_Shape:
				{
					Gravity::C_DeformableDesc* deformableDesc = (Gravity::C_DeformableDesc*)pDesc;
					deformableDesc->Export<Sys::BinarySerialiserAutoEndian>(serial);
					break;
				}
			default:
				{
					BIZ_ASSERT(0);
					return;
				}
			}

			pInst->Export<Sys::BinarySerialiserAutoEndian>(serial);
		}

		//serial << 0xffffffff; //The end of the file
	}

};
