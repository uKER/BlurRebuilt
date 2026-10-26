#ifndef _GAMEPHYSICSMESSAGES_H
#define _GAMEPHYSICSMESSAGES_H

// #include <actor_system/ActorMessages.h>
// #include <Gravity/sharedphysics/MemoryAllocator.h>
// #include <Gravity/sdk/BodyEvents.h>
// #include <gravity/sharedphysics/ContactStreamIterator.h>
#include <misc_shared/uid/FourCC.h>
// #include <shared_actors/SharedActorMessages.h>
// #include <shared_actors/SharedMessageValues.h>
// #include <shared_actors/XtActorMessages.h>
// #include <actor_system/ActorPhysics.h>
#include <Game/ActorComponents/Creation/PhysicsBodyComponentCreate.h>


// Messages
enum E_GamePhysicsBodyMsg
{
	E_GamePhysicsBodyMsg_HiBits = FOURCC_HI20('GPBM'), 

	E_DynObjPhysicsBodyMsg_UpdateShape,

	E_GamePhysicsBodyMsg_End, 
	E_GamePhysicsBodyMsg_Begin = E_GamePhysicsBodyMsg_HiBits + 1, 
	E_GamePhysicsBodyMsg_Count = E_GamePhysicsBodyMsg_End - E_GamePhysicsBodyMsg_Begin
};

#if 0
class C_DynamicObjectPhysicsBodyUpdateShapeMsg : public Messaging::C_MessageDerived<C_DynamicObjectPhysicsBodyUpdateShapeMsg, E_DynObjPhysicsBodyMsg_UpdateShape>
{
public:
	C_DynamicObjectPhysicsBodyUpdateShapeMsg(const Components::C_DynamicObjectPhysicsBodyCreateInfo::TModelElemDescList& elemDescList, int overrideIndex=-1) : m_ElemDescList(elemDescList), m_OverrideIndex(overrideIndex) {}

public:
	Components::C_DynamicObjectPhysicsBodyCreateInfo::TModelElemDescList m_ElemDescList;
	int m_OverrideIndex;
};
#endif

#endif
