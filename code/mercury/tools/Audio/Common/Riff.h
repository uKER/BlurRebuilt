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
// For parsing WAV files
//--------------------------------------------------------------------------------------
struct RIFFHEADER
{
    FOURCC  fccChunkId;
    DWORD   dwDataSize;
};

struct RIFFDATA {
	FOURCC         chunkID;
	long           chunkSize;

//	data
};

#define RIFFCHUNK_FLAGS_VALID   0x00000001

struct RIFFFORMAT {
	FOURCC         chunkID;
	long           chunkSize;

	short          wFormatTag;
	unsigned short wChannels;
	unsigned long  dwSamplesPerSec;
	unsigned long  dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;

	/* Note: there may be additional fields here, depending upon wFormatTag. */
};

//--------------------------------------------------------------------------------------
// Name: class RiffChunk
// Desc: RIFF chunk utility class
//--------------------------------------------------------------------------------------
class RiffChunk	
{
public:
		typedef void*		(*AllocFuncPtr)(size_t size);
		typedef void		(*FreeFuncPtr)(void* ptr);

		static AllocFuncPtr	AllocPtr;
		static FreeFuncPtr	FreePtr;

		bool				m_bOverrideMemFuncs;

		void*				Alloc(size_t size);
		void				Free(void* ptr);

private:
	friend class XMAFile;
	friend class CWaveFile;

    FOURCC            m_fccChunkId;       // Chunk identifier
    RiffChunk*		  m_pParentChunk;     // Parent chunk
    HANDLE            m_hFile;
    DWORD             m_dwDataOffset;     // Chunk data offset
    DWORD             m_dwDataSize;       // Chunk data size
    DWORD             m_dwFlags;          // Chunk flags

public:
	VOID*			  m_pData;			
public:
    RiffChunk();
	~RiffChunk();

    // Initialization
    VOID    Initialize( FOURCC fccChunkId, RiffChunk* pParentChunk, HANDLE hFile );
    HRESULT Open();
    BOOL    IsValid() const { return !!(m_dwFlags & RIFFCHUNK_FLAGS_VALID); }

	VOID GetFourCCString( CHAR* strFourCC );

	// read all data into internal data pointer
	HRESULT ReadData();

    // Chunk information
    FOURCC  GetChunkId() const  { return m_fccChunkId; }
    DWORD   GetDataSize() const { return m_dwDataSize; }

	HRESULT FreeDataAndSetNew( VOID* pData, DWORD dwDataSize, bool bCopyData=true );
	HRESULT SetData( VOID* pData, DWORD dwDataSize, bool bCopyData=true );
    HRESULT Write( BOOL bWriteChunkSize );

private:
	    // read Data into given pointer at specified offset
    HRESULT ReadData( LONG lOffset, VOID* pData, DWORD dwDataSize ) const;
};

} // namespace Audio



