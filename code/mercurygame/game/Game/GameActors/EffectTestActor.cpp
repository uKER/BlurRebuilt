#include "stdafx.h"

#include <Xt/XtGame.h>
#include <Core/UtMaths.h>

#include <misc_shared/messaging/GameMessaging.h>
#include <Misc_shared/NewCurveHermite.h>
#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>

#include <Game/GameActors/EffectTestActor.h>

#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

// particle system integration
#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>

#include <Dynamic/Particle/SpawnerInterface.h>

namespace Game
{

namespace Actors
{

Debug::Bool g_EffectTestActorUseSpawner("Game.Particle.UseSpawner", false);

Debug::Logger& gParticleEventLog = Debug::GetLogger("mercurygame.effectactor");

// Actor Info

C_EffectTestActor::C_EffectTestActor()
{
	LOGINFO(gParticleEventLog, "Constructed effect test actor" );
	m_pCurveData = new Curve::C_Hermite3DData;
	m_pActorCurve = new Curve::C_ActorCurve3D();

	m_pSpawner = new DynamicEffect::Particle::I_Spawner;
}

C_EffectTestActor::~C_EffectTestActor()
{
	SAFE_DELETE_PTR(m_pActorCurve);
	SAFE_DELETE_PTR(m_pCurveData);
	LOGINFO(gParticleEventLog, "Destroyed effect test actor" );
}

bool C_EffectTestActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	
		case ActorMsg::E_ResourceMsg_AllLoaded:
		{
			// Resources all ready - Create should be called now?
			return true;
		}

		case ActorMsg::E_ResourceMsg_Loaded:
		{
			U32 resIndex = Messaging::MessageCast<ActorMsg::C_ResourceLoadedMsg>(pMessage)->GetValue();
			LOGINFO(gParticleEventLog, "Resource loaded message %X", resIndex );
			return true;
		}

		case ActorMsg::E_ResourceMsg_Unloaded:
		{
			U32 resIndex = Messaging::MessageCast<ActorMsg::C_ResourceUnloadedMsg>(pMessage)->GetValue();
			LOGINFO(gParticleEventLog, "Resource unloaded message %X", resIndex );
			return true;
		}

		case GameMessaging::E_GameMsg_Update:
			{
				F32 deltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
				Update( deltaTime );
			}
			return true;
	}

	return C_GameActor::OnMessage(pMessage);
}


bool C_EffectTestActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_EffectTestActorCreate* pCreate = (const C_EffectTestActorCreate*)(GetCreationData().GetData());

	
	return true;
}

bool C_EffectTestActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	Curve::C_HermiteSplineFlags flags;
	flags.calcKnotU = Curve::E_ProcessU_ArcLengthLinear;
	flags.calcTangents = true;
	Curve::C_Hermite3D* pSplineEvaluator = new Curve::C_Hermite3D(flags);
	pSplineEvaluator->SetData(m_pCurveData);

	const C_EffectTestActorCreate* pCreate = (const C_EffectTestActorCreate*)(GetCreationData().GetData());

	Maths::C_QuatTrans quatTrans(pCreate->GetCreationMatrix());

	{
		Curve::C_Hermite3DData::KnotVector& knots = m_pCurveData->GetKnots();
		knots.Clear();
		
		Maths::HVec3 centre(quatTrans.Translation());
		Maths::HF32 radius(2.0f);
		const S32 stepCount = 8;
		const F32 deltaAngle = Maths::Pi * 2.0f / (F32)stepCount;
		F32 currAngle = 0;

		knots.Resize(stepCount + 1);
		for (S32 i = 0; i <= stepCount; i++)
		{
			Curve::C_Hermite3DData::KnotType& knot = knots[i];
			Maths::SinCosAlpha sc_angle(currAngle);

			Maths::HVec3 currPos;

			currPos.X(centre.X() + sc_angle.SinAlpha() * radius);
			currPos.Y(centre.Y() + sc_angle.CosAlpha() * radius);
			currPos.Z(centre.Z());
		    
			currAngle = currAngle + deltaAngle;
			knot.value = currPos;
			knot.tangentScale = 1.f;
		}
		pSplineEvaluator->Process();
		m_pActorCurve->SetCurve(pSplineEvaluator);
		m_pActorCurve->Process(stepCount*2);
	}
	
	m_ParticleEventId = DynamicEffect::Particle::XtInterface::GetEventId(pCreate->m_ParticleEventIndex);

	if(m_ParticleEventId != 0xFFFFFFFF)
	{
		m_EventInstanceHandle = DynamicEffect::Particle::XtInterface::SpawnEvent(m_ParticleEventId, quatTrans);
		DynamicEffect::Particle::C_EventInstance* pEventInstance;
		if((pEventInstance = m_EventInstanceHandle.Object()))
		{
			m_Repeating = pEventInstance->GetEvent()->GetDefinition().bRepeating;
			pEventInstance->SetVelocity(Maths::HVec3(0.0f, 0.0f, 0.0f));
			pEventInstance->SetLighting(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
			pEventInstance->SetSpline(m_pActorCurve);
			pEventInstance->SetSpawner(m_pSpawner);

			// add method with HVec3 and C_Normal
			MathsStorage::Float4A16 vCollisionPosition, vCollisionNormal;
			Maths::Store(pCreate->m_vCollisionPosition, &vCollisionPosition);
			Maths::Store(Maths::HVec3(pCreate->m_vCollisionNormal), &vCollisionNormal);
			pEventInstance->SetCollisionPlane(vCollisionPosition, vCollisionNormal);
		}
	}
		
	AddSubscriber( GameMessaging::E_GameMsg_Update );
	return true;
}

void C_EffectTestActor::Update( F32 deltaTime )
{
	DynamicEffect::Particle::C_EventInstance* pEventInstance;
	if((pEventInstance = m_EventInstanceHandle.Object()))
	{
		if(g_EffectTestActorUseSpawner)
			pEventInstance->SetSpawner(m_pSpawner);
		else
			pEventInstance->SetSpawner(NULL);
	}
	else if(m_Repeating)
	{
		// if this is a persistent emitter then respawn the effect if the handle has gone null (from realtime xt editing, resets, etc)
		const C_EffectTestActorCreate* pCreate = (const C_EffectTestActorCreate*)(GetCreationData().GetData());
		m_EventInstanceHandle = DynamicEffect::Particle::XtInterface::SpawnEvent(m_ParticleEventId, Maths::C_QuatTrans(pCreate->GetCreationMatrix()));

		DynamicEffect::Particle::C_EventInstance* pEventInstance;
		if((pEventInstance = m_EventInstanceHandle.Object()))
		{
			m_Repeating = pEventInstance->GetEvent()->GetDefinition().bRepeating;
			pEventInstance->SetVelocity(Maths::HVec3(0.0f, 0.0f, 0.0f));
			pEventInstance->SetLighting(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
			pEventInstance->SetSpline(m_pActorCurve);
			pEventInstance->SetSpawner(m_pSpawner);

			MathsStorage::Float4A16 vCollisionPosition, vCollisionNormal;
			Maths::Store(pCreate->m_vCollisionPosition, &vCollisionPosition);
			Maths::Store(Maths::HVec3(pCreate->m_vCollisionNormal), &vCollisionNormal);
			pEventInstance->SetCollisionPlane(vCollisionPosition, vCollisionNormal);
		}
	}
}

} // namespace Actors

} // namespace Game


