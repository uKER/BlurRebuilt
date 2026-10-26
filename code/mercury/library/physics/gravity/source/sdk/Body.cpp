#include "StdAfx.h"
#include "gravity/sdk/Body.h"
#include "gravity/sdk/Scene.h"
#include "gravity/sdk/PhysicsSDK.h"
#include "gravity/broadphase/Object.inl"
#include "gravity/sharedphysics/RigidBody.inl"
//#include "gravity/sharedphysics/Integrator.inl"
#include "gravity/sdk/Action.h"
#include "gravity/narrowphase/Shape.h"
#include "gravity/broadphase/distributiontree.inl"
#include "gravity/sdk/inertiatensorgenerator.h"
#include "system/perf.h"

namespace Gravity
{

	C_Body::C_Body(Collision::C_Shape* pShape, SharedPhysics::C_RigidBody& body, const Maths::HF32 PCREF sleepThreshold, const Maths::HF32 PCREF wakeupThreshold, C_Scene* scene,
		const U32 index) : m_Object(index, pShape, &m_GraphNode), m_RigidBody(body), m_pShape(pShape), m_pFutureShape(pShape), m_pOwningScene(scene), m_pAction(NULL),
				m_pExternalConstants(NULL), m_pIntegrationCallback(NULL), m_pUsrData(NULL), m_BodyFlags(E_Type_Apply_Gravity),
				m_pPrev(NULL), m_ContinuousThreshold(Maths::Constants<Maths::HF32>::Zero())
	{
		m_GraphNode.m_Index = index;
		m_GraphNode.m_pBody = this;
		m_GraphNode.m_pRigidBody = &m_RigidBody;
		m_Object.m_pGraphNode = &m_GraphNode;
		SynchroniseState();
		SynchroniseStateFlags();
	}

	C_Body::C_Body() : m_Object(0, NULL, &m_GraphNode), m_pShape(NULL), m_pOwningScene(NULL), m_pAction(NULL), m_pExternalConstants(NULL), m_pIntegrationCallback(NULL), m_pUsrData(NULL), m_BodyFlags(E_Type_Apply_Gravity),
		m_pPrev(NULL), m_ContinuousThreshold(Maths::Constants<Maths::HF32>::Zero())
	{
		m_GraphNode.m_pBody = this;
		m_GraphNode.m_pRigidBody = &m_RigidBody;
		m_Object.m_pGraphNode = &m_GraphNode;
		SynchroniseState();
		SynchroniseStateFlags();
	}

#ifndef PLATFORM_SPU

	C_Body::~C_Body()
	{
		//For now, C_Shape is the only thing which isn't recycled!!!! I'll work out a way to do it later!
		if(m_pShape)
		{
			if(m_pFutureShape != m_pShape)
			{
				delete m_pShape;
				delete m_pFutureShape;
			}
			else
			{
				delete m_pShape;
			}
		}
			
		
		m_pShape = NULL;
		m_pFutureShape = NULL;
		m_pOwningScene = NULL;
		m_pAction = NULL;
		m_pExternalConstants = NULL;
		m_pIntegrationCallback = NULL; 
		m_pUsrData = NULL;
	}

	void C_Body::SetIndex(U32 index)
	{
		m_GraphNode.m_Index = index;
		m_Object.m_Index = index;
	}
#endif

	bool C_Body::MatchExternalState()
	{
		return (m_GraphNode.m_NodeFlags & SharedPhysics::C_GraphNode::E_Type_BodyStateMask) == m_ExternalState.m_ExternalBodyState;
	}
	
	C_Body* C_Body::GetBody(C_Object* pObject)
	{
		return (C_Body*)(((U8*)pObject) - offsetof(C_Body, m_Object));
	}

	C_Body* C_Body::GetBody(SharedPhysics::C_GraphNode* pGraphNode)
	{
		return (C_Body*)((U8*)pGraphNode - offsetof(C_Body, m_GraphNode));
	}

	C_Body* C_Body::GetBody(SharedPhysics::C_RigidBody* pRigid)
	{
		return (C_Body*)((U8*)pRigid - offsetof(C_Body, m_RigidBody));
	}

#ifndef PLATFORM_SPU
	void C_Body::GetConstraints(Btl::Vector<SharedPhysics::C_Constraint*>& resultVector)
	{
		SharedPhysics::C_ConstraintEdge* pEdge = m_GraphNode.m_pPersistentConstraints;
		while(pEdge)
		{
			SharedPhysics::C_ConstraintPoint* pPoint(pEdge->m_pFirstConstraint);
			while(pPoint)
			{
				resultVector.PushBack(pPoint->m_pConstraint);
				pPoint = pPoint->m_pNext;
			}
			pEdge = pEdge->m_pNext;
		}

		SharedPhysics::C_ConstraintTwin* pTwin = m_GraphNode.m_pPersistentTwinConstraints;
		while(pTwin)
		{
			//pEdge = pTwin->m_pEdge;
			pEdge = SharedPhysics::C_ConstraintEdge::GetConstraintEdge(pTwin);
			SharedPhysics::C_ConstraintPoint* pPoint(pEdge->m_pFirstConstraint);
			while(pPoint)
			{
				resultVector.PushBack(pPoint->m_pConstraint);
				pPoint = pPoint->m_pNext;
			}
			pTwin = pTwin->m_pNext;
		}
	}

	void C_Body::SetCentreOfMassOffset(const Maths::HVec3 PCREF offset)
	{
		if (m_pOwningScene)
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_SetCentreOfMassOffset;
			pCommand->m_CommandBundle.m_pBody = this;
			pCommand->m_CommandBundle.SetValue(offset);
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			m_RigidBody.m_CenterOfMassOffset = offset;
		}
	}
#endif
	
	void C_Body::SetMaxLinearVelocity(const Maths::HF32 PCREF maxLinearSpeed)
	{
		m_RigidBody.MaxLinearSpeed(maxLinearSpeed);
	}
	
	void C_Body::SetMaxAngularVelocity(const Maths::HF32 PCREF maxAngularSpeed)
	{
		m_RigidBody.MaxAngularSpeed( maxAngularSpeed );
	}

	Maths::HVec3 C_Body::PointVelocity(const Maths::HVec3 PCREF worldPos) const
	{
		return m_RigidBody.PointVelocity(worldPos);
	}

	/*const SharedPhysics::C_GraphNode* C_Body::GetGraphNode() const
	{
		return &m_GraphNode;
	}*/

	//SharedPhysics::C_GraphNode* C_Body::GetGraphNode()
	//{
	//	return &m_GraphNode;
	//}

	C_IntegrationCallback* C_Body::GetIntegrationCallback()
	{
		return m_pIntegrationCallback;
	}

	const Maths::HF32 C_Body::Mass() const
	{
		return m_RigidBody.Mass();
	}

	const Maths::HF32 C_Body::ReciprocalMass() const
	{
		return m_RigidBody.ReciprocalMass();
	}

	const Maths::HMat33 C_Body::InertiaTensor() const
	{
		return m_RigidBody.m_InertiaTensor;
	}

	S32 C_Body::GetRemapExportBodyId() const
	{
		return m_RemapExportId;
	}

	S32 C_Body::GetOriginalExportBodyId() const
	{
		return m_OriginalExportId;
	}

	void C_Body::SetConstraintBodyId(const S32 id)
	{
		m_ConstraintBodyId = id;
	}

	S32 C_Body::GetConstraintBodyId() const
	{
		return m_ConstraintBodyId;
	}

	Gravity::C_Object* C_Body::GetObject()
	{
		return &m_Object;
	}

	const Gravity::C_Object* const C_Body::GetObject() const
	{
		return &m_Object;
	}


	void C_Body::SynchroniseState()
	{
		//Copy positions etc. into the external state 
		m_RigidBody.ClampLinearAndAngularVelocity(m_RigidBody.MaxLinearSpeed(), m_RigidBody.MaxAngularSpeed());
		m_ExternalState.m_Position = m_RigidBody.m_Position;
		m_ExternalState.m_Orientation = m_RigidBody.m_OrientationMatrix;
		m_ExternalState.m_LinearVelocity = m_RigidBody.m_LinearVelocity;
		m_ExternalState.m_AngularVelocity = m_RigidBody.m_AngularVelocity;
		
		m_RigidBody.m_CenterOfMass = m_RigidBody.m_Position + m_RigidBody.m_CenterOfMassOffset * m_RigidBody.m_OrientationMatrix;
	}

	void C_Body::SynchroniseStateFlags()
	{
		m_ExternalState.m_ExternalBodyState = m_GraphNode.m_NodeFlags & (SharedPhysics::C_GraphNode::E_Type_BodyStateMask);
		m_ExternalState.m_HasStateChanged = false;
	}

	bool C_Body::HasExternalStateChanged()
	{
		bool bResult = m_ExternalState.m_HasStateChanged;
		m_ExternalState.m_HasStateChanged = true;
		return bResult;
	}


	void C_Body::SynchronisePosRot()
	{
		m_Object.Transform(Maths::HMat43(m_RigidBody.m_OrientationMatrix, m_RigidBody.m_Position));
	}

	void C_Body::ClearEdgeFlags()
	{
		m_GraphNode.m_TotalKineticEnergy = m_RigidBody.m_KineticEnergy;
		m_GraphNode.m_IslandIndex = -1;

		SharedPhysics::C_ConstraintEdge* pEdge(m_GraphNode.m_pPersistentConstraints);
		while(pEdge)
		{
			Spu::MemBaseMarker marker;
			SharedPhysics::C_ConstraintEdge* pSpuEdge = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAlloc(pEdge, sizeof(SharedPhysics::C_ConstraintEdge));
			Gravity::C_Body* pPpuBody = Gravity::C_Body::GetBody(pSpuEdge->m_pOutboundNode);
			Gravity::C_Body* pSpuBody = (C_Body*)Spu::FetchAlloc(pPpuBody, sizeof(C_Body));
			if(pSpuBody->TestBodyFlags(C_Body::E_InsertedIntoWorld))
			{
				pSpuEdge->ClearEdgeFlags(SharedPhysics::C_ConstraintEdge::E_Visited_Island_Processing | SharedPhysics::C_ConstraintEdge::E_Visited_Island_Processing2);
#ifdef PLATFORM_SPU
				Spu::Store(pEdge, pSpuEdge, sizeof(SharedPhysics::C_ConstraintEdge));
#endif
			}
			pEdge = pSpuEdge->m_pNext;
		}

		SharedPhysics::C_ConstraintTwin* pTwin(m_GraphNode.m_pPersistentTwinConstraints);
		while(pTwin)
		{
			/*SharedPhysics::C_ConstraintTwin* pSpuTwin = (SharedPhysics::C_ConstraintTwin*)Spu::FetchAllocUnaligned(pTwin, sizeof(SharedPhysics::C_ConstraintTwin));
			SharedPhysics::C_ConstraintEdge* pSpuEdge = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAlloc(pSpuTwin->m_pEdge, sizeof(SharedPhysics::C_ConstraintEdge));
			*/
			SharedPhysics::C_ConstraintEdge* pEdge = SharedPhysics::C_ConstraintEdge::GetConstraintEdge(pTwin);
			SharedPhysics::C_ConstraintEdge* pSpuEdge = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAlloc(pEdge, sizeof(SharedPhysics::C_ConstraintEdge));
			SharedPhysics::C_ConstraintTwin* pSpuTwin = &pSpuEdge->m_TwinConstraint;
			
			Gravity::C_Body* pPpuBody = Gravity::C_Body::GetBody(pSpuEdge->m_pInboundNode);
			Gravity::C_Body* pSpuBody = (C_Body*)Spu::FetchAlloc(pPpuBody, sizeof(C_Body));
			if(pSpuBody->TestBodyFlags(C_Body::E_InsertedIntoWorld) && pSpuBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
			{
				pSpuEdge->ClearEdgeFlags(SharedPhysics::C_ConstraintEdge::E_Visited_Island_Processing | SharedPhysics::C_ConstraintEdge::E_Visited_Island_Processing2);
#ifdef PLATFORM_SPU
				Spu::Store(pEdge, pSpuEdge, sizeof(SharedPhysics::C_ConstraintEdge));
#endif
			}
			pTwin = pSpuTwin->m_pNext;
		}
	}

	void C_Body::LoadExternalForces()
	{
		Spu::MemBaseMarker marker;
		//Simple for now - copy across external force/torque from external buffer and constants (gravity)
		m_RigidBody.m_Force = m_ExternalState.m_Force;
		m_RigidBody.m_Torque = m_ExternalState.m_Torque;
		if(m_pExternalConstants && TestBodyFlags(C_Body::E_Type_Apply_Gravity))
		{
			C_ExternalConstants* pConstants = (C_ExternalConstants*)Spu::FetchAlloc(m_pExternalConstants, sizeof(C_ExternalConstants));
			m_RigidBody.m_Force += (pConstants->m_Gravity * m_RigidBody.Mass());
		}
	}



	void C_Body::Integrate(const SharedPhysics::C_Integrator & integrator, const Maths::C_TimeS PCREF time)
	{
		//Note - if we have actions, we are potentially subdividing the integration step into baby steps. However, 
		//integration should be really cheap so it shouldn't matter too much!

		//if(m_pAction == NULL)
		{
			//No actions so simple code first...
			if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_RK4Integrate))
			{
				m_RigidBody.RK4Integrate(integrator, time);
			}
			else
			{
				m_RigidBody.EulerIntegrate(integrator, time);
			}
		}
		//else
		//{
		//	const Maths::C_TimeS zero(Maths::Constants<Maths::HF32>::Zero());
		//	//Loop round the time step and integrate
		//	if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_RK4Integrate))
		//	{
		//		Maths::C_TimeS remainingTime(time);
		//		while(remainingTime > zero)
		//		{
		//			Maths::C_TimeS timeStep(m_pAction->Apply(remainingTime.m_Scalar));
		//			m_RigidBody.RK4Integrate(integrator, timeStep);
		//			remainingTime -= timeStep;
		//		}
		//	}
		//	else
		//	{
		//		Maths::C_TimeS remainingTime(time);
		//		while(remainingTime > zero)
		//		{
		//			Maths::C_TimeS timeStep(m_pAction->Apply(remainingTime.m_Scalar));
		//			m_RigidBody.EulerIntegrate(integrator, time);
		//			remainingTime -= timeStep;
		//		}
		//	}

		//}
	}

	/*bool C_Body::TestBodyFlags(U32 bodyFlags)
	{
		return (m_BodyFlags & bodyFlags) != 0;
	}*/


	Maths::HMat33 C_Body::Orientation() const
	{
		return m_ExternalState.m_Orientation;
	}

	Maths::C_Quaternion C_Body::GetQuaternion() const
	{
		return m_RigidBody.m_Orientation;
	}

	Maths::HMat43 C_Body::ElementMatrix() const
	{
		//return m_pShape->GetElementMatrix();
		const Maths::HMat33 rotation(m_ExternalState.m_Orientation);
		Maths::HVec3 newPos((-m_ElementOffset) * m_ExternalState.m_Orientation + m_ExternalState.m_Position);
		return Maths::HMat43(rotation, newPos);
	}

	Maths::HMat43 C_Body::InternalElementMatrix() const
	{
		//return m_pShape->GetElementMatrix();
		const Maths::HMat33 rotation(m_RigidBody.m_Orientation);
		Maths::HVec3 newPos((-m_ElementOffset) * m_RigidBody.m_Orientation + m_RigidBody.m_Position);
		return Maths::HMat43(rotation, newPos);
	}

	void C_Body::ElementMatrix(const Maths::HMat43 PCREF matrix)
	{
		Maths::HVec3 newPos(m_ElementOffset*matrix);
		///Maths::C_Quaternion quat(matrix.Rotation());
#ifndef PLATFORM_SPU
		Orientation(matrix.Rotation());
		Position(newPos);
#else
		m_ExternalState.m_Orientation = matrix.Rotation();
		m_ExternalState.m_Position = newPos;
		m_RigidBody.StoreVelocitiesAndPosition(m_RigidBody.m_LinearVelocity,
			m_RigidBody.m_AngularVelocity, m_ExternalState.m_Orientation, m_ExternalState.m_Position);
#endif
	}

	Maths::HVec3 C_Body::Position() const
	{
		return m_ExternalState.m_Position;
	}

	Maths::HVec3 C_Body::CenterOfRotation() const
	{
		return m_ExternalState.m_Position + (m_RigidBody.m_CenterOfMassOffset * m_ExternalState.m_Orientation);
	}


#ifndef PLATFORM_SPU

	void C_Body::MakeFrameOfReference()
	{
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_FrameOfReference);
	}

	void C_Body::UnMakeFrameOfReference()
	{
		m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_FrameOfReference);
	}


	void C_Body::MakeOneWayClothObject()
	{
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_OneWayClothCollision);
	}

	void C_Body::ClearOneWayClothObject()
	{
		m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_OneWayClothCollision);
	}

	void C_Body::SetMass(const Maths::HF32 PCREF mass)
	{
		const Maths::HF32 scale = mass * m_RigidBody.ReciprocalMass();

		m_RigidBody.m_IBody = m_RigidBody.m_IBody * scale;
		m_RigidBody.m_IBodyInverse = C_InertiaTensorGenerator::InverseInertiaTensor(m_RigidBody.m_IBody);
		m_RigidBody.m_InertiaTensor = m_RigidBody.m_IBody;
		m_RigidBody.m_InverseInertiaTensor = m_RigidBody.m_IBodyInverse;
		m_RigidBody.Mass(mass);
		m_RigidBody.ReciprocalMass(mass.Reciprocal());
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_State_Changed);

	}

	void C_Body::Slop(const F32 slop)
	{
		m_GraphNode.Slop(slop);
	}

	void C_Body::Baumgarte(const F32 baumgarte)
	{
		m_GraphNode.Baumgarte(baumgarte);
	}

	void C_Body::FlagInsertedIntoWorld()
	{
		m_BodyFlags |= E_InsertedIntoWorld;
		m_BodyFlags &= ~E_TemporarilyRemoved;
	}

	void C_Body::DisableDefaultIntegrationStep()
	{
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoIntegrate);
	}

	void C_Body::EnableDefaultIntegrationStep()
	{
		m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoIntegrate);
	}

	void C_Body::SetIntegrationCallback(C_IntegrationCallback* pCallback)
	{
		m_pIntegrationCallback = pCallback;
	}
	
	Maths::HVec3 C_Body::GetGravity()
	{
		return m_pExternalConstants->m_Gravity;
	}

	void C_Body::SetExternalConstants(C_ExternalConstants* pExternalConstants)
	{
		m_pExternalConstants = pExternalConstants;
	}

	void C_Body::SetDeactivator(SharedPhysics::C_Deactivator& deactivator)
	{
		m_GraphNode.SetDeactivator(deactivator);
	}

	void C_Body::SetRemapExportBodyId(const S32 id)
	{
		m_RemapExportId=id;
	}

	void C_Body::SetOriginalExportBodyId(const S32 id)
	{
		m_OriginalExportId = id;
	}

	const SharedPhysics::C_GraphNode* C_Body::GraphNode() const __restrict
	{
		return &m_GraphNode;
	}

	void C_Body::SetBody(SharedPhysics::C_RigidBody& body)
	{
		m_RigidBody = body;
		SynchroniseState();
	}

	void C_Body::SetLinearDampingCoefficient(const Maths::HF32 PCREF damping)
	{
		m_RigidBody.LinearDamping(damping);
	}
		
	void C_Body::SetAngularDampingCoefficient(const Maths::HF32 PCREF damping)
	{
		m_RigidBody.AngularDamping(damping);
	}

	void C_Body::SetShape(Collision::C_Shape* pShape)
	{
		if(m_pShape)
			delete m_pShape; //? Nothing can share a shape, so can't cause problems...can it?
		m_pShape = pShape;
		m_pFutureShape = pShape;
		m_Object.m_pShape = pShape;
		m_ElementOffset = m_pShape->m_ElementOffset;
	}

	void C_Body::SetScene(C_Scene* pScene)
	{
		m_pOwningScene = pScene;
	}

	C_Scene* C_Body::GetScene() const
	{
		return m_pOwningScene;
	}

	bool C_Body::IsActive() const
	{
		return m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active);
	}

	bool C_Body::IsDynamic() const
	{
		return m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);
	}

	bool C_Body::IsExternalActive() const
	{
		return (m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_Active) != 0;
	}

	bool C_Body::IsExternalSleeping() const
	{
		return (m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_Sleeping) != 0;
	}

	bool C_Body::IsExternalStatic() const
	{
		return (m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_Static) != 0;
	}

	bool C_Body::IsExternalKinematic() const
	{
		return (m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_KinematicObject) != 0;
	}

	bool C_Body::IsExternalDebris() const
	{
		return (m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_Debris) != 0;
	}

	bool C_Body::IsExternalRaytraceOnly() const
	{
		return (m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_RaytraceOnly) != 0;
	}


	bool C_Body::MustSolve()
		{
#if 1 //def GENERATE_ISLANDS
			return m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node);
#else
			return m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active) && 
				!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Visited_Island_Processing);
#endif
		}

	void C_Body::SetContinuousSpeedThreshold(const Maths::HF32 PCREF speed)
	{
		//const Maths::HF32 continuousThreshold(Maths::Constants<Maths::HF32>::Half() * m_RigidBody.m_Mass * speed * speed);
		//m_ContinuousThreshold = continuousThreshold * continuousThreshold;
		m_ContinuousThreshold = speed * speed;
	}

	bool C_Body::IsContinuousShape() const
	{
		return m_pShape->GetShape() == Collision::E_Continuous_Shape;
	}

	bool C_Body::IsContinuous() const
	{
		/*const Maths::HF32 kinEngSq(m_RigidBody.m_KineticEnergy | m_RigidBody.m_KineticEnergy);
		return kinEngSq >= m_ContinuousThreshold;*/
		const Maths::HF32 speedSq(m_RigidBody.m_LinearVelocity | m_RigidBody.m_LinearVelocity);
		return speedSq >= m_ContinuousThreshold;
	}

	void C_Body::SetBodyFlags(U32 bodyFlag)
	{
		m_BodyFlags |= bodyFlag;
	}

	void C_Body::ClearBodyFlags(U32 bodyFlag)
	{
		m_BodyFlags &= (~bodyFlag);
	}

	void C_Body::SetUserData(void* data)
	{
		m_pUsrData = data;
	}

	void C_Body::SetAction(C_Action* pAction)
	{
		m_pAction = pAction;
	}

	bool C_Body::operator < (const C_Body& other) const
	{
		return m_GraphNode < other.m_GraphNode;
	}

	bool C_Body::operator <= (const C_Body& other) const
	{
		return m_GraphNode <= other.m_GraphNode;
	}

	bool C_Body::operator > (const C_Body& other) const
	{
		return m_GraphNode > other.m_GraphNode;
	}

	bool C_Body::operator >= (const C_Body& other) const
	{
		return m_GraphNode >= other.m_GraphNode;
	}

	bool C_Body::operator == (const C_Body& other) const
	{
		return m_GraphNode == other.m_GraphNode;
	}
	
	bool C_Body::operator != (const C_Body& other) const
	{
		return m_GraphNode != other.m_GraphNode;
	}


	void C_Body::WakeUpBroadPhase()
	{
		C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
		pCommand->m_CommandType = C_DeferredBodyCommand::E_BroadPhaseWakeUp;
		pCommand->m_CommandBundle.m_pBody = this;
		m_pOwningScene->AddCommand(pCommand);
	}
	void C_Body::DoWakeUpBroadPhase(Gravity::C_DistributionTree* tree)
	{
		m_Object.WakeUp(tree);
	}

	void C_Body::ApplyImpulse(const Maths::HVec3 PCREF impulse, const Maths::HVec3 PCREF position)
	{
		BIZ_ASSERT(impulse.Length() <= 1000.f * m_RigidBody.Mass());
		BIZ_ASSERT(impulse.Finite());
		if(!impulse.Finite())
			return;
		if(m_pOwningScene)//TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ApplyImpulse;
			pCommand->m_CommandBundle.SetValue(impulse);
			pCommand->m_CommandBundle.SetPosition(m_RigidBody.m_OrientationMatrix * (position - m_RigidBody.m_Position));
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			m_RigidBody.ApplyImpulseWorldSpace(impulse, position);
		}
	}

	void C_Body::ApplyImpulseLocalSpace(const Maths::HVec3 PCREF impulse, const Maths::HVec3 PCREF position)
	{
		BIZ_ASSERT(impulse.Length() <= 1000.f * m_RigidBody.Mass());
		BIZ_ASSERT(impulse.Finite());
		if(!impulse.Finite())
			return;
		if(m_pOwningScene)//TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ApplyImpulseLocalSpace;
			pCommand->m_CommandBundle.SetValue(impulse);
			pCommand->m_CommandBundle.SetPosition(position);
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			m_RigidBody.ApplyImpulseLocalSpace(impulse, position);
		}
	}

	void C_Body::AddImpulse(const Maths::HVec3 PCREF impulse)
	{
		BIZ_ASSERT(impulse.Length() <= 1000.f * m_RigidBody.Mass());
		BIZ_ASSERT(impulse.Finite());
		if(!impulse.Finite())
			return;
		if(m_pOwningScene)//TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_AddImpulse;
			pCommand->m_CommandBundle.SetValue(impulse);
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			m_RigidBody.AddImpulse(impulse);
		}
	}

	void C_Body::AddAngularImpulse(const Maths::HVec3 PCREF impulse)
	{
		BIZ_ASSERT(impulse.Finite());
		if(!impulse.Finite())
			return;
		if(m_pOwningScene)//TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_AddAngularImpulse;
			pCommand->m_CommandBundle.SetValue( impulse );
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			m_RigidBody.AddAngularImpulse(impulse);
		}
	}

	void C_Body::ApplyForce(const Maths::HVec3 PCREF force, const Maths::HVec3 PCREF position)
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping))
		{
			BIZ_ASSERT(force.Finite());
			BIZ_ASSERT(position.Finite());
			if(!force.Finite() || !position.Finite())
				return;
			const Maths::HVec3 dir(position - m_ExternalState.m_Position);
			m_ExternalState.m_Force += force;
			m_ExternalState.m_Torque += force ^ dir;
		}
	}

	void C_Body::AddForce(const Maths::HVec3 PCREF force)
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping))
		{
			BIZ_ASSERT(force.Finite());
			if(!force.Finite())
				return;
			m_GraphNode.m_Deactivator.Activate();
			m_ExternalState.m_Force += force;
		}
	}

	void C_Body::SetBuoyancyCoefficient(const Maths::HF32 PCREF coefficient)
	{
		m_RigidBody.BuoyancyCoefficient(coefficient);
	}

	void C_Body::SetLinearDragCoefficient(const Maths::HF32 PCREF coefficient)
	{
		m_RigidBody.FluidLinearDragCoefficient(coefficient);
	}

	void C_Body::SetAngularDragCoefficient(const Maths::HF32 PCREF coefficient)
	{
		m_RigidBody.FluidAngularDragCoefficient(coefficient);
	}

	Maths::HF32 C_Body::GetBuoyancyCoefficient() const
	{
		return m_RigidBody.BuoyancyCoefficient();
	}

	Maths::HF32 C_Body::GetLinearDragCoefficient() const
	{
		return m_RigidBody.FluidLinearDragCoefficient();
	}

	Maths::HF32 C_Body::GetAngularDragCoefficient() const
	{
		return m_RigidBody.FluidAngularDragCoefficient();
	}

	void C_Body::CalculateBodyVolume()
	{
		m_pShape->CalculateVolume();
	}

	void C_Body::AddTorque(const Maths::HVec3 PCREF torque)
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping))
		{
			//m_ExternalState.m_Torque += torque;
			BIZ_ASSERT(torque.Finite());
			if(!torque.Finite())
				return;
			m_ExternalState.m_Torque += torque;
		}
	}

	void C_Body::Force(const Maths::HVec3 PCREF force)
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping))
			m_ExternalState.m_Force = force;
	}

	Maths::HVec3 C_Body::Force() const
	{
		return m_ExternalState.m_Force;
	}

	void C_Body::Torque(const Maths::HVec3 PCREF torque)
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping))
			m_ExternalState.m_Torque = torque;
	}

	Maths::HVec3 C_Body::Torque()
	{
		return m_ExternalState.m_Torque;
	}

	void C_Body::Position(const Maths::HVec3 PCREF position)
	{
		BIZ_ASSERT(position.Finite());
		BIZ_ASSERT(!TestBodyFlags( C_Body::E_Type_Removed ));
		m_ExternalState.m_Position = position;
		if(m_pOwningScene)//TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_OverridePosition;
			pCommand->m_CommandBundle.SetValue( position );
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			m_RigidBody.m_Position = position;
			m_RigidBody.m_CenterOfMass = position + m_RigidBody.m_CenterOfMassOffset * m_RigidBody.m_OrientationMatrix;
			m_BodyFlags |= E_Type_Overridden_PosRot;
		}
	}

	void C_Body::Orientation(const Maths::C_Quaternion PCREF orientation)
	{
		BIZ_ASSERT(orientation.Finite());
		BIZ_ASSERT(orientation.Length() > Maths::Constants<Maths::HF32>::Zero());
		BIZ_ASSERT(!TestBodyFlags( C_Body::E_Type_Removed ));
		m_ExternalState.m_Orientation = orientation;
		if(m_pOwningScene)//TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_OverrideOrientation;
			pCommand->m_CommandBundle.SetQuaternion( orientation );
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			
			m_RigidBody.m_Orientation = orientation;
			m_RigidBody.m_OrientationMatrix = orientation;
			m_BodyFlags |= E_Type_Overridden_PosRot;
		}
	}

	void C_Body::Orientation(const Maths::HMat33 PCREF orientation)
	{
		BIZ_ASSERT(orientation.Finite());
		//BIZ_ASSERT(orientation.Length() > Maths::Constants<Maths::HF32>::Zero());
		BIZ_ASSERT(!TestBodyFlags( C_Body::E_Type_Removed ));
		m_ExternalState.m_Orientation = orientation;
		if(m_pOwningScene)//TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_OverrideOrientation;
			pCommand->m_CommandBundle.SetQuaternion( Maths::C_Quaternion(orientation) );
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			
			m_RigidBody.m_Orientation = Maths::C_Quaternion(orientation);
			m_RigidBody.m_OrientationMatrix = orientation;
			m_BodyFlags |= E_Type_Overridden_PosRot;
		}
	}

	void C_Body::LinearVelocity(const Maths::HVec3 PCREF velocity)
	{
		BIZ_ASSERT(velocity.Length() < 1000.f);
		BIZ_ASSERT(velocity.Length().Finite());
		BIZ_ASSERT(!TestBodyFlags( C_Body::E_Type_Removed ));
		m_ExternalState.m_LinearVelocity = velocity;
		if(m_pOwningScene)//TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_OverrideLinearVelocity;
			pCommand->m_CommandBundle.SetValue( velocity );
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			m_RigidBody.m_LinearVelocity = velocity;
			m_RigidBody.m_LinearMomentum = velocity * m_RigidBody.ReciprocalMass();
		}
	}

	Maths::HVec3 C_Body::LinearVelocity() const
	{
		return m_ExternalState.m_LinearVelocity;
	}

	void C_Body::AngularVelocity(const Maths::HVec3 PCREF angVel)
	{
		BIZ_ASSERT(angVel.Length() < 1000.f);
		BIZ_ASSERT(angVel.Length().Finite());
		BIZ_ASSERT(!TestBodyFlags( C_Body::E_Type_Removed ));
		m_ExternalState.m_AngularVelocity = angVel;
		if(m_pOwningScene)//TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_OverrideAngularVelocity;
			pCommand->m_CommandBundle.SetValue( angVel );
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			m_RigidBody.m_AngularVelocity = angVel;
			m_RigidBody.m_AngularMomentum = angVel * m_RigidBody.m_InertiaTensor;
		}
	}

	Maths::HVec3 C_Body::AngularVelocity() const
	{
		return m_ExternalState.m_AngularVelocity;
	}

	Maths::HMat33 C_Body::IBodyMatrix() const
	{
		return m_RigidBody.m_IBody;
	}

	void C_Body::IBodyMatrix(const Maths::HMat33 PCREF matrix)
	{
		if(TestBodyFlags(C_Body::E_InsertedIntoWorld))
		{
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_OverrideInertiaTensor;
			pCommand->m_CommandBundle.SetTensor( matrix );
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		else
		{
			m_RigidBody.m_IBody = matrix;
			m_RigidBody.m_IBodyInverse = C_InertiaTensorGenerator::InverseInertiaTensor(matrix);
			m_RigidBody.StoreVelocitiesAndPosition(m_RigidBody.m_LinearVelocity, 
				m_RigidBody.m_AngularVelocity, m_RigidBody.m_Orientation, m_RigidBody.m_Position);
		}
	}

	Maths::HMat33 C_Body::IBodyInverse() const
	{
		return m_RigidBody.m_IBodyInverse;
	}

	SharedPhysics::C_Integrator& C_Body::GetIntegrator()
	{
		return m_pOwningScene->m_Integrator;
	}

	void C_Body::ApplyDeferredCommand(C_DeferredBodyCommand& command)
	{
		//The only valid command when the body isn't part of a scene is to delete it!
		if(command.m_CommandType ==  C_DeferredBodyCommand::E_ReleaseBody)
		{
			command.m_CommandBundle.m_pScene->ReleaseBody(this);
			return;
		}
		if(m_pOwningScene == NULL)
			return;

		BIZ_ASSERT(m_RigidBody.ValidState());

		switch(command.m_CommandType)
		{
		case C_DeferredBodyCommand::E_OverridePosition:
			m_RigidBody.m_Position = command.m_CommandBundle.GetValue();
			m_RigidBody.m_CenterOfMass = m_RigidBody.m_Position + m_RigidBody.m_CenterOfMassOffset * m_RigidBody.m_OrientationMatrix;
			if(!TestBodyFlags(E_Type_Overridden_Velocities | E_Type_Overridden_PosRot | E_Type_Removed))
				m_pOwningScene->m_OverriddenBodyList.AddToEnd(&C_Scene::BodyPtr(this));
			m_BodyFlags |= E_Type_Overridden_PosRot;
			//m_pOwningScene->m_OverriddenBodyList.AddToEnd(&BodyPtr(this));
			break;
		case C_DeferredBodyCommand::E_OverrideOrientation:
			m_RigidBody.m_Orientation = command.m_CommandBundle.GetQuaternion();
			m_RigidBody.m_OrientationMatrix = m_RigidBody.m_Orientation;
			m_RigidBody.m_CenterOfMass = m_RigidBody.m_Position + m_RigidBody.m_CenterOfMassOffset * m_RigidBody.m_OrientationMatrix;
			if(!TestBodyFlags(E_Type_Overridden_Velocities | E_Type_Overridden_PosRot | E_Type_Removed))
				m_pOwningScene->m_OverriddenBodyList.AddToEnd(&C_Scene::BodyPtr(this));
			m_BodyFlags |= E_Type_Overridden_PosRot;
			//m_pOwningScene->m_OverriddenBodyList.AddToEnd(&BodyPtr(this));
			break;
		case C_DeferredBodyCommand::E_OverrideLinearVelocity:
			{
				const Maths::HVec3 mom(command.m_CommandBundle.GetValue() * m_RigidBody.Mass());
				m_RigidBody.m_LinearVelocity = command.m_CommandBundle.GetValue();
				m_RigidBody.m_LinearMomentum = mom;	
				if(!TestBodyFlags(E_Type_Overridden_Velocities | E_Type_Overridden_PosRot | E_Type_Removed))
					m_pOwningScene->m_OverriddenBodyList.AddToEnd(&C_Scene::BodyPtr(this));
				m_BodyFlags |= E_Type_Overridden_Velocities;
				//m_pOwningScene->m_OverriddenBodyList.AddToEnd(&BodyPtr(this));
			}
			break;
		case C_DeferredBodyCommand::E_OverrideAngularVelocity:
			{
				const Maths::HVec3 angMom(command.m_CommandBundle.GetValue() * m_RigidBody.m_InertiaTensor);
				m_RigidBody.m_AngularVelocity = command.m_CommandBundle.GetValue();
				m_RigidBody.m_AngularMomentum = angMom;
				if(!TestBodyFlags(E_Type_Overridden_Velocities | E_Type_Overridden_PosRot | E_Type_Removed))
					m_pOwningScene->m_OverriddenBodyList.AddToEnd(&C_Scene::BodyPtr(this));
				m_BodyFlags |= E_Type_Overridden_Velocities;
			}
			break;
		case C_DeferredBodyCommand::E_OverrideInertiaTensor:
			{
				const Maths::HMat33 inverse = C_InertiaTensorGenerator::InverseInertiaTensor(command.m_CommandBundle.GetTensor());
				m_RigidBody.m_IBody = command.m_CommandBundle.GetTensor();
				m_RigidBody.m_IBodyInverse = inverse; 
				m_RigidBody.StoreVelocitiesAndPosition(m_RigidBody.m_LinearVelocity, 
				m_RigidBody.m_AngularVelocity, m_RigidBody.m_Orientation, m_RigidBody.m_Position);
				if(!TestBodyFlags(E_Type_Overridden_Velocities | E_Type_Overridden_PosRot | E_Type_Removed))
					m_pOwningScene->m_OverriddenBodyList.AddToEnd(&C_Scene::BodyPtr(this));
				m_BodyFlags |= E_Type_Overridden_Velocities;
				break;
			}
		case C_DeferredBodyCommand::E_ApplyImpulse:
		{
			bool bActivate = m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);			
			if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
			{
				const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
				const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
				const Maths::HF32 impulseLen(command.m_CommandBundle.GetValue().Length());
				const Maths::HF32 vel(impulseLen * m_RigidBody.ReciprocalMass());
				const Maths::HF32 energy(half * vel * vel * m_RigidBody.Mass());
				if(m_GraphNode.m_Deactivator.m_StaticActivateThresholdSq.GE(zero) & (energy*energy).GT(m_GraphNode.m_Deactivator.m_StaticActivateThresholdSq))
				{
					bActivate = true;
				}
			}
			
			if(bActivate)
			{
				_Activate();
				m_RigidBody.ApplyImpulseWorldSpace(command.m_CommandBundle.GetValue(), command.m_CommandBundle.GetPosition() * m_RigidBody.m_OrientationMatrix + m_RigidBody.m_Position);
				if(!TestBodyFlags(E_Type_Overridden_Velocities | E_Type_Overridden_PosRot | E_Type_Removed))
					m_pOwningScene->m_OverriddenBodyList.AddToEnd(&C_Scene::BodyPtr(this));
				m_BodyFlags |= E_Type_Overridden_Velocities;
			//m_pOwningScene->m_OverriddenBodyList.AddToEnd(&BodyPtr(this));
			}
			break;
		}
		case C_DeferredBodyCommand::E_ApplyImpulseLocalSpace:
		{
			bool bActivate = m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);			
			if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
			{
				const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
				const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
				const Maths::HF32 impulseLen(command.m_CommandBundle.GetValue().Length());
				const Maths::HF32 vel(impulseLen * m_RigidBody.ReciprocalMass());
				const Maths::HF32 energy(half * vel * vel * m_RigidBody.Mass());
				if(m_GraphNode.m_Deactivator.m_StaticActivateThresholdSq.GE(zero) & (energy*energy).GT(m_GraphNode.m_Deactivator.m_StaticActivateThresholdSq))
				{
					bActivate = true;
				}
			}
			
			if(bActivate)
			{
				_Activate();
				m_RigidBody.ApplyImpulseLocalSpace(command.m_CommandBundle.GetValue(), command.m_CommandBundle.GetPosition());
				if(!TestBodyFlags(E_Type_Overridden_Velocities | E_Type_Overridden_PosRot | E_Type_Removed))
					m_pOwningScene->m_OverriddenBodyList.AddToEnd(&C_Scene::BodyPtr(this));
				m_BodyFlags |= E_Type_Overridden_Velocities;
			}
			//m_pOwningScene->m_OverriddenBodyList.AddToEnd(&BodyPtr(this));
			break;
		}
		case C_DeferredBodyCommand::E_AddImpulse:
		{
			bool bActivate = m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);			
			if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
			{
				const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
				const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
				const Maths::HF32 impulseLen(command.m_CommandBundle.GetValue().Length());
				const Maths::HF32 vel(impulseLen * m_RigidBody.ReciprocalMass());
				const Maths::HF32 energy(half * vel * vel * m_RigidBody.Mass());
				if(m_GraphNode.m_Deactivator.m_StaticActivateThresholdSq.GE(zero) & (energy*energy).GT(m_GraphNode.m_Deactivator.m_StaticActivateThresholdSq))
				{
					bActivate = true;
				}
			}
			
			if(bActivate)
			{
				_Activate();
				m_RigidBody.AddImpulse(command.m_CommandBundle.GetValue());
				if(!TestBodyFlags(E_Type_Overridden_Velocities | E_Type_Overridden_PosRot | E_Type_Removed))
					m_pOwningScene->m_OverriddenBodyList.AddToEnd(&C_Scene::BodyPtr(this));
				m_BodyFlags |= E_Type_Overridden_Velocities;
			}
			break;
		}

		case C_DeferredBodyCommand::E_AddAngularImpulse:
		{
			bool bActivate = m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);						
			if(bActivate)
			{
				_Activate();
				m_RigidBody.AddAngularImpulse(command.m_CommandBundle.GetValue());
				if(!TestBodyFlags(E_Type_Overridden_Velocities | E_Type_Overridden_PosRot | E_Type_Removed))
					m_pOwningScene->m_OverriddenBodyList.AddToEnd(&C_Scene::BodyPtr(this));
				m_BodyFlags |= E_Type_Overridden_Velocities;
			}
			break;
		}
		case C_DeferredBodyCommand::E_Deactivate:
			{
				_Deactivate();
				//m_pOwningScene->SendDeactivateMessage(this);
			}
			break;
		case C_DeferredBodyCommand::E_Activate:
			{
				_Activate();
				//m_pOwningScene->SendActivateMessage(this);
			}
			break;
		case C_DeferredBodyCommand::E_MakeStatic:
			_MakeStatic();
			break;
		case C_DeferredBodyCommand::E_MakeRaytraceOnly:
			_MakeRaytraceOnly();
			break;
		case C_DeferredBodyCommand::E_DisableRaytraceOnly:
			_DisableRaytraceOnly();
			break;
		case C_DeferredBodyCommand::E_MakeDebris:
			_MakeDebris();
			break;
		case C_DeferredBodyCommand::E_DisableDebris:
			_DisableDebris();
			break;
		case C_DeferredBodyCommand::E_MakeKinematic:
			_MakeKinematic();
			break;
		case C_DeferredBodyCommand::E_AddBody:
			m_pOwningScene->InsertBodyInWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveBody:
			command.m_CommandBundle.m_pScene->RemoveBodyFromWorld(this, command.m_CommandBundle.m_pCallback, command.m_CommandBundle.m_pUserData);
			break;
		case C_DeferredBodyCommand::E_AddMeshBody:
			m_pOwningScene->InsertMeshBodyInWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveMeshBody:
			m_pOwningScene->RemoveMeshBodyFromWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveBodyNoDelete:
			command.m_CommandBundle.m_pScene->RemoveBodyFromWorldNoDelete(this);
			break;
		case C_DeferredBodyCommand::E_BroadPhaseWakeUp:
			DoWakeUpBroadPhase(m_pOwningScene->m_pTree);
			break;
		case C_DeferredBodyCommand::E_AddClothBody:
			m_pOwningScene->InsertClothBodyInWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveClothBody:
			m_pOwningScene->RemoveClothBodyFromWorld(this);
			break;
		case C_DeferredBodyCommand::E_AddDeformableBody:
			m_pOwningScene->InsertDeformableBodyInWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveDeformableBody:
			m_pOwningScene->RemoveDeformableBodyFromWorld(this);
			break;
		case C_DeferredBodyCommand::E_AddRopeBody:
			m_pOwningScene->InsertRopeBodyInWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveRopeBody:
			m_pOwningScene->RemoveRopeBodyFromWorld(this);
			break;
		case C_DeferredBodyCommand::E_AddFluidBody:
			m_pOwningScene->InsertFluidBodyInWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveFluidBody:
			m_pOwningScene->RemoveFluidBodyFromWorld(this);
			break;
		case C_DeferredBodyCommand::E_AddFluidMeshBody:
			m_pOwningScene->InsertFluidMeshBodyInWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveFliudMeshBody:
			m_pOwningScene->RemoveFluidMeshBodyFromWorld(this);
			break;
		case C_DeferredBodyCommand::E_AddStaticBody:
			m_pOwningScene->InsertStaticBodyInWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveStaticBody:
			m_pOwningScene->RemoveStaticBodyFromWorld(this);
			break;
		case C_DeferredBodyCommand::E_SetCentreOfMassOffset:
			this->m_RigidBody.m_CenterOfMassOffset = command.m_CommandBundle.GetValue();
			break;
		case C_DeferredBodyCommand::E_UpdateBodyShape:
			UpdateBodyShape(command.m_CommandBundle.m_pShape);
			break;
		case C_DeferredBodyCommand::E_ChangeBodyState:
			{
				U32 change = (m_ExternalState.m_ExternalBodyState ^ m_GraphNode.m_NodeFlags) & SharedPhysics::C_GraphNode::E_Type_BodyStateMask;
				U32 stateChange = (m_ExternalState.m_ExternalBodyState & ~(m_GraphNode.m_NodeFlags)) & SharedPhysics::C_GraphNode::E_Type_BodyStateMask;
				if(change)
				{
					if(change & SharedPhysics::C_GraphNode::E_Type_RaytraceOnly)
					{
						if(m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_RaytraceOnly)
						{
							_MakeRaytraceOnly();
						}
						else
						{
							_DisableRaytraceOnly();
						}
					}
					if(change & SharedPhysics::C_GraphNode::E_Type_Debris)
					{
						if(m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_Debris)
						{
							_MakeDebris();
						}
						else
						{
							_DisableDebris();
						}
					}

					if(stateChange & SharedPhysics::C_GraphNode::E_Type_Active)
					{
						_Activate();
					}
					else if (stateChange & SharedPhysics::C_GraphNode::E_Type_Sleeping)
					{
						_Deactivate();
					}
					else if (stateChange & SharedPhysics::C_GraphNode::E_Type_Static)
					{
						_MakeStatic();
					}
					else if (stateChange & SharedPhysics::C_GraphNode::E_Type_KinematicObject)
					{
						_MakeKinematic();
					}
				}
				break;
			}
		/*case C_DeferredBodyCommand::E_AddContinuousBody:
			m_pOwningScene->InsertContinuousBodyInWorld(this);
			break;
		case C_DeferredBodyCommand::E_RemoveContinuousBody:
			m_pOwningScene->RemoveContinuousBodyFromWorld(this);*/
			//break;
		default:
			BIZ_ASSERT(0 && "Command not recognised!");
			break;
		}
		BIZ_ASSERT(m_RigidBody.ValidState());
	}

	bool C_Body::Raytrace(Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		PERF_EVENT("Gravity.TraceObject");
		//Raytrace against the shape. Report the first point of contact!
		BIZ_ASSERT(m_pOwningScene);
		Collision::C_CollisionObject obj(m_Object.CollisionObject());
		return m_pOwningScene->m_pCollisionManager->RayTrace(&obj, ray, stack, spuCache);
	}

	void C_Body::ApplyForcesAndTorques(const Maths::HF32 PCREF time)
	{
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		const Maths::HVec3 mom(m_RigidBody.m_LinearMomentum + (m_RigidBody.m_Force * time));
		const Maths::HVec3 angMom(m_RigidBody.m_AngularMomentum + (m_RigidBody.m_Torque* time));

		m_RigidBody.m_LinearVelocity = mom * m_RigidBody.ReciprocalMass();
		m_RigidBody.m_AngularVelocity = angMom * m_RigidBody.m_InverseInertiaTensor;
		m_RigidBody.m_LinearMomentum = mom;
		m_RigidBody.m_AngularMomentum = angMom;
		m_RigidBody.m_Force = m_RigidBody.m_Torque = zero;

	}

	void C_Body::ZeroExternalForces()
	{
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		//Simple for now - copy across external force/torque from external buffer and constants (gravity)
		m_RigidBody.m_Force = zero;
		m_RigidBody.m_Torque = zero;
	}

	void C_Body::ClearExternalForces()
	{
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		m_ExternalState.m_Force = zero;
		m_ExternalState.m_Torque = zero;
	}


	void C_Body::Activate()
	{
		if((m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_Active) != 0)
		{
			BIZ_ASSERT(!IsExternalSleeping());
			BIZ_ASSERT(!IsExternalStatic());
			return;
		}
		
		if((m_BodyFlags & E_InsertedIntoWorld) == 0)
		{
			m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia);
			//Flag it as waking up, so that it's contacts are grabbed from the contact database
			m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active);
			m_GraphNode.m_IslandIndex = -1;
			if(!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			{
				if(m_pOwningScene)
				{
					m_Object.WakeUp(m_pOwningScene->m_pTree);
				}
				else
				{
					if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Cloth))
					{
						m_Object.m_ObjectState = E_ActiveClothObject;
					}
					else
					{
						m_Object.m_ObjectState = E_ActiveObject;
					}
				}
			}
			m_GraphNode.m_Deactivator.Activate();
		}
		else if(!HasExternalStateChanged())
		{
			Gravity::C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ChangeBodyState;
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		m_ExternalState.m_ExternalBodyState &= ~(SharedPhysics::C_GraphNode::E_Type_ZeroInertia);
		m_ExternalState.m_ExternalBodyState |= SharedPhysics::C_GraphNode::E_Type_Active;
	}


	void C_Body::Deactivate()
	{
		if((m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_ZeroInertia) != 0)
		{
			BIZ_ASSERT(!IsExternalActive());
			return;
		}
	
		if((m_BodyFlags & E_InsertedIntoWorld) == 0)
		{
			m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active);
			m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping);
			m_GraphNode.m_Deactivator.m_CountDown = -1;
			m_GraphNode.m_IslandIndex = 0;
			m_RigidBody.m_LinearVelocity = m_RigidBody.m_LinearMomentum = m_RigidBody.m_AngularVelocity = m_RigidBody.m_AngularMomentum = Maths::Constants<Maths::HVec3>::Zero();
			m_RigidBody.CopyVelocityToBVector();
			if(!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			{
				m_Object.PutToSleep();
			}
		}
		else if(!HasExternalStateChanged())
		{
			Gravity::C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ChangeBodyState;
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		m_ExternalState.m_ExternalBodyState &= ~(SharedPhysics::C_GraphNode::E_Type_Active);
		m_ExternalState.m_ExternalBodyState |= SharedPhysics::C_GraphNode::E_Type_Sleeping;
	}

	void C_Body::MakeStatic()
	{
		if((m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_Static) != 0)
		{
			return;
		}
		if((m_BodyFlags & E_InsertedIntoWorld) == 0)
		{
			m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);
			m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static);
			if(!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			{
				m_Object.MakeStatic();
			}
			m_GraphNode.m_IslandIndex = 0;
		}
		else if(!HasExternalStateChanged())
		{
			Gravity::C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ChangeBodyState;
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		m_ExternalState.m_ExternalBodyState &= ~(SharedPhysics::C_GraphNode::E_Type_ZeroInertia | SharedPhysics::C_GraphNode::E_Type_Active);
		m_ExternalState.m_ExternalBodyState |= SharedPhysics::C_GraphNode::E_Type_Static;
	}

	void C_Body::MakeRaytraceOnly()
	{
		if((m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_RaytraceOnly) != 0)
		{
			return;
		}
		
		if((m_BodyFlags & E_InsertedIntoWorld) == 0)
		{
			//Note - you need to have set the state as either static/active beforehand!!!
			//m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);
			m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly);
			m_Object.MakeRaytraceOnly();
			m_GraphNode.m_IslandIndex = 0;
		}
		else if(!HasExternalStateChanged())
		{
			Gravity::C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ChangeBodyState;
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		m_ExternalState.m_ExternalBodyState |= SharedPhysics::C_GraphNode::E_Type_RaytraceOnly;
	}

	void C_Body::DisableRaytraceOnly()
	{
		if((m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_RaytraceOnly) == 0)
		{
			return;
		}
		
		if((m_BodyFlags & E_InsertedIntoWorld) == 0)
		{
			//Note - you need to have set the state as either static/active beforehand!!!
			//m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);
			m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly);
			if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
			{
				if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Cloth))
				{
					m_Object.m_ObjectState = E_ActiveClothObject;
				}
				else
				{
					m_Object.m_ObjectState = E_ActiveObject;
				}
			}
			else if (m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping))
			{
				m_Object.PutToSleep();
			}
			else if (m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
			{
				m_Object.MakeStatic();
			}
			else if (m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			{
				m_Object.MakeKinematic();
			}
			m_GraphNode.m_IslandIndex = 0;
		}
		else if(!HasExternalStateChanged())
		{
			Gravity::C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ChangeBodyState;
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		m_ExternalState.m_ExternalBodyState &= ~(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly);
	}


	void C_Body::MakeDebris()
	{
		if((m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_Debris) != 0)
		{
			return;
		}
		
		if((m_BodyFlags & E_InsertedIntoWorld) == 0)
		{
			//Note - you need to have set the state as either static/active beforehand!!!
			//m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);
			m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris);
			/*if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
			{
				m_Object.WakeUp();
			}
			else
			{
				m_Object.PutToSleep();
			}*/
			m_GraphNode.m_IslandIndex = 0;
		}
		else if(!HasExternalStateChanged())
		{
			Gravity::C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ChangeBodyState;
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		m_ExternalState.m_ExternalBodyState |= SharedPhysics::C_GraphNode::E_Type_Debris;
	}

	void C_Body::DisableDebris()
	{
		if((m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_Debris) == 0)
		{
			return;
		}
		
		
		if((m_BodyFlags & E_InsertedIntoWorld) == 0)
		{
			if(!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris))
				return;	
			//Note - you need to have set the state as either static/active beforehand!!!
			//m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);
			m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris);
			/*if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
			{
				m_Object.WakeUp();
			}
			else
			{
				m_Object.PutToSleep();
			}*/
			m_GraphNode.m_IslandIndex = 0;
		}
		else if(!HasExternalStateChanged())
		{
			Gravity::C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ChangeBodyState;
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		m_ExternalState.m_ExternalBodyState &= ~(SharedPhysics::C_GraphNode::E_Type_Debris);
	}




	void C_Body::MakeKinematic()
	{
		
		if((m_ExternalState.m_ExternalBodyState & SharedPhysics::C_GraphNode::E_Type_KinematicObject) != 0)
		{
			return;
		}
		
		if((m_BodyFlags & E_InsertedIntoWorld) == 0)
		{
			m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static);
			m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject);
			if(!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			{
				m_Object.MakeKinematic();
			}
			m_GraphNode.m_IslandIndex = -1;
		}
		else if(!HasExternalStateChanged())
		{
			Gravity::C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_ChangeBodyState;
			pCommand->m_CommandBundle.m_pBody = this;
			m_pOwningScene->AddCommand(pCommand);
		}
		m_ExternalState.m_ExternalBodyState &= ~(SharedPhysics::C_GraphNode::E_Type_ZeroInertia | SharedPhysics::C_GraphNode::E_Type_Active);
		m_ExternalState.m_ExternalBodyState |= SharedPhysics::C_GraphNode::E_Type_KinematicObject;
	}

	void C_Body::MakeStaticMesh()
	{
		BIZ_ASSERT((m_BodyFlags & E_InsertedIntoWorld) == 0);
		//if((m_BodyFlags & E_InsertedIntoWorld) == 0)
		{
			m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping);
			m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_StaticMesh);
		}
	}

	void C_Body::MakeInstancedCollidableMesh()
	{
		BIZ_ASSERT((m_BodyFlags & E_InsertedIntoWorld) == 0);
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_InstancedMesh);
	}

	void C_Body::_Activate()
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
			return;

		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
		{
			m_pOwningScene->m_KinematicBodyList.RemoveBody(this);
		}
		m_pOwningScene->m_ActiveBodyList.AddBody(this);

		m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject);
		//Flag it as waking up, so that it's contacts are grabbed from the contact database
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active);
		m_GraphNode.m_Deactivator.Activate();
		if(!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			m_Object.WakeUp(m_pOwningScene->m_pTree);
		m_GraphNode.m_IslandIndex = -1;
		C_BodyEvent* pEvent = m_pOwningScene->m_EventStream.CreateEvent();
		pEvent->SetBody(this);
		pEvent->SetEventType(C_BodyEvent::E_WakeUp);
	}

	void C_Body::_Deactivate()
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
			return;
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			m_pOwningScene->m_ActiveBodyList.RemoveBody(this);
		}
		//if(this->m_Object.m_NumNodesContained == 0)
		{
			m_Object.MarkDirty();
			m_pOwningScene->m_pTree->AddDirtyObject(&m_Object);
		}
		m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active );
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping);
		if(!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			m_Object.PutToSleep();

		m_RigidBody.m_LinearVelocity = m_RigidBody.m_LinearMomentum = m_RigidBody.m_AngularVelocity = m_RigidBody.m_AngularMomentum = Maths::Constants<Maths::HVec3>::Zero();
		m_RigidBody.CopyVelocityToBVector();
		m_GraphNode.m_IslandIndex = 0;
		C_BodyEvent* pEvent = m_pOwningScene->m_EventStream.CreateEvent();
		pEvent->SetBody(this);
		pEvent->SetEventType(C_BodyEvent::E_GoToSleep);
	}
	
	void C_Body::_MakeStatic()
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
			return;
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			m_pOwningScene->m_ActiveBodyList.RemoveBody(this);
		}
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
		{
			m_pOwningScene->m_KinematicBodyList.RemoveBody(this);
		}
		//if(this->m_Object.m_NumNodesContained == 0)
		{
			m_Object.MarkDirty();
			m_pOwningScene->m_pTree->AddDirtyObject(&m_Object);
		}
		m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping | 
			SharedPhysics::C_GraphNode::E_Type_KinematicObject);
		m_RigidBody.m_LinearVelocity = m_RigidBody.m_LinearMomentum = m_RigidBody.m_AngularVelocity = m_RigidBody.m_AngularMomentum = Maths::Constants<Maths::HVec3>::Zero();
		m_RigidBody.CopyVelocityToBVector();
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static);
		m_GraphNode.m_IslandIndex = 0;
		if(!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			m_Object.MakeStatic();
	}

	void C_Body::_MakeRaytraceOnly()
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			return;
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly);
		//m_GraphNode.m_IslandIndex = 0;
		m_Object.MakeRaytraceOnly();
	}

	void C_Body::_DisableRaytraceOnly()
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
		{
			m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly);
			if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
			{
				m_Object.WakeUp(m_pOwningScene->m_pTree);
			}
			else if (m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping))
			{
				m_Object.PutToSleep();
			}
			else if (m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
			{
				m_Object.MakeStatic();
			}
			else if (m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			{
				m_Object.MakeKinematic();
			}
			m_GraphNode.m_IslandIndex = 0;
		}
	}

	void C_Body::_MakeDebris()
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris))
			return;
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris);
		//m_GraphNode.m_IslandIndex = 0;
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			m_Object.WakeUp(m_pOwningScene->m_pTree);
		}
		else
		{
			m_Object.PutToSleep();
		}
	}

	void C_Body::_DisableDebris()
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris))
		{
			m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris);
			if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
			{
				m_Object.WakeUp(m_pOwningScene->m_pTree);
			}
			else
			{
				m_Object.PutToSleep();
			}
			m_GraphNode.m_IslandIndex = 0;
		}
	}


	void C_Body::_MakeKinematic()
	{
		if(m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			return;
		if(IsActive())
		{
			m_pOwningScene->m_ActiveBodyList.RemoveBody(this);
		}
		m_pOwningScene->m_KinematicBodyList.AddBody(this);
		m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static);
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject);
		m_GraphNode.m_IslandIndex = -1;
		if(!m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			m_Object.MakeKinematic();
	}

	const Collision::C_CRAxisBox C_Body::GetLocalBoundingBox() const
	{
		return m_pShape->m_LocalBoundingBox;
	}

	const Collision::C_Sphere C_Body::GetLocalBoundingSphere() const
	{
		Collision::C_Sphere sphere;
		sphere.Centre(m_pShape->m_LocalBoundingBox.centre);
		sphere.Radius(m_pShape->m_LocalBoundingBox.extents.Length());
		return sphere;
	}

	void C_BodyList::AddBody(C_Body* pBody)
	{
		BIZ_ASSERT(pBody->m_pNext == NULL && pBody->m_pPrev == NULL && m_pFirstBody != pBody);
		if(pBody->m_pNext != NULL || pBody->m_pPrev != NULL ||m_pFirstBody == pBody)
			return;

		pBody->m_pNext = m_pFirstBody;
		if(m_pFirstBody)
		{
			m_pFirstBody->m_pPrev = pBody;
		}
		pBody->m_pPrev = NULL;
		m_pFirstBody = pBody;
	}

	void C_BodyList::RemoveBody(C_Body* pBody)
	{
		//assert(pBody->m_pNext != NULL || pBody->m_pPrev != NULL || pBody == m_pFirstActiveBody);
		if(pBody->m_pNext)
		{
			pBody->m_pNext->m_pPrev = pBody->m_pPrev;
		}
		if(pBody->m_pPrev)
		{
			pBody->m_pPrev->m_pNext = pBody->m_pNext;
		}

		if(pBody == m_pFirstBody)
		{
			m_pFirstBody = pBody->m_pNext;
		}
		pBody->m_pNext = pBody->m_pPrev = NULL;

	}

	void C_BodyList::ClearList()
	{
		C_Body* pBody = m_pFirstBody;
		while(pBody)
		{
			C_Body* pNextBody = pBody->m_pNext;
			pBody->m_pNext = pBody->m_pPrev = NULL;
			pBody = pNextBody;
		}
		m_pFirstBody = NULL;
	}
	void C_Body::UpdateShape(Collision::C_Shape* pShape)
	{
		if(m_pOwningScene == NULL)
			UpdateBodyShape(pShape);
		else
		{
			m_pFutureShape = pShape;
			C_DeferredBodyCommand* pCommand = m_pOwningScene->GetCommand();
			pCommand->m_CommandType = C_DeferredBodyCommand::E_UpdateBodyShape;
			pCommand->m_CommandBundle.m_pBody = this;
			pCommand->m_CommandBundle.m_pShape = pShape;
			m_pOwningScene->AddCommand(pCommand);
		}
	}

	void C_Body::UpdateBodyShape(Collision::C_Shape* pNewShape)
	{
		const Maths::HMat43 oldElementMat = InternalElementMatrix();
		const Maths::HVec3 oldElementOffset(m_pShape->m_ElementOffset);
		const Maths::HVec3 newElementOffset(m_pFutureShape->m_ElementOffset);
		const Maths::HVec3 elementDifference(oldElementOffset - newElementOffset);
		UpdateConstraints(elementDifference);
		SetShape(pNewShape);
		//ElementMatrix(oldElementMat);
		Maths::HVec3 newPos((m_pShape->m_ElementOffset)*oldElementMat);
		//Maths::C_Quaternion quat(oldElementMat.Rotation());
		const Maths::HMat33 oldRot(oldElementMat.Rotation());
		m_RigidBody.m_Position = newPos;
		m_RigidBody.m_Orientation = Maths::C_Quaternion(oldRot);
		m_RigidBody.m_OrientationMatrix = oldRot;
		m_RigidBody.m_CenterOfMass = m_RigidBody.m_Position + m_RigidBody.m_CenterOfMassOffset * m_RigidBody.m_OrientationMatrix;
		SynchroniseState();
		SynchronisePosRot();
		m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_State_Changed);
	}
	
	void C_Body::UpdateConstraints(const Maths::HVec3 PCREF elementOffset)
	{
		SharedPhysics::C_ConstraintEdge* pEdge = m_GraphNode.m_pPersistentConstraints;
		while(pEdge)
		{
			SharedPhysics::C_ConstraintPoint* pPoint(pEdge->m_pFirstConstraint);
			while(pPoint)
			{
				pPoint->m_pConstraint->ApplyBodyOffset(this, elementOffset);
				pPoint->m_pConstraint->ZeroWarmStartValues();
				pPoint = pPoint->m_pNext;
			}
			pEdge = pEdge->m_pNext;
		}

		SharedPhysics::C_ConstraintTwin* pTwin = m_GraphNode.m_pPersistentTwinConstraints;
		while(pTwin)
		{
			//pEdge = pTwin->m_pEdge;
			pEdge = SharedPhysics::C_ConstraintEdge::GetConstraintEdge(pTwin);
			SharedPhysics::C_ConstraintPoint* pPoint(pEdge->m_pFirstConstraint);
			while(pPoint)
			{
				pPoint->m_pConstraint->ApplyBodyOffset(this, elementOffset);
				pPoint->m_pConstraint->ZeroWarmStartValues();
				pPoint = pPoint->m_pNext;
			}
			pTwin = pTwin->m_pNext;
		}
	}


#endif


};
