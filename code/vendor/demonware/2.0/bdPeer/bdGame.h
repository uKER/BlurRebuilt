// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An interface class for passing commands and game events to a game.

#ifndef BD_GAME_H
#define BD_GAME_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdPlayer);
BD_REFERENCE(bdCommand);

class bdRound;

/// An interface class for passing commands and bdRound events to a game.
/// Derive a class from bdGame to handle the required events.
class bdGame
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdGame();

		/// Virtual destructor.
		virtual ~bdGame();

		/// Called after all commands have been passed to the game via
		/// bdGame::processCommand. This signifies the end of a batch of
		/// commands and is typically used to trigger game logic
		/// eg AI, physics. The time between calls to bdGame::tick
		/// will vary depending on the latency between all peers.
		virtual void tick();

		/// Called to pass commands to a game. Commands are dispatched
		/// to bdGame::processCommand in ordered batches as they are
		/// received from the delivery mechanism. At the end of each
		/// batch a bdGame::tick is issued. To ensure deterministic simulation
		/// bdCommands should be processed in the same oder on all peers.
		/// Each batch of commands is passed to bdGame::processCommand in the
		/// same order on all peers, so no sorting is required.
		/// \param player[in] The player that issued the \a command.
		/// \param command[in] The command issued by \a player.
		virtual void processCommand(const bdPlayerRef player,
									const bdCommandRef command);

		/// Called to indicate the start of a round of play.
		/// \param userData[in] A buffer containing game specific data. The user
		///		data can be specified when calling to bdRound::start()
		virtual void onRoundStart(const bdBitBufferRef userData) =0;

		/// Called to indicate the round has been joined, mid-round as it were.
		/// \todo improve this desc...
		virtual void onRoundJoined();
		
		/// Called to indicate the end of a round of play.
		virtual void onRoundEnd();

		/// Returns true if players are allowed to join mid-round.
		/// If a game requires players/spectators to join after a round
		/// it has started, this method should be overloaded to return
		/// true. In this case bdGame::serialize and bdGame::deserialize
		/// should also be overloaded and implemented.
		/// <b> Late joins are not yet supported. </b>
		/// \return
		/// - True if late joining is allowed.
		/// - False if late joining is not allowed, this is the default.
		virtual bdBool allowLateJoins() const;

		/// Returns true if players are allowed to join mid-round.
		/// This differs from allowLateJoins() in that it can only be 
		/// implemented where the game's state can be transmitted in a single
		/// UDP packet. It requires that the 'game code' implementation uses 
		//  the bdDeliveryInitializer interface so that the host can send
		//  the appropriate initialization state to the new peer.
		//  cf: \ref tutStateBasedMidRoundJoining
		/// \return 
		/// - True if state transmission based late joining is allowed.
		/// - False if state transmission based late joining is not allowed, 
		///   this is the default.
		virtual bdBool allowStateBasedMidRoundJoins() const;

		/// Write the game state to a bdBitBuffer.
		/// Should be overloaded and implemented if late joins or
		/// mid-round recording is required.
		/// \param buffer[in, out] The bdBitBuffer into which to write the
		///		game state.
		virtual void serialize(bdBitBufferRef buffer);

		/// Set the game state from a previously serialized state.
		/// Should be overloaded and implemented if late joins or
		/// mid-round recording is required.
		/// \param buffer[in] The bdBitBuffer from which to read the game
		///		state.
		/// \return
		/// - True if the deserialization was successful.
		/// - False otherwise,
		virtual bdBool deserialize(bdBitBufferRef buffer);

		/// Get a hash value of the game state.
		/// Should be implemented, in debug mode at least, in order to detect 
		/// if the state on peers has diverged. The bdPeer engine will transmit
		/// this every tick and assert where the state on any 2 peers differs.
		//  cf: \ref tutGameStateHashing
		/// \param hash[out] - The hashed value of the game state.
		/// \return 
		/// - True if the hash checking is enabled.
		/// - False otherwise,
		virtual bdBool getGameStateHash(bdUInt32 &hash) const;

};

#endif // BD_GAME_H
