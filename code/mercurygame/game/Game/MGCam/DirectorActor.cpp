#include "stdafx.h"
#pragma warning (push, 4)
#include "DirectorActor.h"
#include "CameraActor.h"
#include "CameraSystem.h"

#include <actor_system/ActorSys.h>

namespace MGCam
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

			//MGCam::System().GameFeedShow(E_GameCam_VehicleFeed);
			return true;
		}
		
 		case E_DirectorMsg_ShowLevelCamera:
		{
			const C_ShowLevelCameraMsg* msg = Messaging::MessageCast_Safe<C_ShowLevelCameraMsg>(pMessage);
			if (msg == 0)
				return false;
			
			Actor::C_ActorHandle handle = msg->GetActorHandle();
			const Cam2::C_Camera* cam = HandleToCamera(handle);

			System().GameFeedShow(cam);
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

	const C_CameraActor* ca = Actor::ActorCast_Safe<C_CameraActor>(actor);
	if (!ca)
		return 0;

	return ca->GetCamera();
}

const Messaging::C_MessageBase* C_XtShowLevelCameraMsg::GetMessage(const Actor::C_Actor*)
{
	if (m_CameraActor)
		m_Msg.SetActorHandle(m_CameraActor->GetActorHandle());
	return &m_Msg;
}

const Messaging::C_MessageBase* C_XtShowGameCameraMsg::GetMessage(const Actor::C_Actor*)
{
	return &m_Msg;
}

} // End of namespace


XT_TYPE(MGCam::C_DirectorActorCreate, "DirectorActor")
	XT_BASE(Actors::C_GameActorCreate)
XT_END_TYPE

XT_TYPE(MGCam::C_XtShowGameCameraMsg, "ShowGameCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(MGCam::C_XtShowLevelCameraMsg, "ShowLevelCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_CameraActor, "CameraActor")
XT_END_TYPE

#pragma warning (pop)
