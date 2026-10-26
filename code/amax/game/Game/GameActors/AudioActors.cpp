
#include "stdafx.h"
#ifndef NOT_USING_AUDIO_COMPONENETS

//#include "Audio/AudioSystem.h"

//#include <GameFlowBase.h>
#include <GameApp.h>
//#include <GameWorld/GameWorld.h>
#include "AudioActors.h"
//#include <GameStateMessages.h>
//#include "GameActors/Character/CharacterMessages.h"
//#include <GameActors/ActorMessages/LevelFlowMessages.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/SharedActorMessages.h>
#include "GameAudio/GameAudioLayer.h"
#include <Game/Messaging/GameflowMessages.h>

#include "ui/Game/Gadgets/ButtonMessages.h"

#include "Ui\SoundEffects.h"

#include <Game/Game.h>
#include <Game/Messaging/VehicleActorMessages.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
//
#include <GameLogic/ActorStudioActors/DynamicObjectActor.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <Gravity/SDK/ShapeDescriptors.h>
#include <Gravity/Triggers/TriggerSystem.h>
#include <gravity/triggers/TriggerEvent.h>

#include <shared_actors/SharedActorUtils.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/GameData/GameData.h>

#include <camera2/stock/DebugFeed.h>
#include <game/AmaxCam/VehicleFeed.h>
#include <game/AmaxCam/CameraSystem.h>

#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"


#include "GameAudio\SystemSubsystem.H"
#include "Game/GameActors/Components/Messages/AudioMessages.h"

#include <GameLogic\Messaging\InGameDialogueMessages.h>

#include "GameLogic/Layers/RaceLogicLayer.h"
#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"
#include "shared_actors\components\interfaces\I_PhysicsBodyComponents.h"
#include <GameLogic/ActorStudioActors/PhysicalObjectActor.h>

#include <Game/Game.h>

#include "Core/Maths/HVec/New/IncludeGeometry.h"
#include "Core/Maths/HVec/New/IncludeGeometry.inl"

#include <GameLogic\ActorStudioActors\TriggerRegionActor.h>
#include <camera2/Camera.h>

DEBUG_ENUM(::Game::E_AUDIO_MASS_TYPE)
DE_OPTION(Game::E_AUDIO_MASS_TYPE_NONE, "None")
DE_OPTION(Game::E_AUDIO_MASS_TYPE_LIGHT, "Light")
DE_OPTION(Game::E_AUDIO_MASS_TYPE_MEDIUM, "Medium")
DE_OPTION(Game::E_AUDIO_MASS_TYPE_HEAVY, "Heavy")
DE_OPTION(Game::E_AUDIO_MASS_TYPE_VERY_HEAVY, "VeryHeavy")
DE_OPTION(Game::E_AUDIO_MASS_TYPE_IMMOVABLE, "Immovable")
END_DEBUG_ENUM

Debug::Float gfTriggerSphereRadius("Audio.Actors.Listener.TriggerSphereRadius", 10.0f);
Debug::Float gfPickupTriggerSphereRadius("Audio.Actors.Listener.PickupTriggerSphereRadius", 50.0f);
Debug::Enum<Game::E_AUDIO_MASS_TYPE> g_eStaticObjThreshold("Audio.Actors.Listener.StationaryObjTriggerMass", Game::E_AUDIO_MASS_TYPE_NONE);
Debug::Float gfOverheadTriggerSphereRadius("Audio.Actors.Listener.OverheadTriggerSphereRadius", 1.0f);
Debug::Float gfOverheadTriggerDelay("Audio.Actors.Listener.OverheadTriggerSphereTriggerDelay", .15f);
Debug::Float gfOverheadTriggerOffsetAngle("Audio.Actors.Listener.OverheadTriggerSphereOffsetAngle", -20.f);


using namespace Util;
using namespace Maths;

XT_TYPE(Game::Actors::C_AudioActorCreate, "AudioActor")
XT_BASE(Actors::C_PosActorCreate)
XT_FIELD(m_pSorceryMovementComponentCreate, "MovementComponent")
XT_FIELD(m_SorceryAudioComponent, "AudioComponent")
XT_END_TYPE

XT_TYPE(Game::Actors::C_MusicActorCreate, "MusicActor")
XT_BASE(Actors::C_GameActorCreate)
XT_END_TYPE

XT_TYPE(Game::Actors::C_UIAudioActorCreate, "UIAudioActor")
XT_BASE(Actors::C_GameActorCreate)
XT_END_TYPE

XT_TYPE(Game::Actors::C_ListenerAudioActorCreate, "ListenerAudioActor")
XT_BASE(Actors::C_GameActorCreate)
XT_END_TYPE

//#define AUDIO_SHOW_PROXIMITY_TRAJECTORIES

#if defined( AUDIO_SHOW_PROXIMITY_TRAJECTORIES )
Maths::HVec3 g_debugPoint1;
Maths::HVec3 g_debugPoint2;
#endif

namespace Game
{
	namespace Actors
	{

		void C_AudioActorCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
		{
			C_PosActorCreate::DoSorceryCallback(pCallbackFn);

			if (m_pSorceryMovementComponentCreate != NULL)
				DoComponentSorceryCallback(pCallbackFn, *m_pSorceryMovementComponentCreate, &m_pMovementCreator);

			DoComponentSorceryCallback(pCallbackFn, m_SorceryAudioComponent, &m_pAudioCreator);
		}

		// Audio Actor
		// -----------
		bool C_AudioActor::InitialiseImpl()
		{
			const C_AudioActorCreate *pAudioActorCreate =  static_cast<const C_AudioActorCreate*>(GetCreationData().GetData());
			
			m_WorldMatrix = pAudioActorCreate->GetCreationMatrix();

			// Create Audio Component
			if (InitialiseComponent(pAudioActorCreate->m_pAudioCreator) == NULL)
				return false;

			// Create movement component:
			if (pAudioActorCreate->GetMovementCreator() != NULL)
			{
				if (!InitialiseComponent(pAudioActorCreate->m_pMovementCreator))
					return false;
			}
			return true;
		}

		bool C_AudioActor::CreateImpl()
		{
			if(!C_GameActor::CreateImpl())
				return false;

			//const C_AudioActorCreate *pAudioActorCreate =  static_cast<const C_AudioActorCreate*>(GetCreationData().GetData());

			AddSubscriber( E_GameflowMessage_EnteredDebugMenu );
			AddSubscriber( E_GameflowMessage_StartRaceLoad );
			AddSubscriber( E_GameflowMessage_RaceStarted );
			AddSubscriber( E_GameflowMessage_RaceFinished );
			AddSubscriber( E_GameflowMessage_StartUILoad );
			AddSubscriber( E_GameflowMessage_UILoaded );
			AddSubscriber( E_GameflowMessage_RaceLoaded );
			AddSubscriber( E_GameflowMessage_ResultsCalculated );
			AddSubscriber( E_GameflowMessage_QuitRace );
			AddSubscriber( E_GameflowMessage_SaveGameLoaded );



			return true;
		}

		bool C_AudioActor::OnMessage(const Messaging::C_MessageBase *pMessage )
		{
			switch(pMessage->Type())
			{
				case ActorMsg::E_GameActorMsg_SetTransform:
					{
						m_WorldMatrix = Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage)->Transform().Matrix();
						break;
					}

				case ActorMsg::E_GameActorMsg_AnimateTransform:
					{
						m_WorldMatrix = Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage)->Transform().Matrix();
						break;
					}
				default:
					return C_GameActor::OnMessage(pMessage);
			}

			return true;
		}

		void C_AudioActor::Serialise(Sys::C_ContextualStream& stream) const
		{
			stream << m_WorldMatrix;
		}

		void C_AudioActor::Deserialise(Sys::C_ContextualStream& stream)
		{
			stream >> m_WorldMatrix;
		}

		void C_AudioActor::SerialiseForSave(Sys::C_ContextualStream& stream) const
		{
			C_GameActor::SerialiseForSave(stream);
			Serialise(stream);
		}

		void C_AudioActor::DeserialiseForSave(Sys::C_ContextualStream& stream)
		{
			C_GameActor::DeserialiseForSave(stream);
			Deserialise(stream);
		}

		void C_AudioActor::SerialiseForSync(Sys::C_ContextualStream& stream) const
		{
			C_GameActor::SerialiseForSync(stream);
			Serialise(stream);
		}

		void C_AudioActor::DeserialiseForSync(Sys::C_ContextualStream& stream)
		{
			C_GameActor::DeserialiseForSync(stream);
			Deserialise(stream);
		}
	
		// Music Actor
		// -----------
		bool C_MusicActor::CreateImpl()
		{
			if(!C_GameActor::CreateImpl())
				return false;

			m_hMusicEntity = C_GameApp::GetInstance()->GetGameAudioLayer()->GetActorEntityHnd(Game::C_GameAudioLayer::eMusicSubsystem);

			AddSubscriber( E_GameflowMessage_EnteredDebugMenu );

			return true;
		}

		bool C_MusicActor::OnMessage(const Messaging::C_MessageBase *pMessage )
		{
			
			return true;

		}


		// UIAudio Actor
		// -----------

		void C_UIAudioActor::Create(Messaging::C_MessageSubscriberSystem* pSubscriberSystem)
		{
			//SetSubscriberSystem( pSubscriberSystem );
			//CreateImpl();

		}
		bool C_UIAudioActor::CreateImpl()
		{

			return true;
		}

		void C_UIAudioActor::Update()
		{

		}

		bool C_UIAudioActor::OnMessage(const Messaging::C_MessageBase *pMessage )
		{

			return true;

		}

			// ListenerAudio Actor
		// -----------

		void C_ListenerAudioActor::Create(Messaging::C_MessageSubscriberSystem* pSubscriberSystem)
		{
			SetSubscriberSystem( pSubscriberSystem );
			CreateImpl();
		}
		bool C_ListenerAudioActor::CreateImpl()
		{
			if(!C_GameActor::CreateImpl())
				return false;

			m_numViewports = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();

			//m_hListenerAudioEntity = C_GameApp::GetInstance()->GetGameAudioLayer()->GetActorEntityHnd(Game::C_GameAudioLayer::eListenerSubsystem);
			PopulateEntities();

			//AddSubscriber( Camera::E_CameraMsg_PlaySoundEffect );
			AddSubscriber( Game::E_VehicleMessage_TriggerActorEvent );
			AddSubscriber( GameMessaging::E_GameMsg_Update );

			m_pObjectTriggerActor = NULL;
			m_pOverheadObjectTriggerActor = NULL;

			for(int i = 0; i< MAX_LISTENER_VIEWPORTS;i++)
				m_pPickupTriggerActor[i] = NULL;

			if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer())
				gameworld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();
			BIZ_ASSERT(gameworld);
			
			//CreateTrigger();

			m_DraftingPrev = false;
			m_currentEntityIndex = 0;
			m_currentTunnelEntityIndex = 0;

			return true;
		}

		C_ListenerAudioActor::~C_ListenerAudioActor()
		{
			Destroy();

		}

		void C_ListenerAudioActor::Destroy()
		{
			for(int i = 0; i < MAX_LISTENER_ENTITIES; i++)
			{
				Audio::InterfaceController::Get().ReleaseEntity(m_hListenerAudioEntity[i]);
				m_hListenerAudioEntity[i].Zero();
			}

			RemoveAllSubscribers();
			DestroyTrigger();
		}

		void C_ListenerAudioActor::PopulateEntities()
		{

			//Util::C_String string = "Listener";
			//pXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(string);
			Util::C_String string = "Listener";

			Audio::XtEntityProxy* pXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(string);
			//if (pXtEntity)
			//{
			//	Game::CacheEntityResources(*pXtEntity, "ListenerSubsystem");
			//}

			if (pXtEntity)
			{
				pXtEntity->SetupEntityType();
				if (pXtEntity->GetEntityTypeHnd().IsValid())
				{

					for(int i = 0; i < MAX_LISTENER_ENTITIES; i++)
					{
						Audio::InterfaceController::Get().CreateEntity(pXtEntity->GetEntityTypeHnd(), m_hListenerAudioEntity[i]);
						Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[i], "StartActionIn");

						// connect to system entity
						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &m_hListenerAudioEntity[i]);

						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hListenerAudioEntity[i], hActionIn);

						// temp. store handle in AudioSystem class
						C_GameApp::GetInstance()->GetGameAudioLayer()->SetActorEntityHnd(Game::C_GameAudioLayer::eListenerSubsystem,m_hListenerAudioEntity[i]);
					}
				}
			}

		}

		//=========================================================================

		void C_ListenerAudioActor::CreateTrigger()
		{
			Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
			//Maths::HMat43 camMatrix = view.DebugFeed().Mat43();

		
				Gravity::C_TriggerLayer* pTriggerLayer = gameworld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects);
				//only do for splitscreen
				if (pTriggerLayer && m_numViewports == 1)
				{

					{

						U32 data[1024];

						SharedPhysics::C_SafeStack<true> stack(data, 1024);	
						m_pObjectTriggerActor = new Gravity::C_TriggerActor;
						Collision::C_CollisionFilter filter;

						F32 x = 1;
						F32 y = 1;
						F32 z = .1;

						Maths::HVec3 size(x,y,z);
						F32 radius32 = gfTriggerSphereRadius;
						const Maths::HF32 radius = radius32;
						m_pObjectTriggerActor->m_pShape = new Gravity::C_TriggerSphere(radius, &filter);//m_BodyCollisionFilter);
						
						//m_pObjectTriggerActor->m_pShape = new Gravity::C_TriggerBox(size * 3, &filter);//m_BodyCollisionFilter);
						Actor::C_ActorHandle handle = GetHandle();
						m_UserData.m_hActorHandle = handle;
						m_pObjectTriggerActor->SetUserData((void*)&m_UserData);
						gameworld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects)->AddTriggerActor(m_pObjectTriggerActor);
						//m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects)->SetMatrix(m_pObjectTriggerActor, pActor->GetMatrix(), stack);
						m_pObjectTriggerActor->SetMatrix(camMatrix, stack);
					}
				}

				Gravity::C_TriggerLayer* pTriggerLayerPickups = gameworld->GetTriggerLayer(Game::E_TriggerLayer_Pickup);
				if (pTriggerLayerPickups)
				{

					//do for each view in splitscreen
					for(int i = 0; i< (int)m_numViewports;i++)
					{

						U32 data[1024];

						SharedPhysics::C_SafeStack<true> stack(data, 1024);	
						m_pPickupTriggerActor[i] = new Gravity::C_TriggerActor;
						Collision::C_CollisionFilter filter;

						F32 x = 1;
						F32 y = 1;
						F32 z = .1;

						Maths::HVec3 size(x,y,z);
						F32 radius32 = gfPickupTriggerSphereRadius;
						const Maths::HF32 radius = radius32;
						m_pPickupTriggerActor[i]->m_pShape = new Gravity::C_TriggerSphere(radius, &filter);//m_BodyCollisionFilter);
						
						//m_pObjectTriggerActor->m_pShape = new Gravity::C_TriggerBox(size * 3, &filter);//m_BodyCollisionFilter);
						Actor::C_ActorHandle handle = GetHandle();
						m_UserData.m_hActorHandle = handle;
						m_pPickupTriggerActor[i]->SetUserData((void*)&m_UserData);
						gameworld->GetTriggerLayer(Game::E_TriggerLayer_Pickup)->AddTriggerActor(m_pPickupTriggerActor[i]);
						//m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects)->SetMatrix(m_pObjectTriggerActor, pActor->GetMatrix(), stack);
						m_pPickupTriggerActor[i]->SetMatrix(camMatrix, stack);
					}
				}

				//create trigger for overhead 
				Gravity::C_TriggerLayer* pTriggerRegionLayer = gameworld->GetTriggerLayer(Game::E_TriggerLayer_TriggerRegion);
				if (pTriggerRegionLayer  && m_numViewports == 1)
				{

					{

						U32 data[1024];

						SharedPhysics::C_SafeStack<true> stack(data, 1024);	
						m_pOverheadObjectTriggerActor = new Gravity::C_TriggerActor;
						Collision::C_CollisionFilter filter;

						F32 x = 1;
						F32 y = 1;
						F32 z = .1;

						Maths::HVec3 size(x,y,z);
						F32 radius32 = gfOverheadTriggerSphereRadius;
						const Maths::HF32 radius = radius32;

						static F32 x2 = 1.5;
						static F32 y2 = 0.5;
						static F32 z2 = 2.5;

						Maths::HVec3 extents(x2,y2,z2);
						m_pOverheadObjectTriggerActor->m_pShape = new Gravity::C_TriggerBox(extents, &filter);			
						//m_pOverheadObjectTriggerActor->m_pShape = new Gravity::C_TriggerSphere(radius, &filter);		
						
						Actor::C_ActorHandle handle = GetHandle();
						m_UserData.m_hActorHandle = handle;
						m_pOverheadObjectTriggerActor->SetUserData((void*)&m_UserData);
						pTriggerRegionLayer->AddTriggerActor(m_pOverheadObjectTriggerActor);
						//m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects)->SetMatrix(m_pObjectTriggerActor, pActor->GetMatrix(), stack);
						m_pOverheadObjectTriggerActor->SetMatrix(camMatrix, stack);
					}
				}
		}

		void C_ListenerAudioActor::DestroyTrigger(/*const Actor::C_ActorHandle& actorHandle*/)
		{
			Gravity::C_TriggerLayer* pTriggerLayer = gameworld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects);
			if (pTriggerLayer && m_pObjectTriggerActor && m_numViewports == 1)
			{				
				m_UserData.m_hActorHandle = GetHandle();
				m_pObjectTriggerActor->SetUserData((void*)&m_UserData);
				
				pTriggerLayer->RemoveTriggerActor(m_pObjectTriggerActor);

				SAFE_DELETE_PTR(m_pObjectTriggerActor);
				m_pObjectTriggerActor = NULL;
			}

			Gravity::C_TriggerLayer* pTriggerLayerPickups = gameworld->GetTriggerLayer(Game::E_TriggerLayer_Pickup);

			//do for each view in splitscreen
			for(int i = 0; i< (int)m_numViewports;i++)
			{
				if (pTriggerLayerPickups && m_pPickupTriggerActor[i])
				{				
					m_UserData.m_hActorHandle = GetHandle();
					m_pPickupTriggerActor[i]->SetUserData((void*)&m_UserData);
					
					pTriggerLayerPickups->RemoveTriggerActor(m_pPickupTriggerActor[i]);

					SAFE_DELETE_PTR(m_pPickupTriggerActor[i]);
					m_pPickupTriggerActor[i] = NULL;
				}
			}

			//delete trigger region actors
			Gravity::C_TriggerLayer* pTriggerRegionLayer = gameworld->GetTriggerLayer(Game::E_TriggerLayer_TriggerRegion);
			if (pTriggerRegionLayer && m_pOverheadObjectTriggerActor && m_numViewports == 1)
			{				
				m_UserData.m_hActorHandle = GetHandle();
				m_pOverheadObjectTriggerActor->SetUserData((void*)&m_UserData);
				
				pTriggerRegionLayer->RemoveTriggerActor(m_pOverheadObjectTriggerActor);

				SAFE_DELETE_PTR(m_pOverheadObjectTriggerActor);
				m_pOverheadObjectTriggerActor = NULL;
			}
		}



		void C_ListenerAudioActor::Update()
		{

				//make the  listener pos be the focus car
				U8 focusVehicle = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetViewportFocusVehicleIndex(0);
				Game::Actors::C_VehicleLogicActor* pMe	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor( focusVehicle );
				
				Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();

				m_linearVelocity =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();
				F32 listenerSpeed = m_linearVelocity.Length();

				//**calcualte the offset of the trigger sphere based on listner speed**//

				//we want to hear a whoosh pass us roughly .125 secs into the sample (generally after the transient in the woosh content we have currently)

				Maths::HMat43 carOffsetMatrix = camMatrix;
				
				if(pMe)
					carOffsetMatrix.AW(pMe->GetPositionInfo().mWorldPosition);
				
				for(int i = 0; i < MAX_LISTENER_ENTITIES; i++)
				{
					
					Audio::EntityDataMemberHandle 	hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[i], "ListenerSpeed");
					Audio::InterfaceController::Get().SetEntityValue(m_hListenerAudioEntity[i], hVector,(F32)listenerSpeed);
				}

				//only send wind for single screen
				if(m_numViewports == 1)
				{
					//send listenerSpeed to the 1st entity only...a bit of a hack but we only want one instance of this
					Audio::EntityDataMemberHandle 	hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[0], "WindSpeed");
					Audio::InterfaceController::Get().SetEntityValue(m_hListenerAudioEntity[0], hVector,(F32)listenerSpeed);
				}
							
				//flyby triggers
				if(m_pObjectTriggerActor  && m_numViewports == 1)
				{
						//::Actors::DynamicObjectActor *pActor = (::Actors::DynamicObjectActor *)Actor::ActorCast< ::Actors::DynamicObjectActor >(info.pActor);
											
					
						Gravity::C_TriggerLayer* pTriggerLayer = gameworld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects);

						if(pTriggerLayer)
						{
							U32 data[1024];
							SharedPhysics::C_SafeStack<true> stack(data, 1024);	

							m_pObjectTriggerActor->SetMatrix(carOffsetMatrix, stack);
						}
						

				}

				Gravity::C_TriggerLayer* pTriggerLayer = gameworld->GetTriggerLayer(Game::E_TriggerLayer_Pickup);
				//do for each view in splitscreen
				for(int i = 0; i< (int)m_numViewports;i++)
				{
							if(m_pPickupTriggerActor[i])
					{
							//::Actors::DynamicObjectActor *pActor = (::Actors::DynamicObjectActor *)Actor::ActorCast< ::Actors::DynamicObjectActor >(info.pActor);
								
							if(pTriggerLayer)
							{
								U32 data[1024];
								SharedPhysics::C_SafeStack<true> stack(data, 1024);	
								//checking splitScreenView is aware of the num of views before getting matrix..
								S32 count = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
								if(count == m_numViewports)
									camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43();
								m_pPickupTriggerActor[i]->SetMatrix(camMatrix, stack);
							}
							

					}
				}

				//region triggers
				if(m_pOverheadObjectTriggerActor  && m_numViewports == 1)
				{
					//depending on speed,move sphere in front of the car/camera
					Maths::HMat43 overheadOffsetMatrix = carOffsetMatrix;

					F32 triggerDistance = listenerSpeed * gfOverheadTriggerDelay;

					F32 yOffsetAngle =  gfOverheadTriggerOffsetAngle; //degrees

					//create a rotation matrix around the X axis by yOffsetAngle
					const Maths::HMat33 rotateX(Maths::RotX(Maths::DegToRad(yOffsetAngle)));

					//create a vector of triggerDistance along Z axis
					Maths::HVec3 ZVector(0,0,triggerDistance);

					//get the Z axis offset by the rotation matrix
					Maths::HVec3 offsetLocalMatrix = ZVector * rotateX;

					//apply offset to the original martix 
					Maths::HVec3 offsetVector = offsetLocalMatrix * overheadOffsetMatrix;

					//set new position
					overheadOffsetMatrix.AW(offsetVector);

					U32 data[1024];
					SharedPhysics::C_SafeStack<true> stack(data, 1024);	

					m_pOverheadObjectTriggerActor->SetMatrix(overheadOffsetMatrix, stack);
				}
				
#if defined( AUDIO_SHOW_PROXIMITY_TRAJECTORIES )
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine( g_debugPoint1, g_debugPoint2, Maths::RGBA( 255, 255, 255, 255 ) );
#endif
		}


		bool C_ListenerAudioActor::OnMessage(const Messaging::C_MessageBase *pMessage )
		{


				switch(pMessage->Type() )
				{

					case GameMessaging::E_GameMsg_Update:
						Update();
						break;

					case Game::E_VehicleMessage_TriggerActorEvent:
					{
						const Game::Actors::C_VehicleCheckTriggerActorEvent* pMsg	= Messaging::MessageCast<Game::Actors::C_VehicleCheckTriggerActorEvent>(pMessage);

						//if(pMsg->pActor->GetControlType() == ::E_VEHICLE_CONTROL_TYPE_PLAYER)
						{

							if(pMsg->mTriggerObject->GetActorTypeId() ==  E_ActorTypeId_TriggerRegion  && m_numViewports == 1)
							{		
								const ::Actors::TriggerRegionActor* pTriggerObject = (const ::Actors::TriggerRegionActor*)pMsg->mTriggerObject;

								if(pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Enter_Event)
								{
									
									if(pTriggerObject->GetCreationData() && pTriggerObject->GetCreationData()->m_VehicleMessage)
									{
										const ActorMsg::C_VehicleEnteredTriggerMsg* pEnterMsg = Messaging::MessageCast<ActorMsg::C_VehicleEnteredTriggerMsg>(pTriggerObject->GetCreationData()->m_VehicleMessage->GetMessage(pTriggerObject));

										//if it's one of the actorStudio placed tunnels...
										if(pEnterMsg->m_TriggerType == Game::E_TRIGGER_TYPE_TUNNEL_ENTER)
										{
										
											m_currentTunnelEntityIndex = ++m_currentTunnelEntityIndex%MAX_LISTENER_ENTITIES;

											Audio::EntityDataMemberHandle hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentTunnelEntityIndex], "ReferenceValue");
											Audio::InterfaceController::Get().SetEntityValue(m_hListenerAudioEntity[m_currentTunnelEntityIndex], hHandle,(F32)pEnterMsg->m_RefNumber);

											hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentTunnelEntityIndex], "TunnelEnterActionIn");
											Audio::InterfaceController::Get().TriggerEntityActionIn(m_hListenerAudioEntity[m_currentTunnelEntityIndex], hHandle);
										}
									}
								}
								else
								if(pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Exit_Event)
								{
									if(pTriggerObject->GetCreationData() && pTriggerObject->GetCreationData()->m_VehicleMessage)
									{
										const ActorMsg::C_VehicleExitedTriggerMsg* pExitMsg = Messaging::MessageCast<ActorMsg::C_VehicleExitedTriggerMsg>(pTriggerObject->GetCreationData()->m_VehicleMessageOnExit->GetMessage(pTriggerObject));

										//if it's one of the actorStudio placed tunnels...
										if(pExitMsg->m_TriggerType == Game::E_TRIGGER_TYPE_TUNNEL_EXIT)
										{
											Audio::EntityDataMemberHandle hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentTunnelEntityIndex], "ReferenceValue");
											Audio::InterfaceController::Get().SetEntityValue(m_hListenerAudioEntity[m_currentTunnelEntityIndex], hHandle,(F32)pExitMsg->m_RefNumber);

											hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentTunnelEntityIndex], "TunnelExitActionIn");
											Audio::InterfaceController::Get().TriggerEntityActionIn(m_hListenerAudioEntity[m_currentTunnelEntityIndex], hHandle);
										}
									}
								}
							}
							else
							{

								
								if(pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Enter_Event)
								{
								
									Maths::HVec3 linearVelocity(0,0,0);
									Maths::HVec3 angularVelocity(0,0,0);
									Maths::HVec3 pos(0,0,0);
									Maths::HVec3 dir(0,0,0);

									//make the camera matrix position be the focus car
									U8 focusVehicle = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetViewportFocusVehicleIndex(0);
									Game::Actors::C_VehicleLogicActor* pMe	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor( focusVehicle );

									//Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
									
									Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
									
									if(pMe)
										camMatrix.AW(pMe->GetPositionInfo().mWorldPosition);

									Maths::HVec3 spherePos = camMatrix.AW();

									U32 mass = 0;
									if(pMsg->mTriggerObject->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject  && m_numViewports == 1)
									{
										::Actors::DynamicObjectActor *pActor = (::Actors::DynamicObjectActor *)Actor::ActorCast< ::Actors::DynamicObjectActor >(pMsg->mTriggerObject);
										if(pActor->m_pBody)
										{
											linearVelocity = pActor->m_pBody->LinearVelocity();

											angularVelocity = pActor->m_pBody->AngularVelocity();

											mass = (U32)pActor->GetAudioMassType();
										}

										if( pActor->GetTrigger()->m_pTriggerShape->GetShape() == Collision::E_Sphere_Shape )
										{
											Collision::C_SphereShape* pSphere = ( Collision::C_SphereShape* )( pActor->GetTrigger()->m_pTriggerShape );
											pos = pSphere->m_Sphere.Centre( );

											//	intersection line is car centre -> DO/PO centre vector
											dir = pos - spherePos;
											dir.Normalise( );

											//	point of intersection between spheres
											pos -= pSphere->m_Sphere.Radius( ) * dir;
										}
									}
									else if(pMsg->mTriggerObject->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject  && m_numViewports == 1)
									{
										::Actors::C_PhysicalObject *pActor = (::Actors::C_PhysicalObject *)Actor::ActorCast< ::Actors::C_PhysicalObject >(pMsg->mTriggerObject);
										const Components::I_PhysicsBody* pIBody = pActor->PhysicsBodyComponent();
										const Gravity::C_Body* pBody = pIBody->CollideBody();
										if(pBody)
										{
											linearVelocity = pBody->LinearVelocity();

											angularVelocity = pBody->AngularVelocity();

											mass = (U32)pActor->GetAudioMassType();

											if( pActor->GetTrigger()->m_pTriggerShape->GetShape() == Collision::E_Sphere_Shape )
											{
												Collision::C_SphereShape* pSphere = ( Collision::C_SphereShape* )( pActor->GetTrigger()->m_pTriggerShape );
												pos = pSphere->m_Sphere.Centre( );

												//	intersection line is car centre -> DO/PO centre vector
												dir = pos - spherePos;
												dir.Normalise( );

												//	point of intersection between spheres
												pos -= pSphere->m_Sphere.Radius( ) * dir;
											}
										}
									}
									else
										break;

									//	cull light objects if they are not moving
									if(mass < (U32)g_eStaticObjThreshold && linearVelocity.LengthSquared() < 0.01f )
										break;
								
									if( dir.LengthSquared() < 0.001f )
										break;
															
									Maths::HVec3 relativeVel = linearVelocity - m_linearVelocity;

									F32 angularSpeed = angularVelocity.Length();

									F32 relativeSpeed = relativeVel.Length();

									F32 speed = linearVelocity.Length();
				

									// AUDIOTODO: move these lookups out of the update

									Audio::EntityDataMemberHandle hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectSpinSpeed");
									Audio::InterfaceController::Get().SetEntityValue(m_hListenerAudioEntity[m_currentEntityIndex],hHandle,angularSpeed);

									hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectFlyBySpeed");
									Audio::InterfaceController::Get().SetEntityValue(m_hListenerAudioEntity[m_currentEntityIndex],hHandle,speed);
									
									hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectFlyByRelativeSpeed");
									Audio::InterfaceController::Get().SetEntityValue(m_hListenerAudioEntity[m_currentEntityIndex], hHandle,relativeSpeed);
									
									hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectFlyByMass");
									Audio::InterfaceController::Get().SetEntityValue(m_hListenerAudioEntity[m_currentEntityIndex], hHandle,(F32)mass);
									
									//hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectFlyByPosition");
									//Audio::InterfaceController::Get().SetEntityVectorIn(m_hListenerAudioEntity[m_currentEntityIndex], hHandle,(F32*)&pos);
									
									hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectFlyByDirecton");
									Audio::InterfaceController::Get().SetEntityVectorIn(m_hListenerAudioEntity[m_currentEntityIndex], hHandle,(F32*)&dir);

									//send 3D info
									Audio::EntityDataMemberHandle  hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ListenerPosition");
									Audio::InterfaceController::Get().SetEntityVectorIn(m_hListenerAudioEntity[m_currentEntityIndex], hVector,(F32*)&camMatrix.AW());

									 hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ListenerDirection");
									Audio::InterfaceController::Get().SetEntityVectorIn(m_hListenerAudioEntity[m_currentEntityIndex], hVector,(F32*)&camMatrix.AZ());

									 hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ListenerUp");
									Audio::InterfaceController::Get().SetEntityVectorIn(m_hListenerAudioEntity[m_currentEntityIndex], hVector,(F32*)&camMatrix.AY());

									hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ListenerVelocity");
									Audio::InterfaceController::Get().SetEntityVectorIn(m_hListenerAudioEntity[m_currentEntityIndex], hVector,(F32*)&m_linearVelocity);

									//	create car sphere
								   const Maths::Point3 centre = spherePos;
								   const Maths::VF32 radius = Maths::MakeVF32( gfTriggerSphereRadius );
								   const Maths::Sphere sphere = Maths::MakeSphere( centre, radius );

								   //	DO/PO (relative) move direction vector
								   Maths::HVec3 trajectory = relativeVel;
								   trajectory.Normalise();

								   //	create a line long enough to intersect with both sides of the car sphere
								   Maths::Point3 point1 = pos - gfTriggerSphereRadius * trajectory;
								   Maths::Point3 point2 = pos + ( 3.f * gfTriggerSphereRadius ) * trajectory;

								   Maths::QuadraticResult quad;

									//	TODO: see why the sphere intersect fails sometimes, possibly legit?
								   if( !Maths::Intersect( sphere, point1, point2, &quad ) )
									   break;

	#if defined( AUDIO_SHOW_PROXIMITY_TRAJECTORIES )
								   g_debugPoint1 = Maths::HVec3( point1 );
								   g_debugPoint2 = Maths::HVec3( point2 );
	#endif

								   Maths::HVec3 entry = Maths::HVec3( point1 + quad.m_X1 * ( point2 - point1 ) );
								   Maths::HVec3 exit = Maths::HVec3( point1 + quad.m_X2 * ( point2 - point1 ) );

									hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectFlyByPosition");
									Audio::InterfaceController::Get().SetEntityVectorIn(m_hListenerAudioEntity[m_currentEntityIndex], hHandle,(F32*)&entry);

									hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectFlyByExitPosition");
									Audio::InterfaceController::Get().SetEntityVectorIn(m_hListenerAudioEntity[m_currentEntityIndex], hHandle,(F32*)&exit);

									//get midpoint of intersecting line, should be closest point to centre of sphere
									Maths::HVec3 midPoint = entry + ( exit - entry ) * 0.5f;

									//get distance from centre of sphere to the midpoint of the cord
									F32 midPointDistance = spherePos.Distance(midPoint);

									hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectFlyByNearestPoint");
									Audio::InterfaceController::Get().SetEntityValue(m_hListenerAudioEntity[m_currentEntityIndex], hHandle,midPointDistance);

									hHandle = Audio::InterfaceController::Get().LookupInstanceComponent(m_hListenerAudioEntity[m_currentEntityIndex], "ObjectFlyByTrigger");
									Audio::InterfaceController::Get().TriggerEntityActionIn(m_hListenerAudioEntity[m_currentEntityIndex], hHandle,false);

									m_currentEntityIndex = ++m_currentEntityIndex%MAX_LISTENER_ENTITIES;


								}
								else if (pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Exit_Event)
								{
									//Undo any state changes made in the above enter event in here - we have left the trigger region!
								}
							}
						}

						break;
					}
				}

			return true;

		}
	}
}
#endif
