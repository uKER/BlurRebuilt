// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Provides utilities for conveniently packing byte-based data into buffers.

#ifndef BD_BYTE_PACKER_H
#define BD_BYTE_PACKER_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdBitBuffer.h>

/// Utility operations on data packed into byte buffers.
/// This class aids in packing data fields into byte buffers. For example, creating
/// network packets.
///
/// The basic operation provided by this class is that of reading or writing bytes
/// to a byte buffer.
class bdBytePacker
{
	public:

		/// Writes a variable of a plain data type into a buffer.
		/// This function writes a fundamental type variable into a buffer. Endianness
		/// conversion may be performed on the variable. Control parameters are used to
		/// protect against buffer overruns, if there is not enough room for the variable 
		/// to be written, or the buffer is BD_NULL, the buffer is left untouched. \a newOffset is
		/// always incremented so that desired buffer sizes can be calculated.
		/// \note To write out structures or arrays, use \c appendBuffer().
		/// \param buffer[in]	Pointer to the start of the output buffer or BD_NULL. 
		///						If this value is BD_NULL the buffer is not touched.
		/// \param bufferSize[in] The length of the buffer in bytes.
		/// \param offset[in]	The offset in bytes from the start of the buffer at which the value 
		///						of \a var is to be written.
		/// \param newOffset[out] Set to the indicate the new \a offset value for subsequent calls. 
		///						  \a newOffset is always set.
		/// \param var[in] The variable to write. Must be a plain data type (e.g. integer, float etc).
		/// \return
		///  - True if the operation was successful, or \a buffer is BD_NULL.
		///  - False otherwise.
		template <typename T>
		static bdBool BD_CALL appendBasicType(void* buffer, 
											  const bdUInt bufferSize,
											  const bdUInt offset, 
											  bdUInt &newOffset, 
											  const T &var)
		{
			T nvar;
			bdBitBuffer::endianSwap(var, nvar);
			const bdBool written = appendBuffer(buffer, 
												bufferSize, 
												offset, 
												newOffset,
												&nvar, 
												sizeof(T));
			return written;
		}

		/// Writes an array of bytes into a buffer.
		/// Control parameters are used to protect against buffer overruns, if there is not enough 
		/// room for the entire source buffer to be written, or either source or destination buffers 
		/// are BD_NULL, the destination buffer is left untouched. \a newOffset is
		/// always incremented so that desired buffer sizes can be calculated.
		/// \note To write fundamental types, use \a appendBasicType
		/// \param dest[in]	Pointer to the start of the output buffer or BD_NULL. 
		///					If this value is BD_NULL the buffer is not touched.
		/// \param destSize[in] The length of the buffer pointed to by \a dest in bytes.
		/// \param offset[in]	The offset in bytes from the start of the \a dest at 
		///						which the data from \a src is to be written.
		/// \param newOffset[out]	Set to the indicate the new \a offset value for subsequent calls. 
		///							\a newOffset is always set.
		/// \param src[in] Pointer to the start of the input buffer.
		/// \param writeSize[in]	The number of bytes to copy from \a src to \a dest. \a src must be 
		///							at least writeSize bytes.
		/// \return
		///  - True if the operation was successful, or if either \a dest or \a src is BD_NULL.
		///  - False otherwise.
		static bdBool BD_CALL appendBuffer(void* dest, 
										   const bdUInt destSize,
										   const bdUInt offset, 
										   bdUInt &newOffset, 
										   const void *const src, 
										   const bdUInt writeSize);
		
		/// Writes a unsigned 16 bit value to a buffer using only 8 bits if possible.
		/// Writes a 16 bit value, that must less that 0x8000 (32768) to a buffer using one 
		/// or two bytes. Values in the range [0, 127] are written using 1 byte and values in 
		/// the range  [128, 32767] are written using 2 bytes.
		/// Control parameters are used to protect against buffer overruns, if there is not 
		/// enough room for the value to be written, or the \a buffer is BD_NULL, the buffer is left 
		/// untouched. \a newOffset is always incremented so that desired buffer sizes can be 
		/// calculated.
		/// Values written using \a appendEncodedUInt16 should be read using \a removeEncodedUInt16.
		/// \param buffer[in]	Pointer to the start of the output buffer or BD_NULL. 
		///						If this value is BD_NULL the buffer is not touched.
		/// \param bufferSize[in] The length of \a buffer in bytes.
		/// \param offset[in]	The offset in bytes from the start of \a buffer at which the 
		///						encoding of \a value is to be written.
		/// \param newOffset[out] Set to the indicate the new \a offset value for subsequent calls. 
		///						  \a newOffset is always set.
		/// \param value[in] The 16 bit variable to encode and write.
		/// \return
		///  - True if the operation was successful, or \a buffer is BD_NULL.
		///  - False otherwise.
		static bdBool BD_CALL appendEncodedUInt16(void* buffer,
												  const bdUInt bufferSize,
												  const bdUInt offset,
												  bdUInt &newOffset,
												  const bdUInt16 value);

		/// Reads the value of a plain data type variable from a buffer.
		/// This function reads a fundamental type value from a buffer. Endianness
		/// conversion may be performed on the variable. Control parameters are used to
		/// protect against buffer overruns, if there is not enough room for the variable 
		/// to be read, or the buffer is BD_NULL, the value is not read. \a newOffset is
		/// always incremented.
		/// \note To read structures or arrays, use \c removeBuffer().
		/// \param buffer[in]	Pointer to the start of the input buffer or BD_NULL. 
		///						If this value is BD_NULL nothing is read from the buffer.
		/// \param bufferSize[in] The length of \a buffer in bytes.
		/// \param offset[in]	The offset in bytes from the start of \a buffer from which the value 
		///						of \a var is to be read.
		/// \param newOffset[out] Set to the indicate the new \a offset value for subsequent calls. 
		///						  \a newOffset is always set.
		/// \param var[out] The variable to receive the value being read. 
		///					Must be a plain data type (e.g. integer, float etc).
		/// \return
		///  - True if the operation was successful, or \a buffer is BD_NULL.
		///  - False otherwise.		
		template <typename T>
		static bdBool BD_CALL removeBasicType(const void* buffer, 
											  const bdUInt bufferSize,
											  const bdUInt offset, 
											  bdUInt &newOffset, 
											  T &var)
		{
			T nvar;
			const bdBool read = removeBuffer(buffer, 
											 bufferSize, 
											 offset, 
											 newOffset,
											 &nvar,
											 sizeof(T));			
			if(read)
			{
				bdBitBuffer::endianSwap(nvar, var);
			}

			return read;
		}

		/// Reads an array of bytes from a buffer.
		/// Control parameters are used to protect against buffer overruns, if there is not enough 
		/// room for the entire buffer to be read, or the either the source or destination buffer 
		/// is BD_NULL, the destination buffer is left untouched. \a newOffset is
		/// always incremented.
		/// \note To read fundamental types, use \a removeBasicType
		/// \param src[in]	Pointer to the start of the input buffer or BD_NULL. 
		///					If this value is BD_NULL nothing is read.
		/// \param srcSize[in] The length of the \a src buffer in bytes.
		/// \param offset[in]	The offset in bytes from the start of the \a src buffer at 
		///						which the read should begin.
		/// \param newOffset[out]	Set to the indicate the new read \a offset value for subsequent calls. 
		///							\a newOffset is always set.
		/// \param dest[in] Pointer to the start of the destination buffer or BD_NULL.
		///					If this value is BD_NULL nothing is read.
		/// \param readSize[in]	The number of bytes to copy from \a src to \a dest. \a dest must be 
		///						at least writeSize bytes.
		/// \return
		///  - True if the operation was successful, or if either \a src or \a dest are BD_NULL.
		///  - False otherwise.
		static bdBool BD_CALL removeBuffer(const void* src, 
										   const bdUInt srcSize,
										   const bdUInt offset, 
										   bdUInt &newOffset, 
										   void *const dest, 
										   const bdUInt readSize);

		/// Reads a unsigned 16 bit value from a buffer.
		/// Reads a 16 bit value that have be written with \a appendEncodedUInt16.
		/// Control parameters are used to protect against buffer overruns, if there is not 
		/// enough room for the value to be read, or the buffer is BD_NULL, the value is not read. 
		/// \a newOffset is always incremented by the number of bytes read.
		/// \param buffer[in]	Pointer to the start of the input buffer or BD_NULL. 
		///						If this value is BD_NULL nothing is read from the buffer.
		/// \param bufferSize[in] The length of the buffer in bytes.
		/// \param offset[in]	The offset in bytes from the start of \a buffer at which the 
		///						encoded value should be read.
		/// \param newOffset[out] Set to the indicate the new \a offset value for subsequent calls. 
		///						  \a newOffset is always set.
		/// \param value[out] The 16 bit variable that receives the decoded value.
		/// \return
		///  - True if the operation was successful, or \a buffer is BD_NULL.
		///  - False otherwise.
		static bdBool BD_CALL removeEncodedUInt16(const bdUByte8* buffer,
												  const bdUInt bufferSize,
												  const bdUInt offset,
												  bdUInt &newOffset,
												  bdUInt16 &value);

		/// Skips forward a number of bytes in a buffer.
		/// \a newOffset is set to the new offset after the specified number of bytes has been skipped. 
		/// The return value indicates if this offset is within the buffer. No read or write operations 
		/// are performed on the buffer.
		/// \param buffer[in] Pointer to the start of the buffer.
		/// \param bufferSize[in] Length of the buffer in bytes.
		/// \param offset[in] Offset from the start of the buffer from which to skip forward.
		/// \param newOffset[out]	The new value of \a offset moved by specified number of bytes. 
		///							This value is always set.
		/// \param bytes[in] Number of bytes to skip.
		/// \return
		///  - True if the new offset if within the buffer.
		///  - False otherwise.
		static bdBool BD_CALL skipBytes(const void *const buffer, 
										const bdUInt bufferSize,
										const bdUInt offset, 
										bdUInt &newOffset, 
										const bdUInt bytes);

		/// Rewinds backwards a number of bytes in a buffer.
		/// \a newOffset is set to the new offset after the specified number of bytes has been rewound. 
		/// The return value indicates if this offset is within the buffer. No read or write operations 
		/// are performed on the buffer.
		/// \param buffer[in] Pointer to the start of the buffer.
		/// \param bufferSize[in] Length of the buffer in bytes.
		/// \param offset[in] Offset from the start of the buffer from which to rewind.
		/// \param newOffset[out]	The new value of \a offset moved by specified number of bytes. 
		///							This value is always set.
		/// \param bytes[in] Number of bytes to rewind.
		/// \return
		///  - True if the new offset if within the buffer.
		///  - False otherwise.
		static bdBool BD_CALL rewindBytes(const void *const buffer, 
										  const bdUInt bufferSize,
										  const bdUInt offset, 
										  bdUInt &newOffset, 
										  const bdUInt bytes);

};

#endif // BD_BYTE_PACKER_H
