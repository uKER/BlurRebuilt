// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Get textual information describing a delivery manager.

#ifndef BD_DELIVERY_MANAGER_INFO_H
#define BD_DELIVERY_MANAGER_INFO_H

#include <bdCore/bdCore.h>

class bdDeliveryManager;

/// Get a text string describing a bdDeliveryManager.
class bdDeliveryManagerInfo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Writes textual information about a bdDeliveryManager to a buffer.
		/// \param deliveryManager[in] A pointer to the bdDeliveryManager whose
		///		information is required.
		/// \param buf[in,out] The pointer to a buffer into which the
		///		information should be written.
		/// \param length[in] The length of the data pointer to by \a buf
		///		in bytes.
		static bdUInt BD_CALL getInfo(const bdDeliveryManager *const deliveryManager,
									  bdNChar8 *buf,
									  const bdUInt length);

	protected:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdDeliveryManagerInfo();
};

#endif // BD_DELIVERY_MANAGER_INFO_H
