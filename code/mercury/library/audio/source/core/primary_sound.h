//-----------------------------------------------------------------------------
// File: DSUtil.h
//
// Desc: 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef DSUTIL_H
#define DSUTIL_H

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include <core/core.h>



//-----------------------------------------------------------------------------
// Classes used by this header
//-----------------------------------------------------------------------------
class CPrimarySound;
class CWaveFile;

namespace Audio
{
	namespace Core
	{
		class Engine;
	}
}


//-----------------------------------------------------------------------------
// Typing macros 
//-----------------------------------------------------------------------------
#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2

#define DSUtil_StopSound(s)         { if(s) s->Stop(); }
#define DSUtil_PlaySound(s)         { if(s) s->Play( 0, 0 ); }
#define DSUtil_PlaySoundLooping(s)  { if(s) s->Play( 0, DSBPLAY_LOOPING ); }


//-----------------------------------------------------------------------------
//	CPrimarySound - used by Audio::Renderer
//-----------------------------------------------------------------------------
class CPrimarySound
{
public:
	CPrimarySound( LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, Audio::Core::Engine* pEngine, DWORD dwNotifySize );
    ~CPrimarySound();

    HRESULT HandleWaveStreamNotification(void* inputBuffer, U32 frameLength, U32 channels, S32& lag);
    HRESULT Reset();

    HRESULT RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored );

    HRESULT ClearBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger );
	LPDIRECTSOUNDBUFFER GetBuffer() { return m_pDSBuffer; }
    HRESULT Play(DWORD dwPriority = 0, DWORD dwFlags = 0);
    HRESULT Stop();

private:
	LPDIRECTSOUNDBUFFER m_pDSBuffer;
    DWORD m_dwDSBufferSize;
    DWORD m_dwCreationFlags;

    DWORD m_dwLastPlayPos;
    DWORD m_dwPlayProgress;
    DWORD m_dwNotifySize;
    DWORD m_dwNextWriteOffset;
	DWORD m_dwLastWriteIndex;

	Audio::Core::Engine* m_pEngine;

};




//-----------------------------------------------------------------------------
// Name: class CWaveFile
// Desc: Encapsulates reading or writing sound data to or from a wave file
//-----------------------------------------------------------------------------
class CWaveFile
{
public:
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    HMMIO         m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO      m_ck;          // Multimedia RIFF chunk
    MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
    DWORD         m_dwSize;      // The size of the wave file
    MMIOINFO      m_mmioinfoOut;
    DWORD         m_dwFlags;
    BOOL          m_bIsReadingFromMemory;
    BYTE*         m_pbData;
    BYTE*         m_pbDataCur;
    ULONG         m_ulDataSize;
    CHAR*         m_pResourceBuffer;

protected:
    HRESULT ReadMMIO();
    HRESULT WriteMMIO( WAVEFORMATEX *pwfxDest );

public:
    CWaveFile();
    ~CWaveFile();

    HRESULT Open( LPTSTR strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags );
    HRESULT OpenFromMemory( BYTE* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx, DWORD dwFlags );
    HRESULT Close();

    HRESULT Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead );
    HRESULT Write( UINT nSizeToWrite, BYTE* pbData, UINT* pnSizeWrote );

    DWORD   GetSize();
    HRESULT ResetFile();
    WAVEFORMATEX* GetFormat() { return m_pwfx; };
};




#endif // DSUTIL_H
