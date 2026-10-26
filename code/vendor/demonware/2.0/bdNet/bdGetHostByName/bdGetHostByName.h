// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

/// Purpose: A platform independent wrapper for performing Domain Name Server lookups

#ifndef BD_GET_HOST_BY_NAME_H
#define BD_GET_HOST_BY_NAME_H

#include <bdCore/bdCore.h>
#include <bdCore/bdThread/bdRunnable.h>
#include <bdNet/bdGetHostByName/bdGetHostByNameConfig.h>

class bdThread;

#define BD_MAX_NUM_ADDRESSES (4u)

/// Wii WARNING: Threading Nintendo socket library calls has been known to cause issues.
/// It has caused DNS lookups to fail intermittently and has caused SoShutdown to 
/// fail because it thinks it still has memory allocated. In particular, doing simultaneous
/// DNS lookups has been particularily error prone.

class bdGetHostByName : public bdRunnable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdStatus
		{
			BD_LOOKUP_UNINITIALIZED, // The class is ready to start a lookup
			BD_LOOKUP_PENDING, // A lookup is running
			BD_LOOKUP_SUCCEEDED, // A name was successfully resolved to one or more IP address(s)
			BD_LOOKUP_FAILED, // A name was resolved to zero IP addresses
			BD_LOOKUP_CANCELLED, // The active lookup was canceled.
			BD_LOOKUP_TIMED_OUT, // The active lookup timed out.
			BD_LOOKUP_ERROR // An error was encountered while starting/running the lookup.
		};

		/// Basic Constructor.
		bdGetHostByName();

		/// Basic Destructor.
		~bdGetHostByName();

		/// Start the DNS lookup.
		/// This call creates a separate worker thread, call getStatus() to
		/// retrieve the status of the name lookup.
		/// \param hostname[in] The name of the host that needs to be resolved.
		/// \param config [in] Optional configuration info for the class.
		/// \return - True: success
		///			- False: failed
		/// \note Once this function has been called, quit() must be
		//        called to avoid any thread related memory leaks.
		bdBool start(	const bdNChar8 *hostname, 
						bdGetHostByNameConfig config = bdGetHostByNameConfig());

		/// Pumps the active host lookup.
		/// This will abort the lookup if it has run for longer than the
		/// timeout.
		void pump();

		/// Reset the class to it's initial state.
		/// Once this is called it is safe to start another host name lookup
		/// using the start function.
		/// \note If called while a lookup is active the lookup will be
		///       aborted.
		void quit();

		/// Gets the current status of the name lookup.
		/// \return The status of the lookup.
		bdGetHostByName::bdStatus  getStatus() const;

		/// Gets the actual number of IP addresses that were returned by the
		/// DNS server.
		/// \return The number of IP addresses returned by the DNS server
		bdUInt getNumAddresses() const;

		/// Gets a specific address from the result set
		/// \param index[in]: The index of the address to be returned
		///                   i.e. (0 .. getNumAddresses() -1)
		/// \return  the InAddr of the specified address.
		bdInAddr getAddressAt(bdUInt index) const;


	protected:

		virtual bdUInt run(void* args);

		void cancelLookup();

		bdStopwatch m_timer;

		bdStatus m_status;

		bdThread *m_thread;

		bdGetHostByNameConfig m_config;

		bdInAddr m_adresses[BD_MAX_NUM_ADDRESSES];

		bdUInt m_numAddresses;

		bdDNSHandle m_handle;

};

#endif  //BD_GET_HOST_BY_NAME_H
