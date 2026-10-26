/////////////////////////////////////////////////////
// Algorithm
/////////////////////////////////////////////////////

#pragma once

#include "utility.h"
#include "sequence.h"

namespace Ut {

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
	for (typename Sequence::Iterator i = s.Begin(); i != s.End(); ++i)
		if (*i == e) return i;

	return i;
}

/// Return first element for which predicate returns true, or end of the sequence if not found.
template <class Sequence, class Predicate>
inline typename Sequence::Iterator FindIf(Sequence& s, Predicate& f)
{
	for (typename Sequence::Iterator i = s.Begin(); i != s.End(); ++i)
		if (f(*i)) return i;

	return i;
}

/// @}

} // end Ut
