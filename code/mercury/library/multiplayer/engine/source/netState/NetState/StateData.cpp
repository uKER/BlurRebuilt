#include "stdafx.h"
#include "stateData.h"
#include "netState/compression/entityCompressor.h"

C_StateData::C_StateData(const int uSize, int uCurrentState) : m_uCurrentState(uCurrentState), m_uSize(uSize)
{
	m_pcData = new char[m_uSize];
	memset(m_pcData, 0, m_uSize);
}
	
C_StateData::C_StateData(const C_StateData& xCopy, int uCurrentState) : m_uCurrentState(uCurrentState), m_uSize(xCopy.Size())
{
	m_pcData = new char[m_uSize];
	memcpy(m_pcData, xCopy.m_pcData, m_uSize);
}

C_StateData::~C_StateData()
{
	if(m_pcData)
	{
		delete [] m_pcData;
		m_pcData = NULL;
	}
}

bool C_StateData::IsStateSame(const C_StateData& rhs) const
{
	if(m_uSize != rhs.Size())							return false;
	if(memcmp(m_pcData, rhs.m_pcData, m_uSize) != 0)	return false;
	return true;
}

bool C_StateData::operator==(const C_StateData& rhs) const
{
	if(m_uCurrentState != rhs.CurrentState())			return false;
	return IsStateSame(rhs);
}

void C_StateData::operator=(const C_StateData& rhs)
{
	memcpy(m_pcData, rhs.m_pcData, m_uSize);
	m_uCurrentState = rhs.CurrentState();
}
