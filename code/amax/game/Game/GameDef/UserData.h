#ifndef GAME_USER_DATA_H
#define GAME_USER_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

class C_PickupMine;
class C_PickupProjectile;

//Embedded user data type
enum E_BodyUserDataType
{
	E_BodyUserDataType_Unknown = 0,

	E_BodyUserDataType_Actor,
	E_BodyUserDataType_MineInner,
	E_BodyUserDataType_MineOuter,
	E_BodyUserDataType_Projectile,

	E_BodyUserDataType_Max,
};

//Base class for user data
class C_BodyUserDataBase
{
public:

	C_BodyUserDataBase()
	{
		m_eType					= E_BodyUserDataType_Unknown;
	}

public:

	E_BodyUserDataType		m_eType;
};

//Actor user data
class C_BodyUserDataActor : public C_BodyUserDataBase
{
public:

	C_BodyUserDataActor()
	{
		m_eType					= E_BodyUserDataType_Actor;
	}

public:

	Actor::C_ActorHandle	m_hActorHandle;
};

//Mine user data
class C_BodyUserDataMine : public C_BodyUserDataBase
{
public:

	C_BodyUserDataMine(const E_BodyUserDataType eType)
	{
		m_eType					= eType;
		m_pMineObject			= NULL;
	}

public:

	C_PickupMine*				m_pMineObject;
};

class C_BodyUserDataProjectile : public C_BodyUserDataBase
{
public:

	C_BodyUserDataProjectile()
	{
		m_eType					= E_BodyUserDataType_Projectile;
		m_pProjectileObject		= NULL;
	}

public:

	C_PickupProjectile*			m_pProjectileObject;
};

} // namespace Game

#endif //  GAME_USER_DATA_H
