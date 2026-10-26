#ifndef WAVEUTILS_H
#define WAVEUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Xt/XtTool.h"

#include "WaveBank.h"
#include "StopWatch.h"

// wavebundler lib
#include "WaveFile.h"
#include "WaveBankIniFile.h"
#include "WaveBankWriter.h"
#include "WaveBundler.h"

#include "ErrorReport.h"
#include "Debug.h"
#include "XtWin\AsyncOpWin.h"

// xdk
#include <xbdm.h>

using namespace Audio;

namespace Wb
{
	typedef void (*FuncNewAsyncOpMessage)(int msgType, Util::C_String& str, void* pContext);

	struct AsyncOpFuncStruct
	{
		FuncNewAsyncOpMessage ptrFunc;
		void* ptrContext;
	};

	extern AsyncOpFuncStruct printPtrs;

	// function pointer type declarations
	typedef void (*FuncSetAsyncOpProgress)(F32 fProgress, void* pContext);
	typedef void (*FuncNewAsyncOpMessage)(int msgType, Util::C_String& str, void* pContext);

		//-----------------------------------------------------------------------------
	// Name: WaveFileInfo
	// Desc: Used for QueryWaveFile
	//-----------------------------------------------------------------------------
	struct WaveFileInfo
	{
		WaveFileInfo()
		{
			nChannels = -1;
			sampleRate = -1;
			bLoop = -1;
			bitDepth = -1;
			bWavReadError = -1;
		};

		bool IsValid() { return !(nChannels == -1 || sampleRate == -1 && bLoop == -1 && bitDepth == -1); }

		// pass this in
		Util::C_String strFilename;

		// these get passed back
		int			nChannels;			
		int			sampleRate;
		int			bLoop;
		int			bitDepth;
		int			bWavReadError;
	};

	enum eQueryWaveFileResult
	{
		// old function used to use bool so these enums trap old results
		eOldResultFailed=0,
		eOldResultSuccess=1,

		eQuerySuccess=2,
		eFailedDllLoad,
		eFailedFunctionLookup,
		eFailedStructSizeMismatch,
		eFailedCouldNotOpenFile,
		eFailedCouldNotGetFormat,
		eFailedCouldNotGetLoopInfo,
		eFailedCouldNotGetCueInfo,
		eFailedCouldNotGetListInfo,

		eNumQueryWavResults,
	};

	static const char* pchQueryWavFileResultStrings[eNumQueryWavResults] =
	{
		"Dll out of date",							
		"Dll out of date",
		"Succeeded",
		"Could not load dll",
		"Could not find function in dll",
		"Structure Size Mismatch",
		"Could Not Open File",
		"Could Not Get Format",
		"Could Not Get Loop Info",
	};



	//-----------------------------------------------------------------------------
	// Name: BuildBankParams
	// Desc: Used for BuildBankFromXmlInMemory
	//-----------------------------------------------------------------------------
	struct BuildBankParams
	{
		BuildBankParams()
		{
			//newMsgFunc = 0;
			//pAsyncOpContext = 0;
			//setProgressFunc = 0;
			fProgressMin = 0.0f;
			fProgressMax = 1.f;
			pStrXmlText = 0;
			bQuantize = true;
			eQuantizeMethod = 0;	// resample
			iResamplerQuality = 0;	// best quality

			bWriteBitDepthFiles = false;
			bWriteQuantizedFiles = false;
			bWriteResampledFiles = false;
			bWriteEncodedFiles = false;
			bDeleteWavFiles = false;
		};

		Util::C_String* pStrXmlText;
		Util::C_String strOutputDir;

		//void* pAsyncOpContext;
		//FuncNewAsyncOpMessage newMsgFunc;
		//FuncSetAsyncOpProgress setProgressFunc;
		F32 fProgressMin;
		F32 fProgressMax;

		bool bQuantize;
		int eQuantizeMethod;
		int iResamplerQuality;

		bool bWriteBitDepthFiles;
		bool bWriteQuantizedFiles;
		bool bWriteResampledFiles;
		bool bWriteEncodedFiles;
		bool bDeleteWavFiles;
	};

	bool BuildBankFromXmlInMemory(BuildBankParams& params);

	Util::C_String FormatTime(F32 fTimeInSecs);
	XtString GetDefaultX360();

	eQueryWaveFileResult QueryWaveFile(WaveFileInfo& waveFileInfo);
	const char* LookupQueryWavResultString(eQueryWaveFileResult eResult);

	void WbPrintError(const char* pchText, void* pContext);
	void WbPrintWarning(const char* pchText, void* pContext);
	void WbPrint(U32 uAttribs, const char* pchText, void* pContext);
}

#endif // WAVEUTILS_H
