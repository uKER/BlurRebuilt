#ifndef MOVIE_W32_MOVIE_H
#define MOVIE_W32_MOVIE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Bink.h"
#include "Movie/W32/Bink/D3D11BinkTextures.h"
#include "Movie/MovieBase.h"

namespace Movie
{

//! C_W32Movie

class C_W32Movie : public C_MovieBase
{
public:

	C_W32Movie();
	virtual ~C_W32Movie();

	bool IsCreated() const;
	bool HasReachedEnd() const;

	void SetPaused(const bool paused);
	void SetVolume(const S32 volume);
	void SetBinkIOSize(const U32 size);

	static bool CreateShaders();
	static void DestroyShaders();

	void Rewind();
	bool Update();
	void Render();
	
	void DebugRender(F32& x, F32& y, const F32 s);

protected:

	bool CreateBink(const C_MovieParams& params);
	void DestroyBink();

	bool CreateTextures();
	void DestroyTextures();

public:

	S32					m_BinkIsDirty;
	bool				m_BinkIsPlaying;
	HBINK				m_pBink;
	BINKTEXTURESET		m_BinkTextureSet;
};

}; // namespace Movie

#include "Movie/W32/W32Movie.inl"

#endif // RENDER_W32_MOVIE_H
