#include "stdafx.h"
#include "gravity/sdk/BodyHelper.h"
#include "gravity/sdk/body.h"
#include "gravity/narrowphase/shapes.h"
#include "gravity/sdk/inertiatensorgenerator.h"

namespace Gravity
{

	void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass, C_Body* pBody, Deserialiser& serial, const Collision::C_CollisionFilter* pFilter, const U16 physicsMaterial, const U16 effectMaterial)
	{
		PERF_EVENT("BodyHelper.CreateBody");
		Gravity::C_ShapeInstance* pInst=new Gravity::C_ShapeInstance;
		pInst->m_pDesc=m_pShapeDesc;
		Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(serial, pFilter);
		pInst->Import(serial);
		pBody->SetRemapExportBodyId(m_pShapeDesc->m_RemapExportID);
		pBody->SetOriginalExportBodyId(m_pShapeDesc->m_OriginalExportID);
		CreateBody(m_pShapeDesc->m_Mass, pInst, pBody, pShape, m_pShapeDesc->GetNodeFlags());
		delete pInst;
	}


	void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, C_Body* pBody, Deserialiser& serial, const Collision::C_CollisionFilter* pFilter, const U16 physicsMaterial, const U16 effectMaterial)
	{
		PERF_EVENT("BodyHelper.CreateBody");
		Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(serial, pFilter);
		pBody->SetRemapExportBodyId(m_pShapeDesc->m_RemapExportID);
		pBody->SetOriginalExportBodyId(m_pShapeDesc->m_OriginalExportID);
		CreateBody(mass, position, orientation, pBody, pShape, m_pShapeDesc->GetNodeFlags());
	}
	//The method is for triangle mesh

	void C_BodyHelper::CreateBody(C_Body* pBody, const Collision::C_CollisionFilter* pFilter)
	{
		PERF_EVENT("BodyHelper.CreateBody");
		Maths::HF32 mass=m_pShapeDesc->m_Mass;
		Maths::HVec3 position(Maths::Constants<Maths::HVec3>::Zero());
		Maths::HMat33 orientation(Maths::Constants<Maths::HMat33>::Identity());
		Collision::C_Shape* pShape=m_pShapeDesc->CreateShape(pFilter);
		pBody->SetRemapExportBodyId(m_pShapeDesc->m_RemapExportID);
		pBody->SetOriginalExportBodyId(m_pShapeDesc->m_OriginalExportID);
		CreateBody(mass, position, orientation, pBody, pShape, 0);
	}



	void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, C_Body* pBody, const Collision::C_CollisionFilter* pFilter, const U16 physicsMaterial, const U16 effectMaterial)
	{
		PERF_EVENT("BodyHelper.CreateBody");
		Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(pFilter);
		/*pShape->SetPhysicsMaterialIndex( physicsMaterial);
		pShape->SetEffectMaterialIndex(effectMaterial);*/
		pBody->SetRemapExportBodyId(m_pShapeDesc->m_RemapExportID);
		pBody->SetOriginalExportBodyId(m_pShapeDesc->m_OriginalExportID);
		pBody->SetConstraintBodyId(m_pShapeDesc->m_ConstraintBodyID);
		CreateBody(mass, position, orientation, pBody, pShape, m_pShapeDesc->GetNodeFlags());
	}

	void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, C_Body* pBody, const Collision::C_CollisionFilter* pFilter)
	{
		PERF_EVENT("BodyHelper.CreateBody");
		Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(pFilter);
		pBody->SetRemapExportBodyId(m_pShapeDesc->m_RemapExportID);
		pBody->SetOriginalExportBodyId(m_pShapeDesc->m_OriginalExportID);
		pBody->SetConstraintBodyId(m_pShapeDesc->m_ConstraintBodyID);
		CreateBody(mass, position, orientation, pBody, pShape, m_pShapeDesc->GetNodeFlags());
	}



	void C_BodyHelper::CreateBody( const Maths::HVec3 PCREF position,const Maths::HMat33 PCREF orientation, C_Body* pBody, const Collision::C_CollisionFilter* pFilter)
	{
		PERF_EVENT("BodyHelper.CreateBody");
		Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(pFilter);
		/*pShape->SetPhysicsMaterialIndex( m_pShapeDesc->m_PhysicsMaterialIndex );
		pShape->SetEffectMaterialIndex( m_pShapeDesc->m_EffectMaterialIndex );*/
		pBody->SetRemapExportBodyId(m_pShapeDesc->m_RemapExportID);
		pBody->SetOriginalExportBodyId(m_pShapeDesc->m_OriginalExportID);
		pBody->SetConstraintBodyId(m_pShapeDesc->m_ConstraintBodyID);
		/*if(m_pShapeDesc->m_Type == Collision::E_Cone_Shape)
		{
			Collision::C_ConeShape* pCone = (Collision::C_ConeShape*)pShape;
			const Maths::HVec3 centerOfMass = position - pCone->m_Cone.m_OffSet * orientation;
			CreateBody(m_pShapeDesc->GetMass(), centerOfMass, orientation, pBody, pShape, m_pShapeDesc->GetNodeFlags());
			return;
		}*/

		CreateBody(m_pShapeDesc->GetMass(), position, orientation, pBody, pShape, m_pShapeDesc->GetNodeFlags());
	}


	void C_BodyHelper::CreateBody( const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position,const Maths::HMat33 PCREF orientation, const Maths::HMat33 PCREF ineriaTensor, C_Body* pBody, const Collision::C_CollisionFilter* pFilter)
	{
		PERF_EVENT("BodyHelper.CreateBody");
		Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(pFilter);
		/*pShape->SetPhysicsMaterialIndex( m_pShapeDesc->m_PhysicsMaterialIndex );
		pShape->SetEffectMaterialIndex( m_pShapeDesc->m_EffectMaterialIndex );*/
		pBody->SetRemapExportBodyId(m_pShapeDesc->m_RemapExportID);
		pBody->SetOriginalExportBodyId(m_pShapeDesc->m_OriginalExportID);
		pBody->SetConstraintBodyId(m_pShapeDesc->m_ConstraintBodyID);
		/*if(m_pShapeDesc->m_Type == Collision::E_Cone_Shape)
		{
			Collision::C_ConeShape* pCone = (Collision::C_ConeShape*)pShape;
			CreateBody(mass, position - pCone->m_Cone.m_OffSet * orientation, orientation, ineriaTensor, pBody, pShape, m_pShapeDesc->GetNodeFlags());
			return;
		}*/

		CreateBody(mass, position, orientation, ineriaTensor, pBody, pShape, m_pShapeDesc->GetNodeFlags());

	}

									//Private methods


	void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass,const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, const Maths::HMat33 PCREF ineriaTensor, C_Body* pBody, Collision::C_Shape* pShape, const U32 nodeFlags)
	{
		PERF_EVENT("BodyHelper.CreateBody2");
		using namespace Maths;

		if(pShape->HasVolume())
		{
			pShape->CalculateVolume();
		}
		pBody->SetShape(pShape);
		//pBody->UpdateShape(pShape);
		//Now we need to work out the inertia tensors...
		//HMat33 iBody(m_pShapeDesc->InertiaTensor(mass));
		//OK. Load up the rigid body and then synchronise the state!
		SharedPhysics::C_RigidBody& rigid(pBody->GetBody());
		rigid.m_IBody = ineriaTensor;
		rigid.m_IBodyInverse = C_InertiaTensorGenerator::InverseInertiaTensor(ineriaTensor);
		rigid.m_InertiaTensor = ineriaTensor;
		rigid.m_InverseInertiaTensor = rigid.m_IBodyInverse;

		Maths::HMat33 rot(orientation);
		rigid.m_Orientation = Maths::C_Quaternion(rot);
		rigid.m_OrientationMatrix = rot;
		rigid.m_Position = position;
		rigid.Mass(mass);
		rigid.ReciprocalMass(mass.Reciprocal());
		const HMat33 transRot(rot.Transposed());
		const HMat33 inverseInertiaTensor( transRot * rigid.m_IBodyInverse * rot );
		const HMat33 inertiaTensor( transRot * rigid.m_IBody * rot );
		rigid.m_InertiaTensor = inertiaTensor;
		rigid.m_InverseInertiaTensor = inverseInertiaTensor;
		pBody->Slop(m_pShapeDesc->GetSlop());
		pBody->SynchroniseState();
		pBody->SynchronisePosRot();

		if(!this->m_pShapeDesc->m_HasCollision)
		{
			pBody->GetObject()->m_ObjectState = Gravity::E_NoCollision;
		}

		if(nodeFlags & SharedPhysics::C_GraphNode::E_Type_Static)
		{
			pBody->MakeStatic();
			if(m_pShapeDesc->m_Type == Collision::E_TriangleMesh_Shape)
			{
				pBody->MakeStaticMesh();
				
			}
			else if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
			{
				pBody->MakeInstancedCollidableMesh();
				pBody->MakeStaticMesh();
			}
		}
		else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_KinematicObject)
		{
			pBody->MakeKinematic();
			if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
			{
				pBody->MakeInstancedCollidableMesh();
			}
		}
		else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_Sleeping)
		{
			pBody->Deactivate();
		}
		else
		{
			pBody->Activate();
		}
	}

	void C_BodyHelper::MakeFloat(const Maths::HF32 PCREF fluidDensity, const Maths::HF32 PCREF scale, const Maths::HF32 PCREF maxBuoyancy, C_Body* pBody,
	const Maths::HF32 PCREF linearDrag, const Maths::HF32 PCREF angularDrag)
	{
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());

		const Maths::HF32 mass = pBody->Mass();
		const Maths::HF32 volume = pBody->GetShape()->m_Volume;

		const Maths::HF32 density(mass/volume);

		const Maths::HF32 ratio(((density*scale)/fluidDensity));

		SharedPhysics::C_RigidBody& body = pBody->GetBody();
		body.BuoyancyCoefficient(Maths::Min(ratio, maxBuoyancy));
		body.FluidLinearDragCoefficient(linearDrag);
		body.FluidAngularDragCoefficient(angularDrag);
	}


	void C_BodyHelper::UpdateBodyShape(const Maths::HF32 PCREF mass, C_Body* pBody, const Collision::C_CollisionFilter* pFilter)
	{
		// Store element matrix, as this needs restoring if the centre of gravity changes with the new shape.
		//const Maths::HMat43 oldElementMat = pBody->ElementMatrix();

		Collision::C_CollisionFilter filter(*pFilter);
		//filter.MergeFlags(m_pShapeDesc->m_CollisionFlag);

		pBody->m_pShape->InvalidateShape();

		Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(pFilter);
		if(pShape->HasVolume())
		{
			pShape->CalculateVolume();
		}

		const Maths::HMat33 iBody = m_pShapeDesc->InertiaTensor(mass);

		//pShape->SetFilter(&filter);

		pBody->UpdateShape(pShape);
		//pBody->SetShape(pShape);

		SharedPhysics::C_RigidBody& rigid(pBody->GetBody());
		rigid.m_IBody = iBody;
		rigid.m_IBodyInverse = C_InertiaTensorGenerator::InverseInertiaTensor(iBody);
		rigid.m_InertiaTensor = iBody;
		rigid.m_InverseInertiaTensor = rigid.m_IBodyInverse;

		rigid.Mass(mass);
		rigid.ReciprocalMass(mass.Reciprocal());
		const Maths::HMat33 transRot(rigid.m_OrientationMatrix.Transposed());
		const Maths::HMat33 inverseInertiaTensor( transRot * rigid.m_IBodyInverse * rigid.m_OrientationMatrix );
		const Maths::HMat33 inertiaTensor( transRot * rigid.m_IBody * rigid.m_OrientationMatrix );
		rigid.m_InertiaTensor = inertiaTensor;
		rigid.m_InverseInertiaTensor = inverseInertiaTensor;
		rigid.m_LinearMomentum = rigid.m_LinearVelocity * mass;
		rigid.m_AngularMomentum = rigid.m_AngularVelocity * rigid.m_InertiaTensor;
		pBody->Slop(m_pShapeDesc->GetSlop());

		//pBody->ElementMatrix(oldElementMat);

		pBody->SynchroniseState();
		//pBody->SynchronisePosRot();
	}


	void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass,const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, C_Body* pBody, Collision::C_Shape* pShape, const U32 nodeFlags)
	{
		PERF_EVENT("BodyHelper.CreateBody2");
		using namespace Maths;

		pBody->SetShape(pShape);
		//Now we need to work out the inertia tensors...
		HMat33 iBody(m_pShapeDesc->InertiaTensor(mass));
		//OK. Load up the rigid body and then synchronise the state!
		SharedPhysics::C_RigidBody& rigid(pBody->GetBody());
		rigid.m_IBody = iBody;
		rigid.m_IBodyInverse = C_InertiaTensorGenerator::InverseInertiaTensor(iBody);
		rigid.m_InertiaTensor = iBody;
		rigid.m_InverseInertiaTensor = rigid.m_IBodyInverse;
		//Maths::HVec3 ax=orientation.AX();
		//Maths::HVec3 ay=orientation.AY();
		//Maths::HVec3 az=orientation.AZ();

		//Maths::HVec3 az2 = ax ^ ay;

		Maths::HMat33 rot(orientation);

		//if((az|az2)<Maths::Constants<Maths::HF32>::Zero())
		//{
		////	__debugbreak();
		//	rot = Maths::HMat33(ax, ay, -az);
		//
		//}

		rigid.m_Orientation = Maths::C_Quaternion(rot);
		rigid.m_OrientationMatrix = rot;
		rigid.m_Position = position;
		rigid.Mass(mass);
		rigid.ReciprocalMass(mass.Reciprocal());
		const HMat33 transRot(rot.Transposed());
		const HMat33 inverseInertiaTensor( transRot * rigid.m_IBodyInverse * rot );
		const HMat33 inertiaTensor( transRot * rigid.m_IBody * rot );
		rigid.m_InertiaTensor = inertiaTensor;
		rigid.m_InverseInertiaTensor = inverseInertiaTensor;
		pBody->Slop(m_pShapeDesc->GetSlop());
		//SharedPhysics::C_GraphNode* pNode(pBody->GetGraphNode());
		//pNode->m_pRigidBody = &rigid;
		pBody->SynchroniseState();
		pBody->SynchronisePosRot();
		if(pShape->HasVolume())
		{
			pShape->CalculateVolume();
		}

		/*if(nodeFlags & SharedPhysics::C_GraphNode::E_Type_Static)
		{
			pBody->MakeStatic();
			if(m_pShapeDesc->m_Type == Collision::E_TriangleMesh_Shape)
			{
				pBody->MakeStaticMesh();
				
			}
			else if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
			{
				pBody->MakeInstancedCollidableMesh();
				pBody->MakeStaticMesh();
			}
		}
		else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_KinematicObject)
		{
			pBody->MakeKinematic();
			if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
			{
				pBody->MakeInstancedCollidableMesh();
			}
		}
		else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_Sleeping)
		{
			pBody->Deactivate();
		}
		else
		{
			pBody->Activate();
		}*/
	}


	void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass, const Gravity::C_ShapeInstance* pInst, C_Body* pBody, Collision::C_Shape* pShape, const U32 nodeFlags)
	{
		PERF_EVENT("BodyHelper.CreateBody2");
		using namespace Maths;

		pBody->SetShape(pShape);
		//Now we need to work out the inertia tensors...
		HMat33 i2Body(m_pShapeDesc->InertiaTensor(mass));

		HMat33 iBody=Maths::Load<Maths::HMat33>(&pInst->m_IneriaTensor);
		//OK. Load up the rigid body and then synchronise the state!
		SharedPhysics::C_RigidBody& rigid(pBody->GetBody());
		rigid.m_IBody = iBody;
		rigid.m_IBodyInverse = C_InertiaTensorGenerator::InverseInertiaTensor(iBody);
		rigid.m_InertiaTensor = iBody;
		rigid.m_InverseInertiaTensor = rigid.m_IBodyInverse;
		//Maths::HVec3 ax=orientation.AX();
		//Maths::HVec3 ay=orientation.AY();
		//Maths::HVec3 az=orientation.AZ();

		//Maths::HVec3 az2 = ax ^ ay;

		Maths::HMat33 rot(Maths::Load<Maths::HMat33>(&pInst->m_Rotation));
		if(pShape->HasVolume())
		{				
			pShape->CalculateVolume();
		}

		//if((az|az2)<Maths::Constants<Maths::HF32>::Zero())
		//{
		////	__debugbreak();
		//	rot = Maths::HMat33(ax, ay, -az);
		//
		//}

		rigid.m_Orientation = Maths::C_Quaternion(rot);
		rigid.m_OrientationMatrix = rot;
		rigid.m_Position = pInst->m_Position;
		rigid.Mass(mass);
		rigid.ReciprocalMass(mass.Reciprocal());
		
		const Maths::HMat33 transRot(rot.Transposed());
		const Maths::HMat33 inverseInertiaTensor( transRot * rigid.m_IBodyInverse * rot );
		const Maths::HMat33 inertiaTensor( transRot * rigid.m_IBody * rot );
		rigid.m_InertiaTensor = inertiaTensor;
		rigid.m_InverseInertiaTensor = inverseInertiaTensor;
		
		pBody->Slop(m_pShapeDesc->GetSlop());
		//SharedPhysics::C_GraphNode* pNode(pBody->GetGraphNode());
		//pNode->m_pRigidBody = &rigid;
		pBody->SynchroniseState();
		pBody->SynchronisePosRot();

		/*if(nodeFlags & SharedPhysics::C_GraphNode::E_Type_Static)
		{
			pBody->MakeStatic();
			if(m_pShapeDesc->m_Type == Collision::E_TriangleMesh_Shape)
			{
				pBody->MakeStaticMesh();
				
			}
			else if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
			{
				pBody->MakeInstancedCollidableMesh();
				pBody->MakeStaticMesh();
			}
		}
		else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_KinematicObject)
		{
			pBody->MakeKinematic();
			if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
			{
				pBody->MakeInstancedCollidableMesh();
			}
		}
		else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_Sleeping)
		{
			pBody->Deactivate();
		}
		else
		{
			pBody->Activate();
		} 
		*/
	}
};
