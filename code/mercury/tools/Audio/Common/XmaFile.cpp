//--------------------------------------------------------------------------------------
// AtgAudio.cpp
//
// Simple WAV file reader, XMA file writer and other audio utilities.  This file is 
// based off of the AtgAudio.h file in the samples common framework.
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"

#include "riff.h"
#include "XmaFile.h"

#include "XmaEncoderDll.h"

namespace Audio
{
//--------------------------------------------------------------------------------------
// FourCC definitions
//--------------------------------------------------------------------------------------
const DWORD AUDIO_FOURCC_RIFF   = 'FFIR';	//"RIFF"
const DWORD AUDIO_FOURCC_WAVE   = 'EVAW'; //"WAVE"
const DWORD AUDIO_FOURCC_FORMAT = ' tmf'; //"fmt "
const DWORD AUDIO_FOURCC_DATA   = 'atad'; //"data"
const DWORD AUDIO_FOURCC_WSMP   = 'pmsw'; //"wsmp"
const DWORD AUDIO_FOURCC_SMPL   = 'pmsl'; //"lsmp"
const DWORD AUDIO_FOURCC_SEEK   = 'kees'; //"seek"

XMAFile::XMAFile()
{
    m_hFile = INVALID_HANDLE_VALUE;
}


//--------------------------------------------------------------------------------------
// Name: ~XMAFile()
// Desc: Destructor
//--------------------------------------------------------------------------------------
XMAFile::~XMAFile()
{
    Close();
}

//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Opens the file for writing and initializes the object
//--------------------------------------------------------------------------------------
HRESULT XMAFile::OpenWrite( const CHAR* strFileName )
{
    HRESULT hr = S_OK;

    Close();

    // Open the file
    m_hFile = CreateFile( strFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( m_hFile == INVALID_HANDLE_VALUE )
        return HRESULT_FROM_WIN32( GetLastError() );

    // Initialize the chunk objects
    m_RiffChunk.Initialize( AUDIO_FOURCC_RIFF, NULL, m_hFile );
    m_FormatChunk.Initialize( AUDIO_FOURCC_FORMAT, &m_RiffChunk, m_hFile );
    m_DataChunk.Initialize( AUDIO_FOURCC_DATA, &m_RiffChunk, m_hFile );
    m_WaveChunk.Initialize( AUDIO_FOURCC_WAVE, &m_RiffChunk, m_hFile );
    m_SeekTableChunk.Initialize( AUDIO_FOURCC_SEEK, &m_RiffChunk, m_hFile );

    return hr;
}

//--------------------------------------------------------------------------------------
// Name: Open()
// Desc: Opens the file for reading and initializes the object
//--------------------------------------------------------------------------------------
HRESULT XMAFile::OpenRead( const CHAR* strFileName )
{
    // If we're already open, close
    Close();
    
    // Open the file
    m_hFile = CreateFile( strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0L, NULL );
    if( m_hFile == INVALID_HANDLE_VALUE )
        return HRESULT_FROM_WIN32( GetLastError() );

    // Initialize the chunk objects
    m_RiffChunk.Initialize( AUDIO_FOURCC_RIFF, NULL, m_hFile );
    m_FormatChunk.Initialize( AUDIO_FOURCC_FORMAT, &m_RiffChunk, m_hFile );
    m_DataChunk.Initialize( AUDIO_FOURCC_DATA, &m_RiffChunk, m_hFile );
    m_WaveChunk.Initialize( AUDIO_FOURCC_WAVE, &m_RiffChunk, m_hFile );
    m_SeekTableChunk.Initialize( AUDIO_FOURCC_SEEK, &m_RiffChunk, m_hFile );

    HRESULT hr = m_RiffChunk.Open();
    if( FAILED(hr) )
        return hr;

    hr = m_FormatChunk.Open();
    if( FAILED(hr) )
        return hr;

    hr = m_DataChunk.Open();
    if( FAILED(hr) )
        return hr;

    // Validate the file type
    FOURCC fccType;
    hr = m_RiffChunk.ReadData( 0, &fccType, sizeof(fccType) );
    if( FAILED(hr) )
        return hr;

    if( AUDIO_FOURCC_WAVE != fccType )
        return HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: GetFormat()
// Desc: Gets the wave file format.  Since Xbox 360 only supports WAVE_FORMAT_PCM,
//       and WAVE_FORMAT_EXTENSIBLE, we know any
//       valid format will fit into a WAVEFORMATEXTENSIBLE struct
//--------------------------------------------------------------------------------------
HRESULT XMAFile::ReadFormat()
{
    //assert( pxmaFormat );
    DWORD dwValidSize = m_FormatChunk.GetDataSize();

    // Anything larger than WAVEFORMATEXTENSIBLE is not a valid Xbox WAV file
    assert( dwValidSize <= sizeof(WAVEFORMATEXTENSIBLE) );

    // Read the format chunk into the buffer
	HRESULT hr = m_FormatChunk.ReadData();
    if( FAILED(hr) )
        return hr;

    // Zero out remaining bytes, in case enough bytes were not read
    //if( dwValidSize < sizeof(WAVEFORMATEXTENSIBLE) )
    //    ZeroMemory( (BYTE*)m_FormatChunk.m_pData + dwValidSize, sizeof(WAVEFORMATEXTENSIBLE) - dwValidSize );
    
    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: Encode()
// Desc: Given a wavfile will encode it into an xma
//--------------------------------------------------------------------------------------
HRESULT XMAFile::Encode( Audio::CWaveFile* waveFile, U32& uNumSamples, U32 u32Flags, int iCompressionQuality, bool useXMA2Format)
{
	HRESULT hr = S_OK;

	WAVEFORMATEX* wfx;

	BYTE* pbData = 0;
	DWORD dwWaveSize = 0;
	XMAENCODERSTREAM encoderStream;

	memset(&encoderStream, 0, sizeof(XMAENCODERSTREAM));

	// Get the format
	wfx = waveFile->GetFormatEx();

	if (wfx==NULL)
	{
		PrintError("EncodeXma() : wave Format is NULL\n");
		return E_FAIL;
	}

	// Verify that the source wave does not have too many channels
	if( wfx->nChannels > XMA_MAXINPUTCHANNELCOUNT )
	{
		PrintError("You must specify source waves with %d or less channels.\n", XMA_MAXINPUTCHANNELCOUNT );
		PrintError("The source file %S has %d channels.\n", "TODO", wfx->nChannels );
		return E_FAIL;
	}

	if (wfx->wBitsPerSample != 16)
	{
		PrintError("Sample must be 16 bits. Its %d bits.\n", wfx->wBitsPerSample);
		return E_FAIL;
	}

	if (iCompressionQuality < 0 || iCompressionQuality > 100)
	{
		PrintError("Compression quality is %d. It must be between 1 and 100.\n", iCompressionQuality );
		return E_FAIL;
	}

	// Find out how big the wave data is
	waveFile->GetDataSize( &dwWaveSize );

	// Set the stream to use our wave data
	pbData = (BYTE*)waveFile->GetSampleData();

	if (pbData==NULL)
	{
		PrintError("EncodeXma() : WaveFile sample data is NULL\n");
		return E_FAIL;
	}

	if (u32Flags & XMAENCODER_LOOP)
	{
		// get the looping info from the wavefile if the file is a loop.
		// otherwise the xma encoder will loop the whole wave.
		if (waveFile->IsLoop())
		{
			waveFile->GetLoopRegion(&encoderStream.LoopStart, &encoderStream.LoopLength);
		}
		else
		{
			encoderStream.LoopStart = 0; 
			if (waveFile->GetNumSamples(&encoderStream.LoopLength) != S_OK)
			{
				PrintError("EncodeXma() : Could not get number of samples\n");
				return E_FAIL;
			}
		}
	}
	

	// Configure the encoder stream
	// NOTE: Speaker assignments currently only work correctly for mono source files
	// To make them work for stereo sources you'd need to set the speaker assignment
	// for the second channel.

	// TODO: WHAT HAPPENS TO A STEREO XMA?

	encoderStream.SpeakerAssignment[0] = 0;
	encoderStream.SpeakerAssignment[1] = 1;

	// Set the size of the input wave data
	encoderStream.BufferSize = dwWaveSize;
	encoderStream.pBuffer	= pbData;

	// Copy the wave format to the XMA format
	memset(&encoderStream.Format, 0, sizeof(WAVEFORMATEXTENSIBLE));
	memcpy(&encoderStream.Format, wfx, waveFile->m_FormatChunk.GetDataSize());

	// set the mem free function to to be the free function from the dll's heap.
	// this is because XMaInMemoryEncoder (which is called from the dll) allocs
	// the memory internally and expects the caller to free it. only thing is
	// we cant free it from here because it was alloced using a different heap.
	// so we free using the same heap it was allocated with (ie the dll's heap).

	m_DataChunk.m_bOverrideMemFuncs = true;
	m_DataChunk.FreePtr = DllFree;
	m_FormatChunk.m_bOverrideMemFuncs = true;
	m_FormatChunk.FreePtr = DllFree;
	m_SeekTableChunk.m_bOverrideMemFuncs = true;
	m_SeekTableChunk.FreePtr = DllFree;

	if (gSettings.gbVerboseMode)
	{
		DWORD dwNumSamples;
		waveFile->GetNumSamples(&dwNumSamples);
		Print(0, "\n%d samples in original source\n", dwNumSamples);
		Print(0, "source loop start = %d, source loop length = %d\n", encoderStream.LoopStart, encoderStream.LoopLength);
	}

	if (useXMA2Format)
	{
		hr = XMA2Encode(1,							// number of streams. TODO: 5.1 xmas
						&encoderStream,
						iCompressionQuality,				
						u32Flags|XMAENCODER_NOFILTER,		// flags. TODO: set looping flag here and allow change of NO_FILTER flag
						64,									// block size in kb. using the same as Xact uses for streaming and non-streaming banks
						&m_DataChunk.m_pData,						// out
						&m_DataChunk.m_dwDataSize,					// out
						(XMA2WAVEFORMAT**)&m_FormatChunk.m_pData,	// out 
						&m_FormatChunk.m_dwDataSize,				// out
						(DWORD**)&m_SeekTableChunk.m_pData,			// out
						&m_SeekTableChunk.m_dwDataSize);			// out

		uNumSamples = ((XMA2WAVEFORMAT*)m_FormatChunk.m_pData)->SamplesEncoded;

		EndianSwitchDWords((DWORD*)&uNumSamples, 1);

		if (gSettings.gbVerboseMode)
			Print(0, "%d samples encoded (returned from encoder)\n", uNumSamples);
	}
	else
	{
		hr = XMAEncode( 1,							// number of streams. TODO: 5.1 xmas
						&encoderStream,
						iCompressionQuality,				
						u32Flags|XMAENCODER_NOFILTER,		// flags. TODO: set looping flag here and allow change of NO_FILTER flag
						&m_DataChunk.m_pData,						// out
						&m_DataChunk.m_dwDataSize,					// out
						(XMAWAVEFORMAT**)&m_FormatChunk.m_pData,	// out 
						&m_FormatChunk.m_dwDataSize,				// out
						(DWORD**)&m_SeekTableChunk.m_pData,			// out
						&m_SeekTableChunk.m_dwDataSize);			// out

		waveFile->GetNumSamples((DWORD*)&uNumSamples);

		uNumSamples = (uNumSamples / 128 + ((uNumSamples % 128 ==0) ? 0:1)) * 128;

		if (gSettings.gbVerboseMode)
		{
			static bool bPrintedInfo=false;
			Print(0, "%d samples encoded\n", uNumSamples);
			Print(0, "Loop start offset bit = %d, Loop end offset bit = %d. Subframe skip = %d. Subframe end = %d\n", 
				((XMAWAVEFORMAT*)m_FormatChunk.m_pData)->XmaStreams[0].LoopStart,
				((XMAWAVEFORMAT*)m_FormatChunk.m_pData)->XmaStreams[0].LoopEnd,
				((XMAWAVEFORMAT*)m_FormatChunk.m_pData)->XmaStreams[0].SubframeData & 0xf,
				((XMAWAVEFORMAT*)m_FormatChunk.m_pData)->XmaStreams[0].SubframeData >> 4);
			if (bPrintedInfo==false)
			{
				Print(0, "Info: samples encoded calculated by rounding source samples to 128. Was NOT returned by encoder\n");
				Print(0, "Info: Subframe end = Subframe number of loop end point [0,3].\n");
				Print(0, "Info: Subframe skip = Number of subframes to skip before decoding and outputting at the loop start point [1,4].\n");
				bPrintedInfo = true;
			}
		}
	}

	if (hr!=S_OK)
	{
		if (hr == XMAENCODER_E_OUTOFMEMORY)
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'OUT OF MEMORY'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_INVALIDARG)
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'INVALID ARG'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_NOTIMPL)         
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'NOT IMPL'  ???\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_FAIL)            
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'FAIL'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_INVALIDLOOP)
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'INVALID LOOP'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_UNSUPPORTEDFORMAT)
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'UNSUPPORTED FORMAT'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_OUTOFRANGE)  
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'OUT OF RANGE'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_READFAILED)
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'READ FAILED'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_INVALIDFRAME)
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'INVALID FRAME'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_INCOMPLETEFRAME)
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'INCOMPLETE FRAME'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_ZEROSTUFFINGBIT)
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'ZERO STUFFING BIT'\n", waveFile->ReadFileName().Chars(), hr);
		else
		if (hr == XMAENCODER_E_GETDATAFAILED)
			PrintError("XmaEncode() : could not encode file '%s'. \nError code (0x%x) 'GETDATA FAILED'\n", waveFile->ReadFileName().Chars(), hr);
		else
			PrintError("XmaEncode() : could not encode file '%s'. Unknown Error code (0x%x)\n", waveFile->ReadFileName().Chars(), hr);

		return E_FAIL;
	}
	
	

	// XMA2WAVEFORMAT struct needs endian swapping before you use it - if looking at it on PC
	//EndianSwitchDWords((DWORD*)&((XMA2WAVEFORMAT*)m_FormatChunk.m_pData)->LoopBegin, 3);
	//EndianSwitchDWords((DWORD*)&((XMA2WAVEFORMAT*)m_FormatChunk.m_pData)->PsuedoBytesPerSec, 5);

	return S_OK;
}
  
//--------------------------------------------------------------------------------------
// Name: Write()
// Desc: Writes the RIFF chunks to the file
// TODO: if we have read in an XMA from a file then the SeekTableChunk will not have been setup
// so the xma will be missing the seek table chunk when written back out.
//--------------------------------------------------------------------------------------
HRESULT XMAFile::Write( /*VOID* pBuffer, DWORD dwBufferSize,
                        VOID* pFormat, DWORD dwFormatSize,
                        VOID* pSeekTable, DWORD dwSeekTableSize*/ )
{
    if( m_hFile == INVALID_HANDLE_VALUE )
        return E_FAIL;

    // Set the data for each chunk
    /*m_FormatChunk.SetData( pFormat, dwFormatSize );
    m_DataChunk.SetData( pBuffer, dwBufferSize );
    m_SeekTableChunk.SetData( pSeekTable, dwSeekTableSize );*/

	// not sure if this is the right thing to do but it seems to produce the right results
	m_RiffChunk.m_dwDataSize = m_FormatChunk.m_dwDataSize + m_DataChunk.m_dwDataSize + m_SeekTableChunk.m_dwDataSize;

    // Write the chunks
    m_RiffChunk.Write( TRUE );
    m_WaveChunk.Write( FALSE );		// what is this WaveChunk for?
    m_FormatChunk.Write( TRUE );
    m_DataChunk.Write( TRUE );
    m_SeekTableChunk.Write( TRUE );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: Close()
// Desc: Closes the object
//--------------------------------------------------------------------------------------
VOID XMAFile::Close()
{
    if( m_hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

} // namespace Audio
