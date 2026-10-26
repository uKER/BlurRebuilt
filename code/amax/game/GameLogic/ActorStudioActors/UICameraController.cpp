#include "stdafx.h"

#include <Game\Game.h>
#include "Game/AmaxCam/DirectorActor.h"

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameApp.h>

#include <GameLogic/ActorStudioActors/UICameraController.h>



//#pragma optimize("", off)



//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)


XT_TYPE(::Actors::UICameraControllerCreationData,		"UICameraController")
	XT_BASE(::Actors::C_PosActorCreate)
	XT_FIELD(m_Director,							"Director")
	XT_FIELD(m_Viewport,							"Viewport")
	XT_FIELD(m_Camera,								"DynamicCamera")
XT_END_TYPE



//=========================================================================
//					Animated Camera Core Routines
//=========================================================================

namespace Actors
{

//using namespace Maths;


UICameraController::UICameraController()
{
}

//=========================================================================

UICameraController::~UICameraController()
{
}

//=========================================================================

bool UICameraController::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	m_WorldMatrix = GetCreationData()->GetCreationMatrix();

	return(true);
}

//=========================================================================
/*
bool UICameraController::PrimeImpl()
{

}
*/
//=========================================================================

bool UICameraController::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);


	AddSubscriber( ActorMsg::E_DynamicObjMsg_UICameraControl );		
	AddSubscriber( ActorMsg::E_DynamicObjMsg_ChangeCameraBehaviour );		

	return(true);
}



//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool UICameraController::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_DynamicObjMsg_UICameraControl:
		{
			const ActorMsg::C_UICameraControlMsg* msg = Messaging::MessageCast<ActorMsg::C_UICameraControlMsg>(pMessage);

			if (msg->m_Viewport != GetCreationData()->m_Viewport)				// Ignore this message if it applies to a different viewport
				break;

			if ((!GetCreationData()->m_Director) || (!GetCreationData()->m_Camera))		// Ignore if camera or director are invalid
			{
				LOGWARN( Resource::g_Log, "Invalid Director or Dynamic Camera in UI Camera Controller");
				break;
			}

			ActorMsg::C_FreezeCameraStateMsg	freezemsg;					// Stop any camera blends that might be happening
			SendMessageToActor(GetCreationData()->m_Camera->GetActorHandle(), &freezemsg);


			ActorMsg::C_SimpleCameraBehaviourMsg		cammsg;					// Send Blend Message to Dynamic camera
			cammsg.m_Blending			= msg->m_MovementType;
			cammsg.m_BlendTime			= msg->m_BlendDuration;
			cammsg.m_CameraProperties	= RLDatabase::Instance().mRLSystems[0]->mUICameraParameters;			// w .Clear();
			cammsg.m_FOV				= RLDatabase::Instance().mRLSystems[0]->mUICameraParameters->m_FOV;		// Simple cam message uses an override for the FOV
			cammsg.m_CameraShake.Clear();
			cammsg.m_ParentActor		= NULL;

			Maths::HMat43 worldmat		= Load(msg->m_WorldMat);
			cammsg.m_Position			= Store(worldmat.Translation());
			cammsg.m_PitchHeadingRole	= Store((Maths::HVec3)worldmat.Rotation().GetRotationPHR());
			cammsg.m_PitchHeadingRole.vx = Maths::RadToDeg(cammsg.m_PitchHeadingRole.vx);			// Convert to Degs
			cammsg.m_PitchHeadingRole.vy = Maths::RadToDeg(cammsg.m_PitchHeadingRole.vy);
			cammsg.m_PitchHeadingRole.vz = Maths::RadToDeg(cammsg.m_PitchHeadingRole.vz);

			SendMessageToActor(GetCreationData()->m_Camera->GetActorHandle(), &cammsg);

			ActorMsg::C_CameraUpdateMsg cameraUpdateMsg( 0.0f );
			SendMessageToActor(GetCreationData()->m_Camera->GetActorHandle(), &cameraUpdateMsg );

			AmaxCam::C_ShowLevelCameraMsg showmsg;			// Tell director actor to use this camera for this viewport
			showmsg.m_ViewportId = GetCreationData()->m_Viewport;
			showmsg.m_CameraActor = GetCreationData()->m_Camera->GetActorHandle();
			SendMessageToActor(GetCreationData()->m_Director->GetActorHandle(), &showmsg);
			break;
		}



		case ActorMsg::E_DynamicObjMsg_ChangeCameraBehaviour:		// Note: This doesn't check for the correct viewport (it assumes 0)
		{
			if ((!GetCreationData()->m_Director) || (!GetCreationData()->m_Camera))		// Ignore if camera or director are invalid
			{
				LOGWARN( Resource::g_Log, "Invalid Director or Dynamic Camera in UI Camera Controller");
				break;
			}

			ActorMsg::C_FreezeCameraStateMsg	freezemsg;						// Stop any camera blends that might be happening
			SendMessageToActor(GetCreationData()->m_Camera->GetActorHandle(), &freezemsg);

			SendMessageToActor(GetCreationData()->m_Camera->GetActorHandle(), pMessage);	// Pass on the message directly to the camera

			ActorMsg::C_CameraUpdateMsg cameraUpdateMsg( 0.0f );
			SendMessageToActor(GetCreationData()->m_Camera->GetActorHandle(), &cameraUpdateMsg );

			AmaxCam::C_ShowLevelCameraMsg showmsg;			// Tell director actor to use this camera for this viewport
			showmsg.m_ViewportId = GetCreationData()->m_Viewport;
			showmsg.m_CameraActor = GetCreationData()->m_Camera->GetActorHandle();
			SendMessageToActor(GetCreationData()->m_Director->GetActorHandle(), &showmsg);
			break;
		}

		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}


}// End namespace Actors



//#pragma optimize("", on)


