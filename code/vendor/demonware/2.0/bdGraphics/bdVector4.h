// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The bitdemon representation of a 4 element vector.

#ifndef BD_VECTOR4_H
#define BD_VECTOR4_H

#include <bdCore/bdCore.h>
#include <bdGraphics/bdMatrix4.h>

/// The bitdemon representation of a 4 element vector.
class bdVector4
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default Constructor.
		inline bdVector4();

		/// Constructor taking 4 floats as args.
		/// \param x[in]	The first  element in the vector.
		/// \param y[in]	The second element in the vector.
		/// \param z[in]	The third  element in the vector.
		/// \param w[in]	The fourth element in the vector.
		///					This has 0 as a default arg so that the vector can
		///					be used a 3 element vector as.
		inline bdVector4(const bdFloat32 x,
						 const bdFloat32 y,
						 const bdFloat32 z,
						 const bdFloat32 w = 0);


		/// Constructor taking a float array as an arg.
		/// \param v[in]	The array to construct the vector with.
		inline bdVector4(bdFloat32 *const v);

		/// Assignment operator taking another vector as an arg.
		/// \param other[in]	The vector whose values are assigned to this
		///						vector.
		inline void operator= (const bdVector4 &other);

		/// Accessor operator for the 'i'th element in the vector.
		/// \param i[in]	Index of the element to be retrieved.
		/// \return A reference to the requested element.
		inline bdFloat32& operator[] (const bdUInt32 i);

		/// Const accessor operator for the 'i'th element in the vector.
		/// \param i[in]	Index of the element to be retrieved.
		/// \return A const reference to the requested element.
		inline const bdFloat32& operator[] (const bdUInt32 i) const;

		/// Negative operator.
		/// \return The negative equivalent of the vector.
		inline bdVector4 operator- ();

		/// Accessor function for the array representing the vector.
		/// \return An array representation of the vector.
		inline const bdFloat32* getData() const;

		/// Set the state of the vector.
		/// \param v[in]	A pointer to an array containing the new state of
		///					the vector.
		inline void set(bdFloat32 *const v);

		/// Set the state an element in the vector.
		/// \param i[in]	Index of the element to be set.
		/// \param v[in]	The value to assign to the 'i'th element.
		inline void set(const bdUInt32 i,
						const bdFloat32 v);

		/// Set the state of the vector.
		/// \param x[in]	The first  element in the vector.
		/// \param y[in]	The second element in the vector.
		/// \param z[in]	The third  element in the vector.
		/// \param w[in]	The fourth element in the vector.
		///					This has 0 as a default arg so that the vector can
		///					be used a 3 element vector.
		inline void set(const bdFloat32 x,
						const bdFloat32 y,
						const bdFloat32 z,
						const bdFloat32 w = 0);

		/// Set the state the 1st element in the vector.
		/// \param x[in]	The value to set.
		inline void setX(const bdFloat32 x);

		/// Set the state the 2nd element in the vector.
		/// \param y[in]	The value to set.
		inline void setY(const bdFloat32 y);

		/// Set the state the 3rd element in the vector.
		/// \param z[in]	The value to set.
		inline void setZ(const bdFloat32 z);

		/// Set the state the 4th element in the vector.
		/// \param w[in]	The value to set.
		inline void setW(const bdFloat32 w);

		/// Accessor for the data in the vector.
		/// \param v[out]	The array to write the data from the vector to.
		inline void get(bdFloat32 *const v) const;

		/// Accessor for a specific element in the vector.
		/// \param i[in]	The index of the element that is sought.
		/// \return	The value at index 'i' in the vector.
		inline bdFloat32 get(const bdUInt32 i) const;

		/// Accessor for the 1st element in the vector.
		/// \return	The value of the first element in  the vector.
		inline bdFloat32 getX() const;

		/// Accessor for the 2nd element in the vector.
		/// \return	The value of the second element in  the vector.
		inline bdFloat32 getY() const;

		/// Accessor for the 3rd element in the vector.
		/// \return	The value of the third element in  the vector.
		inline bdFloat32 getZ() const;

		/// Accessor for the 4th element in the vector.
		/// \return	The value of the fourth element in  the vector.
		inline bdFloat32 getW() const;

		/// Set every value in the vector to Zero.
		inline void zero();

		/// Add another vector to this one.
		/// \param v[in]	The vector to add to this one.
		inline void add4(const bdVector4 &v);

		/// Add the first three elements of another vector to the first three
		/// of this one.
		/// \param v[in]	The vector to add to this one.
		inline void add3(const bdVector4 &v);

		/// Assign the sum of two vectors to this one.
		/// \param v0[in]	The first vector.
		/// \param v1[in]	The second vector.
		/// \todo Does this behave as you might expect.
		inline void add4(const bdVector4 &v0,
						 const bdVector4 &v1);

		/// Assign the sum of two vectors to this one.
		/// Only operate on the first three elements in the vector.
		/// \param v0[in]	The first vector.
		/// \param v1[in]	The second vector.
		/// \todo Does this behave as you might expect.
		inline void add3(const bdVector4 &v0,
						 const bdVector4 &v1);

		/// Subtract another vector from this one.
		/// \param v[in]	The vector to subtract from this one.
		inline void subtract4(const bdVector4 &v);

		/// Subtract the first three elements of another vector from the first
		/// three of this one.
		/// \param v[in]	The vector to subtract from this one.
		inline void subtract3(const bdVector4 &v);

		/// Assign the difference of two vectors to this one.
		/// \param v0[in]	The first vector.
		/// \param v1[in]	The second vector.
		/// \todo Does this behave as you might expect.
		inline void subtract4(const bdVector4 &v0,
							  const bdVector4 &v1);

		/// Assign the difference of two vectors to this one.
		/// Only operate on the first three elements in the vector.
		/// \param v0[in]	The first vector.
		/// \param v1[in]	The second vector.
		/// \todo Does this behave as you might expect.
		inline void subtract3(const bdVector4 &v0,
							  const bdVector4 &v1);

		/// Multiply the first 3 elements in the vector by 's'.
		/// \param s[in]	The value to multiply the elements by.
		inline void multiply3(const bdFloat32 s);

		/// Multiply the first 3 elements in the vector by the first 3 in another vector.
		/// \param s[in]	The vector to multiply this on by.
		inline void multiply3(const bdVector4 &s);

		/// Get the squared length of the vector.
		/// \return The squared length of this vector.
		/// \note This function is quicker where comparisons only care about relative
		///  values as opposed to absolutes.
		inline bdFloat32 getLengthSq() const;

		/// Get the length of this vector.
		/// \return The length of this vector.
		/// \note consider using getLengthSq() if the return value is only being used
		///  for the purposes of comparison.
		inline bdFloat32 getLength() const;

		/// Assign the cross product of two vectors to this one.
		/// \param a[in]	The first vector.
		/// \param b[in]	The second vector.
		inline void cross3(const bdVector4 &a,
						   const bdVector4 &b);

		/// Get the dot product of this vector and another one.
		/// \param a[in]	The other vector.
		/// \return The dot product of the two vectors.
		inline bdFloat32 dot3(const bdVector4 &a);

		/// Normalize the vector.
		inline void normalize();

		/// Post multiply this vector by a matrix.
		/// \param s[in]	The matrix to multiply this vector by.
		inline void postMatrixMultiply(bdMatrix4 &s);

	protected:

		/// Store the vector elements an an array.
		bdFloat32 m_v[4];
};

#include <bdGraphics/bdVector4.inl>

#endif // BD_VECTOR4_H

