#include "NightwingPch.h"
#include "netTestGameWorld.h"
#include "netApp/netApp.h"
#include "netEntities\components\netGameData.h"
#include "netgame/netTestGameFactory.h"
#include "netGame/netHandle/netEntityHandle.h"
#include "netGroup/netGroup_Game.h"
#include "netGroup/netGameMediator.h"
#include "netServices/netServices.h"
#include "netState/gameState.h"
#include "netState/stateManager.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif

namespace netGame
{
	C_GameWorld* C_GameWorld::s_instance = NULL;

	C_GameWorld* C_GameWorld::GetInstance()
	{
		return s_instance;
	}
	C_GameWorld* C_GameWorld::CreateInstance(int hostId)
	{
		assert(!s_instance);
		s_instance = new C_GameWorld(hostId);
		return s_instance;
	}

	void C_GameWorld::DestroyInstance()
	{
		assert(s_instance);

		if(s_instance)
			delete s_instance;

		s_instance = NULL;
	}

	const float g_fWorldSize(20);

	C_GameWorld::C_GameWorld(const int uHostId) 
		: m_xWorldBounds(g_fWorldSize,g_fWorldSize,g_fWorldSize)
		, m_uMyHostId(uHostId)
	{
		m_pxActors = new C_GameActor*[g_uMaxEntities];

		for(int i = 0; i < g_uMaxEntities; ++i)
			m_pxActors[i] = NULL;
	}

	C_GameWorld::~C_GameWorld() 
	{
		if(m_pxActors)
		{
			for(int i = 0; i < g_uMaxEntities; ++i)
			{
				if(m_pxActors[i])
				{
					DestroyActor(i, true);
					//delete m_pxActors[i];
					//m_pxActors[i] = NULL;
				}
			}

			delete [] m_pxActors;
			m_pxActors = NULL;
		}
	}

	bool C_GameWorld::NetworkSync(float dt)
	{
		netGame::C_GameWorld* gameWorld		= this;
		netApp*				  m_app			= netApp::instance();
		netGameMediatorRef    gameMediator	= (m_app->m_game.isNull())? NULL : m_app->m_game->mediator();
		netGameDataRef		  gameData		= (gameMediator.isNull())?  NULL : gameMediator->m_gameData;
		if(gameWorld == NULL)
			return true;

		//------------------------------------------
		// gameworld syncing with gameData
		//------------------------------------------
		if(m_app->m_game.notNull() && m_app->m_game->connected() && gameData.notNull() && gameData->m_gameGroupData.notNull())
		{
			// synchronise game state
			C_GameState* selfGameState = C_StateManager::GetInstance()->GetGameState(E_State_GameWorld);

			if(selfGameState && selfGameState->IsServer())
			{
				for(int uClient = 0; uClient < NET_MAX_CONNECTIONS; ++uClient)
				{
					C_TestGameFactory* pxNetFactory = C_TestGameFactory::GetInstance();
					if(!pxNetFactory) 
						continue;

					int numPlayersInWorld = pxNetFactory->NumOfTypeWithAuthority(uClient, E_NetEnt_Actor_Predator);
					int numPlayersConnected = gameData->getGameGroupData()->countConnectionPlayers(uClient);
					int diff = (numPlayersConnected - numPlayersInWorld);
				
					if(diff > 0)
					{
						pxNetFactory->DistributeEntity(uClient, selfGameState->MyHostId(), E_NetEnt_Actor_Predator, diff);
					}
					else if (diff < 0)
					{
						selfGameState->RequestDeleteClientsEntities(uClient);
						//pxNetFactory->DistributeEntity(uClient, numPlayersConnected);
					}
				}
			}
		}
		return true;
	}

	void C_GameWorld::Update(float t)
	{
		NetworkSync(t);

		if(!m_pxActors)
			return;

		for(int uActorId = 0; uActorId < g_uMaxEntities; ++uActorId)
		{
			C_TestGameFactory* pxNetFactory = C_TestGameFactory::GetInstance();
			if(!pxNetFactory) 
				continue;

			C_GameActor* pxAct = m_pxActors[uActorId];
			if(!pxAct) 
				continue;

			if(!pxNetFactory->Authority(m_uMyHostId, pxAct->LocalId()))
				continue;

			bool bFear = AvoidWorld(uActorId);

			if(OutsideWorld(pxAct->NextPos(t)))
			{
				if(bFear)
					pxAct->m_xVel.Normalise();
				else
					pxAct->NewVel();
			}
			else
			{
				pxAct->Update(t);
			}

			if(!bFear && rand() % 1000 == 0)
				pxAct->NewVel();
		}
	}

	void C_GameWorld::Draw(const Maths::HVec3& xTranslation)
	{
		if(!m_pxActors)
			return;

		// draw grid
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
		
		for (S32 x = -m_xWorldBounds.X()/2.0f + xTranslation.X(); x <= m_xWorldBounds.X()/2.0f + xTranslation.X(); x++)
			Display::BasicRender()->DrawColoured3DLine(	(F32)x, xTranslation.Y(), -(F32)m_xWorldBounds.Z()/2.0f+xTranslation.Z(), 
														(F32)x, xTranslation.Y(), (F32)m_xWorldBounds.Z()/2.0f+xTranslation.Z(), 
														Maths::RGBA(75,75,75,255));

		for (S32 z = -m_xWorldBounds.Z()/2.0f + xTranslation.Z(); z <= m_xWorldBounds.Z()/2.0f + xTranslation.Z(); z++)
			Display::BasicRender()->DrawColoured3DLine(-(F32)m_xWorldBounds.X()/2.0f+xTranslation.X(), xTranslation.Y(), (F32)z, 
														(F32)m_xWorldBounds.X()/2.0f+xTranslation.X(), xTranslation.Y(), (F32)z, 
														Maths::RGBA(75,75,75,255));
		
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);

		for(int uActorId = 0; uActorId < g_uMaxEntities; ++uActorId)
		{
			C_GameActor* pxAct = m_pxActors[uActorId];
			if(!pxAct)
				continue;

			if(!pxAct->m_pxHealthState)
				continue;
			
			Maths::HVec3 xActorPos = pxAct->m_xPos+xTranslation;
			Maths::HVec3 xHealthOffset(0.05,0,0);
			Maths::RGBA  xColour = pxAct->m_xColour;

			Display::BasicRender()->DrawColoured3DSphere(xActorPos, pxAct->m_fRadius, xColour);
			Display::BasicRender()->DrawColoured3DLine(xActorPos, xActorPos+pxAct->m_xVel, xColour);

			// draw health
			Display::BasicRender()->DrawColoured3DLine(
				xActorPos+xHealthOffset, 
				xActorPos+xHealthOffset+Maths::HVec3(0.0,(pxAct->m_fRadius+0.5)*(pxAct->m_pxHealthState->m_fHealth), 0),
				Maths::RGBA(0, 0, 255, 255));

			Maths::HVec3 xGroundPos(xActorPos.X(), 0, xActorPos.Z());
			Maths::RGBA xGroundColour((U8)(xColour.R()*0.5f), (U8)(xColour.G()*0.5f), (U8)(xColour.B()*0.5f), (U8)(255.0f));

			Display::BasicRender()->DrawColoured3DCircle(xGroundPos, Maths::HVec3(0,1,0), pxAct->m_fRadius*.75, xGroundColour);
			
			// draw authority clients
			C_TestGameFactory* pxNetFactory = C_TestGameFactory::GetInstance();
			if(!pxNetFactory) 
				continue;

			if(pxNetFactory->Authority(m_uMyHostId, pxAct->LocalId()))
			{
				Display::BasicRender()->DrawColoured3DLine(
					xActorPos, 
					xActorPos+Maths::HVec3(0,pxAct->m_fRadius+0.5,0),
					Maths::RGBA(0, 255, 0, 255));
			}
		}
	}

	void C_GameWorld::LoadLevel()
	{
		int uClientId = 0;
		const int uServerId = 0;
		const int uNumServerEntities = 10;
		for(int i = 0; i < uNumServerEntities; ++i)
		{
			// TODO: create pickups

			// create some test server run actors
			CreateActor(C_Header(g_sNetIdAutoAssign, E_NetEnt_Actor_Prey, uServerId, E_NetState_ServerBroadcast, g_sCalculateSize, g_sNoParent), true);
		}
	}

	int C_GameWorld::CreateActor(const C_Header& xHeader, bool bServer, int uParentLocalId)
	{
		C_TestGameFactory* pxFactory = C_TestGameFactory::GetInstance();
		if(!pxFactory)
			return false;

		int uActorId = -1;
		for(int uId = 0; uId < g_uMaxEntities; ++uId)
		{
			if(m_pxActors[uId] == NULL)
			{
				uActorId = uId;			
				break;
			}
		}
		if(uActorId == -1)
		{
			assert(!"no free actor slots!");
			return false;
		}
		assert(m_pxActors[uActorId] == NULL);
		
		C_GameActor* pxParentAct = NULL;
		if(uParentLocalId >= 0)
			pxParentAct = GetActor(uParentLocalId);

		bool bSuccess = false;
	
		switch(xHeader.m_uTypeId)
		{
		case E_NetEnt_Actor_Predator:
		case E_NetEnt_Actor_Prey:
			{
				m_pxActors[uActorId] = new C_GameActor(uActorId, xHeader.m_uTypeId);
				if(bServer)
					Spawn(uActorId);

				bSuccess = true;

				LOGINFO(netStateDebug::g_Log, "Created actor (%s) with actor Id %d", pxFactory->NetEntityString(xHeader.m_uTypeId), uActorId);
				break;
			}
		case E_NetEnt_ActorCmp_Health:
			{
				if(pxParentAct && !pxParentAct->m_pxHealthState)
				{
					uActorId = pxParentAct->LocalId();
					pxParentAct->m_pxHealthState = new netGame::C_HealthState(uActorId, E_NetEnt_ActorCmp_Health);
					bSuccess = true;

					LOGINFO(netStateDebug::g_Log, "Created component (%s) for actor Id %d ", pxFactory->NetEntityString(xHeader.m_uTypeId), pxParentAct->LocalId());
				}
				else
				{
					assert(!"problem creating health component!");
					break;
				}
				break;
			}
		default: assert(!"failed to create actor!");
		}

		if(bSuccess)
		{
			// net state
			int uParentType = pxParentAct ? pxParentAct->TypeId() : -1;
			int uParentNetId = pxParentAct ? pxFactory->GetNetHandle(pxParentAct->LocalId()) : -1;
			
			// predict net id we'll use
			int uPredictedNetId = xHeader.m_uEntityId >= 0 ? xHeader.m_uEntityId : pxFactory->PredictNetId();
			pxFactory->SetLocalHandle(uPredictedNetId, uActorId);
			
			// create netstate
			bSuccess &= pxFactory->FactoryActorCreated(xHeader.m_uTypeId, xHeader.m_uAuthorityId, xHeader.m_uEntityId, xHeader.m_uSize, uParentNetId, uParentType);
			assert(bSuccess);


			LOGINFO(netStateDebug::g_Log, "Mapped NetId %d (ParentNetId %d) to LocalHandle %d", uPredictedNetId, uParentNetId, uActorId); 

			// component creation
			if(bServer)
			{
				switch(xHeader.m_uTypeId)
				{
					case E_NetEnt_Actor_Predator:
					case E_NetEnt_Actor_Prey:
						{
							if(bSuccess)
							{
								int uServerId = 0;
								CreateActor(C_Header(g_sNetIdAutoAssign, E_NetEnt_ActorCmp_Health, uServerId, E_NetState_ServerBroadcast, g_sCalculateSize, xHeader.m_uEntityId), bServer, uActorId);
							}

							bSuccess &= true;
							break;
						}
				}
			}
		}

		assert(bSuccess);
		return bSuccess ? uActorId : -1;
	}

	bool C_GameWorld::DestroyActor(const int uId, bool bDestroyNetState)
	{
		C_TestGameFactory* pxFactory = C_TestGameFactory::GetInstance();

		if(uId < 0 || uId >= g_uMaxEntities)
		{
			assert(!"uId out of range!");
			return false;
		}

		if(!m_pxActors[uId])
		{
			assert(!"attempt to destroy NULL!");
			return false;
		}

		if(bDestroyNetState && pxFactory)
		{
			int uNetId = pxFactory->GetNetHandle(uId);
			pxFactory->DestroyNetEntity(uNetId, m_pxActors[uId]->TypeId());
			pxFactory->SetLocalHandle(uNetId, -1);
		}

		// destroy children
		bool bKnownActor = true;
		switch(m_pxActors[uId]->TypeId())
		{
		case E_NetEnt_Actor_Predator:
		case E_NetEnt_Actor_Prey:
			{
				I_TestAppActor* pxChildActor = m_pxActors[uId]->GetComponent(E_NetEnt_ActorCmp_Health);
				if(pxChildActor)
				{
					if(bDestroyNetState && pxFactory)
					{
						int uNetId = pxFactory->GetNetHandle(pxChildActor->LocalId());
						pxFactory->DestroyNetEntity(uNetId, pxChildActor->TypeId());
						pxFactory->SetLocalHandle(uNetId, -1);
					}

					delete m_pxActors[uId];
					m_pxActors[uId] = NULL;
				}
			}
			break;
		case E_NetEnt_ActorCmp_Health:
			break;
		default: 
			assert(!"failed to destroy actor!");
			bKnownActor = false;
			break;
		}

		if(bKnownActor)
		{
			delete m_pxActors[uId];
			m_pxActors[uId] = NULL;
		}

		return true;
	}

	void C_GameWorld::AddActor(C_GameActor* pxActor, const int uId)
	{
		assert(!m_pxActors[uId]);
		m_pxActors[uId] = pxActor;
	}

	C_GameActor* C_GameWorld::GetActor(const int uId)
	{
		if(uId < 0)
			return NULL;

		assert(m_pxActors[uId]);
		return m_pxActors[uId];
	}

	bool C_GameWorld::Spawn(const int uId)
	{
		C_GameActor* pxActor = GetActor(uId);
		if(!pxActor)
			return false;

		switch(pxActor->TypeId())
		{
		case E_NetEnt_Actor_Predator:
			{
				float fRadius = (rand()/(float)RAND_MAX)*0.5f+0.25f;
				float fMaxSpeed = (rand()/(float)RAND_MAX)*15.0f;
				Maths::RGBA xColour(255, rand()%128, rand()%128, 255);
				pxActor->Set(Maths::HVec3(0,fRadius,0), Maths::HVec3(0,0,0), fRadius, fMaxSpeed, xColour);
			}
			break;
		case E_NetEnt_Actor_Prey:
			{
				float fRadius = (rand()/(float)RAND_MAX)*0.25f+0.1f;
				float fMaxSpeed = (rand()/(float)RAND_MAX)*5.0f;
				Maths::RGBA xColour(0, rand()%255, rand()%128+128, 255);
				pxActor->Set(Maths::HVec3(0,fRadius,0), Maths::HVec3(0,0,0), fRadius, fMaxSpeed, xColour);
			}
			break;		
		}

		return true;
	}

	float C_GameWorld::WorldSize()
	{ 
		return g_fWorldSize; 
	}

	bool C_GameWorld::OutsideWorld(const Maths::HVec3& xPos)
	{
		if(xPos.X() >  m_xWorldBounds.X()/2.0f) return true;
		if(xPos.X() < -m_xWorldBounds.X()/2.0f) return true;
		if(xPos.Y() >  m_xWorldBounds.Y()/2.0f) return true;
		if(xPos.Y() < -m_xWorldBounds.Y()/2.0f) return true;
		if(xPos.Z() >  m_xWorldBounds.Z()/2.0f) return true;
		if(xPos.Z() < -m_xWorldBounds.Z()/2.0f) return true;

		return false;
	}

	bool C_GameWorld::AvoidWorld(int uActorId)
	{
		C_GameActor* pxAct = m_pxActors[uActorId];

		if(pxAct->TypeId() == E_NetEnt_Actor_Prey)
		{
			for(int j = 0; j < g_uMaxEntities; ++j)
			{
				C_GameActor* pxTargetAct = m_pxActors[j];
				if(!pxTargetAct || uActorId == j)
					continue;

				if(pxTargetAct->TypeId() == E_NetEnt_Actor_Predator)
				{
					Maths::HVec3 xVel = pxAct->m_xPos - pxTargetAct->m_xPos;
					
					float fDangerDistance = (pxAct->m_fRadius + pxTargetAct->m_fRadius)*(pxAct->m_fRadius + pxTargetAct->m_fRadius);
					
					if(pxTargetAct->m_xVel.LengthSquared() > 0.0001f)
					{
						if(xVel.LengthSquared() < fDangerDistance*20.0f)
						{
							xVel.Y(0);
							xVel.Normalise();
							xVel *= pxAct->m_fMaxSpeed;
							pxAct->m_xVel = (pxAct->m_xVel + xVel) / 2.0f;
							return true;
						}
					}
				}
			}
		}
		return false;
	}
};

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif