#include "stdafx.h"
#include "TestSectionNetState.h"
#include "TestSource/StateApp/netTestStateFactory.h"
#include "netState/netEntity/entitystate.h"
#include "netState/Gamestate.h"
#include "netState/StateManager.h"
#include "netState/StateEnums.h"
#include "netStream/netArray.h"

int	g_uNumEntities(32);
int	g_uNumHosts(8);
int	g_uGameStateToCreateEntitiesIn = 0;

enum E_Debug_Mode
{
	E_Debug_DeltaCompression	= 0,
	E_Debug_StateManagerText	= 1,
	E_Debug_StateManager2d		= 2,
};

DEBUG_ENUM(E_Debug_Mode)
	DE_OPTION(E_Debug_DeltaCompression,	"DeltaCompression")
	DE_OPTION(E_Debug_StateManagerText,	"StateManagerText")
	DE_OPTION(E_Debug_StateManager2d,	"StateManager2d")
END_DEBUG_ENUM

Debug::Enum<E_Debug_Mode>	dbg_eRenderMode					("Multiplayer.StateTestApp.RenderMode", E_Debug_StateManager2d);
Debug::Bool					dbg_bTestCreate					("Multiplayer.StateTestApp.TestCreate", false);
Debug::Bool					dbg_bTestCreateDuringMigration	("Multiplayer.StateTestApp.TestCreateDuringMigration", false);
Debug::Bool					dbg_bTestMigrate				("Multiplayer.StateTestApp.TestMigrate", true);
Debug::Bool					dbg_bForceMigrate				("Multiplayer.StateTestApp.ForceMigrate", false);

//namespace Samples
//{

C_TestStateSection::C_TestStateSection(const Util::C_String& name) : Framework::C_TestSection(name)
{
	const int uServerId = rand() % g_uNumHosts;
	const int uNumServerEntities = g_uNumEntities-(g_uNumHosts-1);
	int uClientHost = 0;

	if(!I_Factory::GetInstance())		new C_TestStateFactory(g_uNumHosts);
	if(!C_StateManager::GetInstance())	new C_StateManager(g_uNumHosts);
	
	for(int i = 0; i < g_uNumHosts; ++i)
	{
		C_StateManager::GetInstance()->CreateGameState(i, i, g_uNumEntities, g_uNumHosts);
		C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(i);
		if(pxGameState)
			pxGameState->ServerId(uServerId);
	}

	for(int i = 0; i < g_uNumEntities; ++i)
	{
		C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uServerId);

		if(pxGameState->EntityState(i) != NULL)
			continue;

		int uDistributionType = E_NetEnt_BlankPeerToPeer;
		int uAuthorityId = rand() % g_uNumHosts;
		if(uAuthorityId == uServerId)
		{
			uDistributionType = E_NetEnt_BlankServer;
		}
		
		// TODO: use distribution target here?
		g_uGameStateToCreateEntitiesIn = uServerId;
		I_Factory::GetInstance()->CreatedGameObject(uDistributionType, uAuthorityId, g_sNetIdAutoAssign, g_sCalculateSize);
	}

	g_uGameStateToCreateEntitiesIn = -1;
}

C_TestStateSection::~C_TestStateSection()
{
	if(I_Factory::GetInstance())		delete I_Factory::GetInstance();
	if(C_StateManager::GetInstance())	delete C_StateManager::GetInstance();
}

void C_TestStateSection::OnUpdate()
{
	// randomly create/destroy states
	if(dbg_bTestCreate)
	{
		if(!Migrating() || dbg_bTestCreateDuringMigration)
		{
			for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
			{
				C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);

				bool bChangedState = false;
				for(int uEntityId = 0; uEntityId < g_uNumEntities; ++uEntityId)
				{
					if(pxGameState->IsServer())
					{
						if(rand() % 256 == 0)
						{
							if(pxGameState->EntityState(uEntityId) == NULL)
							{
								g_uGameStateToCreateEntitiesIn = pxGameState->ServerId();
								int uType = (rand() % 2 == 0) ? E_NetEnt_BlankPeerToPeer : E_NetEnt_BlankServer;
								I_Factory::GetInstance()->CreatedGameObject(uType, rand()%g_uNumHosts, g_sNetIdAutoAssign, g_sCalculateSize);
							}
							else
							{
								pxGameState->RequestDeleteNetState(uEntityId);
							}

							bChangedState |= true;
							
							// force restart
							uEntityId = -1;
							continue;
						}
					}
				}
			}
		}
	}

	// randomly set migration mode
	bool bRandomMigration = false;
	if(dbg_bTestMigrate)
	{
		if(!Migrating())
		{
			bool bRandomMigration = rand() % 250 == 0;
			if(bRandomMigration)
			{
				LOGWARN(netStateDebug::g_Log, "migration triggered");
				dbg_bForceMigrate = true;
			}
		}
	}

	if(dbg_bForceMigrate)
	{
		// initiate migration
		if(!Migrating())
		{
			int uOldServerId = ServerId();
			int uNewServerId = -1;
			int uLatestState = -1;

			for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
			{
				if(uOldServerId == uHostId)
					continue;

				C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);
				int uKnownServerState = pxGameState->InboundAck(uOldServerId);
				if(uKnownServerState > uLatestState)
				{
					uLatestState = uKnownServerState;
					uNewServerId = uHostId;
				}
			}
			
			C_GameState* pxOldServerState = C_StateManager::GetInstance()->GetGameState(uOldServerId);	
			C_GameState* pxNewServerState = C_StateManager::GetInstance()->GetGameState(uNewServerId);

			pxOldServerState->ServerId(uNewServerId);
			pxNewServerState->ServerId(uNewServerId);

			pxOldServerState->RequestDeleteNetStates();

			LOGWARN(netStateDebug::g_Log, "starting migration for host %d (new host %d)", uOldServerId, uNewServerId);
			LOGWARN(netStateDebug::g_Log, "starting migration for host %d (new host %d)", uNewServerId, uNewServerId);
		}

		// update migration
		if(Migrating())
		{
			LOGWARN(netStateDebug::g_Log, "migration in progress");

			for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
			{
				C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);
				if(pxGameState->Migrating())
					continue;

				if(DropState(5, 2))
				{
					LOGWARN(netStateDebug::g_Log, "skipping host %d", uHostId);
					continue;
				}

				int uServerId = ServerId();
				pxGameState->ServerId(uServerId);
				LOGWARN(netStateDebug::g_Log, "starting migration for host %d (new host %d)", uHostId, uServerId);
			}
		}
		
		// end migration
		if(AllMigrating())
		{
			LOGWARN(netStateDebug::g_Log, "ending migration");

			dbg_bForceMigrate = false;
		}
	}

	// update all hosts
	netArray<2048> xPacket;
	for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
	{
		C_GameState* pxAuthorityState = C_StateManager::GetInstance()->GetGameState(uHostId);
		pxAuthorityState->Update(true);

		// all host's update
		for(int uClientId = 0; uClientId < g_uNumHosts; ++uClientId)
		{
			if(DropState(4, 2))
				continue;

			// generate messages
			xPacket.clear();

			if(pxAuthorityState->StateToPacket(uClientId, &xPacket))
			{
				// client receives state
				C_GameState* pxClientState = C_StateManager::GetInstance()->GetGameState(uClientId);

				// distribution
				g_uGameStateToCreateEntitiesIn = uClientId;

				// deserialise
				if(pxClientState->PacketToState(uHostId, &xPacket))
				{
					// update acknowledgements
					pxAuthorityState->OutboundAck(uClientId, uHostId, pxAuthorityState->SequenceNumber());
					pxClientState->InboundAck(uHostId, uClientId, pxAuthorityState->SequenceNumber(), true);

					// update connectivity 
					pxAuthorityState->SetClientActive(uClientId, true);	
					pxClientState->SetClientActive(uHostId, true);
				}
				else
				{
					LOGWARN(netStateDebug::g_Log, "failed packet to state!");
				}
			}
		}
	}
}

void C_TestStateSection::OnRender2DView()
{
	PERF_EVENT("RenderSectionEmpty");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 0, 0, 255));

	switch(dbg_eRenderMode)
	{
	case E_Debug_DeltaCompression:	DebugDeltaCompression();	break;
	case E_Debug_StateManagerText:	DebugStateManagerText();	break;
	case E_Debug_StateManager2d:	DebugStateManager2d();		break;
	};
}

void C_TestStateSection::DebugDeltaCompression()
{
	const float fMaxBuffers = 8;
	const float fXSize = 12.5;
	const float fYSize = 1.0;
	const float fWindowOffsetX = 100;
	const float fWindowOffsetY = 100;

	// draw client's state
	for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
	{
		const float fClientOffsetY = (g_uDefaultEntitySize*fYSize)*(uHostId+1) + fWindowOffsetY;
		C_GameState* pxHostState = C_StateManager::GetInstance()->GetGameState(uHostId);

		// draw client's background (bar)
		Maths::RGBA colour = pxHostState->IsServer() ? Maths::RGBA(0, 0, 255, 128) : Maths::RGBA(0, 0, 255, 32);
		const float fEntityRowWidth = (pxHostState->NumEntities()*fMaxBuffers*fXSize);
		Display::BasicRender()->DrawColoured2DRect(fWindowOffsetX, fClientOffsetY, fEntityRowWidth, g_uDefaultEntitySize*fYSize, colour);
		Display::BasicRender()->DrawColoured2DLine(fWindowOffsetX, fClientOffsetY, fWindowOffsetX+fEntityRowWidth, fClientOffsetY, Maths::RGBA(255, 255, 255, 128));

		// draw entities (block)
		for(int uEntityId = 0; uEntityId < pxHostState->NumEntities(); ++uEntityId)
		{
			C_EntityState* pxHostEntity = pxHostState->EntityState(uEntityId);
			if(!pxHostEntity)
				continue;

			C_StateData* pxHostStateData = pxHostEntity->GetStateData();
			if(!pxHostStateData)
				continue;

			C_GameState* pxAuthorityGameState = C_StateManager::GetInstance()->GetGameState(pxHostEntity->Header().m_uAuthorityId); 
			if(!pxAuthorityGameState)
				continue;

			C_EntityState* pxAuthorityEntity = pxAuthorityGameState->EntityState(uEntityId);
			if(!pxAuthorityEntity)
				continue;

			C_StateDataBuffer* pxAuthorityStateDataBuffer = pxAuthorityEntity->GetStateDataBuffer();
			if(!pxAuthorityStateDataBuffer)
				continue;

			const float fEntityRowOffset = uEntityId * fMaxBuffers * fXSize + fWindowOffsetX;
			float fBufferNum = 0;

			for(int i = 0; i < pxAuthorityStateDataBuffer->NumStates(); ++i)
			{
				if(i >= fMaxBuffers)
					continue;

				C_StateData* pxAuthorityStateData = pxAuthorityStateDataBuffer->GetStateData(i);

				bool bAuthorityA = pxHostState->MyHostId() == pxHostEntity->Header().m_uAuthorityId;
				int uPeerState = pxHostEntity->CurrentState();
				int uAuthorityState = pxAuthorityStateData->CurrentState();
				if(!bAuthorityA && uPeerState > uAuthorityState)
					continue;
				
				const float fBufferRowOffset = fBufferNum * fXSize;
				for(int i = 0; i < pxHostStateData->Size(); ++i)
				{
					Maths::RGBA colour;

					if( bAuthorityA || uPeerState == uAuthorityState)
					{
						char fColour = pxAuthorityStateData->Data(i);
						colour = Maths::RGBA(fColour, fColour, fColour, 255);
					}
					else
					{
						colour = (pxHostStateData->Data(i)==pxAuthorityStateData->Data(i)) ? Maths::RGBA(0, 255, 0, 255) : Maths::RGBA(255, 0, 0, 255);
					}

					float barX = fEntityRowOffset + fBufferRowOffset;
					float barY = (i*fYSize)+fClientOffsetY;
					Display::BasicRender()->DrawColoured2DRect(barX, barY, fXSize*0.9f, fYSize*.9f, colour);
				}

				fBufferNum++;
			}
		}
	}
}

void C_TestStateSection::DebugStateManagerText()
{
	for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
	{
		C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);
		const int uHostsPerColumn = 3;
		int uColumn = 0;
		if(uHostId < uHostsPerColumn)			uColumn = 0;
		else if(uHostId < uHostsPerColumn*2)	uColumn = 1;
		else if(uHostId < uHostsPerColumn*3)	uColumn = 2;

		int uRow = uColumn ? uHostId % uHostsPerColumn : uHostId;

		pxGameState->RenderModeGameState();
		pxGameState->Render(DbUtil::DefaultDebugRenderSys()->Immediate(), uColumn*280.0f, uRow*10*g_uNumEntities, 10.0f, 10.0f);
	}
}

void C_TestStateSection::DebugStateManager2d()
{
	const float fWidth = 95;
	const float fHeight = 500;
	const int uBorder = 30;
	int uPosX = uBorder;
	int uPosY = uBorder;

	// draw client's state
	for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
	{
		C_GameState* pxHostState = C_StateManager::GetInstance()->GetGameState(uHostId);

		// draw client's background
		Maths::RGBA colour;

		if(pxHostState->Migrating())
		{
			float fIntensity = (pxHostState->GetGameStateMode() - E_Mode_Migration_First) / (float)(E_Mode_Migration_Last - E_Mode_Migration_First);
			colour = Maths::RGBA(255, 0, 0, S32(255.0f * fIntensity));
		}
		else 
		{
			colour = Maths::RGBA(0, 0, 255, 255);
		}

		if(pxHostState->IsServer())
		{
			float fW = 10, fW2 = fW * 2.0f;
			Display::BasicRender()->DrawColoured2DRect(uPosX-fW, uPosY-fW, fWidth+fW2, fHeight+fW2, Maths::RGBA(0, 255, 0, 255));
		}
		Display::BasicRender()->DrawColoured2DRect(uPosX, uPosY, fWidth, fHeight, colour);

		// draw entities setup
		int uNumEntities = pxHostState->NumEntities();
		float fEntityBorder = uBorder / (float)(uNumEntities+1.0f);
		float fEntityWidth = (fWidth-uBorder);
		float fEntityHeight = (fHeight-uBorder) / (float)(uNumEntities+1.0f);
		int uEntityPosX = uPosX + (uBorder/2.0f);
		int uEntityStartPosY = uPosY + (uBorder/2.0f);

		// draw entities
		for(int uEntityId = 0; uEntityId < uNumEntities; ++uEntityId)
		{
			C_EntityState* pxHostEntity = pxHostState->EntityState(uEntityId);
			if(!pxHostEntity)
				continue;

			bool bIsHostAuthority = pxHostState->IsAuthoritative(uEntityId);
			int uAuthority = pxHostState->EntityAuthority(uEntityId);
			if(bIsHostAuthority)
			{
				colour = (pxHostEntity->Header().NetStateType()==E_NetState_ServerBroadcast) ? Maths::RGBA(0, 255, 0, 255):Maths::RGBA(0, 128, 0, 255);
			}
			else
			{
				C_GameState* pxAuthorityState = C_StateManager::GetInstance()->GetGameState(uAuthority);	assert(pxAuthorityState);
				C_EntityState* pxAuthoritytEntity = pxAuthorityState->EntityState(uEntityId);
				if(!pxAuthoritytEntity)
					continue;

				int uAuthorityState = pxAuthoritytEntity->CurrentState();
				int uHostState = pxHostEntity->CurrentState();
				float fScaleStates = 128;
				float fDiff = uAuthorityState - uHostState;
				
				if(uAuthorityState == -1)	// in initial state?
				{
					colour = Maths::RGBA(0, 255, 255, 255);
				}
				else if(fDiff < 0)			// state is higher than the authority state
				{
					colour = Maths::RGBA(255, 0, 255, 255);
				}
				else						// normal state
				{
					if(fDiff > fScaleStates) fDiff = fScaleStates;
					const int uIntensity = 255 - (255 * (fDiff / fScaleStates));
					colour = Maths::RGBA(uIntensity, uIntensity, uIntensity, 255);
				}				
			}

			if(pxHostEntity->Header().NetStateType() == E_NetState_Deleting)
			{
				colour = Maths::RGBA(255, 0, 0, 255);					
			}

			// next entity column
			int uEntityPosY = uEntityId * (fEntityHeight + fEntityBorder) + uEntityStartPosY;
			Display::BasicRender()->DrawColoured2DRect(uEntityPosX, uEntityPosY, fEntityWidth, fEntityHeight, colour);
		}

		// next column
		uPosX += fWidth + uBorder;
	}
}

bool C_TestStateSection::DropState(int uShots, int uProbability)
{
	static bool bDropStates = true;
	if(bDropStates)
	{
		for(int i = 0; i < uShots; ++i)
		{
			if(rand() % uProbability == 0) 
				return true;
		}
	}

	return false;
}

bool C_TestStateSection::Migrating()
{
	for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
	{
		C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);
		if(pxGameState->Migrating())
		{
			return true;
		}
	}

	return false;
}

bool C_TestStateSection::AllMigrating()
{
	for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
	{
		C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);
		if(!pxGameState->Migrating())
		{
			return false;
		}
	}

	return true;
}

int C_TestStateSection::ServerId()
{
	int uServerId = -1;
	for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
	{
		C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);

		// if we're not migrating then everyone should have the right server id
		if(!Migrating())
		{
			if(uServerId >= 0)
			{
				assert(uServerId == pxGameState->ServerId() && "hosts have different server id's?");
			}

			uServerId = pxGameState->ServerId();
		}
		// if we're migrating, then only the currently migrating hosts will have the right server id
		else if(pxGameState->Migrating())
		{
			if(uServerId >= 0)
			{
				assert(uServerId == pxGameState->ServerId() && "hosts have different server id's?");
			}

			uServerId = pxGameState->ServerId();
		}
	}
	return uServerId;
}