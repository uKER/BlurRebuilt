#include "stdafx.h"
#include <Perks/PerkLogic/cPerkLogic.h>
#include <perks/PerkSystem/cPerkSystem.h>
#include "AI/AI_Amax.h"
#include "AI/AI_Messages.h"
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <Game/Game.h>
#include <GameApp.h>
#include <game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/ActorStudioActors/ActorStudioActors.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <GameLogic/GameData/GameEventData.h>
#include <System/debug/I_DebugRender.h>
#include <Render/View/ViewManager.h>
#include <Game\GameActors\Components\VehicleLogic\HandofGod\VLC_HandOfGod.h>
#include <Vehicle/VehicleModel.h>
#include <Vehicle/Damage/DamageLayer.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

Debug::Bool g_DestructionDebugRender("Amax.GameMode.Destruction.DebugDraw", false);
Debug::Bool g_DestructionKillAllActive("Amax.GameMode.Destruction.KillAllActive", false);
Debug::Bool g_DestructionSpawnNextWave("Amax.GameMode.Destruction.SpawnNextWave", false);
Debug::Bool g_DestructionSpawningEnabled("Amax.GameMode.Destruction.SpawningEnabled", true, Debug::Persist());
Debug::Float g_DestructionSpawnGhostTime("Amax.GameMode.Destruction.SpawningGhostTime", 0.1f, Debug::Range(0.0f, 10.0f));

Debug::Logger &g_DestructionModeLogger( Debug::GetLogger("Amax.GameMode.Destruction") );

static const char* s_formationString[] = 
{
	"Line",
	"Queue",
	"Box",
	"BackLeft_FrontRight",
	"BackRight_FrontLeft",
	"Arrow",
	"Diamond",
};

static const char* s_typeString[] = 
{
	"None",
	"Easy",
	"Medium",
	"Hard",
	"VHard",
};

static Maths::HF32 s_TwoVehicleWaveData[ DM_WF__Max ][2][2] = 
{		// Normal, Lateral
	{	//DM_WF__Line,
		{ 0.0f, 0.25f},  //Vehicle0
		{ 0.0f, 0.75f }, //Vehicle1
	},
	{	//DM_WF__Queue,
		{ 6.0f, 0.5f},  //Vehicle0
		{-6.0f, 0.5f }, //Vehicle1
	},
	{	//DM_WF__Box,
		{ 0.0f, 0.25f},  //Vehicle0
		{ 0.0f, 0.75f }, //Vehicle1
	},
	{	//DM_WF__BackLeft_FrontRight,
		{ 6.0f, 0.75f},  //Vehicle0
		{-6.0f, 0.25f }, //Vehicle1
	},
	{	//DM_WF__BackRight_FrontLeft,
		{ 6.0f, 0.25f},  //Vehicle0
		{-6.0f, 0.75f }, //Vehicle1
	},
	{	//DM_WF__Arrow,
		{ 6.0f, 0.5f},   //Vehicle0
		{ -6.0f, 0.25f }, //Vehicle1
	},
	{	//DM_WF__Diamond,
		{ 6.0f, 0.5f},  //Vehicle0
		{ -6.0f, 0.25f }, //Vehicle1
	},
};

static Maths::HF32 s_ThreeVehicleWaveData[ DM_WF__Max ][3][2] = 
{		// Normal, Lateral
	{	//DM_WF__Line,
		{ 0.0f, 0.25f},  //Vehicle0
		{ 0.0f, 0.50f }, //Vehicle1
		{ 0.0f, 0.75f }, //Vehicle2
	},
	{	//DM_WF__Queue,
		{ 12.0f, 0.5f},  //Vehicle0
		{ 0.0f, 0.5f }, //Vehicle1
		{-12.0f, 0.5f },//Vehicle2
	},
	{	//DM_WF__Box,
		{ 6.0f, 0.25f},  //Vehicle0
		{ 6.0f, 0.75f }, //Vehicle1
		{-6.0f, 0.25f }, //Vehicle2
	},
	{	//DM_WF__BackLeft_FrontRight,
		{  12.0f, 0.75f},  //Vehicle0
		{  0.0f,  0.5f },  //Vehicle1
		{ -12.0f, 0.25f }, //Vehicle2
	},
	{	//DM_WF__BackRight_FrontLeft,
		{  12.0f, 0.25f},  //Vehicle0
		{  0.0f,  0.5f },  //Vehicle1
		{ -12.0f, 0.75f }, //Vehicle2
	},
	{	//DM_WF__Arrow,
		{ 6.0f, 0.5f},   //Vehicle0
		{-6.0f, 0.25f }, //Vehicle1
		{-6.0f, 0.75f }, //Vehicle2
	},
	{	//DM_WF__Diamond,
		{ 6.0f, 0.5f},   //Vehicle0
		{-6.0f, 0.25f }, //Vehicle1
		{-6.0f, 0.75f }, //Vehicle2
	},
};

static Maths::HF32 s_FourVehicleWaveData[ DM_WF__Max ][4][2] = 
{		// Normal, Lateral
	{	//DM_WF__Line,
		{ 0.0f, 0.125f},  //Vehicle0
		{ 0.0f, 0.375f }, //Vehicle1
		{ 0.0f, 0.625f }, //Vehicle2
		{ 0.0f, 0.875f }, //Vehicle3
	},
	{	//DM_WF__Queue,
		{ 18.0f, 0.5f},  //Vehicle0
		{ 6.0f, 0.5f }, //Vehicle1
		{-6.0f, 0.5f },//Vehicle2
		{-18.0f, 0.5f }, //Vehicle3
	},
	{	//DM_WF__Box,
		{ 6.0f, 0.25f},  //Vehicle0
		{ 6.0f, 0.75f }, //Vehicle1
		{-6.0f, 0.25f }, //Vehicle2
		{-6.0f, 0.75f }, //Vehicle3
	},
	{	//DM_WF__BackLeft_FrontRight,
		{  18.0f, 0.875f },  //Vehicle0
		{  6.0f,  0.625f },  //Vehicle1
		{ -6.0f,  0.375f }, //Vehicle2
		{ -18.0f,   0.125f }, //Vehicle3
	},
	{	//DM_WF__BackRight_FrontLeft,
		{  18.0f, 0.125f },  //Vehicle0
		{  6.0f,  0.375f },  //Vehicle1
		{ -6.0f,  0.625f }, //Vehicle2
		{ -18.0f,   0.875f }, //Vehicle3
	},
	{	//DM_WF__Arrow,
		{ 12.0f, 0.5f},  //Vehicle0
		{ 0.0f, 0.25f }, //Vehicle1
		{ 0.0f, 0.50f }, //Vehicle2
		{ 0.0f, 0.75f }, //Vehicle3
	},
	{	//DM_WF__Diamond,
		{ 12.0f, 0.5f},   //Vehicle0
		{ 0.0f, 0.25f }, //Vehicle1
		{ 0.0f, 0.75f }, //Vehicle2
		{ -12.0f, 0.5f }, //Vehicle3
	},
};

static int s_VehicleWidthData[ DM_WF__Max ][4]= 
{		
		//DM_WF__Line,
		{1,2,3,4},
	
		//DM_WF__Queue,
		{1,1,1,1},
	
		//DM_WF__Box,
		{1,2,2,2},
	
		//DM_WF__BackLeft_FrontRight,
		{1,1,1,1},
		
		//DM_WF__BackRight_FrontLeft,
		{1,1,1,1},
		
		//DM_WF__Arrow,
		{1,2,2,3},
		
		//DM_WF__Diamond,
		{1,2,2,2},
};


namespace airacingtech
{
	using namespace Game;

	//=====================================================================
	// RouteCache
	// - A helper class which allows us to cache and count the routes 
	//   under consideration for spawning the current wave
	//=====================================================================
	class RouteCache
	{
	public:
		
		void								addRoute( airacingtech::AITrackDataRoutes* route );
		int									getCount( airacingtech::AITrackDataRoutes* route );
		int									getRouteCount() const;
		airacingtech::AITrackDataRoutes*	getIthRoute(int index) const;
		void								addSpawned( airacingtech::AITrackDataRoutes* route );
		int									getSpawnedCount( airacingtech::AITrackDataRoutes* route );
	private:

		struct Element
		{
			Element(): mRoute(NULL),mCount(0),mSpawned(0) {}

			airacingtech::AITrackDataRoutes* mRoute;
			int	mCount;
			int mSpawned;
		};

		Btl::Vector<Element, Btl::VectorParameters_Array<CModeDestruction::MAX_DESTRUCTION_ROUTES> > mRouteData;
	};

	//---------------------------------------------------------------------
	// addRoute
	// - If the same route has been already been cached, increase its ref
	//   count. If not, insert it and set to 1.
	//---------------------------------------------------------------------
	void
	RouteCache::addRoute( airacingtech::AITrackDataRoutes* route )
	{
		for(int i=0; i<CModeDestruction::MAX_DESTRUCTION_ROUTES; i++)
		{
			if( mRouteData[i].mRoute == route )
			{
				mRouteData[i].mCount++;
				return;
			}
		}
		for(int i=0; i<CModeDestruction::MAX_DESTRUCTION_ROUTES; i++)
		{
			if( mRouteData[i].mRoute == NULL )
			{
				mRouteData[i].mRoute = route;
				mRouteData[i].mCount++;
				return;
			}
		}
		BIZ_ASSERT_FAIL("RouteCache::addRoute() more routes than we can handle.");
	}

	//---------------------------------------------------------------------
	// getCount
	// - If a route exists in this cache we return its reference count.
	//   Otherwise 0.
	//---------------------------------------------------------------------
	int
	RouteCache::getCount( airacingtech::AITrackDataRoutes* route )
	{
		for(int i=0; i<CModeDestruction::MAX_DESTRUCTION_ROUTES; i++)
		{
			if( mRouteData[i].mRoute == route )
			{
				return mRouteData[i].mCount;
			}
		}	
		return 0;
	}

	//---------------------------------------------------------------------
	// getRouteCount
	// - Returns the number of unique routes cached
	//---------------------------------------------------------------------
	int
	RouteCache::getRouteCount() const
	{
		int count = 0;
		for(int i=0; i<CModeDestruction::MAX_DESTRUCTION_ROUTES; i++)
		{
			if( mRouteData[i].mRoute != NULL )
			{
				count++;
			}
		}	
		return count;
	}

	//---------------------------------------------------------------------
	// getRouteCount
	// - Returns the ith route cached
	//---------------------------------------------------------------------
	airacingtech::AITrackDataRoutes*
	RouteCache::getIthRoute(int index) const
	{
		int count = 0;
		for(int i=0; i<CModeDestruction::MAX_DESTRUCTION_ROUTES; i++)
		{
			if( mRouteData[i].mRoute != NULL )
			{
				if( count == index)
				{
					return mRouteData[i].mRoute;
				}
				count++;
			}
		}	
		return NULL;
	}

	//---------------------------------------------------------------------
	// addSpawned
	// - add to the spawned count of the given route
	//---------------------------------------------------------------------
	void
	RouteCache::addSpawned(airacingtech::AITrackDataRoutes* route)
	{
		for(int i=0; i<CModeDestruction::MAX_DESTRUCTION_ROUTES; i++)
		{
			if( mRouteData[i].mRoute == route )
			{
				mRouteData[i].mSpawned++;
				return;
			}
		}	
		// Although this is safe to return we shouldn't ever get
		// here and I want to know why we have!
		BIZ_ASSERT_FAIL(0,"RouteCache::removeRoute() - Tried to remove a route which doesn't exist");
	}
	//---------------------------------------------------------------------
	// getSpawnedCount
	// - add to the spawned count of the given route
	//---------------------------------------------------------------------
	int
	RouteCache::getSpawnedCount(airacingtech::AITrackDataRoutes* route)
	{
		for(int i=0; i<CModeDestruction::MAX_DESTRUCTION_ROUTES; i++)
		{
			if( mRouteData[i].mRoute == route )
			{
				return mRouteData[i].mSpawned;
			}
		}	
		return -1;
	}

	//=====================================================================
	// CModeDestruction
	//=====================================================================
	CModeDestruction::CModeDestruction() :m_pIGameServerTime(0), m_pITrackData(0)
	{
		m_AIlist.Clear();
		m_startRace = false;	
		m_lastTime = 0.0f;

		m_currentWave = 0;
		m_lastWave = 0;
		m_currentBalancingIndex = 0;

	}

	CModeDestruction::~CModeDestruction()
	{
		
	}

	void CModeDestruction::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_MODE_DESTRUCTION, CID_MODE_DESTRUCTION);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_MODE_DESTRUCTION, CModeDestruction::CreateMe, CModeDestruction::DestroyMe, CHash("ModeDestructionSystemAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_DESTRUCTION, MT_ADD_AI_DRIVER_TO_LIST);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_DESTRUCTION, MT_SET_GAMESTATE_GAME);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_DESTRUCTION, MT_START_AI_RACE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_DESTRUCTION, MT_SET_GAME_MODE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_DESTRUCTION, MT_PRE_UPDATE_GAME_MODE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_DESTRUCTION, MT_POST_INIT_AI_SYSTEM);
		
		
	}

	IComponent *CModeDestruction::CreateMe()
	{
		return new CModeDestruction;
	}

	bool CModeDestruction::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CModeDestruction::Init(CObjectId oid, CParameterNode &paramNode)
	{
		const AmaxRaceModeDestruction* destructionSetupData = (const AmaxRaceModeDestruction*)paramNode.GetData();
		m_DestructionModeSetupData = destructionSetupData;
		return true;
	}

	void CModeDestruction::Deinit(void)
	{
		m_AIlist.Clear();
	}

	EMessageResult CModeDestruction::HandleMessage(const CMessage &msg)
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


			case MT_ADD_AI_DRIVER_TO_LIST:
				{
					msgRet =  MR_TRUE;
					CObjectId *pParams = static_cast<CObjectId *>(msg.mpData);
					const CObjectId& id = *pParams;
					IData *pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));
					
					CObjectId systemID("AmaxSystemAI");
					ISystemAI* systemAI = static_cast<ISystemAI *>(GlobalsComponent::GetObjectManager().QueryInterface(systemID, IID_SYSTEM_AI));


					DestructionAIData destructionAI;
					destructionAI.m_Data = pIData;
					destructionAI.m_Type = DAIT_None;

					// remember to handle the player. They don't have an AI type
					int index = systemAI->GetIndexFromObjectID(id);
					if( index == 0 )
					{
						destructionAI.m_Type = DAIT_None;
					}
					else
					{
					
						Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
						Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(index);
						Game::Actors::C_VehicleActor *pActor = pLogicActor->GetGraphicalActor(index);

						U32 vehicleID = pActor->GetXtIndex();
						if( vehicleID == m_DestructionModeSetupData->vehicleEasy->vehicle->ID )
						{
							destructionAI.m_Type = DAIT_Easy;
						}
						else if( vehicleID == m_DestructionModeSetupData->vehicleMedium->vehicle->ID )
						{
							destructionAI.m_Type = DAIT_Medium;
						}
						else if( vehicleID == m_DestructionModeSetupData->vehicleHard->vehicle->ID )
						{
							destructionAI.m_Type = DAIT_Hard;
						}
						else if( vehicleID == m_DestructionModeSetupData->vehicleVHard->vehicle->ID )
						{
							destructionAI.m_Type = DAIT_VHard;
						}
					}

					m_AIlist.PushBack(destructionAI);	
					m_AIPool.PushBack(destructionAI);
				}
				break;

			case MT_START_AI_RACE:
				{
					m_startRace = true;
					m_currentBalancingIndex = 0;
				}
				break;

			case MT_SET_GAME_MODE:
				{
					InitialiseMode();
				}
				break;

			case MT_PRE_UPDATE_GAME_MODE:
				{
					if (m_startRace)
					{
						Update();
					}
					
				}
				break;

			

			default:
				break;
		}

		return msgRet;
	}

	EComponentTypeId CModeDestruction::GetComponentTypeId(void)
	{
		return CID_MODE_DESTRUCTION;
	}

	void CModeDestruction::CachedInterfaces()
	{
		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));
	
	
		CObjectId trackdataSystem("TrackDataSystem");
		m_pITrackData = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));

	
	}

	void CModeDestruction::PreUpdate()
	{

	}


	void CModeDestruction::Update()
	{
		RLGameMode_Destruction* destructionParams = &RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction;


		int waveCount = m_DestructionModeSetupData->waves.count;
		int	waveCountMaxActive = destructionParams->mMaxWavesActive;
		int wavesDiff = m_currentWave - m_lastWave;
		
		wavesDiff = wavesDiff < 0?(wavesDiff + waveCount):wavesDiff;
		BIZ_ASSERT(wavesDiff>=0);

		if( g_DestructionSpawningEnabled )
		{

			if (m_pIGameServerTime && m_pIGameServerTime->GetTime() > m_lastTime || 
				wavesDiff == 0 || 
				g_DestructionSpawnNextWave )
			{
				
				// and we have the option to limit the max number
				// of waves active at a time	
				
				
				RemoveVehiclesBehind();
				
				// If we have no data dont continue
				if(waveCount > 0 )
				{
					// Dont spawn the first wave until we cross the start line.
					if( m_AIlist[0].m_Data->GetVehicleLogic()->GetLogicState()->GetLap() > 0 )
					{
					
						if( wavesDiff < waveCount )
						{

							if( (wavesDiff < (waveCountMaxActive)) || 
								g_DestructionSpawnNextWave )
							{

								// Clamp the maximum number of 'AI' vehicles active
								// at any time. Set to MAX_VEHICLES-1 to allow all.
								if (m_AIPoolActive.Size() < destructionParams->mMaxAIVehiclesActive )
								{
									
									if(ActivateWave(m_currentWave))
									{
										m_currentWave++;
										if (m_currentWave >= m_DestructionModeSetupData->waves.count)
										{
											m_currentWave = 0;
											m_currentBalancingIndex = 0;
										}
									}
								}
							}
						}
					}				
				}

				m_lastTime = m_pIGameServerTime->GetTime() + 0.5f;	
			}
		}

		CheckForCompletedWaves();
		CheckForManualHOGPlacement(); 

		if(g_DestructionKillAllActive)
		{
			RemoveVehiclesBehind();
			g_DestructionKillAllActive = false;
			m_lastWave = m_currentWave;
		}
	}

	void CModeDestruction::PostUpdate()
	{

	}

	void CModeDestruction::InitialiseMode()
	{	
		PopulatePool();	

		DeActivatePool();

		DisableAutomaticHOGPlacementForAI(); // HOG detection as usual, but placement on track will be done manually by us
	}

	void CModeDestruction::DisableAutomaticHOGPlacementForAI()
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		IData *pIDataPlayer = m_AIlist[0].m_Data;

		for(int i=0; i< m_AIPool.Size(); i++)
		{
			IData *pIData = m_AIPool[i].m_Data;

			if (pIDataPlayer != pIData)
			{
				Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(pIData->GetState().m_index);
				BIZ_ASSERT_MSG(pLogicActor, "CModeDestruction::DisableAutomaticHOGPlacementForAI() < unable to obtain pointer to logic actor");

				// Tell the HOG components to still detect when to initiate a reset, but to leave it to us to actually do it
				Game::Actors::C_EnableDisableAutoHOGPlacementMsg hogEnabled(false);
				pLogicActor->GetActorList()->SendMessageToActorComponent(pLogicActor->GetHandle(),Components::E_IF_VehicleComponent_HandOfGod,&hogEnabled);
			}
		}
	}

	// Since HOG placement is disabled in Destruction mode, vehicle simply flag themselves as requiring a HOG reset but
	// leave it up to us to actually put them back on track (hopefully) in a sensible place
	void CModeDestruction::CheckForManualHOGPlacement()
	{
		// Lets iterate our active pool and check if any AIs are in need of assistance
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		IData *pIDataPlayer = m_AIlist[0].m_Data;

		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			IData *pIData = m_AIPoolActive[i].m_Data;

			if (pIDataPlayer != pIData)
			{
				Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(pIData->GetState().m_index);
				BIZ_ASSERT_MSG(pLogicActor, "CModeDestruction::CheckForManualHOGPlacement() < unable to obtain pointer to logic actor");

				Components::C_VehicleComponent_HandOfGod* pHOG = (Components::C_VehicleComponent_HandOfGod*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod);
				BIZ_ASSERT_MSG(pHOG, "CModeDestruction::CheckForManualHOGPlacement() < unable to obtain pointer to vehicle HOG component");

				if(pHOG->IsManualHOGResetRequired())
				{
					// Grab the Current route from the AI to calc its currentLogicIndex
					const airacingtech::RouteInfo *pRouteInfo = pIData->GetSplineTracker()->GetLogicRoute();
					BIZ_ASSERT(pRouteInfo);
					airacingtech::AITrackDataRoutes *pRoute = pRouteInfo->m_pRoute;
					BIZ_ASSERT(pRoute);
					int currentLogicIndex = pRouteInfo->m_index;
						
					// Work out the distance (in Metres) between the AI and the player
					float playerPos = pIDataPlayer->GetVehicleLogic()->GetLogicState()->GetLogicPosition();	
					float thisPos = pIData->GetVehicleLogic()->GetLogicState()->GetLogicPosition();	
					float dist = AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(playerPos, thisPos);
				
					// store the type && wave of the AI before we deactivate it
					DestructionAIType aiType = m_AIPoolActive[i].m_Type;
					int aiWave = m_AIPoolActive[i].m_wave;

					// deactivate this vehicle (does placevehicles reactivate????) 
					DeActivateVehicle(pIData, true);

					// Will the AI car spawn >= *SAFE_DIST*  ahead of player
					int newSpawnLogicIndex = -1;
					if( dist > DESTRUCTION_SAFE_SPAWN_DIST )
					{
						// Then we just spawn them back on the track where they want to be!
						newSpawnLogicIndex = currentLogicIndex;
					}
					else
					{
						// This AI is spawning too close to the player.. we need to offset them 150 ahead
						// simple respanw at current logic index
						newSpawnLogicIndex = pRoute->GetIndexOffset(currentLogicIndex, DESTRUCTION_SAFE_SPAWN_DIST/4);
					}
					//BIZ_ASSERT_MSG(newSpawnLogicIndex >= 0,"CModeDestruction::CheckForManualHOGPlacement() < newSpawnLogicIndex < 0");
					// Dont ever spawn on logic volume 0. As the AI data functions don't handle this well
					//newSpawnLogicIndex = newSpawnLogicIndex==0?1:newSpawnLogicIndex;
					// Darren : 
					newSpawnLogicIndex = Maths::Max(1, newSpawnLogicIndex);


					// Lets grab the data for the new spawn logic segment so we can work out which route to spawn on
					const airacingtech::AITrackDataCache& dc = pRoute->m_dataCache[newSpawnLogicIndex];
					
					// For now , just take the first route we find that seems valid
					const airacingtech::AITrackDataRoutes* pNewRoute = NULL;
					const int noEntries = dc.m_data.Size();	
					for(int entries = 0; entries <noEntries; entries++ )
					{
						const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];

						if(cachedData.m_pRoute && cachedData.m_pRoute->IsChangeableRoute())
						{
							pNewRoute = cachedData.m_pRoute;
						}
					}
					// If a new route exists (really.. it always should...) spawn
					// If cant fin any new route just leave deactivated!! Not ideal.. but should CERY rarely (if ever) happen
					if( pNewRoute )
					{
						PlaceVehicles(pRoute,pNewRoute,newSpawnLogicIndex,aiWave, aiType,0.5f);
					}

					pHOG->SetManualHOGResetRequired(false);
					LOGINFO(g_DestructionModeLogger, "ManualHOGPlacement: SUCCESS - vehicle %d", pLogicActor->GetVehicleIndex());
				}	
			}
		}
	}

	void CModeDestruction::PopulatePool()
	{
		// Get rid of the player car from the pool of cars....
		m_AIPool.EraseByIndex(0);
	}

	void CModeDestruction::DeActivatePool()
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		for(int i=0; i<m_AIPool.Size(); i++)
		{
			// We want to deactive the vehicle, ie turn off the rendering, collisions, physics update, etc, etc
			// The logic here works correctly,  the game manages to change some of these states, but that is someonelses problem.
			// Message below tries to aliviate the problem.

			IData *pIData = m_AIPool[i].m_Data;
			RemoveVehicleFromWorld(pIData->GetState().m_index);
		}
	}


	bool CModeDestruction::ActivateWave(int waveIndex)
	{
		// Only valid wave index's please.
		BIZ_ASSERT(	waveIndex >= 0 && 
					waveIndex < m_DestructionModeSetupData->waves.count);

		
		airacingtech::AITrackData *pAITrackData = 0;
		if (m_pITrackData)
		{
			pAITrackData = m_pITrackData->GetTrackData();
		}

		
		RLGameMode_Destruction* destructionParams = &RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction;
		const DestructionMode_Wave& waveData = m_DestructionModeSetupData->waves[waveIndex];

		E_DestructionMode_Wave_Formation chosenFormation = waveData.waveFormation;

		LOGINFO(g_DestructionModeLogger, "ActivateWave: %d  formation = %s", waveIndex, s_formationString[chosenFormation]);
		int seperationDist = static_cast<int>(waveData.waveSeparationDistance);
	
		float furthestDist;
		int furthestIndex = FindFurthestAhead(furthestDist);
		
		IData *pIDataThis = 0;
		IData *pIDataPlayer = m_AIlist[0].m_Data;
		BIZ_ASSERT(pIDataPlayer);

		if (furthestIndex == -1 || furthestDist < 0.0f )
		{
			// player
			pIDataThis = pIDataPlayer;
			furthestDist = 0.0f;
		}
		else 
		{
			IData *pIData = m_AIPoolActive[furthestIndex].m_Data;
			BIZ_ASSERT(pIData);
			pIDataThis = pIData;
		}
		

		if(pIDataThis)
		{
			if( (furthestDist + seperationDist) < destructionParams->mForwardsCutOffDistance )
			{
				Maths::HF32 offsetNormal;
				Maths::HF32 offsetLateral;
				DestructionAIType type;

				const airacingtech::RouteInfo *pRouteInfo = pIDataThis->GetSplineTracker()->GetLogicRoute();
				BIZ_ASSERT(pRouteInfo);
				airacingtech::AITrackDataRoutes *pRoute = pRouteInfo->m_pRoute;
				BIZ_ASSERT(pRoute);
				int currentLogicIndex = pRouteInfo->m_index;

				const airacingtech::RouteInfo *pCurrentRouteInfo = pIDataThis->GetSplineTracker()->GetCurrentRoute();
				
				int numVehicles = 0;
				numVehicles += waveData.waveAI.slot1 != DM_AI__None ? 1:0;
				numVehicles += waveData.waveAI.slot2 != DM_AI__None ? 1:0;
				numVehicles += waveData.waveAI.slot3 != DM_AI__None ? 1:0;
				numVehicles += waveData.waveAI.slot4 != DM_AI__None ? 1:0;
				
				
				// Ultimately we want a route to spawn each of the vehicles on 
				AITrackDataRoutes* routesToSpawnOn[MAX_VEHICLES_IN_WAVE];
				for(int i=0;i<MAX_VEHICLES_IN_WAVE;i++)
				{
					routesToSpawnOn[i] = NULL;
				}

				// For each vehicle store the number of routes at its spawn point
				Btl::Vector<AITrackDataRoutes*, Btl::VectorParameters_Static<MAX_DESTRUCTION_ROUTES> > routesAtSpawn[MAX_VEHICLES_IN_WAVE];
				int logicIndexAtSpawn[MAX_VEHICLES_IN_WAVE];

				// Iterate the list of vehicles to spawn and populate our route list
				for(int i=0;i<numVehicles;i++)
				{
					GetFormationOffsets(chosenFormation,i,numVehicles,offsetNormal,offsetLateral);	

					int spawnLogicIndex = pRoute->GetIndexOffset(currentLogicIndex, (int)((seperationDist + offsetNormal)/4));
					const airacingtech::AITrackDataCache& dc = pRoute->m_dataCache[spawnLogicIndex];

					// Dont ever spawn on logic volume 0. As the AI data functions don't handle this well
					logicIndexAtSpawn[i] = spawnLogicIndex==0?1:spawnLogicIndex;

					const int noEntries = dc.m_data.Size();	
					for(int entries = 0; entries <noEntries; entries++ )
					{
						const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];

						if (cachedData.m_pRoute && cachedData.m_pRoute->IsChangeableRoute())
						{
							// A potential candidate route for spawning this AI on
							bool addToList = true;

							if (pAITrackData)
							{					
								// Crossovers... this won`t work when the crossover is a different route
								if (pCurrentRouteInfo && pCurrentRouteInfo->m_pRoute == cachedData.m_pRoute)
								{
									float dist = static_cast<float>(pAITrackData->m_TrackLogicData.m_stepSize * airacingtech::DistanceInfrontBehind(cachedData.m_index , pCurrentRouteInfo->m_index, pAITrackData->m_TrackLogicData.m_noSamples));
									if (dist < 0.0f || dist > 300.0f) // probably a good check to have in 
									{
										addToList = false;
									}
								}

								// lets check the angle of the logic route to the angle of the route it contains...
								Maths::HVec3 dirRoute;
								cachedData.m_pRoute->GetDirection(cachedData.m_index,dirRoute);
								
								Maths::HVec3 dirLogicRoute;
								pRouteInfo->m_pRoute->GetDirection(pRouteInfo->m_index,dirLogicRoute);

								// Using angle instead of dot product as it is much easier to visualise and debug
								float angle = airacingtech::AngleBetween(dirLogicRoute, dirRoute);
								angle = Maths::RadToDeg(angle);
							
								if (Maths::Abs(angle) > 75.0f) // Maybe needs tuning ?
								{
									addToList = false;
								}
							}
							
							if (addToList)
							{
								// First, check if it already exists in the list
								if(Btl::FindIndex(routesAtSpawn[i],cachedData.m_pRoute)!=routesAtSpawn[i].Size())
								{
									// Found a duplicate entry so don't add again.
									addToList = false;
								}

								// Second, check if the centre point of the route segment lies 
								// within the bounds of the logic segment it is included in.
								// There are cases where routes overlap into multiple logic 
								// segments which is caused by bad data but we need to handle this.
								Maths::HVec3 bounds[2];
								Maths::HVec3 boundsCentre;

								cachedData.m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, cachedData.m_index , bounds[0]);
								cachedData.m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, cachedData.m_index , bounds[1]);

								boundsCentre = (bounds[0] + bounds[1])/2.0f;

								Maths::HVec3 boundsCentreFlat(boundsCentre.X(),0.0f,boundsCentre.Z());
								if( !pRoute->InSegmentWidths(boundsCentreFlat,logicIndexAtSpawn[i]))
								{
									// Oopsie. Overlapping routes, bad data, meh!
									addToList = false;
								}

								if( addToList )
								{
									routesAtSpawn[i].PushBack(cachedData.m_pRoute);
									LOGDEBUG(g_DestructionModeLogger, "Vehicle: %d PotentialRoute %d = 0x%x", i, routesAtSpawn[i].Size(), cachedData.m_pRoute );
								}
							}
							
						}
					}
				}

				// If a route exists at the players current position
				// and it is in any vehicles list we can remove it. 
				// This is so that we favour the players current route 
				// in cases where we know we will be spawning them on a 
				// route we know they can get to.
				if( destructionParams->mFavourPlayersOwnRoute)
				{
					const airacingtech::RouteInfo *pRouteInfo = pIDataPlayer->GetSplineTracker()->GetLogicRoute();
					BIZ_ASSERT(pRouteInfo);
					airacingtech::AITrackDataRoutes *pRoute = pRouteInfo->m_pRoute;
					BIZ_ASSERT(pRoute);
					int currentPlayerLogicIndex = pRouteInfo->m_index;

					const airacingtech::RouteInfo *pCurrentRouteInfo = pIDataPlayer->GetSplineTracker()->GetCurrentRoute();
					airacingtech::AITrackDataRoutes *pCurrentRoute = pCurrentRouteInfo?pCurrentRouteInfo->m_pRoute:NULL;
					
					const airacingtech::AITrackDataCache& dc = pRoute->m_dataCache[currentPlayerLogicIndex];

					LOGDEBUG(g_DestructionModeLogger, "Players Current Route = 0x%x", pCurrentRoute );

					// Only remove routes from our lists 
					// if the player has a current route
					if(pCurrentRoute)
					{

						
						const int noEntries = dc.m_data.Size();	

						bool foundPlayerRoute  = false;

						for(int i=0;i<numVehicles;i++)
						{
							if(Btl::FindIndex(routesAtSpawn[i],pCurrentRoute)!=routesAtSpawn[i].Size())
							{
								foundPlayerRoute = true;
							}
						}

						// Don't remove routes if the players current route
						// is not found int
						if(foundPlayerRoute)
						{

							for(int entries = 0; entries <noEntries; entries++ )
							{
								const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];

								if (cachedData.m_pRoute && 
									cachedData.m_pRoute->IsChangeableRoute() )
								{

									LOGDEBUG(g_DestructionModeLogger, "Player Segment Route = 0x%x", cachedData.m_pRoute );

									if(cachedData.m_pRoute != pCurrentRoute)
									{
										// A potential route to be removed.
										// Check if this route exists in any of the 
										// vehicle route lists and if so, remove it.
										for(int i=0;i<numVehicles;i++)
										{
											// Do not remove if this is the last route in this list
											if(routesAtSpawn[i].Size() > 1)
											{
												Btl::RemoveNonOrdered(routesAtSpawn[i],cachedData.m_pRoute);
											}
										}
									}
								}
							}
						}
					}
				}

				// Collect together some data into a cache we can query
				// and update as we are spawning vehicles.
				int totalNumDifferentRoutes = 0;
				RouteCache cache;
				{
					for(int i=0;i<numVehicles;i++)
					{
						for(int j=0; j<routesAtSpawn[i].Size(); j++)
						{
							BIZ_ASSERT(routesAtSpawn[i][j] != NULL);
							cache.addRoute(routesAtSpawn[i][j]);
							LOGDEBUG(g_DestructionModeLogger, "Vehicle: %d FinalRoute %d = 0x%x", i, j, routesAtSpawn[i][j] );
						}
					}
				}

				// Now handle the different cases this throws up.

				// Information we have
				// numVehicles: how many to spawn
				// logicIndexAtSpawn[]: desired logic route segment for each vehicle to spawn at
				// routesAtSpawn[]: variable length list of routes at the spawn segment 
				
				Btl::BitArray16 needsSpawning;
				needsSpawning.ClearBits();
				for( int i=0; i<numVehicles; i++ )
				{
					needsSpawning.SetBit(i);
				}
				
				for( int i=0; i<numVehicles; i++ )
				{
					Btl::BitArray16 toSpawn;
					toSpawn.ClearBits();
					if(needsSpawning.GetBitValue(i))
					{
						toSpawn.SetBit(i);
						
						// So we still have this vehicle to spawn. Are any other vehicles 
						// marked down to spawn at the same segment as this one?
						for( int j=i+1; j<numVehicles; j++ )
						{
							if(needsSpawning.GetBitValue(j))
							{
								if(logicIndexAtSpawn[i] == logicIndexAtSpawn[j])
								{
									toSpawn.SetBit(j);
								}
							}
						}

						// toSpawn now contains the vehicles to be spawned
						// on the same logical segment.
						int numVehiclesToSpawn = toSpawn.CountBitsSet();
						int numRoutesToSpawnOn = routesAtSpawn[i].Size();
						//BIZ_ASSERT(numRoutesToSpawnOn > 0);				
						
						if (numRoutesToSpawnOn > 0)
						{
							// If there is only one route to spawn on it is 
							// a very clear case of which one we will be 
							// spawning on. 
							if( numRoutesToSpawnOn == 1 )
							{
								// Can this route handle 'numVehiclesToSpawn'
								// ignore that question for now
								for( int j=0; j<numVehicles; j++ )
								{
									if(toSpawn.GetBitValue(j))
									{
										type = GetIthVehicleTypeInWave(waveIndex,j);
										GetFormationOffsets(chosenFormation,j,numVehicles,offsetNormal,offsetLateral);	
										PlaceVehicles(pRoute,routesAtSpawn[i][0],logicIndexAtSpawn[j],waveIndex, type,offsetLateral);

										cache.addSpawned(routesAtSpawn[i][0]);
									}
								}
							}
							else
							{
								// For each vehicle to be spawned at this segment
								// find the route with the least spawned vehicles
								// spawned on it so far.
								for( int j=0; j<numVehicles; j++ )
								{
									if(toSpawn.GetBitValue(j))
									{
										int indexWithLeastSpawned = -1;
										int leastSpawned = 100000000;
										for(int k=0;k<numRoutesToSpawnOn;k++)
										{
											int numSpawnedOnRoute = cache.getSpawnedCount(routesAtSpawn[i][k]);
											if( numSpawnedOnRoute < leastSpawned)
											{
												leastSpawned = numSpawnedOnRoute;
												indexWithLeastSpawned = k;
											}
											else if( numSpawnedOnRoute==leastSpawned )
											{
												// In the case where we find two routes with 
												// the same number spawned on each, look
												// to the total No. of each routes cached.
												if( cache.getCount(routesAtSpawn[i][k]) < cache.getCount(routesAtSpawn[i][indexWithLeastSpawned]))
												{
													indexWithLeastSpawned = k;
												}
											}
										}

										// Theres no reason to not find a route
										BIZ_ASSERT(indexWithLeastSpawned>=0);

										type = GetIthVehicleTypeInWave(waveIndex,j);
										GetFormationOffsets(chosenFormation,j,numVehicles,offsetNormal,offsetLateral);	
										PlaceVehicles(pRoute,routesAtSpawn[i][indexWithLeastSpawned],logicIndexAtSpawn[j],waveIndex, type,offsetLateral);

										cache.addSpawned(routesAtSpawn[i][indexWithLeastSpawned]);

									}
								}
							}
						}
	

						// Now that we have dealt with theses vehicles, clear their bits in
						// the 'needsSpawning' bit field
						for( int j=0; j<numVehicles; j++ )
						{
							if(toSpawn.GetBitValue(j))
							{
								needsSpawning.ClearBit(j);
							}
						}
					}
				}

				// Now we have spawned all the cars from our wave. 
				// We now check to see if there were any routes which
				// didn't get any cars spawned on them.
				if( destructionParams->mSpawnExtraAIToFillAllRoutes )
				{

					for(int r=0;r<cache.getRouteCount();r++)
					{
						airacingtech::AITrackDataRoutes* route = cache.getIthRoute(r);
						if(cache.getSpawnedCount(route)==0)
						{
							// We have found an empty route...
							// Do we have any spare vehicles left
							if( m_AIPool.Size() > 0 )
							{
								int logicIndexToSpawnAt = -1;
								for(int i=0;i<numVehicles;i++)
								{
									for(int j=0; j<routesAtSpawn[i].Size(); j++)
									{
										if( routesAtSpawn[i][j] == route )
										{
											logicIndexToSpawnAt = logicIndexAtSpawn[i];
										}
									}
								}

								BIZ_ASSERT(logicIndexToSpawnAt!=-1);

								GetFormationOffsets(chosenFormation,0,1,offsetNormal,offsetLateral);	
								PlaceVehicles(pRoute, route,logicIndexToSpawnAt,waveIndex, m_AIPool[0].m_Type,offsetLateral,true);
								
								LOGDEBUG(g_DestructionModeLogger, "Spawning extra vehicle to satify unfilled route" );

									
							}
						}
					}
				}
				return true;
			}
		}

		LOGWARN(g_DestructionModeLogger, "ActivateWave: %d FAILED", waveIndex);
		return false;
	}

	bool  
	CModeDestruction::CanTrackAccomodateBox(
		AITrackDataRoutes* desiredRoute,
		IData *pIData, 
		F32 frontDistanceAhead, 
		F32 behindDistanceAhead, 
		F32 width 
	) const
	{
		const airacingtech::RouteInfo *pRouteInfo = pIData->GetSplineTracker()->GetLogicRoute();
		BIZ_ASSERT(pRouteInfo);
		airacingtech::AITrackDataRoutes *pRoute = pRouteInfo->m_pRoute;
		BIZ_ASSERT(pRoute);

		int logicIndex = pRouteInfo->m_index;
		int behindIndex = pRoute->GetIndexOffset(logicIndex, (int)(behindDistanceAhead/4));
		int aheadIndex = pRoute->GetIndexOffset(logicIndex, (int)(frontDistanceAhead/4));
		int numSegments = pRoute->GetNumSegments();

		// Push the aheadIndex on by one for our loop below
		aheadIndex++;
		if(aheadIndex >= numSegments)
		{
			aheadIndex -= numSegments;
		}

		for(int i=behindIndex; i!=aheadIndex;)
		{
			const airacingtech::AITrackDataCache& dc = pRoute->m_dataCache[i];

			bool foundRoute = false;
			const int noEntries = dc.m_data.Size();	
			for(int entries = 0; entries <noEntries; entries++ )
			{
				const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];

				if (cachedData.m_pRoute && cachedData.m_pRoute->IsChangeableRoute() )
				{
					// leave our players current route in the list
					if( cachedData.m_pRoute == desiredRoute )
					{
						if(pRoute->GetWidth( i ) < width)
						{
							return false;
							foundRoute = true;
							break;
						}
					}
				}
			}

			if( !foundRoute )
			{
				return false;
			}

			
			i++;
			if(i >= numSegments)
			{
				i -= numSegments;
			}
		}
		return true;
	}

	void 
	CModeDestruction::GetFormationStartAndEndRequired(
		E_DestructionMode_Wave_Formation waveFormation, 
		int numVehicles, 
		F32& frontOffset, 
		F32& backOffset
	) const
	{
	
		static F32 s_maxVehicleLength = 2.0f;

		frontOffset = -Maths::Constants<F32>::Max();
		backOffset = Maths::Constants<F32>::Max();

		switch( numVehicles )
		{

		case 4:
			{
				frontOffset = Maths::Max(frontOffset, s_FourVehicleWaveData[waveFormation][0][0]);
				frontOffset = Maths::Max(frontOffset, s_FourVehicleWaveData[waveFormation][1][0]);
				frontOffset = Maths::Max(frontOffset, s_FourVehicleWaveData[waveFormation][2][0]);
				frontOffset = Maths::Max(frontOffset, s_FourVehicleWaveData[waveFormation][3][0]);
				backOffset = Maths::Min(backOffset, s_FourVehicleWaveData[waveFormation][0][0]);
				backOffset = Maths::Min(backOffset, s_FourVehicleWaveData[waveFormation][1][0]);
				backOffset = Maths::Min(backOffset, s_FourVehicleWaveData[waveFormation][2][0]);
				backOffset = Maths::Min(backOffset, s_FourVehicleWaveData[waveFormation][3][0]);
			}
			break;
		case 3:
			{
				frontOffset = Maths::Max(frontOffset, s_ThreeVehicleWaveData[waveFormation][0][0]);
				frontOffset = Maths::Max(frontOffset, s_ThreeVehicleWaveData[waveFormation][1][0]);
				frontOffset = Maths::Max(frontOffset, s_ThreeVehicleWaveData[waveFormation][2][0]);
				backOffset = Maths::Min(backOffset, s_ThreeVehicleWaveData[waveFormation][0][0]);
				backOffset = Maths::Min(backOffset, s_ThreeVehicleWaveData[waveFormation][1][0]);
				backOffset = Maths::Min(backOffset, s_ThreeVehicleWaveData[waveFormation][2][0]);
			}
			break;
		case 2:
			{
				frontOffset = Maths::Max(frontOffset, s_TwoVehicleWaveData[waveFormation][0][0]);
				frontOffset = Maths::Max(frontOffset, s_TwoVehicleWaveData[waveFormation][1][0]);
				backOffset = Maths::Min(backOffset, s_TwoVehicleWaveData[waveFormation][0][0]);
				backOffset = Maths::Min(backOffset, s_TwoVehicleWaveData[waveFormation][1][0]);
			}
			break;		
		case 1:
			{
				frontOffset = 0.0f;
				backOffset = 0.0f;
			}
			break;
		};

		frontOffset += s_maxVehicleLength / 2.0f;
		backOffset  -= (s_maxVehicleLength / 2.0f);
	}

	F32
	CModeDestruction::GetFormationWidthRequired(
		E_DestructionMode_Wave_Formation waveFormation, 
		int numVehicles
	) const
	{
		// Only valid numVehicles please
		BIZ_ASSERT(	numVehicles >= 1 && 
					numVehicles <= 4);

		static F32 s_maxVehicleWidth = 2.0f;
		static F32 s_minVehicleSeparation = 0.5f;

		int carWidth = s_VehicleWidthData[waveFormation][numVehicles-1];
		F32 width = (carWidth * s_maxVehicleWidth) + ((carWidth-1) * s_minVehicleSeparation);

		return width;
	}

	void
	CModeDestruction::GetFormationOffsets(
		E_DestructionMode_Wave_Formation formation,
		int index, 
		int numVehicles, 
		Maths::HF32& offsetNormal, 
		Maths::HF32& offsetLateral
	) const
	{

		switch( numVehicles )
		{
		case 1:
			{
				offsetNormal = 0.0f;
				offsetLateral = 0.5f;
			}
			break;
		case 2:
			{
				offsetNormal = s_TwoVehicleWaveData[formation][index][0];
				offsetLateral = s_TwoVehicleWaveData[formation][index][1];
			}
			break;
		case 3:
			{
				offsetNormal = s_ThreeVehicleWaveData[formation][index][0];
				offsetLateral = s_ThreeVehicleWaveData[formation][index][1];
			}
			break;
		case 4:
			{
				offsetNormal = s_FourVehicleWaveData[formation][index][0];
				offsetLateral = s_FourVehicleWaveData[formation][index][1];
			}
			break;
		};
		
	}

	CModeDestruction::DestructionAIType 
	CModeDestruction::GetIthVehicleTypeInWave( int waveIndex, int vehicleIndex )
	{
		// Only valid wave index's please.
		BIZ_ASSERT(	waveIndex >= 0 && 
					waveIndex < m_DestructionModeSetupData->waves.count);

		const DestructionMode_Wave& waveData = m_DestructionModeSetupData->waves[waveIndex];
		int vehiclesFound = 0;

		int vehiclesSpawned = 0;
		for(int j=0;j<4;j++)
		{
			E_DestructionMode_AI_Level level = DM_AI__None; 
			switch(j)
			{
			case 0:
				level = waveData.waveAI.slot1;
				break;
			case 1:
				level = waveData.waveAI.slot2;
				break;
			case 2:
				level = waveData.waveAI.slot3;
				break;
			case 3:
				level = waveData.waveAI.slot4;
				break;
			};

			if( level != DM_AI__None )
			{
				if( vehicleIndex == vehiclesFound)
				{
					switch(level)
					{
					case DM_AI__Easy:
						return DAIT_Easy;
						break;
					case DM_AI__Medium:
						return DAIT_Medium;
						break;
					case DM_AI__Hard:
						return DAIT_Hard;
						break;
					case DM_AI__VHard:
						return DAIT_VHard;
						break;
					}
				}
				vehiclesFound++;			
			}
		}

		// If we have requested a vehicle index out of our range, return easy.
		// This happens when we pad the number of vehicles being spawned 
		// in the case where we have to spread a wave over multiple routes.
		return DAIT_Easy;
	}


	void 
	CModeDestruction::PlaceVehicles(
		airacingtech::AITrackDataRoutes* pLogicRoute,
		const airacingtech::AITrackDataRoutes* pRoute,
		int logicSegment,
		int waveIndex, 
		DestructionAIType type, 
		Maths::HF32 offsetLateral,
		bool paddingVehicle
	)
	{
		LOGDEBUG(g_DestructionModeLogger, "PlaceVehicles: route = 0x%x logic segment %d", pRoute, logicSegment );


		BIZ_ASSERT(type != DAIT_None);
		BIZ_ASSERT(pRoute);
		BIZ_ASSERT(pLogicRoute);


		airacingtech::AITrackData *pAITrackData = 0;
		if (m_pITrackData)
		{
			pAITrackData = m_pITrackData->GetTrackData();
		}

		IData *pIDataPlayer = m_AIlist[0].m_Data;
		const airacingtech::RouteInfo *pCurrentRouteInfo = 0;
		if (pIDataPlayer)
		{
			pCurrentRouteInfo = pIDataPlayer->GetSplineTracker()->GetCurrentRoute();
		}
		


		int routeSegment = -1;
	
		const airacingtech::AITrackDataCache& dc = pLogicRoute->m_dataCache[logicSegment];

		const int noEntries = dc.m_data.Size();	
		for(int entries = 0; entries <noEntries; entries++ )
		{
			const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];

			if (cachedData.m_pRoute &&  
				cachedData.m_pRoute == pRoute )
			{

				Maths::HVec3 bounds[2];
				Maths::HVec3 boundsCentre;

				cachedData.m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, cachedData.m_index , bounds[0]);
				cachedData.m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, cachedData.m_index , bounds[1]);

				boundsCentre = (bounds[0] + bounds[1])/2.0f;

				bool create = true;
				if (pAITrackData)
				{					
					// Crossovers... this won`t work when the crossover is a different route
					if (pCurrentRouteInfo && pCurrentRouteInfo->m_pRoute == cachedData.m_pRoute)
					{
						float dist = static_cast<float>(pAITrackData->m_TrackLogicData.m_stepSize * airacingtech::DistanceInfrontBehind(cachedData.m_index , pCurrentRouteInfo->m_index, pAITrackData->m_TrackLogicData.m_noSamples));
						if (dist < 0.0f || dist > 300.0f) // probably a good check to have in 
						{
							create = false;
						}
					}
					
					// lets check the angle of the logic route to the angle of the route it contains...
					Maths::HVec3 dirRoute;
					cachedData.m_pRoute->GetDirection(cachedData.m_index,dirRoute);

					Maths::HVec3 dirLogicRoute;
					pLogicRoute->GetDirection(logicSegment,dirLogicRoute);

					// Using angle instead of dot product as it is much easier to visualise and debug
					float angle = airacingtech::AngleBetween(dirLogicRoute, dirRoute);
					angle = Maths::RadToDeg(angle);

					if (Maths::Abs(angle) > 75.0f) // Maybe needs tuning ?
					{
						create = false;
					}
				}



				// There are some cases where we have the same route in the 
				// logic segment when the data is bad. We can detect this though.
				// The cause of this is when the route loops back on itself but
				// the logic spline is incorrectly placed leading to some overlap
				// capturing the route volume of both parts of this route.
				Maths::HVec3 boundsCentreFlat(boundsCentre.X(),0.0f,boundsCentre.Z());
				if( pLogicRoute->InSegmentWidths( boundsCentreFlat,logicSegment) && create)
				{
					routeSegment = cachedData.m_index;
					break;
				}
			}
		}
		
		//BIZ_ASSERT(routeSegment != -1);
		if (routeSegment > 0)
		{
			routeSegment = Maths::Max(routeSegment, 1);
			Maths::HVec3 centrePosition; 

			Maths::HF32 minY, maxY;
			Maths::HVec3 direction(0.0f,0.0f,1.0f);

			pRoute->GetHeightBounds(minY, maxY, routeSegment);
			pRoute->GetPositionFromOffset(offsetLateral, routeSegment, centrePosition);
			pRoute->GetDirection(routeSegment, direction);
			if(!paddingVehicle)
			{
				m_currentBalancingIndex = m_currentBalancingIndex >= m_AIlist.Size()?1:(m_currentBalancingIndex+1);
			}
			ActivateVehicle(type, waveIndex, m_currentBalancingIndex, centrePosition, direction, 15.0f );
		}
	}


	void CModeDestruction::ActivateVehicle(DestructionAIType type, int waveIndex, int rubberBandingIndex, const Maths::HVec3& position, Maths::HVec3& direction, F32 velocity)
	{

		// We want to enable the vehicle, ie turn on the rendering, collisions, physics update, etc, etc
		// The logic here works correctly,  the game manages to change some of these states, but that is someonelses problem.
		
		RLGameMode_Destruction* destructionParams = &RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction;

		if (m_AIPoolActive.Size() < destructionParams->mMaxAIVehiclesActive )
		{
			// Find an ai of the required type in the pool
			int index = -1;
			for(int i=0;i<m_AIPool.Size();i++)
			{
				if(m_AIPool[i].m_Type == type)
				{
					index=i;
					break;
				}
			}

			// We have verified that we have enough of all vehicle types to accommodate all waves
			// so we should never fail to find a vehicle of the type requested.
			// Well now that we can spawn extra vehicles to accomodate filling up empty routes
			// we have to handle the case that we might not find one of the desired type.
			if(index==-1)
			{
				// In this pretty rare case, just pick the first in the list
				index = 0;
				type = m_AIPool[0].m_Type;
			}
			
			{


				
				DestructionAIData destructionAI = m_AIPool[index];
				IData *pIData = destructionAI.m_Data;
				
				
				int vehicleIdx = pIData->GetState().m_index;
				
				
				
				// Send the respawn message
				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleIdx);
				Game::Actors::C_VehicleActor* pVehActor	= pLogicActor->GetGraphicalActor(vehicleIdx);

				if( Components::C_VehicleComponent_HandOfGod::ResetVehicleOnTrackStatic(pRLC,pLogicActor,position,direction,velocity,g_DestructionSpawnGhostTime,false))
				{
					destructionAI.m_wave = waveIndex;
					destructionAI.m_wreckedCycles = -1;
					destructionAI.m_waveCompletable = true;
					
					m_AIPoolActive.PushBack(destructionAI);

					// Set the rubber banding index for this driver 
					pIData->GetRubberBanding()->SetDriverIndexOverride(rubberBandingIndex);
				
			

					// Send the controls enabled message
					Game::Actors::C_VehicleControlsEnableMsg enableControlsMsg(pVehActor->GetHandle(), vehicleIdx);
					pVehActor->SendMessageToActor(pVehActor->GetHandle(), &enableControlsMsg);

				

					//// Play a special FX
					RLPickupSpecialFX sfx;

					sfx.mLocalSpecialFX=RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction.mSpawnSpecialFX;
					sfx.mExternalSpecialFX=sfx.mLocalSpecialFX;
					sfx.mDurationLoop.mStart=sfx.mDurationLoop.mEnd=0;
					sfx.mDurationIntro.mStart=0.0f;sfx.mDurationIntro.mEnd=5;
					sfx.mDurationOutro.mStart=sfx.mDurationOutro.mEnd=0;
					sfx.mFrequency=1.0f;
					sfx.mSpecialFXLocation = E_PICKUP_SPECIALFX_LOCATION_ACTINGCAR;

					Game::C_InGameSpecialFXManager::Instance().SetUpVehicleSFX(sfx,pVehActor->GetMatrix(),vehicleIdx,-1,Game::E_InGameSpecialFX_Trigger_Repairing, -1.f, Maths::HVec3(1,1,1), Maths::HVec3());

				
				
					AddVehicleToWorld(vehicleIdx);

					// Reset the health
					pLogicActor->ResetHealth();

					m_AIPool.EraseByIndex(index);

					LOGINFO(g_DestructionModeLogger, "ActivateVehicle: SUCCESS - Type=%s RubberbandIndex= %d", s_typeString[type], rubberBandingIndex );

				}
				else
				{

					F32 x,y,z;
					x = position.X();
					y = position.Y();
					z = position.Z();
					LOGWARN(g_DestructionModeLogger, "ActivateVehicle: FAILED - Failed to find any geometry around the desired spawn location[ %.2f, %.2f, %.2f ]. Is the AI-Data out of sync?", x, y, z);
				}
			}
		}
	}

	int CModeDestruction::FindInActivePool(IData *pIDataToDelete)
	{
		int index = -1;

		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			if (pIDataToDelete == m_AIPoolActive[i].m_Data)
			{
				index = i;
				break;
			}
		}

		return index;
	}


	void CModeDestruction::DeActivateVehicle(IData *pIDataToDelete, bool wasActive )
	{
		// We want to deactive the vehicle, ie turn off the rendering, collisions, physics update, etc, etc
		// The logic here works correctly,  the game manages to change some of these states, but that is someonelses problem.
		// Message below tries to aliviate the problem.

		IData *pIData = pIDataToDelete;
		int index = FindInActivePool(pIData);
		BIZ_ASSERT(index >= 0 && index < m_AIPoolActive.Size());
		
		const DestructionAIData& destructionAI = m_AIPoolActive[index];
		m_AIPool.PushBack(destructionAI);
		m_AIPoolActive.EraseByIndex(index);

		int vehicleIndex = pIData->GetState().m_index;
		RemoveVehicleFromWorld(vehicleIndex);

		if( wasActive)
		{
			Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleIndex);
			Game::Actors::C_VehicleActor* pVehActor	= pLogicActor->GetGraphicalActor(vehicleIndex);

			//// Play a special FX
			RLPickupSpecialFX sfx;

			sfx.mLocalSpecialFX=RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction.mRemovedSpecialFX;
			sfx.mExternalSpecialFX=sfx.mLocalSpecialFX;
			sfx.mDurationLoop.mStart=sfx.mDurationLoop.mEnd=0;
			sfx.mDurationIntro.mStart=0.0f;sfx.mDurationIntro.mEnd=5;
			sfx.mDurationOutro.mStart=sfx.mDurationOutro.mEnd=0;
			sfx.mFrequency=1.0f;
			sfx.mSpecialFXLocation = E_PICKUP_SPECIALFX_LOCATION_ACTINGCAR;

			Game::C_InGameSpecialFXManager::Instance().SetUpVehicleSFX(sfx,pVehActor->GetMatrix(),vehicleIndex,-1,Game::E_InGameSpecialFX_Trigger_Repairing, -1.f, Maths::HVec3(1,1,1), Maths::HVec3());

		}
	}


	void CModeDestruction::AddVehicleToWorld(int vehicleIdx)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleIdx);
		Game::Actors::C_VehicleActor* pVehActor	= pLogicActor->GetGraphicalActor(vehicleIdx);
		
		pVehActor->EnableVehicle();
		pVehActor->DeferredUpdate();
		pVehActor->SetVisibleHealthDamageEnabled(false);
		pVehActor->GetVehicleModel()->GetDamageLayer()->ForceDamageContextToRun();
		Game::C_Game::GetInstance()->GetLevelState().mCars[ vehicleIdx ].mState = Game::E_VEHICLE_STATE_RACING;

	}

	void CModeDestruction::RemoveVehicleFromWorld(int vehicleIdx)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleIdx);
		Game::Actors::C_VehicleActor* pVehActor	= pLogicActor->GetGraphicalActor(vehicleIdx);

		
		pVehActor->DisableVehicle();
		pVehActor->DeferredUpdate();
		
	}

	void
	CModeDestruction::ReassignToWave(int oldWave, int newWave)
	{
		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			if( m_AIPoolActive[i].m_wave == oldWave )
			{
				m_AIPoolActive[i].m_wave = newWave;
			}
		}
	}
	
	bool 
	CModeDestruction::CheckForAnyActive(int waveIndex)
	{
		// Check if all vehicles active in this wave have been wrecked.

		IData *pIDataPlayer = m_AIlist[0].m_Data;

		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			IData *pIData = m_AIPoolActive[i].m_Data;

			if (pIDataPlayer != pIData)
			{
				int wave = m_AIPoolActive[i].m_wave;
				if( wave == waveIndex )
				{
					return true;
				}
			}
		}
		return false;
	}

	void CModeDestruction::CheckForCompletedWaves()
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		IData *pIDataPlayer = m_AIlist[0].m_Data;

		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			IData *pIData = m_AIPoolActive[i].m_Data;

			if (pIDataPlayer != pIData)
			{
				Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(pIData->GetState().m_index);
				if( pLogicActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED )
				{
					bool waveCompletable = true;
					//Found a wrecked car. Now see if 
					//this if all the cars left in this wave are wrecked.
					int wave = m_AIPoolActive[i].m_wave;
					int vehiclesInWave = 0;
					for(int j=0; j<m_AIPoolActive.Size(); j++)
					{
						if( m_AIPoolActive[j].m_Data != pIDataPlayer && 
							m_AIPoolActive[j].m_wave == wave )
						{
							Game::Actors::C_VehicleLogicActor *pOtherLogicActor = pLayer->GetLogicVehicleActor(m_AIPoolActive[j].m_Data->GetState().m_index);
							if(pOtherLogicActor->GetCurrentHealthLevel()!=E_HEALTH_LEVEL_WRECKED)
							{
								// Not completed the wave yet then
								waveCompletable = false;
							}
						}
					}

					// Are all the cars wrecked. 
					if(waveCompletable)
					{
						// Is this wave completable. Ie Have all vehicles in the wave been wrecked
						// and not forcibly removed because they dropped too far behind
						if( m_AIPoolActive[i].m_waveCompletable )
						{
							// Yes, so inform the game.
							C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(E_DestructionModeMessage_WaveCompleted);
							LOGINFO(g_DestructionModeLogger, "Wave Wrecked: Wave=%d ", wave );

							// And mark as non-completable so we don't inform the main game
							// more than once.
							m_AIPoolActive[i].m_waveCompletable = false;

							// Mark any others in the same wave as non-completable now as well.
							for(int j=0; j<m_AIPoolActive.Size(); j++)
							{
								if( m_AIPoolActive[j].m_Data != pIDataPlayer && 
									m_AIPoolActive[j].m_wave == wave )
								{	
									m_AIPoolActive[j].m_waveCompletable = false;
									m_AIPoolActive[i].m_wave = -1;
								}
							}
						}
					}
				}
			}
		}
	}

	void CModeDestruction::RemoveVehiclesBehind()
	{
		// Darren : As the rest of the game uses this dirty technique of assuming 0 is the player, then lets join in sod multiplayer and splitscreen
		IData *pIDataPlayer = m_AIlist[0].m_Data;
		
		//Btl::Vector<int>		deactivateList;
		Btl::Vector<IData *>		deactivateList;
		
		RLGameMode_Destruction* destructionParams = &RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction;

		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			IData *pIData = m_AIPoolActive[i].m_Data;

			// work out the distance behind.......

			if (pIDataPlayer != pIData)
			{
				Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(pIData->GetState().m_index);

				float playerPos = pIDataPlayer->GetVehicleLogic()->GetLogicState()->GetLogicPosition();	
				float thisPos = pIData->GetVehicleLogic()->GetLogicState()->GetLogicPosition();	

				float dist = AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(playerPos, thisPos);
				
				bool waveCompletable = true;

				if(g_DestructionKillAllActive)
				{
					LOGINFO(g_DestructionModeLogger, "Removing Vehicle: FORCED REMOVE Wave=%d Type=%s", m_AIPoolActive[i].m_wave, s_typeString[m_AIPoolActive[i].m_Type]);
					waveCompletable = false;
					deactivateList.PushBack(pIData);
				}
				else if( pLogicActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED )
				{

					if(m_AIPoolActive[i].m_wreckedCycles < 0)
					{
						m_AIPoolActive[i].m_wreckedCycles = 6;
					}
					else
					{
						m_AIPoolActive[i].m_wreckedCycles -= 1; 
						if(m_AIPoolActive[i].m_wreckedCycles == 0)
						{
							LOGINFO(g_DestructionModeLogger, "Removing Vehicle: WRECKED Wave=%d Type=%s", m_AIPoolActive[i].m_wave, s_typeString[m_AIPoolActive[i].m_Type]);

							
							deactivateList.PushBack(pIData);
						}
					}
				}
				else if (dist < -m_DestructionModeSetupData->behindCutOffDistance)
				{
					LOGINFO(g_DestructionModeLogger, "Removing Vehicle: FALLEN BEHIND Wave=%d Type=%s", m_AIPoolActive[i].m_wave, s_typeString[m_AIPoolActive[i].m_Type]);
					waveCompletable = false;
					deactivateList.PushBack(pIData);
				}

				if( !waveCompletable  )
				{
					int waveNotCompletable = m_AIPoolActive[i].m_wave;
					if( waveNotCompletable != -1 )
					{
						for(int j=0; j<m_AIPoolActive.Size(); j++)
						{
							if( m_AIPoolActive[j].m_wave == waveNotCompletable )
							{
								m_AIPoolActive[j].m_waveCompletable = false;
								m_AIPoolActive[j].m_wave = -1;
							}
						}
					}
				}
			}
		}

		for(int i=0;i<deactivateList.Size(); i++)
		{
			DeActivateVehicle(deactivateList[i], true);
		}

		int wave = m_lastWave;
		while( wave != m_currentWave)
		{
			
			bool foundAnyInActiveWave  = CheckForAnyActive(wave);
			if(!foundAnyInActiveWave)
			{
				LOGINFO(g_DestructionModeLogger, "Wave Complete: Wave=%d",m_lastWave );
				if( wave != m_lastWave)
				{
					LOGINFO(g_DestructionModeLogger, "Reassigning Wave: Old Wave=%d New Wave=%d",m_lastWave, wave );
					ReassignToWave(m_lastWave,wave);
				}
				
				m_lastWave++;
				if(m_lastWave >= m_DestructionModeSetupData->waves.count)
				{
					m_lastWave = 0;
				}
			}

			wave++;
			if(wave >= m_DestructionModeSetupData->waves.count)
			{
				wave = 0;
			}
		}
	}

	void CModeDestruction::Check()
	{

	}



	int CModeDestruction::FindFurthestAhead(float &furthestDist)
	{
		IData *pIDataPlayer = m_AIlist[0].m_Data;
		float maxDist = -FLT_MAX;
		int furthestIndex = -1;
		furthestDist = maxDist;

		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			IData *pIData = m_AIPoolActive[i].m_Data;

			if (pIDataPlayer != pIData)
			{
				Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(pIData->GetState().m_index);
				if( pLogicActor->GetCurrentHealthLevel() != E_HEALTH_LEVEL_WRECKED )
				{
					float playerPos = pIDataPlayer->GetVehicleLogic()->GetLogicState()->GetLogicPosition();	
					float thisPos = pIData->GetVehicleLogic()->GetLogicState()->GetLogicPosition();	

					float dist = AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(playerPos, thisPos);
					
					if (dist > maxDist && dist > 1)
					{
						maxDist = dist;
						furthestIndex = i;
						furthestDist = dist;
					}
				}
			}
		}

		return furthestIndex;
	}

#ifndef CFG_GOLD
	void CModeDestruction::DebugRender()
	{
		if(g_DestructionDebugRender)
		{

			DbUtil::I_DebugRender* renderer = DbUtil::DefaultDebugRenderSys()->Deferred();
			
			const Framework::C_View& rFrameworkView = Framework::C_ApplicationBase::GetInstance()->GetFullscreenView();
			Maths::C_Rect viewExtents =  rFrameworkView.m_View.m_ViewSettings2D.m_ViewExtents;

			RLGameMode_Destruction* destructionParams = &RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction;

			F32 maxWidthOffset = 10.0f;
			F32 logicLineWidth = 30.0f;
			F32 logicLineX = 60.0f;
			F32 logicLineTopY = viewExtents.Max().Y()-100.0f;
			F32 logicLineBottomY = viewExtents.Min().Y()+60.0f;
			F32 logicLineHeight = logicLineTopY - logicLineBottomY;

			F32 trackLength = AIAmax::AmaxAIR::Instance().TrackLength();

			// Draw a line representing the AI logic line
			renderer->Draw2dBar( Maths::HVec2(logicLineX-(logicLineWidth/2.0f), logicLineBottomY), 
								 Maths::HVec2(logicLineWidth,logicLineHeight), 
								 Maths::RGBA(0xAA,0xAA,0xAA,0xAA));

			// Draw a blob for the player
			
			IData *pIData = m_AIlist[0].m_Data;
			F32 lapPercentage = pIData->GetVehicleLogic()->GetLogicState()->GetLogicPosition();

			Maths::HVec2 lapPositionCentrePoint( logicLineX, logicLineBottomY + ( logicLineHeight * lapPercentage ));
			renderer->Draw2dBar( Maths::HVec2(lapPositionCentrePoint.X() -2.0f, lapPositionCentrePoint.Y() - 2.0f), 
								 Maths::HVec2(4.0f, 4.0f), 
								 Maths::RGBA(0xFF,0xFF,0xFF,0xAA));
				
			

			// Draw lines representing the max spawn ahead the fall behind distance
			F32 spawnAhead = lapPercentage + ( destructionParams->mForwardsCutOffDistance / trackLength );
			F32 fallBehind = lapPercentage + ( -m_DestructionModeSetupData->behindCutOffDistance / trackLength );
			
			if(spawnAhead > 1.0f)
			{
				spawnAhead -= 1.0f;
			}
			if(fallBehind < 0.0f )
			{
				fallBehind += 1.0f;
			}

			renderer->Draw2dBar( Maths::HVec2(logicLineX-(logicLineWidth/2.0f), logicLineBottomY + (spawnAhead * logicLineHeight)), 
								 Maths::HVec2(logicLineWidth, 2.0f), 
								 Maths::RGBA(0xFF,0xFF,0xFF,0xAA));
			renderer->Draw2dBar( Maths::HVec2(logicLineX-(logicLineWidth/2.0f), logicLineBottomY + (fallBehind * logicLineHeight)), 
								 Maths::HVec2(logicLineWidth, 2.0f), 
								 Maths::RGBA(0xFF,0xFF,0xFF,0xAA));

			// Draw a blob to show the desired position of the next wave to be spawned.
			if( m_DestructionModeSetupData->waves.count > 0 )
			{
				F32 aheadDistance = m_DestructionModeSetupData->waves[m_currentWave].waveSeparationDistance;

				float furthestDist;
				int furthestIndex = FindFurthestAhead(furthestDist);

				IData *pIDataThis = 0;

				if (furthestIndex == -1 || furthestDist < 0.0f )
				{
					// player
					IData *pIDataPlayer = m_AIlist[0].m_Data;
					pIDataThis = pIDataPlayer;
					BIZ_ASSERT(pIDataPlayer);
					furthestDist = 0.0f;
				}
				else  
				{
					IData *pIData = m_AIPoolActive[furthestIndex].m_Data;
					BIZ_ASSERT(pIData);
					pIDataThis = pIData;
				}

				if(pIDataThis)
				{
					

					F32 lapPercentage = pIDataThis->GetVehicleLogic()->GetLogicState()->GetLogicPosition();
					F32 nextWave = lapPercentage + ( aheadDistance / trackLength );
					if( nextWave > 1.0f )
					{
						nextWave -= 1.0f;
					}
					renderer->Draw2dBar( Maths::HVec2(logicLineX-(logicLineWidth/2.0f), logicLineBottomY + (nextWave * logicLineHeight)), 
										 Maths::HVec2(logicLineWidth, 2.0f), 
										 Maths::RGBA(0x00,0x00,0xFF,0xFF));

				}
			}


			// Now draw a blob for each active ai-driver
			for( int i=0; i<m_AIPoolActive.Size(); i++)
			{
				IData *pIData = m_AIPoolActive[i].m_Data;

				F32 widthOffset = 0.0f;
				if( pIData->GetSplineTracker()->GetCurrentRoute())
				{
					widthOffset = pIData->GetSplineTracker()->GetCurrentRoute()->m_offsetCentre * 
								  pIData->GetSplineTracker()->GetCurrentRoute()->m_width;
				}
				

				F32 lapPercentage = pIData->GetVehicleLogic()->GetLogicState()->GetLogicPosition();

				Maths::RGBA colour;
				if( m_AIPoolActive[i].m_Type == DAIT_Easy)
				{
					colour = Maths::RGBA(0x00,0xFF,0x00,0xAA);
				}
				else if( m_AIPoolActive[i].m_Type == DAIT_Medium)
				{
					colour = Maths::RGBA(0xFF,0xFF,0x00,0xAA);
				}
				else if( m_AIPoolActive[i].m_Type == DAIT_Hard)
				{
					colour = Maths::RGBA(0xFF,0x00,0x00,0xAA);
				}
				else if( m_AIPoolActive[i].m_Type == DAIT_VHard)
				{
					colour = Maths::RGBA(0x00,0xFF,0xFF,0xAA);
				}

				F32 widthOffsetNormalised = widthOffset / maxWidthOffset;
				widthOffsetNormalised = Maths::Clamped<F32>( widthOffsetNormalised,-1.0f,1.0f); 
					
				Maths::HVec2 lapPositionCentrePoint( logicLineX + (widthOffsetNormalised * (logicLineWidth/2.0f)), logicLineBottomY + ( logicLineHeight * lapPercentage ));
				renderer->Draw2dBar( Maths::HVec2(lapPositionCentrePoint.X() -2.0f, lapPositionCentrePoint.Y() - 2.0f),
									 Maths::HVec2(4.0f, 4.0f ),  
									 colour );

			}
		}
	}
#endif 
}




#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
