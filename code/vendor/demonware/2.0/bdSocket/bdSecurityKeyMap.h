// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Provides a cross platform API to manage the registration, etc of 
//          bdSecurityIDs and bdSecurityKeys.

#ifndef BD_SECURITY_KEY_MAP_H
#define BD_SECURITY_KEY_MAP_H

#include <bdCore/bdCore.h>
#include <bdCore/bdSocket/bdSecurityID.h>
#include <bdCore/bdSocket/bdSecurityKey.h>

#ifndef BD_PLATFORM_XBOX
class bdSecurityKeyMapListener;
#endif //#ifndef BD_PLATFORM_XBOX

/// A class that stores all currently registered sec ID/Key pairs.
/// This class is used to ensure remote instances are communicating
/// using a valid sec ID/Key pair. 
/// It can also be used to generate sec id/key pairs locally 
/// where these are needed.
class bdSecurityKeyMap
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdSecurityKeyMap();

		/// Initialize the sec id/key params to valid values.
		/// \param id[out] The sec id to initialize.
		/// \param key[out] The sec key to initialize.
		/// \return
		///  - True if a sec id/key pair was created.
		///  - False otherwise.
		/// \note This function does not register the sec id/key.
		/// \todo This code does not need to be associated with the key map.
		///       The best, though disruptive, option is to create a 
		///       bdSecurityData which is used to initialize the values.
		///       [EoF@DW]
		static bdBool BD_CALL initKey(	bdSecurityID &id, 
										bdSecurityKey &key);

		/// Registers a sec id/key pair.
		/// The same key cannot be registered multiple times,
		/// however it is safe to call this function multiple times
		/// with the same key once the return value is not acted upon.
		/// \param id[in] The sec id to register.
		/// \param key[in] The sec key to register.
		/// \return
		///  - True if the sec id/key pair was registered as a new entry.
		///  - False otherwise. This will occur where a duplicate key
		///                     is registered.
		bdBool registerKey(	const bdSecurityID &id, 
							const bdSecurityKey &key);

		/// Unregisters a sec id/key pair.
		/// The sec id/key pair is identified by the sec id.
		/// Removing an id/key pairing will trigger a callback
		/// cf bdSecurityKeyMapListener::onSecurityKeyRemove informing 
		/// higher level code that the id/key pair is no longer valid.
		///
		/// One known side-effect of this, at the time of writing, is that
		/// connections created with the key will be torn down with immediate
		/// effect.  [23rd January 2007 -- EoF]
		///
		/// \param id[in] The sec id to unregister.
		/// \return
		///  - True if the sec id/key pair was unregistered.
		///  - False otherwise. This will occur where a key
		///                     was not previously registered.
		bdBool unregisterKey(const bdSecurityID &id);

#if !defined BD_PLATFORM_XBOX && ! defined BD_PLATFORM_XENON

		/// Accessor to the data in the key map.
		/// The key map is searched, and if a matching id is found the related 
		/// key is returned.
		/// \param id[in] The sec id to search for.
		/// \param key[out] The sec key, if any that was found.
		/// \return
		///  - True if the sec id was found.
		///  - False otherwise. This will occur where the sec id is was not 
		///                     matched to any registered sec id/key .
		bdBool get(const bdSecurityID &id, bdSecurityKey &key);

		/// Checks if a given sec id is registered with the key map.
		/// The key map is searched, and if a matching id is found  true is 
		/// returned.
		/// \param id[in] The sec id to search for.
		/// \return
		///  - True if the sec id was found.
		///  - False otherwise. This will occur where the sec id is was not 
		///                     matched to any registered sec id/key.
		bdBool contains(const bdSecurityID &id) const;

		/// Clears all registered sec id/key pairs.
		/// The key map is iterated, and all sec id/key pairs are unregistered.
		void clear();

		/// Registers a key map listener.
		/// Only one listener may be registered, so calling this function 
		/// repeatedly will result in the previous listener being overwritten.
		/// \param listener[in] A pointer to the listener class implementation.
		void registerListener(bdSecurityKeyMapListener* listener);

		/// Unregisters the key map listener.
		/// Only one listener may be registered at a time. Calling this 
		/// function clears the listener and allows for another listener to be
		/// registered. It also ensures that no callback is issued for key map
		/// listener events
		void unregisterListener();

#endif //#if !defined BD_PLATFORM_XBOX && ! defined BD_PLATFORM_XENON

	protected:

#if !defined BD_PLATFORM_XBOX && ! defined BD_PLATFORM_XENON
		bdSecurityKeyMapListener *m_listener;
		bdHashMap<bdSecurityID, bdSecurityKey> m_map;
#endif //#if !defined BD_PLATFORM_XBOX && ! defined BD_PLATFORM_XENON

};

#endif // BD_SECURITY_KEY_MAP_H

