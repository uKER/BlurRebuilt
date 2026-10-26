#ifndef MOVIE_PS3_MOVIE_H
#define MOVIE_PS3_MOVIE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Movie/PS3/Bink/BinkTextures.h"
#include "Movie/PS3/PS3Bink.h"

// [LloydW] Need to undef these types as naughty bink defined them!

#undef U8
#undef S8
#undef U16
#undef S16
#undef U32
#undef S32
#undef U64
#undef S64
#undef F32
#undef F64

#include "Movie/MovieBase.h"

namespace Movie
{

//! C_PS3Movie.

class C_PS3Movie : public C_MovieBase
{
public:

	C_PS3Movie();
	virtual ~C_PS3Movie();

	bool IsCreated() const;
	bool HasReachedEnd() const;

	void SetVolume(const S32 volume);
	void SetBinkIOSize(const U32 size);

	void Rewind();
	bool Update();
	void Render();

	void DebugRender(F32& x, F32& y, const F32 s);

protected:

	bool CreateBink(const C_MovieParams& params);
	void DestroyBink();

	bool SetupSpeakers();
	
	bool CreateShaders();
	void DestroyShaders();

	bool CreateTextures();
	void DestroyTextures();

	#if !CFG_GOLD
	void DrawFrameInfo();
	#endif

protected:

	bool				m_FirstFrame;
	bool				m_IsPlaying;
	HBINK				m_pBink;
	BINKTEXTURESET		m_BinkTextureSet;

	C_PS3Shaders		m_Shaders;
	U8*					m_pMovieData;
};

}; // namespace Movie

#include "Movie/PS3/PS3Movie.inl"

#endif // MOVIE_PS3_MOVIE_H
