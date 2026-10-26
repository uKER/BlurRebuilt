#include "stdafx.h"

#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

// #include <World_Render/Scene.h>

#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>
#include <Dynamic/Common.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>

// #include <system/Memory.h>

#include <Deferred/PrePass/DynamicLights.h>
#include <Game/GameWorld/GameWorld.h>

#include <actor_studio_msg_game/ActorStudioMessaging.h>
#include <Game/Messaging/GameflowMessages.h>

#include <Graphics/CutSceneLights.h>

#include "DynamicLightActor.h"

extern Debug::Colour g_debugBackLightColour;
extern Debug::Vector g_debugBackLightDirection;


//=========================================================================
//		Sorcery definitions
//=========================================================================

// Note: Dynamic Light Actor used to place a dynamic light in a scene. This is now done using the effect generators.
// This actor is now used to specify a backlight direction, colour and intensity



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::LightingRigCreationData,	"LightingRigActor")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_BackLight1,					"BackLight1")
	XT_FIELD(m_BackLight2,					"BackLight2")
	XT_FIELD(m_BackLight3,					"BackLight3")
	XT_FIELD(m_BackLight4,					"BackLight4")
XT_END_TYPE

XT_TYPE(Actors::DynamicLightCreationData,	"BacklightActor")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_Colour,						"Colour")
XT_END_TYPE




namespace Actors
{


//=========================================================================
//					Lighting Rig Actor Core Routines
//=========================================================================


LightingRigActor::LightingRigActor()
{
}

//=========================================================================

LightingRigActor::~LightingRigActor()
{
}

//=========================================================================

bool LightingRigActor::InitialiseImpl()					// Called first
{

	if (!C_GameActor::InitialiseImpl())
		return(false);

	pCreationData = GetCreationData();
	m_WorldMatrix = pCreationData->GetCreationMatrix();
	return(true);
}


//=========================================================================

bool LightingRigActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	AddSubscriber(Game::E_GameflowMessage_RaceLoaded);

	return(true);
}

//=========================================================================

void LightingRigActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
		case Resource::E_Notify_DataReady:
			break;
		
		case Resource::E_Notify_DataError:
			break;
		
		case Resource::E_Notify_NoData:
			break;
	}
}

//=========================================================================

bool LightingRigActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
		case Game::E_GameflowMessage_RaceLoaded:		// Tell backlights that they are part of this rig
		{
			StoreRigActorInLight(GetCreationData()->m_BackLight1);
			StoreRigActorInLight(GetCreationData()->m_BackLight2);
			StoreRigActorInLight(GetCreationData()->m_BackLight3);
			StoreRigActorInLight(GetCreationData()->m_BackLight4);
			break;
		}

		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}

//=========================================================================

void LightingRigActor::UpdateLights()
{
	SetupLightVals(0, GetCreationData()->m_BackLight1);
	SetupLightVals(1, GetCreationData()->m_BackLight2);
	SetupLightVals(2, GetCreationData()->m_BackLight3);
	SetupLightVals(3, GetCreationData()->m_BackLight4);
}

//=========================================================================

void LightingRigActor::SetupLightVals(S32 index, const ::Actors::C_PosActorCreate *backlight)
{
/*						// NOW JUST USING SORCERY
	if (backlight)
	{
		const DynamicLightCreationData	*backlightdata = (DynamicLightCreationData *)backlight;
//		Maths::HVec3					dir = backlightdata->GetCreationMatrix().AZ();


		Actor::C_ActorHandle handle = backlightdata->GetActorHandle();
		if (handle)
		{
			DynamicLightActor *light = (DynamicLightActor *)GetActorList()->GetActorPtr(handle);
			if (light)
			{
				Maths::HVec3	dir = light->m_WorldMatrix.AZ();
				CutScene::SetLight(index, MathsStorage::Float3(-dir.X(), -dir.Y(), -dir.Z()), MathsStorage::Float3(backlightdata->m_Colour.r, backlightdata->m_Colour.g, backlightdata->m_Colour.b), backlightdata->m_Colour.a);
			}
		}
	}
	else
	{
		CutScene::SetLight(index, MathsStorage::Float3(1.0f, 0.0f, 0.0f), MathsStorage::Float3(0.0f, 0.0f, 0.0f), 1.0f);
	}
*/
}

//=========================================================================

void LightingRigActor::StoreRigActorInLight(const ::Actors::C_PosActorCreate *backlight)
{
	if (backlight)
	{
		Actor::C_ActorHandle lighthandle = backlight->GetActorHandle();
		if (lighthandle)
		{
			DynamicLightActor *light = (DynamicLightActor *)GetActorList()->GetActorPtr(lighthandle);
			light->m_RigActor = this;			// Backlight can now tell this rig actor to update if it's position/colour vals change dynamically
		}
	}
}



//=========================================================================
//					DynamicLight Actor Core Routines
//=========================================================================


DynamicLightActor::DynamicLightActor()
{
}

//=========================================================================

DynamicLightActor::~DynamicLightActor()
{
}

//=========================================================================

bool DynamicLightActor::InitialiseImpl()					// Called first
{

	if (!C_GameActor::InitialiseImpl())
		return(false);

	pCreationData = GetCreationData();
	m_WorldMatrix = pCreationData->GetCreationMatrix();
	m_Selected = false;

	return(true);
}


//=========================================================================

bool DynamicLightActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

//	AddSubscriber( GameMessaging::E_GameMsg_Update );		// Need to update every frame (at least for now)
	AddSubscriber(ActorMsg::E_DynamicObjMsg_SorceryUpdate);

	return(true);
}

//=========================================================================

void DynamicLightActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
		case Resource::E_Notify_DataReady:
			break;
		
		case Resource::E_Notify_DataError:
			break;
		
		case Resource::E_Notify_NoData:
			break;
	}
}

//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool DynamicLightActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
		case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		{
			const ActorStudioMsgGame::C_DragMsg* msg = Messaging::MessageCast<ActorStudioMsgGame::C_DragMsg>(pMessage);
			m_WorldMatrix = msg->GetValue();
//			GetCreationData()->m_WorldMatrix = msg->GetValue();
			if (m_RigActor)
				m_RigActor->UpdateLights();				// Update all lights since this one has changed (this could be any of the four in the rig so just update the lot)
			break;
		}

		//case ActorStudioMsgShared::E_ActorStudioMsg_EndDrag:
		//{
		//	
		//	break;
		//}

		case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
		{
			m_Selected = true;
			break;
		}

		case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
		{
			m_Selected = false;
			break;
		}

		case ActorMsg::E_DynamicObjMsg_SorceryUpdate:
		{
			if (m_Selected)
				if (m_RigActor)
					m_RigActor->UpdateLights();				// Update all lights since this one has changed (this could be any of the four in the rig so just update the lot)
			break;
		}

		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}

//=========================================================================

void DynamicLightActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	debugRenderPtr->Draw3dMatrix(m_WorldMatrix, 0.5f);
	if(m_Selected)
		debugRenderPtr->Draw3dText(Maths::HVec3(m_WorldMatrix.Translation()), "Selected");
}


}// End namespace Actors





