#ifndef MESSAGEBASE_H
#define MESSAGEBASE_H

#ifdef _MSC_VER
#pragma once
#endif


#include <misc_shared/Messaging/MessagingDeclare.h>


namespace Messaging
{

/**
@brief All messages are derived from this base type.

Contains just a type identifier and a size, acting as a header allowing
contents to be copied around as raw memory.

All messages must derive directly from this base class, with the following 
restrictions (the C_MessageDerived templated class can help with some of
these):
- a static method "TMsgType StaticType()" must be provided for compatibility
with the MessageCast() helper function.
- the ctor must specifiy a unqiue type identifier and the size of the object.
- data members must be plain-old-data with a maximum alignment of 4 bytes, no
complex ctor/dtor stuff that's won't play nicely with memcpy.
- messages objects may be stored for a number of frames before being 
dispatched, so should not store pointers/references to memory that might not
be around.

In development builds (CFG_GOLD not defined) the game should register debug
information for every message type that it will use, with the 
RegisterMessageDebugInfo() method.
*/
class C_MessageBase
{
public:
	inline C_MessageBase(TMsgType messageType, U32 messageSize);
	inline C_MessageBase(const C_MessageBase& copy);
	inline ~C_MessageBase();
	inline C_MessageBase& operator = (const C_MessageBase& copy);

	TMsgType					Type() const {return m_Type;} ///< @returns the type identifier of this message object
	U32							Size() const {return m_Size;} ///< @returns the size in bytes of this message object

	static TMsgType				StaticType() {return -1;}

#ifndef CFG_GOLD
	const C_MessageDebugInfo*	GetDebugInfo() const {return m_pDebugInfo;} ///< @returns debug info if type was registered, NULL otherwise.
	const char*					GetDebugName() const; ///< @returns debug name for this message type.
#endif

private:
	TMsgType					m_Type;			///< message type identifier
	U32							m_Size;			///< message size in bytes
#ifndef CFG_GOLD
	const C_MessageDebugInfo*	m_pDebugInfo;	///< cached debug info for this message object (can be inspected in the debugger)
#endif

};


/**
@brief Helper class for deriving from C_MessageBase.
When deriving from C_MessageBase, a message class is required to provide a 
StaticType() method and to supply the base class constructor with a type
identifier and the object size:
@code
class MyMessage : public Messaging::C_MessageBase
{
public:
	static Messaging::TMsgType StaticType() {return E_MyGameMessageTypes_MyMessage;}
	MyMessage() : Messaging::C_MessageBase(E_MyGameMessageTypes_MyMessage, sizeof(MyMessage)) {}
};
@endcode
If the type identifier is a compile-time constant then these two tasks can be 
automated by deriving from the C_MessageDerived class:
@code
class MyMessage : public Messaging::C_MessageDerived<MyMessage, E_MyGameMessageTypes_MyMessage> {};
@endcode
@param MessageClass class implementing the message type.
@param TypeID message type identifier.
@param BaseClass base message class must supply a ctor that takes 
(TMsgType, S32), normally C_MessageBase.
*/
template <class MessageClass, TMsgType TypeID, class BaseClass = C_MessageBase>
class C_MessageDerived : public BaseClass
{
public:
	static TMsgType StaticType() {return TypeID;} ///< @returns type identifier for this message class.

protected:
	C_MessageDerived() : BaseClass(TypeID, sizeof(MessageClass)) {}

};


/// Constructs a simple message object.
inline static C_MessageBase BuildSimpleMessage(const TMsgType messageType);


// downcasting functions
template <class T> inline const T* MessageCast_Assert(const C_MessageBase* pMessage);
template <class T> inline const T* MessageCast_Safe(const C_MessageBase* pMessage);
template <class T> inline const T* MessageCast_Reckless(const C_MessageBase* pMessage);
template <class T> inline const T* MessageCast(const C_MessageBase* pMessage);

} // namespace Messaging


#define MESSAGEBASE_INL
#include "MessageBase.inl"
#undef MESSAGEBASE_INL

#endif // MESSAGEBASE_H
