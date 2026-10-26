#include "SharedActorsPch.h"
#include <shared_actors/Utility/PhysicsBodyUserData.h>

#include <actor_system/Actor.h>
#include <btl/allocators/pool.h>
#include <shared_actors/Components/Interfaces/I_PhysicsBodyComponents.h>




namespace PhysicsBodyUserData_Impl
{

static Btl::DynamicPool<Actors::C_SimplePhysicsBodyUserData> g_SimplePhysicsBodyUserDataPool;
static Btl::DynamicPool<Actors::C_PBCUserData> g_PBCUserDataPool;

} // namespace PhysicsBodyUserData_Impl




namespace Actors
{

/// @param typeUid unique identifier for derived type.
C_PhysicsBodyUserData::C_PhysicsBodyUserData(const U32 typeUid)
	: GamePhysics::C_BodyUserData(typeUid)
	, m_pOwner(NULL)
{
}

/// @note: dtor is not virtual, don't delete base class ptr.
C_PhysicsBodyUserData::~C_PhysicsBodyUserData()
{
}




/**
Users should never construct this object directly, use C_SimplePhysicsBodyUserData::Create
to instantiate.
*/
C_SimplePhysicsBodyUserData::C_SimplePhysicsBodyUserData()
	: GamePhysics::C_BodyUserDataImpl<C_SimplePhysicsBodyUserData, C_PhysicsBodyUserData>()
{
}

/// @note: dtor is not virtual, don't delete base class ptr.
C_SimplePhysicsBodyUserData::~C_SimplePhysicsBodyUserData()
{
}

/**
@param pOwner actor.
@returns user data to attach to the physics body.
*/
C_SimplePhysicsBodyUserData* C_SimplePhysicsBodyUserData::Create(const Actor::C_Actor* pOwner)
{
	BIZ_ASSERT(pOwner != NULL);

	C_SimplePhysicsBodyUserData* pUserData = PhysicsBodyUserData_Impl::g_SimplePhysicsBodyUserDataPool.Allocate();

	if (pUserData != NULL)
	{
		pUserData->SetOwner(pOwner);
	}

	return pUserData;
}

/// @param pUserData user data to destroy.
void C_SimplePhysicsBodyUserData::Destroy(C_SimplePhysicsBodyUserData* pUserData)
{
	if (pUserData != NULL)
	{
		PhysicsBodyUserData_Impl::g_SimplePhysicsBodyUserDataPool.Free(pUserData);
	}
}




/**
Users should never construct this object directly, use C_PBCUserData::Create
to instantiate.
*/
C_PBCUserData::C_PBCUserData()
	: GamePhysics::C_BodyUserDataImpl<C_PBCUserData, C_PhysicsBodyUserData>()
	, m_pPBC(NULL)
{
}

/// @note: dtor is not virtual, don't delete base class ptr.
C_PBCUserData::~C_PBCUserData()
{
}

/**
@param pOwner actor using Components::I_PhysicsBody.
@param pPBC physics body component that owns this data.
@returns user data to attach to the physics body.
*/
C_PBCUserData* C_PBCUserData::Create(const Actor::C_Actor* pOwner, const Components::I_PhysicsBody* pPBC)
{
	BIZ_ASSERT(pOwner != NULL);
	BIZ_ASSERT(pPBC != NULL);

	C_PBCUserData* pUserData = PhysicsBodyUserData_Impl::g_PBCUserDataPool.Allocate();

	if (pUserData != NULL)
	{
		pUserData->SetOwner(pOwner);
		pUserData->m_pPBC = pPBC;
	}

	return pUserData;
}

/// @param pUserData user data to destroy.
void C_PBCUserData::Destroy(C_PBCUserData* pUserData)
{
	if (pUserData != NULL)
	{
		PhysicsBodyUserData_Impl::g_PBCUserDataPool.Free(pUserData);
	}
}

} // namespace Actors
