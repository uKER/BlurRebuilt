#include "MiscSharedPch.h"

#include "misc_shared/Context/ContextualStream.h"


namespace Sys
{

/**
@param binaryStream stream object that will be used.
@param hintNumContexts expected number of context items that will be added.
*/
C_ContextualStream::C_ContextualStream(Sys::I_BinaryStream& binaryStream, const U32 hintNumContexts) : 
	m_Flags(0), 
	m_BinaryStream(binaryStream) 
{
	m_ContextItems.Reserve(hintNumContexts);
}

C_ContextualStream::~C_ContextualStream()
{
}

/**
@param interfaceId context interface to search for.
@returns NULL if the interface is not supported.
*/
Sys::I_Anything* C_ContextualStream::GetContextItem(const Sys::C_AnythingUid interfaceId) const
{
	// look for a matching interface
	for (C_ContextItems::ConstIterator it = m_ContextItems.Begin(); it != m_ContextItems.End(); ++it)
		if (it->first == interfaceId)
			return it->second;

	// this interface is not supported
	return NULL;
}

/**
@param interfaceId context interface being implemented.
@param pContextItem context object implementing this interface.
@returns false if the context interface is already supported.
*/
bool C_ContextualStream::AddContextItem(const Sys::C_AnythingUid interfaceId, Sys::I_Anything* pContextItem)
{
	// must be a valid implementation of an interface we don't already support
	if ((pContextItem == NULL) || !pContextItem->IsTypeOf(interfaceId) || (GetContextItem(interfaceId) != NULL))
		return false;
	
	// add the context
	m_ContextItems.PushBack(C_ContextItem(interfaceId, pContextItem));
	return true;
}

/**
@param interfaceId interface to remove context for.
@returns true if the context was removed.
*/
bool C_ContextualStream::RemoveContextItem(const Sys::C_AnythingUid interfaceId)
{
	// look for a matching interface
	for (C_ContextItems::Iterator it = m_ContextItems.Begin(); it != m_ContextItems.End(); ++it)
	{
		if (it->first == interfaceId)
		{
			*it = m_ContextItems.Back();
			m_ContextItems.PopBack();
			return true;
		}
	}

	return false;
}

C_ContextualStreamPos C_ContextualStream::GetStreamPos() const
{
	return C_ContextualStreamPos(m_BinaryStream);
}

} // namespace Sys
