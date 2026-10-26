#pragma once

//#include "audio.h"
//#include "file.h"

namespace Audio
{
	class WaveBankReader
	{
		friend class WaveBank;
	public:
		WaveBankReader(WaveBank* pWaveBank);

		Audio::Result Read(bool bHeadersOnly=false);		// read the wavebank file into the WaveBank class
		bool Show();										// print the details of the bank to the debug stream
	private:
		WaveBankReader();
		
		bool ProcessWaveHeaderChunk(Sys::I_File* file, U32 uChunkSize);
		bool ProcessDataChunk(Sys::I_File* file, U32 uChunkSize);
		bool SkipChunk(Sys::I_File* file, U32 uChunkSize);

		void Error(const char* text, ...);

		#ifdef NON_NATIVE_BANK_READING_SUPPORT
		// lightweight endian swap functions. only used on PC tools side
		void SwapBytes(U8& val1, U8& val2) { U8 temp; temp=val1; val1=val2; val2=temp; }
		void ToNativeEndianU32(U32* pU32);
		void ToNativeEndianU16(U16* pU16);
		#endif
	private:
		int m_iEndian;										// 0 for little 1 for big
		#ifdef NON_NATIVE_BANK_READING_SUPPORT
		bool m_bNeedToSwapEndian;
		#endif

		int m_CurWaveHeaderChunk;							// which wave header chunk are we processing?
		int m_CurDataChunk;									// which data chunk we are processing?

		WaveBank* m_pWaveBank;								// the bank the writer is associated with
	};
};

