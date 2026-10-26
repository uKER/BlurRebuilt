#ifndef PARTICLEEVENTTESTACTOR_H
#define PARTICLEEVENTTESTACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>

namespace Curve
{
	class C_Hermite3DData;
	class C_ActorCurve3D;
}

namespace Game
{
	class C_GameWorld;

	namespace Actors
	{
	
		class C_EffectTestActorCreate : public ::Actors::C_PosActorCreate
		{
		public:
			static Actor::C_ActorTypeId ClassType()			{ return E_ActorTypeId_EffectTest; }
			const Actor::C_ActorTypeId GetType() const		{ return E_ActorTypeId_EffectTest; }

			U32				m_ParticleEventIndex;
			Maths::HVec3	m_vCollisionPosition;
			Maths::C_Normal	m_vCollisionNormal;
		};

		class C_EffectTestActor : public C_GameActor
		{
		public:
			C_EffectTestActor();
			virtual ~C_EffectTestActor();
			static const Actor::C_ActorTypeId	TypeId()			{ return E_ActorTypeId_EffectTest; }
			static const Btl::String32			TypeName()			{ return "EffectTest"; }
			virtual bool OnMessage(const Messaging::C_MessageBase *pMessage);

		protected:
			virtual bool InitialiseImpl();
			virtual bool CreateImpl();

		private:
			void Update( F32 deltaTime );

			DynamicEffect::Particle::C_EventInstanceHandle		m_EventInstanceHandle;
			U32													m_ParticleEventId;
			bool												m_Repeating;
			Curve::C_Hermite3DData*								m_pCurveData;
			Curve::C_ActorCurve3D*								m_pActorCurve;
		};

	} // namespace Actors


} // namespace Game

#endif //  ANIMATIONTESTACTOR_H
