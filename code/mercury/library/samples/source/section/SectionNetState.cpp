#include "StdAfx.h"
#include "Samples/Section/SectionNetState.h"
#include "netState/netEntity/entitystate.h"
#include "netState/Gamestate.h"
#include "netState/StateManager.h"
#include "netGame/netGameEnums.h"
#include "netState/StateEnums.h"
#include "netStream/netPacket/netPacket.h"
#include <bdSocket/bdAddrHandle.h>
#include "gameTestStateApp/game/netGameFactoryStateTest.h"

unsigned int			g_uNumHosts(8);
unsigned int			g_uNumEntities(8);
int						g_uRenderMode = 0;
int						g_uGameStateToCreateEntitiesIn = 0;


//namespace Samples
//{

C_SectionNetState::C_SectionNetState(const Util::C_String& name) 
	: C_Section(name)
{
	netEnums::s_packetVersionNumber = 1;

	const int uServerId = rand() % g_uNumHosts;
	const int uNumServerEntities = g_uNumEntities-(g_uNumHosts-1);
	int uClientHost = 0;

	if(!I_Factory::GetInstance())		new C_StateTestFactory(g_uNumHosts);
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

		int uAuthorityId = (i<uNumServerEntities) ? uServerId:++uClientHost;
		C_Header xHead(g_sNetIdAutoAssign, E_NetEnt_Blank, uAuthorityId, E_NetState_PeerBroadcast, g_uDefaultEntitySize, g_sNoParent);
		
		g_uGameStateToCreateEntitiesIn = uServerId;
		I_Factory::GetInstance()->CreateNetEntity(xHead);
	}
	g_uGameStateToCreateEntitiesIn = -1;
}

C_SectionNetState::~C_SectionNetState()
{
	if(I_Factory::GetInstance())		delete I_Factory::GetInstance();
	if(C_StateManager::GetInstance())	delete C_StateManager::GetInstance();
}

bool C_SectionNetState::ChangeState(C_GameState& xGameState, C_EntityState* xState, int uByteChangeProbability)
{
	if(xState->Header().m_uNetStateType == E_NetState_Deleting)
		return false;

	I_StateData* pxLatestStateInterface = xState->GetStateDataInterface();		assert(pxLatestStateInterface);
	C_StateData* pxLatestState = pxLatestStateInterface->GetStateData(0, true);	assert(pxLatestState);

	C_StateData xNewState(*pxLatestState, xGameState.SequenceNumber());
	
	bool bStateChanged = false;
  	for(int i = 0; i < xNewState.Size(); ++i)
	{
		if(rand() % uByteChangeProbability == 0)
		{
			xNewState.Data(i, rand() % 255);
			bStateChanged = true;
		}
	}
	
	if(bStateChanged)
	{
		pxLatestStateInterface->NewState(xNewState);
	}
	
	return bStateChanged;
}

void C_SectionNetState::OnUpdate(const Samples::C_Input& input, const Samples::C_Timer& timer)	
{
	// change sources
	for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
	{
		C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);

		bool bChangedState = false;
		for(int uEntityId = 0; uEntityId < g_uNumEntities; ++uEntityId)
		{
			static int bTestCreate = false;
			if(bTestCreate)
			{
				// randomly create/destroy states
				if(pxGameState->IsServer() && rand() % 250 == 0)
				{
					if(pxGameState->EntityState(uEntityId) == NULL)
					{
						C_Header xHead(g_sNetIdAutoAssign, E_NetEnt_Blank, rand()%g_uNumHosts, E_NetState_PeerBroadcast, g_uDefaultEntitySize, g_sNoParent);
						
						g_uGameStateToCreateEntitiesIn = pxGameState->ServerId();
						I_Factory::GetInstance()->CreateNetEntity(xHead);
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

			if(!pxGameState->IsAuthoritative(uEntityId))
				continue;

			C_EntityState* pxEntityState = pxGameState->EntityState(uEntityId);
			if(!pxEntityState)
				continue;

			// randomly change states
			bChangedState |= ChangeState(*pxGameState, pxEntityState, 25);
		}

		if(bChangedState)
		{
			pxGameState->SequenceNumber(pxGameState->SequenceNumber()+1);
		}
	}

	// update all hosts
	for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
	{
		C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);
		pxGameState->Update(true);

		// all host's update
		for(int uClientId = 0; uClientId < g_uNumHosts; ++uClientId)
		{
			// does this client lose a state?
			static bool bDropStates = true;
			if(bDropStates)
			{
				bool bLost = false;
				for(int i = 0; i < 2; ++i) { if(rand() % 2 == 0) { bLost = true; } }
				if(bLost) continue;
			}

			// generate messages
			netPacket xPacket;
			xPacket.clear();
			
			bool empty;
			if(pxGameState->GameStateToPacket(uClientId, &xPacket, empty))
			{
				// client receives state
				C_GameState* pxClientState = C_StateManager::GetInstance()->GetGameState(uClientId);
				g_uGameStateToCreateEntitiesIn = uClientId;
				if(pxClientState->PacketToState(uHostId, &xPacket))
				{
					pxGameState->OutboundAck(uClientId, uHostId, pxGameState->SequenceNumber());
					pxClientState->InboundAck(uHostId, uClientId, pxGameState->SequenceNumber());

					// update connectivity 
					pxGameState->SetClientActive(uClientId, true);	
					pxClientState->SetClientActive(uHostId, true);
				}
				else
				{
					assert(!"failed packet to state");
				}
			}
		}
	}
}

void C_SectionNetState::OnRender(const Samples::C_Input& input, const Samples::C_Timer& timer)
{
	PERF_EVENT("RenderSectionEmpty");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 0, 0, 255));

	if(g_uRenderMode)
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
				C_EntityState* pxPeerEntity = pxHostState->EntityState(uEntityId);
				if(!pxPeerEntity)
					continue;

				C_StateData* pxPeerStateData = pxPeerEntity->GetStateData();
				if(!pxPeerStateData)
					continue;

				C_GameState* pxAuthorityGameState = C_StateManager::GetInstance()->GetGameState(pxPeerEntity->Header().m_uAuthorityId); 
				if(!pxAuthorityGameState)
					continue;

				C_EntityState* pxAuthorityEntity = pxAuthorityGameState->EntityState(uEntityId);
				if(!pxAuthorityEntity)
					continue;

				I_StateData* pxAuthorityStateInterface = pxAuthorityEntity->GetStateDataInterface();
				if(!pxAuthorityStateInterface)
					continue;

				const float fEntityRowOffset = uEntityId * fMaxBuffers * fXSize + fWindowOffsetX;
				float fBufferNum = 0;

				int uNumStatesA = pxPeerStateData->NumStates();
				int uNumStatesB = pxAuthorityStateInterface->NumStates();
				for(int i = 0; i < uNumStatesB; ++i)
				{
					if(i >= fMaxBuffers)
						continue;

					C_StateData* pxAuthorityStateData = pxAuthorityStateInterface->GetStateData(i);

					bool bAuthorityA = pxHostState->MyHostId() == pxPeerEntity->Header().m_uAuthorityId;
					int uPeerState = pxPeerEntity->CurrentState();
					int uAuthorityState = pxAuthorityStateData->CurrentState();
					if(!bAuthorityA && uPeerState > uAuthorityState)
						continue;
					
					const float fBufferRowOffset = fBufferNum * fXSize;
					for(int i = 0; i < pxPeerStateData->Size(); ++i)
					{
						Maths::RGBA colour;

						if( bAuthorityA || uPeerState == uAuthorityState)
						{
							char fColour = pxAuthorityStateData->Data(i);
							colour = Maths::RGBA(fColour, fColour, fColour, 255);
						}
						else
						{
							colour = (pxPeerStateData->Data(i)==pxAuthorityStateData->Data(i)) ? Maths::RGBA(0, 255, 0, 255) : Maths::RGBA(255, 0, 0, 255);
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
	else
	{
		for(int uHostId = 0; uHostId < g_uNumHosts; ++uHostId)
		{
			C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uHostId);
			int uColumn = uHostId < 4 ? 0 : 1;
			int uRow = uColumn ? uHostId - 4 : uHostId;

			pxGameState->RenderModeGameState();
			pxGameState->Render(0, uColumn*512, uRow*16*11.5, 11.5, 11.5);
		}
	}
}

void C_SectionNetState::OnInput(const Samples::C_Input& input, const Samples::C_Timer& timer)
{
	int buttons = 0;
	for(int i = 0; i < Samples::E_Button_Count; i ++)
	{
		if(input.IsButtonDn((Samples::E_Button) i))
		{
			g_uRenderMode = !g_uRenderMode;
		}
	}	
}

bool C_SectionNetState::OnCreate()			{	Display::CreateTexture2DFromFile("Textures/Test.dds", &m_Texture);	return true;	}
void C_SectionNetState::OnDestroy()			{	Display::Device()->Destroy(&m_Texture); }

//};