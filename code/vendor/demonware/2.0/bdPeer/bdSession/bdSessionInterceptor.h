// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An interface class to accept messages from a session.

#ifndef BD_SESSION_INTERCEPTOR_H
#define BD_SESSION_INTERCEPTOR_H

#include <bdCore/bdCore.h>

class bdReceivedMessage;

/// An interface class to accept messages from a bdSession.
/// Classes derived from bdSessionInterceptor can be 
/// registered with the session using bdSession::registerInterceptor.
class bdSessionInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
	
		/// Default constructor.
		bdSessionInterceptor();

		/// Virtual destructor.
		virtual ~bdSessionInterceptor();

		/// Accept a message from the bdSession with which \a this is 
		/// registered.
		/// This function must be implemented in any object that has been
		/// registered to accept bdSession messages.
		/// When messages arrive at the bdSession  
		/// bdSessionInterceptor::acceptSession will be called on the 
		/// registered interceptors until true is returned. If no interceptors
		/// accept the message it is discarded.
		/// \param receivedMsg[in] The message to be accepted.
		/// \return
		///  - True if this interceptor handles the message.
		///  - False if this interceptor cannot handle the message.
		virtual bdBool acceptSession(const bdReceivedMessage& receivedMsg) = 0;
};

#endif // BD_SESSION_INTERCEPTOR_H
