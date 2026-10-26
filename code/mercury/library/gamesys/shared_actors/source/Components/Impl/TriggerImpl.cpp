#include "SharedActorsPch.h"

#include <shared_actors/components/impl/TriggerImpl.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorList.h>
#include <game_context/PhysicsContext.h>
#include <gravity/sdk/PhysicsSDK.h>
#include <gravity/sdk/ShapeDescriptors.h>
#include <gravity/triggers/TriggerEvent.h>
#include <gravity/triggers/TriggerSystem.h>
#include <gravity/sdk/ShapeDescriptors.h>
#include <misc_shared/messaging/MessageTypeList.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/Components/Messages/TriggerMessages.h>

// Debug Variables
Debug::Bool 	g_DebugTriggersUseGravity			("ActorDebug.UseGravityTriggers",				true);
Debug::Bool 	g_DebugBoxTriggerComponentRenderDebug		("ActorDebug.Components.BoxTrigger.DebugRender.Enable",				false);
Debug::Bool 	g_DebugSphereTriggerComponentRenderDebug	("ActorDebug.Components.SphereTrigger.DebugRender.Enable",			false);

#define GRAVITY_STACK_SIZE 1024

namespace Components
{

C_VolumeTrigger::C_VolumeTrigger() :
	I_VolumeTrigger(),m_pTriggerLayer(NULL),m_pTrigger(NULL)
{
	// intentionally blank
}

C_VolumeTrigger::~C_VolumeTrigger()
{
	if(m_pTriggerLayer && m_pTrigger && m_pTrigger->m_pUserData)
	{
		m_pTriggerLayer->RemoveTrigger(m_pTrigger);
		delete (Actor::C_ActorHandle *)m_pTrigger->m_pUserData;
	}
	SAFE_DELETE_PTR(m_pTrigger);
}

bool C_VolumeTrigger::InitialiseImpl()
{
	const I_VolumeTriggerCreate* pCreationData = GetCreationData();

	if (pCreationData->TestFlag(I_VolumeTriggerCreate::E_Flag_HandleEnterExit))
		SetFlag(E_Flag_HandleEnterExit);

	if (pCreationData->TestFlag(I_VolumeTriggerCreate::E_Flag_HandleCrossing))
		SetFlag(E_Flag_HandleCrossing);

	if (pCreationData->TestFlag(I_VolumeTriggerCreate::E_Flag_OneShot))
		SetFlag(E_Flag_OneShot);

	if (!pCreationData->TestFlag(I_VolumeTriggerCreate::E_Flag_StartInactive))
		SetFlag(E_Flag_Active);

	return true;
}

bool C_VolumeTrigger::CreateImpl()
{
	SetMatrix(GetActor()->GetMatrix());
	return true;
}

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_VolumeTrigger)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_GameActorMsg_Activate, ActorMsg::E_GameActorMsg_Deactivate)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_GameActorMsg_SetTransform, ActorMsg::E_GameActorMsg_AnimateTransform)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_TriggerMsg_Begin, ActorMsg::E_TriggerMsg_End)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_VolumeTrigger, I_VolumeTrigger)

bool C_VolumeTrigger::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_SetTransform:
		SetMatrix(Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage)->Transform().Matrix());
		break;

	case ActorMsg::E_GameActorMsg_AnimateTransform:
		SetMatrix(Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage)->Transform().Matrix());
		break;

	case ActorMsg::E_GameActorMsg_Activate:
		if(TestFlag(E_Flag_Active) == false)
		{
			SetFlag(E_Flag_Active);
			if(m_pTriggerLayer && m_pTrigger)
			{
				U8 data[GRAVITY_STACK_SIZE * sizeof(void*)];
				SharedPhysics::C_SafeStack<true> stack(data, GRAVITY_STACK_SIZE);

				m_pTriggerLayer->AddTrigger(m_pTrigger);
				m_pTrigger->SetMatrix(m_pTrigger->GetMatrix(), stack);
			}
		}
		break;

	case ActorMsg::E_GameActorMsg_Deactivate:
		if(TestFlag(E_Flag_Active) == true)
		{
			ClearFlag(E_Flag_Active);
			if(m_pTriggerLayer && m_pTrigger)
				m_pTriggerLayer->RemoveTrigger(m_pTrigger);
		}
		break;

	case ActorMsg::E_TriggerMsg_AddVolume:
		m_AdditionalVolumes.PushBack(Messaging::MessageCast<ActorMsg::C_TriggerAddVolumeMsg>(pMessage)->GetActorHandle());
		break;

	case ActorMsg::E_TriggerMsg_SetMask:
		{
			Collision::C_CollisionFilter filter;	

			filter.SetCollisionFlags(0xffffffff);
			filter.SetCollisionMask(Messaging::MessageCast<ActorMsg::C_TriggerSetMaskMsg>(pMessage)->GetValue());

			// Set trigger collision filter
			if(m_pTrigger && m_pTrigger->m_pTriggerShape)
				m_pTrigger->m_pTriggerShape->SetFilter(&filter);

		}
		break;

	default:
		return I_VolumeTrigger::OnMessage(pMessage);
	}

	return true;
}

// Check additional volumes to see if the point lies within
const bool C_VolumeTrigger::IsInsideAdditionalVolumes(const Maths::C_Vertex& point) const
{
	Btl::Vector<Actor::C_ActorHandle>::ConstIterator it = m_AdditionalVolumes.Begin();

	while(it!=m_AdditionalVolumes.End())
	{
		const Actor::C_Actor *pVolumeActor = GetActorList()->GetActorPtr(*it);
		if(pVolumeActor)
		{
			const I_VolumeTrigger *pVolumeTriggerIF = (const I_VolumeTrigger *)pVolumeActor->QueryInterface(E_TriggerIF_Volume);

			if(pVolumeTriggerIF && pVolumeTriggerIF->IsInside(point))
				return true;
		}
		++it;
	}

	return false;
}


C_BoxTrigger::C_BoxTrigger() :
	C_VolumeTrigger(), 
	m_InverseMatrix(Maths::Constants<Maths::HMat43>::Identity())
{
	// intentionally blank
}

C_BoxTrigger::~C_BoxTrigger()
{
	// intentionally blank
}

bool C_BoxTrigger::CreateImpl()
{
	if(!C_VolumeTrigger::CreateImpl())
		return false;

	Gravity::C_Scene *pScene = GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene();

	if(g_DebugTriggersUseGravity && pScene && pScene->GetTriggerSystem())
	{
		m_pTriggerLayer = pScene->GetTriggerSystem()->GetLayer(GetCreationData()->m_TriggerLayerIndex);
		if(m_pTriggerLayer)
		{
			U8 data[GRAVITY_STACK_SIZE * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, GRAVITY_STACK_SIZE);
			Collision::C_CollisionFilter m_Filter;	// TODO: figure this out

			m_Filter.SetCollisionFlags(0xffffffff);
			m_Filter.SetCollisionMask(GetCreationData()->m_TriggerMask);

			Gravity::C_BoxDesc boxDesc;
			boxDesc.m_HalfExtents = GetCreationData()->m_Size * 0.5f;

			// set up trigger
			m_pTrigger = new Gravity::C_Trigger;
			m_pTrigger->m_pTriggerShape = boxDesc.CreateShape(&m_Filter);	
			m_pTrigger->m_pUserData = (void *)new Actor::C_ActorHandle(GetActorHandle());

			// add trigger to layer
			if(TestFlag(E_Flag_Active) == true)
				m_pTriggerLayer->AddTrigger(m_pTrigger);
		}

		SetMatrix(GetActor()->GetMatrix());
	}

	return true;
}

bool C_BoxTrigger::IsInside(const Maths::C_Vertex& point) const
{
	Maths::C_Vertex localPos = point * m_InverseMatrix;
	const Maths::HVec3& size = GetCreationData()->m_Size;

	if (localPos.X() < 0.0f || localPos.X() > size.X())
		return false;

	if (localPos.Y() < 0.0f || localPos.Y() > size.Y())
		return false;

	if (localPos.Z() < 0.0f || localPos.Z() > size.Z())
		return false;

	return true;
}

bool C_BoxTrigger::GetPositionInside(const Maths::C_Vertex& point, Maths::C_Vertex& position) const
{
	const Maths::HVec3& size = GetCreationData()->m_Size;
	position = point * m_InverseMatrix;

	if (position.X() < 0.0f || position.X() > size.X())
		return false;

	if (position.Y() < 0.0f || position.Y() > size.Y())
		return false;

	if (position.Z() < 0.0f || position.Z() > size.Z())
		return false;

	return true;
}

bool C_BoxTrigger::CheckLineIntersection(const Maths::C_Vertex &lineStart, const Maths::C_Vertex &lineEnd) const
{
	Maths::C_Line line(lineStart * m_InverseMatrix, lineEnd * m_InverseMatrix);
	Maths::C_AxisBox box(Maths::Constants<Maths::HVec3>::Zero(), GetCreationData()->m_Size);
	Maths::HF32 enter,exit;
	return Maths::Intersect(line,box,enter,exit);
}

void C_BoxTrigger::SetMatrix(const Maths::HMat43& matrix)
{
	m_InverseMatrix = GetCreationData()->m_Matrix * matrix;
	m_InverseMatrix = m_InverseMatrix.InverseRotTrans();

	//if(m_pTriggerLayer)
	{
		U8 data[GRAVITY_STACK_SIZE * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, GRAVITY_STACK_SIZE);

		const Maths::HMat43 mat = GetCreationData()->m_Matrix * matrix;

		//m_pTriggerLayer->SetTriggerMatrix(,Maths::HMat43(mat.Rotation(), (GetCreationData()->m_Size * 0.5f) * mat) , stack);
		m_pTrigger->SetMatrix(Maths::HMat43(mat.Rotation(), (GetCreationData()->m_Size * 0.5f) * mat) ,stack);
	}
}

#ifndef CFG_GOLD
void C_BoxTrigger::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_DebugBoxTriggerComponentRenderDebug)
		return;

	Maths::HMat43 worldMat = m_InverseMatrix.InverseRotTrans();
	Maths::C_AxisBox box(Maths::Constants<Maths::HVec3>::Zero(), static_cast<Maths::HVec3>(GetCreationData()->m_Size));
	bool InTrigger = false;

	//debugRenderPtr->Draw3dBox(box, worldMat, Maths::RGBA(255,0,0));
	debugRenderPtr->Draw3dMatrix(worldMat);

	for(C_TrackedActors::ConstIterator it = m_TrackedActors.Begin(); it!=m_TrackedActors.End();++it)
	{
		Maths::RGBA col = Maths::RGBA(255,0,0);
		if(it->GetValue().m_InTrigger)
		{
			InTrigger = true;
			col = Maths::RGBA(0,255,0);
		}
	
		debugRenderPtr->Draw3dCross(it->GetValue().m_Position, 0.5f, col);
	}

	if(TestFlag(E_Flag_Active))
	{
		if(InTrigger)
			debugRenderPtr->Draw3dBox(box, worldMat, Maths::RGBA(0,255,0));
		else
			debugRenderPtr->Draw3dBox(box, worldMat, Maths::RGBA(255,0,0));
	}
	else
		debugRenderPtr->Draw3dBox(box, worldMat, Maths::RGBA(64,64,64));

}
#endif

void C_BoxTrigger::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_InverseMatrix;
}

void C_BoxTrigger::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_InverseMatrix;
}

void C_BoxTrigger::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_VolumeTrigger::SerialiseForSave(stream);
	Serialise(stream);
}

void C_BoxTrigger::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_VolumeTrigger::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_BoxTrigger::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_VolumeTrigger::SerialiseForSync(stream);
	Serialise(stream);
}

void C_BoxTrigger::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_VolumeTrigger::DeserialiseForSync(stream);
	Deserialise(stream);
}




C_SphereTrigger::C_SphereTrigger() :
	C_VolumeTrigger(), 
	m_Sphere(Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero()), 
	m_RadiusSquared(0.0f)
{
	// intentionally blank
}

C_SphereTrigger::~C_SphereTrigger()
{
	// intentionally blank
}

bool C_SphereTrigger::IsInside(const Maths::C_Vertex& point) const
{
	const F32 distSqrd = (point - m_Sphere.Centre()).LengthSquared();
	return (distSqrd < m_RadiusSquared);
}

bool C_SphereTrigger::GetPositionInside(const Maths::C_Vertex& point, Maths::C_Vertex& position) const
{
	Maths::C_Vector dir = (point - m_Sphere.Centre());
	const F32 distSqrd = dir.LengthSquared();
	position = (Maths::C_Vertex)dir;
	return (distSqrd < m_RadiusSquared);
}

bool C_SphereTrigger::CheckLineIntersection(const Maths::C_Vertex &lineStart, const Maths::C_Vertex &lineEnd) const
{
	Maths::C_Line line(lineStart, lineEnd);
	Maths::HF32 enter, exit;
	return Maths::Intersect(line, m_Sphere, enter, exit);
}

bool C_SphereTrigger::InitialiseImpl()
{
	if (!C_VolumeTrigger::InitialiseImpl())
		return false;

	const C_SphereTriggerCreate* pCreationData = GetCreationData();
	m_RadiusSquared = pCreationData->m_Sphere.Radius() * pCreationData->m_Sphere.Radius();

	return true;
}

bool	C_SphereTrigger::CreateImpl()
{
	if(!C_VolumeTrigger::CreateImpl())
		return false;

	Gravity::C_Scene *pScene = GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene();

	// Create a sphere trigger in the physics trigger system
	if(g_DebugTriggersUseGravity && pScene && pScene->GetTriggerSystem())
	{
		m_pTriggerLayer = pScene->GetTriggerSystem()->GetLayer(GetCreationData()->m_TriggerLayerIndex);
		if(m_pTriggerLayer)
		{
			U8 data[GRAVITY_STACK_SIZE * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, GRAVITY_STACK_SIZE);
			Collision::C_CollisionFilter m_Filter;	// TODO: figure this out

			m_Filter.SetCollisionFlags(0xffffffff);
			m_Filter.SetCollisionMask(GetCreationData()->m_TriggerMask);

			Gravity::C_SphereDesc sphereDesc;
			sphereDesc.m_Radius = GetCreationData()->m_Sphere.Radius();

			// set up trigger
			m_pTrigger = new Gravity::C_Trigger;
			m_pTrigger->m_pTriggerShape = sphereDesc.CreateShape(&m_Filter);	
			m_pTrigger->m_pUserData = (void *)new Actor::C_ActorHandle(GetActorHandle());

			// add trigger to layer
			if(TestFlag(E_Flag_Active) == true)
				m_pTriggerLayer->AddTrigger(m_pTrigger);
		}

		SetMatrix(GetActor()->GetMatrix());
	}

	return true;
}
void C_SphereTrigger::SetMatrix(const Maths::HMat43& matrix)
{
	const C_SphereTriggerCreate* pCreationData = GetCreationData();
	m_Sphere = Maths::C_Sphere(pCreationData->m_Sphere.Centre() * matrix, pCreationData->m_Sphere.Radius());

	//if(m_pTriggerLayer)
	{
		U8 data[GRAVITY_STACK_SIZE * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, GRAVITY_STACK_SIZE);
		// work out position
		const Maths::HMat43 mat(GetActor()->GetMatrix().Rotation(), GetCreationData()->m_Sphere.Centre() * matrix);

		//m_pTriggerLayer->SetTriggerMatrix(m_pTrigger, mat, stack);
		m_pTrigger->SetMatrix(mat ,stack);
	}
}

#ifndef CFG_GOLD
void C_SphereTrigger::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_DebugSphereTriggerComponentRenderDebug)
		return;
	bool InTrigger = false;

	for(C_TrackedActors::ConstIterator it = m_TrackedActors.Begin(); it!=m_TrackedActors.End();++it)
	{
		Maths::RGBA col = Maths::RGBA(255,0,0);
		if(it->GetValue().m_InTrigger)
		{
			InTrigger = true;
			col = Maths::RGBA(0,255,0);
		}

		debugRenderPtr->Draw3dCross(it->GetValue().m_Position, 0.5f, col);
	}

	if(TestFlag(E_Flag_Active))
	{
		if(InTrigger)
			debugRenderPtr->Draw3dSphere(m_Sphere, Maths::HMat43(), Maths::RGBA(0,255,0));
		else
			debugRenderPtr->Draw3dSphere(m_Sphere, Maths::HMat43(), Maths::RGBA(255,0,0));
	}
	else
		debugRenderPtr->Draw3dSphere(m_Sphere, Maths::HMat43(), Maths::RGBA(64,64,64));

}
#endif

void C_SphereTrigger::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_Sphere;
	stream << m_RadiusSquared;
}

void C_SphereTrigger::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_Sphere;
	stream >> m_RadiusSquared;
}

void C_SphereTrigger::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_VolumeTrigger::SerialiseForSave(stream);
	Serialise(stream);
}

void C_SphereTrigger::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_VolumeTrigger::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_SphereTrigger::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_VolumeTrigger::SerialiseForSync(stream);
	Serialise(stream);
}

void C_SphereTrigger::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_VolumeTrigger::DeserialiseForSync(stream);
	Deserialise(stream);
}


// Trigger Source Component
// ------------------------

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_TriggerSourceVolume)
ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_GameActorMsg_SetTransform, ActorMsg::E_GameActorMsg_AnimateTransform)
ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_TriggerMsg_Begin, ActorMsg::E_TriggerMsg_End)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_TriggerSourceVolume, I_TriggerSourceVolume)


C_TriggerSourceVolume::C_TriggerSourceVolume()
{

}

C_TriggerSourceVolume::~C_TriggerSourceVolume()
{
	if(m_pTriggerActor && m_pTriggerActor->m_pUserData)
	{
		m_pTriggerLayer->RemoveTriggerActor(m_pTriggerActor);
		delete (Actor::C_ActorHandle *)m_pTriggerActor->m_pUserData;
	}
	SAFE_DELETE_PTR(m_pTriggerActor);
}

bool	C_TriggerSourceVolume::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_SetTransform:
		SetMatrix(Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage)->Transform().Matrix());
		return true;

	case ActorMsg::E_GameActorMsg_AnimateTransform:
		SetMatrix(Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage)->Transform().Matrix());
		return true;

	case ActorMsg::E_TriggerMsg_SetMask:
		{
			Collision::C_CollisionFilter filter;	

			filter.SetCollisionFlags(Messaging::MessageCast<ActorMsg::C_TriggerSetMaskMsg>(pMessage)->GetValue());
			filter.SetCollisionMask(0xffffffff);

			// Set trigger collision filter
			if(m_pTriggerActor && m_pTriggerActor->m_pShape)
				m_pTriggerActor->m_pShape->SetFilter(&filter);

		}
		return true;

	default:
		return I_TriggerSourceVolume::OnMessage(pMessage);
	}
}

void C_TriggerSourceVolume::Serialise(Sys::C_ContextualStream& stream) const
{
	// TODO: Serialise what triggers the trigger actor is in
}

void C_TriggerSourceVolume::Deserialise(Sys::C_ContextualStream& stream)
{
	// TODO: Deserialise what triggers the trigger actor is in
}

void C_TriggerSourceVolume::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_TriggerSourceVolume::SerialiseForSave(stream);
	Serialise(stream);
}

void C_TriggerSourceVolume::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_TriggerSourceVolume::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_TriggerSourceVolume::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_TriggerSourceVolume::SerialiseForSync(stream);
	Serialise(stream);
}

void C_TriggerSourceVolume::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_TriggerSourceVolume::DeserialiseForSync(stream);
	Deserialise(stream);
}

// Trigger Source Sphere
C_TriggerSourceSphere::C_TriggerSourceSphere()
{

}

C_TriggerSourceSphere::~C_TriggerSourceSphere()
{

}

bool C_TriggerSourceSphere::InitialiseImpl()
{
	if(!C_TriggerSourceVolume::InitialiseImpl())
		return false;

	return true;
}

bool C_TriggerSourceSphere::CreateImpl()
{
	if(!C_TriggerSourceVolume::CreateImpl())
		return false;

	Gravity::C_Scene *pScene = GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene();

	if(g_DebugTriggersUseGravity && pScene && pScene->GetTriggerSystem())
	{
		m_pTriggerLayer = pScene->GetTriggerSystem()->GetLayer(GetCreationData()->m_TriggerLayerIndex);
		if(m_pTriggerLayer)
		{
			U8 data[GRAVITY_STACK_SIZE * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, GRAVITY_STACK_SIZE);
			Collision::C_CollisionFilter m_Filter;	// TODO: figure this out

			m_Filter.SetCollisionFlags(GetCreationData()->m_TriggerFlags);
			m_Filter.SetCollisionMask(0xffffffff);

			m_pTriggerActor = new Gravity::C_TriggerActor;
			m_pTriggerActor->m_pShape = new Gravity::C_TriggerSphere(GetCreationData()->m_Sphere.Radius(), &m_Filter);
			m_pTriggerActor->m_pUserData = (void *)new Actor::C_ActorHandle(GetActorHandle());

			m_pTriggerLayer->AddTriggerActor(m_pTriggerActor);
		}
	}
	return true;
}

void	C_TriggerSourceSphere::SetMatrix(const Maths::HMat43& matrix)
{
	if(m_pTriggerLayer)
	{
		U8 data[GRAVITY_STACK_SIZE * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, GRAVITY_STACK_SIZE);
		Maths::HMat43 mat(matrix.Rotation(),GetCreationData()->m_Sphere.Centre() * matrix);
		//m_pTriggerLayer->SetTriggerActorMatrix(m_pTriggerActor,mat , stack);
		m_pTriggerActor->SetMatrix(mat ,stack);

	}
}

// Trigger Source Box
C_TriggerSourceBox::C_TriggerSourceBox()
{

}

C_TriggerSourceBox::~C_TriggerSourceBox()
{

}

bool C_TriggerSourceBox::InitialiseImpl()
{
	if(!C_TriggerSourceVolume::InitialiseImpl())
		return false;

	return true;
}

bool C_TriggerSourceBox::CreateImpl()
{
	if(!C_TriggerSourceVolume::CreateImpl())
		return false;

	Gravity::C_Scene *pScene = GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene();

	if(g_DebugTriggersUseGravity && pScene && pScene->GetTriggerSystem())
	{
		m_pTriggerLayer = pScene->GetTriggerSystem()->GetLayer(GetCreationData()->m_TriggerLayerIndex);
		if(m_pTriggerLayer)
		{
			U8 data[GRAVITY_STACK_SIZE * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, GRAVITY_STACK_SIZE);
			Collision::C_CollisionFilter m_Filter;	// TODO: figure this out

			m_Filter.SetCollisionFlags(GetCreationData()->m_TriggerFlags);
			m_Filter.SetCollisionMask(0xffffffff);

			m_pTriggerActor = new Gravity::C_TriggerActor;
			m_pTriggerActor->m_pShape = new Gravity::C_TriggerBox(GetCreationData()->m_Size * 0.5f, &m_Filter);
			m_pTriggerActor->m_pUserData = (void *)new Actor::C_ActorHandle(GetActorHandle());

			m_pTriggerLayer->AddTriggerActor(m_pTriggerActor);
		}
	}
	return true;
}

void	C_TriggerSourceBox::SetMatrix(const Maths::HMat43& matrix)
{
	if(m_pTriggerLayer)
	{
		U8 data[GRAVITY_STACK_SIZE * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, GRAVITY_STACK_SIZE);

		const Maths::HMat43 mat = GetCreationData()->m_Matrix * matrix;

		//m_pTriggerLayer->SetTriggerActorMatrix(m_pTriggerActor,Maths::HMat43(mat.Rotation(), (GetCreationData()->m_Size * 0.5f) * mat) , stack);
		m_pTriggerActor->SetMatrix(mat ,stack);
	}
}

} // namespace Components
