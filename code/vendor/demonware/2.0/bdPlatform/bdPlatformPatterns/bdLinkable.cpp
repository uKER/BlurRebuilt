// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformPatterns/bdLinkable.h>

bdLinkable::bdLinkable()
: m_previous(BD_NULL),
  m_next(BD_NULL),
  m_headPtr(BD_NULL)
{
}

bdLinkable::~bdLinkable()
{
	unlink();
}

void bdLinkable::link(bdLinkable** head)
{	
	// remove any existing links first
	unlink();

	if(head)
	{
		bdLinkable *const headPtr = *head;
		m_previous = BD_NULL;
		m_next = headPtr;

		if(headPtr)
		{
			headPtr->m_previous = this;
		}

		*head = this;
		m_headPtr = head;
	}
}

void bdLinkable::unlink()
{
	if(m_previous)
	{
		m_previous->m_next = m_next;		
	}

	if(m_next)
	{
		m_next->m_previous = m_previous;
	}

	// if we are the current head
	// set the head to be the next element
	if(m_headPtr && *m_headPtr == this)
	{
		if(m_next)
		{
			// assert both this a next have the same root
			// otherwise they are in different lists
			// BD_ASSERT((m_next->m_headPtr) == m_headPtr, "this and next should have the same head pointer");
		}

		*m_headPtr = m_next;		
	}

	m_previous = BD_NULL;
	m_next = BD_NULL;
	m_headPtr = BD_NULL;
}

bdLinkable* bdLinkable::getNext() const
{
	return m_next;
}

bdBool BD_CALL bdLinkable::find(const bdLinkable *const list, 
								const bdLinkable *const link)
{
	bdBool found = false;
	const bdLinkable *currentLink = list;
	while(!found && (currentLink != BD_NULL))
	{
		found = (currentLink == link);
		currentLink = currentLink->getNext();
	}

	return found;
}
