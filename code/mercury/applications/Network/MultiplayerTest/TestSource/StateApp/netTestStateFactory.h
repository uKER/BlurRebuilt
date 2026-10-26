#ifndef NETGAMEFACTORYSTATETEST_H
#define NETGAMEFACTORYSTATETEST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netState/factory/ifactory.h"

class C_TestStateFactory : public I_Factory
{
public:
	C_TestStateFactory(int uNumStates) : I_Factory(uNumStates) {;}
	~C_TestStateFactory()  {}

	// con/des-struction
	bool			CreateGameSideObject(C_Header& xStateInfo, int uGameStateId);
	bool			DestroyGameSideObject(class C_NetHandle&);

	// serialisation
	virtual	bool	SerialiseNetworkEntity(class netStream* pxStream, class C_GameNetEntity& xNetEntity);
	virtual	bool	DeSerialiseNetworkEntity(class netStream* pxStream, class C_GameNetEntity& xNetEntity);

	E_HandleType	GetHandleType(int uTypeId);

	// utils
	const char*		NetEntityString(int eType);	
	const char*		NetStateString(int eType);
	const char*		GameStateString(int eGameState);

	// app
	bool			DistributeEntity(const int uClientId, const int uMyHostId, const int uType, const int uNum) { return false; }

	int				RelevantGameState(int uType);
	int				RelevantNetStateType(int uType);
};

#endif //NETGAMEFACTORYSTATETEST_H