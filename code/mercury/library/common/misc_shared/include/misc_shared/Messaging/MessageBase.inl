#ifndef MESSAGEBASE_INL
#error MessageBase.inl should only be included from MessageBase.h
#endif // MESSAGEBASE_INL

#include <misc_shared/messaging/MessageDebug.h>


namespace Messaging
{

/**
@param messageType type identifier for this object, should unqiuely identify 
the class type.
@param messageSize size in bytes of this object.
*/
inline C_MessageBase::C_MessageBase(TMsgType messageType, U32 messageSize) : 
	m_Type(messageType), m_Size(messageSize)
{
#ifndef CFG_GOLD
	m_pDebugInfo = FindDebugInfo(messageType);

	if (m_pDebugInfo == NULL)
	{
		MissingDebugInfo(messageType);
	}
#endif
}

inline C_MessageBase::C_MessageBase(const C_MessageBase& copy) :
	m_Type(copy.m_Type), m_Size(copy.m_Size)
{
#ifndef CFG_GOLD
	m_pDebugInfo = copy.m_pDebugInfo;
#endif
}

inline C_MessageBase::~C_MessageBase()
{
	// intentionally blank
}

inline C_MessageBase& C_MessageBase::operator = (const C_MessageBase& copy)
{
	m_Type = copy.m_Type;
	m_Size = copy.m_Size;
#ifndef CFG_GOLD
	m_pDebugInfo = copy.m_pDebugInfo;
#endif
	return *this;
}




/**
Downcasts a base message to the specified derived class, asserting that the 
cast is legitimate using manual RTTI.
@param pMessage base message to cast, must not be NULL.
@returns pointer to dervied message class object.
*/
template <class T>
inline const T* MessageCast_Assert(const C_MessageBase* pMessage)
{
//	assert(T::StaticType() == pMessage->Type());
//	assert(sizeof(T) == pMessage->Size());
	return static_cast<const T*>(pMessage);
}

/**
Downcasts a base message to the specified derived class, using manual RTTI to
check if the cast is legitimate.
@param pMessage base message to cast.
@returns pointer to dervied message class object, or NULL if cast is not
supported.
*/
template <class T>
inline const T* MessageCast_Safe(const C_MessageBase* pMessage)
{
	if((pMessage != NULL) && (T::StaticType() == pMessage->Type()) && (sizeof(T) == pMessage->Size()))
		return static_cast<const T*>(pMessage);
	else
		return NULL;
}

/**
Downcasts a base message to the specified derived class, user is entirely
responsible for checking that the cast is legitimate.
@param pMessage base message to cast.
@returns pointer to dervied message class object.
*/
template <class T>
inline const T* MessageCast_Reckless(const C_MessageBase* pMessage)
{
	return static_cast<const T*>(pMessage);
}

/**
Downcasts a base message to the specified derived class.
@param pMessage base message to cast, must not be NULL.
@returns pointer to dervied message class object.
*/
template <class T> 
inline const T* MessageCast(const C_MessageBase* pMessage)
{
	return MessageCast_Assert<T>(pMessage);
}





/**
Simple message types carry no additional data, it is the responsibility of the
user to ensure that complex message type identifiers are not supplied to this
function.
Using this helper function it is easy to add new message types without needing
to implement new message classes.
@code
class MyMessageHandler : public Messaging::C_MessageHandler
{
	virtual bool MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		switch (pMessage->Type())
		{
		case E_MyMessageTypes_SimpleMessage:
			return RespondToSimpleMessage();

		default:
			return false;
		}
	}
};

MyMessageHandler messageHandler;
messageHandler.MessageHandler(&Messaging::BuildSimpleMessage(E_MyMessageTypes_SimpleMessage));
@endcode
@param messageType type identifier for simple message.
@returns message object.
*/
inline C_MessageBase BuildSimpleMessage(const TMsgType messageType)
{
	return C_MessageBase(messageType, sizeof(C_MessageBase));
}

} // namespace Messaging
