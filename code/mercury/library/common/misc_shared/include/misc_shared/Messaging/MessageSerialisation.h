#ifndef MESSAGESERIALISATION_8BA2562F_51BE_4670_8B1F_9DED78AADEC8_H
#define MESSAGESERIALISATION_8BA2562F_51BE_4670_8B1F_9DED78AADEC8_H

#ifdef _MSC_VER
#pragma once
#endif


#include <misc_shared/Messaging/MessagingDeclare.h>


namespace Sys 
{ 
	class C_ContextualStream; 
}


namespace Messaging
{

/**
@brief Interface for message allocation.
Derived classes should implement Allocate and Free.
*/
class I_MessageAllocator
{
public:
	I_MessageAllocator();
	virtual ~I_MessageAllocator();

	C_MessageBase* Create(const TMsgType messageType, const U32 messageSize); ///< Creates a message object.
	C_MessageBase* Copy(const C_MessageBase* pMessage); ///< Copies a message object.
	void Destroy(C_MessageBase* pMessage); ///< Destroys a message object.

private:
	I_MessageAllocator(const I_MessageAllocator&); // can not copy
	I_MessageAllocator& operator = (const I_MessageAllocator&); // can not copy

	/// @brief Allocates storage for a message object.
	/// @param numBytes size of the message in bytes.
	/// @returns numBytes of memory allocated on a 4-byte boundary.
	virtual void* Allocate(const U32 numBytes) = 0;

	/// @brief Frees storage for a message object.
	/// @param pMem memory supplied by Allocate.
	virtual void Free(void* pMem) = 0;

};

/**
@brief Default implementation of I_MessageAllocator.
*/
class C_DefaultMessageAllocator : public I_MessageAllocator
{
public:
	C_DefaultMessageAllocator();
	virtual ~C_DefaultMessageAllocator();

private:
	virtual void* Allocate(const U32 numBytes); // from I_MessageAllocator
	virtual void Free(void* pMem); // from I_MessageAllocator

};

/**
@brief Handles serialisation of messages.
Instantiating an object of this type will add itself to the shared list of
serialisable message types. Clients can then use Serialise and Deserialise
with message data of the corresponding type.
*/
class C_MessageSerialiser
{
public:
	/// Function that serialises message data.
	/// @param stream stream to receive message data.
	/// @param pMessage message object providing data.
	typedef void (*F_Serialise)(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage); 

	/// Function that deserialises message data.
	/// @param stream stream to provide message data.
	/// @param pMessage message object receiving data.
	typedef void (*F_Deserialise)(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

	C_MessageSerialiser(const TMsgType messageType, F_Serialise pSerialise, F_Deserialise pDeserialise);
	~C_MessageSerialiser();

	inline TMsgType GetMessageType() const {return m_MessageType;}

	/// Helper for force instantiation of template members.
	inline void DummyForceLinkage() const {}

	static void Serialise(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage); ///< Serialises the message.
	static C_MessageBase* Deserialise(Sys::C_ContextualStream& stream, I_MessageAllocator& allocator); ///< Deserialises the message.

private:
	TMsgType				m_MessageType;		///< type of message object.
	F_Serialise				m_pSerialise;		///< function the serialises message data.
	F_Deserialise			m_pDeserialise;		///< function that deserialises message data.
	C_MessageSerialiser*	m_pHashTableNext;	///< next item in the same hash bucket

	static const U32 kHashTableSize = 256; ///< number of buckets in hash table
	static const U32 kHashTableMask = 0xff; ///< mask for hash keys
	static C_MessageSerialiser* s_HashTable [kHashTableSize]; ///< hash table buckets

	inline static U32 CalcHash(const TMsgType msgType) {return (((msgType ^ (msgType >> 8)) ^ (msgType >> 16)) ^ (msgType >> 24)) & kHashTableMask;}

};

} // namespace Messaging

#endif // MESSAGESERIALISATION_8BA2562F_51BE_4670_8B1F_9DED78AADEC8_H
