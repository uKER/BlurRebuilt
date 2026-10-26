#include "stdafx.h"
#include "AppHeaders.h"

namespace Audio
{
	// iterate through waves listed in inifile.
	// for each wave, read the waveheader, and read wavedata into the bank object
	// then write all waves to a single file, with a bank header
	bool WaveBankWriter::Write(WaveBankIniFile& iniFile)
	{
		if (m_pWaveBank==NULL)
			return false;

		WaveBank& bank = *m_pWaveBank;

		// setup the bank header
		bank.m_BankHeader.uNumWaves = iniFile.NumWaves();
		
		strncpy(bank.m_BankHeader.pchName, iniFile.BankName().Chars(), g_WbWaveNameSize);
		bank.m_BankHeader.pchName[g_WbWaveNameSize-1]=0;

		bank.m_BankHeader.chEndian = iniFile.Endian();
		bank.m_BankHeader.chStreaming = iniFile.Streaming();

		if (gSettings.gbDeleteIntermediateFiles)
			DeleteTempFiles(iniFile);

		//Audio::Print(0, "Processing wavs...\n");

		#ifdef DISPLAY_PROFILE_INFO
		StopWatch sw;
		#endif

		try
		{
			bool bEverythingWasOk=true;

			for (int i=0; i<iniFile.NumWaves(); i++)
			{
				WaveBankIniFile::Wave *iniWave=iniFile.GetWave(i);
				
				if (iniWave==NULL)		// something went funny. do one
					throw "Could not get wave info from iniFile. wave is NULL";

				WaveBank::Wave* pWave = new WaveBank::Wave;
				if (pWave == NULL)
					throw "Could not allocate WaveBank::Wave";

				bank.m_vpWaves.PushBack(pWave);

				if (bank.m_vpWaves[i]==NULL)
					throw "Could not allocate memory for wavebank wave class.";

				SetProgress((F32)i / (F32)iniFile.NumWaves());

				//Util::C_String strFullFname = iniFile.Path()+iniWave->m_strFilename;
				Util::C_String strFullFname = iniWave->Filename();

				// see what sort of file it should be, read the files into ram into the relevant structures
				switch (iniWave->Format())
				{
					case WaveBankIniFile::E_FORMAT_PCM_WAV_S16:
						if (ReadWav(strFullFname.Chars(), iniWave, bank.GetWave(i), Core::uPCM16BitSignedInt)==false)
							bEverythingWasOk=false;
						break;
					case WaveBankIniFile::E_FORMAT_PCM_WAV_S16_TO_F32:
						// read in signed 16 bit wav and convert to 32 bit float
						if (ReadWav(strFullFname.Chars(), iniWave, bank.GetWave(i), Core::uPCM16BitSignedInt)==false)
							bEverythingWasOk=false;
						if (bEverythingWasOk==true)
							PcmSigned16ToFloat32(bank.GetWave(i));
						break;
					case WaveBankIniFile::E_FORMAT_XMA:
						if (ReadXma(strFullFname.Chars(), iniWave, bank.GetWave(i), Core::uXMA16Bit)==false)
							bEverythingWasOk=false;
						break;
					case WaveBankIniFile::E_FORMAT_ADPCM:
						if (ReadAdpcm(strFullFname.Chars(), iniWave, bank.GetWave(i), Core::uADPCM4Bit)==false)
							bEverythingWasOk=false;
						break;
					case WaveBankIniFile::E_FORMAT_MP3:
						if (ReadMp3(strFullFname.Chars(), iniWave, bank.GetWave(i), Core::uMP3)==false)
							bEverythingWasOk=false;
						break;
					case WaveBankIniFile::E_FORMAT_PCM_WAV_S16_TO_XMA:
						if (ConvertToXma(strFullFname.Chars(), iniWave, bank.GetWave(i), Core::uXMA16Bit)==false)
							bEverythingWasOk=false;
						break;
					case WaveBankIniFile::E_FORMAT_PCM_WAV_S16_TO_XMA2:
						if (ConvertToXma(strFullFname.Chars(), iniWave, bank.GetWave(i), Core::uXMA216Bit)==false)
							bEverythingWasOk=false;
						break;
					case WaveBankIniFile::E_FORMAT_PCM_WAV_S16_TO_ADPCM:
						if (ConvertToAdpcm(strFullFname.Chars(), iniWave, bank.GetWave(i), Core::uADPCM4Bit)==false)
							bEverythingWasOk=false;
						break;
					// TODO
					//case WaveBank::eFormatVAG: 
					//	break;
					default:
						throw "Wave has unknown format.";
						break;
				}

				if (!bEverythingWasOk)
					throw("Could not process wave\n");
			}

			//#ifdef AUDIO_TOOLS_CMD_LINE
			//if (!bEverythingWasOk)
			//	throw("Could not process one or more wav files\n");
			//#endif
		}

		catch(char* str)
		{
			PrintError(str);
			return false;
		}

		#ifdef DISPLAY_PROFILE_INFO
		Audio::Print(TXTCOLOUR_BLUE, "\nReading waves and converting took %.4f seconds.\n", sw.Time());
		#endif

		//Audio::Print(0, "Processed wavs ok\n");
		
		#ifdef DISPLAY_PROFILE_INFO
		sw.Start();
		#endif

		// calculate where the data starts.
		// its after the bank header and all the wave headers.
		int curOffset=bank.m_BankHeader.uSize;
		for (U32 i=0; i<bank.NumWaves(); i++)
			curOffset += bank.GetWave(i)->m_baseHeader.uSize // uSize = base + extra
						+ bank.GetWave(i)->m_baseHeader.uMarkerChunkSize
						+ bank.GetWave(i)->m_baseHeader.uStreamingInfoChunkSize;

		U32 uHeaderSize = sizeof(WaveBank::ChunkHeader);

		// calculate data offsets for wave headers
		for (U32 i=0; i<bank.NumWaves(); i++)
		{
			// align data to 2k boundary if this is a streaming bank
			if (bank.m_BankHeader.chStreaming)
			{
				const U32 uAlign = 2048;
				U32 uActualDataStart = curOffset + uHeaderSize;	
				U32 uAlignedOffset = uActualDataStart / uAlign * uAlign + (((uActualDataStart % uAlign == 0) ? 0 : 1) * uAlign);

				curOffset = uAlignedOffset;
				bank.GetWave(i)->m_baseHeader.uDataStart = curOffset;

				curOffset += bank.GetWave(i)->m_baseHeader.uDataLength;	
			}
			else
			{
				bank.GetWave(i)->m_baseHeader.uDataStart = curOffset + uHeaderSize;
				curOffset += bank.GetWave(i)->m_baseHeader.uDataLength + uHeaderSize;
			}

			//Print(0, "wave %d data offset %d\n", i, bank.GetWave(i)->m_baseHeader.uDataStart);
		}
		
		#ifdef DISPLAY_PROFILE_INFO
		Audio::Print(TXTCOLOUR_BLUE, "Calculating data offsets took %.4f seconds.\n", sw.Time());

		sw.Start();
		#endif
		// convert the endian if needed
		if (WaveDataEndianSwap()==false)
			return false;
		
		#ifdef DISPLAY_PROFILE_INFO
		Audio::Print(TXTCOLOUR_BLUE, "Endian swap took %.4f seconds.\n", sw.Time());
		#endif

		// now write the file to disk
		return WriteBankFile(iniFile);
	}
	
	// the file is written in this way
	//
	// abridged description:
	//
	//	 [bankheader]
	//	 [wave headers]
	//	 [wave data]
	//
	// verbose description:
	//
	//   [bankheader]
	//   [wave 0 base header]
	//   [wave 0 extra header]
	//   [wave 1 base header]
	//   [wave 1 extra header]
	//   [wave 2 base header]
	//   [wave 2 extra header]
	//   [...]
	//	 [wave n base header]
	//   [wave n extra header]
	//	 [wave 0 data header]
	//   [wave 0 data bytes]
	//   [wave 1 data header]
	//   [wave 1 data]
	//   [wave 2 data header]
	//   [wave 2 data]
	//   [...]
	//	 [wave n data header]
	//   [wave n data bytes]
	bool WaveBankWriter::WriteBankFile(WaveBankIniFile& iniFile)
	{
		WaveBank& bank = *m_pWaveBank;
		U32 uFilePos = 0;
		File file;
		U32* piDataSize = new U32[bank.m_BankHeader.uNumWaves];
		U32* piDataStart = new U32[bank.m_BankHeader.uNumWaves];
		U32 uTotalPadBytesWritten=0;
		
		
		#ifdef DISPLAY_PROFILE_INFO
		StopWatch sw;
		#endif
		//Audio::Print("Writing wavebank '%s'...", bank.m_pchFilename);
		
		Util::C_String strFullFilename;

		// if an output path has been set use that otherwise use the same path as the ini file
		if (OutputPath() != "")
			strFullFilename = OutputPath() + bank.Filename();
		else
			strFullFilename = iniFile.Path() + bank.Filename();

		try
		{
			if (OutputPath() != "")
			{
				// see if we need to create the directory
				Util::C_String strPathNoSlash = Util::RemoveTrailingSlash(OutputPath());
				if (Sys::C_FileSystem::Instance().FileExists(strPathNoSlash))
				{
					// see if dir or file
					Sys::C_Fileinfo inf;
					if (Sys::C_FileSystem::Instance().FileInfo(strPathNoSlash, &inf))
					{
						if (inf.m_FileType != Sys::E_FileType_Directory)
							throw Util::C_String().Printf("Destination dir '%s' is not a directory. Is it a file?", OutputPath().Chars());
					}
				}
				else
				{
					Print(0, "Directory '%s' does not exist. Creating...\n", OutputPath().Chars());

					if (Sys::C_FileSystem::Instance().CreateDirectory(OutputPath())==false)
						throw Util::C_String().Printf("Could not create directory '%s'", OutputPath().Chars());
				}
			}

			if (file.OpenWrite(strFullFilename.Chars())==false)
			{
				PrintWarning(Util::C_String(GetWindowsErrorMessage(GetLastError()) + Util::C_String(" [May not be relevant]\n")).Chars());
				throw Util::C_String("could not open wavebank file '") + strFullFilename + "'";
			}

			U32 iNumWaves=bank.m_BankHeader.uNumWaves;

			// endian swap and write bank header
			if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
			{
				EndianSwitchDWords((DWORD*)&bank.m_BankHeader.uSize, 1);
				EndianSwitchDWords((DWORD*)&bank.m_BankHeader.uVersion, 1);
				EndianSwitchDWords((DWORD*)&bank.m_BankHeader.uNumWaves, 1);

			}

			if (file.Write(&bank.m_BankHeader, sizeof(WaveBank::BankHeader))==false)
				throw Util::C_String("Could not write bank header");

			uFilePos += sizeof(WaveBank::BankHeader);

			// endian swap and write wave headers
			for (U32 i=0; i<bank.NumWaves(); i++)
			{
				WaveBank::Wave* wave = bank.GetWave(i);

				if (wave->m_baseHeader.uSize < sizeof(WaveBank::WaveBaseHeader))
					throw(Util::C_String("Total waveheader size is less than base header size"));

				if (wave->m_pExtraHeader==NULL)
					throw(Util::C_String("Extra waveheader is null"));

				// endian swap extra data
				switch (wave->m_baseHeader.uFormat)
				{
					case Core::uPCM16BitSignedInt: 
					case Core::uPCM32BitFloat: 
					{
						if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
						{
							WaveBank::PCMWave* pPcmWave = wave->PcmWave();
							EndianSwitchDWords((DWORD*)&pPcmWave->uSampleRate, 1);
							EndianSwitchDWords((DWORD*)&pPcmWave->uBitsPerSample, 1);
							EndianSwitchDWords((DWORD*)&pPcmWave->uNumChannels, 1);
							EndianSwitchDWords((DWORD*)&pPcmWave->uStride, 1);
						}
							//NativeToBigEndian(wave->PcmWave(), "iiiis");	// uncomment for loop suuport
						}
						break;
					case Core::uXMA16Bit: 
					{
 						if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
						{
							WaveBank::XMAWave* pXmaWave = wave->XmaWave();
							EndianSwitchDWords((DWORD*)&pXmaWave->uSampleRate, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uBitsPerSample, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uNumChannels, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uStride, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uLoopStartOffsetBit, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uLoopEndOffsetBit, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uNumFrames, 1);
						}
					}
						break;
					case Core::uXMA216Bit: 
					{
						if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
						{
							WaveBank::XMAWave* pXmaWave = wave->XmaWave();
							EndianSwitchDWords((DWORD*)&pXmaWave->uBitsPerSample, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uNumChannels, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uStride, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uLoopStartOffsetBit, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uLoopEndOffsetBit, 1);
							EndianSwitchDWords((DWORD*)&pXmaWave->uNumFrames, 1);
						}
					}
						break;
					case Core::uMP3: 
						if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
						{
							WaveBank::MP3Wave* pMp3Wave = wave->Mp3Wave();
							EndianSwitchDWords((DWORD*)&pMp3Wave->uSampleRate, 1);
							EndianSwitchDWords((DWORD*)&pMp3Wave->uBitsPerSample, 1);
							EndianSwitchDWords((DWORD*)&pMp3Wave->uNumChannels, 1);
							EndianSwitchWords((WORD*)&pMp3Wave->uLoop, 1);
						}
						break;
					case Core::uADPCM4Bit: 
						if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
						{
							WaveBank::ADPCMWave* pAdpcmWave = wave->AdpcmWave();
							EndianSwitchDWords((DWORD*)&pAdpcmWave->uSampleRate, 1);
							EndianSwitchDWords((DWORD*)&pAdpcmWave->uBitsPerSample, 1);
							EndianSwitchDWords((DWORD*)&pAdpcmWave->uNumChannels, 1);
							EndianSwitchWords((WORD*)&pAdpcmWave->uLoop, 1);
						}
						break;
					default:
						throw "Wave has unknown format.";
						break;
				}

				// get this before we swap the endian-ness of the base-header
				int iExtraHeaderSize=wave->m_baseHeader.uSize-sizeof(WaveBank::WaveBaseHeader);

				// copy the data offsets and sizes, for using when we write the data later, 
				// as we might mangle them if we need to do the endian swap here
				piDataSize[i] = wave->m_baseHeader.uDataLength;
				piDataStart[i] = wave->m_baseHeader.uDataStart;

				U32 origMarkerChunkSize = wave->m_baseHeader.uMarkerChunkSize;
				U32 origStreamingChunkSize = wave->m_baseHeader.uStreamingInfoChunkSize;

				// Endian swap base header if necessary
				if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
				{
					// note: we dont swap uTag or pchName
					EndianSwitchDWords((DWORD*)&wave->m_baseHeader.uSize, 1);
					EndianSwitchDWords((DWORD*)&wave->m_baseHeader.uDataStart, 1);
					EndianSwitchDWords((DWORD*)&wave->m_baseHeader.uDataLength, 1);
					EndianSwitchDWords((DWORD*)&wave->m_baseHeader.fDefaultLevel, 1);
					EndianSwitchDWords((DWORD*)&wave->m_baseHeader.uFlags, 1);
					EndianSwitchDWords((DWORD*)&wave->m_baseHeader.uFormat, 1);
					EndianSwitchDWords((DWORD*)&wave->m_baseHeader.uMarkerChunkSize, 1);
					EndianSwitchDWords((DWORD*)&wave->m_baseHeader.uStreamingInfoChunkSize, 1);
				}

				if (file.Write(&wave->m_baseHeader, sizeof(WaveBank::WaveBaseHeader))==false)
					throw Util::C_String("Could not write wave base header");
				
				uFilePos += sizeof(WaveBank::WaveBaseHeader);

				if (file.Write(wave->m_pExtraHeader, iExtraHeaderSize)==false)
					throw Util::C_String("Could not write wave extra header");

				uFilePos += iExtraHeaderSize;

				// Endian swap the marker chunk if there is one
				if (origMarkerChunkSize)
				{
					if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
					{
						EndianSwitchDWords((DWORD*)&wave->m_pMarkerChunk->uSize, 1);
						//EndianSwitchDWords((DWORD*)&wave->m_pMarkerChunk->uTag, 1);
						//EndianSwitchDWords((DWORD*)&wave->m_pMarkerChunk->uType, 1);
						EndianSwitchDWords((DWORD*)&wave->m_pMarkerChunk->uNumMarkers, 1);
						Ut::Vector<Marker*>::Iterator it;
						for (it =  wave->m_pMarkerChunk->MarkerList.Begin(); it !=  wave->m_pMarkerChunk->MarkerList.End(); ++it)
						{
							EndianSwitchDWords((DWORD*)&(*it)->uMarkerID, 1);
							EndianSwitchDWords((DWORD*)&(*it)->uSampleOffset, 1);
						}						
					}

					if (file.Write(wave->m_pMarkerChunk, sizeof(MarkerChunk) - 4)==false)
						throw Util::C_String("Could not write marker chunk");			

					uFilePos += sizeof(MarkerChunk) - 4;

					Ut::Vector<Marker*>::Iterator it;
					for (it =  wave->m_pMarkerChunk->MarkerList.Begin(); it !=  wave->m_pMarkerChunk->MarkerList.End(); ++it)
					{
						if (file.Write((*it), sizeof(Marker))==false)
							throw Util::C_String("Could not write marker chunk");	

						uFilePos += sizeof(Marker);
					}		
				}

				// Endian swap the streaming info chunk if there is one
				if (origStreamingChunkSize)
				{
					if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
					{
						EndianSwitchDWords((DWORD*)&wave->m_pStreamingChunk->uSize, 1);
						//EndianSwitchDWords((DWORD*)&wave->m_pStreamingChunk->uTag, 1);
						EndianSwitchDWords((DWORD*)&wave->m_pStreamingChunk->uType, 1);
						EndianSwitchDWords((DWORD*)&wave->m_pStreamingChunk->uCircularBufferSize, 1);
						EndianSwitchDWords((DWORD*)&wave->m_pStreamingChunk->uNumPrecacheChunks, 1);
						EndianSwitchDWords((DWORD*)&wave->m_pStreamingChunk->uReadChunkSize, 1);
						EndianSwitchDWords((DWORD*)&wave->m_pStreamingChunk->uStreamStartBufferSize, 1);
					}

					if (file.Write(wave->m_pStreamingChunk, origStreamingChunkSize)==false)
						throw Util::C_String("Could not write marker chunk");

						uFilePos += origStreamingChunkSize;
				}				
			}

			// if streaming allocate some padding bytes
			U8* puPadBytes = 0;
			if (bank.m_BankHeader.chStreaming)
			{
				puPadBytes = new U8[2048];

				memset(puPadBytes, 0, 2048*sizeof(U8));
				if (!puPadBytes)
					throw Util::C_String().Printf("Could not allocate padding bytes");
			}

			WaveBank::ChunkHeader dataHeader;
			memset(&dataHeader, 0, sizeof(WaveBank::ChunkHeader));
			dataHeader.uTag=*((U32*)g_pchWbTag_Data);

			// write data
			for (U32 i=0; i<bank.NumWaves(); i++)
			{
				dataHeader.uSize = piDataSize[i] + sizeof(WaveBank::ChunkHeader);
				if (bank.m_BankHeader.chEndian==WaveBank::eBigEndian)
					EndianSwitchDWords((DWORD*)&dataHeader.uSize, 1);
				
				// if streaming bank write padding bytes to make sure write the data 2k aligned
				if (bank.m_BankHeader.chStreaming)
				{
					U32 uHeaderStart = piDataStart[i] - sizeof(WaveBank::ChunkHeader);
					if (uFilePos > uHeaderStart)
						throw Util::C_String("File pos is greater than data start offset.");

					U32 uNumPadBytes = uHeaderStart - uFilePos;
					if (uNumPadBytes)
					{
						assert (uNumPadBytes < 2048);

						if (file.Write(puPadBytes, uNumPadBytes)==false)
						{
							delete [] puPadBytes;
							throw Util::C_String("Could not write padding bytes");
						}
						uFilePos += uNumPadBytes;
						uTotalPadBytesWritten += uNumPadBytes;
					}
				}

				if (file.Write(&dataHeader, sizeof(WaveBank::ChunkHeader))==false)
					throw Util::C_String("Could not write data header");

				uFilePos += sizeof(WaveBank::ChunkHeader);

				if (file.Write(bank.GetWave(i)->m_pvWaveData, piDataSize[i])==false)
					throw Util::C_String("Could not write data");
				
				uFilePos += piDataSize[i];

				//if (gSettings.gbVerboseMode)
				//Audio::Print(0, "\nAdded wave '%s'", iniFile.GetWave(i)->Filename().Chars());
			}
			
			if (puPadBytes)
				delete [] puPadBytes;

			Audio::Print(0, "\n");
				
			Audio::Print(0, "Added %d waves.\n\n", bank.NumWaves());
		}

		catch(Util::C_String str)
		{
			PrintError("Error writing file : %s\n", str.Chars());
			delete [] piDataSize;
			delete [] piDataStart;
			file.Close();
			//DeleteFile(fname.data());
			return false;
		}

		delete [] piDataSize;
		delete [] piDataStart;
		file.Close();

		if (OutputPath() == "")
			Audio::Print(0, "Wrote wavebank '%s' ok\n", bank.m_pchFilename);
		else
			Audio::Print(0, "Wrote wavebank '%s' to '%s' ok\n", bank.m_pchFilename, OutputPath().Chars());
		
		if (uTotalPadBytesWritten)
		{
			if (uTotalPadBytesWritten < 1024)
				Print(0, "Wrote %d padding bytes.\n", uTotalPadBytesWritten);
			else
				Print(0, "Wrote %d k of padding bytes.\n", uTotalPadBytesWritten/1024);
		}

		#ifdef DISPLAY_PROFILE_INFO
		Audio::Print(TXTCOLOUR_BLUE, "Writing bank took %.4f seconds.\n", sw.Time());
		#endif

		return true;
	}
	
	bool WaveBankWriter::NeedToEndianSwapDataFormat(U32 uFormat)
	{
		switch (uFormat)
		{
			case Core::uPCM32BitFloat:
			case Core::uPCM16BitSignedInt:
				return true;

			case Core::uXMA16Bit:
			case Core::uXMA216Bit:
				return false;

			case Core::uADPCM4Bit:
				return false;

			case Core::uMP3:
				return false;

			default:
				PrintError("WaveBankWriter::NeedToEndianSwapDataFormat() : format %d not dealt with", uFormat);
				return false;
		}
		return false;
	}

	bool WaveBankWriter::WaveDataEndianSwap()
	{
		WaveBank& bank = *m_pWaveBank;

		// if bank is little endian then no need to do anything
		if (bank.m_BankHeader.chEndian==WaveBank::eLittleEndian)
			return true;

		//Audio::Print(0, "Doing data endian-swap...");

		for (U32 i=0; i<bank.NumWaves(); i++)
		{
			WaveBank::Wave* wave = bank.GetWave(i);

			// if this is an xma then dont endian swap
			if (!NeedToEndianSwapDataFormat(wave->m_baseHeader.uFormat))
				continue;

			U32 bitsPerSample=wave->BitsPerSample();

			if (bitsPerSample == 8)	// no need to do anything
				;
			else
			if (bitsPerSample == 16)
				EndianSwitchWords((U16*)wave->m_pvWaveData, wave->m_baseHeader.uDataLength/2);
			else
			if (bitsPerSample == 32)
				EndianSwitchDWords((DWORD*)wave->m_pvWaveData, wave->m_baseHeader.uDataLength/4);
			else
			{
				PrintError("WaveBankWriter::BankEndianSwap() - wave %d '%s' unsupported sample bitdepth: %d bits per sample\n", i, wave->m_baseHeader.pchName, bitsPerSample);
				return false;
			}
		}

		//Audio::Print(0, "done\n");

		return true;
	}

	// TODO: 24 bit pcm support?
	bool WaveBankWriter::PcmSigned16ToFloat32(WaveBank::Wave* pWave)
	{
		if (pWave->m_baseHeader.uFormat != Core::uPCM16BitSignedInt)
		{
			PrintError("Can only convert signed 16 bit PCM waves to float");
			return false;
		}

		pWave->m_baseHeader.uFormat = Core::uPCM32BitFloat;

		//if (gSettings.gbVerboseMode)
			Audio::Print(0, "Converting '%s' to 32 bit float...", pWave->m_baseHeader.pchName);

		WaveBank::PCMWave* pPcmWave = (WaveBank::PCMWave*)pWave->m_pExtraHeader;

		F32* pfFloatData=0;
		S32 f32Length=0;
		if (S16ToF32((S16*)pWave->m_pvWaveData, pWave->m_baseHeader.uDataLength, &pfFloatData, &f32Length)==false)
			return false;
		
		pPcmWave->uBitsPerSample = 32;

		// delete the old s16 data
		delete pWave->m_pvWaveData;

		// point at the newly converted f32 data and set the new size
		pWave->m_pvWaveData = pfFloatData;
		pWave->m_baseHeader.uDataLength = f32Length;

		//if (gSettings.gbVerboseMode)
			Audio::Print(0, "done\n");

		return true;
	}

	bool WaveBankWriter::Float32ToPcmSigned16(WaveBank::Wave* pWave)
	{
		if (pWave->m_baseHeader.uFormat != Core::uPCM32BitFloat)
		{
			PrintError("Can only convert signed 32 bit float PCM waves to S16");
			return false;
		}

		pWave->m_baseHeader.uFormat = Core::uPCM16BitSignedInt;

		//if (gSettings.gbVerboseMode)
			Audio::Print(0, "Converting '%s' to signed 16 bit PCM...", pWave->m_baseHeader.pchName);

		WaveBank::PCMWave* pPcmWave = (WaveBank::PCMWave*)pWave->m_pExtraHeader;
		
		S16* pS16Data=0;
		S32 s16Length=0;
		F32ToS16((F32*)pWave->m_pvWaveData, pWave->m_baseHeader.uDataLength, &pS16Data, &s16Length);

		pPcmWave->uBitsPerSample = 16;

		// delete the old s16 data
		delete pWave->m_pvWaveData;

		// point at the newly converted S16 data and set the new size
		pWave->m_pvWaveData = pS16Data;
		pWave->m_baseHeader.uDataLength /= 2;

		//if (gSettings.gbVerboseMode)
			Audio::Print(0, "done\n");

		return true;
	}

	//--------------------------------------------------------------------------------------
	// Name: NeedToEncodeAsLoop()
	// Desc: should this wave be encoded as a loop?
	//		 has looping been specified in the ini file? if so that takes precedence.
	//		 if not specified in the ini file then we look at the wavfile to see if its a loop
	//--------------------------------------------------------------------------------------
	bool WaveBankWriter::NeedToEncodeAsLoop(CWaveFile &wavFile, WaveBankIniFile::Wave* pIniFileWave)
	{
		if (pIniFileWave->Loop() == WaveBankIniFile::E_LOOP_NOT_SPECIFIED)
		{
			if (wavFile.IsLoop())
				return true;
			else
				return false;
		}
		else
		if (pIniFileWave->Loop() == WaveBankIniFile::E_LOOP_YES)
			return true;
		else
			return false;

		return false;
	}
	//--------------------------------------------------------------------------------------
	// Name: WriteIntermediateWav()
	// Desc: write temp wav files after we've processed them
	//--------------------------------------------------------------------------------------
	bool WaveBankWriter::WriteIntermediateWav(Util::C_String strOrigFullFilename, Util::C_String strNewRawFilename, CWaveFile &wavFile)
	{
		Util::C_String strOutputPath = Util::ExtractDirectory(strOrigFullFilename);

		Util::C_String strTmpPath = Util::C_String().Printf("%s_WaveBundlerTemp", strOutputPath.Chars());
		if (Sys::C_FileSystem::Instance().FileExists(strTmpPath)==false)
		{
			if (Sys::C_FileSystem::Instance().CreateSingleDirectory(strTmpPath)==false)
			{
				PrintWarning("Could not create directory '%s' for temporary wavs.\n", strTmpPath.Chars());
				return false;
			}
			#ifdef _DEBUG
			else
				Print(0, "Created temp directory '%s' ok\n", strTmpPath.Chars());
			#endif
		}

		Util::C_String strConvertedFname = strOutputPath + "_WaveBundlerTemp\\" + strNewRawFilename;
		if (wavFile.OpenForWrite(strConvertedFname.Chars()) != S_OK)
		{
			PrintWarning("Could not create temporary file '%s'.\n", strConvertedFname.Chars());
			return false;
		}
		if (wavFile.Write() != S_OK)
		{
			PrintWarning("Could not create temporary file '%s'.\n", strConvertedFname.Chars());
			return false;
		}
		wavFile.Close();
		return true;
	}

	//--------------------------------------------------------------------------------------
	// Name: DeleteTempFiles()
	// Desc: for every wav in the ini file delete tmp files
	//--------------------------------------------------------------------------------------
	void WaveBankWriter::DeleteTempFiles(WaveBankIniFile& iniFile)
	{
		Ut::Vector<Util::C_String> vProcessedList;
		
		for (int i=0; i<iniFile.NumWaves(); i++)
		{
			WaveBankIniFile::Wave *iniWave=iniFile.GetWave(i);

			Util::C_String strTmpPath = Util::C_String().Printf("%s_WaveBundlerTemp", Util::ExtractDirectory(iniWave->Filename()).Chars());
			Util::RemoveTrailingSlash(strTmpPath);
				strTmpPath += '\\';

			// see if we've already processed this directory
			bool bProcessed=false;
			for (int i=0; i<vProcessedList.Size(); i++)
				if (vProcessedList[i] == strTmpPath)
					bProcessed = true;

			if (Sys::C_FileSystem::Instance().FileExists(strTmpPath) && !bProcessed)
			{
				Sys::C_DirectoryInfo dirInfo;
				assert(strTmpPath.Length() > 0);
				
				bool success = Sys::C_FileSystem::Instance().DirectoryInfo( strTmpPath , &dirInfo);
				if(success)
				{
					if (dirInfo.NumFiles())
					{
						int iNumDeleted = 0;
						for(U32 i = 0; i < dirInfo.NumFiles(); i++)
						{
							Sys::C_Fileinfo& fileInfo = dirInfo.FileInfo( i );
							if (Sys::C_FileSystem::Instance().DeleteFile(fileInfo.m_FileName))
								iNumDeleted++;
							else
								PrintWarning("Could not delete temp wav file '%s'\n", fileInfo.m_FileName.Chars());
						}
						#ifdef _DEBUG
						Print(0, "Deleted %d temp wav files from '%s'\n", iNumDeleted, strTmpPath.Chars());
						#endif
					}

					if (RemoveDirectory(strTmpPath.Chars())==false)
						PrintWarning("Could not delete temp wav directory '%s'\n", strTmpPath);
				}

				vProcessedList.PushBack(strTmpPath);
			}
		}

		vProcessedList.Destroy();
	}

	//--------------------------------------------------------------------------------------
	// Name: DoSampleProcessing()
	// Desc: do bitrate conversion, quantizing and sample rate conversion if needed
	// TODO: do bit depth conversion last in the sequence of sample processes so we
	//		 are processing the highest quality sample data.
	//--------------------------------------------------------------------------------------
	bool WaveBankWriter::DoSampleProcessing(const char* pchFilename, CWaveFile &wavFile, WaveBankIniFile::Wave* pIniFileWave, U32 uRequiredFormat, int iSampleIsLoop)
	{
		StopWatch sw;

		// bit rate convert if we need to
		if (pIniFileWave->TargetBitDepth() != -1)
		{
			#ifdef DISPLAY_PROFILE_INFO
			sw.Start();
			#endif
			
			if (wavFile.BitDepthConvert(pIniFileWave->TargetBitDepth()) != S_OK)
				return false;

			#ifdef DISPLAY_PROFILE_INFO
			Audio::Print(TXTCOLOUR_BLUE, "Bit depth converting took %.4f seconds.\n", sw.Time());
			#endif
			
			if (gSettings.gbWriteIntermediateBitDepthFiles)
				WriteIntermediateWav(pchFilename, Util::C_String().Printf("%s_%dBits.wav", GetRawFilename(pchFilename).Chars(), pIniFileWave->TargetBitDepth()), wavFile); 
		}

		S32 sQuantize = -1;
		// do we need to quantize?
		if (gSettings.gbQuantizeEnabled)
		{
			if (iSampleIsLoop == 1)
			{
				DWORD numSamples;
				if (wavFile.GetNumSamples(&numSamples) != S_OK)
				{
					PrintError(0, "Could not get number of samples for looping sample '%s'\n", GetRawFilename((char*)pchFilename).Chars());
					return false;
				}

				if (uRequiredFormat == Core::uXMA216Bit && numSamples % 512 != 0)
				{
					//Print(0, "Looping sample '%s' is not 512 sample aligned ready for XMA2 (contains %d samples).\n", GetRawFilename((char*)pchFilename).Chars(), numSamples);
					sQuantize = 512;
				}

				if (uRequiredFormat == Core::uXMA16Bit && numSamples % 128 != 0)
				{
					//Print(0, "Looping sample '%s' is not 128 sample aligned ready for XMA (contains %d samples). Quantizing...\n", GetRawFilename((char*)pchFilename).Chars(), numSamples);
					sQuantize = 128;
				}

				if (uRequiredFormat == Core::uADPCM4Bit && numSamples % 64 != 0)
				{
					//Print(0, "Looping sample '%s' is not 64 sample aligned (contains %d samples).\n", GetRawFilename((char*)pchFilename).Chars(), numSamples);
					sQuantize = 64;
				}
			}
		}

		// if needs quantizing (resample method) and resampling then do in one operation
		// if needs quantizing (insert delete) and resampling do in 2 operations
		// if just need resampling and no quantize then just do the resample
		// if need quantizing (resample) but not resampling then do quantize by resample
		// if need quantizing (insert delete) but not resampling then do quantize by insert/delete 

		// TODO: if any of these operations did not need to do anytting (ie was already required sample rate then dont write out temp file
		bool bWriteIntermediateFile = false;

		// sample rate convert it if we need to
		if (pIniFileWave->TargetSampleRate() != -1)
		{
			#ifdef DISPLAY_PROFILE_INFO
			sw.Start();
			#endif
			
			if (sQuantize != -1 && pIniFileWave->QuantizeMethod() == WaveBankIniFile::E_QUANTIZE_RESAMPLE)
			{
				// quantize resample combination
				if (wavFile.Quantize(CWaveFile::eQuantizeResample, sQuantize, sQuantize/2, pIniFileWave->TargetSampleRate(), gSettings.giResamplerQuality) != S_OK)
					return false;
				
				if (gSettings.gbWriteIntermediateResampledFiles || gSettings.gbWriteIntermediateQuantizedFiles)
					bWriteIntermediateFile = true;
			}
			else
			{
				// standard resample
				if (wavFile.SampleRateConvert(pIniFileWave->TargetSampleRate(), gSettings.giResamplerQuality) != S_OK)
					return false;

				if (gSettings.gbWriteIntermediateResampledFiles)
					bWriteIntermediateFile = true;
			}

			#ifdef DISPLAY_PROFILE_INFO
			Audio::Print(TXTCOLOUR_BLUE, "Sample rate converting took %.4f seconds.\n", sw.Time());
			#endif
		}
		else
		{
			if (sQuantize != -1 && pIniFileWave->QuantizeMethod() == WaveBankIniFile::E_QUANTIZE_RESAMPLE)
			{
				// quantize by resampling. no change in sample rate
				if (wavFile.Quantize(CWaveFile::eQuantizeResample, sQuantize, sQuantize/2, 0, gSettings.giResamplerQuality) != S_OK)
					return false;

				if (gSettings.gbWriteIntermediateQuantizedFiles)
					bWriteIntermediateFile = true;
			}
		}

		// write intermediate file for resampled, quantized by resample and quantize resample combo
		if (bWriteIntermediateFile)
		{
			Util::C_String strQuant = Util::C_String().Printf("_Quantzd_%d_Resamp", sQuantize);
			Util::C_String strResamp = Util::C_String().Printf("_%dHz", pIniFileWave->TargetSampleRate());
			Util::C_String strConvertedFname = Util::C_String().Printf("%s%s%s.wav", 
				GetRawFilename(pchFilename).Chars(), 
				pIniFileWave->TargetSampleRate() == -1 ? "" : strResamp.Chars(), 
				(sQuantize == -1 || pIniFileWave->QuantizeMethod() == WaveBankIniFile::E_QUANTIZE_INSERT_DELETE) ? "" : strQuant.Chars());
			WriteIntermediateWav(pchFilename, strConvertedFname, wavFile); 
		}

		if (sQuantize != -1 && pIniFileWave->QuantizeMethod() == WaveBankIniFile::E_QUANTIZE_INSERT_DELETE)
		{
			// quantize by insert delete
			if (wavFile.Quantize(CWaveFile::eQuantizeInsertDelete, sQuantize, sQuantize/2) != S_OK)
				return false;

			if (gSettings.gbWriteIntermediateQuantizedFiles)
				WriteIntermediateWav(pchFilename, Util::C_String().Printf("%s_Quantzd_%d_InsDel.wav", GetRawFilename(pchFilename).Chars(), sQuantize), wavFile); 
		}

		return true;
	}
}	// namespace Audio
