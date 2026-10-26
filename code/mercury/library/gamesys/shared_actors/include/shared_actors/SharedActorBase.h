#ifndef SHARED_ACTOR_BASE_H
#define SHARED_ACTOR_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/Actor.h>

namespace Actors
{

class C_SharedActorBase : public Actor::C_Actor
{
public:

	virtual bool					OnMessage(const Messaging::C_MessageBase* pMessage);

private:

};

};	// namespace Actors

#endif