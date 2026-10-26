#include "MiscSharedPch.h"
#include <misc_shared/messaging/MessageTypeList.h>

#include <algorithm>

#include <core/containers/genesis_containers/Sort.h>


namespace Messaging
{

I_MessageTypeVisitor::~I_MessageTypeVisitor()
{
}




/**
@param hintCapacity reserves memory for this many results.
@param pExcluded if non-NULL specifies message types to ignore.
@returns .
*/
C_MessageTypeListAccumulator::C_MessageTypeListAccumulator(const U32 hintCapacity)
	: I_MessageTypeVisitor()
	, m_Results()
{
	if (hintCapacity > 0)
	{
		m_Results.Reserve(hintCapacity);
	}
}

C_MessageTypeListAccumulator::~C_MessageTypeListAccumulator()
{
}

bool C_MessageTypeListAccumulator::VisitMessageType(const C_MessageTypeRange& range)
{
	Insert(m_Results, range);
	return true;
}




I_MessageTypeList::~I_MessageTypeList()
{
}




/**
@param pMessageTypes must not be NULL, array of messages types terminated by kTerminator.
@param pParent may be NULL, we inherit all message types in this list.
*/
C_MessageTypeListFixed::C_MessageTypeListFixed(const C_MessageTypeRange* pMessageRanges, const I_MessageTypeList* pParent)
	: I_MessageTypeList()
	, m_pMessageRanges(pMessageRanges)
	, m_pParent(pParent)
{
	BIZ_ASSERT(m_pMessageRanges != NULL);
}

/**
@param copy object to copy.
*/
C_MessageTypeListFixed::C_MessageTypeListFixed(const C_MessageTypeListFixed& copy)
	: I_MessageTypeList(copy)
	, m_pMessageRanges(copy.m_pMessageRanges)
	, m_pParent(copy.m_pParent)
{
}

C_MessageTypeListFixed::~C_MessageTypeListFixed()
{
}

/**
@param copy object to copy.
@returns this.
*/
C_MessageTypeListFixed& C_MessageTypeListFixed::operator = (const C_MessageTypeListFixed& copy)
{
	I_MessageTypeList::operator = (copy);
	m_pMessageRanges = copy.m_pMessageRanges;
	m_pParent = copy.m_pParent;
	return *this;
}

U32 C_MessageTypeListFixed::CalcNumRanges() const
{
	U32 count = 0;

	for (const C_MessageTypeRange* pRange = m_pMessageRanges; Validate(*pRange); ++pRange)
	{
		++count;
	}

	return (m_pParent != NULL) ? count + m_pParent->CalcNumRanges() : count;
}

U32 C_MessageTypeListFixed::CalcNumTypes() const
{
	U32 count = 0;

	for (const C_MessageTypeRange* pRange = m_pMessageRanges; Validate(*pRange); ++pRange)
	{
		count += GetSize(*pRange);
	}

	return (m_pParent != NULL) ? count + m_pParent->CalcNumTypes() : count;
}

/**
@param msgType message type to find.
@returns true if this message type is in the list.
*/
bool C_MessageTypeListFixed::Find(const TMsgType msgType) const
{
	for (const C_MessageTypeRange* pRange = m_pMessageRanges; Validate(*pRange); ++pRange)
	{
		if (Contains(*pRange, msgType))
		{
			return true;
		}
	}

	return (m_pParent != NULL) ? m_pParent->Find(msgType) : false;
}

bool C_MessageTypeListFixed::Visit(I_MessageTypeVisitor& visitor) const
{
	for (const C_MessageTypeRange* pRange = m_pMessageRanges; Validate(*pRange); ++pRange)
	{
		if (!visitor.VisitMessageType(*pRange))
		{
			return false;
		}
	}

	return (m_pParent != NULL) ? m_pParent->Visit(visitor) : true;
}




C_MessageTypeListVariable::C_MessageTypeListVariable()
	: I_MessageTypeList()
	, m_MessageTypeRanges()
{
}

/**
@param copy object to copy.
*/
C_MessageTypeListVariable::C_MessageTypeListVariable(const C_MessageTypeListVariable& copy)
	: I_MessageTypeList(copy)
	, m_MessageTypeRanges(copy.m_MessageTypeRanges)
{
}

C_MessageTypeListVariable::~C_MessageTypeListVariable()
{
}

/**
@param copy object to copy.
@returns this.
*/
C_MessageTypeListVariable& C_MessageTypeListVariable::operator = (const C_MessageTypeListVariable& copy)
{
	I_MessageTypeList::operator = (copy);
	m_MessageTypeRanges = copy.m_MessageTypeRanges;
	return *this;
}

/**
@param msgType message type to add.
*/
void C_MessageTypeListVariable::Insert(const TMsgType msgType)
{
	Messaging::Insert(m_MessageTypeRanges, msgType);
}

/**
@param list list of message types to add.
*/
void C_MessageTypeListVariable::Insert(const I_MessageTypeList& list)
{
	// get new message types
	C_MessageTypeListAccumulator visitor(list.CalcNumRanges());
	list.Visit(visitor);

	// merge into our message types
	Messaging::Insert(m_MessageTypeRanges, visitor.GetResults());
}

U32 C_MessageTypeListVariable::CalcNumRanges() const
{
	return m_MessageTypeRanges.Size();
}

U32 C_MessageTypeListVariable::CalcNumTypes() const
{
	return Messaging::CalcSize(m_MessageTypeRanges);
}

bool C_MessageTypeListVariable::Find(const TMsgType msgType) const
{
	return Contains(m_MessageTypeRanges, msgType);
}

bool C_MessageTypeListVariable::Visit(I_MessageTypeVisitor& visitor) const
{
	for (C_MessageTypeRanges::ConstIterator it = m_MessageTypeRanges.Begin(); it != m_MessageTypeRanges.End(); ++it)
	{
		if (!visitor.VisitMessageType(*it))
		{
			return false;
		}
	}

	return true;
}

} // namespace Messaging
