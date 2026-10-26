// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A variable sized FIFO buffer, which stores data in the minimum
// number of bits.

#ifndef BD_BIT_BUFFER_H
#define BD_BIT_BUFFER_H

#include <bdCore/bdString/bdString.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdContainers/bdFastArray.h>

#define BD_BIT_BUFFER_TYPES_BEGIN	enum bdBitBufferDataType {
#define BD_BIT_BUFFER_TYPE(x, y)	x,
#define BD_BIT_BUFFER_TYPES_END(x)	BD_BB_MAX_TYPE = x };

#include <bdCore/bdContainers/bdBitBufferTypes.h>

#undef BD_BIT_BUFFER_TYPES_BEGIN
#undef BD_BIT_BUFFER_TYPE
#undef BD_BIT_BUFFER_TYPES_END

#define BD_BB_TYPE_MAX_STRING_LENGTH 40

#define BD_BB_NUM_HEADER_BITS (1) // one bit for type checking

BD_REFERENCE(bdBitBuffer);

/// A variable sized FIFO buffer.
/// This buffer class stores data, which can inserted and extracted as
/// fundamental types or as an arbitrary amount of binary data. A datum is
/// stored in only the required number of bits required to represent its
/// precision. For example, a datum inserted as a boolean is represented by one
/// bit.
/// No type information is stored in the buffer, therefore insertion and
/// extraction operation must be performed carefully and symmetrically. In
/// particular, data should be read out in the same order that they were
/// written with, and with matching read and write functions.
/// This class is useful for serialization operations that require optimal
/// memory and CPU performance.
/// The first bit in the bitBuffer indicates whether or not the bitBuffer is type checked.
class bdBitBuffer : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		//
		// Construction and destruction.
		//

		/// Specific capacity constructor.
		/// Initialize the buffer with \a capacityBits initial storage
		/// capacity.
		/// \param capacityBits[in]	The initial capacity for the buffer,
		///							expressed in bits.
		/// \param typeChecked[in]	Flag to indicate if the data in the buffer
		///                         is type checked.
		bdBitBuffer(const bdUInt capacityBits = 0,
			        const bdBool typeChecked = false);

		/// Specific value constructor.
		/// Initialize the buffer to contain the same data as \a bits, with
		/// capacity \a numBits. \a numBits should be the size of the data in
		/// the \a bits array.
		/// \param bits[in]		An array containing the data to initialize the
		///						buffer with.
		/// \param numBits[in]	The size of the data (number of bits) in the
		///						\a bits array.
		/// \param dataHasTypeCheckedBit[in] Flag to indicate if the supplied
		///                                  array is type checked.
		bdBitBuffer(const bdUByte8 *bits,
					const bdUInt numBits,
					const bdBool dataHasTypeCheckedBit = true);

		/// Destructor.
		~bdBitBuffer();

		//
		// Methods.
		//

		/// Get the size of the data in bytes.
		/// \return	The number of bytes occupied by the data in the buffer.
		inline bdUInt getDataSize() const;

		/// Get the number of bits explicitly written to the buffer.
		/// This does not include and header bits automatically written
		/// but only the data written bu the user via the write calls.
		/// \return The number of bits explicitly written to the buffer.
		inline bdUInt getNumBitsWritten() const;

		/// Get const access to the internal data buffer.
		/// \return	A pointer to the buffer's data.
		inline const bdUByte8* getData() const;

		/// Get access to the internal data buffer.
		/// \return	A pointer to the buffer's data.
		inline bdUByte8* getData();

		/// Check whether a read operation has failed.
		/// Use this method to check if a previous operation attempted to
		/// extract non-existent information from the buffer.
		/// Improve code security with this method if this buffer has been
		/// received from an untrusted source, e.g., the  network, disk or
		/// console.
		/// \return
		/// -	True, if a prior read operation has failed.
		/// -	False, if all prior read operations on this buffer completed
		///		successfully.
		inline bdBool failedRead() const;

		/// Get the value of the write pointer.
		/// The value of the write pointer is the total number of bits that
		/// have been written to the buffer.
		/// \return	The number of bits that have been written to the buffer.
		inline bdUInt getWritePosition() const;

		/// Get the value of the read pointer.
		/// The value of the read pointer is the total number of bits that
		/// have been read from the buffer.
		/// \return	The number of bits that have been read from the buffer.
		inline bdUInt getReadPosition() const;

		/// Explicitly set the value of the read pointer.
		/// \param bitPosition[in]	The value to set the read pointer to, in
		///							bits.
		inline void setReadPosition(const bdUInt bitPosition);

		inline void resetReadPosition();

		/// Explicitly set the value of the write pointer.
		/// You can write to any position within a the buffer, including
		/// positions beyond the end of the current buffer.
		/// \param bitPosition[in]	The value to set the write pointer to, in
		///							bits.
		inline void setWritePosition(const bdUInt bitPosition);

		inline void resetWritePosition();

		//
		// Insertion methods.
		//

		/// Insert a boolean into the buffer.
		/// For convenience, this method returns the value of the parameter \a
		/// b. This makes a typical use of this method (delta compression) more
		/// compact and easy to read.
		/// \param b[in]	The boolean to insert.
		/// \return
		/// -	True, if b was true.
		/// -	False, if b was false.
		inline bdBool writeBool(const bdBool b);

		/// Insert a native 8-bit character into the buffer.
		/// \param c[in]	The character to insert.
		inline void writeNChar8(const bdNChar8 c);

		/// Insert a signed byte into the buffer.
		/// \param b[in] The byte to insert.
		inline void writeByte8(const bdByte8 b);

		/// Insert an unsigned byte into the buffer.
		/// \param b[in] The byte to insert.
		inline void writeUByte8(const bdUByte8 b);

		/// Insert a signed 16-bit integer into the buffer.
		/// \param i[in]	The integer to insert.
		inline void writeInt16(const bdInt16 i);

		/// Insert an unsigned 16-bit integer into the buffer.
		/// \param u[in]	The integer to insert.
		inline void writeUInt16(const bdUInt16 u);

		/// Insert a signed 32-bit integer into the buffer.
		/// \param i[in]	The integer to insert.
		inline void writeInt32(const bdInt32 i);

		/// Insert an unsigned 32-bit integer into the buffer.
		/// \param u[in]	The integer to insert.
		inline void writeUInt32(const bdUInt32 u);

		/// Insert an signed 64-bit integer into the buffer.
		/// \param i[in]	The integer to insert.
		inline void writeInt64(const bdInt64 i);

		/// Insert an unsigned 64-bit integer into the buffer.
		/// \param u[in]	The integer to insert.
		inline void writeUInt64(const bdUInt64 u);

		/// Insert an unsigned variable-bit integer into the buffer. The
		/// integer will be stored in the minimum number of bits required to
		/// represent the range of values between \a begin and \a end.
		/// \note \a u will be clamped to ensure it is inside the range
		/// [begin, end].
		/// \param u[in]		The integer to insert.
		/// \param begin[in]	The minimum possible value of \a u.
		/// \param end[in]		The maximum possible value of \a u.
		/// \param typeChecked [in]	Flag to indicate if the value is type
		///                         checked.
		void writeRangedUInt32(const bdUInt u,
							   const bdUInt begin,
							   const bdUInt end,
							   const bdBool typeChecked = true);

		/// Insert a signed variable-bit integer into the buffer. The
		/// integer will be stored in the minimum number of bits required to
		/// represent the range of values between \a begin and \a end.
		/// \note \a i will be clamped to ensure it is inside the range
		/// [begin, end].
		/// \param i[in]		The integer to insert.
		/// \param begin[in]	The minimum possible value of \a i.
		/// \param end[in]		The maximum possible value of \a i.
		void writeRangedInt32(const bdInt i,
							  const bdInt begin,
							  const bdInt end);

		/// Insert a 32-bit floating-point number into the buffer.
		/// \param f[in]	The floating-point number to insert.
		inline void writeFloat32(const bdFloat32 f);

		/// Insert a 64-bit floating-point number into the buffer.
		/// \param f[in]	The floating-point number to insert.
		inline void writeFloat64(const bdFloat64 &f);

		/// Insert a float restricted by range and precision into the buffer.
		/// Use this method to write floats with a minimum required number of bits.
		/// Specify \a begin and \a end to enclose the range of values the float
		/// is expected to lie in. \a precision specifies the maximum error
		/// (absolute) that is tolerated.
		/// \note \f$\frac{|end-start|}{precision}\f$ must be less then or equal
		/// to \f$2^{32}\f$.
		/// \note Using very large numbers might lead to greater imprecision then desired.
		/// \note The method will use \f$log_2(\frac{|end-start|}{precision})\f$ bits
		/// for any given float.
		/// \note \b Example: To write a percentage value with
		/// precision of up to 2 decimal places, \a begin = 0, \a end = 100, \a precision = 0.01.
		/// \note \a f will be clamped to ensure it is inside the range
		/// [begin, end].
		/// \param f[in] The floating-point number to insert.
		/// \param begin[in] The lowest value \a f is expected to take.
		/// \param end[in] The highest value \a f is expected to take.
		/// \param precision[in] The maximum error tolerated.
		void writeRangedFloat32(const bdFloat32 f,
								const bdFloat32 begin,
								const bdFloat32 end,
								const bdFloat32 precision);

		inline void writeString(const bdNChar8 *const s, 
								const bdUWord maxLen);

		/// Insert a bdString into the buffer.
		/// \param s[in]	The bdString to insert.
		inline void writeString(const bdString& s);

		/// Insert binary data into the buffer
		/// \param blob[in] A reference to the data to insert.
		/// \param length[in] The length of the blob.
		inline void writeBlob(const void *const blob,
							  const bdUInt32 length);


		/// Insert a bitwise copy of an arbitrary type into the buffer.
		/// \param data[in] A reference to the data to insert.
		template <typename T>
		inline void writeFull(const T& data)
		{
			writeDataType(BD_BB_FULL_TYPE);
			writeBits(&data, sizeof(T) << 3);
		}

		/// Insert an arbitrary amount of binary data into the buffer.
		/// \param bits[in]		A pointer to the data to insert.
		/// \param numBits[in]	The amount of data to insert, in bits.
		void writeBits(const void *bits,
					   const bdUInt numBits);

		//
		// Extraction methods.
		//

		/// Extract a boolean from the buffer.
		/// \param b[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		inline bdBool readBool(bdBool &b);

		/// Extract and test a boolean from the buffer. This function returns
		/// true if the read was successful AND the value been read is true.
		/// \return
		///	-	True, if the read was successful AND the value read was also true.
		/// -	False, if there was no data to read or the value read was false.
		inline bdBool testBool();

		/// Extract a native 8-bit character from the buffer.
		/// \param c[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the operation was successful.
		/// -	False, if there was no data to read.
		inline bdBool readNChar8(bdNChar8 &c);

		/// Extract a signed byte from the buffer.
		/// \param b[out] The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		inline bdBool readByte8(bdByte8 &b);

		/// Extract an unsigned byte from the buffer.
		/// \param b[out] The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		inline bdBool readUByte8(bdUByte8 &b);

		/// Extract a signed 16-bit integer from the buffer.
		/// \param i[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		inline bdBool readInt16(bdInt16 &i);

		/// Extract an unsigned 16-bit integer from the buffer.
		/// \param u[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		inline bdBool readUInt16(bdUInt16 &u);

		/// Extract a signed 32-bit integer from the buffer.
		/// \param i[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		/// \todo Explain why this takes a bdInt rather than a bdInt32
		inline bdBool readInt32(bdInt &i);

		/// Extract an unsigned 32-bit integer from the buffer.
		/// \param u[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		/// \todo Explain why this takes a bdUInt rather than a bdUInt32
		inline bdBool readUInt32(bdUInt &u);

		/// Extract an signed 64-bit integer from the buffer.
		/// \param i[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		inline bdBool readInt64(bdInt64 &i);

		/// Extract an unsigned 64-bit integer from the buffer.
		/// \param u[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		inline bdBool readUInt64(bdUInt64 &u);

		/// Extract an unsigned variable-bit integer from the buffer.
		/// \param u[out]		The variable to receive the datum.
		/// \param begin[in]	The minimum possible value of \a u.
		/// \param end[in]		The maximum possible value of \a u.
		/// \param typeChecked[in] Flag to indicate if the buffer is type
		///                        checked
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		bdBool readRangedUInt32(bdUInt &u,
							    const bdUInt begin,
							    const bdUInt end,
								const bdBool typeChecked = true);

		/// Extract a variable-bit integer from the buffer.
		/// \param i[out]		The variable to receive the datum.
		/// \param begin[in]	The minimum possible value of \a i.
		/// \param end[in]		The maximum possible value of \a i.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		bdBool readRangedInt32(bdInt &i,
							   const bdInt begin,
							   const bdInt end);

		/// Extract 32-bit floating-point number from the buffer.
		/// \param f[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		inline bdBool readFloat32(bdFloat32 &f);

		/// Extract 64-bit floating-point number from the buffer.
		/// \param f[out]	The variable to receive the datum.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		inline bdBool readFloat64(bdFloat64 &f);

		/// Extract a float restricted by range and precision from the buffer.
		/// Use this method to read floats written with writeRangedFloat32.
		/// \a begin, \a end, and \a precision must be the same as in the
		/// corresponding call to writeRangedFloat32.
		/// \note Due to approximate nature of floats, \a f extracted might
		/// lie outside the [begin ; end] interval.
		/// \param f[out] The floating-point number to extract.
		/// \param begin[in] The lowest value \a f is expected to take.
		/// \param end[in] The highest value \a f is expected to take.
		/// \param precision[in] The maximum error tolerated.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was no data to read.
		bdBool readRangedFloat32(bdFloat32 &f,
								 const bdFloat32 begin,
								 const bdFloat32 end,
								 const bdFloat32 precision);


		/// Extract the length of null-terminated character string from the buffer.
		/// \param length[out]	Upon successful return this will be set the the number of
		///						characters in the string, not including the null terminator.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, otherwise.
		inline bdBool getStringLength(bdUWord &length);

		/// Extract a null-terminated character string from the buffer.
		/// \param s[in]	A pointer to extract the characters to, will always null terminated.
		/// \param maxLen[in] The size of the string buffer \a s, in bytes. Regardless of \a maxLen
		///					  the entire string is read from the buffer.
		/// \return
		///	-	True, if the read was successful.
		/// -	False otherwise.
		inline bdBool readString(bdNChar8 *const s, 
								 const bdUWord maxLen);

		/// Extract a null-terminated character string from the buffer.
		/// \param s[in]	A pointer to the destination buffer. If \a s is BD_NULL
		///					nothing is read from the buffer but \a length is set.
		/// \param maxLen[in] The maximum size of the string. The actual size
		///					  of the string may be less than maxLen characters.
		/// \param length[in|out]	If \a s is non-null this should be set to the length 
		///							of the destination buffer. Upon return this is set to 
		///							the number of characters in the string contained in the string, 
		///							not including null terminator.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, otherwise.
		inline bdBool readString(bdNChar8 *const s,
								 const bdUWord maxLen,
								 bdUWord& length);

		/// Extract a bdString from the buffer.
		/// \param s[out]	A bdString to extract the characters to.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was not enough data to read.
		/// \warning If you use this method to read from a buffer that was
		/// received from an untrusted source (e.g., the network), your code
		/// may be exploited to allocate an arbitrary amount of memory within
		/// \a s. Consider using bdBitBuffer::readBits() instead.
		inline bdBool readString(bdString& s);

		/// Extract the length of binary blob data from the buffer.
		/// Reads the size of a blob, in bytes, from the buffer, such that
		/// readBlob can be called afterwards.
		/// \param length[out]	Upon successful return this will be set to the size 
		///						of the blob in bytes.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, otherwise.
		inline bdBool getBlobLength(bdUInt32& length);

		/// Extract a binary blob data from the buffer.
		/// \param blob[in] 		A pointer to the destination buffer, may be NULL.
		/// \param length[in|out] 	The size of the destination buffer. Upon return, this
		/// 						contains the length of the blob in the buffer, in bytes.
		///							Regardless of \a length the entire blob is read from the buffer.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, otherwise.
		inline bdBool readBlob(bdUByte8 *const blob,
							   bdUInt32& length);

		/// Extract a bitwise copy of an arbitrary type from the buffer.
		/// \param data[out] A reference to a variable to receive the data.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was not enough data to read.
		template <typename T>
		inline bdBool readFull(T& data)
		{
			readDataType(BD_BB_FULL_TYPE);
			return readBits(&data, sizeof(T) << 3);
		}

		/// Extract an arbitrary amount of binary data from the buffer.
		/// \param bits[in]		A pointer to extract the data to.
		/// \param numBits[in]	The amount of data to extract, in bits.
		/// \return
		///	-	True, if the read was successful.
		/// -	False, if there was not enough data to read.
		bdBool readBits(void *bits,
					    bdUInt numBits);

		/// A function specifically for bdXMLConverter.
		///  Reads the next byte in the BitBuffer, assumes that the next byte is a bdParamType.
		/// \return
		///	-	True, if the read was successful, and it found a bdParamType.
		/// -	False, if the next byte read doesn't match an existing bdParamType.
		bdBitBufferDataType readDataType();

		/// En/Disables typechecking for a bitbuffer. This is used by applications that wish to
		/// read type information by themselves
		/// \param flag[in] Boolean to indicate whether typechecking should be performed
		void setTypeCheck(const bdBool flag);

		bdBool getTypeCheck() const;

		static void BD_CALL typeToString(const bdBitBufferDataType type,
										 bdNChar8 *const strBuffer,
										 const bdUWord strLength);

		/// Appends the contents of another bitbuffer to this bitbuffer.
		/// The two bitbuffers have to be both typechecked or not typechechecked.
		/// This operation allocates temporary data from the heap
		/// \param other[in] the source bitbuffer
		/// \return -	True, if the data could be copied
		///			-	False, if the bitbuffers are of different type.
		bdBool append(bdBitBuffer &other);

		/// Converts the endianness of a basic type.
		/// \param src[in] The variable to convert.
		/// \param dest[out] The endian swapped variable.
		template <typename T>
		static void BD_CALL endianSwap(const T& src, T& dest)
		{
#if defined BD_BIG_ENDIAN
			bdUByte8 *pDest = reinterpret_cast<bdUByte8*>(&dest);
			const bdUByte8 *pSrc = reinterpret_cast<const bdUByte8*>(&src);
			for (bdUInt i=0; i < sizeof(T); i++)
			{
				bdUByte8 *destByte = pDest + sizeof(T) - 1 - i;
				*destByte = *(pSrc+i);
			}
#else
			dest = src;
#endif
		}

	protected:

		/// If the buffer is type checked, this function writes paramType to the buffer.
		/// This is used internally in bdBitBuffer by all write functions except writeBits.
		/// \param dataType[in]		The type of parameter that will be written to the buffer
		///							immediately after this function.
		void writeDataType(const bdBitBufferDataType dataType);

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



	protected:

		/// The internal data buffer.
		bdFastArray<bdUByte8> m_data;

		/// The write position in bits within the buffer.
		bdUInt m_writePosition;

		/// One beyond the maximum write position.
		bdUInt m_maxWritePosition;

		/// The read position in bits within the buffer.
		bdUInt m_readPosition;

		/// A flag to indicate that a read operation failed.
		bdBool m_failedRead;

		/// A flag which controls the type checking of the bitBuffer
		bdBool m_typeChecked;
};

#include <bdCore/bdContainers/bdBitBuffer.inl>

#endif // BD_BIT_BUFFER_H
