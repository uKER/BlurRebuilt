#ifndef WAVEFILE_H
#define WAVEFILE_H
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>

#include "riff.h"

namespace Audio
{
	//-----------------------------------------------------------------------------
	// Name: class CWaveFile
	// Desc: Wave file utility class
	//-----------------------------------------------------------------------------
	class CWaveFile
	{
	public:
		
		enum eQuantizeMode
		{
			eQuantizeResample=0,
			eQuantizeInsertDelete,
		};

		//-----------------------------------------------------------------------------
		// Name: struct SAMPLER_CHUNK_HEADER
		// Desc: sampler chunk
		//-----------------------------------------------------------------------------
		struct SAMPLER_CHUNK_HEADER
		{
			//ULONG cbSize;
			ULONG ulManufacturer;		// 	0 - 0xFFFFFFFF
			ULONG ulProduct;			// 	0 - 0xFFFFFFFF
			ULONG ulSamplePeriod;		// 	0 - 0xFFFFFFFF
			ULONG ulMidiUnityNote;		// 	0 - 127
			ULONG ulMidiPitchFraction;	// 	0 - 0xFFFFFFFF
			ULONG ulSmpteFormat;		// 	0, 24, 25, 29, 30
			ULONG ulSmpteOffset;		// 	0 - 0xFFFFFFFF
			ULONG ulNumSampleLoops;		// 	0 - 0xFFFFFFFF
			ULONG ulSamplerData;		// 	0 - 0xFFFFFFFF
		};

		//-----------------------------------------------------------------------------
		// Name: struct SAMPLE_LOOP
		// Desc: containas info of a loop section
		//-----------------------------------------------------------------------------
		struct SAMPLE_LOOP
		{
			ULONG ulCuePointId;
			ULONG ulType; 	
			ULONG ulStart; 	
			ULONG ulEnd;  	
			ULONG ulFraction;
			ULONG ulPlayCount;
		};

		//-----------------------------------------------------------------------------
		// Name: struct SAMPLER_CHUNK_S
		// Desc: sampler chunk
		//-----------------------------------------------------------------------------
		typedef struct SAMPLER_CHUNK_S
		{
			SAMPLER_CHUNK_HEADER	ChunkHeader;
			SAMPLE_LOOP				SampleLoop;
		}SAMPLER_CHUNK;



		//-----------------------------------------------------------------------------
		// Name: struct CUE_CHUNK_HEADER
		// Desc: cue chunk
		//-----------------------------------------------------------------------------
		struct CUE_CHUNK_HEADER
		{
			ULONG ulNumCuePoints;		// 		
		};

		//-----------------------------------------------------------------------------
		// Name: struct CUE_POINT
		// Desc: cue point
		//-----------------------------------------------------------------------------
		struct CUE_POINT
		{
			ULONG ulCueID;				//
			ULONG ulPosition;			// Play position, sequential
			ULONG ulDataChunkID;		// RIFF ID of corresponding data chunk
			ULONG ulChunkStart;			// Byte offset to Data Chunk *
			ULONG ulBlockStart;			// Byte Offset to sample of first channel
			ULONG ulSampleOffset;		// Byte offset to sample byte of first channel
		};

		//-----------------------------------------------------------------------------
		// Name: struct CUE_CHUNK
		// Desc: cue chunk
		//-----------------------------------------------------------------------------
		typedef struct CUE_CHUNK_S
		{
			CUE_CHUNK_HEADER		CueHeader;
			Ut::Vector<CUE_POINT*>	CueList;
		}CUE_CHUNK;



		//-----------------------------------------------------------------------------
		// Name: struct CUE_LABEL_CHUNK_HEADER
		// Desc: cue point info
		//-----------------------------------------------------------------------------
		struct LIST_CHUNK_HEADER
		{
			FOURCC ulType;			// = "adtl" for LABEL
		};

		//-----------------------------------------------------------------------------
		// Name: struct LABEL
		// Desc: cue point info
		//-----------------------------------------------------------------------------
		struct LABEL
		{
			FOURCC ulChunkID;			// = "labl"
			ULONG  ulChunkDataSize;		// = 4 + (LABELLength +1)
			ULONG  ulDataChunkID;		// RIFF ID of corresponding data chunk
			CHAR*  cueLabel;				// Variable size = LABEL + 1 (null terminator) (+ 1 if it needs aligning to 2 byte boundary)
		};

		//-----------------------------------------------------------------------------
		// Name: struct LABEL_CHUNK
		// Desc: LABEL chunk. Currently only 'labl' (LABELs) are supported. Other types
		// of lists could be supported by adding them in here. 
		//-----------------------------------------------------------------------------
		typedef struct LIST_CHUNK_S
		{
			LIST_CHUNK_HEADER		ListHeader;
			Ut::Vector<LABEL*>		LabelList;
		}LIST_CHUNK;

		//-----------------------------------------------------------------------------
		// Name: struct LIST_CHUNK_LIST
		// Desc: a list of LISTS
		//-----------------------------------------------------------------------------
		typedef struct LIST_CHUNK_LIST_S
		{
			Ut::Vector<LIST_CHUNK>		LISTList;
		}LIST_CHUNK_LIST;		


	public:
		HANDLE      m_hFile;            // File handle
	
		RiffChunk  m_RiffChunk;			// RIFF chunk
		RiffChunk  m_FormatChunk;		// Format chunk
		RiffChunk  m_DataChunk;			// Data chunk
	    RiffChunk  m_SamplerChunk;		// Sampler chunk
		RiffChunk  m_CueChunk;			// Cue chunk	
		RiffChunk  m_ListChunk;			// List chunk
	private:
		Util::C_String m_strReadFilename;
		Util::C_String m_strWriteFilename;

	public:
		CWaveFile();
		~CWaveFile();

		// Initialization
		HRESULT OpenForRead( const CHAR* strFileName );
		HRESULT OpenForWrite( const CHAR* strFileName );
		VOID    Close();

		Util::C_String& ReadFileName() { return m_strReadFilename; }
		Util::C_String& WriteFileName() { return m_strWriteFilename; }

		// GetFormat is used for PCM format, GetFormatEx is used for non-PCM formats
		// WAVEFORMATEX includes additional bytes of data not present in PCM
		//WAVEFORMAT* GetFormat() const { return (WAVEFORMAT*)m_FormatChunk.m_pData; }

		// note: if using this function on PCM data dont use/access cbSize as it wont exist.
		// I know this is horrible but so is the shit MS wave format
		WAVEFORMATEX* GetFormatEx() const { return (WAVEFORMATEX*)m_FormatChunk.m_pData; }

		// read File format from disk
		HRESULT ReadFormat();

		// read sample data into internal memory
		HRESULT ReadSample() { return m_DataChunk.ReadData(); }

		// Get the sample data
		VOID* GetSampleData() const { return m_DataChunk.m_pData; }

		// read cue data into internal memory
		HRESULT ReadCues() { return m_CueChunk.ReadData(); }

		// Get the cue data
		VOID* GetCueData() const { return m_CueChunk.m_pData; }

		// read data
		HRESULT Read( VOID* pBuffer, DWORD dwBytesToRead, DWORD* pdwRead=NULL );

		HRESULT Write();

		// is this wave a loop?
		bool IsLoop();

		// read sampler chunk into internal memory
		HRESULT ReadLoopInfo();

		// read cue chunk into internal memory
		HRESULT ReadCueInfo();

		// read list chunk into internal memory
		HRESULT ReadListInfo();

		// copy the cue data in to the wavebank passed in
		HRESULT CopyInCueData(WaveBank::Wave* pBankWave);
		
		SAMPLER_CHUNK* GetLoopInfo() const { return (SAMPLER_CHUNK*)m_SamplerChunk.m_pData; }

		// Loop region
		HRESULT GetLoopRegion( DWORD* pdwStart, DWORD* pdwLength );

		
		// get size of sample data in bytes
		VOID    GetDataSize( DWORD* pdwDuration ) { *pdwDuration = m_DataChunk.GetDataSize(); }

		// get num samples in wave
		HRESULT GetNumSamples(DWORD* pdwNumSamples);
		
		// get num channels in wave
		HRESULT GetNumChannels(DWORD* pdwNumSamples);

		// debug
		void LogChunkHeaders(const char* pchFilename);
		void CWaveFile::LogFormatChunk();
		void CWaveFile::LogSamplerChunk();

		//---------------------------------------------------------------------
		// sample processing 
		//---------------------------------------------------------------------
		// sample rate converter. specify target sample rate in hz. 
		// can specify quality. defaults to best quality
		HRESULT SampleRateConvert(U32 uSampleRate, int iQuality = 0);

		// bit depth converter
		HRESULT BitDepthConvert(U32 uBitDepth);

		// quantizer. can choose insert/delete samples method or resample method.
		// with resample method can quantize and resample to new sample rate in one operation.
		HRESULT Quantize(eQuantizeMode eMode, U32 uQuantize, U32 uThreshold, U32 uDestSampleRate=0, U32 uQuality=0);

private:
		// uNumSamplesReq is an optional parameter. the src can be asked for a specific number of samples. [note: you may not get exactly the number you asked for]
		// bAlterPitch resamples but does not change the existing sample rate, thus altering the pitch. can be used to quantize.
		HRESULT SampleRateConvertF64(F64 fRatio, bool bAlterPitch = false, int iQuality = 0, U32 uNumSamplesReq = 0);
		HRESULT QuantizeInsertDelete(DWORD dwNumBytesIn, DWORD dwNumBytesOut, U32 uQuantize);
	};
}	// namespace Audio


#endif
