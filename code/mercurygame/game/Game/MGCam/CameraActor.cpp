#include "stdafx.h"
#pragma warning (push, 4)
#include "CameraActor.h"

#include "CameraSystem.h"
#include <camera2/camera.h>
#include <misc_shared/Messaging/GameMessaging.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/Components/Messages/MovementMessages.h>

namespace MGCam
{
using namespace Maths;

void C_CameraActorCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_PosActorCreate::DoSorceryCallback(pCallbackFn);

	if (m_pSorceryMovementComponentCreate != NULL)
		DoComponentSorceryCallback(pCallbackFn, *m_pSorceryMovementComponentCreate, &m_pMovementCreator);
}

C_CameraActor::C_CameraActor()
{
	m_Camera = 0;
	m_Aiming = false;
}

C_CameraActor::~C_CameraActor()
{
	KillImpl();
}

bool C_CameraActor::InitialiseImpl()
{
	m_Matrix = GetCreationData()->GetCreationMatrix();
	
	const C_CameraActorCreate* pCamActorCreate = GetCreationData();
	if (pCamActorCreate && pCamActorCreate->GetMovementCreator())
	{
		if (InitialiseComponent(pCamActorCreate->m_pMovementCreator) == NULL)
			return false;
	}

	return true;
}

bool C_CameraActor::CreateImpl()
{
	m_Camera = new Cam2::C_Camera;
	if (!m_Camera)
		return false;

	m_Camera->SetPosition(m_Matrix.AW());
	m_Camera->SetOrient((C_Quaternion)m_Matrix.Rotation());
	m_Camera->GetPrevPosition();

	AddSubscriber(GameMessaging::E_GameMsg_Update);
	return true;
}

void C_CameraActor::KillImpl()
{
	// Call GameFeedRemove just in case this camera is being shown, to avoid a hanging pointer:
	MGCam::System().GameFeedRemove(m_Camera);

	SAFE_DELETE_PTR(m_Camera)
}

bool C_CameraActor::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_Update:
		{
			if (m_Aiming)
				m_Camera->LookAt(m_AimPoint);

			return true;
		}
		
		case ActorMsg::E_GameActorMsg_AnimateTransform:
		{
			const ActorMsg::C_ActorAnimateTransformMsg *pMsg = Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage);
			
			if (pMsg)
			{
				m_Camera->GetPrevPosition();
				m_Camera->SetPosition(pMsg->Transform().m_Translation);
				m_Camera->SetOrient(pMsg->Transform().m_Quaternion);
				m_Matrix = m_Camera->Mat43();
				
				if (m_Aiming)
					m_Camera->LookAt(m_AimPoint);
				return true;
			}
		}

		case ActorMsg::E_GameActorMsg_SetTransform:
		{
			const ActorMsg::C_ActorSetTransformMsg *pMsg = Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage);
			if (pMsg)
			{
				m_Camera->GetPrevPosition();
				m_Camera->SetPosition(pMsg->Transform().m_Translation);
				m_Camera->SetOrient(pMsg->Transform().m_Quaternion);
				m_Matrix = m_Camera->Mat43();
			}
			
			return true;
		}

		default:
			return Actor::C_Actor::OnMessage(pMessage);
	}
}

#ifndef CFG_GOLD
void C_CameraActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!m_Camera)
		return;
	
	debugRenderPtr->Draw3dText(m_Camera->Position(), 1.f, GetActorName());
	m_Camera->DebugRender();

	Actor::C_Actor::DebugRender(debugRenderPtr);
}
#endif

const Maths::HMat43& C_CameraActor::GetMatrix(void) const
{
	return m_Matrix;
}

}

XT_TYPE(MGCam::C_CameraActorCreate, "CameraActor")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_pSorceryMovementComponentCreate, "MovementComponent")
XT_END_TYPE

#pragma warning (pop)
