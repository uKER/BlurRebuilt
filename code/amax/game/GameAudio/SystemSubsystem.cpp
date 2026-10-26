#include "stdafx.h"

#include "GameAudioLayer.h"

#include <GameApp.h>
#include "AppInterfaceLayer.h"
#include "SystemSubsystem.h"
#include "XtAudio.h"

#include <GameLogic\Profile\GameProfileManager.h>
//#include "ui/Game/Gadgets/ButtonMessages.h"
//
//#include <Ui/SoundEffects.h>

#include <GameLogic/Profile/GameProfile.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
#include "MusicSubsystem.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

Debug::Float g_MusicVolume("Audio.System.VolumeSettings.MusicVolume", 1,  Debug::Range(0.f, 1.f),Debug::Persist());
#ifdef PLATFORM_PC
// [Blur2PC] T144: retail PC 0x00e3d750; replaces the audio quality option when not -1
Debug::Int g_AudioDetailLevel("Audio.System.AudioDetailLevel", -1, Debug::Range(-1, 2), Debug::Persist());
#endif

//-----------------------------------------------------------------------------
// SystemSubsystem::SystemSubsystem
//-----------------------------------------------------------------------------
SystemSubsystem::SystemSubsystem()
{

	m_showingOverlay = false;
	m_disableSfxFades = false;
#ifdef PLATFORM_PC
	m_uAppliedAudioQuality = 0;	// [Blur2PC] T144: retail constructor 0x00c8d6e0
#endif
}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool SystemSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{
	switch (u32MessageId)
	{

		case Audio::eInit:
			{
				for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
				{
					Audio::EntityInstance* pInstance = m_aEntityInstance[i];
					if (pInstance)
					{
						Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("Suspend"));
						if(hActionOut != -1)
							Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(pInstance->GetAudioEntityHandle(), eSuspendCallback), hActionOut);
					}
				}

				SetSubscriberSystem( &Game::C_Game::GetInstance()->GetSubscriberSystem() );
				AddSubscriber(ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoEntry);
				AddSubscriber(ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoExit);
				AddSubscriber(Game::E_VehicleMessage_AudioWaterSplash);
			}
			break;

		case Audio::eMute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,0);
				}
			}
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eSuspend, 0xffffffff);
			break;

		case Audio::eUnmute:
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eResume, 0xffffffff);
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,1);
				}
			}
			break;

		case Audio::eLoadLevel:
			m_disableSfxFades = false;
			ProcessCustomMessage(eFadeOutMusic);
			ProcessCustomMessage(eFadeOutSfx);
			break;

		case Audio::eLoadUI:
		case Audio::eBeginLevel:
			m_disableSfxFades = false;
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					//raise the volume in system
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("BeginLevel"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn,false);


					S32	views = Game::C_Game::GetInstance()->GetLevelState().mNumViewports;
					
					hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("Viewports"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hActionIn,(F32)views);

					S32	cars = Game::C_Game::GetInstance()->GetLevelState().mNumCars;

					hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("CarsInRace"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hActionIn,(F32)cars);

					//send level params to all the default objects
					switch(Game::C_Game::GetInstance()->GetLevelState().mRaceMode)
					{
						case Game::E_RaceMode_Destruction:
							{
								Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("EventType"));
								Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn, 2);
							}
							break;

						case Game::E_RaceMode_Checkpoint:
							{
								Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("EventType"));
								Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn, 3);
							}
							break;

						case Game::E_RaceMode_BossBattle:
							{
								Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("EventType"));
								Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn, 4);
								
							}
							break;

						case Game::E_RaceMode_Multiplayer_Destruction:
							{
								Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("EventType"));
								Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn, 1);
								
							}
							break;

						default:
							{
								Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("EventType"));
								Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn, 0);
							}
							break;
					}

					//reset the music if it had previously slowed
					ProcessCustomMessage(ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoExit);

				}

				//fade in sounds
				ProcessCustomMessage(eFadeInMusic);
				ProcessCustomMessage(eFadeInSfx);
			}
			break;


		case Audio::eEndLevel:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					//send a mute message to system
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("EndLevel"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn,false);
				}

				ProcessCustomMessage(eFadeOutMusic);
				ProcessCustomMessage(eFadeOutSfx);
			}
			break;

		case Audio::eRaceFinished:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("RaceFinished"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn,false);
				}
			}
			break;

		default:
			break;
		}
	return true;
}

void SystemSubsystem::LookupComponentHandles()
{
	for (U32 uId = 0; uId < NumMessageIds; uId++)
	{
		Audio::EntityDataMemberHandle memberHandle = -1;
		memberHandle = Audio::InterfaceController::Get().LookupInstanceComponent( m_hSystemEntity, GetString( uId ) );
		if (memberHandle == -1)
			LOGWARN(Game::g_Log, DBSTR" could not lookup component '%s'", GetString( uId ));	
		m_aHandle.PushBack( memberHandle );
	}
}

bool SystemSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data, void* pData)
{
	switch (u32MessageId)
	{
		case eConnectToSystem:
		{
			// check entity type?
			if (pData)
			{
				AudioEntityHandle* phEntity = static_cast<AudioEntityHandle*>(pData);
				Audio::EntityDefinitionHandle hDef = Audio::InterfaceController::Get().GetDefinitionHandle(*phEntity);
				Audio::LocalFunctionHandle hFunc = Audio::InterfaceController::Get().LookupParameterisedFunctionById(hDef, "DoConnections", m_uSystemEntityDefId);
				// only try to connect if "DoConnections" func exists
				if (hFunc != 0xffffffff)
				{
					Audio::InterfaceController::Get().RunFunctionInParameterisedEntityInstance(*phEntity, m_hSystemEntity, hFunc);
				}
			}
			break;
		}

		case eInitSystem:
		{
			// create a system entity
			Util::C_String strSystem = "System";
			Audio::XtEntityProxy* pXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(strSystem);
			if (pXtEntity)
			{
				pXtEntity->SetupEntityType();

				if (pXtEntity->GetEntityTypeHnd().IsValid())
				{
					Audio::InterfaceController::Get().CreateEntity(pXtEntity->GetEntityTypeHnd(), m_hSystemEntity);
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "StartActionIn");

					// connect to renderer
					Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(pXtEntity->GetEntityDefHnd());
					if (pDef)
					{
						if (pDef->LookupInstanceComponent("OutputToRenderer") != 0xffffffff)
							Audio::InterfaceController::Get().SetMaster(m_hSystemEntity, "OutputToRenderer");

						m_uSystemEntityDefId = pDef->GetId();
					}

					// trigger the "Start" action in the entity
					Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, hActionIn);

					// store handle in AudioSystem class
					//C_GameApp::GetInstance()->GetAudioSys()->SetSystemEntityHnd(m_hSystemEntity);

				//do the VO osc paramOuts
					for(int i = 0; i < OSCILLOSCOPE_BANDS / 2; i++) // get low values
					{
						  char lowBuffer [50];
						  char highBuffer[50];

						 sprintf (lowBuffer, "Low%d", i);


						m_OscParamOutHandles[i] = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, lowBuffer);
						m_OscReturnDataHandles[i] = Audio::InterfaceController::Get().TrackValue(m_hSystemEntity, m_OscParamOutHandles[i]);

						//put in 2nd half of array
						 sprintf (highBuffer, "High%d", i);

						m_OscParamOutHandles[i+(OSCILLOSCOPE_BANDS / 2)] = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, highBuffer);
						m_OscReturnDataHandles[i+(OSCILLOSCOPE_BANDS / 2)] = Audio::InterfaceController::Get().TrackValue(m_hSystemEntity, m_OscParamOutHandles[i + (OSCILLOSCOPE_BANDS / 2)]);


					}

					LookupComponentHandles();
				}
			}
			break;
		}

		case eSuspendCallback:
		{

			//Audio::InterfaceController::Get().BroadcastMessage(Audio::eSuspend, 0xffffffff);
			//DebugBreak();
			break;
		}

		case eSetUIOptions:
		{
			if (pData)
			{

					Game::ProfileOptions* profileOptions = (Game::ProfileOptions*)pData;


					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_MusicVolume");
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,profileOptions->mAudioMusicVolume);

					hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_SFXVolume");
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,profileOptions->mAudioSfxVolume);

					C_GameApp::GetInstance()->GetGameAudioLayer()->SetSFXVolume(profileOptions->mAudioSfxVolume);

					hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_MenuMusicVolume");
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,profileOptions->mAudioMenuMusicVolume);

					hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_VoiceVolume");
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,profileOptions->mAudioVoiceVolume);

					hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_LFEBoost");
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,profileOptions->mAudiLFEeBoost);

					hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_DynamicRange");
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,profileOptions->mAudioDynamicRange);

					hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_MenuMusicMode");
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,profileOptions->mAudioMusicMode);

					hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_OutputMode");
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,profileOptions->mAudioOutputType);

					U8 MusicType = profileOptions->mAudioMusicType;
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::SetMusicType,MusicType );

#ifdef PLATFORM_PC
					// [Blur2PC] T144: retail PC (0x00c8c684) restarts the output engine at the rate of the audio
					// quality option when it changes: low 24000 Hz, medium 36000 Hz, high 48000 Hz. The engine
					// starts at 24000 Hz, which is also "last applied" at start. Retail looks the component up
					// and never sets it.
					Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_PCQualitySetting");
					m_uAudioQuality = profileOptions->mAudioQuality;
					if (g_AudioDetailLevel != -1)
						m_uAudioQuality = g_AudioDetailLevel;
					if (m_uAudioQuality != m_uAppliedAudioQuality)
					{
						m_uAppliedAudioQuality = m_uAudioQuality;

						U32 uSampleRate = 24000;
						if (m_uAudioQuality == 1)
							uSampleRate = 36000;
						else if (m_uAudioQuality == 2)
							uSampleRate = 48000;

						Audio::InterfaceController::Get().SetSystemOutputSampleRate(uSampleRate);
					}
#endif

					F32 RoomSize = 4; //in meters

					switch(profileOptions->mAudioRoomSize)
					{

						case 0:
							{
								RoomSize = 2;
							}

						break;

						case 1:
							{
								RoomSize = 4;
							}

						break;

						default:
						case 2:
							{
								RoomSize = 8;
							}

						break;

						case 3:
							{
								RoomSize = 25;
							}

						break;
					}


					//format speaker options
					//enum SpeakerOption
					//{
					//	eHeadphone,
					//	eStereoHiFi,
					//	eSurround,

					//	eNumSpeakerOptions
					//};

					//enum SpeakerConfig
					//{
					//	eStereo,
					//	eFivePointOne,
					//	eSevenPointOne,

					//	eNumSpeakerModes
					//};



					U32 SpeakerOption = 2;
					

					switch(profileOptions->mAudioOutputType)
					{

						case 0:
							{
								SpeakerOption = (int)Audio::ISystem::eHeadphone;
								Audio::InterfaceController::Get().SetSystemSpeakerDistance(SpeakerOption,4); //medium
							//	SpeakerConfig = (int)Audio::ISystem::eStereo;

							////profileOptions->
								Audio::InterfaceController::Get().SetSystemSpeakerAngles(Audio::ISystem::eFivePointOne,
									330.f,
									30.f,
									250.f,
									110.f,
									0.0,
									0.0);

							}

						break;

						default:
						case 1:
							{

								SpeakerOption = (int)Audio::ISystem::eSurround;

								//send angles to nick so he can offset UI sounds, offset from, what UI is sending by 90 degrees
								F32 FLSpeaker = profileOptions->mAudioSpeakerAngleFL + 90.0f;
								if(FLSpeaker > 180.0f)
									FLSpeaker -= 360.0f;
								hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_FLSpeakerAngle");
								Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,FLSpeaker/180.f);


								F32 FRSpeaker = profileOptions->mAudioSpeakerAngleFR + 90.0f;
								if(FRSpeaker > 180.0f)
									FRSpeaker -= 360.0f;
								hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_FRSpeakerAngle");
								Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,FRSpeaker/180.f);


								F32 RLSpeaker = profileOptions->mAudioSpeakerAngleRL + 90.0f;
								if(RLSpeaker > 180.0f)
									RLSpeaker -= 360.0f;
								hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_RLSpeakerAngle");
								Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,RLSpeaker/180.f);

								F32 RRSpeaker = profileOptions->mAudioSpeakerAngleRR + 90.0f;
								if(RRSpeaker > 180.0f)
									RRSpeaker -= 360.0f;
								hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hSystemEntity, "UIOptions_RRSpeakerAngle");
								Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, hParamIn,RRSpeaker/180.f);


								//set in audio system
								//C_GameApp::GetInstance()->GetGameAudioLayer()->GetSystem()->SetSpeakerOption(SpeakerConfig);

								//rescale values for system
								if(FLSpeaker < 0.0f)
									FLSpeaker += 360.0f;

								if(FRSpeaker < 0.0f)
									FRSpeaker += 360.0f;

								//HACK  NBygrave....:) fix up in panner at later stage
								if(FRSpeaker == 0.f)
									FRSpeaker = 0.001f;

								if(RLSpeaker < 0.0f)
									RLSpeaker += 360.0f;

								if(RRSpeaker < 0.0f)
									RRSpeaker += 360.0f;

								
								//HACK  NBygrave....:) fix up in panner at later stage
								if(RRSpeaker == 0.f)
									RRSpeaker = 0.001f;


								////profileOptions->
								Audio::InterfaceController::Get().SetSystemSpeakerAngles(Audio::ISystem::eFivePointOne,
									FLSpeaker,
									FRSpeaker,
									RLSpeaker,
									RRSpeaker,
									0.0,
									0.0);

							}

							Audio::InterfaceController::Get().SetSystemSpeakerDistance(SpeakerOption,RoomSize);

							break;


					}




					

					//commit the settings
					Audio::InterfaceController::Get().SetSystemSpeakerOption(SpeakerOption);
				}
					
					
			}
			break;


			case eDuckMusicLevel:
			{
				Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(DuckMusic),0);
			}

			break;

			case eRaiseMusicLevel:				
			{
				Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(DuckMusic),1);
			}
			break;

			case eFadeOutMusic:				
			{
				LOGDEBUG( Audio::g_Log, DBSTR" fade out music");
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(StartMusicFadeOut),false);
			}
			break;

			case eFadeOutSfx:				
			{
				if(!m_disableSfxFades)
				{
					LOGDEBUG(Audio::g_Log, DBSTR" fade out sfx");
					Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(StartSfxFadeOut),false);
				}

			}
			break;

			case eFadeInMusic:				
			{
				LOGDEBUG(Audio::g_Log, DBSTR" fade in music");
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(StartMusicFadeIn),false);
			}
			break;

			case eFadeInSfx:				
			{
				if(!m_disableSfxFades)
				{
					LOGDEBUG(Audio::g_Log, DBSTR" fade in sfx");
					Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(StartSfxFadeIn),false);
				}
			}
			break;

			case eFocusCarHeight:
			{
				if(pData)
				{
					F32* height = static_cast<F32*>(pData);
					F32 dfg = *height;
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(FocusCarHeight), dfg);
				}

			}
			break;

			case eEnterSloMo:
			{
				//Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(BigJump),1);
			}
			break;

			case eExitSloMo:
			{
				//Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(BigJump),0);
			}
			break;

			case eRGBMedium:
			{
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(RGBMedium),false);
			}
			break;

			case eRGBHard:
			{
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(RGBHard),false);
			}
			break;

			case eNearestCarDistance:
			{
				if (pData)
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(NearestCarDistance),*((F32 *)pData));
			}
			break;

			case eCarsInRadius:
			{
				if (pData)
					Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(CarsInRadius),*((F32 *)pData));
			}
			break;

			case eGamePausedQuit:
			{
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(GamePausedQuitActionIn),false);
			}
			break;

			
			case eEndofRaceFadeOutSfx:				
			{		
				m_disableSfxFades = true;
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(StartSfxFadeOut),false);
			}
			break;

			case eEndofRaceFadeInSfx:				
			{						
			//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(StartSfxFadeIn),false);
				m_disableSfxFades = false;
			}
			break;







	}

	return true;
}

void SystemSubsystem::Update()
{
	//music vol override
	Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(MusicVolumeOverride), g_MusicVolume);

	//silence game
	if(!C_GameApp::GetInstance()->GetGameAudioLayer()->TitleHasBGMusicControl())
	{
			Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(MusicVolumeOverride),0);
	}

	if(!m_showingOverlay && SysProfile::C_Manager::Instance().ShowingSystemOverlay())
	{
		ProcessCustomMessage(eFadeOutMusic);
		ProcessCustomMessage(eFadeOutSfx);
		m_showingOverlay = true;
	}
	else
	if(m_showingOverlay && !SysProfile::C_Manager::Instance().ShowingSystemOverlay())
	{
		ProcessCustomMessage(eFadeInMusic);
		ProcessCustomMessage(eFadeInSfx);
		m_showingOverlay = false;
	}
	



	//get Vo oscilloscope values
	for(int i = 0; i < OSCILLOSCOPE_BANDS ; i++)
	{
		F32 Value = 0.f;
		Audio::InterfaceController::Get().GetValue(Value, m_OscReturnDataHandles[i]);	
		//printf("\nBand %d, value%f:", i,Value);
		m_OscValues[ i ] = Maths::Clamp(Value,-1.0f,1.0f);;
	}

}

bool SystemSubsystem::MessageHandler(const Messaging::C_MessageBase *pMessage )
{

	switch(pMessage->Type())
	{
		case ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoEntry:
			//Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity, GetHandle(BigJump),1);
		break;

		case ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoExit:
			//Audio::InterfaceController::Get().SetEntityValue(m_hSystemEntity,GetHandle(BigJump),0);
		break;

		case Game::E_VehicleMessage_AudioWaterSplash:
			Audio::InterfaceController::Get().TriggerEntityActionIn(m_hSystemEntity, GetHandle(WaterJump),false);
		break;

	}
	return true;

}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


