
#ifndef NOT_USING_AUDIO_COMPONENETS

#include "../GameComponents.h"
#include <camera2/stock/DebugFeed.h>	// [Blur2PC]
#include "../AudioComponents.h"

#include "actor_system/Actor.h"
#include "GameApp.h"
////#include "Game.h"

#include "Game/GameActors/Components/Messages/AudioMessages.h"

#include "BaseMessageHandler.h"
#include <misc_shared/Messaging/GameMessaging.h>
#include <game/AmaxCam/CameraSystem.h>

#include <game/AmaxCam/VehicleFeed.h>


#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <Game/Messaging/PickupActorMessages.h>
#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"

#include <shared_actors/Components/Impl/ResourceLoaderImpl.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <Game/Messaging/GameflowMessages.h>

#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include "Game\Messaging\gamemessages.h"
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <GameLogic/ActorStudioActors/DynamicObjectActor.h>

#include <GameLogic/ActorStudioActors/PhysicalObjectActor.h>

#include <Vehicle/VehicleModel.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <game\GameActors\Vehicles\VehicleActor.h>

#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <GameAudio/GameAudioLayer.h>

//#include <shared_actors/Components/Interfaces/I_PhysicsBodyComponents.h>


// vehicle

//#include "racingphysics/vehicles/vehiclemanager.h"
#include "racingphysics/vehicles/car/car.h"

//raytrace stuff
#include <gravity/narrowphase/Ray.h>
#include <gravity/sdk/BodyHelper.h>
#include <gravity/sdk/ShapeDescriptors.h>
#include <shared_actors/SharedActorUtils.h>

#include <Game/GameDef/Polyflags.h>
#include <Game/GameDef/Materials.h>

#include <SchedulerConfig.h>
#include <AI/AI_Amax.h>

#include <shared_actors/Utility/VisibilityDetection.h>
#include <GameLogic\GameData\AmaxDialogue.h>
#include <GameLogic\Messaging\InGameDialogueMessages.h>


#include <Gravity/SDK/ShapeDescriptors.h>
#include <Gravity/Triggers/TriggerSystem.h>
#include <gravity/triggers/TriggerEvent.h>

#include <shared_actors/SharedActorUtils.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"
//#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include "UISubsystem.h"

#include "CollisionSubsystem.h"


#include <shared_actors/Components/Messages/MovementMessages.h>

#include <shared_actors/Components/Impl/MovementComponents.h>

#include <Shared_Actors/Components/Impl/MovementSpline.h>

#include <Game/Messaging/GameflowMessages.h>

#include <system/debug/I_DebugRender.h>

#include <GameLogic/ActorStudioActors/PickupPositionActor.h>

#include <Ui/UiInterface.h>

#include <camera2/Camera.h>

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

Debug::Logger &g_AudioVehiclesLogger( Debug::GetLogger("Audio.Game.Vehicles") );

#define NEW_DISTANCE_SCALING
//Lodding for occlusion is WIP for cars
//#define VEHICLE_OCCLUSION_LODDING

#ifdef PLATFORM_PS3
#define AUDIBLE_CARS 3
#else
#define AUDIBLE_CARS 3
#endif
#define AUDIBLE_CARS_SPLITSCREEN 5

Debug::Bool gbAICarsEnabled("Audio.MessageHandler.Vehicles.AICarsEnabled", true);
Debug::Bool gbPlayerCarEngineEnabled("Audio.MessageHandler.Vehicles.PlayerCarEngineEnabled", true);
Debug::Bool gbPlayerCarCollisionsEnabled("Audio.MessageHandler.Vehicles.PlayerCarCollisionsEnabled", true);
Debug::Bool gbAICarEngineEnabled("Audio.MessageHandler.Vehicles.AICarEngineEnabled", true);
Debug::Bool gbAICarCollisionsEnabled("Audio.MessageHandler.Vehicles.AICarCollisionsEnabled", true);

Debug::Float gfCarCollisionImpulseThresholdHighNew("Audio.MessageHandler.Vehicles.CarCollisionImpulseThresholdHighNew", 50.f);
Debug::Float gfCarCollisionImpulseThresholdLowNew("Audio.MessageHandler.Vehicles.CarCollisionImpulseThresholdLowNew", 0.8f);
Debug::Float gfCarCollisionPeakDecayRatioNew("Audio.MessageHandler.Vehicles.CarCollisionPeakDecayRatioNew", 0.9f);

Debug::Float gfCarScrapeFrictionThresholdLow("Audio.MessageHandler.Vehicles.CarScrapeFrictionThresholdLow", 0.01f);
Debug::Float gfScrapeFrictionPeakDecayRatioNew("Audio.MessageHandler.Vehicles.ScrapeFrictionPeakDecayRatioNew", 0.9f);

Debug::Float gfDynamicObjCollisionImpulseThresholdLowNew("Audio.MessageHandler.DynamicObjects.DynamicObjCollisionImpulseThresholdLowNew", 0.001f, Debug::Range(0.0f, 100.f));
Debug::Float gfDynamicObjCollisionPeakDecayRatioNew("Audio.MessageHandler.DynamicObjects.DynamicObjCollisionPeakDecayRatioNew", 0.9f);

Debug::Bool bDebugUsingDistanceScaling("Audio.MessageHandler.Vehicles.UsingDistanceScaling", true);

Debug::Bool g_RenderReflectionPositions("Audio.MessageHandler.Vehicles.ShowReflectionPositions", false);

Debug::Float g_RenderReflectionX("Audio.MessageHandler.Vehicles.X", 1.f, Debug::Range(-1.0f, 1.f));
Debug::Float g_RenderReflectionY("Audio.MessageHandler.Vehicles.Y", 0, Debug::Range(-1.0f, 1.f));
Debug::Float g_RenderReflectionZ("Audio.MessageHandler.Vehicles.Z", 0, Debug::Range(-1.0f, 1.f));

Debug::Float g_RenderReflectionAngle("Audio.MessageHandler.Vehicles.Angle", 270, Debug::Range(-360.0f, 360.f));

Debug::Bool g_DoReflections("Audio.MessageHandler.Vehicles.DoReflections", true, Debug::Persist());

Debug::Float g_RayLength("Audio.MessageHandler.Vehicles.RayLength", 100.0, Debug::Range(0.f, 1000.f));

Debug::Float g_RayHeight("Audio.MessageHandler.Vehicles.RayHeight", .53);

Debug::Bool g_AIReflections("Audio.MessageHandler.Vehicles.DoAIReflections",true, Debug::Persist());

// Sam: the max distance of dynamic objects and physical objects is 100 so could this be reduced?
// only issue could be when an AI car in the distance hits a dynamic object which bounces past the camera
Debug::Float g_DOCullDistance("Audio.MessageHandler.DynamicObjects.CullDistance", 350.0, Debug::Range(0.f, 1000.f));

Debug::Float g_TriggerBoxYOffset("Audio.MessageHandler.Vehicles.TriggerActorYOffset", 0.0, Debug::Range(0.f, 10.f));
Debug::Float g_TriggerBoxZOffset("Audio.MessageHandler.Vehicles.TriggerActorZOffset", 0.0, Debug::Range(0.f, 10.f));


Debug::Bool g_doInstanceSuspending("Audio.MessageHandler.Wheels.DoInstanceSuspend",true);

Debug::Bool g_usingMultipleListeners("Audio.MessageHandler.Default.UsingMultipleListeners",true);

Debug::Bool g_RenderDistanceScale("Audio.MessageHandler.Vehicles.ShowDistanceScale", false, Debug::Persist());

Debug::Bool g_UsingNewDistanceScale("Audio.MessageHandler.Vehicles.UsingNewDistanceScale", true, Debug::Persist());

Debug::Bool g_SuspendingZeroEntity("Audio.MessageHandler.Vehicles.SuspendingZeroEntity", true, Debug::Persist());

Debug::Bool g_RenderBumperCamPos("Audio.MessageHandler.Vehicles.ShowBumperCamPos", false);
Debug::Float g_RenderBumperCamPosOffset("Audio.MessageHandler.Vehicles.BumperCamPosOffset", .1);


Debug::Bool g_RenderAudioPositions("Audio.ShowAudioPositions", false);

Debug::Bool g_SlowMoOverrideControls("Audio.SlowMo.OverrideControls", true);
Debug::Float g_SlowMoThrottle("Audio.SlowMo.Throttle", 0.f, Debug::Range(0.f, 1.f));
Debug::Float g_SlowMoRPMDecay("Audio.SlowMo.RPMDecay", 0.995f, Debug::Range(0.95f, 1.f));

Debug::Bool g_doWheelInstanceSuspendBailOut("Audio.MessageHandler.Wheels.DoSuspendBailout", true);

Debug::Bool gbDeleteIfOutOfRange("Audio.MessageHandler.DynamicObjects.ReleaseIfOutOfRange", true);


Debug::Bool gbUseCarScaleForWheels("Audio.MessageHandler.Wheels.UseCarScaleForWheels", true);

Debug::Float g_MaxCollisionDistance("Audio.MessageHandler.Vehicles.MaxCollisionDistance", 250.f, Debug::Range(0.0f, 10000.f));


Debug::Bool g_DoOcclusion("Audio.MessageHandler.Vehicles.Occlusion.DoOcclusion", true);
Debug::Float g_OcclusionRayDistanceMax("Audio.MessageHandler.Vehicles.Occlusion.OcclusionRayDistanceMax", 250.0, Debug::Range(0.f, 1000.f));
Debug::Bool g_RenderOcclusionRays("Audio.MessageHandler.Vehicles.Occlusion.RenderOcclusionRays", false);
Debug::Float g_OcclusionRayHeight("Audio.MessageHandler.Vehicles.Occlusion.OcclusionRayHeight", 0.53);

Debug::Float g_DO_lifetime("Audio.MessageHandler.DynamicObjects.DOLifetime", 3.f);

Debug::Bool g_doSharedEntityMatrixUpdate("Audio.MessageHandler.Vehicles.DoSharedMatricUpdate", true);

Debug::Float gbWheelSuspendDelay("Audio.MessageHandler.Wheels.SuspendingDelay", 2.0f);

#ifdef VEHICLE_OCCLUSION_LODDING
Debug::Int g_RaytraceUpdateLimit("Audio.MessageHandler.Vehicles.RaytraceUpdateLimit", 30);
#endif

Debug::Int g_DistanceScalingSuspendDelay("Audio.MessageHandler.Vehicles.DistanceScalingSuspendDelay", 1,Debug::Range(1, 30));

extern Debug::Bool g_bAudioSuspendEntities;






//Debug::Float gfCarCollisionImpulseThresholdHigh("Audio.Vehicles.CarCollisionImpulseThresholdHigh", 50.f);
//Debug::Float gfCarCollisionImpulseThresholdLow("Audio.Vehicles.CarCollisionImpulseThresholdLow", 0.1f);

//Debug::Float gfCarCollisionPeakDecayRatio("Audio.Vehicles.CarCollisionPeakDecayRatio", 0.9f);



//#define GENERATE_ENUM_STRINGS
//#include "BaseMessages.h"
//#undef GENERATE_ENUM_STRINGS



//	WorldObjectMessageHandler
Components::WorldObjectMessageHandler::WorldObjectMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom  ) : BaseMessageHandler( hEntityDefinition, pCom  )
{
	m_pCom = pCom;
	for (U32 uId = 0; uId < NumMessageIds; uId++)
	{
		Audio::EntityDataMemberHandle memberHandle = -1;
		if(uId != FINAL_REQUISITE_EXTERNAL)
		{
			memberHandle = Audio::InterfaceController::Get().LookupInstanceComponent( hEntityDefinition, WorldObjectMessageHandler::GetString( uId ) );
		}
		m_aHandle.PushBack( memberHandle );
			
		#ifndef CFG_GOLD
		//check for required params
		if(memberHandle == -1 && uId < FINAL_REQUISITE_EXTERNAL)
		{
			//Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hEntityDefinition)->Name()
			char name[512]; // [LloydW] This was running over the end of the array :-(
			strcpy_s(name,sizeof(name),WorldObjectMessageHandler::GetString( uId ));
			strcat_s(name,sizeof(name),": Required Audio External is Missing. Check audio data or let audio coder know this is optional");
			Audio::InterfaceController::Get().Error(name);
			DebugBreak();
		}
		#endif
	}

	m_distanceThreshold = 0;
	const AudioEntityHandle hCurrentEntity = m_pCom->GetEntityHnd();

	if(hCurrentEntity.IsValid())
	{

		g_hMaxDistanceHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_pCom->GetEntityHnd(), "MaxDistanceOutput");
		g_hMaxDistancePODataHandle = Audio::InterfaceController::Get().TrackValue(m_pCom->GetEntityHnd(), g_hMaxDistanceHandle);

		//send a pos far away before first update to prevent loud noise
		Maths::HVec3 farAway(1000,1000,1000);
		Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ObjectPosition], (F32*)&farAway);

	}

	//AddSubscriber( ActorMsg::E_MovementMsg_Spline);
	
	m_PrevObjPos = Maths::HVec3(Maths::UNDEFINED);
	m_PrevObjDir = Maths::HVec3(Maths::UNDEFINED);
	m_PrevObjUp = Maths::HVec3(Maths::UNDEFINED);
}

bool Components::WorldObjectMessageHandler::ProcessMessage(BaseMessageHandlerInfo& info)
{
	PERF_EVENT("WorldObjectMessageHandler::ProcessMessage");
	if(!m_pCom->GetEntityHnd().IsValid())
		return true;

	const AudioEntityHandle& hCurrentEntity = m_pCom->GetEntityHnd();
	
	switch(info.pMessage->Type())
	{
		case GameMessaging::E_GameMsg_UpdateEnable:
			info.pHandler->AddSubscriber(GameMessaging::E_GameMsg_Update);
			break;

		case GameMessaging::E_GameMsg_UpdateDisable:
			info.pHandler->RemoveSubscriber(GameMessaging::E_GameMsg_Update);
			break; 

		case  Actor::E_ActorMsg_ComponentCreated:
		{
			m_isSplitScreen = false;

			if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 1)
			{
				m_isSplitScreen = true;

			}
		}
		break;


		// component will need updating while moving
		case GameMessaging::E_GameMsg_Update:
		{
			PERF_EVENT("Audio.WorldObjectUpdate");
	
			Maths::HMat43 camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
			Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();
	
			//sort out viewports, get correct listener for view. for splitScreen, get clostest object to the listener
			Perf::BeginNamedEvent("Audio.SplitScreen.MultiListeners");

			if(g_usingMultipleListeners)
			{

				m_isSplitScreen = false;

				if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 1)
				{
					m_isSplitScreen = true;

				}

				if(m_isSplitScreen)//max 4 views
				{
					//int closestView = 0;
					int currentView = 0;
					//set closest view
					
					Maths::HF32 nearestDistance(Maths::Constants<Maths::HF32>::Max()); //big value
					
					for(int i = 0 ; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); i++)
					{

						Maths::HVec3 entityDistance = ((Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43().AW())  - (info.pActor->GetMatrix().AW()));
						const Maths::HF32 length = Maths::Length2(entityDistance);
						
						if(length < nearestDistance)
						{
							nearestDistance = length;
							currentView = i;
						}

					}

					camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Mat43();
					velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Live().Velocity();

				}
			}

			Perf::EndNamedEvent();


		//	Maths::HMat43 camMatrix = camMatrix.Mat43();
			//Maths::HVec3 velocity = 

			//suspend entities based on distance value			
			//static F32 distanceThreshold = 00.0;  //char

			//get the suspend distance distance// TODO, put back in after 1st playable
		//	distanceThreshold = Audio::InterfaceController::Get().GetEntityValue(hCurrentEntity, m_aHandle[MaxDistanceOutput]);

			//if it exists and is set

			
			if(m_distanceThreshold)
			{
				const Maths::HF32 distanceThresholdSq(m_distanceThreshold * m_distanceThreshold);
				
				//get object distance to listener
				Maths::HVec3 entityDistance = ((camMatrix.AW())  - (info.pActor->GetMatrix().AW()));

				Maths::HF32 length2 = Maths::Length2(entityDistance);

				// check if entity is within distance
				if(length2 > distanceThresholdSq)
				{
					//Audio::InterfaceController::Get().MuteEntity(hCurrentEntity);
					Audio::InterfaceController::Get().SuspendEntity(hCurrentEntity);
					break;

				}
				else
				{
					Audio::InterfaceController::Get().ResumeEntity(hCurrentEntity);
					//Audio::InterfaceController::Get().UnmuteEntity(hCurrentEntity);
				}
			}
			else
			{
				if(g_hMaxDistanceHandle && g_hMaxDistancePODataHandle.IsValid()) 
				{
					Audio::InterfaceController::Get().GetValue(m_distanceThreshold, g_hMaxDistancePODataHandle);

					if(m_distanceThreshold)
						Audio::InterfaceController::Get().UntrackValue(g_hMaxDistancePODataHandle);
				}

			}
				

			Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ListenerPosition], (F32*)(&camMatrix.AW()));
			Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ListenerDirection], (F32*)(&camMatrix.AZ()));					
			Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ListenerUp], (F32*)(&camMatrix.AY()));
			Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ListenerVelocity], (F32*)(&velocity));

			{
				Maths::HVec3 vec = info.pActor->GetMatrix().AW();
				if (!m_PrevObjPos.All_EQ(vec))
				{
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ObjectPosition], (F32*)(&vec));
					m_PrevObjPos = vec;
				}
			
				vec = info.pActor->GetMatrix().AZ();
				if (!m_PrevObjDir.All_EQ(vec))
				{
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ObjectDirection], (F32*)(&vec));
					m_PrevObjDir = vec;
				}
			
				vec = info.pActor->GetMatrix().AY();
				if (!m_PrevObjUp.All_EQ(vec))
				{
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ObjectUp], (F32*)(&vec));
					m_PrevObjUp = vec;
				}
			}

			//get at the movement component if this is a moving audio object
			const Components::C_MovementAttacher* mover = (const Components::C_MovementAttacher*)info.pActor->QueryInterface(E_IF_Movement);

			if(mover)
			{
			
				F32 doppler = mover->m_DopplerSpeed;

				if(doppler)
				{
					Maths::HVec3 velocity = info.pActor->GetMatrix().AZ() * doppler;
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ObjectVelocity], (F32*)(&velocity));

					if(m_aHandle[ObjectSpeed])
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[ObjectSpeed], (F32)doppler);
				}
				

			}

						#ifndef CFG_GOLD
						//DEBUG DEBUG DEBUG
						if(g_RenderAudioPositions)
						{
							S32 red = 0xff;
							S32 green = 0xff;
							S32 blue = 0xff;
							F32 radius = 1;

							Maths::RGBA colourYellow = Maths::RGBA(red, green, 0x00, 0xff);
							Maths::RGBA colourRed = Maths::RGBA(red, 0x00, 0x00, 0x00);
							Maths::RGBA colourBlue = Maths::RGBA(0x00, 0x00, blue, 0xff);
							Maths::RGBA colourGreen = Maths::RGBA(0x00, green, 0x00, 0xff);

							//image source green
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(info.pActor->GetMatrix().AW(),radius,colourGreen);
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(info.pActor->GetMatrix().AW(),Util::C_String().Printf("%s", info.pActor->GetActorName()));

						}
						#endif
			
			break;
		}

		case ActorMsg::E_AudioMsg_TriggerActionFromStrHash:
		{
			const ActorMsg::C_AudioTriggerActionFromStrHashMsg *msg = Messaging::MessageCast<ActorMsg::C_AudioTriggerActionFromStrHashMsg>(info.pMessage);

			Audio::EntityDataMemberHandle hnd =	Audio::InterfaceController::Get().LookupInstanceComponent(info.hEntityDef, msg->m_uStringHash);
			Audio::InterfaceController::Get().TriggerEntityActionIn(m_pCom->GetEntityHnd(), hnd);
			break;
		}

		case ActorMsg::E_AudioMsg_SetParamFromStrHash:
		{
			const ActorMsg::C_AudioSetParamFromStrHashMsg *msg = Messaging::MessageCast<ActorMsg::C_AudioSetParamFromStrHashMsg>(info.pMessage);

			Audio::EntityDataMemberHandle hnd =	Audio::InterfaceController::Get().LookupInstanceComponent(info.hEntityDef, msg->m_uStringHash);
			Audio::InterfaceController::Get().SetEntityValue(m_pCom->GetEntityHnd(), hnd, msg->m_fParam);
			break;
		}

		case ActorMsg::E_AudioMsg_Event:
		{
			const ActorMsg::C_AudioEventMsg* pEvent = Messaging::MessageCast<ActorMsg::C_AudioEventMsg>(info.pMessage);
			Audio::InterfaceController::Get().TriggerEntityActionIn( m_pCom->GetEntityHnd(), GetHandle( pEvent->GetValue() ) );
		}
		break;

		case ActorMsg::E_AudioMsg_SetParameter:
		{
			const ActorMsg::C_AudioSetParameterMsg* pEvent = Messaging::MessageCast<ActorMsg::C_AudioSetParameterMsg>(info.pMessage);
			Audio::InterfaceController::Get().SetEntityValue( m_pCom->GetEntityHnd(), GetHandle( pEvent->MsgID ), pEvent->fValue );
		}
		break;

		case ActorMsg::E_AudioMsg_SetVector:
		{
			const ActorMsg::C_AudioSetVectorMsg* pEvent = Messaging::MessageCast<ActorMsg::C_AudioSetVectorMsg>(info.pMessage);
			Audio::InterfaceController::Get().SetEntityVectorIn( m_pCom->GetEntityHnd(), GetHandle( pEvent->MsgID ), (F32*)(&pEvent->vValue));
		}
		break;


	case ActorMsg::E_AudioMsg_WaitForEvent:
		{
			// TODO: make this work!

			// Get hash value of event name
			U32	eventHash = Messaging::MessageCast<ActorMsg::C_AudioWaitForEventMsg>(info.pMessage)->GetValue();

			// Get actor handle to notify on event - send a ActorMsg::E_GameActorMsg_ActionCompleted to this
			// actor when the event has occurred
			m_EventNotifyHandle = Messaging::MessageCast<ActorMsg::C_AudioWaitForEventMsg>(info.pMessage)->GetActorHandle();

			// TODO: Move this to the part of the code that detects the event has been triggered
			info.pActor->SendMessageToActor(m_EventNotifyHandle, ActorMsg::E_GameActorMsg_ActionCompleted);
		}
		return true;

	//case ActorMsg::E_AudioMsg_HotSwitchEntity:
	//{
	//	const ActorMsg::C_HotSwitchEntityMsg *pMsg = Messaging::MessageCast<ActorMsg::C_HotSwitchEntityMsg>(info.pMessage);
	//	return SwitchEntity(pMsg->m_hXtEntity);
	//}

		//case ActorMsg::E_AudioMsg_EntityAudioStopped:
		//	//if (m_pManager != NULL && m_pManager->Type() == E_EntitySuspenderResumer)
		//	//{
		//	//	m_pManager->Suspend();
		//	//}
		//	return true;

		//case ActorMsg::E_AudioMsg_InstanceAudioStopped:
		//	//if (m_pManager != NULL)
		//	//{
		//	//	const ActorMsg::C_InstanceAudioStoppedMsg* pMsg = Messaging::MessageCast<ActorMsg::C_InstanceAudioStoppedMsg>(info.pMessage);
		//	//	BIZ_ASSERT(m_pManager->Type() == E_InstanceSuspenderResumer);
		//	//	if (m_pManager->Type() == E_InstanceSuspenderResumer)
		//	//		((C_InstanceSuspenderResumer*)(info.pMessage))->SuspendInstance(pMsg->m_sInstanceId);
		//	//}
		//	return true;

		default:
			return true;
	}

	return true;
}

//	statics
bool Components::VehicleMessageHandler::m_isSlowMo = false;

//	VehicleMessageHandler
Components::VehicleMessageHandler::VehicleMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, C_AudioEntity_AdditionalSpawned* pCom  ) : WorldObjectMessageHandler( hEntityDefinition, pCom ),
	m_bHorn(false),
	m_bEngineEnabled(true),
	m_bCollisionsEnabled(true)
{
	m_pCom = pCom;
	m_rightRayDistance = 100.0;
	m_leftRayDistance = 100.0;
	m_PrevGear = -2;  //-2, since it could be -1 for reverse?
	m_PrevClutchValue = -1;
	m_prevCameraPreset = -1;
	m_PrevHealthLevel = -1;
	m_PrevHealth = -1;
	m_reverbCounter = 0;
	m_prevFocusCar = -1;
	m_prevThrottle = FLT_MAX;
	m_prevPadThrottle = FLT_MAX;
	m_prevLeftReflectionVal = -1;
	m_prevRightReflectionVal = -1;
	m_prevOcclusionValue = -1;

	Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(pCom->GetActor());

	m_prevPriorityVols[pActor->GetVehicleIndex()] = FLT_MAX;
	m_prevSharedPriorityVols[pActor->GetVehicleIndex()] = FLT_MAX;

	for (U32 uId = WorldObjectMessageHandler::NumMessageIds; uId < NumSharedEntityMessageIds; uId++)
	{
		Audio::EntityDataMemberHandle memberHandle = -1;
		if(uId != FINAL_REQUISITE_EXTERNAL && uId < NumMessageIds)
		{
			memberHandle = Audio::InterfaceController::Get().LookupInstanceComponent( hEntityDefinition, VehicleMessageHandler::GetString( uId ) );
		}
		else
		//get the shared member handle
		if(uId != FINAL_REQUISITE_EXTERNAL)
		{
			memberHandle = Audio::InterfaceController::Get().LookupInstanceComponent( pCom->GetSharedEntityDefHnd(), VehicleMessageHandler::GetSharedEntityString( uId ) );
		}

		m_aHandle.PushBack( memberHandle );

		#ifndef CFG_GOLD
			//check for required params
		if(memberHandle == -1 && uId < FINAL_REQUISITE_EXTERNAL)
		{
			char name[512]; // [LloydW] This was running over the end of the array :-(
			strcpy_s(name,sizeof(name),VehicleMessageHandler::GetString( uId ));
			strcat_s(name,sizeof(name),": Required Audio External is Missing. Check audio data or let audio coder know this is optional or see sound designer");
			Audio::InterfaceController::Get().Error(name);
		//	BIZ_ASSERT(memberHandle != -1 && name);
			
		}
		#endif
	}

	m_Policy = Game::C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy();
	m_Group = Scheduler::CreateGroup(false);
	m_pLeftRay = new Collision::C_Ray;
	m_pRightRay = new Collision::C_Ray;
	m_pOcclusionRay = new Collision::C_Ray; 

	m_rpm = 0.f;

	//m_pDynamicObjectTriggerActor = NULL;
	//m_pGameWorld = NULL;
	//m_actorHandle = NULL;

	// force slo-mo reset on any car created
	m_isSlowMo = false;
	m_isShielded = false;
	m_isSilentRunning = false;
	m_bayDisabled = false;
	m_distanceToOcclusionDestination = 100000;

	m_prevRightReflectionSurface = -1;
	m_prevLeftReflectionSurface = -1;
}

Components::VehicleMessageHandler::~VehicleMessageHandler()
{
	Scheduler::Flush(m_Group);

	delete m_pLeftRay;
	delete m_pRightRay;
	delete m_pOcclusionRay;

	//Gravity::C_TriggerLayer* pTriggerLayer = m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects);
	//if (pTriggerLayer /*&& m_pDynamicObjectTriggerActor*/)
	//{
	//	//Actor::C_ActorHandle defaultHandle;
	//	m_pDynamicObjectTriggerActor->SetUserData( SharedActorUtils::HandleToUserData( m_actorHandle ) );
	//	
	//	pTriggerLayer->RemoveTriggerActor(m_pDynamicObjectTriggerActor);

	//	SAFE_DELETE_PTR(m_pDynamicObjectTriggerActor);
	//}
	Scheduler::DestroyGroup(m_Group);
}

void Components::VehicleMessageHandler::VehicleCollision(int mVehicleIndex, const Game::C_ImpactMsg* p_pMsg, const Maths::HMat43 matrix, AUDIO_COLLISION_COLLIDES_WITH collisionOther)
{
	F32 fImpulse = p_pMsg->mCollisionInfo.m_AudioImpulse;
	F32 fFriction = p_pMsg->mCollisionInfo.m_GameplayFriction;
	fFriction = p_pMsg->mCollisionInfo.m_AudioFriction;
	//F32 fImpulse = msg->m_ImpulseMagnitude / pDamageObject->Mass();
	//const C_PhysicsBodyEventMsg* msg = Messaging::MessageCast<C_PhysicsBodyEventMsg>(info.pMessage);

	//printf("GameplayImpact: %f\n",p_pMsg->m_CollisionInfo.m_GameplayImpulse);
	//printf("PhysicsImpact: %f\n",p_pMsg->m_CollisionInfo.m_PhysicsImpulse);

	if (fImpulse > gfCarCollisionImpulseThresholdLowNew)
	{

		//get only collect if impulse is larger than previous max
		if(fImpulse > m_collideParams[mVehicleIndex].m_fMaxImpulseStrength)
		{
			m_collideParams[mVehicleIndex].m_bCollisionThisFrame = true;
			m_collideParams[mVehicleIndex].m_fMaxImpulseStrength = fImpulse;	
			m_collideParams[mVehicleIndex].m_fMaxFriction = fFriction;


			Maths::HVec3 carSpacePos = p_pMsg->mCollisionInfo.m_ContactPoints[0].m_ContactPoint * matrix.InverseRotTrans();

			m_collideParams[mVehicleIndex].m_Position = carSpacePos;
			m_collideParams[mVehicleIndex].m_vehicleIndex = mVehicleIndex;
			m_collideParams[mVehicleIndex].m_Shielded = false;
			m_collideParams[mVehicleIndex].m_MaterialType =  p_pMsg->mCollisionInfo.m_EffectMaterial;

			//if(p_pMsg->
 			m_collideParams[mVehicleIndex].m_CollidesWith = collisionOther;

			m_collideParams[mVehicleIndex].m_CollisionHash = p_pMsg->mHashValue;

			m_collideParams[mVehicleIndex].m_fPreviousMaxImpulseStrength = m_collideParams[mVehicleIndex].m_fMaxImpulseStrength;

		}				


	}
	//else //treat it as a scrape
	//if(fImpulse > gfCarScrapeFrictionThresholdLow)
	//{

	//	//get only collect if impulse is larger than previous max
	//	if(fImpulse > m_collideParams[mVehicleIndex].m_fMaxImpulseStrength)
	//	{
	//		m_scrapeParams[mVehicleIndex].m_bScrapeThisFrame = true;
	//		m_scrapeParams[mVehicleIndex].m_fMaxFriction = fImpulse;	
	//		m_scrapeParams[mVehicleIndex].m_fMaxFrictionStrength = fImpulse;	

	//		Maths::HVec3 carSpacePos = p_pMsg->mCollisionInfo.m_ContactPoints[0].m_ContactPoint * matrix.InverseRotTrans();

	//		m_scrapeParams[mVehicleIndex].m_Position = carSpacePos;
	//		m_scrapeParams[mVehicleIndex].m_vehicleIndex = 0;
	//		m_scrapeParams[mVehicleIndex].m_Shielded = false;
	//		m_scrapeParams[mVehicleIndex].m_MaterialType =  p_pMsg->mCollisionInfo.m_EffectMaterial;
	//		m_scrapeParams[mVehicleIndex].m_CollidesWith = collisionOther;
	//	}

	//}



	//static float gfCarFriction = 2000;

	//if (p_pMsg->m_CollisionInfo.m_GameplayFriction > gfCarFriction)
	//{
	//	printf("\nimpact: %f",p_pMsg->m_CollisionInfo.m_GameplayImpulse);
	//	printf("\nscrape: %f",p_pMsg->m_CollisionInfo.m_GameplayFriction);

	//}
}

U32 Components::VehicleMessageHandler::GetOcclusionValue(Maths::HMat43 raySourceMatrix, Maths::HMat43 rayDestMatrix,F32 distanceToDest, Gravity::C_Scene* scene)
{
	PERF_EVENT("Audio.VMHOcclus");
	
		Collision::C_CollisionFilter rayOcclusionCollisionFilter( 0xffffffff, 
			Game::PolyFlags::kDynamicObjectSolid, 0, 0, 0, 0xffffffff, Game::PolyFlags::kbWorld);	

		//Scheduler::Flush(m_Group);

		Collision::C_RayTracingResult& 	result = m_pOcclusionRay->m_result;

		Maths::HVec3 slightRamp(0,(float)g_OcclusionRayHeight,0);//do ray from a metre up
		Maths::HVec3 raySourcePosition = Maths::HVec3_Add(raySourceMatrix.AW(),slightRamp);		
		Maths::HVec3 rayDestPosition = rayDestMatrix.AW();

		U32 occlusionResult = 0;
		F32 rayDistance = distanceToDest;
		
		//if occluded by mesh
		if (result.Hit())
		{
			rayDistance = (float)result.Distance();
			occlusionResult = 1;
		}

		m_pOcclusionRay->InitialiseRay(raySourcePosition,rayDestPosition,&rayOcclusionCollisionFilter);
		
		//m_pOcclusionRay->InitialiseRay(raySourcePosition, (Maths::C_Normal)rayDirection, (float)g_OcclusionRayDistanceMax, &rayOcclusionCollisionFilter, 0.1f);
		
		scene->DoDeferredRayTrace(m_pOcclusionRay,NULL,m_Group,m_Policy);

		#ifndef CFG_GOLD
		//DEBUG DEBUG DEBUG
		if(g_RenderOcclusionRays)
		{

			Maths::HVec3 rayDirectionTemp = (rayDestMatrix.AW() - raySourcePosition);
			Maths::HVec3 rayDirection = Normalised(rayDirectionTemp);

			S32 red = 0xff;
			S32 green = 0xff;
			S32 blue = 0xff;
			F32 radius = 1;

			Maths::RGBA colourYellow = Maths::RGBA(red, green, 0x00, 0xff);
			Maths::RGBA colourRed = Maths::RGBA(red, 0x00, 0x00, 0x00);
			Maths::RGBA colourBlue = Maths::RGBA(0x00, 0x00, blue, 0xff);
			Maths::RGBA colourGreen = Maths::RGBA(0x00, green, 0x00, 0xff);

			//image source green
			Maths::RGBA occColour = colourGreen;

			if(occlusionResult)
				occColour = colourRed;

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(raySourcePosition,radius,occColour);
		
			//image direction
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(raySourcePosition, rayDirection, rayDistance, 1, occColour);
				
		}
		#endif


	return occlusionResult;
}

bool Components::VehicleMessageHandler::ProcessMessage(BaseMessageHandlerInfo& info)
{
		if(!m_pCom->GetEntityHnd().IsValid())
			return true;

		const AudioEntityHandle& hCurrentEntity = m_pCom->GetEntityHnd();

		const AudioEntityHandle& hCurrentSharedEntity = m_pCom->GetSharedEntityHnd();

		Game::Actors::C_VehicleActor* pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor >( info.pActor );

		switch(info.pMessage->Type())
		{

			case ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoEntry:
			{
				if ( pActor->IsFocusVehicle() )
				{
					m_isSlowMo = true;

					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eEnterSloMo,0);
				}
			}
			break;

			case ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoExit:
			{
				if ( pActor->IsFocusVehicle() )
				{
					m_isSlowMo = false;

					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eExitSloMo,0);
				}
			}
			break;

			case ActorMsg::E_AudioMsg_ConnectEntity:
			{

				//info.msg.entity
				const ActorMsg::C_AudioConnectEntityMsg* pMsg = Messaging::MessageCast<ActorMsg::C_AudioConnectEntityMsg>(info.pMessage);

				Audio::EntityDefinitionHandle hDef = Audio::InterfaceController::Get().GetDefinitionHandle(pMsg->m_EntityInstanceHandle);

				Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(m_pCom->GetSharedEntityDefHnd());
			
				Audio::LocalFunctionHandle hFunc = Audio::InterfaceController::Get().LookupParameterisedFunctionById(hDef, "DoConnections", pDef->GetId());
				// only try to connect if "DoConnections" func exists
				if (hFunc != 0xffffffff)
				{
					Audio::InterfaceController::Get().RunFunctionInParameterisedEntityInstance(pMsg->m_EntityInstanceHandle, hCurrentSharedEntity  , hFunc);
				} 
			}
			break;

			case Actor::E_ActorMsg_ComponentCreated:
			{

				//set messageSubscribers
				
				//info.pHandler->AddSubscriber( Game::E_VehicleMessage_ShieldedImpactEvent);
				
				
			//	info.pHandler->AddSubscriber( Game::E_PickupMessage_PickupPickedUp );

				//info.pHandler->AddSubscriber(Game::E_PickupMessage_MissileExploded);
				//info.pHandler->AddSubscriber(Game::E_PickupMessage_MineExploded);
				//info.pHandler->AddSubscriber(Game::E_PickupMessage_PickupInitiated);
				info.pHandler->AddSubscriber(Game::E_PickupMessage_PickupFinished);
				info.pHandler->AddSubscriber(Game::E_PickupMessage_HitByPickup);

				//info.pHandler->AddSubscriber(Game::E_PickupMessage_ChargePerk);
				info.pHandler->AddSubscriber(E_VehicleLogicMsg_StartDelayToPerk);
				info.pHandler->AddSubscriber(E_VehicleLogicMsg_FiredPerk);

				info.pHandler->AddSubscriber(Game::E_VehicleMessage_LeftRace);

				info.pHandler->AddSubscriber( E_VehicleLogicMsg_FiredPerk );
				info.pHandler->AddSubscriber( E_VehicleLogicMsg_PerkAudioTrigger );
				//info.pHandler->AddSubscriber( E_VehicleLogicMsg_PerkSFXOveride );

				//info.pHandler->AddSubscriber( E_VehicleLogicMsg_HitByPerk );
				info.pHandler->AddSubscriber( E_VehicleLogicMsg_ShieldedPerk );
				
				//info.pHandler->AddSubscriber( ::Actors::E_VisibilityMsg_OnVisible );
			//	info.pHandler->AddSubscriber( ::Actors::E_VisibilityMsg_OnHidden );

				info.pHandler->AddSubscriber( ActorMsg::E_DynamicObjMsg_VehicleEnteredTrigger );


			//	info.pHandler->AddSubscriber( Game::E_PickupMessage_GrantPickup );
				
				info.pHandler->AddSubscriber( E_VehicleLogicMsg_PerkEnded );

				info.pHandler->AddSubscriber( Game::E_GameflowMessage_RaceStarted );

				info.pHandler->AddSubscriber( ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoEntry );
				info.pHandler->AddSubscriber( ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoExit );

				if(m_aHandle[PopTrigger] != -1)
					m_hQueryPopTrigger = Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hCurrentEntity), m_aHandle[PopTrigger]);

				if(m_aHandle[BangTrigger] != -1)
					m_hQueryBangTrigger = Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hCurrentEntity), m_aHandle[BangTrigger]);
				
				if(m_aHandle[LeftPopTrigger] != -1)
					m_hQueryLeftPopTrigger = Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hCurrentEntity), m_aHandle[LeftPopTrigger]);
				
				if(m_aHandle[RightPopTrigger] != -1)
					m_hQueryRightPopTrigger = Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hCurrentEntity), m_aHandle[RightPopTrigger]);
				
				if(m_aHandle[LeftBangTrigger] != -1)
					m_hQueryLeftBangTrigger = Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hCurrentEntity), m_aHandle[LeftBangTrigger]);
				
				if(m_aHandle[RightBangTrigger] != -1)
					m_hQueryRightBangTrigger = Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hCurrentEntity), m_aHandle[RightBangTrigger]);

				Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(info.pActor);
				
				m_AudioRPMLimit = pActor->GetAudioRPMLimit();
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[AudioRPMLimit], m_AudioRPMLimit );
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[AudioForcedInduction], (F32)pActor->GetAudioForcedInduction());
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[ExhaustType], (F32)pActor->GetAudioExhaustType());
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[EngineType], (F32)pActor->GetAudioEngineType());				

				//Audio::InterfaceController::Get().MuteEntity(hCurrentEntity);
				Audio::InterfaceController::Get().SuspendEntity(hCurrentEntity);

				m_isFlaggedForSuspend[pActor->GetVehicleIndex()].frameDelay = g_DistanceScalingSuspendDelay;
				m_isFlaggedForSuspend[pActor->GetVehicleIndex()].suspendState = E_SUSPEND_RESUME_STATE_UNKNOWN;

				m_isPendingSuspend = false;
		//		m_bEnableCollisions = false;
				m_isWrecked = false;

				m_isSplitScreen = false;

				if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 1)
				{
					m_isSplitScreen = true;


				}

				for(int i = 0;i<MAX_NUMBER_AUDIO_CARS;i++)
				{
					m_NewCarDistanceScale[i].distanceFromListener = 1000000;
					m_NewCarDistanceScale[i].CarID = -1;
					m_NewCarDistanceScale[i].volScale = 0;
					m_NewCarDistanceScale[i].shouldUpdate = true;
					m_NewCarDistanceScale[i].isFocusCar = false;
					m_NewCarDistanceScale[i].listenerViewPort = 0;
					m_NewCarDistanceScale[i].isMuted = false;

				}

			}
			break;

			//hack to mute collisions until race starts (glass smashes ass cars are dropped on the track)
			case Game::E_GameflowMessage_RaceStarted:
				{
					m_bEnableCollisions = true;

				}
				break;

			//case isAGhostCar.... can piggy on to this hopefully
			case Game::E_VehicleMessage_LeftRace: 
			{
				const Game::Actors::C_VehicleLeftRaceMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleLeftRaceMsg>(info.pMessage);
				
				Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(info.pActor);
				const int vehicleIndex = pActor->GetVehicleIndex();

				if(vehicleIndex == pMsg->m_VehicleIndex)
					m_CarDistanceScale[vehicleIndex].shouldUpdate = false;

				break;
			}

			//case E_VehicleLogicMsg_EnableDisablePickupSystem:
			//{

			//	const C_VehicleLogicMsg_EnableDisablePickupSystem* msg = Messaging::MessageCast<C_VehicleLogicMsg_EnableDisablePickupSystem>(info.pMessage);

			//	//send to shared enitity
			//	if(msg->mDisablePickupTime > 0 && !m_bayDisabled)
			//	{
			//		//SEND THE SHIELD INFO TO THE SHARED ENTITY
			//		if(hCurrentSharedEntity.IsValid())
			//			Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[ScramblerActivated],false);

			//		m_bayDisabled = true;

			//	}
			//	else
			//	if(msg->mDisablePickupTime == 0 && m_bayDisabled)
			//	{
			//		//SEND THE SHIELD INFO TO THE SHARED ENTITY
			//		if(hCurrentSharedEntity.IsValid())
			//			Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[ScramblerDeactivated],false);

			//		m_bayDisabled = false;
			//	}	
			//
			//}
			//break;

			case E_VehicleLogicMsg_PowerUpFXDurations:
			{
				C_VehicleLogicMsg_PowerUpFXDurations* msg =  (C_VehicleLogicMsg_PowerUpFXDurations*)info.pMessage;
				if(hCurrentSharedEntity.IsValid())	
				{
					Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity,m_aHandle[DurationIntro], (F32)msg->mDurationIntro);	
					Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity,m_aHandle[DurationLoop], (F32)msg->mDurationLoop);	
					Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity,m_aHandle[DurationOutro], (F32)msg->mDurationOutro);	
				}				
			}
			break;

		//	case GameMessaging::E_GameMsg_Update:
			case Game::E_GameflowMessage_UpdateDeferredAudio:
				{	
					PERF_EVENT("Audio.VehicleMessageHandler");
					
					if(!hCurrentEntity.IsValid())
						return true;

#ifdef VEHICLE_OCCLUSION_LODDING
					//counter that increments over 30 frames and then resets
				//	m_currentFrame = m_currentFrame++ % g_RaytraceUpdateLimit;
#endif

					if (info.pActor->GetActorTypeId() != Game::E_ActorTypeId_Vehicle)
						return true;

					Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(info.pActor);

					class RacingPhysics::Car *pCar = pActor->GetVehiclePhysics();

					Gravity::C_Scene* scene = pActor->GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene();
					
					if(!pCar)
						return false;

					RacingPhysics::Vector3R pvelocity = pCar->GetVelocity();

					const int vehicleIndex = pActor->GetVehicleIndex();
					m_vehicleIndex = vehicleIndex;


					//[TODO CIAN, remove and place in worldupdate
					//update 3d information
					////UpdateOrientation(pVehicle->GetMatrix(),pvelocity,hCurrentEntity);
					
						//////update object orientation 
					Maths::HMat43 objectMatrix = pActor->GetMatrix();

					//Audio::EntityDataMemberHandle hVectorIn;

					//AudioEntityHandle handle = info.hEntity;
					//Audio::EntityDataMemberHandle hVectorIn;

					//AudioEntityHandle handle = info.hEntity;
					AudioEntityHandle handle = hCurrentEntity;

					RacingPhysics::Vector3R  objectVelocity = pvelocity;

					int audibleCars = AUDIBLE_CARS;

					Maths::HMat43 camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
					Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();

					F32 nearestDistance = 1000000; //big value
					
					//sort out viewports, get correct listener for view
					if(m_isSplitScreen )//max 4 views
					{
						PERF_EVENT("Audio.VMH_IsSplitscreen");
						audibleCars = AUDIBLE_CARS_SPLITSCREEN;

						if((vehicleIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))
						{
														camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Mat43();
							velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Live().Velocity();

						}
						else
						{
							//ai cars need to be assigned to a listener, for this frame, give this vehicle to closest listener
							//get closest listener
							int currentView = 0;
							//set closest view
							
							
							
							for(int i = 0 ; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() ; i++)
							{

								Maths::HVec3 entityDistance = ((Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43().AW())  - (info.pActor->GetMatrix().AW()));
								F32 length = Maths::Length(entityDistance);
								
								if(length < nearestDistance)
								{
									nearestDistance = length;
									currentView = i;
								}

							}

							camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Mat43();
							velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Live().Velocity();

						}
						
					}

					U32 numVehicles = 0;
					U32 numViewports = 1;
					U32 removedVehicles = 0;
					bool finalVehicleHasFocus = false;
					bool bSuspended = false;

					static bool scaleActive = true;

					//check if focusVehicle
					bool isFocusVehicle = false;
					Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
					if(pRLC)
					{
						numVehicles = pRLC->GetNumVehicles();
						numViewports = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
						
						//check all viewports to see if this car has focus
						for(int i = 0; i < (int)numViewports; i++)
						{
							if(vehicleIndex == pRLC->GetViewportFocusVehicleIndex(i))
							{
								isFocusVehicle = true;

							 
								//for distance algorithim, need to know if final vehicle has focus
								if(vehicleIndex == numVehicles-1)
									finalVehicleHasFocus = true;
								
							}

						}

					}
					
					Maths::HVec3 camPos;
					Maths::HVec3 carPos = objectMatrix.AW();

					Maths::HMat43 listenerMatrix = camMatrix;

					S32 cameraPreset  = AmaxCam::CameraSystem().VehicleFeed()->GetAudioMapping();
					
					if(cameraPreset != m_prevCameraPreset)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[CameraPreset], (F32)cameraPreset);
						m_prevCameraPreset = (F32)cameraPreset;

					}

					//if in bonnet/bumper cam, set listener to centre of car to prevents small 'wobbles' in doppler
					//cam,era mapping change...fixging this up to reflect that
					if(isFocusVehicle && cameraPreset == 4)
					{
						//because of a bug where the rpm wobbles, set the listner back about 10cm from the centre of the car						
						Maths::HVec3 offsetLength = -camMatrix.AZ() * g_RenderBumperCamPosOffset;

						camPos = carPos + offsetLength;

						if(g_RenderBumperCamPos)
						{							
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(camPos,3.0f,Maths::RGBA(0x00, 0xff, 0x00, 0xff));
						}
					
						velocity = objectVelocity;
					}
					else
						camPos = camMatrix.AW();
						

					//do distance scaling algorithm

					F32 distanceFromListener = m_distanceToOcclusionDestination = camPos.Distance(carPos);
					#ifndef CFG_GOLD				
					if(g_RenderDistanceScale)
					{
						static F32 val = 15.f;
						F32 scaleRender = val * m_prevPriorityVols[vehicleIndex];
						Maths::HVec3 PosUp(carPos.X(),carPos.Y() + scaleRender,carPos.Z());
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine( carPos, PosUp, Maths::RGBA( 255, 255, 255, 255 ) );
					}
					#endif

					//if there are enough vehicles to warrent distance scaling
					if((int)numVehicles > audibleCars && bDebugUsingDistanceScaling && !isFocusVehicle)
					{
						PERF_EVENT("Audio.VMHDistanceScaling");
								
							//check if entities have been marked for resume/suspend
							if(m_isFlaggedForSuspend[vehicleIndex].suspendState == E_SUSPEND_RESUME_STATE_FLAGGED_FOR_SUSPEND)
							{
								if(m_isFlaggedForSuspend[vehicleIndex].frameDelay == 0)
								{
									Audio::InterfaceController::Get().SuspendEntity(hCurrentEntity);
									if(hCurrentSharedEntity.IsValid())
										Audio::InterfaceController::Get().SuspendEntity(hCurrentSharedEntity);
									m_isFlaggedForSuspend[vehicleIndex].suspendState = E_SUSPEND_RESUME_STATE_SUSPENDED;
									m_isFlaggedForSuspend[vehicleIndex].frameDelay = g_DistanceScalingSuspendDelay;
								}
								else
									m_isFlaggedForSuspend[vehicleIndex].frameDelay--;
							}
							else
							if(m_isFlaggedForSuspend[vehicleIndex].suspendState == E_SUSPEND_RESUME_STATE_FLAGGED_FOR_RESUME)
							{
								if(m_isFlaggedForSuspend[vehicleIndex].frameDelay == 0)
								{
									Audio::InterfaceController::Get().ResumeEntity(hCurrentEntity);
									if(hCurrentSharedEntity.IsValid())
										Audio::InterfaceController::Get().ResumeEntity(hCurrentSharedEntity);
									m_isFlaggedForSuspend[vehicleIndex].suspendState = E_SUSPEND_RESUME_STATE_RESUMED;
									m_isFlaggedForSuspend[vehicleIndex].frameDelay = g_DistanceScalingSuspendDelay;
								}
								else
									m_isFlaggedForSuspend[vehicleIndex].frameDelay--;
							}
							
							//set ghosted vehicles to far away
							if(pActor->IsEliminated())
								distanceFromListener = 10000;

							//set disabled vehicles to far away
							if(!pActor->IsEnabled())
								distanceFromListener = 10000;

							//do distance shaping for angle of car in relation to forward view
							//if car is directly behind listener, take is 2x distance away								
							
							//get direction to car from camera
							Maths::HVec3 shapingDirection = carPos - camPos;
														
							//normalise vector
							Maths::HVec3 normalisedDirection = Normalised(shapingDirection);


							//angle from straight ahead of camera to car
							F32 angleFromForward = (acosf ( camMatrix.AZ() | normalisedDirection)) / Maths::Pi;

							angleFromForward += 1; //add 1 to bring to 1->2 range. 1 is straight ahead, 2 directly behind

							//scale distance based on car angle from straight ahead
							distanceFromListener *= angleFromForward;


							U32 nonFocusAudibleCars = audibleCars - numViewports; 

							for(int i = nonFocusAudibleCars;i >= 0;i--)
							{
								if(distanceFromListener < m_NewCarDistanceScale[i].distanceFromListener)
								{
									F32 tempDistanceHold = m_NewCarDistanceScale[i].distanceFromListener;

									m_NewCarDistanceScale[i+1] = m_NewCarDistanceScale[i];
		
									m_NewCarDistanceScale[i].distanceFromListener = distanceFromListener;
									m_NewCarDistanceScale[i].CarID = vehicleIndex;
									m_NewCarDistanceScale[i].isFocusCar = isFocusVehicle;
									m_NewCarDistanceScale[i].hEntityHandle = hCurrentEntity;
									m_NewCarDistanceScale[i].hPriorityMemberHandle[0] = m_aHandle[PriorityVolume];
									m_NewCarDistanceScale[i].hPrioritySharedMemberHandle[0] = m_aHandle[PriorityVolumeShared];
									m_NewCarDistanceScale[i].hSharedEntityHandle = hCurrentSharedEntity;

									//suspend the car that has dropped off the end of audible index threshold, won't know the entity handle next time so have to suspend it now
									if(m_NewCarDistanceScale[nonFocusAudibleCars+1].hEntityHandle.IsValid())
									{
										int carId = m_NewCarDistanceScale[nonFocusAudibleCars+1].CarID;
										if (m_prevPriorityVols[carId] != 0.f)
										{
											Audio::InterfaceController::Get().SetEntityValue(m_NewCarDistanceScale[nonFocusAudibleCars+1].hEntityHandle, m_NewCarDistanceScale[nonFocusAudibleCars+1].hPriorityMemberHandle[0], 0.0f);
											m_prevPriorityVols[carId] = 0.f;											
										}

										if(m_NewCarDistanceScale[nonFocusAudibleCars+1].hSharedEntityHandle.IsValid())
										{
											if (m_prevSharedPriorityVols[carId] != 0.f)
											{
												Audio::InterfaceController::Get().SetEntityValue(m_NewCarDistanceScale[nonFocusAudibleCars+1].hSharedEntityHandle, m_NewCarDistanceScale[nonFocusAudibleCars+1].hPrioritySharedMemberHandle[0], 0.0f);
												m_prevSharedPriorityVols[carId] = 0.f;
											}
										}

										//mark for suspend next g_DistanceScalingSuspendDelay frames
										if(m_isFlaggedForSuspend[carId].suspendState != E_SUSPEND_RESUME_STATE_SUSPENDED && m_isFlaggedForSuspend[carId].suspendState != E_SUSPEND_RESUME_STATE_FLAGGED_FOR_SUSPEND)
										{
											m_isFlaggedForSuspend[carId].suspendState = E_SUSPEND_RESUME_STATE_FLAGGED_FOR_SUSPEND;
											m_isFlaggedForSuspend[carId].frameDelay = g_DistanceScalingSuspendDelay;
										}

										
									}


								}
								else
								if(distanceFromListener > m_NewCarDistanceScale[nonFocusAudibleCars].distanceFromListener)//if not audible
								{

										m_NewCarDistanceScale[i].volScale = 0.0f;
										
										if (m_prevPriorityVols[vehicleIndex] != 0)
										{
											Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[PriorityVolume], 0.0f);
											m_prevPriorityVols[vehicleIndex] = 0;																					
										}

										if(hCurrentSharedEntity.IsValid())
										{
											if (m_prevSharedPriorityVols[vehicleIndex] != 0.f)
											{
												Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity, m_aHandle[PriorityVolumeShared], 0.0f);
												m_prevSharedPriorityVols[vehicleIndex] = 0.f;												
											}
										}

										//mark for suspend next g_DistanceScalingSuspendDelay frames
										if(m_isFlaggedForSuspend[vehicleIndex].suspendState != E_SUSPEND_RESUME_STATE_SUSPENDED && m_isFlaggedForSuspend[vehicleIndex].suspendState != E_SUSPEND_RESUME_STATE_FLAGGED_FOR_SUSPEND)
										{
											m_isFlaggedForSuspend[vehicleIndex].suspendState = E_SUSPEND_RESUME_STATE_FLAGGED_FOR_SUSPEND;
											m_isFlaggedForSuspend[vehicleIndex].frameDelay = g_DistanceScalingSuspendDelay;
										}	

									break; // exit loop, car is further away than last car
								}
								else // if audible but further away than some cars
									break; // exit loop, car will get no closer
							}

							//all distances have been calculated, apply suspending, distance scaling, and reset the distance array
							U32 finalVehicle = numVehicles-1;

							if(finalVehicleHasFocus) //set 'final' vehicle to be the penultimate one
								finalVehicle = numVehicles-2;

							//all vehicle distances have been entered in the array, now we can post process them
							if(vehicleIndex == finalVehicle)
							{
								//get the volume scaling values
								F32 audibleDistance = m_NewCarDistanceScale[nonFocusAudibleCars].distanceFromListener;
								F32 closestCarDistance = m_NewCarDistanceScale[0].distanceFromListener;
								
								//take distance from camera
									closestCarDistance = 0;

								F32 recalculatedDistanceRange = audibleDistance - closestCarDistance;

								if(recalculatedDistanceRange<=0)
									recalculatedDistanceRange = .000001f;

								//the post process, apply the volume scale and suspends.
								for(int i = 0; i < (int)nonFocusAudibleCars+1;i++)
								{

									//get ratio of current distance to maxdistance
									F32 volPercent = Maths::Clamp((m_NewCarDistanceScale[i].distanceFromListener - closestCarDistance) / recalculatedDistanceRange,0.0f,1.0f);
									
									volPercent	*= volPercent * volPercent;//powf(volPercent,3.0); //weight the vol scale, cubic		

									F32 volScale = 1.0f - volPercent; //get vol scale			

									//end calculate priority vol
									//set the volume scale for debugging
									m_NewCarDistanceScale[i].volScale = volScale;

									//resume this entity since it should be audible									
									int carID = m_NewCarDistanceScale[i].CarID;															

									if (m_prevPriorityVols[carID] != volScale)
									{
										Audio::InterfaceController::Get().SetEntityValue(m_NewCarDistanceScale[i].hEntityHandle, m_NewCarDistanceScale[i].hPriorityMemberHandle[0], volScale);
										m_prevPriorityVols[carID] = volScale;
										
									}
									
									if(m_NewCarDistanceScale[i].hSharedEntityHandle.IsValid())
									{
										if (m_prevSharedPriorityVols[carID] != volScale)
										{
											Audio::InterfaceController::Get().SetEntityValue(m_NewCarDistanceScale[i].hSharedEntityHandle, m_NewCarDistanceScale[i].hPrioritySharedMemberHandle[0], volScale);
											m_prevSharedPriorityVols[carID] = volScale;
										}
									}
									
									// mark for resume, reset it's counter	
									if(volScale <= 0)//id the volume has been set to zero, suspend this object next turn
									{
										if(m_isFlaggedForSuspend[carID].suspendState != E_SUSPEND_RESUME_STATE_SUSPENDED && m_isFlaggedForSuspend[carID].suspendState != E_SUSPEND_RESUME_STATE_FLAGGED_FOR_SUSPEND)
										{
											m_isFlaggedForSuspend[carID].suspendState = E_SUSPEND_RESUME_STATE_FLAGGED_FOR_SUSPEND;
											m_isFlaggedForSuspend[carID].frameDelay = g_DistanceScalingSuspendDelay;
										}
									}
									else
									{
										if(m_isFlaggedForSuspend[carID].suspendState != E_SUSPEND_RESUME_STATE_RESUMED && m_isFlaggedForSuspend[carID].suspendState != E_SUSPEND_RESUME_STATE_FLAGGED_FOR_RESUME)
										{
											m_isFlaggedForSuspend[carID].suspendState = E_SUSPEND_RESUME_STATE_FLAGGED_FOR_RESUME;
											m_isFlaggedForSuspend[carID].frameDelay = g_DistanceScalingSuspendDelay;
										}
									}

									#ifndef CFG_GOLD
									//print distances scale
									if(g_RenderDistanceScale)
									{
										static F32 xPos = 100;
										F32 yPos = 150 +  (40 * (F32)i);

										DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(xPos,yPos),Util::C_String().Printf("Distance:%f", m_NewCarDistanceScale[i].distanceFromListener));
										DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(xPos+200,yPos),Util::C_String().Printf("CarID:%d", m_NewCarDistanceScale[i].CarID));
										DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(xPos+400,yPos),Util::C_String().Printf("VolScale:%f", m_NewCarDistanceScale[i].volScale));

									}
									#endif

									//reset the distance array
									m_NewCarDistanceScale[i].distanceFromListener = 100000;
									m_NewCarDistanceScale[i].hEntityHandle.Zero();

									//put the distances in a separate array so the wheels can query during the actor updates
									m_LockedCarDistanceScale[i] = m_NewCarDistanceScale[i];
								}


							}
		
					}
					else
					{

						Audio::InterfaceController::Get().ResumeEntity(hCurrentEntity);

						if(hCurrentSharedEntity.IsValid())
						{
							Audio::InterfaceController::Get().ResumeEntity(hCurrentSharedEntity);

							if (m_prevSharedPriorityVols[vehicleIndex] != 1.f)
							{
								Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity, m_aHandle[PriorityVolumeShared], 1.0f);
								m_prevSharedPriorityVols[vehicleIndex] = 1.f;
							}
						}

						if (m_prevPriorityVols[vehicleIndex] != 1.f)
						{
							Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[PriorityVolume], 1.0f);
							m_prevPriorityVols[vehicleIndex] = 1.f;
						}
						//resume this entity since it should be audible
						m_isFlaggedForSuspend[vehicleIndex].suspendState = E_SUSPEND_RESUME_STATE_RESUMED;
						m_isFlaggedForSuspend[vehicleIndex].frameDelay = g_DistanceScalingSuspendDelay;
						

					}
		
					//end distance scaling

					//do collision params

					bool bDoCollisions = m_collideParams[vehicleIndex].m_bCollisionThisFrame;
					bool bDoScrapes = m_scrapeParams[vehicleIndex].m_bScrapeThisFrame;

					//detect sudden impact
					F32 fGndSpeed = (float)pvelocity.Length();

					//get delta of speeds
					F32 fGndSpeedDelta = m_fPreviousGndSpeed - fGndSpeed;

					//store for next frame
					m_fPreviousGndSpeed = fGndSpeed;

					// send physics collision event
					if (bDoCollisions)
					{
						PERF_EVENT("Audio.VMHCollisions");
						if (m_collideParams[vehicleIndex].m_fMaxImpulseStrength > m_collideParams[vehicleIndex].m_fDecayingPeakImpulse)
						{
							//if sudden stop, ignore collidesWith and treat as a wall collision - hack to get around bug where we get DO/PO actor in car->world collision
							static F32 suddenImpactThreshold = 2.5;
							if(fGndSpeedDelta > suddenImpactThreshold && m_collideParams[vehicleIndex].m_CollidesWith == AUDIO_COLLISION_COLLIDES_WITH_OTHER)//was a collision with a DO/PO
							{
								m_collideParams[vehicleIndex].m_CollidesWith = AUDIO_COLLISION_COLLIDES_WITH_WORLD;
							}

							m_collideParams[vehicleIndex].m_fDecayingPeakImpulse = m_collideParams[vehicleIndex].m_fMaxImpulseStrength;

							//	sending current collision parameters to collision subsystem
							//	AUDIOTODO: we should try and move these to collision subsystem
							//only send if not shielded. 3rd param of custom message is vehicle index
							if(m_isShielded)
								m_collideParams[vehicleIndex].m_Shielded = true;
							else
								m_collideParams[vehicleIndex].m_Shielded = false;

							Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eCollisionSubsystem, CollisionSubsystem::E_CollisionSubsystem_Impact,vehicleIndex,&m_collideParams[vehicleIndex]);
						}

						m_collideParams[vehicleIndex].m_bCollisionThisFrame =  false;
						m_collideParams[vehicleIndex].m_fMaxImpulseStrength = 0.f;
					}
					
					//	AUDIOTODO: is this needed anymore?
					if(bDoScrapes)
					{
						PERF_EVENT("Audio.VMHScrapes");
						if (m_scrapeParams[vehicleIndex].m_fMaxFrictionStrength > m_scrapeParams[vehicleIndex].m_fDecayingPeakImpulse)
						{
						//what sort of body did the vehicle collide with?
							//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[CollidesWith],(F32)m_scrapeParams[vehicleIndex].m_CollidesWith);

							//	get collision position in world space
							Maths::HVec3 objPos = m_scrapeParams[vehicleIndex].m_Position/* * pRLC->GetVehicleActor(vehicleIndex)->GetMatrix()*/;
							Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[CollisionPosition], (F32*)&objPos);

							//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[ScrapeFriction], m_scrapeParams[vehicleIndex].m_fMaxFriction);//
							//printf("\nScrape: %f",m_scrapeParams[vehicleIndex].m_fMaxFriction);

							Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[ParticleScrape],false);

						}

							m_scrapeParams[vehicleIndex].m_bScrapeThisFrame =  false;
							m_scrapeParams[vehicleIndex].m_fMaxImpulseStrength = 0.f;					

					}
					 
					m_collideParams[vehicleIndex].m_fDecayingPeakImpulse *= gfCarCollisionPeakDecayRatioNew;
					m_scrapeParams[vehicleIndex].m_fDecayingPeakImpulse *= gfScrapeFrictionPeakDecayRatioNew;
					//end do collisions

					if(Audio::InterfaceController::Get().IsEntitySuspended(hCurrentEntity))
						break;

					//mute any ghosted vehicle
					if(pActor->IsEliminated() && !isFocusVehicle)
					{
						if (m_prevPriorityVols[vehicleIndex] != 0.f)
						{
							Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[PriorityVolume], 0.0f);
							m_prevPriorityVols[vehicleIndex] = 0.f;
						}

						if(hCurrentSharedEntity.IsValid())
						{
							if (m_prevSharedPriorityVols[vehicleIndex] != 0.f)
							{
								Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity, m_aHandle[PriorityVolumeShared], 0.0f);
								m_prevSharedPriorityVols[vehicleIndex] = 0.f;
							}
						}
						break;
					}

					/*U32 isVisible = 1;
					isVisible = pActor->GetVisibleDetect()->IsVisible();

					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[IsVisible], (F32)isVisible);*/

					{
						PERF_EVENT("Audio.VMHReflect");

					if (m_prevFocusCar != (S32)isFocusVehicle)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FocusVehicle], (F32)isFocusVehicle);
						m_prevFocusCar = isFocusVehicle;
					}

					Maths::HMat43 vehicleMatrix;
					//Maths::HMat43 modelMatrix = pActor->GetVehicleModel()->GetMatrix();
					Maths::HVec3 splineDirection;
					Maths::HVec3 soundDir;

					//******************REFLECTIONS********************************

#ifdef VEHICLE_OCCLUSION_LODDING
					//determine the Raytrace update rate
					F32 occlusionDistanceRatio  = distanceFromListener / g_OcclusionRayDistanceMax;
					U32 updateRate = (U32)(g_RaytraceUpdateLimit * occlusionDistanceRatio);

					bool processOcclusion = false;

					if( updateRate % g_RaytraceUpdateLimit == 0)
						processOcclusion = true;
#endif

					//if(g_AIReflections)
					//	isFocusVehicle = true;

					if(/*isFocusVehicle && */g_DoReflections && !m_isSplitScreen)//if(0)//
					{
						//float rightRayDistance;
						//float leftRayDistance;

						Maths::HVec3 slightRamp(0,(float)g_RayHeight,0);//do ray from a metre up
						Maths::HVec3 start = Maths::HVec3_Add(carPos,slightRamp);
						
						Maths::HVec3 startDir = objectMatrix.AZ();  //car direction	

						airacingtech::ISplineTracker *pVehicleSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(0);

						Maths::HVec3 validPos(0,0,0);
						Maths::HVec3 validDir(0,0,0); 
						float validSpeed = 0;

						if(pVehicleSplineTracker)
						{
							pVehicleSplineTracker->GetValidPosDir(validPos,validDir,validSpeed);
							startDir = validDir;

						}
							
						vehicleMatrix = objectMatrix;
								

						//vehicle direction
						splineDirection = startDir;//vehicleMatrix.AZ();

						//create normal - cross product of spline dir to vehicle up, get right axis
						Maths::C_Normal rightRayDir = Maths::C_Normal(vehicleMatrix.AY() ^ splineDirection);

						//create normal - cross product of spline dir to vehicle up, get right axis
						Maths::C_Normal leftRayDir = Maths::C_Normal(splineDirection ^vehicleMatrix.AY());

						Collision::C_CollisionFilter rayCollisionFilter( 0xffffffff, 
																		Game::PolyFlags::kDynamicObjectSolid, 0, 0, 0, 0xffffffff, Game::PolyFlags::kbWorld);				// Probably need to check ONLY for static scene geometry


						F32 rightAngle = 0;
						F32 leftAngle  = 0;
						
						
						Collision::C_RayTracingResult& 	result = m_pRightRay->m_result;
							

							Maths::C_Normal rightReflectionNormal;
							if (result.Hit())
							{

								//rightRayDistance = start.Y() - result.HitPoint().Y();
								//rightRayDistance = result.HitPoint().Y();
								if((float)(float)result.Distance() > .2f)  //hack to prevent bad rays
									m_rightRayDistance = (float)result.Distance();
									

								rightReflectionNormal = result.Normal();
								rightAngle = Maths::RadToDeg(acosf(rightRayDir | rightReflectionNormal));

								//send m
								U32 surface = result.GetEffectMaterial();

								if(surface != m_prevRightReflectionSurface)
								{
									Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RightReflectionSurface],(F32)surface);
									m_prevRightReflectionSurface = surface;
								}
					
							}


						
						
						
							//ray left

							result = m_pLeftRay->m_result;
							
							Maths::C_Normal leftReflectionNormal;
							if (result.Hit())
							{

								//leftRayDistance = start.Y() - result.HitPoint().Y();
								//leftRayDistance = result.HitPoint().Y();
								if((float)result.Distance() > .2f)  //hack to prevent bad rays
									m_leftRayDistance = (float)result.Distance();


								leftReflectionNormal = result.Normal();
								leftAngle = Maths::RadToDeg(acosf(leftRayDir | leftReflectionNormal));
								
								U32 surface = result.GetEffectMaterial();

								if(surface != m_prevLeftReflectionSurface)
								{
									Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[LeftReflectionSurface],(F32)surface);
									m_prevLeftReflectionSurface = surface;

								}
								
				
							}		
						

						//ray right
						m_pRightRay->InitialiseRay(start, rightRayDir, (float)g_RayLength, &rayCollisionFilter, 0.1f);
						m_pLeftRay->InitialiseRay(start, leftRayDir,(float) g_RayLength, &rayCollisionFilter, 0.1f);
						
						scene->DoDeferredRayTrace(m_pRightRay,NULL,m_Group,m_Policy, true);
						scene->DoDeferredRayTrace(m_pLeftRay,NULL,m_Group,m_Policy, true);
	
						//calculate reflection orientation
						Maths::HVec3 leftReflectionSurfacePosition = Maths::HVec3_Add((leftRayDir * m_leftRayDistance),start);
						Maths::HVec3 leftReflectionSourcePosition = Maths::HVec3_Add((leftRayDir * m_leftRayDistance*2),start);
						
						//want the plane to be perpendicular to the ray, so zero the Y axis
						Maths::HVec3 tempVector(leftReflectionNormal);
						tempVector.ZeroY();
						Maths::C_Normal newLeftNormal(tempVector.Normalised());
						leftReflectionNormal = newLeftNormal; 
						
						//get what side of the reflection plane the camera/listener lies
						F32 leftReflectionSide = ((Maths::HVec3)leftReflectionNormal | camPos) +  (((Maths::HVec3)leftReflectionNormal*-1)  | leftReflectionSurfacePosition);

						{
							S32 reflectVal = (leftReflectionSide <= 0) ? 0 : 1;
							if (reflectVal != m_prevLeftReflectionVal)
							{
								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[LeftReflectionLevel], (F32)reflectVal);
								m_prevLeftReflectionVal = reflectVal;
							}
						}

						Maths::HVec3 rightReflectionSurfacePosition = Maths::HVec3_Add((rightRayDir * m_rightRayDistance),start);
						Maths::HVec3 rightReflectionSourcePosition = Maths::HVec3_Add((rightRayDir * m_rightRayDistance*2),start);


						//want the plane to be perpendicular to the ray, so zero the Y axis
						tempVector = rightReflectionNormal;
						tempVector.ZeroY();
						Maths::C_Normal newrightNormal(tempVector.Normalised());
						rightReflectionNormal = newrightNormal; 

						F32 rightReflectionSide = ((Maths::HVec3)rightReflectionNormal | camPos) +  (((Maths::HVec3)rightReflectionNormal*-1)  | rightReflectionSurfacePosition);

						//get what side of the reflection plane the camera/listener lies
						{
							S32 reflectVal = (rightReflectionSide <= 0) ? 0 : 1;
							if (reflectVal != m_prevRightReflectionVal)
							{
								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RightReflectionLevel], (F32)reflectVal);
								m_prevRightReflectionVal = reflectVal;
							}
						}

						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[RightReflectionPosition], (F32*)(&rightReflectionSourcePosition));
						//reflection is in the direction it came from
						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[RightReflectionDirection], (F32*)(&(rightRayDir * -1)));
						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[RightReflectionUp], (F32*)(&info.pActor->GetMatrix().AY()));
						
						//set the normal
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RightNormalAngle],rightAngle);

						
						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[LeftReflectionPosition], (F32*)(&leftReflectionSourcePosition));
						//reflection is in the direction it came from
						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[LeftReflectionDirection], (F32*)(&(leftRayDir * -1)));
						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[LeftReflectionUp], (F32*)(&info.pActor->GetMatrix().AY()));
						//set the normal
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[LeftNormalAngle],leftAngle);


						#ifndef CFG_GOLD
						//DEBUG DEBUG DEBUG
						if(g_RenderReflectionPositions)
						{
							S32 red = 0xff;
							S32 green = 0xff;
							S32 blue = 0xff;
							F32 radius = 1;

							Maths::RGBA colourYellow = Maths::RGBA(red, green, 0x00, 0xff);
							Maths::RGBA colourRed = Maths::RGBA(red, 0x00, 0x00, 0x00);
							Maths::RGBA colourBlue = Maths::RGBA(0x00, 0x00, blue, 0xff);
							Maths::RGBA colourGreen = Maths::RGBA(0x00, green, 0x00, 0xff);

							//image source green
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(rightReflectionSourcePosition,radius,colourGreen);
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(leftReflectionSourcePosition,radius,colourGreen);

							//image direction
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(rightReflectionSourcePosition, rightRayDir * -1, m_rightRayDistance, 1, colourGreen);
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(leftReflectionSourcePosition, leftRayDir * -1, m_leftRayDistance, 1, colourGreen);


							//ray direction yellow
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(start, rightRayDir, m_rightRayDistance, 1, colourYellow);
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(start, leftRayDir , m_leftRayDistance, 1, colourYellow);


							//do distance shaping so cars in front view get distance priority over those at rear...
							//cars directly behind are x2 distance away...
							if(vehicleIndex == 0)
								camPos = objectMatrix.AW();							
							
							Maths::HVec3 carPos = objectMatrix.AW();

							static Maths::HVec3 shapingDir;

							if(vehicleIndex == 1)
								shapingDir = carPos - camPos;
														
							Maths::HVec3 normalShapingDir = Normalised(shapingDir);

							F32 angle = (acosf ( camMatrix.AZ() | normalShapingDir)) / Maths::Pi;
							
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(camPos,normalShapingDir,10,1,colourGreen);


							

							//left normal blue
							//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(leftReflectionSurfacePosition, leftReflectionNormal , 30, 1, colourBlue);
							//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(rightReflectionSurfacePosition, rightReflectionNormal , 30, 1, Maths::RGBA(0x00, 0x00, 0xff, 0xff));

							////left plane red
							//Maths::C_Normal norm = leftReflectionNormal ^ Maths::C_Normal(1,0,1);
							//
				
							Maths::C_Plane plano(leftReflectionNormal,leftReflectionSurfacePosition);
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlane(plano,  leftReflectionSurfacePosition, 1000, colourRed);

							//	//left plane red
							Maths::C_Plane plano2(rightReflectionNormal,rightReflectionSurfacePosition);
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlane(plano2,  rightReflectionSurfacePosition, 1000, colourBlue);

						





						}
							//DEBUG DEBUG DEBUG
						#endif



					}
					//*******************END REFLECTIONS******************************************

					} //end PERF_EVENT("Audio.VMHDistanceScaling");

					//occlusion
										
					U32 occlusionValue = 0;

					//only do occlusion for other vehicles and never in splitscreen. only if less than max length
					if(g_DoOcclusion  && !m_isSplitScreen && !isFocusVehicle )
					{
						if((m_distanceToOcclusionDestination < g_OcclusionRayDistanceMax))
							occlusionValue = GetOcclusionValue(objectMatrix,camMatrix,m_distanceToOcclusionDestination,scene);
						else
							occlusionValue = 1;  //set to occluded if out of range
					}
					
					//send occlusion to the shared entity
					if(hCurrentSharedEntity.IsValid() && m_prevOcclusionValue != occlusionValue)					
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity,m_aHandle[OcclusionValue], (F32)occlusionValue);	
						m_prevOcclusionValue = occlusionValue;
					}
					//set the audio occlusion value in the vehicle
					pActor->SetAudioOcclusionValue(occlusionValue);

					//Send Vectors

					//Maths::HMat43 camMatrix = view.DebugFeed().Mat43();
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[CarListenerPosition], (F32*)(&camPos));
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[CarListenerDirection], (F32*)(&camMatrix.AZ()));					
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[CarListenerUp], (F32*)(&camMatrix.AY()));
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[CarListenerVelocity], (F32*)(&velocity));

					Audio::InterfaceController::Get().SetEntityVectorIn(handle,  m_aHandle[CarPosition],(F32*)(&carPos));
					Audio::InterfaceController::Get().SetEntityVectorIn(handle,  m_aHandle[CarUp], (F32*)(&objectMatrix.AY()));
					Audio::InterfaceController::Get().SetEntityVectorIn(handle,  m_aHandle[CarDirection], (F32*)(&(objectMatrix.AZ()* -1)));
					Audio::InterfaceController::Get().SetEntityVectorIn(handle,  m_aHandle[CarVelocity], (F32*)&objectVelocity);

					if(g_doSharedEntityMatrixUpdate)
					{
						if(hCurrentSharedEntity.IsValid())
						{
							Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentSharedEntity, m_aHandle[ListenerPosition], (F32*)(&camPos));
							Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentSharedEntity, m_aHandle[ListenerDirection], (F32*)(&camMatrix.AZ()));					
							Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentSharedEntity, m_aHandle[ListenerUp], (F32*)(&camMatrix.AY()));
							Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentSharedEntity, m_aHandle[ListenerVelocity], (F32*)(&velocity));

							Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentSharedEntity,  m_aHandle[ObjectPosition],(F32*)(&carPos));
							Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentSharedEntity,  m_aHandle[ObjectUp], (F32*)(&objectMatrix.AY()));
							Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentSharedEntity,  m_aHandle[ObjectDirection], (F32*)(&(objectMatrix.AZ()* -1)));
							Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentSharedEntity,  m_aHandle[ObjectVelocity], (F32*)&objectVelocity);
						}
					}

					if( g_SlowMoOverrideControls )
					{
						F32 engineRPM = (F32)pCar->Engine().RPM();
						F32 limiterRPM = pCar->Engine().LimiterRPM();

						if(engineRPM >= limiterRPM)
								Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[HitLimiter], false);

						F32 fThrottle = g_SlowMoThrottle;
						F32 fPadThrottle = g_SlowMoThrottle;
						if( m_isSlowMo )
							m_rpm *= g_SlowMoRPMDecay;
						else
						{
							m_rpm = engineRPM * .0001f;
							fThrottle = pCar->Engine().AdjustedThrottle();
							fPadThrottle = pCar->Controls().throttle;
						}

												
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity,m_aHandle[DynamicsRPM], m_rpm);					

						if (m_prevThrottle != fThrottle)
						{
							Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[Throttle], fThrottle);
							m_prevThrottle = fThrottle;
						}

						if (m_prevPadThrottle != fPadThrottle)
						{
							Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[PadThrottle], fPadThrottle);
							m_prevPadThrottle = fPadThrottle;
						}
					}
					else
					{

						F32 engineRPM = (F32)pCar->Engine().RPM();
						F32 limiterRPM = pCar->Engine().LimiterRPM();

						if(engineRPM >= limiterRPM)
								Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[HitLimiter], false);

				
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity,m_aHandle[DynamicsRPM], engineRPM * .0001f);					
						
						F32 fThrottle = pCar->Engine().AdjustedThrottle();
						if (m_prevThrottle != fThrottle)
						{
							Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[Throttle], fThrottle);
							m_prevThrottle = fThrottle;
						}

						F32 fPadThrottle = pCar->Controls().throttle;
						if (m_prevPadThrottle != fPadThrottle)
						{
							Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[PadThrottle], fPadThrottle);
							m_prevPadThrottle = fPadThrottle;
						}
					}

					//Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[Horn], false);

					//turbo
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[Turbo], pCar->Engine().Boost());

					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[GroundSpeed], fGndSpeed);

					F32 clutch = pCar->Transmission().ClutchEngaged();
					if(clutch != m_PrevClutchValue)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[Clutch], clutch);
						m_PrevClutchValue = clutch;
					}

					F32 gear = (F32)pCar->Transmission().Gear();
					if(gear != m_PrevGear)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[Gear], (F32)pCar->Transmission().Gear());
						m_PrevGear = gear;
					}

					if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
					{
						PERF_EVENT("Audio.VMHRLLay");
						Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
						S32 mCurrentHealth = 0;
						
						if(pLogicActor)
						{

							if(isFocusVehicle)
							{
								mCurrentHealth = pLogicActor->GetCurrentHealth();
								E_HEALTH_LEVEL			mCurrentHealthLevel = pLogicActor->GetCurrentHealthLevel();

								if(mCurrentHealthLevel != m_PrevHealthLevel)
								{
									Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[OverallDamageLevel], (F32)mCurrentHealthLevel);
									m_PrevHealthLevel = mCurrentHealthLevel;
									
									//send healthlevel to UI
									Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eUISubsystem, UISubsystem::eCurrentHealthLevel,0,&mCurrentHealthLevel);
								}

								if(mCurrentHealth != m_PrevHealth)
								{

									if(hCurrentSharedEntity.IsValid())
										Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity, m_aHandle[OverallDamageValue], (F32)mCurrentHealth);

									m_PrevHealth = mCurrentHealth;
									
									//send health to UI
									Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eUISubsystem, UISubsystem::eCurrentHealth,0,&mCurrentHealth);
								}
							}
							else
							{								
								//if non focuscar gets wrecked
								if(hCurrentSharedEntity.IsValid() && pLogicActor->IsWreckedOrEliminated() && !m_isWrecked)
								{
									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[Wrecked],false);
									//fwd on to UI
									//Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eUISubsystem, UISubsystem::eCurrentHealth,0,&mCurrentHealth);
									m_isWrecked = true;

								}
								else
								if(!pLogicActor->IsWreckedOrEliminated() && m_isWrecked)
								{
									m_isWrecked = false;
								}
									
							}

							if(pLogicActor->mShieldTime > 0.f && !m_isShielded)
							{
								//SEND THE SHIELD INFO TO THE SHARED ENTITY
								if(hCurrentSharedEntity.IsValid())
									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[ShieldInitiated],false);
								m_isShielded = true;
							}
							else if(pLogicActor->mShieldTime <= 0.f && m_isShielded)
							{
								//SEND THE SHIELD INFO TO THE SHARED ENTITY
								if(hCurrentSharedEntity.IsValid())
									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[ShieldFinished],false);
								m_isShielded = false;

							}

							if(pLogicActor->IsCloaked() && !m_isSilentRunning)
							{
								//SEND THE SHIELD INFO TO THE SHARED ENTITY
								if(hCurrentSharedEntity.IsValid())
									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[SilentRunningInitiated],false);
								m_isSilentRunning = true;
							}
							else if(!pLogicActor->IsCloaked() && m_isSilentRunning)
							{
								//SEND THE SHIELD INFO TO THE SHARED ENTITY
								if(hCurrentSharedEntity.IsValid())
									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[SilentRunningFinished],false);
								m_isSilentRunning = false;
							}					

						}
					}

					/////debug...remove.....
					//static bool check5 = false;
					//if(check5)
					//	return false;
					////debug remove....

					//check if there has been a pop, then broadcast a message.
					if (Audio::InterfaceController::Get().GetCallbackTriggeredCount(m_hQueryPopTrigger, true))
					{
						Game::Actors::C_VehicleExhaustPopMsg msg;
						msg.m_VehicleIndex = vehicleIndex;
						msg.m_ExhaustPopType = Game::Actors::C_VehicleExhaustPopMsg::E_EXHAUSTPOP_POP;
						/*[ChadW] - BroadcastMessage_CleanUp*///pRLC->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);
						pActor->TriggerExhaustPop(&msg);
					}
					if (Audio::InterfaceController::Get().GetCallbackTriggeredCount(m_hQueryBangTrigger, true))
					{
						Game::Actors::C_VehicleExhaustPopMsg msg;
						msg.m_VehicleIndex = vehicleIndex;
						msg.m_ExhaustPopType = Game::Actors::C_VehicleExhaustPopMsg::E_EXHAUSTPOP_BANG;
						/*[ChadW] - BroadcastMessage_CleanUp*///pRLC->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);
						pActor->TriggerExhaustPop(&msg);
					}
					if (Audio::InterfaceController::Get().GetCallbackTriggeredCount(m_hQueryLeftPopTrigger, true))
					{
						Game::Actors::C_VehicleExhaustPopMsg msg;
						msg.m_VehicleIndex = vehicleIndex;
						msg.m_ExhaustPopType = Game::Actors::C_VehicleExhaustPopMsg::E_EXHAUSTPOP_LEFTPOP;
						/*[ChadW] - BroadcastMessage_CleanUp*///pRLC->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);
						pActor->TriggerExhaustPop(&msg);
					}
					if (Audio::InterfaceController::Get().GetCallbackTriggeredCount(m_hQueryRightPopTrigger, true))
					{
						Game::Actors::C_VehicleExhaustPopMsg msg;
						msg.m_VehicleIndex = vehicleIndex;
						msg.m_ExhaustPopType = Game::Actors::C_VehicleExhaustPopMsg::E_EXHAUSTPOP_RIGHTPOP;
						/*[ChadW] - BroadcastMessage_CleanUp*///pRLC->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);
						pActor->TriggerExhaustPop(&msg);
					}
					if (Audio::InterfaceController::Get().GetCallbackTriggeredCount(m_hQueryLeftBangTrigger, true))
					{
						Game::Actors::C_VehicleExhaustPopMsg msg;
						msg.m_VehicleIndex = vehicleIndex;
						msg.m_ExhaustPopType = Game::Actors::C_VehicleExhaustPopMsg::E_EXHAUSTPOP_LEFTBANG;
						/*[ChadW] - BroadcastMessage_CleanUp*///pRLC->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);
						pActor->TriggerExhaustPop(&msg);
					}
					if (Audio::InterfaceController::Get().GetCallbackTriggeredCount(m_hQueryRightBangTrigger, true))
					{
						Game::Actors::C_VehicleExhaustPopMsg msg;
						msg.m_VehicleIndex = vehicleIndex;
						msg.m_ExhaustPopType = Game::Actors::C_VehicleExhaustPopMsg::E_EXHAUSTPOP_RIGHTBANG;
						/*[ChadW] - BroadcastMessage_CleanUp*///pRLC->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);
						pActor->TriggerExhaustPop(&msg);
					}

					return WorldObjectMessageHandler::ProcessMessage(info);


			}
		break;

		case ActorMsg::E_DynamicObjMsg_VehicleEnteredTrigger:
		{
			const ActorMsg::C_VehicleEnteredTriggerMsg* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleEnteredTriggerMsg>(info.pMessage);

		
			if(pMsg->m_TriggerType == Game::E_TRIGGER_TYPE_TUNNEL_ENTER)
			{
				m_reverbCounter++;

				if(m_reverbCounter == 1)
				{
					S32 tunnelPreset = pMsg->m_RefNumber;
					
					if(tunnelPreset <= AUDIO_TUNNEL_TYPE_REVERB_WHOOSH_HEAVY)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[TunnelPreset],(F32)tunnelPreset);
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[InTunnel],(F32)1);
					}
				}

			}

		

			
		}
		break;

		case ActorMsg::E_DynamicObjMsg_VehicleExitedTrigger:
		{
			const ActorMsg::C_VehicleExitedTriggerMsg* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleExitedTriggerMsg>(info.pMessage);

			if(pMsg->m_TriggerType ==  Game::E_TRIGGER_TYPE_TUNNEL_EXIT)
			{
				m_reverbCounter--;
				//S32 tunnelPreset = pMsg->m_RefNumber;
				//TunnelPreset

				S32 tunnelPreset = pMsg->m_RefNumber;
					
				if(tunnelPreset <= AUDIO_TUNNEL_TYPE_REVERB_WHOOSH_HEAVY)
				{
					if(m_reverbCounter == 0)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[InTunnel],(F32)0);
						
					}
				}

			}
			
		}
		break;


		//}
		////call world object
		//switch(info.pMessage->Type())
		//{

			case ActorMsg::E_ResourceMsg_AllLoaded:
				{
				}
			break;

			case Game::E_VehicleMessage_AudioVehiclePartSlamming:
				{
					const Game::Actors::C_VehicleAudioVehiclePartSlamming* p_pMsg = Messaging::MessageCast<Game::Actors::C_VehicleAudioVehiclePartSlamming>(info.pMessage);

					switch(p_pMsg->m_PartCode)
					{
						case Vehicle::E_PartCode_Bonnet:
							break;

					}
					

				}
				break;


			case Game::E_VehicleMessage_AudioImpactEvent:
				{

					if(!m_bEnableCollisions)
						return true;

					const Game::Actors::C_VehicleAudioImpactMsg* p_pMsg = Messaging::MessageCast<Game::Actors::C_VehicleAudioImpactMsg>(info.pMessage);
		
					if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_ScrapeStatic || p_pMsg->m_Event == Vehicle::E_DefferedEvent_ScrapeDynamic)
					{
						//3rd param of custom message is vehicle index
						if(m_isShielded)
							m_collideParams[m_vehicleIndex].m_Shielded = true;
						else
							m_collideParams[m_vehicleIndex].m_Shielded = false;

						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eCollisionSubsystem, CollisionSubsystem::E_CollisionSubsystem_Scrape,m_vehicleIndex,&m_collideParams[m_vehicleIndex]);

						break;
					}



					//cian 
					//part impacts are now going to a different entity contained in the Vehicle Component AdditonalEntity
					Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity, m_aHandle[PartCollisionForce],m_collideParams[m_vehicleIndex].m_fPreviousMaxImpulseStrength);

					//if there is no car part entity, break here
					if(!hCurrentSharedEntity.IsValid())
						break;
				
					switch(p_pMsg->m_PartCode)
					{
						case Vehicle::E_PartCode_Bonnet:
							if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
							{
								//printf("Bonnetr Part closed\n");
								Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BonnetClosed],false);
							}
							else 
							if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
							{
								//printf("Bonnetr Part opened\n");
								Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BonnetOpened],false);
							}
							else
							{
								if(p_pMsg->m_Attached)
									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BonnetCollision],false);
							}
							break;

						case Vehicle::E_PartCode_Boot:
							if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
							{
								//printf("Boot Part closed\n");
								Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BootClosed],false);
							}
							else
							if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
							{
								//printf("Boot Part opened\n");
								Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BootOpened],false);
							}
							else
							{
								if(p_pMsg->m_Attached)
									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BootCollision],false);
							}
							break;

						case Vehicle::E_PartCode_Wing:
							{

								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Right)
								{
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
										break;
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
										break;
									else
									{
										if(p_pMsg->m_Attached)
											Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[WingRightCollision],false);
									}

								}


								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Left)
								{
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
										break;
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
										break;
									else
									{
										if(p_pMsg->m_Attached)
											Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[WingLeftCollision],false);
									}
								}

							}
							break;

						case Vehicle::E_PartCode_Mirror:
							{

								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Right)
								{
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
										break;
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
										break;
									else
									{
										if(p_pMsg->m_Attached)
										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[MirrorRightCollision],false);
									}
								}

								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Left)
								{
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
										break;
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
										break;
									else
									{
										if(p_pMsg->m_Attached)
										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[MirrorLeftCollision],false);
									}
								}

							}
							break;

						case Vehicle::E_PartCode_Door:
							{
								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Right || p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_FrontRight)
								{
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
									{
										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorFrontRightClosed],false);
									}
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
									{
										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorFrontRightOpened],false);
									}
									else
									{
										if(p_pMsg->m_Attached)
											Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorRightCollision],false);
									}
								}

								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Left || p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_FrontLeft)
								{
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
									{
										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorFrontLeftClosed],false);
									}
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
									{
										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorFrontLeftOpened],false);
									}
									else
									{
										if(p_pMsg->m_Attached)
												Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorLeftCollision],false);
									}
								}

								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_BackLeft)
								{

									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
									{
											Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorBackLeftClosed],false);
									}
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
									{
											Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorBackLeftOpened],false);
									}
									else
									{
										if(p_pMsg->m_Attached)
											Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorBackLeftCollision],false);

										
									}
								}

								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_BackRight)
								{
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
									{
										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorBackRightClosed],false);
									}
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
									{
										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorBackRightOpened],false);
									}
									else
									{
										if(p_pMsg->m_Attached)
											Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DoorBackRightCollision],false);
									}
								}
							}
							break;

						case Vehicle::E_PartCode_Bumper:
							{
								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Front)
								{

									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
									{
										//Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BumperFrontClosed],false);
									}
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
									{
										//Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BumperFrontOpened],false);
									}
									else
									{
										if(p_pMsg->m_Attached)
											Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BumperFrontCollision],false);
									}
								}

								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Back)
								{

									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
									{
										//Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BumperBackClosed],false);
									}
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
									{
										//Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BumperBackOpened],false);
									}
									else
									{
										if(p_pMsg->m_Attached)
											Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BumperBackCollision],false);
									}
								}
							}
							break;

						case Vehicle::E_PartCode_LicensePlate:
							{
								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Front)
								{

									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_Unknown)
									{
										
										//Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BumperFrontCollision],false);
									}
								}
								else
								if(p_pMsg->m_PartSubCode == Vehicle::E_PartSubCode_Back)
								{

									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_Unknown)
									{
										
										//Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[BumperFrontCollision],false);
									}
								}
							}
							break;

						default:
							{

									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartClosed)
										break;
									else
									if(p_pMsg->m_Event == Vehicle::E_DefferedEvent_PartOpened)
										break;
									else
									{
										Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity, m_aHandle[DamagedPartCode],(F32)p_pMsg->m_PartCode);
										Audio::InterfaceController::Get().SetEntityValue(hCurrentSharedEntity, m_aHandle[DamagedPartSubCode],(F32)p_pMsg->m_PartSubCode);
										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentSharedEntity, m_aHandle[DamagedPartActionIn],false);
									}

							}
							break;





					}
						
				}
				break;
			case Game::E_GameLogicMessage_Impact:
			{

				if(!m_bEnableCollisions)
						return true;

				const Game::C_ImpactMsg* p_pMsg		= Messaging::MessageCast<Game::C_ImpactMsg>(info.pMessage);
				Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

				Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(info.pActor);

				Maths::HMat43 camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();

				//sort out viewports, get correct listener for view
				if(m_isSplitScreen && (pActor->GetVehicleIndex() < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
				{
					camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(pActor->GetVehicleIndex()).DebugFeed().Mat43();
				}
				else
				if(m_isSplitScreen && (pActor->GetVehicleIndex() >= Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
				{
					//if it's an AI car, we wish to play the sound from the closest viewport
					int currentView = 0;
					
					F32 nearestDistance = 1000000; //big value
					
					for(int i = 0 ; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); i++)
					{

						Maths::HVec3 entityDistance = ((Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43().AW())  - (pActor->GetMatrix().AW()));
						F32 length = Maths::Length(entityDistance);
						
						if(length < nearestDistance)
						{
							nearestDistance = length;
							currentView = i;
						}

					}

					camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Mat43();
				}

				//don't process collisions more than distance
				
				if(camMatrix.AW().Distance(pActor->GetMatrix().AW()) > (F32)g_MaxCollisionDistance)
					break;

				const Actor::C_Actor* actorA = pRLC->GetActorList()->GetActorPtr(p_pMsg->mActorHandleA);
				const Actor::C_Actor* actorB = pRLC->GetActorList()->GetActorPtr(p_pMsg->mActorHandleB);

				const Game::Actors::C_VehicleActor* pVehActorA = (const Game::Actors::C_VehicleActor*)pRLC->GetActorList()->GetActorPtr(p_pMsg->mActorHandleA);
				const Game::Actors::C_VehicleActor* pVehActorB = (const Game::Actors::C_VehicleActor*)pRLC->GetActorList()->GetActorPtr(p_pMsg->mActorHandleB);

				AUDIO_COLLISION_COLLIDES_WITH otherActor = AUDIO_COLLISION_COLLIDES_WITH_WORLD; //is not vehicle or dyn obj


				if(actorA  && (actorA == pActor) && actorA->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)
				{
					//const Game::Actors::C_VehicleActor* pVehActorA = (const Game::Actors::C_VehicleActor*)actorA;

					if(actorB)
					{
						if(actorB->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)
								otherActor = AUDIO_COLLISION_COLLIDES_WITH_VEHICLE;
						else
							if(actorB->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject || actorB->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
								otherActor = AUDIO_COLLISION_COLLIDES_WITH_OTHER;
					}


					//if I am an AI and actorB is the player, i don't want to play a collison
					if(otherActor == AUDIO_COLLISION_COLLIDES_WITH_VEHICLE && ((pVehActorA && pVehActorA->GetControlType() == ::E_VEHICLE_CONTROL_TYPE_AI) && (pVehActorB && pVehActorB->GetControlType() == ::E_VEHICLE_CONTROL_TYPE_PLAYER)))
					{
						//do nothing
					}
					else
						VehicleCollision(pVehActorA->GetVehicleIndex(), p_pMsg,pActor->GetMatrix(),otherActor);
				}
				else
				{
					if(actorB && (actorB == pActor)  && actorB->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)
					{
						//const Game::Actors::C_VehicleActor* pVehActorB = (const Game::Actors::C_VehicleActor*)actorB;

						if(actorA)
						{
							if(actorA->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)
									otherActor = AUDIO_COLLISION_COLLIDES_WITH_VEHICLE;
							else
								if(actorA->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject || actorA->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
									otherActor = AUDIO_COLLISION_COLLIDES_WITH_OTHER;
						}

						//if I am an AI and actorA is the player, i don't want to play a collison
						if(otherActor == AUDIO_COLLISION_COLLIDES_WITH_VEHICLE && ((pVehActorB && pVehActorB->GetControlType() ==::E_VEHICLE_CONTROL_TYPE_AI) && (pVehActorA && pVehActorA->GetControlType() == ::E_VEHICLE_CONTROL_TYPE_PLAYER)))
						{

							//do nohting
						}
						else 
						if(otherActor == AUDIO_COLLISION_COLLIDES_WITH_VEHICLE && (( pVehActorB && pVehActorB->GetControlType() == ::E_VEHICLE_CONTROL_TYPE_AI) && (pVehActorA && pVehActorA->GetControlType() == ::E_VEHICLE_CONTROL_TYPE_AI)))
						{
							//do nohting
						}
						else
							VehicleCollision(pVehActorB->GetVehicleIndex(), p_pMsg,pActor->GetMatrix(),otherActor);
					}
				}
			}
			break;

		// Visibility messages.
		//
		case ::Actors::E_VisibilityMsg_OnVisible:
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[IsVisible], (F32)1);
			break;

		case ::Actors::E_VisibilityMsg_OnHidden:
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[IsVisible], (F32)0);
			break;

			//break;			
			//default:
			//	return WorldObjectMessageHandler::ProcessMessage(info);
		}//end switch

	return true;
}

//	UIMessageHandler
Components::UIMessageHandler::UIMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom   ) : WorldObjectMessageHandler( hEntityDefinition, pCom )
{
	m_pCom = pCom;
	for (U32 uId = WorldObjectMessageHandler::NumMessageIds; uId < NumMessageIds; uId++)
		m_aHandle.PushBack( Audio::InterfaceController::Get().LookupInstanceComponent( hEntityDefinition, UIMessageHandler::GetString( uId ) ) );
}

bool Components::UIMessageHandler::ProcessMessage(BaseMessageHandlerInfo& info)
{
	switch(info.pMessage->Type())
	{
		case ActorMsg::E_AudioMsg_TriggerActionFromHandle:
		{
			const ActorMsg::C_AudioTriggerActionFromHandleMsg* msg = Messaging::MessageCast<ActorMsg::C_AudioTriggerActionFromHandleMsg>(info.pMessage);
			if (msg->m_uEntityDataMemberHnd != 0xffffffff)
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_pCom->GetEntityHnd(), msg->m_uEntityDataMemberHnd);
			break;
		}

		default:
			return WorldObjectMessageHandler::ProcessMessage(info);
	}

	return true;
}

//	MusicMessageHandler
Components::MusicMessageHandler::MusicMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom   ) : WorldObjectMessageHandler( hEntityDefinition, pCom )
{
	m_pCom = pCom;
	for (U32 Musicd = WorldObjectMessageHandler::NumMessageIds; Musicd < NumMessageIds; Musicd++)
		m_aHandle.PushBack( Audio::InterfaceController::Get().LookupInstanceComponent( hEntityDefinition, MusicMessageHandler::GetString( Musicd ) ) );
}

bool Components::MusicMessageHandler::ProcessMessage(BaseMessageHandlerInfo& info)
{
	switch(info.pMessage->Type())
	{
		case ActorMsg::E_AudioMsg_TriggerActionFromHandle:
		{
			const ActorMsg::C_AudioTriggerActionFromHandleMsg* msg = Messaging::MessageCast<ActorMsg::C_AudioTriggerActionFromHandleMsg>(info.pMessage);
			if (msg->m_uEntityDataMemberHnd != 0xffffffff)
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_pCom->GetEntityHnd(), msg->m_uEntityDataMemberHnd);
			break;
		}

		default:
			return WorldObjectMessageHandler::ProcessMessage(info);
	}

	return true;
}

//	VOMessageHandler
Components::VOMessageHandler::VOMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom   ) : WorldObjectMessageHandler( hEntityDefinition, pCom )
{
	m_pCom = pCom;
	for (U32 VOd = WorldObjectMessageHandler::NumMessageIds; VOd < NumMessageIds; VOd++)
		m_aHandle.PushBack( Audio::InterfaceController::Get().LookupInstanceComponent( hEntityDefinition, VOMessageHandler::GetString( VOd ) ) );

	Audio::InterfaceController::Get().SuspendEntity(m_pCom->GetEntityHnd());
}

bool Components::VOMessageHandler::ProcessMessage(BaseMessageHandlerInfo& info)
{
	return true;
}

//	DynamicObjMessageHandler
Components::DynamicObjMessageHandler::DynamicObjMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom   ) : WorldObjectMessageHandler( hEntityDefinition, pCom )
{
	m_pCom = pCom;
	for (U32 uId = WorldObjectMessageHandler::NumMessageIds; uId < NumMessageIds; uId++)
	{
				Audio::EntityDataMemberHandle memberHandle = -1;
		if(uId != FINAL_REQUISITE_EXTERNAL)
		{
			memberHandle = Audio::InterfaceController::Get().LookupInstanceComponent( hEntityDefinition, DynamicObjMessageHandler::GetString( uId ) );
		}
			m_aHandle.PushBack( memberHandle );

		#ifndef CFG_GOLD
			//check for required params
		if(memberHandle == -1 && uId < FINAL_REQUISITE_EXTERNAL)
		{
			char name[512]; // [LloydW] This was running over the end of the array :-(
			strcpy_s(name,sizeof(name),DynamicObjMessageHandler::GetString( uId ));
			strcat_s(name,sizeof(name),": Required Audio External is Missing. Check audio data or let audio coder know this is optional");
			Audio::InterfaceController::Get().Error(name);
			DebugBreak();
		}
		#endif
	}

	m_MarkForDelete = false;
	m_CollisionHash = 0;
//	m_pTriggerObject = NULL;
//	m_pGameWorld = NULL;
}

Components::DynamicObjMessageHandler::~DynamicObjMessageHandler()
{
	//if(m_actorHandle)
		//DestroyTrigger(m_actorHandle);
}

void Components::DynamicObjMessageHandler::SetCallbackState(const AudioEntityHandle& hEntity, CallbackState state)
{
	m_eCallbackState = state;

	if (hEntity.IsValid())
		C_DynamicEntityList::Get().SetCallbackState(const_cast<AudioEntityHandle&>(hEntity), state == eWaitingForCallback);
}
		

void Components::DynamicObjMessageHandler::DynamicCollision(const Game::C_ImpactMsg* p_pMsg, Actor::C_ActorHandle actorHandle, AUDIO_COLLISION_COLLIDES_WITH collisionOther, U32 occlusionValue)
{
	//if(m_CollisionHash == p_pMsg->mHashValue)
	//	return;
	
	F32 fImpulse = p_pMsg->mCollisionInfo.m_GameplayImpulse;
	fImpulse = p_pMsg->mCollisionInfo.m_MaxImpulse;
	//fImpulse = p_pMsg->mCollisionInfo.m_AudioImpulse;
	
	if (fImpulse > gfDynamicObjCollisionImpulseThresholdLowNew && fImpulse > m_DynamicObjectCollideParams[0].m_fMaxImpulseStrength)
	{
		m_DynamicObjectCollideParams[0].m_fMaxImpulseStrength = fImpulse;
		m_DynamicObjectCollideParams[0].m_bCollisionThisFrame = true;
		m_DynamicObjectCollideParams[0].m_Position = p_pMsg->mCollisionInfo.m_ContactPoints[0].m_ContactPoint;
		m_DynamicObjectCollideParams[0].dynamicObjectActorHandle = actorHandle;
		m_DynamicObjectCollideParams[0].m_fDecayingPeakImpulse = m_DynamicObjectCollideParams[0].m_fMaxImpulseStrength;
		m_DynamicObjectCollideParams[0].m_isVisible = p_pMsg->mCollisionInfo.m_actorIsVisible;
		m_DynamicObjectCollideParams[0].m_CollidesWith = collisionOther;
		m_DynamicObjectCollideParams[0].m_occlusionValue = occlusionValue;
		
		//m_collideParams[p_pMsg->m_VehicleIndex].m_Shielded = false;
		//m_collideParams[p_pMsg->m_VehicleIndex].m_MaterialType =  p_pMsg->m_CollisionInfo.m_SurfaceType;

	}

	m_CollisionHash = p_pMsg->mHashValue;

	//m_bEnableCollisions = false;
			
}

bool Components::DynamicObjMessageHandler::ProcessMessage(BaseMessageHandlerInfo& info)
{

	const AudioEntityHandle& hCurrentEntity = m_pCom->GetEntityHnd();

	switch(info.pMessage->Type())
	{
		case ActorMsg::E_AudioMsg_ReleaseEntity:
			//printf("releasing entity 0x%x on message\n", m_pCom->m_hEntityInst.ArrayHandle());
			m_pCom->ReleaseEntityInstance();
			break;

		case Actor::E_ActorMsg_ComponentCreated:
		{
				info.pHandler->AddSubscriber( Actor::E_ActorMsg_PhysicsBodyEvent );			

				m_bEnableCollisions = true;
				SetCallbackState(hCurrentEntity, eCallbackNotSet);
				m_OcclusionValue = 0;
				break;
		}

			//hack to mute collisions until race starts (glass smashes ass cars are dropped on the track)
		case Game::E_GameflowMessage_RaceStarted:
		{
			m_bEnableCollisions = true;

		}
		break;

		case GameMessaging::E_GameMsg_Update:
		{

			PERF_EVENT("Audio.DO_UpdateMessage");

			if(!hCurrentEntity.IsValid())
				return true;

			if (Audio::InterfaceController::Get().GetCallbackTriggeredCount(m_hQueryEntityFinished, true))
			{
				//printf("0x%x entity finished callback received\n", hCurrentEntity.ArrayHandle());
				SetCallbackState(hCurrentEntity, eCallbackReceived);
			}

			Maths::HMat43  camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
			Maths::HVec3  velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();

			//for splitscreen, need to check ditance from nearest listener (viewport)
				if(g_usingMultipleListeners)
				{

					m_isSplitScreen = false;

					if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 1)
					{
						m_isSplitScreen = true;

					}

					if(m_isSplitScreen)//max 4 views
					{
						int currentView = 0;
						//set closest view
						
						Maths::HF32 nearestDistance = Maths::Constants<Maths::HF32>::Max(); //big value
						
						for(int i = 0 ; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() ; i++)
						{

							Maths::HVec3 entityDistance = ((Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43().AW())  - (info.pActor->GetMatrix().AW()));
							F32 length = Maths::Length2(entityDistance);
							
							if(length < nearestDistance)
							{
								nearestDistance = length;
								currentView = i;
							}
						}
						camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Mat43();
						velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Live().Velocity();

					}
				}

			//if it exists and is set
			if(g_DOCullDistance)
			{

				const Maths::HF32 cullDistSq((F32)g_DOCullDistance * (F32)g_DOCullDistance);
				//get object distance to listener
				Maths::HVec3 entityDistance = ((camMatrix.AW())  - (info.pActor->GetMatrix().AW()));

				Maths::HF32 length = Maths::Length2(entityDistance);

				// check if entity is within distance
				if(gbDeleteIfOutOfRange && length > g_DOCullDistance)
				{
					Audio::InterfaceController::Get().SuspendEntity(hCurrentEntity);
					SetCallbackState(hCurrentEntity, eCallbackReceived);
					m_MarkForDelete = true;		
				}

				#ifndef CFG_GOLD
				C_DynamicEntityList::Get().SetDistance(const_cast<AudioEntityHandle&>(hCurrentEntity), length);
				#endif
			}

			//if for some reason the physics hasn't sent this object to sleep, kill it
			if(C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - m_TimeTriggered > g_DO_lifetime)
			{
				m_MarkForDelete = true;
				SetCallbackState(hCurrentEntity, eCallbackReceived);
			}

			if (m_MarkForDelete && m_eCallbackState == eCallbackReceived)
			{
					info.pHandler->RemoveSubscriber( GameMessaging::E_GameMsg_Update );
					info.pHandler->RemoveSubscriber( Game::E_GameLogicMessage_Impact );

					info.pHandler->RemoveSubscriber( Game::E_GameMsg_PhysicalObjectImpact );

					info.pHandler->RemoveSubscriber( ActorMsg::E_DynamicObjMsg_DynamicObjectApplyImpulse );
					
					m_pCom->ReleaseEntityInstance();
					//m_pCom = NULL;
					m_MarkForDelete = false;
					SetCallbackState(hCurrentEntity, eCallbackNotSet);
					break;
			}

			bool bDoDynamicObjCollision = m_DynamicObjectCollideParams[0].m_bCollisionThisFrame;

			if(bDoDynamicObjCollision)
			{
				PERF_EVENT("FirstFrame");
				//if(m_DynamicObjectCollideParams[0].dynamicObjectActorHandle == info.pActor->GetHandle())
				{
					F32 force = m_DynamicObjectCollideParams[0].m_fMaxImpulseStrength;
					//Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity,m_aHandle[ObjectPosition],(F32*)& m_DynamicObjectCollideParams[0].m_Position);
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[CollisionForce],force);
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[CollidesWith],(F32)m_DynamicObjectCollideParams[0].m_CollidesWith);
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[OcclusionValue],(F32)m_DynamicObjectCollideParams[0].m_occlusionValue);
					
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[PhysicalObjectImpactForce], force);	
					Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[PhysicalObjectHit]);	
					Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[DynamicObjectHit]);	

					SetCallbackState(hCurrentEntity, eWaitingForCallback);

					//reset the time for DO culling 
					m_TimeTriggered = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();

					m_MarkForDelete = false;
					
				}
			 
			 		m_DynamicObjectCollideParams[0].m_bCollisionThisFrame =  false;
					m_DynamicObjectCollideParams[0].m_fMaxImpulseStrength = 0.f;				
			}

			m_DynamicObjectCollideParams[0].m_fDecayingPeakImpulse *= gfDynamicObjCollisionPeakDecayRatioNew;

			return WorldObjectMessageHandler::ProcessMessage(info);
			
			break;
		}
				
		case ActorMsg::E_AudioMsg_TriggerActionFromHandle:
		{
			const ActorMsg::C_AudioTriggerActionFromHandleMsg* msg = Messaging::MessageCast<ActorMsg::C_AudioTriggerActionFromHandleMsg>(info.pMessage);
			if (msg->m_uEntityDataMemberHnd != 0xffffffff)
				Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, msg->m_uEntityDataMemberHnd);
			break;
		}

		case Game::E_GameMsg_PhysicalObjectImpact:
		{

			if(!m_bEnableCollisions)
						return true;

			const Game::C_PhysicalObjectImpactMsg* msg = Messaging::MessageCast<Game::C_PhysicalObjectImpactMsg>(info.pMessage);
				
				switch( msg->m_ImpactType )
				{
					default:
					break;
										
					case Game::E_PhysicalObjectImpactType_Break:
					{
						if(hCurrentEntity.IsValid())
						{
							static F32 scaler = 5;
							Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[PhysicalObjectImpactForce], msg->m_ImpactSize * scaler);	
							Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[PhysicalObjectBreak]);
							
							SetCallbackState(hCurrentEntity, eWaitingForCallback);	

							//reset the time for DO culling 
							m_TimeTriggered = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
						}							
					}
					break;
				}
		}
		break;

		case Actor::E_ActorMsg_PhysicsBodyEvent:
		{

				if(!m_bEnableCollisions)
					return true;

				const C_PhysicsBodyEventMsg* msg = Messaging::MessageCast<C_PhysicsBodyEventMsg>(info.pMessage);
				
				switch( msg->m_BodyEvent.GetEventType() )
				{
					default:
					break;
				

					case Gravity::C_BodyEvent::E_WakeUp:
						{							
							if(!hCurrentEntity.IsValid())
							{
								//don't create objects that are more than the cull distance away
								F32 distanceThreshold = g_DOCullDistance;

								Maths::HMat43  camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
								Maths::HVec3  velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();

								//for splitscreen, need to check ditance from nearest listener (viewport)
								if(g_usingMultipleListeners)
								{
									m_isSplitScreen = false;
									if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 1)
									{
										m_isSplitScreen = true;
									}

									if(m_isSplitScreen)//max 4 views
									{
										int currentView = 0;
										//set closest view
										
										Maths::HF32 nearestDistance = Maths::Constants<Maths::HF32>::Max(); //big value
										
										for(int i = 0 ; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() ; i++)
										{
											Maths::HVec3 entityDistance = ((Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43().AW())  - (info.pActor->GetMatrix().AW()));
											Maths::HF32 length = Maths::Length2(entityDistance);
											
											if(length < nearestDistance)
											{
												nearestDistance = length;
												currentView = i;
											}
										}

										//set the listener based on closed view
										camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Mat43();
										velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Live().Velocity();
									}
								}
								//if it exists and is set
								if(distanceThreshold)
								{

									//get object distance to listener
									const Maths::HF32 disThreshSq(distanceThreshold * distanceThreshold);
									Maths::HVec3 entityDistance = ((camMatrix.AW())  - (info.pActor->GetMatrix().AW()));

									F32 length = Maths::Length2(entityDistance);

									// check if entity is within distance
									if(length < disThreshSq)
									{
										m_pCom->CreateEntityInstance();		
										info.pHandler->AddSubscriber( GameMessaging::E_GameMsg_Update );
										info.pHandler->AddSubscriber( Game::E_GameLogicMessage_Impact );

										info.pHandler->AddSubscriber( Game::E_GameMsg_PhysicalObjectImpact );

										info.pHandler->AddSubscriber( ActorMsg::E_DynamicObjMsg_DynamicObjectApplyImpulse );


										if( m_aHandle[EntityFinished] != -1)
											m_hQueryEntityFinished= Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hCurrentEntity), m_aHandle[EntityFinished]);
										
										SetCallbackState(hCurrentEntity, eWaitingForCallback);

										Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[StartActionIn],false);

										//set the time so we can delete it if it doesn't get culled
										m_TimeTriggered = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();

									}								
								}

								if(hCurrentEntity.IsValid())
								Audio::InterfaceController::Get().ResumeEntity(hCurrentEntity);
							}
						}

						
					break;

					case Gravity::C_BodyEvent::E_GoToSleep:	
						{
							if(hCurrentEntity.IsValid())
								m_MarkForDelete = true;		
						}
					break;
				}

				break;
		}

		case ActorMsg::E_DynamicObjMsg_DynamicObjectApplyImpulse:
			{

				PERF_EVENT("Audio.DynamicObjectApplyImpulse");
					const ActorMsg::C_DynamicObjectApplyImpulseMsg* msg = Messaging::MessageCast<ActorMsg::C_DynamicObjectApplyImpulseMsg>(info.pMessage);

							if(!hCurrentEntity.IsValid())
							{
								//don't create objects that are more than 350 meteres away
								const Maths::HF32 distanceThreshold2 = Maths::HF32(((F32)g_DOCullDistance) * ((F32)g_DOCullDistance));

								Maths::HMat43  camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
								Maths::HVec3  velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();

							//for splitscreen, need to check ditance from nearest listener (viewport)
								if(g_usingMultipleListeners)
								{
									m_isSplitScreen = false;

									if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 1)
										m_isSplitScreen = true;

									if(m_isSplitScreen)//max 4 views
									{
										int currentView = 0;
										//set closest view										
										Maths::HF32 nearestDistance = Maths::Constants<Maths::HF32>::Max(); //big value
										
										for(int i = 0 ; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() ; i++)
										{
											Maths::HVec3 entityDistance = ((Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43().AW())  - (info.pActor->GetMatrix().AW()));
											const Maths::HF32 length = Maths::Length2(entityDistance);
											
											if(length < nearestDistance)
											{
												nearestDistance = length;
												currentView = i;
											}
										}
										camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Mat43();
										velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Live().Velocity();
									}
								}
								//if it exists and is set
								if(distanceThreshold2)
								{
									//get object distance to listener
									Maths::HVec3 entityDistance = ((camMatrix.AW())  - (info.pActor->GetMatrix().AW()));

									F32 length2 = Maths::Length2(entityDistance);
									// check if entity is within distance
									if(length2 < distanceThreshold2)
									{											
										m_pCom->CreateEntityInstance();		
										info.pHandler->AddSubscriber( GameMessaging::E_GameMsg_Update );
										info.pHandler->AddSubscriber( Game::E_GameLogicMessage_Impact );
										info.pHandler->AddSubscriber( Game::E_GameMsg_PhysicalObjectImpact );
									}
								}

								if(hCurrentEntity.IsValid())
								{
									if( m_aHandle[EntityFinished] != -1)
										m_hQueryEntityFinished= Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hCurrentEntity), m_aHandle[EntityFinished]);
									
									SetCallbackState(hCurrentEntity, eWaitingForCallback);

									//reset the time for DO culling 
									m_TimeTriggered = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();

									Audio::InterfaceController::Get().ResumeEntity(hCurrentEntity);

									Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ObjectPosition], (F32*)(&info.pActor->GetMatrix().AW()));
									Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ObjectDirection], (F32*)(&info.pActor->GetMatrix().AZ()));
									Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[ObjectUp], (F32*)(&info.pActor->GetMatrix().AY()));

									F32 force = msg->m_Force.Length();
									Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[HitByPerkForce],force);

									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[HitByPerk],false);


								}
							}
			}
			break;
		
		case Game::E_GameLogicMessage_Impact:
			{
				PERF_EVENT("Audio.ImpactMessage");

				const Game::C_ImpactMsg* p_pMsg		= Messaging::MessageCast<Game::C_ImpactMsg>(info.pMessage);
				Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

				const Actor::C_Actor* actorA = pRLC->GetActorList()->GetActorPtr(p_pMsg->mActorHandleA);
				const Actor::C_Actor* actorB = pRLC->GetActorList()->GetActorPtr(p_pMsg->mActorHandleB);


				AUDIO_COLLISION_COLLIDES_WITH otherActor = AUDIO_COLLISION_COLLIDES_WITH_WORLD;  // 2 is world collision

				if(actorA  && (actorA->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject || actorA->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject))
				{

					if(info.pActor->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)
					{
						::Actors::DynamicObjectActor *pActor = (::Actors::DynamicObjectActor *)Actor::ActorCast< ::Actors::DynamicObjectActor >(info.pActor);					
						
						if(pActor != actorA)
							return true;					
					}
					else
					if(info.pActor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
					{
						::Actors::C_PhysicalObject *pActor = (::Actors::C_PhysicalObject *)Actor::ActorCast< ::Actors::C_PhysicalObject >(info.pActor);
				
						if(pActor != actorA)
							return true;
					}

					if(actorB)
					{
						if(actorB->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)
						{
								otherActor = AUDIO_COLLISION_COLLIDES_WITH_VEHICLE;
								//set occlusion value
								Game::Actors::C_VehicleActor *pVehActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(actorB);
								m_OcclusionValue= pVehActor->GetAudioOcclusionValue();	
						}
						else
							if(actorB->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject || actorB->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
								otherActor = AUDIO_COLLISION_COLLIDES_WITH_OTHER;
					}

					DynamicCollision(p_pMsg, p_pMsg->mActorHandleA,otherActor,m_OcclusionValue);


				}

				if(actorB  && (actorB->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject || actorB->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject))
				{

					if(info.pActor->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)
					{
						::Actors::DynamicObjectActor *pActor = (::Actors::DynamicObjectActor *)Actor::ActorCast< ::Actors::DynamicObjectActor >(info.pActor);
						
						if(pActor != actorB)
							return true;
					}
					else
					if(info.pActor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
					{
						::Actors::C_PhysicalObject *pActor = (::Actors::C_PhysicalObject *)Actor::ActorCast< ::Actors::C_PhysicalObject >(info.pActor);


						if(pActor != actorB)
							return true;
					}

					if(actorA)
					{
						if(actorA->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)
						{
								otherActor = AUDIO_COLLISION_COLLIDES_WITH_VEHICLE;
								//set occlusion value
								Game::Actors::C_VehicleActor *pVehActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(actorA);
								m_OcclusionValue = pVehActor->GetAudioOcclusionValue();


						}
						else
							if(actorA->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject || actorA->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
								otherActor = AUDIO_COLLISION_COLLIDES_WITH_OTHER;
					}

					DynamicCollision(p_pMsg, p_pMsg->mActorHandleB,otherActor,m_OcclusionValue);	

				}
				break;
			}


		//default:
			//return WorldObjectMessageHandler::ProcessMessage(info);
	}

	//switch(info.pMessage->Type())
	{
		//default:
		//	return WorldObjectMessageHandler::ProcessMessage(info);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// C_AudioWheelComponent
//////////////////////////////////////////////////////////////////////////////

static const char* g_AudioWheelComponet_DataMemberNames [] =
{
#define AUDIO_WHEEL_OBJECT_MESSAGE(x) #x
	AUDIO_WHEEL_OBJECT_MESSAGES
#undef AUDIO_WHEEL_OBJECT_MESSAGE
};

#define AUDIO_WHEEL_ACTIONOUTS \
	AUDIO_MESSAGE( FLWheelControllerNone ), \
	AUDIO_MESSAGE( FRWheelControllerNone ), \
	AUDIO_MESSAGE( RLWheelControllerNone ), \
	AUDIO_MESSAGE( RRWheelControllerNone ), \
	AUDIO_MESSAGE( FLWheel3DMixerNone ), \
	AUDIO_MESSAGE( FRWheel3DMixerNone ), \
	AUDIO_MESSAGE( RLWheel3DMixerNone ), \
	AUDIO_MESSAGE( RRWheel3DMixerNone ), \
	AUDIO_MESSAGE( FLWheelDry_AudioStopped ), \
	AUDIO_MESSAGE( FRWheelDry_AudioStopped ), \
	AUDIO_MESSAGE( RLWheelDry_AudioStopped ), \
	AUDIO_MESSAGE( RRWheelDry_AudioStopped ), \
	AUDIO_MESSAGE( FLWheelCommonNone ), \
	AUDIO_MESSAGE( FRWheelCommonNone ), \
	AUDIO_MESSAGE( RLWheelCommonNone ), \
	AUDIO_MESSAGE( RRWheelCommonNone ), \
	AUDIO_MESSAGE( FLWheelOffRoad_AudioStopped ), \
	AUDIO_MESSAGE( FRWheelOffRoad_AudioStopped ), \
	AUDIO_MESSAGE( RLWheelOffRoad_AudioStopped ), \
	AUDIO_MESSAGE( RRWheelOffRoad_AudioStopped ), \
	AUDIO_MESSAGE( AIWheelOffRoad_AudioStopped ), \
	AUDIO_MESSAGE( AIWheelDry_AudioStopped ),\
	AUDIO_MESSAGE( AIWheelControllerNone ), \
	AUDIO_MESSAGE( AIWheel3DMixerNone ), \
	AUDIO_MESSAGE( AIWheelCommonNone )


#define AUDIO_WHEEL_INSTANCES \
	AUDIO_IMESSAGE( FLWheelController ), \
	AUDIO_IMESSAGE( FRWheelController ), \
	AUDIO_IMESSAGE( RLWheelController ), \
	AUDIO_IMESSAGE( RRWheelController ), \
	AUDIO_IMESSAGE( FLWheel3DMixer ), \
	AUDIO_IMESSAGE( FRWheel3DMixer ), \
	AUDIO_IMESSAGE( RLWheel3DMixer ), \
	AUDIO_IMESSAGE( RRWheel3DMixer ), \
	AUDIO_IMESSAGE( FLWheelDry ), \
	AUDIO_IMESSAGE( FRWheelDry ), \
	AUDIO_IMESSAGE( RLWheelDry ), \
	AUDIO_IMESSAGE( RRWheelDry ), \
	AUDIO_IMESSAGE( FLWheelCommon  ), \
	AUDIO_IMESSAGE( FRWheelCommon  ), \
	AUDIO_IMESSAGE( RLWheelCommon  ), \
	AUDIO_IMESSAGE( RRWheelCommon  ), \
	AUDIO_IMESSAGE( FLWheelOffroad ), \
	AUDIO_IMESSAGE( FRWheelOffroad ), \
	AUDIO_IMESSAGE( RLWheelOffroad ), \
	AUDIO_IMESSAGE( RRWheelOffroad ), \
	AUDIO_IMESSAGE( AIWheelOffroad ), \
	AUDIO_IMESSAGE( AIWheelDry ), \
	AUDIO_IMESSAGE( AIWheelController ), \
	AUDIO_IMESSAGE( AIWheel3DMixer ), \
	AUDIO_IMESSAGE( AIWheelCommon )



class C_WheelSuspenderResumer : public C_InstanceSuspenderResumer
{
public:
	#undef AUDIO_MESSAGE
	#define AUDIO_MESSAGE( x ) x
	enum ActionOutId
	{
		AUDIO_WHEEL_ACTIONOUTS,
		NumActionOutIds
	};

	#undef AUDIO_IMESSAGE
	#define AUDIO_IMESSAGE( x ) x
	enum InstanceId
	{
		AUDIO_WHEEL_INSTANCES,
		NumInstanceIds
	};

	char* GetInstanceString( U32 uId ) const
	{
		#undef AUDIO_IMESSAGE
		#define AUDIO_IMESSAGE( x ) #x
		static char* strMessages[] =
		{
			AUDIO_WHEEL_INSTANCES
		}; 

		StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumInstanceIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

		BIZ_ASSERT( uId < NumInstanceIds );

		return strMessages[ uId ];
	}

	char* GetActionOutString( U32 uId ) const
	{
		#undef AUDIO_MESSAGE
		#define AUDIO_MESSAGE( x ) #x
		static char* strMessages[] =
		{
			AUDIO_WHEEL_ACTIONOUTS
		}; 

		StaticAssert( lengthof(strMessages) == NumActionOutIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );
		StaticAssert( lengthof(strMessages) == NumInstanceIds, NUM_ACTIONOUT_STRINGS_DONT_MATCH_NUM_INSTANCES );

		BIZ_ASSERT( uId < NumActionOutIds );

		return strMessages[ uId ];
	}

	C_WheelSuspenderResumer(const I_AudioEntityDataSource* pDataSource)
		: C_InstanceSuspenderResumer(pDataSource)
	{
	}

	~C_WheelSuspenderResumer() {}

	//bool Suspend(const Audio::EntityDataMemberHandle hCom)
	//{
	//	if (hCom == uSuspendAll)
	//	{
	//		SuspendAll();
	//		return true;
	//	}
	//
	//	return false;
	//}

	bool Resume(const Audio::EntityDataMemberHandle hComponent)
	{
		U32 uComId = LookupComponentId(hComponent);

		if (uComId == I_AudioEntityDataSource::uInvalidDataMemberId)
			return false;

		// TODO: better way to do this?
		//if (uComId == C_AudioWheelComponent::E_DataMember_StateChanged)
		//	return true;

		//ResumeInstance(Listener);
		//ResumeInstance(WheelObject);

		ResumeInstance(FLWheelController );  
		ResumeInstance(FRWheelController );  
		ResumeInstance(RLWheelController );  
		ResumeInstance(RRWheelController  ); 
		ResumeInstance(FLWheel3DMixer );
		ResumeInstance(FRWheel3DMixer ); 
		ResumeInstance(RLWheel3DMixer ); 
		ResumeInstance(RRWheel3DMixer ); 
		ResumeInstance(FLWheelCommon  ); 
		ResumeInstance(FRWheelCommon  ); 
		ResumeInstance(RLWheelCommon  ); 
		ResumeInstance(RRWheelCommon  ); 
		ResumeInstance(FLWheelDry  ); 
		ResumeInstance(FRWheelDry  ); 
		ResumeInstance(RLWheelDry  ); 
		ResumeInstance(RRWheelDry  ); 


		//if (IsPlayer())
		//{
		//	ResumeInstance(MovementClothes);
		//	ResumeInstance(VO_NonVerbal_General);
		//}

		switch (uComId)
		{

		case Components::WheelMessageHandler::FLWheelDry_AudioResumed:				
			ResumeInstance(FLWheelDry);
			break;

		case Components::WheelMessageHandler::FRWheelDry_AudioResumed:				
			ResumeInstance(FRWheelDry);
			break;

		case Components::WheelMessageHandler::RLWheelDry_AudioResumed:				
			ResumeInstance(RLWheelDry);
			break;

		case Components::WheelMessageHandler::RRWheelDry_AudioResumed:				
			ResumeInstance(RRWheelDry);
			break;


		case Components::WheelMessageHandler::FLWheelOffRoad_AudioResumed:				
			ResumeInstance(FLWheelOffroad);
			break;

		case Components::WheelMessageHandler::FRWheelOffRoad_AudioResumed:				
			ResumeInstance(FRWheelOffroad);
			break;

		case Components::WheelMessageHandler::RLWheelOffRoad_AudioResumed:				
			ResumeInstance(RLWheelOffroad);
			break;

		case Components::WheelMessageHandler::RRWheelOffRoad_AudioResumed:				
			ResumeInstance(RRWheelOffroad);
			break;


		default:
			DebugBreak();
		//	LOGWARN(GameAudio::g_Log, "ResumeInstance() : component %d '%s' not dealt with", uComId, GetDataSource()->GetDataMemberName(uComId));
			return false;
		}

		return true;

		/*U32 uInstanceId = GetInstanceId(uComId);

		if (uInstanceId == uInvalidInstanceId)
		{
			LOGWARN(GameAudio::g_Log, "ResumeInstance() : could not lookup instance for component %d '%s'", uComId, GetDataSource()->GetDataMemberName(uComId));
			return false;
		}

		ResumeInstance(uInstanceId);

		return true;*/
	}

	void SuspendInstance(U32 uInstanceId)
	{
		if (!g_bAudioSuspendEntities)
			return;
			
		// temp for emma. never suspend movement clothes and voicecs instance for player.
		//if (IsPlayer() && (uInstanceId == MovementClothes || uInstanceId == VO_NonVerbal_General))
			//return;

		C_InstanceSuspenderResumer::SuspendInstance(uInstanceId);

	}

	//bool IsPlayer() { return (GetInstanceHandle(Movement_Shimmying) == I_AudioEntityDataSource::uInvalidDataMemberHandle) ? false : true; }

protected:
	U32 GetNumInstances() const { return NumInstanceIds; }
};

C_SuspendResumeManagerBase* Components::WheelMessageHandler::CreateSuspendResumeManager() const
{
	return new C_WheelSuspenderResumer(this);
}

U32 Components::WheelMessageHandler::GetNumDataMembers() const
{
	return RRWheelOffRoad_AudioResumed;

	//return 0;
}

const char* Components::WheelMessageHandler::GetDataMemberName(const U32 index) const
{
	//if ((index > E_DataMember_Prev) && (index < E_DataMember_Count))
	//{
	//	return g_AudioCharacterComponet_DataMemberNames [index - (E_DataMember_Prev + 1)];
	//}

	//return C_AudioBaseComponent::GetDataMemberName(index);
	return NULL;
}

Audio::EntityDataMemberHandle Components::WheelMessageHandler::GetDataMemberHandle(const U32 id) const
{ 
	return (id < static_cast<U32>(m_aHandle.Size())) ? m_aHandle[ id ] : uInvalidDataMemberHandle; 
	//return -1;
}

bool Components::WheelMessageHandler::TriggerAction(const Audio::EntityDataMemberHandle hEntityMember, const bool stopOnFail) const
{
	bool bOk = true;

	if (hEntityMember == uInvalidDataMemberHandle)
		bOk = false;
	else
	{
		if (m_pManager != NULL)
		{
			if (!m_pManager->IsCallbackSet())
				bOk = false;

			if (!m_pManager->Resume(hEntityMember))
				bOk = false;
		}

		if (!Audio::InterfaceController::Get().TriggerEntityActionIn(m_pCom->GetEntityHnd(), hEntityMember, false))
			bOk = false;
	}

	////// temp fix for character entities getting suspended. only send stopped msg for EntitySuspenderResumer 
	////// TODO: need a better way to do this
	////if (!m_pManager || (m_pManager && m_pManager->Type() == E_EntitySuspenderResumer))
	////{
	////	if (stopOnFail && !bOk)
	////		SendMessageToActor(ActorMsg::E_AudioMsg_EntityAudioStopped);
	////}

	return bOk;
}



//**** wheel message handler******************************
//	WheelMessageHandler
Components::WheelMessageHandler::WheelMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom   ) : WorldObjectMessageHandler( hEntityDefinition, pCom ),	I_AudioEntityDataSource()
{
	m_pCom = pCom;

	m_PrevFLAttached = true;
	m_PrevFRAttached = true;
	m_PrevRLAttached = true;
	m_PrevRRAttached = true;

	m_FLSurfaceType_Prev = -1;
	m_FRSurfaceType_Prev = -1;
	m_RLSurfaceType_Prev = -1;
	m_RRSurfaceType_Prev = -1;

	m_prevFLIsOnGround = -1.f;
	m_prevFRIsOnGround = -1.f;
	m_prevRLIsOnGround = -1.f;
	m_prevRRIsOnGround = -1.f;

	m_prevFLWheelHitModifier = -1.f;
	m_prevFRWheelHitModifier = -1.f;
	m_prevRLWheelHitModifier = -1.f;
	m_prevRRWheelHitModifier = -1.f;

	m_prevFLWheelDirection = Maths::HVec3(Maths::UNDEFINED);
	m_prevFRWheelDirection = Maths::HVec3(Maths::UNDEFINED);
	m_prevRLWheelDirection = Maths::HVec3(Maths::UNDEFINED);
	m_prevRRWheelDirection = Maths::HVec3(Maths::UNDEFINED);

	m_prevBrake = -1.f;
	m_prevHandBrake = -1.f;

	m_prevFocusCarHeight = FLT_MAX;

	for (U32 uId = WorldObjectMessageHandler::NumMessageIds; uId < NumMessageIds; uId++)
	{
		Audio::EntityDataMemberHandle memberHandle = -1;
		if(uId != FINAL_REQUISITE_EXTERNAL)
		{
			memberHandle = Audio::InterfaceController::Get().LookupInstanceComponent( hEntityDefinition, WheelMessageHandler::GetString( uId ) );
		}
			m_aHandle.PushBack( memberHandle );

		#ifndef CFG_GOLD
		//check for required params
		if(memberHandle == -1 && uId < FINAL_REQUISITE_EXTERNAL)
		{
			char name[512]; // [LloydW] This was running over the end of the array :-(
			strcpy_s(name,sizeof(name),WheelMessageHandler::GetString( uId ));
			strcat_s(name,sizeof(name),": Required Audio External is Missing. Check audio data or let audio coder know this is optional");
			Audio::InterfaceController::Get().Error(name);
			DebugBreak();
		}
		#endif	
		

	}

//	m_pManager = CreateSuspendResumeManager();
	m_hEntityDefinition = hEntityDefinition;


}

bool Components::WheelMessageHandler::ProcessMessage(BaseMessageHandlerInfo& info)
{

	if(!m_pCom->GetEntityHnd().IsValid())
			return true;

	const AudioEntityHandle& hCurrentEntity = m_pCom->GetEntityHnd();

	switch(info.pMessage->Type())
	{
			case Actor::E_ActorMsg_ComponentCreated:
			{

				//set messageSubscribers
				info.pHandler->AddSubscriber(Game::E_VehicleMessage_LeftRace);
				info.pHandler->AddSubscriber(ActorMsg::E_AudioMsg_ConnectEntity);

				info.pHandler->AddSubscriber(E_VehicleLogicMsg_FiredPerk);

			//	info.pHandler->AddSubscriber(ActorMsg::E_AudioMsg_InstanceAudioStopped);
			
				F32 FLWheelOffset = 0.0;

				if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 1)
				{
					Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(info.pActor);
					if(pActor)
					{
						const int vehicleIndex = pActor->GetVehicleIndex();
						if(vehicleIndex>0)
							FLWheelOffset = (F32)vehicleIndex / (F32)Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();

					}

				}

				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelStartOffset], FLWheelOffset);		 
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelStartOffset], 0.4);		 
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelStartOffset], 0.6);		 
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelStartOffset], 0.8);

			//	Audio::InterfaceController::Get().SuspendEntity();
					
				m_isPendingSuspend = false;
				m_isSplitScreen = false;

				if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 1)
				{
					m_isSplitScreen = true;

				}

				for(int i = 0;i<MAX_NUMBER_AUDIO_CARS;i++)
				{
					m_NewWheelDistanceScale[i].distanceFromListener = 1000000;
					m_NewWheelDistanceScale[i].CarID = -1;
					m_NewWheelDistanceScale[i].volScale = 0;
					m_NewWheelDistanceScale[i].shouldUpdate = true;
					m_NewWheelDistanceScale[i].isFocusCar = false;
					m_NewWheelDistanceScale[i].listenerViewPort = 0;
					m_NewWheelDistanceScale[i].isMuted = false;

				}

				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelSurfaceType], -1);
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelSurfaceType], -1);
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelSurfaceType], -1);
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelSurfaceType], -1);

				//esnuse vol is at max for all cars going into level
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelPriorityVolume], 1.0f);
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelPriorityVolume], 1.0f);
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelPriorityVolume], 1.0f);
				Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelPriorityVolume], 1.0f);	

				SuspendControllerInstances(hCurrentEntity,true);
				m_isSuspended = true;
				m_prevFLSurface = E_Wheel_Surface_Unknown;
				m_prevFRSurface = E_Wheel_Surface_Unknown;
				m_prevRLSurface = E_Wheel_Surface_Unknown;
				m_prevRRSurface = E_Wheel_Surface_Unknown;

				m_FLPendingSuspend = false;
				m_FRPendingSuspend = false;
				m_RLPendingSuspend = false;
				m_RRPendingSuspend = false;

				m_prevAISurface = E_Wheel_Surface_Unknown;
				m_AIPendingSuspend = false;
			
			}
			break;

			case ActorMsg::E_AudioMsg_ConnectEntity:
			{

				//info.msg.entity
				const ActorMsg::C_AudioConnectEntityMsg* pMsg = Messaging::MessageCast<ActorMsg::C_AudioConnectEntityMsg>(info.pMessage);

				Audio::EntityDefinitionHandle hDef = Audio::InterfaceController::Get().GetDefinitionHandle(pMsg->m_EntityInstanceHandle);

				Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(m_pCom->GetEntityDefHnd());
			
				Audio::LocalFunctionHandle hFunc = Audio::InterfaceController::Get().LookupParameterisedFunctionById(hDef, "DoConnections", pDef->GetId());
				// only try to connect if "DoConnections" func exists
				if (hFunc != 0xffffffff)
				{
					Audio::InterfaceController::Get().RunFunctionInParameterisedEntityInstance(pMsg->m_EntityInstanceHandle, hCurrentEntity  , hFunc);
				} 
			}
			break;

			case	E_VehicleLogicMsg_FiredPerk:
			{
				//Get the vehicleIndex
				const C_VehicleLogicMsg_FiredPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_FiredPerkMsg >(info.pMessage);

				U8 vehicleIndex = msg->mVehicleIndex;
				enum E_PICKUP_TYPE ePickupType = msg->mPickupType;

				F32 pickupTier  = (F32)msg->mLevel;

				Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(info.pActor);

				if(vehicleIndex == pActor->GetVehicleIndex())
				{
					switch(ePickupType)
					{
						case E_PICKUP_TYPE_NITROUS:
							Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[FLWheelNitrousInitiated],false);
							Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[FRWheelNitrousInitiated],false);
							Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[RLWheelNitrousInitiated],false);
							Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[RRWheelNitrousInitiated],false);
						break;
					}
				}
			}


		//	case GameMessaging::E_GameMsg_Update:
			case Game::E_GameflowMessage_UpdateDeferredAudio:
				{	
					PERF_EVENT("Audio.WheelMessageHandler");

					

					if (info.pActor->GetActorTypeId() != Game::E_ActorTypeId_Vehicle)
						return true;

					Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(info.pActor);

					class RacingPhysics::Car *pCar = pActor->GetVehiclePhysics();

					if(!pCar)
						return false;

					RacingPhysics::Vector3R pvelocity = pCar->GetVelocity();

					const int vehicleIndex = pActor->GetVehicleIndex();
					
						//////update object orientation 
					Maths::HMat43 objectMatrix = pActor->GetMatrix();

					
					//AudioEntityHandle handle = info.hEntity;
					AudioEntityHandle handle = hCurrentEntity;

					RacingPhysics::Vector3R  objectVelocity = pvelocity;
					Maths::HMat43 camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
					Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();

					int audibleCars = AUDIBLE_CARS;

					F32 nearestDistance = 1000000; //big value

					


					//sort out viewports, get correct listener for view
					if(m_isSplitScreen )//max 4 views
					{
						audibleCars = AUDIBLE_CARS_SPLITSCREEN;

						if((vehicleIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))
						{
							camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Mat43();
							velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Live().Velocity();

						}
						else
						{
							//ai cars need to be assigned to a listener, for this frame, give this vehicle to closest listener
							//get closest listener
							int currentView = 0;
							//set closest view
							
														
							for(int i = 0 ; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() ; i++)
							{

								Maths::HVec3 entityDistance = ((Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43().AW())  - (info.pActor->GetMatrix().AW()));
								F32 length = Maths::Length(entityDistance);
								
								if(length < nearestDistance)
								{
									nearestDistance = length;
									currentView = i;
								}

							}

							camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Mat43();
							velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Live().Velocity();

						}
						
					}

					Maths::HVec3 camPos = camMatrix.AW();
					Maths::HVec3 carPos = objectMatrix.AW();
						
					U32 numVehicles = 0;
					U32 numViewports = 1;
					U32 removedVehicles = 0;
					bool isFocusVehicle = false;
					bool finalVehicleHasFocus = false;

					Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
					if(pRLC)
					{
						numVehicles = pRLC->GetNumVehicles();

						numViewports = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
						
						for(int i = 0; i < (int)numViewports; i++)
						{
							if(vehicleIndex == pRLC->GetViewportFocusVehicleIndex(i))
							{
								isFocusVehicle = true;
							 
								//for distance algorithim, need to know if final vehicle has focus
								if(vehicleIndex == numVehicles-1)
									finalVehicleHasFocus = true;
							}
						}

					}

					Maths::HMat43 vehicleMatrix;
					Maths::HVec3 vehicleDirection;
					Maths::HVec3 soundDir;

					bool bSuspended = false;

					//Perf::BeginNamedEvent("Audio.DS.DistanceScalingTotal");

					//get the distance scale based on the car's distance which was calculated last frame
					if(gbUseCarScaleForWheels)
					{
							//only do for non-focus, audible cars
							if((int)numVehicles > audibleCars && bDebugUsingDistanceScaling && !isFocusVehicle)
							{
								
									bool shouldUpdate = false;
									S32 arrayIndex = -1;

									//check to see if current index is within in the audible car Slots
									for(int i = 0;i < audibleCars;i++)
									{
										//if this car matches an audible car ID
										if(vehicleIndex == m_LockedCarDistanceScale[i].CarID)
										{
											//store the index of the car
											shouldUpdate = true;
											arrayIndex = i;

											break;

										}
									}

									if(shouldUpdate)
									{
										//audible car, resume and apply volScale

										//only if previously suspended...this is the huge perf optimisation
										if(m_isSuspended)
										{
											ResumeControllerInstances(hCurrentEntity);
											m_isSuspended = false;

											//send a dummy surface so it won't suspend when it wakes up
											m_FLSurfaceType_Prev = -1;
											m_FRSurfaceType_Prev = -1;
											m_RLSurfaceType_Prev = -1;
											m_RRSurfaceType_Prev = -1;

											Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelSurfaceType], (float)-1);
											Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelSurfaceType], (float)-1);
											Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelSurfaceType], (float)-1);
											Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelSurfaceType], (float)-1);

										}

										//no need to set vol scale since wheel goes though car mixer
										//send the vol scale as per the carDistance array
										//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelPriorityVolume], m_LockedCarDistanceScale[arrayIndex].volScale);
										//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelPriorityVolume], m_LockedCarDistanceScale[arrayIndex].volScale);
										//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelPriorityVolume], m_LockedCarDistanceScale[arrayIndex].volScale);
										//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelPriorityVolume], m_LockedCarDistanceScale[arrayIndex].volScale);

									}
									else
									{
										//does not need to update
										if(!m_isSuspended)
										{

											m_FLSurfaceType_Prev = -1;
											m_FRSurfaceType_Prev = -1;
											m_RLSurfaceType_Prev = -1;
											m_RRSurfaceType_Prev = -1;

											Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelSurfaceType], (float)-1);
											Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelSurfaceType], (float)-1);
											Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelSurfaceType], (float)-1);
											Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelSurfaceType], (float)-1);

											//car is inaudible, suspend and bail
											//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelPriorityVolume], 0.f);
											//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelPriorityVolume], 0.f);
											//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelPriorityVolume], 0.f);
											//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelPriorityVolume], 0.f);

											SuspendControllerInstances(hCurrentEntity,true);
											m_isSuspended = true;
										}

										return true;


									}
								
							}
							else
							{
								//don't run through the algorithm because we're either a focus car or all cars are audible

								if(m_isSuspended)
								{
									ResumeControllerInstances(hCurrentEntity);
									m_isSuspended = false;

									m_FLSurfaceType_Prev = -1;
									m_FRSurfaceType_Prev = -1;
									m_RLSurfaceType_Prev = -1;
									m_RRSurfaceType_Prev = -1;

									Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelSurfaceType], (float)-1);
									Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelSurfaceType], (float)-1);
									Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelSurfaceType], (float)-1);
									Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelSurfaceType], (float)-1);
								}

/*								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelPriorityVolume], 1.0f);
								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelPriorityVolume], 1.0f);
								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelPriorityVolume], 1.0f);
								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelPriorityVolume], 1.0f);	*/	
							
							}

					
					}
					else	
					{
					#ifdef NEW_DISTANCE_SCALING
					//if(g_UsingNewDistanceScale)
					{

					//if there are enough vehicles to warrant distance scaling
					if((int)numVehicles > audibleCars && bDebugUsingDistanceScaling && !isFocusVehicle)
					{

							static AudioEntityHandle hZeroWheelEntity;

							if(m_isSuspended)
							{
								m_FLSurfaceType_Prev = -1;
								m_FRSurfaceType_Prev = -1;
								m_RLSurfaceType_Prev = -1;
								m_RRSurfaceType_Prev = -1;

								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelSurfaceType], (float)-1);
								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelSurfaceType], (float)-1);
								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelSurfaceType], (float)-1);
								Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelSurfaceType], (float)-1);
								//Audio::InterfaceController::Get().SuspendEntity(hCurrentEntity);
								SuspendControllerInstances(hCurrentEntity, true);
								m_isSuspended = false;

							}

							if(g_SuspendingZeroEntity)
							{
								if(hZeroWheelEntity.IsValid())
								{
									m_FLSurfaceType_Prev = -1;
									m_FRSurfaceType_Prev = -1;
									m_RLSurfaceType_Prev = -1;
									m_RRSurfaceType_Prev = -1;

									Audio::InterfaceController::Get().SetEntityValue(hZeroWheelEntity, m_aHandle[FLWheelSurfaceType], (float)-1);
									Audio::InterfaceController::Get().SetEntityValue(hZeroWheelEntity, m_aHandle[FRWheelSurfaceType], (float)-1);
									Audio::InterfaceController::Get().SetEntityValue(hZeroWheelEntity, m_aHandle[RLWheelSurfaceType], (float)-1);
									Audio::InterfaceController::Get().SetEntityValue(hZeroWheelEntity, m_aHandle[RRWheelSurfaceType], (float)-1);

									SuspendControllerInstances(hZeroWheelEntity, true);
									m_isSuspended = false;
									hZeroWheelEntity.Zero(); //reset


								}
							}

							F32 distanceFromListener = camPos.Distance(carPos);

							U32 nonFocusAudibleCars = audibleCars - numViewports; 

							for(int i = nonFocusAudibleCars;i >= 0;i--)
							{
								if(distanceFromListener < m_NewWheelDistanceScale[i].distanceFromListener)
								{
									F32 tempDistanceHold = m_NewWheelDistanceScale[i].distanceFromListener;

									m_NewWheelDistanceScale[i+1] = m_NewWheelDistanceScale[i];
		
									m_NewWheelDistanceScale[i].distanceFromListener = distanceFromListener;
									m_NewWheelDistanceScale[i].CarID = vehicleIndex;
									m_NewWheelDistanceScale[i].isFocusCar = isFocusVehicle;
									m_NewWheelDistanceScale[i].hEntityHandle = hCurrentEntity;
									m_NewWheelDistanceScale[i].hPriorityMemberHandle[0] = m_aHandle[FLWheelPriorityVolume];
									m_NewWheelDistanceScale[i].hPriorityMemberHandle[1] = m_aHandle[FRWheelPriorityVolume];
									m_NewWheelDistanceScale[i].hPriorityMemberHandle[2] = m_aHandle[RLWheelPriorityVolume];
									m_NewWheelDistanceScale[i].hPriorityMemberHandle[3] = m_aHandle[RRWheelPriorityVolume];

									//suspend the car that has dropped off the end of audible index threshold
									if(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle.IsValid())
									{
									//	Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[audibleCars].hEntityHandle, m_aHandle[PriorityVolume], 0.0f);

										Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle, m_NewWheelDistanceScale[nonFocusAudibleCars+1].hPriorityMemberHandle[0], 0.0f);
										Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle, m_NewWheelDistanceScale[nonFocusAudibleCars+1].hPriorityMemberHandle[1], 0.0f);
										Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle, m_NewWheelDistanceScale[nonFocusAudibleCars+1].hPriorityMemberHandle[2], 0.0f);
										Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle, m_NewWheelDistanceScale[nonFocusAudibleCars+1].hPriorityMemberHandle[3], 0.0f);

								//		Audio::InterfaceController::Get().SuspendEntity(m_NewWheelDistanceScale[audibleCars].hEntityHandle);
										SuspendControllerInstances(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle,true);
									}


								}
								else
								if(distanceFromListener > m_NewWheelDistanceScale[nonFocusAudibleCars].distanceFromListener)//if not audible
								{

										m_NewWheelDistanceScale[i].volScale = 0.0f;
									//	Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[PriorityVolume], 0.0f);

										Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelPriorityVolume], 0.0f);
										Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelPriorityVolume], 0.0f);
										Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelPriorityVolume], 0.0f);
										Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelPriorityVolume], 0.0f);
										//Audio::InterfaceController::Get().SuspendEntity(hCurrentEntity);
										m_isSuspended = true;

									break; // exit loop, car is further away than last car
								}
								else // if audible but further away than some cars
									break; // exit loop, car will get no closer
							}

							//all distances have been calculated, apply suspending, distance scaling, and reset the distance array
							U32 finalVehicle = numVehicles-1;

							if(finalVehicleHasFocus) //set 'final' vehicle to be the penultimate one
								finalVehicle = numVehicles-2;

							//all vehicle distances have been entered in the array, now we can post process them
							if(vehicleIndex == finalVehicle)
							{

								//get the volume scaling values
								F32 audibleDistance = m_NewWheelDistanceScale[nonFocusAudibleCars].distanceFromListener;
								F32 closestCarDistance = m_NewWheelDistanceScale[0].distanceFromListener;

								//take closest distance from camera
									closestCarDistance = 0;

								F32 recalculatedDistanceRange = audibleDistance - closestCarDistance;

								if(recalculatedDistanceRange<=0)
									recalculatedDistanceRange = .000001f;

								//the post process, apply the volume scale and suspends.
								for(int i = 0; i < (int)nonFocusAudibleCars+1;i++)
								{

									//Audio::InterfaceController::Get().ResumeEntity(m_NewWheelDistanceScale[i].hEntityHandle);
									//ResumeControllerInstances(m_NewWheelDistanceScale[i].hEntityHandle);

									//get ratio of current distance to maxdistance
									F32 volPercent = Maths::Clamp((m_NewWheelDistanceScale[i].distanceFromListener - closestCarDistance) / recalculatedDistanceRange,0.0f,1.0f);
									
									volPercent	*= volPercent * volPercent;//powf(volPercent,3.0); //weight the vol scale, cubic		

									F32 volScale = 1.0f - volPercent; //get vol scale			

									//end calculate priority vol
									//set the volume scale for debugging
									m_NewWheelDistanceScale[i].volScale = volScale;

									//resume this entity since it should be audible
									//Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[i].hEntityHandle, m_aHandle[PriorityVolume], volScale);
									Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle, m_NewWheelDistanceScale[nonFocusAudibleCars+1].hPriorityMemberHandle[0], volScale);
									Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle, m_NewWheelDistanceScale[nonFocusAudibleCars+1].hPriorityMemberHandle[1], volScale);
									Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle, m_NewWheelDistanceScale[nonFocusAudibleCars+1].hPriorityMemberHandle[2], volScale);
									Audio::InterfaceController::Get().SetEntityValue(m_NewWheelDistanceScale[nonFocusAudibleCars+1].hEntityHandle, m_NewWheelDistanceScale[nonFocusAudibleCars+1].hPriorityMemberHandle[3], volScale);
									
									if(volScale <= 0)//id the volume has been set to zero, suspend this object next turn
										hZeroWheelEntity = m_NewWheelDistanceScale[i].hEntityHandle;
									else
									{

										//we cannot resume an instance that was already resumed, since the wheel controller wants
										//control over the wheel surface suspending/resuming
										//bool previouslyResumed = false;
										//for(int k = 0; k <(int)nonFocusAudibleCars;k++)
										//{

										//	if(m_NewWheelInstanceResumeArray[k] == m_NewWheelDistanceScale[i].hEntityHandle)
										//		previouslyResumed = true;

										//}
											

									//	if(!previouslyResumed) // only resume if previously suspended
										{
											ResumeControllerInstances(m_NewWheelDistanceScale[i].hEntityHandle);

											//store the handle, we don't want to resume next time
											//m_NewWheelInstanceResumeArray[i] = m_NewWheelDistanceScale[i].hEntityHandle;

										}

										m_FLSurfaceType_Prev = -1;
										m_FRSurfaceType_Prev = -1;
										m_RLSurfaceType_Prev = -1;
										m_RRSurfaceType_Prev = -1;

										Audio::InterfaceController::Get().SetEntityValue(hZeroWheelEntity, m_aHandle[FLWheelSurfaceType], (float)-1);
										Audio::InterfaceController::Get().SetEntityValue(hZeroWheelEntity, m_aHandle[FRWheelSurfaceType], (float)-1);
										Audio::InterfaceController::Get().SetEntityValue(hZeroWheelEntity, m_aHandle[RLWheelSurfaceType], (float)-1);
										Audio::InterfaceController::Get().SetEntityValue(hZeroWheelEntity, m_aHandle[RRWheelSurfaceType], (float)-1);
									}
									

									////print distances scale
									//if(g_RenderDistanceScale)
									//{

									//	static F32 xPos = 100;
									//	F32 yPos = 150 +  (40 * (F32)i);

									//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(xPos,yPos),Util::C_String().Printf("Distance:%f", m_NewWheelDistanceScale[i].distanceFromListener));
									//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(xPos+200,yPos),Util::C_String().Printf("CarID:%d", m_NewWheelDistanceScale[i].CarID));
									//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(xPos+400,yPos),Util::C_String().Printf("VolScale:%f", m_NewWheelDistanceScale[i].volScale));
									//}

									//reset the distance array
									m_NewWheelDistanceScale[i].distanceFromListener = 100000;
									m_NewWheelDistanceScale[i].hEntityHandle.Zero();
								}

							}
		
					}
					else
					{
						//Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[PriorityVolume], 1.0f);
						ResumeControllerInstances(hCurrentEntity);

						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelPriorityVolume], 1.0f);
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelPriorityVolume], 1.0f);
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelPriorityVolume], 1.0f);
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelPriorityVolume], 1.0f);						//resume this entity since it should be audible
						//Audio::InterfaceController::Get().ResumeEntity(hCurrentEntity);
						

					}

}

#endif

} // end use car scale for wheels

					//Perf::EndNamedEvent();
					//no need to update if out of range
					//if(bSuspended)
					//	break;

					//since we do instance suspending in this entity, we can check if one of the wheels common in a player car
					//or the common in an AI car is suspended we can bail
						if(!gbUseCarScaleForWheels)
						{
							if(g_doWheelInstanceSuspendBailOut)
							{
								Audio::EntityTypeDataMemberHandle instanceHandle = Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheelCommon");
								if(instanceHandle != -1)
									if(Audio::InterfaceController::Get().IsInstanceSuspended(hCurrentEntity,instanceHandle))
										break;

								instanceHandle = Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheelCommon");
								if(instanceHandle != -1)
									if(Audio::InterfaceController::Get().IsInstanceSuspended(hCurrentEntity,instanceHandle))
										break;
							}
						}

					//***** end actionOUts

					Vehicle::C_DamageLayer* damageLayer = pActor->GetVehicleModel()->GetDamageLayer();
					
					//update Wheels
					bool attached = !damageLayer->GetWheel(0)->m_TyreBurst;
					if(m_PrevFLAttached != attached)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelAttached], (F32)attached);
						m_PrevFLAttached = attached;
					}

					attached = !damageLayer->GetWheel(1)->m_TyreBurst;
					if(m_PrevFRAttached != attached)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelAttached], (F32)attached);
						m_PrevFRAttached = attached;
					}

					attached = !damageLayer->GetWheel(2)->m_TyreBurst;
					if(m_PrevRLAttached != attached)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelAttached], (F32)attached);
						m_PrevRLAttached = attached;
					}

					attached = !damageLayer->GetWheel(3)->m_TyreBurst;
					if(m_PrevRRAttached != attached)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelAttached], (F32)attached);
						m_PrevRRAttached = attached;
					}



					//calculate wheel direction
					Maths::C_Quaternion WheelLeftFront(vehicleMatrix.AY(), Maths::DegToRad(45.0));
					vehicleDirection = vehicleMatrix.AZ();
					soundDir = vehicleDirection * WheelLeftFront;					

					//send left front Dir
					if (!m_prevFLWheelDirection.All_EQ(soundDir))
					{
						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[FLWheelDirection], (F32*)(&soundDir));
						m_prevFLWheelDirection = soundDir;
					}

					//front right
					Maths::C_Quaternion WheelRightFront(vehicleMatrix.AY(), Maths::DegToRad(315.0));
					vehicleDirection = vehicleMatrix.AZ();
					soundDir = vehicleDirection * WheelRightFront;
			

					if (!m_prevFRWheelDirection.All_EQ(soundDir))
					{
						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[FRWheelDirection], (F32*)(&soundDir));
						m_prevFRWheelDirection = soundDir;
					}

					//rear left
					Maths::C_Quaternion WheelLeftRear(vehicleMatrix.AY(), Maths::DegToRad(135.0));
					vehicleDirection = vehicleMatrix.AZ();
					soundDir = vehicleDirection * WheelLeftRear;
																												 
					if (!m_prevRLWheelDirection.All_EQ(soundDir))
					{
						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[RLWheelDirection], (F32*)(&soundDir));
						m_prevRLWheelDirection = soundDir;
					}

					//rear right
					Maths::C_Quaternion WheelRightRear(vehicleMatrix.AY(), Maths::DegToRad(225.0));
					vehicleDirection = vehicleMatrix.AZ();
					soundDir = vehicleDirection * WheelRightRear;

					if (!m_prevRRWheelDirection.All_EQ(soundDir))
					{													 
						Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[RRWheelDirection], (F32*)(&soundDir));
						m_prevRRWheelDirection = soundDir;
					}

					//end wheels direction

					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelDownForce], (pCar->Wheel(0).GroundForce() | pCar->GetAxis().AxisY()) / pCar->Wheel(0).CalculateStaticLoad());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelDownForce], (pCar->Wheel(1).GroundForce() | pCar->GetAxis().AxisY()) / pCar->Wheel(1).CalculateStaticLoad());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelDownForce], (pCar->Wheel(2).GroundForce() | pCar->GetAxis().AxisY()) / pCar->Wheel(2).CalculateStaticLoad());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelDownForce], (pCar->Wheel(3).GroundForce() | pCar->GetAxis().AxisY()) / pCar->Wheel(3).CalculateStaticLoad());	
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelHitModifierDepth], (F32)pCar->Wheel(0).SurfaceModifierDepth());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelHitModifierDepth], (F32)pCar->Wheel(1).SurfaceModifierDepth());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelHitModifierDepth], (F32)pCar->Wheel(2).SurfaceModifierDepth());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelHitModifierDepth], (F32)pCar->Wheel(3).SurfaceModifierDepth());
					
					FLIsOnGround = (F32)pCar->WheelOnGround(0);
					if (m_prevFLIsOnGround != FLIsOnGround)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelOnGround], FLIsOnGround);	
						m_prevFLIsOnGround = FLIsOnGround;
					}

					//only send Surface modifier if the wheels are on the ground
					{
					F32 modifier = (F32)pCar->Wheel(0).SurfaceModifier();
					if(FLIsOnGround && m_prevFLWheelHitModifier != modifier)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelHitModifier], modifier);	
						m_prevFLWheelHitModifier = modifier;
					}

					FRIsOnGround = (F32)pCar->WheelOnGround(1);
					if (m_prevFRIsOnGround != FRIsOnGround)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelOnGround], FRIsOnGround );
						m_prevFRIsOnGround = FRIsOnGround;
					}
					
					modifier = (F32)pCar->Wheel(1).SurfaceModifier();
					if(FRIsOnGround && m_prevFRWheelHitModifier != modifier)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelHitModifier], modifier);	
						m_prevFRWheelHitModifier = modifier;
					}

					RLIsOnGround = (F32)pCar->WheelOnGround(2);
					if (m_prevRLIsOnGround != RLIsOnGround)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelOnGround], RLIsOnGround);	
						m_prevRLIsOnGround = RLIsOnGround;
					}

					modifier = (F32)pCar->Wheel(2).SurfaceModifier();
					if(RLIsOnGround && m_prevRLWheelHitModifier != modifier)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelHitModifier], modifier);		
						m_prevRLWheelHitModifier = modifier;
					}

					RRIsOnGround = (F32)pCar->WheelOnGround(3);
					if (m_prevRRIsOnGround != RRIsOnGround)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelOnGround], RRIsOnGround );
						m_prevRRIsOnGround = RRIsOnGround;
					}

					modifier = (F32)pCar->Wheel(3).SurfaceModifier();
					if(RRIsOnGround && m_prevRRWheelHitModifier != modifier)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelHitModifier], modifier);
						m_prevRRWheelHitModifier = modifier;
					}
					}

					F32 distanceFromGround = 0.0f;

					if(!FLIsOnGround && !FRIsOnGround && !RLIsOnGround && !RRIsOnGround && isFocusVehicle)//only do for focus vehicle
					{
						//get car height
						Maths::HVec3 height = carPos - pActor->GetGroundPosition();
						distanceFromGround = height.Y();							

						//Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eFocusCarHeight,0,&distanceFromGround);
					}
					
					if( isFocusVehicle && m_prevFocusCarHeight != distanceFromGround)
					{
						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eFocusCarHeight,0,&distanceFromGround);
						m_prevFocusCarHeight = distanceFromGround;
					}
					
					//Maths::HMat43 camMatrix = view.DebugFeed().Mat43();

					//Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[WheelListenerPosition], (F32*)(&camMatrix.AW()));
					//Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[WheelListenerDirection], (F32*)(&camMatrix.AZ()));					
					//Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[WheelListenerUp], (F32*)(&camMatrix.AY()));
					//Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[WheelListenerVelocity], (F32*)(&velocity));
					
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[FLWheelPosition], (F32*)&pCar->Wheel(0).GetWheelPosition());		 
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[FRWheelPosition], (F32*)&pCar->Wheel(1).GetWheelPosition());		 
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[RLWheelPosition], (F32*)&pCar->Wheel(2).GetWheelPosition());		 
					Audio::InterfaceController::Get().SetEntityVectorIn(hCurrentEntity, m_aHandle[RRWheelPosition], (F32*)&pCar->Wheel(3).GetWheelPosition());
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelSpeed], (F32)(pCar->Wheel(0).GetAngularVelocity1D() * pCar->Wheel(0).Radius()));		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelSpeed], (F32)(pCar->Wheel(1).GetAngularVelocity1D() * pCar->Wheel(1).Radius()));		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelSpeed], (F32)(pCar->Wheel(2).GetAngularVelocity1D() * pCar->Wheel(2).Radius()));		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelSpeed], (F32)(pCar->Wheel(3).GetAngularVelocity1D() * pCar->Wheel(3).Radius()));
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelSpringForce], pCar->Wheel(0).GroundForce() | pCar->GetAxis().AxisY());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelSpringForce], pCar->Wheel(1).GroundForce() | pCar->GetAxis().AxisY());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelSpringForce], pCar->Wheel(2).GroundForce() | pCar->GetAxis().AxisY());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelSpringForce], pCar->Wheel(3).GroundForce() | pCar->GetAxis().AxisY());

					//send braking
					F32 brake = pCar->Controls().brake;
					if (m_prevBrake != brake)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLPadBrake], brake);		 
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRPadBrake], brake);		 
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLPadBrake], brake);		 
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRPadBrake], brake);
						m_prevBrake = brake;
					}
					

					F32 handbrake = pCar->Controls().handbrake;
/*					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLHandBrake], handbrake);		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRHandBrake], handbrake);*/	
					//to rears only
					if (m_prevHandBrake != handbrake)
					{
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLHandBrake], handbrake);		 
						Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRHandBrake], handbrake);
						m_prevHandBrake = handbrake;
					}
					

					if(vehicleIndex == 0 && !m_isSplitScreen)
						DoWheelSurfacesAndInstanceSuspending(hCurrentEntity, pCar);
					else
						DoAIWheelSurfacesAndInstanceSuspending(hCurrentEntity, pCar);

					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelXSlide],  pCar->Wheel(0).SlipAngle());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelXSlide], pCar->Wheel(1).SlipAngle());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelXSlide], pCar->Wheel(2).SlipAngle());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelXSlide], pCar->Wheel(3).SlipAngle());
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelZSlide], pCar->Wheel(0).WheelSpin());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelZSlide], pCar->Wheel(1).WheelSpin());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelZSlide], pCar->Wheel(2).WheelSpin());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelZSlide], pCar->Wheel(3).WheelSpin());

					float meanStaticLoad = pCar->Wheel(0).CalculateStaticLoad() + pCar->Wheel(1).CalculateStaticLoad() + pCar->Wheel(2).CalculateStaticLoad() +pCar->Wheel(3).CalculateStaticLoad();

					meanStaticLoad = meanStaticLoad / 4.f;

					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelStaticLoad], pCar->Wheel(0).CalculateStaticLoad());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelStaticLoad], pCar->Wheel(1).CalculateStaticLoad());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelStaticLoad], pCar->Wheel(2).CalculateStaticLoad());		 
					Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelStaticLoad], pCar->Wheel(3).CalculateStaticLoad());

					return WorldObjectMessageHandler::ProcessMessage(info);

			}
		break;

		case ActorMsg::E_AudioMsg_TriggerActionFromHandle:
		{
			const ActorMsg::C_AudioTriggerActionFromHandleMsg* msg = Messaging::MessageCast<ActorMsg::C_AudioTriggerActionFromHandleMsg>(info.pMessage);
			if (msg->m_uEntityDataMemberHnd != 0xffffffff)
				Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, msg->m_uEntityDataMemberHnd);
			break;
		}

		//case ActorMsg::E_AudioMsg_EntityAudioStopped:
		//	if (m_pManager != NULL && m_pManager->Type() == E_EntitySuspenderResumer)
		//	{
		//		m_pManager->Suspend();
		//	}
		//	return true;

		//case ActorMsg::E_AudioMsg_InstanceAudioStopped:
		//	if (m_pManager != NULL)
		//	{
		//		const ActorMsg::C_InstanceAudioStoppedMsg* pMsg = Messaging::MessageCast<ActorMsg::C_InstanceAudioStoppedMsg>(info.pMessage);
		//		BIZ_ASSERT(m_pManager->Type() == E_InstanceSuspenderResumer);
		//		if (m_pManager->Type() == E_InstanceSuspenderResumer)
		//			((C_InstanceSuspenderResumer*)(info.pMessage))->SuspendInstance(pMsg->m_sInstanceId);
		//	}
		//	return true;
			

		//default:
		//	return WorldObjectMessageHandler::ProcessMessage(info);
	}

	//switch(info.pMessage->Type())
	{
		//default:
			//return WorldObjectMessageHandler::ProcessMessage(info);
	}

	return true;
}

void Components::WheelMessageHandler::DoAIWheelSurfacesAndInstanceSuspending(AudioEntityHandle hCurrentEntity, class RacingPhysics::Car *pCar)
{
	//do suspending if pending
	if(m_AIPendingSuspend)
	{
		F32 timeSinceSuspend = C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - m_AISuspendTime;

		if(timeSinceSuspend >= gbWheelSuspendDelay)
		{
			m_AIPendingSuspend = false;
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, m_AISuspendMessage,hCurrentEntity.ArrayHandle().GetValue());
		}

	}

	//get surface type for each wheel		

	if (FLIsOnGround)
	{
		m_FLSurfaceType = pCar->Wheel(0).SurfaceType();

		if (m_FLSurfaceType != m_FLSurfaceType_Prev)
		{
			Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelSurfaceType], (float)m_FLSurfaceType);
			m_FLSurfaceType_Prev = m_FLSurfaceType;
		}
	}
	else
		m_FLSurfaceType_Prev = -1;

	if (FRIsOnGround)
	{
		m_FRSurfaceType = pCar->Wheel(1).SurfaceType();

		if (m_FRSurfaceType != m_FRSurfaceType_Prev)
		{
			Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelSurfaceType], (float)m_FRSurfaceType);
			m_FRSurfaceType_Prev = m_FRSurfaceType;		
		}
	}
	else
		m_FRSurfaceType_Prev = -1;
	

	if (RLIsOnGround)
	{
		m_RLSurfaceType = pCar->Wheel(2).SurfaceType();

		if (m_RLSurfaceType != m_RLSurfaceType_Prev)
		{
			Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelSurfaceType], (float)m_RLSurfaceType);		 
			m_RLSurfaceType_Prev = m_RLSurfaceType;
		}
	}
	else
		m_RLSurfaceType_Prev = -1;

	if (RRIsOnGround)
	{
		m_RRSurfaceType = pCar->Wheel(3).SurfaceType();

		if (m_RRSurfaceType != m_RRSurfaceType_Prev)
		{
			Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelSurfaceType], (float)m_RRSurfaceType);
			m_RRSurfaceType_Prev = m_RRSurfaceType;		
		}
	}
	else
		m_RRSurfaceType_Prev = -1;

	//special case for AI wheels
	//check all the surface types, the highest type on our list takes precedence.
	S32 frontMax = Maths::Max(m_FLSurfaceType_Prev,m_FRSurfaceType_Prev);
	S32 rearMax = Maths::Max(m_RLSurfaceType_Prev,m_RRSurfaceType_Prev);
	S32 maxSurface = Maths::Max(frontMax,rearMax);

	E_Wheel_Surface newSurface = RoadSurface(maxSurface);

	//check if it's gone off/on road
	if(m_prevAISurface != newSurface)
	{
		if(newSurface == E_Wheel_Surface_OnRoad)
		{
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eAIWheelDry_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								
		
			m_AISuspendMessage = WheelSubsystem::eAIWheelOffRoad_AudioStopped;
		}
		else
		{
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eAIWheelOffRoad_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								

			m_AISuspendMessage = WheelSubsystem::eAIWheelDry_AudioStopped;

		}
		
		m_AISuspendTime =  C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
		m_AIPendingSuspend = true;

		m_prevAISurface = newSurface;				

	}

}

void Components::WheelMessageHandler::DoWheelSurfacesAndInstanceSuspending(AudioEntityHandle hCurrentEntity, class RacingPhysics::Car *pCar)
{
	//do suspending if pending
	if(m_FLPendingSuspend)
	{
		F32 timeSinceSuspend = C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - m_FLSuspendTime;

		if(timeSinceSuspend >= gbWheelSuspendDelay)
		{
			m_FLPendingSuspend = false;
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, m_FLSuspendMessage,hCurrentEntity.ArrayHandle().GetValue());
		}

	}

	if(m_FRPendingSuspend)
	{
		F32 timeSinceSuspend = C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - m_FRSuspendTime;

		if(timeSinceSuspend >= gbWheelSuspendDelay)
		{
			m_FRPendingSuspend = false;
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, m_FRSuspendMessage,hCurrentEntity.ArrayHandle().GetValue());
		}

	}

	if(m_RLPendingSuspend)
	{
		F32 timeSinceSuspend = C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - m_RLSuspendTime;

		if(timeSinceSuspend >= gbWheelSuspendDelay)
		{
			m_RLPendingSuspend = false;
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, m_RLSuspendMessage,hCurrentEntity.ArrayHandle().GetValue());
		}

	}

	if(m_RRPendingSuspend)
	{
		F32 timeSinceSuspend = C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - m_RRSuspendTime;

		if(timeSinceSuspend >= gbWheelSuspendDelay)
		{
			m_RRPendingSuspend = false;
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, m_RRSuspendMessage,hCurrentEntity.ArrayHandle().GetValue());
		}

	}



	//get surface type for each wheel		

	if (FLIsOnGround)
	{
		m_FLSurfaceType = pCar->Wheel(0).SurfaceType();

		if (m_FLSurfaceType != m_FLSurfaceType_Prev)
		{
			Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FLWheelSurfaceType], (float)m_FLSurfaceType);
			m_FLSurfaceType_Prev = m_FLSurfaceType;

			E_Wheel_Surface newSurface = RoadSurface(m_FLSurfaceType);

			//check if it's gone off/on road
			if(m_prevFLSurface != newSurface)
			{
				if(newSurface == E_Wheel_Surface_OnRoad)
				{
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eFLWheelDry_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								
				
					m_FLSuspendMessage = WheelSubsystem::eFLWheelOffRoad_AudioStopped;
				}
				else
				{
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eFLWheelOffRoad_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								

					m_FLSuspendMessage = WheelSubsystem::eFLWheelDry_AudioStopped;

				}
				
				m_FLSuspendTime =  C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
				m_FLPendingSuspend = true;

				m_prevFLSurface = newSurface;				

			}

		}
	}
	else
		m_FLSurfaceType_Prev = -1;

	if (FRIsOnGround)
	{
		m_FRSurfaceType = pCar->Wheel(1).SurfaceType();

		if (m_FRSurfaceType != m_FRSurfaceType_Prev)
		{
			Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[FRWheelSurfaceType], (float)m_FRSurfaceType);
			m_FRSurfaceType_Prev = m_FRSurfaceType;

			E_Wheel_Surface newSurface= RoadSurface(m_FRSurfaceType);

			//check if it's gone off/on road
			if(m_prevFRSurface != newSurface)
			{
				if(newSurface == E_Wheel_Surface_OnRoad)
				{
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eFRWheelDry_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								
					m_FRSuspendMessage = WheelSubsystem::eFRWheelOffRoad_AudioStopped;
				}
				else
				{
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eFRWheelOffRoad_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								
					m_FRSuspendMessage = WheelSubsystem::eFRWheelDry_AudioStopped;
				}

				m_FRSuspendTime =  C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
				m_FRPendingSuspend = true;
				m_prevFRSurface = newSurface;				

			}
		}
	}
	else
		m_FRSurfaceType_Prev = -1;
	

	if (RLIsOnGround)
	{
		m_RLSurfaceType = pCar->Wheel(2).SurfaceType();

		if (m_RLSurfaceType != m_RLSurfaceType_Prev)
		{
			Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RLWheelSurfaceType], (float)m_RLSurfaceType);		 
			m_RLSurfaceType_Prev = m_RLSurfaceType;


			E_Wheel_Surface newSurface = RoadSurface(m_RLSurfaceType);

			//check if it's gone off/on road
			if(m_prevRLSurface!= newSurface)
			{
				if(newSurface == E_Wheel_Surface_OnRoad)
				{
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eRLWheelDry_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								
					m_RLSuspendMessage = WheelSubsystem::eRLWheelOffRoad_AudioStopped;
				}
				else
				{
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eRLWheelOffRoad_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								
					m_RLSuspendMessage = WheelSubsystem::eRLWheelDry_AudioStopped;

				}

				m_RLSuspendTime =  C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
				m_RLPendingSuspend = true;

				m_prevRLSurface= newSurface;				

			}						
		}
	}
	else
		m_RLSurfaceType_Prev = -1;

	if (RRIsOnGround)
	{
		m_RRSurfaceType = pCar->Wheel(3).SurfaceType();

		if (m_RRSurfaceType != m_RRSurfaceType_Prev)
		{
			Audio::InterfaceController::Get().SetEntityValue(hCurrentEntity, m_aHandle[RRWheelSurfaceType], (float)m_RRSurfaceType);
			m_RRSurfaceType_Prev = m_RRSurfaceType;

			E_Wheel_Surface newSurface = RoadSurface(m_RRSurfaceType);

			//check if it's gone off/on road
			if(m_prevRRSurface != newSurface)
			{
				if(newSurface == E_Wheel_Surface_OnRoad)
				{
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eRRWheelDry_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								
					m_RRSuspendMessage = WheelSubsystem::eRRWheelOffRoad_AudioStopped;
				}
				else
				{
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eWheelSubsystem, WheelSubsystem::eRRWheelOffRoad_AudioResumed,hCurrentEntity.ArrayHandle().GetValue());								
					m_RRSuspendMessage = WheelSubsystem::eRRWheelDry_AudioStopped;
				}

				m_RRSuspendTime =  C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
				m_RRPendingSuspend = true;

				m_prevRRSurface = newSurface;				

			}	
		}
	}
	else
		m_RRSurfaceType_Prev = -1;

}


void Components::WheelMessageHandler::DoActionOuts(AudioEntityHandle hCurrentEntity)
{
}

Components::WheelMessageHandler::E_Wheel_Surface Components::WheelMessageHandler::RoadSurface(U32 surface)
{

	switch(surface)
	{
		case Game::E_EffectMaterial_Default:						
		case Game::E_EffectMaterial_Tarmac:							
		case Game::E_EffectMaterial_Concrete:							
		case Game::E_EffectMaterial_Rock:								
		case Game::E_EffectMaterial_Water:								
		case Game::E_EffectMaterial_Brick:								
		case Game::E_EffectMaterial_Wood:								
		case Game::E_EffectMaterial_Metal:								
		case Game::E_EffectMaterial_Plastic:							
		case Game::E_EffectMaterial_Glass:								
		case Game::E_EffectMaterial_Rubber:							
		case Game::E_EffectMaterial_Upholstery:						
		case Game::E_EffectMaterial_Cardboard:							
		case Game::E_EffectMaterial_Flesh:								
		case Game::E_EffectMaterial_WaterDeep:							
		case Game::E_EffectMaterial_ConcreteFenceChain:				
		case Game::E_EffectMaterial_ConcreteFencePlastic:				
		case Game::E_EffectMaterial_MetalHollow:						
		case Game::E_EffectMaterial_MetalArmco:						
		case Game::E_EffectMaterial_MetalSheet:						
		case Game::E_EffectMaterial_MetalFenceChain:					
		case Game::E_EffectMaterial_MetalFenceSolid:					
		case Game::E_EffectMaterial_MetalFenceSlatted:					
		case Game::E_EffectMaterial_MetalSolidPost:					
		case Game::E_EffectMaterial_WackyMagic:						
		case Game::E_EffectMaterial_PavingStone:						
		case Game::E_EffectMaterial_Vehicle:							
			return E_Wheel_Surface_OnRoad;
		break;

		case Game::E_EffectMaterial_DirtLightRough:					
		case Game::E_EffectMaterial_DirtHeavyRough:					
		case Game::E_EffectMaterial_TrainTracks:						
		case Game::E_EffectMaterial_Vegetation:						
		case Game::E_EffectMaterial_Dirt:								
		case Game::E_EffectMaterial_Mud:								
		case Game::E_EffectMaterial_Grass:								
		case Game::E_EffectMaterial_Gravel:							
		case Game::E_EffectMaterial_Sand:								
		case Game::E_EffectMaterial_WetSand:							
			return E_Wheel_Surface_OffRoad;
		break;

		default:
			return E_Wheel_Surface_Unknown;
		break;

	}


}


namespace Audio
{
	StringHash HashFLWheelController	= StringHash( "FLWheelController" );
	StringHash HashFRWheelController	= StringHash( "FRWheelController" );
	StringHash HashRLWheelController	= StringHash( "RLWheelController" );
	StringHash HashRRWheelController	= StringHash( "RRWheelController" );
	StringHash HashFLWheel3DMixer		= StringHash( "FLWheel3DMixer" );
	StringHash HashFRWheel3DMixer		= StringHash( "FRWheel3DMixer" );
	StringHash HashRLWheel3DMixer		= StringHash( "RLWheel3DMixer" );
	StringHash HashRRWheel3DMixer		= StringHash( "RRWheel3DMixer" );
	StringHash HashFLWheelCommon		= StringHash( "FLWheelCommon" );
	StringHash HashFRWheelCommon		= StringHash( "FRWheelCommon" );
	StringHash HashRLWheelCommon		= StringHash( "RLWheelCommon" );
	StringHash HashRRWheelCommon		= StringHash( "RRWheelCommon" );
	StringHash HashAIWheelController	= StringHash( "AIWheelController" );
	StringHash HashAIWheel3DMixer		= StringHash( "AIWheel3DMixer" );
	StringHash HashAIWheelCommon		= StringHash( "AIWheelCommon" );
	StringHash HashAIWheelOffroad		= StringHash( "AIWheelOffroad" );
	StringHash HashAIWheelDry			= StringHash( "AIWheelDry" );
	StringHash HashFLWheelDry			= StringHash( "FLWheelDry" );
	StringHash HashFRWheelDry			= StringHash( "FRWheelDry" );
	StringHash HashRLWheelDry			= StringHash( "RLWheelDry" );
	StringHash HashRRWheelDry			= StringHash( "RRWheelDry" );
	StringHash HashFLWheelOffroad		= StringHash( "FLWheelOffroad" );
	StringHash HashFRWheelOffroad		= StringHash( "FRWheelOffroad" );
	StringHash HashRLWheelOffroad		= StringHash( "RLWheelOffroad" );
	StringHash HashRRWheelOffroad		= StringHash( "RRWheelOffroad" );
}

void Components::WheelMessageHandler::ResumeControllerInstances(AudioEntityHandle hCurrentEntity, bool resumeAll)
{

#ifdef USE_STRING_HASHES_FOR_ENTITY_DEFINITIONS
	Audio::EntityDataMemberHandle handle = Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheelController );

		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,handle);
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheelController));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheelController));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheelController));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheel3DMixer));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheel3DMixer));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheel3DMixer));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheel3DMixer));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheelCommon));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheelCommon));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheelCommon));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheelCommon));

		//AI instances
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheelController));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheel3DMixer));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheelCommon));

		if(resumeAll)
		{
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheelOffroad));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheelDry));

			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheelDry));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheelDry));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheelDry));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheelDry));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheelOffroad));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheelOffroad));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheelOffroad));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheelOffroad));
		}
#else
	//these want to always be awake if the car is in range

	Audio::EntityDataMemberHandle handle = Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheelController");

		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,handle);
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheelController"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheelController"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheelController"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheel3DMixer"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheel3DMixer"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheel3DMixer"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheel3DMixer"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheelCommon"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheelCommon"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheelCommon"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheelCommon"));

		//AI instances
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheelController"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheel3DMixer"));
		Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheelCommon"));

		if(resumeAll)
		{
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheelOffroad"));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheelDry"));

			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheelDry"));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheelDry"));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheelDry"));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheelDry"));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheelOffroad"));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheelOffroad"));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheelOffroad"));
			Audio::InterfaceController::Get().ResumeInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheelOffroad"));
		}
#endif

}

void Components::WheelMessageHandler::SuspendControllerInstances(AudioEntityHandle hCurrentEntity, bool suspendAll)
{


	//these want to always be awake if the car is in range
#ifdef USE_STRING_HASHES_FOR_ENTITY_DEFINITIONS

	Audio::EntityDataMemberHandle handle = Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheelController);

		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,handle);
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheelController));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheelController));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheelController));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheel3DMixer));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheel3DMixer));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheel3DMixer));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheel3DMixer));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheelCommon));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheelCommon));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheelCommon));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheelCommon));

		//AI instances
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheelController));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheel3DMixer));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheelCommon));


		//suspendAll = true;
		if(suspendAll)
		{
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheelDry));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashAIWheelOffroad));

			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheelDry));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheelDry));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheelDry));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheelDry));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFLWheelOffroad));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashFRWheelOffroad));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRLWheelOffroad));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, Audio::HashRRWheelOffroad));

		}
#else

	Audio::EntityDataMemberHandle handle = Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheelController");

		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,handle);
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheelController"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheelController"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheelController"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheel3DMixer"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheel3DMixer"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheel3DMixer"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheel3DMixer"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheelCommon"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheelCommon"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheelCommon"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheelCommon"));

		//AI instances
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheelController"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheel3DMixer"));
		Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheelCommon"));


		//suspendAll = true;
		if(suspendAll)
		{
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheelDry"));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "AIWheelOffroad"));

			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheelDry"));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheelDry"));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheelDry"));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheelDry"));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FLWheelOffroad"));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "FRWheelOffroad"));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RLWheelOffroad"));
			Audio::InterfaceController::Get().SuspendInstance(hCurrentEntity,Audio::InterfaceController::Get().LookupInstance(m_hEntityDefinition, "RRWheelOffroad"));

		}

#endif

}


//	PickupPositionMessageHandler
Components::PickupPositionMessageHandler::PickupPositionMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom   ) : WorldObjectMessageHandler( hEntityDefinition, pCom )
{
	m_pCom = pCom;
	for (U32 PickupPositionId = WorldObjectMessageHandler::NumMessageIds; PickupPositionId < NumMessageIds; PickupPositionId++)
		m_aHandle.PushBack( Audio::InterfaceController::Get().LookupInstanceComponent( hEntityDefinition, PickupPositionMessageHandler::GetString( PickupPositionId ) ) );
}

bool Components::PickupPositionMessageHandler::ProcessMessage(BaseMessageHandlerInfo& info)
{

	//if(!m_pCom->GetEntityHnd().IsValid())
	//		return true;

	const AudioEntityHandle& hCurrentEntity = m_pCom->GetEntityHnd();

	switch(info.pMessage->Type())
	{
		case ActorMsg::E_AudioMsg_TriggerActionFromHandle:
		{
			const ActorMsg::C_AudioTriggerActionFromHandleMsg* msg = Messaging::MessageCast<ActorMsg::C_AudioTriggerActionFromHandleMsg>(info.pMessage);
			if (msg->m_uEntityDataMemberHnd != 0xffffffff)
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_pCom->GetEntityHnd(), msg->m_uEntityDataMemberHnd);
			break;
		}

		case Actor::E_ActorMsg_ComponentCreated:
		{
			m_wakeSleepCount = 0;

		}
		break;

		case GameMessaging::E_GameMsg_Update:
			{
				if(!m_pCom->GetEntityHnd().IsValid())
					return true;
			
				//go to world update if active
				if(!Audio::InterfaceController::Get().IsEntitySuspended(hCurrentEntity))
					return WorldObjectMessageHandler::ProcessMessage(info);
			}
			break;

		//case ActorMsg::E_AudioMsg_Pickups_Awarded:
		case Game::E_PickupMessage_PickupPickedUp:
			{

				if(!m_pCom->GetEntityHnd().IsValid())
					return true;

				//we have picked it up and it's not on track, so effectively 'sleeping' though we will not get a sleep msg, decrease the ref count accordingly
				m_wakeSleepCount--;

				const Game::Actors::PickupPickedUpMsg* msg =  (const Game::Actors::PickupPickedUpMsg*)info.pMessage;

				//check if focus car
				Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				if(pRLC)
				{
					S32 numVehicles = pRLC->GetNumVehicles();

					if(msg->mVehicleIndex >= numVehicles)
						return true;
				
					//we only want car assigned to viewports to make pickup sounds..
					if(msg->mVehicleIndex >= Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount())
						 break;
				}

				switch(msg->mePickupType)
						{

						case E_PICKUP_TYPE_FAN:
							{
								if(msg->mIsGoldenFanDemand)
									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[FanDemandPickupActionIn],false);
								else
									Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[GoldenFanDemandPickupActionIn],false);
							}
							break;

						case E_PICKUP_TYPE_STOPWATCH:
								Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[StopwatchPickupActionIn],false);
							break;

							//all other pickups get the defauls sound
							default:
								Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[PerkPickupActionIn],false);
							break;
						}
				
			}
			break;

		case Game::E_PickupMessage_WakeAudio:
		{

			if(!hCurrentEntity.IsValid())
				m_pCom->CreateEntityInstance();		

			Audio::InterfaceController::Get().ResumeEntity(hCurrentEntity);

			m_wakeSleepCount++;

			const Game::Actors::WakeAudioPickupMsg* msg =  (const Game::Actors::WakeAudioPickupMsg*)info.pMessage;

			//new actions			
			if(msg->mActorType == Game::E_ActorTypeId_PickupPositionDecoy)
				Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[StartDecoyActionIn],false);	
			else
			{
				if(msg->mePickupType == E_PICKUP_TYPE_FAN)
				{
					if(msg->mIsGoldenFanDemand)
						Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[StartGoldenFanDemandActionIn],false);
					else
						Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[StartFanDemandActionIn],false);
				}
				else
				if(msg->mePickupType == E_PICKUP_TYPE_STOPWATCH)
					Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[StartStopwatchActionIn],false);
				else
					Audio::InterfaceController::Get().TriggerEntityActionIn(hCurrentEntity, m_aHandle[StartActionIn],false);
			}



		}
		break;

		case Game::E_PickupMessage_SleepAudio:
		{

			if(hCurrentEntity.IsValid())
			{
				m_wakeSleepCount--;

				if(m_wakeSleepCount <= 0)
				{
					Audio::InterfaceController::Get().SuspendEntity(hCurrentEntity);
					m_pCom->ReleaseEntityInstance();
				}
			}

		}
		break;
					

		default:
				if(!m_pCom->GetEntityHnd().IsValid())
					return true;
			return WorldObjectMessageHandler::ProcessMessage(info);
	}

	return true;
}


#endif //#ifndef NOT_USING_AUDIO_COMPONENETS
