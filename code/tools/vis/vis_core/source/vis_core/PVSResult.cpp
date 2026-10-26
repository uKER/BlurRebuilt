#include "stdafx.h"

#include "Vis_Core\PVSResult.h"

IMPLEMENT_PERSISTENCE(PVSResult, "PVSResult");

PVSResult::PVSResult()
{
	m_pData = NULL;
	m_iDataSize = 0;
}

PVSResult::PVSResult(int nPolyRefs)
{
	// create and initialize byte array to zero
	m_iDataSize = (nPolyRefs+7)/8;
	m_pData = new BYTE[m_iDataSize];
	memset(m_pData, 0, m_iDataSize);
}

PVSResult::~PVSResult()
{
	delete m_pData;
}

// serialize/deserialization code
void PVSResult::write( ost::Engine &ar )
{
	ar << m_iDataSize;
	ar.writeBinary(m_pData, m_iDataSize);
}

void PVSResult::read ( ost::Engine &ar )
{
	ar >> m_iDataSize;
	m_pData = new BYTE[m_iDataSize];
	ar.readBinary(m_pData, m_iDataSize);
}
