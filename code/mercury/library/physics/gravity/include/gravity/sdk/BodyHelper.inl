#include "gravity/sdk/BodyHelper.h"

namespace Gravity
{


	//void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass, C_Body* pBody, Deserialiser& serial, Collision::C_CollisionFilter* pFilter, const U16 materialIndex)
	//{
	//	Gravity::C_ShapeInstance* pInst=new Gravity::C_ShapeInstance;
	//	pInst->m_pDesc=m_pShapeDesc;
	//	Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(serial, pFilter);
	//	pInst->Import(serial);
	//	pBody->SetBodyId(m_pShapeDesc->m_ID);
	//	CreateBody(m_pShapeDesc->m_Mass, pInst, pBody, pShape, m_pShapeDesc->GetNodeFlags());
	//	delete pInst;
	//}


	//void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
	//	const Maths::HMat33 PCREF orientation, C_Body* pBody, Deserialiser& serial, Collision::C_CollisionFilter* pFilter, const U16 materialIndex)
	//{
	//	Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(serial, pFilter);
	//	pBody->SetBodyId(m_pShapeDesc->m_ID);
	//	CreateBody(mass, position, orientation, pBody, pShape, m_pShapeDesc->GetNodeFlags());
	//}
	////The method is for triangle mesh

	//void C_BodyHelper::CreateBody(C_Body* pBody, Collision::C_CollisionFilter* pFilter)
	//{
	//	Maths::HF32 mass=Maths::Constants<Maths::HF32>::Zero();
	//	Maths::HVec3 position;
	//	Maths::HMat33 orientation;
	//	Collision::C_Shape* pShape=m_pShapeDesc->CreateShape(pFilter);
	//	pBody->SetBodyId(m_pShapeDesc->m_ID);
	//	CreateBody(mass, position, orientation, pBody, pShape, 0);
	//}



	//void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
	//	const Maths::HMat33 PCREF orientation, C_Body* pBody, Collision::C_CollisionFilter* pFilter, const U16 materialIndex)
	//{
	//	Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(pFilter);
	//	pShape->SetMaterialIndex( materialIndex );
	//	pBody->SetBodyId(m_pShapeDesc->m_ID);
	//	CreateBody(mass, position, orientation, pBody, pShape, m_pShapeDesc->GetNodeFlags());
	//}


	//void C_BodyHelper::CreateBody( const Maths::HVec3 PCREF position,const Maths::HMat33 PCREF orientation, C_Body* pBody, Collision::C_CollisionFilter* pFilter)
	//{
	//	Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(pFilter);
	//	pShape->SetMaterialIndex( m_pShapeDesc->m_MaterialIndex );
	//	pBody->SetBodyId(m_pShapeDesc->m_ID);
	//	CreateBody(m_pShapeDesc->GetMass(), position, orientation, pBody, pShape, m_pShapeDesc->GetNodeFlags());
	//}


	//void C_BodyHelper::CreateBody( const Maths::HVec3 PCREF position,const Maths::HMat33 PCREF orientation, const Maths::HMat33 PCREF ineriaTensor, C_Body* pBody, Collision::C_CollisionFilter* pFilter)
	//{
	//	Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(pFilter);
	//	pShape->SetMaterialIndex( m_pShapeDesc->m_MaterialIndex );
	//	pBody->SetBodyId(m_pShapeDesc->m_ID);
	//	CreateBody(m_pShapeDesc->GetMass(), position, orientation, ineriaTensor, pBody, pShape, m_pShapeDesc->GetNodeFlags());
	//}

	//								//Private methods


	//void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass,const Maths::HVec3 PCREF position, 
	//	const Maths::HMat33 PCREF orientation, const Maths::HMat33 PCREF ineriaTensor, C_Body* pBody, Collision::C_Shape* pShape, const U32 nodeFlags)
	//{
	//	using namespace Maths;

	//	pBody->SetShape(pShape);
	//	//Now we need to work out the inertia tensors...
	//	//HMat33 iBody(m_pShapeDesc->InertiaTensor(mass));
	//	//OK. Load up the rigid body and then synchronise the state!
	//	SharedPhysics::C_RigidBody& rigid(pBody->GetBody());
	//	rigid.m_IBody = ineriaTensor;
	//	rigid.m_IBodyInverse = ineriaTensor.Inverse();

	//	Maths::HMat33 rot(orientation);
	//	rigid.m_Orientation = Maths::C_Quaternion(rot);
	//	rigid.m_Position = position;
	//	rigid.m_Mass = mass;
	//	rigid.m_ReciprocalMass = mass.Reciprocal();
	//	const HMat33 transRot(rot.Transposed());
	//	const HMat33 inverseInertiaTensor( transRot * rigid.m_IBodyInverse * rot );
	//	const HMat33 inertiaTensor( transRot * rigid.m_IBody * rot );
	//	rigid.m_InertiaTensor = inertiaTensor;
	//	rigid.m_InverseInertiaTensor = inverseInertiaTensor;
	//	pBody->Slop(m_pShapeDesc->GetSlop());
	//	pBody->SynchroniseState();
	//	pBody->SynchronisePosRot();

	//	if(nodeFlags & SharedPhysics::C_GraphNode::E_Type_Static)
	//	{
	//		pBody->MakeStatic();
	//		if(m_pShapeDesc->m_Type == Collision::E_TriangleMesh_Shape)
	//		{
	//			pBody->MakeStaticMesh();
	//			
	//		}
	//		else if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
	//		{
	//			pBody->MakeInstancedCollidableMesh();
	//			pBody->MakeStaticMesh();
	//		}
	//	}
	//	else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_KinematicObject)
	//	{
	//		pBody->MakeKinematic();
	//		if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
	//		{
	//			pBody->MakeInstancedCollidableMesh();
	//		}
	//	}
	//	else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_Sleeping)
	//	{
	//		pBody->Deactivate();
	//	}
	//	else
	//	{
	//		pBody->Activate();
	//	}
	//}


	//void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass,const Maths::HVec3 PCREF position, 
	//	const Maths::HMat33 PCREF orientation, C_Body* pBody, Collision::C_Shape* pShape, const U32 nodeFlags)
	//{
	//	using namespace Maths;

	//	pBody->SetShape(pShape);
	//	//Now we need to work out the inertia tensors...
	//	HMat33 iBody(m_pShapeDesc->InertiaTensor(mass));
	//	//OK. Load up the rigid body and then synchronise the state!
	//	SharedPhysics::C_RigidBody& rigid(pBody->GetBody());
	//	rigid.m_IBody = iBody;
	//	rigid.m_IBodyInverse = iBody.Inverse();
	//	//Maths::HVec3 ax=orientation.AX();
	//	//Maths::HVec3 ay=orientation.AY();
	//	//Maths::HVec3 az=orientation.AZ();

	//	//Maths::HVec3 az2 = ax ^ ay;

	//	Maths::HMat33 rot(orientation);

	//	//if((az|az2)<Maths::Constants<Maths::HF32>::Zero())
	//	//{
	//	////	__debugbreak();
	//	//	rot = Maths::HMat33(ax, ay, -az);
	//	//
	//	//}

	//	rigid.m_Orientation = Maths::C_Quaternion(rot);
	//	rigid.m_Position = position;
	//	rigid.m_Mass = mass;
	//	rigid.m_ReciprocalMass = mass.Reciprocal();
	//	const HMat33 transRot(rot.Transposed());
	//	const HMat33 inverseInertiaTensor( transRot * rigid.m_IBodyInverse * rot );
	//	const HMat33 inertiaTensor( transRot * rigid.m_IBody * rot );
	//	rigid.m_InertiaTensor = inertiaTensor;
	//	rigid.m_InverseInertiaTensor = inverseInertiaTensor;
	//	pBody->Slop(m_pShapeDesc->GetSlop());
	//	//SharedPhysics::C_GraphNode* pNode(pBody->GetGraphNode());
	//	//pNode->m_pRigidBody = &rigid;
	//	pBody->SynchroniseState();
	//	pBody->SynchronisePosRot();

	//	if(nodeFlags & SharedPhysics::C_GraphNode::E_Type_Static)
	//	{
	//		pBody->MakeStatic();
	//		if(m_pShapeDesc->m_Type == Collision::E_TriangleMesh_Shape)
	//		{
	//			pBody->MakeStaticMesh();
	//			
	//		}
	//		else if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
	//		{
	//			pBody->MakeInstancedCollidableMesh();
	//			pBody->MakeStaticMesh();
	//		}
	//	}
	//	else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_KinematicObject)
	//	{
	//		pBody->MakeKinematic();
	//		if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
	//		{
	//			pBody->MakeInstancedCollidableMesh();
	//		}
	//	}
	//	else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_Sleeping)
	//	{
	//		pBody->Deactivate();
	//	}
	//	else
	//	{
	//		pBody->Activate();
	//	}
	//}


	//void C_BodyHelper::CreateBody(const Maths::HF32 PCREF mass, const Gravity::C_ShapeInstance* pInst, C_Body* pBody, Collision::C_Shape* pShape, const U32 nodeFlags)
	//{
	//	using namespace Maths;

	//	pBody->SetShape(pShape);
	//	//Now we need to work out the inertia tensors...
	//	HMat33 i2Body(m_pShapeDesc->InertiaTensor(mass));

	//	HMat33 iBody=Maths::Load<Maths::HMat33>(&pInst->m_IneriaTensor);
	//	//OK. Load up the rigid body and then synchronise the state!
	//	SharedPhysics::C_RigidBody& rigid(pBody->GetBody());
	//	rigid.m_IBody = iBody;
	//	rigid.m_IBodyInverse = iBody.Inverse();
	//	//Maths::HVec3 ax=orientation.AX();
	//	//Maths::HVec3 ay=orientation.AY();
	//	//Maths::HVec3 az=orientation.AZ();

	//	//Maths::HVec3 az2 = ax ^ ay;

	//	Maths::HMat33 rot(Maths::Load<Maths::HMat33>(&pInst->m_Rotation));

	//	//if((az|az2)<Maths::Constants<Maths::HF32>::Zero())
	//	//{
	//	////	__debugbreak();
	//	//	rot = Maths::HMat33(ax, ay, -az);
	//	//
	//	//}

	//	rigid.m_Orientation = Maths::C_Quaternion(rot);
	//	rigid.m_Position = pInst->m_Position;
	//	rigid.m_Mass = mass;
	//	rigid.m_ReciprocalMass = mass.Reciprocal();
	//	
	//	const Maths::HMat33 transRot(rot.Transposed());
	//	const Maths::HMat33 inverseInertiaTensor( transRot * rigid.m_IBodyInverse * rot );
	//	const Maths::HMat33 inertiaTensor( transRot * rigid.m_IBody * rot );
	//	rigid.m_InertiaTensor = inertiaTensor;
	//	rigid.m_InverseInertiaTensor = inverseInertiaTensor;
	//	
	//	pBody->Slop(m_pShapeDesc->GetSlop());
	//	//SharedPhysics::C_GraphNode* pNode(pBody->GetGraphNode());
	//	//pNode->m_pRigidBody = &rigid;
	//	pBody->SynchroniseState();
	//	pBody->SynchronisePosRot();

	//	if(nodeFlags & SharedPhysics::C_GraphNode::E_Type_Static)
	//	{
	//		pBody->MakeStatic();
	//		if(m_pShapeDesc->m_Type == Collision::E_TriangleMesh_Shape)
	//		{
	//			pBody->MakeStaticMesh();
	//			
	//		}
	//		else if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
	//		{
	//			pBody->MakeInstancedCollidableMesh();
	//			pBody->MakeStaticMesh();
	//		}
	//	}
	//	else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_KinematicObject)
	//	{
	//		pBody->MakeKinematic();
	//		if (m_pShapeDesc->m_Type == Collision::E_InstancedTriangleMesh_Shape)
	//		{
	//			pBody->MakeInstancedCollidableMesh();
	//		}
	//	}
	//	else if (nodeFlags & SharedPhysics::C_GraphNode::E_Type_Sleeping)
	//	{
	//		pBody->Deactivate();
	//	}
	//	else
	//	{
	//		pBody->Activate();
	//	}
	//}

};

