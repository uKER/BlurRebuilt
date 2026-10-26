// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_ROUND_H
#define BD_ROUND_H

#include <bdCore/bdCore.h>

#include <bdPeer/bdSession/bdSessionInterceptor.h>

#include <bdPeer/bdDelivery/bdDeliveryManagerListener.h>
#include <bdPeer/bdDelivery/bdDeliveryInterceptor.h>
#include <bdPeer/bdDelivery/bdDeliveryInitializer.h>

class bdSession;
class bdDeliveryManager;
class bdPlayerManager;
class bdGame;
class bdCommand;

BD_REFERENCE(bdPlayer);

/// Synchronizes rounds of game play across all peers.
/// bdRound synchronizes the start, end and entry into rounds of game play. It 
/// queries a bdGame interface to enable players to join mid round, or tells
/// them to wait till the current round has completed.
/// bdRound retrieves bdCommands from a bdDeliveryManager and passes them onto
/// a bdGame.
/// A pseudo random number generator is provided and can be seeded at the 
/// start of a round.
class bdRound :	public bdDeliveryManagerListener,
				public bdDeliveryInterceptor,
				public bdDeliveryInitializer,
				public bdSessionInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Defines the possible states of a bdRound.
		/// - BD_WAITING we have tried to join while a round of game play is
		///		already in progress, and the game does not support mid-round 
		///		joins (see bdGame::allowLateJoins).
		/// - BD_IN_LOBBY the round is yet to start.
		/// - BD_IN_ROUND we are taking part in a round of game play.
		/// - BD_UPDATING we have tried to join while a round of game play is
		///		already in progress, and the game support mid-round joins 
		///		(see bdGame::allowLateJoins). 
		///		<b> Late joins are not yet supported. <\b>
		enum bdRoundStatus
		{
			BD_ROUND_UNINITIALIZED,
			BD_WAITING,
			BD_IN_LOBBY,
			BD_IN_ROUND,
			BD_UPDATING,
			BD_MAX_ROUND_STATUS
		};

		/// Constructor.
		bdRound();

		/// Virtual destructor.
		virtual ~bdRound();

		/// Initialization.
		/// \param deliveryManager[in] A pointer to a bdDeliveryManager that 
		///		will delivers messages for the \a game.
		/// \param playerManager[in] A pointer to a bdPlayerManager that 
		///		communicates through \a deliveryManager.
		/// \param game[in] A pointer to a bdGame interface to which commands
		///		should be dispatched.
		bdBool init(bdDeliveryManager *const deliveryManager,
					bdPlayerManager *const playerManager,
					bdGame *const game);

		void quit();

		/// Get the status of this round.
		/// \return The current status of the bdRound.
		bdRoundStatus getStatus() const;

		/// Get the pseudo random number generator.
		/// The pseudo random number generator, is seeded at the start of 
		/// each round see bdRound::start.
		/// \return A pointer to the bdRounds pseudo random number generator.
		bdRandom* getRandom();

		/// Begin a round of game play.
		/// Adds a start round message into the underlying bdDeliveryManager.
		/// The round will not start until this message is dispatched by the 
		/// bdDeliveryManager. Hence starting a round is A-synchronous, to 
		/// be informed of round start events overload bdGame::onRoundStart
		/// in the bdGame instance passed to the constructor.
		/// \param seed[in] A 32 bit integer with which to seed the pseudo 
		///		random number generator
		/// \param userData[in] A buffer containing game specific data. This data
		///		is passed through to the onRoundStart callback in bdGame 
		bdBool start(	const bdUInt32 seed,
						bdBitBuffer *const userData = BD_NULL);

		/// Ends a round of game play.
		/// Adds a end round message into the underlying bdDeliveryManager.
		/// The round will not end until this message is dispatched by the 
		/// bdDeliveryManager. Hence ending a round is A-synchronous, to 
		/// be informed of round end events overload bdGame::onRoundEnd
		/// in the bdGame instance passed to the constructor.		
		bdBool end();

		/// Adds a bdCommand to be sent to all peers.
		/// \param player[in] The player with which the \a command should be 
		///		associated, generally the player who generated it.
		///	\param command[in] The bdCommand to be sent to all peers.
		void addCommand(const bdPlayerRef player,
						bdCommand& command);		

		/// Write the state of the bdRound to a bdBitBuffer.
		/// \param buffer[in,out] The bdBitBuffer into which to write our state.
		void serialize(bdBitBufferRef buffer);

		/// Set the state of this bdRound from a previously 
		/// serialized state.
		/// \param buffer[in] The bdBitBuffer from which to read our state.
		bdBool deserialize(bdBitBufferRef buffer);

		void addHash(const bdUInt32 hash) const;

	protected:		

		virtual bdBool acceptDelivery(const bdDeliveryMessage& receivedMsg);	

		virtual void onDeliveryEnd();

		/// Handler for a start round message.
		/// \param startMessage[in] : the start round message
		void handleStart(const bdDeliveryMessage& startMsg);

		/// Handler for an end round message.
		/// \param endMessage[in] : the end round message
		void handleEnd(const bdDeliveryMessage& endMsg);

		/// Handler for a user command message.
		/// \param commandMsg[in] : the user command message
		void handleCommand(const bdDeliveryMessage& commandMsg);

		void handleHash(const bdDeliveryMessage& commandMsg);

		virtual void sendInitData(const bdConnectionRef connection);

		virtual bdBool acceptSession(const bdReceivedMessage& receivedMsg);

		void handleInitData(const bdReceivedMessage& initData);

		virtual void onDeliveryInitCompleted();

	protected:

		friend class bdGameLog;

	protected:

		bdDeliveryManager *m_deliveryManager;
		bdPlayerManager *m_playerManager;
		bdGame *m_game;

#ifdef BD_DEBUG
		bdBool m_randomSeeded;
#endif //BD_DEBUG
		bdRandom m_random;

		bdRoundStatus m_status;

		bdBool m_hashSet;

		bdUInt32 m_gameStateHash;
};


#endif // BD_ROUND_H
