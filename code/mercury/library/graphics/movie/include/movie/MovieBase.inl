#ifndef MOVIE_MOVIE_BASE_INL
#define MOVIE_MOVIE_BASE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Movie
{

INLINE const bool C_MovieBase::GetAudio() const
{
	return m_Audio;
}

INLINE void C_MovieBase::SetAudio(const bool audio)
{
	m_Audio = audio;
}

INLINE const bool C_MovieBase::GetVideo() const
{
	return m_Video;
}

INLINE void C_MovieBase::SetVideo(const bool video)
{
	m_Video = video;
}

INLINE const bool C_MovieBase::GetPaused() const
{
	return m_Paused;
}

INLINE void C_MovieBase::SetPaused(const bool paused)
{
	m_Paused = paused;
}

INLINE const bool C_MovieBase::GetLooping() const
{
	return m_Looping;
}

INLINE void C_MovieBase::SetLooping(const bool looping)
{
	m_Looping = looping;
}

INLINE const S32 C_MovieBase::GetVolume() const
{
	return m_Volume;
}

INLINE void C_MovieBase::SetVolume(const S32 volume)
{
	m_Volume = volume;
}

INLINE const Util::C_String& C_MovieBase::GetFileName() const
{
	return m_FileName;
}

INLINE C_MovieParams* C_MovieBase::GetParams()
{
	return &m_Params;
}

INLINE const Display::C_Texture2D* C_MovieBase::GetTexture()
{
	return &m_RenderTexture.GetTexture();
}

INLINE const F32 C_MovieBase::GetTotalTime() const
{
	return m_TotalTime;
}

INLINE const F32 C_MovieBase::GetCurrentTime() const
{
	return m_CurrentTime;
}

}; // namespace Movie

#endif // MOVIE_MOVIE_BASE_INL
