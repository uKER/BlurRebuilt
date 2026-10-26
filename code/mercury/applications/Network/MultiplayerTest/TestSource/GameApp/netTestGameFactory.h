#ifndef NETTESTGAMEFACTORY_H
#define NETTESTGAMEFACTORY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netGame/netGameFactory.h"

const int MAX_ENTITIES = 1000;

// C_TestGameFactory
class C_TestGameFactory : public C_Factory
{
public:
	C_TestGameFactory();
	~C_TestGameFactory();

	// con/de-struction
	bool	CreateNetEntity(class C_Header& xStateInfo);
	bool	DestroyNetEntity2(const int uEntityId, int const uGameStateId, const bool bDestroyActor = false);

	// app
	bool			DistributeEntity(const int uClientId, const int uMyHostId, const int uType, const int uNum);

	/*
	virtual	bool	SerialiseGameActor(class netStream* stream, class C_GameNetEntity&);
	virtual	bool	SerialiseGameComponent(class netStream* stream, class C_GameNetEntity&);
	virtual	bool	DeSerialiseGameActor(class netStream* stream, class C_GameNetEntity&);
	virtual	bool	DeSerialiseGameComponent(class netStream* stream, class C_GameNetEntity&);
	/**/

	int				RelevantGameState(int uType);
	int				RelevantNetStateType(int uType);

	// utils
	const char*		NetEntityString(int eType);

	int		GetNetHandle(int uLocalId);
	int		GetLocalHandle(int uNetId)						{ return m_uLocalHandle[uNetId]; }
	void	SetLocalHandle(int uNetId, int uLocalHandle)	{ assert(uNetId >= 0); if(uNetId >= 0) m_uLocalHandle[uNetId] = uLocalHandle; }
	bool	Authority(int uHostId, int uLocalId);
	int		PredictNetId();

	static C_TestGameFactory* GetInstance() { return (C_TestGameFactory*)I_Factory::GetInstance(); }

private:

	int	m_uLocalHandle[MAX_ENTITIES];
};

#endif //NETTESTGAMEFACTORY_H