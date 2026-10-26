// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Wrapper class to tie together the modules required for a
///		peer-to-peer system.

#ifndef BD_PEER_H
#define BD_PEER_H

#include <bdCore/bdCore.h>

#include <bdPeer/bdSession/bdSession.h>
#include <bdPeer/bdDelivery/bdDeliveryManager.h>
#include <bdPeer/bdPlayer/bdPlayerManager.h>
#include <bdPeer/bdRound.h>

#include <bdNet/bdNet.h>

class bdGame;

BD_REFERENCE(bdPlayer);
BD_REFERENCE(bdGameInfo);

/// Encapsulated a peer is a peer-to-peer command driven simulation.
/// bdPeer ties together all the elements required to create a peer-to-peer
/// system, namely bdSession, bdDeliveryManager, bdPlayerManager and bdRound.
/// It contains all the code needed to get these classes working together and
/// lots of sanity checks.
class bdPeer
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor.
		/// \param game[in] A pointer to a bdGame interface to which commands
		///		should be dispatched.
		/// \param messageLimit[in (optional)] The latency compensation
		///	message limit. 
		/// \param sessionHandler[in (optional)] A pointer to a
		///		bdSessionHandler to be queried on the host when new peers try
		///		to join.
		bdPeer(bdGame *const game,
			   const bdUInt messageLimit = 1,
			   bdSessionHandler *const sessionHandler = BD_NULL);

		// Destructor.
		~bdPeer();		

		/// Get the underlying bdSession.
		/// \return a pointer to the bdSession used by this bdPeer.
		bdSession* getSession();

		/// Get the underlying bdDeliveryManager.
		/// \return a pointer to the bdDeliveryManager used by this bdPeer.
		bdDeliveryManager* getDeliveryManager();

		/// Get the underlying bdPlayerManager.
		/// \return a pointer to the bdPlayerManager used by this bdPeer.
		bdPlayerManager* getPlayerManager();

		/// Get the underlying bdRound.
		/// \return a pointer to the bdRound used by this bdPeer.
		bdRound* getRound();

		/// Get the game interface used by this bdPeer.
		/// \return a pointer to the bdGame interface to which commands are
		///		dispatched.
		bdGame* getGame();

		/// Connect to a peer-to-peer system.
		/// Attempts to connect to a peer-to-peer game specified by
		/// \a gameInfo. If \a gameInfo specified the host to be local
		/// the game will be hosted by this instance.
		/// \param gameInfo[in] A bdGameInfo instance that identifies the game
		///		to connect to.
		///	\param userData[in (optional)] User data to be sent to the host
		///		with the join session request. See bdSession::join.
		/// \return
		/// - True, if the session join was initiated.
		/// - False, otherwise.
		/// \note The secID & secKey present in the gameInfo must be registered
		///       before calling this function.
		bdBool connect(const bdGameInfoRef gameInfo,
					   bdBitBuffer *const userData = BD_NULL);

		/// Gracefully disconnect from a peer-to-peer system.
		void leave();

		/// Request the addition of a player to the system.
		/// Player addition is A-synchronous for more information see
		///	bdPlayerManager::add.
		/// \param player[in] A reference to a bdPlayer or derived class that
		///		is to be added.
		/// \return
		/// - True if the add player request was made.
		/// - False otherwise.
		bdBool addPlayer(bdPlayer& player);

		/// Request the removal of a player from the system.
		/// Player removal is A-synchronous for more information see
		///	bdPlayerManager::remove.
		/// \param player[in] A reference to a bdPlayer or derived class that
		///		is to be removed.
		/// \return
		/// - True if the remove player request was made.
		/// - False otherwise.
		bdBool removePlayer(const bdPlayerRef player) const;

		/// Begin a round of game play.
		/// Starting a round A-synchronous for more information see
		///	bdRound::start.
		/// \param seed[in] A 32 bit integer with which to seed the pseudo
		///		random number generator
		/// \param userData[in] A buffer containing game specific data. This data
		///		is passed through to the onRoundStart callback in bdGame
		/// \return
		/// - True if the start round request was made.
		/// - False otherwise.
		bdBool startRound(const bdUInt32 seed = 1000,
						  bdBitBuffer *const userData = BD_NULL);

		/// End a round of game play.
		/// Ending a round A-synchronous for more information see
		///	bdRound::end.
		/// \return
		/// - True if the end round request was made.
		/// - False otherwise.
		bdBool endRound();

		/// Check that sending of commands/messages will succeed.
		/// Adds an additional condition to bdDeliveryManager::okToSend
		/// that a round of game play in progress. Otherwise commands
		/// sent will be ignored.
		/// \return
		/// - True if it is sensible to send commands.
		/// - False otherwise.
		bdBool okToSendCommands() const;

		/// Add a command to be sent to all peers.
		/// Commands will only be added if a round of game play is in progress.
		/// For more information see bdRound::addCommand.
		/// \param player[in] The player with which the \a command should be
		///		associated, generally the player who generated it.
		///	\param command[in] The bdCommand to be sent to all peers.
		/// - True if the command was added.
		/// - False otherwise.
		bdBool addCommand(const bdPlayerRef player,
						  bdCommand& command);

		/// Set the interval at which messages and commands are sent to all
		/// peers.
		/// \param interval[in] The time in seconds to wait between sending
		///		to all peers.
		void setSendInterval(const bdFloat32 interval);

		/// Record the elapsed time and send messages and commands if the send
		/// interval has elapsed.
		/// \param deltaTime[in] The elapsed time, this can we wall time or
		///		virtual time. Wall time is recommended.
		/// \return
		/// - True if any messages and commands where sent.
		/// - False otherwise.
		bdBool step(const bdFloat32 deltaTime = 0);

		/// Is bdGameLog being used to replay through this bdPeer.
		/// \return
		/// - True if a replay in in progress.
		/// - False otherwise.
		bdBool getReplaying() const;

		/// Signal that a replay is in progress.
		/// Selects whether to return the normal or dedicated replay bdRound
		/// and bdPlayerManager from calls to bdPeer::getRound and
		/// bdPeer::getPlayerManager.
		/// \param replaying[in]
		/// - True to return the replay player manager and round.
		/// - False to return the bdPeers player manager and round.
		void setReplaying(const bdBool replaying);

		/// Set the replay player manager.
		/// Sets bdPlayerManager returned by calls to bdPeer::getPlayerManager
		/// when setReplaying::setReplaying has been set to true.
		/// \param replayPlayerManager[in] The pointer to the bdPlayerManager
		///		to be returned from bdPeer::getPlayerManager.
		void setReplayPlayerManager(bdPlayerManager *const replayPlayerManager);

		/// Set the replay round.
		/// Sets bdRound returned by calls to bdPeer::getRound
		/// when setReplaying::setReplaying has been set to true.
		/// \param replayRound[in] The pointer to the bdPlayerManager
		///		to be returned from bdPeer::getPlayerManager.
		void setReplayRound(bdRound *const replayRound);

	protected:

		bdGame *m_game;
		bdSession m_session;
		bdDeliveryManager m_deliveryManager;
		bdPlayerManager m_playerManager;
		bdRound m_round;

		bdBool m_replaying;
		bdPlayerManager *m_replayPlayerManager;
		bdRound *m_replayRound;

		bdFloat32 m_sendInterval;
		bdFloat32 m_elapsedTime;

};

#endif // BD_PEER_H
