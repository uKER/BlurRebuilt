#ifndef NETGAMEFACTORY_H
#define NETGAMEFACTORY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/map.h"
#include "netstate/factory/ifactory.h"

// C_SessionFactory
class C_SessionFactory : public I_Factory
{
public:
						C_SessionFactory(int uGameStates);
	virtual				~C_SessionFactory();

	// con/de-struction
	bool				CreateGameSideObject(C_Header& xStateInfo, int uGameStateId);
	bool				DestroyGameSideObject(class C_NetHandle&);

	// serialisation
	virtual	bool		SerialiseNetworkEntity(class netStream*, class C_GameNetEntity&);
	virtual	bool		DeSerialiseNetworkEntity(class netStream*, class C_GameNetEntity&);

	E_HandleType		GetHandleType(int uTypeId);

	// distribution
	virtual int			RelevantGameState(int uType);
	virtual int			RelevantNetStateType(int uType);

	// utils
	const char*			NetEntityString(int eType);
	const char*			NetStateString(int eType);
	const char*			GameStateString(int eGameState);
};

#endif //NETGAMEFACTORY_H