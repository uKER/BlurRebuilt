#ifndef BTL_ALGORITHM_H
#define BTL_ALGORITHM_H
#ifdef _MSC_VER
#pragma once
#endif

/////////////////////////////////////////////////////
// Algorithm
/////////////////////////////////////////////////////

#include "sequence.h"

namespace Btl 
{

/////////////////////////////////////////////////////////

/// @addtogroup Containers
/// @{

/// Call a function (or function object) for each element in the sequence.
template <class Sequence, class Functor>
inline void ForEach(Sequence& s, Functor& f)
{
	for (typename Sequence::Iterator i = s.Begin(); i != s.End(); ++i)
		f(*i);
}

/// Return first element that equals input, or end of the sequence if not found.
template <class Sequence, typename Element>
inline typename Sequence::Iterator Find(Sequence& s, const Element& e)
{
	typename Sequence::Iterator i;
	for (i = s.Begin(); i != s.End(); ++i)
		if (*i == e) return i;

	return i;
}

/// Return first element that equals input, or end of the sequence if not found.
template <class Sequence, typename Element>
inline typename Sequence::ConstIterator Find(const Sequence& s, const Element& e)
{
	typename Sequence::ConstIterator i;
	for (i = s.Begin(); i != s.End(); ++i)
		if (*i == e) return i;

	return i;
}

/// Replace elements element that equal the test value.
template <class Sequence, typename Element>
inline void Replace(Sequence& s, const Element& value, const Element& newValue)
{
	for (typename Sequence::Iterator i = s.Begin(); i != s.End(); ++i)
		if (*i == value) 
			*i = newValue;
}

/// Return first element for which predicate returns true, or end of the sequence if not found.
template <class Sequence, class Predicate>
inline typename Sequence::Iterator FindIf(Sequence& s, Predicate& f)
{
	typename Sequence::Iterator i;
	for (i = s.Begin(); i != s.End(); ++i)
		if (f(*i)) return i;

	return i;
}

/// Advance a forward iterator by a specified number of steps.
template <class Iterator>
inline void Advance(Iterator& i, U32 iValue)
{
	for(U32 iCurrent=0; iCurrent<iValue; ++iCurrent, ++i)
		;

	return;
}

/// @}

}

#endif
