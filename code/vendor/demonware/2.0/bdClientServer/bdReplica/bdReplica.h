// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The basic replicated object in BitDemon.

#ifndef BD_REPLICA_H
#define BD_REPLICA_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdReplica/bdReplicaTypes.h>
#include <bdClientServer/bdReplica/bdInterpolationInterface.h>
#include <bdClientServer/bdReplica/bdUpdate/bdUpdateSettings.h>
#include <bdClientServer/bdReplica/bdUpdate/bdUpdateWindow.h>
#include <bdClientServer/bdCommand/bdUserCommand.h>

class bdCommandWindow;
class bdClientReplicaData;
class bdServerReplicaData;

BD_REFERENCE(bdReplica);
BD_REFERENCE(bdConnection);

/// The basic replicated object in BitDemon.
/// Used as a building block for all replicated objects.
class bdReplica : public bdReferencable,
				  public bdInterpolationInterface
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// A set of flags to indicate the usage settings of a replica class.
		/// BD_DO_DELTA_COMPRESSION: indicates whether updates from the server are
		/// delta compressed (DEFAULT: True).
		/// Overwrite the getReplicaSettings() function to enable/disable a specific
		/// feature.
		enum bdReplicaSettings
		{
			BD_DO_DELTA_COMPRESSION      = (1 << 0),
			BD_MAX_REPLICA_SETTING       = (1 << 1)
		};

		/// Default constructor.
		/// Initialises the replica to be of type BD_REPLICA, derived classes
		/// should change this to the appropriate type registered with the
		/// BD_REGISTER_CLASS macro. This enables the remote creation of derived
		/// objects through bdReplicaFactory.
		bdReplica();

		/// Virtual destructor.
		/// Releases any memory held by the replica.
		virtual ~bdReplica();

		/// Sets the bdReplicas id.
		/// \param id[in] The object's object id.
		void setId(const bdReplicaId id);

		/// Gets the bdReplicas id.
		/// \return The object's object id.
		const bdReplicaId& getId() const;

		/// Gets the bdReplicas type.
		/// \return The object's type.
		bdReplicaType getType() const;

		/// Final initialization of a remotely created object.
		/// The default implementation of this function is empty. Overload it
		/// in derived classes in order to set state information that may
		/// not come from the client.
		virtual void initialize();

		/// Applies a user command to the object and its interpolation target.
		/// Will call applyCommand on this object directly and also
		/// on its interpolation target if it exists and is active.
		/// \param command[in]	The command to be applied.
		/// \param newCommand[in]
		/// - True if this is a new command.
		///	- False if the command has been applied before.
		void processCommand(bdUserCommand *const command,
							const bdBool newCommand);

		/// Steps the replica forward in time. If this replica is owned by his
		/// game instance, or if it is a primary on a server, step records the
		/// amount of extrapolation time performed on it.
		/// For secondary replicas it calculates the next interpolation step.
		/// \param secs[in]		The amount of time in seconds the replica should be
		///						advanced.
		/// \param doExtrapolation[in] Flag to indicate whether the replica should be
		///						extrapolated
		void step(const bdFloat32 secs,
				  const bdBool doExtrapolation);

		/// Creates an update of data needed to initialize a remote instance
		/// of this class.
		/// \param buffer[out] The buffer to write the data to.
		virtual void writeReplicaInfo(bdBitBufferRef buffer);

		/// Takes a buffer of initial state data used to initialize the
		/// replica.
		/// \param buffer[in] The buffer with the state info.
		virtual void readReplicaInfo(bdBitBufferRef buffer);

		/// Creates data needed to maintain the replica on the server.
		/// This includes a set of all hosts to who have a copy of this replica.
		/// It can also include windows for delta updating clients.
		/// The actual data created depends the replicas settings.
		/// \return A pointer to the newly created primary replica data.
		bdServerReplicaData* createServerData();

		/// Retrieves a pointer to the replicas server side data.
		/// \return
		/// - A pointer to the replicas server data.
		/// - BD_NULL, if no such data exists.
		bdServerReplicaData* getServerData();

		/// Creates data needed to maintain the replica on a client.
		/// This data can include storage needed for backed-up object states
		/// when using for delta updates.
		/// The actual data created depends the replicas settings.
		/// \return A pointer to the newly created client data.
		bdClientReplicaData* createClientData();

		/// Retrieves a pointer to the replicas client data.
		/// \return
		/// - A pointer to the replicas client data.
		/// - BD_NULL, if no such data exists.
		bdClientReplicaData* getClientData();

		/// Writes an update about this bdReplica to be sent to a given
		/// \a host. The \a host parameter is only used/required if the
		/// object is set to used delta compressed update messages
		/// (BD_DO_DELTA_COMPRESSION).
		/// \param connection[in] The host to create the update for.
		/// \param buffer[in,out] A reference to a buffer into which the
		///						  update should be written.
		bdBool createUpdate(bdConnectionRef connection,
							bdBitBufferRef buffer);

		/// Handles the reception of an update.
		bdBool receiveUpdate(bdBitBufferRef buffer,
							 bdUpdateSeqNum& updateSeqNum);

		/// Set settings for this replica.
		/// \param settings[in] Bitfield flag representing the settings.
		void setSettings(const bdUInt32 settings);

		/// Clear settings for this replica.
		/// \param settings[in] Bitfield flag representing the settings.
		void clearSettings(const bdUInt32 settings);

		/// Checks the settings of this replica, ie. whether it makes use
		/// of strict/non-strict extrapolation and/or delta compression.
		/// In order to switch off a particular feature, set the value
		/// of m_replicaSettings accordingly in the constructor of the
		/// derived class.
		/// \param settings[in] : The settings to check
		/// \return
		///  - True if the setting is enabled
		///  - False if the setting is disabled
		bdBool testSettings(const bdUInt32 settings) const;

		/// Writes the object's state into a bitbuffer.
		/// \param buffer[in,out] The bitbuffer into which the state is written.
		virtual void serialize(bdBitBufferRef buffer);

		/// Reads the object's state from a bitbuffer
		/// \param buffer[in] The bitbuffer from which the state is read.
		/// \return True if the state could be read succesfully, false otherwise
		virtual bdBool deserialize(bdBitBufferRef buffer);

	protected:

		/// Create a delta update for 'host' in 'buffer'.
		/// A delta update is 'delta'ed against previously sent updates
		/// that have been acknowledged by the host.
		/// \param connection[in]	The host who will receive the update
		/// \param buffer[out]	The buffer to put the update in.
		/// \return
		///  - True if the update was successfully created.
		///  - False otherwise.
		bdBool createDeltaUpdate(bdConnectionRef connection,
								 bdBitBufferRef buffer);

		/// Create a non delta update for 'host' in 'buffer'.
		/// A non delta update is a self contained update.
		/// \param connection[in]	The host who will receive the update
		/// \param buffer[out]	The buffer to put the update in.
		/// \return
		///  - True if the update was successfully created.
		///  - False otherwise.
		bdBool createNonDeltaUpdate(bdConnectionRef connection,
									bdBitBufferRef buffer);

		/// Process a delta update.
		/// \param buffer[in]				The buffer containing the update.
		/// \param target[in]				A pointer to the interpolation target.
		/// \param updateSeqNum[in]			The sequence number of the update.
		/// \param interpolationBreak[in]	Flag to indicate if this is an
		///									interpolation break.
		/// \return
		///  - True if the update was correctly processed.
		///  - False otherwise.
		bdBool receiveDeltaUpdate(bdBitBufferRef buffer,
								  bdInterpolationInterface *const target,
								  bdUpdateSeqNum& updateSeqNum,
								  bdBool& interpolationBreak);

		/// Process a non delta update.
		/// \param buffer[in]				The buffer containing the update.
		/// \param target[in]				A pointer to the interpolation target.
		/// \param updateSeqNum[in]			The sequence number of the update.
		/// \param interpolationBreak[in]	Flag to indicate if this is an
		///									interpolation break.
		/// \return
		///  - True if the update was correctly processed.
		///  - False otherwise.
		bdBool receiveNonDeltaUpdate(bdBitBufferRef buffer,
									 bdInterpolationInterface *const target,
									 bdUpdateSeqNum& updateSeqNum,
									 bdBool& interpolationBreak) const;

		/// Performs interpolation on the replica.
		/// Should be overridden in derived classes to implement interpolation.
		/// \param targetState[in] The replica that contains the actual target state.
		/// \param targetRatio[in] A value between 0.0 and 1.0 to indicate how
		/// far towards the target state the replica should be interpolated.
		virtual void interpolate(bdInterpolationInterface *const targetState,
								 const bdFloat32 targetRatio);

	protected:

		/// The id of this bdReplica.
		/// The bdReplicaId is generated by the the server.
		bdReplicaId m_id;

		/// The id of this bdReplica.
		/// The bdReplicaType should be set in the bdReplica constructor.
		bdReplicaType m_type;

		/// Store the settings for this replica.
		bdUInt32 m_settings;

		/// A pointer to the server data for this replica.
		bdServerReplicaData *m_serverData;

		/// A pointer to the client data for this replica.
		bdClientReplicaData *m_clientData;
};

/// Class to intelligently compare replica refs, mostly used in sorted containers.
class bdReplicaRefLessThan
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Comparison functor for instances of bdReplicaRef.
		/// \param x[in]	The first replica ref.
		/// \param y[in]	The second replica ref.
		/// return
		///  - True if x is 'less than' y.
		///  - False otherwise.
		inline bool operator() (const bdReplicaRef x,
								const bdReplicaRef y) const
		{
			// we will compare the object id of the 2 replicas
			// although this is potentially non deterministic
			return x->getId() < y->getId();
		}
};

#include <bdClientServer/bdReplica/bdReplica.inl>

#endif //BD_REPLICA_H

