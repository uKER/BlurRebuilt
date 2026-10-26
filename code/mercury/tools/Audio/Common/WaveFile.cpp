#include "stdafx.h"

#include "riff.h"
#include "WaveFile.h"
// secret rabbit sample rate converter code
#include "samplerate.h"


namespace Audio
{
	//-----------------------------------------------------------------------------
	// FourCC definitions
	//-----------------------------------------------------------------------------

	//const DWORD FOURCC_RIFF   = 'FFIR';
	const DWORD FOURCC_WAVE   = 'EVAW';	// WAVE
	const DWORD FOURCC_FORMAT = ' tmf'; // fmt 
	const DWORD FOURCC_DATA   = 'atad'; // data
	const DWORD FOURCC_WSMP   = 'pmsw'; // wsmp
	const DWORD FOURCC_SMPL   = 'lpms'; // smpl
	const DWORD FOURCC_CUE	  = ' euc'; // cue 
	const DWORD FOURCC_ADTL   = 'ltda'; // adtl
	const DWORD FOURCC_LABL   = 'lbal'; // labl
	//const DWORD FOURCC_LIST   = 'TSIL'; // LIST

	//-----------------------------------------------------------------------------
	// Name: CWaveFile()
	// Desc: Object constructor.
	//-----------------------------------------------------------------------------
	CWaveFile::CWaveFile()
	{
		m_hFile = INVALID_HANDLE_VALUE;
	}

	//-----------------------------------------------------------------------------
	// Name: ~CWaveFile()
	// Desc: Object destructor.
	//-----------------------------------------------------------------------------
	CWaveFile::~CWaveFile()
	{	
		CUE_CHUNK* pCueData = (CUE_CHUNK*)m_CueChunk.m_pData;

		if (pCueData != NULL)
		{		
			if (pCueData->CueList.Size())
			{
				Ut::Vector<CUE_POINT*>::Iterator it;
				for (it = pCueData->CueList.Begin(); it != pCueData->CueList.End(); ++it)
				{
					delete (*it);
				}
			}
			pCueData->CueList.Destroy();
		}

		LIST_CHUNK* pLabelData = (LIST_CHUNK*)m_ListChunk.m_pData;

		if (pLabelData != NULL)
		{
			if (pLabelData->LabelList.Size())
			{
				Ut::Vector<LABEL*>::Iterator it;
				for (it = pLabelData->LabelList.Begin(); it != pLabelData->LabelList.End(); ++it)
				{
					delete (*it)->cueLabel;
					delete (*it);
				}
			}
			pLabelData->LabelList.Destroy();
		}

		Close();
	}

	//-----------------------------------------------------------------------------
	// Name: LogChunkHeaders()
	// Desc: Prints out the chunk list for a wav file
	//-----------------------------------------------------------------------------
	void CWaveFile::LogChunkHeaders(const char* pchFilename)
	{
		LONG lOffset = 0;

		Audio::Print(0, "Printing chunk list for %s...\n", pchFilename);
		
		// skip RIFF chunk to get to the WAVE chunk
		lOffset = m_RiffChunk.m_dwDataOffset;
		lOffset += sizeof(FOURCC);

		// Read each child chunk header until we find the one we're looking for
		for( ;; )
		{
			if( INVALID_SET_FILE_POINTER == SetFilePointer( m_hFile, lOffset, NULL, FILE_BEGIN ) )
				return;

			RIFFHEADER rhRiffHeader;
			DWORD dwRead;
			if( 0 == ReadFile( m_hFile, &rhRiffHeader, sizeof(rhRiffHeader), &dwRead, NULL ) )
				return;

			// Hit EOF 
			if( 0 == dwRead )
				return;
			
			Audio::Print(0, "Chunk = '%c%c%c%c', size = %d bytes\n", *((char*)&rhRiffHeader.fccChunkId), *(((char*)&rhRiffHeader.fccChunkId)+1), *(((char*)&rhRiffHeader.fccChunkId)+2), *(((char*)&rhRiffHeader.fccChunkId)+3), rhRiffHeader.dwDataSize);

			lOffset += sizeof(rhRiffHeader) + rhRiffHeader.dwDataSize;
		}

		Audio::Print(0, "done.\n");
	}

	//-----------------------------------------------------------------------------
	// Name: LogFormatChunk()
	// Desc: 
	//-----------------------------------------------------------------------------
	void CWaveFile::LogFormatChunk()
	{
		Print(0, "LogFormatChunk()...\n");

		Audio::Print(0, "  chunk size is %d bytes\n", m_FormatChunk.GetDataSize());

		WAVEFORMATEX* pFormat = GetFormatEx();
		if (pFormat == NULL)
		{
			PrintWarning("m_FormatChunk.m_pData is NULL!\n");
			return;
		}

		if (pFormat->wFormatTag == WAVE_FORMAT_PCM)
		{
			if (m_FormatChunk.GetDataSize() != sizeof(PCMWAVEFORMAT))
			{
				PrintWarning("PCM m_FormatChunk data size (%d bytes) does not match PCMWAVEFORMAT size (%d bytes). BAD?\n", m_FormatChunk.GetDataSize(), sizeof(PCMWAVEFORMAT));
			}
		}
		else
		{
			if (m_FormatChunk.GetDataSize() != sizeof(WAVEFORMATEX))
			{
				PrintWarning("non PCM m_FormatChunk data size (%d bytes) does not match WAVEFORMATEX size (%d bytes). BAD?\n", m_FormatChunk.GetDataSize(), sizeof(WAVEFORMATEX));
			}
		}

		Print(0, "  wFormatTag = %d (%s)\n", pFormat->wFormatTag, pFormat->wFormatTag==1 ? "PCM" :"non PCM");
		Print(0, "  nChannels = %d\n", pFormat->nChannels);
		Print(0, "  nSamplesPerSec = %d\n", pFormat->nSamplesPerSec);
		Print(0, "  nAvgBytesPerSec = %d\n", pFormat->nAvgBytesPerSec);
		Print(0, "  nBlockAlign = %d\n", pFormat->nBlockAlign);
		Print(0, "  wBitsPerSample = %d\n", pFormat->wBitsPerSample);
		// if PCM format cbSize does not exist
		if (pFormat->wFormatTag != WAVE_FORMAT_PCM)
		{
			Print(0, "  cbSize (additional data size) = %d\n", pFormat->cbSize);
		}
	}

	//-----------------------------------------------------------------------------
	// Name: LogSamplerChunk()
	// Desc: Prints out the chunk list for a wav file
	//-----------------------------------------------------------------------------
	void CWaveFile::LogSamplerChunk()
	{
		Print(0, "\nLogSamplerChunk()...\n");

		SAMPLER_CHUNK* pChunk = GetLoopInfo();
		if (pChunk == NULL)
		{
			Print(0, "  Sampler Chunk does not exist\n\n");
			return;
		}

		if (m_SamplerChunk.GetDataSize() != sizeof(SAMPLER_CHUNK))
		{
			PrintWarning("m_SamplerChunk data size (%d bytes) does not match SAMPLER_CHUNK size (%d bytes). BAD?\n", m_SamplerChunk.GetDataSize(), sizeof(SAMPLER_CHUNK));
		}

		Print(0, "  ChunkHeader.ulManufacturer= %d (0x%x)\n", pChunk->ChunkHeader.ulManufacturer, pChunk->ChunkHeader.ulManufacturer);
		Print(0, "  ChunkHeader.ulMidiPitchFraction= %d (0x%x)\n", pChunk->ChunkHeader.ulMidiPitchFraction, pChunk->ChunkHeader.ulMidiPitchFraction);
		Print(0, "  ChunkHeader.ulMidiUnityNote= %d\n", pChunk->ChunkHeader.ulMidiUnityNote);
		Print(0, "  ChunkHeader.ulNumSampleLoops= %d (0x%x)\n", pChunk->ChunkHeader.ulNumSampleLoops, pChunk->ChunkHeader.ulNumSampleLoops);
		Print(0, "  ChunkHeader.ulProduct= %d (0x%x)\n", pChunk->ChunkHeader.ulProduct, pChunk->ChunkHeader.ulProduct);
		Print(0, "  ChunkHeader.ulSamplePeriod= %d (0x%x)\n", pChunk->ChunkHeader.ulSamplePeriod, pChunk->ChunkHeader.ulSamplePeriod);
		Print(0, "  ChunkHeader.ulSamplerData= %d (0x%x)\n", pChunk->ChunkHeader.ulSamplerData, pChunk->ChunkHeader.ulSamplerData);
		Print(0, "  ChunkHeader.ulSmpteFormat= %d\n", pChunk->ChunkHeader.ulSmpteFormat);
		Print(0, "  ChunkHeader.ulSmpteOffset= %d (0x%x)\n", pChunk->ChunkHeader.ulSmpteOffset, pChunk->ChunkHeader.ulSmpteOffset);

		Print(0, "  SampleLoop.ulCuePointId= %d (0x%x)\n", pChunk->SampleLoop.ulCuePointId, pChunk->SampleLoop.ulCuePointId);
		Print(0, "  SampleLoop.ulType= %d (0x%x)\n", pChunk->SampleLoop.ulType, pChunk->SampleLoop.ulType);
		Print(0, "  SampleLoop.ulStart= %d (0x%x)\n", pChunk->SampleLoop.ulStart, pChunk->SampleLoop.ulStart);
		Print(0, "  SampleLoop.ulEnd= %d (0x%x)\n", pChunk->SampleLoop.ulEnd, pChunk->SampleLoop.ulEnd);
		Print(0, "  SampleLoop.ulFraction= %d (0x%x)\n", pChunk->SampleLoop.ulFraction, pChunk->SampleLoop.ulFraction);
		Print(0, "  SampleLoop.ulPlayCount= %d (0x%x)\n\n", pChunk->SampleLoop.ulPlayCount, pChunk->SampleLoop.ulPlayCount);
	}

	//-----------------------------------------------------------------------------
	// Name: OpenForRead()
	// Desc: Initializes the object.
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::OpenForRead( const CHAR* strFileName )
	{
		// If we're already open, close
		Close();
	    
		// Open the file
		m_hFile = CreateFile( strFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
							OPEN_EXISTING, 0L, NULL );
		if( INVALID_HANDLE_VALUE == m_hFile )
			return HRESULT_FROM_WIN32( GetLastError() );

		DWORD dwFileSize = GetFileSize(m_hFile,  NULL);

		if (dwFileSize == 0)
		{
			PrintError("CWaveFile::OpenForRead(%s) - File size is 0!\n", strFileName);
			return E_FAIL;
		}

		// Initialize the chunk objects
		m_RiffChunk.Initialize( FOURCC_RIFF, NULL, m_hFile );
		m_FormatChunk.Initialize( FOURCC_FORMAT, &m_RiffChunk, m_hFile );
		m_DataChunk.Initialize( FOURCC_DATA, &m_RiffChunk, m_hFile );
		m_SamplerChunk.Initialize( FOURCC_SMPL, &m_RiffChunk, m_hFile );
		m_CueChunk.Initialize( FOURCC_CUE, &m_RiffChunk, m_hFile );
		m_ListChunk.Initialize( FOURCC_LIST, &m_RiffChunk, m_hFile );

		HRESULT hr = m_RiffChunk.Open();
		if( FAILED(hr) )
		{
			PrintError("CWaveFile::OpenForRead(%s) - Could not open riff chunk\n", strFileName);
			return hr;
		}

		// debug
		#ifdef _DEBUG
		LogChunkHeaders(strFileName);
		#endif

		// format chunk must exist
		hr = m_FormatChunk.Open();
		if( FAILED(hr) )
		{
			PrintError("CWaveFile::OpenForRead(%s) - Could not open format chunk\n", strFileName);
			return hr;
		}

		// data chunk must exist
		hr = m_DataChunk.Open();
		if( FAILED(hr) )
		{
			PrintError("CWaveFile::OpenForRead(%s) - Could not open data chunk\n", strFileName);
			return hr;
		}

		// sampler chunk is optional
		// Wave Sample chunk is not required and from some testing with wav files never exists
		m_SamplerChunk.Open();
		
		// Open cue points if they exist
		m_CueChunk.Open();

		// Open the list chunk if we have cues
		m_ListChunk.Open();
	
		// Validate the file type
		FOURCC fccType;
		hr = m_RiffChunk.ReadData( 0, &fccType, sizeof(fccType) );
		if( FAILED(hr) )
			return hr;

		if( FOURCC_WAVE != fccType )
			return HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );

		m_strReadFilename = strFileName;

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: OpenForWrite()
	// Desc: Initializes the object.
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::OpenForWrite( const CHAR* strFileName )
	{
		// If we're already open, close
		Close();
	    
		// Open the file
		m_hFile = CreateFile( strFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
							CREATE_ALWAYS, 0L, NULL );
		if( INVALID_HANDLE_VALUE == m_hFile )
			return HRESULT_FROM_WIN32( GetLastError() );

		m_strWriteFilename = strFileName;

		m_RiffChunk.Initialize( FOURCC_RIFF, NULL, m_hFile );
		m_FormatChunk.Initialize( FOURCC_FORMAT, &m_RiffChunk, m_hFile );
		m_DataChunk.Initialize( FOURCC_DATA, &m_RiffChunk, m_hFile );
		m_SamplerChunk.Initialize( FOURCC_SMPL, &m_RiffChunk, m_hFile );

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: ReadFormat()
	// Desc: Reads the wave file format from disk
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::ReadFormat()
	{
		DWORD dwValidSize = m_FormatChunk.GetDataSize();

		//Audio::Print("chunk size is %d\n", dwValidSize);

		// Anything larger than WAVEFORMATEXTENSIBLE is not a valid Xbox WAV file
		assert( dwValidSize <= sizeof(WAVEFORMATEXTENSIBLE) );

		assert(m_FormatChunk.m_pData==NULL);

		// Read the format chunk into the buffer
		HRESULT hr = m_FormatChunk.ReadData();
		if( FAILED(hr) )
			return hr;

		// Zero out remaining bytes, in case enough bytes were not read
		//if( dwValidSize < sizeof(WAVEFORMATEXTENSIBLE) )
		//	ZeroMemory( (BYTE*)m_FormatChunk.m_pData + dwValidSize, sizeof(WAVEFORMATEXTENSIBLE) - dwValidSize );
	 
		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: Read()
	// Desc: Reads bytes from the file into a given buffer
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::Read( VOID* pBuffer, DWORD dwBytesToRead, DWORD* pdwRead )
	{
		DWORD dwBytesRead;
		DWORD* pdwBytesRead=&dwBytesRead; 
		if (pdwRead)
			pdwBytesRead = pdwRead;

		if (ReadFile(m_hFile, pBuffer, dwBytesToRead, pdwBytesRead, NULL)==false)
			return HRESULT_FROM_WIN32( GetLastError() );

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: ReadLoopInfo()
	// Desc: Reads the sampler chunk to get the looping information
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::ReadLoopInfo()
	{
		HRESULT hr = S_OK;
		SAMPLER_CHUNK *ws;
		
		// Check to see if there is a sampler chunk.
		// if none then just return
		if( !m_SamplerChunk.IsValid() )
			return S_OK;
	    
		ws = (SAMPLER_CHUNK*)m_SamplerChunk.Alloc(sizeof(SAMPLER_CHUNK));
		memset(ws, 0, sizeof(SAMPLER_CHUNK));

		// Read the sampler struct from the chunk
		hr = m_SamplerChunk.ReadData( 0, &ws->ChunkHeader, sizeof( SAMPLER_CHUNK_HEADER ) );
		if( FAILED( hr ) )
		{
			free(ws);
			return hr;
		}

		if (ws->ChunkHeader.ulNumSampleLoops == 0)
		{
			free(ws);
			//Audio::Print(0, "Sampler chunk exists but no loops are present.");
			return S_OK;
		}

		// Currently, only 1 loop region is supported
		if(ws->ChunkHeader.ulNumSampleLoops != 1)
		{
			free(ws);
			PrintWarning("Wav has %d loops. Only 1 loop is supported.\n", ws->ChunkHeader.ulNumSampleLoops);
			return E_FAIL;
		}

		// Read the loop region
		hr = m_SamplerChunk.ReadData( sizeof( SAMPLER_CHUNK_HEADER ), (void*)&ws->SampleLoop, sizeof( SAMPLE_LOOP ) );
		if( FAILED( hr ) )
		{
			free(ws);
			return hr;
		}
	    
		m_SamplerChunk.SetData((void*)ws, sizeof(SAMPLER_CHUNK), false);

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: ReadCuesInfo()
	// Desc: Reads the cue chunk to get the cue markers
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::ReadCueInfo()
	{
		HRESULT hr = S_OK;
		CUE_CHUNK *ws;
		
		// Check to see if there is a cue chunk.
		// if none then just return
		if( !m_CueChunk.IsValid() )
			return S_OK;
	    
		//ws = (CUE_CHUNK*)m_CueChunk.Alloc(sizeof(CUE_CHUNK));
		//memset(ws, 0, sizeof(CUE_CHUNK));

		// Find out how many cue points we have so we know how much memory to allocate to ws
		ULONG numCuePoints;
		hr = m_CueChunk.ReadData( 0, &numCuePoints, sizeof( ULONG ) );

		Print(0, "  %d Cue points\n", numCuePoints);

		// Read the cue struct from the chunk
		ws = (CUE_CHUNK*)m_CueChunk.Alloc( sizeof( CUE_CHUNK_HEADER ) + ( sizeof(CUE_POINT) * numCuePoints ) );
		memset(ws, 0, sizeof( CUE_CHUNK_HEADER ) + ( sizeof( CUE_POINT ) * numCuePoints ) );

		hr = m_CueChunk.ReadData( 0, &ws->CueHeader, sizeof( CUE_CHUNK_HEADER ) );
		if( FAILED( hr ) )
		{
			free(ws);
			return hr;
		}

		for (U32 cueCount = 0; cueCount < ws->CueHeader.ulNumCuePoints; cueCount++)
		{
			CUE_POINT* cp = new CUE_POINT();
			hr = m_CueChunk.ReadData( sizeof( CUE_CHUNK_HEADER ) + ( sizeof( CUE_POINT )* cueCount), cp, sizeof( CUE_POINT ));

			if( FAILED( hr ) )
			{
				free(ws);
				return hr;
			}

			ws->CueList.PushBack(cp);

			Print(0, "  %d : sample offset %d, cue id %d\n", cueCount, cp->ulSampleOffset, cp->ulCueID);
		}
	    
		m_CueChunk.SetData((void*)ws, sizeof(ws), false);

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: ReadListInfo()
	// Desc: Reads the sampler chunk to get the looping information
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::ReadListInfo()
	{
		HRESULT hr = S_OK;
		LIST_CHUNK *ws;
		
		// Check to see if there is a sampler chunk.
		// if none then just return
		if( !m_ListChunk.IsValid() )
			return S_OK;
	    
		ws = (LIST_CHUNK*)m_ListChunk.Alloc(sizeof(LIST_CHUNK));
		memset(ws, 0, sizeof(LIST_CHUNK));

		// Read the list header struct from the chunk
		hr = m_ListChunk.ReadData( 0, &ws->ListHeader, sizeof( LIST_CHUNK_HEADER ) );
		if( FAILED( hr ) )
		{
			free(ws);
			return hr;
		}

		DWORD tempOffset = sizeof( LIST_CHUNK_HEADER );

		// Get the type
		DWORD tempFourCC;
		m_ListChunk.ReadData( tempOffset, &tempFourCC, sizeof( FOURCC ));

		// Check if we support the type of list
		if (tempFourCC == FOURCC_LABL)
		{
			while ( tempOffset < m_ListChunk.m_dwDataSize )
			{
				LABEL* lb = new LABEL();

				m_ListChunk.ReadData( tempOffset, &lb->ulChunkID, sizeof( FOURCC ));
				tempOffset += sizeof( FOURCC );

				m_ListChunk.ReadData( tempOffset, &lb->ulChunkDataSize, sizeof( ULONG ));
				tempOffset += sizeof( ULONG );

				m_ListChunk.ReadData( tempOffset, &lb->ulDataChunkID, sizeof( ULONG ));
				tempOffset += sizeof( ULONG );

				DWORD uStrLen = lb->ulChunkDataSize - sizeof( ULONG );
				lb->cueLabel = new char[uStrLen];
				/*for (U32 charCount = 0; charCount < lb->ulChunkDataSize - sizeof( ULONG ) - 1; charCount++)
				{
					CHAR tempChar; 
					m_ListChunk.ReadData( tempOffset + charCount, &tempChar, 1);
					lb->cueLabel[charCount] = tempChar;
				}*/

				m_ListChunk.ReadData( tempOffset, lb->cueLabel, uStrLen-1);
				lb->cueLabel[uStrLen-1] = 0;
				
				// read past label
				tempOffset += lb->ulChunkDataSize - sizeof( ULONG );

				// align to 2 byte boundary
				if (tempOffset % 2 != 0)
					tempOffset += 1; 

				ws->LabelList.PushBack(lb);

				Print(0, "  Label %d: '%s'\n", lb->ulDataChunkID, lb->cueLabel);
			}
		}
		else
		{
			free(ws);
			return S_OK;
		}
	    
		m_ListChunk.SetData((void*)ws, sizeof(ws), false);

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: CopyInCueData()
	// Desc: Fills in the MarkerChunk of the pBankWave
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::CopyInCueData(WaveBank::Wave* pBankWave)
	{
		Print(0, "  Copying cue data...\n");

		int numMarkers = 0;
		pBankWave->m_pMarkerChunk = new MarkerChunk();

		pBankWave->m_pMarkerChunk->uTag = m_CueChunk.GetChunkId();
		pBankWave->m_pMarkerChunk->uSize = m_CueChunk.GetDataSize();
		pBankWave->m_pMarkerChunk->uType = m_CueChunk.GetChunkId();
		pBankWave->m_pMarkerChunk->uNumMarkers = 0;

		CUE_CHUNK* pCueData = (CUE_CHUNK*)m_CueChunk.m_pData;

		LIST_CHUNK* pListChunk = 0;

		if (m_ListChunk.IsValid())
			pListChunk = (LIST_CHUNK*)m_ListChunk.m_pData;

		Ut::Vector<CUE_POINT*>::Iterator it;
		int markerCount = 0;
		for (it = pCueData->CueList.Begin(); it != pCueData->CueList.End(); ++it)
		{
			Marker* pMarker = new Marker();
			pMarker->uMarkerID = (*it)->ulCueID;
			pMarker->uSampleOffset = (*it)->ulSampleOffset;	
			if (pListChunk)
			{
				//strcpy_s(pMarker->pchName, markerNameSize, pListChunk->LabelList.At(markerCount++)->cueLabel);
				const char* pchSrc = pListChunk->LabelList.At(markerCount++)->cueLabel;
				strcpy_s(pMarker->pchName, strlen(pchSrc)+1, pchSrc);
			}
			pBankWave->m_pMarkerChunk->MarkerList.PushBack(pMarker);
			Print(0, "  Adding marker %d, offset %d, '%s'\n", pMarker->uMarkerID, pMarker->uSampleOffset, pMarker->pchName);
		}

		pBankWave->m_pMarkerChunk->uNumMarkers = markerCount;

		Print(0, "  marker count is %d\n", markerCount);

		pBankWave->m_baseHeader.uMarkerChunkSize = sizeof( MarkerChunk ) + ( sizeof( Marker ) * markerCount );
		Print(0, "  pBankWave->m_baseHeader.uMarkerChunkSize = %d\n", pBankWave->m_baseHeader.uMarkerChunkSize);

		return S_OK;
	}


	//-----------------------------------------------------------------------------
	// Name: SampleRateConvert()
	// Desc: Converts the sample rate of the sample data given a sample rate
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::SampleRateConvert(U32 uSampleRate, int iQuality)
	{
		WAVEFORMATEX* pFormat = GetFormatEx();

		if (pFormat == NULL)
		{
			PrintError("CWaveFile::SampleRateConvert(): Could not get wav format.\n");
			return E_FAIL;
		}

		F64 fRatio = (uSampleRate/(F64)pFormat->nSamplesPerSec);
		return SampleRateConvertF64(fRatio, false, iQuality);
	}

	//-----------------------------------------------------------------------------
	// Name: SampleRateConvert()
	// Desc: Converts the sample rate of the sample data given a F32 ratio
	//		 if bAlterPitch is true this will change the duration/pitch of the sample 
	//		 but leave the sample rate intact. the sample will be resampled by the 
	//	     required ratio then the sample rate set back to the original sample rate.
	//		 this is useful for quantizing samples.
	//		 if just require a straight resample then set uAlterPitch to false.
	//-----------------------------------------------------------------------------
	// Note on the fact you may not get as many samples as you ask for...
	//
	// from a post on the internet, from the author of libsamplerate:
	// http://music.columbia.edu/pipermail/music-dsp/2004-April/060003.html
	//
	// > Erik, can that be given its scale as simply the number of samples the
	// > result will contain and then meet that specification exactly?
	// [sam: in other words, can we ask for a specific number of samples?]
	//
	// No, but is should get pretty damn close. If you have N input samples
	// and you want M output samples, simply calculate the conversion ratio 
	// as:
	//
	// 	ratio = ((double) M) / N ;
	//
	// and then run the converter, using src_simple as follows:
	//
	// 	SRC_DATA data ;
	// 	int channels = 1 ; /* Can be more than 1; channels are interleaved. */
	//
	// 	data.data_in = input_buffer ; /* User supplied float* buffer */
	// 	data.data_out = output_buffer ; /* User supplied float* buffer */
	//
	// 	data.input_frames = N ;
	// 	data.output_frames = M ;
	//
	// 	data.src_ratio = ((double) M) / N ;
	//
	//	error = src_simple (&data, converter_type, channels) ;
	//	if (error != 0)
	//	puts (src_strerror (error)) ;
	//
	// After that call (assuming error == 0), data.output_frames_gen should be
	// M +/- 1. 
	//
	//The reason it isn't exactly M is that SRC was designed to be used with
	//a constant stream of input samples and fixing the termination conditions
	//more exactly would hinder the perfromance in streaming applications.
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::SampleRateConvertF64(F64 fRatio, bool bAlterPitch, int iQuality, U32 uNumSamplesReq)
	{
		// nothing to do - sample rate is already the requested rate
		if (fRatio == 1.0f)
		{
			return S_OK;
		}

		if (m_DataChunk.IsValid()==false)
		{
			PrintError("CWaveFile::SampleRateConvert(): Data is not valid.\n");
			return E_FAIL;
		}

		WAVEFORMATEX* pFormat = GetFormatEx();

		if (pFormat == NULL)
		{
			PrintError("CWaveFile::SampleRateConvert(): Could not get wav format.\n");
			return E_FAIL;
		}

		if (pFormat->wBitsPerSample != 16)
		{
			PrintError("CWaveFile::SampleRateConvert(): Bits per sample was %d and not 16 as required.\n", pFormat->wBitsPerSample);
			return E_FAIL;
		}

		S16* ps16Src = (S16*)GetSampleData();

		if (!ps16Src)
		{
			PrintError("CWaveFile::SampleRateConvert(): Sample data is NULL.\n", pFormat->wBitsPerSample);
			return E_FAIL;
		}

		if (iQuality < SRC_SINC_BEST_QUALITY || iQuality > SRC_LINEAR)
		{
			PrintError("CWaveFile::SampleRateConvert(): Quality value of %d is out of range. Expected value between %d and %d.\n", iQuality, SRC_SINC_BEST_QUALITY, SRC_LINEAR);
			return E_FAIL;
		}

		DWORD dwNumSamplesIn = 0;
		if (GetNumSamples(&dwNumSamplesIn) != S_OK)
		{
			PrintError("CWaveFile::SampleRateConvert(): Could not get number of samples.\n");
			return E_FAIL;
		}

		// ALL UNITS ARE IN SAMPLES UNLESS SPECIFIED

		// convert to F32 for the src requires this
		S32 s32LengthIn = dwNumSamplesIn * pFormat->nChannels; 
		F32* pfFloatDataIn = new F32[s32LengthIn];
		if (!pfFloatDataIn)
			return E_FAIL;
		#ifdef _DEBUG
		memset(pfFloatDataIn, 0, sizeof(F32)*s32LengthIn);
		#endif
		src_short_to_float_array(ps16Src, pfFloatDataIn, s32LengthIn);
				
		SRC_DATA srcData;
		U32 uNewSampleRate = (U32)(pFormat->nSamplesPerSec * fRatio);
		if (!uNumSamplesReq)
			Print(0, "'%s': Sample rate converting to %d hz...Q%d ", GetRawFilename(m_strReadFilename.Chars()).Chars(), uNewSampleRate, iQuality);

		// use ceil for samples where s32LengthIn * fRatio is fractionally off eg 48767.9999999999999?
		//U32 uRequestedLength = (U32) /*ceil*/((F64)s32LengthIn * fRatio);
		U32 uRequestedLength = uNumSamplesReq ? uNumSamplesReq * pFormat->nChannels: (U32) ((F64)s32LengthIn * fRatio);

		F32 *pfConverted = new F32[uRequestedLength];

		if (!pfConverted)
		{
			delete [] pfFloatDataIn;
			return E_FAIL;
		}

		// from libsamplerate docs...

		// "The input_frames and output_frames fields supply the converter with the lengths of the arrays (in frames) pointed
		// to by the data_in and data_out pointers respectively. For monophinc data, these values would indicate the length of
		// the arrays while for multi channel data these values would be equal to the the length of the array divided by the number of channels.

		// Finally, the fRatio field specifies the conversion ratio defined as the input sample rate divided by the output sample rate. 

		// The input_frames_used and output_frames_gen fields are set by the converter to inform the caller of the number of frames consumed
		// from the data_in array and the number of frames generated in the data_out array respectively. These values are for the current call to src_process only."

		srcData.data_in = pfFloatDataIn;
		srcData.input_frames = dwNumSamplesIn;
        srcData.data_out = pfConverted;
		srcData.output_frames = uRequestedLength / pFormat->nChannels;
		srcData.src_ratio = bAlterPitch ? fRatio : fRatio;
		srcData.end_of_input = 1;
		
		// do the conversion
		int error = src_simple(&srcData, 4, pFormat->nChannels);

		if (error)
		{
			delete [] pfFloatDataIn;
			delete [] pfConverted;
			PrintError("Failed. Error returned was '%s'\n", src_strerror(error));
			return E_FAIL;
		}	

		//Print(0, "\nSRC asked for %d & gen. %d frames. gave %d & used %d frames. ratio %f. input frames * ratio = %f\n", srcData.output_frames, srcData.output_frames_gen, srcData.input_frames, srcData.input_frames_used, srcData.src_ratio, srcData.input_frames * srcData.src_ratio);

		// did the src use all the given frames?
		if (srcData.input_frames != srcData.input_frames_used)
		{
			if (srcData.input_frames > srcData.input_frames_used)
			{
				if (srcData.input_frames - srcData.input_frames_used > 1)
					PrintWarning("CWaveFile::SampleRateConvert(): %d frames not used. num used = %d, num input = %d\n", srcData.input_frames-srcData.input_frames_used, srcData.input_frames_used, srcData.input_frames);
				#ifdef _DEBUG
				else
					PrintWarning("CWaveFile::SampleRateConvert(): 1 frame not used. num used = %d, num input = %d\n", srcData.input_frames_used, srcData.input_frames);
				#endif
			}
			else
			if (srcData.input_frames < srcData.input_frames_used)
			{
				PrintError("CWaveFile::SampleRateConvert(): more frames used that gernerated! Should not be possible. num used = %d, num input = %d\n", srcData.input_frames_used, srcData.input_frames);
				return false;
			}
		}

		// did we get how many frames we asked for?
		if (srcData.output_frames != srcData.output_frames_gen)
		{
			if (srcData.output_frames_gen < srcData.output_frames)
			{
				if (srcData.output_frames - srcData.output_frames_gen > 1)
				{
					PrintWarning("CWaveFile::SampleRateConvert(): %d frames too few were generated. num generated = %d, num requested = %d\n", srcData.output_frames - srcData.output_frames_gen, srcData.output_frames_gen, srcData.output_frames);
					return false;
				}
				#ifdef _DEBUG
				else
					PrintWarning("CWaveFile::SampleRateConvert(): 1 frame too few was generated. num generated = %d, num requested = %d\n", srcData.output_frames_gen, srcData.output_frames);
				#endif
			}
		}

		// convert back to S16
		S32 s16Length = srcData.output_frames /*srcData.output_frames_gen*/ * pFormat->nChannels;
		//S16* ps16Data = new S16[s16Length];
		S16* ps16Data = (S16*)m_DataChunk.Alloc(s16Length*sizeof(S16));
		if (!ps16Data)
		{
			delete [] pfFloatDataIn;
			delete [] pfConverted;
			return E_FAIL;
		}
		#ifdef _DEBUG
		memset(ps16Data, 0, sizeof(S16)*s16Length);
		#else
		// pad with 0s if we didnt get enough samples generated
		if (srcData.output_frames_gen < srcData.output_frames)
			memset(ps16Data, 0, sizeof(S16)*s16Length);
		#endif

		src_float_to_short_array (pfConverted, ps16Data, s16Length);

		// free original wavedata
		//m_DataChunk.Free(ps16Src);
		
		// free F32 data
		delete [] pfFloatDataIn;
		delete [] pfConverted;

		// set new data size and sample rate
		pFormat->nSamplesPerSec = uNewSampleRate;
		pFormat->nAvgBytesPerSec = uNewSampleRate * (pFormat->wBitsPerSample / 8);

		m_DataChunk.FreeDataAndSetNew(ps16Data, s16Length*sizeof(S16), false);
		//m_DataChunk.m_pData = ps16Data;
		//m_DataChunk.m_dwDataSize = s16Length*sizeof(S16);

		// correct sample loops
		SAMPLER_CHUNK* pSamplerChunk = GetLoopInfo();
		if (pSamplerChunk)
		{
			pSamplerChunk->SampleLoop.ulStart = (ULONG)(pSamplerChunk->SampleLoop.ulStart * fRatio);
			pSamplerChunk->SampleLoop.ulEnd =  (ULONG)(pSamplerChunk->SampleLoop.ulEnd * fRatio);
			if (pSamplerChunk->SampleLoop.ulEnd > (ULONG)srcData.output_frames_gen-1) 
				pSamplerChunk->SampleLoop.ulEnd = (ULONG)srcData.output_frames_gen-1;
		}

		if (!uNumSamplesReq)
			Print(TXTCOLOUR_GREEN, "ok\n");

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: QuantizeInsertDelete()
	// Desc: Quantizes a sample by inserting/removing samples
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::QuantizeInsertDelete(DWORD dwNumBytesIn, DWORD dwNumBytesOut, U32 uQuantize)
	{
		// nothing to do
		if (dwNumBytesIn == dwNumBytesOut)
		{
			return S_OK;
		}

		if (m_DataChunk.IsValid()==false)
		{
			PrintError("CWaveFile::QuantizeInsertDelete(): Data is not valid.\n");
			return E_FAIL;
		}

		WAVEFORMATEX* pFormat = GetFormatEx();

		if (pFormat == NULL)
		{
			PrintError("CWaveFile::QuantizeInsertDelete(): Could not get wav format.\n");
			return E_FAIL;
		}

		if (pFormat->wBitsPerSample != 16)
		{
			PrintError("CWaveFile::QuantizeInsertDelete(): Bits per sample was %d and not 16 as required.\n", pFormat->wBitsPerSample);
			return E_FAIL;
		}

		BYTE* pDataIn = (BYTE*)GetSampleData();

		if (!pDataIn)
		{
			PrintError("CWaveFile::QuantizeInsertDelete(): Sample data is NULL.\n");
			return E_FAIL;
		}

		Audio::Print(0, "'%s': Quantizing to %d samples by %s %d samples...", 
			GetRawFilename(m_strReadFilename.Chars()).Chars(), uQuantize, 
			dwNumBytesIn > dwNumBytesOut ? "removing" : "inserting", 
			dwNumBytesIn > dwNumBytesOut ? (dwNumBytesIn/2 - dwNumBytesOut/2) / pFormat->nChannels : (dwNumBytesOut/2 - dwNumBytesIn/2) / pFormat->nChannels);

		DWORD InOffset = 0;
		DWORD OutOffset = 0;
		int iChannels = pFormat->nChannels;

		BYTE* pDataOut = new BYTE[dwNumBytesOut];
		if (!pDataOut)
		{
			PrintError("CWaveFile::QuantizeInsertDelete(): Could not allocate %d bytes for output sample.\n", dwNumBytesOut);
			return E_FAIL;
		}

		#ifdef _DEBUG
		memset (pDataOut, 0, dwNumBytesOut);
		#endif

		//	diff
		if (dwNumBytesIn != dwNumBytesOut)
		{
			int nChunks = (((int)dwNumBytesOut - (int)dwNumBytesIn) / iChannels) / 2;

			if (nChunks < 0)
			{
				//	remove
				nChunks *= -1;

				short* pSampleIn = (short*)pDataIn;
				short* pSampleOut = (short*)pDataOut;

				int iOffset = 0;

				//if (g_bVerbose)
					printf ("from ", nChunks);

				float fChunkLen = (float)dwNumBytesIn/2 / (float)nChunks;

				//	chunk
				for (int iChunk=0; iChunk<nChunks; iChunk++)
				{
					int iGradient = 0xffff;
					int iStartAt = int((fChunkLen * iChunk + 0.5f) / iChannels) * iChannels;
					int iThisChunkLength = (int(((fChunkLen * (iChunk + 1))+0.5f) / iChannels) * iChannels) - iStartAt;
					
					//	find shallowest combined gradient for all channels
					for (int iCount = iChannels; iCount < iThisChunkLength - iChannels; iCount += iChannels)
					{
						short iTemp1stChannel = (abs(pSampleIn[iStartAt + iCount - iChannels] - pSampleIn[iStartAt + iCount]) + abs(pSampleIn[iStartAt + iCount] - pSampleIn[iStartAt + iCount+ iChannels]));
						short iTemp2ndChannel = 0;
						if (iChannels == 2)
							iTemp2ndChannel = (abs(pSampleIn[iStartAt + iCount - iChannels + 1] - pSampleIn[iStartAt + iCount + 1]) + abs(pSampleIn[iStartAt + iCount + 1] - pSampleIn[iStartAt + iCount + iChannels + 1]));
						int iTemp = iTemp1stChannel + iTemp2ndChannel;

						if (iTemp < iGradient)
						{
							iOffset = iCount;
							iGradient = iTemp;
						}
					}
					
					//if (g_bVerbose)
					{
						printf ("%d", iStartAt + iOffset);
						if (iChunk < nChunks-1)
							printf (", ");
						else
							printf ("\n");
					}

					//	copy to output
					for (int iStart=0; iStart<iOffset; iStart += iChannels)
					{
						DWORD dwWritePos = iStartAt-(iChunk * iChannels)+iStart;
						if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwNumBytesOut)))
							break;
						pSampleOut[dwWritePos] = pSampleIn[iStartAt+iStart];
						if (iChannels == 2)
						{
							dwWritePos = iStartAt-(iChunk * iChannels) + iStart + 1;
							if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwNumBytesOut)))
								break;
							pSampleOut[dwWritePos] = pSampleIn[iStartAt+iStart + 1];
						}
					}

					int iEnd;
					for (iEnd=iOffset; iEnd <= iThisChunkLength; iEnd += iChannels)
					{
						DWORD dwWritePos = iStartAt-(iChunk * iChannels)+iEnd;
						if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwNumBytesOut)))
							break;
						pSampleOut[dwWritePos] = pSampleIn[iStartAt+iEnd];
						if (iChannels == 2)
						{
							dwWritePos = iStartAt-(iChunk * iChannels)+iEnd + 1;
							if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwNumBytesOut)))
								break;
							pSampleOut[dwWritePos] = pSampleIn[iStartAt+iEnd+iChannels + 1];
						}
					}

					//printf("%d of %d", iStartAt-iChunk+iEnd, dwNumBytesOut/2);
				}
			}
			else
			{
				//	insert
				short* pSampleIn = (short*)pDataIn;
				short* pSampleOut = (short*)pDataOut;

				int iOffset = 0;

				//if (g_bVerbose)
					printf ("at ");

				float fChunkLen = (float)(dwNumBytesIn / 2) / (float)nChunks;

				//	chunk
				for (int iChunk=0; iChunk<nChunks; iChunk++)
				{
					int iGradient = 0xffff;
					int iStartAt = int((fChunkLen * iChunk + 0.5f) / iChannels) * iChannels;

					int iThisChunkLength = (int(((fChunkLen * (iChunk + 1))+0.5f) / iChannels) * iChannels) - iStartAt;
				
					//	find shallowest gradient
					// iThisChunkLength IS 0! iThisChunkLength - iChannels IS A HUGE NUMBER
					for (int iCount = iChannels; iCount < iThisChunkLength - iChannels; iCount += iChannels)
					{
						int iTemp;
						short iTemp1stChannel = (abs(pSampleIn[iStartAt + iCount - iChannels] - pSampleIn[iStartAt + iCount]) + abs(pSampleIn[iStartAt + iCount] - pSampleIn[iStartAt + iCount + iChannels]));
						short iTemp2ndChannel = 0;

						if (iChannels == 2)
						{
							iTemp2ndChannel = (abs(pSampleIn[iStartAt + iCount - iChannels + 1] - pSampleIn[iStartAt + iCount + 1]) + abs(pSampleIn[iStartAt + iCount + 1] - pSampleIn[iStartAt + iCount + iChannels + 1]));
						}

						iTemp = iTemp1stChannel + iTemp2ndChannel;
						
						if (iTemp < iGradient)
						{
							iOffset = iCount;		// remember this as the best candidate so far
							iGradient = iTemp;
						}
					}

					//if (g_bVerbose)
					{
						printf ("%d", iStartAt + iOffset);
						//printf ("%2d :%d. ", iChunk+1, iStartAt + iOffset);
						//printf ("iStartAt: %d. ", iStartAt);
						//printf("chunklen = %4.2f %d\n", fChunkLen, iThisChunkLength);
			
						if (iChunk < nChunks-1)
							printf (", ");
						else
							printf ("\n\n");
					}

					//	copy beginning segment to output (ie everything up to the point we are inserting at)
					for (int iStart=0; iStart<=iOffset; iStart += iChannels)
					{
						DWORD dwWritePos = iStartAt+(iChunk * iChannels)+iStart;
						if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwNumBytesOut)))	// bounds checking
							break;
						pSampleOut[dwWritePos] = pSampleIn[iStartAt+iStart];
						if (iChannels == 2)
						{
							dwWritePos = iStartAt+(iChunk * iChannels)+iStart + 1;
							if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwNumBytesOut)))	// bounds checking
								break;
							pSampleOut[dwWritePos] = pSampleIn[iStartAt+iStart + 1];
						}
					}

					//	copy end segment to output (ie after the point we inserted at - repeating the last sample)
					int iEnd;
					for (iEnd=iOffset; iEnd < iThisChunkLength; iEnd += iChannels)
					{
						DWORD dwWritePos = iStartAt+(iChunk * iChannels)+iEnd + iChannels;

						if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwNumBytesOut)))	// bounds checking
							break;
						pSampleOut[dwWritePos] = pSampleIn[iStartAt+iEnd];
						if (iChannels == 2)
						{
							dwWritePos = iStartAt+(iChunk * iChannels)+iEnd + iChannels + 1;
							if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwNumBytesOut)))	// bounds checking
								break;
							pSampleOut[dwWritePos] = pSampleIn[iStartAt+iEnd + 1];
						}
					}

					//printf("%d of %d", iStartAt+iChunk+iEnd+1, dwNumBytesOut/2);
				}
			}
		}

		//	same
		else
		{
			memcpy(pDataOut, pDataIn, dwNumBytesIn);
			//if (g_bVerbose)
				printf ("\n\n");
		}

		#ifdef _DEBUG
		// test start
		bool bLoopWholeFile=false;
		DWORD dwNumSamples;
		SAMPLER_CHUNK* pS = GetLoopInfo();
		if (pS)
		{
			GetNumSamples(&dwNumSamples);
			if (dwNumSamples == pS->SampleLoop.ulEnd-1)
				bLoopWholeFile = true;
		}
		// test end
		#endif

		// free original wavedata
		m_DataChunk.Free(pDataIn);

		// set new wavedata
		m_DataChunk.m_pData = pDataOut;
		m_DataChunk.m_dwDataSize = dwNumBytesOut;

		// correct sample loops
		SAMPLER_CHUNK* pSamplerChunk = GetLoopInfo();
		if (pSamplerChunk)
		{
			DWORD dwNumSamples;
			if (GetNumSamples(&dwNumSamples) != S_OK)
			{
				PrintError("CWaveFile::QuantizeInsertDelete(): Could not get num samples after quantization to correct loop info.\n");
				return E_FAIL;
			}
			F64 fRatio = (F64)dwNumBytesOut / (F64)dwNumBytesIn;
			pSamplerChunk->SampleLoop.ulStart = (ULONG)(pSamplerChunk->SampleLoop.ulStart * fRatio);
			pSamplerChunk->SampleLoop.ulEnd =  (ULONG)(pSamplerChunk->SampleLoop.ulEnd * fRatio);

			#ifdef _DEBUG
			// test
			if (bLoopWholeFile && pSamplerChunk->SampleLoop.ulEnd !=  dwNumSamples-1)
				PrintError("CWaveFile::QuantizeInsertDelete():  sample loop not corrected ok.\n");
			#endif	

			if (pSamplerChunk->SampleLoop.ulEnd > dwNumSamples-1) 
				pSamplerChunk->SampleLoop.ulEnd = dwNumSamples-1;
		}

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: Quantize()
	// Desc: quantize a sample to be multiple of a specified number (uQuantize) of samples.
	//		 used to prepare samples for conversion to formats which require
	//		 samples to be aligned.
	//		 this function can also resample to another samplerate if required at the same
	//		 time as quantizing.
	//		 if uDestSampleRate is not specified then the samplerate will not be changed.
	//		 NOTE: only 16 bit samples supported currently
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::Quantize(eQuantizeMode eMode, U32 uQuantize, U32 uThreshold, U32 uDestSampleRate, U32 uQuality)
	{
		BYTE* pDataOut=0;
		WAVEFORMATEX* waveFormat;

		if (uQuantize <= uThreshold)
		{
			PrintError("Threshold must be less than block size\n");
			return E_FAIL;
		}

		waveFormat = GetFormatEx();
		if (waveFormat==NULL)
		{
			PrintError("Aborting, could not get format\n");
			return E_FAIL;
		}

		if (waveFormat->wBitsPerSample != 16)
		{
			PrintError("Aborting, sample must be 16 bit\n");
			return E_FAIL;
		}
		else if (waveFormat->nChannels < 1 || waveFormat->nChannels > 2)
		{
			PrintError("Aborting, illegal number of channels\n");
			return E_FAIL;
		}

		// Read the wave file data. this needs to be done before we can get the number of samples 
		if (m_DataChunk.m_pData == NULL)
		{
			if (FAILED(ReadSample()))
				return E_FAIL;
		}

		U32 uRate;
		if (uDestSampleRate == 0)
			uRate = waveFormat->nSamplesPerSec;
		else
			uRate = uDestSampleRate;

		// ====================
		// all units in SAMPLES
		// ====================

		DWORD dwNumSamplesIn=0;
		DWORD dwNumSamplesOut=0;
	 
		if (GetNumSamples(&dwNumSamplesIn) == E_FAIL)
		{
			PrintError("Aborting, could not get number of samples of input wav file\n");
			return E_FAIL;
		}

		if (dwNumSamplesIn % uQuantize == 0)
		{
			Audio::Print(0, "Sample is already quantized to %d samples (%d samples). Nothing to do.\n", uQuantize, dwNumSamplesIn);
			return S_OK;
		}
		
		U32 uBytesPerSample = waveFormat->wBitsPerSample / 8;
		U32 uChannels = waveFormat->nChannels;
		U32 uQuantizeBlockSize = uQuantize/* * uChannels*/;
		F32 fSampleRateDifRatio = (F32)uRate / (F32)waveFormat->nSamplesPerSec;
		U32 uDstNumSamplesUnaligned = (U32)(dwNumSamplesIn * fSampleRateDifRatio);

		// Get new data chunk size to the nearest quantize block size. dont allow 0 samples
		dwNumSamplesOut = max((uQuantizeBlockSize * (int)(uDstNumSamplesUnaligned / uQuantizeBlockSize)), uQuantizeBlockSize);

		unsigned int uSamplesDiscarded = uDstNumSamplesUnaligned % uQuantizeBlockSize;

		// if the bytes we have got rid of is less than the threshold then we round down.
		// if there are more bytes than the threshold then we round up to the nearest quantize block
		if (uSamplesDiscarded > uThreshold)
			dwNumSamplesOut += uQuantizeBlockSize;

		if (eMode == eQuantizeInsertDelete)
		{
			QuantizeInsertDelete(dwNumSamplesIn * uBytesPerSample * uChannels, dwNumSamplesOut * uBytesPerSample * uChannels, uQuantize);
		}
		else
		{
			F64 fRatio = (F64)dwNumSamplesOut / (F64)dwNumSamplesIn;
			
			if (uDestSampleRate)
				Audio::Print(0, "'%s': Quantizing to %d samples and resampling to %dhz... Q%d ", GetRawFilename(m_strReadFilename.Chars()).Chars(), uQuantize, uRate, uQuality);
			else
				Audio::Print(0, "'%s': Quantizing to %d samples... Q%d ", GetRawFilename(m_strReadFilename.Chars()).Chars(), uQuantize, uQuality);

			SampleRateConvertF64(fRatio, true, uQuality, dwNumSamplesOut);

			// set the sample rate if we have resampled as part of the quantize
			waveFormat->nSamplesPerSec = uRate;
			waveFormat->nAvgBytesPerSec = uRate * uBytesPerSample;
		}

		DWORD dwNumSamples=0;
		if (GetNumSamples(&dwNumSamples) != S_OK)
		{
			PrintError("Could not get number samples to check alignment\n");
			return E_FAIL;
		}

		if (dwNumSamples % uQuantize)
		{
			int iNumSamplesOff = dwNumSamples % uQuantize;
			PrintError("Sample alignment failed! %d samples off. Num samples after quantization is %d.\n", iNumSamplesOff, dwNumSamples);
			return E_FAIL;
		}

		Audio::Print(TXTCOLOUR_GREEN, "ok\n");

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: BitDepthConvert()
	// Desc: Converts the sample rate of the sample data
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::BitDepthConvert(U32 uBitDepth)
	{
		if (m_DataChunk.IsValid()==false)
		{
			PrintError("CWaveFile::BitDepthRateConvert(): Data is not valid.\n");
			return E_FAIL;
		}

		WAVEFORMATEX* pFormat = GetFormatEx();

		if (pFormat == NULL)
		{
			PrintError("CWaveFile::BitDepthRateConvert(): Could not get wav format.\n");
			return E_FAIL;
		}

		// nothing to do - bit rate is already the requested rate
		if (pFormat->wBitsPerSample == uBitDepth)
		{
			//Print(0, "Bit depth converting '%s.wav' from %d to %d bit... SKIPPED\n", GetRawFilename(m_strReadFilename.c_str()), pFormat->wBitsPerSample, uBitDepth);
			return S_OK;
		}

		Print(0, "'%s': Bit depth converting  from %d to %d bit... ", GetRawFilename(m_strReadFilename.Chars()).Chars(), pFormat->wBitsPerSample, uBitDepth);

		if (uBitDepth != 16)
		{
			PrintError("CWaveFile::BitDepthConvert(): only 24 bit to 16 bit supported at the moment. Asked for unsupported  bit depth %d bits\n", uBitDepth);
			return E_FAIL;
		}

		if (pFormat->wBitsPerSample != 24)
		{
			PrintError("CWaveFile::BitDepthConvert(): only 24 bit to 16 bit supported at the moment. File is %d bits\n", pFormat->wBitsPerSample);
			return E_FAIL;
		}

		U8* pu8Src = (U8*)GetSampleData();

		if (!pu8Src)
		{
			PrintError("CWaveFile::BitDepthConvert(): Sample data is NULL.\n");
			return E_FAIL;
		}

		// TODO: nice generic bit of code here

		// 24 to 16 bit for now
		DWORD dwSampleSize;
		GetDataSize(&dwSampleSize);
		U8* pu8End = pu8Src + dwSampleSize;
		U8* pu8Cur = pu8Src;

		S32 u32DstSize = dwSampleSize / 3 * 2;
		//U16* pu16Dst = new U16[u32DstSize];
		U16* pu16Dst = (U16*)m_DataChunk.Alloc(u32DstSize * 2);
		U16* pu16DstCur = pu16Dst;
		memset(pu16Dst, 0, u32DstSize);

		while (pu8Cur < pu8End)
		{
			// lose the 8 least significant bits
			pu8Cur++;

			*pu16DstCur = (U16)(*pu8Cur);
			pu8Cur++;

			*pu16DstCur |= (U16)((*pu8Cur) << 8);
			pu8Cur++;
			
			pu16DstCur++;
		}
		
		// free original wavedata
		//m_DataChunk.Free(pu8Src);
		
		// set new bit rate
		pFormat->wBitsPerSample = uBitDepth;
		pFormat->nBlockAlign = (uBitDepth / 8) * pFormat->nChannels;
		pFormat->nAvgBytesPerSec = pFormat->nSamplesPerSec * pFormat->nBlockAlign;

		m_DataChunk.FreeDataAndSetNew(pu16Dst, u32DstSize, false);
		//m_DataChunk.SetData(pu16Dst, u32DstSize);
		// set new sample data for wavefile
		//m_DataChunk.m_pData = pu16Dst;
		//m_DataChunk.m_dwDataSize = u32DstSize;

		Print(TXTCOLOUR_GREEN, "ok\n");

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: GetNumSamples()
	// Desc: Get number of samples in the wave data
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::GetNumSamples(DWORD* pdwNumSamples)
	{	
		*pdwNumSamples = 0;

		// took this out because we should legally be able to query the number of samples without reading the data
	/*	if (m_DataChunk.m_pData == NULL)
			return E_FAIL;*/

		WAVEFORMATEX* pFormat = GetFormatEx();

		if (pFormat == NULL)
			return E_FAIL;

		if (pdwNumSamples == NULL)
			return E_FAIL;

		*pdwNumSamples = m_DataChunk.m_dwDataSize / (pFormat->wBitsPerSample / 8) / pFormat->nChannels;

		return S_OK;
	}
	
	//-----------------------------------------------------------------------------
	// Name: GetNumChannels()
	// Desc: Get number of channels in the wave data
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::GetNumChannels(DWORD* pdwNumChannels)
	{	
		*pdwNumChannels = 0;

		/*if (m_DataChunk.m_pData == NULL)
			return E_FAIL;*/

		WAVEFORMATEX* pFormat = GetFormatEx();

		if (pFormat == NULL)
			return E_FAIL;

		if (pdwNumChannels == NULL)
			return E_FAIL;

		*pdwNumChannels = pFormat->nChannels;

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: Write()
	// Desc: Saves an audio file.
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::Write()
	{
		Print(0, "Writing WavFile '%s'...", m_strWriteFilename.Chars());

		// Write the chunks
		m_RiffChunk.m_dwDataSize = m_DataChunk.m_dwDataSize + m_FormatChunk.m_dwDataSize + m_SamplerChunk.m_dwDataSize + 28;

		if (m_RiffChunk.Write( TRUE ) != S_OK)
		{
			PrintError("CWaveFile::Write() : Could not write riff chunk\n");
			return E_FAIL;
		}
		if (m_FormatChunk.Write( TRUE ) != S_OK)
		{
			PrintError("CWaveFile::Write() : Could not write format chunk\n");
			return E_FAIL;
		}
		if (m_DataChunk.Write( TRUE ) != S_OK)
		{
			PrintError("CWaveFile::Write() : Could not write data chunk\n");
			return E_FAIL;
		}
		// sampler chunk is optional
		if (GetLoopInfo())
		{
			if (m_SamplerChunk.Write( TRUE ) != S_OK)
			{
				PrintError("CWaveFile::Write() : Could not write sampler chunk\n");
				return E_FAIL;
			}
		}
		Print(TXTCOLOUR_GREEN, "ok\n");

		return S_OK;
	}

	//-----------------------------------------------------------------------------
	// Name: GetLoop
	// Desc: Is this wave a loop?
	// NOTE: this is crap the way it works currently as it must read the sampler
	//       chunk from the file every time its called.
	//-----------------------------------------------------------------------------
	bool CWaveFile::IsLoop()
	{
		SAMPLER_CHUNK* ws = GetLoopInfo();

		if (ws == NULL)
			return false;

		if (ws->ChunkHeader.ulNumSampleLoops != 1)
			return false;

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: GetLoopRegion
	// Desc: Gets the loop region, in terms of samples.
	//		 If no sampler chunk exists or there are no loops specified then
	//		 pdwStart and pdwLength will be set to 0xffffffff
	//-----------------------------------------------------------------------------
	HRESULT CWaveFile::GetLoopRegion( DWORD *pdwStart, DWORD *pdwLength )
	{
		SAMPLER_CHUNK* ws = GetLoopInfo();

		if (ws == NULL)
		{
			*pdwStart = 0xffffffff;
			*pdwLength = 0xffffffff;
			return S_OK;
		}

		*pdwStart = ws->SampleLoop.ulStart;
		*pdwLength = ws->SampleLoop.ulEnd - ws->SampleLoop.ulStart + 1;

		return S_OK;
	}
	   
	//-----------------------------------------------------------------------------
	// Name: Close()
	// Desc: Closes the object
	//-----------------------------------------------------------------------------
	VOID CWaveFile::Close()
	{
		if( m_hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( m_hFile );
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}
}	// namespace Audio
