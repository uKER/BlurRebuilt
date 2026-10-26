// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Get textual information describing a sec id or key.

#ifndef BD_SECURITY_INFO_H
#define BD_SECURITY_INFO_H

#include <bdCore/bdSocket/bdSecurityID.h>
#include <bdCore/bdSocket/bdSecurityKey.h>


#define BD_SECURITY_ID_STRING_LENGTH (BD_SECURITY_ID_LENGTH*2 + (BD_SECURITY_ID_LENGTH/4))
#define BD_SECURITY_KEY_STRING_LENGTH (BD_SECURITY_KEY_LENGTH*2 + (BD_SECURITY_KEY_LENGTH/4))

/// Defines safe static functions for converting a sec id or key to a string 
/// representation.
/// These functions ought to be used whenever a sec id or key needs to printed 
/// to bdLog output
class bdSecurityInfo
{
	public:
	
		/// Prints the sec id to the supplied buffer.
		/// \param id[in] The sec id to print.
		/// \param buffer[in] The buffer to print the id to.
		/// \param len[in] The size of the passed in buffer.
		/// \return 
		///  - If greater than 0 an integer representing how much data was 
		///    written to the buffer.
		///  - If equal to 0 an indication that there was not enough space in 
		///    the buffer to write the data.
		static bdUInt BD_CALL toString(	const bdSecurityID &id, 
										bdNChar8* buffer, 
										bdUInt len);

		/// Prints the sec key to the supplied buffer.
		/// \param key[in] The sec key to print.
		/// \param buffer[in] The buffer to print the id to.
		/// \param len[in] The size of the passed in buffer.
		/// \return 
		///  - If greater than 0 an integer representing how much data was 
		///    written to the buffer.
		///  - If equal to 0 an indication that there was not enough space in 
		///    the buffer to write the data.
		static bdUInt BD_CALL toString(	const bdSecurityKey &key, 
										bdNChar8* buffer, 
										bdUInt len);

	protected:

		/// Prints the data from buffer in a human readable format in 
		/// outBuffer.
		/// \param buffer[in] The buffer containing the data.
		/// \param bufferLen[in] The length of <i> buffer. </i>
		/// \param outBuffer[out] The buffer that the human readable data will
		///                       be written to.
		/// \param outBufferLen[in] The length of <i> outBuffer. </i>
		/// \return 
		///  - If greater than 0 an integer representing how much data was 
		///    written to the <i> outBuffer. </i> 
		///  - If equal to 0 an indication that there was not enough space in 
		///    the <i> outBuffer. </i> to write the data.
		static bdUInt BD_CALL bufferToString(const bdUByte8* buffer,
											 const bdUInt bufferLen,
											 bdNChar8* outBuffer,
											 const bdUInt outBufferLen);

		/// A protected constructor prevents this class from being initialized.
		bdSecurityInfo();
};



#endif // BD_SECURITY_INFO_H

