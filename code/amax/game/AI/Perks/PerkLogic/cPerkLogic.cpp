#include "stdafx.h"
#include <ai/Perks/PerkLogic/cPerkLogic.h>
#include <ai/perks/PerkSystem/iPerkSystem.h>
#include <ai/Perks/PerkAndSkillUsage/iPerkSkillUsage.h>
#include <AI/AI_Amax.h>
#include <AI/AI_Messages.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <Game/Game.h>
#include <GameApp.h>
#include <game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/ActorStudioActors/ActorStudioActors.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <trackdata/itrackdata.h>
#include <utilityai/utilityai.h>



#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif



Debug::Bool g_OveridePickupCollect("AmaxAI.PerkLogic.CollectPickups.Overide",false);
Debug::Int  g_OveridePickupTypeToCollect("AmaxAI.PerkLogic.CollectPickups.Type",6, Debug::Range(0,7), Debug::Step(1));

namespace airacingtech
{
	using namespace Game;

	CPerkLogicAI::CPerkLogicAI() : m_pIData(0), m_pIPerkSystem(0), m_lastReceived(0.0f), m_pIGameServerTime(0), m_pIPerkSkillUsage(0)
	{
		m_TargetPerk.m_nearPerk = -1;
		m_TargetPerk.m_nearPerkRoute = -1;
		m_TargetPerk.m_TimeStamp = 0.0f;
		m_TargetPerk.m_prefernceValue = 0.0f;
		m_TargetPerk.m_PerkType = -1;
	}

	CPerkLogicAI::~CPerkLogicAI()
	{
		m_pIData = 0;
		m_pIPerkSystem = 0;
	}

	void CPerkLogicAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_LOGIC, CID_PERK_LOGIC);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_LOGIC, CPerkLogicAI::CreateMe, CPerkLogicAI::DestroyMe, CHash("PerkLogicAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_LOGIC, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_LOGIC, MT_SEGMENT_TRIGGER);	
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_LOGIC, MT_PRE_UPDATE_GAME_COMPONENTS);
	}

	IComponent *CPerkLogicAI::CreateMe()
	{
		return new CPerkLogicAI;
	}

	bool CPerkLogicAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkLogicAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkLogicAI::Deinit(void)
	{

	}

	EMessageResult CPerkLogicAI::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{

			case MT_POST_INIT_AI_SYSTEM:
				{
					msgRet =  MR_TRUE;
					CachedInterfaces();	
				}
				break;


			case MT_PRE_UPDATE_GAME_COMPONENTS:
				{
					PreUpdate();
				}
				break;


			case MT_UPDATE_GAME_COMPONENTS:
				{
					Update();
				}
				break;


			case MT_POST_UPDATE_GAME_COMPONENTS:
				{
					PostUpdate();
				}
				break;

			case MT_SEGMENT_TRIGGER:
				{
					HandleTrigger();
				}
				break;

			default:
				break;
		}

		return msgRet;
	}

	EComponentTypeId CPerkLogicAI::GetComponentTypeId(void)
	{
		return CID_PERK_LOGIC;
	}

	void CPerkLogicAI::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId perkSystemID("PerkSystemAISystem");
		m_pIPerkSystem = static_cast<IPerkSystem *>(GlobalsComponent::GetObjectManager().QueryInterface(perkSystemID, IID_PERK_SYSTEM));


		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));


		m_pIPerkSkillUsage = static_cast<IPerkSkillUsage *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERKS_SKILL_USAGE));


		CObjectId trackdataSystem("TrackDataSystem");
		m_pITrackData = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));


	}

/*
	void CPerkLogicAI::HandleTrigger()
	{
		if (m_pIPerkSystem && !m_pIPerkSystem->GetAIPerkUsage() && m_pIData && m_pIGameServerTime && m_pIGameServerTime->GetTime() > m_lastReceived)
		{
			// update every 0.5 second(s) only
			m_lastReceived = m_pIGameServerTime->GetTime() + 0.5f;

			ISplineTracker	*pSplineTracker = m_pIData->GetSplineTracker();
			if (pSplineTracker)
			{
				const RouteInfo *pRouteInfo = pSplineTracker->GetCurrentRoute();
				if (pRouteInfo)
				{
					int nearPerk = -1;
					int nearPerkRoute = -1;
					float maxDist = FLT_MAX;

					int numPerks = m_pIPerkSystem->GetNumPerks();
					for(int i=0; i<numPerks; i++)
					{
						AIAmax::AIPerk &perk = m_pIPerkSystem->GetPerk(i);
						int numRoutes = perk.m_routePerkInfo.Size();

						const Actor::C_Actor *pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(perk.m_actorHandle);
						assert(pActor);
						::Actors::PickupPositionActor *pPerkActor = (::Actors::PickupPositionActor*)Actor::ActorCast< ::Actors::PickupPositionActor>(pActor);
						assert(pPerkActor);

						if (perk.m_perkActive && perk.m_perkEnabled)
						{
							for(int j=0; j<numRoutes; j++)
							{
								if (perk.m_routePerkInfo[j].m_RouteNo == pRouteInfo->m_RouteIndex)
								{						
									if (pRouteInfo->m_index < perk.m_routePerkInfo[j].m_Index &&
										pRouteInfo->m_index > perk.m_routePerkInfo[j].m_Index - 25) // 20 - 16 Go for longer distance, make it velocity based ????
									{
										// TODO : Raycast height, so AI doesn`t go for perks on the jumps causing them to crash out
										// rough calc for the y height
										bool heightTest = false;
										float diffY = pPerkActor->GetPosition().Y() - m_pIData->GetVehicleState()->GetState()->GetPosition().Y();
										if (diffY > -0.5f && diffY < 2.0f)
										{
											heightTest = true;
										}
										
										
									

										if (Maths::Abs(perk.m_routePerkInfo[j].m_Dist) < 1.0f && heightTest)
										{
											float diffDist = pRouteInfo->m_offset - perk.m_routePerkInfo[j].m_Dist;

					
											// Test for repair.........now
											Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
											assert(pLayer);
											Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);
											assert(pLogicActor);

											bool goForIt = true;
											if (pPerkActor->GetEnumPickupType() == E_PICKUP_TYPE_REPAIR)
											{
												if (pLogicActor->GetCurrentHealthLevel() != E_HEALTH_LEVEL_HIGH)
												{
													goForIt = true;
												}
												else
												{
													goForIt = false;
												}
											}

											// NOTE : this will need more work.......
											// we now have a list of perks we can possibly go for....
											// now the hard part..priority....
											// OK we have a problem, currently the perks are changing type randomly at runtime, doesn`t update the stored list in gamelogiccore
											// therefore... we`ll only go for the nearest in the x direction.....if it`s within reach....
											if (Maths::Abs(diffDist) < maxDist && goForIt)
											{
												nearPerk = i;
												nearPerkRoute = j;
												maxDist = Maths::Abs(diffDist);//perk.m_routePerkInfo[j].m_Dist;
											}
										}
									}
								}
							}
						}
					}

					if (nearPerk != -1)
					{
						// we have found a perk to go for.. set it as a target.......
						m_TargetPerk.m_nearPerk = nearPerk;
						m_TargetPerk.m_nearPerkRoute = nearPerkRoute;
						m_TargetPerk.m_TimeStamp = m_pIGameServerTime->GetTime();
					}

				}
			}
		}
	}
	*/


	void CPerkLogicAI::HandleTrigger()
	{
		if (m_pIPerkSystem && m_pIPerkSkillUsage && !m_pIPerkSystem->GetAIPerkUsage() && m_pIData && m_pIGameServerTime && m_pIGameServerTime->GetTime() > m_lastReceived &&
			m_pIData->GetVehicleLogic()  && m_pIData->GetVehicleLogic()->GetLogicState()->GetRacePercentage() < 1.0f)
		{
			m_lastReceived = m_pIGameServerTime->GetTime();

			float maxLateralDist = 8.5f; // could increase this
			if (m_pIPerkSkillUsage)
			{
				AIPerkSkillzXt *pSkillz = m_pIPerkSkillUsage->GetSkillz();
				if (pSkillz)
				{
					maxLateralDist *= pSkillz->m_Aggression;
					maxLateralDist = Maths::Max(maxLateralDist, 5.0f); // 5 metres.....
				}
			}


			ISplineTracker	*pSplineTracker = m_pIData->GetSplineTracker();
			if (pSplineTracker)
			{
				const RouteInfo *pRouteInfo = pSplineTracker->GetCurrentRoute();
				if (pRouteInfo)
				{
					int nearPerk = -1;
					int nearPerkRoute = -1;
					float maxDist = FLT_MAX;

					int numPerks = m_pIPerkSystem->GetNumPerks();
					for(int i=0; i<numPerks; i++)
					{
						AIAmax::AIPerk &perk = m_pIPerkSystem->GetPerk(i);
						int numRoutes = perk.m_routePerkInfo.Size();

						const Actor::C_Actor *pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(perk.m_actorHandle);
						assert(pActor);
						::Actors::PickupPositionActor *pPerkActor = (::Actors::PickupPositionActor*)Actor::ActorCast< ::Actors::PickupPositionActor>(pActor);
						assert(pPerkActor);

						bool isPerkTargeted = true;
						AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();
						if (pAISkillz && pAISkillz->m_Aggression >= 0.75f)//0.74f
						{
							isPerkTargeted = false;
						}
						else
						{
							if (perk.m_targetBy == -1)
							{
								isPerkTargeted = false;
							}
						}


						if (perk.m_perkActive && perk.m_perkEnabled && isPerkTargeted == false)
						{
							for(int j=0; j<numRoutes; j++)
							{
								if (perk.m_routePerkInfo[j].m_RouteNo == pRouteInfo->m_RouteIndex)
								{						
									if (pRouteInfo->m_index < perk.m_routePerkInfo[j].m_Index &&
										pRouteInfo->m_index > perk.m_routePerkInfo[j].m_Index - 25) // 20 - 16 Go for longer distance, make it velocity based ????
									{
										// TODO : Raycast height, so AI doesn`t go for perks on the jumps causing them to crash out
										// rough calc for the y height
										//bool heightTest = false;
										//float diffY = pPerkActor->GetPosition().Y() - m_pIData->GetVehicleState()->GetState()->GetPosition().Y();
										//if (diffY > -0.5f && diffY < 2.0f)
										//{
										//	heightTest = true;
										//}

										//if (Maths::Abs(perk.m_routePerkInfo[j].m_Dist) < 1.0f)// && heightTest)
										if (Maths::Abs(perk.m_routePerkInfo[j].m_Offset) < 1.0f)// && heightTest)
										{
											float d = pRouteInfo->m_pRoute->GetDistanceUsingOffsetParamAndLine(pRouteInfo->m_offset, AITrackDataLines::TRACK_DATA_LINE_RACING, pRouteInfo->m_index);
											float perkDistanceToRacingLine = perk.m_routePerkInfo[j].m_Dist;
											float diffDist = perkDistanceToRacingLine - d;
											float diffDistInMetres = diffDist;

											Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
											assert(pLayer);
											Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);
											assert(pLogicActor);

											bool goForIt = true;
											
											if (g_OveridePickupCollect)
											{
												if (pPerkActor->GetEnumPickupType() != g_OveridePickupTypeToCollect)
												{
													goForIt = false;
												}
												else
												{
													goForIt = true;
												}
											}

																	
											// dist check
											bool distCheck = false;
											if (Maths::Abs(diffDistInMetres) < maxLateralDist) 
											{	
												distCheck = true;
											}

											
											if (goForIt && distCheck)
											{					
												// no target perk at present
												if (m_TargetPerk.m_nearPerk == -1)
												{
													// we have found a perk to go for.. set it as a target.......
													m_TargetPerk.m_nearPerk = i;
													m_TargetPerk.m_nearPerkRoute = j;
													m_TargetPerk.m_TimeStamp = m_pIGameServerTime->GetTime();
													m_TargetPerk.m_PerkType = pPerkActor->GetEnumPickupType();
													m_TargetPerk.m_prefernceValue = m_pIPerkSkillUsage->GetPerkPreferenceValue(pPerkActor->GetEnumPickupType());
													m_TargetPerk.m_InitialDist = diffDistInMetres;
													perk.m_targetBy = m_pIData->GetState().m_index;
													
												}
												else
												{
													// target perk based upon the priority.. might need to include a distance check

													if (m_pIPerkSkillUsage)
													{
														AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();

														// if the same type
														if (m_TargetPerk.m_PerkType == pPerkActor->GetEnumPickupType())
														{
															if (Maths::Abs(diffDistInMetres) < Maths::Abs(m_TargetPerk.m_InitialDist))
															{
																// we have found a perk to go for.. set it as a target.......
																m_TargetPerk.m_nearPerk = i;
																m_TargetPerk.m_nearPerkRoute = j;
																m_TargetPerk.m_TimeStamp = m_pIGameServerTime->GetTime();
																m_TargetPerk.m_PerkType = pPerkActor->GetEnumPickupType();
																m_TargetPerk.m_prefernceValue = m_pIPerkSkillUsage->GetPerkPreferenceValue(pPerkActor->GetEnumPickupType());
																m_TargetPerk.m_InitialDist = diffDistInMetres;
																perk.m_targetBy = m_pIData->GetState().m_index;
																
																


															}
														}
														else
														{

															// check the health..........
															float healthPercent = ((static_cast<float>(pLogicActor->GetCurrentHealth() + 1.0f)) / static_cast<float>(pLogicActor->GetMaxHealth()));
															bool overrideRepair = false;
															
															if (pAISkillz && healthPercent < pAISkillz->m_Repair.m_Defensive )
															{
																overrideRepair = true;
															}


															if (m_TargetPerk.m_PerkType == E_PICKUP_TYPE_REPAIR && overrideRepair)
															{
																// do nothing if we have already selected a repair for the target
															}
															else
															{
																if (pPerkActor->GetEnumPickupType() == E_PICKUP_TYPE_REPAIR && overrideRepair)
																{
																	// we have found a perk to go for.. set it as a target.......
																	m_TargetPerk.m_nearPerk = i;
																	m_TargetPerk.m_nearPerkRoute = j;
																	m_TargetPerk.m_TimeStamp = m_pIGameServerTime->GetTime();
																	m_TargetPerk.m_PerkType = pPerkActor->GetEnumPickupType();
																	m_TargetPerk.m_prefernceValue = m_pIPerkSkillUsage->GetPerkPreferenceValue(pPerkActor->GetEnumPickupType());
																	m_TargetPerk.m_InitialDist = diffDistInMetres;
																	perk.m_targetBy = m_pIData->GetState().m_index;
																	
																}
																else
																{
																	float newPref = m_pIPerkSkillUsage->GetPerkPreferenceValue(pPerkActor->GetEnumPickupType());
																	if (newPref > m_TargetPerk.m_prefernceValue && pPerkActor->GetEnumPickupType() != E_PICKUP_TYPE_REPAIR)
																	{
																		// we have found a perk to go for.. set it as a target.......
																		m_TargetPerk.m_nearPerk = i;
																		m_TargetPerk.m_nearPerkRoute = j;
																		m_TargetPerk.m_TimeStamp = m_pIGameServerTime->GetTime();
																		m_TargetPerk.m_PerkType = pPerkActor->GetEnumPickupType();
																		m_TargetPerk.m_prefernceValue = m_pIPerkSkillUsage->GetPerkPreferenceValue(pPerkActor->GetEnumPickupType());
																		m_TargetPerk.m_InitialDist = diffDistInMetres;
																		perk.m_targetBy = m_pIData->GetState().m_index;
																		
																	}
																}
															}	
														}							
													}
												}
											}
										}
									}
								}
							}
						}
					}

					

				}
			}
		}
	}




	void CPerkLogicAI::PreUpdate()
	{
		if (m_pIPerkSystem && m_pIData && m_pIGameServerTime)
		{
			ISplineTracker	*pSplineTracker = m_pIData->GetSplineTracker();
			if (pSplineTracker)
			{
				const RouteInfo *pRouteInfo = pSplineTracker->GetCurrentRoute();
				if (pRouteInfo)
				{
					// lets get the rubberbanding in order to get the template
					if (m_TargetPerk.m_nearPerk != -1)
					{
						// get the perk......
						AIAmax::AIPerk &perk = m_pIPerkSystem->GetPerk(m_TargetPerk.m_nearPerk);
						//float offsetRacingLine = perk.m_routePerkInfo[m_TargetPerk.m_nearPerkRoute].m_Dist;
						float offsetRacingLine = perk.m_routePerkInfo[m_TargetPerk.m_nearPerkRoute].m_Offset;
					
						if (pRouteInfo->m_RouteIndex == perk.m_routePerkInfo[m_TargetPerk.m_nearPerkRoute].m_RouteNo)
						{
							if (pRouteInfo->m_index >= perk.m_routePerkInfo[m_TargetPerk.m_nearPerkRoute].m_Index)
							{
								// reset the target perk.
								m_TargetPerk.m_nearPerk = -1;
								m_TargetPerk.m_nearPerkRoute = -1;
								m_TargetPerk.m_TimeStamp = 0.0f;
								perk.m_targetBy = -1;
							}
							else
							{
								// default. maybe base upon the aggression.....
								//m_pIData->SetOverrideSteerPerk(true, offsetRacingLine, 5.0f, 5.0f); 

								
								// here we need to check the avoidance and the pickup stash

								// check the pickup stash
								if (m_pIPerkSkillUsage->GetPickupStashSize() < 3)
								{
									m_pIData->SetOverrideSteerPerk(true, offsetRacingLine, 5.0f, 1.0f);
								}
							}

						}
						else
						{
							// reset the target perk.
							m_TargetPerk.m_nearPerk = -1;
							m_TargetPerk.m_nearPerkRoute = -1;
							m_TargetPerk.m_TimeStamp = 0.0f;
							perk.m_targetBy = -1;
						}
					}
				}

				// We will avoid the shocks first, then the mines....not based upon skill at present.
				if (!CheckForShocks())
				{
					bool avoidMines = CheckForMines();
				}
				

			}
		}
	}

	void CPerkLogicAI::Update()
	{

	}

	void CPerkLogicAI::PostUpdate()
	{

	}

	bool CPerkLogicAI::CheckForMines()
	{
		bool avoid = false;
		
		if (m_pITrackData && m_pIPerkSystem && m_pIPerkSkillUsage && !m_pIPerkSystem->GetAIPerkUsage() && m_pIData && m_pIGameServerTime &&
			m_pIData->GetVehicleLogic() && m_pIData->GetVehicleLogic()->GetLogicState()->GetRacePercentage() < 1.0f &&
			m_pIData->GetSplineTracker())
		{

			airacingtech::ISplineTracker *pISplineTracker = m_pIData->GetSplineTracker();
			const airacingtech::RouteInfo *pLogicRoute = pISplineTracker->GetLogicRoute();
			const airacingtech::RouteInfo *pCurrentRoute = pISplineTracker->GetCurrentRoute();
			if (pLogicRoute && pCurrentRoute)
			{
				int routeIndex;
				float racingOffset; 
				float zDist;
				float radius = 3.0f;
				int perkIndex = GetNearestPerkTypeInfront(pLogicRoute->m_index, E_PICKUP_TYPE_BOMB, routeIndex, racingOffset, zDist, radius);	
				if (perkIndex != -1 && routeIndex != -1 && racingOffset != FLT_MAX && zDist > 0.0f && zDist < 150.0f)
				{
					// we have found a mine
					if (zDist < 150.0f)
					{
						avoid = true;
						m_pIData->SetOverrideSteerPerk(true, racingOffset, 2.5f, 0.5f);	// 0.5
					}
				}
			}
		}

		return avoid;
	}


	bool CPerkLogicAI::CheckForShocks()
	{
		bool avoid = false;
		if (m_pITrackData && m_pIPerkSystem && m_pIPerkSkillUsage && !m_pIPerkSystem->GetAIPerkUsage() && m_pIData && m_pIGameServerTime &&
			m_pIData->GetVehicleLogic() && m_pIData->GetVehicleLogic()->GetLogicState()->GetRacePercentage() < 1.0f &&
			m_pIData->GetSplineTracker())
		{

			airacingtech::ISplineTracker *pISplineTracker = m_pIData->GetSplineTracker();
			const airacingtech::RouteInfo *pLogicRoute = pISplineTracker->GetLogicRoute();
			const airacingtech::RouteInfo *pCurrentRoute = pISplineTracker->GetCurrentRoute();
			if (pLogicRoute && pCurrentRoute)
			{
				int routeIndex;
				float racingOffset; 
				float zDist;
				float radius = 6.0f; // might be too big...



				// skill 
				AIPerkSkillzXt *pSkillz = m_pIPerkSkillUsage->GetSkillz();
				if (pSkillz)
				{
					if (pSkillz->m_Shock.m_Avoidance < 0.25f)
					{
						radius *= 0.5f;
					}
					else if (pSkillz->m_Shock.m_Avoidance < 0.5f)
					{
						radius *= 0.75f;
					}
				}
				

				int perkIndex = GetNearestPerkTypeInfront(pLogicRoute->m_index, E_PICKUP_TYPE_SHOCK, routeIndex, racingOffset, zDist, radius);	
				if (perkIndex != -1 && routeIndex != -1 && racingOffset != FLT_MAX && zDist > 0.0f && zDist < 150.0f)
				{
					// we have found a mine
					if (zDist < 150.0f)
					{
						avoid = true;
						m_pIData->SetOverrideSteerPerk(true, racingOffset, 2.5f, 0.5f);	// 0.5
					}
				}
			}
		}

		return avoid;
	}



	int CPerkLogicAI::GetNearestPerkTypeInfront(int myIndex, E_PICKUP_TYPE pickup, int &routeIndex, float &racingOffset, float &zDist, float radius)
	{
		int val = -1;
		airacingtech::AITrackData *pAITrackData = m_pITrackData->GetTrackData();
		airacingtech::ISplineTracker *pISplineTracker = m_pIData->GetSplineTracker();
		const airacingtech::RouteInfo *pCurrentRoute = pISplineTracker->GetCurrentRoute();

		routeIndex = -1;
		racingOffset = FLT_MAX;
		zDist = FLT_MAX;

		if (pAITrackData && pAITrackData->m_TrackLogicData.m_pLogicRoute && pCurrentRoute)
		{
			float minDIST = FLT_MAX;

			// get the nearest in front...

			Game::ActivePickupList& pickupList = Game::C_PickupsManager::Instance().GetActivePickupList();

			for(int i=0; i<m_pIPerkSkillUsage->GetNumProjectiles(); i++)
			{
				Game::C_PickupBase* pPickup = m_pIPerkSkillUsage->GetProjectile(i);
				if (pPickup)
				{
				
					// Darren : Error checking code for rare bug....

					/*
					int pickupTreeSize = pickupList.Size();
					int originalSize = m_pIPerkSkillUsage->DebugActivePickupCount();
					
					if (pickupTreeSize != originalSize)
					{
						BIZ_WARNING_MSG(false, "cPerkLogic - AI : List of projectiles are different");
					}
					*/
				
					
					bool found = false;

					for( ActivePickupList::Iterator it = pickupList.Begin(); it != pickupList.End(); it++ )
					{
						C_PickupBase* pPickupPerk = it->GetValue();

						if(pPickup == pPickupPerk)
						{
							found = true;
							break;
						}
					}

					if (found)
					{
						Game::AITrackableData& data = pPickup->GetAITrackableData();
						if (data.mAILogicIndex != -1 && pPickup->GetType() == pickup)
						{		
							float dist = static_cast<float>(pAITrackData->m_TrackLogicData.m_stepSize * airacingtech::DistanceInfrontBehind(data.mAILogicIndex , myIndex, pAITrackData->m_TrackLogicData.m_noSamples));
							if (dist > 0.0f && dist < 150.0f) // this was 80.0f;
							{
								// map into spline space.....

								const airacingtech::AITrackDataCache& dc = pAITrackData->m_TrackLogicData.m_pLogicRoute->m_dataCache[data.mAILogicIndex];
								const int noEntries = dc.m_data.Size();	

								for(int entries = 0; entries <noEntries; entries++ )
								{
									const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];
								
									if (cachedData.m_pRoute && cachedData.m_pRoute->IsChangeableRoute() && cachedData.m_pRoute == pCurrentRoute->m_pRoute)
									{
										// we have found one valid route....
										// More check here please
											
										Maths::HVec3 pos = pPickup->GetMatrix().AW();
										int foundIndex = FindNearest(cachedData.m_index, pos, pCurrentRoute);
										
										if (foundIndex != -1)
										{
											float racingLineOffset = m_pITrackData->WorkOutOffsetToRacingSpline(pCurrentRoute->m_pRoute, pos, foundIndex);
											float racingLineDist = pCurrentRoute->m_pRoute->GetDistanceUsingOffsetParamAndLine(racingLineOffset, AITrackDataLines::TRACK_DATA_LINE_RACING, foundIndex);
											float d = pCurrentRoute->m_pRoute->GetDistanceUsingOffsetParamAndLine(pCurrentRoute->m_offset, AITrackDataLines::TRACK_DATA_LINE_RACING, pCurrentRoute->m_index);
											float diffDist = racingLineDist - d;
										
											if (Maths::Abs(diffDist) < radius)
											{
												if (dist < minDIST)
												{
													minDIST = dist;
													zDist = dist;
													val = i;
													routeIndex = foundIndex;
													racingOffset = racingLineOffset;
												
													// todo, check the gap

													if (diffDist < 0.0f)
													{
														// on the left, we need to turn right...
														float moveToDist = d + (radius - Maths::Abs(diffDist)) + 1.5f;

														// is there enough room to turn to the right
														float rightOffset = 1.0f;
														float rightDistance = pCurrentRoute->m_pRoute->GetDistanceUsingOffsetParamAndLine(rightOffset, AITrackDataLines::TRACK_DATA_LINE_RACING, foundIndex);
														float distToRight = rightDistance - racingLineDist;
														
														float checkRadius =  radius - 1.0f;
														if (distToRight > checkRadius)
														{
															pCurrentRoute->m_pRoute->CalculateXOffsetFromXDistance(pCurrentRoute->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , moveToDist, racingOffset );
														
															// check the racing offset....
															if (racingOffset < pCurrentRoute->m_offset) // this is what we are steering
															{
																racingOffset = pCurrentRoute->m_offset;
																val = -1;
															}
														}
														else
														{
															moveToDist = d - Maths::Abs(diffDist) - radius;
															pCurrentRoute->m_pRoute->CalculateXOffsetFromXDistance(pCurrentRoute->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , moveToDist, racingOffset );
														
															if (racingOffset > pCurrentRoute->m_offset) // this is what we are steering
															{
																racingOffset = pCurrentRoute->m_offset;
																val = -1;
															}
														
														}
													}
													else 
													{
														// on the right, we need to turn left
														float moveToDist = d - (radius - Maths::Abs(diffDist)) - 1.5f;
														
														float leftOffset = -1.0f;
														float leftDistance = pCurrentRoute->m_pRoute->GetDistanceUsingOffsetParamAndLine(leftOffset, AITrackDataLines::TRACK_DATA_LINE_RACING, foundIndex);
														float distToLeft = leftDistance - racingLineDist;
														float checkRadius =  (radius - 1.0f);
														if (distToLeft < - checkRadius)
														{
															pCurrentRoute->m_pRoute->CalculateXOffsetFromXDistance(pCurrentRoute->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , moveToDist, racingOffset );
															if (racingOffset > pCurrentRoute->m_offset) // this is what we are steering
															{
																racingOffset = pCurrentRoute->m_offset;
																val = -1;
															}
														}
														else
														{
															moveToDist = d + Maths::Abs(diffDist) + radius;
															pCurrentRoute->m_pRoute->CalculateXOffsetFromXDistance(pCurrentRoute->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , moveToDist, racingOffset );
														
															if (racingOffset < pCurrentRoute->m_offset) // this is what we are steering
															{
																racingOffset = pCurrentRoute->m_offset;
																val = -1;
															}
														}
													}
												}
											}
										}
										
									}
								}
							}
						}
					}
					else
					{
						//BIZ_WARNING_MSG(false, "cPerkLogic - AI : The actor list of active pickups and the AI list is different, code above assumes it will be a problem in the actor list.");
					}
				}
			}
		}

		return val;
	}

	int CPerkLogicAI::FindNearest(int Index, Maths::HVec3 pos, const airacingtech::RouteInfo *pRouteInfo)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		float paramDist = 0.f;

		int val = -1;
		{
			int startIndex = pRouteInfo->m_pRoute->GetIndexOffset(Index, -5);//-5); // probably get away with 5
			int endIndex = pRouteInfo->m_pRoute->GetIndexOffset(Index, 12);//5);

			if (pRouteInfo->m_pRoute->IsLoopedTrack() & (startIndex > endIndex))
			{
				int st = startIndex;
				int ed = pRouteInfo->m_pRoute->GetNumSegments() - 1;
				val =  pRouteInfo->m_pRoute->FindNearest( st, ed, pos, paramDist, pRouteInfo->m_pRoute->GetStepSize());
				if (val == -1)
				{
					st = 1;
					ed = endIndex;
					val =  pRouteInfo->m_pRoute->FindNearest( st, ed, pos, paramDist, pRouteInfo->m_pRoute->GetStepSize());
				}
			}
			else
			{
				startIndex = Maths::Max(startIndex-1, 1);
				val =  pRouteInfo->m_pRoute->FindNearest( startIndex, endIndex, pos, paramDist, pRouteInfo->m_pRoute->GetStepSize());
			}					
		}

		return val;
	}

	

}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
