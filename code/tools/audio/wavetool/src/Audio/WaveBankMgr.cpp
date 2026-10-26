// sorcery
#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Xt/XtTool.h"
#include "Application/SorceryApp.h"
#include <Xt/Impl/CryptRandom.h>
#include "BuildBankAsync.h"

// audio
#include <iostream>


#include "WaveBankMgr.h"
#include "WaveBankPane.h"
#include "StopWatch.h"
#ifndef WAVEUTILS_H
#include "WaveUtils.h"
#endif

// mercury
#include "System/Misc/StringUtil.h"
#include "system/serialise.h"

// xdk
#include <xbdm.h>

namespace Wb
{

	FuncPrintMessage BankBuilder::_pPrintMsgFn = DefaultPrintMsgFn;	
	FuncUpdateProgress BankBuilder::_pUpdateProgressFn = 0;	
	FuncPrintMessage ProjectSettings::_pPrintMsgFn = 0;	

	const F32 BankBuilder::fBuildStageProgressBarRatio = 0.9f;

	XtString GetDefaultX360();
	Util::C_String FormatTime(F32 fTimeInSecs);

	char* _pchPresetNameLookup [NumPlatforms] = 
	{
		"AudioFormatPreset360",
		"AudioFormatPresetPS3",
		"AudioFormatPresetPC"
	};

	char* _pchFormatLookup [NumWaveFormats] = 
	{
		"PCM",
		"XMA",
		"XMA2",
		"ADPCM"
	};

	char* _pchPlatformLookup [NumPlatforms] = 
	{
		"360",
		"PS3",
		"PC",
	};

	const int kNumResampQualSettings = 5;
	char* _pchResamplerQualityLookup [kNumResampQualSettings] =
	{
		"BEST_QUALITY_AND_SLOWEST",
		"VERY_GOOD",
		"GOOD",
		"NOT_VERY_GOOD",
		"WORST_QUALITY_AND_FASTEST"
	};

	const int kNumQuantizeMethods = 2;
	char* _pchQuantizeMethodLookup [kNumQuantizeMethods] =
	{
		"RESAMPLE",
		"INSERT_OR_DELETE_SAMPLES",
	};


	Util::C_String PresetName(int index) { return (index >= 0 && index <= NumPlatforms) ? Util::C_String(_pchPresetNameLookup[index]) : Util::C_String("UnknownPreset"); }
	Util::C_String FormatName(int index) { return (index >= 0 && index <= NumWaveFormats) ? Util::C_String(_pchFormatLookup[index]) : Util::C_String("UnknownFormat"); }
	Util::C_String PlatformName(int index) { return (index >= 0 && index <= NumPlatforms) ? Util::C_String(_pchPlatformLookup[index]) : Util::C_String("UnknownPlatform"); }
	Util::C_String ResamplerQualityName(int index) { return (index >= 0 && index <= kNumResampQualSettings) ? Util::C_String(_pchResamplerQualityLookup[index]) : Util::C_String("UnknownResamplerQuality"); }
	Util::C_String QuantizeMethodName(int index) { return (index >= 0 && index <= kNumQuantizeMethods) ? Util::C_String(_pchQuantizeMethodLookup[index]) : Util::C_String("UnknownQuantizeMethod"); }

	XtString kXtField_BankPresetId		= "presetId";
	XtString kXtField_WavePresetId		= "presetId";
	XtString kXtField_WaveId			= "id";
	XtString kXtField_WaveName			= "name";
	XtString kXtField_WavePath			= "path";
	XtString kXtField_WaveChannels		= "channels";
	XtString kXtField_WaveSampleRateIn	= "sampleRateIn";
	XtString kXtField_WaveBitDepthIn	= "bitDepthIn";
	XtString kXtField_WaveLoopIn		= "loopIn";
	XtString kXtField_Format			= "format";
	XtString kXtField_WaveSampleRateOut	= "sampleRateOut";
	XtString kXtField_WaveBitDepthOut	= "bitDepthOut";
	XtString kXtField_WaveQuantizeMethod= "quantizeMethod";

	XtString kXtField_SettingPrefix						= "Settings";
	XtString kXtField_SettingOutputPath					= "BanksOutputPath";
	XtString kXtField_SettingDeploymentPath				= "DeploymentPath";
	XtString kXtField_SettingDeploymentPathStreaming	= "DeploymentPathStreaming";
	XtString kXtField_SettingPlatformToBuild			= "PlatformsToBuild";
	XtString kXtField_SettingSamplesRootPath			= "SamplesRootPath";

	Util::C_String strPresetNone = "<None>";

	#ifdef _DEBUG
	ProjectSettings& ps = ProjectSettings::Get();
	BankBuilder& bb = BankBuilder::Get();
	PresetManager& pm = PresetManager::Get();
	#endif


	//-----------------------------------------------------------------------------
	// Name: LookupXXX
	// Desc: convert from string to int/enum
	//-----------------------------------------------------------------------------
	int LookupFormat(Util::C_String& str)
	{
		for (int i=0; i<NumWaveFormats; i++)
			if (str == FormatName(i))
				return i;
		return -1;
	}

	int LookupResamplerQuality(Util::C_String& str)
	{
		for (int i=0; i<kNumResampQualSettings; i++)
			if (str == ResamplerQualityName(i))
				return i;
		return -1;
	}

	int LookupQuantizeMethod(Util::C_String& str)
	{
		for (int i=0; i<kNumQuantizeMethods; i++)
			if (str == QuantizeMethodName(i))
				return i;
		return -1;
	}

	//-----------------------------------------------------------------------------
	// Name: ExtractBankName
	// Desc: strips the folder names from the name to give the bank name
	//-----------------------------------------------------------------------------
	Util::C_String ExtractBankName(Util::C_String str)
	{
		Util::C_String strName;
		int dotPos = str.RevSearch('.');
		if (dotPos < 0)
			strName = str;
		else
			strName = str(dotPos+1, str.Length()) ;
		return strName;
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::AddWave
	// Desc: add a wave to the bank
	//-----------------------------------------------------------------------------
	bool WaveBankMgr::AddWave(Wave* pWave)
	{
		// make unique id if not already done
		if (!pWave->_uniqueId)
			pWave->_uniqueId = MakeUniqueId();

		if (!pWave->_uniqueId)
			return false;

		waves.PushBack(pWave);	

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::GetCommonPath
	// Desc: look at the path of all samples and get the most used path
	//-----------------------------------------------------------------------------
	Util::C_String WaveBankMgr::GetCommonPath()
	{
		struct StrRef
		{
			StrRef() { count = 1; }
			Util::C_String str;
			int count;
		};

		Ut::Vector<StrRef> strs;

		// look through list of strings already found and see if matches
		// if does match increment count
		// if it doesnt match then add to list of strings
		for (int i=0; i<NumWaves(); i++)
		{
			int j;
			for (j=0; j<strs.Size(); j++)
			{
				if (Waves(i)->Path() == strs[j].str)
				{
					strs[j].count++;
					break;
				}
			}
			if (j==strs.Size())
			{
				StrRef ref;
				ref.str = Waves(i)->Path();
				strs.PushBack(ref);
			}
		}

		// now see which string has the highest count
		int maxCount=0;
		Util::C_String& str = Util::C_String("");

		for (int j=0; j<strs.Size(); j++)
		{
			if (strs[j].count > maxCount)
			{
				maxCount = strs[j].count;
				str = strs[j].str;
			}
		}

		return str;
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::MakeUniqueId
	// Desc: 
	//-----------------------------------------------------------------------------
	XtId WaveBankMgr::MakeUniqueId()
	{
		CryptRandom cr;

		if (!cr.Initialise())
			return 0;

		for (int i=0; i<32; i++)
		{
			XtId id = (XtId)cr();

			if (!id)
				continue;

			if (id < 32768)
				continue;

			// check it is unique
			for (int j=0; j<NumWaves(); j++)
			{
				if (Waves(j)->Id() == id)
					id = 0;
			}

			if (id == 0)
				continue;

			return id;
		}

		return 0;
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::Lookup
	// Desc: lookup a Wave by name
	//-----------------------------------------------------------------------------
	Wave* WaveBankMgr::Lookup(char* pchName)
	{
		for (int i=0; i < waves.Size(); i++)
		{
			if (waves[i]->Filename() == Util::C_String(pchName))
				return waves[i];
		}

		return NULL;
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::PresetIndex
	// Desc: 
	//-----------------------------------------------------------------------------
	int WaveBankMgr::PresetIndex(int platform)
	{ 
		return presetIndex[platform]; 
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::SetPreset
	// Desc: 
	//-----------------------------------------------------------------------------
	void WaveBankMgr::SetPreset(int index, int platform)
	{ 
		presetIndex[platform] = index;

		if (index == ePreset_None)
			pPreset[platform] = PresetManager::Get().NonePreset(platform);
		else
			pPreset[platform] = &(PresetManager::Get().Presets(platform)[index]);

		for (int i=0; i<waves.Size(); i++)
		{
			waves[i]->SetPreset(index);
		}
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::LookupIndex
	// Desc: get a wave's index from its id
	//-----------------------------------------------------------------------------
	int	WaveBankMgr::LookupIndex(int id)
	{
		for (int i=0; i<waves.Size(); i++)
		{
			if (id == waves[i]->_uniqueId)
			{
				return i;
			}
		}

		return -1;
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::DestroyWaves
	// Desc: clear all waves from list
	//-----------------------------------------------------------------------------
	void WaveBankMgr::DestroyWaves()
	{
		for (int i=0; i<waves.Size(); i++)
			delete waves[i];

		waves.Clear();
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::PopulateFromDom
	// Desc: Fill the WaveBankMgr class in from the db record
	//-----------------------------------------------------------------------------
	bool WaveBankMgr::PopulateFromDom(Xt::DomRecord* record)
	{
		if (record == NULL)
			return false;

		Xt::DomArray* domWaves = Xt::AcquireArray(&record->root.data, "waves");
		
		if (!domWaves)
			return false;

		DestroyWaves();

		for (int p=0; p<NumPlatforms; p++)
			SetPreset(ePreset_None, p);

		name = ExtractBankName(Util::C_String((const char*)record->name));
		dbName = Util::C_String((const char*)record->name);

		// set bank's presets to the default depending on platform
		for (int p=0; p<NumPlatforms; p++)
		{
			if (PresetManager::Get().DefaultPresetIndex(p) != ePreset_None)
				SetPreset(PresetManager::Get().DefaultPresetIndex(p), p);
		}	
		
		//Bank().name = &(*(Xt::AsText(Xt::At(&record->root, XtString("bankName")))));
		bStreaming = strtol(Xt::AsText(Xt::At(&record->root.data, XtString("streaming"))), 0, 10) ? 1 : 0;

		for (int p=0; p<NumPlatforms; p++)
		{
			Xt::DomBlock* platformBlock = (Xt::DomBlock*)Xt::At(&record->root.data, XtString(PlatformName(p).Chars()));
			if (platformBlock)
			{
				// if bank has a preset set use that. otherwise use the "none" preset
				XtId presetId = Xt::ParseId(AsText(At(platformBlock, kXtField_BankPresetId)));
				if (presetId)
					SetPreset(PresetManager::Get().LookupIndex(presetId), p);
				else
					SetPreset(ePreset_None, p);
			}
		}

		FormatPreset* pBankPreset = Preset();

		for(Xt::DomArray::IndexIterator it(domWaves); it; ++it)
		{
			Xt::DomBlock* block = AsBlock(it);

			if (!block)
				continue;

			// add to our wave list
			Wave* pWave = CreateWave();
			if (!pWave)
				continue;

			pWave->_path = (const char*)Xt::AsText(At(block, kXtField_WavePath));
			pWave->_filename = (const char*)Xt::AsText(At(block, kXtField_WaveName));

			pWave->_nChannels = strtol(Xt::AsText(At(block, kXtField_WaveChannels)), 0, 10);
			pWave->_sampleRateIn = strtol(Xt::AsText(At(block, kXtField_WaveSampleRateIn)), 0, 10);
			pWave->_bitDepthIn = strtol(Xt::AsText(At(block, kXtField_WaveBitDepthIn)), 0, 10);
			pWave->_bLoopIn = strtol(Xt::AsText(At(block, kXtField_WaveLoopIn)), 0, 10) ? true : false;
			pWave->_uniqueId = (int)(LPARAM)Xt::ParseId(AsText(At(block, kXtField_WaveId)));

			XtString xtsQuantMethod = Xt::AsText(At(block, kXtField_WaveQuantizeMethod));
			if (xtsQuantMethod != "DEFAULT")
				pWave->SetQuantizeMethod((eQuantizeMethod)LookupQuantizeMethod(Util::C_String((const char*)xtsQuantMethod)));

			// see if the wave has an optional platform block
			for (int p=0; p<NumPlatforms; p++)
			{
				Xt::DomBlock* platformBlock = (Xt::DomBlock*)Xt::At(block, XtString(PlatformName(p).Chars()));
				if (platformBlock)
				{
					if (At(platformBlock, kXtField_BankPresetId))
					{
						// if wave's platform block has a preset id then use that
						if (AsText(At(platformBlock, kXtField_BankPresetId)) == "None")
						{
							pWave->SetPreset(ePreset_None, p);

							// get custom format here
							pWave->Preset(p)->SetFormat((eWaveFormat)LookupFormat(Util::C_String((const char*)AsText(At(platformBlock, kXtField_Format)))));
							pWave->Preset(p)->SetBitDepth(strtol(AsText(At(platformBlock, kXtField_WaveBitDepthOut)), 0, 10));
							pWave->Preset(p)->SetSampleRate(strtol(AsText(At(platformBlock, kXtField_WaveSampleRateOut)), 0, 10));

							// TODO: xma properties
						}
						else
						{
							XtId presetId = Xt::ParseId(AsText(At(platformBlock, kXtField_BankPresetId)));
							if (presetId)
							{
								pWave->SetPreset(PresetManager::Get().LookupIndex(presetId), p);
							}
						}
					}
				}
			}

			AddWave(pWave);
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::WriteToDom
	// Desc: 
	//-----------------------------------------------------------------------------
	bool WaveBankMgr::WriteToDom(Xt::DomRecord* record)
	{
		if (!record)
			return false;

		Util::C_String str;

		Xt::DomArray* arr = Xt::AcquireArray(&record->root.data, "waves");;
		if (!arr)
			return false;
		Xt::Clear(arr);

		str.Printf("%d", IsStreaming() ? 1 : 0);
		Xt::Set(&record->root.data, "streaming", XtString(str.Chars()));

		for (int p=0; p<NumPlatforms; p++)
		{
			// create block for each platform
			Xt::DomBlock* platformBlock = new Xt::DomBlock;
			
			Xt::Set(&record->root.data, XtString(PlatformName(p).Chars()), platformBlock);

			if (PresetIndex(p) == ePreset_None)
			{
				Xt::Set(platformBlock, kXtField_BankPresetId, XtString("0"));
			}
			else
			{
				XtString strPresetId = Format("%08X", PresetManager::Get().Preset(PresetIndex(p))->GetXtId());

				Xt::Set(platformBlock, kXtField_BankPresetId, strPresetId);
			}
		}

		for(int i=0; i<NumWaves(); i++)
		{
			Xt::DomBlock* mySubBlock = Xt::Append(arr, new Xt::DomBlock);
			XtString strId = Format("%08X", Waves(i)->Id());

			{
				Wave* pWave = Waves(i);
				if (!pWave)
					continue;

				Xt::Set(mySubBlock, kXtField_WaveId, strId);
				Xt::Set(mySubBlock, kXtField_WaveName, XtString(pWave->Filename()));
				Xt::Set(mySubBlock, kXtField_WavePath, XtString(pWave->Path()));

				if (pWave->BitDepth() != 0xffffffff)
				{
					str.Printf("%d", pWave->BitDepth());
					Xt::Set(mySubBlock, kXtField_WaveBitDepthIn, XtString(str.Chars()));
				}

				str.Printf("%d", pWave->Loop());
				Xt::Set(mySubBlock, kXtField_WaveLoopIn, XtString(str.Chars()));

				if (pWave->Channels() != 0xffffffff)
				{
					str.Printf("%d", pWave->Channels());
					Xt::Set(mySubBlock, kXtField_WaveChannels, XtString(str.Chars()));
				}

				if (pWave->SampleRate() != 0xffffffff)
				{
					str.Printf("%d", pWave->SampleRate());
					Xt::Set(mySubBlock, kXtField_WaveSampleRateIn, XtString(str.Chars()));
				}

				if (pWave->QuantizeMethod() == eQuantizeDefault)
					str = "DEFAULT";
				else
					str = QuantizeMethodName(pWave->QuantizeMethod());
				Xt::Set(mySubBlock, kXtField_WaveQuantizeMethod, XtString(str.Chars()));

				// create block for each platform (if needed)
				for (int p=0; p<NumPlatforms; p++)
				{
					if (pWave->PresetIndex(p) != ePreset_InheritFromBank)
					{
						Xt::DomBlock* platformBlock = new Xt::DomBlock;

						// write the custom format if preset is to to "none"
						if (pWave->PresetIndex(p) == ePreset_None)
						{
							Xt::Set(platformBlock, kXtField_WavePresetId, XtString("None"));

							str.Printf("%d", pWave->Preset(p)->BitDepth());
							Xt::Set(platformBlock, kXtField_WaveBitDepthOut, XtString(str.Chars()));

							str.Printf("%s", FormatName(pWave->Preset(p)->Format()).Chars());
							Xt::Set(platformBlock, kXtField_Format, XtString(str.Chars()));

							str.Printf("%d", pWave->Preset(p)->SampleRate());
							Xt::Set(platformBlock, kXtField_WaveSampleRateOut, XtString(str.Chars()));

							// TODO: xma properties
						}
						else
						{
							XtString strPresetId = Format("%08X", PresetManager::Get().Preset(pWave->PresetIndex(p))->GetXtId(), p);
							Xt::Set(platformBlock, kXtField_WavePresetId, strPresetId);
						}
						
						Xt::Set(mySubBlock, XtString(PlatformName(p).Chars()), platformBlock);
					}
				}
			}
		}
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::RescanWavFiles
	// Desc: 
	//-----------------------------------------------------------------------------
	void WaveBankMgr::RescanWavFiles()
	{
		Util::C_String strMessage;
		bool bWereErrors = false;
		bool bAbort = false;

		for (int i=0; i<NumWaves(); i++)
		{
			WaveFileInfo info;

			info.strFilename = Waves(i)->Path() + Waves(i)->Filename();
			
			eQueryWaveFileResult eQueryResult = eQuerySuccess;
			eQueryResult = QueryWaveFile(info);

			if (eQueryResult == eQuerySuccess)
			{
				if (info.IsValid())
					Waves(i)->_eFileStatus = Wave::eFoundGotInfo;
				else
					Waves(i)->_eFileStatus = Wave::eFoundNotGotInfo;
			}
			else
			{
				bWereErrors = true;

				if (info.bWavReadError == 1)
					Waves(i)->_eFileStatus = Wave::eNotFoundOnDisk;

				if (eQueryResult == eOldResultFailed || eQueryResult == eOldResultSuccess || eQueryResult == eFailedDllLoad || eQueryResult == eFailedFunctionLookup)
				{
					static bool bShownAlready = false;
					if (bShownAlready)
						return;

					strMessage +=  Util::C_String().Printf("Could not query wav files. Dll call failed. Reason: %s\n", LookupQueryWavResultString(eQueryResult));
					bAbort = true;
					bShownAlready = true;
				}
				else
				{
					strMessage +=  Util::C_String().Printf("Could not get wav file info for %s... Reason: %s\n", info.strFilename.Chars(), LookupQueryWavResultString(eQueryResult));
				}

				if (bAbort)
					break;
			}
			
			Waves(i)->SetBitDepth(info.bitDepth);
			Waves(i)->SetChannels(info.nChannels);
			Waves(i)->SetSampleRate(info.sampleRate);
		}

		if (bWereErrors)
			MessageBox(NULL, _T(strMessage.Chars()), _T("Hey"), MB_OK);
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::FoundAllWavFiles
	// Desc: 
	//-----------------------------------------------------------------------------
	bool WaveBankMgr::FoundAllWavFiles()
	{
		for (int i=0; i<NumWaves(); i++)
		{
			if (Waves(i)->_eFileStatus == Wave::eNotFoundOnDisk)
				return false;
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: WaveBankMgr::operator =
	// Desc: makes a copy of any dynamically allocated data
	//-----------------------------------------------------------------------------
	WaveBankMgr& WaveBankMgr::operator = (const WaveBankMgr& rhs)
	{
		if (this == &rhs)
			return *this;

		this->bStreaming = rhs.bStreaming;
		this->dbName = rhs.dbName;
		this->name = rhs.name;
		this->pPreset = rhs.pPreset;
		this->presetIndex = rhs.presetIndex;

		// make a copy of the Waves
		for (int i=0; i<rhs.NumWaves(); i++)
		{
			Wave* pWave = this->CreateWave();
			if (pWave)
			{
				*pWave = *rhs.Waves(i);
				pWave->_pBank = this;
				this->AddWave(pWave);
			}
		}

		return *this;
	} 
	//-----------------------------------------------------------------------------
	// Name: Wave::SetPreset
	// Desc: 
	//-----------------------------------------------------------------------------
	void Wave::SetPreset(int preset, int platform)
	{
		_presetIndex[platform] = preset;
	}

	//-----------------------------------------------------------------------------
	// Name: Wave::Preset
	// Desc: get the preset for this wave
	//-----------------------------------------------------------------------------
	FormatPreset* Wave::Preset(int platform)
	{ 
		int index = PresetIndex(platform);

		if (index >= 0)
		{
			if (index < PresetManager::Get().Presets(platform).Size())
				return PresetManager::Get().Preset(index);
		}
		else
		{
			if (index == ePreset_InheritFromBank)
				return _pBank->Preset(platform); 
			else
			if (index == ePreset_None)
				return &_customPreset[platform];
		}

		// should never get here but prefer returning custom preset to NULL pointer
		return &_customPreset[platform];
	}

	//-----------------------------------------------------------------------------
	// Name: Wave::Wave
	// Desc: 
	//-----------------------------------------------------------------------------
	Wave::Wave(WaveBankMgr* bank)
	{
		_pBank = bank;
		_bLoopIn = false;
		_nChannels = -1;
		_bitDepthIn = -1;
		_sampleRateIn = -1;
		_uniqueId = 0;
		_eQuantizeMethod = eQuantizeDefault; //ProjectSettings::Get().DefaultQuantizeMethod();
		_eFileStatus = eNotLooked;

		for (int p=0; p<NumPlatforms; p++)
		{
			if (PresetManager::Get().DefaultPresetIndex(p) == ePreset_None)
				SetPreset(ePreset_None, p);
			else
				SetPreset(ePreset_InheritFromBank, p);

	//		pPreset[p] = NULL;
			_customPreset[p] = *(PresetManager::Get().NonePreset(p));
		}
	}

	//-----------------------------------------------------------------------------
	// Name: Preset::GetRecord
	// Desc: get the dom record for a format preset
	//-----------------------------------------------------------------------------
	Xt::DomRecord* FormatPreset::GetRecord(Xt::DomDb* db)
	{
		return FindRecordById(db, GetXtId());
	}

	//-----------------------------------------------------------------------------
	// Name: PresetManager::PresetManager()
	// Desc: see if we need to add this item to the list of presets 
	//-----------------------------------------------------------------------------
	PresetManager::PresetManager()
	{
		for (int p=0; p<NumPlatforms; p++)
			SetDefaultPresetIndex(p, ePreset_None);

		nonePreset[ePlatform360].name = strPresetNone;
		nonePreset[ePlatform360].eFormat = eWaveFormatXMA;
		nonePreset[ePlatform360].sampleRate = 48000;
		nonePreset[ePlatform360].bitDepth = 16;
		nonePreset[ePlatform360].xmaQuality = 70;
		nonePreset[ePlatform360].xmaHighFreqFilter = false;
		nonePreset[ePlatform360].xtId = 0;

		nonePreset[ePlatformPS3].name = strPresetNone;
		nonePreset[ePlatformPS3].eFormat = eWaveFormatADPCM;
		nonePreset[ePlatformPS3].sampleRate = 48000;
		nonePreset[ePlatformPS3].bitDepth = 16;
		nonePreset[ePlatformPS3].xmaQuality = -1;
		nonePreset[ePlatformPS3].xmaHighFreqFilter = false;
		nonePreset[ePlatformPS3].xtId = 0;

		nonePreset[ePlatformPC].name = strPresetNone;
		nonePreset[ePlatformPC].eFormat = eWaveFormatPCM;
		nonePreset[ePlatformPC].sampleRate = 48000;
		nonePreset[ePlatformPC].bitDepth = 16;
		nonePreset[ePlatformPC].xmaQuality = -1;
		nonePreset[ePlatformPC].xmaHighFreqFilter = false;
		nonePreset[ePlatformPC].xtId = 0;
	}

	//-----------------------------------------------------------------------------
	// Name: PresetManager::AddPreset()
	// Desc: see if we need to add this item to the list of presets 
	//-----------------------------------------------------------------------------
	void PresetManager::AddPreset(Xt::DomDb* db, Xt::DomRecord* record, int platform)
	{
		LarvaSchema*	schema		= db->schema;
		LarvaType*		recordType	= TypeOf(schema, record);

		if (!recordType || (recordType->kind != kLtStruct))
			return;

		if (strcmp((const char*)record->root.type, PresetName(platform).Chars()))
			return;
		
		FormatPreset preset;

		// get fields
		const char* pchLastDot = strrchr((const char*)record->name, '.');
		if (!pchLastDot)
			return;
		preset.name = Util::C_String(pchLastDot + 1);
		
		Util::C_String str = (const char*)Xt::AsText(Xt::At(&record->root.data, XtString("Format")));

		int f;
		for (f=0; f<NumWaveFormats; f++)
			if (str == Util::C_String(FormatName(f).Chars()))
				break;

		if (f == NumWaveFormats)
			return;

		preset.xtId = record->id;

		preset.eFormat = (eWaveFormat)f;

		str = (const char*) Xt::AsText(Xt::At(&record->root.data, XtString("BitDepth")));
		preset.bitDepth = strtol(str.Chars(), 0, 10);
		
		str = (const char*) Xt::AsText(Xt::At(&record->root.data, XtString("SampleRate")));
		preset.sampleRate = strtol(str.Chars(), 0, 10);

		// TODO: XMA fields

		str = (const char*) Xt::AsText(Xt::At(&record->root.data, XtString("XMACompressionQuality")));
		if (str != "")
			preset.xmaQuality = strtol(str.Chars(), 0, 10);

		str = (const char*) Xt::AsText(Xt::At(&record->root.data, XtString("XMAHighFrequencyFilter")));
		if (str != "")
			preset.xmaHighFreqFilter = strtol(str.Chars(), 0, 10) ? true : false;

		Presets(platform).PushBack(preset);
	}

	//-----------------------------------------------------------------------------
	// Name: FnSortPresets()
	// Desc: used by quicksort to sort the presets list
	//-----------------------------------------------------------------------------
	int FnSortPresets(const void* p1 , const void* p2)
	{
		return strcmp(((FormatPreset*)p1)->Name(), ((FormatPreset*)p2)->Name());
	}

	//-----------------------------------------------------------------------------
	// Name: PresetManager::Populate()
	// Desc: see if we need to add this item to the list of presets 
	//-----------------------------------------------------------------------------
	void PresetManager::PopulateFromDom(Xt::DomDb* db)
	{
		for (int p=0; p<NumPlatforms; p++)
		{
			Presets(p).Clear();

			SetDefaultPresetIndex(p, ePreset_None);

			for(Xt::DomDb::Iterator itPack(db); itPack; ++itPack)
			{
				for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
				{
					AddPreset(db, itRecord, p);
				}
			}

			bool bFoundDefault = false;

			// swap "Default to the top"
			for (int i=0; i<Presets(p).Size(); i++)
			{
				if (!_stricmp(Presets(p)[i].name, "Default"))
				{
					// swap Default entry with first entry in the list
					if (i != 0)
					{
						FormatPreset temp = Presets(p)[0];
						Presets(p)[0] = Presets(p)[i];
						Presets(p)[i] = temp;
					}
					bFoundDefault = true;
					break;
				}
			}

			int sortStart = bFoundDefault ? 1 : 0;
			int numToSort = bFoundDefault ? Presets(p).Size()-1 : Presets(p).Size();

			if (numToSort > 1)
				qsort((void*)&Presets(p)[sortStart], numToSort, sizeof(FormatPreset), FnSortPresets); 

			if (bFoundDefault)
				SetDefaultPresetIndex(p, 0);
			else
				SetDefaultPresetIndex(p, ePreset_None);
		}
	}

	//-----------------------------------------------------------------------------
	// Name: PresetManager::LookupIndex
	// Desc: get the index of a preset from its id
	//-----------------------------------------------------------------------------
	int PresetManager::LookupIndex(XtId presetId, int platform)
	{
		for (int p=0; p<Presets(platform).Size(); p++)
		{
			if (presetId == Presets()[p].xtId)
			{
				return p;
			}
		}

		return -1;
	}

	void BankBuilder::DefaultPrintMsgFn(int msgType, Util::C_String& str, void* pContext)
	{ 
		OutputDebugString(str.Chars()); 
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::CreateXml
	// Desc: create the xml in memory to pass
	//-----------------------------------------------------------------------------
	bool BankBuilder::CreateXml(WaveBankMgr* pBank, ePlatform ePlatform, Util::C_String& str)
	{
		str += Util::C_String("<WaveBank name=\"") + pBank->Name() + "\" ";

		switch (ePlatform)
		{
			case ePlatform360:
			case ePlatformPS3:
				str += "endian=\"big\" ";
				break;
			case ePlatformPC:
				str += "endian=\"little\" ";
				break;
		}

		// TODO: pitchshift up and down for the bank/waves

		str += pBank->IsStreaming() ? "streaming=\"yes\" " : "streaming=\"no\" ";
		str += "bitDepth=\"16\">\n";
		

		for (int i=0; i<pBank->NumWaves(); i++)
		{		
			Wave* pWave = pBank->Waves(i);
			str += "\t<wave\n";
			str += Util::C_String("\t\tfilename = \"") + pWave->Path() + pWave->Filename() + "\"\n";
			str += Util::C_String("\t\tinternalName = \"") + pWave->Name() + "\"\n";

			str += "\t\tformat = ";
			switch (pWave->Preset(ePlatform)->Format())
			{
				case eWaveFormatPCM:
					str += "\"pcm_wav_s16\"\n";
				break;
				case eWaveFormatXMA:
					str += "\"pcm_wav_s16_to_xma\"\n";
					str += Util::C_String("\t\tquality = ") + Util::C_String().Printf("\"%d\"\n", pWave->Preset(ePlatform)->XmaQuality());
				break;
				case eWaveFormatXMA2:
					str += "\"pcm_wav_s16_to_xma2\"\n";
					str += Util::C_String("\t\tquality = ") + Util::C_String().Printf("\"%d\"\n", pWave->Preset(ePlatform)->XmaQuality());
				break;
				case eWaveFormatADPCM:
					str += "\"pcm_wav_s16_to_adpcm\"\n";
				break;
				default:
				break;
			}
			
			str += Util::C_String().Printf("\t\tsamplerate = \"%d\"\n", pWave->Preset(ePlatform)->SampleRate());

			str += Util::C_String().Printf("\t\tloop = \"%s\"", pWave->Loop() ? "yes" : "no");

			if (pWave->Loop() && ProjectSettings::Get().QuantizeEnabled())
			{
				eQuantizeMethod eQuantize;
				if (pWave->QuantizeMethod() == eQuantizeDefault)
					eQuantize = ProjectSettings::Get().DefaultQuantizeMethod();
				else
					eQuantize = pWave->QuantizeMethod();
				str += Util::C_String().Printf("\n\t\tquantizemethod = \"%s\"", eQuantize == eQuantizeInsertDelete ? "insertdelete" : "resample");
			}
			str += "/>\n";
		}

		str += "</WaveBank>";

		// output to debug stream
		//OutputDebugString(str.Chars());

		// output xml file 
		if (ProjectSettings::Get().XmlWritingEnabled())
		{
			FILE* file;
			fopen_s(&file, Util::C_String(PlatformSettings(ePlatform).OutputPath() + pBank->Name() + ".xml").Chars(), "wb");
			if (file)
			{
				fwrite(str.Chars(), sizeof(char), str.Length(), file);
				fclose(file);
			}
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: PresetManager::AddBank()
	// Desc: see if we need to add this banks to the list of banks to be built
	//-----------------------------------------------------------------------------
	void BankBuilder::AddBank(Xt::DomDb* db, Xt::DomRecord* record, Util::C_String& strFilter)
	{
		LarvaSchema*	schema		= db->schema;
		LarvaType*		recordType	= TypeOf(schema, record);

		if (!recordType || (recordType->kind != kLtStruct))
			return;

		if (strcmp((const char*)record->root.type, "WaveBank"))
			return;

		// has a subset filter been specified?
		if (strFilter != "")
		{
			// see if record name starts with the substring we are interested in
			if (Util::C_String((const char*)record->name).Search(strFilter) != 0)
				return;
		}

		// lets create a new bank and add it to the list
		WaveBankMgr* pBank = new WaveBankMgr;
		if (!pBank)
			return;

		if (pBank->PopulateFromDom(record) == false)
		{
			delete pBank;
			return;
		}

		AddToBankList(pBank);
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::CheckForDuplicateBankNames
	// Desc: see if there are banks with duplicate names in the list of banks
	//-----------------------------------------------------------------------------
	bool BankBuilder::CheckForDuplicateBankNames()
	{ 
		for (int i=0; i<_banksToBuild.Size(); i++)
		{
			for (int j=0; j<_banksToBuild.Size(); j++)
			{
				if (i!=j && _banksToBuild[i]->Name() == _banksToBuild[j]->Name())
				{
					PrintMessage(eMsgInfo, (Util::C_String)Util::C_String().Printf("Found duplicate banks named '%s'", _banksToBuild[i]->Name().Chars()));
					PrintMessage(eMsgInfo, (Util::C_String)Util::C_String().Printf("[Banks are '%s' and '%s']", _banksToBuild[i]->FullDbName().Chars(), _banksToBuild[j]->FullDbName().Chars()));
					return true;
				}
			}
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::Build
	// Desc: build a baf file for a single platform, given a WaveBankMgr struct
	//-----------------------------------------------------------------------------
	bool BankBuilder::Build(WaveBankMgr* pBank, ePlatform platform)
	{
		if (NumBanksInList()==0)
			_numBanksBuilt = 0;

		if (!pBank)
			return false;

		PrintMessage(eMsgInfo, (Util::C_String)Util::C_String().Printf("Building bank '%s'...", pBank->Name().Chars()));

		// iterate through bank and create xml
		Util::C_String xmlText;
		CreateXml(pBank, platform, xmlText);

		// setup progress update values
		F32 fSingleBankFraction = NumBanksInList() ? 1.0f / TotalNumBanksToBuild() : 1.0f;
		fSingleBankFraction *= ProgressBarBuildStageRatio();
		F32 fProgressMin = NumBanksBuilt() * fSingleBankFraction;
		F32 fProgressMax = fProgressMin + fSingleBankFraction;

		BuildBankParams params;
		params.pStrXmlText = &xmlText;
		params.strOutputDir = PlatformSettings(platform).OutputPath();
		params.fProgressMin = fProgressMin;
		params.fProgressMax = fProgressMax;
		params.bQuantize = ProjectSettings::Get().QuantizeEnabled();
		params.eQuantizeMethod = ProjectSettings::Get().DefaultQuantizeMethod();
		params.iResamplerQuality = ProjectSettings::Get().ResamplerQuality();
		params.bWriteBitDepthFiles = ProjectSettings::Get().BitDepthWavWritingEnabled();
		params.bWriteQuantizedFiles = ProjectSettings::Get().QuantizedWavWritingEnabled();
		params.bWriteResampledFiles = ProjectSettings::Get().ResampledWavWritingEnabled();
		params.bWriteEncodedFiles = ProjectSettings::Get().EncodedWavWritingEnabled();
		params.bDeleteWavFiles = ProjectSettings::Get().WavDeletingEnabled();

		// pass xml to wavebundler
		bool bOk = BuildBankFromXmlInMemory(params);

		if (NumBanksInList()==0)
		{
			if (bOk)
				_numBanksBuilt = 1;
			else
				_numBanksBuilt = 0;
		}

		return bOk;
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::Build
	// Desc: build single bank for all platforms
	//-----------------------------------------------------------------------------
	bool BankBuilder::Build(WaveBankMgr& bank)
	{
		ClearBankList();

		// make copy of bank. otherwise the bank being passed in will get deleted when the list is cleared
		WaveBankMgr* pBankToAdd = new WaveBankMgr;
		if (!pBankToAdd)
			return false;
		*pBankToAdd = bank;

		AddToBankList(pBankToAdd);
		
		return Build();
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::BuildAll
	// Desc: go through all WaveBankMgrs in the db and build them for all platforms enabled 
	//-----------------------------------------------------------------------------
	bool BankBuilder::BuildAll(Xt::DomDb* db, Util::C_String& strFilter)
	{ 
		ClearBankList();

		// build list of banks to build from db
		for(Xt::DomDb::Iterator itPack(db); itPack; ++itPack)
		{
			for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
				AddBank(db, itRecord, strFilter);
		}

		return Build();
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::Build
	// Desc: build banks in the list for all enabled platforms
	//-----------------------------------------------------------------------------
	bool BankBuilder::Build()
	{ 
		StopWatch sw;

		_numBanksBuilt = 0;
		_curStage = eStageBuild;

		if (CheckForDuplicateBankNames())
		{
			_curStage = eStageIdle;
			return false;
		}

		for (int p=0; p <NumPlatforms; p++)
		{
			if (PlatformSettings(p).BuildEnabled())
			{
				PrintMessage(eMsgInfo, Util::C_String("——————————————————————————————————————————————————"));
				PrintMessage(eMsgNone, (Util::C_String)Util::C_String().Printf("                                                 [Building %d bank%s for platform %s]", NumBanksInList(), NumBanksInList()>1?"s":"", PlatformName(p).Chars()));
				PrintMessage(eMsgInfo, Util::C_String("——————————————————————————————————————————————————"));

				for (int i=0; i<_banksToBuild.Size(); i++)
				{
					SetProgress(((F32)(p * NumBanksInList()) + (F32)i) / (F32)TotalNumBanksToBuild());

					if (Build(_banksToBuild[i], (ePlatform)p) == false)
					{
						_curStage = eStageIdle;
						return false;
					}
					_numBanksBuilt++;
				}
			}
		}

		PrintMessage(eMsgInfo, (Util::C_String)Util::C_String().Printf("Built %d bank%s successfully in %s", NumBanksBuilt(), NumBanksBuilt()>1?"s":"", FormatTime(sw.Time()).Chars()));

		Deploy();

		ClearBankList();

		_curStage = eStageIdle;

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::ClearBankList
	// Desc: 
	//-----------------------------------------------------------------------------
	void BankBuilder::ClearBankList()
	{ 
		// go through all WaveBankMgrs in the db and build them
		for (int i=0; i<_banksToBuild.Size(); i++)
			delete _banksToBuild[i];
		_banksToBuild.Clear();
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::NumPlatformsToBuild
	// Desc: 
	//-----------------------------------------------------------------------------
	int BankBuilder::NumPlatformsToBuild()
	{ 
		int count=0;
		for (int p=0; p <NumPlatforms; p++)
		{
			if (PlatformSettings(p).BuildEnabled())
				count++;
		}
		return count;
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::NumPlatformsToDeploy
	// Desc: 
	//-----------------------------------------------------------------------------
	int BankBuilder::NumPlatformsToDeploy()
	{ 
		int count=0;
		for (int p=0; p <NumPlatforms; p++)
		{
			if (PlatformSettings(p).BuildEnabled() && PlatformSettings(p).NeedToCopyToKit())
				count++;
		}
		return count;
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::BankBuilder
	// Desc: 
	//-----------------------------------------------------------------------------
	BankBuilder::BankBuilder()
	{
		_printMsgContext = NULL;
		SetMessagePrintCallback(DefaultPrintMsgFn);

		_numBanksBuilt = 0;
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::SetProgress
	// Desc: Set the progress bar's status
	//-----------------------------------------------------------------------------
	void BankBuilder::SetProgress(F32 fProgress) 
	{
		F32 fRatio = ProgressBarBuildStageRatio();
		if (_curStage == eStageBuild)
			fProgress *= fRatio;
		else
		if (_curStage == eStageDeploy)
			fProgress = fRatio + (1.f-fRatio)*fProgress;

		if (_pUpdateProgressFn) 
			_pUpdateProgressFn(fProgress, _updateProgressContext); 
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::Deploy
	// Desc: copies banks to platform's game image. eg 360 copy to kit, ps3 to 
	//		 local game image
	//-----------------------------------------------------------------------------
	bool BankBuilder::Deploy()
	{ 
		if (!NumPlatformsToDeploy())
			return true;

		StopWatch sw;

		_curStage = eStageDeploy;

		int filesCopied = 0;

		for (int p=0; p <NumPlatforms; p++)
		{
			if (PlatformSettings(p).BuildEnabled() && PlatformSettings(p).NeedToCopyToKit())
			{
				if (p == ePlatform360)
					PrintMessage(eMsgInfo, (Util::C_String)Util::C_String().Printf("[Platform %s] Copying %d file%s in '%s' to kit '%s'...", PlatformName(p).Chars(), NumBanksInList(), NumBanksInList()>1?"s":"", PlatformSettings(p).OutputPath().Chars(), Util::C_String((const char*)GetDefaultX360()).Chars()));
				else
					PrintMessage(eMsgInfo, (Util::C_String)Util::C_String().Printf("[Platform %s] Copying %d file%s in '%s' to game image...", PlatformName(p).Chars(), NumBanksInList(), NumBanksInList()>1?"s":"", PlatformSettings(p).OutputPath().Chars()));

				for (int f=0; f<NumBanksInList(); f++)
				{
					bool bSuccess = true;
					SetProgress((((F32)p*(F32)NumBanksInList()+f) / (F32)TotalNumBanksToDeploy()));

					WaveBankMgr* pBank = _banksToBuild[f];
					if (!pBank)
						continue;

					switch (p)
					{
						case ePlatform360:
						{
							Util::C_String strSrc = PlatformSettings(p).OutputPath() + pBank->FileName();
							Util::C_String strDeployPath = PlatformSettings(p).DeploymentPath();
							if (pBank->IsStreaming() && PlatformSettings(p).StreamingDeploymentPath() != "")
								strDeployPath = PlatformSettings(p).StreamingDeploymentPath();
							Util::C_String strDst = Util::C_String("devkit:\\") + strDeployPath + _banksToBuild[f]->FileName();
							
							PrintMessage(eMsgNone, (Util::C_String)Util::C_String().Printf("Copying '%s' to '%s'...", /*strSrc.Chars()*/ _banksToBuild[f]->FileName().Chars(), strDst.Chars()));

							if (FAILED(DmSendFile(strSrc.Chars(), strDst.Chars())))
							{
								CreateDirectoriesX360(XtString(strDst.Chars()));

								if (FAILED(DmSendFile(strSrc.Chars(), strDst.Chars())))
								{
									PrintMessage(eMsgError, Util::C_String("Copy failed"));
									bSuccess = false;
									break;
								}
							}		
						}
						break;
						case ePlatformPC:
						case ePlatformPS3:
						{
							Util::C_String strSrc = PlatformSettings(p).OutputPath() + _banksToBuild[f]->FileName();
							Util::C_String strDeployPath = PlatformSettings(p).DeploymentPath();
							if (pBank->IsStreaming() && PlatformSettings(p).StreamingDeploymentPath() != "")
								strDeployPath = PlatformSettings(p).StreamingDeploymentPath();
							Util::C_String strDst = strDeployPath + _banksToBuild[f]->FileName();
							
							PrintMessage(eMsgNone, (Util::C_String)Util::C_String().Printf("Copying '%s' to '%s'...", /*strSrc.Chars()*/ _banksToBuild[f]->FileName().Chars(), strDst.Chars()));

							if (CopyFile(strSrc, strDst, FALSE) == FALSE)	
							{
								CreateDirectories(XtString(strDst.Chars()));

								if (CopyFile(strSrc, strDst, FALSE) == FALSE)	
								{
									PrintMessage(eMsgError, Util::C_String("Copy failed"));
									bSuccess = false;
									break;
								}
							}		
						}
						break;

					}
					
					if (bSuccess == true)
						filesCopied++;
				}
			}
		}

		PrintMessage(eMsgInfo, (Util::C_String)Util::C_String().Printf("Copied %d file%s for %d platform%s in %s", filesCopied, filesCopied>1?"s":"", NumPlatformsToDeploy(), NumPlatformsToDeploy()>1?"s":"", FormatTime(sw.Time()).Chars())); 

		return false;
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::CreateDirectoriesX360()
	// Desc: this may want to be a standalone function one day?
	//-----------------------------------------------------------------------------
	bool BankBuilder::CreateDirectoriesX360(const XtString& path)
	{
		const char* start = path;
		const char* drive = start;

		for(const char* p = path; *p; p++)
		{
			if (*p == ':')
			{
				drive = p + 1;
				break;
			}
		}

		do
		{
			while((*drive == '\\') || (*drive == '/'))
				drive++;

			while((*drive != '\\') && (*drive != '/') && (*drive != '\0'))
				drive++;

			if (*drive == '\0')
				break;

			XtString subPath(start, (int)(drive - start));

			HRESULT result = DmMkdir(subPath);

			if (SUCCEEDED(result))
			{
				PrintMessage(eMsgNone, (Util::C_String)Util::C_String().Printf("Created directory on x360: %s", (const char*)subPath));
			}
			else
			{
				if (result != XBDM_ALREADYEXISTS)
				{
					XtString xbox = GetDefaultX360();

					PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Failed to create directory on x360: %s", (const char*)subPath));
					PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Devkit: %s", (const char*)xbox));

					return false;
				}
			}
		}
		while(*drive != '\0');

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: GetWindowsErrorMessage()
	// Desc: 
	//-----------------------------------------------------------------------------
	XtString GetWindowsErrorMessage(int errorCode)
	{
		char* message;

		XtString result;

		if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				0, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message, 0, 0
			) >= 0)
		{
			for(char* ptr = message; *ptr != '\0'; ptr++)
			{
				if (!isprint(*ptr))
					*ptr = ' ';
			}

			result = Format("%s", message);

			LocalFree(message);
		}
		else
		{
			result = Format("Error code %i", errorCode);
		}

		return result;
	}

	//-----------------------------------------------------------------------------
	// Name: BankBuilder::CreateDirectories()
	// Desc: this may want to be a standalone function one day?
	//-----------------------------------------------------------------------------
	bool BankBuilder::CreateDirectories(const XtString& path)
	{
		const char* start = path;
		const char* drive = start;

		if ((drive[0] == '\\') && (drive[1] == '\\'))
		{
			while((*drive == '\\') || (*drive == '/'))
				drive++;

			while((*drive != '\\') && (*drive != '/') && (*drive != '\0'))
				drive++;
		}
		else
		{
			for(const char* p = path; *p; p++)
			{
				if (*p == ':')
				{
					drive = p + 1;
					break;
				}
			}
		}

		do
		{
			while((*drive == '\\') || (*drive == '/'))
				drive++;

			while((*drive != '\\') && (*drive != '/') && (*drive != '\0'))
				drive++;

			if (*drive == '\0')
				break;

			XtString subPath(start, (int)(drive - start));

			if (CreateDirectory(subPath, 0))
			{
				PrintMessage(eMsgNone, (Util::C_String)Util::C_String().Printf("Created directory: %s", (const char*)subPath));
			}
			else
			{
				if (GetLastError() != ERROR_ALREADY_EXISTS)
				{
					XtString error = GetWindowsErrorMessage(GetLastError());

					PrintMessage(eMsgWarning, (Util::C_String)Util::C_String().Printf("Failed to create directory: %s", (const char*)subPath));
					PrintMessage(eMsgWarning, (Util::C_String)Util::C_String().Printf("Windows error: '%s'", (const char*)error));

					return false;
				}
			}
		}
		while(*drive != '\0');

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: ProjectSettings::SetDomMonitorCallback
	// Desc: setup a callback which gets called when the project settings record
	//		 gets modified. this is not currently used as it doesnt work the 
	//		 way I wanted. I wanted it to update project settings every time you
	//		 changed any fields in the projectsettings record so when you went back
	//		 to the WaveBankMgr window it would automatically have the latest 
	//		 projectsettings info but it doesn't work like that. now I just update
	//		 when the window gains focus and before a build.
	//-----------------------------------------------------------------------------
	bool ProjectSettings::SetDomMonitorCallback(Xt::DomDb* db)
	{
		// check if already set the callback
		if (_domMonitor.callback != NULL)
			return true;

		// get project settings block
		for(Xt::DomDb::Iterator itPack(db); itPack; ++itPack)
		{
			for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
			{
				Xt::DomRecord* record = (Xt::DomRecord*)itRecord;

				LarvaSchema*	schema		= db->schema;
				LarvaType*		recordType	= TypeOf(schema, record);

				if (!recordType || (recordType->kind != kLtStruct))
					continue;

				if (Util::C_String((const char*)record->root.type) == Util::C_String("WaveBanksProjectSettings"))
				{
					Xt::Monitor(&_domMonitor, record);
					_domMonitor.SetCallback(DomCallback, &ProjectSettings::Get());
					return true;
				}
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	// Name: ProjectSettings::DomCallback
	// Desc: callback for when projectsettings record gets modified.
	//		 updates the ProjectSettings with the latest info from the db.
	//-----------------------------------------------------------------------------
	void ProjectSettings::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, ProjectSettings* pProjectSettings)
	{
		if (*monitor == event->from)
		{
			OutputDebugString("Updating project settings\n");
			Xt::DomRecord* pRecord = (Xt::DomRecord*) monitor->element;
			pProjectSettings->UpdateFromRecord(pRecord);
		}
	}

	//-----------------------------------------------------------------------------
	// Name: ProjectSettings::Update
	// Desc: update the project settings given a pointer to the db.
	//		 performs validation on the data and reports errors via asyncop or message box.
	//-----------------------------------------------------------------------------
	bool ProjectSettings::Update(Xt::DomDb* db)
	{
		// find the project settings record in the db
		// make sure there is only one
		bool bFound = false;
		for(Xt::DomDb::Iterator itPack(db); itPack; ++itPack)
		{
			for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
			{
				Xt::DomRecord* record = (Xt::DomRecord*)itRecord;

				LarvaSchema*	schema		= db->schema;
				LarvaType*		recordType	= TypeOf(schema, record);

				if (!recordType || (recordType->kind != kLtStruct))
					continue;

				if (Util::C_String((const char*)record->root.type) == Util::C_String("WaveBanksProjectSettings"))
				{
					if (bFound)
					{
						PrintMessage(eMsgError, Util::C_String("Found more than one 'WaveBanksProjectSettings' record. There should only be one."));
						return false;
					}

					UpdateFromRecord(record);
					bFound = true;
				}
			}
		}

		if (!bFound)
		{
			PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Could not find a 'WaveBanksProjectSettings' record. Please make one."));
			return false;
		}

		//if (Validate() == false)
		//	return false;

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: ProjectSettings::Update
	// Desc: update the project settings given a record. used in the DomMonitor callback.
	//       performs no validation.
	//-----------------------------------------------------------------------------
	bool ProjectSettings::UpdateFromRecord(Xt::DomRecord* pRecord)
	{
		if (pRecord == NULL)
			return false;

		OutputDebugString("Updating project settings\n");

		Xt::DomBlock* toBuildBlock = (Xt::DomBlock*)Xt::At(&pRecord->root.data, "PlatformsToBuild");
		if (!toBuildBlock)
		{
			PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Could not find the '%s' field in the 'WaveBanksProjectSettings' record.", (const char*)"PlatformsToBuild"));
			return false;
		}

		Util::C_String str = (const char*) Xt::AsText(Xt::At(&pRecord->root.data, XtString("WriteXmlToDisk")));
		_bWriteXml = strtol(str.Chars(), 0, 10) ? true: false;

		str = (const char*) Xt::AsText(Xt::At(&pRecord->root.data, XtString("DefaultQuantizeMethod")));
		int iMethod = LookupQuantizeMethod(str);
		if (iMethod != -1)
			_eQuantizeMethod = (eQuantizeMethod)iMethod;

		str = (const char*) Xt::AsText(Xt::At(&pRecord->root.data, XtString("QuantizeLoopsIfNeeded")));
		_bQuantizeEnabled = strtol(str.Chars(), 0, 10) ? true: false;

		str = (const char*) Xt::AsText(Xt::At(&pRecord->root.data, XtString("SampleRateConversionQuality")));
		int iQuality = LookupResamplerQuality(str);
		if (iQuality != -1)
			_iResamplerQuality = iQuality;

		_strSamplesRootPath = (const char*) Xt::AsText(Xt::At(&pRecord->root.data, kXtField_SettingSamplesRootPath));

		str = (const char*) Xt::AsText(Xt::At(&pRecord->root.data, XtString("WriteXmlToDisk")));
		_bWriteXml = strtol(str.Chars(), 0, 10) ? true: false;

		{
			Xt::DomBlock* pBlock = (Xt::DomBlock*)Xt::At(&pRecord->root.data, XtString("WavWritingSettings"));
			if (pBlock)
			{
				//PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Could not find the 'WavWritingSettings' field in the 'WaveBanksProjectSettings' record."));
				//return false;

				_bWriteBitDepthWavs = strtol(AsText(At(pBlock, XtString("WriteWavsToDiskAfterConvertingBitDepth"))), 0, 10) ? true : false;
				_bWriteResampledWavs = strtol(AsText(At(pBlock, XtString("WriteWavsToDiskAfterResampling"))), 0, 10) ? true : false;
				_bWriteQuantizedWavs = strtol(AsText(At(pBlock, XtString("WriteWavsToDiskAfterQuantizing"))), 0, 10) ? true : false;
				_bWriteEncodedFiles = strtol(AsText(At(pBlock, XtString("WriteEncodedFilesToDiskAfterEncoding"))), 0, 10) ? true : false;
				_bDeleteTempFiles = strtol(AsText(At(pBlock, XtString("CleanupWavFilesWhenBuildingBanks"))), 0, 10) ? true : false;
			}
		}

		for (int p=0; p<NumPlatforms; p++)
		{
			XtString str = "Settings" + XtString(PlatformName(p).Chars());
			Xt::DomBlock* platformBlock = (Xt::DomBlock*)Xt::At(&pRecord->root.data, str);
			if (!platformBlock)
			{
				PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Could not find the '%s' field in the 'WaveBanksProjectSettings' record.", (const char*)str));
				return false;
			}
			
			PlatformSettings(p).SetOutputPath((const char*)AsText(At(platformBlock, kXtField_SettingOutputPath)));
			PlatformSettings(p).SetDeploymentPath((const char*)AsText(At(platformBlock, kXtField_SettingDeploymentPath)));
			PlatformSettings(p).SetStreamingDeploymentPath((const char*)AsText(At(platformBlock, kXtField_SettingDeploymentPathStreaming)));

			PlatformSettings(p).SetNeedToCopyToKit(strtol(AsText(At(platformBlock, XtString("CopyToKit"))), 0, 10) ? true : false);

			str = XtString("Build") + XtString(PlatformName(p).Chars());
			PlatformSettings(p).SetBuildEnabled(strtol(AsText(At(toBuildBlock, str)), 0, 10) ? true : false);
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: ProjectSettings::Validate
	// Desc: 
	//-----------------------------------------------------------------------------
	bool ProjectSettings::Validate()
	{
		bool bBuildPlatformSet = false;
		Util::C_String directory, filename, extension;

		if (SamplesRootPath() == "")
		{
			PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Could not get directory from field '%s' in 'WaveBanksProjectSettings' record.", (const char*)kXtField_SettingSamplesRootPath));
			return false;
		}

		if (_eQuantizeMethod != eQuantizeInsertDelete && _eQuantizeMethod != eQuantizeResample)
		{
			PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Invalid quantize method %d.", _eQuantizeMethod));
			return false;
		}

		if (_iResamplerQuality < -1 || _iResamplerQuality > 4)
		{
			PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Invalid resampler quality %d.", _iResamplerQuality));
			return false;
		}

		for (int p=0; p<NumPlatforms; p++)
		{
			if (PlatformSettings(p).BuildEnabled())
			{
				if (PlatformSettings(p).OutputPath() == "")
				{
					PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Could not get directory from field '%s' for platform %s in 'WaveBanksProjectSettings' record.", (const char*)kXtField_SettingOutputPath, PlatformName(p).Chars()));
					return false;
				}
			}

			if (PlatformSettings(p).NeedToCopyToKit())
			{
				if (PlatformSettings(p).DeploymentPath() == "")
				{
					PrintMessage(eMsgError, (Util::C_String)Util::C_String().Printf("Could not get directory from field '%s' for platform %s in 'WaveBanksProjectSettings' record.", (const char*)kXtField_SettingDeploymentPath, PlatformName(p).Chars()));
					return false;
				}
			}

			if (PlatformSettings(p).BuildEnabled())
				bBuildPlatformSet = true;
		}

		if (!bBuildPlatformSet)
		{
			PrintMessage(eMsgError, Util::C_String("No build platforms are set in the 'WaveBanksProjectSettings' record"));
			return false;
		}

		return true;
	}

	////-----------------------------------------------------------------------------
	//// Name: FormatTime()
	//// Desc: Given a time in secs gives you a formatted string
	////-----------------------------------------------------------------------------
	//Util::C_String FormatTime(F32 fTimeInSecs)
	//{
	//	if (fTimeInSecs >= 60.f*60.f)
	//	{
	//		U32 uHours = (int)(fTimeInSecs/(60.f*60.f));
	//		U32 uMins = (int)(fmod(fTimeInSecs, (60.f*60.f))/60.f);
	//		return Util::C_String().Printf("%d hour%s %d min%s %d secs", uHours, uHours>1?"s":"", uMins, uMins>1?"s":"", (int)fmod(fTimeInSecs, 60.f));
	//	}

	//	if (fTimeInSecs >= 60.f)
	//	{
	//		U32 uMins = (int)(fTimeInSecs/60.f);
	//		return Util::C_String().Printf("%d min%s %d secs", uMins, uMins>1?"s":"", (int)fmod(fTimeInSecs, 60.f));
	//	}

	//	return Util::C_String().Printf("%.2f secs", fTimeInSecs);
	//}

	////-----------------------------------------------------------------------------
	//// Name: GetDefaultX360()
	//// Desc: 
	////-----------------------------------------------------------------------------
	//XtString GetDefaultX360()
	//{
	//	XtString result;

	//	const int kMaxChars = 256;

	//	if (char* p = result.Capture(kMaxChars, false))
	//	{
	//		DWORD length = kMaxChars;

	//		if (FAILED(DmGetXboxName(p, &length)))
	//		{
	//			_snprintf_s(p, kMaxChars, _TRUNCATE, "(unknown)");
	//			length = 9;
	//		}

	//		result.Truncate(length);
	//	}

	//	return result;
	//}

} //namespace Wb

