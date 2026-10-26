#ifndef MOVIE_MOVIE_INTERFACE_H
#define MOVIE_MOVIE_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Movie/MovieDefines.h"

namespace Movie
{

class C_MovieParams;

//! I_Movie

class I_Movie
{
public:

	virtual ~I_Movie() {};

	virtual const bool GetAudio() const = 0;
	virtual void SetAudio(const bool audio) = 0;

	virtual const bool GetVideo() const = 0;
	virtual void SetVideo(const bool video) = 0;
	
	virtual const bool GetPaused() const = 0;
	virtual void SetPaused(const bool paused) = 0;
	
	virtual const bool GetLooping() const = 0;
	virtual void SetLooping(const bool looping) = 0;

	virtual const Util::C_String& GetFileName() const = 0;

	virtual const Display::C_Texture2D* GetTexture() = 0;
	virtual const F32 GetCurrentTime() const = 0;
	virtual const F32 GetTotalTime() const = 0;

	virtual bool Open(const Util::C_String& fileName, const C_MovieParams& params) = 0;
	virtual void Close() = 0;
};

}; // namespace Movie

#endif // MOVIE_MOVIE_INTERFACE_H
