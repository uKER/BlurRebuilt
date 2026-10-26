#include "stdafx.h"
#include "gamestate.h"
#include "netDebug/netDebug.h"
#include "netEntity/entityState.h"
#include "netStream/netStream.h"
#include "netEnums/netEnums.h"
#include "netState/factory/ifactory.h"
#include "netState/netHandle/NetHandle.h"
#include "netstate/StateEnums.h"
#include "netState/NetHandle/netEntityHandle.h"
#include <system/debug/BuildStamp.h>

int	C_GameState::g_sCurrentWorkingState = 0;

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

// C_GameState::C_GameStateHeader
bool C_GameState::C_GameStateHeader::Serialise(class netStream* stream) const
{
	stream->writeType(m_uTheirStateSQN);
	stream->writeType(m_uMyStateAck);
	stream->writeType(m_eGameStateMode);
	return stream->ok();
}

bool C_GameState::C_GameStateHeader::Deserialise(class netStream* stream)
{
	stream->readType(m_uTheirStateSQN);
	stream->readType(m_uMyStateAck);
	stream->readType(m_eGameStateMode);
	return stream->ok();
}

// C_GameState::C_NetState
C_GameState::C_NetState::C_NetState() 
	: m_pxEntityState(NULL)
	, m_uActive(false) 
	, m_uMigrationStatus(NULL)
	, m_bCopyState(false)
{;}

C_EntityState* C_GameState::C_NetState::EntityState()
{
	return m_pxEntityState;
}

bool C_GameState::C_NetState::EntityState(C_EntityState* pxEntityState)
{
	if(m_pxEntityState)
	{
		assert(!"attempt to create a netstate without destroying previous!");
		return false;
	}
	m_pxEntityState = pxEntityState;
	m_uActive = true;
	return true;
}

void C_GameState::C_NetState::CreateMigrationStates(int uNumClients)
{
	assert(!m_uMigrationStatus);
	m_uMigrationStatus = new int[uNumClients];
}

void C_GameState::C_NetState::DestroyMigrationStates()
{
	if(m_uMigrationStatus)
	{
		delete [] m_uMigrationStatus;
		m_uMigrationStatus = NULL;
	}
}

void C_GameState::C_NetState::SetClientMigrationStatus(int uClientId, int uStatus, bool bServer)
{
	m_uMigrationStatus[uClientId] = uStatus;
}

void C_GameState::C_NetState::SetMigrationStatus(int uNumClients, int uStatus, bool bServer)
{
	for(int i = 0; i < uNumClients; ++i)
	{
		m_uMigrationStatus[i] = uStatus;
	}
}

int C_GameState::C_NetState::GetMigrationStatus(int uClientId)
{
	return m_uMigrationStatus[uClientId];
}

void C_GameState::C_NetState::DestroyNetState()
{
	delete m_pxEntityState;
	m_pxEntityState = NULL;
	m_bCopyState = false;
	m_uActive = false;
}

bool C_GameState::C_NetState::IsActive()	
{ 
	if(m_uActive) 
	{
		assert(m_pxEntityState);
		return true;
	}
	
	assert(!m_pxEntityState);
	return false;
}

bool C_GameState::C_NetState::StateSerialised(const C_Header& xHeader)
{
	if(m_pxEntityState->Header().NetStateType() == E_NetState_Deleting)
	{
		// don't deserialise deleting entities!
		
		// must ensure that deleted states don't serialise data (header only), otherwise we get packet misalignmet

		return false;
	}

	return true;
}

bool C_GameState::C_NetState::EntityValidityCheck(const C_Header& xHeader, short int uEntitySize)
{
	if(	!m_pxEntityState || 
		m_pxEntityState->Header().m_uTypeId != xHeader.m_uTypeId ||
		m_pxEntityState->Header().m_uEntityId != xHeader.m_uEntityId || 
		m_pxEntityState->Header().m_uAuthorityId != xHeader.m_uAuthorityId)
	{
		assert(!"Entity failed vality check!");
		return false;
	}

	if(	m_pxEntityState &&
		m_pxEntityState->GetStateData(false) &&
		m_pxEntityState->GetStateData(false)->Size() != uEntitySize)
	{
		assert(!"Entity failed size check!");
		return false;
	}

	return true;
}

bool C_GameState::C_NetState::SkipState(int uSize, netStream* stream)
{
	C_Compression xCompression(uSize);
	xCompression.deserialise(stream);
	stream->setReadByteCount(stream->getReadByteCount() + xCompression.DifferenceSize());
	return true;
}

void C_GameState::C_NetState::SetMessageHeaderIdentifiction(C_Header& xMessageHeader)
{
	xMessageHeader = m_pxEntityState->Header();
}

bool C_GameState::C_NetState::SetMessageHeaderDataPackFlags(C_Header& xMessageHeader, int uClientId, C_GameState& xGameState)
{
	C_Header& xEntityHeader	= m_pxEntityState->Header();

	bool bIsAuthoritative	= xGameState.IsAuthoritative(xEntityHeader.m_uEntityId);
	bool bIsServer			= xGameState.IsServer();
	bool bIsClientServer	= uClientId == xGameState.ServerId();
	int uOutboundAck		= xGameState.OutboundAck(uClientId);
	int uCurrentState		= m_pxEntityState ? m_pxEntityState->CurrentState() : E_EntityState_UnSet;
	int uClientsAck			= xGameState.OutboundAck(uClientId);

	bool bSendEntitiy = false;	
	switch(xEntityHeader.NetStateType())
	{
	case E_NetState_Deleting:
		if(bIsServer)
		{
			xMessageHeader.SetFlag(E_DataFlag_Header);
			xMessageHeader.SetFlag(E_DataFlag_DestructionState);
			bSendEntitiy = true;
		}
		break;
	case E_NetState_ServerBroadcast:
		if(bIsServer && bIsAuthoritative)
		{
			if(uCurrentState > E_EntityState_UnSet)
			{
				if(m_pxEntityState->PrepareClientsState(uClientsAck))
				{
					xMessageHeader.SetFlag(E_DataFlag_DeltaCompressed);
					bSendEntitiy = true;
				}
			}
		}
		break;
	case E_NetState_PeerToServer:
		if(bIsAuthoritative && bIsClientServer)
		{
			if(uCurrentState > E_EntityState_UnSet)
			{
				if(m_pxEntityState->PrepareClientsState(uClientsAck))
				{
					xMessageHeader.SetFlag(E_DataFlag_DeltaCompressed);
					bSendEntitiy = true;
				}
			}
		}
		break;
	case E_NetState_PeerBroadcast:
		if(bIsAuthoritative)
		{
			if(uCurrentState > E_EntityState_UnSet)
			{
				if(m_pxEntityState->PrepareClientsState(uClientsAck))
				{
					xMessageHeader.SetFlag(E_DataFlag_DeltaCompressed);
					bSendEntitiy = true;
				}
			}
		}
		break;
	default: assert(!"unknown netstate!");
	};

	// server creates state on all clients
	if(bIsServer)
	{
		bool bCreationState = m_pxEntityState->CreationRequired(uOutboundAck);
		if(bCreationState)
		{
			if(m_pxEntityState->PrepareFullState())
			{
				xMessageHeader.SetFlag(E_DataFlag_CreationState);
				xMessageHeader.SetFlag(E_DataFlag_FullState);
				bSendEntitiy = true;
			}
		}
	}

	return bSendEntitiy;
}

bool C_GameState::C_NetState::SetMessageHeaderMigrationFlags(C_Header& xMessageHeader, int uClientId, class C_GameState& xGameState)
{
	if(!xGameState.Migrating())
		return false;

	bool bIsServer			= xGameState.IsServer();
	bool bIsClientServer	= uClientId == xGameState.ServerId();
	bool bSendMigration = false;

	switch(xGameState.GetGameStateMode())
	{
	case E_Mode_Migration_ServerKeepAlive:
		{
			if(bIsServer)
			{
				xMessageHeader.SetFlag(E_DataFlag_Header);
				xMessageHeader.SetFlag(E_DataFlag_MigrationKeepAlive);

				bSendMigration = true;
			}
		}
		break;
	case E_Mode_Migration_ClientReport:
		{
			if(bIsClientServer)
			{
				xMessageHeader.SetFlag(E_DataFlag_Header);
				xMessageHeader.SetFlag(E_DataFlag_MigrationClientReport);

				bSendMigration = true;
			}
		}
		break;
	case E_Mode_Migration_Creation:
		{
			if(bIsServer)
			{
				int uMigrationStatus = GetMigrationStatus(uClientId);
				bool bCreateState = (uMigrationStatus==E_Entity_Migration_New) && (xMessageHeader.NetStateType()!=E_NetState_Deleting);
				if(bCreateState)
				{
					if(m_pxEntityState->PrepareFullState())
					{
						xMessageHeader.SetFlag(E_DataFlag_CreationState);
						xMessageHeader.SetFlag(E_DataFlag_FullState);
						bSendMigration = true;
					}
				}
			}
		}
		break;
	}

	return bSendMigration;
}

bool C_GameState::C_NetState::PackMessage(C_Header& xMessageHeader, int uOutboundAck, netStream* stream)
{
	return m_pxEntityState->EntityStateToPacket(xMessageHeader, uOutboundAck, stream);
}

bool C_GameState::C_NetState::HandleCreationMsg(C_Header& xMessageHeader, C_GameState& xGameState)
{
	bool bCreatingState = xMessageHeader.GetFlag(E_DataFlag_CreationState);
	if(!bCreatingState)
		return false;

	I_Factory* pxFactory = I_Factory::GetInstance()->GetInstance();
	if(!pxFactory)
	{
		assert(false);
		return false;
	}

	if(xGameState.IsServer())
	{
		LOGWARN(netStateLog::g_Log, "<C_GameState::HandleCreationMsg %d> Server receiving create message?! Dropping.", g_sCurrentWorkingState);
		return false;
	}

	if(m_pxEntityState)
	{
		LOGINFO(netStateLog::g_Log, "<C_GameState::HandleCreationMsg %d> Processing Creation message again %s[%d]", g_sCurrentWorkingState, pxFactory->NetEntityString(xMessageHeader.m_uTypeId), xMessageHeader.m_uEntityId);
		return false;
	}

	// create game game object adapter
	bool bSuccess = pxFactory->CreateGameObject(xMessageHeader);
	if(!bSuccess)
		return false;

	// create game object
	bSuccess &= pxFactory->CreatedGameObject(xMessageHeader.m_uTypeId, xMessageHeader.m_uAuthorityId, xMessageHeader.m_uEntityId, xMessageHeader.m_uSize);
	if(!bSuccess)
		return false;

	m_pxEntityState = xGameState.EntityState(xMessageHeader.m_uEntityId);
	LOGINFO(netStateLog::g_Log, "<C_GameState::HandleCreationMsg %d> Created %s[%d]", g_sCurrentWorkingState, pxFactory->NetEntityString(xMessageHeader.m_uTypeId), xMessageHeader.m_uEntityId);

	return true;
}

bool C_GameState::C_NetState::HandleDestructionMsg(C_Header& xMessageHeader, C_GameState& xGameState)
{
	bool bDestruction = xMessageHeader.GetFlag(E_DataFlag_DestructionState);
	if(!bDestruction)
		return false;

	if(m_pxEntityState)
	{
		I_Factory::GetInstance()->DestroyNetEntity2(xMessageHeader.m_uEntityId, xGameState.GameStateId(), true);
		DestroyNetState();
	}

	return true;
}

bool C_GameState::C_NetState::HandleMigrationMsg(C_Header& xMessageHeader, C_GameState& xGameState, int uClientId)
{
	bool bKeepAlive = xMessageHeader.GetFlag(E_DataFlag_MigrationKeepAlive);
	bool bClientReport = xMessageHeader.GetFlag(E_DataFlag_MigrationClientReport);
	if(!(bKeepAlive || bClientReport))
		return false;

	bool bServer = xGameState.IsServer();

	if(bKeepAlive)
	{
		assert(!bServer);
		
		// sanity check
		int uServerState = xGameState.GetClientState(xGameState.ServerId());
		if(uServerState != E_Mode_Migration_ServerKeepAlive)
		{
			LOGWARN(netStateLog::g_Log, "<C_GameState::HandleMigrationMsg %d> Client Receiving keep alive message when server isn't in E_Mode_Migration_ServerKeepAlive!?", g_sCurrentWorkingState);
			return false;
		}

		if(!bServer)
		{
			SetMigrationStatus(xGameState.NumClients(), E_Entity_Migration_Alive, bServer);
		}

		return true;
	}

	if(bClientReport)
	{
		assert(bServer);
		
		// sanity check
		int uClientState = xGameState.GetClientState(uClientId);
		if(uClientState != E_Mode_Migration_ClientReport)
		{
			LOGWARN(netStateLog::g_Log, "<C_GameState::HandleMigrationMsg %d> Client Receiving keep alive message when server isn't in E_Mode_Migration_ServerKeepAlive!?", g_sCurrentWorkingState);
			return false;
		}

		if(bServer)
		{
			SetClientMigrationStatus(uClientId, E_Entity_Migration_Alive, bServer);
		}

		return true;
	}

	return false;
}

bool C_GameState::C_NetState::HandleUnPackMsg(C_Header& xMessageHeader, C_GameState& xGameState, class netStream* stream)
{
	bool bDeltaCompress = xMessageHeader.GetFlag(E_DataFlag_DeltaCompressed);
	bool bFullState = xMessageHeader.GetFlag(E_DataFlag_FullState);
	if(!(bDeltaCompress || bFullState))
		return true;

	I_Factory* pxFactory = I_Factory::GetInstance()->GetInstance();
	if(!pxFactory)
	{
		assert(false);
		return false;
	}

	if(!m_pxEntityState)
	{
		if(xGameState.MigrationActive(xGameState.ServerId()))
		{
			LOGWARN(netStateLog::g_Log, "<C_GameState[%d]::HandleUnPackMsg %d> No state to deserialise into during migration! Will skip and acknowledge state.", xGameState.MyHostId(), g_sCurrentWorkingState);
			SkipState(xMessageHeader.m_uSize, stream);
			return stream->ok();
		}
		else
		{
			LOGWARN(netStateLog::g_Log, "<C_GameState[%d]::HandleUnPackMsg %d> No state to deserialise into! Dropping.", xGameState.MyHostId(), g_sCurrentWorkingState);
			return false;
		}
	}

	bool bCreationState = xMessageHeader.GetFlag(E_DataFlag_CreationState);
	int uCurrentState = bCreationState ? E_EntityState_Creating : g_sCurrentWorkingState;

	// TODO (faster bFullState unpack!)
	if(m_pxEntityState->StreamToState(stream, uCurrentState))
	{
		m_bCopyState = true;
	}

	return stream->ok();
}

bool C_GameState::C_NetState::Update()
{
	if(m_bCopyState)
	{
		if(m_pxEntityState)
		{
			if(m_pxEntityState->StateToEntity())
			{
				m_bCopyState = false;
			}
		}
	}

	return true;
}

// C_GameState
C_GameState::C_GameState(const int uMyGameStateId, const int uMyHostId, const int uNumEntities, const int uNumClients) 
	: C_AckSystem(uNumClients)
	, m_xDataAnalyser(uNumEntities)
	, m_eGameStateMode(E_Mode_Serialisation)
	, m_uMyHostId(uMyHostId)
	, m_uNumEntities(uNumEntities)
	, m_uSequenceNumber(0) 
	, m_uServerId(-1)
	, m_uGameStateId(uMyGameStateId)
	, m_bDeletionInThisFrame(false)
	, m_bNetworkTickInThisFrame(false)
{
	m_pxNetState = new C_NetState[m_uNumEntities];

	for(int i = 0; i < m_uNumEntities; ++i)
	{
		m_pxNetState[i].CreateMigrationStates(uNumClients);
	}
}

C_GameState::~C_GameState()
{
	for(int i = 0; i < m_uNumEntities; ++i)
	{
		m_pxNetState[i].DestroyMigrationStates();

		C_EntityState* pxEntityState = EntityState(i);
		if(!pxEntityState) 
			continue;

		if(I_Factory::GetInstance())
		{
			if(!I_Factory::GetInstance()->DestroyNetEntity2(pxEntityState->Header().m_uEntityId, m_uGameStateId, false))
			{
				assert(!"failed to destroy actor");
			}
			if(pxEntityState->Header().NetStateType() != E_NetState_Deleting)
			{
				assert(!"this entity wasn't properly flagged as deleting!?");
			}
		}

		m_pxNetState[i].DestroyNetState();
	}

	if(m_pxNetState)
	{
		delete [] m_pxNetState;
		m_pxNetState = NULL;
	}
}

int	C_GameState::EntityAuthority(const int uEntityId)
{
	C_EntityState* pxEntityState = EntityState(uEntityId); assert(pxEntityState);
	return pxEntityState->Header().m_uAuthorityId;
}

bool C_GameState::IsAuthoritative(int uEntityId) const
{
	C_EntityState* pxEntityState = EntityState(uEntityId);
	if(!pxEntityState)	return false;
	return m_uMyHostId == pxEntityState->Header().m_uAuthorityId;
}

bool C_GameState::ServerId(int uServerId)	
{ 
	if(m_uServerId == uServerId)
		return false;

	if(m_uServerId == -1)
	{
		m_uServerId = uServerId; 
		return false;
	}

	if(Migrating())
	{
		LOGWARN(netStateLog::g_Log, "attempt to change to migrating state when already in migration state!");
		return false;
	}

	if(uServerId != MyHostId())
	{
		int uServerState = C_AckSystem::GetClientState(uServerId);
		if(uServerState != E_Mode_Serialisation && uServerState != E_Mode_Migration_Start)
		{
			LOGWARN(netStateLog::g_Log, "Can't change to that server %d [%s] until I receive a more recent message from him!", uServerId, C_AckSystem::StateString(uServerState));
			return false;
		}
	}

	// trigger migration
	LOGWARN(netStateLog::g_Log, "Server changed from %d to %d", m_uServerId, uServerId);
	
	MigrateServer(uServerId, m_uServerId);

	return true;
}

// this function assumes that only a complete gamestate will be received at any time.
// .. therefor the highest numbered net-entity will be the last game-state received. 
int	C_GameState::CurrentGameState()
{
	int uCurrentState = 0;
	for(int uEntityId = 0; uEntityId < m_uNumEntities; ++uEntityId)
	{
		C_EntityState* pxEntity = EntityState(uEntityId);
		if(!pxEntity)
			continue;

		if(IsAuthoritative(uEntityId))
			continue;

		if(pxEntity->CurrentState() > uCurrentState)
			uCurrentState = pxEntity->CurrentState();
	}

	return uCurrentState;
}

bool C_GameState::EntityState(int uEntityId, C_EntityState* pxEntityState)
{
	return m_pxNetState[uEntityId].EntityState(pxEntityState);
}

C_EntityState*	C_GameState::EntityState(int uEntityId) const
{
	if(!m_pxNetState)
		return NULL;

	if(!m_pxNetState[uEntityId].IsActive())
		return NULL;

	return m_pxNetState[uEntityId].EntityState();
}

C_GameState::C_NetState* C_GameState::NetState(int uEntityId) const
{
	if(!m_pxNetState)
		return NULL;

	return &m_pxNetState[uEntityId];
}

int	C_GameState::GetFreeNetId(int uRequestedId)
{
	int uNetId = uRequestedId;
	if(uNetId == -1)
	{
		int i;

		for(i = 0; i < m_uNumEntities; ++i)
		{
			if(EntityState(i) == NULL)
			{
				uNetId = i;
				break;
			}
		}

		if(uNetId == -1)
		{
			assert(!"failed to create net id!?");
		}
	}

	if(EntityState(uNetId))
	{
		assert(!"attempt to create entity where one already exists!");
	}

	return uNetId;
}

bool C_GameState::CreateNetState(C_Header& xStateInfo, netSerialisable* pxNetEntity, int uSize)
{
	if(xStateInfo.m_uEntityId == -1)
	{
		assert(!"entity id must be selected before CreateNetState() is called");
		return false;
	}

	assert(xStateInfo.m_uEntityId >= 0 && xStateInfo.m_uEntityId < m_uNumEntities);
	assert(EntityState(xStateInfo.m_uEntityId) == NULL && "Need to handle removal of existing netstate!");
	
	bool bSuccess = false;
	switch(xStateInfo.NetStateType())
	{
	case E_NetState_PeerToServer:
	case E_NetState_ServerBroadcast:
	case E_NetState_PeerBroadcast:
		{
			// create entity state
			bool bAuthority = m_uMyHostId == xStateInfo.m_uAuthorityId;
			int uCreatedSQN = SequenceNumber()+1; // created on the next network SQN
			int uInitialSQN = IsServer() ? E_EntityState_Creating : E_EntityState_UnSet;
			C_EntityState* pxEntityState = new C_EntityState(xStateInfo, pxNetEntity, uCreatedSQN, uInitialSQN, uSize, bAuthority);
			bSuccess = EntityState(xStateInfo.m_uEntityId, pxEntityState);
			if(!bSuccess)
			{
				delete pxEntityState;
				assert(!"failed to create entityState!");
			}

			if(bSuccess && IsServer())
				pxEntityState->CreateInitialState();
			
		}
		break;
	default: assert(!"need to implement net state creation!"); break;
	}

	
	return bSuccess;
}

bool C_GameState::RequestDeleteClientsEntities(int uClientId)
{
	// should only be called by server
	assert(IsServer());

	for(int uEntityId = 0; uEntityId < m_uNumEntities; ++uEntityId)
	{
		C_EntityState* pxEntityState = EntityState(uEntityId);
		if(!pxEntityState)
			continue;

		const C_Header& xHeader = pxEntityState->Header();
		if(xHeader.m_uAuthorityId == uClientId)
		{
			I_Factory::GetInstance()->DestroyNetEntity2(uEntityId, m_uGameStateId, true);
		}
	}

	return false;
}

bool C_GameState::RequestDeleteNetState(int uEntityId)
{
	if(EntityState(uEntityId))
	{
		EntityState(uEntityId)->Delete(m_uSequenceNumber+1);
		m_bDeletionInThisFrame = true;
		return true;
	}
	
	return false;
}

bool C_GameState::RequestDeleteNetStates()
{
	for(int i = 0; i < NumEntities(); ++i)
	{
		RequestDeleteNetState(i);
	}

	return true;
}

bool C_GameState::StateToPacket(const int uClientId, class netStream* stream)
{
	I_Factory* pxFactory = I_Factory::GetInstance();
	if(!pxFactory)
		return false;

	if(m_uMyHostId == uClientId)
		return false;

	// write game-state header
	C_GameStateHeader xGameStateHeader(SequenceNumber(), InboundAck(uClientId), m_eGameStateMode);
	xGameStateHeader.Serialise(stream);

	// entity serialisation
	bool bSentData = false;
	for(int uEntityId = 0; uEntityId < NumEntities(); ++uEntityId)
	{
		C_NetState* pxNetState = &m_pxNetState[uEntityId];
		if(!pxNetState->IsActive())
			continue;

		C_Header xMsgHeader;
		pxNetState->SetMessageHeaderIdentifiction(xMsgHeader);
		bool bSendEntity = pxNetState->SetMessageHeaderDataPackFlags(xMsgHeader, uClientId, *this);
		bool bSendMigration = pxNetState->SetMessageHeaderMigrationFlags(xMsgHeader, uClientId, *this);

		if(bSendEntity || bSendMigration)
		{
			// entity header
			if(netDebug::dbg_enableEntityMarkers)
				stream->writeMarker("eth");

			bool bPacked = pxNetState->PackMessage(xMsgHeader, OutboundAck(uClientId), stream);
			if(bPacked)
			{
				C_EntityState* pxEntityState = pxNetState->EntityState();
				if(pxEntityState)
				{
					I_Factory* pxFactory = I_Factory::GetInstance()->GetInstance(); assert(pxFactory);
					m_xDataAnalyser.OutgoingState(pxEntityState, pxFactory->NetEntityString(pxEntityState->Header().m_uTypeId));
				}

				bSentData = true;
			}

			// entity footer
			if(netDebug::dbg_enableEntityMarkers)
				stream->writeMarker("etf");
		}
	}

	// migration serialisation
	bool bAckRequired = AckRequired(uClientId);
	bAckRequired |= MigrationActive(uClientId);
	if(!bSentData && !bAckRequired)
		return false;

	if(!stream->ok())
		return false;

	AckSent(uClientId);
	return true;
}

bool C_GameState::PacketToState(const int uClientId, class netStream* stream)
{
	// read game state header
	C_GameStateHeader xGameStateHeader;
	xGameStateHeader.Deserialise(stream);

	// update ack system's migration flag
	int uOldState = C_AckSystem::GetClientState(uClientId);
	C_AckSystem::SetClientState(uClientId, (int)xGameStateHeader.m_eGameStateMode);

	UpdateMigrationStages();

	g_sCurrentWorkingState = xGameStateHeader.m_uTheirStateSQN;
	
	// process packet
	C_Header xMessageHeader;
	int streamStart = stream->getReadByteCount();
	bool bAckState = false;
	while (stream->ok() && !stream->eof())
	{
		// debug header
		if(!DebugHeader(stream, streamStart))
			continue;

		// handle entity creation
		xMessageHeader.Deserialise(stream);

		// TODO: split entity header and entity message move migration + creation states to messages
		bool bHeader = xMessageHeader.GetFlag(E_DataFlag_Header);
		bool bDeltaCompress = xMessageHeader.GetFlag(E_DataFlag_DeltaCompressed);
		bool bFullState = xMessageHeader.GetFlag(E_DataFlag_FullState);

		C_NetState* pxNetState = NetState(xMessageHeader.m_uEntityId); assert(pxNetState);

		bool bCreation = pxNetState->HandleCreationMsg(xMessageHeader, *this);
		bool bDestruction = pxNetState->HandleDestructionMsg(xMessageHeader, *this);
		bool bMigration = pxNetState->HandleMigrationMsg(xMessageHeader, *this, uClientId);
		bool bReceivedData = pxNetState->HandleUnPackMsg(xMessageHeader, *this, stream);

		if(!bReceivedData)
		{
			LOGINFO(netStateLog::g_Log, "<C_GameState::PacketToState %d> Dropped state from client %d", g_sCurrentWorkingState, uClientId); 
			C_AckSystem::SetClientState(uClientId, uOldState);
			return false;
		}

		bAckState |= bReceivedData || bMigration || bCreation;

		// log data
		if(bCreation)
		{
			// TODO
		}

		if(bDestruction)
		{
			// TODO
		}

		if(bMigration)
		{
			// TODO
		}

		if(bReceivedData)
		{
			C_EntityState* pxEntityState = pxNetState->EntityState();
			if(pxEntityState)
			{
				I_Factory* pxFactory = I_Factory::GetInstance()->GetInstance(); assert(pxFactory);
				m_xDataAnalyser.IncomingState(pxEntityState, pxFactory->NetEntityString(pxEntityState->Header().m_uTypeId));
			}
		}	

		// debug footer
		DebugFooter(xMessageHeader, stream, streamStart);
	}

	// acknowlede state
	if(stream->ok())
	{
		OutboundAck(uClientId, m_uMyHostId, xGameStateHeader.m_uMyStateAck);
		InboundAck(uClientId, m_uMyHostId, xGameStateHeader.m_uTheirStateSQN, bAckState);
	}
	else
	{
		C_AckSystem::SetClientState(uClientId, uOldState);
	}

	return stream->ok();
}

void C_GameState::Update(bool bNetworkFrame)
{
	if(C_AckSystem::ActiveClients() == 0)
	{
		GameStateMode(E_Mode_Serialisation);
	}

	m_bNetworkTickInThisFrame = bNetworkFrame;

	// do authority entity to state updates every network tick
	if(bNetworkFrame)
	{
		m_xDataAnalyser.Update();

		// update acknowledgements, and clean out buffers
		UpdateAcknowledgements();

		bool bNewGameState = false;
		int uNextSQN = m_uSequenceNumber+1;

		// entity to state
		for(int uEntityId = 0; uEntityId < NumEntities(); ++uEntityId)
		{
			if(!IsAuthoritative(uEntityId))
				continue;

			C_EntityState* pxEntityState = EntityState(uEntityId);
			if(!pxEntityState)
				continue;

			if(pxEntityState->EntityToState(uNextSQN))
				bNewGameState = true;
		}

		if(m_bDeletionInThisFrame)
		{
			bNewGameState = true;
			m_bDeletionInThisFrame = false;
		}

		if(bNewGameState)
			m_uSequenceNumber = uNextSQN;
	}

	// state update
	int uOldestAcknowledgedState = OldestOutboundState();
	for(int uEntityId = 0; uEntityId < m_uNumEntities; ++uEntityId)
	{
		// poll netstate update
		C_GameState::C_NetState* pxNetState = NetState(uEntityId);
		if(pxNetState)
			pxNetState->Update();

		// handle state deletion
		C_EntityState* pxEntityState = pxNetState->EntityState();
		if(!pxEntityState)
			continue;

		if(pxEntityState->Header().NetStateType() != E_NetState_Deleting)
			continue;

		if(uOldestAcknowledgedState >= pxEntityState->CurrentState())
			pxNetState->DestroyNetState();
	}

	// set self acknowledgements, always up-to-date with self!
	C_AckSystem::OutboundAck(m_uMyHostId, m_uMyHostId, m_uSequenceNumber);
	C_AckSystem::InboundAck(m_uMyHostId, m_uMyHostId, m_uSequenceNumber, false);
}

// process acknowledgements for authoritative states
void C_GameState::UpdateAcknowledgements()
{
	int uOldestOutboundState = C_AckSystem::OldestOutboundState();

	// ack 
	for(int uEntityId = 0; uEntityId < m_uNumEntities; ++uEntityId)
	{
		if(!IsAuthoritative(uEntityId))
			continue;

		C_EntityState* pxEntityState = EntityState(uEntityId);
		if(!pxEntityState)
			continue;

		C_StateDataBuffer* pxStateDataBuffer = pxEntityState->GetStateDataBuffer();
		if(!pxStateDataBuffer)
			continue;

		pxStateDataBuffer->Acknowledge(uOldestOutboundState);
	}
}

void C_GameState::GameStateMode(E_GameState_Mode eMode)
{
	if(m_eGameStateMode == eMode)
		return;

	LOGWARN(netStateLog::g_Log, "%s %d (Server %d[%s]) changed from %s to %s", MyHostId()==ServerId() ? "Server":"Client", MyHostId(), ServerId(), StateString(C_AckSystem::GetClientState(ServerId())), StateString(m_eGameStateMode), StateString(eMode));
	m_eGameStateMode = eMode;

	if(IsServer())
	{
		switch(m_eGameStateMode)
		{
		case E_Mode_Migration_Start:
			{
				for(int uEntityId = 0; uEntityId < m_uNumEntities; ++uEntityId)
				{
					C_NetState* pxNetState = NetState(uEntityId);
					if(!pxNetState->IsActive())
						continue;

					pxNetState->SetMigrationStatus(NumClients(), E_Entity_Migration_New, IsServer());
				}
			}
			break;
		}
	}
	else
	{
		switch(m_eGameStateMode)
		{
		case E_Mode_Migration_Start:
			{
				for(int uEntityId = 0; uEntityId < m_uNumEntities; ++uEntityId)
				{
					C_NetState* pxNetState = NetState(uEntityId);
					if(!pxNetState->IsActive())
						continue;

					pxNetState->SetMigrationStatus(NumClients(), E_Entity_Migration_AssumedDead, IsServer());
				}
			}
			break;
		case E_Mode_Migration_ClientReport:
			{
				// delete entities that are still assumed dead
				for(int uEntityId = 0; uEntityId < m_uNumEntities; ++uEntityId)
				{
					C_NetState* pxNetState = NetState(uEntityId);
					if(!pxNetState->IsActive())
						continue;

					if(pxNetState->GetMigrationStatus(0) == E_Entity_Migration_AssumedDead)
					{
						RequestDeleteNetState(pxNetState->EntityState()->Header().m_uEntityId);
					}
				}
			}
			break;
		}
	}
}

void C_GameState::UpdateMigrationStages()
{
	if(!Migrating())
		return;

	if(IsServer())
	{
		switch(m_eGameStateMode)
		{
		case E_Mode_Migration_Start:
			{
				if(C_AckSystem::AllClientsState(E_Mode_Migration_Start))
				{
					GameStateMode(E_Mode_Migration_ServerKeepAlive);
				}
			}
			break;
		case E_Mode_Migration_ServerKeepAlive:
			{
				if(C_AckSystem::AllClientsState(E_Mode_Migration_ServerKeepAlive))
				{
					GameStateMode(E_Mode_Migration_ClientReport);
				}
			}
			break;
		case E_Mode_Migration_ClientReport:
			{
				if(C_AckSystem::AllClientsState(E_Mode_Migration_ClientReport))
				{
					GameStateMode(E_Mode_Migration_Creation);
				}
			}
			break;
		case E_Mode_Migration_Creation:
			{
				if(C_AckSystem::AllClientsState(E_Mode_Migration_Creation))
				{
					GameStateMode(E_Mode_Migration_Finish);
				}
			}
			break;
		case E_Mode_Migration_Finish:
			{
				if(C_AckSystem::AllClientsState(E_Mode_Migration_Finish))
				{
					GameStateMode(E_Mode_Serialisation);
				}
			}
			break;
		}
	}
	else
	{
		switch(m_eGameStateMode)
		{
		case E_Mode_Migration_Start:
			{
				if(C_AckSystem::GetClientState(ServerId()) == E_Mode_Migration_ServerKeepAlive)
				{
					GameStateMode(E_Mode_Migration_ServerKeepAlive);
				}
			}
			break;
		case E_Mode_Migration_ServerKeepAlive:
			{
				if(C_AckSystem::GetClientState(ServerId()) == E_Mode_Migration_ClientReport)
				{
					GameStateMode(E_Mode_Migration_ClientReport);
				}
			}
			break;
		case E_Mode_Migration_ClientReport:
			{
				if(C_AckSystem::GetClientState(ServerId()) == E_Mode_Migration_Creation)
				{
					GameStateMode(E_Mode_Migration_Creation);
				}
			}
			break;
		case E_Mode_Migration_Creation:
			{
				if(C_AckSystem::GetClientState(ServerId()) == E_Mode_Migration_Finish)
				{
					GameStateMode(E_Mode_Migration_Finish);
				}
			}
			break;
		case E_Mode_Migration_Finish:
			{
				if(C_AckSystem::GetClientState(ServerId()) == E_Mode_Serialisation)
				{
					GameStateMode(E_Mode_Serialisation);
				}
			}
			break;
		}
	}
}

void C_GameState::MigrateServer(int uNewServerId, int uOldServerId)
{
	// migrate server entities
	for(int uEntityId = 0; uEntityId < m_uNumEntities; ++uEntityId)
	{
		C_EntityState* pxEntityState = EntityState(uEntityId);
		if(pxEntityState)
		{
			if(pxEntityState->Header().NetStateType() == E_NetState_ServerBroadcast)
			{
				pxEntityState->MigrateToNewServer(uNewServerId, uOldServerId, MyHostId());
			}
		}
	}

	m_uServerId = uNewServerId;

	GameStateMode(E_Mode_Migration_Start);
}

bool C_GameState::MigrationActive(int uClientId) const
{
	if(Migrating())
		return true;

	for(int i = E_Mode_Migration_First; i < E_Mode_Migration_Last; ++i)
	{
		if(C_AckSystem::GetClientState(uClientId) == i)
			return true;
	}

	return false;
}

void C_GameState::RenderModeGameState() 
{
	netDebug::dbg_renderMode = E_DbgRnd_GameState; 
}

void C_GameState::Render(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h)
{
	if(!debugRenderPtr)
		return;

	if(netDebug::dbg_renderMode == E_DbgRnd_GameState)
	{
		I_Factory* pxFactory = I_Factory::GetInstance();
		if(pxFactory)
		{
			float uLine = y;
			h = 24;
			w = 24;

			// debug mode
			char debugMode[64];
			switch(netDebug::dbg_renderGameStateMode)
			{
			case E_DbgMode_State:			sprintf_s(debugMode, sizeof(debugMode), "Debug State");				break;
			case E_DbgMode_Bandwidth:		sprintf_s(debugMode, sizeof(debugMode), "Debug Bandwidth");			break;
			case E_DbgMode_BandwidthGUI:	sprintf_s(debugMode, sizeof(debugMode), "Debug Bandwidth GUI");		break;
			case E_DbgMode_Memory:			sprintf_s(debugMode, sizeof(debugMode), "Debug Memory");			break;
			case E_DbgMode_EntityData:		sprintf_s(debugMode, sizeof(debugMode), "Debug Entity");			break;
			}

			// game state header string
			char str[1024];
			sprintf_s(	str, sizeof(str), "%s : %s - Sqn[%d]\tHostId[%d]\tServerId[%d]",
						pxFactory->GameStateString(m_uGameStateId),	debugMode,	m_uSequenceNumber,	m_uMyHostId, m_uServerId);

			debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine+=h), str, IsServer() ? Maths::RGBA(125, 190, 255, 255) : Maths::RGBA(125, 125, 255, 255), h);

			int uStartPoint = netDebug::dbg_entityDebugStartPoint > m_uNumEntities ? m_uNumEntities-1 : netDebug::dbg_entityDebugStartPoint;

			// game state debug output
			switch(netDebug::dbg_renderGameStateMode)
			{
			case E_DbgMode_State:
				{
					// output acknowedgements
					for(int i = 0; i < C_AckSystem::NumClients(); ++i)
					{
						if(C_AckSystem::DebugOutput(i, str, sizeof(str), (i==m_uServerId)?"Ser":"Cli"))
						{
							Maths::RGBA xColour = (i == ServerId()) ? Maths::RGBA(0, 255, 0, 255) : Maths::RGBA(0, 190, 0, 255);
							debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine+=h), str, xColour, h);
						}
					}

					// output entitites
					for(int i = uStartPoint; i < m_uNumEntities; ++i)
					{			
						if(EntityState(i) && EntityState(i)->DebugState(str, sizeof(str)))
						{
							Maths::RGBA xColour = IsAuthoritative(i) ? Maths::RGBA(255, 255, 0, 255) : Maths::RGBA(200, 200, 0, 255);
							
							if(EntityState(i)->Header().NetStateType() == E_NetState_Deleting)
							{
								xColour = Maths::RGBA(128, 0, 0, 255);
							}
							
							debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine+=h), str, xColour, h);
						}
					}
				}
				break;
			case E_DbgMode_Bandwidth:
				{
					uLine+=h;
					// output data analyser header
					int numlines = m_xDataAnalyser.DebugOutputSummary(str, sizeof(str));
					debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine), str, Maths::RGBA(0, 255, 0, 255), h);
					uLine+=h*numlines;

					// output type summary
					numlines = m_xDataAnalyser.DebugOutputTypeSummary(str, sizeof(str));
					debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine), str, Maths::RGBA(0, 255, 0, 255), h);
					uLine+=h*numlines;

					// output data
					for(int i = uStartPoint; i < m_uNumEntities; ++i)
					{	
						if(m_xDataAnalyser.IsActive(i))
						{
							sprintf_s(str, sizeof(str), "Id[%d]", i);
							numlines = m_xDataAnalyser.DebugOutputEntity(i, str, sizeof(str));
							debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine), str, IsAuthoritative(i) ? Maths::RGBA(255, 255, 0, 255):Maths::RGBA(200, 200, 0, 255), h);
							uLine+=h*numlines;
						}
					}
				}
				break;
			case E_DbgMode_BandwidthGUI:
				{
					uLine+=h;
					// output data analyser header
					int numlines = m_xDataAnalyser.DebugOutputSummary(str, sizeof(str));
					debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine), str, Maths::RGBA(0, 255, 0, 255), h);
					uLine+=h*numlines;

					m_xDataAnalyser.DebugGui(debugRenderPtr);
				}
				break;
			case E_DbgMode_Memory:
				{
					debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine+=h), "Debug Memory", Maths::RGBA(0, 255, 0, 255), h);

					// memory summary 
					int uStateSize = 0;
					int uHeaderSize = 0;
					for(int i = 0; i < m_uNumEntities; ++i)
					{
						C_EntityState* pxEntityState = EntityState(i);
						if(!pxEntityState)
							continue;
						
						C_Header xHeader = pxEntityState->Header();
						uStateSize += xHeader.m_uSize;
						uHeaderSize += sizeof(xHeader);
					}
					
					// output header size
					sprintf_s(	str, sizeof(str), "Total Headers: %d", uHeaderSize);
					debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine+=h), str, Maths::RGBA(0, 255, 0, 255), h);

					// output state size
					sprintf_s(	str, sizeof(str), "Total State Data: %d", uStateSize);
					debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine+=h), str, Maths::RGBA(0, 255, 0, 255), h);
						
					// output entitites
					for(int i = uStartPoint; i < m_uNumEntities; ++i)
					{			
						if(EntityState(i) && EntityState(i)->DebugMemory(str, sizeof(str)))
						{
							debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine+=h), str, IsAuthoritative(i) ? Maths::RGBA(255, 255, 0, 255):Maths::RGBA(200, 200, 0, 255), h);
						}
					}
				}
				break;
			case E_DbgMode_EntityData:
				{
					debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine+=h), "Debug Entity", Maths::RGBA(0, 255, 0, 255), h);

					for(int i = uStartPoint; i < m_uNumEntities; ++i)
					{
						if(EntityState(i) && EntityState(i)->DebugEntity(str, sizeof(str), m_uGameStateId))
						{
							debugRenderPtr->Draw2dText(Maths::HVec2(x, uLine+=h), str, IsAuthoritative(i) ? Maths::RGBA(255, 255, 0, 255):Maths::RGBA(200, 200, 0, 255), h);
						}
					}
				}
				break;
			}
		}
	}
}

// misalignment debugging
bool C_GameState::DebugHeader(class netStream* stream, int streamStart)
{
	// entity header
	if(netDebug::dbg_enableEntityMarkers)
	{
		stream->readMarker("eth");

		// keep re-trying so I can debug the mis-alignment
		if((stream->m_status == netStream::STATUS_FAILED_READ_VERSION))
		{
			LOGFATAL(netDebug::g_Log, "<C_GameState::DebugHeader> ENTITY START MISALIGNMENT AT (%d)", streamStart);
			if(netDebug::dbg_debugPacketMisalignments)
			{
				stream->setReadByteCount(streamStart);
				stream->setStatus(netStream::STATUS_OK);
				return false;
			}
		}
	}
	return true;
}

// misalignment debugging
void C_GameState::DebugFooter(C_Header& xHeader, class netStream* stream, int streamStart)
{
	if(netDebug::dbg_enableEntityMarkers)
	{
		stream->readMarker("etf");

		// keep re-trying so I can debug the mis-alignment
		if(stream->m_status == netStream::STATUS_FAILED_READ_VERSION)
		{
			LOGFATAL(netDebug::g_Log, "<C_GameState::DebugFooter> ENTITY END MISALIGNMENT AT (%d). ent(%d) type(%d) auth(%d) state(%d).", streamStart,
								xHeader.m_uEntityId,
								xHeader.m_uTypeId,
								xHeader.m_uAuthorityId,
								xHeader.NetStateType());
	
			if(netDebug::dbg_debugPacketMisalignments)
			{
				stream->setReadByteCount(streamStart);
				stream->setStatus(netStream::STATUS_OK);
			}
		}
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif