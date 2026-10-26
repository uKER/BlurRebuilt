// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: 

#ifndef BD_DELIVERY_INITIALIZER_H
#define BD_DELIVERY_INITIALIZER_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdConnection);

class bdDeliveryInitializer
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
	
		/// Default constructor.
		bdDeliveryInitializer();

		/// Virtual destructor.
		virtual ~bdDeliveryInitializer();
		
		virtual void sendInitData(const bdConnectionRef connection) = 0;
};

#endif // BD_DELIVERY_INITIALIZER_H
