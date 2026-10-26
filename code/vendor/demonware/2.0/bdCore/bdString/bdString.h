// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Reference counted string.

#ifndef BD_STRING_H
#define BD_STRING_H

#include <bdCore/bdMemory/bdMemory.h>

/// The capacity of a string grows in chunks of BD_CAPACITY_INCREMENT.
#define BD_CAPACITY_INCREMENT (64)

/// Header data for a bdString. This class is placed in memory immediately
/// before the string data of a bdString.
class bdStringData
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// An accessor method for the string that this header data refers to.
		/// \return A pointer to the first character in the string.
		bdNChar8* getString()
		{
			return reinterpret_cast<bdNChar8*>(this+1);
		}

		/// The reference count of this string.
		bdUInt m_referenceCount;

		/// The number of characters in this string, not including the null
		/// terminator.
		bdUWord m_length;

		/// The size of the buffer allocated for character storage.
		bdUWord m_capacity;
};

/// String representation and manipulation routines.
/// The string data is shared and reference counted. It is only copied when
/// changed.
class bdString
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		//
		// Construction and destruction.
		//

		/// Default constructor that creates an empty string.
		/// The string is set to the global empty string representation
		/// g_emptyString, shared by all empty strings, and reference
		/// counted.
		bdString();

		/// Copy constructor.
		/// Initializes \a *this to be the same as \a s. The string data will
		/// be shared, not copied.
		/// \param s[in] The string to use when constructing the copy.
		bdString(const bdString& s);

		/// Creates a string and initializes it to the string pointed
		/// to by \a s.
		/// \param s[in] A pointer to the string to use when constructing the
		///  copy.
		/// \note \a s must be null terminated.
		bdString(const bdNChar8 *const s);

		/// Destructor, destroys a string.
		/// If the string is not shared any associated memory will be released.
		~bdString();

		//
		// Operators (excluding new and delete).
		//

		/// Assign \a s to \a *this.
		/// \param s[in] The string to use.
		/// \return A reference to \a this string.
		bdString& operator=(const bdString& s);

		/// Assign the string pointed to by \a s to *this.
		/// \param s[in] The string to use.
		/// \return A reference to \a this string.
		bdString& operator=(const bdNChar8 *const s);

		/// Compare \a *this to \a s.
		/// \param s[in] The string to compare this string to.
		/// \return True if the characters in \a this and \a s match exactly.
		/// \note This operator is case sensitive.
		bdBool operator==(const bdString& s) const;

		/// Compare \a *this to the string pointed to by \a s.
		/// \param s[in] A pointer to the string to compare this string to.
		/// \return True if the characters in \a this and \a s match exactly.
		/// \note This operator is case sensitive.
		bdBool operator==(const bdNChar8 *const s) const;

		/// Compare \a *this to \a s.
		/// \param s[in] The string to compare this string to.
		/// \return True if the characters in \a this and \a s differ.
		/// \note This operator is case sensitive.
		bdBool operator!=(const bdString& s) const;

		/// Compare \a *this to the string pointed to by \a s.
		/// \param s[in] A pointer to the string to compare this string to.
		/// \return True if the characters in \a this and \a s differ.
		/// \note This operator is case sensitive.
		bdBool operator!=(const bdNChar8 *const s) const;

		/// Concatenate two strings.
		/// \param s[in] The string to concatenate with this string.
		/// \return A new string which is the concatenation of \a *this
		///  and \a s.
		bdString operator+(const bdString& s) const;

		/// Concatenate two strings.
		/// \param s[in] A pointer to the string to concatenate with
		///  this string.
		/// \return A new string which is the concatenation of \a *this and
		///  and the string pointed to by \a s.
		bdString operator+(const bdNChar8 *const s) const;

		/// Concatenate \a s to \a *this.
		/// \param s[in] The string to concatenate onto the end of this string.
		/// \return A reference to \a this string.
		bdString& operator+=(const bdString& s);

		/// Concatenate the string pointed to by \a s to \a *this.
		/// \param s[in] A pointer to the string to concatenate onto the end
		///  of this string.
		/// \return A reference to \a this string.
		bdString& operator+=(const bdNChar8 *const s);

		/// Append a single character to the string.
		/// \param c[in] The character to concatenate onto the end of \a this
		///  string.
		/// \return A reference to \a this string.
		bdString& operator+=(const bdNChar8 c);

		/// Defines the type used in the implicit casting of a bdString
		/// to a const bdNChar8*.
		typedef const bdNChar8* bdLPCTSTR;

		/// Conversion operator so that bdString can be used as a bdNChar8*.
		/// Allows implicit casting of a bdString to a const bdNChar8*.
		/// \note This means that you can pass a bdString directly to a
		///  function that requires a [const bdNChar8* or char*] argument, or
		///  treat a bdString the same as a const bdNChar8*.
		/// \remark For example myString[i] is perfectly valid where myString
		///  is a bdString instance.
		/// \remark Do not use this with a printf, as it will not work on the unix platform.
		///  Instead, use getBuffer()
		operator bdLPCTSTR() const;

		//
		// Non static methods.
		//

		/// Finds first occurrence of \a c in \a *this.
		/// \param c[in] The character to search for.
		/// \param i[out] The index of the first occurrence of \a c in
		///  \a this string.
		/// \return
		///  - True if \a c is found.
		///  - False otherwise.
		/// \note If found it sets i to be the index of the first occurrence of \a c
		///  in \a *this. If not found i is left unchanged.
		bdBool findFirst(const bdNChar8 c,
						 bdUWord& i) const;

		/// Returns a section of the string.
		/// Returns a new string consisting of the characters between the
		/// being'th (inclusive) and the end'th (exclusive) characters of
		/// \a *this.
		/// \param begin[in] The index at which the required section begins.
		/// \param end[in] The index at which the required section ends.
		/// return A string containing a copy of the section requested.
		/// \note If end is greater than the length of the string the section
		///  returned will be from begin to the end of the string. There is
		///  no guarantee that a string of length (end - begin) will be
		///  returned.
		bdString getSection(const bdUWord begin,
							bdUWord end) const;

		/// Returns the number of characters in \a *this, not including the null
		/// terminator.
		/// \return The length of the string.
		bdUWord getLength() const;

		/// An explicit method to return a pointer to the string data
		/// in \a *this.
		/// \return A pointer to the string data in \a *this.
		/// \note The implicit cast operator bdLPCTSTR() const achieves the
		///  same result transparently.
		const bdNChar8* getBuffer() const;

	protected:

		/// An accessor function for the string data class associated with
		///  this string.
		/// \return A pointer to the bdStringData for this string.
		bdStringData* getStringData() const;

		/// Sets the string to the global empty string representation
		/// g_emptyString, shared by all empty strings, and reference
		/// counted.
		void initialize();

		/// Increments the reference count of \a stringData.
		/// \param stringData[in,out] A pointer to the string data whose
		///  reference count will be incremented.
		void addReference(bdStringData* stringData) const;

		/// Decrements the reference count of \a stringData.
		/// \param stringData[in,out] A pointer to the string data whose
		///  reference count will be decremented.
		/// \note On release of the final reference, ie m_referenceCount == 0,
		/// the memory associated with the string is released.
		void removeReference(bdStringData* stringData) const;

		/// Ensures that this string has enough capacity to store a
		/// character string of length \a length.
		/// \param length[in] The length of the string for which the buffer
		///  is being allocated.
		/// \note \a length need not include the null terminating character.
		/// \note As this function does not call removeReference; the old
		///  buffer is still available after allocation. This allows the
		///  old string to be copied into the new buffer.
		/// \note The capacity of a string increases in increments of
		///  \a BD_CAPACITY_INCREMENT; therefore the allocated buffer may be up to
		///  (\a BD_CAPACITY_INCREMENT -1) bytes larger than length.
		void allocateBuffer(const bdUWord length);

		/// Deallocates the memory associated with this string. This includes
		/// the bdStringData header.
		/// \param stringData A pointer to the string data for this class.
		/// \note When this function is called, the pointer to \a stringData will
		///  be known, therefore it is more efficient to pass it than re-calculate it.
		void freeBuffer(bdStringData* stringData) const;

		/// Checks to see if there is enough capacity to store a string of
		/// \a length characters. \a length need not include the null terminator.
		/// \param length[in] The amount of capacity we require.
		/// \return
		///  - True if there is enough capacity.
		///  - False otherwise.
		bdBool enoughCapacity(const bdUWord length) const;

	protected:

		/// A pointer the the actual string.
		bdNChar8* m_string;
};

#endif // BD_STRING_H

