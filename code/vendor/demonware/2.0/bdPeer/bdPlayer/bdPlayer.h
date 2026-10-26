// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The base class used to represent a player in a peer-to-peer game.

#ifndef BD_PLAYER_H
#define BD_PLAYER_H

#include <bdCore/bdCore.h>

/// The data type to use for bdPlayer numbering.
/// This identifies a player uniquely across all peers.
typedef bdUByte8 bdPlayerId;
#define BD_PLAYER_ID_READ readUByte8
#define BD_PLAYER_ID_WRITE writeUByte8

BD_REFERENCE(bdPlayer);

/// The base class used to represent a player in a peer-to-peer game.
/// Derive from this call if you wish to add any extra information to your
/// players. bdPlayer::serialize and bdPlayer::deserialize should be overloaded
/// to read and write the extra data in the derived class. Use the
/// BD_REGISTER_PLAYER_CLASS macro to register a derived bdPlayer class with
/// the bdPlayerFactory.
class bdPlayer : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdPlayer();

		/// Virtual destructor.
		virtual ~bdPlayer();

		/// Sets the bdPlayerId of this player.
		/// \warning bdPlayersIds are assigned by the bdPlayerManager to be
		///		unique across all peers. Do not reset bdPlayerIds, doing so
		///		will have undefined results.
		/// \param id[in] The bdPlayerId for this player.
		void setId(const bdPlayerId id);

		/// Get the bdPlayerId of this player. bdPlayersIds are assigned by the
		/// bdPlayerManager to be unique across all peers.
		/// \return The id of this player.
		bdPlayerId getId() const;

		/// Sets the address of this player.
		/// The address identifies the players machine.
		/// \warning The bdPlayerManager will set all player addresses,
		///		do not reset player addresses, doing so will have undefined
		///		results.
		/// \param addr[in] The location of this player.
		void setAddr(const bdCommonAddrRef addr);

		/// Get the address of the bdPlayers machine.
		/// \return The network address of the bdPlayers machine.
		bdCommonAddrRef getAddr() const;

		/// Is this a local player.
		/// \return
		/// - True if the player is on the local machine.
		/// - False otherwise.
		bdBool isLocal() const;

		/// Serializes the bdPlayers state into a bdBitBuffer.
		/// \param buffer[in] The bdBitBuffer into which the state
		///		should be written.
		virtual void serialize(bdBitBufferRef buffer);

		/// Deserializes the object's state from a bdBitBuffer.
		/// \param buffer[in] The bdBitBuffer from which the state should
		///		be read.
		/// \return
		/// - True if the state could be read successfully.
		/// - False otherwise.
		virtual bdBool deserialize(bdBitBufferRef buffer);

	protected:

		/// The identifier of this bdPlayer.
		bdPlayerId m_id;

		/// The address of this bdPlayer.
		bdCommonAddrRef m_addr;
};

/// A function used to sort bdPlayers by their bdPlayerIds.
class bdPlayerSort
{
	public:

		/// bdPlayer ordering function. Orders bdPlayers using their
		/// bdPlayerIds.
		/// \param x
		/// \param y
		/// \return
		/// - True if bdPlayer \a x has a bdPlayerId less than that
		///		of \a y.
		///	- False otherwise.
		inline bdBool operator() (const bdPlayerRef& x,
								  const bdPlayerRef& y) const
		{
			const bdBool lessThan = x->getId() < y->getId();
			return lessThan;
		}
};

#endif // BD_PLAYER_H


