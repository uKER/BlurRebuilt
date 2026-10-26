//--------------------------------------------------------------------------------------
// AtgAudio.h
//
// Simple WAV file reader, XMA file writer and other audio utilities.  This file is 
// based off of the AtgAudio.h file in the samples common framework.
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once

#include <windows.h>
//#include <mmreg.h>      // For WAVEFORMATEX
#include <assert.h>


namespace Audio
{

//--------------------------------------------------------------------------------------
// Misc type definitions
//--------------------------------------------------------------------------------------
typedef DWORD FOURCC, *PFOURCC, *LPFOURCC;


//--------------------------------------------------------------------------------------
// Format tags
//--------------------------------------------------------------------------------------
#define WAVE_FORMAT_PCM                     1
#define WAVE_FORMAT_EXTENSIBLE              0xFFFE


//--------------------------------------------------------------------------------------
// For parsing WAV files
//--------------------------------------------------------------------------------------
#ifndef _WAVEFORMATEXTENSIBLE_
#define _WAVEFORMATEXTENSIBLE_

typedef struct 
{
    WAVEFORMATEX    Format;                 // WAVEFORMATEX data

    union 
    {
        WORD        wValidBitsPerSample;    // Bits of precision
        WORD        wSamplesPerBlock;       // Samples per block of audio data
        WORD        wReserved;              // Unused -- must be 0
    } Samples;

    DWORD           dwChannelMask;          // Channel usage bitmask
    GUID            SubFormat;              // Sub-format identifier
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE, *LPWAVEFORMATEXTENSIBLE;

typedef const WAVEFORMATEXTENSIBLE *LPCWAVEFORMATEXTENSIBLE;

#endif // _WAVEFORMATEXTENSIBLE_

//--------------------------------------------------------------------------------------
// Name: class XMAFile
// Desc: XMA file utility class (for use on the PC)
//--------------------------------------------------------------------------------------

// NEEDS TO WORK FOR AN IN MEMORY XMA AND ONE FROM DISK
// NOTE: only XMA2 is supported. 
// NOTE: i was half way through converting to XMA2 then I realised I cannot get the looping
//       subframe info, so for now this is still an XMA1.

class XMAFile
{
    HANDLE     m_hFile;            // File handle
    RiffChunk  m_RiffChunk;        // RIFF chunk
    RiffChunk  m_FormatChunk;      // Format chunk
    RiffChunk  m_DataChunk;        // Data chunk
    RiffChunk  m_WaveChunk;        // Wave chunk
    RiffChunk  m_SeekTableChunk;   // Seek table chunk

public:
    XMAFile();
    ~XMAFile();

	HRESULT OpenWrite( const CHAR* strFileName );
 
    HRESULT OpenRead( const CHAR* strFileName ); 

    // Get File format
	void* GetFormat() const { return m_FormatChunk.m_pData; }

	// Read file format from disk
    HRESULT ReadFormat();

	// read the sample from disk into an internal pointer
	HRESULT ReadSample() { return m_DataChunk.ReadData(); }

	// Get the sample data
	VOID* GetSampleData() const { return m_DataChunk.m_pData; }

    // File properties
    VOID    GetDuration( DWORD* pdwDuration ) const { *pdwDuration = m_DataChunk.GetDataSize(); }

	// encode a wavefile into an xma
	HRESULT Encode(Audio::CWaveFile* waveFile, U32& uNumSamples, U32 u32Flags=0, int iCompressionQuality=75, bool useXMA2Format = false);

	// write the xma file to disk
	HRESULT Write();

    VOID Close();
};

} // namespace Audio


