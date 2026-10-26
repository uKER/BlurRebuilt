#ifndef NETGAMEWORLD_H
#define NETGAMEWORLD_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netstate/StateEnums.h"
#include "netGame/netGameEnums.h"
#include "gameActors/netTestGameActor.h"
#include "netEnums/netMessageEnums.h"

class C_Header;

namespace netGame
{
	// simple test world
	class C_GameWorld
	{
	public:
		static C_GameWorld* GetInstance();
		static C_GameWorld* CreateInstance(int hostId);
		static void DestroyInstance();

						C_GameWorld(const int uHostId);
						~C_GameWorld();
		void			Update(float t);
		void			Draw(const Maths::HVec3& xTranslation);
		void			LoadLevel();
		void			AddActor(C_GameActor* pxActor, const int uId);
		int				CreateActor(const C_Header& xHeader, bool bServer, int uParentLocalId = -1);
		bool			DestroyActor(const int uId, bool bDestroyNetState);
		C_GameActor*	GetActor(const int uId);
		bool			Spawn(const int uId);
		float			WorldSize();
	private:
		bool			OutsideWorld(const Maths::HVec3& xPos);
		bool			AvoidWorld(int uActorId);
		bool			NetworkSync(float dt);
	private:
		const int			m_uMyHostId;
		C_GameActor**		m_pxActors;
		const Maths::HVec3	m_xWorldBounds;

		static C_GameWorld* s_instance;
	};
};

#endif //NETGAMEWORLD_H