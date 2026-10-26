#pragma once

#include "core/core.h"
#include "core/containers.h"

#include "types.h"
#include "SampleTypes.h"


namespace Audio
{	
	static const char* g_pchWbFileExt = ".baf";

	static const char* g_pchWbTag_Bank = "BANK";
	static const char* g_pchWbTag_Wave = "WAVE";
	static const char* g_pchWbTag_Data = "DATA";
	
	static const int g_WbWaveNameSize=32;
	static const int g_WbNameSize=32;

	static const U32 g_WbVersion=4;

	class WaveBank
	{
		friend class WaveBankWriter;
		friend class WaveBankReader;
	public:

		//-----------------------------------------------------------------------------
		// Memory alloc stuff.
		//-----------------------------------------------------------------------------
		enum AllocType
		{
			eVirtual=0,
			ePhysical,
			eNumAllocTypes
		};
		void* operator	new (size_t size) { return Alloc(size); }
		void  operator  delete (void* ptr) { return Free(ptr); }

		typedef void*	(*AllocFuncPtr)(size_t size, U32 uAlignment, AllocType eType);
		typedef void	(*FreeFuncPtr)(void* ptr, AllocType eType);
	
		// functions for setting your own mem functions
		static void		SetMemAllocFunctions(AllocFuncPtr fptrAlloc, FreeFuncPtr fptrFree) 
						{ 
							AllocPtr = fptrAlloc; 
							FreePtr = fptrFree; 
						}
	private:
		// the actual calls which is used internally to alloc and free memory
		static void*	Alloc(size_t size, U32 uAlignment=1, AllocType eType=eVirtual) { return AllocPtr(size, uAlignment, eType); }
		static void		Free(void* ptr, AllocType eType=eVirtual) { FreePtr(ptr, eType); }
		
		// function pointers for replacement alloc and free
		static AllocFuncPtr	AllocPtr;
		static FreeFuncPtr	FreePtr;	

		// default alloc and free replacements. can be overridden if needed
		static void* DefaultAlloc(size_t size, U32 uAlignment, AllocType eType);
		static void DefaultFree(void* ptr, AllocType eType);

	public:
		enum eEndianType
		{
			eLittleEndian=0,
			eBigEndian
		};
		 
		// pitch shift
		
		#define GET_PITCHSHIFT_UP(flags)		(flags&PITCHSHIFT_UP_MASK)
		#define GET_PITCHSHIFT_DOWN(flags)		(flags&PITCHSHIFT_DOWN_MASK)
		#define GET_PITCHSHIFT_QUALITY(flags)	((flags>>PITCHSHIFT_QUALITY_SHIFT)&PITCHSHIFT_QUALITY_MASK)
		
		#define PITCHSHIFT_QUALITY_SHIFT	4
		#define PITCHSHIFT_DOWN_SHIFT		3
		
		#define PITCHSHIFT_UP_MASK			0x07
		#define PITCHSHIFT_DOWN_MASK		0x08
		#define PITCHSHIFT_QUALITY_MASK		0x70
		#define FLAG_HAS_XMA_SEEK_TABLE		0x80

		#define PITCHSHIFT_UP_OFF		0x0
		#define PITCHSHIFT_UP_1_OCTAVE	0x1
		#define PITCHSHIFT_UP_2_OCTAVE	0x2
		
		struct BankHeader
		{
			U32 uTag;			// header tag 'BANK'
			U32	uSize;			// header size (includin the tag and size variables)
			U32	uVersion;		// version number of this bank
			U32	uNumWaves;		// number of waves in this bank
			U8	chEndian;		// endian-type. 0 for little 1 for big
			U8	chStreaming;	// is this bank streaming?
			char pchName[g_WbNameSize];		// the bank name
		};

		// the initial wave header. each wave will have one of these and
		// will be followed by the additional headers below
		
		// when adding extra fields:
		// deal with endian swapping in WaveBankWriter::WriteBankFile
		// deal with endian swapping in WaveBankReader::ProcessWaveHeaderChunk
		// modify WavebankReader::Show
		struct WaveBaseHeader
		{
			U32 uTag;
			U32 uSize;						// size of this header and the following extra header (including the tag and size)
			U32 uFormat;					// this must come directly after the tag and size	
			char pchName[g_WbWaveNameSize];
			U32 uDataStart;					// offset in the file where the data starts
			U32 uDataLength;				// the length in bytes of the data
			F32 fDefaultLevel;
			U32 uFlags;
		};

		// these are additional headers to the base header. one of these should accompany the base waveheader
		struct PCMWave
		{
			U32 uSampleRate;
			U32 uBitsPerSample;
			U32 uNumChannels;
			U32 uStride;
			U8	uNumLoops;								// num times to loop. 0 = no loop. 255 = infinite loop	
		};

		struct XMAWave
		{
			U32 uSampleRate;
			U32 uBitsPerSample;
			U32 uNumChannels;
			U32 uStride;
			U32 uLoopStartOffsetBit;					// loop start offset, in bits
			U32 uLoopEndOffsetBit;						// loop end offset, in bits
			U8	uNumLoops;								// num times to loop. 0 = no loop. 255 = infinite loop
			U8	uSubframeData;							// subframe data. format is eeee ssss. eeee = subframe end point. ssss = subframes to skip
			U32 uNumFrames;								// num frames in the source wave to get num Samples, multiply by 512
		};

		struct VagWave
		{
			U32 uSampleRate;
			U32 uNumChannels;
		};

		struct ADPCMWave
		{
			U32 uSize;
			U32 uSampleRate;
			U32	uNrSamples;
			U8	uLoop;
			U8  uNrInterleavedStreams;
			U8	uBitsPerSample;
			U8	uNumChannels;
		};

		// notes: 
		// uStartSample and uEndSample are needed for seamless looping.
		// uStartSample : which sample does the actual sound start on? when an mp3 is encoded the encoder adds 
		//		padding samples to the start of the mp3. the start sample tells us how many samples we can 
		//		discard when decoding. 
		// uEndSample: which sample does the sound actually end on? when an mp3 is encoded the encoder adds
		//		padding frames to the end of the mp3. the end sample is the last sample that was in the 
		//		source sample. when decoding we can discard any samples after that one.
		struct MP3Wave
		{
			U32 uSampleRate;		
			U32 uBitsPerSample;		// always 16? doesnt seem to be available in the mp3 format
			U32 uNumChannels;		
			U16	uLoop;				
			U32 uStartSample;		
			U32 uEndSample;	
		};

		struct ChunkHeader
		{
			U32 uTag;
			U32 uSize;									// size of the following chunk data, including this header. 
		};												// so actual chunk size is uSize-sizeof(ChunkHeader)
		
    public:
		//-----------------------------------------------------------------------------
		// Wave
		//-----------------------------------------------------------------------------
		class Wave
		{
			public:
				void* operator	new (size_t size) { return WaveBank::Alloc(size); }
				void  operator  delete (void* ptr) { return WaveBank::Free(ptr); }

								Wave() { Init(); }
								void Init();

				XMAWave*		XmaWave() { return (XMAWave*)m_pExtraHeader; }
				PCMWave*		PcmWave() { return (PCMWave*)m_pExtraHeader; }
				ADPCMWave*		AdpcmWave() { return (ADPCMWave*)m_pExtraHeader; }
				MP3Wave*		Mp3Wave() { return (MP3Wave*)m_pExtraHeader; }

				U32				BitsPerSample();						// get the bits per sample. if this does not apply then returns 0
				U32				NumChannels();							// get number of channels. if this does not apply then returns 0
				U32				Loop();									// is it a loop?
				U32				SampleRate();							

				U32				DataStartOffset() { return m_baseHeader.uDataStart; }		// offset in the file where the data for this wave starts
				U32				DataLength() { return m_baseHeader.uDataLength; }			// size of the data
				F32				LengthInSecs(); 

			public:
				// the headers that describe the waves
				WaveBaseHeader	m_baseHeader;
				void*			m_pExtraHeader;							// this extra header needs to be casted into a PCMWave, XMAWave etc

				// the data (note: if its a streaming bank this will be NULL)
				void*			m_pvWaveData;
		};
		
	private:
		void			Init();
		void			InitBankHeader();

	public:
						WaveBank() { Init(); }
						WaveBank(const char* pchFname);
						~WaveBank();

		U32				NumWaves() { return m_vpWaves.Size(); }
		Wave*			GetWave(int index) { return m_vpWaves[index]; }

		U32				MemoryFootprint() { return m_MemoryFootprint; }

		char*			Filename() { return m_pchFilename; }
		void			SetFilename(const char* pchFname);

		bool			IsStreamingBank() { return m_BankHeader.chStreaming==1; }

		S32				BankFormat();
		bool			ContainsFormat(U32 uFormat);
	
	public:
		BankHeader		m_BankHeader;
		U32				m_MemoryFootprint;			// size in bytes of this wavebank's data and headers

	private:
		char*			m_pchFilename;



		// the waves
		Ut::Vector<Wave*> m_vpWaves;
	};
};



