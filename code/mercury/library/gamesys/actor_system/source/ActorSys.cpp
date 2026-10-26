#include "ActorSystemPch.h"

#include <actor_system/ActorSys.h>


/**
@mainpage Actor System User Guide

Note: throughout this manual the term C_Super is used to refer to the direct 
base class of any derived class.

The units of currency in the actor system are actors and components. Instances of 
Actor::C_Actor are stand-alone objects that exists in it's their right and 
contains zero, one or more instances of Actor::I_ActorComponent. A component only 
ever exists inside an actor.

Actors are referred to using the Actor::C_ActorHandle type, which identifies
a specific Actor::C_Actor object within a given Actor::C_ActorList. A 
Component::C_ComponentInterfaceId can be used to refer to a component object
within that actor, implying that an actor can have at most one component of 
any given interface type.

Other than this actors and components are very similar in nature. Each actor
or component type has a corresponding creation data type, which a factory
object can use to create the actor or component object. Actors and components
have a lifetime model that takes them through various stages of creation. They
derive from Messaging::C_MessageHandler and receive messages from the 
Actor::C_ActorList object they belong to. They support serialisation and
deserialisation for save games and for network synchronisation.

The framework provided by the actor system is fairly complex but most actor
and component code only relies on small parts of this and is comparitively
simple. The trick to keeping that code simple is knowing how to work within
the framework:
- @subpage actorsys_rtti
- @subpage actorsys_creation
- @subpage actorsys_lifetime
- @subpage actorsys_dependencies
- @subpage actorsys_messaging
- @subpage actorsys_serialisation
- @subpage actorsys_context
- @subpage actorsys_debug
- @subpage actorsys_resources

Specific differences between actors and components are covered here:
- @subpage actor_main
- @subpage component_main
*/

/**
@page actorsys_rtti Run Time Type Information.

There are various class hierarchies used by the actor system where a base
class object needs to be downcast to a derived class, or where the derived type
needs to be referenced for some other reason. We're not expecting to have C++
RTTI enabled so we use integer unique IDs as type information. Some examples
of this are:
- Actor::C_ActorTypeId.
- Component::C_ComponentTypeId.
- Component::C_ComponentInterfaceId.
- Messaging::TMsgType.
- Anything::C_AnythingUid.

In all cases, users of the system are responsible for ensuring uniqueness of
these type identifiers. It is recommended that related type identifiers are 
grouped together into sequential values defined by an enumerated type:
@code
#include <misc_shared/UID/FourCC.h>

enum E_LightActorType
{
	E_LightActorType_HiBits = FOURCC_HI20('LGTA'), // this must not clash with FOURCC codes used to see other actor type enums

	E_LightActorType_Directional, 
	E_LightActorType_Point, 
	E_LightActorType_Spot,

	E_LightActorType_End, 
	E_LightActorType_Begin = E_LightActorType_HiBits + 1,
	E_LightActorType_Count = E_LightActorType_End - E_LightActorType_Begin
};
@endcode

Apply common sense when defining these type identifiers. If you put too many
into the same monolithic header file then you'll have to recompile your entire
game every time you want to change that file. If you split them across too
many header files then you might have trouble identifying duplicate values. 
Find a balance that works for you.
*/

/**
@page actorsys_creation Creating Actors and Components.

The process of creating actors or components involves creation data and 
factories. The game configures a factory to create a specific object in 
response to a requested type identifier. That object knows how to interpret
the information contained within the creation data.

Creation data can either come from ActorStudio or be created at runtime 
using an additional factory type. In either case it is referenced by an
abstract smart-pointer type that has reference-counting behaviour and can be
serialised. The DataRef::C_Ptr type is specialised to 
Actor::C_ActorCreationData::C_DataRef for actor creation data, and to
Component::I_ComponentCreator::C_DataRef for component creation data. The 
actor or component object holds this reference over it's entire lifetime.

Runtime creation data must support serialisation, as the actor or component
it creates may also need to be serialised. Deserialisation of creation data
only ever happens before the actor or component is created.
@code
void C_MyType::Serialise(Sys::C_ContextualStream& stream) const
{
	C_Super::Serialise(stream);

	stream << m_Something;
	stream << m_SomethingElse;
}

void C_MyType::Deserialise(Sys::C_ContextualStream& stream)
{
	C_Super::Deserialise(stream);

	stream >> m_Something;
	stream >> m_SomethingElse;
}
@endcode

When building creation data at runtime, it is easiest to use a helper class
that will request a creation data object from the appropriate factory, 
provide a writeable pointer to that object for filling the creation data, and
hold a data reference to that object that can then be used to create the actor
or component object. Actor::C_ActorCreationDataFactoryHelper and 
Component::C_ComponentCreatorFactoryHelper are the helper classes for this.

Sometimes we want to be able to create the same type of actor or component
using creation data from ActorStudio or at runtime. Here we typically make a
base class that is used by the actor or component, and derive from this to
make two different creation data types.
@code
class C_MyCreationDataBase : public C_Super
{
public:
	// the usual stuff, such as RTTI

	// accessors that will be used by the actor or component
	virtual const char* GetSomeString() const = 0;

};

class C_MyCreationDataActorStudio : public C_MyCreationDataBase
{
public:
	// Xt annotated members
	XtSz8 m_SomeString;

	// from C_MyCreationDataBase
	virtual const char* GetSomething() const {return m_SomeString.Text();}

};

class C_MyCreationDataRuntime : public C_MyCreationDataBase
{
public:
	// from C_MyCreationDataBase
	virtual const char* GetSomething() const {return m_SomeString.Chars();}

private:
	// member data
	Btl::String m_SomeString;

};
@endcode

See @ref actor_creation and @ref component_creation for specific details of
creation data types.
*/

/** 
@page actorsys_lifetime Lifetime State.

Creating an actor or component is a complicated affair but the framework 
handles most of the book-keeping, so game actor code should only need to plug
specific details into the correct places. Each object has an 
ActorSys::E_LifetimeState and there are various hooks in the actor and 
component base classes that specify the transitions between these states.

Construction of the object happens first (assuming that the factory creation
method for that type invokes the ctor). This should do little more than basic
member data initialisation. The actor system then hooks up the creation data 
reference to the object, after which Actor::C_Actor::GetCreationData or
Component::I_GameComponent::GetCreator will give valid creation data objects.

The object now has ActorSys::E_LifetimeState_Constructed, and will next be
initialised. Here we shouldn't be doing much more than setting member variables
to values supplied by the creation data. Actors should also initialise their
components here, see @ref actor_components. We should return true if the object 
is successfully initialised, or false if the object can not be created.
@code
bool C_MyType::InitialiseImpl()
{
	// initialise base class
	if (!C_Super::InitialiseImpl())
		return false;

	// set members based on creation data state
	const C_MyTypeCreate* pCreate = static_cast<const C_MyTypeCreate*>(GetCreationData().GetData());

	m_WorldMatrix = pCreate->GetCreationMatrix();
	m_SomethingElse = pCreate->GetInitialSomethingElse();

	// actors may initialise components here

	// initialised ok
	return true;
}
@endcode

When we create a fresh object from ActorStudio data or from runtime creation
data it will get initialised. If we're loading a saved game then this will not
occur and the object will instead be deserialised. See @ref 
actorsys_serialisation for more details.

The object now has ActorSys::E_LifetimeState_Initialised, and will next be
primed. This is used to request asynchronous loading of any additional data
that the object needs. If the object is waiting for any data then it should
return false. See @ref actorsys_resources for more details.
@code
bool C_MyType::PrimeImpl()
{
	// prime base class
	bool primed = C_Super::PrimeImpl();

	// request any data that we need
	const C_MyTypeCreate* pCreate = static_cast<const C_MyTypeCreate*>(GetCreationData().GetData());

	m_DataHandle.Load(pCreate->GetDataFileName());

	primed &= !m_DataHandle.IsWaiting();

	// done
	return primed;
}
@endcode

If we returned false from PrimeImpl then we now have 
ActorSys::E_LifetimeState_Priming. We need to call SetPrimed when all the data 
we requested has notified us that it's ready.
@code
void C_MyType::SomeKindOfNotificationCallback(void* pSomeKindOfData)
{
	if (!m_DataHandle.IsWaiting() && !m_OtherDataHandle.IsWaiting())
	{
		// we're no longer waiting for data
		SetPrimed();
	}
}
@endcode

The object now has ActorSys::E_LifetimeState_Primed, and will eventually be
created. It is not uncommon to prime a collection of objects in parallel and
then create them all when none of them are waiting for data, so there may be
a significant delay between priming and creating an object.

When we create an object there are a few things we typically want to do. We
might subscribe to message types that we want our Actor::C_ActorList to 
broadcast to us. We might hook into game world systems such as rendering, 
physics or audio. Again, we should return false if we can not create the 
object for some reason.
@code
bool C_MyType::CreateImpl()
{
	// create base class
	if (!C_Super::CreateImpl())
		return false;

	// make sure our data was loaded
	if (!m_DataHandle.IsValid())
		return false;

	// subscribe to some messages
	AddSubscriber(E_MyMessageTypes_SomeMessage);

	// hook into game world
	m_pSomething = C_GameWorld::GetInstance()->CreateSomething(m_DataHandle.Get());

	if (m_pSomething == NULL)
		return false;

	// created successfully
	return true;
}
@endcode

The object now has ActorSys::E_LifetimeState_Created and is a valid entity in
the game. It will remain so until it is killed, where it should unhook itself
from the game world.
@code
void C_MyType::KillImpl()
{
	// unhook from game world
	if (m_pSomething != NULL)
	{
		C_GameWorld::GetInstance()->DestroySomething(m_pSomething);
		m_pSomething = NULL;
	}

	// kill base class
	C_Super::KillImpl();
}
@endcode

The object now has ActorSys::E_LifetimeState_Killed and will be cleaned up by
the actor system, at which point the object will be destructed (assuming that
the factory destroy function invoked the dtor).
*/

/**
@page actorsys_dependencies Dependencies between Actors and Components.

Although it's preferable to avoid dependencies by code design, there may be
situations where a component depends upon the state of the actor that owns it
or upon the state of another component owned by the same actor. If this is the
case then the component may need to perform one-time setup using this state, 
but the state may not be valid at the point CreateImpl() is called.

We can override InternalDependenciesCreated() to deal with this:
@code
void C_MyType::InternalDependenciesCreated()
{
	C_Super::InternalDependenciesCreated();

	// perform one-time setup making use of GetActor(), GetActor()->QueryInterface(), etc.
}
@endcode

Another kind of dependency that can not always be resolved during CreateImpl()
is where an actor or component depends on the state of another actor belonging 
to the same ActorStudio actor list. We have no control over the order that 
actors within a list are created so we must assume that we can not access the
other actor(s) in our CreateImpl().

When all actors in a list have been created ExternalDependenciesCreate() will
be called:
void C_MyType::ExternalDependenciesCreated()
{
	C_Super::ExternalDependenciesCreated();

	// perform one-time setup making use of GetActorList()->GetActorPtr(...).
}
*/

/**
@page actorsys_messaging Messaging and the Actor System.

The actor system builds on the messaging system, see @ref messaging_main for 
an overview.

Every actor and component is a Messaging::C_MessageHandler and the 
Actor::C_ActorList has a Messaging::C_MessageSubscriberSystem. Actors and 
components always use the subscriber system of the actor list they belong to.

Messages are processed by overriding OnMessage method:
@code
bool C_MyType::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
	case E_MyMsgType_SomeMessage:
		// do something
		return true;

	default:
		return C_Super::OnMessage(pMessage);
	}
}
@endcode

The OnMessage method of actors and components should never be called
directly. There are 3 basic ways of sending messages in the actor system:
- Actor::C_ActorList::BroadcastMessage sends the message to all actors and 
components subscribed to it.
- Actor::C_ActorList::SendMessageToActor sends the message to an actor and 
to all of it's components.
- Actor::C_ActorList::SendMessageToActorComponent sends the message to a
specific component of an actor.

The actor system supports user-configurable messages queues, allowing messages
to be deferred until the user choses to flush a queue. All of the above 
methods have an optional parameter that specifies the index of the message 
queue to use, with the default value specifing that the message should be sent
immediately instead of being queued.
*/

/**
@page actorsys_serialisation Serialisation of Actors and Components.

There are actually two very different applications of serialisation with the
actor system, which share some common functionality. Serialisation involves
writing to a Sys::C_ContextualStream object, and deserialisation reading from
the same type, which allows extensible behaviour.

Serialisation for save is used to store the state of a group of actors and
components, such that they can be killed and later recreated from the 
serialised data. Serialisation for sync is used to synchronise the state of
an actor or component, such that several copies can exist independently and 
mirror each other.

In either case, if the creation data of an actor or component was provided by
ActorStudio then it is serialised using a unique identifier contained in the
Xt database. For creation data provided dynamically at runtime, the state of
that data must be serialised:
@code
void C_MyCreationType::Serialise(Sys::C_ContextualStream& stream) const
{
	C_Super::Serialise(stream);

	stream << m_Something;
	stream << m_SomethingElse;
}

void C_MyCreationType::Deserialise(Sys::C_ContextualStream& stream)
{
	C_Super::Deserialise(stream);

	stream >> m_Something;
	stream >> m_SomethingElse;
}
@endcode

Deserialisation for save of actors and components replaces the usual 
initialisation phase. After the data has been deserialise the object has
ActorSys::E_LifetimeState_Initialised and will then be primed and created.
@code
void C_MyCreationType::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_Super::SerialiseForSave(stream);

	stream << m_WorldMatrix;
	stream << m_SomethingElse;
}

void C_MyCreationType::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_Super::DeserialiseForSave(stream);

	stream >> m_WorldMatrix;
	stream >> m_SomethingElse;
}
@endcode

Deserialisation for sync happens while the actor or components is already
created, so often requires more complex processing than simply overwriting
member data.
@code
void C_MyCreationType::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_Super::SerialiseForSync(stream);

	stream << m_WorldMatrix;
	stream << m_SomethingElse;
}

void C_MyCreationType::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_Super::DeserialiseForSync(stream);

	Maths::HMat43 worldMatix;
	stream >> worldMatix;
	SetWorldMatrix(worldMatix);

	U32 somethingElse;
	stream >> somethingElse;
	SetSomethingElse(somethingElse);
}
@endcode

It's not uncommon to need to serialise handles to other actors. This is
supported through the Actor::I_ActorHandleSerialisation context. The code that
provides the stream object should add a suitable implementation of this context
to the stream before serialising or deserialising the objects. They can then
read and write actor handles using the stream operators.
@code
#include <actor_system/ActorSerialisation.h>

// a bare-bones implementation of I_ActorHandleSerialisation
class C_SampleActorHandleSerialisation : public Actor::I_ActorHandleSerialisation
{
public:
	static const U32 kInvalidIndex = 0xffffffff;

	C_SampleActorHandleSerialisation(const Actor::C_ActorHandle* pActorHandles, const U32 numActorActors)
		: Actor::I_ActorHandleSerialisation()
		, m_pActorHandles(pActorHandles)
		, m_NumActorActors(numActorActors)
	{
	}

	// from Actor::I_ActorHandleSerialisation
	virtual U32 ActorHandleToU32(const Actor::C_ActorHandle& value)
	{
		for (U32 i = 0; i < m_NumActorActors; ++i)
			if (m_pActorHandles[i] == value)
				return i;

		return kInvalidIndex;
	}

	// from Actor::I_ActorHandleSerialisation
	virtual Actor::C_ActorHandle U32ToActorHandle(const U32 value)
	{
		if (value < m_NumActorActors)
			return m_pActorHandles[value];

		return Actor::C_ActorHandle();
	}

private:
	const Actor::C_ActorHandle*	m_pActorHandles; // weak ptr, does not own this data
	U32							m_NumActorActors; // number of entries in m_pActorHandles

};

// some pretend bit of code that wants to serialise an actor handle
void Write(Sys::C_ContextualStream& stream)
{
	Actor::C_ActorHandle someActor = GetSomeActor();
	stream << someActor;
}

// some pretend bit of code that wants to deserialise an actor handle
void Read(Sys::C_ContextualStream& stream)
{
	Actor::C_ActorHandle someActor;
	stream >> someActor;
	SetSomeActor(someActor);
}
@endcode
*/

/**
@page actorsys_context How do I get the Game World?

The short answer is that you don't get the game world.

In game code it is common for actors and components to use the game world to
access systems and objects. This introduces physical code dependencies which
can greatly increase build times and increase the amount of effort needed to
refactor code. Shared code does not have a game world so needs another way to
access these systems and objects.

The solution used both for decoupling game code and for supplying information 
to shared code is for every actor to exist in a context supplied by the user.
Currently this is set for an entire actor list and shared by every actor and
component in that list, but in future we may extend this to allow individual
actors to be assigned unique contexts.

We can define a number of different contexts for actors, eg. render context; 
physics context; audio context; etc. Each of these is represented by an 
interface class used by the actor and component code, and a concrete class
implemented by game code. Each context uses Sys::I_Anything to provide RTTI
that allows actors and components to request the context interface if it is
present.
@code
#include <misc_shared/Context/I_Anything.h>
#include <misc_shared/UID/FourCC.h>

// the context interface
class I_MyContext : public Sys::C_Anything<FOURCC_U32('MYCT')>
{
public:
	virtual C_SomeSystem& GetSomeSystem() const = 0;
};

// some actor code that needs this context
void C_MyActor::DoSomething()
{
	I_MyContext* pMyContext = GetContext<I_MyContext>();

	if (pMyContext != NULL)
	{
		pMyContext->GetSomeSystem().DoSomethingWithMyData(m_Data);
	}
}
@endcode

If an actor or component can not exist without a specific context then it is
not uncommon to check for the existence of this context in the CreateImpl 
function and to fail to create the actor if the context is missing. It is 
also not uncommon to cache pointers to systems or objects as member data of
the actor or component, whether or not this is valid behaviour depends on the
semantics of the context interface that supplied those pointers. Note that 
contexts can not be accessed in actor and component destructors, any cleanup
code that requires a context should be performed in KillImpl.

Games using actors or components that require contexts should provide these
contexts to the actor list before creating any actors, and should not remove
those contexts until after the actors have been destroyed. The game will need
to create concrete context objects and add these to the Sys::I_ContextProvider
object used by the actor list.
@code
// concrete implementation of I_MyContext
class C_MyContext : public I_MyContext
{
public:
	virtual C_SomeSystem& GetSomeSystem() const
	{
		return C_GameWorld::GetInstance()->GetSomeSystem();
	}
};

// my game's implementation of Sys::I_ContextProvider for the actor system
class C_MyActorContextProvider : public Sys::I_ContextProvider
{
public:
	virtual const Sys::I_Anything* GetContext(const C_AnythingUid typeId) const
	{
		switch (typeId)
		{
		case I_MyContext::kClassUid:
			return &m_MyContext;

		default:
			return NULL;
		}
	}

private:
	C_MyContext	m_MyContext;

};

// in some code where my game creates an actor list...
void CreateActorList()
{
	m_pActorContextProvider = new C_MyActorContextProvider;
	m_pActorList = new Actor::C_ActorList(kMaxNumActors, kNumActorMsgQueues, m_pActorFactory, m_pActorContextProvider);
}
@endcode
*/

/**
@page actorsys_debug Debugging with the Actor System.

Actor and component code needs to be debugged in conventional ways like any
other code, but it also needs to diagnose problems that originate from data
generated in ActorStudio. Having diagnosed problems it then needs to report 
these to the appropriate people, and if at all possible to recover gracefully
so that the remaining data can be processed.

Reporting typically happens either by sending text to debug logs or by 
displaying debug information on screen as part of the DebugRender phase. 
Either of these may utilise debug variables to configure the amount of debug
information supplied. It is typical to group these variables under the 
ActorDebug name, and further into "ActorDebug.Actors.<ActorType" or 
"ActorDebug.Components.<ComponentType>" if they relate to specific actor or
components types. Note also that the actor system is heavily dependent upon 
the messaging system and so can make use of the debug features detailed in
messaging_debug.

It is sometimes the case that actors or components can be hard-coded to send
text to a specific debug logger. Sometimes, especially in shared code, this is
not possible and the correct debug logger must be determined at runtime based
on available data. The actor system provides functions for obtaining debug
logger objects to use in various situations:
- for data setup in ActorStudio, use Actor::GetDebugLogLevelDesign and if
possible pass in the actor.
- for data setup in Sorcery, use Actor::GetDebugLogSorcery and if possible
pass in the record or handle.
- for data provided as stand-alone assets, use Actor::GetDebugLogAsset and if
possible pass in the filename that the asset was loaded from.
- for other data use Actor::GetDebugLogGeneral.

Each game should supply an implementation of Actor::I_ActorDebugLogs that
handles these requests and applies any game-specific filtering that might be
available. The game should create this object and pass it to 
Actor::SetDebugLogs as part of system initialisation.
@code
class C_MyActorDebugLogs : public Actor::I_ActorDebugLogs
{
public:
	virtual Debug::Logger* GetGeneral() const
	{
		// return your catch-all debug logger
	}

	virtual Debug::Logger* GetLevelDesign(const C_Actor* pActor) const
	{
		if (pActor != NULL)
		{
			// you might filter certain actors into specific log, perhaps based on pActor->GetActorTypeId()
		}

		// maybe detect the current level and direct output based on that
	}

	virtual Debug::Logger* GetSorcery(const XtRecord* pRecord) const
	{
		if (pRecord != NULL)
		{
			// you might filter based on pRecord->type, or by searching for which XtPack owns this record, etc.
		}

		// use some generic sorcery data logger
	}

	virtual Debug::Logger* GetAsset(const Util::C_String& filename) const
	{
		// you might filter by directory, extension, naming convention, etc.
	}

};
@endcode

There are some special message types related to debug render. The base 
Actor::C_Actor and Actor::I_ActorComponent classes handle these and call the 
DebugRender function in response:
- GameMessaging::E_GameMsg_DebugRenderSpecific can be broadcast to debug
render only those actors and components that are subscribed to it.
- GameMessaging::E_GameMsg_DebugRenderOn can be sent to an actor or component
to ask it to subscribe to GameMessaging::E_GameMsg_DebugRenderSpecific.
- GameMessaging::E_GameMsg_DebugRenderOff can be sent to an actor or component
to ask it to unsubscribe from GameMessaging::E_GameMsg_DebugRenderSpecific.

With more than a handful of actors and components in the same scene the value
of debug render rapidly diminishes. It's common to use a debug variable for
each actor and component type to enable the debug render for that type, so 
that users can select which types they want to see debug info for.
@code
#ifndef CFG_GOLD
static Debug::Bool g_MyActorDebugRenderEnabled("ActorDebug.Actors.MyActor.DebugRenderEnable", false);

void C_MyActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_MyActorDebugRenderEnabled)
		return;

	// draw stuff
}
#endif // CFG_GOLD
@endcode
*/

/**
@page actorsys_resources Managing Resources with the Actor System.

The resource system supports asynchronous loading of data. Clients own 
Resource::C_Handle objects which reference count Resource::C_Resource objects, 
and implement Resource::C_Listener interfaces that are notified when resource 
data is ready for use.

Actors and components are ideal places to store resource handles and to 
implement resource listeners. The creation data for the actor or component
should specify the resource object to load. In the PrimeImpl phase these
resources can be loaded using handles owned by the actor or component, and
when all resources have notified then the actor or component can call
SetPrimed.

When an actor or component has to manage a large number of resources it might
be desirable to use ResourceUtils::I_ResourceBag to manage the handles and
listener.
*/

/**
@page actor_main Actors.

Actors represent stand-alone entities within the actor system. The main usage
of actors is with ActorStudio, where instances of specific actor types can be
manipulated. Actors are sometimes created at runtime too.
- @subpage actor_new
- @subpage actor_creation
- @subpage actor_composition
- @subpage actor_cookbook
*/

/**
@page actor_new Quick Start Guide to Creating a New Actor Type

So, we've identified a need for a new actor type... 

We need a unique type identifier we can assign to C_ActorTypeId. See 
@ref actorsys_rtti for a detailed explanation, for now we'll use this
placeholder:
@code
#include <misc_shared/uid/FourCC.h>

const Actor::C_ActorTypeId kMyActorTypeId = FOURCC_U32('MYAT');
@endcode

Actors are created through a factory interface using creation data objects. 
See @ref actor_creation for more detail, for now we'll use this
placeholder:
@code
#include <actor_system/ActorCreation.h>

class C_MyActorCreate : public C_Super
{
public:
	C_MyActorCreate();
	virtual ~C_MyActorCreate();

	virtual const Actor::C_ActorTypeId GetType() const {return kMyActorTypeId;} // from Actor::I_ActorCreationData

};
@endcode

Now we can define the actor itself. You won't need to do everything listed
here, see the various subsections for more detail:
@code
#include <actor_system/Actor.h>

class C_MyActor : public C_Super
{
public:
	C_MyActor();
	virtual ~C_MyActor();

	// these are used by the factory Register template function
	static const Actor::C_ActorTypeId TypeId() {return kMyActorTypeId;}
	static const Actor::C_ActorTypeName TypeName() {return "MyActor";}

	// these might be implemented by a game-specific base class or you might do it yourself
	virtual const char* GetActorName() const; // from Actor::C_Actor
	virtual const Maths::C_Vertex GetPosition() const; // from Actor::C_Actor
	virtual const Maths::C_Vector GetDirection(void) const; // from Actor::C_Actor
	virtual const Maths::HMat43& GetMatrix() const; // from Actor::C_Actor

	// messaging
	virtual bool OnMessage(const Messaging::C_MessageBase *pMessage); // from Actor::C_Actor

	// serialisation
	virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const; // from Actor::C_Actor
	virtual void DeserialiseForSave(Sys::C_ContextualStream& stream); // from Actor::C_Actor

	virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const; // from Actor::C_Actor
	virtual void DeserialiseForSync(Sys::C_ContextualStream& stream); // from Actor::C_Actor

#ifndef CFG_GOLD
	// debug
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const; // from Actor::C_Actor
#endif

protected:
	// lifetime
	virtual bool InitialiseImpl(); // from Actor::C_Actor
	virtual bool PrimeImpl(); // from Actor::C_Actor
	virtual bool CreateImpl(); // from Actor::C_Actor
	virtual void KillImpl(); // from Actor::C_Actor

};
@endcode

To use our actor we need to register it with the factory, which takes the
creation data object we defined earlier and uses this to create our actor:
@code
m_pActorFactory->Register<C_MyActor>();
@endcode

That's it for the basic framework, now we just need to fill in the gaps.
*/

/**
@page actor_creation Actor Creation Data

See @ref actorsys_creation for an overview of creation data.

Actor creation data that comes from ActorStudio can contain component creation
data embedded within it. This presents a special problem in that we need to
notify the actor system about this embedded data so that it can identift the
unique IDs within them. The creation data for many actors is itself embedded 
within a single Xt record, which receives a callback when it is loaded and
unloaded. This in turn calls DoSorceryCallback on every actor creation data,
which can be overridden to notify embedded component creation data.
@code
class C_MyActorCreate ... // the class we started in the Quick Start section
{
public:
	// data members that will be annotated for Xt

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn); // from Actor::C_ActorCreationData

};

void C_MyActorCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn) // from Actor::C_ActorCreationData
{
	C_Super::DoSorceryCallback(pCallbackFn);

	// for each data member that contains component creation data we call
	DoComponentSorceryCallback(pCallbackFn, m_ComponentCreationData);
}
@endcode

This class should be Xt annotated and must specify XT_BASE. There is no value
in adding XT_CALLBACK as there should never be records of this type.
@code
XT_TYPE(C_MyActorCreate, "MyActor")
	XT_BASE(C_Super)
	// annotated fields
XT_END_TYPE

XT_REGISTER_TYPE(C_MyActorCreate)
@endcode

Actor creation data built at runtime needs serialisation methods, and a static 
ClassType function to identify it with the factory.
@code
class C_MyActorCreate ... // the class we started in the Quick Start section
{
public:
	static Actor::C_ActorTypeId ClassType() {return kMyActorTypeId;}

	// accessors for member data

	virtual void Serialise(Sys::C_ContextualStream& stream) const; // from Actor::C_ActorCreationData
	virtual void Deserialise(Sys::C_ContextualStream& stream); // from Actor::C_ActorCreationData

private:
	// member data

};
@endcode

We'll need to register this runtime creation data with the factory, which
then allows us to create instances of it.
@code
#include <actor_system/ActorFactory.h>

// register runtime creation data
Actor::C_ActorCreationDataFactory::Register<C_MyActorCreate>(kMyActorTypeId);

// create an instance of the creation data
Actor::C_ActorCreationData::C_DataRef pCreationData;
{
	Actor::C_ActorCreationDataFactoryHelper<C_MyActorCreate> pHelper;
	pHelper->SetSomething();
	pCreationData = pHelper.Get();
}
@endcode
*/

/**
@page actor_composition Building Actors from Components

The actor system can be used with only actors and no components if desired.
Components allow for sharing code between different actor types without 
needing to employ class inheritence. They also allow specific areas of
functionality to be encapsulated away from the rest of the actor, making 
them useful even if only one actor type uses them.

One major limitation of actor composition is that an actor can not have more
than one component with the same interface type. This in turn means that an
actor can not have several instances of the same component type.

It is possible for actors to spawn and remove components on-the-fly if they 
want to, but it is recommended that components are created during the 
InitialiseImpl phase of the actor and automatically destroyed by the actor 
system when the actor is destructed.
@code
bool C_MyActorType::InitialiseImpl()
{
	if (!C_Super::InitialiseImpl())
		return false;

	// the actor system guarantees that we have a valid creation data object of the correct type
	const C_MyActorCreationData* pCreationData = static_cast<const C_MyActorCreationData*>(GetCreationData().GetData());

	// if our creation data was provided by ActorStudio then it might contain component creation data
	if (!InitialiseComponentFromSorcery(&pCreationData->m_SorceryComponent))
		return false;

	// we can also make component creation data at runtime
	{
		Component::C_ComponentCreatorFactoryHelper<C_MyComponentCreationData> pComponentCreate;
		pComponentCreate->SetSomething(pCreationData->GetSomething());
		
		if (!InitialiseComponent(pComponentCreate.Get()))
			return false;
	}

	return true;
}
@endcode

Actors can query information from their components through interfaces. See
@ref component_interfaces for more details.
*/

/**
@page actor_cookbook Cookbook for Common Actor Patterns

- @subpage actor_cookbook_systemproxy
*/

/**
@page actor_cookbook_systemproxy System Proxy Actors

One common pattern we see is where level designers want to send messages in
ActorStudio that will be handled by some game system. The temptation is to
fold the functionality of that game system into the actor that is referenced
in ActorStudio. This leads to some common problems, such as having several
instances of the actor type created by different ActorStudio scenes that are
loaded at the same time.

Using Actor::C_SystemProxyActor as a base class, we can create and actor type 
for use in ActorStudio and have multiple instances of that actor type
referencing the same system object. To start with we need to create the system
and be able to access the instance some how: maybe a singleton object, global
object, member of some encompassing game object (eg. game app, game world, 
etc.) or as part of the actor context.
@code
class C_MySystem
{
public:
	static C_MySystem* GetInstance(); // singleton accessor

	// whatever functionality the system needs

};
@endcode

There may be data that we need to serialise and deserialised along with the
system proxy actors. If we only want to serialise one copy of this data, and
respond to the deserialisation once, regardless of how many proxy actors are
involved then we can provide a serialisation policy.
@code
class C_MySerialisationPolicy
{
public:
	static void SerialiseForSave(Sys::C_ContextualStream& stream, const C_ActorList* pActorList, const C_MySystem* pSystem);
	static void DeserialiseForSave(Sys::C_ContextualStream& stream, const C_ActorList* pActorList, C_MySystem* pSystem);

	static void SerialiseForSync(Sys::C_ContextualStream& stream, const C_ActorList* pActorList, const C_MySystem* pSystem);
	static void DeserialiseForSync(Sys::C_ContextualStream& stream, const C_ActorList* pActorList, C_MySystem* pSystem);

};
@endcode

We might want to perform some actions when the first system proxy actor is 
created, or when the last system proxy actor is destroyed. To do so we need
to provide a lifetime policy.
@code
class C_MyLifetimePolicy
{
public:
	static void OnFirstProxyConstructed(const C_ActorList* pActorList, C_MySystem* pSystem);
	static void OnLastProxyDestroyed(const C_ActorList* pActorList, C_MySystem* pSystem);

};
@endcode

Now we can make our system proxy actor. We shouldn't be using many of the
standard actor hooks, such as lifetime and serialisation, as this is handled
by the policies we provide. The main thing we're going to want to do is to
provide a OnMessage function for messages sent from ActorStudio.
@code
class C_MySystemProxyActor : public Actor::C_SystemProxyActor<C_MySystem, C_MySerialisationPolicy, C_MyLifetimePolicy>
{
public:
	static const Actor::C_ActorTypeId TypeId() {return kMySystemProxyActorType;}
	static const Actor::C_ActorTypeName TypeName() {return "MySystem";}
	
	virtual bool OnMessage(const Messaging::C_MessageBase* pMessage);

};
@endcode

The set of all instances of system proxy actors for the same system object in
the same actor list is represented by Actor::C_SystemProxyGroup. These are
reference counted objects that can be used to send messages to all such actors.
@code
// obtain a smart pointer to system proxy group for pMySystem in pActorList
Actor::C_SystemProxyGroup<MySystem>::C_Ptr pMySystemProxyGroup = Actor::C_SystemProxyGroup<MySystem>::Acquire(pActorList, pMySystem);

// send a message to all actors in this group
const C_MyMessageType msg(someData);
pMySystemProxyGroup->SendMessageToProxies(&msg);
@endcode
*/

/**
@page component_main Components

Components never exist in isolation, they are only ever found within actors.
- @subpage component_new
- @subpage component_creation
- @subpage component_interfaces
- @subpage component_messaging
*/

/**
@page component_new Components Quick Start Guide

To add a new component we need an interface type and a concrete type, with
type identifiers both both, and a creation data type.

Our interface defines how the rest of the code will access the component type:
@code
const Component::C_InterfaceTypeId kMyComponentInterfaceId = FOURCC_U32('IMCP');

class I_MyComponent : public C_Super
{
public:
	static Component::C_ComponentInterfaceId InterfaceType() {return kMyComponentInterfaceId;}
	virtual Component::C_ComponentInterfaceId GetInterfaceID() const {return kMyComponentInterfaceId;} // from Component::I_GameComponent
	
	// interface methods etc.

};
@endcode

Now we need a creation data object that we can use to instantiate the concrete
component:
@code
const Component::C_ComponentTypeId kMyComponentTypeId = FOURCC_U32('MYCP');

class C_MyComponentCreate : public C_Super
{
public:
	static Component::C_ComponentTypeId ClassType() {return kMyComponentTypeId;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return kMyComponentTypeId;} // from Component::I_ComponentCreator
	
	// data used to configure the component etc.

};
@endcode

Our concrete class looks a little like this:
@code
class C_MyComponent : public I_MyComponent
{
public:
	C_MyComponent();
	virtual ~C_MyComponent();

	static Component::C_ComponentTypeId ClassType() {return kMyComponentTypeId;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return kMyComponentTypeId;} // from Component::I_GameComponent

	// serialisation
	virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const; // from Component::I_GameComponent
	virtual void DeserialiseForSave(Sys::C_ContextualStream& stream); // from Component::I_GameComponent

	virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const; // from Component::I_GameComponent
	virtual void DeserialiseForSync(Sys::C_ContextualStream& stream); // from Component::I_GameComponent

	// messaging
	virtual bool OnMessage(const Messaging::C_MessageBase* pMessage); // from Actor::I_ActorComponent

#ifndef CFG_GOLD
	// debug
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const; // from Component::I_GameComponent
#endif

protected:
	// lifetime
	virtual bool InitialiseImpl(); // from Component::I_GameComponent
	virtual bool PrimeImpl(); // from Component::I_GameComponent
	virtual bool CreateImpl(); // from Component::I_GameComponent
	virtual void KillImpl(); // from Component::I_GameComponent

};
@endcode

We then need to register the concrete class with the component factory, so
that the creation data type can be used:
@code
Component::C_GameComponentFactory::GetInstance()->Register<C_MyComponent>();
@endcode
*/

/**
@page component_creation Component Creation Data

See @ref actorsys_creation and @ref actor_creation.

As with actor creation a given component type may be created either from 
ActorStudio or at runtime, but not both. The former is a little simpler than 
actor creation data.
@code
class C_MyComponentCreate ... // our class from the quick start section
{
public:
	// data members that will be annotated for Xt

};

XT_TYPE(C_MyComponentCreate, "MyComponent")
	XT_BASE(C_Super)
	// annotate data members
XT_END_TYPE

XT_REGISTER_TYPE(C_MyComponentCreate)
@endcode

Recall that component creation data originating from ActorStudio must be
registered by the actor creation data the contains it. Every actor creation
data type that contains these components must implement DoSorceryCallback.

Runtime components need to implement serialisation functions and be 
registered with the factory.
@code
class C_MyComponentCreate ... // our class from the quick start section
{
public:
	static Actor::C_ActorTypeId ClassType() {return kMyActorTypeId;}

	// accessors for member data

	virtual void Serialise(Sys::C_ContextualStream& stream) const; // from Actor::C_ActorCreationData
	virtual void Deserialise(Sys::C_ContextualStream& stream); // from Actor::C_ActorCreationData

private:
	// member data

};

Component::C_ComponentCreatorFactory::Register<C_MyComponentCreate>();

Component::I_ComponentCreator::C_DataRef pCreationData;
{
	Component::C_ComponentCreatorFactoryHelper<C_MyComponentCreate> pHelper;
	pHelper->SetSomething();
	pCreationData = pHelper.Get();
}
@endcode
*/

/**
@page component_interfaces Component Interfaces

The only code that should interact with concrete component types is the 
registration of those types with the component factory. Other code can use 
the corresponding creation data type(s), and can interact with the component
interface type exposed by the component.

Interfaces are identified by Component::C_ComponentInterfaceId. This can be
used to determine if an actor has a component that implements an interface, 
and to send a message to that component.
@code
bool C_MyActorType::HasSomeComponentInterface() const
{
	return QueryInterface(kMyComponentInterfaceId) != NULL;
}

void C_MyActorType::SetSomething(const U32 something)
{
	const C_SetSomethingMsg msg(something);
	SendMessageToComponent(kMyComponentInterfaceId, &msg);
}
@endcode

An interface can also provide an abstract class that the concrete component 
derives from, so users can cast an actor component to this interface class.
@code
U32 C_MyActorType::GetSomething() const
{
	const I_MyComponentInterface* pComponent = QueryInterface<I_MyComponentInterface>();

	if (pComponent != NULL)
		return pComponent->GetSomething();

	return m_DefaultSomething;
}
@endcode
*/

/**
@page component_messaging Component Message Handling

Just like actors, components may receive messages either by subscribing to a
broadcast or by messaging being sent directly. A message sent to an actor may
be addressed to a specific component (by interface type), in which case only
that component will receive the message. Otherwise all components, along with 
the actor itself, will receive the message. A message that the actor doesn't
handle itself must be handled by at least one component.

If an actor contains many components and each component has the opportunity to
handle every message sent to the actor, this could add up to a large overhead.
The temptation is to send messages directly to component interfaces inside an
actor, but this couples the message sending code to the implementation of the
actor in terms of components, making future code changes more difficult. An
alternative solution is to specify which message types the actor should forward
to each component. This can be set up for each component type, and is inherited
(and can be extended) by component types derived from these. There is a suit of
macros to help with this, see ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE.
*/

/**
@namespace ActorSys
@brief Common types and functionality shared by Actor and Component code.
See @ref actorsys_main.
*/

/**
@namespace Actor
@brief Framework for game entities.

To use the actor system you will need to create at least one C_ActorList 
object. This owns all actor data, which is creates with C_ActorFactory:
@code
Sys::I_ContextProvider* pActorContextProvider = new C_MyGameActorContextProvider();

Actor::C_ActorFactory* pActorFactory = new Actor::C_ActorFactory(kMaxActorTypes);
m_pActorFactory->Register<C_MyActor>();

Actor::C_ActorList* pActorList = new Actor::C_ActorList(kMaxActors, kMaxActorGroups, kNumMessageQueues, pActorFactory, pActorContextProvider);
@endcode

Using a smart pointer to C_ActorCreationData, which can be provided by Xt
data or from a factory using C_ActorCreationDataFactoryHelper, the actor list
can create actors:
@code
Actor::C_ActorCreationDataFactoryHelper<C_MyActor> pCreationData;
pCreationData->SetWibble(2);

Actor::C_ActorHandle actorHandle = pActorList->Spawn(pCreationData.Get());
@endcode

We can obtain a pointer to an actor if we need to query something:
@code
const Actor::C_Actor* pActor = pActorList->GetActorPtr(actorHandle);
if (pActor != NULL)
{
	const C_MyActor* pMyActor = Actor::ActorCast<C_MyActor>(pActor);
	U32 wibble = pMyActor->GetWibble();
}
@endcode

We can send a message to an actor if we need to change something:
@code
C_MyMsg message(4);
pActorList->SendMessageToActor(actorHandle, &message);
@endcode
*/

/**
@namespace Component
@brief Framework for reusable components.

Components are created using I_ComponentCreator objects provided by Xt or
created at runtime using C_ComponentCreatorFactoryHelper, which are used
with C_GameComponentFactory.

We can ask an actor for an component by interface:
@code
const I_MyComponent* pComponent = pActor->QueryInterface<I_MyComponent>();
@endcode

We can send a message to a specific component:
@code
C_MyMsg message(4);
pActorList->SendMessageToActorComponent(actorHandle, &message, I_MyComponent::kClassUid);
@endcode
*/

/**
@defgroup actor_group_main Actor System Core

The core of the actor system consists of a list of actor objects which are
built from creation data.

Clients define C_ActorCreationData objects, which may be supplied as static
data from Xt or may be created at runtime. These are passed to a C_ActorList
object which created C_Actor objects using the C_ActorTypeId specified. The
created actor is accessed through C_ActorHandle.
*/

/**
@defgroup actor_group_factory Actor and Component Factories

Actors are created by a C_ActorFactory object that is configured to create
types based on C_ActorTypeId values. For C_ActorCreationData created at 
runtime there is also a C_ActorCreationDataFactory that interprets the same
type information, and a C_ActorCreationDataFactoryHelper utility that reduces
the amount of code needed for the creation process.

Similarly, components are created by the C_GameComponentFactory based on
C_ComponentTypeId values, and I_ComponentCreator objects can be created
dynamically using C_ComponentCreatorFactory assisted by the 
C_ComponentCreatorFactoryHelper utility.
*/

/**
@defgroup actor_group_component Actor Components

Comoponents are created by actors to provide reusable units of functionality.
An I_ComponentCreator object is used by the actor to create an I_ActorComponent
object of the corresponding C_ComponentTypeId. These components are accessed
via an interface specified by C_ComponentInterfaceId.
*/

/**
@defgroup actor_group_messages Actor Messages

The actor system uses the Messaging system for communication, with C_ActorList
providing a subscriber system and both C_Actor and I_GameComponent acting as
message handlers.

There are a number of system-level messages defined by E_ActorMsg which are
used by the actor system.

When implementing new message types that contain actor-related data, the 
C_MessageActorHandle and C_MessageActorValue utilities can be used to automate
some coding.
*/

/**
@defgroup actor_group_serialisation Actor Serialisation

Actor and components objects support serialisation and deserialisation, which
is used for persisting or synchronising state. Each type of actor and
component should supply appropriate implementations.

Where a C_ActorHandle is used to refer to an actor, this is serialised using
a implementation of I_ActorHandleSerialisation appropriate to the context of
the serialisation.

Depending on the reason for serialisation, there may be certain data which can
be omitted for efficiency. This is controlled by C_SerialisationConfig.
*/

/**
@defgroup actor_group_utility Actor Utilities

Miscellaneous stuff used with the actor system.
*/
