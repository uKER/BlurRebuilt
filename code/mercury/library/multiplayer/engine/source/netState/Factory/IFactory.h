#ifndef _I_Factory
#define _I_Factory

#include <btl/containers/map.h>
#include "netState\netEntity\entityHeader.h"
#include "netState\stateEnums.h"

class I_Factory
{
private:
	typedef Btl::Map<int, class C_GameNetEntity*> NetEntityList;
public:
							I_Factory(int uGameStates);
	virtual					~I_Factory();

	virtual void			Init() {;}

	// game side con/des-struction interface
	virtual bool			CreateGameSideObject(C_Header& xStateInfo, int uGameStateId) = 0;
	virtual bool			DestroyGameSideObject(class C_NetHandle&) = 0;

	// con/des-struction
	virtual bool			CreatedGameObject(int uType, int uAuthority, int uDesiredNetId, int uSize);
	virtual bool			CreateGameObject(C_Header& xStateInfo);
	virtual bool			DestroyNetEntity(const int uLocalId, const int uTypeId, const bool bDestroyActor = false);
	virtual bool			DestroyNetEntity2(const int uEntityId, int const uGameStateId, const bool bDestroyActor = false);

	// serialise
	virtual	bool			SerialiseNetworkEntity(class netStream*, class C_GameNetEntity&) = 0;
	virtual	bool			DeSerialiseNetworkEntity(class netStream*, class C_GameNetEntity&) = 0;

	// app
	virtual bool			DistributeEntity(const int uClientId, const int uMyHostId, const int uType, const int uNum) { assert(false); return false; }
	virtual E_HandleType	GetHandleType(int uTypeId) = 0;

	// distribution
	virtual int				RelevantGameState(int uType) = 0;
	virtual int				RelevantNetStateType(int uType) = 0;

	// mediator
	bool					Authority(int uHostId, class C_NetHandle& xNetHandle);
	class C_GameNetEntity*	FindGameEntity(const class C_NetHandle& xNetHandle) const;

	// utils
	virtual const char*		NetEntityString(int eType) = 0;
	virtual const char*		NetStateString(int eType) = 0;
	virtual const char*		GameStateString(int eGameState) = 0;
	virtual bool			DebugGameActor(int uNetId, int uGameStateId, char* szStr, int uStringSize) { return "Default"; }

	// instance
	static I_Factory*		GetInstance() { return m_pxInstance; }
	static I_Factory*		CreateInstance();

protected:
	C_GameNetEntity*		CreateNetEntityAdapter(int uType, int uAuthority, int uDesiredNetId);
	void					RemoveNetEntityAdapter(const C_NetHandle& xNetHandle);
	int						GetPredeterminedNetId(int uDesiredNetId, int uType);

private:
	static I_Factory*	m_pxInstance;
	const int			m_uNumGameStates;
	NetEntityList*		m_xGameNetEntity;
};



#endif