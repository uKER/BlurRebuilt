#ifndef MOVIE_W32_MOVIE_INL
#define MOVIE_W32_MOVIE_INL

namespace Movie
{

INLINE bool C_W32Movie::IsCreated() const
{
	return m_pBink ? true : false;
}

}; // namespace Movie

#endif // MOVIE_W32_MOVIE_INL
