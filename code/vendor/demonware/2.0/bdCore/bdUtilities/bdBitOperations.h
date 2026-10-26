// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines functions & macros for performing operations involving bits

#ifndef BD_BIT_OPERATIONS
#define BD_BIT_OPERATIONS

#include <bdCore/bdMemory/bdMemory.h>

/// A macro to get the 'i'th bit in d
#define BD_GET_BIT(d, i) (((d) >> (i)) & 0x1)
/// A macro to set the 'i'th bit in d
#define BD_SET_BIT(d, i) ((d) |= (0x1 << (i)))
/// A macro to clear the 'i'th bit in d
#define BD_CLEAR_BIT(d, i) ((d) &= ~(0x1 << (i)))

/// A macro to get the number of bytes in 'n' bits
#define BD_NUM_BITS_TO_NUM_BYTES(n) (((n)>>3) + (((n) & 0x7)?1:0))

/// A macro to get the number of bits in 'n' bytes
#define BD_NUM_BYTES_TO_NUM_BITS(n) ((n)*8)

/// A macro to determine whether a value is a power of 2.
#define BD_IS_POWER_OF_2(n) (((n) & ((n)-1)) == 0)

/// A macro to round \a n up to the next multiple of \a m (which
/// must be a power of 2). If \a n is already a multiple of \a m
/// the macro evaluates to \a n.
#define BD_NEXT_MULTIPLE_OF_M(n, m)  (((n) + ((m)-1)) & (~((m)-1)))

/// A macro to round \a n down to the previous multiple of \a m (which
/// must be a power of 2). If \a n is already a multiple of \a m
/// the macro evaluates to \a n.
#define BD_PREVIOUS_MULTIPLE_OF_M(n, m) ((n) & ~((m)-1))

/// A macro to determine if \a n is a multiple of \a m (which
/// must be a power of 2).
#define BD_IS_MULTIPLE_OF_M(n, m) ((n & ((m)-1)) == 0)

/// Wrapper for some common bit operations.
class bdBitOperations
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Get the next power of 2 greater than or equal to the value passed in.
		/// \param	v[in]	The value which the result must be greater than or equal
		///					to.
		/// \return	The closest power of 2 which is greater than or equal to
		///			the value passed in.
		static bdUInt BD_CALL nextPowerOf2(const bdUInt v);

		/// Get the index of the highest bit that is set in \a v.
		/// \param	v[in]	The value to search for the high bit index.
		/// \return			The index of the high bit number.
		static bdUInt BD_CALL highBitNumber(bdUInt v);

};

#endif // BD_BIT_OPERATIONS
