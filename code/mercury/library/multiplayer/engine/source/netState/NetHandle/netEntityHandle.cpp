#include "stdafx.h"
#include "netEntityHandle.h"
#include "netstate/StateEnums.h"
#include "netStream/netStreamSizeCalculator.h"
#include "netstate/factory/ifactory.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

// C_GameNetEntity
C_GameNetEntity::C_GameNetEntity(const C_NetHandle& xNetHandle, const int uType/*, const C_GameHandle& xGameHandle*/) 
	: m_xNetHandle(xNetHandle)
	, m_uType(uType)
{
	;
}

E_HandleType C_GameNetEntity::HandleType() const
{
	I_Factory* pxFactory = I_Factory::GetInstance();
	if(!pxFactory)
		return E_Handle_Invalid;

	if(m_uType == -1)
		return E_Handle_Invalid;

	return pxFactory->GetHandleType(m_uType);
}

bool C_GameNetEntity::serialise(class netStream* stream)
{
	I_Factory* pxFactory = I_Factory::GetInstance();
	if(!pxFactory)
		return false;
	
	return pxFactory->SerialiseNetworkEntity(stream, *this);
}

bool C_GameNetEntity::deserialise(class netStream* stream)
{
	I_Factory* pxFactory = I_Factory::GetInstance();
	if(!pxFactory)
		return false;

	return pxFactory->DeSerialiseNetworkEntity(stream, *this);
}

int C_GameNetEntity::CalculateSize()
{
	if(HandleType() == E_Handle_System)
		return g_uDefaultEntitySize;

	// TODO: write new buffer to avoid this!
	netStreamSizeCalculator sizeCalculator;
	serialise(&sizeCalculator);
	return sizeCalculator.getWriteByteCount();
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
