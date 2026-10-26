#include "stdafx.h"
#include "AppHeaders.h"

#define XMA_PACKET_SIZE 2048
#define XMA_PACKET_HEADER_SIZE 4
#define XMA_PACKET_PAYLOAD_SIZE 2044
#define XMA_SUBFRAME_SIZE 128
#define XMA_FRAME_SIZE 512

// Bits 0-7
#define XMA_PACKET_HEADER_FRAME_SKIP_BIT_LOC        0
#define XMA_PACKET_HEADER_FRAME_SKIP_BIT_COUNT      8

#define XMA_PACKET_HEADER_FRAME_SKIP_MAX            ((1 << XMA_PACKET_HEADER_FRAME_SKIP_BIT_COUNT) - 1)

// Bits 8-10
#define XMA_PACKET_HEADER_INFO_BIT_LOC              8
#define XMA_PACKET_HEADER_INFO_BIT_COUNT            3

#define XMA_PACKET_HEADER_INFO_OFFSET_MAX           ((1 << XMA_PACKET_HEADER_INFO_BIT_COUNT) - 1)

#define XMA_PACKET_HEADER_INFO_XMA1                 0
#define XMA_PACKET_HEADER_INFO_XMA2                 1

// Bits 11-25
#define XMA_PACKET_HEADER_FRAME_OFFSET_BIT_LOC      11
#define XMA_PACKET_HEADER_FRAME_OFFSET_BIT_COUNT    15

#define XMA_PACKET_HEADER_FRAME_OFFSET_MAX          ((1 << XMA_PACKET_HEADER_FRAME_OFFSET_BIT_COUNT) - 1)

// Bits 26-31
#define XMA_PACKET_HEADER_FRAME_COUNT_BIT_LOC       26
#define XMA_PACKET_HEADER_FRAME_COUNT_BIT_COUNT     6

#define XMA_PACKET_HEADER_FRAME_COUNT_MAX           ((1 << XMA_PACKET_HEADER_FRAME_COUNT_BIT_COUNT) - 1)

#define PACKET_FRAME_COUNT(pktHdr) ( ( ( pktHdr ) >> XMA_PACKET_HEADER_FRAME_COUNT_BIT_LOC ) & XMA_PACKET_HEADER_FRAME_COUNT_MAX )
#define PACKET_FRAME_OFFSET(pktHdr) ( ( ( pktHdr ) >> XMA_PACKET_HEADER_FRAME_OFFSET_BIT_LOC ) & XMA_PACKET_HEADER_FRAME_OFFSET_MAX )

namespace Audio
{
	// read the xma file from disk into the XMAFile specified
	bool WaveBankWriter::ReadXmaFromDisk(const char* pchFilename, Audio::XMAFile &xmaFile)
	{
		// open the file
		if(FAILED(xmaFile.OpenRead(pchFilename)))
		{
			PrintError("Could not open Xma file '%s'\n", GetRawFilename(pchFilename));
			return false;
		}
		
		// get the format	
		if(FAILED(xmaFile.ReadFormat()))
		{
			PrintError("Could not get wave format for '%s'\n", GetRawFilename(pchFilename));
			return false;
		}

		if (xmaFile.ReadSample()==S_FALSE)
		{
			PrintError("Reading xma sample data.\n");
			return false;
		}

		return true;
	}

	// read an xma from memory and copy it to the Wavebank::Wave.
	// copy xma details and wave data into Wavebank::Wave ready for writing to bank file
	bool WaveBankWriter::ProcessXmaInMemory(Audio::XMAFile* xmaFile, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat, U32 uNumFrames)
	{
		// get the format	
		void* xmaFormat = xmaFile->GetFormat();

		if (xmaFormat==NULL)
		{
			PrintError("Could not get wave format for '%s'\n", pIniFileWave->m_strFilename.data());
			return false;
		}

		// check this is the number of bits we expected
		int uNumberBitsExpected=16;

		if (uRequiredFormat!=Core::uXMA16Bit && uRequiredFormat!=Core::uXMA216Bit)
		{
			PrintError("Unknown format for '%s'. Only 16 bit XMA supported at the moment.\n", pIniFileWave->m_strFilename.data());
			return false;
		}
		
		// fill in the WaveBank headers...
		pBankWave->m_baseHeader.uFormat = uRequiredFormat;

		pBankWave->m_baseHeader.fDefaultLevel = pIniFileWave->m_fDefaultLevel;
		strncpy(pBankWave->m_baseHeader.pchName, pIniFileWave->m_strInternalName.data(), waveNameSize);
		pBankWave->m_baseHeader.pchName[waveNameSize-1] = 0; // ensure null termination

		xmaFile->GetDuration((DWORD*)&pBankWave->m_baseHeader.uDataLength);
		
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->m_iPitchShiftUp;
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->m_iPitchShiftDown << PITCHSHIFT_DOWN_SHIFT;
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->m_iPitchShiftQuality << PITCHSHIFT_QUALITY_SHIFT;
		
		DWORD uNumStreams;
		
		if (uRequiredFormat == Audio::Core::uXMA16Bit)
		{
			uNumStreams = ((XMAWAVEFORMAT*)xmaFormat)->NumStreams;
			pBankWave->m_baseHeader.uSize = sizeof(WaveBank::WaveBaseHeader)+sizeof(WaveBank::XMAWave);			
			pBankWave->m_pExtraHeader = malloc(sizeof(WaveBank::XMAWave));
		}
		else
		{
			uNumStreams = ((XMA2WAVEFORMAT*)xmaFormat)->NumStreams;
		pBankWave->m_baseHeader.uSize = sizeof(WaveBank::WaveBaseHeader)+sizeof(WaveBank::XMAWave) + uNumFrames * sizeof(U32);			
		pBankWave->m_pExtraHeader = malloc(pBankWave->m_baseHeader.uSize - sizeof(WaveBank::WaveBaseHeader));
		}
		
		if (!pBankWave->m_pExtraHeader)
		{
			PrintError("\nError: could not allocate RAM for XMA Wave header\n");
			return false;
		}
	
		if (uNumStreams > 1)
		{
			PrintError("\nXma contains more than 1 stream. Not currently supported.\n");
			return false;
		}
		
		if (uNumStreams == 0)
		{
			PrintError("\nXma contains no streams.\n");
			return false;
		}

		WaveBank::XMAWave* hdr=(WaveBank::XMAWave*)pBankWave->m_pExtraHeader;

		memset(hdr, 0, sizeof(WaveBank::XMAWave));
		
		if (uRequiredFormat == Audio::Core::uXMA16Bit)
		{
			hdr->uBitsPerSample = ((XMAWAVEFORMAT*)xmaFormat)->BitsPerSample;
			hdr->uNumChannels = ((XMAWAVEFORMAT*)xmaFormat)->XmaStreams[0].Channels;
			hdr->uSampleRate = ((XMAWAVEFORMAT*)xmaFormat)->XmaStreams[0].SampleRate;
			hdr->uNumFrames = uNumFrames;
		}

		else
		{
			// for xma2
			hdr->uBitsPerSample = 16;
			hdr->uNumChannels = ((XMA2WAVEFORMAT*)xmaFormat)->Streams[0].Channels;
			hdr->uSampleRate = ((XMA2WAVEFORMAT*)xmaFormat)->SampleRate;
			hdr->uNumFrames = uNumFrames;
		}


		// if loopend or loopstart is non zero there should be a loop present.
		// there might be a better way to detect if a loop is present but this should work.
		// for some reason the xmaFormat->LoopCount doesnt get set by the encoder.
		// for xma2
		if (uRequiredFormat == Audio::Core::uXMA16Bit)
		{
			if (((XMAWAVEFORMAT*)xmaFormat)->XmaStreams[0].LoopEnd || ((XMAWAVEFORMAT*)xmaFormat)->XmaStreams[0].LoopStart)
			{
				hdr->uNumLoops = 255;		// hardcode to infinite loop. can add num loops via ini file later
				hdr->uLoopEndOffsetBit = ((XMAWAVEFORMAT*)xmaFormat)->XmaStreams[0].LoopEnd; 
				hdr->uLoopStartOffsetBit = ((XMAWAVEFORMAT*)xmaFormat)->XmaStreams[0].LoopStart; 
				hdr->uSubframeData = ((XMAWAVEFORMAT*)xmaFormat)->XmaStreams[0].SubframeData;
			}
		}
		else
		{
			if (((XMA2WAVEFORMAT*)xmaFormat)->LoopEnd || ((XMA2WAVEFORMAT*)xmaFormat)->LoopBegin)
			{
				hdr->uNumLoops = 255;		// hardcode to infinite loop. add num loops via ini file later?
				hdr->uLoopEndOffsetBit = ((XMA2WAVEFORMAT*)xmaFormat)->LoopEnd;		// at this point loopEnd & loopBegin is in samples but gets converted to a bit offset in BuildSeektable
				hdr->uLoopStartOffsetBit = ((XMA2WAVEFORMAT*)xmaFormat)->LoopBegin; 
				//Print("
				// we calculate the subframedata in BuildSeekTable later
				//hdr->uSubframeData = ((XMA2WAVEFORMAT*)xmaFormat)->Streams[0].SubframeData;
			}
		}

		// TODO: eventually, we want to get this information from the XMA2WAVEFORMAT struct when using XMA2
		//hdr->uNumSamples = pBankWave->m_baseHeader.uDataLength / hdr->uBitsPerSample / 8 / hdr->uNumChannels;

		// round up to 128 samples

		// TODO. stride is how big each interleaved stereo segment is. ie stride of 1 is 1 left sample then 1 right sample
		// stride of 256 is 256 l samples, then 256 r samples, then 256 l samples etc. 
		// nickb said it might not be needed but good to have
		hdr->uStride = 0;
		
		// get the data
		// THIS MUST BE MALLOC AS IT WILL BE FREED BY free in Audio::System::Free
		pBankWave->m_pvWaveData = (char*) malloc(pBankWave->m_baseHeader.uDataLength);
		if (!pBankWave->m_pvWaveData)
		{	
			PrintError("could not allocate RAM for XMA Wave data\n");
			return false;
		}

		char* pchSampleData = (char*)xmaFile->GetSampleData();

		if (pchSampleData==NULL)
		{
			PrintError("sample data does not exist for XMA Wave\n");
			return false;
		}

		// copy the xma sample data
		memcpy(pBankWave->m_pvWaveData, pchSampleData, pBankWave->m_baseHeader.uDataLength);

		return true;
	}

	bool BuildSeekTable(const U8* pPacket, U32 frameCount, U32* seekTable, U32 loopCount, U32 &loopStart, U32 &loopEnd, U8& subFrameData, DWORD dataSize)
	{
		const U8* pCurrentPacket = pPacket;

		U32 currentFrame = 0;
		U32 currentPacket = 0;
		U32 currentPacketOffsetBit = 0;

		// loopstart and loopend come in as number of samples 
		// but then gets converted to a bit offset below
		DWORD loopStartOrig = loopStart;
		DWORD loopEndOrig = loopEnd;
		DWORD loopStartFrame = loopStart;
		DWORD loopEndFrame = loopEnd;

		EndianSwitchDWords(&loopStartFrame, 1);
		EndianSwitchDWords(&loopEndFrame, 1);
		
		loopEndFrame -= 1;

		if (loopCount > 0)
		{
			if (bVerboseMode)
				Print(0, "Loop start is %d, Loop end is %d. (in samples)\n", loopStartFrame, loopEndFrame);

			U8 loopSubFrameSkip = (U8) ((loopStartFrame / XMA_SUBFRAME_SIZE) & 0x3);
			U8 loopSubFrameEnd = (U8) ((loopEndFrame / XMA_SUBFRAME_SIZE) & 0x3);
			
			if (loopSubFrameSkip == 0)
			{
				loopSubFrameSkip = 4;
				Assert(loopStartFrame != 0);
				--loopStartFrame;
			}

			loopStartFrame /= XMA_FRAME_SIZE;
			loopEndFrame /= XMA_FRAME_SIZE;

			subFrameData = loopSubFrameSkip | (loopSubFrameEnd << 4);
			
			if (bVerboseMode)
			{
				Print(0, "Calculated subframe skip of %d & subframe end of %d.\n", loopSubFrameSkip, loopSubFrameEnd);
				Print(0, "Number of frames = %d. loop start frame = %d. loop end frame = %d\n", frameCount, loopStartFrame, loopEndFrame);
			}

			static bool bPrintedInfo=false;
			if (bPrintedInfo==false)
			{
				Print(0, "Info: Subframe end = Subframe number of loop end point [0,3].\n");
				Print(0, "Info: Subframe skip = Number of subframes to skip before decoding and outputting at the loop start point [1,4].\n");
				bPrintedInfo = true;
			}
		}
		
		while (currentFrame < frameCount)
		{
			DWORD packetHeader = *(U32*) pCurrentPacket;
			EndianSwitchDWords(&packetHeader, 1);

			//Parse the packet to find the frame count and initial offset
			U32 packetFrameCount = PACKET_FRAME_COUNT(packetHeader);
			U32 currentPacketFrame = 0;
			U32 currentFrameOffsetBit = PACKET_FRAME_OFFSET(packetHeader) + XMA_PACKET_HEADER_SIZE * 8;
			bool lastFrame = false;

			if (currentFrameOffsetBit > 16383)
			{
				currentFrameOffsetBit += 32;
			}

			while (!lastFrame)
			{
				//Print(0, "%3d processing frame %3d of %3d packet frames. total = %d. loopStartFrame = %d loopEndFrame = %d\n", currentFrame, currentPacketFrame, packetFrameCount, frameCount, loopStartFrame, loopEndFrame);

				seekTable[currentFrame] = currentFrameOffsetBit + currentPacketOffsetBit;

				if (loopCount > 0)
				{
					if (currentFrame == loopStartFrame)
					{
						loopStart = currentFrameOffsetBit + currentPacketOffsetBit;
						//Print(0, "created loopStart\n");
					}

					if (currentFrame == loopEndFrame)
					{
						loopEnd = currentFrameOffsetBit + currentPacketOffsetBit;
						//Print(0, "created loopend\n");
					}
				}

				// check for buffer overruns
				if (!((DWORD)pCurrentPacket + (currentFrameOffsetBit / 8) <  ((DWORD)pPacket + dataSize)))
				{
					PrintError("BuildSeekTable() - Buffer read overrun when walking through xma bitstream\n");
					return false;
				}

				DWORD packetFrameOffset;
				DWORD frameHeaderShift = (17 - (currentFrameOffsetBit & 0x7));

				if (currentFrameOffsetBit > 16369 && currentFrameOffsetBit < 16384)
				{
					DWORD packetFrameOffset1 = *((DWORD*)(pCurrentPacket + (currentFrameOffsetBit / 8 & 0xfffffffe)));
					DWORD packetFrameOffset2 = *((DWORD*)(pCurrentPacket + XMA_PACKET_SIZE + XMA_PACKET_HEADER_SIZE - 2));

					EndianSwitchDWords(&packetFrameOffset1, 1);
					EndianSwitchDWords(&packetFrameOffset2, 1);

					packetFrameOffset = (packetFrameOffset1 & 0xffff0000) | (packetFrameOffset2 & 0xffff);

					if (currentFrameOffsetBit & 0x1)
					{
						frameHeaderShift -= 8;
					}
				}
				else
				{
					packetFrameOffset = *((DWORD*)(pCurrentPacket + (currentFrameOffsetBit / 8)));

					EndianSwitchDWords(&packetFrameOffset, 1);
				}

				WORD currentFrameLengthBits = (WORD) (packetFrameOffset >> frameHeaderShift) & 0x7fff;
				WORD nextFrameOffsetBit = currentFrameOffsetBit + currentFrameLengthBits;

				if (nextFrameOffsetBit > 32736)
				{
					lastFrame = true;
				}

				else
				{
					if (currentFrameOffsetBit <= 16383 && nextFrameOffsetBit > 16383)
					{
						nextFrameOffsetBit += 32;
					}

					currentFrameOffsetBit = nextFrameOffsetBit;

					WORD lastFrameOffsetBit = currentFrameOffsetBit - 1;

					if (lastFrameOffsetBit == 16415)
					{
						lastFrameOffsetBit -= 32;
					}
					
					// check for buffer overruns
					if (!((DWORD)pCurrentPacket + (lastFrameOffsetBit / 8) <  ((DWORD)pPacket + dataSize)))
					{
						PrintError("BuildSeekTable() - Buffer read overrun when walking through xma bitstream\n");
						return false;
					}

					BYTE frameEnd = *((BYTE*)(pCurrentPacket + (lastFrameOffsetBit / 8)));

					lastFrame = (frameEnd >> (7 - (lastFrameOffsetBit & 0x7)) & 0x01) == 0 ? true : false;
				}

				++currentFrame;
				++currentPacketFrame;
				Assert(currentFrame <= frameCount, "");
			}

			Assert(lastFrame);
			pCurrentPacket += XMA_PACKET_SIZE;
			currentPacketOffsetBit += XMA_PACKET_SIZE * 8;
			++currentPacket;
		}
		
		if (loopCount > 0)
		{
			if (loopStartOrig == loopStart)
			{
				PrintError("BuildSeekTable() - Did not create loopStart bit offset\n");
				return false;
			}
			
			if (loopEndOrig == loopEnd)
			{
				PrintError("BuildSeekTable() - Did not create loopEnd bit offset. Is your sample 512 sample aligned?\n");
				return false;
			}
		}

		return true;
	}

	bool WaveBankWriter::ConvertToXma(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat)
	{
		if (uRequiredFormat != Core::uXMA216Bit && uRequiredFormat != Core::uXMA16Bit)
		{
			PrintError("ConvertToXma() - invalid format\n");
			return false;
		}

		// read the wav in and encode it to an xma in memory - then process the xma.
		CWaveFile wavFile;
		Audio::XMAFile xmaFile;

		if (ReadWavFromDisk(pchFilename, &wavFile)==false)
			return false;
	
		// sample rate convert it if we need to
		if (pIniFileWave->m_iTargetSampleRate != -1)
		{
			wavFile.SampleRateConvert(pIniFileWave->m_iTargetSampleRate);

			if (bWriteIntermediateFiles)
			{
				char pchConvertedFilename[MAX_PATH];
				_snprintf(pchConvertedFilename, MAX_PATH, "%s_%dHz.wav", GetRawFilename(pchFilename), pIniFileWave->m_iTargetSampleRate);
				wavFile.OpenForWrite(pchConvertedFilename);
				wavFile.Write();
			}
		}

		Print(0, "Converting '%s' to %s. Quality=%d...", GetRawFilename((char*)pchFilename), uRequiredFormat == Core::uXMA216Bit ? "Xma2" :"Xma1", pIniFileWave->m_iCompressionQuality);

		// take loop info from wav file unless specified in inifile
		bool bEncodeAsLoop=false;
		if (pIniFileWave->m_eLoop == WaveBankIniFile::E_LOOP_NOT_SPECIFIED)
		{
			if (wavFile.IsLoop())
				bEncodeAsLoop = true;
			else
				bEncodeAsLoop = false;
		}
		else
		if (pIniFileWave->m_eLoop == WaveBankIniFile::E_LOOP_YES)
			bEncodeAsLoop = true;
		else
			bEncodeAsLoop = false;

		if (bEncodeAsLoop)
		{
			DWORD numSamples;

			wavFile.GetNumSamples(&numSamples);

			if (numSamples % 128 != 0)
			{
				PrintWarning("Sample '%s' is not %s sample aligned (contains %d samples)", GetRawFilename((char*)pchFilename), uRequiredFormat == Core::uXMA216Bit ? "512" : "128", numSamples);
			}
		}

		U32 uNumSamples;

		if (FAILED(xmaFile.Encode(&wavFile, uNumSamples, bEncodeAsLoop? XMAENCODER_LOOP:0 , pIniFileWave->m_iCompressionQuality, uRequiredFormat == Core::uXMA216Bit)))
			return false;
		
		U32 uNumFrames = (uNumSamples - 1) / XMA_FRAME_SIZE + 1;

		if (ProcessXmaInMemory(&xmaFile, pIniFileWave, pBankWave, uRequiredFormat, uNumFrames)==false)
			return false;
		
		if (uRequiredFormat == Core::uXMA216Bit)
		{
			WaveBank::XMAWave* pWave = pBankWave->XmaWave();
			U32* seekTable = (U32*)((U8*) pBankWave->m_pExtraHeader + sizeof(WaveBank::XMAWave));

			if (!BuildSeekTable((U8*) pBankWave->m_pvWaveData, pWave->uNumFrames, seekTable, pWave->uNumLoops, pWave->uLoopStartOffsetBit, pWave->uLoopEndOffsetBit, pWave->uSubframeData,pBankWave->DataLength()))
			{
				PrintError("Could not build seek table\n");
				return false;
			}
			EndianSwitchDWords((DWORD*) seekTable, pWave->uNumFrames);
		}

		if (bWriteIntermediateFiles)
		{
			if (strlen(pchFilename)<5)
				return false;
			char* pchStr = new char[strlen(pchFilename)+1];
			strcpy(pchStr, pchFilename);
			strcpy( &pchStr[strlen(pchStr)-3], "xma");
			Print(0, "\nWriting intermediate xma file '%s'...\n", pchStr); 
			if (SUCCEEDED(xmaFile.OpenWrite( pchStr )))
				xmaFile.Write();
		}
		
		Print(FOREGROUND_GREEN | FOREGROUND_INTENSITY, "ok ");
		
		if (pBankWave->m_baseHeader.uDataLength < 1024)
			Print(0, "[Compressed to %d bytes]\n", pBankWave->m_baseHeader.uDataLength);
		else
			Print(0, "[Compressed to %d kB]\n", pBankWave->m_baseHeader.uDataLength/1024);
		
		return true;
	}

	// read an xma from disk into the WaveBank::Wave
	bool WaveBankWriter::ReadXma(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat)
	{
		Audio::XMAFile xmaFile;

		if (ReadXmaFromDisk(pchFilename, xmaFile)==false)
		{
			return false;
		}
		
		if (ProcessXmaInMemory(&xmaFile, pIniFileWave, pBankWave, uRequiredFormat, 0)==false)
		{
			return false;
		}
		
		return true;
	}

}	// namespace Audio
