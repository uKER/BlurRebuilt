// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: recording and playback of messges.

#ifndef BD_GAME_LOG_H
#define BD_GAME_LOG_H

#include <bdCore/bdCore.h>

#include <bdPeer/bdDelivery/bdDeliveryLogger.h>
#include <bdPeer/bdDelivery/bdDeliveryManagerListener.h>
#include <bdPeer/bdDelivery/bdDeliveryMessage.h>
#include <bdPeer/bdPlayer/bdPlayerManager.h>
#include <bdPeer/bdRound.h>

class bdPeer;
class bdGame;

/// Records and plays replays.
/// Efficiently records all messages sent to a peer and reissues them on demand.
/// A bdGameLog records any messages that are dispatched from a
/// bdDeliveryManager. It can be used for recording and replaying networked
/// game activity for example entire rounds of play, sections of a round and
/// even lobby activity.
class bdGameLog : public bdDeliveryLogger,
			      public bdDeliveryManagerListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdGameLog();

		/// Virtual destructor.
		virtual ~bdGameLog();

		/// Begin recording.
		/// Starts recording all messages passed to \a peer for later replay.
		/// \param peer[in] A pointer to the peer whose messages should be
		///		recorded.
		void start(bdPeer *const peer);

		/// Stop recording.
		void stop();

		/// Prepare a replay.
		/// \param game[in] A pointer the the bdGame interface to which the
		///		recorded messages should be issued.
		bdBool startReplay(bdGame *const game);

		/// Stop a replay.
		void stopReplay();

		/// Issue recorded messages.
		/// Issue the next batch of recorded message to the bdGame interface
		/// specified by a preceding call to bdGameLog::startReplay.
		/// \return
		/// - True if there are more messages to issue.
		/// - False if all recorded messages have been issued.
		bdBool issueMessages();

		/// Get the bdPlayerManager used while replaying.
		/// To ensure replay messages don't interfere with a bdPeers
		/// player manager the bdGameLog uses an internal
		/// bdPlayerManager to handle any recorded player manager messages.
		/// \return A pointer to the replay bdPlayerManager.
		bdPlayerManager* getPlayerManager();

		/// Get the bdRound used while replaying.
		/// To ensure replay messages don't interfere with a bdPeers
		/// round the bdGameLog uses an internal bdRound to handle any
		/// recorded round messages.
		/// \return A pointer to the replay bdRound.
		bdRound* getRound();

	protected:

		virtual void logMessage(const bdDeliveryMessage& receivedMsg);

		virtual void onDeliveryEnd();

	protected:

		bdPeer *m_peer;

		/// The bdPlayerManager used while replaying.
		bdPlayerManager m_playerManager;

		/// The bdRound used while replaying.
		bdRound m_round;

		/// Buffer for initial replay state.
		bdBitBufferRef m_buffer;

		/// The recorded messages.
		bdArray<bdDeliveryMessage> m_messages;

		bdUInt m_numMessages;
		bdUInt m_numEmptyFrames;

		bdFastArray<bdUInt> m_messageCount;
		bdFastArray<bdUInt> m_emptyFrameCount;

		bdUInt m_numEmptyFramesPlayed;
		bdUInt m_counterIndex;
		bdUInt m_messageIndex;
};


#endif // BD_GAME_HISTORY_H

