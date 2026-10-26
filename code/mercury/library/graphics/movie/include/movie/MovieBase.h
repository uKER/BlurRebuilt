#ifndef MOVIE_MOVIE_BASE_H
#define MOVIE_MOVIE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Movie/MovieInterface.h"

struct BINKFRAMEBUFFERS;

namespace Movie
{

extern Debug::Logger& g_MovieLog;

typedef void (*MovieFunctionPtr)( void );

class cMovieCallBackFunctor
{
public:
	virtual ~cMovieCallBackFunctor() {}
	virtual void operator()(void *param){};
	virtual void Call(void *param) {};
};

template <class TClass> class TMovieFunctor : public cMovieCallBackFunctor
{
private:
	void (TClass::*fpt)(void *);
	TClass* pt2Object;       

public:
	TMovieFunctor(TClass* _pt2Object, void(TClass::*_fpt)(void *)) 
	{ 
		pt2Object = _pt2Object;  
		fpt=_fpt; 
	};

	virtual void operator()(void *param)
	{ 
		(*pt2Object.*fpt)(param);
	};  

	virtual void Call(void *param)
	{ 
		(*pt2Object.*fpt)(param);
	};             
};

//! E_MovieCreateFlags

enum E_MovieCreateFlags
{
	E_MovieCreateFlags_StreamFromDisk		= 0x00000001,
	E_MovieCreateFlags_LoadIntoMemory		= 0x00000002,
	E_MovieCreateFlags_SetBufferIOSize		= 0x00000004,
	E_MovieCreateFlags_PlayFromMemory		= 0x00000008
};

//! E_MovieMemoryFlags

enum E_MovieMemoryFlags
{
	E_MovieMemoryFlags_System,
	E_MovieMemoryFlags_Graphics,
};

//! C_MovieParams

class C_MovieParams
{
public:

	C_MovieParams();
	~C_MovieParams();

	void Reset();

public:
	static const U32				m_NumTracks = 6;

	U32								m_CreateFlags;
	U32								m_MemoryFlags;
	U32								m_BinkIOSize;
	U32								m_TrackIDsToPlay[ m_NumTracks ];

	void*							m_pMovieData;
};

//! C_MovieBase

class C_MovieBase : public I_Movie
{
public:

	C_MovieBase();
	virtual ~C_MovieBase();

	const bool GetAudio() const;
	void SetAudio(const bool audio);

	const bool GetVideo() const;
	void SetVideo(const bool video);
	
	const bool GetPaused() const;
	void SetPaused(const bool paused);
	
	const bool GetLooping() const;
	void SetLooping(const bool looping);

	const S32 GetVolume() const;
	void SetVolume(const S32 volume);
	
	const Util::C_String& GetFileName() const;
	const F32 GetCurrentTime() const;
	const F32 GetTotalTime() const;

	C_MovieParams* GetParams();

	const Display::C_Texture2D* GetTexture();

	static void SetFileSystemCallBacks( cMovieCallBackFunctor* PauseFunction, cMovieCallBackFunctor* ResumeFunction );

	static cMovieCallBackFunctor*	mPauseFileSystem;
	static cMovieCallBackFunctor*	mResumeFileSystem;

	static void CalculateUVRange(const BINKFRAMEBUFFERS& binkBuffers, F32& uEnd0, F32& vEnd0, F32& uEnd1, F32& vEnd1);
	
protected:

	bool							m_Audio;
	bool							m_Video;
	bool							m_Paused;
	bool							m_Looping;
	F32								m_TotalTime;
	F32								m_CurrentTime;
	S32								m_Volume;
	Util::C_String					m_FileName;
	C_MovieParams					m_Params;
	Display::C_RenderTexture2D		m_RenderTexture;
};

}; // namespace Movie

#include "Movie/MovieBase.inl"

#endif // MOVIE_MOVIE_BASE_H
