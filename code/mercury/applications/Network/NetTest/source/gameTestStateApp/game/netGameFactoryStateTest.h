#ifndef NETGAMEFACTORYSTATETEST_H
#define NETGAMEFACTORYSTATETEST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netGame/netGameFactory.h"

class C_StateTestFactory : public C_Factory
{
public:
	C_StateTestFactory(int uNumStates) : C_Factory(uNumStates) {;}
	~C_StateTestFactory()  {}

	// con/des-struction
	bool				CreateNetEntity(C_Header& xStateInfo);
	bool				DestroyNetEntity(const int uEntityId, const int uGameStateId, const bool bDestroyActor = false);

	virtual	bool		SerialiseGameActor(class netStream* stream, class C_GameNetEntity&)			{ return false; }
	virtual	bool		SerialiseGameComponent(class netStream* stream, class C_GameNetEntity&)		{ return false; }
	virtual	bool		DeSerialiseGameActor(class netStream* stream, class C_GameNetEntity&)		{ return false; }
	virtual	bool		DeSerialiseGameComponent(class netStream* stream, class C_GameNetEntity&)	{ return false; }

	int					RelevantGameState(int uType);
	int					RelevantNetStateType(int uType);

	// app
	bool			DistributeEntity(const int uClientId, const int uMyHostId, const int uType, const int uNum) { return false; }

	// utils
	virtual const char* NetEntityString(int eEntity);
	virtual const char* GameStateString(int eGameState);
};

#endif //NETGAMEFACTORYSTATETEST_H