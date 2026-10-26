#include "MiscSharedPch.h"

#include <misc_shared/Messaging/MessageSerialisation.h>

#include <misc_shared/Context/ContextualStream.h>
#include <misc_shared/Messaging/MessageBase.h>
#include <system/debug/log/DebugLog.h>


namespace Messaging
{

I_MessageAllocator::I_MessageAllocator()
{
	// intentionally blank
}

I_MessageAllocator::~I_MessageAllocator()
{
	// intentionally blank
}

/**
@param messageType type of message.
@param messageSize size of message.
@returns base class pointer to message object (extra data in derived class will not be initialised).
*/
C_MessageBase* I_MessageAllocator::Create(const TMsgType messageType, const U32 messageSize)
{
	return new (Allocate(messageSize)) C_MessageBase(messageType, messageSize);
}

/**
Performs a memcpy of data.
@param pMessage message to copy.
@returns newly allocated copy of message.
*/
C_MessageBase* I_MessageAllocator::Copy(const C_MessageBase* pMessage)
{
	C_MessageBase* pCopy = NULL;

	if (pMessage != NULL)
	{
		pCopy = reinterpret_cast<C_MessageBase*>(Allocate(pMessage->Size()));
		memcpy(pCopy, pMessage, pMessage->Size());
	}

	return pCopy;
}

/**
@param pMessage message objects supplied by Create or Copy.
*/
void I_MessageAllocator::Destroy(C_MessageBase* pMessage)
{
	if (pMessage != NULL)
	{
		pMessage->~C_MessageBase();
		Free(pMessage);
	}
}





C_DefaultMessageAllocator::C_DefaultMessageAllocator() : 
	I_MessageAllocator()
{
	// intentionally blank
}

C_DefaultMessageAllocator::~C_DefaultMessageAllocator()
{
	// intentionally blank
}

void* C_DefaultMessageAllocator::Allocate(const U32 numBytes)
{
	return new U8 [numBytes];
}

void C_DefaultMessageAllocator::Free(void* pMem)
{
	delete [] reinterpret_cast<U8*>(pMem);
}




C_MessageSerialiser* C_MessageSerialiser::s_HashTable [C_MessageSerialiser::kHashTableSize] = {NULL};

C_MessageSerialiser::C_MessageSerialiser(const TMsgType messageType, F_Serialise pSerialise, F_Deserialise pDeserialise)
	: m_MessageType(messageType)
	, m_pSerialise(pSerialise)
	, m_pDeserialise(pDeserialise)
	, m_pHashTableNext(NULL)
{
	const U32 hash = CalcHash(messageType);
	BIZ_ASSERT(hash < kHashTableSize);

#ifndef CFG_SPEED
	for (const C_MessageSerialiser* pItem = s_HashTable[hash]; pItem != NULL; pItem = pItem->m_pHashTableNext)
	{
		if (pItem->m_MessageType == messageType)
		{
			BIZ_ASSERT_FAIL("Duplicate serialisation for message type %d", messageType);
		}
	}
#endif

	m_pHashTableNext = s_HashTable[hash];
	s_HashTable[hash] = this;
}

C_MessageSerialiser::~C_MessageSerialiser()
{
	const U32 hash = CalcHash(m_MessageType);
	BIZ_ASSERT(hash < kHashTableSize);

	for (C_MessageSerialiser** ppItem = &s_HashTable[hash]; *ppItem != NULL; ppItem = &(*ppItem)->m_pHashTableNext)
	{
		if (*ppItem == this)
		{
			*ppItem = m_pHashTableNext;
			break;
		}
	}
}

/**
@param stream stream to receive the message data.
@param pMessage message data to serialise.
*/
void C_MessageSerialiser::Serialise(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	// write the type and size, followed by the data if necessary
	TMsgType messageType = -1;
	U32 messageSize = 0;
	F_Serialise pSerialise = NULL;

	if (pMessage != NULL)
	{
		messageType = pMessage->Type();
		messageSize = pMessage->Size();

		// lookup the serialisation function if the message contains data
		if (messageSize > sizeof(C_MessageBase))
		{
			const U32 hash = CalcHash(messageType);
			BIZ_ASSERT(hash < kHashTableSize);

			for (const C_MessageSerialiser* pItem = s_HashTable[hash]; pItem != NULL; pItem = pItem->m_pHashTableNext)
			{
				if (pItem->m_MessageType == messageType)
				{
					pSerialise = pItem->m_pSerialise;
					break;
				}
			}

#ifndef CFG_GOLD
			if (pSerialise == NULL)
			{
				LOGERROR(Messaging::GetDefaultLog(), "Don't know how to serialise message type %s (0x%8x)", pMessage->GetDebugName(), messageType);
			}
#endif
		}
	}

	// write stuff
	stream << messageType;
	stream << messageSize;

	if (pSerialise != NULL)
		pSerialise(stream, pMessage);
}

/**
@param stream stream to provide the message data.
@param allocator allocator to create the message object.
@returns newly created message object.
*/
C_MessageBase* C_MessageSerialiser::Deserialise(Sys::C_ContextualStream& stream, I_MessageAllocator& allocator)
{
	C_MessageBase* pMessage = NULL;
	{
		// read the type and size, followed by the data if necessary
		TMsgType messageType = -1;
		U32 messageSize = 0;
		F_Deserialise pDeserialise = NULL;

		stream >> messageType;
		stream >> messageSize;
		
		// find the function to read the data if necessary
		if (messageSize > sizeof(C_MessageBase))
		{
			const U32 hash = CalcHash(messageType);
			BIZ_ASSERT(hash < kHashTableSize);

			for (const C_MessageSerialiser* pItem = s_HashTable[hash]; pItem != NULL; pItem = pItem->m_pHashTableNext)
			{
				if (pItem->m_MessageType == messageType)
				{
					pDeserialise = pItem->m_pDeserialise;
					break;
				}
			}

#ifndef CFG_SPEED
			if (pDeserialise == NULL)
			{
				LOGERROR(Messaging::GetDefaultLog(), "Don't know how to deserialise message type %s (0x%8x)", Messaging::FindDebugName(messageType), messageType);
			}
#endif
		}

		// create the message
		if (messageSize > 0)
		{
			pMessage = allocator.Create(messageType, messageSize);

			if (pDeserialise != NULL)
				pDeserialise(stream, pMessage);
		}
	}

	return pMessage;
}

} // namespace Messaging
