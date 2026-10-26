// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines templated functors that implement comparison operations.
//			Useful for templated container classes.

#ifndef BD_COMPARISON_FUNCTORS_H
#define BD_COMPARISON_FUNCTORS_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

/// This class provides a less-than comparison between two instances of a 
/// class in the form of a templated functor.
template <class T>
class bdLessThan
{
	public:
		
		/// Compare two instances of \c T.
		/// \param	x[in]	The potentially lesser instance.
		/// \param	y[in]	The potentially greater instance.
		/// \return
		/// -	True, if x < y
		/// -	False, if x > y
		inline bdBool operator() (const T& x,
								  const T& y) const
		{
			return x < y;
		}
};

/// This class provides a greater-than comparison between two instances of a 
/// class in the form of a templated functor.
template <class T>
class bdGreaterThan
{
	public:
		
		/// Compare two instances of \c T.
		/// \param	x[in]	The potentially greater instance.
		/// \param	y[in]	The potentially lesser instance.
		/// \return
		/// -	True, if x > y
		/// -	False, if x < y
		inline bdBool operator() (const T& x,
								  const T& y) const
		{
			return x > y;
		}
};

/// This class provides an equality comparison between two instances of a 
/// class in the form of a templated functor.
template <class T>
class bdEquals
{
	public:
		
		/// Compare two instances of \c T.
		/// \param	x[in]	An instance to check for equality.
		/// \param	y[in]	An instance to check for equality.
		/// \return
		/// -	True, if x == y
		/// -	False, if x != y
		inline bdBool operator() (const T& x,
								  const T& y) const
		{
			return x == y;
		}
};

#endif // BD_COMPARISON_FUNCTORS_H
