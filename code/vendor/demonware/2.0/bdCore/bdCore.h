// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Initialization and shutdown of framework memory management, error
//			handling and singletons.

#ifndef BD_CORE_H
#define BD_CORE_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

#define BD_VERSION "2.3.2"


/// Initialization and shutdown of framework memory management, error handling
/// and singletons.
class bdCore
{
	public:

		/// Initialise bdCore with its defaults.
		/// \param defaultMemoryFunctions[in]
		/// - true means init() will set up the default memory functions (to use the bitdemon memory calls).
		/// - false means init() will not change the current memory bindings. This is useful if, for example,
		/// you have already set up bdMemory to use different memory functions.
		static void BD_CALL init(const bdBool defaultMemoryFunctions = true);

		/// Shutdown bdCore correctly.
		static void BD_CALL quit();

		/// Flag to indicate if this class has been initialized.
		static bdBool m_initialized;

	protected:

		/// Protected constructor since all the functions are static.
		bdCore();


};

// Commonly used objects are included here for convenience. So users need only
// include <bdCore/bdCore.h> rather than the specific object headers.

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#include <bdCore/bdContainers/bdFastArray.h>
#include <bdCore/bdContainers/bdArray.h>
#include <bdCore/bdContainers/bdBitBuffer.h>
#include <bdCore/bdContainers/bdByteBuffer.h>
#include <bdCore/bdContainers/bdHashMap.h>
#include <bdCore/bdContainers/bdLinkedList.h>
#include <bdCore/bdContainers/bdPriorityQueue.h>
#include <bdCore/bdContainers/bdPriorityHeap.h>
#include <bdCore/bdContainers/bdQueue.h>
#include <bdCore/bdContainers/bdSet.h>
#include <bdCore/bdContainers/bdSingleton.h>
#include <bdCore/bdContainers/bdSequenceNumber.h>
#include <bdCore/bdContainers/bdSequenceNumberStore.h>

#include <bdCore/bdSocket/bdDefaultSocket.h>
#include <bdCore/bdSocket/bdAddr.h>
#include <bdCore/bdSocket/bdCommonAddr.h>
#include <bdCore/bdSocket/bdSecurityKey.h>
#include <bdCore/bdSocket/bdSecurityID.h>
#include <bdCore/bdSocket/bdEndpoint.h>

#include <bdCore/bdString/bdString.h>

#include <bdCore/bdTiming/bdStopwatch.h>

#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdReference/bdReference.h>

#include <bdCore/bdUtilities/bdBitOperations.h>
#include <bdCore/bdUtilities/bdRandom.h>
#include <bdCore/bdUtilities/bdTrulyRandom.h>

#include <bdCore/bdMemory/bdMemory.h>


#endif // BD_CORE_H

