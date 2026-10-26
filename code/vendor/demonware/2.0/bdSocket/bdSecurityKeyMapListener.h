// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-ADHOC PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Listener to allow higher level code to be notified of key events!

#ifndef BD_SECURITY_KEY_MAP_LISTENER_H
#define BD_SECURITY_KEY_MAP_LISTENER_H

#include <bdCore/bdCore.h>

/// Callback interface to permit higher level code be notified when a 
/// sec id/key is unregistered.
/// This may be used to drive certain cleanup routines as determined by 
/// the higher level code.
class bdSecurityKeyMapListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// This is invoked when a sec id/key pair is unregistered with the
		/// bdSecurityKeyMap.
		/// \param id[out] The sec id that has been unregistered.
		virtual void onSecurityKeyRemove(const bdSecurityID &id) = 0;

		/// Virtual destructor ensures proper memory cleanup.
		virtual ~bdSecurityKeyMapListener()
		{
		};
};

#endif // BD_SECURITY_KEY_MAP_LISTENER_H

