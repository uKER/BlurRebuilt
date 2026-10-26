// WaveBundlerDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "WaveBank.h"
#include "WaveFile.h"
#include "WaveBankIniFile.h"
#include "WaveBankWriter.h"
#include "WaveBundlerDll.h"
#include "ErrorReport.h"
#include "Debug.h"
#include "StopWatch.h"

#define WBDLL_EXPORT extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
    return TRUE;
}

using namespace Audio;

struct AsyncOpFuncStruct
{
	FuncNewAsyncOpMessage ptrFunc;
	void* ptrContext;
};

enum eMessageType
{
	eMsgNormal = 0,
	eMsgWarning = 2,
	eMsgError = 3,
};

static AsyncOpFuncStruct printPtrs;

// strings used to buffer up debug strings. 
// strings are buffered until a carriage return is printed then the string up to that point is printed out.
// this is because the wavebundler does stuff like Print("Compressing..."); Print("[compressed to 3k]"); Print("Ok\n");
// and the print function in sorcery prints a new line for every single string.
static Util::C_String strTmpPrint;
static Util::C_String strTmpErr;
static Util::C_String strTmpWarn;

//-----------------------------------------------------------------------------
// Name: GetPrintableString
// Desc: return a string if there is a printable one. ie anything up to a 
//		 carriage return. strips the printable string from the string passed in
//-----------------------------------------------------------------------------
Util::C_String GetPrintableString(Util::C_String& str)
{
	for (;;)
	{
		int pos = str.Search("\n");
		
		// if found no carriage return quit without printing it out.
		// we wait until we get one and then print it out
		if (pos == -1)
			break;

		Util::C_String strToPrint = str(0, pos);

		// new string is now everything after the carriage return
		str = str(pos+1, str.Length());
		if (strToPrint != Util::C_String(""))
			return strToPrint;
	}

	return Util::C_String("");
}

//-----------------------------------------------------------------------------
// Name: DllOutputString
// Desc: print a string by passing it to the calling app's print function
//-----------------------------------------------------------------------------
void DllOutputString(eMessageType eType, const char* pchText, void* pContext)
{
	AsyncOpFuncStruct *ptrs = (AsyncOpFuncStruct*)pContext;

	// if we are printing a warning or error check if the last normal print
	// ended with a carriage return. if it hasnt then this means the warning 
	// will get printed out before the print string (as strings get buffered 
	// up until they get a carriage return and then printed out). this 
	// prevents that by printing out the print string first by forcing 
	// a carriage return.
	if (eType == eMsgWarning || eType == eMsgError)
	{
		if (strTmpPrint.Length() && strTmpPrint[strTmpPrint.Length()-1] != '\n')
		{
			strTmpPrint += Util::C_String("\n");
			Util::C_String strToPrint = GetPrintableString(strTmpPrint);
			printPtrs.ptrFunc(0, strToPrint, printPtrs.ptrContext); 
		}
	}

	Util::C_String& str = strTmpPrint;
	if (eType == eMsgWarning)
		str = strTmpWarn;
	else
	if (eType == eMsgError)
		str = strTmpErr;

	str += pchText;

	Util::C_String strToPrint;
	for (;;)
	{
		strToPrint = GetPrintableString(str);
		if (strToPrint == Util::C_String(""))
			break;
		// call the function to print the string
		ptrs->ptrFunc((int)eType, strToPrint, ptrs->ptrContext);
	}
}

//-----------------------------------------------------------------------------
// Name: DllPrintError
// Desc: overridden error report function
//-----------------------------------------------------------------------------
void DllPrintError(const char* pchText, void* pContext)
{
	DllOutputString(eMsgError, pchText, pContext);
}

//-----------------------------------------------------------------------------
// Name: DllPrintWarning
// Desc: overridden warning report function
//-----------------------------------------------------------------------------
void DllPrintWarning(const char* pchText, void* pContext)
{
	DllOutputString(eMsgWarning, pchText, pContext);
}

//-----------------------------------------------------------------------------
// Name: DllPrint
// Desc: overridden print function
//-----------------------------------------------------------------------------
void DllPrint(U32 uAttribs, const char* pchText, void* pContext)
{
	DllOutputString(eMsgNormal, pchText, pContext);
}

//-----------------------------------------------------------------------------
// Name: BuildBankFromXmlInMemory
// Desc: 
//-----------------------------------------------------------------------------
WBDLL_EXPORT bool BuildBankFromXmlInMemory(BuildBankParams& params)
{
	WaveBankIniFile iniFile;

	StopWatch sw;

	// setup global options vars
	gbQuantizeEnabled =  params.bQuantize;
	giQuantizeMethod = params.eQuantizeMethod;
	giResamplerQuality = params.iResamplerQuality;

	gbWriteIntermediateBitDepthFiles = params.bWriteBitDepthFiles;
	gbWriteIntermediateResampledFiles = params.bWriteResampledFiles;
	gbWriteIntermediateQuantizedFiles = params.bWriteQuantizedFiles;
	gbWriteIntermediateEncodedFiles = params.bWriteEncodedFiles;
	gbDeleteIntermediateFiles = params.bDeleteWavFiles;

	// setup error reporting functions so they get piped through 
	// the AsyncOp thread functions so our debug is displayed on the output window
	if (params.newMsgFunc)
	{
		static AsyncOpFuncStruct errorPtrs;
		errorPtrs.ptrFunc = params.newMsgFunc;
		errorPtrs.ptrContext = params.pContext;
		SetErrorReportCallback(DllPrintError, &errorPtrs);	

		static AsyncOpFuncStruct warningPtrs;
		warningPtrs.ptrFunc = params.newMsgFunc;
		warningPtrs.ptrContext = params.pContext;
		SetWarningReportCallback(DllPrintWarning, &warningPtrs);

		printPtrs.ptrFunc = params.newMsgFunc;
		printPtrs.ptrContext = params.pContext;
		SetPrintStringCallback(DllPrint, &printPtrs);
	}

	#ifdef _DEBUG
	Print(0, "[DEBUG BUILD] [DEBUG BUILD] [DEBUG BUILD] [DEBUG BUILD] [DEBUG BUILD] [DEBUG BUILD]\n");
	#endif

	if (params.pStrXmlText==NULL)
		return false;

	// load the xml ini file and parse it
	if (iniFile.Parse(params.pStrXmlText->Chars(), params.pStrXmlText->Length()) == false)
		return false;

	WaveBank bank(iniFile.BankName().Chars());
	WaveBankWriter writer(&bank);

	if (params.setProgressFunc)
	{
		writer.SetProgressUpdateCallback(params.setProgressFunc, params.pContext);
		writer.SetProgressMinMax(params.fProgressMin, params.fProgressMax);
	}

	writer.SetOutputPath(params.strOutputDir);

	if (writer.Write(iniFile)==false)
		return false;

	Audio::Print(0, "Building bank took %s.\n", FormatTime(sw.Time()).Chars());

	return true;
}

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
	eFailedCouldNotGetListInfo
};

//-----------------------------------------------------------------------------
// Name: QueryWaveFile
// Desc: 
//-----------------------------------------------------------------------------
WBDLL_EXPORT eQueryWaveFileResult QueryWaveFile(WaveFileInfo& waveFileInfo, int structSize)
{
	Audio::CWaveFile waveFile;

	if (structSize != sizeof(WaveFileInfo))
		return eFailedStructSizeMismatch;

	if (FAILED(waveFile.OpenForRead(waveFileInfo.strFilename.Chars())))
	{
		waveFileInfo.bWavReadError = true;
		return eFailedCouldNotOpenFile;
	}

	if (FAILED(waveFile.ReadFormat()))
	{
		waveFileInfo.bWavReadError = true;
		return eFailedCouldNotGetFormat;
	}
	
	if (FAILED(waveFile.ReadLoopInfo()))
	{
		waveFileInfo.bWavReadError = true;
		return eFailedCouldNotGetLoopInfo;
	}

	if (FAILED(waveFile.ReadCueInfo()))
	{
		waveFileInfo.bWavReadError = true;
		return eFailedCouldNotGetCueInfo;
	}

	if (FAILED(waveFile.ReadListInfo()))
	{
		waveFileInfo.bWavReadError = true;
		return eFailedCouldNotGetListInfo;
	}

	WAVEFORMATEX* pFormat = waveFile.GetFormatEx();
	
	if (!pFormat)
		return eFailedCouldNotGetFormat;

	waveFileInfo.bLoop = waveFile.IsLoop();
	waveFileInfo.nChannels = pFormat->nChannels;
	waveFileInfo.sampleRate = pFormat->nSamplesPerSec;
	waveFileInfo.bitDepth = pFormat->wBitsPerSample;

	waveFileInfo.bWavReadError = false;

	return eQuerySuccess;
}

