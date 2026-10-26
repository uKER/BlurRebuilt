#ifndef ACTORMESSAGEHANDLE_H
#define ACTORMESSAGEHANDLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageData.h>
#include <misc_shared/messaging/MessageSerialisation.h>


namespace Actor
{

/**
@brief Base class for messages carrying an actor handle.
@ingroup actor_group_messages

For messages that carry an actor handle as data, this class can be used to 
automate the class generation.
@code
typedef Actor::C_MessageActorHandle<E_MyMessageTypes_MyMessage> C_MyMessageClass;

bool MyMessageHandler::MessageHandler(const C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case E_MyMessageTypes_MyMessage:
		{
			Actor::C_ActorHandle actorHandle = Messaging::MessageCast<C_MyMessageClass>(pMessage)->GetActorHandle();
			// do something with actor handle
		}
		return true;

	default:
		return MyBaseClass::MessageHandler(pMessage);
	}
}
@endcode

@param t_TypeId TMsgType identifying the message class.
*/
template <Messaging::TMsgType t_TypeId>
class C_MessageActorHandle : public C_MessageActorData
{
public:
	C_MessageActorHandle() : C_MessageActorData(t_TypeId, sizeof(C_MessageActorHandle)) {s_Serialiser.DummyForceLinkage();}
	explicit C_MessageActorHandle(const C_ActorHandle actorHandle) : C_MessageActorData(t_TypeId, sizeof(C_MessageActorHandle), actorHandle) {s_Serialiser.DummyForceLinkage();}
	// auto: C_MessageActorHandle(const C_MessageActorHandle& copy);
	// auto: ~C_MessageActorHandle();
	// auto: C_MessageActorHandle& operator=(const C_MessageActorHandle& copy);

	static Messaging::TMsgType StaticType() {return t_TypeId;} ///< @returns type identifier for this message class.

private:
	static Messaging::C_MessageSerialiser	s_Serialiser; ///< registers serialisation functions

};

template <Messaging::TMsgType t_TypeId>
Messaging::C_MessageSerialiser C_MessageActorHandle<t_TypeId>::s_Serialiser(t_TypeId, &C_MessageActorData::SerialiseContents, &C_MessageActorData::DeserialiseContents);

} // namespace Actor


#endif // ACTORMESSAGEHANDLE_H
