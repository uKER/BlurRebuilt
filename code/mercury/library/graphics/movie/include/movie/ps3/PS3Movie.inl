#ifndef MOVIE_PS3_MOVIE_INL
#define MOVIE_PS3_MOVIE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Movie
{

INLINE bool C_PS3Movie::IsCreated() const
{
	return m_pBink ? true : false;
}

}; // namespace Movie

#endif // MOVIE_PS3_MOVIE_INL
