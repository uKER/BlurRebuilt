// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef BD_SEQUENCE_NUMBER_H
#define BD_SEQUENCE_NUMBER_H

#include <bdCore/bdMemory/bdMemory.h>


/// Provides limited-size sequence numbers. For example, 2-byte sequence numbers,
/// with arithmetic and comparison operations defined. Internally, limited-range
/// sequence numbers are mapped onto a continuous positive integer space: 4-byte
/// signed integers, with negative space reserved for invalid numbers and -1, the
/// initial number. This internal value can be retrived with \c getValue().
///
/// \note Since this performs serial number arithmetic, the absolute difference
/// between two adjacent sequence numbers should be less than half of the
/// available number space (2^bits / 2).
///
/// \note  Use where internal state is likely to reach 2^31 should be avoided as
/// wrap-around is not yet supported.
class bdSequenceNumber
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Create the first sequence number.
		/// \param seqNum The initial sequence number. Set to a special negative
		///		value by default.
		bdSequenceNumber(const bdInt seqNum = -1);

		/// Create a new sequence number.
		/// \param last Sequence number preceding this one.
		/// \param seqNumber New limited-range sequence number.
		/// \param bits Number of bits used by the unsigned limited-range
		///		sequence number.
		bdSequenceNumber(const bdSequenceNumber &last,
						 const bdUInt seqNumber,
						 const bdUInt bits);

		/// Set to a new value.
		/// \param last Sequence number preceding this one.
		/// \param seqNumber New limited-range sequence number.
		/// \param bits Number of bits used by the unsigned limited-range
		///		sequence number.
		void set(const bdSequenceNumber &last,
						   const bdUInt seqNumber,
						   const bdUInt bits);

		/// The 4-byte sequence value this sequence number maps to.
		/// \return Signed integer (only positive space used) this number maps to.
		bdInt32 getValue() const;

		bdSequenceNumber operator + (const bdSequenceNumber& other) const;

		bdSequenceNumber &operator += (const bdSequenceNumber& other);

		bdSequenceNumber &operator ++ ();

		bdSequenceNumber operator ++ (bdInt val);

		bdSequenceNumber operator - (const bdSequenceNumber& other) const;

		bdBool operator > (const bdSequenceNumber &other) const;

		bdBool operator < (const bdSequenceNumber &other) const;

		bdBool operator <= (const bdSequenceNumber &other) const;

		bdBool operator >= (const bdSequenceNumber &other) const;

		bdBool operator == (const bdSequenceNumber &other) const;

		bdBool operator != (const bdSequenceNumber &other) const;

	protected:

		/// Sequence number after mapping onto continuous space.
		bdInt m_seqNum;
};

#endif // BD_SEQUENCE_NUMBER_H
