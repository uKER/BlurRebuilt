// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A window to store updates in an ordered fashion.

#ifndef BD_UPDATE_WINDOW_H
#define BD_UPDATE_WINDOW_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdReplica/bdUpdate/bdUpdateSettings.h>

/// A window to store dirty flags of updates in an ordered fashion.
class bdUpdateWindow
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdUpdateWindow();

		/// Adds a dirtyFlags into the window. It is added to the
		/// end of the window.
		/// \param dirtyFlags[in] The dirtyFlags to be added
		void add(const bdUInt32 dirtyFlags);

		/// Checks whether dirty flags for an update with the given sequence number are in the window.
		/// \param updateNum[in]	The sequence number of the update.
		/// \return -True : the update is in the window
		///         -False: the update is not in the window
		bdBool inWindow(const bdUpdateSeqNum updateNum) const;

		/// Gets the number of dirty flags received since updateNumber
		/// \param updateNum[in] The sequence number of the last update
		/// \return : The number of update since updateNumber
		bdUpdateSeqNum getUpdatesSince(const bdUpdateSeqNum updateNum) const;

		/// Gets the dirty flags with the updateNumber
		/// \param updateNum[in] The sequence number of the update
		/// \return : The dirty flags in form of a bdUInt32 if it exists, 0 otherwise.
		bdUInt32 get(const bdUpdateSeqNum updateNum) const;

		/// Gets the sequence number of the most recent update
		/// \return The sequence number of the most recent update.
		bdUpdateSeqNum getUpdateNum() const;

	protected:	

		/// An array of size BD_UPDATE_HISTORY_SIZE to store dirty flags in form
		/// of bitbuffers.
		bdUInt32 m_dirtyFlags[BD_UPDATE_HISTORY_SIZE];

		/// The next sequence number to be used
		bdUpdateSeqNum m_nextUpdateNum;
};

#endif // BD_UPDATE_WINDOW_H
