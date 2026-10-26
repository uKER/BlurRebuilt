#include "AudioPch.h"

#include "..\..\include\Audio.h"
#include "WaveBank.h"
#include "WaveBankReader.h"
#include "Util.h"
#include "../SystemHeaders.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

// TODO: get rid of small disk reads and small mem allocations.
// each wave header and data chunk is alloc'd and loaded seperately. 
// solutions:
//		-read entire bank into memory in 1 go and parse memory?
//		-rearrange bank format so all headers are at the start and data follows the header. 
//		 read all headers in 1 read/alloc and read data in 1 read/alloc. will need to 2k align xma data on disk if reading 
//		 straight into memory.

namespace Audio
{
	//-----------------------------------------------------------------------------
	// Name: WaveBankReader
	//-----------------------------------------------------------------------------
	WaveBankReader::WaveBankReader(WaveBank* pWaveBank) 
	{ 
		// test endian 
		U16 x = 1;
		if (*(char*)&x) 
			m_iEndian = 0; // little endian
		else 
			m_iEndian = 1; // big endian

		m_pWaveBank=pWaveBank; 
	}

	#ifdef NON_NATIVE_BANK_READING_SUPPORT
	//-----------------------------------------------------------------------------
	// Name: ToNativeEndianU32
	// Desc: Endian swap a four byte U32
	//-----------------------------------------------------------------------------
	void WaveBankReader::ToNativeEndianU32(U32* pU32)
	{
		if (!m_bNeedToSwapEndian)
			return;
		U8* auBytes = (U8*)pU32;
		SwapBytes(auBytes[0], auBytes[3]);
		SwapBytes(auBytes[1], auBytes[2]);
	}

	//-----------------------------------------------------------------------------
	// Name: ToNativeEndianU16
	// Desc: Endian swap a 2 byte U16
	//-----------------------------------------------------------------------------
	void WaveBankReader::ToNativeEndianU16(U16* pU16)
	{
		if (!m_bNeedToSwapEndian)
			return;
		U8* auBytes = (U8*)pU16;
		SwapBytes(auBytes[0], auBytes[1]);
	}
	#endif	// #ifdef NON_NATIVE_BANK_READING_SUPPORT

	//-----------------------------------------------------------------------------
	// Name: Error
	// Desc: 
	//-----------------------------------------------------------------------------
	void WaveBankReader::Error(const char* format, ...)
	{
		char buffer[ 1024 ];
		va_list arglist;
		va_start( arglist, format );
		vsprintf_s( buffer, 1024, format, arglist );
		va_end( arglist );
		AUDIO_SYS_ERROR("WaveBankReader: Failed to load bank '%s'. \nReason: %s\n", m_pWaveBank ? m_pWaveBank->Filename() : "Unknown", buffer);
	}

	//-----------------------------------------------------------------------------
	// Name: Read
	// Desc: Read a wavebank and all its data.
	//		 [Unless its a streaming bank and then only the wave headers will be read
	//		 and not the data.]
	//		 Memory is allocated when needed and data read into that memory.
	//-----------------------------------------------------------------------------
	Audio::Result WaveBankReader::Read(bool bHeadersOnly)
	{
		Sys::I_File* file;

		// jkc- ifdef'ed out on ps3, was hanging in gamefile.cpp/ overflow() / readpausemutex.wait()

		if (m_pWaveBank==NULL)
		{
			Error("WaveBank object is NULL.");
			return Audio::Failed;
		}
	
		WaveBank& bank=*m_pWaveBank;

		if (bank.m_pchFilename==NULL)
		{
			Error("WaveBank name is NULL.");
			return Audio::Failed;
		}

		AUDIO_SYS_DEBUG("Reading wavebank '%s'...", bank.m_pchFilename);

		file = Sys::GetDefaultFileSystem()->Open(bank.m_pchFilename);
		if (!file)
		{
			// this needs to stay as a warning, as it needs to fail gracefully if looking in multiple paths
			Error("Cannot open file.");
			return Audio::WaveBankNotFound;
		}

		Sys::C_StreamSize uFileSize=file->Length();

		// read header
		if (!file->Read(&bank.m_BankHeader, sizeof(WaveBank::BankHeader)))
		{
			delete file;
			Error("Could not read bank header");
			return Audio::Failed;
		}
		
		if (bank.IsStreamingBank())
			bHeadersOnly = true;

		#ifndef NON_NATIVE_BANK_READING_SUPPORT
		// is this bank the right endian type?
		if (bank.m_BankHeader.chEndian != m_iEndian)
		{
			delete file;
			Error("Endian of bank does not match this platform's endian type. Bank=%d. This platform=%d. [0=little, 1=big]", bank.m_BankHeader.chEndian, m_iEndian);
			return Audio::Failed;
		}
		#else
		if (bank.m_BankHeader.chEndian != m_iEndian)
			m_bNeedToSwapEndian = true;
		else
			m_bNeedToSwapEndian = false;

		ToNativeEndianU32(&bank.m_BankHeader.uSize);
		ToNativeEndianU32(&bank.m_BankHeader.uVersion);
		ToNativeEndianU32(&bank.m_BankHeader.uNumWaves);
		#endif

		// make sure header is ok and check the version
		if (bank.m_BankHeader.uTag != *((U32*)g_pchWbTag_Bank))
		{
			delete file;
			Error("Invalid tag in bank header");
			return Audio::WaveBankCorrupt;
		}

		if (bank.m_BankHeader.uNumWaves == 0)
		{
			delete file;
			Error("No waves in bank.");
			return Audio::Failed;
		}
		
		if (bank.m_BankHeader.uVersion != g_WbVersion)
		{
			delete file;
			if (bank.m_BankHeader.uVersion > g_WbVersion)
				Error("Bank format is newer than the wavebank reader supports.\nYou will need a new exe with the version %d wavebank reader to read it\nBank version is %d, wavebank reader version is %d", bank.m_BankHeader.uVersion, bank.m_BankHeader.uVersion, g_WbVersion);
			else
				Error("Bank format is an old unsupported version.\nPlease re-export the bank.\nThe file bank version is %d. Code version is %d", bank.m_BankHeader.uVersion, g_WbVersion);
			return Audio::WaveBankIncompatibleVersion;
		}

		m_CurDataChunk = 0;
		m_CurWaveHeaderChunk = 0;

		while (file->GetPosition() < uFileSize)
		{
			WaveBank::ChunkHeader header;

			// get chunk tag and size
			if (file->Read(&header, sizeof(WaveBank::ChunkHeader)) == 0)
			{
				delete file;
				return Audio::Failed;
			}

			#ifdef NON_NATIVE_BANK_READING_SUPPORT	
			ToNativeEndianU32(&header.uSize);
			#endif
	
			if (header.uTag == *((U32*)g_pchWbTag_Wave))
			{
				if (ProcessWaveHeaderChunk(file, header.uSize)==false)
				{
					delete file;
					return Audio::WaveBankCorrupt;
				}
				// If streaming bank, and we've read all the waves, then align to the data 2k-8
				if (bHeadersOnly && (U32)m_CurWaveHeaderChunk == bank.m_BankHeader.uNumWaves)
					break;
#if 0
				{
					// Streaming Banks' data is 2k Aligned
					U32 filePos = file->GetPosition();
					if (!(filePos & 2047)) filePos += 2048;
					filePos += 2047;
					filePos &= ~2047;
					filePos -= 2048; // Back out 2k
					file->SetPosition( filePos );
					U8 buffer[2048];
					file->Read(&buffer,2048-sizeof(WaveBank::ChunkHeader));
					//filePos -= sizeof(WaveBank::ChunkHeader); // Then take 8 of for Chunk Header
					//file->SetPosition( filePos );
				}
#endif
			}
			else
			if (header.uTag == *((U32*)g_pchWbTag_Data))
			{
				if (ProcessDataChunk(file, header.uSize)==false)
				{
					delete file;
					return Audio::WaveBankCorrupt;
				}
			}
			else
			{
				Error("Unknown chunk tag '%c%c%c%c'", *((char*)&header.uTag), *(((char*)&header.uTag)+1), *(((char*)&header.uTag)+2), *(((char*)&header.uTag)+3));
				if (SkipChunk(file, header.uSize)==false)
				{
					delete file;
					return Audio::WaveBankCorrupt;
				}
			}
		}

		bool bBankIsBroken=false;

		// check if bank was corrupt/incomplete
		if (bank.m_BankHeader.uNumWaves != bank.m_vpWaves.Size())
		{
			Error("Num waves in header is %d but %d wave-header(s) exist in bank file.", bank.m_BankHeader.uNumWaves, bank.m_vpWaves.Size());
			bBankIsBroken=true;
		}

		// check all the data was read in correctly
		if (!bHeadersOnly && bank.m_BankHeader.uNumWaves != (U32)m_CurDataChunk)
		{
			Error("Num waves in header is %d but %d wave data-chunk(s) exist in bank file.", bank.m_BankHeader.uNumWaves, m_CurDataChunk);
			bBankIsBroken=true;
		}
		
		delete file;

		if (bBankIsBroken)
		{
			delete file;
			// do an error here when error routines are available
			Error("BANK IS INCOMPLETE/CORRUPT !");
			return WaveBankCorrupt;
		}

		return Audio::Success;
	}

	//-----------------------------------------------------------------------------
	// Name: ProcessWaveHeaderChunk
	// Desc: Read a wave header chunk
	//-----------------------------------------------------------------------------
	bool WaveBankReader::ProcessWaveHeaderChunk(Sys::I_File* file, U32 uChunkSize)
	{
		WaveBank& bank = *m_pWaveBank;
		WaveBank::Wave* wave = new WaveBank::Wave;
		bool bOk;

		if (wave==NULL)
		{
			Error("Could not allocate memory for WaveBank::Wave");
			return false;
		}

		//AUDIO_SYS_DEBUG("Got wave chunk %d", bank.m_vpWaves.size());

		wave->m_baseHeader.uSize = (U32)uChunkSize;

		// read in the rest of the WaveBaseHeader
		bOk = file->Read(&wave->m_baseHeader.uFormat, sizeof(WaveBank::WaveBaseHeader)-sizeof(U32)*2);

		if (!bOk)
			return false;

		#ifdef NON_NATIVE_BANK_READING_SUPPORT
		ToNativeEndianU32(&wave->m_baseHeader.uFormat);
		ToNativeEndianU32(&wave->m_baseHeader.uDataLength);
		ToNativeEndianU32(&wave->m_baseHeader.uDataStart);
		ToNativeEndianU32((U32*)&wave->m_baseHeader.fDefaultLevel);
		ToNativeEndianU32((U32*)&wave->m_baseHeader.uFlags);
		#endif

		U32 uExtraHeaderSize;
		// allocate the extra header
		switch (wave->m_baseHeader.uFormat)
		{
			case Core::uPCM16BitSignedInt: 
			case Core::uPCM32BitFloat:
				uExtraHeaderSize = sizeof(WaveBank::PCMWave);
				wave->m_pExtraHeader = WaveBank::Alloc(uExtraHeaderSize);
				break;
			case Core::uXMA16Bit: 
			case Core::uXMA216Bit: 
				// Using the same calculation here as the check below to account for the seek
				// table that is tagged onto the end of the extra header.
				uExtraHeaderSize = wave->m_baseHeader.uSize-sizeof(WaveBank::WaveBaseHeader);
				wave->m_pExtraHeader = WaveBank::Alloc(uExtraHeaderSize);
				if (uExtraHeaderSize != sizeof(WaveBank::XMAWave))
					wave->m_baseHeader.uFlags |= FLAG_HAS_XMA_SEEK_TABLE;
				break;
			case Core::uADPCM4Bit: 
				uExtraHeaderSize = sizeof(WaveBank::ADPCMWave);
				wave->m_pExtraHeader = WaveBank::Alloc(uExtraHeaderSize);
				break;
			case Core::uMP3: 
				uExtraHeaderSize = sizeof(WaveBank::MP3Wave);
				wave->m_pExtraHeader = WaveBank::Alloc(uExtraHeaderSize);
				break;
			default:
				Error("Wave '%s' has unknown format - %d.", wave->m_baseHeader.pchName, wave->m_baseHeader.uFormat);
				return false;
		}

		// check we are about to read in the correct amount of bytes for the extra header.
		// we need to make sure the amount of bytes we allocated for the extra header matches
		// the amount of bytes the file thinks the extra header is. the file works out the size
		// based on TotalHeaderSize - BaseHeaderSize = ExtraHeaderSize, but if the baseheader or
		// extra header struct has changed since we built the bank then this assumption will be wrong.
		U32 uExtraHeaderSizeFromFile = wave->m_baseHeader.uSize-sizeof(WaveBank::WaveBaseHeader);
		if (uExtraHeaderSize != uExtraHeaderSizeFromFile)
		{
			Error("Header size mismatch! Code thinks extra header size should be %d bytes but file thinks it should be %d bytes. Try re-exporting the bank.", uExtraHeaderSize, uExtraHeaderSizeFromFile);
			return false;
		}

		bank.m_MemoryFootprint += uExtraHeaderSize;

		bOk = file->Read(wave->m_pExtraHeader, uExtraHeaderSize);
		
		if (!bOk)
			return false;
		
		#ifdef NON_NATIVE_BANK_READING_SUPPORT
		// endian swap the extra header
		switch (wave->m_baseHeader.uFormat)
		{
			case Core::uPCM16BitSignedInt: 
			case Core::uPCM32BitFloat: 
			{
				WaveBank::PCMWave* hdr=(WaveBank::PCMWave*)wave->m_pExtraHeader;
				ToNativeEndianU32(&hdr->uSampleRate);
				ToNativeEndianU32(&hdr->uNumChannels);
				ToNativeEndianU32(&hdr->uBitsPerSample);
				ToNativeEndianU32(&hdr->uStride);
				//ToNativeEndianU16(&hdr->uLoop);
				}
				break;
			case Core::uXMA16Bit: 
			case Core::uXMA216Bit: 
			{
				WaveBank::XMAWave* hdr=(WaveBank::XMAWave*)wave->m_pExtraHeader;
				ToNativeEndianU32(&hdr->uSampleRate);
				ToNativeEndianU32(&hdr->uBitsPerSample);
				ToNativeEndianU32(&hdr->uNumChannels);
				ToNativeEndianU32(&hdr->uStride);
				// no need to swap uNumLoops as its a U8
				ToNativeEndianU32(&hdr->uLoopStartOffsetBit);
				ToNativeEndianU32(&hdr->uLoopEndOffsetBit);
				// no need to swap uSubframeData as its a U8
				ToNativeEndianU32(&hdr->uNumFrames);
				}
				break;
			case Core::uADPCM4Bit: 
			{
				WaveBank::ADPCMWave* hdr=(WaveBank::ADPCMWave*)wave->m_pExtraHeader;
				ToNativeEndianU32(&hdr->uSampleRate);
				ToNativeEndianU32(&hdr->uBitsPerSample);
				ToNativeEndianU32(&hdr->uNumChannels);
				ToNativeEndianU16(&hdr->uLoop);
				}
				break;
			case Core::uMP3: 
			{
				WaveBank::MP3Wave* hdr=(WaveBank::MP3Wave*)wave->m_pExtraHeader;
				ToNativeEndianU32(&hdr->uSampleRate);
				ToNativeEndianU32(&hdr->uBitsPerSample);
				ToNativeEndianU32(&hdr->uNumChannels);
				ToNativeEndianU32(&hdr->uStartSample);
				ToNativeEndianU32(&hdr->uEndSample);
				ToNativeEndianU16(&hdr->uLoop);
				}
				break;
			default:
				Error("Format not dealt with");
				break;
		}
		#endif

        bank.m_vpWaves.PushBack(wave);
		m_CurWaveHeaderChunk++;
		
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: ProcessDataChunk
	// Desc: Read a data chunk. Allocates memory needed within this function.
	//		 Note: XMA data allocates 2k aligned physical memory.
	//		 Note: We have already read the data chunk header before this is called.
	//-----------------------------------------------------------------------------
	bool WaveBankReader::ProcessDataChunk(Sys::I_File* file, U32 uChunkSize)
	{
		WaveBank& bank = *m_pWaveBank;

		if (bank.IsStreamingBank())
		{
#if 0
			// Seek & pull loop positions for ADPCM file
			WaveBank::Wave* wave = bank.m_vpWaves[m_CurDataChunk];
			if (wave->m_baseHeader.uFormat == Core::uADPCM4Bit)
			{
				U32 current_file_position = file->GetPosition();
				file->SetPosition( wave->m_baseHeader.uDataStart );
				U8 loopEnd[8] = { 0 };
				file->Read( &loopEnd, sizeof( loopEnd ) );
				if (loopEnd[0] == 0x08)
				{
					wave->m_uPCMLoopPosEnd = (((U8)loopEnd[7] << 24 | (U8)loopEnd[6] << 16 | (U8)loopEnd[5] << 8 | (U8)loopEnd[4]));
				}
				U32 uDataSize = uChunkSize - sizeof(WaveBank::ChunkHeader);
				U32 uNewPos = current_file_position + uDataSize ;
				if (!(uNewPos & 2047)) uNewPos += 2048;
				uNewPos += 2047;
				uNewPos &= ~2047;
				uNewPos -= 2048; // Back out 2k
				file->SetPosition( uNewPos );
				U8 buffer[2048];
				file->Read(&buffer,2048-sizeof(WaveBank::ChunkHeader));
				//uNewPos -= sizeof(WaveBank::ChunkHeader); // chunk header
				//file->SetPosition( uNewPos );
				m_CurDataChunk++;
				return true;
			}
			else
			{
				U32 current_file_position = file->GetPosition();
				U32 uDataSize = uChunkSize - sizeof(WaveBank::ChunkHeader);
				U32 uNewPos = current_file_position + uDataSize ;
				// Compensate for Bug In WaveBankUtil
				if (!(uNewPos & 2047)) uNewPos += 2048;
				uNewPos += 2047;
				uNewPos &= ~2047;
				uNewPos -= 2048; // Back out 2k
				file->SetPosition( uNewPos );
				U8 buffer[2048];
				file->Read(&buffer,2048-sizeof(WaveBank::ChunkHeader));
//				uNewPos -= sizeof(WaveBank::ChunkHeader); // chunk header
//				file->SetPosition( uNewPos );
				m_CurDataChunk++;
				return true;
			}
#endif
			Error("Trying to read a data chunk on a streaming bank");
			return false;
		}

		if (m_CurDataChunk>(int)bank.m_vpWaves.Size())
		{
			Error("Read a data chunk before reading a Wave chunk.");
			return false;
		}

		WaveBank::Wave* wave = bank.m_vpWaves[m_CurDataChunk];

		U32 uDataSize = uChunkSize - sizeof(WaveBank::ChunkHeader);
  
		{
			wave->m_pvWaveData = WaveBank::Alloc(uDataSize, 128, WaveBank::eVirtual);
			bank.m_MemoryFootprint += Btl::AlignUp(uDataSize, 128);;
		}
		
		if (wave->m_pvWaveData==NULL)
		{
			Error("Could not allocate %k (%d bytes) for wave '%s'\n", uDataSize/1024, uDataSize, wave->m_baseHeader.pchName);
			return false;
		}

		if (file->Read(wave->m_pvWaveData, uDataSize)==false)
		{
			Error("Could not read in wave data for wave '%s'\n", wave->m_baseHeader.pchName);
			return false;
		}
		
		#ifdef NON_NATIVE_BANK_READING_SUPPORT
		if (wave->BitsPerSample()==16)
		{
			for (U32 i=0; i<uDataSize/2; i++)
				ToNativeEndianU16(((U16*)wave->m_pvWaveData)+i);
		}
		else
		if (wave->BitsPerSample()==32)
		{
			for (U32 i=0; i<uDataSize/4; i++)
				ToNativeEndianU32(((U32*)wave->m_pvWaveData)+i);
		}
		else
			; // TODO. deal with non constant bits per sample or bitrates other than 16 or 32
		#endif

		m_CurDataChunk++;

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: SkipChunk
	// Desc: Skip an unknown chunk
	//-----------------------------------------------------------------------------
	bool WaveBankReader::SkipChunk(Sys::I_File* file, U32 uChunkSize)
	{
		AUDIO_SYS_DEBUG("Skipping unknown chunk");
		
		if (uChunkSize == 0)
		{
			Error("Trying to skip chunk of size 0");
			return false;
		}

		// we've already read the tag and the size, so subtract them from the size
		uChunkSize-=sizeof(U32)*2;

		// allocate some mem to read the chunk into. i know this is not the best way to do it
		char* pchData = (char*)WaveBank::Alloc(uChunkSize);
		if (file->Read(pchData, uChunkSize)==false)
		{
			WaveBank::Free(pchData);
			Error("Could not skip chunk");
			return false;
		}

		WaveBank::Free(pchData);
		
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: SkipChunk
	// Desc: show the info for the bank we just read in
	//		 PLEASE NOTE: this uses printfs so the BankInfo command line tool can
	//		 print the bank information to the command line.
	//-----------------------------------------------------------------------------
	bool WaveBankReader::Show()
	{
		#ifdef AUDIO_TOOLS
		if (m_pWaveBank==NULL)
		{
			printf("WaveBankReader::Show() : no bank loaded");
			return false;
		}

		WaveBank& bank=*m_pWaveBank;

		// please dont change these printfs. see above note 
		printf("\nBank name = '%s'\n", bank.m_BankHeader.pchName);
		printf("Version = %d\n", bank.m_BankHeader.uVersion);
		printf("Endian is %s\n", bank.m_BankHeader.chEndian?"Big":"Little");
		printf("%s\n", bank.IsStreamingBank()?"Streaming":"Non-Streaming");
		printf("Num waves = %d\n", bank.m_BankHeader.uNumWaves);
		printf("Waves:\n\n");

		for (U32 i=0; i<bank.NumWaves(); i++)
		{
			WaveBank::Wave* wave = bank.GetWave(i);
			
			if (!wave)
			{
				printf("\n! WAVE %d DOES NOT EXIST !\n", i); 
				continue;
			}

			printf("%3d : ", i+1); 
			printf("'%s', ", wave->m_baseHeader.pchName);
			printf("%d bytes, ", wave->m_baseHeader.uDataLength);
			printf("pitch flags 0x%x, ", wave->m_baseHeader.uFlags);

			if (wave->m_pExtraHeader==NULL)
			{
				printf("!No extra header!\n");
				continue;
			}
			switch (wave->m_baseHeader.uFormat)
			{
				case Core::uPCM16BitSignedInt: 
				case Core::uPCM32BitFloat: 
				{
					WaveBank::PCMWave* hdr=(WaveBank::PCMWave*)wave->m_pExtraHeader;
					if (wave->m_baseHeader.uFormat==Core::uPCM16BitSignedInt)
						printf("Pcm S16, ");
					else
					if (wave->m_baseHeader.uFormat==Core::uPCM32BitFloat)
						printf("Pcm F32, ");
					printf("%d Hz, ", hdr->uSampleRate);
					if (hdr->uNumChannels == 1)
						printf("Mono, ");
					else
					if (hdr->uNumChannels == 2)
						printf("Stereo, ");
					else
						printf("%d Channels", hdr->uNumChannels);
					
					if (hdr->uNumLoops==255)
						printf("Infinite loop");
					else
					if (!hdr->uNumLoops)
						printf("One shot");
					else
						printf("Loop count = %d", hdr->uNumLoops);
					printf("\n");
					
					}
					break;
				case Core::uXMA16Bit: 
				case Core::uXMA216Bit: 
				{
					WaveBank::XMAWave* hdr=(WaveBank::XMAWave*)wave->m_pExtraHeader;
					printf("%s, ", wave->m_baseHeader.uFormat==Core::uXMA16Bit ? "Xma" : "Xma2"); 
					printf("%d Hz, ",hdr->uSampleRate);
					if (hdr->uNumChannels == 1)
						printf("Mono, ");
					else
					
					if (hdr->uNumChannels == 2)
						printf("Stereo, ");
					else
						printf("%d Channels, ", hdr->uNumChannels);

					if (hdr->uNumLoops==255)
						printf("Infinite loop");
					else
					if (!hdr->uNumLoops)
						printf("One shot");
					else
						printf("Loop count = %d", hdr->uNumLoops);
					//printf(", Num samples is %d", hdr->uNumSamples);
					printf("\n");
					}
					break;
				case Core::uADPCM4Bit: 
				{
					WaveBank::ADPCMWave* hdr=(WaveBank::ADPCMWave*)wave->m_pExtraHeader;
					printf("Adpcm, "); 
					printf("%d Hz, ", hdr->uSampleRate);
					if (hdr->uNumChannels == 1)
						printf("Mono, ");
					else
					if (hdr->uNumChannels == 2)
						printf("Stereo, ");
					else
						printf("%d Channels, ", hdr->uNumChannels);
					if (hdr->uLoop)
						printf("Infinite loop");
					else
						printf("One shot");
					printf("\n");
					}
					break;
				case Core::uMP3: 
				{
					WaveBank::MP3Wave* hdr=(WaveBank::MP3Wave*)wave->m_pExtraHeader;
					printf("Mp3, "); 
					printf("%d Hz, ", hdr->uSampleRate);
					if (hdr->uNumChannels == 1)
						printf("Mono, ");
					else
					if (hdr->uNumChannels == 2)
						printf("Stereo, ");
					else
						printf("%d Channels, ", hdr->uNumChannels);
					if (hdr->uLoop)
						printf("Loop");
					else
						printf("One shot");
					printf("\n");
					}
				default: 
					printf("Format %d : Unknown\n", wave->m_baseHeader.uFormat);
					break;
			}
		}
		#endif	// #ifdef AUDIO_TOOLS

		return true;
	}
};

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
