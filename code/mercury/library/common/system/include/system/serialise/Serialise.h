#ifndef __SYSTEM_SERIALISE_H__
#define __SYSTEM_SERIALISE_H__

namespace Sys
{

template<typename S, typename T>
FORCE_INLINE void Serialise(S& s, const T& t)
{
	((T&)t).Serialise(s);
}

template<typename S, typename T>
FORCE_INLINE void Deserialise(S& s, T& t)
{
	t.Deserialise(s);
}

}

#endif

