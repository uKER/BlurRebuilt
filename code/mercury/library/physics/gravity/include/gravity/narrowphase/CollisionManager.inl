#ifndef COLLISIONMANAGER_INL
#define COLLISIONMANAGER_INL

#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/Shapes.h"



namespace Collision
{


template <U32 MaxPatches, U32 MaxContacts, U32 DesiredPatches, U32 DesiredContacts>
NOALIAS FORCE_INLINE void C_CollisionManager::StoreContacts(const C_CollisionObject* const a, 
		const C_CollisionObject*  const b, SharedPhysics::C_ContactStream& stream, 
		C_CollisionResult<MaxPatches, MaxContacts>& results)
{
	ReduceContactsAndPackToStream<MaxPatches, MaxContacts, DesiredPatches, DesiredContacts>(results, stream, results.m_pCurrentCollider);
}

template <U32 MaxPatches, U32 MaxContacts, U32 DesiredPatches, U32 DesiredContacts>
NOALIAS FORCE_INLINE void C_CollisionManager::StoreContactsSpu(const C_CollisionObject* const a, 
		const C_CollisionObject*  const b, SharedPhysics::C_ContactStream& stream, 
		C_CollisionResult<MaxPatches, MaxContacts>& results)
{
	ReduceContactsAndPackToStreamSpu<MaxPatches, MaxContacts, DesiredPatches, DesiredContacts>(results, stream, results.m_pCurrentCollider);
}


//NB - character controller is always body A!!!
template <U32 MaxPatches, U32 MaxContacts, U32 DesiredPatches, U32 DesiredContacts>
inline void C_CollisionManager::StoreContactsCharacter(const C_CollisionObject* const a, 
	const C_CollisionObject*  const b, SharedPhysics::C_ContactStream& stream, 
	C_CollisionResult<MaxPatches, MaxContacts>& results)
{
	ReduceContactsAndPackToStream<MaxPatches, MaxContacts, DesiredPatches, DesiredContacts>(results, stream,
		results.m_pCurrentCollider);
}

};

#endif

