// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An interface used to enable object interpolation.

#ifndef BD_INTERPOLATION_INTERFACE_H
#define BD_INTERPOLATION_INTERFACE_H

#include <bdCore/bdCore.h>


// Highest order bit of the dirty flag which is used to indicate
// a interpolation break.
#define BD_INTERPOLATION_BREAK static_cast<bdUInt32>(1 << 31u)

#include <bdClientServer/bdCommand/bdUserCommand.h>

/// An interface used to enable object interpolation.
/// Any class that inherits from bdInterpolationInterface can be used
/// as an interpolation target. Typically this is just another copy
/// of a class derived from a bdReplica, but this may not be optimal
/// (in memory) as the bdReplica derived class may contain data that is not
/// required to perform interpolation. If this is the case you can derive your
/// interpolation target directly off bdInterpolationInterface rather than
/// use your bdReplica derived class.
class bdInterpolationInterface
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constuctor.
		bdInterpolationInterface();

		/// Virtual destructor.
		virtual ~bdInterpolationInterface();

		/// Applies a user command to the object.
		/// This function should be overridden to update the objects
		/// state in response to a user command. The command may come over
		/// the network and be a local cache of an old command.
		/// \note You may call this function directly on a class which
		/// inherits from bdReplica, but it is recommended that you call
		/// bdReplica::processCommand instead.
		/// \param command[in]	The command to be applied to the object.
		/// \param newCommand[in]
		/// - True if this is a new command.
		///	- False if the command has been applied before.
		virtual void applyCommand(bdUserCommand *const command,
								  const bdBool newCommand);

		/// Can be overridden in order to perform extrapolation on the
		/// object.
		/// \param secs[in]		The amount of time for which this object should be
		///						extrapolated.
		/// \param isReplay[in] Flag to indicate whether this extrapolation is
		///						performed as part of a replay of previous commands
		virtual void extrapolate(const bdFloat32 secs,
								 const bdBool isReplay);

		/// Serializes the object's state into a bitbuffer.
		/// \param	buffer[in]	The bdBitBuffer into which the state is
		///						serialized.
		virtual void serialize(bdBitBufferRef buffer);

		/// Deserializes the object's state from a bitbuffer.
		/// \param	buffer[in]	The bdBitBuffer from which the state is
		///						deserialized.
		/// \return
		/// - True if the state could be read succesfully.
		/// - False otherwise.
		virtual bdBool deserialize(bdBitBufferRef buffer);

		/// Sets the dirty flags.
		/// The current set of dirty flags are bitwise or'ed with \a flags.
		/// \param flags[in] The new flags to be set.
		void setDirtyFlags(const bdUInt32 flags);

		/// Gets the current set of dirty flags.
		/// \return The current set of dirty flags.
		bdUInt32 getDirtyFlags() const;

		/// Checks the dirty flags.
		/// Tests a set of \a flags against the current set of dirty flags.
		/// \param flags[in] The set of dirty flags to check.
		/// \return
		/// - True  If any of the flags specified by \a flags are set.
		///	- False If none of the flags specified by \a flags are set.
		bdBool testDirtyFlags(const bdUInt32 flags = BD_UINT32_MAX) const;

		/// Clears the specified dirty flags.
		/// Any bits set in  \a flags are cleared in the dirty flags.
		/// \param flags[in]	The set of dirty flags to be cleared.
		void clearDirtyFlags(const bdUInt32 flags = BD_UINT32_MAX);

		/// Write the entire serializable object state to \a buffer.
		/// This is achieved by temporarily setting all dirty flags
		/// and calling serialize. The original dirty flags are then
		/// reinstated.
		/// \param buffer[out]	A reference to a bdBitBuffer to which
		///						the full state is written.
		void writeFullState(bdBitBufferRef buffer);

	protected:

		/// A set of flags to be used for delta compression of messages.
		bdUInt32 m_dirtyFlags;
};

#endif // BD_INTERPOLATION_INTERFACE_H
