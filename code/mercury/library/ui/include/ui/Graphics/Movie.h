// Movie.h
#ifndef MOVIE_H
#define MOVIE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Movie/Movie.h>

#include "ui/Graphics/UITextureManager.h"
#include "ui/Graphics/Simple2D.h"

namespace UI
{
extern char *gHddDirectorySearchPath;

#define MAX_MOVIE_SLOTS			12

class Movie
{
public:
	Movie();
	~Movie();

	void Create
	(
		const S32	index, 
		const char*	filename, 
		const bool	in_memory, 
		const U32	io_memory_size = 0, 
		const bool	bStillRenderTheWorld = false, 
		const bool	bBackgroundMovie = false,
		const S32	volume = 32768  // This is normal. 0 - 65536
	);

	void Reset();
	void Destroy();	
	bool Update();
	bool IsPaused(void) const	{ return !m_Active; }
	bool IsCreated(void) const	{ return m_pMovie && m_pMovie->IsCreated() ? true : false; }
	void Pause(const bool pause);
	void Looping(const bool loop);
	void UseSound(bool useSound) { m_bUseSound = useSound; }
	bool StillRenderTheWorld()		{ return m_bStillRenderTheWorld; }
	bool IsBackgroundMovie() { return m_bIsBackgroundMovie; }
	void SetBackgroundMovie(bool background) { m_bIsBackgroundMovie = background;};
	void SetTrackIDsToPlay(const U32 track0, const U32 track1, const U32 track2, const U32 track3, const U32 track4, const U32 track5);
	void DebugRender(F32& x, F32& y, const F32 s);
	void SetVolume(S32 vol){if(m_pMovie) m_pMovie->SetVolume( vol );};

	UITextureHandle GetTexture( ) const;

	const ::Movie::C_Movie*			MoviePlayer()		{ return m_pMovie; }

private:
	bool							m_Active;
	bool							m_bMemoryResident;		// It's held in memory - no DVD access goodness!!!
	bool							m_bUseSound;
	S32								m_Index;
	::Movie::C_Movie*				m_pMovie;
	::Movie::C_MovieParams			m_MovieParams;
	bool							m_bStillRenderTheWorld;
	bool							m_bIsBackgroundMovie;
	bool							m_bNeedToRestoreBackgroundMusic;
	S32								m_Volume;  
};

extern Movie gMovie[MAX_MOVIE_SLOTS];

// Class for adding movies as alternate simple2d textures
class C_MovieAlternateTexture : public C_Simple2D::I_AlternateTexture
{
public:
	C_MovieAlternateTexture( const S32 index ) : m_Index( index )
	{
		m_Name.Printf("MOVIE%d",index);
	}

	UITextureHandle				Handle( )	{ return gMovie[ m_Index ].GetTexture(); }
	const Util::C_String32&		Name( )		{ return m_Name; }

private:
	S32					m_Index;
	Util::C_String32	m_Name;
};

}; // namespace UI

#endif
