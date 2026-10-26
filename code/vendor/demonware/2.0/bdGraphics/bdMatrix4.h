// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The bitdemon representation of a 4x4 Matrix.

#ifndef BD_MATRIX4_H
#define BD_MATRIX4_H

#include <bdCore/bdCore.h>
#include <bdGraphics/bdPlatformMath.h>

/// The bitdemon representation of a 4x4 Matrix.
class bdMatrix4
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default Constructor.
		inline bdMatrix4();

		/// Constructor taking a pointer to an array of initial values.
		inline bdMatrix4(bdFloat32 *const matrix);

		/// Assignment operator taking another bdMatrix4.
		/// \param other[in] The matrix to copy.
		inline void operator= (const bdMatrix4 &other);

		/// Assignment operator taking an array of floats.
		/// \param other[in] A pointer to the array to copy.
		inline void operator= (const bdFloat32 *other);

		/// Get read and write access to the \a i'th element.
		/// \param i[in]	The index of the item to be accessed.
		/// \return The value at index i.
		inline bdFloat32& operator[] (const bdUInt32 i);

		/// Accessor function for the array that stores the matrix.
		/// \return A pointer to the array of floats for this matrix.
		inline const bdFloat32* getData() const;

		/// Set the state of the matrix to be the same as the passed in array.
		/// \param v[in]	The array to copy the data from.
		inline void set(bdFloat32 *const v);

		/// Write the state of the matrix into the passed in array.
		/// \param v[out]	The array to copy the data to.
		inline void get(bdFloat32 *const v) const;

	protected:

		/// Internal representation of the matrix as a 16 item array.
		bdFloat32 m_matrix[16];
};

#include <bdGraphics/bdMatrix4.inl>

#endif // BD_MATRIX4_H

