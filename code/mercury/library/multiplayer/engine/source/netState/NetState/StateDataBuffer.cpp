#include "stdafx.h"
#include "StateDataBuffer.h"
#include "netState/netState/BufferedStateData.h"
#include "netState/compression/entityCompressor.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

C_StateDataBuffer::C_StateDataBuffer(const int uMaxStates, const int uEntityDataSize) 
	: m_uMaxStates(uMaxStates)
	, m_uIndex(-1)
	, m_uNumStates(0)
	, m_pxBuffer(NULL)
{
	NewBuffer(uMaxStates, uEntityDataSize);
}

C_StateDataBuffer::~C_StateDataBuffer() 
{
	DestroyBuffer();
}

void C_StateDataBuffer::ResizeBuffer(int uMaxStates, int uEntityDataSize)
{
	DestroyBuffer();
	NewBuffer(uMaxStates, uEntityDataSize);

	m_uIndex = -1;
	m_uNumStates = 0;
}

void C_StateDataBuffer::NewBuffer(int uMaxStates, int uEntityDataSize)
{
	m_uMaxStates = uMaxStates;
	m_pxBuffer = new C_BufferedEntity*[m_uMaxStates];
	for(int i = 0; i < m_uMaxStates; ++i)
	{
		m_pxBuffer[i] = new C_BufferedEntity(uEntityDataSize);
	}
}

void C_StateDataBuffer::DestroyBuffer()
{
	if(m_pxBuffer)
	{
		for(int i = 0; i < m_uMaxStates; ++i)
		{
			if(m_pxBuffer[i])
			{
				delete m_pxBuffer[i];
				m_pxBuffer[i] = NULL;
			}
		}

		delete [] m_pxBuffer;
		m_pxBuffer = NULL;
	}
}

int	C_StateDataBuffer::CurrentState() const	
{ 
	if(m_uNumStates && m_uIndex >= 0 && m_uIndex < m_uMaxStates)
		return m_pxBuffer[m_uIndex]->State().CurrentState();

	return E_EntityState_UnSet;
}

C_StateData* C_StateDataBuffer::FindState(const int uStateNum) const
{
	// start at the current state
	for(int i = 0; i < m_uMaxStates; ++i)
	{
		// work backwards and return the latest state that the client received
		int uIndex = ((m_uIndex-i) >= 0) ? m_uIndex-i : (m_uIndex-i)+m_uMaxStates;
		C_StateData& pxEntity = m_pxBuffer[uIndex]->State();
		if(pxEntity.CurrentState() < E_EntityState_Creating)
			continue;

		if(pxEntity.CurrentState() <= uStateNum)
			return &pxEntity;
	}
	return NULL;
}

bool C_StateDataBuffer::NewState(const C_StateData& xState)
{
	C_BufferedEntity& xPrev = *m_pxBuffer[m_uIndex];

	bool bSameState = xPrev.State().IsStateSame(xState);
	bool bForceNewState = CurrentState() <= E_EntityState_Creating;

	if(!bSameState || bForceNewState)
	{
		IncrementBuffer();
		C_BufferedEntity& xCurrent = *m_pxBuffer[m_uIndex];;
		xCurrent.SetState(xState, xPrev.State());
		return true;
	}

	return false;
}

void C_StateDataBuffer::Acknowledge(int uSqn)
{
	int uEncounteredOldStates = 0;
	int uCount = m_uNumStates;
	for(int i = 0; i < uCount; ++i)
	{
		// remove any states before this acknowledged state (ensure at least one state is in the buffer)
		int uIndex = ((m_uIndex-i) >= 0) ? m_uIndex-i : (m_uIndex-i)+m_uMaxStates;

		if(uSqn >= m_pxBuffer[uIndex]->State().CurrentState())
		{
			if(uEncounteredOldStates > 0)
				m_uNumStates--;

			uEncounteredOldStates++;
		}

		assert(m_uNumStates > 0);
	}
}

C_StateData* C_StateDataBuffer::Latest(bool bCreateIfNone)
{
	if(m_uNumStates == 0)
	{
		if(!bCreateIfNone)
			return NULL;
		
		IncrementBuffer();
		m_pxBuffer[m_uIndex]->State().CurrentState(E_EntityState_UnSet);
	}

	return &m_pxBuffer[m_uIndex]->State();
}

C_Compression&	C_StateDataBuffer::GetCompression(int i)
{
	// 0 is current state, 1 is current state-1
	int uIndex = ((m_uIndex-i) >= 0) ? m_uIndex-i : (m_uIndex-i)+m_uMaxStates;
	return m_pxBuffer[uIndex]->Compression();
}

C_StateData* C_StateDataBuffer::GetStateData(int i, const bool bCreateIfNone)
{ 
	if(NumStates() == 0)
		return Latest(bCreateIfNone);

	// 0 is current state, 1 is current state-1
	int uIndex = ((m_uIndex-i) >= 0) ? m_uIndex-i : (m_uIndex-i)+m_uMaxStates;
	return &m_pxBuffer[uIndex]->State();	
}

void C_StateDataBuffer::IncrementBuffer() 
{
	m_uIndex = (m_uIndex+1)<m_uMaxStates ? m_uIndex+1 : 0;
	m_uNumStates = (m_uNumStates+1)<m_uMaxStates ? m_uNumStates+1 : m_uMaxStates;
}

bool C_StateDataBuffer::PrepareClientsState(class C_Compression& xCompress, int uClientState)
{
	return xCompress.Compress(*this, uClientState);
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
