#include "stdafx.h"

#include "GameAudioLayer.h"
#include "AppInterfaceLayer.h"
#include "DefaultSubsystem.h"

#include "GameAudio\SystemSubsystem.H"
#include "Game/GameActors/Components/VehicleLogic/VLC_Pickups.h"
#include <game/AmaxCam/CameraSystem.h>

#include <game/AmaxCam/VehicleFeed.h>
#include "GameApp.h"

#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"
#include <camera2/stock/DebugFeed.h>

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF


Debug::Bool g_bTestObject("Audio.SubSystem.Default.Testobject.EnableTestObject", false, Debug::Persist());
Debug::Bool g_bTestObjectCreate("Audio.SubSystem.Default.Testobject.EnableTestObjectCreate", false);
Debug::Bool g_bTestObjectDelete("Audio.SubSystem.Default.Testobject.EnableTestObjectDelete", false);

Debug::Bool g_bLoadDefaultMines("Audio.SubSystem.Default.LoadMines", false, Debug::Persist());

int m_testEntityIndex = 0;

#define MAX_MINES_AT_ONCE 60  //20 cars x 3 mines potentially...

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool DefaultSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{
	switch (u32MessageId)
	{

	case Audio::eInit:
			{

			}
			break;

		case Audio::eBeginLevel:
			{
				m_EntityInstanceCount = 0;
				//for(int entityIndex = 0; entityIndex < 60; entityIndex++)
				//{
				//	m_trackObject[entityIndex].m_markForDelete = false;
				//	m_trackObject[entityIndex].m_hEntity = NULL;
				//	Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[entityIndex].m_hEntity, "EntityFinished");
				//	m_trackObject[entityIndex].m_hQueryEntityFinished = Audio::InterfaceController::Get().SetCallback(m_trackObject[entityIndex].m_hEntity, hActionOut,4);

				//}



				for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
				{
					Audio::EntityInstance* pInstance = m_aEntityInstance[i];
					if (pInstance)
					{
						Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StartActionIn"));
						Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
					}
				}

				if(!g_bLoadDefaultMines)
				break;

				Util::C_String string = "Mine";
				/*Audio::XtEntityProxy* */m_pMineXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(string);

				if (m_pMineXtEntity)
				{
					Game::CacheEntityResources(*m_pMineXtEntity, string.Chars());
					
					if (m_pMineXtEntity->GetEntityTypeHnd().IsValid())
					{
						//AudioEntityHandle hEntity = Audio::InterfaceController::Get().CreateEntity(pXtEntity->GetEntityTypeHnd());
						
						//Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(hEntity, "StartActionIn");

						// connect to system entity
						//Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &hEntity);

						//Audio::InterfaceController::Get().TriggerEntityActionIn(hEntity, hActionIn);

						// temp. store handle in AudioSystem class
						//C_GameApp::GetInstance()->GetGameAudioLayer()->SetActorEntityHnd(Game::C_GameAudioLayer::eUISubsystem,hEntity);
					}
				}

			}
			break;
		case Audio::eEndLevel:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StopActionIn"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
				}
			}

			
			if(!g_bLoadDefaultMines)
				break;

			for(int i = 0; i< MAX_MINES_AT_ONCE;i++)
			{
				if(m_trackObject[i].m_hEntity.IsValid())
				{
					Audio::InterfaceController::Get().ReleaseEntity(m_trackObject[i].m_hEntity);
			
					m_trackObject[i].m_hEntity = NULL;
				}
			}

				//// free the resources
				//const I_AudioEntity_CreateInfo* pCreateData = GetCreationData();

				//if (pCreateData && pCreateData->GetXtEntityDef())
				//	pCreateData->GetXtEntityDef()->ReleaseEntityType();
			if (m_pMineXtEntity)
				m_pMineXtEntity->ReleaseEntityType();

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
			break;

		case Audio::eUnmute:
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

		case Audio::eSuspend:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::InterfaceController::Get().SuspendEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;

		case Audio::eResume:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;
		
	}
	return true;
}

bool DefaultSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data, void* pData)
{

	//[cian] test systems using mine audio as test bed.
	//send custom message to default system
	if(g_bTestObject)
	{

	}

	
	if(!g_bLoadDefaultMines)
		return false;


	switch(u32MessageId)
	{

		case E_DefaultSubSystemMessage_Mine:
		{
			//Components::PerkUpdater* pu =  (Components::PerkUpdater*)pData;

			//if(pu)
			{

				int entityIndex = 0;

				
					//entityIndex = pu->mUniqueID;

				//if(entityIndex >= 60)
				//{
				//	return false;
				//}

				//entityIndex = entityIndex % MAX_MINES_AT_ONCE;

				switch(u32Data)
				{

					// AUDIOTODO: this code doesnt look like its called. get rid of it?
					case E_DefaultSubSystemMineCreate:
						{

						//	const C_VehicleLogicMsg_DroppedPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_DroppedPerkMsg >(pMessage);
							C_VehicleLogicMsg_DroppedPerkMsg* pu =  (C_VehicleLogicMsg_DroppedPerkMsg*)pData;
							
							entityIndex = pu->mUniqueID;
							entityIndex = entityIndex % MAX_MINES_AT_ONCE;

							//handle if a mine of dupe index comes in...release it and make another...
							if(m_trackObject[entityIndex].m_hEntity.IsValid())
							{
								Audio::InterfaceController::Get().ReleaseEntity(m_trackObject[entityIndex].m_hEntity);
								m_trackObject[entityIndex].m_hEntity = NULL;
								m_trackObject[entityIndex].m_markForDelete = false;
								m_EntityInstanceCount--;
							
							}

							if(!m_trackObject[entityIndex].m_hEntity.IsValid())
							{
								Audio::InterfaceController::Get().CreateEntity(m_pMineXtEntity->GetEntityTypeHnd(), m_trackObject[entityIndex].m_hEntity);
								m_EntityInstanceCount++;

					
							m_trackObject[entityIndex].m_markForDelete = false;
							Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[entityIndex].m_hEntity, "EntityFinished");
							Audio::DefaultEntityCallback* pCallbackData = new Audio::DefaultEntityCallback(m_trackObject[entityIndex].m_hEntity, 4);

							if(hActionOut != -1)
								m_trackObject[entityIndex].m_hQueryEntityFinished = Audio::InterfaceController::Get().SetCallback(pCallbackData, hActionOut);

									

								Audio::EntityDefinitionHandle	m_hEntityDefinition = m_pMineXtEntity->GetEntityDefHnd();
								Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(m_hEntityDefinition);
								if (pDef && pDef->LookupInstanceComponent(Audio::StringHash("OutputToRenderer")) != 0xffffffff)
									Audio::InterfaceController::Get().SetMaster(m_trackObject[entityIndex].m_hEntity, "OutputToRenderer");

								Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &m_trackObject[entityIndex].m_hEntity);
							}				


							m_trackObject[entityIndex].m_Position = pu->mMatrix.AW();

							Audio::EntityDataMemberHandle hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[entityIndex].m_hEntity, "MinePosition");
							Audio::InterfaceController::Get().SetEntityVectorIn(m_trackObject[entityIndex].m_hEntity, hVectorIn,  (F32*)&m_trackObject[entityIndex].m_Position);

							
							Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[entityIndex].m_hEntity, "MineActive");
							Audio::InterfaceController::Get().TriggerEntityActionIn(m_trackObject[entityIndex].m_hEntity, hActionIn);		


							hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[entityIndex].m_hEntity, "StartActionIn");
							Audio::InterfaceController::Get().TriggerEntityActionIn(m_trackObject[entityIndex].m_hEntity, hActionIn);		
						}


						//PerkUpdat
					break;

					case E_DefaultSubSystemMineExplode:  //explode
						{
							//const C_VehicleLogicMsg_DestroyPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_DestroyPerkMsg >(pMessage);
							//C_VehicleLogicMsg_DestroyPerkMsg* pu =  (C_VehicleLogicMsg_DestroyPerkMsg*)pData;

							//entityIndex = pu->mUniqueID;


							//Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[entityIndex].m_hEntity, "MineExplode");
							//Audio::InterfaceController::Get().TriggerEntityActionIn(m_trackObject[entityIndex].m_hEntity, hActionIn);
						}
					break;

					case E_DefaultSubSystemMineDestroy:  //erase
						{
							m_trackObject[entityIndex].m_markForDelete = true;
						}

					break;

				}
			}
				
		}
		break;

		case E_DefaultSubSystemMessage_TestSound:
			{
				switch(u32Data)
				{

				case E_DefaultSubSystemMineCreate:

					{
						
						//handle if a mine of dupe index comes in...release it and make another...
							if(m_trackObject[m_testEntityIndex].m_hEntity.IsValid())
							{
								Audio::InterfaceController::Get().ReleaseEntity(m_trackObject[m_testEntityIndex].m_hEntity);
								m_trackObject[m_testEntityIndex].m_hEntity = NULL;
								m_trackObject[m_testEntityIndex].m_markForDelete = false;
								m_EntityInstanceCount--;
							
							}

							if(!m_trackObject[m_testEntityIndex].m_hEntity.IsValid())
							{
								Audio::InterfaceController::Get().CreateEntity(m_pMineXtEntity->GetEntityTypeHnd(), m_trackObject[m_testEntityIndex].m_hEntity);
								m_EntityInstanceCount++;

						
								m_trackObject[m_testEntityIndex].m_markForDelete = false;
								Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[m_testEntityIndex].m_hEntity, "EntityFinished");
								Audio::DefaultEntityCallback* pCallbackData = new Audio::DefaultEntityCallback(m_trackObject[m_testEntityIndex].m_hEntity, 4);

								if(hActionOut != -1)
									m_trackObject[m_testEntityIndex].m_hQueryEntityFinished = Audio::InterfaceController::Get().SetCallback(pCallbackData, hActionOut);

								//m_trackObject[m_testEntityIndex].m_hQueryEntityFinished = Audio::InterfaceController::Get().SetCallback(m_trackObject[m_testEntityIndex].m_hEntity, hActionOut,4);

									

								Audio::EntityDefinitionHandle	m_hEntityDefinition = m_pMineXtEntity->GetEntityDefHnd();
								Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(m_hEntityDefinition);
								if (pDef && pDef->LookupInstanceComponent(Audio::StringHash("OutputToRenderer")) != 0xffffffff)
									Audio::InterfaceController::Get().SetMaster(m_trackObject[m_testEntityIndex].m_hEntity, "OutputToRenderer");

								Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &m_trackObject[m_testEntityIndex].m_hEntity);
							}
							

							//m_trackObject[m_testEntityIndex].m_Position = //pu->mMatrix.AW();
							Maths::HMat43 camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
							//Maths::HMat43 camMatrix = view.DebugFeed().Mat43();
							m_trackObject[m_testEntityIndex].m_Position =camMatrix.AW();

							Audio::EntityDataMemberHandle hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[m_testEntityIndex].m_hEntity, "MinePosition");
							Audio::InterfaceController::Get().SetEntityVectorIn(m_trackObject[m_testEntityIndex].m_hEntity, hVectorIn,  (F32*)(&m_trackObject[m_testEntityIndex].m_Position));



							
							Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[m_testEntityIndex].m_hEntity, "MineActive");
							Audio::InterfaceController::Get().TriggerEntityActionIn(m_trackObject[m_testEntityIndex].m_hEntity, hActionIn);		


							hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[m_testEntityIndex].m_hEntity, "StartActionIn");
							Audio::InterfaceController::Get().TriggerEntityActionIn(m_trackObject[m_testEntityIndex].m_hEntity, hActionIn);

							m_testEntityIndex++;
						

					}
				break;

				case E_DefaultSubSystemMineDestroy:

					{
						//int m_testEntityIndex = 0;
						if(m_testEntityIndex >= 0)
						{
							m_testEntityIndex--;
							Audio::InterfaceController::Get().ReleaseEntity(m_trackObject[m_testEntityIndex].m_hEntity);
							m_trackObject[m_testEntityIndex].m_hEntity = NULL;
							m_trackObject[m_testEntityIndex].m_markForDelete = false;
							m_EntityInstanceCount--;
							
						}
						

					}
					break;

				}
			
			}
			break;
		}
	

	return true;
}

void	DefaultSubsystem::Update()
{
	
	if(!g_bLoadDefaultMines)
				return;

	if(g_bTestObject)
	{
		if(g_bTestObjectCreate)
		{
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eDefaultSubsystem, E_DefaultSubSystemMessage_TestSound,E_DefaultSubSystemMineCreate);
			g_bTestObjectCreate = false;
		}

		if(g_bTestObjectDelete)
		{

			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eDefaultSubsystem, E_DefaultSubSystemMessage_TestSound,E_DefaultSubSystemMineDestroy);
			g_bTestObjectDelete = false;
		}


	}

	if(m_EntityInstanceCount > 0)
	{
		for(int entityIndex = 0; entityIndex < 60; entityIndex++)
		{

			if(!m_trackObject[entityIndex].m_hEntity.IsValid())
				continue;

			
			Maths::HMat43 camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
			//Maths::HMat43 camMatrix = view.DebugFeed().Mat43();

			Audio::EntityDataMemberHandle hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[entityIndex].m_hEntity, "ListenerPosition");
			Audio::InterfaceController::Get().SetEntityVectorIn(m_trackObject[entityIndex].m_hEntity, hVectorIn,  (F32*)(&camMatrix.AW()));

			hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[entityIndex].m_hEntity, "ListenerDirection");
			Audio::InterfaceController::Get().SetEntityVectorIn(m_trackObject[entityIndex].m_hEntity, hVectorIn,  (F32*)(&camMatrix.AZ()));

			hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_trackObject[entityIndex].m_hEntity, "ListenerUp");
			Audio::InterfaceController::Get().SetEntityVectorIn(m_trackObject[entityIndex].m_hEntity, hVectorIn,  (F32*)(&camMatrix.AY()));

			//check if ready for delete
			U32 callbackCount = Audio::InterfaceController::Get().GetCallbackTriggeredCount(m_trackObject[entityIndex].m_hQueryEntityFinished);

			F32 distanceThreshold = 200.f;
			//if it exists and is set
			if(distanceThreshold)
			{
				//get object distance to listener
				Maths::HVec3 entityDistance = ((camMatrix.AW())  - (m_trackObject[entityIndex].m_Position));

				F32 length = Maths::Length(entityDistance);

				// check if entity is within distance
				if(length > distanceThreshold)
				{
					//Audio::InterfaceController::Get().MuteEntity(hCurrentEntity);
					Audio::InterfaceController::Get().SuspendEntity(m_trackObject[entityIndex].m_hEntity);
					callbackCount = 1;  //set to delete since we can no longer recieve callback if suspended

				}
				else
				{
					Audio::InterfaceController::Get().ResumeEntity(m_trackObject[entityIndex].m_hEntity);
					//Audio::InterfaceController::Get().UnmuteEntity(hCurrentEntity);
				}
			}

			//			

			if(m_trackObject[entityIndex].m_markForDelete && callbackCount)
			{
				Audio::InterfaceController::Get().ReleaseEntity(m_trackObject[entityIndex].m_hEntity);
				m_trackObject[entityIndex].m_hEntity = NULL;
				m_trackObject[entityIndex].m_markForDelete = false;
				m_EntityInstanceCount--;
			}
		}
	}

	


}
#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


