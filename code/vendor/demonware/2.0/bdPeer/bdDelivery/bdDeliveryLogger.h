// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Interface classes for peeking at messages dispatched from a 
//	bdDeliveryManager.

#ifndef BD_DELIVERY_LOGGER_H
#define BD_DELIVERY_LOGGER_H

#include <bdCore/bdCore.h>

class bdDeliveryMessage;

/// An interface class to peek at all messages dispatched from a 
/// bdDeliveryManager.
/// Classes derived from bdDeliveryLogger can be 
/// registered with the a bdDeliveryManager using 
/// bdDeliveryManager::registerLogger.
class bdDeliveryLogger
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
	
		/// Default constructor.
		bdDeliveryLogger();

		/// Virtual destructor.
		virtual ~bdDeliveryLogger();

		/// Peek at a message dispatched from the bdDeliveryManager with 
		/// which \a this is registered.
		/// This function must be implemented in any object that has been
		/// registered to log bdDeliveryManager messages.
		/// When messages arrive at the bdDeliveryManager 
		/// bdDeliveryLogger::logMessage will be called on all 
		/// registered loggers.
		/// \param receivedMsg[in] The dispatched message.
		virtual void logMessage(const bdDeliveryMessage& receivedMsg) = 0;
};

#endif // BD_DELIVERY_LOGGER_H
