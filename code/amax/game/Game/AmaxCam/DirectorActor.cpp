#include "stdafx.h"
#pragma warning (push, 4)
#include "DirectorActor.h"
#include "CameraActor.h"
#include "CameraSystem.h"
#include "Helper.h"

#pragma warning (push, 3)
#include <GameLogic/ActorStudioActors/DynamicCameraActor.h>
#include <GameLogic/ActorStudioActors/ShowRoomCamera.h>
#include <GameLogic/ActorStudioActors/AnimatedCamera.h>
#include <actor_system/ActorSys.h>
#include <Game/GameWorld/GameWorld.h>
#pragma warning (pop)

#include <camera2/Camera.h>

namespace AmaxCam
{

C_DirectorActor::C_DirectorActor()
{
}

C_DirectorActor::~C_DirectorActor()
{
}

bool C_DirectorActor::InitialiseImpl()
{
	return true;
}

bool C_DirectorActor::CreateImpl()
{
	return true;
}

bool C_DirectorActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
 		case E_DirectorMsg_ShowGameCamera:
		{
			const C_ShowGameCameraMsg* msg = Messaging::MessageCast_Safe<C_ShowGameCameraMsg>(pMessage);
			if (msg == 0)
				return false;

			AmaxCam::CameraSystem().GameFeedShow(msg->m_ViewportId, msg->m_CameraType, msg->m_WhichFeed);
			return true;
		}
		
 		case E_DirectorMsg_ShowLevelCamera:
		{
			//OutputDebugString("E_DirectorMsg_ShowLevelCamera.\n");
			const C_ShowLevelCameraMsg* msg = Messaging::MessageCast_Safe<C_ShowLevelCameraMsg>(pMessage);
			if (msg == 0)
			{
				//OutputDebugString("Bail, no msg.\n");
				return false;
			}

			//Detect camera participating in startup race condition.
			{
				Actor::C_ActorList* gal = GetActorList();
				const Actor::C_Actor* actor = NULL;
				if (gal) {
					actor = gal->GetActorPtr(msg->m_CameraActor);
				}
				if (actor) {
					DataRef::C_SorceryUid uid = actor->GetCreationData()->GetSorceryUid();
					if (uid == 0xd7317c3d)
					{
						//OutputDebugString("Ignoring rogue camera.\n");
						return true;
					}
				}
			}
			
			const Cam2::C_Camera* cam = HandleToCamera(msg->m_CameraActor);

			AmaxCam::CameraSystem().GameFeedShow(msg->m_ViewportId, cam, msg->m_WhichFeed);
			return true;
		}

 		case E_DirectorMsg_ShowEffect:
		{
			const C_ShowEffectMsg* msg = Messaging::MessageCast_Safe<C_ShowEffectMsg>(pMessage);
			if (msg == 0)
				return false;
			
			Game::C_GameWorld *gameWorld = GetGameWorld();
			if (gameWorld)
				gameWorld->CreateEnvironmentSettingsFromSorcery( msg->m_EffectType == E_Effect_BlackAndWhite );

			return true;
		}

		default:
			return Actor::C_Actor::OnMessage(pMessage);
	}
}

const Cam2::C_Camera* C_DirectorActor::HandleToCamera(const Actor::C_ActorHandle& handle)
{
	Actor::C_ActorList* gal = GetActorList();
	if (!gal)
		return 0;

	const Actor::C_Actor* actor = gal->GetActorPtr(handle);
	if (!actor)
		return 0;

	switch(actor->GetActorTypeId())
	{
		case Game::E_ActorTypeId_CameraActor:
		{
			const C_CameraActor* ca = Actor::ActorCast_Safe<C_CameraActor>(actor);
			if (!ca)
				return 0;

			return ca->GetCamera();
			break;
		}

		case Game::E_ActorTypeId_DynamicCameraActor:
		{
			const Actors::DynamicCameraActor* dca = Actor::ActorCast_Safe<Actors::DynamicCameraActor>(actor);
			if (!dca)
				return 0;

			return dca->GetCamera();
			break;
		}
		
		case Game::E_ActorTypeId_ShowRoomCamera:
		{
			const Actors::ShowRoomCamera* srca = Actor::ActorCast_Safe<Actors::ShowRoomCamera>(actor);
			if (!srca)
				return 0;

			return srca->GetCamera();
			break;
		}
		
		case Game::E_ActorTypeId_AnimatedCamera:
		{
			const Actors::AnimatedCamera* aca = Actor::ActorCast_Safe<Actors::AnimatedCamera>(actor);
			if (!aca)
				return 0;

			return aca->GetCamera();
			break;
		}
		
		default:
			return 0;
	}
}

class C_XtShowGameCameraMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtShowGameCameraMsg()
	{
		m_CameraType = E_GameCam_None;
		m_ViewportId = 0;
		m_WhichFeed = E_WhichFeed_Both;
	}

	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext) OVERRIDE
	{
		m_Msg.m_CameraType = m_CameraType;
		m_Msg.m_ViewportId  = m_ViewportId;
		m_Msg.m_WhichFeed = m_WhichFeed;
		return &m_Msg;
	}

	C_ShowGameCameraMsg m_Msg;
	E_GameCam m_CameraType;
	S32 m_ViewportId;
	E_WhichFeed m_WhichFeed;
};

class C_XtShowLevelCameraMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtShowLevelCameraMsg()
	{
		m_CameraActor = 0;
		m_ViewportId = 0;
		m_WhichFeed = E_WhichFeed_Both;
	}

	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *) OVERRIDE
	{
		if (m_CameraActor)
		{
			m_Msg.m_CameraActor = m_CameraActor->GetActorHandle();
		}
		
		m_Msg.m_ViewportId  = m_ViewportId;
		m_Msg.m_WhichFeed = m_WhichFeed;

		return &m_Msg;
	}

	C_ShowLevelCameraMsg m_Msg;
	Actors::C_GameActorCreate* m_CameraActor;			// Tony - In case we want to use a game specific camera type
	S32 m_ViewportId;
	E_WhichFeed m_WhichFeed;
};

const Messaging::C_MessageBase* C_XtShowEffectMsg::GetMessage(const Actor::C_Actor *)
{
	m_Msg.m_EffectType = m_EffectType;
	return &m_Msg;
}

} // End of namespace

XT_TYPE(AmaxCam::C_DirectorActorCreate, "DirectorActor")
	XT_BASE(Actors::C_GameActorCreate)
XT_END_TYPE

XT_TYPE(AmaxCam::C_XtShowGameCameraMsg, "ShowGameCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_CameraType, "CameraType")
	XT_FIELD(m_ViewportId, "ViewportId")
	XT_FIELD(m_WhichFeed, "WhichFeed")
XT_END_TYPE

XT_TYPE(AmaxCam::E_GameCam, "AmaxGameCam")
	XT_OPTION(AmaxCam::E_GameCam_None, "None")
	XT_OPTION(AmaxCam::E_GameCam_VehicleFeed, "VehicleFeed")
XT_END_TYPE

XT_TYPE(AmaxCam::C_XtShowLevelCameraMsg, "ShowLevelCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_CameraActor, "CameraActor")
	XT_FIELD(m_ViewportId, "ViewportId")
	XT_FIELD(m_WhichFeed, "WhichFeed")
XT_END_TYPE

XT_TYPE(AmaxCam::E_Effect, "AmaxEffect")
	XT_OPTION(AmaxCam::E_Effect_Normal, "Normal")
	XT_OPTION(AmaxCam::E_Effect_BlackAndWhite, "BlackAndWhite")
XT_END_TYPE

XT_TYPE(AmaxCam::C_XtShowEffectMsg, "ShowEffect")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_EffectType, "EffectType")
XT_END_TYPE

XT_TYPE(AmaxCam::E_WhichFeed, "AmaxWhichFeed")
	XT_OPTION(AmaxCam::E_WhichFeed_Both, "Both")
	XT_OPTION(AmaxCam::E_WhichFeed_Current, "Current")
	XT_OPTION(AmaxCam::E_WhichFeed_Future, "Future")
XT_END_TYPE

#pragma warning (pop)
