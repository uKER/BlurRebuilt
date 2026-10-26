#ifndef ACTORHANDLELISTITEM_H
#define ACTORHANDLELISTITEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>

#include <Btl/Allocators/pool.h>
#include <Btl/Containers/list.h>


namespace Actor
{

/**
@ingroup actor_group_utility
@brief List item for use with C_ActorHandleList.
*/
class C_ActorHandleListItem : public Btl::ListElementDouble<C_ActorHandleListItem>
{
public:
	void					SetHandle(const C_ActorHandle &handle) {m_Handle = handle;}
	const C_ActorHandle &	GetHandle() const {return m_Handle;}

private:
	C_ActorHandle			m_Handle;		///< handle of actor

};

} // namespace Actor


#endif // ACTORHANDLELISTITEM_H
