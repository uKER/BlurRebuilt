// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A circular data buffer.

#ifndef BD_CIRCULAR_BUFFER_H
#define BD_CIRCULAR_BUFFER_H

#include <bdCore/bdMemory/bdMemory.h>

/// This provides a circular buffer facility.
///
/// This buffer provides a wrapping mode (circular), and a shifting mode
/// (non-circular) of operation. Default is wrapping mode, and shifting is
/// activated by passing in \a false as the second parameter to the constructor.
///
/// Access to the buffer is in "blocks" -- maximally long contiguous
/// parts of the buffer (in wrapping mode, available block of free space
/// can be shorter than total available free space, due to it being spread
/// across the begining and the end of the buffer).
///
/// In shifting mode, it is necessary to call \a alignUsedBlock, which shifts
/// all of the data in the buffer to its beggining and thus ensures there is
/// space available in it.
template <typename T>
class bdCircularBuffer
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Specific size constructor.
		/// \param size[in]	The amount of elements to allocate in
		///					this buffer. The actual capacity of the buffer
		///					will be 1 less than that.
		/// \param wrappingAllowed[in] Specifies circular mode of buffer operation.
		///  - True, to make the buffer operate circularly.
		///  - False, to make the buffer operate non-circularly. In this case
		///  user needs to call \a alignUsedBlock regularly.
		bdCircularBuffer(const bdUInt size, 
						 T* const buf = BD_NULL, 
						 const bdBool wrappingAllowed = true);

		/// Destructor.
		~bdCircularBuffer();

		/// Returns beginning of the unused area in the buffer.
		/// \return Pointer to the beginning of the unused area in the buffer.
		T* getFreeBlockStart();

		/// Returns size of the area that \a getFreeBlockStart returned.
		/// \return Number of elements that can be put into the free block pointed by
		/// \a getFreeBlockStart.
		bdUInt getFreeBlockSize();

		/// Mark the first \a size elements in the free area (provided by 
		/// \a getFreeBlockStart) as used, thus appending them to the end of
		/// the used area.
		/// \param size[in] Number of elements at the start of free area to mark
		/// as used. Must not be more than the value returned by \a getFreeBlockSize.
		void protectBlock(bdUInt size);

		/// Returns beginning of the used area in the buffer.
		/// \return Pointer to the beginning of the used area in the buffer.
		T* getUsedBlockStart();

		/// Returns size of the area that \a getUsedBlockStart returned.
		/// \return Number of elements that can be removed from the used block pointed
		/// by \a getUsedBlockStart.
		bdUInt getUsedBlockSize();

		/// Mark the first \a size elements in the used area (provided by
		/// \a getUsedBlockStart) as free, thus appending them to the end of
		/// the free area.
		/// \param size[in] Number of elements at the start of used area to mark
		/// as free. Must not be more than the value returned by \a getUsedBlockSize.
		void freeBlock(bdUInt size);

		/// In shifting mode, shift all data to the beginning of the buffer so that
		/// the used area begins at the start of it.
		void alignUsedBlock();

		/// Returns the total number of elements that can be appended to the buffer.
		/// \return The total size of the free space in the buffer.
		bdUInt getTotalFreeSpace();

		/// Returns the total number of elements that can be removed from the buffer.
		/// \return The total number of elements in the buffer.
		bdUInt getTotalUsedSpace();

		/// Put a basic type variable into the buffer, converting to Little-Endian if needed.
		/// \param var[in] The variable to append to the buffer.
		/// \return True on success, false if there is not enough space in the buffer.
		template <typename U>
		bdBool appendBasicType(const U &var)
		{
			U nvar;
			bdBitBuffer::endianSwap(var, nvar);
			const bdBool ok = appendBuffer(reinterpret_cast<const bdUByte8*>(&nvar), sizeof(U));
			return ok;
		}

		/// Remove a basic type value from the buffer, converting to host byte order.
		/// \param var[out] Receives the value removed.
		/// \return True on success, false if there isn't enough data in the buffer.
		template <typename U>
		bdBool removeBasicType(U &var)
		{
			U nvar;
			const bdBool ok = removeBuffer(reinterpret_cast<bdUByte8*>(&nvar), sizeof(U));
			if(ok)
			{
				bdBitBuffer::endianSwap(nvar, var);
			}
			return ok;
		}

		/// Put elements from an array into the buffer.
		/// \param var[in] An array of elements to be put into the buffer.
		/// \param size[in] Number of elements to be put into the buffer.
		/// \return True on success, false if there is not enough space in the buffer.
		bdBool appendBuffer(const T* const var, const bdUInt size);

		/// Remove elements from the buffer into an array.
		/// \param var[in] An array to put the elements into.
		/// \param size[in] Number of elements to be removed from the buffer.
		/// \return True on success, false if there are less than \a size elements
		/// in the buffer.
		bdBool removeBuffer(T *const var, const bdUInt size);
		
	protected:

		/// See if we need to wrap our pointers around, and do it.
		void attemptWrap();
		/// True, if the buffer is in a wrapping mode.
		bdBool m_canWrap;
		/// Number of elements allocated to the buffer.
		bdUInt m_size;
		/// Pointer to the start of the buffer's memory storage area.
		T *m_data;
		/// Pointer the byte past the buffer's memory storage area.
		T *m_dataEnd;
		/// Pointer to the start of the used data area.
		T *m_start;
		/// Pointer to the end of the used data area.
		T *m_end;

		bdBool m_allocated;
};

#include <bdCore/bdContainers/bdCircularBuffer.inl>

#endif // BD_CIRCULAR_BUFFER_H
