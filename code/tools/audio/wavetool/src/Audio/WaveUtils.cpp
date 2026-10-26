#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Xt/XtTool.h"
#include "BuildBankAsync.h"
#include "WaveBank.h"
#include "WaveFile.h"
#include "WaveBankIniFile.h"
#include "WaveBankWriter.h"
#include "WaveBundlerDll.h"
#include "ErrorReport.h"
#include "Debug.h"
#include "StopWatch.h"
#include "XtWin\AsyncOpWin.h"

#include "WaveUtils.h"

using namespace Audio;

namespace Wb
{

AsyncOpFuncStruct printPtrs;

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
// Name: OutputString
// Desc: print a string by passing it to the calling app's print function
//-----------------------------------------------------------------------------
void OutputString(AsyncOpIcon eType, const char* pchText, void* pContext)
{
	AsyncOpFuncStruct *ptrs = (AsyncOpFuncStruct*)pContext;

	// if we are printing a warning or error check if the last normal print
	// ended with a carriage return. if it hasnt then this means the warning 
	// will get printed out before the print string (as strings get buffered 
	// up until they get a carriage return and then printed out). this 
	// prevents that by printing out the print string first by forcing 
	// a carriage return.
	if (eType == kAoiWarning || eType == kAoiError)
	{
		if (strTmpPrint.Length() && strTmpPrint[strTmpPrint.Length()-1] != '\n')
		{
			strTmpPrint += Util::C_String("\n");
			Util::C_String strToPrint = GetPrintableString(strTmpPrint);
			printPtrs.ptrFunc(0, strToPrint, printPtrs.ptrContext); 
		}
	}

	Util::C_String& str = strTmpPrint;
	if (eType == kAoiWarning)
		str = strTmpWarn;
	else
	if (eType == kAoiError)
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
// Name: PrintError
// Desc: overridden error report function
//-----------------------------------------------------------------------------
void WbPrintError(const char* pchText, void* pContext)
{
	OutputString(kAoiError, pchText, pContext);
}

//-----------------------------------------------------------------------------
// Name: PrintWarning
// Desc: overridden warning report function
//-----------------------------------------------------------------------------
void WbPrintWarning(const char* pchText, void* pContext)
{
	OutputString(kAoiWarning, pchText, pContext);
}

//-----------------------------------------------------------------------------
// Name: Print
// Desc: overridden print function
//-----------------------------------------------------------------------------
void WbPrint(U32 uAttribs, const char* pchText, void* pContext)
{
	OutputString(kAoiInfo, pchText, pContext);
}

//-----------------------------------------------------------------------------
// Name: FormatTime()
// Desc: Given a time in secs gives you a formatted string
//-----------------------------------------------------------------------------
Util::C_String FormatTime(F32 fTimeInSecs)
{
	if (fTimeInSecs >= 60.f*60.f)
	{
		U32 uHours = (int)(fTimeInSecs/(60.f*60.f));
		U32 uMins = (int)(fmod(fTimeInSecs, (60.f*60.f))/60.f);
		return Util::C_String().Printf("%d hour%s %d min%s %d secs", uHours, uHours>1?"s":"", uMins, uMins>1?"s":"", (int)fmod(fTimeInSecs, 60.f));
	}

	if (fTimeInSecs >= 60.f)
	{
		U32 uMins = (int)(fTimeInSecs/60.f);
		return Util::C_String().Printf("%d min%s %d secs", uMins, uMins>1?"s":"", (int)fmod(fTimeInSecs, 60.f));
	}

	return Util::C_String().Printf("%.2f secs", fTimeInSecs);
}

//-----------------------------------------------------------------------------
// Name: GetDefaultX360()
// Desc: 
//-----------------------------------------------------------------------------
XtString GetDefaultX360()
{
	XtString result;

	const int kMaxChars = 256;

	if (char* p = result.Capture(kMaxChars, false))
	{
		DWORD length = kMaxChars;

		if (FAILED(DmGetXboxName(p, &length)))
		{
			_snprintf_s(p, kMaxChars, _TRUNCATE, "(unknown)");
			length = 9;
		}

		result.Truncate(length);
	}

	return result;
}

void SetDebugLogCallbacks(FuncNewAsyncOpMessage func, void* pContext)
{
	static AsyncOpFuncStruct errorPtrs;
	errorPtrs.ptrFunc = func;
	errorPtrs.ptrContext = pContext;
	SetErrorReportCallback(WbPrintError, &errorPtrs);	

	static AsyncOpFuncStruct warningPtrs;
	warningPtrs.ptrFunc = func;
	warningPtrs.ptrContext = pContext;
	SetWarningReportCallback(WbPrintWarning, &warningPtrs);

	printPtrs.ptrFunc = func;
	printPtrs.ptrContext = pContext;
	SetPrintStringCallback(WbPrint, &printPtrs);
}

//-----------------------------------------------------------------------------
// Name: BuildBankFromXmlInMemory
// Desc: 
//-----------------------------------------------------------------------------
bool BuildBankFromXmlInMemory(BuildBankParams& params)
{
	WaveBankIniFile iniFile;

	StopWatch sw;

	// setup global options vars
	gSettings.gbQuantizeEnabled =  params.bQuantize;
	gSettings.giQuantizeMethod = params.eQuantizeMethod;
	gSettings.giResamplerQuality = params.iResamplerQuality;

	gSettings.gbWriteIntermediateBitDepthFiles = params.bWriteBitDepthFiles;
	gSettings.gbWriteIntermediateResampledFiles = params.bWriteResampledFiles;
	gSettings.gbWriteIntermediateQuantizedFiles = params.bWriteQuantizedFiles;
	gSettings.gbWriteIntermediateEncodedFiles = params.bWriteEncodedFiles;
	gSettings.gbDeleteIntermediateFiles = params.bDeleteWavFiles;

	// setup error reporting functions so they get piped through  the AsyncOp thread functions so our debug is displayed on the output window
	SetDebugLogCallbacks(AsyncOpNewMessageFn, gpAsyncOp);

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

	writer.SetProgressUpdateCallback(AsyncOpSetProgressFn, gpAsyncOp);
	writer.SetProgressMinMax(params.fProgressMin, params.fProgressMax);

	writer.SetOutputPath(params.strOutputDir);

	if (writer.Write(iniFile)==false)
		return false;

	Audio::Print(0, "Building bank took %s.\n", FormatTime(sw.Time()).Chars());

	// reset the asyncop pointer to null. 
	// otherwise any prints called outside of the asyncop will be using an out of scope asyncop pointer
	SetDebugLogCallbacks(AsyncOpNewMessageFn, 0);

	return true;
}

//-----------------------------------------------------------------------------
// Name: QueryWaveFile
// Desc: 
//-----------------------------------------------------------------------------
eQueryWaveFileResult QueryWaveFile(WaveFileInfo& waveFileInfo)
{
	Audio::CWaveFile waveFile;

	/*if (structSize != sizeof(WaveFileInfo))
		return Wb::eFailedStructSizeMismatch;*/

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

const char* LookupQueryWavResultString(eQueryWaveFileResult eResult)
{
	return pchQueryWavFileResultStrings[eResult];
}

} // namespace Wb