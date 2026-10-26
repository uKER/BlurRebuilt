// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to process all incoming messages and dispatch them
//			appropriately.

#ifndef BD_DISPATCHER_H
#define BD_DISPATCHER_H

#include <bdCore/bdCore.h>

class bdConnectionStore;
class bdDispatchInterceptor;

BD_REFERENCE(bdConnection);

/// A class to process all incoming messages and dispatch them
///	appropriately.
class bdDispatcher
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdDispatcher();

		/// Dispatches all messages.
		/// \warning It is important that there are no bdConnectionManager
		/// iterators in use while this function is called as it may cause
		/// modifications to bdConnectionManager and invalidate any iterators.
		void process(bdConnectionRef connection);

		/// Registers an interceptor with the dispatcher.
		/// \param interceptor[in]	The interceptor to be registered with this
		///							dispatcher.
		void registerInterceptor(bdDispatchInterceptor *const interceptor);

		/// Removes an existing interceptor from the dispatcher.
		/// \param interceptor[in]	The interceptor to be removed from this
		///							dispatcher.
		void unregisterInterceptor(bdDispatchInterceptor *const interceptor);

		/// This will clear all the currently registered dispatch interceptors.
		/// Is is used to tidy up the class and prepare it for re use.
		void reset();

	protected:

		/// An array of all currently registered interceptors.
		bdFastArray<bdDispatchInterceptor*> m_interceptors;
};


#endif // BD_DISPATCHER_H
