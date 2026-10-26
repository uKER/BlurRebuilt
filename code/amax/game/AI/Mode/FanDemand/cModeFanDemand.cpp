#include "stdafx.h"

#include <Mode/FanDemand/cModeFanDemand.h>

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
#include <Game/GameActors/Components/GameComponents.h>
#include <Game\GameActors\Components\VehicleLogic\HandofGod\VLC_HandOfGod.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


Debug::Float g_FanDemandSeperationDist("Amax.GameMode.FanDemand.SeperationDist", 50, Debug::Range(1.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_FanDemandCutoffDist("Amax.GameMode.FanDemand.CutoffDist", 150, Debug::Range(1.0f, 1000.0f), Debug::Step(1.0f));
Debug::Int g_FanDemandMaxActive("Amax.GameMode.FanDemand.MaxActive", 14, Debug::Range(1, 20), Debug::Step(1));
Debug::Float g_FanDemandBehindDist("Amax.GameMode.FanDemand.BehindDist", 50, Debug::Range(1.0f, 1000.0f), Debug::Step(1.0f));



namespace airacingtech
{
	using namespace Game;

	CModeFanDemand::CModeFanDemand() :m_pIGameServerTime(0)
	{
		m_AIlist.Clear();
		m_startRace = false;	
		m_lastTime = 0.0f;

		// TODO : This needs setting up from sorcery, but for now
		m_PatternSequence.PushBack(4);
		m_PatternSequence.PushBack(4);
		m_PatternSequence.PushBack(4);
		m_PatternSequence.PushBack(4);
		m_PatternSequence.PushBack(4);
		m_PatternSequence.PushBack(4);
		m_PatternSequence.PushBack(4); //ignores the last sequence, due to wrap around
		m_currentPattern = 0;
		m_lastPattern = 0;


	}

	CModeFanDemand::~CModeFanDemand()
	{
		
	}

	void CModeFanDemand::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_MODE_FANDEMAND, CID_MODE_FANDEMAND);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_MODE_FANDEMAND, CModeFanDemand::CreateMe, CModeFanDemand::DestroyMe, CHash("ModeFanDemandSystemAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_FANDEMAND, MT_ADD_AI_DRIVER_TO_LIST);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_FANDEMAND, MT_SET_GAMESTATE_GAME);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_FANDEMAND, MT_START_AI_RACE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_FANDEMAND, MT_SET_GAME_MODE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_FANDEMAND, MT_PRE_UPDATE_GAME_MODE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_MODE_FANDEMAND, MT_POST_INIT_AI_SYSTEM);
	}

	IComponent *CModeFanDemand::CreateMe()
	{
		return new CModeFanDemand;
	}

	bool CModeFanDemand::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CModeFanDemand::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CModeFanDemand::Deinit(void)
	{
		m_AIlist.Clear();
	}

	EMessageResult CModeFanDemand::HandleMessage(const CMessage &msg)
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
					m_AIlist.PushBack(pIData);	
					m_AIPool.PushBack(pIData);
				}
				break;

			case MT_START_AI_RACE:
				{
					m_startRace = true;
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

	EComponentTypeId CModeFanDemand::GetComponentTypeId(void)
	{
		return CID_MODE_FANDEMAND;
	}

	void CModeFanDemand::CachedInterfaces()
	{
		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));
	}

	void CModeFanDemand::PreUpdate()
	{

	}


	void CModeFanDemand::Update()
	{
		if( m_AIlist[0]->GetVehicleLogic()->GetLogicState()->GetLap() <= 0 )
			return;


		if (m_pIGameServerTime && m_pIGameServerTime->GetTime() > m_lastTime)
		{
			if (m_lastPattern == m_currentPattern)
			{
				if (m_AIPoolActive.Size() < g_FanDemandMaxActive)
				{
					ActivateVehicles(m_PatternSequence[m_currentPattern]);
					m_currentPattern++;
					if (m_currentPattern >= m_PatternSequence.Size())
					{
						m_currentPattern = 0;
					}
				}
				
			}
			else
			{
				RemoveVehiclesBehind();
			}

			m_lastTime = m_pIGameServerTime->GetTime() + 2.0f;

			// TODO : 
			Check();
		}
		else if (m_pIGameServerTime)
		{
			// Temporary solution :
			// keep calling this to stop colliding with invisible vehicles ? Somehting in the game logic keeps turning collisions and other stuff back on
			DeActivatePool();
		}
		

		

	}

	void CModeFanDemand::PostUpdate()
	{

	}

	void CModeFanDemand::InitialiseMode()
	{	
		PopulatePool();	

		DeActivatePool();
	}

	
	void CModeFanDemand::PopulatePool()
	{
		// Get rid of the player car from the pool of cars....
		m_AIPool.EraseByIndex(0);
	}

	void CModeFanDemand::DeActivatePool()
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		for(int i=0; i<m_AIPool.Size(); i++)
		{
			// We want to deactive the vehicle, ie turn off the rendering, collisions, physics update, etc, etc
			// The logic here works correctly,  the game manages to change some of these states, but that is someonelses problem.
			// Message below tries to aliviate the problem.

			IData *pIData = m_AIPool[i];
			
			Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(pIData->GetState().m_index);
		
			// Don`t ask !!!
			// to get around collisions being turned back on in the game loop. Don`t ask me again the game is a mess.
			ActorMsg::C_SetVehicleLogicMsg logicMsg;
			logicMsg.m_Ghosting = false; 
			logicMsg.m_Damage = true;
			logicMsg.m_Drafting = false;
			logicMsg.m_HandOfGod = false;
			logicMsg.m_Pickups = false;
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_AIController, &logicMsg);
			
			pLogicActor->GetGraphicalActor(pIData->GetState().m_index)->DisableVehicle();
		
		}
	}


	void CModeFanDemand::ActivateVehicles(int numVehicles)
	{
		int seperationDist = static_cast<int>(g_FanDemandSeperationDist);
	
		float furthestDist;
		int furthestIndex = FindFurthestAhead(furthestDist);
		
		IData *pIDataThis = 0;
		
		if (furthestIndex == -1)
		{
			// player
			IData *pIDataPlayer = m_AIlist[0];
			pIDataThis = pIDataPlayer;
			BIZ_ASSERT(pIDataPlayer);
		}
		else if (furthestIndex > 0 && (furthestDist > 0.0f && furthestDist < g_FanDemandCutoffDist) ) // we don`t want to spanw too many
		{
			IData *pIData = m_AIPoolActive[furthestIndex];
			BIZ_ASSERT(pIData);
			pIDataThis = pIData;
		}
		
		
		if (pIDataThis)
		{
			switch(numVehicles)
			{
			case 1:
				{
					Maths::HF32 offsetLateral(0.5f);
					PlaceVehicles(pIDataThis, seperationDist, offsetLateral);
				}
				break;

			case 2:
				{
					Maths::HF32 offsetLateral(0.25f);
					PlaceVehicles(pIDataThis, seperationDist, offsetLateral);

					Maths::HF32 offsetLateral2(0.75f);
					PlaceVehicles(pIDataThis, seperationDist+6, offsetLateral2);
				}
				break;

			case 3:
				{
					Maths::HF32 offsetLateral(0.5f);
					Maths::HF32 offsetLateral1(0.25f);
					Maths::HF32 offsetLateral2(0.75f);
					PlaceVehicles(pIDataThis, seperationDist, offsetLateral);
					PlaceVehicles(pIDataThis, seperationDist + 6, offsetLateral1);
					PlaceVehicles(pIDataThis, seperationDist - 6, offsetLateral2);
				}
				break;

			case 4:
				{
					Maths::HF32 offsetLateral(0.5f);
					Maths::HF32 offsetLateral1(0.25f);
					Maths::HF32 offsetLateral2(0.75f);
					PlaceVehicles(pIDataThis, seperationDist - 12, offsetLateral);
					PlaceVehicles(pIDataThis, seperationDist - 6, offsetLateral1);
					PlaceVehicles(pIDataThis, seperationDist, offsetLateral2);
					PlaceVehicles(pIDataThis, seperationDist + 6, offsetLateral);
				}
				break;

			default:
				break;
			}
		}

		

	}

	void CModeFanDemand::PlaceVehicles(IData *pIData, int seperationDist, Maths::HF32 offsetLateral)
	{
		const airacingtech::RouteInfo *pRouteInfo = pIData->GetSplineTracker()->GetLogicRoute();
		BIZ_ASSERT(pRouteInfo);
		airacingtech::AITrackDataRoutes *pRoute = pRouteInfo->m_pRoute;
		BIZ_ASSERT(pRoute);

		int logicIndex = pRouteInfo->m_index;

		int newIndex = pRoute->GetIndexOffset(logicIndex, seperationDist);
		Maths::HVec3 centrePosition(0.0f,0.0f,0.0f); 
		pRoute->GetPositionFromOffset(offsetLateral, newIndex, centrePosition);

		Maths::HF32 minY, maxY;
		Maths::HVec3 direction(0.0f,0.0f,1.0f);

		const airacingtech::AITrackDataCache& dc = pRoute->m_dataCache[newIndex];
		const int noEntries = dc.m_data.Size();	

		for(int entries = 0; entries <noEntries; entries++ )
		{
			const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];

			if (cachedData.m_pRoute && cachedData.m_pRoute->IsChangeableRoute())
			{
				cachedData.m_pRoute->GetHeightBounds(minY, maxY, cachedData.m_index);
				cachedData.m_pRoute->GetPositionFromOffset(offsetLateral, cachedData.m_index, centrePosition);
				cachedData.m_pRoute->GetDirection(cachedData.m_index, direction);
				break;
			}
		}

		centrePosition.Y(maxY-1.0f);
		Maths::HVec3 targetPos(centrePosition.X(), centrePosition.Y() - 1000.0f, centrePosition.Z());
		Maths::HVec3 hitPosition(0.0f,0.0f,0.0f);

		// now trace a line down.............to get the track height..........

//		bool LOS = LineOfSight(pIData->GetState().m_index, centrePosition, targetPos, hitPosition);
//		if (!LOS)
//		{
			//we need to move the car to this position and re-initialise the ai at this point....

			Maths::C_Normal up(0.0f, 1.0f, 0.0f);
			direction = up ^ direction;
			Maths::C_Normal dir(direction.X(), direction.Y(), direction.Z());
			Maths::AxisX aDir(dir);
			Maths::HMat33  orientation;
			orientation = Maths::HMat33( Maths::AxisY( up ), aDir );


			Maths::HMat43 worldMatrixPoint( orientation, centrePosition );
			BIZ_ASSERT(centrePosition.Finite());

			Maths::HVec3 velocity = worldMatrixPoint.AZ() * 15.0f;

			ActivateVehicle(worldMatrixPoint, velocity);
//		}
	}

	bool CModeFanDemand::LineOfSight(int vehicleIndex, Maths::HVec3 position, Maths::HVec3 targetPosition, Maths::HVec3 &hitPosition)
	{

		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleIndex);
		

		bool canSee = false;

		Maths::HVec3 targetPos = targetPosition;
		Maths::HVec3 startPos = position;

		Collision::C_TransparentRay ray;
		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);

		S32 vehGroupId = pLogicActor->GetGraphicalActor(vehicleIndex)->GetPhysicalGroupID();

		Collision::C_CollisionFilter raySolidFilter;
		raySolidFilter.SetBodyMask(PolyFlags::kbWorld);//PolyFlags::kbPerk | PolyFlags::kbVehicle | PolyFlags::kbWorld;
		raySolidFilter.SetGroupId(vehGroupId);
		raySolidFilter.SetGroupFlags(0);

		Collision::C_CollisionFilter rayTransFilter;
		rayTransFilter.SetBodyMask(PolyFlags::kbWorld);//PolyFlags::kbDynamicObject;
		rayTransFilter.SetGroupId(vehGroupId);
		rayTransFilter.SetGroupFlags(0);

		float dist = Maths::Distance(startPos, targetPos);
		Maths::HVec3 direction = Maths::Normalise(targetPos - startPos);
		Maths::C_Normal normal(direction);

		ray.SetNumLayers(50);
		ray.InitialiseRay(startPos, normal, dist, &raySolidFilter, &rayTransFilter, 0.1f);
		ray.SetMaxCosAngle(-0.5f);
		pLogicActor->GetGraphicalActor(vehicleIndex)->GetPhysicsScene()->DoTransparentRayTrace(ray, stack);
		Collision::C_RayTracingResult* result = ray.GetResult();

		//DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dLine(position, targetPosition, Maths::RGBA(255,255,0));

		if(ray.m_SolidResult.Hit())
		{
			canSee = false;
			hitPosition = ray.m_SolidResult.HitPoint();
		}
		else
		{
			hitPosition = targetPosition;
			canSee = true;
		}


		return canSee;
	}


	void CModeFanDemand::ActivateVehicle(Maths::HMat43 worldMatrixPoint, Maths::HVec3 velocity)
	{

		// We want to enable the vehicle, ie turn on the rendering, collisions, physics update, etc, etc
		// The logic here works correctly,  the game manages to change some of these states, but that is someonelses problem.
		

		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		if (m_AIPool.Size() > 1)
		{
			IData *pIData = m_AIPool[0];

/*
			Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(pIData->GetState().m_index);
			pLogicActor->GetGraphicalActor(pIData->GetState().m_index)->EnableVehicle();

			Game::Actors::C_VehicleActor* pVehActor		= pLogicActor->GetGraphicalActor(pIData->GetState().m_index);
			Game::Actors::C_VehicleResetAtWorldPositionMsg resetMsg(pVehActor->GetHandle(), pIData->GetState().m_index, worldMatrixPoint, velocity);
			pVehActor->SendMessageToActor(pVehActor->GetHandle(), &resetMsg);
*/

			// Send the respawn message
			int vehicleIdx = pIData->GetState().m_index;

			Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
			Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleIdx);
			Game::Actors::C_VehicleActor* pVehActor	= pLogicActor->GetGraphicalActor(vehicleIdx);

			if( Components::C_VehicleComponent_HandOfGod::ResetVehicleOnTrackStatic(pRLC,pLogicActor,worldMatrixPoint.Translation(),worldMatrixPoint.AZ(),velocity.Length(),2.0f,false))
			{
				m_AIPoolActive.PushBack(pIData);

				Game::Actors::C_VehicleControlsEnableMsg enableControlsMsg(pVehActor->GetHandle(), vehicleIdx);
				pVehActor->SendMessageToActor(pVehActor->GetHandle(), &enableControlsMsg);

				AddVehicleToWorld(vehicleIdx);

				pLogicActor->ResetHealth();

				m_AIPool.EraseByIndex(0);
			}
		}
		
	}



	void CModeFanDemand::AddVehicleToWorld(int vehicleIdx)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleIdx);
		Game::Actors::C_VehicleActor* pVehActor	= pLogicActor->GetGraphicalActor(vehicleIdx);
		
		pVehActor->EnableVehicle();
		pVehActor->DeferredUpdate();
		pVehActor->SetVisibleHealthDamageEnabled(false);
		Game::C_Game::GetInstance()->GetLevelState().mCars[ vehicleIdx ].mState = Game::E_VEHICLE_STATE_RACING;
/*
		ActorMsg::C_SetVehicleLogicMsg logicMsg;
		logicMsg.m_Ghosting = true; 
		logicMsg.m_Damage = true;
		logicMsg.m_Drafting = true;
		logicMsg.m_HandOfGod = false;
		logicMsg.m_Pickups = false;

		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_AIController, &logicMsg);
*/
	}


	void CModeFanDemand::DeActivateVehicle(int i)
	{
		// We want to deactive the vehicle, ie turn off the rendering, collisions, physics update, etc, etc
		// The logic here works correctly,  the game manages to change some of these states, but that is someonelses problem.
		// Message below tries to aliviate the problem.

		IData *pIData = m_AIPoolActive[i];
		m_AIPoolActive.EraseByIndex(i);
		m_AIPool.PushBack(pIData);

		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(pIData->GetState().m_index);

		// Don`t ask !!!!
		// Oh well, to get around collisions being turned back on in the game loop. Don`t ask me again the game is a mess.
		ActorMsg::C_SetVehicleLogicMsg logicMsg;
		logicMsg.m_Ghosting = false; 
		logicMsg.m_Damage = true;
		logicMsg.m_Drafting = false;
		logicMsg.m_HandOfGod = false;
		logicMsg.m_Pickups = false;

		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_AIController, &logicMsg);
		pLogicActor->GetGraphicalActor(pIData->GetState().m_index)->DisableVehicle();
		
	}

	int CModeFanDemand::FindInActivePool(IData *pIDataToDelete)
	{
		int index = -1;

		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			if (pIDataToDelete == m_AIPoolActive[i])
			{
				index = i;
				break;
			}
		}

		return index;
	}


	void CModeFanDemand::DeActivateVehicle(IData *pIDataToDelete)
	{
		// We want to deactive the vehicle, ie turn off the rendering, collisions, physics update, etc, etc
		// The logic here works correctly,  the game manages to change some of these states, but that is someonelses problem.
		// Message below tries to aliviate the problem.

		IData *pIData = pIDataToDelete;//m_AIPoolActive[i];
		int index = FindInActivePool(pIData);
		BIZ_ASSERT(index >= 0 && index < m_AIPoolActive.Size());
		
		m_AIPoolActive.EraseByIndex(index);
		m_AIPool.PushBack(pIDataToDelete);

		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(pIData->GetState().m_index);

		// Don`t ask !!!!
		// Oh well, to get around collisions being turned back on in the game loop. Don`t ask me again the game is a mess.
		ActorMsg::C_SetVehicleLogicMsg logicMsg;
		logicMsg.m_Ghosting = false; 
		logicMsg.m_Damage = true;
		logicMsg.m_Drafting = false;
		logicMsg.m_HandOfGod = false;
		logicMsg.m_Pickups = false;

		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_AIController, &logicMsg);
		pLogicActor->GetGraphicalActor(pIData->GetState().m_index)->DisableVehicle();

	}


	void CModeFanDemand::RemoveVehiclesBehind()
	{
		// Darren : As the rest of the game uses this dirty technique of assuming 0 is the player, then lets join in sod multiplayer and splitscreen
		IData *pIDataPlayer = m_AIlist[0];
		
		//Btl::Vector<int>		deactivateList;
		Btl::Vector<IData *>		deactivateList;
		
		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			IData *pIData = m_AIPoolActive[i];

			// work out the distance behind.......

			if (pIDataPlayer != pIData)
			{
				float playerPos = pIDataPlayer->GetVehicleLogic()->GetLogicState()->GetLogicPosition();	
				float thisPos = pIData->GetVehicleLogic()->GetLogicState()->GetLogicPosition();	

				float dist = AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(playerPos, thisPos);
				
				if (dist < -g_FanDemandBehindDist)
				{
					//deactivateList.PushBack(i);
					deactivateList.PushBack(pIData);
					if (deactivateList.Size() == 1)
					{
						m_lastPattern++;
						m_lastPattern = Maths::Min(m_lastPattern, m_currentPattern);
					}
				}
			}
		}

		for(int i=0;i<deactivateList.Size(); i++)
		{
			DeActivateVehicle(deactivateList[i]);
		}



	}

	void CModeFanDemand::Check()
	{
		for(int i=1;i<m_AIlist.Size(); i++)
		{
			IData *pIData = m_AIlist[i];

			U32 iIndex1 = Btl::FindIndex(m_AIPool, pIData);
			U32 iIndex2 = Btl::FindIndex(m_AIPoolActive, pIData);

			if ((iIndex1 == m_AIPool.Size() && iIndex2 == m_AIPoolActive.Size()) ||
				(iIndex1 != m_AIPool.Size() && iIndex2 != m_AIPoolActive.Size())
				)
			{
				BIZ_ASSERT_FAIL("FanDemand Mode : (Check 1) Spawning Pools have gone tits up!!!!!");
			}
		}

		int count = static_cast<int>(m_AIPoolActive.Size() + m_AIPool.Size());
	
		if (count > m_AIlist.Size()-1)
		{
			BIZ_ASSERT_FAIL("FanDemand Mode : (Check 2) Spawning Pools have gone tits up!!!!!");
		}

	}



	int CModeFanDemand::FindFurthestAhead(float &furthestDist)
	{
		IData *pIDataPlayer = m_AIlist[0];
		float maxDist = -FLT_MAX;
		int furthestIndex = -1;
		furthestDist = maxDist;

		for(int i=0; i<m_AIPoolActive.Size(); i++)
		{
			IData *pIData = m_AIPoolActive[i];

			if (pIDataPlayer != pIData)
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

		return furthestIndex;
	}


}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
