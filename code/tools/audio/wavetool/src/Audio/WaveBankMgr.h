#ifndef WAVEBANK_H
#define WAVEBANK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\mercurycore.h>
#include <core\containers.h>
#include <system/misc/StringUtil.h>

namespace Wb
{

	Util::C_String PresetName(int index);
	Util::C_String FormatName(int index);
	Util::C_String PlatformName(int index);
	Util::C_String ResamplerQualityName(int index);
	Util::C_String QuantizeMethodName(int index);

	extern XtString kXtField_BankPresetId;
	extern XtString kXtField_WavePresetId;
	extern XtString kXtField_WaveId;
	extern XtString kXtField_WaveName;
	extern XtString kXtField_WavePath;
	extern XtString kXtField_WaveChannels;
	extern XtString kXtField_WaveSampleRateIn;
	extern XtString kXtField_WaveBitDepthIn;
	extern XtString kXtField_WaveLoopIn;
	extern XtString kXtField_Format;
	extern XtString kXtField_WaveSampleRateOut;
	extern XtString kXtField_WaveBitDepthOut;

	extern Util::C_String strPresetNone;

	Util::C_String ExtractBankName(Util::C_String str);
	Util::C_String FormatTime(F32 fTimeInSecs);

	struct WaveBankMgr;
	struct Wave;
	struct FormatPreset;
	struct PresetManager;

	typedef Ut::Vector<FormatPreset> PresetList;

	typedef void (*FuncPrintMessage)(int msgType, Util::C_String& str, void* pContext);
	typedef void (*FuncUpdateProgress)(F32 fProgress, void* pContext);

	enum ePresets
	{
		ePreset_None = -1,
		ePreset_InheritFromBank = -2,
	};

	enum ePlatform
	{
		ePlatform360 = 0,
		ePlatformPS3,
		ePlatformPC,
		NumPlatforms
	};

	enum eQuantizeMethod
	{
		eQuantizeDefault = -1,
		eQuantizeResample = 0,
		eQuantizeInsertDelete,
	};

	// is there a way round having a duplicate enum here to the one in the schema?
	enum eWaveFormat
	{
		eWaveFormatPCM = 0,
		eWaveFormatXMA,
		eWaveFormatXMA2,
		eWaveFormatADPCM,
		NumWaveFormats,
	};

	enum eBankBuildMode
	{
		eBuildCurrentBank=0,
		eBuildAllBanks,
		eBuildSubsetOfBanks,
	};

	enum eMsgType
	{
		eMsgNone=0,
		eMsgInfo,
		eMsgWarning,
		eMsgError,
	};

	//-----------------------------------------------------------------------------
	// Name: PlatformSettings
	//-----------------------------------------------------------------------------
	struct ProjectPlatformSettings
	{
		friend struct ProjectPlatformSettings;
	public:
		ProjectPlatformSettings() : bCopyToKit(false),
									bBuild(false) {} 

		Util::C_String	OutputPath() { return _strOutputPath; }
		Util::C_String	DeploymentPath() { return _strDeploymentPath; }
		Util::C_String	StreamingDeploymentPath() { return _strDeploymentPathStreaming; }
		bool			NeedToCopyToKit() { return bCopyToKit; }
		bool			BuildEnabled() { return bBuild; }

		void 			SetOutputPath(Util::C_String str) { _strOutputPath = Util::RemoveLeadingSlash(Util::RemoveTrailingSlash(str)); if (_strOutputPath.Length()) _strOutputPath += '\\'; }
		void 			SetDeploymentPath(Util::C_String str) { _strDeploymentPath = Util::RemoveLeadingSlash(Util::RemoveTrailingSlash(str)); if (_strDeploymentPath.Length()) _strDeploymentPath += '\\'; }
		void 			SetStreamingDeploymentPath(Util::C_String str) { _strDeploymentPathStreaming = Util::RemoveLeadingSlash(Util::RemoveTrailingSlash(str)); if (_strDeploymentPathStreaming.Length()) _strDeploymentPathStreaming += '\\'; }
		void 			SetNeedToCopyToKit(bool bCopy) { bCopyToKit = bCopy; }
		void 			SetBuildEnabled(bool bBool) { bBuild = bBool; }

	private:
		bool			bBuild;
		bool			bCopyToKit;
		Util::C_String 	_strOutputPath;								// where the baf files will be written to
		Util::C_String 	_strDeploymentPath;							// where the baf files will be copied to
		Util::C_String 	_strDeploymentPathStreaming;				// where the streaming baf files will be copied to
	};

	//-----------------------------------------------------------------------------
	// Name: ProjectSettings
	//-----------------------------------------------------------------------------
	struct ProjectSettings
	{
		// use a monitor so when project settings are modified we get a callback?

	public:
		static ProjectSettings&				Get() 
											{ 
												static ProjectSettings instance;
												return instance;
											}
											ProjectSettings() : _ePlatform(ePlatform360),
																_bWriteXml(false),
																_iResamplerQuality(0),
																_bQuantizeEnabled(true),
																_eQuantizeMethod(eQuantizeResample),
																_bWriteBitDepthWavs(false),
																_bWriteResampledWavs(false),
																_bWriteQuantizedWavs(false),
																_bWriteEncodedFiles(false),
																_bDeleteTempFiles(false),
																_printMsgContext(NULL) {}

		Util::C_String&						SamplesRootPath() { return _strSamplesRootPath; }

		ProjectPlatformSettings&			PlatformSettings(int platform) { return _settings[platform]; }

		bool								Update(Xt::DomDb* db);									// get the latest settings from the db
		bool								UpdateFromRecord(Xt::DomRecord* pRecord);

		bool								Validate();

		void								SetMessagePrintCallback(FuncPrintMessage fn, void* pContext=0) 
											{ 
												_pPrintMsgFn = fn; 
												_printMsgContext = pContext; 
											}
		bool								XmlWritingEnabled() { return _bWriteXml; }
		int									ResamplerQuality() { return _iResamplerQuality; }

		bool								QuantizeEnabled() { return _bQuantizeEnabled; }
		eQuantizeMethod						DefaultQuantizeMethod() { return _eQuantizeMethod; }

		bool								BitDepthWavWritingEnabled() { return _bWriteBitDepthWavs; }
		bool								ResampledWavWritingEnabled() { return _bWriteResampledWavs; }
		bool								QuantizedWavWritingEnabled() { return _bWriteQuantizedWavs; }
		bool								EncodedWavWritingEnabled() { return _bWriteEncodedFiles; }
		bool								WavDeletingEnabled() { return _bDeleteTempFiles; }

		ePlatform							PlatformCurr() { return _ePlatform; }
		void								SetPlatformCurr(ePlatform p) { _ePlatform = p; }

	public:
		static void							DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, ProjectSettings* pProjectSettings);
		bool								SetDomMonitorCallback(Xt::DomDb* db);
		
	private:
		void								PrintMessage(eMsgType msgType, Util::C_String& str) { if (_pPrintMsgFn) _pPrintMsgFn(msgType, str, _printMsgContext); }

	private:

		ePlatform							_ePlatform;								// which platform is currently enabled? ie selected in the dropdown in the bank pane.

		bool								_bWriteXml;
		int									_iResamplerQuality;
		bool								_bQuantizeEnabled;
		eQuantizeMethod						_eQuantizeMethod;

		bool								_bWriteBitDepthWavs;
		bool								_bWriteResampledWavs;
		bool								_bWriteQuantizedWavs;
		bool								_bWriteEncodedFiles;
		bool								_bDeleteTempFiles;

		static FuncPrintMessage				_pPrintMsgFn;							// callback for printing debug messages
		void*								_printMsgContext;						// an optional context to be passed through to the print message callback

		Ut::UtArrayVector<ProjectPlatformSettings, NumPlatforms>	_settings;
		Util::C_String												_strSamplesRootPath;

	//public:
		Xt::DomMonitor _domMonitor;
	};

	//-----------------------------------------------------------------------------
	// Name: FormatPreset
	//-----------------------------------------------------------------------------
	struct FormatPreset
	{
		friend struct PresetManager;

	public:

		FormatPreset()
		{
			eFormat = (eWaveFormat)-1;
			bitDepth = -1;
			sampleRate = -1;
			xmaQuality = -1;
			xmaHighFreqFilter = false;
			xtId = 0;
		};

		Util::C_String&		Name() { return name; }
		eWaveFormat			Format() { return eFormat; }
		int					BitDepth() { return bitDepth; }
		int					SampleRate() { return sampleRate; }
		XtId				GetXtId() { return xtId; }
		int					XmaQuality() { return xmaQuality; }
		bool				XmaHighFreqFilterEnabled() { return xmaHighFreqFilter; }

		Xt::DomRecord*		GetRecord(Xt::DomDb* db);

		void 				SetFormat(eWaveFormat format) { if (format >= 0 && format < NumWaveFormats) eFormat = format; }
		void 				SetBitDepth(int depth) { bitDepth = depth; }
		void 				SetSampleRate(int rate) { sampleRate = rate; }
		void 				SetXmaQuality(int quality) { xmaQuality = quality; }
		void 				SetXmaHighFreqFilterEnabled(bool bEnabled) { xmaHighFreqFilter = bEnabled; }

	private:
		Util::C_String		name;
		eWaveFormat 		eFormat;
		int					bitDepth;
		int					sampleRate;

		// xma only
		int					xmaQuality;
		bool				xmaHighFreqFilter;

		XtId				xtId;
		// store Xt::DomMonitor here so we can detect when preset has changed
	};

	//-----------------------------------------------------------------------------
	// Name: PresetManager
	//-----------------------------------------------------------------------------
	struct PresetManager
	{
	public:
		static PresetManager& Get() 
		{ 
			static PresetManager instance;
			return instance;
		}

	private:
						PresetManager();

	public:
		void			PopulateFromDom(Xt::DomDb* db);
		void			AddPreset(Xt::DomDb* db, Xt::DomRecord* record, int platform);		

		PresetList&		Presets(int platform = ProjectSettings::Get().PlatformCurr()) { return list[platform]; }
		int				DefaultPresetIndex(int platform = ProjectSettings::Get().PlatformCurr()) { return defaultPresetIndex[platform];  }
		FormatPreset*	DefaultPreset(int platform = ProjectSettings::Get().PlatformCurr()) { if (DefaultPresetIndex() == -1) return NULL; return &(Presets(platform)[DefaultPresetIndex()]); }
		FormatPreset*	Preset(int index, int platform = ProjectSettings::Get().PlatformCurr()) { if (index < 0 || index >= Presets(platform).Size()) return NULL; return &(Presets(platform)[index]); }
		FormatPreset*	NonePreset(int platform = ProjectSettings::Get().PlatformCurr()) { return &nonePreset[platform]; }
		int				LookupIndex(XtId presetId, int platform = ProjectSettings::Get().PlatformCurr());

	private:
		void			SetDefaultPresetIndex(int platform, int presetIndex) { defaultPresetIndex[platform] = presetIndex;  }

		Ut::UtArrayVector<FormatPreset, NumPlatforms>	nonePreset;				// the format settings to use if preset selected was "None"
		Ut::UtArrayVector<int, NumPlatforms>			defaultPresetIndex;		// index of the special case "Default" preset record from the db
		Ut::UtArrayVector<PresetList, NumPlatforms>		list;					// array of presets. one for each platform
	};



	//-----------------------------------------------------------------------------
	// Name: Wave
	//-----------------------------------------------------------------------------
	struct Wave
	{
		friend struct WaveBankMgr;	
	private:
		Wave() {};

	public:

		enum eFileStatus
		{
			eNotLooked=-1,
			eNotFoundOnDisk=0,
			eFoundGotInfo,
			eFoundNotGotInfo,
		};

		Wave(WaveBankMgr*);
		void				SetPreset(int preset, int platform = ProjectSettings::Get().PlatformCurr());
		int					PresetIndex(int platform = ProjectSettings::Get().PlatformCurr()) { return _presetIndex[platform]; }
		FormatPreset*		Preset(int platform = ProjectSettings::Get().PlatformCurr());
		
		Util::C_String		Name() { return Util::RemoveFileExtension(_filename); }
		Util::C_String&		Filename() { return _filename; }
		Util::C_String&		Path() { return _path; }

		bool				Loop() { return _bLoopIn; }
		int					Channels() { return _nChannels; }
		int					BitDepth() { return _bitDepthIn; }
		int					SampleRate() { return _sampleRateIn; }
		int					Id() { return _uniqueId; }
		eQuantizeMethod		QuantizeMethod() { return _eQuantizeMethod; }	// IF DEFAULT RETURN SETTINGS FROM PROJECTSETTINGS?

		void				SetLoop(bool bLoop) { _bLoopIn = bLoop; }
		void				SetChannels(int num) { _nChannels = num; }
		void				SetBitDepth(int depth) { _bitDepthIn = depth; }
		void				SetSampleRate(int rate) { _sampleRateIn = rate; }
		void				SetQuantizeMethod(eQuantizeMethod method) { _eQuantizeMethod = method; }

		eFileStatus			FileStatus() { return _eFileStatus; }

	private:
		Util::C_String		_filename;
		Util::C_String		_path;
		bool				_bLoopIn;
		int					_nChannels;
		int					_bitDepthIn;
		int					_sampleRateIn;
		eQuantizeMethod		_eQuantizeMethod;		

		eFileStatus			_eFileStatus;

		Ut::UtArrayVector<FormatPreset, NumPlatforms> _customPreset;			// for when ePreset_None is selected.

		int					_uniqueId;

		WaveBankMgr*			_pBank;

		Ut::UtArrayVector<int, NumPlatforms> _presetIndex;
	};

	//-----------------------------------------------------------------------------
	// Name: WaveBank
	//-----------------------------------------------------------------------------
	struct WaveBankMgr
	{
	public:
							WaveBankMgr()
							{
								bStreaming = false;
								for (int p=0; p<NumPlatforms; p++)
								{
									presetIndex[p] = ePreset_None;
									pPreset[p] = NULL;
								}
							}
							~WaveBankMgr()	{ DestroyWaves(); }

		Wave*				CreateWave() { return new Wave(this); }
		bool				AddWave(Wave* pWave);
		void				DestroyWave(int i) { delete Waves(i); waves.EraseByIndex(i); }
		XtId				MakeUniqueId();
		
		int					PresetIndex(int platform = ProjectSettings::Get().PlatformCurr() );
		void				SetPreset(int index, int platform = ProjectSettings::Get().PlatformCurr());

		int					NumWaves() const { return waves.Size(); }
		Wave*				Waves(int index) const { return waves[index]; }
		Wave*				Lookup(char* pchName);
		int					LookupIndex(int id);
		void				DestroyWaves();
		Util::C_String		GetCommonPath();
		void				RescanWavFiles();
		
		FormatPreset*		Preset(int platform = ProjectSettings::Get().PlatformCurr()) { return pPreset[platform]; }
		Util::C_String		FileName() { return name + Util::C_String(".baf"); }
		void				SetName(Util::C_String& pName) { name = pName; }
		Util::C_String&		Name() { return name; }
		Util::C_String&		FullDbName() { return dbName; }
		bool				IsStreaming() { return bStreaming; }
		bool				FoundAllWavFiles();

		void				SetStreaming(bool bEnabled) { bStreaming = bEnabled; }

		bool				PopulateFromDom(Xt::DomRecord* record);
		bool				WriteToDom(Xt::DomRecord* record);

		WaveBankMgr&			operator = (const WaveBankMgr& w); 

	private:

		// dont forget to deal with add any new data members in the = operator

		Util::C_String		name;						// the name of the bank as it will be written to disk. 
		Util::C_String		dbName;						// the name of the record in the db
		bool				bStreaming;
		
		Ut::Vector<Wave*>	waves;
		
		Ut::UtArrayVector<int, NumPlatforms>			presetIndex;
		Ut::UtArrayVector<FormatPreset*, NumPlatforms>	pPreset;
	};

	//-----------------------------------------------------------------------------
	// Name: BuilderPlatformSettings
	//-----------------------------------------------------------------------------
	struct BuilderPlatformSettings
	{
	public:
						BuilderPlatformSettings() : _bCopyToKit(false), _bBuild(false) {}
		bool			NeedToCopyToKit() { return _bCopyToKit; }
		bool			BuildEnabled() { return _bBuild; }
		Util::C_String& OutputPath() { return _strOutputPath; }
		Util::C_String& DeploymentPath() { return _strDeploymentPath; }
		Util::C_String& StreamingDeploymentPath() { return _strDeploymentPathStreaming; }

		void 			SetCopyToKit(bool bCopy) { _bCopyToKit = bCopy; }
		void 			SetBuildEnabled(bool bBuild) { _bBuild = bBuild; }
		void 			SetOutputPath(Util::C_String str) { _strOutputPath = str; }
		void 			SetDeploymentPath(Util::C_String str) { _strDeploymentPath = str; }
		void 			SetStreamingDeploymentPath(Util::C_String str) { _strDeploymentPathStreaming = str; }

	private:
		bool 			_bCopyToKit;		
		bool 			_bBuild;	
		Util::C_String _strOutputPath;		
		Util::C_String _strDeploymentPath;		
		Util::C_String _strDeploymentPathStreaming;		
	};

	//-----------------------------------------------------------------------------
	// Name: BankBuilder
	//-----------------------------------------------------------------------------
	struct BankBuilder
	{
	public:
		enum Stage
		{
			eStageIdle,
			eStageBuild,
			eStageDeploy,
		};
		
		static const F32			fBuildStageProgressBarRatio;

									~BankBuilder() { ClearBankList(); }

		static BankBuilder& 		Get() 
									{ 
										static BankBuilder instance;
										return instance;
									}
		void						SetMessagePrintCallback(FuncPrintMessage fn, void* pContext=0) 
									{ 
										_pPrintMsgFn = fn; 
										_printMsgContext = pContext; 
									}
		void						SetProgressUpdateCallback(FuncUpdateProgress fn, void* pContext=0) 
									{ 
										_pUpdateProgressFn = fn; 
										_updateProgressContext = pContext; 
									}
		int 						NumBanksBuilt() { return _numBanksBuilt; }											
		int 						NumBanksInList() { return _banksToBuild.Size(); }
		int 						TotalNumBanksToBuild() { return NumBanksInList() * NumPlatformsToBuild(); }				// num banks to build taking into account how many platforms to build
		int 						NumPlatformsToBuild();
		int							NumPlatformsToDeploy();
		int 						TotalNumBanksToDeploy() { return NumBanksInList() * NumPlatformsToDeploy(); }				// num banks to build taking into account how many platforms to build

		BuilderPlatformSettings&	PlatformSettings(int platform) { return _platformSettings[platform]; }

		bool 						Build(WaveBankMgr& pBank);												// build one bank for all platforms
		bool 						Build(WaveBankMgr* pBank, ePlatform platform);							// build one bank for one platform
		bool 						BuildAll(Xt::DomDb* db, Util::C_String& strFilter=Util::C_String(""));	// build all banks. optional filter can be specified to build bank subsets
		
		bool						Deploy();															// copy to platform's game image

		bool						CreateDirectoriesX360(const XtString& path);
		bool						CreateDirectories(const XtString& path);
		
	private:
									BankBuilder();

		void 						ClearBankList();
		bool 						Build();												// all build banks in the list
		void 						AddToBankList(WaveBankMgr* pBank) { _banksToBuild.PushBack(pBank); }
		void 						AddBank(Xt::DomDb* db, Xt::DomRecord* record, Util::C_String& strFilter);
		void 						PrintMessage(eMsgType type, Util::C_String& str) 
									{ 
										if (_pPrintMsgFn) 
											_pPrintMsgFn(type, str, _printMsgContext); 
									}
		F32							ProgressBarBuildStageRatio() { return NumPlatformsToDeploy() ? fBuildStageProgressBarRatio : 1.0f; }	// how much of the progress bar does the build stage take up?
		void						SetProgress(F32 fProgress);
		static void					DefaultPrintMsgFn(int msgType, Util::C_String& str, void* pContext);
		bool 						CreateXml(WaveBankMgr* pBank, ePlatform ePlatform, Util::C_String& str);
		bool 						CheckForDuplicateBankNames();
		
	private:

		static FuncPrintMessage		_pPrintMsgFn;						// callback for printing debug messages
		static FuncUpdateProgress	_pUpdateProgressFn;					// callback for setting the progress of the build process
		void*						_printMsgContext;					// an optional context to be passed through to the print message callback
		void*						_updateProgressContext;				// an optional context to be passed through to the update progress callback

		Stage						_curStage;							

		int							_numBanksBuilt;						// num banks built last time a build was set off. if this is 0 then the build failed

		Ut::Vector<WaveBankMgr*>		_banksToBuild;						// temporary list of banks to build

		Ut::UtArrayVector<BuilderPlatformSettings, NumPlatforms> _platformSettings;		// settings per platform
	};

}	// namespace Wb

#endif // WAVEBANK_H
