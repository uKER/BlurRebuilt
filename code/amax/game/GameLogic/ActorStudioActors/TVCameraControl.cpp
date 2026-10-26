#include "stdafx.h"

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameApp.h>

#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>

//#include <Graphics/PostFXSettings.h>

#include "TVCameraControl.h"


//OPTIMISE_OFF
//#pragma optimize("", off)



//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::TVCameraControlCreationData,		"TVCameraControl")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_Region,								"Region")
	XT_FIELD(m_CameraMessage,						"CameraMessage")
	XT_FIELD(m_OptionalSecondMessage,				"OptionalSecondMessage")
XT_END_TYPE


//=========================================================================
//					CameraTarget Actor Core Routines
//=========================================================================



namespace Actors
{

using namespace Maths;


//=========================================================================

TVCameraControlActor::TVCameraControlActor()
{
	m_Active = false;
}

//=========================================================================

TVCameraControlActor::~TVCameraControlActor()
{
}

//=========================================================================

bool TVCameraControlActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	m_WorldMatrix = GetCreationData()->GetCreationMatrix();

	return(true);
}


//=========================================================================

bool TVCameraControlActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	AddSubscriber(ActorMsg::E_DynamicObjMsg_EnableTVCameras);
	AddSubscriber(ActorMsg::E_DynamicObjMsg_DisableTVCameras);

	Initialise();

	return(true);
}


//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool TVCameraControlActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_DynamicObjMsg_EnableTVCameras:
		{
			AddSubscriber(GameMessaging::E_GameMsg_PostUpdate);
			Initialise();
			break;
		}

		case ActorMsg::E_DynamicObjMsg_DisableTVCameras:
		{
			RemoveSubscriber(GameMessaging::E_GameMsg_PostUpdate);
			break;
		}

		case GameMessaging::E_GameMsg_PostUpdate:
		{
			if (PlayerInRegion())
			{
				if (!m_Active)				// Send camera message if just entered
				{
					ActorMsg::C_XtChangeCameraBehaviourMsg	*msgbase;

					msgbase = GetCreationData()->m_CameraMessage;				// Main camera message
					if (msgbase)
					{
						const Messaging::C_MessageBase	*cammsg = msgbase->GetMessage(this);
						C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( cammsg );
					}

					msgbase = GetCreationData()->m_OptionalSecondMessage;		// Secondary camera message (usually for small camera pans)
					if (msgbase)
					{
						const Messaging::C_MessageBase	*cammsg = msgbase->GetMessage(this);
						C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( cammsg );
					}

				}
				m_Active = true;
			}
			else
			{
				m_Active = false;
			}
		
			break;
		}


		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}


//=========================================================================

void TVCameraControlActor::Initialise()
{
	XtSphere		sphere = GetCreationData()->m_Region;


	m_Active = false;				// Vehicle not in region
	
	m_Centre = Maths::HVec3(sphere.position.vx, sphere.position.vy, sphere.position.vz);
	m_Centre += m_WorldMatrix.Translation();
	m_Radius2 = sphere.radius * sphere.radius;

	Game::Actors::RaceLogicCore	*pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	m_PlayerVehicle = (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(0);		// Player always has index 0
}

//=========================================================================

bool TVCameraControlActor::PlayerInRegion()
{
	Maths::HVec3	playerpos = m_PlayerVehicle->GetMatrix().Translation();
	Maths::HVec3	distvec = playerpos - m_Centre;
	F32				dist = distvec.Length2();

	if (dist < m_Radius2)
		return(true);

	return(false);
}



}// End namespace Actors



//#pragma optimize("", on)



//=========================================================================
//						Might be useful one day
//=========================================================================


//		LOGINFO( Resource::g_Log, "THIS IS A DEBUG MESSAGE\n");




