#ifndef ACTORSERIALISATION_05205E72_3547_4885_B340_1637AF18AE4B_H
#define ACTORSERIALISATION_05205E72_3547_4885_B340_1637AF18AE4B_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <misc_shared/Context/I_Anything.h>
#include <misc_shared/UID/FourCC.h>

namespace Sys
{
	class C_ContextualStream;
}


namespace Actor
{

/// Interface for serialising actor handles.
class I_ActorHandleSerialisation : public Sys::C_Anything<FOURCC_U32('IAHS')>
{
public:
	/// @param value actor handle to serialise.
	/// @returns a unique integer that can be deserialised with U32ToActorHandle().
	virtual U32 ActorHandleToU32(const C_ActorHandle& value) = 0;

	/// @param value a unique integer provided by ActorHandleToU32().
	/// @returns the actor handle represented by this value.
	virtual C_ActorHandle U32ToActorHandle(const U32 value) = 0;

};

// overload the default Serialise/Deserialise implementations for C_ActorHandle
void Serialise(Sys::C_ContextualStream& ostream, const C_ActorHandle& value);
void Deserialise(Sys::C_ContextualStream& istream, C_ActorHandle& value);

} // namespace Actor


#endif // ACTORSERIALISATION_05205E72_3547_4885_B340_1637AF18AE4B_H
