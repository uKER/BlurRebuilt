#include "stdafx.h"
#include "bufferedStateData.h"
#include "netstate/stateEnums.h"

C_BufferedEntity::C_BufferedEntity(int uSize) 
	: m_xCompressData(uSize)
	, m_xState(uSize)
{
	;
}

void C_BufferedEntity::SetState(const C_StateData& xNewState, const C_StateData& xPrevState) 
{
	assert(xNewState.Size() == xPrevState.Size());
	int uNewStateSqn = xNewState.CurrentState();
	int uPrevStateSqn = xPrevState.CurrentState();

	if(uNewStateSqn == uPrevStateSqn)
	{
		m_xCompressData.FullState();
	}
	else if(uPrevStateSqn <= E_EntityState_Creating)
	{
		m_xCompressData.FullState();
	}
	else if(uNewStateSqn <= E_EntityState_Creating)
	{
		m_xCompressData.FullState();
	}
	else
	{
		m_xCompressData.Compress(xNewState, xPrevState);
	}

	m_xState = xNewState;
}

C_StateData& C_BufferedEntity::State() 
{ 
	return m_xState; 
}

C_Compression& C_BufferedEntity::Compression()
{
	return m_xCompressData;
}