// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An interface for an object which can intercept messages
//			before they get dispatched.

#ifndef BD_DISPATCH_INTERCEPTOR_H
#define BD_DISPATCH_INTERCEPTOR_H

#include <bdCore/bdCore.h>

class bdReceivedMessage;

/// An interface for an object which can intercept messages before they get
/// dispatched.
class bdDispatchInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdDispatchInterceptor();

		/// Virtual Destructor
		virtual ~bdDispatchInterceptor();

		/// Accept a message.
		/// This function must be implemented in any object that has been
		/// registered to accept messages.
		/// bdDispatcherImpl::process() will ask each registered interceptor in
		/// turn if it can handle the message.
		/// If any interceptor returns true then processing for that message
		/// will cease.
		/// \param receivedMsg[in]	The message to be handled.
		/// \return
		///  - True if this interceptor can and will handle the message.
		///  - False if this interceptor cannot handle the message.
		virtual bdBool accept(const bdReceivedMessage& receivedMsg) = 0;
};

#endif // BD_DISPATCH_INTERCEPTOR_H
