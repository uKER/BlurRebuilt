// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Abstraction of the session hash data.

#ifndef BD_SESSION_DATA_H
#define BD_SESSION_DATA_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdConnection);
BD_REFERENCE(bdSessionData);

/// Abstraction of the session hash in the bdPeer library. 
/// Abstracted to the bdSessionData class. 
/// cf. BD_REGISTER_SESSION_DATA_CLASS
/// This abstraction allows the developer to create a derived class and register 
/// that. 
/// At the time of writing [24th Jan 2007 - EoF@DW] the ExpandedSesionData example 
/// in the Peer examples illustrates how you might use this to get information 
/// on the connection graph of the p2p session.
class bdSessionData : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor.
		bdSessionData();

		/// Virtual destructor.
		virtual ~bdSessionData();

		/// Serialize the session data into the buffer.
		/// \param buffer[in] The buffer into which this class shall serialize 
		///                   itself.
		virtual void serialize(bdBitBufferRef buffer);

		/// Deserialize the session data from the buffer.
		/// \param buffer[in] The buffer into which this class shall 
		///                   deserialize itself.
		/// \return
		/// - True, if the session data was deserialized correctly.
		/// - False, otherwise.
		virtual bdBool deserialize(bdBitBufferRef buffer);

		/// Accessor to the connection that this data refers to.
		/// \return
		/// - A reference to the connection, this may be BD_NULL if the 
		///   connection is not set.
		bdConnectionRef getConnection() const;

		/// Set the connection associated with this session data.
		/// \param conn[in]	The connection associated with this session data.
		void setConnection(bdConnectionRef conn);

		/// Check if this session data is the same as <i> other </i>
		/// \param other[in] The session data to compare to this.
		/// \return
		/// - True, if the 2 sessions' data are equivalent.
		/// - False, otherwise.
		virtual bdBool equals(bdSessionDataRef other) const;

		/// Instructs this class to recalculate it's internal hash
		/// based on the list of peers supplied.
		/// This is invoked whenever the session membership alters.
		/// \param peers[in] The list of peers in the session.
		virtual void calculateLocalHash(bdArray<bdSessionDataRef> peers);

	protected:

		/// The connection to the peer.
		bdConnectionRef m_connection;

		/// A hash of the peer's view of all the other peer IDs.
		/// This is the basis of the consistency checking.
		bdUInt32 m_hash;

};

#endif // BD_SESSION_DATA_H
