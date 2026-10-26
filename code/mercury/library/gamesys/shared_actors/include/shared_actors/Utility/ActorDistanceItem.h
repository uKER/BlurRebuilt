#ifndef SHAREDACTORS_ACTORDISTANCEITEM_H
#define SHAREDACTORS_ACTORDISTANCEITEM_H

#ifdef _MSC_VER
#pragma once
#endif


#include <actor_system/ActorDeclare.h>


namespace Actor
{

// Sortable Actor Distance Item
struct C_ActorDistanceItem
{
	C_ActorDistanceItem(F32 fDist, Actor::C_ActorHandle actorHandle):m_Distance(fDist),m_ActorHandle(actorHandle){}

	F32				m_Distance;
	Actor::C_ActorHandle	m_ActorHandle;

	bool operator<(const C_ActorDistanceItem& other) { return m_Distance < other.m_Distance; }

	typedef Btl::Vector<C_ActorDistanceItem> C_List;
};

} // namespace Actor


#endif // SHAREDACTORS_ACTORDISTANCEITEM_H
