// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef BD_LINKABLE_H
#define BD_LINKABLE_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

class bdLinkable
{
	public:

		bdLinkable();

		virtual ~bdLinkable();

		void link(bdLinkable **head);

		void unlink();

		bdLinkable* getNext() const;

		static bdBool BD_CALL find(const bdLinkable *const list, 
								   const bdLinkable *const link);

	protected:

		bdLinkable *m_previous;

		bdLinkable *m_next;

		bdLinkable **m_headPtr;
};

#endif // BD_LINKABLE_H
