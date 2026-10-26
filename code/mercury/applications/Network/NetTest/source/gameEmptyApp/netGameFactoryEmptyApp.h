#ifndef NETGAMEFACTORYEMPTYAPP_H
#define NETGAMEFACTORYEMPTYAPP_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netstate/factory/ifactory.h"

class netGame
{
		
	class factoryEmptyApp
	{
	public:
		factoryEmptyApp() {}
		~factoryEmptyApp()  {}

		// con/des-struction
		bool CreateNetEntity(C_Header& xStateInfo, int uGameStateId) { return false; }
		bool DestroyNetEntity(const int uEntityId, const int uGameStateId, const bool bDestroyActor = false) { return false; }

		// mediator stuff
		int	  NumOfTypeWithAuthority(int uHostId, int uTypeId) { return 0; }
		bool  Authority(int uHostId, class C_NetHandle& xNetHandle) { return false; };
		bool  Authority(int uHostId, class C_GameHandle& xGameHandle) { return false; }
		class netSerialisable*	GetNetworkEntity(class C_GameHandle& xGameHandle) { return false; }

		// utils
		virtual const char* NetEntityString(int eEntity) { return NULL; }
		virtual const char* NetStateString(int eType) { return NULL; }
		virtual const char* GameStateString(int eGameState) { return NULL; }
	};
};
#endif //NETGAMEFACTORYEMPTYAPP_H