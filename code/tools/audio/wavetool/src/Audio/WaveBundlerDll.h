//#ifndef WAVEBUNDLERDLL_H
//#define WAVEBUNDLERDLL_H
//
//#ifdef _MSC_VER
//#pragma once
//#endif
//
//#include <core\mercurycore.h>
//#include <core\containers.h>
//
//// function pointer type declarations
//typedef void (*FuncSetAsyncOpProgress)(F32 fProgress, void* pContext);
//typedef void (*FuncNewAsyncOpMessage)(int msgType, Util::C_String& str, void* pContext);
//
//
//
////-----------------------------------------------------------------------------
//// Name: WaveFileInfo
//// Desc: Used for QueryWaveFile
////-----------------------------------------------------------------------------
//struct WaveFileInfo
//{
//	WaveFileInfo()
//	{
//		nChannels = -1;
//		sampleRate = -1;
//		bLoop = -1;
//		bitDepth = -1;
//		bWavReadError = -1;
//	};
//
//	bool IsValid() { return !(nChannels == -1 || sampleRate == -1 && bLoop == -1 && bitDepth == -1); }
//
//	// pass this in
//	Util::C_String strFilename;
//
//	// these get passed back
//	int			nChannels;			
//	int			sampleRate;
//	int			bLoop;
//	int			bitDepth;
//	int			bWavReadError;
//};
//
//enum eQueryWaveFileResult
//{
//	// old function used to use bool so these enums trap old results
//	eOldResultFailed=0,
//	eOldResultSuccess=1,
//
//	eQuerySuccess=2,
//	eFailedDllLoad,
//	eFailedFunctionLookup,
//	eFailedStructSizeMismatch,
//	eFailedCouldNotOpenFile,
//	eFailedCouldNotGetFormat,
//	eFailedCouldNotGetLoopInfo,
//
//	eNumQueryWavResults,
//};
//
////-----------------------------------------------------------------------------
//// Name: BuildBankParams
//// Desc: Used for BuildBankFromXmlInMemory
////-----------------------------------------------------------------------------
//struct BuildBankParams
//{
//	BuildBankParams()
//	{
//		newMsgFunc = 0;
//		pContext = 0;
//		setProgressFunc = 0;
//		fProgressMin = 0.0f;
//		fProgressMax = 1.f;
//		pStrXmlText = 0;
//		bQuantize = true;
//		eQuantizeMethod = 0;	// resample
//		iResamplerQuality = 0;	// best quality
//
//		bWriteBitDepthFiles = false;
//		bWriteQuantizedFiles = false;
//		bWriteResampledFiles = false;
//		bWriteEncodedFiles = false;
//		bDeleteWavFiles = false;
//	};
//
//	Util::C_String* pStrXmlText;
//	Util::C_String strOutputDir;
//
//	void* pContext;
//	FuncNewAsyncOpMessage newMsgFunc;
//	FuncSetAsyncOpProgress setProgressFunc;
//	F32 fProgressMin;
//	F32 fProgressMax;
//
//	bool bQuantize;
//	int eQuantizeMethod;
//	int iResamplerQuality;
//
//	bool bWriteBitDepthFiles;
//	bool bWriteQuantizedFiles;
//	bool bWriteResampledFiles;
//	bool bWriteEncodedFiles;
//	bool bDeleteWavFiles;
//};
//
//typedef eQueryWaveFileResult (*FuncQueryWaveFile)(WaveFileInfo& waveFileInfo, int structSize);
//typedef bool (*FuncBuildBankFromXmlInMemory)(BuildBankParams& params, int structSize);
//
//
////-----------------------------------------------------------------------------
//// Name: Dll
////-----------------------------------------------------------------------------
//class Dll
//{
//public:
//	Dll() : _hModDll(0) {}
//
//public:
//	bool				LoadDll();
//	bool				Loaded() { return _hModDll != 0; }
//
//private:
//	HMODULE				DllHandle() { return _hModDll; }
//	void				SetFnName(int index, const char* pchName);
//
//	virtual const char* Filename() = 0;
//	virtual int			NumFunctions() = 0;
//	virtual const char*	FunctionName(int index) = 0;
//
//protected:
//	struct Func
//	{
//		Func() { fn = NULL; }
//		FARPROC fn;
//	};
//	FARPROC				GetFn(int index);
//
//private:
//
//	static bool			_bDoneDllNotFoundMessage;
//
//	HMODULE				_hModDll;								// handle to the dll
//
//	Ut::UtVector<Ut::DynamicVector<Func>>			_fns;		// list of FARPROCS (function pointers)
//};
//
////-----------------------------------------------------------------------------
//// Name: WaveBundlerDll
////-----------------------------------------------------------------------------
//class WaveBundlerDll : public Dll
//{
//public:
//	static WaveBundlerDll&	Get() 
//	{ 
//		static WaveBundlerDll instance;
//		return instance;
//	}
//	eQueryWaveFileResult QueryWaveFile(WaveFileInfo& waveFileInfo);
//	bool BuildBankFromXmlInMemory(BuildBankParams& params);
//	void SetAsyncOpContext(void* pVoid) { _pContext = pVoid; }
//private:
//	enum eFunctions
//	{
//		eFunc_QueryWaveFile=0,
//		eFunc_BuildBank,
//		eNumFunctions,
//	};
//	WaveBundlerDll() :
//		_pContext(0)
//	{
//	};
//	static const char* _pchFuncNames[eNumFunctions];
//	int NumFunctions() { return eNumFunctions; }
//	const char* FunctionName(int index) { return _pchFuncNames[index]; }
//	const char* Filename() { return "WaveBundler.dll"; }
//
//private:
//	void* _pContext;				// context for async op stuff
//};
//
//const char* LookupQueryWavResultString(eQueryWaveFileResult eResult);
//
//#endif // WAVEBUNDLERDLL_H
