// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The messages dispatched by a bdDeliveryManager.

#ifndef BD_DELIVERY_MESSAGE_H
#define BD_DELIVERY_MESSAGE_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdMessage/bdMessage.h>

BD_REFERENCE(bdMessage);
BD_REFERENCE(bdCommonAddr);

/// A message dispatched by a bdDeliveryManager.
/// Combines a bdMessage with the address of the sender, 'from', and that of
/// original sender, the 'about' address. The 'from' and 'about'
/// addresses can differ when the bdDeliveryManager dispatching them has
/// received a recovery message. Such messages swapped between peers when a
/// host disconnects to ensure all peers process exactly the same messages.
class bdDeliveryMessage
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdDeliveryMessage();

		/// Constructor.
		/// \param message[in] The bdMessage to be delivered.
		/// \param from[in] The sender address.
		/// \param about[in] The original sender of the message.
		bdDeliveryMessage(const bdMessageRef message,
						  const bdCommonAddrRef from,
						  const bdCommonAddrRef about);

		/// Destructor.
		~bdDeliveryMessage();

		/// Get the message for the delivery package.
		/// \return the bdMessage from the bdDeliveryMessage.
		bdMessageRef getMessage() const;

		/// Get the address of the sender.
		/// \return the address of the sender.
		bdCommonAddrRef getFromAddr() const;

		/// Get the address of the originator.
		/// \return the address of the originator.
		bdCommonAddrRef getAboutAddr() const;

		/// Set the address of the originator.
		/// \param about[in] The address of the peer that originally sent the
		///		message.
		void setAboutAddr(const bdCommonAddrRef about);

		/// Less than operator.
		/// Used to sort beDeliveryMessages before dispatching to
		/// ensure consistency across all peers. Uses the
		/// 'about' address to preform the comparison.
		/// \param other[in] A reference to a bdDeliveryMessage to which
		///		\a this should be compared.
		///	\return
		/// - True is \a this is less than \a other.
		/// - False otherwise.
		bdBool operator < (const bdDeliveryMessage& other) const;

	protected:

		bdMessageRef m_message;
		bdCommonAddrRef m_fromAddr;
		bdCommonAddrRef m_aboutAddr;
};

#endif // BD_DELIVERY_MESSAGE_H
