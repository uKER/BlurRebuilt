#include "stdafx.h"
#include "entitystate.h"
#include "netDebug/netDebug.h"
#include "netEnums/netEnums.h"
#include "netStream/netBuffer.h"
#include "netstate/factory/ifactory.h"
#include "netstate/StateEnums.h"
#include "netstate/gameState.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

const int g_uBufferSize = 64;

// netstate interface
C_EntityState::C_EntityState(const C_Header& xHeader, class netSerialisable* pxEntity, int uCreatedSQN, int uInitialSQN, int uSize, bool bBuffered) 
	: m_xHeader(xHeader)
	, m_pxNetEntity(pxEntity)
	, m_xCompression(uSize)
	, m_uCreatedStateSQN(uCreatedSQN)
{
	assert(uSize > 0);

	if(bBuffered)
	{
		m_pxStateData = new C_StateDataBuffer(g_uBufferSize, uSize);
	}
	else
	{
		m_pxStateData = new C_StateDataBuffer(1, uSize);
		//m_pxStateData = new C_StateData(uSize, uInitialSQN);
	}
}

C_EntityState::~C_EntityState()
{
	if(m_pxStateData) delete m_pxStateData;
	m_pxStateData = NULL;
}

bool C_EntityState::StateToEntity()
{
	if(!m_pxNetEntity)
		return false;
	
	C_StateData* pxEntityState = GetStateData();
	if(!pxEntityState)
	{
		assert(!"no entity state!");
		return false;
	}
	
	C_StateData& xRef(*pxEntityState);
	netBuffer buff(xRef.m_pcData, xRef.m_uSize, xRef.m_uSize);
	return m_pxNetEntity->deserialise(&buff);
}

bool C_EntityState::EntityToState(int uSqn)
{
	if(!m_pxNetEntity)
		return false;

	if(!m_pxStateData)
		return false;

	if(m_xHeader.NetStateType() == E_NetState_Deleting)
	{
		assert(false);
		return false;
	}

	// TODO: assert(IsAuthority)
	int uNumStates = m_pxStateData->NumStates();
	bool bFirstState = (uNumStates == 0);
	if(bFirstState)
	{
		C_StateData xCopy(*GetStateData(true), uSqn);
		netBuffer buff(xCopy.m_pcData, xCopy.m_uSize);
		return m_pxNetEntity->serialise(&buff);
	}
	else
	{
		C_StateData xCopy(*GetStateData(false), uSqn);
		netBuffer buff(xCopy.m_pcData, xCopy.m_uSize);
		if(!m_pxNetEntity->serialise(&buff)) return false;
		return m_pxStateData->NewState(xCopy);
	}
}

bool C_EntityState::EntityStateToPacket(C_Header& xHeader, int uAck, class netStream* stream)
{
	bool bHeader = xHeader.GetFlag(E_DataFlag_Header);
	bool bFull = xHeader.GetFlag(E_DataFlag_FullState);
	bool bCompress = xHeader.GetFlag(E_DataFlag_DeltaCompressed);

	if(bFull && bCompress)
	{
		LOGWARN(netStateDebug::g_Log, "attempt to send state as full and compressed? should be one or other!");
	}

	bool bWriteData = bHeader || bFull || bCompress;
	if(bWriteData)
	{
		// write header
		xHeader.Serialise(stream);
		
		// write state
		if(bCompress || bFull)
		{
			StateToStream(stream);
			return stream->ok();
		}
	}
	return false;
}

bool C_EntityState::PrepareClientsState(int uClientsState)
{
	assert(m_pxStateData);
	return m_pxStateData->PrepareClientsState(m_xCompression, uClientsState);
}

bool C_EntityState::PrepareFullState()
{
	m_xCompression.FullState();
	return true;
}

void C_EntityState::CreateInitialState()
{
	C_StateData& xRef(*GetStateData(true));
	netBuffer buff(xRef.m_pcData, xRef.m_uSize);
	if(m_pxNetEntity)
		m_pxNetEntity->serialise(&buff);
}

void C_EntityState::MigrateToNewServer(int uNewServerId, int uOldServerId, int uMyHostId)
{
	if(uNewServerId == uMyHostId)
	{
		m_pxStateData->ResizeBuffer(g_uBufferSize, Header().m_uSize);
		CreateInitialState();
	}
	else
	{
		if(Header().m_uAuthorityId == uOldServerId)
		{
			m_pxStateData->ResizeBuffer(1, Header().m_uSize);
			CreateInitialState();
		}
	}

	Header().m_uAuthorityId = uNewServerId;
}

bool C_EntityState::CreationRequired(int uClientsAck)
{
	if(m_xHeader.NetStateType() == E_NetState_Deleting)
		return false;

	if(uClientsAck < m_uCreatedStateSQN)
		return true;

	if(uClientsAck == E_EntityState_UnSet)
		return true;

	// WHY IS THIS HERE?
	//if(CurrentState() == E_EntityState_Creating)
		//return true;

	return false;
}

void C_EntityState::Delete(const int uDeletedStateId)
{
	//assert(m_xHeader.m_uNetStateType != E_NetState_Deleting);
	if(m_xHeader.NetStateType() != E_NetState_Deleting)
	{
		m_xHeader.NetStateType(E_NetState_Deleting);
		C_StateData* pxEntityState = GetStateData(true);
		pxEntityState->CurrentState(uDeletedStateId);
		m_pxNetEntity = NULL;
	}

	assert(m_pxNetEntity == NULL);
}

bool C_EntityState::StateToStream(class netStream* stream)
{
	if(Header().NetStateType() == E_NetState_Deleting)
	{
		LOGWARN(netStateDebug::g_Log, "skipping state data serialisation, because state is deleting.");
		return true;
	}

	C_StateData* pxLatestState = GetStateData(false);
	if(!pxLatestState)
	{
		assert(!"no entity state: attempt to serialise an entity before a state was added!");
		return false;
	}

	// write compression header
	m_xCompression.serialise(stream);

	// write compressed state data
	for(int i = 0; i < pxLatestState->Size(); ++i)
	{
		if(m_xCompression.IsDifferent(i))	
			stream->writeType(pxLatestState->Data(i));
	}

	return stream->ok();
}

bool C_EntityState::StreamToState(class netStream* stream, int uCurrentState)
{
	C_StateData* pxLatestState = GetStateData(true);
	if(!pxLatestState)
	{
		assert(!"no entity state!");
		return false;
	}

	// read compression header
	m_xCompression.deserialise(stream);

	// read new state data
	char cByte;
	C_StateData xNewState(pxLatestState->Size(), uCurrentState); // NOTE: this could be 1k+! may need to move keep a working buffer around for this
	for(int i = 0; i < xNewState.Size(); ++i)
	{
		if(m_xCompression.IsDifferent(i))
		{
			stream->readType(cByte);
			xNewState.Data(i, cByte);
		}
	}

	bool bCompressionOk = true;
	
	// don't un-compress states when they are deleting
	if(Header().NetStateType() != E_NetState_Deleting)
	{
		// uncompress into the latest state
		bCompressionOk = m_xCompression.UnCompress(*pxLatestState, xNewState);
	}

	return stream->ok() && bCompressionOk;
}

// debugging function
bool C_EntityState::DebugState(char* str, int uBufferSize)
{
	I_Factory* pxFactory = I_Factory::GetInstance();
	if(!pxFactory)
		return false;

	char strType[64];
	sprintf_s(strType, sizeof(strType), "%s", pxFactory->NetStateString(Header().NetStateType()));	

	int uState = m_pxStateData ? m_pxStateData->CurrentState() : -1;
	int uStateSize = m_pxStateData ? m_pxStateData->GetStateData(0) ? m_pxStateData->GetStateData(0)->Size() : 0 : 0;

	sprintf_s(	str, 
				uBufferSize, 
				"Id[%d]\tEnt[%s]\tNet[%s]\tAth[%d]\tState[%d]",
				Header().m_uEntityId,
				pxFactory->NetEntityString(Header().m_uTypeId),
				strType,
				Header().m_uAuthorityId,
				uState);

	return true;
}

// debugging function
bool C_EntityState::DebugMemory(char* str, int uBufferSize)
{
	I_Factory* pxFactory = I_Factory::GetInstance();
	if(!pxFactory)
		return false;

	int uBuffers = m_pxStateData ? m_pxStateData->MaxStates() : 0;
	int uStateSize = m_pxStateData ? m_pxStateData->GetStateData(0) ? m_pxStateData->GetStateData(0)->Size() : 0 : 0;
	int uTotalSize = uBuffers * uStateSize;

	sprintf_s(	str, 
				uBufferSize, 
				"Id[%d]\tEnt[%s]\tSize[%d]\tBuffers[%d]\tTotal[%d]",
				Header().m_uEntityId,
				pxFactory->NetEntityString(Header().m_uTypeId),
				uStateSize,
				uBuffers, 
				uTotalSize);

	return true;
}

// debugging function
bool C_EntityState::DebugEntity(char* str, int uBufferSize, int uGameStateId)
{
	I_Factory* pxFactory = I_Factory::GetInstance();
	if(!pxFactory)
		return false;

	return pxFactory->DebugGameActor(Header().m_uEntityId, uGameStateId, str, uBufferSize);
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
