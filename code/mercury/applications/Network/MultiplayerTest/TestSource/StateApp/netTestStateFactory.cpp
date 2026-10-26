#include "stdafx.h"
#include "netTestStateFactory.h"
#include "netState/gameState.h"
#include "netState/stateManager.h"
#include "netstate/netHandle/netEntityHandle.h"
#include "netStream/netStream.h"

extern int g_uGameStateToCreateEntitiesIn;

bool C_TestStateFactory::CreateGameSideObject(C_Header& xStateInfo, int uGameStateId)
{
	// APP STUFF
	return true;
}
bool C_TestStateFactory::DestroyGameSideObject(class C_NetHandle&)
{
	// APP STUFF
	return true;
}

bool C_TestStateFactory::SerialiseNetworkEntity(class netStream* pxStream, class C_GameNetEntity& xNetEntity)
{
	// randomly change entity source

	const int uSize = 16;
	char buffer[uSize];
	for(int i = 0; i < uSize; ++i)
		buffer[i] = rand() % 255;

	pxStream->writeBytes(buffer, uSize);
	return pxStream->ok();
}

bool C_TestStateFactory::DeSerialiseNetworkEntity(class netStream* pxStream, class C_GameNetEntity& xNetEntity)	
{
	const int uSize = 16;
	char buffer[uSize];
	pxStream->readBytes(buffer, uSize);
	return pxStream->ok();
}

int	C_TestStateFactory::RelevantGameState(int uType)
{
	assert(g_uGameStateToCreateEntitiesIn >= 0);

	switch(uType)
	{
	case E_NetEnt_BlankServer:
	case E_NetEnt_BlankPeerToPeer:	return g_uGameStateToCreateEntitiesIn;
	default:						assert(false);
	}
	return -1;
}

int	C_TestStateFactory::RelevantNetStateType(int uType)
{
	switch(uType)
	{
	case E_NetEnt_BlankServer:		return E_NetState_ServerBroadcast;
	case E_NetEnt_BlankPeerToPeer:	return E_NetState_PeerBroadcast;
	default:				assert(false);
	}
	return -1;
}

E_HandleType C_TestStateFactory::GetHandleType(int uTypeId)
{ 
	return E_Handle_System; 
}

// util
const char* C_TestStateFactory::NetStateString(int eType)
{
	return "unknown"; 
}

const char* C_TestStateFactory::NetEntityString(int eType)
{
	switch(eType)
	{
	case E_NetEnt_BlankPeerToPeer:	return "BlankP2P";
	case E_NetEnt_BlankServer:		return "BlankSVR";
	default:						return "Unknown";
	}
}

const char* C_TestStateFactory::GameStateString(int eGameState)
{
	switch(eGameState)
	{
	case 0:		return "Host 0";
	case 1:		return "Host 1";
	case 2:		return "Host 2";
	case 3:		return "Host 3";
	case 4:		return "Host 4";
	case 5:		return "Host 5";
	case 6:		return "Host 6";
	case 7:		return "Host 7";
	default:	return "Unknown";
	}
}