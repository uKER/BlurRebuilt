// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A fixed size data buffer.

#ifndef BD_BYTE_BUFFER_H
#define BD_BYTE_BUFFER_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdContainers/bdBitBuffer.h>

BD_REFERENCE(bdByteBuffer);

#define BD_TYPED_ARRAY_HEADER_SIZE (10u)

/// A class for serializing/deserializing basic data types
class bdByteBuffer : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Specific size constructor.
		/// \param size[in]	The amount of memory to allocate to 
		///					this buffer.
		/// \param isTypeChecked[in] Flag to indicate whether all values are prefixed by a type value
		/// \note: size can be set to 0. In this case all write functions won't store the data, but the
		///        m_size member variable will be incremented to keep track of the required memory.
		///	       Then allocateBuffer() can be called to actually allocate a buffer big enough that can
		///        hold all previously written data elements.
		inline bdByteBuffer(const bdUInt size, bdBool isTypeChecked = false);

		/// Constructor
		/// \param bytes[in] Pointer to a memory block where the data is stored.
		/// \param size[in]	The size of the allocated memory block.
		/// \param isTypeChecked[in] Flag to indicate whether all values are prefixed by a type value
		/// \note This constructor will allocate memory of the specified size and then
		///       copy the data from the source buffer to the allocated buffer
		inline bdByteBuffer(void *bytes, const bdUInt size, bdBool isTypeChecked = false);

		/// Destructor.
		virtual ~bdByteBuffer();

		/// Allocates a buffer to store data elements. The size of the buffer is equal to the m_size member variable.
		/// Usage scenario for this function is:
		/// 1) Construct byte buffer of size 0
        /// 2) call write functions, this will keep track of the required memory
		/// 3) call allocateBuffer()
		/// 4) repeat calling write functions to actually store the data elements in the buffer
		void allocateBuffer();

		inline bdUByte8& operator[] (const bdUInt i);

		inline bdUByte8 operator[] (const bdUInt i) const;

		/// Get the size of this bytebuffer.
		/// \return The total size in bytes.
		inline bdUInt getSize() const;

		/// Get the currently used number of bytes.
		/// \return The number of used bytes in the buffer.
		bdUInt getDataSize() const;

		/// Get a pointer to the data section.
		/// \return A pointer to the data section of this buffer.
		inline bdUByte8* getData();

		/// Get a pointer to the data section, const version.
		/// \return A pointer to the data section of this buffer.
		inline const bdUByte8* getData() const;

		/// Get the maximum number of bytes that can be written to this buffer
		inline bdUInt getMaxWriteSize() const;

		/// Get the maximum number of bytes that can be read from this buffer
		inline bdUInt getMaxReadSize() const;
		
		/// Denote that the all writes subsequent to this call and proceeding the next call 
		/// to writeArrayEnd are to be sent in array form. Sending a number of values of the
		/// same type in an array is more efficient than sending them individually as in the 
		/// case of the array version the type information only has to be written once.
		bdBool writeArrayStart(bdUByte8 type, bdUInt32 numElements, bdUInt32 elementSize);

		/// Denote the end of an array section. This must be called if writeArrayStruct has 
		/// been called
		void writeArrayEnd();

		/// Read in an array of values of the expectedType
		bdBool readArrayStart(bdUByte8 expectedType, bdUInt32& numElements);

		/// Denote the end of an array section. This must be called if writeArrayStruct has 
		/// been called
		void readArrayEnd();
		
		/// Write a boolean value to the byte buffer.
		/// \param b[in] The boolean value to be written.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeBool(const bdBool b);

		/// Insert a native 8-bit character into the buffer.
		/// \param c[in]	The character to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeNChar8(const bdNChar8 c);

		/// Insert a signed byte into the buffer.
		/// \param b[in] The byte to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeByte8(const bdByte8 b);

		/// Insert an unsigned byte into the buffer.
		/// \param b[in] The byte to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeUByte8(const bdUByte8 b);

		/// Insert a signed 16-bit integer into the buffer.
		/// \param i[in]	The integer to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeInt16(const bdInt16 i);

		/// Insert an unsigned 16-bit integer into the buffer.
		/// \param u[in]	The integer to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeUInt16(const bdUInt16 u);

		/// Insert a signed 32-bit integer into the buffer.
		/// \param i[in]	The integer to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeInt32(const bdInt32 i);

		/// Insert an unsigned 32-bit integer into the buffer.
		/// \param u[in]	The integer to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeUInt32(const bdUInt32 u);

		/// Insert an signed 64-bit integer into the buffer.
		/// \param i[in]	The integer to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeInt64(const bdInt64 i);

		/// Insert an unsigned 64-bit integer into the buffer.
		/// \param u[in]	The integer to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeUInt64(const bdUInt64 u);

		/// Insert a 32-bit floating-point number into the buffer.
		/// \param f[in]	The floating-point number to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeFloat32(const bdFloat32 f);

		/// Insert a 64-bit floating-point number into the buffer.
		/// \param f[in]	The floating-point number to insert.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeFloat64(const bdFloat64 f);
		
		/// Insert a null-terminated string into the buffer.
		/// \param s[in] Pointer to the string.
		/// \param maxLen[in] Maximum length of the string (including the
		///					  null terminator).
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		/// \note  Using a  null string pointer will result in a NaN value
		///        being written to the buffer.
		bdBool writeString(const bdNChar8 *const s, 
								const bdUWord maxLen);

		/// Insert binary data into the buffer
		/// \param blob[in] A reference to the data to insert.
		/// \param length[in] The length of the blob.
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeBlob(const void *const blob,
							  const bdUInt32 length);

		/// Insert a NaN type into the buffer.
		/// \note This should not be used on non type checked byte buffers!	
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeNAN();

		/// Insert a no type into the buffer.
		/// \note This should ONLY be used as the last element in the buffer, 
		///       to indicate that no more data is left..!	
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool writeNoType();

		template <typename U>
		bdBool write(const U &var)
		{
			U evar;
			bdBitBuffer::endianSwap(var, evar);
			return write(&evar, sizeof(evar));
		}

		/// Write an arbitrary number of bytes.
		/// \param data[in] Pointer to the data block
		/// \param size[in] Number of bytes to write
		/// \return True : The value was written successfully.
		///			False: The value could not be written (buffer overrun)
		bdBool write(const void *data, const bdUInt size);
		
		template <typename U>
		bdBool read(U &var)
		{
			U temp;
			const bdBool result = read(&temp, sizeof(temp));
			if(true == result)
			{
				bdBitBuffer::endianSwap(temp, var);
			}
			return result;
		}

		bdBool read(void *data, bdUInt size);

		/// Extract a boolean from the buffer.
		/// \param b[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readBool(bdBool &b);


		/// Extract a native 8-bit character from the buffer.
		/// \param c[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the operation was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readNChar8(bdNChar8 &c);

		/// Extract a signed byte from the buffer.
		/// \param b[out] The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readByte8(bdByte8 &b);

		/// Extract an unsigned byte from the buffer.
		/// \param b[out] The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readUByte8(bdUByte8 &b);

		/// Extract a signed 16-bit integer from the buffer.
		/// \param i[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readInt16(bdInt16 &i);

		/// Extract an unsigned 16-bit integer from the buffer.
		/// \param u[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readUInt16(bdUInt16 &u);

		/// Extract a signed 32-bit integer from the buffer.
		/// \param i[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		/// \todo Explain why this takes a bdInt rather than a bdInt32
		bdBool readInt32(bdInt &i);

		/// Extract an unsigned 32-bit integer from the buffer.
		/// \param u[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		/// \todo Explain why this takes a bdUInt rather than a bdUInt32
		bdBool readUInt32(bdUInt &u);

		/// Extract an signed 64-bit integer from the buffer.
		/// \param i[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readInt64(bdInt64 &i);

		/// Extract an unsigned 64-bit integer from the buffer.
		/// \param u[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readUInt64(bdUInt64 &u);

		/// Extract 32-bit floating-point number from the buffer.
		/// \param f[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readFloat32(bdFloat32 &f);

		/// Extract 64-bit floating-point number from the buffer.
		/// \param f[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readFloat64(bdFloat64 &f);

		/// Determine the length of a string in the buffer.
		/// \param length
		bdBool getStringLength(bdUInt &length);

		/// Extract a null-terminated character string from the buffer.
		/// \param s[in]	A pointer to the destination buffer. If \a s is BD_NULL
		///					nothing is read from the buffer but \a length is set.
		/// \param maxLen[in] The maximum size of the string. The actual size
		///					  of the string may be less than maxLen characters.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read, or there was a type mismatch.
		bdBool readString(bdNChar8 *const s,
						  const bdUWord maxLen);


		/// Extract a binary blob data from the buffer.
		/// \param blob[in] 		A pointer to the destination buffer, may be NULL.
		/// \param length[in|out] 	The size of the destination buffer. Upon return, this
		/// 						contains the length of the blob in the buffer, in bytes.
		///							Regardless of \a length the entire blob is read from the buffer.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, otherwise.
		bdBool readBlob(bdUByte8 *const blob,
							   bdUInt32& length);

		/// Extract a NaN type from the buffer.
		/// \note This should not be used on non type checked byte buffers!	
		bdBool readNAN();

		/// En- or disable type checking
		void setTypeCheck(const bdBool flag);

		// FIXME: It seems dangerous to use invoke this on a non type checked byte buffer
		bdBitBufferDataType readDataType();
		
		/// Reset the read and write positions to the beginning of the data segment.
		void reset();

	protected:

		/// If the buffer is type checked, this function writes paramType to the buffer.
		/// This is used internally in bdBitBuffer by all write functions except writeBits.
		/// \param dataType[in]		The type of parameter that will be written to the buffer
		///							immediately after this function.
		bdBool writeDataType(const bdBitBufferDataType dataType);

		/// If the buffer is type checked, this function reads the parameter type from the buffer
		///  and throws an error if the read bdParamType doesn't match the expected bdParamType.
		///  This is used internally in bdBitBuffer by all read functions except readBits.
		///  Outputs an error to BD_LOG_LEVEL "bdCore/bitBuffer" if expected type doesn't match read
		///  type.
		/// \param expectedDataType[in]		The expected bdParamType.
		/// \return
		///	-	True, if the read was successful, and it found a bdParamType.
		/// -	False, if the next byte read doesn't match an existing bdParamType.
		bdBool readDataType(const bdBitBufferDataType expectedDataType);


		/// Actual size for the byte buffer
		bdUInt m_size;

		/// Pointer to the byte array
		bdUByte8 *m_data;

		/// Current readposition
		bdUByte8 *m_readPtr;

		/// Current write position
		bdUByte8 *m_writePtr;

		/// Type checked flag
		bdBool	m_typeChecked;

		/// Type checked flag
		bdBool	m_typeCheckedCopy;
};

#include <bdCore/bdContainers/bdByteBuffer.inl>

#endif // BD_BYTE_BUFFER_H
