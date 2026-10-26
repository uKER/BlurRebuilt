// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "QOSListener.h"
#include <bdCore/bdSocket/bdCommonAddrInfo.h>


QOSListener::QOSListener()
{
}

QOSListener::~QOSListener()
{
}

void QOSListener::onQoSProbeSuccess(const bdQoSProbeInfo &info)
{
	m_numProbeRepliesReceived ++;
	bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
	bdCommonAddrInfo::getInfo(info.m_addr, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);

	bdPrintf("\nQuery For Game Info Succeeded to: \n%s \n"  , commonAddrInfo );
	if(	BD_NULL != info.m_data)
	{
		bdUInt32 numPlayers = 0;
		bdMemcpy(&numPlayers , info.m_data , sizeof(numPlayers));
		bdPrintf("Num Players Reported :  %u \n"  , numPlayers);
	}
	else
	{
		bdPrintf("No Game Data received \n" );
	}

	if(true == info.m_disabled)
	{
		bdPrintf("Host is in the disabled state");
	}

	bdNChar8 addrString[BD_ADDR_STRING_SIZE];
	info.m_realAddr.toString(addrString , BD_ADDR_STRING_SIZE);

	bdPrintf("IP addr used to communicate with host:  %s \n"  , addrString);
	bdPrintf("Latency To Host:  %f \n\n"  , info.m_latency);
}

void QOSListener::onQoSProbeFail(bdCommonAddrRef addr)
{
	m_numProbeRepliesReceived ++;
	bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
	bdCommonAddrInfo::getInfo(addr, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
	bdPrintf("Query For Game Info Failed to %s\n"  , commonAddrInfo);
}

void QOSListener::init(bdUInt numProbes)
{
	m_numProbeRepliesExpected = numProbes;
	m_numProbeRepliesReceived = 0;
}

bdBool QOSListener::isFinished()
{
	bdBool result = false;
	if(m_numProbeRepliesExpected == m_numProbeRepliesReceived)
	{
		result = true;
	}
	return result;
}

