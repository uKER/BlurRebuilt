#include "MiscSharedPch.h"

#include <misc_shared/Messaging/Messaging.h>


/**
@namespace Messaging

Contains the core types comprising the @ref messaging_main.
*/

/**
@page messaging_main Messaging System.

The message system provides a framework for passing messages to objects. There
are several concepts at the core of this:
- @subpage messaging_types.
- @subpage messaging_objects.
- @subpage messaging_handlers.
- @subpage messaging_subscribers.
- @subpage messaging_debug.
- @subpage messaging_serialisation.
*/

/**
@page messaging_types Message Types.

Each message has a type, specified by a type identifier integer stores as 
Messaging::TMsgType. Users of the messaging system are responsible for 
ensuring uniqueness of these type identifiers by whatever means is 
appropriate, to ensure that the meaning of any message can be correctly 
derived from this type identifier.

For compile-time unique identifiers it is recommended practice to create an 
enumerated type that assigns sequential values to message types using a four-
character-code to generate the high 20 bits.
@code
#include <misc_shared/UID/FourCC.h>

enum E_MyMessageType
{
	E_MyMessageType_HiBits = FOURCC_HI20('MYMT'), // pick a sensible FOURCC that is unlikely to clash with other systems

	// start of messages
	E_MyMessageType_Foo, 
	E_MyMessageType_Bar, 

	// end of messages
	E_MyMessageType_End, 
	E_MyMessageType_Begin = E_MyMessageType_HiBits + 1, 
	E_MyMessageType_Count = E_MyMessageType_End - E_MyMessageType_Begin
};
@endcode

Some message types have additional data associated with the message, in which
case there will be a type derived from Messaging::C_MessageBase which is 
uniquely associated with the message type. See @ref messaging_objects for more
information.

To help with identifying problems in message sending and handling, each message
type can be given debug information that the messaging system can display in a
human-readable form when problems occur. See @ref messaging_debug for more
information.
*/

/**
@page messaging_objects Message Objects.

Every message object is of base type Messaging::C_MessageBase, which gets sent 
to message handlers. For a simple message with no additional data, the base 
object can be constructed directly or by using the helper function 
Messaging::BuildSimpleMessage.

Complex message objects are derived from Messaging::C_MessageBase and add 
additional message data. These derived types have a unique 
Messaging::TMsgType and a fixed size. Message objects can be memcpy'd so must 
only contain plain-old-data with at most 4-byte alignment, should not contain 
any virtual functions or multiple inheritence, and should not have memory 
ownership behaviour or refer to memory that might become invalid while copies 
of the message object exist.
@code
#include <misc_shared/Messaging/MessageBase.h>

const Messaging::TMsgType kMyMessageType = 8245124; // don't do this!

class C_MyMessageClass : public Messaging::C_MessageBase // no other bases
{
public:
	C_MyMessageClass() : Messaging::C_MessageBase(kMyMessageType, sizeof(C_MyMessageClass)) {}

	// dtor isn't virtual and mustn't require any code to execute

	static Messaging::TMsgType StaticType() {return kMyMessageType;} // must implement this for template/macro compatibility

	// no virtual functions

	// accessors for member data

private:
	// POD member data

};
@endcode

Rather than risk making mistakes in the reporting of message type or object
size, it is advisable to use this helper class which produces equivalent code:
@code
#include <misc_shared/Messaging/MessageBase.h>

const Messaging::TMsgType kMyMessageType = 8245124; // don't do this!

class C_MyMessageClass : public Messaging::C_MessageDerived<C_MyMessageClass, kMyMessageType> // no other bases
{
public:
	C_MyMessageClass() {}

	// dtor isn't virtual and mustn't require any code to execute

	// no virtual functions

	// accessors for member data

private:
	// POD member data

};
@endcode

The most common complex messages carry a single piece of additional data, such
as an integer, floating-point, vector or matrix value. The 
Messaging::C_MessageValue class can be used for most of these:
@code
#include <misc_shared/Messaging/MessageValue.h>

const Messaging::TMsgType kMyMessageType = 8245124; // don't do this!

typedef Messaging:C_MessageValue<Maths::HVec3, kMyMessageType> C_MyMessageClass;
@endcode

Here we're specifying a message that carries a 3-element floating point vector
as additional data. Messaging::C_MessageValue provides a ctor that initialises 
this data, accessors for the data, and serialisation code too. It relies on the 
existence of a Messaging::C_MessageValueStorage class appropriate to the data 
type. The messaging system provides a templated implementation for simple types 
with up to 4-byte alignment (eg. bool, char, S8, S16, S32, U8, U16, U32, F32) 
and some specialisations for common complex types (eg. Maths::HVec3, 
Maths::HMat43). You can provide your own specialisations for custom data types
so long as you can store that data in some appropriate POD representation.
*/

/**
@page messaging_handlers Message Handling.

An object that wants to receive messages should be derived from 
Messaging::C_MessageHandler:
@code
class C_MyMessageHandler : public Messaging::C_MessageHandler
{
public:
	virtual bool MessageHandler(const Messaging::C_MessageBase* pMessage);

};
@endcode

Inside the MessageHandler function we can inspect the type of the message
object and downcast to the appropriate class if necessary. If we handle the
message then we should return true, otherwise we should forward the message
to our direct parent class or return false:
@code
bool C_MyMessageHandler::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	// pMessage will never be NULL

	switch (pMessage->Type())
	{
	case kMyMessageType:
		{
			const C_MyMessageClass* pMyMessage = Messaging::MessageCast<C_MyMessageClass>(pMessage);
			// do stuff with pMyMessage
		}
		return true;

	default:
		// if you're derived from a different type then forward to that, eg.
		//     return C_MyMessageClassParent::MessageHandler(pMessage);
		// otherwise you don't know how to handle the message so...
		return false;
	}
}
@endcode

Each message handler is associated with a subscriber system (more on that
later), but if we want to subscribe to messages from multiple systems then 
there is a Messaging::C_MessageMultiHandler helper class that automates some 
of the work.
*/

/**
@page messaging_subscribers Message Subscription.

Message subscription is a process for broadcasting a message to zero, one or 
more message handlers, rather than sending a message to a specific handler.

A Messaging::C_MessageSubscriberSystem object can broadcast a message to all 
handlers that subscribe to the correct message type from that system:
@code
#include <misc_shared/Messaging/C_MessageSubscriberSystem.h>

Messaging::C_MessageSubscriberSystem subscriberSystem;

C_MyMessageHandler messageHandler;
messageHandler.SetSubscriberSystem(&subscriberSystem);
messageHandler.AddSubscriber(kMyMessageType);

const C_MyMessageClass message;
subscriberSystem.BroadcastMessage(&message);
@endcode

Subscriptions can be enabled and disabled, although it is often preferable to
remove a subscription until it is needed again. They can also be filtered 
using a bit-pattern, such that a message is only received if there is at
least one matching bit between the pattern used to subscribe and the pattern
used to broadcast:
@code
messageHandler.EnableSubscriber(kMyMessageType, false);
subscriberSystem.BroadcastMessage(&message); // message is not received by messageHandler

messageHandler.EnableSubscriber(kMyMessageType, true);
subscriberSystem.BroadcastMessage(&message); // message is received by messageHandler

messageHandler.SetSubscriberMask(kMyMessageType, 0x1);
subscriberSystem.BroadcastMessage(&message, 0x6); // message is not received by messageHandler

messageHandler.SetSubscriberMask(kMyMessageType, 0x2);
subscriberSystem.BroadcastMessage(&message, 0x6); // message is received by messageHandler
@endcode
*/

/**
@page messaging_debug Message Debugging.

Systems using lots of messaging can be difficult to debug, so the messaging
system provides a debug interface for associating human-readable information
with message types in debug and release builds.

Each message type should be registered with the debug system:
@code
#include <misc_shared/Messaging/MessageDebug.h>

#ifndef CFG_GOLD
Messaging::RegisterDebugInfo(kMyMessageType, "MyMessageType");
#endif
@endcode

If you're following the recommended pattern of assigning messages types from
an enum then you can register all these types in one go:
@code
#include <misc_shared/Messaging/MessageDebug.h>

#ifndef CFG_GOLD
const char* messageNames [] = 
{
	"E_MyMessageType_Foo", 
	"E_MyMessageType_Bar", 
};

StaticAssert(lengthof(messageNames) == E_MyMessageType_Count, array_size_must_match_enum_count);
Messaging::RegisterMessageDebugInfo(messageNames, E_MyMessageType_Count, E_MyMessageType_Begin);
#endif
@endcode

The messaging system uses this debug into to send information to debug loggers
when problems occur. You can direct this to a specific logger using the 
Messaging::SetDefaultLog function.

If you want to log your own debug information then you can query the debug 
info for a message type with Messaging::FindDebugInfo, or if you have a message
object to associate with the problem then you can use 
Messaging::C_MessageBase::GetDebugInfo or Messaging::C_MessageBase::GetDebugName.

Note that there are some debug functions to dump this info at runtime, the
debug variables that trigger these can be found under "system.messaging".
*/

/**
@page messaging_serialisation Message Serialisation.

Messages are designed to be deferred and/or processed across threads or over
networks if necessary, hence the many restrictions placed on message objects.

In order to support some of this functionality, it is necessary to know how to
serialise and deserialise the data contained in message objects. The
Messaging::C_MessageValue class automatically provides serialisation, but 
custom message classes will need to do this themselves if those message types 
are to be serialised. The message class should have a 
Messaging::C_MessageSerialiser object:
@code
#include <misc_shared/Messaging/MessageSerialisation.h>

class C_MyMessageClass : public Messaging::C_MessageDerived<C_MyMessageClass, kMyMessageType> // no other bases
{
public:
	C_MyMessageClass() {}

	// dtor isn't virtual and mustn't require any code to execute

	// no virtual functions

	// accessors for member data

	static void SerialiseContents(Sys::C_ContextualStream& stream, const Messaging::C_MessageBase* pMessage); // write member data to stream
	static void DeserialiseContents(Sys::C_ContextualStream& stream, Messaging::C_MessageBase* pMessage); // read member data from stream

private:
	// POD member data

	static Messaging::C_MessageSerialiser s_Serialiser;

};

Messaging::C_MessageSerialiser C_MyMessageClass::s_Serialiser(C_MyMessageClass::StaticType(), &C_MyMessageClass::SerialiseContents, &C_MyMessageClass::DeserialiseContents);

void C_MyMessageClass::SerialiseContents(Sys::C_ContextualStream& stream, const Messaging::C_MessageBase* pMessage)
{
	const C_MyMessageClass* pMyMessage = static_cast<const C_MyMessageClass*>(pMessage);
	// stream << pMyMessage->m_Something;
}

void C_MyMessageClass::DeserialiseContents(Sys::C_ContextualStream& stream, Messaging::C_MessageBase* pMessage)
{
	C_MyMessageClass* pMyMessage = static_cast<C_MyMessageClass*>(pMessage);
	// stream >> pMyMessage->m_Something;
}
@endcode
*/
