#include "ActorSystemPch.h"

#include "actor_system/ActorSerialisation.h"

#include <misc_shared/Context/ContextualStream.h>


namespace Actor
{

/**
@class I_ActorHandleSerialisation
@ingroup actor_group_serialisation

Handles can not be serialised directly as they refer to addresses in local
memory. If an application wants to serialise handles it will need to implement
this interface, eg.
@code
class C_ActorHandleLUT : public Actor::I_ActorHandleSerialisation
{
public:
	// methods to populate m_Data, serialise and deserialise the contents, and to create actors from it

	// from Actor::I_ActorHandleSerialisation
	virtual U32 ActorHandleToU32(const C_ActorHandle& value) {return Find(value);}
	virtual C_ActorHandle U32ToActorHandle(const U32 value) {return m_Data[value];}

private:
	Btl::Vector<C_ActorHandle> m_Data;

};
@endcode

The stream used for serialisation of actor handles should then have an 
instance of this implementation added as context, eg.
@code
void SerialiseTheGame(Sys::I_BinaryStream& stream)
{
	Sys::C_ContextualStream ostream;

	C_ActorHandleLUT* pActorHandleLUT = new C_ActorHandleLUT;
	pActorHandleLUT->Init(C_GameWorld::Get()->GetActorList());
	pActorHandleLUT->Serialise(stream);
	ostream.AddContextItem(Actor::I_ActorHandleSerialisation::ClassUid(), pActorHandleLUT);

	// serialise game objects into ostream...
}
@endcode

Code wanting to serialise actor handles should use the stream operators on
C_ContextualStream objects as normal:
@code
struct Example
{
	Actor::C_ActorHandle m_ActorHandle;

	void Serialise(C_ContextualStream& ostream) const {ostream << m_ActorHandle;}
	void Deserialise(C_ContextualStream& istream) const {istream >> m_ActorHandle;}

};
@endcode
*/

void Serialise(Sys::C_ContextualStream& ostream, const C_ActorHandle& value)
{
	ostream << ostream.GetContextItem<I_ActorHandleSerialisation>()->ActorHandleToU32(value);
}

void Deserialise(Sys::C_ContextualStream& istream, C_ActorHandle& value)
{
	U32 temp = 0;
	istream >> temp;
	value = istream.GetContextItem<I_ActorHandleSerialisation>()->U32ToActorHandle(temp);
}

} // namespace Actor
