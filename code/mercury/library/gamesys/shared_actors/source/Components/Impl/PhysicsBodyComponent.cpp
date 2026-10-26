#include "SharedActorsPch.h"
#include <shared_actors/Components/Impl/PhysicsBodyComponent.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorMessages.h>
#include <game_context/PhysicsContext.h>
#include <game_resource/ModelData.h>
#include <gravity/narrowphase/Primitive.h>
#include <gravity/narrowphase/Shapes.h>
#include <Gravity/sdk/Scene.h>
#include <Gravity/sdk/BodyHelper.h>
#include <Gravity/sdk/KinematicController.h>
#include <Gravity/sdk/ShapeDescriptors.h>
#include <gravity/importer/Importer.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <misc_shared/messaging/MessageTypeList.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/PhysicsBodyComponentCreate.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <shared_actors/Components/Interfaces/I_ElementManager.h>
#include <shared_actors/Components/Interfaces/I_ModelResource.h>
#include <shared_actors/SharedActorUtils.h>
#include <shared_actors/Utility/PhysicsBodyUserData.h>
#include <system/perf.h>

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

Debug::Bool g_PhysicsBodyDebugRender("ActorDebug.Components.PhysicsBody.DebugRender.Enable", false);
Debug::Bool g_PhysicsBodyShowRenderMatrix("ActorDebug.Components.PhysicsBody.DebugRender.ShowRenderMatrix", false);

Debug::Float g_PhysicsBodyMaxAngularSpeed("ActorDebug.Components.PhysicsBody.MaxAngularSpeed", 20.0f);
Debug::Float g_PhysicsBodyAngularDampingCoeff("ActorDebug.Components.PhysicsBody.AngularDampingCoeff", 0.99f);

Debug::Float g_PhysicsBuoyancyScale("ActorDebug.Components.PhysicsBody.Buoyancy.Scale", 30.f, Debug::Range(0.f, 100.f));
Debug::Float g_PhysicsMaxBuoyancy("ActorDebug.Components.PhysicsBody.Buoyancy.MaxScale", 4.f, Debug::Range(0.f, 100.f));

Debug::Float g_PhysicsLinearDragScale("ActorDebug.Components.PhysicsBody.Buoyancy.LinearDragScale", 1.f, Debug::Range(0.f, 100.f));
Debug::Float g_PhysicsAngularDragScale("ActorDebug.Components.PhysicsBody.Buoyancy.AngularDragScale", 0.5f, Debug::Range(0.f, 100.f));

Debug::Bool g_PhysicsBodyLogShapeMapping("ActorDebug.Components.PhysicsBody.LogShapeMapping", false);

Debug::Bool gPbcAllowContinuousShapes("ActorDebug.Components.PhysicsBody.AllowContinuousShapes", true);

using namespace Maths;


namespace Components
{

// Helper fns:
static void DeleteShapeDesc(Gravity::C_ShapeDesc*& shapeDesc)
{
	if (shapeDesc)
	{
		if (shapeDesc->m_Type == Collision::E_Continuous_Shape)
		{
			Gravity::C_ContinuousShapeDesc* contDesc = static_cast<Gravity::C_ContinuousShapeDesc*>(shapeDesc);
			delete contDesc->m_pDiscreteShape;
			delete contDesc->m_pContinuousShape;
		}

		delete shapeDesc;
	}
}


// Creates a continuous shape descriptor from the given desc.
void MakeContinuousShapeDesc(C_PhysicsBodyShapeDesc& shapeDesc)
{
	if (!gPbcAllowContinuousShapes)
		return;

	BIZ_ASSERT(shapeDesc.GetShapeDesc()->m_Type != Collision::E_Continuous_Shape && "shape already continuous");

	Gravity::C_ContinuousShapeDesc* contDesc = new Gravity::C_ContinuousShapeDesc;
	shapeDesc.TakeOwnership(contDesc);
	contDesc->m_pDiscreteShape = shapeDesc.GetShapeDesc();

	Gravity::C_SphereDesc* desc = new Gravity::C_SphereDesc;
	shapeDesc.TakeOwnership(desc);
	desc->m_Mass = shapeDesc.GetShapeDesc()->m_Mass;
	const Maths::HVec3& extents = shapeDesc.GetShapeDesc()->GetBoundingBox().extents;
	desc->m_Radius = Maths::Max(extents.X(), Maths::Max(extents.Y(), extents.Z()));
	desc->m_Margin = shapeDesc.GetShapeDesc()->m_Margin;
	desc->m_Epsilon = shapeDesc.GetShapeDesc()->m_Epsilon;
	contDesc->m_pContinuousShape = desc;

	shapeDesc.Set(shapeDesc.GetMass(), contDesc);
}






C_PhysicsBodyShapeDesc::C_PhysicsBodyShapeDesc()
	: m_Mass(0.0f)
	, m_pShapeDesc(NULL)
{
	for (U32 i = 0; i < kMaxOwnedShapeDescs; ++i)
	{
		m_ppOwnedShapeDescs[i] = NULL;
	}
}

C_PhysicsBodyShapeDesc::C_PhysicsBodyShapeDesc(const C_PhysicsBodyShapeDesc& copy)
	: m_Mass(copy.m_Mass)
	, m_pShapeDesc(copy.m_pShapeDesc)
{
	for (U32 i = 0; i < kMaxOwnedShapeDescs; ++i)
	{
		m_ppOwnedShapeDescs[i] = copy.m_ppOwnedShapeDescs[i];
	}
}

C_PhysicsBodyShapeDesc::~C_PhysicsBodyShapeDesc()
{
}

C_PhysicsBodyShapeDesc& C_PhysicsBodyShapeDesc::operator = (const C_PhysicsBodyShapeDesc& copy)
{
	m_Mass = copy.m_Mass;
	m_pShapeDesc = copy.m_pShapeDesc;

	for (U32 i = 0; i < kMaxOwnedShapeDescs; ++i)
	{
		m_ppOwnedShapeDescs[i] = copy.m_ppOwnedShapeDescs[i];
	}

	return *this;
}

void C_PhysicsBodyShapeDesc::Clear()
{
	m_Mass = 0.0f;
	m_pShapeDesc = NULL;

	for (U32 i = 0; i < kMaxOwnedShapeDescs; ++i)
	{
		m_ppOwnedShapeDescs[i] = NULL;
	}
}

void C_PhysicsBodyShapeDesc::Set(const F32 mass, Gravity::C_ShapeDesc* pShapeDesc)
{
	m_Mass = mass;
	m_pShapeDesc = pShapeDesc;
}

bool C_PhysicsBodyShapeDesc::TakeOwnership(Gravity::C_ShapeDesc* pShapeDesc)
{
	if (pShapeDesc != NULL)
	{
		for (U32 i = 0; i < kMaxOwnedShapeDescs; ++i)
		{
			if (m_ppOwnedShapeDescs[i] == NULL)
			{
				m_ppOwnedShapeDescs[i] = pShapeDesc;
				return true;
			}
		}
	}

	return false;
}

void C_PhysicsBodyShapeDesc::DestroyOwned()
{
	for (U32 i = 0; i < kMaxOwnedShapeDescs; ++i)
	{
		delete m_ppOwnedShapeDescs[i];
		m_ppOwnedShapeDescs[i] = NULL;
	}
}



//////////////////////////////////////////////////////////////////////////
// Gravity Body Component
//
C_PhysicsBody::C_PhysicsBody()
	: m_pCollideBody(NULL)
	, m_IsEnabled(true)
	, m_BodyActive(true)
	, m_KinematicController(NULL)
	, m_PhysicsScene(NULL)
	, m_pUserData(NULL)
{
}

C_PhysicsBody::~C_PhysicsBody()
{
	DestroyBody();
	m_ShapeDesc.DestroyOwned();

	if (m_pUserData != NULL)
	{
		Actors::C_PBCUserData::Destroy(m_pUserData);
		m_pUserData = NULL;
	}
}


void C_PhysicsBody::DestroyBody()
{
	if (m_KinematicController)
	{
		ReleaseKinematicController(m_KinematicController);
		m_KinematicController = NULL;
	}

	if (CollideBody())
	{
		BIZ_ASSERT_MSG((!CollideBody()->GetScene() || CollideBody()->GetScene()==m_PhysicsScene), "Body has invalid scene!");
		CollideBody()->SetUserData(NULL);
		PhysicsScene()->RemoveBody(CollideBody());
		m_pCollideBody = NULL;
	}
}


bool C_PhysicsBody::InitialiseImpl()
{
	m_CreationMatrix = GetCreationData()->m_Matrix;
	m_BodyType = GetCreationData()->m_BodyType;

	return true;
}


bool C_PhysicsBody::CreateImpl()
{
	const C_PhysicsBodyCreateInfo* pCreationData = GetCreationData();

	m_PhysicsScene = GetActor()->GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene();
	BIZ_ASSERT(m_PhysicsScene);

	//AddSubscriber(E_PhysicsBodyMsg_CollisionEvent);
	//AddSubscriber(GameMessaging::E_GameMsg_Update);

	m_pUserData = Actors::C_PBCUserData::Create(GetActor(), this);
	{
		GamePhysics::C_CollisionFilter collisionFilter(0, pCreationData->m_MinCollisionImpulse);

		switch (pCreationData->m_ContactEventLevel)
		{
		case C_PhysicsBodyCreateInfo::E_ContactEventLevel_All:
			collisionFilter.AddContactType(SharedPhysics::RESTING);
			// fall through
		case C_PhysicsBodyCreateInfo::E_ContactEventLevel_NotResting:
			collisionFilter.AddContactType(SharedPhysics::SLIDING);
			collisionFilter.AddContactType(SharedPhysics::SEPERATING);
			// fall through
		case C_PhysicsBodyCreateInfo::E_ContactEventLevel_OnlyCollision:
			collisionFilter.AddContactType(SharedPhysics::COLLIDING);
			// fall through
		case C_PhysicsBodyCreateInfo::E_ContactEventLevel_None:
			break;

		case C_PhysicsBodyCreateInfo::E_ContactEventLevel_Count:
		default:
			BIZ_ASSERT_FAIL(DBSTR " Unknown E_ContactEventLevel");
			break;
		}

		if (pCreationData->m_bFluidEvents)
			collisionFilter.AcceptFluidEvents();
		else
			collisionFilter.RejectFluidEvents();

		m_pUserData->SetCollisionFilter(collisionFilter);
	}

	CreateBody();

	return true;
}


void C_PhysicsBody::InternalDependenciesCreatedImpl()
{
	I_PhysicsBody::InternalDependenciesCreatedImpl();

	if (!CollideBody())
	{
		OnShapeChanged(GetCreationData()->m_OverrideCollisionFilterIndex);
	}
}


void C_PhysicsBody::ExternalDependenciesCreatedImpl()
{
	I_PhysicsBody::ExternalDependenciesCreatedImpl();

	if (CollideBody())
	{
		const C_PhysicsBodyTransformChanged transformChangedMsg(RenderMatrix());
		SendMessageToActor(&transformChangedMsg);
	}
}


Gravity::C_KinematicController* C_PhysicsBody::AddKinematicController(Gravity::C_Body* body)
{
	Gravity::C_KinematicController* controller = new Gravity::C_KinematicController(body);
	PhysicsScene()->AddKinematicController(controller);
	return controller;
}


void C_PhysicsBody::ReleaseKinematicController(Gravity::C_KinematicController*& controller)
{
	if (controller)
	{
		PhysicsScene()->RemoveKinematicController(controller);
		delete controller;
		controller = NULL;
	}
}


const Collision::C_CollisionFilter& C_PhysicsBody::CollisionFilter() const
{
	if (CollideBody())
		return *CollideBody()->GetShape()->GetFilter();
	else
		return GetCreationData()->m_CollisionFilter;
}


Collision::C_CollisionFilter& C_PhysicsBody::CollisionFilter()
{
	return *const_cast<Collision::C_CollisionFilter*>(CollideBody()->GetShape()->GetFilter());
}


ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_PhysicsBody)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(Actor::E_ActorMsg_RemoveCollision, Actor::E_ActorMsg_PhysicsBodyEvent)
	ACTOR_COMPONENT_MESSAGE_TYPES_SINGLE(ActorMsg::E_MovementNotificationMsg_MovementComplete)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_GameActorMsg_Enable, ActorMsg::E_GameActorMsg_Disable)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_GameActorMsg_SetTransform, ActorMsg::E_GameActorMsg_AnimateTransform)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(E_PhysicsBodyMsg_Begin, E_PhysicsBodyMsg_End)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_PhysicsBody, I_PhysicsBody)


bool C_PhysicsBody::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		case Actor::E_ActorMsg_PhysicsBodyEvent:
		{
			const C_PhysicsBodyEventMsg* msg = Messaging::MessageCast<C_PhysicsBodyEventMsg>(pMessage);
			OnPhysicsBodyEvent(msg->m_BodyEvent);
		}	break;

//		case E_GameActorMsg_UpdatePhysicsWorld:
		case E_PhysicsBodyMsg_SetCollideMaterialMask:
		{
			static S32 count = 0;
			if (count++ < 8)
				LOGWARN(Actor::GetDebugLogGeneral(), "Gravity body component received old physics system message");	// Old system :P
		}	break;

		case GameMessaging::E_GameMsg_Update:
			Update(Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue());
			break;

		case ActorMsg::E_GameActorMsg_Enable:
		case E_PhysicsBodyMsg_WorldInsert:
			if (!m_IsEnabled && CollideBody() && !CollideBody()->GetScene())
			{
				PhysicsScene()->AddBody(CollideBody());
			}
			m_IsEnabled = true;
			break;

		case ActorMsg::E_GameActorMsg_Disable:
		case E_PhysicsBodyMsg_WorldRemove:
			if (m_IsEnabled && CollideBody() && CollideBody()->GetScene())
			{
				PhysicsScene()->TemporarilyRemoveBody(CollideBody());
			}
			m_IsEnabled = false;
			break;

		case E_PhysicsBodyMsg_Deactivate:
			if (CollideBody())
			{
				CollideBody()->Deactivate();
				RemoveSubscriber(GameMessaging::E_GameMsg_Update);
			}
			break;

		case E_PhysicsBodyMsg_Activate:
			if (CollideBody())
			{
				CollideBody()->Activate();
				AddSubscriber(GameMessaging::E_GameMsg_Update);
			}
			break;

		case E_PhysicsBodyMsg_SetPositionAndOrientation:
			// Sets actual body position (not accounting for render offset).  Use body->ElementMatrix(m) to set pos with render matrix.
			if (CollideBody())
			{
				const C_PhysicsBodySetPositionAndOrientation *msg = Messaging::MessageCast<C_PhysicsBodySetPositionAndOrientation>(pMessage);
				Activate();
				CollideBody()->Position(msg->GetValue().m_Translation);
				CollideBody()->Orientation(msg->GetValue().m_Quaternion);
			}
			break;

		case E_PhysicsBodyMsg_ApplyForce:
			if (CollideBody())
			{
				const C_PhysicsBodyApplyForce* msg = Messaging::MessageCast<C_PhysicsBodyApplyForce>(pMessage);
				Activate();
				// Note: ApplyForce is screwed at the mo, use ApplyImpulse!  CollideBody()->ApplyForce(msg->Force(), msg->WorldPosition());
#ifndef CFG_SPEED
				// Check for large impulses:
				const F32 massFactor = 1000.0f * Mass();
				if (msg->Force().Length() > massFactor)
				{
					LOGWARN(Actor::GetDebugLogGeneral(), "Actor '%s': large impulse applied (mass:%.02f)", GetActor()->GetActorName(), (float)Mass());
					CollideBody()->ApplyImpulse(Maths::HVec3(msg->Force()).Normalised().Normal()*massFactor*0.9f, msg->WorldPosition());
				}
				else
#endif
				{
					CollideBody()->ApplyImpulse(msg->Force(), msg->WorldPosition());
				}
			}
			break;

		case E_PhysicsBodyMsg_ApplyImpulse:
			if (CollideBody())// && BodyType()==ActorPhysics::E_BodyType_Dynamic)
			{
				const C_PhysicsBodyApplyImpulse* msg = Messaging::MessageCast<C_PhysicsBodyApplyImpulse>(pMessage);
				Activate();
#ifndef CFG_SPEED
				// Check for large impulses:
				const F32 massFactor = 1000.0f * Mass();
				if (msg->Impulse().Length() > massFactor)
				{
					LOGWARN(Actor::GetDebugLogGeneral(), "Actor '%s': large impulse applied (mass:%.02f)", GetActor()->GetActorName(), (float)Mass());
					CollideBody()->ApplyImpulse(Maths::HVec3(msg->Impulse().Get()).Normalised().Normal()*massFactor*0.9f, msg->WorldPosition());
				}
				else
#endif
				{
					CollideBody()->ApplyImpulse(msg->Impulse().Get() * Mass(), msg->WorldPosition());
				}
			}
			break;

		case E_PhysicsBodyMsg_ApplyLocalImpulse:
			if (CollideBody())// && BodyType()==ActorPhysics::E_BodyType_Dynamic)
			{
				const C_PhysicsBodyApplyLocalImpulse* msg = Messaging::MessageCast<C_PhysicsBodyApplyLocalImpulse>(pMessage);
				Activate();
				if (msg->Impulse().Get().Length() > 100.0f)
					LOGWARN(Actor::GetDebugLogGeneral(), "LargeImpulse Warning over 100.0 %0.2fx %0.2fy %0.2fz", static_cast<float>( msg->Impulse().Get().X()), static_cast<float>(msg->Impulse().Get().Y()), static_cast<float>(msg->Impulse().Get().Z()) );	
				Maths::HVec3 impulse = msg->Impulse().Get() * Mass();
				CollideBody()->ApplyImpulseLocalSpace(msg->LocalImpulse() ? impulse : impulse * CollideBody()->ElementMatrix().InverseRotTrans().Rotation() , msg->LocalPosition() ? msg->Position() : msg->Position() * CollideBody()->ElementMatrix().InverseRotTrans());
			}
			break;

		case E_PhysicsBodyMsg_AddTorque:
			if (CollideBody())// && BodyType()==ActorPhysics::E_BodyType_Dynamic)
			{
				const C_PhysicsBodyAddTorque *msg = Messaging::MessageCast<C_PhysicsBodyAddTorque>(pMessage);
				Activate();
				CollideBody()->AddTorque(msg->GetValue());
			}
			break;

		case E_PhysicsBodyMsg_SetLinearMomentum:
			if (CollideBody())// && BodyType()==ActorPhysics::E_BodyType_Dynamic)
			{
				const C_PhysicsBodySetLinearMomentum* msg = Messaging::MessageCast<C_PhysicsBodySetLinearMomentum>(pMessage);
				Activate();
				CollideBody()->LinearVelocity(msg->GetValue().Get()/CollideBody()->Mass());
			}
			break;

		case E_PhysicsBodyMsg_SetLinearVelocity:
			if (CollideBody())// && BodyType()==ActorPhysics::E_BodyType_Dynamic)
			{
				const C_PhysicsBodySetLinearVelocity* msg = Messaging::MessageCast<C_PhysicsBodySetLinearVelocity>(pMessage);
				Activate();
				CollideBody()->LinearVelocity(msg->GetValue().Get());
			}
			break;

		case E_PhysicsBodyMsg_SetAngularMomentum:
			if (CollideBody())// && BodyType()==ActorPhysics::E_BodyType_Dynamic)
			{
				const C_PhysicsBodySetAngularMomentum* msg = Messaging::MessageCast<C_PhysicsBodySetAngularMomentum>(pMessage);
				Activate();
				CollideBody()->AngularVelocity(msg->GetValue().Get()/CollideBody()->Mass());
			}
			break;

		case E_PhysicsBodyMsg_SetAngularVelocity:
			if (CollideBody())// && BodyType()==ActorPhysics::E_BodyType_Dynamic)
			{
				const C_PhysicsBodySetAngularVelocity* msg = Messaging::MessageCast<C_PhysicsBodySetAngularVelocity>(pMessage);
				Activate();
				CollideBody()->AngularVelocity(msg->GetValue().Get());
			}
			break;

		case E_PhysicsBodyMsg_SetMaxLinearVelocity:
			if (CollideBody())
			{
				const C_PhysicsBodySetMaxLinearVelocity* msg = Messaging::MessageCast<C_PhysicsBodySetMaxLinearVelocity>(pMessage);
				CollideBody()->SetMaxLinearVelocity(msg->GetValue());
			}	break;

		case E_PhysicsBodyMsg_SetMaxAngularVelocity:
			if (CollideBody())
			{
				const C_PhysicsBodySetMaxAngularVelocity* msg = Messaging::MessageCast<C_PhysicsBodySetMaxAngularVelocity>(pMessage);
				CollideBody()->SetMaxAngularVelocity(msg->GetValue());
			}	break;

		case E_PhysicsBodyMsg_SetLinearDampingFactor:
			if (CollideBody())
			{
				const C_PhysicsBodySetLinearDampingFactor* msg = Messaging::MessageCast<C_PhysicsBodySetLinearDampingFactor>(pMessage);
				CollideBody()->SetLinearDampingCoefficient(msg->GetValue());
			}	break;

		case E_PhysicsBodyMsg_SetAngularDampingFactor:
			if (CollideBody())
			{
				const C_PhysicsBodySetAngularDampingFactor* msg = Messaging::MessageCast<C_PhysicsBodySetAngularDampingFactor>(pMessage);
				CollideBody()->SetAngularDampingCoefficient(msg->GetValue());
			}	break;

		case E_PhysicsBodyMsg_UpdateShape:
			OnShapeChanged(Messaging::MessageCast<C_PhysicsUpdateShapeMsg>(pMessage)->GetValue());
			break;

		case E_PhysicsBodyMsg_MakeStatic:
			if (CollideBody())
			{
				CollideBody()->MakeStatic();
				ReleaseKinematicController(m_KinematicController);
			}
			break;

		case E_PhysicsBodyMsg_MakeDynamic:
			if (CollideBody())
			{
				CollideBody()->Activate();
				ReleaseKinematicController(m_KinematicController);
			}
			break;

		case E_PhysicsBodyMsg_MakeAnimated:
			if (CollideBody())
			{
				CollideBody()->MakeKinematic();
				if (!m_KinematicController)//? && !GetCreationData()->m_IsConveyorBelt)
					m_KinematicController = AddKinematicController(CollideBody());
			}
			break;

		case E_PhysicsBodyMsg_SetCollideGroup:
			if (CollideBody())
			{
				const C_PhysicsBodySetCollideGroup* msg = Messaging::MessageCast<C_PhysicsBodySetCollideGroup>(pMessage);
				CollisionFilter().SetGroupId(msg->GetValue());
			}
			break;

		case E_PhysicsBodyMsg_SetCollideGroupMask:
			if (CollideBody())
			{
				const C_PhysicsBodySetCollideGroupMask* msg = Messaging::MessageCast<C_PhysicsBodySetCollideGroupMask>(pMessage);
				CollisionFilter().SetGroupMask(msg->GetValue());
			}
			break;

		case E_PhysicsBodyMsg_SetCollideGroupFlags:
			if (CollideBody())
			{
				const C_PhysicsBodySetCollideGroupFlags* msg = Messaging::MessageCast<C_PhysicsBodySetCollideGroupFlags>(pMessage);
				CollisionFilter().SetGroupFlags(msg->GetValue());
			}
			break;

		case E_PhysicsBodyMsg_SetCollideGlobalFlags:
			if (CollideBody())
			{
				const C_PhysicsBodySetCollideGlobalFlags* msg = Messaging::MessageCast<C_PhysicsBodySetCollideGlobalFlags>(pMessage);
				CollisionFilter().SetCollisionFlags(msg->GetValue());
			}
			break;

		case E_PhysicsBodyMsg_SetCollideGlobalMask:
			if (CollideBody())
			{
				const C_PhysicsBodySetCollideGlobalMask* msg = Messaging::MessageCast<C_PhysicsBodySetCollideGlobalMask>(pMessage);
				CollisionFilter().SetCollisionMask(msg->GetValue());
			}
			break;
		case E_PhysObjPhysicsBodyMsg_OverrideCollision:
			{
				if (CollideBody())
				{
					const C_PhysicalObjectPhysicsBodyOverrideCollisionMsg* msg = Messaging::MessageCast<C_PhysicalObjectPhysicsBodyOverrideCollisionMsg>(pMessage);
					if (GetCreationData()->m_OverrideBodyFlagsAndMask)
						OverrideCollisionFlagsAndMask(CollideBody()->GetShape(), GetCreationData()->m_CollisionFilter.GetCollisionFlags(), GetCreationData()->m_CollisionFilter.GetCollisionMask());
					if (msg->m_OverrideIndex >= 0)
						SetIndexCollisionFlags(CollideBody()->GetShape(), msg->m_OverrideIndex, msg->m_Flags, msg->m_Mask);
				}
				
			}
			break;

		case E_PhysicsBodyMsg_SetBodyFlag:
			if (CollideBody())
			{
				const C_PhysicsBodySetBodyFlagMsg* msg = Messaging::MessageCast<C_PhysicsBodySetBodyFlagMsg>(pMessage);
				CollideBody()->SetBodyFlags(msg->GetValue());
			}
			break;

		case E_PhysicsBodyMsg_ClearBodyFlag:
			if (CollideBody())
			{
				const C_PhysicsBodyClearBodyFlagMsg* msg = Messaging::MessageCast<C_PhysicsBodyClearBodyFlagMsg>(pMessage);
				CollideBody()->ClearBodyFlags(msg->GetValue());
			}
			break;

		case E_PhysicsBodyMsg_ForceActive:
		{
			ForceActive();
			break;
		}

		case E_PhysicsBodyMsg_AllowDeactivate:
		{
			AllowDeactivate();
			break;
		}

		case E_PhysicsBodyMsg_SetActivationThreshold:
			SetActivationThreshold(Messaging::MessageCast<C_PhysicsBodySetActivationThresholdMsg>(pMessage)->GetValue()); 
			break;

		case ActorMsg::E_GameActorMsg_SetTransform:
		{
			const ActorMsg::C_ActorSetTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage);
			SetTransform(msg->Transform());
			break;
		}

		case ActorMsg::E_GameActorMsg_AnimateTransform:
			if (CollideBody())
			{
				const ActorMsg::C_ActorAnimateTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage);
				if (CanAnimate())
				{
					AnimateTransform(msg->Transform(), msg->m_TimeStep);
				}
			}
			break;

		case ActorMsg::E_MovementNotificationMsg_MovementComplete:
			if (CollideBody() && CanAnimate())
			{
				//assert(m_KinematicController);
				//m_KinematicController->Stop();
			}
			break;

		case Actor::E_ActorMsg_RemoveCollision:
			DestroyBody();
			break;

		case E_PhysicsBodyMsg_MakeFrameOfReference:
			if (CollideBody())
			{
				if (Messaging::MessageCast<C_PhysicsBodyMakeFrameOfReferenceMsg>(pMessage)->GetValue())
					CollideBody()->MakeFrameOfReference();
				else
					CollideBody()->UnMakeFrameOfReference();
			}
			break;

		case E_PhysicsBodyMsg_NoBuoyancy:
			if(CollideBody())
			{
				if(Messaging::MessageCast<C_PhysicsBodyDisableBuoyancyMsg>(pMessage)->GetValue())
					CollideBody()->GetGraphNode()->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoBuoyancy);
				else
					CollideBody()->GetGraphNode()->ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoBuoyancy);
			}
			break;

		case E_PhysicsBodyMsg_SetCentreOfMassOffset:
			if (CollideBody())
			{
				CollideBody()->SetCentreOfMassOffset(Messaging::MessageCast<C_PhysicsBodySetCentreOfMassOffset>(pMessage)->GetValue());
			}
			break;

		default:
			return I_PhysicsBody::OnMessage(pMessage);
	}

#ifndef CFG_SPEED
	if (!m_pCollideBody)
	{
		if (pMessage->Type()>=E_PhysicsBodyMsg_Begin && pMessage->Type()<=E_PhysicsBodyMsg_End && pMessage->Type()!=E_PhysicsBodyMsg_UpdateShape)
		{
			static int count = 0;
			if (count++ < 10)
			{
				LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), "%s: PBC with no collide body received message '%s' (0x%8x)", GetActor()->GetActorName(), pMessage->GetDebugName(), pMessage->Type());
			}
		}
	}
#endif

	return true;
}


void C_PhysicsBody::OnPhysicsBodyEvent(const Gravity::C_BodyEvent& bodyEvent)
{
	switch (bodyEvent.GetEventType())
	{
		case Gravity::C_BodyEvent::E_WakeUp:
//			BIZ_ASSERT(!m_BodyActive);
			m_BodyActive = true;
			AddSubscriber(GameMessaging::E_GameMsg_Update);
			break;

		case Gravity::C_BodyEvent::E_GoToSleep:
//			BIZ_ASSERT(m_BodyActive);
			m_BodyActive = false;
			RemoveSubscriber(GameMessaging::E_GameMsg_Update);
			break;

		case Gravity::C_BodyEvent::E_RemoveConstraint:
			// Ignore.
			break;

		default:
			LOGWARN(Actor::GetDebugLogGeneral(), DBSTR"unhandled event (%d)", bodyEvent.GetEventType());
			break;
	}
}


// Checkpoint serialisation
void C_PhysicsBody::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_PhysicsBody::SerialiseForSave(stream);

	stream.WriteAs<bool>(CollideBody() ? true : false);
	if (CollideBody())
	{
		stream.WriteAs<S32>(BodyType());
		stream.WriteAs<bool>(m_IsEnabled);
		stream.WriteAs<bool>(m_BodyActive);
		stream << RenderMatrix();
		stream << CollideBody()->LinearVelocity();
		stream << CollideBody()->AngularVelocity();
	}
}


void C_PhysicsBody::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_PhysicsBody::DeserialiseForSave(stream);

	const bool hasBody = stream.ReadAs<bool>();
	if (hasBody)
	{
		m_BodyType = ActorPhysics::TBodyType(stream.ReadAs<S32>());
		m_IsEnabled = stream.ReadAs<bool>();
		m_BodyActive = stream.ReadAs<bool>();
		stream >> m_CreationMatrix;
		stream >> m_CreationLinVel;
		stream >> m_CreationAngVel;
	}
}


// Network Serialisation
void C_PhysicsBody::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_PhysicsBody::SerialiseForSync(stream);

	Maths::HVec3 linearVel(0,0,0);
	Maths::HVec3 angularVel(0,0,0);

	if(CollideBody())
	{
		linearVel = CollideBody()->LinearVelocity();
		angularVel = CollideBody()->AngularVelocity();
	}
	else
	{
		LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR "no collide body");
	}

	stream << Matrix();
	stream << linearVel;
	stream << angularVel;
}


void C_PhysicsBody::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_PhysicsBody::DeserialiseForSync(stream);

	Maths::HMat43 mat;
	Maths::HVec3 linearVel;
	Maths::HVec3 angularVel;
	stream >> mat;
	stream >> linearVel;
	stream >> angularVel;

	if (CollideBody())
	{
		CollideBody()->GetBody().StoreVelocitiesAndPosition(linearVel, angularVel, mat.Rotation(), mat.AW());
	}
}


Gravity::C_Body* C_PhysicsBody::CreateBody(ActorPhysics::TBodyType bodyType, C_PhysicsBodyShapeDesc& shapeDesc, const Maths::HMat43& matrix, const Collision::C_CollisionFilter& filter, const SharedPhysics::C_Deactivator* deactivation)
{
	PERF_EVENT("PBC::CreateBody");
	if (!PhysicsScene())
	{
		LOGERROR(Actor::GetDebugLogGeneral(), DBSTR"No scene for create body call!");
		return NULL;
	}

	BIZ_ASSERT(shapeDesc.GetShapeDesc());

	Gravity::C_Body* pBody(PhysicsScene()->CreateBody());
	pBody->SetUserData(m_pUserData);
	pBody->SetExternalConstants(GetActor()->GetContext<GameContext::I_PhysicsContext>()->GetDefaultExternalConstants());
	pBody->Baumgarte(0.8f);
	pBody->Slop(0.000001f);

	if (GetCreationData()->m_ContinuousCollision && shapeDesc.GetShapeDesc()->m_Type != Collision::E_Continuous_Shape)
	{
		MakeContinuousShapeDesc(shapeDesc);
		pBody->SetContinuousSpeedThreshold(1.0f);
	}

	if (GetCreationData()->m_CentreOfMassOffset.LengthSquared() > 0.0f)
	{
		pBody->SetCentreOfMassOffset(GetCreationData()->m_CentreOfMassOffset);
	}

	Collision::C_CollisionFilter* pFilter = const_cast<Collision::C_CollisionFilter*>(&filter); // to be consted in gravity.
	Gravity::C_BodyHelper helper(shapeDesc.GetShapeDesc());
	//helper.CreateBody(shapeDesc.GetMass(), matrix.AW(), matrix.Rotation(), pBody, pFilter, 0, 0);
	helper.CreateBody(shapeDesc.GetMass(), matrix.AW(), matrix.Rotation(), pBody, pFilter);

	helper.MakeFloat(300.f, F32(g_PhysicsBuoyancyScale * GetCreationData()->m_BuoyancyCoeffs.m_Coeff),
							F32(g_PhysicsMaxBuoyancy * GetCreationData()->m_BuoyancyCoeffs.m_MaxCoeff),
							pBody,
							F32(g_PhysicsLinearDragScale * GetCreationData()->m_BuoyancyCoeffs.m_LinearDragCoeff),
							F32(g_PhysicsAngularDragScale * GetCreationData()->m_BuoyancyCoeffs.m_AngularDragCoeff));

	SharedPhysics::C_Deactivator deactivator;
	if (deactivation)
	{
		deactivator = *deactivation;
		deactivator.m_ActivateThresholdSq = deactivator.m_ActivateThresholdSq * pBody->Mass();
		const Maths::HVec3 angularEnergy(Maths::HVec3(deactivator.m_SleepThresholdSq) * pBody->IBodyMatrix());
		deactivator.m_SleepThresholdSq = Maths::Max(deactivator.m_SleepThresholdSq * pBody->Mass() + 
			angularEnergy.Length(), deactivator.m_ActivateThresholdSq);
	}
	else
	{
		deactivator.m_ActivateThresholdSq = 0.01f * pBody->Mass();
		deactivator.m_StaticActivateThresholdSq = 0.001f * pBody->Mass();
		deactivator.m_SleepThresholdSq = 0.01f * pBody->Mass();
		deactivator.m_sSleepCountdown = 15;
	}
	pBody->SetDeactivator(deactivator);

	switch (bodyType)
	{
		case ActorPhysics::E_BodyType_Dynamic:
			pBody->Deactivate();
			//AddSubscriber(GameMessaging::E_GameMsg_Update);
			break;

		case ActorPhysics::E_BodyType_Static:
			{
				if(deactivator.m_StaticActivateThresholdSq == 0.f)
				{
					pBody->Deactivate();
				}
				else
					pBody->MakeStatic();
			}
			break;

		case ActorPhysics::E_BodyType_Animated:
			pBody->MakeKinematic();
			break;

		default:
			LOGERROR(Actor::GetDebugLogGeneral(), DBSTR"unhandled body type (%d)", bodyType);
			break;
	}

	pBody->SetMaxAngularVelocity((Maths::HF32)g_PhysicsBodyMaxAngularSpeed);
	pBody->SetAngularDampingCoefficient((Maths::HF32)g_PhysicsBodyAngularDampingCoeff);	// Stop 'perpetual motion' especially in cylinders.

	if (!m_BodyActive)
		pBody->Deactivate();

	// Insert in scene last - commands get deferred after this (executed immediately when not in scene)
	if (m_IsEnabled)
		PhysicsScene()->AddBody(pBody);

	return pBody;
}


bool C_PhysicsBody::CreateBody()
{
	PERF_EVENT("C_PhysicsBody.CreateBody");
	{
		C_PhysicsBodyShapeDesc shapeDesc;
		CreateShape(shapeDesc);

		if (shapeDesc.GetShapeDesc() == NULL)
		{
			shapeDesc.DestroyOwned();
			return false;
		}

		m_ShapeDesc.DestroyOwned();
		m_ShapeDesc = shapeDesc;
	}

	const Maths::HMat43 mat(m_CreationMatrix.Rotation(), m_ShapeDesc.GetShapeDesc()->m_RenderOffset * m_CreationMatrix);
	m_pCollideBody = CreateBody(m_BodyType, m_ShapeDesc, mat, GetCreationData()->m_CollisionFilter, &GetCreationData()->m_Deactivator);
	if (m_pCollideBody)
	{
		CollideBody()->GetBody().StoreVelocitiesAndPosition(m_CreationLinVel, m_CreationAngVel, mat.Rotation(), mat.AW());

		if (m_BodyType == ActorPhysics::E_BodyType_Animated)//? && !GetCreationData()->m_IsConveyorBelt)
		{
			if (!m_KinematicController)
				m_KinematicController = AddKinematicController(m_pCollideBody);
		}

		SendMessageToActor(E_PhysicsBodyNotification_BodyCreated);
		SendMessageToActor(E_PhysicsBodyNotification_BodyShapeChanged);

		return true;
	}
	
	return false;
}


void C_PhysicsBody::OnShapeChanged(S32 overrideIndex)
{
	if (CollideBody())
	{
		C_PhysicsBodyShapeDesc shapeDesc;
		CreateShape(shapeDesc);

		if (shapeDesc.GetShapeDesc() != NULL)
		{
			m_ShapeDesc.DestroyOwned();
			m_ShapeDesc = shapeDesc;

			Collision::C_CollisionFilter filter = CollisionFilter();
			filter.SetCollisionFlags(GetCreationData()->m_CollisionFilter.GetCollisionFlags());
			filter.SetCollisionMask(GetCreationData()->m_CollisionFilter.GetCollisionMask());
			if (GetCreationData()->m_ContinuousCollision && (m_ShapeDesc.GetShapeDesc()->m_Type & Collision::E_Continuous_Shape)==0)
				MakeContinuousShapeDesc(m_ShapeDesc);
			Gravity::C_BodyHelper helper(m_ShapeDesc.GetShapeDesc());
			helper.UpdateBodyShape(shapeDesc.GetMass(), m_pCollideBody, &filter);
			SendMessageToActor(E_PhysicsBodyNotification_BodyShapeChanged);
		}
		else
		{
			//Clear out any data we may own
			LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR"Trying to create an empty shape");
			shapeDesc.DestroyOwned();
		}
	}
	else
	{
		CreateBody();

		if (CollideBody())
		{
			const C_PhysicsBodyTransformChanged transformChangedMsg(RenderMatrix());
			SendMessageToActor(&transformChangedMsg);
		}
	}
}


void C_PhysicsBody::Update(const Maths::C_TimeS& timeStep)
{
	if (!CollideBody())
		return;

	if (m_IsEnabled && CollideBody()->IsActive())
	{
		const C_PhysicsBodyTransformChanged transformChangedMsg(RenderMatrix());
		SendMessageToActor(&transformChangedMsg);

//#ifndef CFG_GOLD
		{
			const Gravity::C_Body* pBody = CollideBody();

			if ((pBody != NULL) && 
				(pBody->GetScene() != NULL) && 
				pBody->IsActive() && 
				(pBody->Position().Y() < pBody->GetScene()->GetWorldBounds().m_min.Y()) && 
				(pBody->LinearVelocity().Y() < -25.0f))
			{
//				LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR "Actor %s fell out of world (x:%.02f z:%.02f)", GetActor()->GetActorName(), (F32)pBody->Position().X(), (F32)pBody->Position().Z());
				SendMessageToActor(Actor::E_ActorMsg_Kill);
			}
		}
//#endif
	}
}


// Activates body if dynamic.
// Note: calling Activate on a gravity body changes its type to dynamic.
void C_PhysicsBody::Activate()
{
	if (BodyType()==ActorPhysics::E_BodyType_Dynamic)
	{
		CollideBody()->Activate();
	}
}


void C_PhysicsBody::SetTransform(const Maths::C_QuatTrans& transform)
{
	Gravity::C_Body* body = CollideBody();
	if (body)
	{
		if (CanAnimate())
		{
			// Stop kinematic bodies:
			m_KinematicController->Teleport(transform.Matrix());
		}
		else
		{
			// Set pos using render matrix:
			body->ElementMatrix(transform.Matrix());
		}

 		const C_PhysicsBodyTransformChanged transformChangedMsg(RenderMatrix());
		SendMessageToActor(&transformChangedMsg);
	}
}


void C_PhysicsBody::AnimateTransform(const Maths::C_QuatTrans& transform, F32 timeStep)
{
	BIZ_ASSERT(CollideBody() && CanAnimate());

	m_KinematicController->SetTargetTransformation(transform, timeStep);
	
	const C_PhysicsBodyTransformChanged transformChangedMsg(RenderMatrix());
	SendMessageToActor(&transformChangedMsg);
}


Maths::HMat43 C_PhysicsBody::Matrix() const
{
	if(CollideBody())
		return Maths::HMat43(CollideBody()->Orientation(), CollideBody()->Position());
	else
		return Maths::HMat43();
}


Maths::HMat43 C_PhysicsBody::RenderMatrix() const
{
	if(CollideBody())
		return m_pCollideBody->ElementMatrix();
	else
		return Maths::HMat43();
}


Maths::HVec3 C_PhysicsBody::ShapeCentroid() const
{
	if (CollideBody())
		return CollideBody()->ElementOffset();
	return Maths::HVec3();
}


Collision::C_Sphere C_PhysicsBody::WorldBoundingSphere() const
{
	Collision::C_Sphere localSphere(CollideBody()->GetLocalBoundingSphere());
	return Collision::C_Sphere(localSphere.Centre() * Matrix(), localSphere.Radius());
}


void C_PhysicsBody::AllowDeactivate()
{
	if (CollideBody())
	{
		SharedPhysics::C_Deactivator deactivator(CollideBody()->GraphNode()->Deactivator());
		deactivator.m_ActivateThresholdSq = 0.001f * 0.001f * CollideBody()->Mass();
		deactivator.m_SleepThresholdSq = 0.01f * 0.01f * CollideBody()->Mass();
		CollideBody()->SetDeactivator(deactivator);
	}
}


void C_PhysicsBody::ForceActive()
{
	if (CollideBody())
	{
		SharedPhysics::C_Deactivator deactivator(CollideBody()->GraphNode()->Deactivator());
		deactivator.m_ActivateThresholdSq = -1.0f;
		deactivator.m_SleepThresholdSq = -1.0f;
		CollideBody()->SetDeactivator(deactivator);
	}
}


void C_PhysicsBody::SetActivationThreshold(float val)
{
	if (CollideBody())
	{
		SharedPhysics::C_Deactivator deactivator(CollideBody()->GraphNode()->Deactivator());
		deactivator.m_ActivateThresholdSq = val*val;
		CollideBody()->SetDeactivator(deactivator);
	}
}

// nasty helper function to override collision filter flags
bool C_PhysicsBody::SetIndexCollisionFlags(Collision::C_Shape* pShape, U32 index, U32 flags, U32 mask)
{
	bool bMatch = false;
	if(pShape->m_Index == index)
	{
		pShape->m_Filter.SetCollisionFlags( flags);
		pShape->m_Filter.SetCollisionMask(mask);
	}

	if (pShape->GetShape() == Collision::E_Composite_Shape)
	{
		Collision::C_CompositeShape* pCompShape((Collision::C_CompositeShape*)pShape);
		Collision::C_CompositeShapeIterator iter(pCompShape);
		//for(U32 i = 0; i < pCompShape->m_NumShapes; ++i)
		while(iter.HasNextShape())
		{
			iter.NextShape();
			Collision::C_Shape* pSubShape = iter.GetCurrentShape<Collision::C_Shape>();
			switch(pSubShape->GetShape())
			{
			case Collision::E_Composite_Shape:
				{
					SetIndexCollisionFlags(pSubShape, index, flags, mask);
					if(pCompShape->GetIndex() == index)
					{
						pCompShape->m_Filter.SetCollisionFlags(flags);
						pCompShape->m_Filter.SetCollisionMask(mask);
					}
					break;
				}

			default:
				{
					if (pSubShape->GetIndex() == index)
					{
						pSubShape->m_Filter.SetCollisionMask(mask);
						pSubShape->m_Filter.SetCollisionFlags(flags);
						bMatch = true;
					}
					break;
				}
			}
		}
	}

	return bMatch;
}


void C_PhysicsBody::OverrideCollisionFlagsAndMask(Collision::C_Shape* pShape, U32 flags, U32 mask)
{
	if(pShape->m_ShapeType != Collision::E_InstancedTriangleMesh_Shape && 
		pShape->m_ShapeType != Collision::E_TriangleMesh_Shape)
	{
		pShape->m_Filter.SetCollisionFlags(flags);
		pShape->m_Filter.SetCollisionMask(mask);
	}


	if (pShape->GetShape() == Collision::E_Composite_Shape)
	{
		Collision::C_CompositeShape* pCompShape((Collision::C_CompositeShape*)pShape);
		Collision::C_CompositeShapeIterator iter(pCompShape);
		//for(U32 i = 0; i < pCompShape->m_NumShapes; ++i)
		while(iter.HasNextShape())
		{
			iter.NextShape();
			Collision::C_Shape* pSubShape = iter.GetCurrentShape<Collision::C_Shape>();
			switch(pSubShape->GetShape())
			{
			case Collision::E_Composite_Shape:
				{
					OverrideCollisionFlagsAndMask(pSubShape, flags, mask);
					pCompShape->m_Filter.SetCollisionMask(mask);
					pCompShape->m_Filter.SetCollisionFlags(flags);
					break;
				}
			case Collision::E_TriangleMesh_Shape:
				{
					pCompShape->m_Filter.SetCollisionMask(mask);
					break;
				}
			case Collision::E_InstancedTriangleMesh_Shape:
				{
					pCompShape->m_Filter.SetCollisionMask(mask);
					break;
				}
			default:
				{
					pSubShape->m_Filter.SetCollisionMask(mask);
					pSubShape->m_Filter.SetCollisionFlags(flags);
					break;
				}
			}
		}
	}
}


#ifndef CFG_GOLD
#include <btl/Utility/string.h>
void C_PhysicsBody::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_PhysicsBodyDebugRender)
		return;

	if (!CollideBody())
	{
		debugRenderPtr->Draw3dText(Matrix().AW(), "No body");
		return;
	}

	debugRenderPtr->Draw3dMatrix(Matrix());
	debugRenderPtr->Draw3dText(Matrix().AW(), "BodyMat");

	if (g_PhysicsBodyShowRenderMatrix)
	{
		debugRenderPtr->Draw3dMatrix(RenderMatrix(), 0.5f);
		debugRenderPtr->Draw3dText(RenderMatrix().AW(), "RenderMat");
	}

	Btl::String str(BodyType()==ActorPhysics::E_BodyType_Dynamic ? "Dynamic" : (BodyType()==ActorPhysics::E_BodyType_Animated ? "Animated" : (BodyType()==ActorPhysics::E_BodyType_Static ? "Static" : "INVALID!")));
	str += Btl::String::Format(" %.1fKg", F32(Mass()));

	if (IsSubscribedTo(GameMessaging::E_GameMsg_Update))
		str += Btl::String("\nSubscribed to update");

	debugRenderPtr->Draw3dText(Matrix().AW()-Maths::HVec3(0,0.5,0), str.Chars());
}

#endif //CFG_GOLD



//////////////////////////////////////////////////////////////////////////
// Physical Object Physics Body
//
bool C_PhysicalObjectPhysicsBody::InitialiseImpl()
{
	m_ModelElemDesc = GetCreationData()->m_ElementDescs;
	return C_PhysicsBody::InitialiseImpl();
}


C_PhysicalObjectPhysicsBody::~C_PhysicalObjectPhysicsBody()
{
}


void C_PhysicalObjectPhysicsBody::CreateShape(C_PhysicsBodyShapeDesc& shapeDesc) const
{
	PERF_EVENT("C_PhysicalsBody.CreateShape");
	const I_ModelResource *pModelResource = (const I_ModelResource *)GetActor()->QueryInterface(Components::E_IF_ModelResource);
	F32 mass = 0.0f;

	if (!pModelResource || !pModelResource->GetModelData())
	{
		LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR"Model resource not available");
		return;
	}

	const Components::I_DamageableElementManager* elemMgr = (const Components::I_DamageableElementManager*)(QueryInterface(Components::E_IF_DamageableElementManager));
	if (!elemMgr)
		return;

	const S32 physObjElemCount = elemMgr->FlatElementDefs().Size();// GetCreationData()->ElementDefs().Size();
	Gravity::C_CompositeDesc* newDesc = new Gravity::C_CompositeDesc(NULL, physObjElemCount);
	shapeDesc.TakeOwnership(newDesc);
	newDesc->m_DecomposeSingleComposite = false;	// Stop decomposition of single-element composite shapes.

// 	const GameResource::C_Model* pModel = pModelResource->GetModelData()->GetRootModel();	//[MGB]ModelIndex
// 	BIZ_ASSERT_MSG(pModel, "NULL model!");

	if (g_PhysicsBodyLogShapeMapping)
		LOGINFO(Actor::GetDebugLogGeneral(), "Create Shape for actor '%s' (%d parts) -------------------------", GetActor()->GetActorName(), physObjElemCount);

	// Build a composite descriptor from active phys obj elements:
	S32 e=0;
	for (e=0 ; e<physObjElemCount ; e++)
	{
		const BasePhysicalObject::C_ElementDef* elemDef = elemMgr->FlatElementDefs()[e];
		elemDef->SetupModelData(pModelResource->GetModelData()); // todo: eurgh!
		
		const S32 elemDmgLevel = m_ModelElemDesc[e];
		if (elemDmgLevel >= 0)
		{
			const F32 elemMass = elemDef->m_DamageLevels[elemDmgLevel]->m_Mass;
			const Gravity::C_ImportShape* gs = elemDef->m_DamageLevels[elemDmgLevel]->m_pShape;
			if (gs)
			{
				BIZ_ASSERT(gs->GetShapeDesc() && gs->GetShapeDesc()->ValidShapeDescriptor());
				newDesc->AddShape(gs->GetShapeDesc(), gs->GetImportMatrix(), elemMass);
				mass += elemMass;

				if (g_PhysicsBodyLogShapeMapping)
					LOGINFO(Actor::GetDebugLogGeneral(), "ObjPart %d: (ModelElem %d: '%s') = CompositeDescPrim %d [shape type %d]", e, elemDef->m_DamageLevels[elemDmgLevel]->ModelElementIndex(), elemDef->m_Name.Text(), newDesc->shapes.Size()-1, gs->GetShapeDesc()->m_Type);
			}
			else
			{
				if (g_PhysicsBodyLogShapeMapping)
					LOGINFO(Actor::GetDebugLogGeneral(), "ObjPart %d: (ModelElem %d: '%s') - No import shape", e, elemDef->m_DamageLevels[elemDmgLevel]->ModelElementIndex(), elemDef->m_Name.Text());
			}
		}
	}

	if (newDesc->GetNumValidShapes() <= 0)
	{
		LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR"%s: no valid shapes in descriptor", GetActor()->GetActorName());
		return;
	}

	newDesc->RemapShape();
	newDesc->SetRemapped();

	shapeDesc.Set(mass, newDesc);
}

void C_PhysicalObjectPhysicsBody::UpdatePivotShapes(Collision::C_Shape* pShape, const Components::I_DamageableElementManager* elemMgr)
{
	if (pShape->GetShape() == Collision::E_Composite_Shape)
	{
		Collision::C_CompositeShape* pCompShape((Collision::C_CompositeShape*)pShape);
		Collision::C_CompositeShapeIterator iter(pCompShape);
		U32 index = 0;
		//for(U32 i = 0; i < pCompShape->m_NumShapes; ++i)
		while(iter.HasNextShape())
		{
			U32 i = index++;
			iter.NextShape();
			Collision::C_Shape* pSubShape = iter.GetCurrentShape<Collision::C_Shape>();
			switch(pSubShape->GetShape())
			{
			case Collision::E_Composite_Shape:
				{
					UpdatePivotShapes(pSubShape, elemMgr);
					break;
				}

			default:
				{
					S32 elemIndex = elemMgr->ElementIndexFromFlatPrimitiveIndex(pSubShape->GetIndex());
					if (elemIndex > 0)
					{
						const BasePhysicalObject::C_ElementDef* elemDef = elemMgr->FlatElementDefs()[elemIndex];
						if (elemDef->m_Pivoted)
						{
							const Maths::HMat43&elementMat = elemMgr->ElementMatrix(elemIndex);
							//Collision::C_ShapeWrapper* pShapeWrapper = &pCompShape->m_pShapes[i];
							const Gravity::C_ImportShape* gs = elemDef->m_DamageLevels[0]->m_pShape;
							if (gs)
							{
								Gravity::C_CompositeDesc* compDesc = gs->GetShapeDesc()->GetComposite();
								if (compDesc)
								{
									// strip out the render offset before pivoting
									Maths::HMat43 pivotMat(elementMat.Rotation(), -compDesc->m_RenderOffset);
									const Maths::HVec3 offsetTranslation(compDesc->m_RenderOffset * elementMat.Rotation());
									pSubShape->m_RelativeTransformation = compDesc->shapes[i].m_RelativeTransformation * pivotMat;
									pSubShape->m_RelativeTransformation.Translation(pSubShape->m_RelativeTransformation.Translation() + offsetTranslation);
								}
							}
						}
					}
					
					break;
				}
			}
		}
	}
	else 
	{
		//const C_ElementState &element = elemMgr->GetElement(pShape->m_Index);
		//Collision::C_ShapeWrapper* pShapeWrapper = pShape;
		//pShapeWrapper->m_RelativeTransformation = element.m_Matrix;
	}
}


bool C_PhysicalObjectPhysicsBody::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		case E_PhysObjPhysicsBodyMsg_UpdateShape:
		{
			const C_PhysicalObjectPhysicsBodyUpdateShapeMsg* msg = Messaging::MessageCast<C_PhysicalObjectPhysicsBodyUpdateShapeMsg>(pMessage);
			m_ModelElemDesc = msg->m_ElemDescList;
			OnShapeChanged(msg->m_OverrideIndex);
		}	break;

 		case E_PhysObjPhysicsBodyMsg_OverrideCollision:
 			{
 				const C_PhysicalObjectPhysicsBodyOverrideCollisionMsg* msg = Messaging::MessageCast<C_PhysicalObjectPhysicsBodyOverrideCollisionMsg>(pMessage);
 				if (GetCreationData()->m_OverrideBodyFlagsAndMask)
 					OverrideCollisionFlagsAndMask(CollideBody()->GetShape(), GetCreationData()->m_CollisionFilter.GetCollisionFlags(), GetCreationData()->m_CollisionFilter.GetCollisionMask());
 				if (msg->m_OverrideIndex >= 0)
 					SetIndexCollisionFlags(CollideBody()->GetShape(), msg->m_OverrideIndex, msg->m_Flags, msg->m_Mask);
 			}
 			break;

		case E_PhysObjPhysicsBodyMsg_UpdatePivotShapes:
			{
				const I_DamageableElementManager* elemMgr = (const I_DamageableElementManager*)(QueryInterface(Components::E_IF_DamageableElementManager));
				if (elemMgr)
				{
					if (elemMgr->ContainsPivotedElement())
					{
						UpdatePivotShapes(CollideBody()->GetShape(), elemMgr);
					}
				}
				
					
			}
			break;

		default:
			return C_PhysicsBody::OnMessage(pMessage);
	}

	return true;
}


void C_PhysicalObjectPhysicsBody::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_PhysicsBody::SerialiseForSave(stream);

	const S32 count = m_ModelElemDesc.Size();
	stream << count;
	for (S32 e=0 ; e<count ; e++)
		stream << m_ModelElemDesc[e];
}


void C_PhysicalObjectPhysicsBody::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_PhysicsBody::DeserialiseForSave(stream);

	BIZ_ASSERT(m_ModelElemDesc.Empty());
	S32 count;
	stream >> count;
	for (S32 e=0 ; e<count ; e++)
	{
		S32 val;
		stream >> val;
		m_ModelElemDesc.PushBack(val);
	}
}


void C_PhysicalObjectPhysicsBody::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_PhysicsBody::SerialiseForSync(stream);

	const S32 count = m_ModelElemDesc.Size();
	stream << count;
	for (S32 e=0 ; e<count ; e++)
		stream << m_ModelElemDesc[e];
}


void C_PhysicalObjectPhysicsBody::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_PhysicsBody::DeserialiseForSync(stream);

	BIZ_ASSERT(m_ModelElemDesc.Empty());
	S32 count;
	stream >> count;
	for (S32 e=0 ; e<count ; e++)
	{
		S32 val;
		stream >> val;
		m_ModelElemDesc.PushBack(val);
	}
}

} // namespace Components

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
