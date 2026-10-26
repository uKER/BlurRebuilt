#include "stdafx.h"
#include "acksystem.h"
#include "netstate/StateEnums.h"

C_ClientAcknowledgement::C_ClientAcknowledgement() 
	: m_bActive(false)
	, m_uState(E_Mode_None)
	, m_uOutboundAck(-1)
	, m_uInboundAck(-1)
{;}

// acknowledgement system
C_AckSystem::C_AckSystem(int uNumClients) 
	: m_uNumClients(uNumClients)
{
	m_pxClient = new C_ClientAcknowledgement[m_uNumClients];
}

C_AckSystem::~C_AckSystem() 
{
	delete [] m_pxClient;
}

void C_AckSystem::OutboundAck(const int uAcknowledgedHostId, const int uMyHostId, const int uAcknowledgedState)
{
	/*
	if(uAcknowledgedHostId == uMyHostId)
	{
		assert(uAcknowledgedHostId != uMyHostId);
		return;
	}*/

	if(!m_pxClient[uAcknowledgedHostId].Active())
	{
		return; //TODO: - throw warning.
	}

	m_pxClient[uAcknowledgedHostId].OutboundAck(uAcknowledgedState);
}


int	C_AckSystem::OutboundAck(const int uAcknowledgedHostId) const
{
	if(!m_pxClient[uAcknowledgedHostId].Active())
		return 0;

	return m_pxClient[uAcknowledgedHostId].OutboundAck();
}

void C_AckSystem::InboundAck(const int uAcknowledgedHostId, const int uMyHostId, const int uAcknowledgedState, bool bAcknowledge)
{
	/*
	if(uAcknowledgedHostId == uMyHostId)
	{
		assert(uAcknowledgedHostId != uMyHostId);
		return;
	}*/

	if(!m_pxClient[uAcknowledgedHostId].Active())
	{
		return; //TODO: - throw warning.
	}

	m_pxClient[uAcknowledgedHostId].InboundAck(uAcknowledgedState, bAcknowledge);
}

int	C_AckSystem::InboundAck(const int uAcknowledgedHostId)
{
	if(!m_pxClient[uAcknowledgedHostId].Active())
		return 0;

	return m_pxClient[uAcknowledgedHostId].InboundAck();
}

bool C_AckSystem::AckRequired(int uClient)
{
	if(!IsClientActive(uClient))
		return false;

	return m_pxClient[uClient].AckRequired();
}

void C_AckSystem::AckSent(int uClient)
{
	m_pxClient[uClient].AckSent();
}

bool C_AckSystem::IsClientActive(int uClient)
{
	assert(uClient >= 0 && uClient < m_uNumClients);
	return 	m_pxClient[uClient].Active();
}

void C_AckSystem::SetClientActive(int uClient, bool bActive)
{
	m_pxClient[uClient].Active(bActive);
	if(!bActive)
	{
		m_pxClient[uClient].OutboundAck(-1);
		m_pxClient[uClient].InboundAck(-1, false);
	}
}

void C_AckSystem::SetClientState(int uClient, int uState)
{
	m_pxClient[uClient].State(uState);
}

int C_AckSystem::GetClientState(int uClient) const
{
	return m_pxClient[uClient].State();
}

bool C_AckSystem::AllClientsState(int uState)
{
	for(int i = 0; i < m_uNumClients; ++i)
	{
		if(m_pxClient[i].Active())
		{
			if(m_pxClient[i].State() != uState)
				return false;
		}
	}
	return true;
}

int C_AckSystem::ActiveClients() const
{
	int uNum = 0;
	for(int i = 0; i < m_uNumClients; ++i)
	{
		if(m_pxClient[i].Active())
			++uNum;
	}
	return uNum;
}

int C_AckSystem::OldestOutboundState() const
{
	int uOldestState = INT_MAX;
	
	if(ActiveClients() == 0)
		return uOldestState;
	
	for(int i = 0; i < m_uNumClients; ++i)
	{
		if(m_pxClient[i].Active() && m_pxClient[i].OutboundAck() < uOldestState)
		{
			uOldestState = m_pxClient[i].OutboundAck();
		}
	}
	return uOldestState;
}

bool C_AckSystem::DebugOutput(int i, char* str, int uBufferSize, const char* strHostType)
{
	bool bOutput = false;
	if(m_pxClient[i].Active())
	{
		sprintf_s(str, uBufferSize, "%s[%d]\tOutAck[%d]\tInAck[%d]State[%s]", strHostType, i, m_pxClient[i].OutboundAck(), m_pxClient[i].InboundAck(), StateString(m_pxClient[i].State()));
		bOutput = true;
	}
	return bOutput;
}

char* C_AckSystem::StateString(int uState)
{
	switch(uState)
	{
	case E_Mode_None:						return "none";
	case E_Mode_Creation:					return "creation";
	case E_Mode_Destruction:				return "destruction";
	case E_Mode_Serialisation:				return "serialisation";
	case E_Mode_Migration_Start:			return "migrationStart";
	case E_Mode_Migration_ServerKeepAlive:	return "migrationKeepAlive";
	case E_Mode_Migration_ClientReport:		return "migrationRequestKeepAlive";
	case E_Mode_Migration_Creation:			return "migrationCreation";
	case E_Mode_Migration_Finish:			return "migrationFinished";
	default:								return "unknown";
	}
}