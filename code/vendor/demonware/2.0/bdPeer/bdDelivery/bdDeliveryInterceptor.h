// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Interface classes for accepting messages from a bdDeliveryManager.

#ifndef BD_DELIVERY_INTERCEPTOR_H
#define BD_DELIVERY_INTERCEPTOR_H

#include <bdCore/bdCore.h>

class bdDeliveryMessage;

/// An interface class to accept messages from a bdDeliveryManager.
/// Classes derived from bdDeliveryInterceptor can be 
/// registered with the a bdDeliveryManager using 
/// bdDeliveryManager::registerInterceptor.
class bdDeliveryInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
	
		/// Default constructor.
		bdDeliveryInterceptor();

		/// Virtual destructor.
		virtual ~bdDeliveryInterceptor();
		
		/// Accept a message from the bdDeliveryManager with which \a this 
		/// is registered.
		/// This function must be implemented in any object that has been
		/// registered to accept bdDeliveryManager messages.
		/// When messages arrive at the bdDeliveryManager 
		/// bdDeliveryInterceptor::acceptDelivery will be called on the 
		/// registered interceptors until true is returned. If no interceptors
		/// accept the message it is discarded.
		/// \param receivedMsg[in] The message to be accepted.
		/// \return
		///  - True if this interceptor handles the message.
		///  - False if this interceptor cannot handle the message.
		virtual bdBool acceptDelivery(const bdDeliveryMessage& receivedMsg) = 0;
};

#endif // BD_DELIVERY_INTERCEPTOR_H
