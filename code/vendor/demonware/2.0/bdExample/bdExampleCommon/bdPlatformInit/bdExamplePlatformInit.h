// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_PLATFORM_INIT_H
#define BD_PLATFORM_INIT_H

#include <bdCore/bdCore.h>

class bdNetStartParams;

// This data structure is used to store platform specific startup parameters
class bdPlatformParams
{
	public:

		bdPlatformParams();

#if defined BD_PLATFORM_PSP || defined BD_PLATFORM_PS2
	// Path to the modules directory
	bdString m_modulePath;
#endif

#if defined BD_PLATFORM_PS3
	bdBool m_NPSignIn;
#endif

};


// A class to perform platform specific initialization and clean up 
class bdExamplePlatformInit
{
	public:

		/// Performs platform specific initialization 
		/// \param netParams[out] After successful initialization, netParams will
		///				be set up to contain all necessary network related information 
		/// \param platformParams[in] Optional additional startup parameters
		/// \return Platform specific return value
		static bdInt BD_CALL init(bdNetStartParams &netParams, 
								  bdPlatformParams *platformParams = BD_NULL);

		/// Performs platform specific shut down operations
		static void BD_CALL quit();

	protected:

		bdExamplePlatformInit();
};

#endif // BD_PLATFORM_INIT_H
