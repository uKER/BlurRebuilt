// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE:

#ifndef BD_BANDWIDTH_TEST_RESULTS_H
#define BD_BANDWIDTH_TEST_RESULTS_H

#include <bdCore/bdCore.h>

/// Stores the results of a bandwidth test.
/// Instances of this class are returned by the bdBandwidthTestClient after
/// a successful test.
class bdBandwidthTestResults
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdBandwidthTestResults();

		/// Resets the results back to those set the the constructor.
		void clear();

		/// Write the content of this class to a buffer.
		/// Used to sent the client side results of a test to the server.
		/// \param buffer[in]	A pointer to an array of bytes to which the the 
		///						results should be written. The array should be at least
		///						sizeof(bdBandwidthTestResults).
		/// \param bufferSize[in]	The size of the data pointed to \a buffer in bytes.
		/// \return	
		/// - true, if the entire class was successfully written to the buffer.
		/// - false, otherwise. This usually indicates that \a bufferSize is too small.
		bdBool serialize(bdUByte8 *const buffer, 
						 const bdUInt bufferSize) const;

		/// Set the contents of this class from a buffer.
		/// Used to read the server side results of a test sent from the server.
		/// \param buffer[in]	A reference counted buffer containing the data to read
		///						into this class instance.
		/// - true, if the entire class was successfully read from the buffer.
		/// - false, otherwise. This usually indicates that \a buffer is too small.
		bdBool deserialize(const bdByteBufferRef buffer);

		/// Calculate the bandwidth in Kilobits per second.
		/// \return the bandwidth in Kilobits per second (Kbps).
		bdFloat32 getBandwidth() const;

	public:

		/// The number of bytes received.
		bdUInt32 m_bytesReceived;

		/// The time over which the data was received.
		bdUInt32 m_receivePeriodMs;

		/// The average sequence of all received packets.
		bdUInt32 m_avgSeqNumber;

		/// The minimum sequence of all received packets.
		bdUInt32 m_minSeqNumber;

		/// The maximum sequence of all received packets.
		bdUInt32 m_maxSeqNumber;
};

#endif // BD_BANDWIDTH_TEST_RESULTS_H
