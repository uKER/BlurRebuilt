#ifndef BTL_PAIR_H
#define BTL_PAIR_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////////
// Pair
//////////////////////////////////////////////////////////

namespace Btl
{

/// @addtogroup Misc
/// @{

/// An object containing two other objects.
template <typename First, typename Second>
class Pair
{
public:
	Pair() {}
	Pair(const First& f, const Second& s) : first(f), second(s) {} ///< @a

	First first; ///< @a 
	Second second; ///< @a 
};

/// Creates a Pair containing copies of the two function parameters.
template <typename First, typename Second>
Pair<First, Second> MakePair(const First& f, const Second& s)
{
	return Pair<First, Second>(f, s);
}

/// @a 
template <typename First, typename Second>
inline bool operator < (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	if (p2.first < p1.first)
		return false;
	else if (p1.first < p2.first || p1.second < p2.second)
		return true;
	else 
		return false;
}

/// @a 
template <typename First, typename Second>
inline bool operator > (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return p2 < p1;
}

/// @a 
template <typename First, typename Second>
inline bool operator >= (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return !(p1 < p2);
}

/// @a 
template <typename First, typename Second>
inline bool operator <= (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return !(p2 < p1);
}

/// @a 
template <typename First, typename Second>
inline bool operator == (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return p1.first == p2.first && p1.second == p2.second;
}

/// @a 
template <typename First, typename Second>
inline bool operator != (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return p1.first != p2.first || p1.second != p2.second;
}

/// @a
template <typename First, typename Second>
U32 Hash(const Pair<First, Second>& p)
{
	U32 hash = Hash(p.first);
	return Hash(p.second, hash);
}

/// @}

}

#endif
